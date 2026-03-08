#pragma once
#include "DetectionService.h"
#include "FishDetailForm.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Collections::Generic;
using namespace System::ComponentModel;

namespace KoiTracker {

    public ref class MainForm : public Form {

    public:
        MainForm() {
            _service = gcnew DetectionService();
            _currentFishes = gcnew List<FishTrack^>();
            _detailForm = nullptr;
            _frameBitmap = nullptr;
            InitializeComponent();
            StyleGrid();
            WireEvents();
        }

    protected:
        ~MainForm() { if (components) delete components; }

        // =====================================================
        //  CONTROL DECLARATIONS  (Designer อ่านส่วนนี้)
        // =====================================================
    private:
        DetectionService^ _service;
        List<FishTrack^>^ _currentFishes;
        FishDetailForm^ _detailForm;
        Bitmap^ _frameBitmap;

        System::Windows::Forms::Panel^ _toolbar;
        System::Windows::Forms::Button^ _btnStart;
        System::Windows::Forms::Button^ _btnPause;
        System::Windows::Forms::Button^ _btnStop;
        System::Windows::Forms::Label^ _lblTitle;
        System::Windows::Forms::Label^ _lblFps;
        System::Windows::Forms::Label^ _lblFishCount;

        System::Windows::Forms::Panel^ _videoPanel;
        System::Windows::Forms::PictureBox^ _pictureBox;

        System::Windows::Forms::Panel^ _rightPanel;
        System::Windows::Forms::Panel^ _statsPanel;
        System::Windows::Forms::DataGridView^ _grid;
        System::Windows::Forms::Label^ _lblStatActive;
        System::Windows::Forms::Label^ _lblStatLost;
        System::Windows::Forms::Label^ _lblStatTotal;
        System::Windows::Forms::Label^ _lblStatSpecies;
        System::Windows::Forms::Label^ _lblGridHeader;

        System::Windows::Forms::Panel^ _logPanel;
        System::Windows::Forms::Label^ _lblLogHeader;
        System::Windows::Forms::ListBox^ _listLog;

        System::ComponentModel::Container^ components;

        // =====================================================
        //  InitializeComponent — Designer อ่านฟังก์ชันนี้
        // =====================================================
    private:
        void InitializeComponent() {
            components = gcnew System::ComponentModel::Container();

            _toolbar = gcnew System::Windows::Forms::Panel();
            _btnStart = gcnew System::Windows::Forms::Button();
            _btnPause = gcnew System::Windows::Forms::Button();
            _btnStop = gcnew System::Windows::Forms::Button();
            _lblTitle = gcnew System::Windows::Forms::Label();
            _lblFps = gcnew System::Windows::Forms::Label();
            _lblFishCount = gcnew System::Windows::Forms::Label();
            _videoPanel = gcnew System::Windows::Forms::Panel();
            _pictureBox = gcnew System::Windows::Forms::PictureBox();
            _rightPanel = gcnew System::Windows::Forms::Panel();
            _statsPanel = gcnew System::Windows::Forms::Panel();
            _grid = gcnew System::Windows::Forms::DataGridView();
            _lblStatActive = gcnew System::Windows::Forms::Label();
            _lblStatLost = gcnew System::Windows::Forms::Label();
            _lblStatTotal = gcnew System::Windows::Forms::Label();
            _lblStatSpecies = gcnew System::Windows::Forms::Label();
            _lblGridHeader = gcnew System::Windows::Forms::Label();
            _logPanel = gcnew System::Windows::Forms::Panel();
            _lblLogHeader = gcnew System::Windows::Forms::Label();
            _listLog = gcnew System::Windows::Forms::ListBox();

            this->SuspendLayout();
            _toolbar->SuspendLayout();
            _videoPanel->SuspendLayout();
            _rightPanel->SuspendLayout();
            _statsPanel->SuspendLayout();
            _logPanel->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(_pictureBox))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(_grid))->BeginInit();

            // ---- Toolbar ----
            _toolbar->BackColor = System::Drawing::Color::FromArgb(35, 38, 52);
            _toolbar->Dock = System::Windows::Forms::DockStyle::Top;
            _toolbar->Height = 50;
            _toolbar->Name = L"_toolbar";
            _toolbar->Controls->Add(_lblTitle);
            _toolbar->Controls->Add(_btnStart);
            _toolbar->Controls->Add(_btnPause);
            _toolbar->Controls->Add(_btnStop);
            _toolbar->Controls->Add(_lblFps);
            _toolbar->Controls->Add(_lblFishCount);

