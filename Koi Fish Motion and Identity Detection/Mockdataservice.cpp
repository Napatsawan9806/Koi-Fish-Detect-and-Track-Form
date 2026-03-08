#include "MockDataService.h"
#include <cmath>

using namespace System;

namespace KoiTracker {

    MockDataService::MockDataService() {
        _fishes = gcnew List<FishTrack^>();
        _rng = gcnew Random(42);
        _nextId = 1;
        _frameIndex = 0;
        _fpsCounter = 0;
        PondSize = SizeF(640, 480);

        _timer = gcnew Timer();
        _timer->Interval = 33; // ~30 FPS
        _timer->Tick += gcnew EventHandler(this, &MockDataService::OnTick);

        _fpsTimer = gcnew Timer();
        _fpsTimer->Interval = 1000;
        _fpsTimer->Tick += gcnew EventHandler(this, &MockDataService::OnFpsTick);
    }

    // ---- Public Control ----

    void MockDataService::Start() {
        InitialSpawn();
        _timer->Start();
        _fpsTimer->Start();
    }

    void MockDataService::Stop() {
        _timer->Stop();
        _fpsTimer->Stop();
        _fishes->Clear();
        _nextId = 1;
        _frameIndex = 0;
        _fpsCounter = 0;
    }

    void MockDataService::Pause() {
        _timer->Enabled = !_timer->Enabled;
    }

    // ---- Internal ----

    void MockDataService::OnTick(Object^ sender, EventArgs^ e) {
        _frameIndex++;
        _fpsCounter++;
        UpdateFishes();
        MaybeSpawnOrLose();
        OnFrameUpdated(_fishes);
    }

    void MockDataService::OnFpsTick(Object^ sender, EventArgs^ e) {
        OnFpsUpdated(_fpsCounter);
        _fpsCounter = 0;
    }

    void MockDataService::InitialSpawn() {
        _fishes->Clear();
        _nextId = 1;

        array<KoiSpecies>^ species = {
            KoiSpecies::Kohaku,
            KoiSpecies::Showa,
            KoiSpecies::Bekko,
            KoiSpecies::Utsuri,
            KoiSpecies::Tancho
        };

        for (int i = 0; i < species->Length; i++)
            _fishes->Add(SpawnFish(species[i]));
    }

    FishTrack^ MockDataService::SpawnFish(KoiSpecies species) {
        PointF pos = PointF(
            (float)_rng->Next(50, (int)PondSize.Width - 50),
            (float)_rng->Next(50, (int)PondSize.Height - 50)
        );

        FishTrack^ fish = gcnew FishTrack(_nextId++, species, pos);
        fish->Confidence = (float)(_rng->NextDouble() * 0.25 + 0.72); // 72-97%
        fish->Status = FishStatus::New;
        fish->Velocity = RandomVelocity(3.0f);

        OnFishDetected(fish);
        return fish;
    }

    FishTrack^ MockDataService::SpawnFishRandom() {
        array<KoiSpecies>^ all = {
            KoiSpecies::Kohaku, KoiSpecies::Showa,  KoiSpecies::Bekko,
            KoiSpecies::Utsuri, KoiSpecies::Asagi,  KoiSpecies::Tancho
        };
        return SpawnFish(all[_rng->Next(all->Length)]);
    }

    PointF MockDataService::RandomVelocity(float maxSpeed) {
        double angle = _rng->NextDouble() * Math::PI * 2.0;
        float  speed = (float)(_rng->NextDouble() * maxSpeed + 0.5);
        return PointF((float)(Math::Cos(angle) * speed),
            (float)(Math::Sin(angle) * speed));
    }

    void MockDataService::UpdateFishes() {
        for each (FishTrack ^ fish in _fishes) {
            if (fish->Status == FishStatus::Lost) continue;

            PointF center = fish->Center;
            PointF vel = fish->Velocity;

            // เปลี่ยนทิศสุ่ม
            if (_rng->NextDouble() < 0.03)
                vel = RandomVelocity(3.0f);

            // เลี้ยวเมื่อชนขอบ
            float margin = 40.0f;
            if (center.X < margin || center.X > PondSize.Width - margin) vel.X = -vel.X;
            if (center.Y < margin || center.Y > PondSize.Height - margin) vel.Y = -vel.Y;

            fish->Velocity = vel;

            // อัพเดตตำแหน่ง
            float nx = Math::Max(25.0f, Math::Min(PondSize.Width - 25.0f, center.X + vel.X));
            float ny = Math::Max(15.0f, Math::Min(PondSize.Height - 15.0f, center.Y + vel.Y));

            fish->BoundingBox = RectangleF(nx - 25, ny - 15, 50, 30);
            fish->Trajectory->Add(PointF(nx, ny));
            if (fish->Trajectory->Count > 120)
                fish->Trajectory->RemoveAt(0);

            fish->Speed = (float)Math::Sqrt(vel.X * vel.X + vel.Y * vel.Y);
            fish->LastSeen = DateTime::Now;
            fish->FrameCount++;

            // confidence ขยับนิดหน่อย
            float drift = (float)(_rng->NextDouble() * 0.02 - 0.01);
            fish->Confidence = Math::Max(0.5f, Math::Min(0.99f, fish->Confidence + drift));

            if (fish->Status == FishStatus::New && fish->FrameCount > 10)
                fish->Status = FishStatus::Active;
        }
    }

    void MockDataService::MaybeSpawnOrLose() {
        // นับปลาที่ยัง active
        int active = 0;
        for each (FishTrack ^ f in _fishes)
            if (f->Status != FishStatus::Lost) active++;

        // สุ่มให้ปลาหาย
        if (active > 2 && _rng->NextDouble() < 0.004) {
            for each (FishTrack ^ f in _fishes) {
                if (f->Status == FishStatus::Active) {
                    f->Status = FishStatus::Lost;
                    OnFishLost(f);
                    break;
                }
            }
        }

        // สุ่ม spawn ปลาใหม่
        if (active < 8 && _rng->NextDouble() < 0.005)
            _fishes->Add(SpawnFishRandom());

        // ลบปลาที่ Lost นานกว่า 10 วินาที
        List<FishTrack^>^ toRemove = gcnew List<FishTrack^>();
        for each (FishTrack ^ f in _fishes)
            if (f->Status == FishStatus::Lost &&
                (DateTime::Now - f->LastSeen).TotalSeconds > 10)
                toRemove->Add(f);

        for each (FishTrack ^ f in toRemove)
            _fishes->Remove(f);
    }
}