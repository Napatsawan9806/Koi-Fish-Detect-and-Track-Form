#pragma once
#include "FishTrack.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Windows::Forms;
using namespace System::Drawing;

namespace KoiTracker {

    // delegate สำหรับ events
    public delegate void FrameUpdatedHandler(List<FishTrack^>^ fishes);
    public delegate void FishEventHandler(FishTrack^ fish);
    public delegate void FpsUpdatedHandler(int fps);

    /// <summary>
    /// จำลองข้อมูลจาก Detection + Tracking engine (~30 FPS)
    /// ทีหลังแทนด้วย API จริงได้เลย
    /// </summary>
    public ref class MockDataService {
    public:
        // Events
        event FrameUpdatedHandler^ OnFrameUpdated;
        event FishEventHandler^ OnFishDetected;
        event FishEventHandler^ OnFishLost;
        event FpsUpdatedHandler^ OnFpsUpdated;

        property SizeF PondSize;
        property bool IsRunning { bool get() { return _timer != nullptr && _timer->Enabled; } }
        property List<FishTrack^>^ Fishes { List<FishTrack^>^ get() { return _fishes; } }

        MockDataService();

        void Start();
        void Stop();
        void Pause();

    private:
        List<FishTrack^>^ _fishes;
        Timer^ _timer;
        Timer^ _fpsTimer;
        Random^ _rng;
        int                 _nextId;
        int                 _frameIndex;
        int                 _fpsCounter;

        void OnTick(Object^ sender, EventArgs^ e);
        void OnFpsTick(Object^ sender, EventArgs^ e);
        void InitialSpawn();
        FishTrack^ SpawnFish(KoiSpecies species);
        FishTrack^ SpawnFishRandom();
        PointF     RandomVelocity(float maxSpeed);
        void       UpdateFishes();
        void       MaybeSpawnOrLose();
    };
}