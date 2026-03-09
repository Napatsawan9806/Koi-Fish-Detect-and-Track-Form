#include "MainForm.h"
#include <cmath>

namespace KoiTracker {

    // =====================================================
    //  WIRE EVENTS
    // =====================================================

    void MainForm::WireEvents() {
        _btnStart->Click += gcnew EventHandler(this, &MainForm::BtnStart_Click);
        _btnPause->Click += gcnew EventHandler(this, &MainForm::BtnPause_Click);
        _btnStop->Click += gcnew EventHandler(this, &MainForm::BtnStop_Click);
        _btnHeatmap->Click += gcnew EventHandler(this, &MainForm::BtnHeatmap_Click);
        _btnSchooling->Click += gcnew EventHandler(this, &MainForm::BtnSchooling_Click);
        _btnBehavior->Click += gcnew EventHandler(this, &MainForm::BtnBehavior_Click);
        _btnDashboard->Click += gcnew EventHandler(this, &MainForm::BtnDashboard_Click);

        _pictureBox->Paint += gcnew PaintEventHandler(this, &MainForm::OnPictureBoxPaint);
        _pictureBox->Click += gcnew EventHandler(this, &MainForm::OnPictureBoxClick);

        _grid->CellDoubleClick += gcnew DataGridViewCellEventHandler(this, &MainForm::OnGridDoubleClick);

        _service->OnFrameUpdated += gcnew FrameUpdatedHandler(this, &MainForm::OnFrameUpdated);
        _service->OnFishDetected += gcnew FishEventHandler(this, &MainForm::OnFishDetected);
        _service->OnFishLost += gcnew FishEventHandler(this, &MainForm::OnFishLost);
        _service->OnFpsUpdated += gcnew FpsUpdatedHandler(this, &MainForm::OnFpsUpdated);
        _service->OnError += gcnew ErrorHandler(this, &MainForm::OnServiceError);
        _service->OnFrameBitmap += gcnew FrameBitmapHandler(this, &MainForm::OnFrameBitmap);
        this->Resize += gcnew EventHandler(this, &MainForm::OnFormResize);
    }

    // =====================================================
    //  SET VIEW MODE
    // =====================================================

    void MainForm::SetViewMode(ViewMode mode) {
        _viewMode = mode;

        _btnHeatmap->BackColor = Color::FromArgb(40, 44, 62);
        _btnHeatmap->ForeColor = Color::FromArgb(160, 165, 185);
        _btnHeatmap->FlatAppearance->BorderSize = 0;
        _btnSchooling->BackColor = Color::FromArgb(40, 44, 62);
        _btnSchooling->ForeColor = Color::FromArgb(160, 165, 185);
        _btnSchooling->FlatAppearance->BorderSize = 0;
        _btnBehavior->BackColor = Color::FromArgb(40, 44, 62);
        _btnBehavior->ForeColor = Color::FromArgb(160, 165, 185);
        _btnBehavior->FlatAppearance->BorderSize = 0;

        switch (mode) {
        case ViewMode::Heatmap:
            _btnHeatmap->BackColor = Color::FromArgb(50, 70, 145);
            _btnHeatmap->ForeColor = Color::White;
            break;
        case ViewMode::Schooling:
            _btnSchooling->BackColor = Color::FromArgb(55, 105, 50);
            _btnSchooling->ForeColor = Color::White;
            break;
        case ViewMode::Behavior:
            _btnBehavior->BackColor = Color::FromArgb(85, 50, 120);
            _btnBehavior->ForeColor = Color::White;
            break;
        default: break;
        }
    }

    // =====================================================
    //  FRAME UPDATE
    // =====================================================

