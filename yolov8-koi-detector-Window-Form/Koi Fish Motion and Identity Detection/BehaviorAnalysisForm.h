#pragma once
#include "FishTrack.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Collections::Generic;
using namespace System::ComponentModel;

namespace KoiTracker {

    // ─── Speed Bar Chart ─────────────────────────────────────────────────────
    // Shows horizontal bars per fish, colored by activity level
    public ref class SpeedChartPanel : public Control {
    public:
        List<FishTrack^>^ Fishes;
        float PondAvgSpeed;

        SpeedChartPanel() {
            Fishes = nullptr;
            PondAvgSpeed = 0.0f;
            this->DoubleBuffered = true;
        }

    protected:
        virtual void OnPaint(PaintEventArgs^ e) override {
            Control::OnPaint(e);
            Graphics^ g = e->Graphics;
            g->Clear(this->BackColor);
            g->SmoothingMode = Drawing2D::SmoothingMode::AntiAlias;

            if (Fishes == nullptr || Fishes->Count == 0) {
                Drawing::Font^ f = gcnew Drawing::Font("Segoe UI", 9);
                SolidBrush^ b = gcnew SolidBrush(Color::FromArgb(100, 110, 130));
                g->DrawString("No fish data", f, b, 10.0f, 10.0f);
                delete f; delete b;
                return;
            }

            // Collect active fish only, sorted by AvgSpeed descending (insertion sort)
            List<FishTrack^>^ active = gcnew List<FishTrack^>();
            for each (FishTrack^ f in Fishes)
                if (f->Status != FishStatus::Lost) active->Add(f);

            for (int i = 1; i < active->Count; i++) {
                FishTrack^ key = active[i];
                int j = i - 1;
                while (j >= 0 && active[j]->AvgSpeed < key->AvgSpeed) {
                    active[j + 1] = active[j];
                    j--;
                }
                active[j + 1] = key;
            }

            float maxSpeed = 0.1f;
            for each (FishTrack^ f in active)
                if (f->AvgSpeed > maxSpeed) maxSpeed = f->AvgSpeed;

            int labelW = 42;
            int valW   = 38;
            int barAreaW = this->Width - labelW - valW - 8;
            int n      = active->Count;
            int rowH   = (n > 0) ? Math::Max(14, Math::Min(28, (this->Height - 20) / n)) : 20;

            Drawing::Font^ fnt = gcnew Drawing::Font("Consolas", 7.5f);
            SolidBrush^ lblBr  = gcnew SolidBrush(Color::FromArgb(160, 165, 195));

            for (int i = 0; i < n && i * rowH + 10 < this->Height - 5; i++) {
                FishTrack^ fish = active[i];
                int y = 10 + i * rowH;

                // Fish ID label
                g->DrawString(String::Format("#{0}", fish->FishID), fnt, lblBr, 4.0f, (float)y + 1);

                // Bar color by activity
                Color barColor;
                switch (fish->Activity) {
                case ActivityLevel::Resting:  barColor = Color::FromArgb(80, 220, 120);  break;
                case ActivityLevel::Cruising: barColor = Color::FromArgb(220, 200, 80);  break;
                case ActivityLevel::Active:   barColor = Color::FromArgb(255, 140, 40);  break;
                case ActivityLevel::Erratic:  barColor = Color::FromArgb(255, 80,  80);  break;
                default:                      barColor = Color::FromArgb(100, 100, 120); break;
                }
                float t    = fish->AvgSpeed / maxSpeed;
                int   barW = (int)(t * barAreaW);
                if (barW < 2) barW = 2;

                SolidBrush^ barBr = gcnew SolidBrush(Color::FromArgb(190, barColor.R, barColor.G, barColor.B));
                g->FillRectangle(barBr, (float)labelW, (float)y + 2, (float)barW, (float)(rowH - 5));
                delete barBr;

                // Speed value
                SolidBrush^ valBr = gcnew SolidBrush(barColor);
                g->DrawString(String::Format("{0:F1}", fish->AvgSpeed), fnt, valBr,
                    (float)(labelW + barAreaW + 4), (float)y + 1);
                delete valBr;
            }

            // Pond average vertical line
            if (PondAvgSpeed > 0.0f && barAreaW > 0) {
                float avgX = labelW + (PondAvgSpeed / maxSpeed) * barAreaW;
                if (avgX > labelW && avgX < labelW + barAreaW) {
                    Pen^ avgPen = gcnew Pen(Color::FromArgb(160, 180, 210, 255), 1.5f);
                    avgPen->DashStyle = Drawing2D::DashStyle::Dash;
                    g->DrawLine(avgPen, avgX, 4.0f, avgX, (float)(this->Height - 6));
                    delete avgPen;
                    SolidBrush^ avgBr = gcnew SolidBrush(Color::FromArgb(140, 180, 210, 255));
                    g->DrawString("avg", fnt, avgBr, avgX + 2, 4.0f);
                    delete avgBr;
                }
            }

            delete fnt; delete lblBr;
        }
    };

