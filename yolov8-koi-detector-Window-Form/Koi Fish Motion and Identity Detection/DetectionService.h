#pragma once
#include "KoiDetectCore.h"
#include "FishTrack.h"
#include "MatHelper.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace KoiTracker {

    // delegates
    public delegate void FrameUpdatedHandler(List<FishTrack^>^ fishes);
    public delegate void FishEventHandler(FishTrack^ fish);
    public delegate void FpsUpdatedHandler(int fps);
    public delegate void FrameBitmapHandler(Bitmap^ frame);
    public delegate void ErrorHandler(String^ message);

    /// <summary>
    /// ᷹��� MockDataService � �� KoiDetector + KoiTracker ��ԧ
    /// </summary>
    public ref class DetectionService {
    public:
        event FrameUpdatedHandler^ OnFrameUpdated;
        event FishEventHandler^ OnFishDetected;
        event FishEventHandler^ OnFishLost;
        event FpsUpdatedHandler^ OnFpsUpdated;
        event FrameBitmapHandler^ OnFrameBitmap;
        event ErrorHandler^ OnError;

        property SizeF PondSize;
        property bool  IsRunning { bool get() { return _running; } }

        // ROI in video coordinates; Empty = full frame
        property System::Drawing::Rectangle RoiRect;

        DetectionService();
        ~DetectionService();

        void SetModel(String^ modelPath);
        void Start(String^ videoPath);
        void Stop();
        void Pause();

        // Grab the first frame of a video for ROI preview (resized to max 800x560)
        static Bitmap^ GetFirstFrame(String^ videoPath, int% videoW, int% videoH);

        property System::Drawing::Size VideoSize { System::Drawing::Size get() { return _videoSize; } }

    private:
        System::Drawing::Size _videoSize;
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