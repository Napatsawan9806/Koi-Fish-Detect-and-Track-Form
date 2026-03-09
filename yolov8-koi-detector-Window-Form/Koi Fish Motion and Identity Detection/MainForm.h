#pragma once
#include "DetectionService.h"
#include "FishDetailForm.h"
#include "BehaviorAnalysisForm.h"
#include "HeatmapForm.h"
#include "SchoolingForm.h"
#include "ROISelectForm.h"
#include "AnalysisDashboardForm.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Collections::Generic;
using namespace System::ComponentModel;

namespace KoiTracker {

    // View modes — mutually exclusive, only one active at a time
    enum class ViewMode { Live, Heatmap, Schooling, Behavior };

    public ref class MainForm : public Form {

    public:
        MainForm() {
            _service = gcnew DetectionService();
            _currentFishes = gcnew List<FishTrack^>();
            _detailForm = nullptr;
            _behaviorForm = nullptr;
            _heatmapForm = nullptr;
            _schoolingForm = nullptr;
            _dashboardForm = nullptr;
            _frameBitmap = nullptr;
            _viewMode = ViewMode::Live;
            _alertFrameCount = 0;
            _heatmapPeak = 0;
            _heatmapFrames = 0;
            InitializeComponent();
            StyleGrid();
            WireEvents();
        }

    protected:
        ~MainForm() { if (components) delete components; }

    private:
        // ---- Service / State ----
        DetectionService^ _service;
        List<FishTrack^>^ _currentFishes;
        FishDetailForm^ _detailForm;
        BehaviorAnalysisForm^ _behaviorForm;
        HeatmapForm^ _heatmapForm;
        SchoolingForm^ _schoolingForm;
        AnalysisDashboardForm^ _dashboardForm;
        Bitmap^ _frameBitmap;
        ViewMode               _viewMode;
        int                    _alertFrameCount;
        int                    _heatmapPeak;
        int                    _heatmapFrames;
        DateTime               _lastBehaviorUpdate;
        DateTime               _lastSchoolingUpdate;
        DateTime               _lastDashboardUpdate;

        // ---- Toolbar ----
        System::Windows::Forms::Panel^ _toolbar;
        System::Windows::Forms::Button^ _btnStart;
        System::Windows::Forms::Button^ _btnPause;
        System::Windows::Forms::Button^ _btnStop;
        System::Windows::Forms::Label^ _lblSep;       // visual separator
        System::Windows::Forms::Button^ _btnHeatmap;
        System::Windows::Forms::Button^ _btnSchooling;
        System::Windows::Forms::Button^ _btnBehavior;
        System::Windows::Forms::Button^ _btnDashboard;
        System::Windows::Forms::Label^ _lblTitle;
        System::Windows::Forms::Label^ _lblFps;
        System::Windows::Forms::Label^ _lblFishCount;

        // ---- Video ----
        System::Windows::Forms::Panel^ _videoPanel;
        System::Windows::Forms::PictureBox^ _pictureBox;

        // ---- Right Panel ----
        System::Windows::Forms::Panel^ _rightPanel;
        System::Windows::Forms::Panel^ _statsPanel;
        System::Windows::Forms::Label^ _lblStatActive;
        System::Windows::Forms::Label^ _lblStatActiveLbl;
        System::Windows::Forms::Label^ _lblStatLost;
        System::Windows::Forms::Label^ _lblStatLostLbl;
        System::Windows::Forms::Label^ _lblStatTotal;
        System::Windows::Forms::Label^ _lblStatTotalLbl;
        System::Windows::Forms::Label^ _lblGridHeader;
        System::Windows::Forms::DataGridView^ _grid;

        // ---- Info Panels (swapped by ViewMode) ----
        System::Windows::Forms::Panel^ _panelInfoLive;
        System::Windows::Forms::Label^ _lblInfoLiveTitle;
        System::Windows::Forms::Label^ _lblInfoLiveContent;

        System::Windows::Forms::Panel^ _panelInfoHeatmap;
        System::Windows::Forms::Label^ _lblInfoHeatmapTitle;
        System::Windows::Forms::Label^ _lblInfoHeatmapContent;

        System::Windows::Forms::Panel^ _panelInfoSchooling;
        System::Windows::Forms::Label^ _lblInfoSchoolingTitle;
        System::Windows::Forms::Label^ _lblInfoSchoolingContent;

        System::Windows::Forms::Panel^ _panelInfoBehavior;
        System::Windows::Forms::Label^ _lblInfoBehaviorTitle;
        System::Windows::Forms::Label^ _lblInfoBehaviorContent;

        // ---- Log ----
        System::Windows::Forms::Panel^ _logPanel;
        System::Windows::Forms::Label^ _lblLogHeader;
        System::Windows::Forms::ListBox^ _listLog;

        System::ComponentModel::Container^ components;

        // =====================================================
        //  InitializeComponent
        // =====================================================
    private:
        void InitializeComponent() {
            components = gcnew System::ComponentModel::Container();

            // -- allocate all controls --
            _toolbar = gcnew System::Windows::Forms::Panel();
            _btnStart = gcnew System::Windows::Forms::Button();
            _btnPause = gcnew System::Windows::Forms::Button();
            _btnStop = gcnew System::Windows::Forms::Button();
            _lblSep = gcnew System::Windows::Forms::Label();
            _btnHeatmap = gcnew System::Windows::Forms::Button();
            _btnSchooling = gcnew System::Windows::Forms::Button();
            _btnBehavior = gcnew System::Windows::Forms::Button();
            _btnDashboard = gcnew System::Windows::Forms::Button();
            _lblTitle = gcnew System::Windows::Forms::Label();
            _lblFps = gcnew System::Windows::Forms::Label();
            _lblFishCount = gcnew System::Windows::Forms::Label();
            _videoPanel = gcnew System::Windows::Forms::Panel();
            _pictureBox = gcnew System::Windows::Forms::PictureBox();
            _rightPanel = gcnew System::Windows::Forms::Panel();
            _statsPanel = gcnew System::Windows::Forms::Panel();
            _lblStatActive = gcnew System::Windows::Forms::Label();
            _lblStatActiveLbl = gcnew System::Windows::Forms::Label();
            _lblStatLost = gcnew System::Windows::Forms::Label();
            _lblStatLostLbl = gcnew System::Windows::Forms::Label();
            _lblStatTotal = gcnew System::Windows::Forms::Label();
            _lblStatTotalLbl = gcnew System::Windows::Forms::Label();
            _lblGridHeader = gcnew System::Windows::Forms::Label();
            _grid = gcnew System::Windows::Forms::DataGridView();

            _panelInfoLive = gcnew System::Windows::Forms::Panel();
            _lblInfoLiveTitle = gcnew System::Windows::Forms::Label();
            _lblInfoLiveContent = gcnew System::Windows::Forms::Label();
            _panelInfoHeatmap = gcnew System::Windows::Forms::Panel();
            _lblInfoHeatmapTitle = gcnew System::Windows::Forms::Label();
            _lblInfoHeatmapContent = gcnew System::Windows::Forms::Label();
            _panelInfoSchooling = gcnew System::Windows::Forms::Panel();
            _lblInfoSchoolingTitle = gcnew System::Windows::Forms::Label();
            _lblInfoSchoolingContent = gcnew System::Windows::Forms::Label();
            _panelInfoBehavior = gcnew System::Windows::Forms::Panel();
            _lblInfoBehaviorTitle = gcnew System::Windows::Forms::Label();
            _lblInfoBehaviorContent = gcnew System::Windows::Forms::Label();

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

            // ===================== TOOLBAR =====================
            _toolbar->BackColor = System::Drawing::Color::FromArgb(30, 33, 46);
            _toolbar->Dock = System::Windows::Forms::DockStyle::Top;
            _toolbar->Height = 50;
            _toolbar->Name = L"_toolbar";
            _toolbar->Controls->Add(_lblTitle);
            _toolbar->Controls->Add(_btnStart);
            _toolbar->Controls->Add(_btnPause);
            _toolbar->Controls->Add(_btnStop);
            _toolbar->Controls->Add(_lblSep);
            _toolbar->Controls->Add(_btnHeatmap);
            _toolbar->Controls->Add(_btnSchooling);
            _toolbar->Controls->Add(_btnBehavior);
            _toolbar->Controls->Add(_btnDashboard);
            _toolbar->Controls->Add(_lblFps);
            _toolbar->Controls->Add(_lblFishCount);

            _lblTitle->AutoSize = true;
            _lblTitle->BackColor = System::Drawing::Color::Transparent;
            _lblTitle->Font = gcnew System::Drawing::Font(L"Segoe UI", 14, System::Drawing::FontStyle::Bold);
            _lblTitle->ForeColor = System::Drawing::Color::FromArgb(100, 200, 255);
            _lblTitle->Location = System::Drawing::Point(12, 13);
            _lblTitle->Text = L"KOI TRACKER";

            // ---- Control buttons ----
            _btnStart->BackColor = System::Drawing::Color::FromArgb(40, 160, 90);
            _btnStart->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            _btnStart->FlatAppearance->BorderSize = 0;
            _btnStart->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold);
            _btnStart->ForeColor = System::Drawing::Color::White;
            _btnStart->Location = System::Drawing::Point(200, 10);
            _btnStart->Size = System::Drawing::Size(80, 30);
            _btnStart->Text = L"Start";
            _btnStart->Cursor = System::Windows::Forms::Cursors::Hand;

            _btnPause->BackColor = System::Drawing::Color::FromArgb(190, 130, 30);
            _btnPause->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            _btnPause->FlatAppearance->BorderSize = 0;
            _btnPause->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold);
            _btnPause->ForeColor = System::Drawing::Color::White;
            _btnPause->Location = System::Drawing::Point(287, 10);
            _btnPause->Size = System::Drawing::Size(80, 30);
            _btnPause->Text = L"Pause";
            _btnPause->Cursor = System::Windows::Forms::Cursors::Hand;
            _btnPause->Enabled = false;

