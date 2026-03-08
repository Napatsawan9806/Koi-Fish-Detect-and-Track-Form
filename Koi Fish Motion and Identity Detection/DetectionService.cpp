#include "DetectionService.h"
#include <msclr/marshal_cppstd.h>

namespace KoiTracker {

    // =====================================================
    //  CONSTRUCTOR / DESTRUCTOR
    // =====================================================

    DetectionService::DetectionService() {
        _running = false;
        _paused = false;
        _fpsCount = 0;
        _modelPath = nullptr;
        _videoPath = nullptr;
        _fishes = gcnew List<FishTrack^>();
        _prevIds = gcnew Dictionary<int, bool>();
        _bgThread = nullptr;

        _fpsTimer = gcnew Timer();
        _fpsTimer->Interval = 1000;
        _fpsTimer->Tick += gcnew EventHandler(this, &DetectionService::FpsTick);
    }

    DetectionService::~DetectionService() {
        Stop();
    }

    // =====================================================
    //  PUBLIC API
    // =====================================================

    void DetectionService::SetModel(String^ modelPath) {
        _modelPath = modelPath;
    }

    void DetectionService::Start(String^ videoPath) {
        if (_running) return;
        _videoPath = videoPath;
        _running = true;
        _paused = false;
        _fpsCount = 0;
        _fishes->Clear();
        _prevIds->Clear();

        _bgThread = gcnew System::Threading::Thread(
            gcnew System::Threading::ThreadStart(this, &DetectionService::InferLoop)
        );
        _bgThread->IsBackground = true;
        _bgThread->Start();

        _fpsTimer->Start();
    }

    void DetectionService::Stop() {
        _running = false;
        _fpsTimer->Stop();
        if (_bgThread != nullptr && _bgThread->IsAlive)
            _bgThread->Join(3000);
        _fishes->Clear();
        _prevIds->Clear();
    }

    void DetectionService::Pause() {
        _paused = !_paused;
    }

    // =====================================================
    //  FPS TIMER
    // =====================================================

    void DetectionService::FpsTick(Object^ sender, EventArgs^ e) {
        OnFpsUpdated(_fpsCount);
        _fpsCount = 0;
    }

    // =====================================================
    //  INFERENCE LOOP (background thread)
    // =====================================================

    void DetectionService::InferLoop() {
        try {
            // แปลง managed String^ → native std::string
            msclr::interop::marshal_context ctx;
            std::string modelPath = ctx.marshal_as<std::string>(_modelPath);
            std::string videoPath = ctx.marshal_as<std::string>(_videoPath);

            KoiDetector      detector(modelPath);
            NativeKoiTracker tracker;

            cv::VideoCapture cap(videoPath);
            if (!cap.isOpened()) return;

            cv::Mat  gammaLUT = buildGammaLUT(Config::GAMMA);
            cv::Rect roi(0, 0,
                (int)cap.get(cv::CAP_PROP_FRAME_WIDTH),
                (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT));

            int frameIdx = 0;
            std::vector<SmoothDetection> lastTracked;

            while (_running) {
                if (_paused) {
                    System::Threading::Thread::Sleep(50);
                    continue;
                }

                cv::Mat frame;
                cap >> frame;
                if (frame.empty()) {
                    // วิดีโอหมด → loop กลับต้น
                    cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                    lastTracked.clear();
                    continue;
                }

                frameIdx++;
                cv::Mat processed = applyGamma(frame, gammaLUT);

                // run detection ทุก PROCESS_EVERY เฟรม
                if (frameIdx % Config::PROCESS_EVERY == 0) {
                    cv::Rect safeRoi = roi & cv::Rect(0, 0, processed.cols, processed.rows);
                    cv::Mat  roiImg = processed(safeRoi);

                    auto raw = detector.detect(roiImg, Config::CONF_THRESH);

                    std::vector<Detection> sized;
                    for (auto& d : raw)
                        if (d.box.width <= Config::MAX_BOX_W &&
                            d.box.height <= Config::MAX_BOX_H)
                            sized.push_back(d);

                    auto filtered = filterOverlap(sized, Config::OVERLAP_THRESH);
                    lastTracked = tracker.update(filtered, processed, safeRoi.tl());
                }

                // วาด bounding box ลงบน frame
                drawDetections(processed, lastTracked, roi.tl());

                // ส่ง frame ไปแสดงใน PictureBox
                FireFrameBitmap(processed);

                // อัพเดต FishTrack^ list
                UpdateFishes(lastTracked);

                _fpsCount++;
                System::Threading::Thread::Sleep(1);
            }
        }
        catch (const Ort::Exception& ex) {
            System::Diagnostics::Debug::WriteLine(
                String::Format(L"[ONNX Error] {0}", gcnew String(ex.what())));
        }
        catch (const cv::Exception& ex) {
            System::Diagnostics::Debug::WriteLine(
                String::Format(L"[OpenCV Error] {0}", gcnew String(ex.what())));
        }
        catch (const std::exception& ex) {
            System::Diagnostics::Debug::WriteLine(
                String::Format(L"[STD Error] {0}", gcnew String(ex.what())));
        }
        catch (System::Exception^ ex) {
            System::Diagnostics::Debug::WriteLine(
                String::Format(L"[Managed Error] {0}\n{1}", ex->Message, ex->StackTrace));
        }
        catch (...) {
            System::Diagnostics::Debug::WriteLine(L"[DetectionService] Unknown error");
        }
    }

