#pragma once
#include "FishTrack.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Collections::Generic;

namespace KoiTracker {

    // ─── Custom canvas for schooling / social visualization ───────────────────
    public ref class SchoolingPanel : public Control {
    public:
        List<FishTrack^>^ Fishes;
        int VideoW;
        int VideoH;

        // Output stats for the form header (computed in UpdateFishes -> _ComputeGroups)
        int LastGroupCount;
        int LastSchoolingCount;
        int LastSoloCount;

        SchoolingPanel() {
            Fishes    = nullptr;
            VideoW    = 640; VideoH = 480;
            LastGroupCount = 0; LastSchoolingCount = 0; LastSoloCount = 0;
            _active   = nullptr;
            _groupId  = nullptr;
            _groupSize = nullptr;
            this->DoubleBuffered = true;
        }

        void UpdateFishes(List<FishTrack^>^ fishes, int videoW, int videoH) {
            Fishes = fishes;
            if (videoW > 0) VideoW = videoW;
            if (videoH > 0) VideoH = videoH;
            _ComputeGroups();   // O(n²) runs here, NOT in OnPaint
            Invalidate();
        }

    private:
        // Cached group computation results — updated in UpdateFishes(), read in OnPaint()
        List<FishTrack^>^    _active;
        array<int>^          _groupId;
        Dictionary<int,int>^ _groupSize;

        void _ComputeGroups() {
            _active    = gcnew List<FishTrack^>();
            _groupId   = nullptr;
            _groupSize = nullptr;

            if (Fishes == nullptr) return;

            for each (FishTrack^ f in Fishes)
                if (f->Status != FishStatus::Lost) _active->Add(f);

            const float SCHOOL_RADIUS = 120.0f;
            _groupId = gcnew array<int>(_active->Count);
            for (int i = 0; i < _groupId->Length; i++) _groupId[i] = i;
            for (int i = 0; i < _active->Count; i++) {
                for (int j = i + 1; j < _active->Count; j++) {
                    float dx = _active[i]->Center.X - _active[j]->Center.X;
                    float dy = _active[i]->Center.Y - _active[j]->Center.Y;
                    if (Math::Sqrt((double)(dx * dx + dy * dy)) <= SCHOOL_RADIUS)
                        _groupId[j] = _groupId[i];
                }
            }

            _groupSize = gcnew Dictionary<int,int>();
            for (int i = 0; i < _active->Count; i++) {
                if (!_groupSize->ContainsKey(_groupId[i])) _groupSize[_groupId[i]] = 0;
                _groupSize[_groupId[i]]++;
            }

            // Update stats counters
            int schoolGroups = 0, schoolFish = 0, soloFish = 0;
            for each (KeyValuePair<int,int> kv in _groupSize) {
                if (kv.Value > 1) { schoolGroups++; schoolFish += kv.Value; }
                else soloFish++;
            }
            LastGroupCount     = schoolGroups;
            LastSchoolingCount = schoolFish;
            LastSoloCount      = soloFish;
        }

    protected:
        virtual void OnPaint(PaintEventArgs^ e) override {
            Control::OnPaint(e);
            Graphics^ g = e->Graphics;
            g->SmoothingMode = Drawing2D::SmoothingMode::AntiAlias;

            // ── Background: dark pool ────────────────────────────────────────
            g->Clear(Color::FromArgb(6, 10, 18));

            // Subtle grid lines (pool grid)
            Pen^ gridPen = gcnew Pen(Color::FromArgb(22, 200, 220, 255), 1.0f);
            gridPen->DashStyle = Drawing2D::DashStyle::Dot;
            int gridStep = this->Width / 8;
            for (int gx = gridStep; gx < this->Width; gx += gridStep)
                g->DrawLine(gridPen, (float)gx, 0.0f, (float)gx, (float)this->Height);
            gridStep = this->Height / 6;
            for (int gy = gridStep; gy < this->Height; gy += gridStep)
                g->DrawLine(gridPen, 0.0f, (float)gy, (float)this->Width, (float)gy);
            delete gridPen;

            // Use cached results from last UpdateFishes() call
            List<FishTrack^>^    active    = _active;
            array<int>^          groupId   = _groupId;
            Dictionary<int,int>^ groupSize = _groupSize;

            if (active == nullptr || active->Count == 0) {
                Drawing::Font^ fnt = gcnew Drawing::Font("Segoe UI", 11);
                SolidBrush^    br  = gcnew SolidBrush(Color::FromArgb(50, 80, 130));
                g->DrawString(L"Waiting for tracking data...", fnt, br,
                    40.0f, (float)(this->Height / 2 - 16));
                delete fnt; delete br;
                return;
            }

            float scX = (float)this->Width  / VideoW;
            float scY = (float)this->Height / VideoH;

            // Group palette
            array<Color>^ gPalette = {
                Color::FromArgb(100, 180, 255),  // blue
                Color::FromArgb(120, 255, 160),  // green
                Color::FromArgb(255, 220, 80),   // yellow
                Color::FromArgb(255, 140, 200),  // pink
                Color::FromArgb(140, 220, 255),  // cyan
                Color::FromArgb(200, 160, 255),  // lavender
            };

            // ── Draw group bubbles (semi-transparent ellipse) ────────────────
            for each (KeyValuePair<int, int> kv in groupSize) {
                if (kv.Value <= 1) continue;
                int gid = kv.Key;
                Color gc = gPalette[gid % gPalette->Length];

                // Bounding box of group members
                float minX = Single::MaxValue, minY = Single::MaxValue;
                float maxX = Single::MinValue, maxY = Single::MinValue;
                for (int i = 0; i < active->Count; i++) {
                    if (groupId[i] != gid) continue;
                    float cx = active[i]->Center.X * scX;
                    float cy = active[i]->Center.Y * scY;
                    if (cx < minX) minX = cx; if (cx > maxX) maxX = cx;
                    if (cy < minY) minY = cy; if (cy > maxY) maxY = cy;
                }
                float pad = 32.0f;
                SolidBrush^ fillBr = gcnew SolidBrush(Color::FromArgb(28, gc.R, gc.G, gc.B));
                Pen^        edgePen = gcnew Pen(Color::FromArgb(110, gc.R, gc.G, gc.B), 1.5f);
                edgePen->DashStyle = Drawing2D::DashStyle::Dash;
                g->FillEllipse(fillBr, minX - pad, minY - pad,
                    (maxX - minX) + 2 * pad, (maxY - minY) + 2 * pad);
                g->DrawEllipse(edgePen, minX - pad, minY - pad,
                    (maxX - minX) + 2 * pad, (maxY - minY) + 2 * pad);
                delete fillBr; delete edgePen;
            }

            // ── Draw connection lines between same-group fish ────────────────
            for (int i = 0; i < active->Count; i++) {
                for (int j = i + 1; j < active->Count; j++) {
                    if (groupId[i] != groupId[j]) continue;
                    if (groupSize[groupId[i]] <= 1) continue;
                    Color gc = gPalette[groupId[i] % gPalette->Length];
                    float ax = active[i]->Center.X * scX;
                    float ay = active[i]->Center.Y * scY;
                    float bx = active[j]->Center.X * scX;
                    float by = active[j]->Center.Y * scY;
                    Pen^ lp = gcnew Pen(Color::FromArgb(140, gc.R, gc.G, gc.B), 1.5f);
                    g->DrawLine(lp, ax, ay, bx, by);
                    delete lp;
                }
            }

            // ── Draw fish circles + labels ────────────────────────────────────
            Drawing::Font^ fnt    = gcnew Drawing::Font("Segoe UI", 7.5f, FontStyle::Bold);
            Drawing::Font^ fntAct = gcnew Drawing::Font("Segoe UI", 7);
            const float fishR = 11.0f;

            for (int i = 0; i < active->Count; i++) {
                FishTrack^ f  = active[i];
                float      cx = f->Center.X * scX;
                float      cy = f->Center.Y * scY;
                bool       inGroup = (groupSize->ContainsKey(groupId[i]) &&
                                      groupSize[groupId[i]] > 1);

                // Fish dot
                SolidBrush^ dotBr = gcnew SolidBrush(
                    Color::FromArgb(230, f->TrackColor.R, f->TrackColor.G, f->TrackColor.B));
                Pen^ ringPen = gcnew Pen(Color::FromArgb(200, 255, 255, 255), 1.5f);
                g->FillEllipse(dotBr, cx - fishR, cy - fishR, fishR * 2, fishR * 2);
                g->DrawEllipse(ringPen, cx - fishR, cy - fishR, fishR * 2, fishR * 2);
                delete dotBr; delete ringPen;

                // Isolated blinking ring
                if (f->IsIsolated && (DateTime::Now.Millisecond / 400) % 2 == 0) {
                    Pen^ isoPen = gcnew Pen(Color::FromArgb(210, 255, 55, 55), 2.0f);
                    g->DrawEllipse(isoPen,
                        cx - fishR - 7, cy - fishR - 7,
                        (fishR + 7) * 2, (fishR + 7) * 2);
                    delete isoPen;
                }

                // ID label
                SolidBrush^ txtBr = gcnew SolidBrush(Color::White);
                g->DrawString(String::Format("#{0}", f->FishID), fnt, txtBr,
                    cx + fishR + 3.0f, cy - fishR);
                delete txtBr;

                // Activity level indicator
                String^ actStr;
                Color   actClr;
                switch (f->Activity) {
                case ActivityLevel::Resting:  actStr = "Z";  actClr = Color::FromArgb(80, 220, 120);  break;
                case ActivityLevel::Erratic:  actStr = "!!"; actClr = Color::FromArgb(255, 80, 80);   break;
                case ActivityLevel::Active:   actStr = "!";  actClr = Color::FromArgb(255, 150, 40);  break;
                default:                      actStr = "";   actClr = Color::Transparent;              break;
                }
                if (actStr->Length > 0) {
                    SolidBrush^ actBr = gcnew SolidBrush(actClr);
                    g->DrawString(actStr, fntAct, actBr, cx + fishR + 3.0f, cy + 2.0f);
                    delete actBr;
                }

                // Speed label (small, below)
                SolidBrush^ spdBr = gcnew SolidBrush(Color::FromArgb(150, 180, 200));
                g->DrawString(String::Format("{0:F1}", f->AvgSpeed), fntAct, spdBr,
                    cx - fishR, cy + fishR + 2.0f);
                delete spdBr;
            }

            delete fnt; delete fntAct;
        }
    };

    // ─── SchoolingForm ────────────────────────────────────────────────────────
    public ref class SchoolingForm : public Form {
    public:
        SchoolingForm();
        void UpdateFishes(List<FishTrack^>^ fishes, int videoW, int videoH);
        void BtnClose_Click(Object^ sender, EventArgs^ e);

    private:
        SchoolingPanel^ _canvas;
        Label^          _lblStats;

        void InitializeComponent();
    };
}
