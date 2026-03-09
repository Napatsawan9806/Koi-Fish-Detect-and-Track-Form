#include "SchoolingForm.h"

namespace KoiTracker {

    SchoolingForm::SchoolingForm() {
        InitializeComponent();
    }

    void SchoolingForm::InitializeComponent() {
        this->Text            = L"Schooling & Social Analysis  \x2014  Live";
        this->ClientSize      = System::Drawing::Size(700, 580);
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::Sizable;
        this->MaximizeBox     = true;
        this->MinimizeBox     = true;
        this->StartPosition   = FormStartPosition::WindowsDefaultLocation;
        this->BackColor       = Color::FromArgb(6, 10, 18);
        this->ForeColor       = Color::White;
        this->Font            = gcnew Drawing::Font("Segoe UI", 9);
        this->MinimumSize     = System::Drawing::Size(520, 450);

        // ─── HEADER ───────────────────────────────────────────────────────
        Panel^ header = gcnew Panel();
        header->BackColor = Color::FromArgb(12, 35, 22);
        header->Location  = Point(0, 0);
        header->Size      = System::Drawing::Size(700, 52);

        Label^ lblTitle = gcnew Label();
        lblTitle->AutoSize  = false;
        lblTitle->Text      = L"SCHOOLING  \x2022  SOCIAL NETWORK";
        lblTitle->Font      = gcnew Drawing::Font("Segoe UI", 11, FontStyle::Bold);
        lblTitle->ForeColor = Color::FromArgb(120, 255, 170);
        lblTitle->Location  = Point(10, 14);
        lblTitle->Size      = System::Drawing::Size(290, 26);
        header->Controls->Add(lblTitle);

        // Legend
        Label^ lblLegend = gcnew Label();
        lblLegend->AutoSize  = false;
        lblLegend->Text      = L"Dashed ellipse = school group  |  Blinking ring = isolated  |  Z/!/!! = activity";
        lblLegend->Font      = gcnew Drawing::Font("Segoe UI", 7.5f);
        lblLegend->ForeColor = Color::FromArgb(100, 140, 110);
        lblLegend->Location  = Point(308, 18);
        lblLegend->Size      = System::Drawing::Size(380, 18);
        header->Controls->Add(lblLegend);

        _lblStats = gcnew Label();
        _lblStats->AutoSize  = false;
        _lblStats->Text      = L"Groups: 0  |  Schooling: 0 fish  |  Solo: 0";
        _lblStats->Font      = gcnew Drawing::Font("Consolas", 8);
        _lblStats->ForeColor = Color::FromArgb(130, 200, 150);
        _lblStats->Location  = Point(10, 34);
        _lblStats->Size      = System::Drawing::Size(320, 16);
        header->Controls->Add(_lblStats);

        // Close button
        Button^ btnClose = gcnew Button();
        btnClose->Text      = L"Close";
        btnClose->Font      = gcnew Drawing::Font("Segoe UI", 8.5f);
        btnClose->Location  = Point(610, 12);
        btnClose->Size      = System::Drawing::Size(70, 28);
        btnClose->BackColor = Color::FromArgb(20, 55, 35);
        btnClose->ForeColor = Color::White;
        btnClose->FlatStyle = FlatStyle::Flat;
        btnClose->FlatAppearance->BorderSize = 0;
        btnClose->Click    += gcnew EventHandler(this, &SchoolingForm::BtnClose_Click);
        header->Controls->Add(btnClose);

        this->Controls->Add(header);

        // ─── CANVAS ───────────────────────────────────────────────────────
        _canvas = gcnew SchoolingPanel();
        _canvas->Location  = Point(10, 56);
        _canvas->Size      = System::Drawing::Size(680, 502);
        _canvas->BackColor = Color::FromArgb(6, 10, 18);
        this->Controls->Add(_canvas);

        // ─── CLOSE BUTTON ─────────────────────────────────────────────────
        Button^ btnCloseMain = gcnew Button();
        btnCloseMain->Text      = L"Close";
        btnCloseMain->Font      = gcnew Drawing::Font("Segoe UI", 9);
        btnCloseMain->Location  = Point(300, 560);
        btnCloseMain->Size      = System::Drawing::Size(100, 28);
        btnCloseMain->BackColor = Color::FromArgb(20, 55, 35);
        btnCloseMain->ForeColor = Color::White;
        btnCloseMain->FlatStyle = FlatStyle::Flat;
        btnCloseMain->FlatAppearance->BorderSize = 0;
        btnCloseMain->Click    += gcnew EventHandler(this, &SchoolingForm::BtnClose_Click);
        this->Controls->Add(btnCloseMain);
    }

    void SchoolingForm::UpdateFishes(List<FishTrack^>^ fishes, int videoW, int videoH) {
        _canvas->UpdateFishes(fishes, videoW, videoH);
        // Update stats from computed values in canvas
        int active = 0;
        for each (FishTrack^ f in fishes)
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
