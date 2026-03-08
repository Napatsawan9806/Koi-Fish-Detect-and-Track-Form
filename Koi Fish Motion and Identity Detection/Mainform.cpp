#include "MainForm.h"
#include <cmath>

namespace KoiTracker {

    // =====================================================
    //  WIRE EVENTS
    // =====================================================

    void MainForm::WireEvents() {
        _service->OnFrameBitmap += gcnew KoiTracker::FrameBitmapHandler(
            this, &MainForm::OnFrameBitmap);

        _btnStart->Click += gcnew EventHandler(this, &MainForm::BtnStart_Click);
        _btnPause->Click += gcnew EventHandler(this, &MainForm::BtnPause_Click);
        _btnStop->Click += gcnew EventHandler(this, &MainForm::BtnStop_Click);
        _btnHeatmap->Click += gcnew EventHandler(this, &MainForm::BtnHeatmap_Click);
        _btnSchooling->Click += gcnew EventHandler(this, &MainForm::BtnSchooling_Click);

        _pictureBox->Paint += gcnew PaintEventHandler(this, &MainForm::OnPictureBoxPaint);
        _pictureBox->Click += gcnew EventHandler(this, &MainForm::OnPictureBoxClick);

        _grid->CellDoubleClick += gcnew DataGridViewCellEventHandler(this, &MainForm::OnGridDoubleClick);

        _service->OnFrameUpdated += gcnew FrameUpdatedHandler(this, &MainForm::OnFrameUpdated);
        _service->OnFishDetected += gcnew FishEventHandler(this, &MainForm::OnFishDetected);
        _service->OnFishLost += gcnew FishEventHandler(this, &MainForm::OnFishLost);
        _service->OnFpsUpdated += gcnew FpsUpdatedHandler(this, &MainForm::OnFpsUpdated);
        this->Resize += gcnew EventHandler(this, &MainForm::OnFormResize);
    }

    // =====================================================
    //  FRAME UPDATE
    // =====================================================

    void MainForm::OnFrameUpdated(List<FishTrack^>^ fishes) {
        if (this->InvokeRequired) {
            this->Invoke(gcnew FrameUpdatedHandler(this, &MainForm::OnFrameUpdated), fishes);
            return;
        }

        _currentFishes = fishes;
        AccumulateHeatmap(fishes);
        _pictureBox->Invalidate();

        UpdateGrid(fishes);
        UpdateStats(fishes);

        if (_detailForm != nullptr && !_detailForm->IsDisposed) {
            int detailId = (int)_detailForm->Tag;
            for each (FishTrack ^ f in fishes)
                if (f->FishID == detailId) { _detailForm->UpdateData(f); break; }
        }

        int active = 0;
        for each (FishTrack ^ f in fishes)
            if (f->Status != FishStatus::Lost) active++;
        _lblFishCount->Text = String::Format("Fish: {0}", active);
    }

    void MainForm::OnFishDetected(FishTrack^ fish) {
        SafeLog(String::Format("[{0}] NEW  #{1}", DateTime::Now.ToString("HH:mm:ss"), fish->FishID));
    }

    void MainForm::OnFishLost(FishTrack^ fish) {
        SafeLog(String::Format("[{0}] LOST #{1}", DateTime::Now.ToString("HH:mm:ss"), fish->FishID));
    }

    void MainForm::OnFpsUpdated(int fps) {
        if (this->InvokeRequired) {
            this->Invoke(gcnew FpsUpdatedHandler(this, &MainForm::OnFpsUpdated), fps);
            return;
        }
        _lblFps->Text = String::Format("FPS: {0}", fps);
    }

    void MainForm::OnFormResize(Object^ sender, EventArgs^ e) {
        _rightPanel->Left = this->ClientSize.Width - _rightPanel->Width - 5;
        _videoPanel->Width = this->ClientSize.Width - _rightPanel->Width - 20;
        _pictureBox->Width = _videoPanel->Width - 20;
        _logPanel->Width = this->ClientSize.Width - 10;
        _logPanel->Top = this->ClientSize.Height - _logPanel->Height - 5;
        _listLog->Width = _logPanel->Width - 10;
    }

    // =====================================================
    //  DRAWING
    // =====================================================

