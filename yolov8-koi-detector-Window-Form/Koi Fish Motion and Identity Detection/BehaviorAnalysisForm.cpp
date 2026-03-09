#include "BehaviorAnalysisForm.h"

namespace KoiTracker {

    BehaviorAnalysisForm::BehaviorAnalysisForm() {
        InitializeComponent();
    }

    // =====================================================
    //  INITIALIZE COMPONENT
    // =====================================================

    void BehaviorAnalysisForm::InitializeComponent() {
        this->Text            = L"Behavior Analysis  \x2014  Live";
        this->ClientSize      = System::Drawing::Size(840, 590);
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::Sizable;
        this->MaximizeBox     = true;
        this->MinimizeBox     = true;
        this->StartPosition   = FormStartPosition::WindowsDefaultLocation;
        this->BackColor       = Color::FromArgb(22, 25, 36);
        this->ForeColor       = Color::White;
        this->Font            = gcnew Drawing::Font("Segoe UI", 9);
        this->MinimumSize     = System::Drawing::Size(700, 520);

        (cli::safe_cast<ISupportInitialize^>(_fishGrid = gcnew DataGridView()))->BeginInit();

        // ─── HEADER PANEL ─────────────────────────────────────────────────
        Panel^ header = gcnew Panel();
        header->BackColor = Color::FromArgb(55, 28, 85);
        header->Location  = Point(0, 0);
        header->Size      = System::Drawing::Size(840, 62);

        Label^ lblTitle = gcnew Label();
        lblTitle->AutoSize  = false;
        lblTitle->Text      = L"BEHAVIOR ANALYSIS";
        lblTitle->Font      = gcnew Drawing::Font("Segoe UI", 13, FontStyle::Bold);
        lblTitle->ForeColor = Color::FromArgb(215, 170, 255);
        lblTitle->Location  = Point(14, 16);
        lblTitle->Size      = System::Drawing::Size(230, 30);
        header->Controls->Add(lblTitle);

        // ── Active stat box ──
        _lblActiveVal           = gcnew Label();
        _lblActiveVal->AutoSize  = false;
        _lblActiveVal->Text      = L"--";
        _lblActiveVal->Font      = gcnew Drawing::Font("Segoe UI", 16, FontStyle::Bold);
        _lblActiveVal->ForeColor = Color::FromArgb(80, 220, 120);
        _lblActiveVal->Location  = Point(258, 8);
        _lblActiveVal->Size      = System::Drawing::Size(110, 30);
        _lblActiveVal->TextAlign = ContentAlignment::MiddleCenter;
        header->Controls->Add(_lblActiveVal);

        Label^ capActive = gcnew Label();
        capActive->AutoSize  = false;
        capActive->Text      = L"ACTIVE";
        capActive->Font      = gcnew Drawing::Font("Segoe UI", 7);
        capActive->ForeColor = Color::FromArgb(120, 130, 155);
        capActive->Location  = Point(258, 42);
        capActive->Size      = System::Drawing::Size(110, 16);
        capActive->TextAlign = ContentAlignment::MiddleCenter;
        header->Controls->Add(capActive);

        // ── Avg Speed stat box ──
        _lblAvgSpeedVal           = gcnew Label();
        _lblAvgSpeedVal->AutoSize  = false;
        _lblAvgSpeedVal->Text      = L"--";
        _lblAvgSpeedVal->Font      = gcnew Drawing::Font("Segoe UI", 16, FontStyle::Bold);
        _lblAvgSpeedVal->ForeColor = Color::FromArgb(100, 180, 255);
        _lblAvgSpeedVal->Location  = Point(378, 8);
        _lblAvgSpeedVal->Size      = System::Drawing::Size(110, 30);
        _lblAvgSpeedVal->TextAlign = ContentAlignment::MiddleCenter;
        header->Controls->Add(_lblAvgSpeedVal);

        Label^ capSpeed = gcnew Label();
        capSpeed->AutoSize  = false;
        capSpeed->Text      = L"AVG SPEED (px/f)";
        capSpeed->Font      = gcnew Drawing::Font("Segoe UI", 7);
        capSpeed->ForeColor = Color::FromArgb(120, 130, 155);
        capSpeed->Location  = Point(378, 42);
        capSpeed->Size      = System::Drawing::Size(110, 16);
        capSpeed->TextAlign = ContentAlignment::MiddleCenter;
        header->Controls->Add(capSpeed);

        // ── Schooling stat box ──
        _lblSchoolingVal           = gcnew Label();
        _lblSchoolingVal->AutoSize  = false;
        _lblSchoolingVal->Text      = L"--";
        _lblSchoolingVal->Font      = gcnew Drawing::Font("Segoe UI", 16, FontStyle::Bold);
        _lblSchoolingVal->ForeColor = Color::FromArgb(120, 255, 180);
        _lblSchoolingVal->Location  = Point(498, 8);
        _lblSchoolingVal->Size      = System::Drawing::Size(110, 30);
        _lblSchoolingVal->TextAlign = ContentAlignment::MiddleCenter;
        header->Controls->Add(_lblSchoolingVal);

        Label^ capSchool = gcnew Label();
        capSchool->AutoSize  = false;
        capSchool->Text      = L"SCHOOLING";
        capSchool->Font      = gcnew Drawing::Font("Segoe UI", 7);
        capSchool->ForeColor = Color::FromArgb(120, 130, 155);
        capSchool->Location  = Point(498, 42);
        capSchool->Size      = System::Drawing::Size(110, 16);
        capSchool->TextAlign = ContentAlignment::MiddleCenter;
        header->Controls->Add(capSchool);

        // ── Isolated stat box ──
        _lblIsolatedVal           = gcnew Label();
        _lblIsolatedVal->AutoSize  = false;
        _lblIsolatedVal->Text      = L"--";
        _lblIsolatedVal->Font      = gcnew Drawing::Font("Segoe UI", 16, FontStyle::Bold);
        _lblIsolatedVal->ForeColor = Color::FromArgb(255, 140, 80);
        _lblIsolatedVal->Location  = Point(618, 8);
        _lblIsolatedVal->Size      = System::Drawing::Size(110, 30);
        _lblIsolatedVal->TextAlign = ContentAlignment::MiddleCenter;
        header->Controls->Add(_lblIsolatedVal);

        Label^ capIso = gcnew Label();
        capIso->AutoSize  = false;
        capIso->Text      = L"ISOLATED";
        capIso->Font      = gcnew Drawing::Font("Segoe UI", 7);
        capIso->ForeColor = Color::FromArgb(120, 130, 155);
        capIso->Location  = Point(618, 42);
        capIso->Size      = System::Drawing::Size(110, 16);
        capIso->TextAlign = ContentAlignment::MiddleCenter;
        header->Controls->Add(capIso);

        this->Controls->Add(header);

        // ─── LEFT: FISH GRID ──────────────────────────────────────────────
        Label^ gridHdr = gcnew Label();
        gridHdr->AutoSize  = false;
        gridHdr->Text      = L"PER FISH BREAKDOWN";
        gridHdr->Font      = gcnew Drawing::Font("Segoe UI", 8.5f, FontStyle::Bold);
        gridHdr->ForeColor = Color::FromArgb(130, 140, 170);
        gridHdr->Location  = Point(10, 70);
        gridHdr->Size      = System::Drawing::Size(200, 20);
        this->Controls->Add(gridHdr);

        _fishGrid->AllowUserToAddRows     = false;
        _fishGrid->AllowUserToDeleteRows  = false;
        _fishGrid->BackgroundColor        = Color::FromArgb(22, 25, 35);
        _fishGrid->BorderStyle            = System::Windows::Forms::BorderStyle::None;
        _fishGrid->ColumnHeadersHeight    = 26;
        _fishGrid->EnableHeadersVisualStyles = false;
        _fishGrid->Font       = gcnew Drawing::Font("Segoe UI", 8.5f);
        _fishGrid->GridColor  = Color::FromArgb(42, 46, 62);
        _fishGrid->Location   = Point(10, 92);
        _fishGrid->ReadOnly   = true;
        _fishGrid->RowHeadersVisible = false;
        _fishGrid->SelectionMode     = DataGridViewSelectionMode::FullRowSelect;
        _fishGrid->Size       = System::Drawing::Size(408, 248);
        (cli::safe_cast<ISupportInitialize^>(_fishGrid))->EndInit();
        this->Controls->Add(_fishGrid);
        StyleFishGrid();

        // ─── LEFT: ALERTS ─────────────────────────────────────────────────
        Label^ alertHdr = gcnew Label();
        alertHdr->AutoSize  = false;
        alertHdr->Text      = L"BEHAVIORAL ALERTS";
        alertHdr->Font      = gcnew Drawing::Font("Segoe UI", 8.5f, FontStyle::Bold);
        alertHdr->ForeColor = Color::FromArgb(130, 140, 170);
        alertHdr->Location  = Point(10, 348);
        alertHdr->Size      = System::Drawing::Size(200, 20);
        this->Controls->Add(alertHdr);

        _alertList = gcnew ListBox();
        _alertList->BackColor   = Color::FromArgb(16, 18, 26);
        _alertList->BorderStyle = System::Windows::Forms::BorderStyle::None;
        _alertList->Font        = gcnew Drawing::Font("Consolas", 8.5f);
        _alertList->ForeColor   = Color::FromArgb(255, 180, 80);
        _alertList->Location    = Point(10, 370);
        _alertList->Size        = System::Drawing::Size(408, 168);
        this->Controls->Add(_alertList);

        // ─── RIGHT: SPEED CHART ───────────────────────────────────────────
        Label^ speedHdr = gcnew Label();
        speedHdr->AutoSize  = false;
        speedHdr->Text      = L"SPEED DISTRIBUTION";
        speedHdr->Font      = gcnew Drawing::Font("Segoe UI", 8.5f, FontStyle::Bold);
        speedHdr->ForeColor = Color::FromArgb(130, 140, 170);
        speedHdr->Location  = Point(428, 70);
        speedHdr->Size      = System::Drawing::Size(250, 20);
        this->Controls->Add(speedHdr);

        _speedChart = gcnew SpeedChartPanel();
        _speedChart->Location  = Point(428, 92);
        _speedChart->Size      = System::Drawing::Size(400, 188);
        _speedChart->BackColor = Color::FromArgb(16, 19, 28);
        this->Controls->Add(_speedChart);

        // ─── RIGHT: ZONE MAP ──────────────────────────────────────────────
        Label^ zoneHdr = gcnew Label();
        zoneHdr->AutoSize  = false;
        zoneHdr->Text      = L"ZONE ACTIVITY (3x3)";
        zoneHdr->Font      = gcnew Drawing::Font("Segoe UI", 8.5f, FontStyle::Bold);
        zoneHdr->ForeColor = Color::FromArgb(130, 140, 170);
        zoneHdr->Location  = Point(428, 290);
        zoneHdr->Size      = System::Drawing::Size(200, 20);
        this->Controls->Add(zoneHdr);

        _zoneMap = gcnew ZoneMapPanel();
        _zoneMap->Location  = Point(428, 312);
        _zoneMap->Size      = System::Drawing::Size(195, 195);
        _zoneMap->BackColor = Color::FromArgb(16, 19, 28);
        this->Controls->Add(_zoneMap);

        // ─── RIGHT: ACTIVITY BREAKDOWN ────────────────────────────────────
        Label^ analysisHdr = gcnew Label();
        analysisHdr->AutoSize  = false;
        analysisHdr->Text      = L"ACTIVITY BREAKDOWN";
        analysisHdr->Font      = gcnew Drawing::Font("Segoe UI", 8.5f, FontStyle::Bold);
        analysisHdr->ForeColor = Color::FromArgb(130, 140, 170);
        analysisHdr->Location  = Point(634, 290);
        analysisHdr->Size      = System::Drawing::Size(195, 20);
        this->Controls->Add(analysisHdr);

        _lblAnalysis = gcnew Label();
        _lblAnalysis->AutoSize  = false;
        _lblAnalysis->Location  = Point(634, 312);
        _lblAnalysis->Size      = System::Drawing::Size(195, 195);
        _lblAnalysis->Font      = gcnew Drawing::Font("Consolas", 8.5f);
        _lblAnalysis->ForeColor = Color::FromArgb(180, 190, 210);
        _lblAnalysis->BackColor = Color::FromArgb(16, 19, 28);
        _lblAnalysis->Text      = L"";
        this->Controls->Add(_lblAnalysis);

        // ─── CLOSE BUTTON ─────────────────────────────────────────────────
        _btnClose = gcnew Button();
        _btnClose->Text      = L"Close";
        _btnClose->Font      = gcnew Drawing::Font("Segoe UI", 9);
        _btnClose->Location  = Point(370, 552);
        _btnClose->Size      = System::Drawing::Size(100, 30);
        _btnClose->BackColor = Color::FromArgb(60, 40, 80);
        _btnClose->ForeColor = Color::White;
        _btnClose->FlatStyle = FlatStyle::Flat;
        _btnClose->FlatAppearance->BorderSize = 0;
        _btnClose->Click    += gcnew EventHandler(this, &BehaviorAnalysisForm::BtnClose_Click);
        this->Controls->Add(_btnClose);
    }