            _btnStop->BackColor = System::Drawing::Color::FromArgb(180, 50, 50);
            _btnStop->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            _btnStop->FlatAppearance->BorderSize = 0;
            _btnStop->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold);
            _btnStop->ForeColor = System::Drawing::Color::White;
            _btnStop->Location = System::Drawing::Point(374, 10);
            _btnStop->Size = System::Drawing::Size(80, 30);
            _btnStop->Text = L"Stop";
            _btnStop->Cursor = System::Windows::Forms::Cursors::Hand;
            _btnStop->Enabled = false;

            // ---- Separator ----
            _lblSep->BackColor = System::Drawing::Color::FromArgb(55, 60, 80);
            _lblSep->Location = System::Drawing::Point(467, 10);
            _lblSep->Size = System::Drawing::Size(2, 30);

            // ---- View toggle buttons ----
            _btnHeatmap->BackColor = System::Drawing::Color::FromArgb(40, 44, 62);
            _btnHeatmap->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            _btnHeatmap->FlatAppearance->BorderSize = 0;
            _btnHeatmap->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold);
            _btnHeatmap->ForeColor = System::Drawing::Color::FromArgb(160, 165, 185);
            _btnHeatmap->Location = System::Drawing::Point(478, 10);
            _btnHeatmap->Size = System::Drawing::Size(90, 30);
            _btnHeatmap->Text = L"Heatmap";
            _btnHeatmap->Cursor = System::Windows::Forms::Cursors::Hand;

            _btnSchooling->BackColor = System::Drawing::Color::FromArgb(40, 44, 62);
            _btnSchooling->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            _btnSchooling->FlatAppearance->BorderSize = 0;
            _btnSchooling->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold);
            _btnSchooling->ForeColor = System::Drawing::Color::FromArgb(160, 165, 185);
            _btnSchooling->Location = System::Drawing::Point(575, 10);
            _btnSchooling->Size = System::Drawing::Size(90, 30);
            _btnSchooling->Text = L"Schooling";
            _btnSchooling->Cursor = System::Windows::Forms::Cursors::Hand;

            _btnBehavior->BackColor = System::Drawing::Color::FromArgb(40, 44, 62);
            _btnBehavior->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            _btnBehavior->FlatAppearance->BorderSize = 0;
            _btnBehavior->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold);
            _btnBehavior->ForeColor = System::Drawing::Color::FromArgb(160, 165, 185);
            _btnBehavior->Location = System::Drawing::Point(672, 10);
            _btnBehavior->Size = System::Drawing::Size(90, 30);
            _btnBehavior->Text = L"Behavior";
            _btnBehavior->Cursor = System::Windows::Forms::Cursors::Hand;

            // ---- Dashboard button (separator line before it) ----
            _btnDashboard->BackColor = System::Drawing::Color::FromArgb(55, 35, 90);
            _btnDashboard->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            _btnDashboard->FlatAppearance->BorderSize = 1;
            _btnDashboard->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(120, 80, 180);
            _btnDashboard->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold);
            _btnDashboard->ForeColor = System::Drawing::Color::FromArgb(200, 160, 255);
            _btnDashboard->Location = System::Drawing::Point(772, 10);
            _btnDashboard->Size = System::Drawing::Size(110, 30);
            _btnDashboard->Text = L"\x2665  Dashboard";
            _btnDashboard->Cursor = System::Windows::Forms::Cursors::Hand;

            _lblFps->AutoSize = true;
            _lblFps->BackColor = System::Drawing::Color::Transparent;
            _lblFps->Font = gcnew System::Drawing::Font(L"Segoe UI", 9);
            _lblFps->ForeColor = System::Drawing::Color::FromArgb(120, 180, 120);
            _lblFps->Location = System::Drawing::Point(898, 17);
            _lblFps->Text = L"FPS: --";

            _lblFishCount->AutoSize = true;
            _lblFishCount->BackColor = System::Drawing::Color::Transparent;
            _lblFishCount->Font = gcnew System::Drawing::Font(L"Segoe UI", 9);
            _lblFishCount->ForeColor = System::Drawing::Color::FromArgb(200, 200, 120);
            _lblFishCount->Location = System::Drawing::Point(983, 17);
            _lblFishCount->Text = L"Fish: 0";

            // ===================== VIDEO =====================
            _videoPanel->BackColor = System::Drawing::Color::FromArgb(12, 16, 24);
            _videoPanel->Location = System::Drawing::Point(0, 50);
            _videoPanel->Size = System::Drawing::Size(660, 520);
            _videoPanel->Controls->Add(_pictureBox);

            _pictureBox->BackColor = System::Drawing::Color::FromArgb(10, 40, 60);
            _pictureBox->Cursor = System::Windows::Forms::Cursors::Cross;
            _pictureBox->Location = System::Drawing::Point(10, 10);
            _pictureBox->Size = System::Drawing::Size(640, 480);
            _pictureBox->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;

            // ===================== RIGHT PANEL =====================
            _rightPanel->BackColor = System::Drawing::Color::FromArgb(28, 31, 43);
            _rightPanel->Location = System::Drawing::Point(665, 50);
            _rightPanel->Size = System::Drawing::Size(420, 560);
            _rightPanel->Controls->Add(_statsPanel);
            _rightPanel->Controls->Add(_lblGridHeader);
            _rightPanel->Controls->Add(_grid);
            _rightPanel->Controls->Add(_panelInfoLive);
            _rightPanel->Controls->Add(_panelInfoHeatmap);
            _rightPanel->Controls->Add(_panelInfoSchooling);
            _rightPanel->Controls->Add(_panelInfoBehavior);

            // -- Stats bar (top of right panel) --
            _statsPanel->BackColor = System::Drawing::Color::FromArgb(33, 36, 50);
            _statsPanel->Location = System::Drawing::Point(5, 5);
            _statsPanel->Size = System::Drawing::Size(410, 64);
            _statsPanel->Controls->Add(_lblStatActive);
            _statsPanel->Controls->Add(_lblStatActiveLbl);
            _statsPanel->Controls->Add(_lblStatLost);
            _statsPanel->Controls->Add(_lblStatLostLbl);
            _statsPanel->Controls->Add(_lblStatTotal);
            _statsPanel->Controls->Add(_lblStatTotalLbl);

            _lblStatActive->AutoSize = true;
            _lblStatActive->Font = gcnew System::Drawing::Font(L"Segoe UI", 20, System::Drawing::FontStyle::Bold);
            _lblStatActive->ForeColor = System::Drawing::Color::FromArgb(80, 220, 120);
            _lblStatActive->Location = System::Drawing::Point(10, 8);
            _lblStatActive->Text = L"0";

            _lblStatLost->AutoSize = true;
            _lblStatLost->Font = gcnew System::Drawing::Font(L"Segoe UI", 20, System::Drawing::FontStyle::Bold);
            _lblStatLost->ForeColor = System::Drawing::Color::FromArgb(220, 100, 80);
            _lblStatLost->Location = System::Drawing::Point(130, 8);
            _lblStatLost->Text = L"0";

            _lblStatTotal->AutoSize = true;
            _lblStatTotal->Font = gcnew System::Drawing::Font(L"Segoe UI", 20, System::Drawing::FontStyle::Bold);
            _lblStatTotal->ForeColor = System::Drawing::Color::FromArgb(100, 180, 255);
            _lblStatTotal->Location = System::Drawing::Point(250, 8);
            _lblStatTotal->Text = L"0";

            _lblStatActiveLbl->AutoSize = true;
            _lblStatActiveLbl->Font = gcnew System::Drawing::Font(L"Segoe UI", 7.5f);
            _lblStatActiveLbl->ForeColor = System::Drawing::Color::FromArgb(100, 110, 130);
            _lblStatActiveLbl->Location = System::Drawing::Point(16, 46);
            _lblStatActiveLbl->Text = L"ACTIVE";

            _lblStatLostLbl->AutoSize = true;
            _lblStatLostLbl->Font = gcnew System::Drawing::Font(L"Segoe UI", 7.5f);
            _lblStatLostLbl->ForeColor = System::Drawing::Color::FromArgb(100, 110, 130);
            _lblStatLostLbl->Location = System::Drawing::Point(136, 46);
            _lblStatLostLbl->Text = L"LOST";

            _lblStatTotalLbl->AutoSize = true;
            _lblStatTotalLbl->Font = gcnew System::Drawing::Font(L"Segoe UI", 7.5f);
            _lblStatTotalLbl->ForeColor = System::Drawing::Color::FromArgb(100, 110, 130);
            _lblStatTotalLbl->Location = System::Drawing::Point(256, 46);
            _lblStatTotalLbl->Text = L"TOTAL";

            // -- Grid header --
            _lblGridHeader->AutoSize = true;
            _lblGridHeader->Font = gcnew System::Drawing::Font(L"Segoe UI", 8.5f, System::Drawing::FontStyle::Bold);
            _lblGridHeader->ForeColor = System::Drawing::Color::FromArgb(130, 140, 170);
            _lblGridHeader->Location = System::Drawing::Point(8, 77);
            _lblGridHeader->Text = L"FISH  TRACKS";

            // -- Grid (shorter to leave room for info panel) --
            _grid->AllowUserToAddRows = false;
            _grid->AllowUserToDeleteRows = false;
            _grid->BackgroundColor = System::Drawing::Color::FromArgb(22, 25, 35);
            _grid->BorderStyle = System::Windows::Forms::BorderStyle::None;
            _grid->ColumnHeadersHeight = 26;
            _grid->EnableHeadersVisualStyles = false;
            _grid->Font = gcnew System::Drawing::Font(L"Segoe UI", 8.5f);
            _grid->GridColor = System::Drawing::Color::FromArgb(42, 46, 62);
            _grid->Location = System::Drawing::Point(5, 96);
            _grid->ReadOnly = true;
            _grid->RowHeadersVisible = false;
            _grid->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
            _grid->Size = System::Drawing::Size(410, 265);   // 265 px (was 350)

            // ===================== INFO PANELS (all at Y=368, H=184) =====================
            // NOTE: NO DockStyle used — absolute positions only (avoids AutoSize+Dock StackOverflow)

            // -- Live panel --
            _panelInfoLive->BackColor = System::Drawing::Color::FromArgb(24, 27, 38);
            _panelInfoLive->Location = System::Drawing::Point(5, 368);
            _panelInfoLive->Size = System::Drawing::Size(410, 184);
            _lblInfoLiveTitle->AutoSize = false;
            _lblInfoLiveTitle->BackColor = System::Drawing::Color::FromArgb(45, 50, 70);
            _lblInfoLiveTitle->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold);
            _lblInfoLiveTitle->ForeColor = System::Drawing::Color::White;
            _lblInfoLiveTitle->Location = System::Drawing::Point(0, 0);
            _lblInfoLiveTitle->Size = System::Drawing::Size(410, 28);
            _lblInfoLiveTitle->Text = L"LIVE  \x2022  OVERVIEW";
            _lblInfoLiveTitle->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            _lblInfoLiveTitle->Padding = System::Windows::Forms::Padding(8, 0, 0, 0);
            _lblInfoLiveContent->AutoSize = false;
            _lblInfoLiveContent->Location = System::Drawing::Point(10, 34);
            _lblInfoLiveContent->Size = System::Drawing::Size(394, 146);
            _lblInfoLiveContent->Font = gcnew System::Drawing::Font(L"Consolas", 8.5f);
            _lblInfoLiveContent->ForeColor = System::Drawing::Color::FromArgb(190, 200, 220);
            _lblInfoLiveContent->BackColor = System::Drawing::Color::Transparent;
            _lblInfoLiveContent->Text = L"";
            _panelInfoLive->Controls->Add(_lblInfoLiveTitle);
            _panelInfoLive->Controls->Add(_lblInfoLiveContent);

            // -- Heatmap panel --
            _panelInfoHeatmap->BackColor = System::Drawing::Color::FromArgb(24, 27, 38);
            _panelInfoHeatmap->Location = System::Drawing::Point(5, 368);
            _panelInfoHeatmap->Size = System::Drawing::Size(410, 184);
            _lblInfoHeatmapTitle->AutoSize = false;
            _lblInfoHeatmapTitle->BackColor = System::Drawing::Color::FromArgb(50, 70, 145);
            _lblInfoHeatmapTitle->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold);
            _lblInfoHeatmapTitle->ForeColor = System::Drawing::Color::White;
            _lblInfoHeatmapTitle->Location = System::Drawing::Point(0, 0);
            _lblInfoHeatmapTitle->Size = System::Drawing::Size(410, 28);
            _lblInfoHeatmapTitle->Text = L"HEATMAP  \x2022  COVERAGE";
            _lblInfoHeatmapTitle->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            _lblInfoHeatmapTitle->Padding = System::Windows::Forms::Padding(8, 0, 0, 0);
            _lblInfoHeatmapContent->AutoSize = false;
            _lblInfoHeatmapContent->Location = System::Drawing::Point(10, 34);
            _lblInfoHeatmapContent->Size = System::Drawing::Size(394, 146);
            _lblInfoHeatmapContent->Font = gcnew System::Drawing::Font(L"Consolas", 8.5f);
            _lblInfoHeatmapContent->ForeColor = System::Drawing::Color::FromArgb(190, 200, 220);
            _lblInfoHeatmapContent->BackColor = System::Drawing::Color::Transparent;
            _lblInfoHeatmapContent->Text = L"";
            _panelInfoHeatmap->Controls->Add(_lblInfoHeatmapTitle);
            _panelInfoHeatmap->Controls->Add(_lblInfoHeatmapContent);

            // -- Schooling panel --
            _panelInfoSchooling->BackColor = System::Drawing::Color::FromArgb(24, 27, 38);
            _panelInfoSchooling->Location = System::Drawing::Point(5, 368);
            _panelInfoSchooling->Size = System::Drawing::Size(410, 184);
            _lblInfoSchoolingTitle->AutoSize = false;
            _lblInfoSchoolingTitle->BackColor = System::Drawing::Color::FromArgb(55, 105, 50);
            _lblInfoSchoolingTitle->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold);
            _lblInfoSchoolingTitle->ForeColor = System::Drawing::Color::White;
            _lblInfoSchoolingTitle->Location = System::Drawing::Point(0, 0);
            _lblInfoSchoolingTitle->Size = System::Drawing::Size(410, 28);
            _lblInfoSchoolingTitle->Text = L"SCHOOLING  \x2022  SOCIAL";
            _lblInfoSchoolingTitle->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            _lblInfoSchoolingTitle->Padding = System::Windows::Forms::Padding(8, 0, 0, 0);
            _lblInfoSchoolingContent->AutoSize = false;
            _lblInfoSchoolingContent->Location = System::Drawing::Point(10, 34);
            _lblInfoSchoolingContent->Size = System::Drawing::Size(394, 146);
            _lblInfoSchoolingContent->Font = gcnew System::Drawing::Font(L"Consolas", 8.5f);
            _lblInfoSchoolingContent->ForeColor = System::Drawing::Color::FromArgb(190, 200, 220);
            _lblInfoSchoolingContent->BackColor = System::Drawing::Color::Transparent;
            _lblInfoSchoolingContent->Text = L"";
            _panelInfoSchooling->Controls->Add(_lblInfoSchoolingTitle);
            _panelInfoSchooling->Controls->Add(_lblInfoSchoolingContent);

            // -- Behavior panel --
            _panelInfoBehavior->BackColor = System::Drawing::Color::FromArgb(24, 27, 38);
            _panelInfoBehavior->Location = System::Drawing::Point(5, 368);
            _panelInfoBehavior->Size = System::Drawing::Size(410, 184);
            _lblInfoBehaviorTitle->AutoSize = false;
            _lblInfoBehaviorTitle->BackColor = System::Drawing::Color::FromArgb(85, 50, 120);
            _lblInfoBehaviorTitle->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold);
            _lblInfoBehaviorTitle->ForeColor = System::Drawing::Color::White;
            _lblInfoBehaviorTitle->Location = System::Drawing::Point(0, 0);
            _lblInfoBehaviorTitle->Size = System::Drawing::Size(410, 28);
            _lblInfoBehaviorTitle->Padding = System::Windows::Forms::Padding(8, 0, 0, 0);
            _lblInfoBehaviorTitle->Text = L"BEHAVIOR  \x2022  ACTIVITY";
            _lblInfoBehaviorTitle->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            _lblInfoBehaviorContent->AutoSize = false;
            _lblInfoBehaviorContent->Location = System::Drawing::Point(10, 34);
            _lblInfoBehaviorContent->Size = System::Drawing::Size(394, 146);
            _lblInfoBehaviorContent->Font = gcnew System::Drawing::Font(L"Consolas", 8.5f);
            _lblInfoBehaviorContent->ForeColor = System::Drawing::Color::FromArgb(190, 200, 220);
            _lblInfoBehaviorContent->BackColor = System::Drawing::Color::Transparent;
            _lblInfoBehaviorContent->Text = L"";
            _panelInfoBehavior->Controls->Add(_lblInfoBehaviorTitle);
            _panelInfoBehavior->Controls->Add(_lblInfoBehaviorContent);

            // Only Live panel visible at start
            _panelInfoHeatmap->Visible = false;
            _panelInfoSchooling->Visible = false;
            _panelInfoBehavior->Visible = false;

            // ===================== LOG PANEL =====================
            _logPanel->BackColor = System::Drawing::Color::FromArgb(18, 20, 28);
            _logPanel->Location = System::Drawing::Point(0, 578);
            _logPanel->Size = System::Drawing::Size(1085, 100);
            _logPanel->Controls->Add(_lblLogHeader);
            _logPanel->Controls->Add(_listLog);

            _lblLogHeader->AutoSize = true;
            _lblLogHeader->Font = gcnew System::Drawing::Font(L"Segoe UI", 8.5f, System::Drawing::FontStyle::Bold);
            _lblLogHeader->ForeColor = System::Drawing::Color::FromArgb(100, 110, 130);
            _lblLogHeader->Location = System::Drawing::Point(8, 4);
            _lblLogHeader->Text = L"EVENT LOG";

            _listLog->BackColor = System::Drawing::Color::FromArgb(16, 18, 26);
            _listLog->BorderStyle = System::Windows::Forms::BorderStyle::None;
            _listLog->Font = gcnew System::Drawing::Font(L"Consolas", 8);
            _listLog->ForeColor = System::Drawing::Color::FromArgb(140, 190, 140);
            _listLog->Location = System::Drawing::Point(5, 22);
            _listLog->Size = System::Drawing::Size(1075, 72);

            // ===================== MAIN FORM =====================
            this->BackColor = System::Drawing::Color::FromArgb(22, 25, 35);
            this->ClientSize = System::Drawing::Size(1100, 690);
            this->Font = gcnew System::Drawing::Font(L"Segoe UI", 9);
            this->ForeColor = System::Drawing::Color::White;
            this->MinimumSize = System::Drawing::Size(900, 620);
            this->Text = L"Koi Tracker  —  Dashboard";
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
        //  METHOD DECLARATIONS
        // =====================================================
    private:
        void WireEvents();
        void SetViewMode(ViewMode mode);
        void OnServiceError(String^ message);
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
        void BtnHeatmap_Click(Object^ sender, EventArgs^ e);
        void BtnSchooling_Click(Object^ sender, EventArgs^ e);
        void BtnBehavior_Click(Object^ sender, EventArgs^ e);
        void BtnDashboard_Click(Object^ sender, EventArgs^ e);
        void AnalyzeSocialBehavior(List<FishTrack^>^ fishes);
        void StartTracking();
        void StopTracking();
        void UpdateGrid(List<FishTrack^>^ fishes);
        void UpdateStats(List<FishTrack^>^ fishes);
        void StyleGrid();
        void Log(String^ msg);
        void SafeLog(String^ msg);
        Bitmap^ CropFish(FishTrack^ fish);

        void OnFrameBitmap(Bitmap^ bmp) {
            if (this->InvokeRequired) {
                this->BeginInvoke(gcnew FrameBitmapHandler(this, &MainForm::OnFrameBitmap), bmp);
                return;
            }
            Bitmap^ old = safe_cast<Bitmap^>(_pictureBox->Image);
            _pictureBox->Image = bmp;
            if (old != nullptr) delete old;
        }
    };
}