    void MainForm::DrawPondBackground(Graphics^ g) {}

    void MainForm::OnPictureBoxPaint(Object^ sender, PaintEventArgs^ e) {
        if (_currentFishes == nullptr || _currentFishes->Count == 0) return;
        if (_pictureBox->Image == nullptr) return;

        Graphics^ g = e->Graphics;
        g->SmoothingMode = Drawing2D::SmoothingMode::AntiAlias;

        // 1) heatmap (ชั้นล่างสุด)
        DrawHeatmap(g);

        float scaleX = (float)_pictureBox->Width / _pictureBox->Image->Width;
        float scaleY = (float)_pictureBox->Height / _pictureBox->Image->Height;

        // 2) schooling lines
        DrawSchooling(g, scaleX, scaleY);

        // 3) trajectory (ชั้นบนสุด)
        for each (FishTrack ^ fish in _currentFishes) {
            if (fish->Status == FishStatus::Lost) continue;
            List<PointF>^ pts = fish->Trajectory;
            if (pts->Count < 2) continue;
            for (int i = 1; i < pts->Count; i++) {
                float alpha = (float)i / pts->Count;
                PointF p0 = PointF(pts[i - 1].X * scaleX, pts[i - 1].Y * scaleY);
                PointF p1 = PointF(pts[i].X * scaleX, pts[i].Y * scaleY);
                Pen^ pen = gcnew Pen(Color::FromArgb((int)(alpha * 200), fish->TrackColor), 2.0f);
                g->DrawLine(pen, p0, p1);
                delete pen;
            }
        }
    }

    void MainForm::OnPictureBoxClick(Object^ sender, EventArgs^ e) {
        MouseEventArgs^ me = safe_cast<MouseEventArgs^>(e);
        if (_pictureBox->Image == nullptr) return;

        float scaleX = (float)_pictureBox->Image->Width / _pictureBox->Width;
        float scaleY = (float)_pictureBox->Image->Height / _pictureBox->Height;
        float videoX = me->X * scaleX;
        float videoY = me->Y * scaleY;

        for each (FishTrack ^ fish in _currentFishes) {
            if (fish->Status == FishStatus::Lost) continue;
            if (fish->BoundingBox.Contains(videoX, videoY)) {
                Log(String::Format("[{0}] Click #{1} conf={2}",
                    DateTime::Now.ToString("HH:mm:ss"), fish->FishID, fish->ConfidenceText));
                if (_detailForm == nullptr || _detailForm->IsDisposed) {
                    _detailForm = gcnew FishDetailForm(fish);
                    _detailForm->Tag = fish->FishID;
                    _detailForm->Show(this);
                }
                else {
                    _detailForm->UpdateData(fish);
                    _detailForm->Tag = fish->FishID;
                    _detailForm->BringToFront();
                }
                break;
            }
        }
    }

    // =====================================================
    //  SCHOOLING
    // =====================================================

    void MainForm::DrawSchooling(Graphics^ g, float scaleX, float scaleY) {
        if (!_showSchooling) return;

        // รัศมี schooling ในหน่วย video pixel
        const float SCHOOL_RADIUS = 120.0f;

        // วาดเส้นเชื่อมระหว่างปลาที่อยู่ใกล้กัน
        for (int i = 0; i < _currentFishes->Count; i++) {
            FishTrack^ a = _currentFishes[i];
            if (a->Status == FishStatus::Lost) continue;

            for (int j = i + 1; j < _currentFishes->Count; j++) {
                FishTrack^ b = _currentFishes[j];
                if (b->Status == FishStatus::Lost) continue;

                float dx = a->Center.X - b->Center.X;
                float dy = a->Center.Y - b->Center.Y;
                float dist = (float)Math::Sqrt((double)(dx * dx + dy * dy));

                if (dist <= SCHOOL_RADIUS) {
                    // ยิ่งใกล้ยิ่ง opaque
                    float t = 1.0f - (dist / SCHOOL_RADIUS);
                    int alpha = (int)(t * 180);

                    PointF pa = PointF(a->Center.X * scaleX, a->Center.Y * scaleY);
                    PointF pb = PointF(b->Center.X * scaleX, b->Center.Y * scaleY);

                    Pen^ pen = gcnew Pen(Color::FromArgb(alpha, 120, 255, 180), 1.5f);
                    pen->DashStyle = Drawing2D::DashStyle::Dash;
                    g->DrawLine(pen, pa, pb);
                    delete pen;

                    // วงกลมเล็กที่ center ของปลาแต่ละตัว
                    SolidBrush^ dot = gcnew SolidBrush(Color::FromArgb(alpha, 120, 255, 180));
                    g->FillEllipse(dot, pa.X - 4.0f, pa.Y - 4.0f, 8.0f, 8.0f);
                    g->FillEllipse(dot, pb.X - 4.0f, pb.Y - 4.0f, 8.0f, 8.0f);
                    delete dot;
                }
            }
        }
    }

