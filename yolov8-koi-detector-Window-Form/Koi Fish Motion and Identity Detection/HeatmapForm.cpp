#include "HeatmapForm.h"

namespace KoiTracker {

    HeatmapForm::HeatmapForm() {
        InitializeComponent();
    }

    void HeatmapForm::InitializeComponent() {
        this->Text            = L"Position Heatmap  \x2014  Accumulated Coverage";
        this->ClientSize      = System::Drawing::Size(690, 578);
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::Sizable;
        this->MaximizeBox     = true;
        this->MinimizeBox     = true;
        this->StartPosition   = FormStartPosition::WindowsDefaultLocation;
        this->BackColor       = Color::FromArgb(6, 10, 24);
        this->ForeColor       = Color::White;
        this->Font            = gcnew Drawing::Font("Segoe UI", 9);
        this->MinimumSize     = System::Drawing::Size(520, 450);

        // ─── HEADER ───────────────────────────────────────────────────────
        Panel^ header = gcnew Panel();
        header->BackColor = Color::FromArgb(8, 18, 50);
        header->Location  = Point(0, 0);
        header->Size      = System::Drawing::Size(690, 52);

        Label^ lblTitle = gcnew Label();
        lblTitle->AutoSize  = false;
        lblTitle->Text      = L"HEATMAP  \x2022  POSITION DENSITY";
        lblTitle->Font      = gcnew Drawing::Font("Segoe UI", 11, FontStyle::Bold);
        lblTitle->ForeColor = Color::FromArgb(100, 180, 255);
        lblTitle->Location  = Point(10, 14);
        lblTitle->Size      = System::Drawing::Size(310, 26);
        header->Controls->Add(lblTitle);

        // Color scale hint
        Label^ lblScale = gcnew Label();
        lblScale->AutoSize  = false;
        lblScale->Text      = L"Blue \x2192 Cyan \x2192 Yellow \x2192 Red  (Low \x2192 High density)";
        lblScale->Font      = gcnew Drawing::Font("Segoe UI", 8);
        lblScale->ForeColor = Color::FromArgb(120, 140, 180);
        lblScale->Location  = Point(320, 18);
        lblScale->Size      = System::Drawing::Size(280, 18);
        header->Controls->Add(lblScale);

        // Stats label
        _lblStats = gcnew Label();
        _lblStats->AutoSize  = false;
        _lblStats->Text      = L"Frames: 0  |  Active Fish: 0  |  Peak: 0";
        _lblStats->Font      = gcnew Drawing::Font("Consolas", 8);
        _lblStats->ForeColor = Color::FromArgb(130, 160, 200);
        _lblStats->Location  = Point(10, 34);
        _lblStats->Size      = System::Drawing::Size(300, 16);
        header->Controls->Add(_lblStats);

        // Clear button
        Button^ btnClear = gcnew Button();
        btnClear->Text      = L"Clear";
        btnClear->Font      = gcnew Drawing::Font("Segoe UI", 8.5f);
        btnClear->Location  = Point(578, 12);
        btnClear->Size      = System::Drawing::Size(70, 28);
        btnClear->BackColor = Color::FromArgb(35, 55, 110);
        btnClear->ForeColor = Color::White;
        btnClear->FlatStyle = FlatStyle::Flat;
        btnClear->FlatAppearance->BorderSize = 0;
        btnClear->Click    += gcnew EventHandler(this, &HeatmapForm::BtnClear_Click);
        header->Controls->Add(btnClear);

        this->Controls->Add(header);

        // ─── CANVAS ───────────────────────────────────────────────────────
        _canvas = gcnew HeatmapPanel();
        _canvas->Location  = Point(10, 56);
        _canvas->Size      = System::Drawing::Size(670, 502);
        _canvas->BackColor = Color::FromArgb(4, 7, 22);
        this->Controls->Add(_canvas);

        // ─── CLOSE BUTTON ─────────────────────────────────────────────────
        Button^ btnClose = gcnew Button();
        btnClose->Text      = L"Close";
        btnClose->Font      = gcnew Drawing::Font("Segoe UI", 9);
        btnClose->Location  = Point(295, 561);
        btnClose->Size      = System::Drawing::Size(100, 30);
        btnClose->BackColor = Color::FromArgb(30, 45, 90);
        btnClose->ForeColor = Color::White;
        btnClose->FlatStyle = FlatStyle::Flat;
        btnClose->FlatAppearance->BorderSize = 0;
        btnClose->Click    += gcnew EventHandler(this, &HeatmapForm::BtnClose_Click);
        this->Controls->Add(btnClose);
    }

    void HeatmapForm::Accumulate(List<FishTrack^>^ fishes, int videoW, int videoH) {
        _canvas->Accumulate(fishes, videoW, videoH);
        _lblStats->Text = String::Format(
            "Frames: {0}  |  Active Fish: {1}  |  Peak: {2}",
            _canvas->FrameCount, _canvas->ActiveFish, _canvas->MaxVal);
    }

    void HeatmapForm::BtnClear_Click(Object^ sender, EventArgs^ e) {
        _canvas->Reset();
        _lblStats->Text = L"Frames: 0  |  Active Fish: 0  |  Peak: 0";
    }

    void HeatmapForm::BtnClose_Click(Object^ sender, EventArgs^ e) {
        this->Hide();   // hide rather than close so accumulation can continue
    }
}
