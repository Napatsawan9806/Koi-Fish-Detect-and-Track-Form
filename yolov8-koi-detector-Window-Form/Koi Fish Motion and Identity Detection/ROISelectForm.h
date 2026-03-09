#pragma once
#include "FishTrack.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace KoiTracker {

    // ─── ROI Selection Form ───────────────────────────────────────────────────
    // Shows the first frame of the video; user drags to select a rectangle.
    // Returns SelectedROI in original VIDEO coordinate space.
    public ref class ROISelectForm : public Form {
    public:
        ROISelectForm(Bitmap^ displayBitmap, int videoW, int videoH);

        // Result: rectangle in VIDEO coordinates (0,0,0,0 = full frame)
        property System::Drawing::Rectangle SelectedROI {
            System::Drawing::Rectangle get() { return _selectedROI; }
        }

        void BtnConfirm_Click(Object^ sender, EventArgs^ e);
        void BtnFullFrame_Click(Object^ sender, EventArgs^ e);
        void BtnCancel_Click(Object^ sender, EventArgs^ e);
        void OnPbMouseDown(Object^ sender, MouseEventArgs^ me);
        void OnPbMouseMove(Object^ sender, MouseEventArgs^ me);
        void OnPbMouseUp(Object^ sender, MouseEventArgs^ me);
        void OnPbPaint(Object^ sender, PaintEventArgs^ e);

    private:
        int     _videoW, _videoH;
        Bitmap^ _bitmap;        // display-sized preview bitmap

        PictureBox^ _pb;
        Label^      _lblInstruction;
        Label^      _lblCoords;
        Button^     _btnConfirm;
        Button^     _btnFullFrame;
        Button^     _btnCancel;

        Point   _dragStart;
        Point   _dragEnd;
        bool    _dragging;
        bool    _hasSelection;

        System::Drawing::Rectangle _selectedROI;  // in video coords

        void InitializeComponent();
        System::Drawing::Rectangle PbRectToVideoRect(Point a, Point b);
    };
}
