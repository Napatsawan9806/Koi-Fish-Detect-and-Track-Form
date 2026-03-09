#include "ROISelectForm.h"

namespace KoiTracker {

    ROISelectForm::ROISelectForm(Bitmap^ displayBitmap, int videoW, int videoH) {
        _videoW       = videoW;
        _videoH       = videoH;
        _bitmap       = displayBitmap;
        _dragging     = false;
        _hasSelection = false;
        _dragStart    = Point(0, 0);
        _dragEnd      = Point(0, 0);
        _selectedROI  = System::Drawing::Rectangle::Empty;
        InitializeComponent();
    }

    // =====================================================
    //  INITIALIZE COMPONENT
    // =====================================================

    void ROISelectForm::InitializeComponent() {
        int bw = _bitmap->Width;
        int bh = _bitmap->Height;
        int formW = bw + 20;      // 10px margin each side
        int formH = bh + 62 + 55; // 62 = header, 55 = buttons row

        this->Text            = L"Select Region of Interest";
        this->ClientSize      = System::Drawing::Size(formW, formH);
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
        this->MaximizeBox     = false;
        this->MinimizeBox     = false;
        this->StartPosition   = FormStartPosition::CenterParent;
        this->BackColor       = Color::FromArgb(22, 25, 36);
        this->ForeColor       = Color::White;
        this->Font            = gcnew Drawing::Font("Segoe UI", 9);

        // ─── HEADER ───────────────────────────────────────────────────────
        Panel^ header = gcnew Panel();
        header->BackColor = Color::FromArgb(30, 34, 50);
        header->Location  = Point(0, 0);
        header->Size      = System::Drawing::Size(formW, 60);

        _lblInstruction = gcnew Label();
        _lblInstruction->AutoSize  = false;
        _lblInstruction->Text      = L"Click and drag on the video frame to select the processing area.";
        _lblInstruction->Font      = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
        _lblInstruction->ForeColor = Color::FromArgb(190, 200, 230);
        _lblInstruction->Location  = Point(10, 8);
        _lblInstruction->Size      = System::Drawing::Size(formW - 20, 22);
        header->Controls->Add(_lblInstruction);

        _lblCoords = gcnew Label();
        _lblCoords->AutoSize  = false;
        _lblCoords->Text      = L"No selection \u2014 or click \"Use Full Frame\" to process everything";
        _lblCoords->Font      = gcnew Drawing::Font("Consolas", 8);
        _lblCoords->ForeColor = Color::FromArgb(130, 160, 200);
        _lblCoords->Location  = Point(10, 34);
        _lblCoords->Size      = System::Drawing::Size(formW - 20, 18);
        header->Controls->Add(_lblCoords);
        this->Controls->Add(header);

        // ─── PICTURE BOX ──────────────────────────────────────────────────
        _pb = gcnew PictureBox();
        _pb->Image    = _bitmap;
        _pb->Location = Point(10, 62);
        _pb->Size     = System::Drawing::Size(bw, bh);
        _pb->SizeMode = PictureBoxSizeMode::Normal;
        _pb->Cursor   = System::Windows::Forms::Cursors::Cross;
        _pb->BorderStyle = System::Windows::Forms::BorderStyle::None;

        _pb->MouseDown += gcnew MouseEventHandler(this, &ROISelectForm::OnPbMouseDown);
        _pb->MouseMove += gcnew MouseEventHandler(this, &ROISelectForm::OnPbMouseMove);
        _pb->MouseUp   += gcnew MouseEventHandler(this, &ROISelectForm::OnPbMouseUp);
        _pb->Paint     += gcnew PaintEventHandler(this, &ROISelectForm::OnPbPaint);
        this->Controls->Add(_pb);

        // ─── BUTTONS ──────────────────────────────────────────────────────
        int btnY = 62 + bh + 12;

        _btnConfirm = gcnew Button();
        _btnConfirm->Text      = L"Confirm ROI";
        _btnConfirm->Font      = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
        _btnConfirm->Location  = Point(formW / 2 - 175, btnY);
        _btnConfirm->Size      = System::Drawing::Size(115, 32);
        _btnConfirm->BackColor = Color::FromArgb(40, 160, 90);
        _btnConfirm->ForeColor = Color::White;
        _btnConfirm->FlatStyle = FlatStyle::Flat;
        _btnConfirm->FlatAppearance->BorderSize = 0;
        _btnConfirm->Enabled   = false;   // requires a drawn selection
        _btnConfirm->Click    += gcnew EventHandler(this, &ROISelectForm::BtnConfirm_Click);
        this->Controls->Add(_btnConfirm);

        _btnFullFrame = gcnew Button();
        _btnFullFrame->Text      = L"Use Full Frame";
        _btnFullFrame->Font      = gcnew Drawing::Font("Segoe UI", 9);
        _btnFullFrame->Location  = Point(formW / 2 - 55, btnY);
        _btnFullFrame->Size      = System::Drawing::Size(115, 32);
        _btnFullFrame->BackColor = Color::FromArgb(50, 70, 145);
        _btnFullFrame->ForeColor = Color::White;
        _btnFullFrame->FlatStyle = FlatStyle::Flat;
        _btnFullFrame->FlatAppearance->BorderSize = 0;
        _btnFullFrame->Click    += gcnew EventHandler(this, &ROISelectForm::BtnFullFrame_Click);
        this->Controls->Add(_btnFullFrame);

        _btnCancel = gcnew Button();
        _btnCancel->Text      = L"Cancel";
        _btnCancel->Font      = gcnew Drawing::Font("Segoe UI", 9);
        _btnCancel->Location  = Point(formW / 2 + 65, btnY);
        _btnCancel->Size      = System::Drawing::Size(115, 32);
        _btnCancel->BackColor = Color::FromArgb(80, 40, 40);
        _btnCancel->ForeColor = Color::White;
        _btnCancel->FlatStyle = FlatStyle::Flat;
        _btnCancel->FlatAppearance->BorderSize = 0;
        _btnCancel->Click    += gcnew EventHandler(this, &ROISelectForm::BtnCancel_Click);
        this->Controls->Add(_btnCancel);
    }

