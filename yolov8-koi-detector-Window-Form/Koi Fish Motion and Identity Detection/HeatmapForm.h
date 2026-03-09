#pragma once
#include "FishTrack.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Collections::Generic;

namespace KoiTracker {

    // ─── Custom paint canvas for accumulated position heatmap ─────────────────
    public ref class HeatmapPanel : public Control {
    public:
        array<int, 2>^   Accum;         // [CanvasH, CanvasW] accumulated counts
        int              CanvasW;
        int              CanvasH;
        int              MaxVal;
        int              FrameCount;
        int              ActiveFish;
        List<FishTrack^>^ CurrentFishes;
        int              LastVideoW;
        int              LastVideoH;

        HeatmapPanel() {
            CanvasW = 640; CanvasH = 480;
            Accum   = nullptr;
            MaxVal  = 0; FrameCount = 0; ActiveFish = 0;
            CurrentFishes = nullptr;
            LastVideoW = 640; LastVideoH = 480;
            this->DoubleBuffered = true;
        }

        void Reset() {
            Accum      = gcnew array<int, 2>(CanvasH, CanvasW);
            MaxVal     = 0;
            FrameCount = 0;
            ActiveFish = 0;
            Invalidate();
        }

        // Called every frame from MainForm (even when form is hidden)
        void Accumulate(List<FishTrack^>^ fishes, int videoW, int videoH) {
            if (videoW <= 0) videoW = CanvasW;
            if (videoH <= 0) videoH = CanvasH;
            LastVideoW    = videoW;
            LastVideoH    = videoH;
            CurrentFishes = fishes;

            if (Accum == nullptr)
                Accum = gcnew array<int, 2>(CanvasH, CanvasW);

            float scX   = (float)CanvasW / videoW;
            float scY   = (float)CanvasH / videoH;
            const int R = 28;

            ActiveFish = 0;
            for each (FishTrack^ f in fishes) {
                if (f->Status == FishStatus::Lost) continue;
                ActiveFish++;
                int px = (int)(f->Center.X * scX);
                int py = (int)(f->Center.Y * scY);

                for (int dy = -R; dy <= R; dy++) {
                    for (int dx = -R; dx <= R; dx++) {
                        int nx = px + dx, ny = py + dy;
                        if (nx < 0 || ny < 0 || nx >= CanvasW || ny >= CanvasH) continue;
                        float dist = (float)Math::Sqrt((double)(dx * dx + dy * dy));
                        if (dist < R) {
                            int add = (int)((1.0f - dist / R) * 8);
                            Accum[ny, nx] += add;
                            if (Accum[ny, nx] > MaxVal) MaxVal = Accum[ny, nx];
                        }
                    }
                }
            }
            FrameCount++;
            if (this->Visible) Invalidate();
        }

    protected:
        virtual void OnPaint(PaintEventArgs^ e) override {
            Control::OnPaint(e);
            Graphics^ g = e->Graphics;
            g->SmoothingMode = Drawing2D::SmoothingMode::AntiAlias;

            // ── Background: deep navy ────────────────────────────────────────
            g->Clear(Color::FromArgb(4, 7, 22));

            if (Accum == nullptr || MaxVal <= 0) {
                Drawing::Font^ fnt = gcnew Drawing::Font("Segoe UI", 11);
                SolidBrush^    br  = gcnew SolidBrush(Color::FromArgb(55, 80, 160));
                g->DrawString(L"Waiting for tracking data...\nStart tracking to begin recording.",
                    fnt, br, 40.0f, (float)(this->Height / 2 - 24));
                delete fnt; delete br;
                return;
            }

            float dScX = (float)this->Width  / CanvasW;
            float dScY = (float)this->Height / CanvasH;
            const int step = 4;

            // ── Draw heatmap cells ───────────────────────────────────────────
            // Colormap: blue→cyan→green→yellow→red  (jet-like, starts from blue bg)
            for (int cy = 0; cy < CanvasH; cy += step) {
                for (int cx = 0; cx < CanvasW; cx += step) {
                    int val = Accum[cy, cx];
                    if (val == 0) continue;

                    float t = Math::Min(1.0f, (float)val / (float)MaxVal);
                    if (t < 0.03f) continue;

                    int r, gv, b, alpha;
                    alpha = (int)(t * 210) + 35;

                    if (t < 0.25f) {
                        // blue → cyan
                        float s = t / 0.25f;
                        r = 0; gv = (int)(s * 210); b = 255;
                    }
                    else if (t < 0.5f) {
                        // cyan → yellow
                        float s = (t - 0.25f) / 0.25f;
                        r = (int)(s * 220); gv = 210 + (int)(s * 45); b = (int)((1.0f - s) * 255);
                    }
                    else if (t < 0.75f) {
                        // yellow → orange
                        float s = (t - 0.5f) / 0.25f;
                        r = 220 + (int)(s * 35); gv = 255 - (int)(s * 120); b = 0;
                    }
                    else {
                        // orange → red → white-hot
                        float s = (t - 0.75f) / 0.25f;
                        r = 255; gv = (int)((1.0f - s) * 135); b = (int)(s * 80);
                    }

                    SolidBrush^ br = gcnew SolidBrush(Color::FromArgb(alpha, r, gv, b));
                    g->FillRectangle(br,
                        cx * dScX, cy * dScY,
                        step * dScX + 1.0f, step * dScY + 1.0f);
                    delete br;
                }
            }

            // ── Current fish positions as white dots ─────────────────────────
            if (CurrentFishes != nullptr) {
                float fishScX = (LastVideoW > 0) ? (float)this->Width  / LastVideoW : 1.0f;
                float fishScY = (LastVideoH > 0) ? (float)this->Height / LastVideoH : 1.0f;
                Drawing::Font^ fnt = gcnew Drawing::Font("Segoe UI", 7.5f, FontStyle::Bold);

                for each (FishTrack^ f in CurrentFishes) {
                    if (f->Status == FishStatus::Lost) continue;
                    float px = f->Center.X * fishScX;
                    float py = f->Center.Y * fishScY;

                    SolidBrush^ dotBr = gcnew SolidBrush(f->TrackColor);
                    Pen^        ring  = gcnew Pen(Color::White, 1.5f);
                    g->FillEllipse(dotBr, px - 5.0f, py - 5.0f, 10.0f, 10.0f);
                    g->DrawEllipse(ring,  px - 7.0f, py - 7.0f, 14.0f, 14.0f);
                    delete dotBr; delete ring;

                    SolidBrush^ txtBr = gcnew SolidBrush(Color::White);
                    g->DrawString(String::Format("#{0}", f->FishID), fnt, txtBr, px + 8.0f, py - 9.0f);
                    delete txtBr;
                }
                delete fnt;
            }
        }
    };

    // ─── HeatmapForm ──────────────────────────────────────────────────────────
    public ref class HeatmapForm : public Form {
    public:
        HeatmapForm();
        void Accumulate(List<FishTrack^>^ fishes, int videoW, int videoH);
        void BtnClear_Click(Object^ sender, EventArgs^ e);
        void BtnClose_Click(Object^ sender, EventArgs^ e);

    private:
        HeatmapPanel^ _canvas;
        Label^        _lblStats;

        void InitializeComponent();
    };
}
