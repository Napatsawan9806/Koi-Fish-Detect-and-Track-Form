#include "AnalysisDashboardForm.h"

namespace KoiTracker {

    // ============================================================
    //  TrendChartPanel — DrawSeries helper (no lambda)
    // ============================================================
    void TrendChartPanel::DrawSeries(Graphics^ g, int seriesIdx, float maxVal,
        Color col, int W, int H, float xStep)
    {
        int n = History->Count;
        array<PointF>^ pts = gcnew array<PointF>(n);
        for (int i = 0; i < n; i++) {
            float v;
            switch (seriesIdx) {
            case 0: v = History[i].AvgSpeed;              break;
            case 1: v = History[i].SchoolingPct;          break;
            case 2: v = (float)History[i].AlertCount;     break;
            default: v = 0.0f;                            break;
            }
            if (v > maxVal) v = maxVal;
            float px = (MAX_HISTORY - n + i) * xStep;
            float py = H - (v / maxVal) * (H - 4) - 2;
            pts[i] = PointF(px, py);
        }
        g->DrawLines(gcnew Pen(col, 1.5f), pts);
    }

    // ============================================================
    //  TrendChartPanel — OnPaint (no lambda)
    // ============================================================
    void TrendChartPanel::OnPaint(PaintEventArgs^ e)
    {
        Graphics^ g = e->Graphics;
        g->SmoothingMode = Drawing2D::SmoothingMode::AntiAlias;
        int W = this->Width, H = this->Height;

        g->Clear(Color::FromArgb(10, 12, 22));
        Pen^ gridPen = gcnew Pen(Color::FromArgb(30, 40, 60));
        for (int i = 1; i < 5; i++)
            g->DrawLine(gridPen, 0, H * i / 5, W, H * i / 5);

        if (History->Count < 2) return;

        float xStep = (float)W / (MAX_HISTORY - 1);
        DrawSeries(g, 0, 30.0f, Color::FromArgb(100, 180, 255), W, H, xStep);  // Speed
        DrawSeries(g, 1, 100.0f, Color::FromArgb(80, 220, 120), W, H, xStep);  // Schooling
        DrawSeries(g, 2, 10.0f, Color::FromArgb(255, 150, 50), W, H, xStep);  // Alerts

        // Y-axis labels (right side)
        Drawing::Font^ fnt = gcnew Drawing::Font("Consolas", 7);
        SolidBrush^ dimBr = gcnew SolidBrush(Color::FromArgb(50, 65, 95));
        g->DrawString(L"100%", fnt, dimBr, (float)(W - 32), 2.0f);
        g->DrawString(L"50%", fnt, dimBr, (float)(W - 26), (float)(H / 2 - 6));
        g->DrawString(L"0", fnt, dimBr, (float)(W - 10), (float)(H - 14));
    }

    // ============================================================
    //  Constructor
    // ============================================================
    AnalysisDashboardForm::AnalysisDashboardForm() {
        _rawScoreHistory = gcnew array<int>(SCORE_SMOOTH);
        for (int i = 0; i < SCORE_SMOOTH; i++) _rawScoreHistory[i] = 100;
        _scoreHistoryIdx = 0;
        _smoothedScore = 100;
        InitializeComponent();
    }