    // =====================================================
    //  FIRE FRAME BITMAP
    // =====================================================

    void DetectionService::FireFrameBitmap(const cv::Mat& frame) {
        Bitmap^ bmp = MatHelper::MatToBitmap(frame);
        if (bmp != nullptr)
            OnFrameBitmap(bmp);
    }

    // =====================================================
    //  UPDATE FISH TRACKS
    // =====================================================

    void DetectionService::UpdateFishes(const std::vector<SmoothDetection>& tracked) {
        auto newFishes = gcnew List<FishTrack^>();
        auto currentIds = gcnew Dictionary<int, bool>();

        for (const auto& sd : tracked) {
            currentIds[sd.id] = true;

            // หาว่ามี FishTrack นี้อยู่แล้วมั้ย
            FishTrack^ existing = nullptr;
            for each (FishTrack ^ f in _fishes)
                if (f->FishID == sd.id) { existing = f; break; }

            if (existing == nullptr) {
                existing = gcnew FishTrack(
                    sd.id,
                    KoiSpecies::Unknown,
                    PointF((float)sd.center_x, (float)sd.center_y)
                );
                existing->Status = FishStatus::New;
                OnFishDetected(existing);
            }

            // อัพเดตข้อมูล
            existing->BoundingBox = RectangleF(sd.x1, sd.y1,
                (float)sd.bbox_w, (float)sd.bbox_h);
            existing->Confidence = sd.conf;

            // trajectory
            PointF newCenter = PointF((float)sd.center_x, (float)sd.center_y);
            existing->Trajectory->Add(newCenter);
            if (existing->Trajectory->Count > 120)
                existing->Trajectory->RemoveAt(0);

            // speed — คำนวณจาก center เดิม
            PointF oldCenter = existing->Center;
            float dx = newCenter.X - oldCenter.X;
            float dy = newCenter.Y - oldCenter.Y;
            existing->Speed = (float)Math::Sqrt(dx * dx + dy * dy);

            existing->LastSeen = DateTime::Now;
            existing->FrameCount++;

            if (existing->Status == FishStatus::New && existing->FrameCount > 5)
                existing->Status = FishStatus::Active;

            newFishes->Add(existing);
        }

        // ปลาที่หายไป
        for each (FishTrack ^ f in _fishes)
            if (!currentIds->ContainsKey(f->FishID) && f->Status != FishStatus::Lost) {
                f->Status = FishStatus::Lost;
                OnFishLost(f);
            }

        _fishes = newFishes;
        OnFrameUpdated(_fishes);
    }
}