    // =====================================================
    //  HEATMAP
    // =====================================================

    void MainForm::AccumulateHeatmap(List<FishTrack^>^ fishes) {
        if (!_showHeatmap) return;
        if (_heatmapData == nullptr)
            _heatmapData = gcnew array<int, 2>(_pictureBox->Height, _pictureBox->Width);
        if (_pictureBox->Image == nullptr) return;

        float scaleX = (float)_pictureBox->Width / _pictureBox->Image->Width;
        float scaleY = (float)_pictureBox->Height / _pictureBox->Image->Height;

        for each (FishTrack ^ fish in fishes) {
            if (fish->Status == FishStatus::Lost) continue;
            int px = (int)(fish->Center.X * scaleX);
            int py = (int)(fish->Center.Y * scaleY);
            int radius = 20;
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int nx = px + dx, ny = py + dy;
                    if (nx < 0 || ny < 0 || nx >= _pictureBox->Width || ny >= _pictureBox->Height) continue;
                    float dist = (float)Math::Sqrt((double)(dx * dx + dy * dy));
                    if (dist < radius)
                        _heatmapData[ny, nx] += (int)((1.0f - dist / radius) * 5);
                }
            }
        }
    }

    void MainForm::DrawHeatmap(Graphics^ g) {
        if (!_showHeatmap || _heatmapData == nullptr) return;

        int maxVal = 1;
        for (int y = 0; y < _pictureBox->Height; y++)
            for (int x = 0; x < _pictureBox->Width; x++)
                if (_heatmapData[y, x] > maxVal) maxVal = _heatmapData[y, x];

        int step = 4;
        for (int y = 0; y < _pictureBox->Height; y += step) {
            for (int x = 0; x < _pictureBox->Width; x += step) {
                float t = Math::Min(1.0f, (float)_heatmapData[y, x] / maxVal);
                if (t < 0.05f) continue;
                int alpha = (int)(t * 160);
                int r = (int)(255 * Math::Min(1.0f, t * 2));
                int g2 = (int)(255 * Math::Max(0.0f, 1.0f - Math::Abs(t - 0.5f) * 2));
                int b = (int)(255 * Math::Max(0.0f, 1.0f - t * 2));
                SolidBrush^ brush = gcnew SolidBrush(Color::FromArgb(alpha, r, g2, b));
                g->FillRectangle(brush, x, y, step, step);
                delete brush;
            }
        }
    }

    // =====================================================
    //  GRID
    // =====================================================

    void MainForm::UpdateGrid(List<FishTrack^>^ fishes) {
        _grid->Rows->Clear();
        for each (FishTrack ^ f in fishes) {
            int row = _grid->Rows->Add(
                f->StatusIcon,
                String::Format("#{0}", f->FishID),
                f->ConfidenceText,
                String::Format("{0:F1}", f->Speed),
                f->FirstSeen.ToString("HH:mm:ss")
            );
            _grid->Rows[row]->Tag = f->FishID;

            Color rowColor = (f->Status == FishStatus::Lost) ? Color::FromArgb(50, 40, 40)
                : (f->Status == FishStatus::New) ? Color::FromArgb(40, 50, 35)
                : Color::FromArgb(28, 31, 42);
            _grid->Rows[row]->DefaultCellStyle->BackColor = rowColor;

            Color confColor = (f->Confidence > 0.85f) ? Color::FromArgb(100, 220, 120)
                : (f->Confidence > 0.70f) ? Color::FromArgb(220, 200, 80)
                : Color::FromArgb(220, 100, 80);
            _grid->Rows[row]->Cells["ColConf"]->Style->ForeColor = confColor;
        }
    }

    void MainForm::OnGridDoubleClick(Object^ sender, DataGridViewCellEventArgs^ e) {
        if (e->RowIndex < 0) return;
        int id = (int)_grid->Rows[e->RowIndex]->Tag;
        for each (FishTrack ^ f in _currentFishes) {
            if (f->FishID == id) {
                if (_detailForm == nullptr || _detailForm->IsDisposed) {
                    _detailForm = gcnew FishDetailForm(f);
                    _detailForm->Tag = f->FishID;
                    _detailForm->Show(this);
                }
                else {
                    _detailForm->UpdateData(f);
                    _detailForm->Tag = f->FishID;
                    _detailForm->BringToFront();
                }
                break;
            }
        }
    }

    // =====================================================
    //  STATS
    // =====================================================

    void MainForm::UpdateStats(List<FishTrack^>^ fishes) {
        int active = 0, lost = 0, total = 0;
        for each (FishTrack ^ f in fishes) {
            if (f->Status == FishStatus::Active) active++;
            if (f->Status == FishStatus::Lost)   lost++;
            if (f->FishID > total) total = f->FishID;
        }
        _lblStatActive->Text = active.ToString();
        _lblStatLost->Text = lost.ToString();
        _lblStatTotal->Text = total.ToString();

        // นับจำนวนคู่ที่ schooling
        if (_showSchooling) {
            const float SCHOOL_RADIUS = 120.0f;

            // Union-Find แบบง่าย — ใช้ Dictionary แทน
            Dictionary<int, int>^ group = gcnew Dictionary<int, int>();
            int groupId = 0;

            for (int i = 0; i < fishes->Count; i++) {
                if (fishes[i]->Status == FishStatus::Lost) continue;
                if (!group->ContainsKey(i)) group[i] = groupId++;

                for (int j = i + 1; j < fishes->Count; j++) {
                    if (fishes[j]->Status == FishStatus::Lost) continue;
                    float dx = fishes[i]->Center.X - fishes[j]->Center.X;
                    float dy = fishes[i]->Center.Y - fishes[j]->Center.Y;
                    if (Math::Sqrt((double)(dx * dx + dy * dy)) <= SCHOOL_RADIUS) {
                        // j เข้ากลุ่มเดียวกับ i
                        group[j] = group[i];
                    }
                    else {
                        if (!group->ContainsKey(j)) group[j] = groupId++;
                    }
                }
            }

            // นับกลุ่มที่มีสมาชิกมากกว่า 1 ตัว
            Dictionary<int, int>^ groupSize = gcnew Dictionary<int, int>();
            for each (KeyValuePair<int, int> kv in group) {
                if (!groupSize->ContainsKey(kv.Value)) groupSize[kv.Value] = 0;
                groupSize[kv.Value]++;
            }
            int schoolGroups = 0;
            for each (KeyValuePair<int, int> kv in groupSize)
                if (kv.Value > 1) schoolGroups++;

            _lblStatSpecies->Text = String::Format("Schooling groups: {0}", schoolGroups);
        }
    }

    // =====================================================
    //  BUTTON HANDLERS
    // =====================================================

    void MainForm::BtnStart_Click(Object^ sender, EventArgs^ e) { StartTracking(); }
    void MainForm::BtnStop_Click(Object^ sender, EventArgs^ e) { StopTracking(); }
    void MainForm::BtnPause_Click(Object^ sender, EventArgs^ e) {
        _service->Pause();
        _btnPause->Text = _service->IsRunning ? "Pause" : "Resume";
    }
    void MainForm::BtnHeatmap_Click(Object^ sender, EventArgs^ e) {
        _showHeatmap = !_showHeatmap;
        _btnHeatmap->BackColor = _showHeatmap
            ? Color::FromArgb(180, 60, 60)
            : Color::FromArgb(60, 80, 160);
        if (!_showHeatmap) _heatmapData = nullptr;
        _pictureBox->Invalidate();
    }
    void MainForm::BtnSchooling_Click(Object^ sender, EventArgs^ e) {
        _showSchooling = !_showSchooling;
        _btnSchooling->BackColor = _showSchooling
            ? Color::FromArgb(180, 60, 60)
            : Color::FromArgb(80, 120, 60);
        _pictureBox->Invalidate();
    }

    // =====================================================
    //  TRACKING CONTROL
    // =====================================================

    void MainForm::StartTracking() {
        OpenFileDialog^ dlg = gcnew OpenFileDialog();
        dlg->Title = L"เลือกไฟล์วิดีโอ";
        dlg->Filter = L"Video Files|*.mp4;*.avi;*.mov;*.mkv|All Files|*.*";
        if (dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK) return;

        String^ exeDir = System::IO::Path::GetDirectoryName(
            System::Reflection::Assembly::GetExecutingAssembly()->Location);
        String^ modelPath = System::IO::Path::Combine(exeDir, L"best50.onnx");

        _service->SetModel(modelPath);
        _service->PondSize = SizeF((float)_pictureBox->Width, (float)_pictureBox->Height);
        _service->Start(dlg->FileName);

        _btnStart->Enabled = false;
        _btnPause->Enabled = true;
        _btnStop->Enabled = true;
        Log(String::Format("[{0}] Tracking started: {1}",
            DateTime::Now.ToString("HH:mm:ss"),
            System::IO::Path::GetFileName(dlg->FileName)));
    }

    void MainForm::StopTracking() {
        _service->Stop();
        _currentFishes->Clear();
        _heatmapData = nullptr;
        _pictureBox->Image = nullptr;
        _pictureBox->Invalidate();
        _grid->Rows->Clear();
        _btnStart->Enabled = true;
        _btnPause->Enabled = false;
        _btnStop->Enabled = false;
        _lblFps->Text = "FPS: --";
        _lblFishCount->Text = "Fish: 0";
        _lblStatSpecies->Text = L"";
        Log(String::Format("[{0}] Tracking stopped", DateTime::Now.ToString("HH:mm:ss")));
    }

    // =====================================================
    //  HELPERS
    // =====================================================

    void MainForm::Log(String^ msg) {
        _listLog->Items->Insert(0, msg);
        if (_listLog->Items->Count > 200)
            _listLog->Items->RemoveAt(200);
    }

    void MainForm::SafeLog(String^ msg) {
        if (this->InvokeRequired) this->Invoke(gcnew Action<String^>(this, &MainForm::Log), msg);
        else Log(msg);
    }

    void MainForm::StyleGrid() {
        _grid->DefaultCellStyle->BackColor = Color::FromArgb(28, 31, 42);
        _grid->DefaultCellStyle->ForeColor = Color::FromArgb(220, 220, 240);
        _grid->DefaultCellStyle->SelectionBackColor = Color::FromArgb(60, 80, 130);
        _grid->DefaultCellStyle->SelectionForeColor = Color::White;
        _grid->ColumnHeadersDefaultCellStyle->BackColor = Color::FromArgb(40, 44, 60);
        _grid->ColumnHeadersDefaultCellStyle->ForeColor = Color::FromArgb(160, 180, 220);
        _grid->ColumnHeadersDefaultCellStyle->Font = gcnew Drawing::Font("Segoe UI", 8.5f, FontStyle::Bold);
        _grid->AlternatingRowsDefaultCellStyle->BackColor = Color::FromArgb(32, 35, 48);
        _grid->RowTemplate->Height = 26;
        _grid->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;

        _grid->Columns->Add("ColSt", "");
        _grid->Columns->Add("ColID", "ID");
        _grid->Columns->Add("ColConf", "Conf");
        _grid->Columns->Add("ColSpeed", "Speed");
        _grid->Columns->Add("ColSeen", "Seen");

        _grid->Columns["ColSt"]->Width = 36;
        _grid->Columns["ColID"]->Width = 42;
        _grid->Columns["ColSeen"]->Width = 65;
        _grid->Columns["ColSt"]->AutoSizeMode = DataGridViewAutoSizeColumnMode::None;
        _grid->Columns["ColID"]->AutoSizeMode = DataGridViewAutoSizeColumnMode::None;
        _grid->Columns["ColSeen"]->AutoSizeMode = DataGridViewAutoSizeColumnMode::None;
    }

    void MainForm::AddStatItem(String^ label, Label^% valRef, Color color, int x) {}
}