    // ============================================================
    //  InitializeComponent
    // ============================================================
    void AnalysisDashboardForm::InitializeComponent() {
        this->Text = L"Koi Dashboard \x2014 Health & Trend";
        this->ClientSize = System::Drawing::Size(860, 828);
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::Sizable;
        this->MaximizeBox = true;
        this->MinimizeBox = true;
        this->StartPosition = FormStartPosition::WindowsDefaultLocation;
        this->BackColor = Color::FromArgb(10, 13, 24);
        this->ForeColor = Color::White;
        this->Font = gcnew Drawing::Font("Segoe UI", 9);
        this->MinimumSize = System::Drawing::Size(700, 560);

        // ─── HEADER ──────────────────────────────────────────────────────
        Panel^ header = gcnew Panel();
        header->BackColor = Color::FromArgb(18, 22, 48);
        header->Location = Point(0, 0);
        header->Size = System::Drawing::Size(860, 64);
        header->Anchor = AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right;

        // Left: icon dot
        Panel^ headerDot = gcnew Panel();
        headerDot->BackColor = Color::FromArgb(100, 180, 255);
        headerDot->Location = Point(14, 20);
        headerDot->Size = System::Drawing::Size(6, 24);
        header->Controls->Add(headerDot);

        // Left: title
        Label^ lblTitle = gcnew Label();
        lblTitle->Text = L"KOI HEALTH DASHBOARD";
        lblTitle->Font = gcnew Drawing::Font("Segoe UI", 13, FontStyle::Bold);
        lblTitle->ForeColor = Color::FromArgb(150, 200, 255);
        lblTitle->AutoSize = false;
        lblTitle->Location = Point(26, 10);
        lblTitle->Size = System::Drawing::Size(300, 26);
        header->Controls->Add(lblTitle);

        // Left: subtitle
        Label^ lblSub = gcnew Label();
        lblSub->Text = L"Live analysis  \x2022  Health Score  \x2022  Trend History";
        lblSub->Font = gcnew Drawing::Font("Segoe UI", 8);
        lblSub->ForeColor = Color::FromArgb(90, 110, 160);
        lblSub->AutoSize = false;
        lblSub->Location = Point(26, 38);
        lblSub->Size = System::Drawing::Size(360, 16);
        header->Controls->Add(lblSub);

        // Right: version badge
        Label^ lblVer = gcnew Label();
        lblVer->Text = L"v1.0";
        lblVer->Font = gcnew Drawing::Font("Segoe UI", 7.5f);
        lblVer->ForeColor = Color::FromArgb(60, 80, 130);
        lblVer->AutoSize = false;
        lblVer->Location = Point(560, 24);
        lblVer->Size = System::Drawing::Size(40, 16);
        lblVer->TextAlign = ContentAlignment::MiddleCenter;
        header->Controls->Add(lblVer);

        // Right: separator line
        Panel^ hdrSep = gcnew Panel();
        hdrSep->BackColor = Color::FromArgb(40, 50, 100);
        hdrSep->Location = Point(608, 16);
        hdrSep->Size = System::Drawing::Size(1, 32);
        header->Controls->Add(hdrSep);

        // Right: Clear Trend button
        Button^ btnClear = gcnew Button();
        btnClear->Text = L"\x21BA  Clear Trend";
        btnClear->Font = gcnew Drawing::Font("Segoe UI", 8.5f);
        btnClear->Location = Point(618, 17);
        btnClear->Size = System::Drawing::Size(106, 30);
        btnClear->BackColor = Color::FromArgb(30, 40, 80);
        btnClear->ForeColor = Color::FromArgb(140, 170, 240);
        btnClear->FlatStyle = FlatStyle::Flat;
        btnClear->FlatAppearance->BorderSize = 1;
        btnClear->FlatAppearance->BorderColor = Color::FromArgb(60, 90, 170);
        btnClear->Click += gcnew EventHandler(this, &AnalysisDashboardForm::BtnClear_Click);
        header->Controls->Add(btnClear);

        // Right: Close button
        Button^ btnCloseHdr = gcnew Button();
        btnCloseHdr->Text = L"\x2715";
        btnCloseHdr->Font = gcnew Drawing::Font("Segoe UI", 10, FontStyle::Bold);
        btnCloseHdr->Location = Point(732, 17);
        btnCloseHdr->Size = System::Drawing::Size(30, 30);
        btnCloseHdr->BackColor = Color::FromArgb(60, 28, 28);
        btnCloseHdr->ForeColor = Color::FromArgb(220, 100, 100);
        btnCloseHdr->FlatStyle = FlatStyle::Flat;
        btnCloseHdr->FlatAppearance->BorderSize = 0;
        btnCloseHdr->Click += gcnew EventHandler(this, &AnalysisDashboardForm::BtnClose_Click);
        header->Controls->Add(btnCloseHdr);

        // Bottom border line on header
        Panel^ headerBorder = gcnew Panel();
        headerBorder->BackColor = Color::FromArgb(35, 45, 100);
        headerBorder->Location = Point(0, 63);
        headerBorder->Size = System::Drawing::Size(860, 1);
        header->Controls->Add(headerBorder);

        this->Controls->Add(header);

        // ─── HEALTH SCORE PANEL ──────────────────────────────────────────
        Panel^ scorePanel = gcnew Panel();
        scorePanel->BackColor = Color::FromArgb(16, 20, 40);
        scorePanel->Location = Point(10, 72);
        scorePanel->Size = System::Drawing::Size(200, 130);
        scorePanel->BorderStyle = System::Windows::Forms::BorderStyle::None;

        Label^ capScore = gcnew Label();
        capScore->Text = L"HEALTH SCORE";
        capScore->Font = gcnew Drawing::Font("Segoe UI", 7.5f, FontStyle::Bold);
        capScore->ForeColor = Color::FromArgb(100, 110, 150);
        capScore->AutoSize = false;
        capScore->Location = Point(0, 8);
        capScore->Size = System::Drawing::Size(200, 18);
        capScore->TextAlign = ContentAlignment::MiddleCenter;
        scorePanel->Controls->Add(capScore);

        _lblScore = gcnew Label();
        _lblScore->Text = L"--";
        _lblScore->Font = gcnew Drawing::Font("Segoe UI", 38, FontStyle::Bold);
        _lblScore->ForeColor = Color::FromArgb(100, 220, 140);
        _lblScore->AutoSize = false;
        _lblScore->Location = Point(0, 26);
        _lblScore->Size = System::Drawing::Size(200, 60);
        _lblScore->TextAlign = ContentAlignment::MiddleCenter;
        scorePanel->Controls->Add(_lblScore);

        _scoreBar = gcnew Panel();
        _scoreBar->BackColor = Color::FromArgb(30, 35, 55);
        _scoreBar->Location = Point(14, 96);
        _scoreBar->Size = System::Drawing::Size(172, 12);
        scorePanel->Controls->Add(_scoreBar);

        _scoreBarFill = gcnew Panel();
        _scoreBarFill->BackColor = Color::FromArgb(80, 200, 120);
        _scoreBarFill->Location = Point(0, 0);
        _scoreBarFill->Size = System::Drawing::Size(0, 12);
        _scoreBar->Controls->Add(_scoreBarFill);

        _lblScoreCaption = gcnew Label();
        _lblScoreCaption->Text = L"Waiting...";
        _lblScoreCaption->Font = gcnew Drawing::Font("Segoe UI", 8, FontStyle::Italic);
        _lblScoreCaption->ForeColor = Color::FromArgb(130, 150, 180);
        _lblScoreCaption->AutoSize = false;
        _lblScoreCaption->Location = Point(0, 110);
        _lblScoreCaption->Size = System::Drawing::Size(200, 18);
        _lblScoreCaption->TextAlign = ContentAlignment::MiddleCenter;
        scorePanel->Controls->Add(_lblScoreCaption);
        this->Controls->Add(scorePanel);

        // ─── STAT CARDS ──────────────────────────────────────────────────
        int cardY = 72, cardX = 220;
        this->Controls->Add(MakeStatCard("ACTIVE FISH", _lblActive, Color::FromArgb(80, 220, 120), cardX, cardY));
        this->Controls->Add(MakeStatCard("AVG SPEED", _lblSpeed, Color::FromArgb(100, 180, 255), cardX + 108, cardY));
        this->Controls->Add(MakeStatCard("SCHOOLING %", _lblSchooling, Color::FromArgb(120, 255, 180), cardX + 216, cardY));
        this->Controls->Add(MakeStatCard("ISOLATED", _lblIsolated, Color::FromArgb(255, 140, 80), cardX + 324, cardY));
        this->Controls->Add(MakeStatCard("ALERTS", _lblAlerts, Color::FromArgb(255, 90, 90), cardX + 432, cardY));
        this->Controls->Add(MakeStatCard("HEATMAP PEAK", _lblPeak, Color::FromArgb(200, 160, 255), cardX + 540, cardY));

        _lblScoreDetail = gcnew Label();
        _lblScoreDetail->Text = L"";
        _lblScoreDetail->Font = gcnew Drawing::Font("Consolas", 7.5f);
        _lblScoreDetail->ForeColor = Color::FromArgb(110, 130, 170);
        _lblScoreDetail->AutoSize = false;
        _lblScoreDetail->Location = Point(220, 164);
        _lblScoreDetail->Size = System::Drawing::Size(626, 40);
        this->Controls->Add(_lblScoreDetail);

        // ─── TREND CHART HEADER + LEGEND EXPLANATION ─────────────────────
        Panel^ trendHeaderPanel = gcnew Panel();
        trendHeaderPanel->BackColor = Color::FromArgb(14, 18, 38);
        trendHeaderPanel->Location = Point(10, 212);
        trendHeaderPanel->Size = System::Drawing::Size(836, 54);
        this->Controls->Add(trendHeaderPanel);

        // Title
        Label^ trendHdr = gcnew Label();
        trendHdr->Text = L"TREND CHART  \x2014  Last 300 Frames";
        trendHdr->Font = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
        trendHdr->ForeColor = Color::FromArgb(130, 160, 220);
        trendHdr->AutoSize = false;
        trendHdr->Location = Point(8, 6);
        trendHdr->Size = System::Drawing::Size(280, 18);
        trendHeaderPanel->Controls->Add(trendHdr);

        // Description line
        Label^ trendDesc = gcnew Label();
        trendDesc->Text = L"Pond-wide averages over time  \x2022  X axis: time (newest = right)  \x2022  Y axis: value level (higher = more)";
        trendDesc->Font = gcnew Drawing::Font("Segoe UI", 7.5f);
        trendDesc->ForeColor = Color::FromArgb(80, 100, 150);
        trendDesc->AutoSize = false;
        trendDesc->Location = Point(8, 26);
        trendDesc->Size = System::Drawing::Size(500, 16);
        trendHeaderPanel->Controls->Add(trendDesc);

        // Legend: Avg Speed
        Panel^ dot1 = gcnew Panel();
        dot1->BackColor = Color::FromArgb(100, 180, 255);
        dot1->Location = Point(520, 14);
        dot1->Size = System::Drawing::Size(20, 4);
        trendHeaderPanel->Controls->Add(dot1);
        Label^ leg1 = gcnew Label();
        leg1->Text = L"Avg Speed (px/frame, max 30)";
        leg1->Font = gcnew Drawing::Font("Segoe UI", 7.5f);
        leg1->ForeColor = Color::FromArgb(100, 180, 255);
        leg1->AutoSize = false;
        leg1->Location = Point(544, 6);
        leg1->Size = System::Drawing::Size(180, 20);
        trendHeaderPanel->Controls->Add(leg1);

        // Legend: Schooling
        Panel^ dot2 = gcnew Panel();
        dot2->BackColor = Color::FromArgb(80, 220, 120);
        dot2->Location = Point(520, 32);
        dot2->Size = System::Drawing::Size(20, 4);
        trendHeaderPanel->Controls->Add(dot2);
        Label^ leg2 = gcnew Label();
        leg2->Text = L"Schooling % (0-100%)";
        leg2->Font = gcnew Drawing::Font("Segoe UI", 7.5f);
        leg2->ForeColor = Color::FromArgb(80, 220, 120);
        leg2->AutoSize = false;
        leg2->Location = Point(544, 24);
        leg2->Size = System::Drawing::Size(150, 20);
        trendHeaderPanel->Controls->Add(leg2);

        // Legend: Alerts
        Panel^ dot3 = gcnew Panel();
        dot3->BackColor = Color::FromArgb(255, 150, 50);
        dot3->Location = Point(700, 14);
        dot3->Size = System::Drawing::Size(20, 4);
        trendHeaderPanel->Controls->Add(dot3);
        Label^ leg3 = gcnew Label();
        leg3->Text = L"Alert Count (max 10)";
        leg3->Font = gcnew Drawing::Font("Segoe UI", 7.5f);
        leg3->ForeColor = Color::FromArgb(255, 150, 50);
        leg3->AutoSize = false;
        leg3->Location = Point(724, 6);
        leg3->Size = System::Drawing::Size(130, 20);
        trendHeaderPanel->Controls->Add(leg3);

        // How-to-read hint
        Label^ leg3hint = gcnew Label();
        leg3hint->Text = L"spike = many alerts";
        leg3hint->Font = gcnew Drawing::Font("Segoe UI", 7);
        leg3hint->ForeColor = Color::FromArgb(120, 80, 50);
        leg3hint->AutoSize = false;
        leg3hint->Location = Point(724, 24);
        leg3hint->Size = System::Drawing::Size(130, 16);
        trendHeaderPanel->Controls->Add(leg3hint);

        // ─── TREND CHART ─────────────────────────────────────────────────
        _trend = gcnew TrendChartPanel();
        _trend->Location = Point(10, 268);
        _trend->Size = System::Drawing::Size(836, 180);
        _trend->BackColor = Color::FromArgb(10, 12, 22);
        this->Controls->Add(_trend);

        // ─── ALERT LOG ───────────────────────────────────────────────────
        Label^ alertHdr = gcnew Label();
        alertHdr->Text = L"ALERT LOG  (current frame)";
        alertHdr->Font = gcnew Drawing::Font("Segoe UI", 8.5f, FontStyle::Bold);
        alertHdr->ForeColor = Color::FromArgb(100, 120, 170);
        alertHdr->AutoSize = false;
        alertHdr->Location = Point(10, 456);
        alertHdr->Size = System::Drawing::Size(220, 18);
        this->Controls->Add(alertHdr);

        _alertLog = gcnew ListBox();
        _alertLog->BackColor = Color::FromArgb(14, 16, 28);
        _alertLog->ForeColor = Color::FromArgb(255, 180, 80);
        _alertLog->BorderStyle = System::Windows::Forms::BorderStyle::None;
        _alertLog->Font = gcnew Drawing::Font("Consolas", 8.5f);
        _alertLog->Location = Point(10, 476);
        _alertLog->Size = System::Drawing::Size(836, 110);
        this->Controls->Add(_alertLog);

        // ─── POND HEALTH SUMMARY ─────────────────────────────────────────
        Panel^ summaryHeaderPanel = gcnew Panel();
        summaryHeaderPanel->BackColor = Color::FromArgb(14, 30, 20);
        summaryHeaderPanel->Location = Point(10, 594);
        summaryHeaderPanel->Size = System::Drawing::Size(836, 30);

        Label^ summaryHdr = gcnew Label();
        summaryHdr->Text = L"POND HEALTH SUMMARY";
        summaryHdr->Font = gcnew Drawing::Font("Segoe UI", 8.5f, FontStyle::Bold);
        summaryHdr->ForeColor = Color::FromArgb(100, 230, 140);
        summaryHdr->AutoSize = false;
        summaryHdr->Location = Point(8, 6);
        summaryHdr->Size = System::Drawing::Size(300, 18);
        summaryHeaderPanel->Controls->Add(summaryHdr);

        Label^ summaryNote = gcnew Label();
        summaryNote->Text = L"Computed from all tracked data";
        summaryNote->Font = gcnew Drawing::Font("Segoe UI", 7.5f, FontStyle::Italic);
        summaryNote->ForeColor = Color::FromArgb(70, 120, 90);
        summaryNote->AutoSize = false;
        summaryNote->Location = Point(310, 7);
        summaryNote->Size = System::Drawing::Size(220, 16);
        summaryHeaderPanel->Controls->Add(summaryNote);
        this->Controls->Add(summaryHeaderPanel);

        _summaryBox = gcnew RichTextBox();
        _summaryBox->BackColor = Color::FromArgb(10, 16, 12);
        _summaryBox->ForeColor = Color::FromArgb(180, 210, 190);
        _summaryBox->BorderStyle = System::Windows::Forms::BorderStyle::None;
        _summaryBox->Font = gcnew Drawing::Font("Consolas", 8.5f);
        _summaryBox->ReadOnly = true;
        _summaryBox->ScrollBars = RichTextBoxScrollBars::Vertical;
        _summaryBox->Location = Point(10, 624);
        _summaryBox->Size = System::Drawing::Size(836, 160);
        this->Controls->Add(_summaryBox);

        // ─── CLOSE BUTTON ────────────────────────────────────────────────
        Button^ btnClose = gcnew Button();
        btnClose->Text = L"Close";
        btnClose->Font = gcnew Drawing::Font("Segoe UI", 9);
        btnClose->Location = Point(380, 790);
        btnClose->Size = System::Drawing::Size(100, 28);
        btnClose->BackColor = Color::FromArgb(28, 38, 75);
        btnClose->ForeColor = Color::White;
        btnClose->FlatStyle = FlatStyle::Flat;
        btnClose->FlatAppearance->BorderSize = 0;
        btnClose->Click += gcnew EventHandler(this, &AnalysisDashboardForm::BtnClose_Click);
        this->Controls->Add(btnClose);
    }

