#include "SchoolingForm.h"

namespace KoiTracker {

    SchoolingForm::SchoolingForm() {
        InitializeComponent();
    }

    void SchoolingForm::InitializeComponent() {
        this->Text = L"Schooling & Social Analysis  \x2014  Live";
        this->ClientSize = System::Drawing::Size(700, 580);
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::Sizable;
        this->MaximizeBox = true;
        this->MinimizeBox = true;
        this->StartPosition = FormStartPosition::WindowsDefaultLocation;
        this->BackColor = Color::FromArgb(6, 10, 18);
        this->ForeColor = Color::White;
        this->Font = gcnew Drawing::Font("Segoe UI", 9);
        this->MinimumSize = System::Drawing::Size(520, 450);

        // ─── HEADER ───────────────────────────────────────────────────────
        // Layout (height=56): left block = title (row1) + stats (row2)
        //                     center     = legend (vertically centered, 2 lines)
        //                     right      = Close button (vertically centered)
        Panel^ header = gcnew Panel();
        header->BackColor = Color::FromArgb(12, 35, 22);
        header->Location = Point(0, 0);
        header->Size = System::Drawing::Size(700, 56);
        header->Anchor = AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right;

        // Row 1 – title (left)
        Label^ lblTitle = gcnew Label();
        lblTitle->AutoSize = false;
        lblTitle->Text = L"SCHOOLING  \x2022  SOCIAL NETWORK";
        lblTitle->Font = gcnew Drawing::Font("Segoe UI", 11, FontStyle::Bold);
        lblTitle->ForeColor = Color::FromArgb(120, 255, 170);
        lblTitle->Location = Point(12, 8);
        lblTitle->Size = System::Drawing::Size(270, 22);
        header->Controls->Add(lblTitle);

        // Row 2 – stats (left, under title)
        _lblStats = gcnew Label();
        _lblStats->AutoSize = false;
        _lblStats->Text = L"Groups: 0  |  Schooling: 0 fish  |  Solo: 0";
        _lblStats->Font = gcnew Drawing::Font("Consolas", 7.5f);
        _lblStats->ForeColor = Color::FromArgb(130, 200, 150);
        _lblStats->Location = Point(12, 32);
        _lblStats->Size = System::Drawing::Size(270, 16);
        header->Controls->Add(_lblStats);

        // Center – legend (2 lines, vertically centered)
        Label^ lblLegend = gcnew Label();
        lblLegend->AutoSize = false;
        lblLegend->Text = L"Ellipse = school group  |  Blinking ring = isolated\nZ = resting  |  ! = active  |  !! = erratic";
        lblLegend->Font = gcnew Drawing::Font("Segoe UI", 7.5f);
        lblLegend->ForeColor = Color::FromArgb(100, 155, 115);
        lblLegend->TextAlign = ContentAlignment::MiddleCenter;
        lblLegend->Location = Point(290, 0);
        lblLegend->Size = System::Drawing::Size(316, 56);
        header->Controls->Add(lblLegend);

        // Right – Close button (vertically centered)
        Button^ btnClose = gcnew Button();
        btnClose->Text = L"Close";
        btnClose->Font = gcnew Drawing::Font("Segoe UI", 8.5f);
        btnClose->Location = Point(616, 14);
        btnClose->Size = System::Drawing::Size(72, 28);
        btnClose->BackColor = Color::FromArgb(20, 65, 40);
        btnClose->ForeColor = Color::White;
        btnClose->FlatStyle = FlatStyle::Flat;
        btnClose->FlatAppearance->BorderSize = 1;
        btnClose->FlatAppearance->BorderColor = Color::FromArgb(50, 130, 80);
        btnClose->Click += gcnew EventHandler(this, &SchoolingForm::BtnClose_Click);
        header->Controls->Add(btnClose);

        this->Controls->Add(header);

        // ─── CANVAS ───────────────────────────────────────────────────────
        _canvas = gcnew SchoolingPanel();
        _canvas->Location = Point(10, 60);
        _canvas->Size = System::Drawing::Size(680, 510);
        _canvas->BackColor = Color::FromArgb(6, 10, 18);
        this->Controls->Add(_canvas);
    }

    void SchoolingForm::UpdateFishes(List<FishTrack^>^ fishes, int videoW, int videoH) {
        _canvas->UpdateFishes(fishes, videoW, videoH);
        // Update stats from computed values in canvas
        int active = 0;
        for each (FishTrack ^ f in fishes)
            if (f->Status != FishStatus::Lost) active++;
        _lblStats->Text = String::Format(
            "Groups: {0}  |  Schooling: {1} fish  |  Solo: {2}  |  Active: {3}",
            _canvas->LastGroupCount,
            _canvas->LastSchoolingCount,
            _canvas->LastSoloCount,
            active);
    }

    void SchoolingForm::BtnClose_Click(Object^ sender, EventArgs^ e) {
        this->Close();
    }
}