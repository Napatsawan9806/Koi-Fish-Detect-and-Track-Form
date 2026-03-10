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

    // =====================================================
    //  GET FIRST FRAME  (for ROI preview)
    // =====================================================

    Bitmap^ DetectionService::GetFirstFrame(String^ videoPath, int% videoW, int% videoH) {
        videoW = 0; videoH = 0;
        try {
            msclr::interop::marshal_context ctx;
            std::string path = ctx.marshal_as<std::string>(videoPath);

            cv::VideoCapture cap(path);
            if (!cap.isOpened()) return nullptr;

            cv::Mat frame;
            cap >> frame;
            if (frame.empty()) return nullptr;

            videoW = frame.cols;
            videoH = frame.rows;

            // Scale down for display (max 800 x 560)
            float scale = Math::Min(800.0f / frame.cols, 560.0f / frame.rows);
            cv::Mat display;
            if (scale < 1.0f)
                cv::resize(frame, display, cv::Size((int)(frame.cols * scale), (int)(frame.rows * scale)));
            else
                display = frame;

            Bitmap^ bmp = MatHelper::MatToBitmap(display);
            return bmp;
        }
        catch (...) {
            return nullptr;
        }
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

            // ตรวจสอบว่าไฟล์ model มีอยู่จริง
            if (!System::IO::File::Exists(_modelPath)) {
                OnError(String::Format("Model file not found:\n{0}", _modelPath));
                _running = false;
                _fpsTimer->Stop();
                return;
            }

            KoiDetector      detector(modelPath);
            NativeKoiTracker tracker;

            cv::VideoCapture cap(videoPath);
            if (!cap.isOpened()) {
                OnError(String::Format("Cannot open video file:\n{0}", _videoPath));
                _running = false;
                _fpsTimer->Stop();
                return;
            }

            cv::Mat  gammaLUT = buildGammaLUT(Config::GAMMA);
            int vw = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
            int vh = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);
            _videoSize = System::Drawing::Size(vw, vh);

            // Apply user-selected ROI (Empty = full frame)
            cv::Rect roi(0, 0, vw, vh);
            bool hasCustomRoi = false;
            if (RoiRect.Width > 0 && RoiRect.Height > 0) {
                roi = cv::Rect(RoiRect.X, RoiRect.Y, RoiRect.Width, RoiRect.Height);
                roi &= cv::Rect(0, 0, vw, vh);   // clamp to frame bounds
                hasCustomRoi = true;
            }

            int frameIdx = 0;
            std::vector<SmoothDetection> lastTracked;
            auto lastDisplayTime = std::chrono::steady_clock::now();

            while (_running) {
                if (_paused) {
                    System::Threading::Thread::Sleep(50);
                    continue;
                }

                cv::Mat frame;
                cap >> frame;
                if (frame.empty()) {
                    cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                    lastTracked.clear();
                    continue;
                }

                frameIdx++;
                cv::Mat processed = applyGamma(frame, gammaLUT);

                // run detection ทุก PROCESS_EVERY เฟรม
                bool didDetect = (frameIdx % Config::PROCESS_EVERY == 0);
                if (didDetect) {
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

                // Draw ROI border + dim outside when a custom ROI is active
                if (hasCustomRoi) {
                    // Dim entire frame to 35%, then restore ROI at full brightness
                    cv::Mat dimmed;
                    processed.convertTo(dimmed, -1, 0.35, 0);
                    processed(roi).copyTo(dimmed(roi));
                    processed = dimmed;
                    // Draw cyan border around ROI
                    cv::rectangle(processed, roi, cv::Scalar(255, 220, 0), 2, cv::LINE_AA);
                }

                // ส่ง frame ไปแสดงใน PictureBox ไม่เกิน 30fps
                auto now = std::chrono::steady_clock::now();
                long long msElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastDisplayTime).count();
                if (msElapsed >= 33) {
                    cv::Mat displayFrame;
                    cv::resize(processed, displayFrame, cv::Size(640, 480), 0, 0, cv::INTER_LINEAR);
                    FireFrameBitmap(displayFrame);
                    lastDisplayTime = now;
                }

                // อัพเดต grid/stats เฉพาะตอน detect เท่านั้น
                if (didDetect)
                    UpdateFishes(lastTracked);

                _fpsCount++;
            }
        }
        catch (const Ort::Exception& ex) {
            String^ msg = String::Format("ONNX Error: {0}", gcnew String(ex.what()));
            System::Diagnostics::Debug::WriteLine(msg);
            OnError(msg);
        }
        catch (const cv::Exception& ex) {
            String^ msg = String::Format("OpenCV Error: {0}", gcnew String(ex.what()));
            System::Diagnostics::Debug::WriteLine(msg);
            OnError(msg);
        }
        catch (const std::exception& ex) {
            String^ msg = String::Format("Error: {0}", gcnew String(ex.what()));
            System::Diagnostics::Debug::WriteLine(msg);
            OnError(msg);
        }
        catch (System::Exception^ ex) {
            System::Diagnostics::Debug::WriteLine(ex->Message);
            OnError(ex->Message);
        }
        catch (...) {
            OnError("Unknown error in detection thread");
        }
        _running = false;
        _fpsTimer->Stop();
    }

    // =====================================================
    //  FIRE FRAME BITMAP
    // =====================================================

    void DetectionService::FireFrameBitmap(const cv::Mat& frame) {
        Bitmap^ bmp = MatHelper::MatToBitmap(frame);
        if (bmp == nullptr) return;
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
            for each(FishTrack ^ f in _fishes)
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

            // อัพเดตข้อมูล  (แปลงจาก ROI-local → full-frame video coords)
            float ox = (float)RoiRect.X;
            float oy = (float)RoiRect.Y;

            // 1. Set BoundingBox ก่อนเสมอ — Center property อ่านจาก BoundingBox
            existing->BoundingBox = RectangleF(
                sd.x1 + ox, sd.y1 + oy,
                (float)sd.bbox_w, (float)sd.bbox_h);
            existing->Confidence = sd.conf;

            // 2. Center จาก BoundingBox จริง (full-frame coords)
            PointF newCenter = existing->Center;

            // 3. ถ้าเป็นปลาใหม่ (Trajectory ว่างอยู่) ให้ add จุดเริ่มต้นที่ Center จริง
            if (existing->Trajectory->Count == 0)
                existing->Trajectory->Add(newCenter);

            // 4. Add trajectory ต่อเนื่อง
            existing->Trajectory->Add(newCenter);
            if (existing->Trajectory->Count > 120)
                existing->Trajectory->RemoveAt(0);

            // speed — คำนวณจาก trajectory (2 จุดล่าสุด)
            float speedVal = 0.0f;
            if (existing->Trajectory->Count >= 2) {
                PointF p0 = existing->Trajectory[existing->Trajectory->Count - 2];
                float tdx = newCenter.X - p0.X;
                float tdy = newCenter.Y - p0.Y;
                speedVal = (float)Math::Sqrt(tdx * tdx + tdy * tdy);
            }
            existing->Speed = speedVal;

            // Behavior: update speed history circular buffer
            existing->SpeedHistory[existing->SpeedHistoryIdx] = speedVal;
            existing->SpeedHistoryIdx = (existing->SpeedHistoryIdx + 1) % 30;

            // rolling avg speed (use min(FrameCount+1, 30) valid entries)
            float speedSum = 0.0f;
            int validCount = Math::Min(existing->FrameCount + 1, 30);
            for (int k = 0; k < validCount; k++)
                speedSum += existing->SpeedHistory[k];
            existing->AvgSpeed = (validCount > 0) ? speedSum / validCount : 0.0f;

            // classify activity
            if (existing->AvgSpeed < 2.0f)  existing->Activity = ActivityLevel::Resting;
            else if (existing->AvgSpeed < 8.0f)  existing->Activity = ActivityLevel::Cruising;
            else if (existing->AvgSpeed < 20.0f) existing->Activity = ActivityLevel::Active;
            else                                  existing->Activity = ActivityLevel::Erratic;

            // zone visits (3x3 grid in video-coordinate space)
            if (_videoSize.Width > 0 && _videoSize.Height > 0) {
                int zx = Math::Max(0, Math::Min(2, (int)(newCenter.X / _videoSize.Width * 3)));
                int zy = Math::Max(0, Math::Min(2, (int)(newCenter.Y / _videoSize.Height * 3)));
                existing->ZoneVisits[zy * 3 + zx]++;
            }

            existing->LastSeen = DateTime::Now;
            existing->FrameCount++;

            if (existing->Status == FishStatus::New && existing->FrameCount > 5)
                existing->Status = FishStatus::Active;

            newFishes->Add(existing);
        }

        // ปลาที่หายไป
        for each(FishTrack ^ f in _fishes)
            if (!currentIds->ContainsKey(f->FishID) && f->Status != FishStatus::Lost) {
                f->Status = FishStatus::Lost;
                OnFishLost(f);
            }

        _fishes = newFishes;
        OnFrameUpdated(_fishes);
    }
}