    // ============================================================
    //  MakeStatCard
    // ============================================================
    Panel^ AnalysisDashboardForm::MakeStatCard(String^ caption, Label^% valueLabel,
        Color valueColor, int x, int y)
    {
        Panel^ card = gcnew Panel();
        card->BackColor = Color::FromArgb(18, 22, 40);
        card->Location = Point(x, y);
        card->Size = System::Drawing::Size(100, 86);

        Label^ val = gcnew Label();
        val->Text = L"--";
        val->Font = gcnew Drawing::Font("Segoe UI", 18, FontStyle::Bold);
        val->ForeColor = valueColor;
        val->AutoSize = false;
        val->Location = Point(0, 14);
        val->Size = System::Drawing::Size(100, 36);
        val->TextAlign = ContentAlignment::MiddleCenter;
        card->Controls->Add(val);
        valueLabel = val;

        Label^ cap = gcnew Label();
        cap->Text = caption;
        cap->Font = gcnew Drawing::Font("Segoe UI", 6.5f);
        cap->ForeColor = Color::FromArgb(90, 100, 135);
        cap->AutoSize = false;
        cap->Location = Point(0, 56);
        cap->Size = System::Drawing::Size(100, 20);
        cap->TextAlign = ContentAlignment::MiddleCenter;
        card->Controls->Add(cap);

        return card;
    }