    // =====================================================
    //  STYLE FISH GRID
    // =====================================================

    void BehaviorAnalysisForm::StyleFishGrid() {
        _fishGrid->DefaultCellStyle->BackColor          = Color::FromArgb(26, 29, 40);
        _fishGrid->DefaultCellStyle->ForeColor          = Color::FromArgb(210, 215, 235);
        _fishGrid->DefaultCellStyle->SelectionBackColor = Color::FromArgb(65, 45, 110);
        _fishGrid->DefaultCellStyle->SelectionForeColor = Color::White;
        _fishGrid->ColumnHeadersDefaultCellStyle->BackColor = Color::FromArgb(36, 28, 56);
        _fishGrid->ColumnHeadersDefaultCellStyle->ForeColor = Color::FromArgb(160, 140, 210);
        _fishGrid->ColumnHeadersDefaultCellStyle->Font      = gcnew Drawing::Font("Segoe UI", 8.5f, FontStyle::Bold);
        _fishGrid->AlternatingRowsDefaultCellStyle->BackColor = Color::FromArgb(30, 26, 46);
        _fishGrid->RowTemplate->Height = 24;
        _fishGrid->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;

        _fishGrid->Columns->Add("BColID",       "ID");
        _fishGrid->Columns->Add("BColActivity", "Activity");
        _fishGrid->Columns->Add("BColSpeed",    "Avg Speed");
        _fishGrid->Columns->Add("BColTopZone",  "Top Zone");
        _fishGrid->Columns->Add("BColIso",      "Isolated");
        _fishGrid->Columns->Add("BColFrames",   "Frames");

        _fishGrid->Columns["BColID"]->Width      = 42;
        _fishGrid->Columns["BColID"]->AutoSizeMode = DataGridViewAutoSizeColumnMode::None;
        _fishGrid->Columns["BColIso"]->Width     = 58;
        _fishGrid->Columns["BColIso"]->AutoSizeMode = DataGridViewAutoSizeColumnMode::None;
        _fishGrid->Columns["BColFrames"]->Width  = 62;
        _fishGrid->Columns["BColFrames"]->AutoSizeMode = DataGridViewAutoSizeColumnMode::None;
    }

