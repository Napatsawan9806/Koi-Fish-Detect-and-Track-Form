#include "FishDetailForm.h"

namespace KoiTracker {

    FishDetailForm::FishDetailForm(FishTrack^ fish) {
        _fish = fish;
        InitializeComponent();
    }

    void FishDetailForm::InitializeComponent() {
        this->Text = String::Format("Fish Detail  �  ID #{0}", _fish->FishID);
        this->Size = System::Drawing::Size(380, 530);
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
        this->MaximizeBox = false;
        this->MinimizeBox = false;
        this->StartPosition = FormStartPosition::CenterParent;
        this->BackColor = Color::FromArgb(30, 30, 40);
        this->ForeColor = Color::White;

        // Color band
        _colorBand = gcnew Panel();
        _colorBand->Dock = DockStyle::Top;
        _colorBand->Height = 6;
        _colorBand->BackColor = _fish->TrackColor;

        // Header
        _header = gcnew Panel();
        _header->Dock = DockStyle::Top;
        _header->Height = 64;
        _header->BackColor = Color::FromArgb(40, 40, 55);

        _lblTitle = gcnew Label();
        _lblTitle->Text = String::Format("Fish ID #{0}", _fish->FishID);
        _lblTitle->Font = gcnew Drawing::Font("Segoe UI", 15, FontStyle::Bold);
        _lblTitle->ForeColor = Color::White;
        _lblTitle->Location = Point(14, 6);
        _lblTitle->AutoSize = true;

        _lblStatus = gcnew Label();
        _lblStatus->Text = _fish->StatusIcon + "  " + _fish->Status.ToString();
        _lblStatus->Font = gcnew Drawing::Font("Segoe UI", 9);
        _lblStatus->ForeColor = Color::FromArgb(180, 180, 200);
        _lblStatus->Location = Point(16, 38);
        _lblStatus->AutoSize = true;

        // Fish preview (crop จาก frame ปัจจุบัน) — มุมขวาของ header
        _fishPreview = gcnew PictureBox();
        _fishPreview->Location = Point(264, 6);
        _fishPreview->Size = System::Drawing::Size(106, 52);
        _fishPreview->SizeMode = PictureBoxSizeMode::Zoom;
        _fishPreview->BackColor = Color::FromArgb(20, 20, 30);
        _fishPreview->BorderStyle = BorderStyle::FixedSingle;
        (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(_fishPreview))->BeginInit();
        (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(_fishPreview))->EndInit();

        _header->Controls->Add(_lblTitle);
        _header->Controls->Add(_lblStatus);
        _header->Controls->Add(_fishPreview);

        // Info rows
        int row = 75, rh = 30;
        _lblSpeciesVal = MakeValLabel(_fish->Species.ToString(), 150, row); row += rh;
        _lblConfVal = MakeValLabel(_fish->ConfidenceText, 150, row); row += rh;
        _lblSpeedVal = MakeValLabel(String::Format("{0:F1}", _fish->Speed), 150, row); row += rh;
        _lblFramesVal = MakeValLabel(_fish->FrameCount.ToString(), 150, row); row += rh;
        _lblFirstVal = MakeValLabel(_fish->FirstSeen.ToString("HH:mm:ss"), 150, row); row += rh;
        _lblLastVal = MakeValLabel(_fish->LastSeen.ToString("HH:mm:ss"), 150, row); row += rh;
        _lblDurVal = MakeValLabel(_fish->DurationText, 150, row);

        row = 75;
        this->Controls->Add(MakeKeyLabel("Species:", 16, row)); row += rh;
        this->Controls->Add(MakeKeyLabel("Confidence:", 16, row)); row += rh;
        this->Controls->Add(MakeKeyLabel("Speed (px/f):", 16, row)); row += rh;
        this->Controls->Add(MakeKeyLabel("Frames seen:", 16, row)); row += rh;
        this->Controls->Add(MakeKeyLabel("First seen:", 16, row)); row += rh;
        this->Controls->Add(MakeKeyLabel("Last seen:", 16, row)); row += rh;
        this->Controls->Add(MakeKeyLabel("Duration:", 16, row));

        this->Controls->Add(_lblSpeciesVal);
        this->Controls->Add(_lblConfVal);
        this->Controls->Add(_lblSpeedVal);
        this->Controls->Add(_lblFramesVal);
        this->Controls->Add(_lblFirstVal);
        this->Controls->Add(_lblLastVal);
        this->Controls->Add(_lblDurVal);

        // Trajectory label
        Label^ tLbl = gcnew Label();
        tLbl->Text = "Trajectory (last 120 frames)";
        tLbl->Font = gcnew Drawing::Font("Segoe UI", 8.5f);
        tLbl->ForeColor = Color::FromArgb(140, 140, 160);
        tLbl->Location = Point(16, 295);
        tLbl->AutoSize = true;
        this->Controls->Add(tLbl);

        // Trajectory panel (larger to fill extra height)
        _trajPanel = gcnew TrajectoryPanel(_fish);
        _trajPanel->Location = Point(16, 315);
        _trajPanel->Size = System::Drawing::Size(344, 140);
        _trajPanel->BackColor = Color::FromArgb(20, 25, 35);
        this->Controls->Add(_trajPanel);

        // Close button
        _btnClose = gcnew Button();
        _btnClose->Text = "Close";
        _btnClose->Font = gcnew Drawing::Font("Segoe UI", 9);
        _btnClose->Location = Point(140, 470);
        _btnClose->Size = System::Drawing::Size(100, 32);
        _btnClose->BackColor = Color::FromArgb(60, 60, 80);
        _btnClose->ForeColor = Color::White;
        _btnClose->FlatStyle = FlatStyle::Flat;
        _btnClose->FlatAppearance->BorderColor = Color::FromArgb(80, 80, 100);
        _btnClose->Click += gcnew EventHandler(this, &FishDetailForm::BtnClose_Click);
        this->Controls->Add(_btnClose);

        // ���� header ��� color band
        this->Controls->Add(_header);
        this->Controls->Add(_colorBand);
    }