    // ============================================================
    //  ComputeHealthScore
    // ============================================================
    int AnalysisDashboardForm::ComputeHealthScore(List<FishTrack^>^ fishes,
        float pondAvg,
        float schoolingPct,
        int   alertCount)
    {
        int score = 100;
        int active = 0, erratic = 0, isolated = 0;
        for each(FishTrack ^ f in fishes) {
            if (f->Status == FishStatus::Lost) continue;
            active++;
            if (f->Activity == ActivityLevel::Erratic) erratic++;
            if (f->IsIsolated) isolated++;
        }

        // Alert penalty: ลดลงทีละ 3 คะแนน แต่สูงสุดลดได้แค่ 30
        // (เดิม alertCount*5 ไม่มี cap — ทำให้กระโดดแรง)
        score -= Math::Min(alertCount * 3, 30);

        // Isolated penalty: ลดลงทีละ 6 แต่สูงสุด 24
        score -= Math::Min(isolated * 6, 24);

        // Speed penalty/bonus
        if (pondAvg < 0.1f) score -= 10;

        // Schooling bonus
        if (schoolingPct > 60.0f) score += 10;

        // Erratic penalty
        if (active > 0 && (float)erratic / active > 0.30f) score -= 10;

        return Math::Max(0, Math::Min(100, score));
    }