    // =====================================================
    //  BUTTON HANDLERS
    // =====================================================

    void ROISelectForm::BtnConfirm_Click(Object^ sender, EventArgs^ e) {
        this->DialogResult = System::Windows::Forms::DialogResult::OK;
        this->Close();
    }

    void ROISelectForm::BtnFullFrame_Click(Object^ sender, EventArgs^ e) {
        // Return empty rectangle = "no ROI restriction = full frame"
        _selectedROI = System::Drawing::Rectangle::Empty;
        this->DialogResult = System::Windows::Forms::DialogResult::OK;
        this->Close();
    }

    void ROISelectForm::BtnCancel_Click(Object^ sender, EventArgs^ e) {
        this->DialogResult = System::Windows::Forms::DialogResult::Cancel;
        this->Close();
    }

    // =====================================================
    //  MOUSE EVENTS
    // =====================================================

    void ROISelectForm::OnPbMouseDown(Object^ sender, MouseEventArgs^ me) {
        if (me->Button != System::Windows::Forms::MouseButtons::Left) return;
        _dragStart = Point(
            Math::Max(0, Math::Min(me->X, _pb->Width - 1)),
            Math::Max(0, Math::Min(me->Y, _pb->Height - 1)));
        _dragEnd      = _dragStart;
        _dragging     = true;
        _hasSelection = false;
        _btnConfirm->Enabled = false;
        _pb->Invalidate();
    }

    void ROISelectForm::OnPbMouseMove(Object^ sender, MouseEventArgs^ me) {
        if (!_dragging) return;
        _dragEnd = Point(
            Math::Max(0, Math::Min(me->X, _pb->Width - 1)),
            Math::Max(0, Math::Min(me->Y, _pb->Height - 1)));

        System::Drawing::Rectangle r = PbRectToVideoRect(_dragStart, _dragEnd);
        _lblCoords->Text = String::Format(
            "ROI preview:  x={0}  y={1}  w={2}  h={3}  (video coords)",
            r.X, r.Y, r.Width, r.Height);
        _pb->Invalidate();
    }

    void ROISelectForm::OnPbMouseUp(Object^ sender, MouseEventArgs^ me) {
        if (!_dragging) return;
        _dragEnd = Point(
            Math::Max(0, Math::Min(me->X, _pb->Width - 1)),
            Math::Max(0, Math::Min(me->Y, _pb->Height - 1)));
        _dragging = false;

        int pw = Math::Abs(_dragEnd.X - _dragStart.X);
        int ph = Math::Abs(_dragEnd.Y - _dragStart.Y);

        if (pw >= 10 && ph >= 10) {
            _hasSelection        = true;
            _selectedROI         = PbRectToVideoRect(_dragStart, _dragEnd);
            _btnConfirm->Enabled = true;
            _lblCoords->Text = String::Format(
                L"ROI: ({0}, {1})  {2} \u00D7 {3} px \u2014 press \"Confirm ROI\" to apply",
                _selectedROI.X, _selectedROI.Y,
                _selectedROI.Width, _selectedROI.Height);
        }
        else {
            _hasSelection        = false;
            _btnConfirm->Enabled = false;
            _lblCoords->Text = L"Selection too small \u2014 drag a larger area";
        }
        _pb->Invalidate();
    }