    // ─── Zone Map (3x3 aggregate heatmap) ────────────────────────────────────
    public ref class ZoneMapPanel : public Control {
    public:
        List<FishTrack^>^ Fishes;

        ZoneMapPanel() {
            Fishes = nullptr;
            this->DoubleBuffered = true;
        }

    protected:
        virtual void OnPaint(PaintEventArgs^ e) override {
            Control::OnPaint(e);
            Graphics^ g = e->Graphics;
            g->Clear(this->BackColor);
            g->SmoothingMode = Drawing2D::SmoothingMode::AntiAlias;

            if (Fishes == nullptr || Fishes->Count == 0) {
                Drawing::Font^ f = gcnew Drawing::Font("Segoe UI", 9);
                SolidBrush^ b = gcnew SolidBrush(Color::FromArgb(100, 110, 130));
                g->DrawString("No data", f, b, 10.0f, 10.0f);
                delete f; delete b;
                return;
            }

            // Aggregate zone visits across all active fish
            array<int>^ totals = gcnew array<int>(9);
            int maxV = 1;
            for each (FishTrack^ fish in Fishes) {
                if (fish->Status == FishStatus::Lost) continue;
                for (int z = 0; z < 9; z++) {
                    totals[z] += fish->ZoneVisits[z];
                    if (totals[z] > maxV) maxV = totals[z];
                }
            }

            float cw = (float)this->Width  / 3.0f;
            float ch = (float)this->Height / 3.0f;

            Drawing::Font^ fntBig = gcnew Drawing::Font("Segoe UI", 9.0f, FontStyle::Bold);
            Drawing::Font^ fntSub = gcnew Drawing::Font("Segoe UI", 7.0f);
            array<String^>^ zoneNames = { "NW", "N", "NE", "W", "C", "E", "SW", "S", "SE" };

            for (int z = 0; z < 9; z++) {
                int zx = z % 3, zy = z / 3;
                float x = zx * cw, y = zy * ch;
                float t = (float)totals[z] / (float)maxV;

                // Background intensity
                int alpha = (int)(t * 155) + 25;
                SolidBrush^ fillBr = gcnew SolidBrush(Color::FromArgb(alpha, 80, 140, 255));
                g->FillRectangle(fillBr, x, y, cw, ch);
                delete fillBr;

                // Zone name (top-left)
                SolidBrush^ nameBr = gcnew SolidBrush(Color::FromArgb(160, 170, 200));
                g->DrawString(zoneNames[z], fntSub, nameBr, x + 4.0f, y + 4.0f);
                delete nameBr;

                // Percentage (center)
                int pct = (int)(t * 100.0f);
                SolidBrush^ pctBr = gcnew SolidBrush(Color::White);
                String^ pctStr = String::Format("{0}%", pct);
                SizeF sz = g->MeasureString(pctStr, fntBig);
                g->DrawString(pctStr, fntBig, pctBr,
                    x + (cw - sz.Width) / 2.0f,
                    y + (ch - sz.Height) / 2.0f);
                delete pctBr;
            }

            // Grid lines
            Pen^ gp = gcnew Pen(Color::FromArgb(90, 200, 210, 230), 1.0f);
            for (int i = 1; i < 3; i++) {
                g->DrawLine(gp, cw * i, 0.0f, cw * i, (float)this->Height);
                g->DrawLine(gp, 0.0f, ch * i, (float)this->Width, ch * i);
            }
            delete gp;

            delete fntBig; delete fntSub;
        }
    };

    // ─── BehaviorAnalysisForm ─────────────────────────────────────────────────
    public ref class BehaviorAnalysisForm : public Form {
    public:
        BehaviorAnalysisForm();
        void UpdateAnalysis(List<FishTrack^>^ fishes);
        void BtnClose_Click(Object^ sender, EventArgs^ e) { this->Close(); }

    private:
        // Header stat labels
        Label^ _lblActiveVal;
        Label^ _lblAvgSpeedVal;
        Label^ _lblSchoolingVal;
        Label^ _lblIsolatedVal;

        // Fish breakdown grid
        DataGridView^ _fishGrid;

        // Alerts
        ListBox^ _alertList;

        // Charts
        SpeedChartPanel^ _speedChart;
        ZoneMapPanel^    _zoneMap;

        // Activity breakdown text
        Label^ _lblAnalysis;

        Button^ _btnClose;

        void InitializeComponent();
        void StyleFishGrid();
    };
}
