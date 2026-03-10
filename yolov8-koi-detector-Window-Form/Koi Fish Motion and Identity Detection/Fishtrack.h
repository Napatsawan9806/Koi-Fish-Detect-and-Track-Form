#pragma once
#include <Windows.h>
#include <vector>

using namespace System;
using namespace System::Drawing;
using namespace System::Collections::Generic;

namespace KoiTracker {

    // ��¾ѹ����Ҥ���
    public enum class KoiSpecies {
        Unknown,
        Kohaku,   // ���-ᴧ
        Showa,    // ��-ᴧ-���
        Bekko,    // ���/ᴧ/����ͧ �ըش��
        Utsuri,   // �� �����
        Asagi,    // ���-ᴧ
        Tancho    // ��� �شᴧ�����
    };

    public enum class FishStatus {
        New,
        Active,
        Lost
    };

    public enum class ActivityLevel {
        Resting,   // AvgSpeed < 2.0 px/frame
        Cruising,  // 2.0 - 8.0
        Active,    // 8.0 - 20.0
        Erratic    // >= 20.0
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

        // Behavior Analysis fields
        float           AvgSpeed;
        ActivityLevel   Activity;
        array<int>^ ZoneVisits;      // 3x3 grid = 9 zones
        array<float>^ SpeedHistory;    // circular buffer, 30 entries
        int             SpeedHistoryIdx;
        bool            IsIsolated;
        int             IsolationFrames;

        // ����Ѻ MockDataService
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
            AvgSpeed = 0.0f;
            Activity = ActivityLevel::Resting;
            ZoneVisits = gcnew array<int>(9);
            SpeedHistory = gcnew array<float>(30);
            SpeedHistoryIdx = 0;
            IsIsolated = false;
            IsolationFrames = 0;

            // BoundingBox เริ่มต้นจาก startPos (center)
            // Trajectory จะถูก add จาก Center จริงๆ ใน DetectionService
            // หลังจาก BoundingBox ถูก set แล้ว ไม่ add ที่นี่
            BoundingBox = RectangleF(startPos.X - 25, startPos.Y - 15, 50, 30);
            // ไม่ add Trajectory ที่นี่ -- จะ add จาก Center ที่แม่นยำใน UpdateFishes
        }

        // ���˹觡�ҧ bounding box
        property PointF Center{
            PointF get() {
                return PointF(BoundingBox.X + BoundingBox.Width / 2.0f,
                    BoundingBox.Y + BoundingBox.Height / 2.0f);
            }
        }

            // ��ͤ��� confidence
            property String^ ConfidenceText{
                String ^ get() {
                    return String::Format("{0:F0}%", Confidence * 100);
                }
        }

            // �ͤ͹ʶҹ�
            property String^ StatusIcon{
                String ^ get() {
                    switch (Status) {
                    case FishStatus::New:    return L"[NEW]";
                    case FishStatus::Active: return L"[OK]";
                    case FishStatus::Lost:   return L"[!]";
                    default:                 return L"[?]";
                    }
                }
        }

            // Activity level as short text
            property String^ ActivityText{
                String ^ get() {
                    switch (Activity) {
                    case ActivityLevel::Resting:  return L"REST";
                    case ActivityLevel::Cruising: return L"CRSE";
                    case ActivityLevel::Active:   return L"ACTV";
                    case ActivityLevel::Erratic:  return L"ERTC";
                    default:                      return L"----";
                    }
                }
        }

            // ���ҷ�� track ������
            property String^ DurationText{
                String ^ get() {
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