    // =====================================================
    //  PAINT OVERLAY
    // =====================================================

    void ROISelectForm::OnPbPaint(Object^ sender, PaintEventArgs^ e) {
        if (!_dragging && !_hasSelection) return;

        Graphics^ g = e->Graphics;
        g->SmoothingMode = Drawing2D::SmoothingMode::AntiAlias;

        int x1 = Math::Min(_dragStart.X, _dragEnd.X);
        int y1 = Math::Min(_dragStart.Y, _dragEnd.Y);
        int x2 = Math::Max(_dragStart.X, _dragEnd.X);
        int y2 = Math::Max(_dragStart.Y, _dragEnd.Y);
        int w  = x2 - x1;
        int h  = y2 - y1;

        if (w < 2 || h < 2) return;

        // ── Dark overlay OUTSIDE selection (4 strips) ─────────────────────
        SolidBrush^ dark = gcnew SolidBrush(Color::FromArgb(130, 0, 0, 0));
        g->FillRectangle(dark, 0,  0,         _pb->Width, y1);          // top
        g->FillRectangle(dark, 0,  y2,        _pb->Width, _pb->Height - y2); // bottom
        g->FillRectangle(dark, 0,  y1,        x1, h);                   // left
        g->FillRectangle(dark, x2, y1,        _pb->Width - x2, h);      // right
        delete dark;

        // ── Selection border (yellow dashed) ──────────────────────────────
        Pen^ selPen = gcnew Pen(Color::FromArgb(255, 255, 215, 0), 2.0f);
        selPen->DashStyle = Drawing2D::DashStyle::Dash;
        g->DrawRectangle(selPen, x1, y1, w, h);
        delete selPen;

        // ── Corner handles ────────────────────────────────────────────────
        SolidBrush^ hBr = gcnew SolidBrush(Color::Yellow);
        Pen^        hPen = gcnew Pen(Color::Black, 1.0f);
        const int hs = 7;
        array<Point>^ corners = {
            Point(x1, y1), Point(x2, y1), Point(x1, y2), Point(x2, y2)
        };
        for each (Point c in corners) {
            g->FillRectangle(hBr, c.X - hs/2, c.Y - hs/2, hs, hs);
            g->DrawRectangle(hPen, c.X - hs/2, c.Y - hs/2, hs, hs);
        }
        delete hBr; delete hPen;

        // ── Dimensions label inside selection ─────────────────────────────
        if (w > 60 && h > 20) {
            System::Drawing::Rectangle vr = PbRectToVideoRect(_dragStart, _dragEnd);
            String^ dimStr = String::Format(L"{0}\u00D7{1}", vr.Width, vr.Height);
            Drawing::Font^ dimFont = gcnew Drawing::Font("Segoe UI", 8, FontStyle::Bold);
            SizeF           dimSz  = g->MeasureString(dimStr, dimFont);
            float           tx     = x1 + (w - dimSz.Width)  / 2.0f;
            float           ty     = y1 + (h - dimSz.Height) / 2.0f;
            SolidBrush^     shadow = gcnew SolidBrush(Color::FromArgb(180, 0, 0, 0));
            SolidBrush^     label  = gcnew SolidBrush(Color::White);
            g->DrawString(dimStr, dimFont, shadow, tx + 1.0f, ty + 1.0f);
            g->DrawString(dimStr, dimFont, label,  tx, ty);
            delete dimFont; delete shadow; delete label;
        }
    }

    // =====================================================
    //  COORDINATE CONVERSION
    // =====================================================

    System::Drawing::Rectangle ROISelectForm::PbRectToVideoRect(Point a, Point b) {
        int x1 = Math::Min(a.X, b.X);
        int y1 = Math::Min(a.Y, b.Y);
        int x2 = Math::Max(a.X, b.X);
        int y2 = Math::Max(a.Y, b.Y);

        // Scale: display bitmap coords → video coords
        float scX = (_bitmap->Width  > 0) ? (float)_videoW / _bitmap->Width  : 1.0f;
        float scY = (_bitmap->Height > 0) ? (float)_videoH / _bitmap->Height : 1.0f;

        int vx = (int)(x1 * scX);
        int vy = (int)(y1 * scY);
        int vw = (int)((x2 - x1) * scX);
        int vh = (int)((y2 - y1) * scY);

        // Clamp to video bounds
        vx = Math::Max(0, Math::Min(vx, _videoW - 1));
        vy = Math::Max(0, Math::Min(vy, _videoH - 1));
        vw = Math::Max(1, Math::Min(vw, _videoW - vx));
        vh = Math::Max(1, Math::Min(vh, _videoH - vy));

        return System::Drawing::Rectangle(vx, vy, vw, vh);
    }
}
