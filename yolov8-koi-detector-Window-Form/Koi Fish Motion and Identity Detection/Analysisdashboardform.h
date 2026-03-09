#pragma once
#include "FishTrack.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace KoiTracker {

    // ================================================================
    //  TrendPoint  — one data snapshot stored per frame-tick
    // ================================================================
    value struct TrendPoint {
        float AvgSpeed;
        float SchoolingPct;
        int   ActiveCount;
        int   AlertCount;
    };

    // ================================================================
    //  TrendChartPanel  — custom GDI+ panel for the trend graph
    // ================================================================
    public ref class TrendChartPanel : public Panel {
    public:
        List<TrendPoint>^ History;
        static const int  MAX_HISTORY = 300;

        TrendChartPanel() {
            History = gcnew List<TrendPoint>();
            this->DoubleBuffered = true;
            this->ResizeRedraw = true;
        }

        void Push(TrendPoint pt) {
            History->Add(pt);
            if (History->Count > MAX_HISTORY)
                History->RemoveAt(0);
            this->Invalidate();
        }

        void Reset() { History->Clear(); this->Invalidate(); }

    protected:
        virtual void OnPaint(PaintEventArgs^ e) override;

    private:
        void DrawSeries(Graphics^ g, int seriesIdx, float maxVal, Color col,
            int W, int H, float xStep);
    };

    // ================================================================
    //  AnalysisDashboardForm
    // ================================================================
    public ref class AnalysisDashboardForm : public Form {
    public:
        AnalysisDashboardForm();
        void UpdateDashboard(List<FishTrack^>^ fishes,
            int heatmapPeak,
            int heatmapFrames);

    private:
        Label^ _lblScore;
        Label^ _lblScoreCaption;
        Label^ _lblScoreDetail;
        Panel^ _scoreBar;
        Panel^ _scoreBarFill;

        Label^ _lblActive;
        Label^ _lblSpeed;
        Label^ _lblSchooling;
        Label^ _lblIsolated;
        Label^ _lblAlerts;
        Label^ _lblPeak;

        TrendChartPanel^ _trend;
        ListBox^ _alertLog;
        RichTextBox^ _summaryBox;

        void  InitializeComponent();
        void  BtnClear_Click(Object^ sender, EventArgs^ e);
        void  BtnClose_Click(Object^ sender, EventArgs^ e);
        void  UpdatePondSummary(List<FishTrack^>^ fishes,
            float pondAvg, float schoolingPct,
            int isolatedCount, int alertCount,
            int score, int heatmapPeak, int heatmapFrames);
        Panel^ MakeStatCard(String^ caption, Label^% valueLabel,
            Color valueColor, int x, int y);
        int   ComputeHealthScore(List<FishTrack^>^ fishes,
            float pondAvg, float schoolingPct,
            int alertCount);
        Color ScoreColor(int score);
    };

} // namespace KoiTracker