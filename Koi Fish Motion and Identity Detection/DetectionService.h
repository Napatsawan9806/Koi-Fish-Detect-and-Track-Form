#pragma once
#include "KoiDetectCore.h"
#include "FishTrack.h"
#include "MatHelper.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace KoiTracker {

    // delegates — เหมือน MockDataService เดิมทุกอย่าง
    public delegate void FrameUpdatedHandler(List<FishTrack^>^ fishes);
    public delegate void FishEventHandler(FishTrack^ fish);
    public delegate void FpsUpdatedHandler(int fps);
    public delegate void FrameBitmapHandler(Bitmap^ frame);

    /// <summary>
    /// แทนที่ MockDataService — ใช้ KoiDetector + KoiTracker จริง
    /// </summary>
    public ref class DetectionService {
    public:
        event FrameUpdatedHandler^ OnFrameUpdated;
        event FishEventHandler^ OnFishDetected;
        event FishEventHandler^ OnFishLost;
        event FpsUpdatedHandler^ OnFpsUpdated;
        event FrameBitmapHandler^ OnFrameBitmap;

        property SizeF PondSize;
        property bool  IsRunning { bool get() { return _running; } }

        DetectionService();
        ~DetectionService();

        void SetModel(String^ modelPath);
        void Start(String^ videoPath);
        void Stop();
        void Pause();

    private:
        bool                       _running;
        bool                       _paused;
        int                        _fpsCount;
        String^ _modelPath;
        String^ _videoPath;
        List<FishTrack^>^ _fishes;
        Dictionary<int, bool>^ _prevIds;
        System::Threading::Thread^ _bgThread;
        Timer^ _fpsTimer;

        void FpsTick(Object^ sender, EventArgs^ e);
        void InferLoop();
        void UpdateFishes(const std::vector<SmoothDetection>& tracked);
        void FireFrameBitmap(const cv::Mat& frame);
    };
}