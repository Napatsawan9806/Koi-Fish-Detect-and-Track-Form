#pragma once
#include "FishTrack.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace KoiTracker {

    // Mini control วาด trajectory
    public ref class TrajectoryPanel : public Control {
    public:
        FishTrack^ Fish;

        TrajectoryPanel(FishTrack^ fish) {
            Fish = fish;
            this->DoubleBuffered = true;
        }

    protected:
        virtual void OnPaint(PaintEventArgs^ e) override {
            Control::OnPaint(e);
            Graphics^ g = e->Graphics;
            g->SmoothingMode = Drawing2D::SmoothingMode::AntiAlias;
            g->Clear(this->BackColor);

            List<PointF>^ pts = Fish->Trajectory;
            if (pts == nullptr || pts->Count < 2) return;

            // หา bounding
            float minX = Single::MaxValue, minY = Single::MaxValue;
            float maxX = Single::MinValue, maxY = Single::MinValue;
            for each (PointF p in pts) {
                if (p.X < minX) minX = p.X;
                if (p.Y < minY) minY = p.Y;
                if (p.X > maxX) maxX = p.X;
                if (p.Y > maxY) maxY = p.Y;
            }

            float rangeX = Math::Max(maxX - minX, 10.0f);
            float rangeY = Math::Max(maxY - minY, 10.0f);
            float pad = 10.0f;
            float scaleX = (this->Width - pad * 2) / rangeX;
            float scaleY = (this->Height - pad * 2) / rangeY;

            // วาดเส้น gradient
            for (int i = 1; i < pts->Count; i++) {
                float alpha = (float)i / pts->Count;
                Pen^ pen = gcnew Pen(Color::FromArgb((int)(alpha * 220), Fish->TrackColor), 1.5f);
                PointF a = PointF((pts[i - 1].X - minX) * scaleX + pad, (pts[i - 1].Y - minY) * scaleY + pad);
                PointF b = PointF((pts[i].X - minX) * scaleX + pad, (pts[i].Y - minY) * scaleY + pad);
                g->DrawLine(pen, a, b);
                delete pen;
            }

            // จุดปัจจุบัน
            PointF last = PointF(
                (pts[pts->Count - 1].X - minX) * scaleX + pad,
                (pts[pts->Count - 1].Y - minY) * scaleY + pad
            );
            SolidBrush^ br = gcnew SolidBrush(Fish->TrackColor);
            g->FillEllipse(br, last.X - 4.0f, last.Y - 4.0f, 8.0f, 8.0f);
            delete br;
        }
    };

    // ---- FishDetailForm ----
    public ref class FishDetailForm : public Form {
    public:
        FishDetailForm(FishTrack^ fish);
        void UpdateData(FishTrack^ fish);

        // Event handler สำหรับปุ่ม Close
        void BtnClose_Click(Object^ sender, EventArgs^ e) { this->Close(); }

    private:
        FishTrack^ _fish;

        // Controls
        Panel^ _colorBand;
        Panel^ _header;
        Label^ _lblTitle;
        Label^ _lblStatus;
        Label^ _lblSpeciesVal;
        Label^ _lblConfVal;
        Label^ _lblSpeedVal;
        Label^ _lblFramesVal;
        Label^ _lblFirstVal;
        Label^ _lblLastVal;
        Label^ _lblDurVal;
        TrajectoryPanel^ _trajPanel;
        Button^ _btnClose;

        void InitializeComponent();
        Label^ MakeKeyLabel(String^ text, int x, int y);
        Label^ MakeValLabel(String^ text, int x, int y);
    };
}