    void FishDetailForm::UpdateData(FishTrack^ fish) {
        _fish = fish;
        _lblSpeciesVal->Text = fish->Species.ToString();
        _lblConfVal->Text = fish->ConfidenceText;
        _lblSpeedVal->Text = String::Format("{0:F1}", fish->Speed);
        _lblFramesVal->Text = fish->FrameCount.ToString();
        _lblLastVal->Text = fish->LastSeen.ToString("HH:mm:ss");
        _lblDurVal->Text = fish->DurationText;
        _lblStatus->Text = fish->StatusIcon + "  " + fish->Status.ToString();
        _trajPanel->Fish = fish;
        _trajPanel->Invalidate();
    }

    void FishDetailForm::UpdateCrop(Bitmap^ bmp) {
        Bitmap^ old = safe_cast<Bitmap^>(_fishPreview->Image);
        _fishPreview->Image = bmp;
        if (old != nullptr) delete old;
    }

    Label^ FishDetailForm::MakeKeyLabel(String^ text, int x, int y) {
        Label^ lbl = gcnew Label();
        lbl->Text = text;
        lbl->Font = gcnew Drawing::Font("Segoe UI", 9);
        lbl->ForeColor = Color::FromArgb(140, 140, 160);
        lbl->Location = Point(x, y);
        lbl->Size = System::Drawing::Size(130, 24);
        lbl->TextAlign = ContentAlignment::MiddleLeft;
        return lbl;
    }

    Label^ FishDetailForm::MakeValLabel(String^ text, int x, int y) {
        Label^ lbl = gcnew Label();
        lbl->Text = text;
        lbl->Font = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
        lbl->ForeColor = Color::White;
        lbl->Location = Point(x, y);
        lbl->Size = System::Drawing::Size(190, 24);
        lbl->TextAlign = ContentAlignment::MiddleLeft;
        return lbl;
    }
}