    // =====================================================
    //  UPDATE ANALYSIS  (called every frame update)
    // =====================================================

    void BehaviorAnalysisForm::UpdateAnalysis(List<FishTrack^>^ fishes) {
        // ── Compute summary metrics ──────────────────────────────────────────
        float totalSpeed  = 0.0f;
        int   activeCount = 0;
        for each (FishTrack^ f in fishes) {
            if (f->Status == FishStatus::Lost) continue;
            totalSpeed += f->AvgSpeed;
            activeCount++;
        }
        float pondAvgSpeed = (activeCount > 0) ? totalSpeed / activeCount : 0.0f;

        // Schooling count
        const float SCHOOL_RADIUS = 120.0f;
        int schoolingCount = 0;
        for (int i = 0; i < fishes->Count; i++) {
            if (fishes[i]->Status == FishStatus::Lost) continue;
            for (int j = 0; j < fishes->Count; j++) {
                if (i == j || fishes[j]->Status == FishStatus::Lost) continue;
                float dx = fishes[i]->Center.X - fishes[j]->Center.X;
                float dy = fishes[i]->Center.Y - fishes[j]->Center.Y;
                if (Math::Sqrt((double)(dx * dx + dy * dy)) <= SCHOOL_RADIUS) {
                    schoolingCount++;
                    break;
                }
            }
        }
        float schoolingPct = (activeCount > 0) ? (float)schoolingCount / activeCount * 100.0f : 0.0f;

        int isolatedCount = 0;
        for each (FishTrack^ f in fishes)
            if (f->IsIsolated) isolatedCount++;

        // ── Update header stats ──────────────────────────────────────────────
        _lblActiveVal->Text    = activeCount.ToString();
        _lblAvgSpeedVal->Text  = String::Format("{0:F1}", pondAvgSpeed);
        _lblSchoolingVal->Text = String::Format("{0:F0}%", schoolingPct);
        _lblIsolatedVal->Text  = isolatedCount.ToString();

        // ── Update fish breakdown grid ───────────────────────────────────────
        _fishGrid->Rows->Clear();
        array<String^>^ zoneNames = { "NW", "N", "NE", "W", "C", "E", "SW", "S", "SE" };
        for each (FishTrack^ f in fishes) {
            if (f->Status == FishStatus::Lost) continue;

            // Find top zone
            int topZone = 0, topVisits = 0;
            for (int z = 0; z < 9; z++) {
                if (f->ZoneVisits[z] > topVisits) {
                    topVisits = f->ZoneVisits[z];
                    topZone   = z;
                }
            }
            String^ topZoneName = (topVisits > 0) ? zoneNames[topZone] : "--";

            int row = _fishGrid->Rows->Add(
                String::Format("#{0}", f->FishID),
                f->Activity.ToString(),
                String::Format("{0:F1}", f->AvgSpeed),
                topZoneName,
                f->IsIsolated ? "YES" : "no",
                f->FrameCount.ToString()
            );
            _fishGrid->Rows[row]->Tag = f->FishID;

            // Row background by status
            _fishGrid->Rows[row]->DefaultCellStyle->BackColor =
                (f->IsIsolated) ? Color::FromArgb(48, 38, 20) : Color::FromArgb(26, 29, 40);

            // Activity cell color
            Color actColor;
            switch (f->Activity) {
            case ActivityLevel::Resting:  actColor = Color::FromArgb(80, 220, 120);  break;
            case ActivityLevel::Cruising: actColor = Color::FromArgb(200, 200, 80);  break;
            case ActivityLevel::Active:   actColor = Color::FromArgb(255, 140, 40);  break;
            case ActivityLevel::Erratic:  actColor = Color::FromArgb(255, 80,  80);  break;
            default:                      actColor = Color::FromArgb(130, 130, 140); break;
            }
            _fishGrid->Rows[row]->Cells["BColActivity"]->Style->ForeColor = actColor;

            if (f->IsIsolated)
                _fishGrid->Rows[row]->Cells["BColIso"]->Style->ForeColor = Color::FromArgb(255, 140, 80);
        }

        // ── Update alerts list ───────────────────────────────────────────────
        _alertList->Items->Clear();
        const float LETHARGY_RATIO    = 0.20f;
        const float HYPERACTIVE_RATIO = 3.0f;
        bool anyAlert = false;

        if (pondAvgSpeed > 0.1f && activeCount > 1) {
            for each (FishTrack^ f in fishes) {
                if (f->Status == FishStatus::Lost || f->FrameCount < 30) continue;
                if (f->AvgSpeed < pondAvgSpeed * LETHARGY_RATIO) {
                    _alertList->Items->Add(String::Format(
                        "[LETHARGIC]   Fish #{0}   spd={1:F1}  (pond avg={2:F1})",
                        f->FishID, f->AvgSpeed, pondAvgSpeed));
                    anyAlert = true;
                }
                if (f->AvgSpeed > pondAvgSpeed * HYPERACTIVE_RATIO) {
                    _alertList->Items->Add(String::Format(
                        "[HYPERACTIVE] Fish #{0}   spd={1:F1}  (pond avg={2:F1})",
                        f->FishID, f->AvgSpeed, pondAvgSpeed));
                    anyAlert = true;
                }
                if (f->IsIsolated) {
                    _alertList->Items->Add(String::Format(
                        "[ISOLATED]    Fish #{0}   alone for {1} frames",
                        f->FishID, f->IsolationFrames));
                    anyAlert = true;
                }
            }
        }
        if (!anyAlert) {
            _alertList->Items->Add("  No behavioral alerts detected.");
            _alertList->ForeColor = Color::FromArgb(100, 200, 120);
        }
        else {
            _alertList->ForeColor = Color::FromArgb(255, 180, 80);
        }

        // ── Activity distribution + top/slowest fish ─────────────────────────
        int restCount = 0, cruiseCount = 0, activeAct = 0, erraticCount = 0;
        FishTrack^ fastest = nullptr;
        FishTrack^ slowest = nullptr;
        for each (FishTrack^ f in fishes) {
            if (f->Status == FishStatus::Lost) continue;
            switch (f->Activity) {
            case ActivityLevel::Resting:  restCount++;    break;
            case ActivityLevel::Cruising: cruiseCount++;  break;
            case ActivityLevel::Active:   activeAct++;    break;
            case ActivityLevel::Erratic:  erraticCount++; break;
            }
            if (fastest == nullptr || f->AvgSpeed > fastest->AvgSpeed) fastest = f;
            if (f->FrameCount >= 10)
                if (slowest == nullptr || f->AvgSpeed < slowest->AvgSpeed) slowest = f;
        }

        System::Text::StringBuilder^ sb = gcnew System::Text::StringBuilder();
        sb->AppendFormat("Z Resting   {0}\n", restCount);
        sb->AppendFormat("~ Cruising  {0}\n", cruiseCount);
        sb->AppendFormat("! Active    {0}\n", activeAct);
        sb->AppendFormat("!!Erratic   {0}\n\n", erraticCount);

        if (fastest != nullptr)
            sb->AppendFormat("Fastest:\n  #{0} ({1:F1} px/f)\n\n",
                fastest->FishID, fastest->AvgSpeed);
        if (slowest != nullptr && slowest != fastest)
            sb->AppendFormat("Slowest:\n  #{0} ({1:F1} px/f)\n",
                slowest->FishID, slowest->AvgSpeed);

        sb->AppendFormat("\nPool avg:\n  {0:F1} px/f\n", pondAvgSpeed);
        sb->AppendFormat("Schooling:\n  {0:F0}%\n", schoolingPct);

        _lblAnalysis->Text = sb->ToString();

        // ── Refresh charts ───────────────────────────────────────────────────
        _speedChart->Fishes        = fishes;
        _speedChart->PondAvgSpeed  = pondAvgSpeed;
        _speedChart->Invalidate();

        _zoneMap->Fishes = fishes;
        _zoneMap->Invalidate();
    }
}