    void MainForm::OnFrameUpdated(List<FishTrack^>^ fishes) {
        if (this->InvokeRequired) {
            this->BeginInvoke(gcnew FrameUpdatedHandler(this, &MainForm::OnFrameUpdated), fishes);
            return;
        }

        _currentFishes = fishes;
        UpdateGrid(fishes);
        UpdateStats(fishes);
        AnalyzeSocialBehavior(fishes);

        System::Drawing::Size vs = _service->VideoSize;
        int videoW = (vs.Width > 0) ? vs.Width : 640;
        int videoH = (vs.Height > 0) ? vs.Height : 480;

        // HeatmapForm
        if (_heatmapForm != nullptr && !_heatmapForm->IsDisposed) {
            _heatmapForm->Accumulate(fishes, videoW, videoH);
            _heatmapPeak = _heatmapForm->LastPeak;
            _heatmapFrames = _heatmapForm->LastFrameCount;
        }

        // SchoolingForm
        if (_schoolingForm != nullptr && !_schoolingForm->IsDisposed && _schoolingForm->Visible) {
            if ((DateTime::Now - _lastSchoolingUpdate).TotalMilliseconds >= 200) {
                _schoolingForm->UpdateFishes(fishes, videoW, videoH);
                _lastSchoolingUpdate = DateTime::Now;
            }
        }

        // BehaviorForm
        if (_behaviorForm != nullptr && !_behaviorForm->IsDisposed && _behaviorForm->Visible) {
            if ((DateTime::Now - _lastBehaviorUpdate).TotalMilliseconds >= 300) {
                _behaviorForm->UpdateAnalysis(fishes);
                _lastBehaviorUpdate = DateTime::Now;
            }
        }

        // DashboardForm
        if (_dashboardForm != nullptr && !_dashboardForm->IsDisposed && _dashboardForm->Visible) {
            if ((DateTime::Now - _lastDashboardUpdate).TotalMilliseconds >= 500) {
                _dashboardForm->UpdateDashboard(fishes, _heatmapPeak, _heatmapFrames);
                _lastDashboardUpdate = DateTime::Now;
            }
        }

        if (_detailForm != nullptr && !_detailForm->IsDisposed) {
            int detailId = (int)_detailForm->Tag;
            for each (FishTrack ^ f in fishes) {
                if (f->FishID == detailId) {
                    _detailForm->UpdateData(f);
                    Bitmap^ crop = CropFish(f);
                    if (crop != nullptr) _detailForm->UpdateCrop(crop);
                    break;
                }
            }
        }

        int active = 0;
        for each (FishTrack ^ f in fishes)
            if (f->Status != FishStatus::Lost) active++;
        _lblFishCount->Text = String::Format("Fish: {0}", active);
    }

