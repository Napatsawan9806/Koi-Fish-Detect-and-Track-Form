#pragma once
#include <Windows.h>
#include <vector>

using namespace System;
using namespace System::Drawing;
using namespace System::Collections::Generic;

namespace KoiTracker {

    // สายพันธุ์ปลาคาร์ฟ
    public enum class KoiSpecies {
        Unknown,
        Kohaku,   // ขาว-แดง
        Showa,    // ดำ-แดง-ขาว
        Bekko,    // ขาว/แดง/เหลือง มีจุดดำ
        Utsuri,   // ดำ มีลาย
        Asagi,    // ฟ้า-แดง
        Tancho    // ขาว จุดแดงบนหัว
    };

    public enum class FishStatus {
        New,
        Active,
        Lost
    };

    public ref class FishTrack {
    public:
        int             FishID;
        KoiSpecies      Species;
        float           Confidence;     // 0.0 - 1.0
        RectangleF      BoundingBox;
        List<PointF>^ Trajectory;
        FishStatus      Status;
        DateTime        FirstSeen;
        DateTime        LastSeen;
        Color           TrackColor;
        float           Speed;
        int             FrameCount;

        // สำหรับ MockDataService
        PointF          Velocity;

        FishTrack(int id, KoiSpecies species, PointF startPos) {
            FishID = id;
            Species = species;
            Confidence = 0.0f;
            Trajectory = gcnew List<PointF>();
            Status = FishStatus::New;
            FirstSeen = DateTime::Now;
            LastSeen = DateTime::Now;
            TrackColor = GenerateColor(id);
            Speed = 0.0f;
            FrameCount = 0;
            Velocity = PointF(0, 0);

            BoundingBox = RectangleF(startPos.X - 25, startPos.Y - 15, 50, 30);
            Trajectory->Add(startPos);
        }

        // ตำแหน่งกลาง bounding box
        property PointF Center {
            PointF get() {
                return PointF(BoundingBox.X + BoundingBox.Width / 2.0f,
                    BoundingBox.Y + BoundingBox.Height / 2.0f);
            }
        }

        // ข้อความ confidence
        property String^ ConfidenceText {
            String^ get() {
                return String::Format("{0:F0}%", Confidence * 100);
            }
        }

        // ไอคอนสถานะ
        property String^ StatusIcon {
            String^ get() {
                switch (Status) {
                case FishStatus::New:    return L"[NEW]";
                case FishStatus::Active: return L"[OK]";
                case FishStatus::Lost:   return L"[!]";
                default:                 return L"[?]";
                }
            }
        }

        // เวลาที่ track มาแล้ว
        property String^ DurationText {
            String^ get() {
                TimeSpan dur = DateTime::Now - FirstSeen;
                return String::Format("{0:D2}:{1:D2}", (int)dur.TotalMinutes, dur.Seconds);
            }
        }

    private:
        static Color GenerateColor(int id) {
            array<Color>^ palette = {
                Color::FromArgb(255, 80,  80),
                Color::FromArgb(80,  200, 120),
                Color::FromArgb(80,  160, 255),
                Color::FromArgb(255, 180, 50),
                Color::FromArgb(200, 80,  255),
                Color::FromArgb(80,  220, 220),
                Color::FromArgb(255, 120, 180),
                Color::FromArgb(180, 255, 80),
            };
            return palette[id % palette->Length];
        }
    };
}