#pragma once
#include <opencv2/opencv.hpp>

using namespace System;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace System::Runtime::InteropServices;

namespace KoiTracker {

    /// <summary>
    /// แปลง cv::Mat (BGR) → System::Drawing::Bitmap^ สำหรับแสดงใน PictureBox
    /// </summary>
    public ref class MatHelper abstract sealed {
    public:
        static Bitmap^ MatToBitmap(const cv::Mat& mat) {
            if (mat.empty()) return nullptr;

            cv::Mat bgr;
            if (mat.channels() == 1)
                cv::cvtColor(mat, bgr, cv::COLOR_GRAY2BGR);
            else
                mat.copyTo(bgr);

            Bitmap^ bmp = gcnew Bitmap(bgr.cols, bgr.rows, PixelFormat::Format24bppRgb);
            System::Drawing::Rectangle rect(0, 0, bmp->Width, bmp->Height);
            BitmapData^ bmpData = bmp->LockBits(rect, ImageLockMode::WriteOnly, bmp->PixelFormat);

            for (int y = 0; y < bgr.rows; y++) {
                IntPtr dest = IntPtr(bmpData->Scan0.ToInt64() + (long long)y * bmpData->Stride);
                const uchar* src = bgr.ptr<uchar>(y);
                memcpy(dest.ToPointer(), src, bgr.cols * 3);
            }

            bmp->UnlockBits(bmpData);
            return bmp;
        }
    };
}