    void MainForm::OnServiceError(String^ message) {
        if (this->InvokeRequired) {
            this->Invoke(gcnew ErrorHandler(this, &MainForm::OnServiceError), message);
            return;
        }
        _btnStart->Enabled = true;
        _btnPause->Enabled = false;
        _btnStop->Enabled = false;
        _lblFps->Text = "FPS: --";
        _lblFishCount->Text = "Fish: 0";
        Log(String::Format("[{0}] ERROR: {1}", DateTime::Now.ToString("HH:mm:ss"), message));
        MessageBox::Show(message, "Detection Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
    }

    void MainForm::OnFishDetected(FishTrack^ fish) {
        SafeLog(String::Format("[{0}] NEW  #{1}", DateTime::Now.ToString("HH:mm:ss"), fish->FishID));
    }
    void MainForm::OnFishLost(FishTrack^ fish) {
        SafeLog(String::Format("[{0}] LOST #{1}", DateTime::Now.ToString("HH:mm:ss"), fish->FishID));
    }
    void MainForm::OnFpsUpdated(int fps) {
        if (this->InvokeRequired) {
            this->BeginInvoke(gcnew FpsUpdatedHandler(this, &MainForm::OnFpsUpdated), fps);
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

    void MainForm::OnPictureBoxPaint(Object^ sender, PaintEventArgs^ e) {
        if (_currentFishes == nullptr || _currentFishes->Count == 0) return;
        if (_pictureBox->Image == nullptr) return;

        Graphics^ g = e->Graphics;
        g->SmoothingMode = Drawing2D::SmoothingMode::AntiAlias;

        System::Drawing::Size vs = _service->VideoSize;
        float scaleX = (vs.Width > 0) ? (float)_pictureBox->Width / vs.Width : 1.0f;
        float scaleY = (vs.Height > 0) ? (float)_pictureBox->Height / vs.Height : 1.0f;

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

        System::Drawing::Size vs = _service->VideoSize;
        float scaleX = (vs.Width > 0) ? (float)vs.Width / _pictureBox->Width : 1.0f;
        float scaleY = (vs.Height > 0) ? (float)vs.Height / _pictureBox->Height : 1.0f;
        float videoX = me->X * scaleX;
        float videoY = me->Y * scaleY;

        for each (FishTrack ^ fish in _currentFishes) {
            if (fish->Status == FishStatus::Lost) continue;
            if (fish->BoundingBox.Contains(videoX, videoY)) {
                Log(String::Format("[{0}] Click #{1} conf={2}",
                    DateTime::Now.ToString("HH:mm:ss"), fish->FishID, fish->ConfidenceText));
                Bitmap^ crop = CropFish(fish);
                if (_detailForm == nullptr || _detailForm->IsDisposed) {
                    _detailForm = gcnew FishDetailForm(fish);
                    _detailForm->Tag = fish->FishID;
                    if (crop != nullptr) _detailForm->UpdateCrop(crop);
                    _detailForm->Show(this);
                }
                else {
                    _detailForm->UpdateData(fish);
                    _detailForm->Tag = fish->FishID;
                    if (crop != nullptr) _detailForm->UpdateCrop(crop);
                    _detailForm->BringToFront();
                }
                break;
            }
        }
    }

    Bitmap^ MainForm::CropFish(FishTrack^ fish) {
        if (_pictureBox->Image == nullptr) return nullptr;
        System::Drawing::Size vs = _service->VideoSize;
        if (vs.Width == 0 || vs.Height == 0) return nullptr;

        float sx = (float)_pictureBox->Image->Width / vs.Width;
        float sy = (float)_pictureBox->Image->Height / vs.Height;
        int pad = 10;
        int x = Math::Max(0, (int)(fish->BoundingBox.X * sx) - pad);
        int y = Math::Max(0, (int)(fish->BoundingBox.Y * sy) - pad);
        int w = Math::Min(_pictureBox->Image->Width - x, (int)(fish->BoundingBox.Width * sx) + pad * 2);
        int h = Math::Min(_pictureBox->Image->Height - y, (int)(fish->BoundingBox.Height * sy) + pad * 2);
        if (w <= 0 || h <= 0) return nullptr;

        Bitmap^ frame = safe_cast<Bitmap^>(_pictureBox->Image);
        Bitmap^ crop = gcnew Bitmap(w, h);
        Graphics^ g = Graphics::FromImage(crop);
        g->DrawImage(frame,
            System::Drawing::Rectangle(0, 0, w, h),
            System::Drawing::Rectangle(x, y, w, h),
            GraphicsUnit::Pixel);
        delete g;
        return crop;
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
                f->Species.ToString(),
                f->ConfidenceText,
                String::Format("{0:F1}", f->Speed),
                f->ActivityText,
                f->FirstSeen.ToString("HH:mm:ss")
            );
            _grid->Rows[row]->Tag = f->FishID;

            Color rowColor = (f->Status == FishStatus::Lost) ? Color::FromArgb(48, 38, 38)
                : (f->Status == FishStatus::New) ? Color::FromArgb(38, 48, 33)
                : Color::FromArgb(26, 29, 40);
            _grid->Rows[row]->DefaultCellStyle->BackColor = rowColor;

            Color confColor = (f->Confidence > 0.85f) ? Color::FromArgb(80, 220, 120)
                : (f->Confidence > 0.70f) ? Color::FromArgb(220, 200, 80)
                : Color::FromArgb(220, 100, 80);
            _grid->Rows[row]->Cells["ColConf"]->Style->ForeColor = confColor;

            Color actColor;
            switch (f->Activity) {
            case ActivityLevel::Resting:  actColor = Color::FromArgb(80, 220, 120);  break;
            case ActivityLevel::Cruising: actColor = Color::FromArgb(200, 200, 80);  break;
            case ActivityLevel::Active:   actColor = Color::FromArgb(255, 140, 40);  break;
            case ActivityLevel::Erratic:  actColor = Color::FromArgb(255, 80, 80);  break;
            default:                      actColor = Color::FromArgb(130, 130, 140); break;
            }
            _grid->Rows[row]->Cells["ColAct"]->Style->ForeColor = actColor;
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

        // Live info panel
        {
            Dictionary<KoiSpecies, int>^ counts = gcnew Dictionary<KoiSpecies, int>();
            for each (FishTrack ^ f in fishes) {
                if (f->Status == FishStatus::Lost) continue;
                if (!counts->ContainsKey(f->Species)) counts[f->Species] = 0;
                counts[f->Species]++;
            }
            System::Text::StringBuilder^ sb = gcnew System::Text::StringBuilder();
            for each (KeyValuePair<KoiSpecies, int> kv in counts)
                sb->AppendFormat("{0,-12} {1}\n", kv.Key.ToString(), kv.Value);
            if (sb->Length == 0) sb->Append("No fish detected.");
            _lblInfoLiveContent->Text = sb->ToString();
        }

        // Schooling info panel
        {
            const float SCHOOL_RADIUS = 120.0f;
            Dictionary<int, int>^ group = gcnew Dictionary<int, int>();
            int groupId = 0;
            for (int i = 0; i < fishes->Count; i++) {
                if (fishes[i]->Status == FishStatus::Lost) continue;
                if (!group->ContainsKey(i)) group[i] = groupId++;
                for (int j = i + 1; j < fishes->Count; j++) {
                    if (fishes[j]->Status == FishStatus::Lost) continue;
                    float dx = fishes[i]->Center.X - fishes[j]->Center.X;
                    float dy = fishes[i]->Center.Y - fishes[j]->Center.Y;
                    if (Math::Sqrt((double)(dx * dx + dy * dy)) <= SCHOOL_RADIUS)
                        group[j] = group[i];
                    else if (!group->ContainsKey(j))
                        group[j] = groupId++;
                }
            }
            Dictionary<int, int>^ groupSize = gcnew Dictionary<int, int>();
            for each (KeyValuePair<int, int> kv in group) {
                if (!groupSize->ContainsKey(kv.Value)) groupSize[kv.Value] = 0;
                groupSize[kv.Value]++;
            }
            int schoolGroups = 0, schoolFish = 0;
            for each (KeyValuePair<int, int> kv in groupSize)
                if (kv.Value > 1) { schoolGroups++; schoolFish += kv.Value; }

            float pct = (active > 0) ? (float)schoolFish / active * 100.0f : 0.0f;
            System::Text::StringBuilder^ sb = gcnew System::Text::StringBuilder();
            sb->AppendFormat("Groups       {0}\n", schoolGroups);
            sb->AppendFormat("Schooling    {0} fish  ({1:F0}%)\n", schoolFish, pct);
            sb->AppendFormat("Solo         {0} fish\n", active - schoolFish);
            _lblInfoSchoolingContent->Text = sb->ToString();
        }
    }

    // =====================================================
    //  BUTTON HANDLERS
    // =====================================================

    void MainForm::BtnStart_Click(Object^ sender, EventArgs^ e) { StartTracking(); }
    void MainForm::BtnStop_Click(Object^ sender, EventArgs^ e) { StopTracking(); }
    void MainForm::BtnPause_Click(Object^ sender, EventArgs^ e) {
        _service->Pause();
        _btnPause->Text = _service->IsRunning ? L"Pause" : L"Resume";
    }

    void MainForm::BtnHeatmap_Click(Object^ sender, EventArgs^ e) {
        SetViewMode(ViewMode::Heatmap);
        if (_heatmapForm == nullptr || _heatmapForm->IsDisposed)
            _heatmapForm = gcnew HeatmapForm();
        if (!_heatmapForm->Visible)
            _heatmapForm->Show(this);
        else
            _heatmapForm->BringToFront();
    }

    void MainForm::BtnSchooling_Click(Object^ sender, EventArgs^ e) {
        SetViewMode(ViewMode::Schooling);
        if (_schoolingForm == nullptr || _schoolingForm->IsDisposed) {
            _schoolingForm = gcnew SchoolingForm();
            _schoolingForm->Show(this);
            if (_currentFishes != nullptr && _currentFishes->Count > 0) {
                System::Drawing::Size vs = _service->VideoSize;
                int videoW = (vs.Width > 0) ? vs.Width : 640;
                int videoH = (vs.Height > 0) ? vs.Height : 480;
                _schoolingForm->UpdateFishes(_currentFishes, videoW, videoH);
            }
        }
        else {
            _schoolingForm->BringToFront();
        }
    }

    void MainForm::BtnBehavior_Click(Object^ sender, EventArgs^ e) {
        SetViewMode(ViewMode::Behavior);
        if (_behaviorForm == nullptr || _behaviorForm->IsDisposed) {
            _behaviorForm = gcnew BehaviorAnalysisForm();
            _behaviorForm->Show(this);
            if (_currentFishes != nullptr && _currentFishes->Count > 0)
                _behaviorForm->UpdateAnalysis(_currentFishes);
        }
        else {
            _behaviorForm->BringToFront();
        }
    }

    void MainForm::BtnDashboard_Click(Object^ sender, EventArgs^ e) {
        if (_dashboardForm == nullptr || _dashboardForm->IsDisposed) {
            _dashboardForm = gcnew AnalysisDashboardForm();
            _dashboardForm->Show(this);
            if (_currentFishes != nullptr && _currentFishes->Count > 0)
                _dashboardForm->UpdateDashboard(_currentFishes, _heatmapPeak, _heatmapFrames);
        }
        else if (!_dashboardForm->Visible) {
            _dashboardForm->Show(this);   // <-- เพิ่มบรรทัดนี้
            _dashboardForm->BringToFront();
        }
        else {
            _dashboardForm->BringToFront();
        }
        _btnDashboard->BackColor = Color::FromArgb(90, 50, 150);
        _btnDashboard->ForeColor = Color::White;
    }

    // =====================================================
    //  BEHAVIOR ANALYSIS
    // =====================================================

    void MainForm::AnalyzeSocialBehavior(List<FishTrack^>^ fishes) {
        const float SCHOOL_RADIUS = 120.0f;
        const int   ISOLATION_THRESHOLD = 300;
        const float LETHARGY_RATIO = 0.20f;
        const float HYPERACTIVE_RATIO = 3.0f;

        float totalSpeed = 0.0f;
        int   activeCount = 0;
        for each (FishTrack ^ f in fishes) {
            if (f->Status == FishStatus::Lost) continue;
            totalSpeed += f->AvgSpeed;
            activeCount++;
        }
        float pondAvgSpeed = (activeCount > 0) ? totalSpeed / activeCount : 0.0f;

        array<bool>^ inSchool = gcnew array<bool>(fishes->Count);
        int schoolingCount = 0;
        for (int i = 0; i < fishes->Count; i++) {
            if (fishes[i]->Status == FishStatus::Lost) continue;
            for (int j = 0; j < fishes->Count; j++) {
                if (i == j || fishes[j]->Status == FishStatus::Lost) continue;
                float dx = fishes[i]->Center.X - fishes[j]->Center.X;
                float dy = fishes[i]->Center.Y - fishes[j]->Center.Y;
                if (Math::Sqrt((double)(dx * dx + dy * dy)) <= SCHOOL_RADIUS) {
                    inSchool[i] = true; break;
                }
            }
            if (inSchool[i]) schoolingCount++;
        }
        float schoolingPct = (activeCount > 0) ? (float)schoolingCount / activeCount * 100.0f : 0.0f;

        int isolatedCount = 0;
        for (int i = 0; i < fishes->Count; i++) {
            if (fishes[i]->Status == FishStatus::Lost) continue;
            if (!inSchool[i] && activeCount > 1) {
                fishes[i]->IsolationFrames++;
                if (fishes[i]->IsolationFrames >= ISOLATION_THRESHOLD)
                    fishes[i]->IsIsolated = true;
            }
            else {
                fishes[i]->IsolationFrames = 0;
                fishes[i]->IsIsolated = false;
            }
            if (fishes[i]->IsIsolated) isolatedCount++;
        }

        // Update behavior info panel
        {
            System::Text::StringBuilder^ sb = gcnew System::Text::StringBuilder();
            sb->AppendFormat("Avg Speed    {0:F1} px/f\n", pondAvgSpeed);
            sb->AppendFormat("Schooling    {0:F0}%\n", schoolingPct);
            sb->AppendFormat("Isolated     {0} fish\n\n", isolatedCount);

            bool anyAlert = false;
            if (pondAvgSpeed > 0.1f) {
                for each (FishTrack ^ f in fishes) {
                    if (f->Status == FishStatus::Lost || f->FrameCount < 30) continue;
                    if (f->AvgSpeed < pondAvgSpeed * LETHARGY_RATIO) {
                        sb->AppendFormat("[!] #{0}  LETHARGIC  ({1:F1})\n", f->FishID, f->AvgSpeed);
                        anyAlert = true;
                    }
                    if (f->AvgSpeed > pondAvgSpeed * HYPERACTIVE_RATIO) {
                        sb->AppendFormat("[!] #{0}  HYPERACTIVE  ({1:F1})\n", f->FishID, f->AvgSpeed);
                        anyAlert = true;
                    }
                    if (f->IsIsolated) {
                        sb->AppendFormat("[!] #{0}  ISOLATED\n", f->FishID);
                        anyAlert = true;
                    }
                }
            }
            if (!anyAlert && activeCount > 0) sb->Append("No alerts.");
            _lblInfoBehaviorContent->Text = sb->ToString();
        }

        // Periodic alert logging
        _alertFrameCount++;
        if (_alertFrameCount >= 300) {
            _alertFrameCount = 0;
            if (pondAvgSpeed > 0.1f) {
                for each (FishTrack ^ f in fishes) {
                    if (f->Status == FishStatus::Lost || f->FrameCount < 30) continue;
                    if (f->AvgSpeed < pondAvgSpeed * LETHARGY_RATIO)
                        SafeLog(String::Format("[{0}] ALERT Lethargic #{1} speed={2:F1}",
                            DateTime::Now.ToString("HH:mm:ss"), f->FishID, f->AvgSpeed));
                    if (f->AvgSpeed > pondAvgSpeed * HYPERACTIVE_RATIO)
                        SafeLog(String::Format("[{0}] ALERT Hyperactive #{1} speed={2:F1}",
                            DateTime::Now.ToString("HH:mm:ss"), f->FishID, f->AvgSpeed));
                    if (f->IsIsolated)
                        SafeLog(String::Format("[{0}] ALERT Isolated #{1}",
                            DateTime::Now.ToString("HH:mm:ss"), f->FishID));
                }
            }
        }
    }

    // =====================================================
    //  TRACKING CONTROL
    // =====================================================

    void MainForm::StartTracking() {
        OpenFileDialog^ dlg = gcnew OpenFileDialog();
        dlg->Title = L"Select Video File";
        dlg->Filter = L"Video Files|*.mp4;*.avi;*.mov;*.mkv|All Files|*.*";
        if (dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK) return;

        String^ exeDir = System::IO::Path::GetDirectoryName(
            System::Reflection::Assembly::GetExecutingAssembly()->Location);
        String^ modelPath = System::IO::Path::Combine(exeDir, L"best50.onnx");

        int videoW = 0, videoH = 0;
        Bitmap^ preview = DetectionService::GetFirstFrame(dlg->FileName, videoW, videoH);
        if (preview != nullptr) {
            ROISelectForm^ roiForm = gcnew ROISelectForm(preview, videoW, videoH);
            System::Windows::Forms::DialogResult roiResult = roiForm->ShowDialog(this);
            delete preview;
            if (roiResult == System::Windows::Forms::DialogResult::Cancel) return;
            _service->RoiRect = roiForm->SelectedROI;
        }
        else {
            _service->RoiRect = System::Drawing::Rectangle::Empty;
        }

        _service->SetModel(modelPath);
        _service->PondSize = SizeF((float)_pictureBox->Width, (float)_pictureBox->Height);
        _service->Start(dlg->FileName);

        if (_heatmapForm == nullptr || _heatmapForm->IsDisposed)
            _heatmapForm = gcnew HeatmapForm();
        else
            _heatmapForm->BtnClear_Click(nullptr, nullptr);

        _heatmapPeak = 0;
        _heatmapFrames = 0;

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
        _pictureBox->Image = nullptr;
        _pictureBox->Invalidate();
        _grid->Rows->Clear();
        _btnStart->Enabled = true;
        _btnPause->Enabled = false;
        _btnStop->Enabled = false;
        _btnPause->Text = L"Pause";
        _lblFps->Text = "FPS: --";
        _lblFishCount->Text = "Fish: 0";
        Log(String::Format("[{0}] Tracking stopped", DateTime::Now.ToString("HH:mm:ss")));
    }

    // =====================================================
    //  STYLE GRID
    // =====================================================

    void MainForm::StyleGrid() {
        _grid->DefaultCellStyle->BackColor = Color::FromArgb(26, 29, 40);
        _grid->DefaultCellStyle->ForeColor = Color::FromArgb(210, 215, 235);
        _grid->DefaultCellStyle->SelectionBackColor = Color::FromArgb(55, 75, 125);
        _grid->DefaultCellStyle->SelectionForeColor = Color::White;
        _grid->ColumnHeadersDefaultCellStyle->BackColor = Color::FromArgb(36, 40, 56);
        _grid->ColumnHeadersDefaultCellStyle->ForeColor = Color::FromArgb(140, 155, 200);
        _grid->ColumnHeadersDefaultCellStyle->Font = gcnew Drawing::Font("Segoe UI", 8.5f, FontStyle::Bold);
        _grid->AlternatingRowsDefaultCellStyle->BackColor = Color::FromArgb(30, 33, 46);
        _grid->RowTemplate->Height = 25;
        _grid->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;

        _grid->Columns->Add("ColSt", "");
        _grid->Columns->Add("ColID", "ID");
        _grid->Columns->Add("ColSpecies", "Species");
        _grid->Columns->Add("ColConf", "Conf");
        _grid->Columns->Add("ColSpeed", "Speed");
        _grid->Columns->Add("ColAct", "Act");
        _grid->Columns->Add("ColSeen", "Seen");

        _grid->Columns["ColSt"]->Width = 34;
        _grid->Columns["ColID"]->Width = 40;
        _grid->Columns["ColAct"]->Width = 40;
        _grid->Columns["ColSeen"]->Width = 62;
        _grid->Columns["ColSt"]->AutoSizeMode = DataGridViewAutoSizeColumnMode::None;
        _grid->Columns["ColID"]->AutoSizeMode = DataGridViewAutoSizeColumnMode::None;
        _grid->Columns["ColAct"]->AutoSizeMode = DataGridViewAutoSizeColumnMode::None;
        _grid->Columns["ColSeen"]->AutoSizeMode = DataGridViewAutoSizeColumnMode::None;
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
        if (this->InvokeRequired)
            this->BeginInvoke(gcnew Action<String^>(this, &MainForm::Log), msg);
        else
            Log(msg);
    }

} // namespace KoiTracker