    // ============================================================
    //  ScoreColor
    // ============================================================
    Color AnalysisDashboardForm::ScoreColor(int score) {
        if (score >= 80) return Color::FromArgb(80, 220, 120);
        if (score >= 55) return Color::FromArgb(220, 200, 60);
        if (score >= 35) return Color::FromArgb(255, 140, 50);
        return                   Color::FromArgb(255, 70, 70);
    }

    // ============================================================
    //  UpdateDashboard
    // ============================================================
    void AnalysisDashboardForm::UpdateDashboard(List<FishTrack^>^ fishes,
        int heatmapPeak,
        int heatmapFrames)
    {
        // ── Basic metrics ──────────────────────────────────────────────
        float totalSpeed = 0.0f;
        int   activeCount = 0;
        for each(FishTrack ^ f in fishes) {
            if (f->Status == FishStatus::Lost) continue;
            totalSpeed += f->AvgSpeed;
            activeCount++;
        }
        float pondAvg = (activeCount > 0) ? totalSpeed / activeCount : 0.0f;

        // Schooling
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
        float schoolingPct = (activeCount > 0)
            ? (float)schoolingCount / activeCount * 100.0f : 0.0f;

        int isolatedCount = 0;
        for each(FishTrack ^ f in fishes)
            if (f->IsIsolated) isolatedCount++;

        // Alerts
        int alertCount = 0;
        _alertLog->Items->Clear();
        const float LETHARGY_RATIO = 0.20f;
        const float HYPERACTIVE_RATIO = 3.0f;

        if (pondAvg > 0.1f && activeCount > 1) {
            for each(FishTrack ^ f in fishes) {
                if (f->Status == FishStatus::Lost || f->FrameCount < 30) continue;
                if (f->AvgSpeed < pondAvg * LETHARGY_RATIO) {
                    _alertLog->Items->Add(String::Format(
                        "[LETHARGIC]    Fish #{0}  spd={1:F1}  avg={2:F1}",
                        f->FishID, f->AvgSpeed, pondAvg));
                    alertCount++;
                }
                if (f->AvgSpeed > pondAvg * HYPERACTIVE_RATIO) {
                    _alertLog->Items->Add(String::Format(
                        "[HYPERACTIVE]  Fish #{0}  spd={1:F1}  avg={2:F1}",
                        f->FishID, f->AvgSpeed, pondAvg));
                    alertCount++;
                }
                if (f->IsIsolated) {
                    _alertLog->Items->Add(String::Format(
                        "[ISOLATED]     Fish #{0}  alone {1} frames",
                        f->FishID, f->IsolationFrames));
                    alertCount++;
                }
            }
        }
        if (_alertLog->Items->Count == 0) {
            _alertLog->Items->Add("  No behavioral alerts detected.");
            _alertLog->ForeColor = Color::FromArgb(80, 180, 100);
        }
        else {
            _alertLog->ForeColor = Color::FromArgb(255, 180, 80);
        }

        // Health score — compute raw then smooth with rolling average
        int rawScore = ComputeHealthScore(fishes, pondAvg, schoolingPct, alertCount);
        _rawScoreHistory[_scoreHistoryIdx] = rawScore;
        _scoreHistoryIdx = (_scoreHistoryIdx + 1) % SCORE_SMOOTH;

        int scoreSum = 0;
        for (int i = 0; i < SCORE_SMOOTH; i++) scoreSum += _rawScoreHistory[i];
        int   score = scoreSum / SCORE_SMOOTH;
        Color sColor = ScoreColor(score);

        _lblScore->Text = score.ToString();
        _lblScore->ForeColor = sColor;
        _scoreBarFill->Width = (int)(_scoreBar->Width * score / 100.0f);
        _scoreBarFill->BackColor = sColor;

        String^ caption;
        if (score >= 80) caption = L"Excellent - pond looks healthy";
        else if (score >= 60) caption = L"Good - minor issues detected";
        else if (score >= 40) caption = L"Fair - monitor closely";
        else                  caption = L"Poor - action recommended";
        _lblScoreCaption->Text = caption;

        // Score breakdown
        int erraticCnt = 0, actCnt = 0;
        for each(FishTrack ^ f in fishes) {
            if (f->Status == FishStatus::Lost) continue;
            actCnt++;
            if (f->Activity == ActivityLevel::Erratic) erraticCnt++;
        }
        _lblScoreDetail->Text = String::Format(
            "Raw {0}  ->  Smoothed {1}/100  |  Alerts -{2}  |  Isolated -{3}  |  Speed {4}  |  Schooling {5}  |  Erratic {6}",
            rawScore,
            score,
            Math::Min(alertCount * 3, 30),
            Math::Min(isolatedCount * 6, 24),
            (pondAvg < 0.1f ? "-10 (no movement)" : "OK"),
            (schoolingPct > 60.0f ? "+10" : "--"),
            (actCnt > 0 && (float)erraticCnt / actCnt > 0.30f ? "-10" : "OK"));

        // Stat cards
        _lblActive->Text = activeCount.ToString();
        _lblSpeed->Text = String::Format("{0:F1}", pondAvg);
        _lblSchooling->Text = String::Format("{0:F0}%", schoolingPct);
        _lblIsolated->Text = isolatedCount.ToString();
        _lblAlerts->Text = alertCount.ToString();
        _lblPeak->Text = heatmapPeak.ToString();
        _lblAlerts->ForeColor = (alertCount > 0)
            ? Color::FromArgb(255, 90, 90)
            : Color::FromArgb(80, 220, 120);

        // Push trend
        TrendPoint tp;
        tp.AvgSpeed = pondAvg;
        tp.SchoolingPct = schoolingPct;
        tp.ActiveCount = activeCount;
        tp.AlertCount = alertCount;
        _trend->Push(tp);

        // Summary text
        UpdatePondSummary(fishes, pondAvg, schoolingPct,
            isolatedCount, alertCount,
            score, heatmapPeak, heatmapFrames);
    }