            _lblTitle->AutoSize = true;
            _lblTitle->BackColor = System::Drawing::Color::Transparent;
            _lblTitle->Font = gcnew System::Drawing::Font(L"Segoe UI", 14, System::Drawing::FontStyle::Bold);
            _lblTitle->ForeColor = System::Drawing::Color::FromArgb(100, 200, 255);
            _lblTitle->Location = System::Drawing::Point(12, 13);
            _lblTitle->Name = L"_lblTitle";
            _lblTitle->Text = L"KOI TRACKER";

            _btnStart->BackColor = System::Drawing::Color::FromArgb(40, 160, 90);
            _btnStart->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            _btnStart->FlatAppearance->BorderSize = 0;
            _btnStart->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold);
            _btnStart->ForeColor = System::Drawing::Color::White;
            _btnStart->Location = System::Drawing::Point(200, 10);
            _btnStart->Name = L"_btnStart";
            _btnStart->Size = System::Drawing::Size(88, 30);
            _btnStart->Text = L"Start";
            _btnStart->Cursor = System::Windows::Forms::Cursors::Hand;

            _btnPause->BackColor = System::Drawing::Color::FromArgb(200, 140, 30);
            _btnPause->Enabled = false;
            _btnPause->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            _btnPause->FlatAppearance->BorderSize = 0;
            _btnPause->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold);
            _btnPause->ForeColor = System::Drawing::Color::White;
            _btnPause->Location = System::Drawing::Point(295, 10);
            _btnPause->Name = L"_btnPause";
            _btnPause->Size = System::Drawing::Size(88, 30);
            _btnPause->Text = L"Pause";
            _btnPause->Cursor = System::Windows::Forms::Cursors::Hand;

            _btnStop->BackColor = System::Drawing::Color::FromArgb(190, 50, 50);
            _btnStop->Enabled = false;
            _btnStop->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            _btnStop->FlatAppearance->BorderSize = 0;
            _btnStop->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold);
            _btnStop->ForeColor = System::Drawing::Color::White;
            _btnStop->Location = System::Drawing::Point(390, 10);
            _btnStop->Name = L"_btnStop";
            _btnStop->Size = System::Drawing::Size(88, 30);
            _btnStop->Text = L"Stop";
            _btnStop->Cursor = System::Windows::Forms::Cursors::Hand;

            _lblFps->AutoSize = true;
            _lblFps->BackColor = System::Drawing::Color::Transparent;
            _lblFps->Font = gcnew System::Drawing::Font(L"Segoe UI", 9);
            _lblFps->ForeColor = System::Drawing::Color::FromArgb(150, 200, 150);
            _lblFps->Location = System::Drawing::Point(510, 17);
            _lblFps->Name = L"_lblFps";
            _lblFps->Text = L"FPS: --";

            _lblFishCount->AutoSize = true;
            _lblFishCount->BackColor = System::Drawing::Color::Transparent;
            _lblFishCount->Font = gcnew System::Drawing::Font(L"Segoe UI", 9);
            _lblFishCount->ForeColor = System::Drawing::Color::FromArgb(200, 200, 120);
            _lblFishCount->Location = System::Drawing::Point(590, 17);
            _lblFishCount->Name = L"_lblFishCount";
            _lblFishCount->Text = L"Fish: 0";

            // ---- Video Panel ----
            _videoPanel->BackColor = System::Drawing::Color::FromArgb(15, 20, 30);
            _videoPanel->Location = System::Drawing::Point(0, 50);
            _videoPanel->Name = L"_videoPanel";
            _videoPanel->Size = System::Drawing::Size(660, 520);
            _videoPanel->Controls->Add(_pictureBox);

            _pictureBox->BackColor = System::Drawing::Color::FromArgb(10, 40, 60);
            _pictureBox->Cursor = System::Windows::Forms::Cursors::Cross;
            _pictureBox->Location = System::Drawing::Point(10, 10);
            _pictureBox->Name = L"_pictureBox";
            _pictureBox->Size = System::Drawing::Size(640, 480);
            _pictureBox->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;

            // ---- Right Panel ----
            _rightPanel->BackColor = System::Drawing::Color::FromArgb(30, 33, 45);
            _rightPanel->Location = System::Drawing::Point(665, 50);
            _rightPanel->Name = L"_rightPanel";
            _rightPanel->Size = System::Drawing::Size(420, 560);
            _rightPanel->Controls->Add(_statsPanel);
            _rightPanel->Controls->Add(_lblGridHeader);
            _rightPanel->Controls->Add(_grid);
            _rightPanel->Controls->Add(_lblStatSpecies);

            _statsPanel->BackColor = System::Drawing::Color::FromArgb(35, 38, 52);
            _statsPanel->Location = System::Drawing::Point(5, 5);
            _statsPanel->Name = L"_statsPanel";
            _statsPanel->Size = System::Drawing::Size(410, 60);
            _statsPanel->Controls->Add(_lblStatActive);
            _statsPanel->Controls->Add(_lblStatLost);
            _statsPanel->Controls->Add(_lblStatTotal);

            _lblStatActive->AutoSize = true;
            _lblStatActive->Font = gcnew System::Drawing::Font(L"Segoe UI", 18, System::Drawing::FontStyle::Bold);
            _lblStatActive->ForeColor = System::Drawing::Color::FromArgb(80, 220, 120);
            _lblStatActive->Location = System::Drawing::Point(10, 18);
            _lblStatActive->Name = L"_lblStatActive";
            _lblStatActive->Text = L"0";

            _lblStatLost->AutoSize = true;
            _lblStatLost->Font = gcnew System::Drawing::Font(L"Segoe UI", 18, System::Drawing::FontStyle::Bold);
            _lblStatLost->ForeColor = System::Drawing::Color::FromArgb(220, 100, 80);
            _lblStatLost->Location = System::Drawing::Point(110, 18);
            _lblStatLost->Name = L"_lblStatLost";
            _lblStatLost->Text = L"0";

            _lblStatTotal->AutoSize = true;
            _lblStatTotal->Font = gcnew System::Drawing::Font(L"Segoe UI", 18, System::Drawing::FontStyle::Bold);
            _lblStatTotal->ForeColor = System::Drawing::Color::FromArgb(100, 180, 255);
            _lblStatTotal->Location = System::Drawing::Point(210, 18);
            _lblStatTotal->Name = L"_lblStatTotal";
            _lblStatTotal->Text = L"0";

            _lblGridHeader->AutoSize = true;
            _lblGridHeader->Font = gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold);
            _lblGridHeader->ForeColor = System::Drawing::Color::FromArgb(180, 180, 200);
            _lblGridHeader->Location = System::Drawing::Point(8, 72);
            _lblGridHeader->Name = L"_lblGridHeader";
            _lblGridHeader->Text = L"Active Fish Tracks";

            _grid->AllowUserToAddRows = false;
            _grid->AllowUserToDeleteRows = false;
            _grid->BackgroundColor = System::Drawing::Color::FromArgb(25, 28, 38);
            _grid->BorderStyle = System::Windows::Forms::BorderStyle::None;
            _grid->ColumnHeadersHeight = 28;
            _grid->EnableHeadersVisualStyles = false;
            _grid->Font = gcnew System::Drawing::Font(L"Segoe UI", 8.5f);
            _grid->GridColor = System::Drawing::Color::FromArgb(50, 55, 70);
            _grid->Location = System::Drawing::Point(5, 95);
            _grid->Name = L"_grid";
            _grid->ReadOnly = true;
            _grid->RowHeadersVisible = false;
            _grid->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
            _grid->Size = System::Drawing::Size(410, 350);

            _lblStatSpecies->Font = gcnew System::Drawing::Font(L"Segoe UI", 8.5f);
            _lblStatSpecies->ForeColor = System::Drawing::Color::FromArgb(150, 150, 170);
            _lblStatSpecies->Location = System::Drawing::Point(8, 450);
            _lblStatSpecies->Name = L"_lblStatSpecies";
            _lblStatSpecies->Size = System::Drawing::Size(410, 80);
            _lblStatSpecies->Text = L"";

            // ---- Log Panel ----
            _logPanel->BackColor = System::Drawing::Color::FromArgb(20, 22, 30);
            _logPanel->Location = System::Drawing::Point(0, 578);
            _logPanel->Name = L"_logPanel";
            _logPanel->Size = System::Drawing::Size(1085, 100);
            _logPanel->Controls->Add(_lblLogHeader);
            _logPanel->Controls->Add(_listLog);

            _lblLogHeader->AutoSize = true;
            _lblLogHeader->Font = gcnew System::Drawing::Font(L"Segoe UI", 8.5f, System::Drawing::FontStyle::Bold);
            _lblLogHeader->ForeColor = System::Drawing::Color::FromArgb(150, 150, 170);
            _lblLogHeader->Location = System::Drawing::Point(8, 4);
            _lblLogHeader->Name = L"_lblLogHeader";
            _lblLogHeader->Text = L"Event Log";

            _listLog->BackColor = System::Drawing::Color::FromArgb(18, 20, 28);
            _listLog->BorderStyle = System::Windows::Forms::BorderStyle::None;
            _listLog->Font = gcnew System::Drawing::Font(L"Consolas", 8);
            _listLog->ForeColor = System::Drawing::Color::FromArgb(160, 200, 160);
            _listLog->Location = System::Drawing::Point(5, 22);
            _listLog->Name = L"_listLog";
            _listLog->Size = System::Drawing::Size(1075, 72);

            // ---- Main Form ----
            this->BackColor = System::Drawing::Color::FromArgb(25, 28, 38);
            this->ClientSize = System::Drawing::Size(1100, 690);
            this->Font = gcnew System::Drawing::Font(L"Segoe UI", 9);
            this->ForeColor = System::Drawing::Color::White;
            this->MinimumSize = System::Drawing::Size(900, 620);
            this->Name = L"MainForm";
            this->Text = L"Koi Tracker  -  Dashboard";
            this->Controls->Add(_toolbar);
            this->Controls->Add(_videoPanel);
            this->Controls->Add(_rightPanel);
            this->Controls->Add(_logPanel);

            _toolbar->ResumeLayout(false);
            _toolbar->PerformLayout();
            _videoPanel->ResumeLayout(false);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(_pictureBox))->EndInit();
            _statsPanel->ResumeLayout(false);
            _statsPanel->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(_grid))->EndInit();
            _rightPanel->ResumeLayout(false);
            _rightPanel->PerformLayout();
            _logPanel->ResumeLayout(false);
            _logPanel->PerformLayout();
            this->ResumeLayout(false);
            this->PerformLayout();
        }

        // =====================================================
        //  METHOD DECLARATIONS  (logic อยู่ใน MainForm.cpp)
        // =====================================================
    private:
        void WireEvents();
        void OnFrameUpdated(List<FishTrack^>^ fishes);
        void OnFishDetected(FishTrack^ fish);
        void OnFishLost(FishTrack^ fish);
        void OnFpsUpdated(int fps);
        void OnPictureBoxPaint(Object^ sender, PaintEventArgs^ e);
        void OnPictureBoxClick(Object^ sender, EventArgs^ e);
        void OnGridDoubleClick(Object^ sender, DataGridViewCellEventArgs^ e);
        void OnFormResize(Object^ sender, EventArgs^ e);
        void BtnStart_Click(Object^ sender, EventArgs^ e);
        void BtnPause_Click(Object^ sender, EventArgs^ e);
        void BtnStop_Click(Object^ sender, EventArgs^ e);
        void StartTracking();
        void StopTracking();
        void UpdateGrid(List<FishTrack^>^ fishes);
        void UpdateStats(List<FishTrack^>^ fishes);
        void DrawPondBackground(Graphics^ g);
        void StyleGrid();
        void Log(String^ msg);
        void SafeLog(String^ msg);
        void AddStatItem(String^ label, Label^% valRef, Color color, int x);

        void OnFrameBitmap(Bitmap^ bmp) {
            if (this->InvokeRequired) {
                this->Invoke(gcnew KoiTracker::FrameBitmapHandler(
                    this, &MainForm::OnFrameBitmap), bmp);
                return;
            }
            if (_pictureBox != nullptr)
                _pictureBox->Image = bmp;
        }
    };
}