    // ============================================================
    //  UpdatePondSummary
    // ============================================================
    void AnalysisDashboardForm::UpdatePondSummary(
        List<FishTrack^>^ fishes,
        float pondAvg, float schoolingPct,
        int isolatedCount, int alertCount,
        int score, int heatmapPeak, int heatmapFrames)
    {
        int activeCount = 0, restCount = 0, cruiseCount = 0,
            activeAct = 0, erraticCount = 0;
        FishTrack^ fastest = nullptr;
        FishTrack^ slowest = nullptr;

        for each(FishTrack ^ f in fishes) {
            if (f->Status == FishStatus::Lost) continue;
            activeCount++;
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

        float erraticRatio = (activeCount > 0) ? (float)erraticCount / activeCount * 100.0f : 0.0f;
        float restingRatio = (activeCount > 0) ? (float)restCount / activeCount * 100.0f : 0.0f;

        // Trend analysis
        float trendSpeedDelta = 0.0f;
        float trendSchoolDelta = 0.0f;
        bool  hasTrend = false;
        int   trendAlertTotal = 0;

        if (_trend->History->Count >= 2) {
            hasTrend = true;
            int n = _trend->History->Count;
            int lb = Math::Min(60, n);
            trendSpeedDelta = _trend->History[n - 1].AvgSpeed - _trend->History[n - lb].AvgSpeed;
            trendSchoolDelta = _trend->History[n - 1].SchoolingPct - _trend->History[n - lb].SchoolingPct;
            for (int i = Math::Max(0, n - 300); i < n; i++)
                trendAlertTotal += _trend->History[i].AlertCount;
        }

        System::Text::StringBuilder^ sb = gcnew System::Text::StringBuilder();

        // Verdict
        String^ verdict;
        if (score >= 80) verdict = L"[OK] Overall pond health: EXCELLENT";
        else if (score >= 60) verdict = L"[OK] Overall pond health: GOOD";
        else if (score >= 40) verdict = L"[!!] Overall pond health: FAIR";
        else                  verdict = L"[!!] Overall pond health: POOR";

        sb->AppendFormat("{0}  (Score {1}/100)\n", verdict, score);
        sb->Append(L"-------------------------------------------------------------------------\n");

        // Fish activity breakdown
        sb->AppendFormat(
            L"Fish tracked : {0} active   |   Resting {1}  Cruising {2}  Active {3}  Erratic {4}\n",
            activeCount, restCount, cruiseCount, activeAct, erraticCount);

        // Speed
        if (fastest != nullptr) {
            int slowID = (slowest != nullptr) ? slowest->FishID : -1;
            float slowSpd = (slowest != nullptr) ? slowest->AvgSpeed : 0.0f;
            sb->AppendFormat(
                L"Speed        : avg {0:F1} px/f   fastest #{1} ({2:F1})   slowest #{3} ({4:F1})\n",
                pondAvg, fastest->FishID, fastest->AvgSpeed, slowID, slowSpd);
        }

        // Schooling
        sb->AppendFormat(
            L"Schooling    : {0:F0}%  ({1}/{2} fish in groups)   Isolated: {3} fish\n",
            schoolingPct, (int)(schoolingPct * activeCount / 100.0f),
            activeCount, isolatedCount);

        // Heatmap
        if (heatmapFrames > 0) {
            String^ hmNote = (heatmapPeak > 50)
                ? L"High concentration detected -> check that zone"
                : L"Coverage looks distributed";
            sb->AppendFormat(
                L"Heatmap      : {0} frames accumulated   peak density = {1}   {2}\n",
                heatmapFrames, heatmapPeak, hmNote);
        }

        // Alerts
        sb->AppendFormat(
            L"Alerts now   : {0}   |   Cumulative alerts (last 300f): {1}\n",
            alertCount, trendAlertTotal);

        sb->Append(L"-------------------------------------------------------------------------\n");
        sb->Append(L"Interpretation:\n");

        // Speed interpretation
        if (pondAvg < 0.5f)
            sb->Append(L"  * Very low movement detected. Fish may be lethargic - check oxygen and water temperature.\n");
        else if (pondAvg > 20.0f)
            sb->Append(L"  * Unusually high average speed. Possible stress - inspect for predators or water quality issues.\n");
        else
            sb->Append(L"  * Movement speed is within normal range.\n");

        // Erratic interpretation
        if (erraticRatio > 30.0f)
            sb->AppendFormat(L"  * {0:F0}% erratic movement. Strong stress indicator - check ammonia, pH, or overcrowding.\n", erraticRatio);
        else if (erraticRatio > 10.0f)
            sb->AppendFormat(L"  * {0:F0}% erratic fish. Minor stress present - monitor closely.\n", erraticRatio);

        // Resting interpretation
        if (restingRatio > 60.0f)
            sb->AppendFormat(L"  * {0:F0}% of fish resting. Possible low dissolved oxygen or illness. Verify water parameters.\n", restingRatio);

        // Schooling interpretation
        if (schoolingPct < 20.0f && activeCount > 2)
            sb->Append(L"  * Low schooling rate. Fish avoiding each other - possible territorial stress or injury.\n");
        else if (schoolingPct > 80.0f)
            sb->Append(L"  * High schooling rate. Fish grouping well - good sign of social comfort.\n");

        // Isolated interpretation
        if (isolatedCount > 0)
            sb->AppendFormat(L"  * {0} fish isolated. May be ill, injured, or being bullied. Observe individually.\n", isolatedCount);

        // Trend interpretation
        if (hasTrend) {
            if (trendSpeedDelta < -2.0f)
                sb->Append(L"  * Speed trend declining. Activity level dropping - watch for lethargy.\n");
            else if (trendSpeedDelta > 2.0f)
                sb->Append(L"  * Speed trend rising. Increased activity - may indicate feeding time or early stress.\n");
            if (trendSchoolDelta < -15.0f)
                sb->Append(L"  * Schooling % has decreased recently. Group cohesion breaking down - investigate.\n");
        }

        // Cumulative alerts
        if (trendAlertTotal > 20)
            sb->AppendFormat(L"  * High cumulative alert count ({0}) in last 300 frames. Persistent problems - take action.\n", trendAlertTotal);
        else if (trendAlertTotal == 0 && hasTrend)
            sb->Append(L"  * No alerts in recent history. Pond is stable.\n");

        // Recommendation
        sb->Append(L"-------------------------------------------------------------------------\n");
        if (score >= 80)
            sb->Append(L"Recommendation : Pond is healthy. Continue normal feeding and maintenance schedule.\n");
        else if (score >= 60)
            sb->Append(L"Recommendation : Minor concerns present. Check water quality parameters within 24 hours.\n");
        else if (score >= 40)
            sb->Append(L"Recommendation : Several issues detected. Test water quality immediately and observe isolated fish.\n");
        else
            sb->Append(L"Recommendation : Critical condition. Perform immediate water test, aeration check, and fish health inspection.\n");

        _summaryBox->Text = sb->ToString();

        // Colour the verdict line
        _summaryBox->SelectionStart = 0;
        _summaryBox->SelectionLength = (_summaryBox->Lines->Length > 0)
            ? _summaryBox->Lines[0]->Length : 0;
        _summaryBox->SelectionColor = ScoreColor(score);
        _summaryBox->SelectionStart = 0;
    }

    // ============================================================
    //  Button handlers
    // ============================================================
    void AnalysisDashboardForm::BtnClear_Click(Object^ sender, EventArgs^ e) {
        _trend->Reset();
    }

    void AnalysisDashboardForm::BtnClose_Click(Object^ sender, EventArgs^ e) {
        this->Hide();
    }

} // namespace KoiTracker