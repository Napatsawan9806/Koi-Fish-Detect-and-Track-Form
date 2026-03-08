#pragma once
/**
 * ================================================================
 *   KoiDetectCore.h  —  Koi Fish Detector + Tracker Core
 *   ดึงมาจาก test6.cpp (ลบ main() และ openVideoDialog() ออก)
 *   ใช้ใน DetectionService.h/.cpp
 * ================================================================
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <cmath>

#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>

 // ================================================================
 //  CONFIG
 // ================================================================
namespace Config {
    constexpr int   INPUT_W = 640;
    constexpr int   INPUT_H = 640;
    constexpr float CONF_THRESH = 0.7f;
    constexpr float OVERLAP_THRESH = 0.45f;
    constexpr int   MAX_BOX_W = 80;
    constexpr int   MAX_BOX_H = 80;
    constexpr float GAMMA = 0.50f;
    constexpr int   PROCESS_EVERY = 8;
    constexpr float BOX_SMOOTH = 1.0f;

    constexpr float W_IOU = 0.45f;
    constexpr float W_COLOR = 0.35f;
    constexpr float W_MOTION = 0.20f;
    constexpr float MATCH_THRESH = 0.25f;
    constexpr int   MAX_MISS = 12;
    constexpr int   MAX_MISS_DELETE = 20;
    constexpr float HIST_LR = 0.08f;
    constexpr int   HIST_BINS = 32;
    constexpr float REID_THRESH = 0.55f;
    constexpr int   BANK_MAX_AGE = 300;
}

// ================================================================
//  STRUCT
// ================================================================
struct Detection {
    cv::Rect box;
    float    conf = 0.f;
};

struct SmoothDetection {
    float x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    float conf = 0.f;
    int   id = -1;
    int   bbox_w = 0;
    int   bbox_h = 0;
    int   center_x = 0;
    int   center_y = 0;

    SmoothDetection() = default;
    SmoothDetection(const Detection& d, int trackId = -1)
        : x1((float)d.box.x), y1((float)d.box.y)
        , x2((float)(d.box.x + d.box.width))
        , y2((float)(d.box.y + d.box.height))
        , conf(d.conf), id(trackId)
    {
        recalc();
    }

    void recalc() {
        bbox_w = (int)(x2 - x1);
        bbox_h = (int)(y2 - y1);
        center_x = (int)(x1 + bbox_w * 0.5f);
        center_y = (int)(y1 + bbox_h * 0.5f);
    }

    void lerpTo(const SmoothDetection& target, float t) {
        x1 = x1 + (target.x1 - x1) * t;
        y1 = y1 + (target.y1 - y1) * t;
        x2 = x2 + (target.x2 - x2) * t;
        y2 = y2 + (target.y2 - y2) * t;
        conf = conf + (target.conf - conf) * t;
        recalc();
    }

    cv::Rect toRect() const {
        return cv::Rect((int)x1, (int)y1, bbox_w, bbox_h);
    }
};

// ================================================================
//  COLOR HISTOGRAM
// ================================================================
static cv::Mat computeColorHist(const cv::Mat& bgr, const cv::Rect& box) {
    cv::Rect safe = box & cv::Rect(0, 0, bgr.cols, bgr.rows);
    if (safe.width <= 4 || safe.height <= 4) return cv::Mat();
    cv::Mat patch = bgr(safe), hsv;
    cv::cvtColor(patch, hsv, cv::COLOR_BGR2HSV);
    int   histSize[] = { Config::HIST_BINS, Config::HIST_BINS };
    float hRanges[] = { 0, 180 };
    float sRanges[] = { 0, 256 };
    const float* ranges[] = { hRanges, sRanges };
    int   channels[] = { 0, 1 };
    cv::Mat hist;
    cv::calcHist(&hsv, 1, channels, cv::Mat(), hist, 2, histSize, ranges, true, false);
    cv::normalize(hist, hist, 0, 1, cv::NORM_MINMAX);
    return hist;
}

static float histSimilarity(const cv::Mat& a, const cv::Mat& b) {
    if (a.empty() || b.empty()) return 0.5f;
    double d = cv::compareHist(a, b, cv::HISTCMP_BHATTACHARYYA);
    return (float)(1.0 - d);
}

// ================================================================
//  VELOCITY MODEL
// ================================================================
struct VelocityModel {
    float vx = 0, vy = 0;
    float x = 0, y = 0;

    void update(const cv::Rect& box) {
        float nx = box.x + box.width * 0.5f;
        float ny = box.y + box.height * 0.5f;
        vx = vx * 0.7f + (nx - x) * 0.3f;
        vy = vy * 0.7f + (ny - y) * 0.3f;
        x = nx;  y = ny;
    }

    cv::Point2f predict(int dt = 1) const {
        return { x + vx * dt, y + vy * dt };
    }
};

// ================================================================
//  TRACKER
// ================================================================
class NativeKoiTracker {
public:
    struct Track {
        int           id;
        cv::Rect      box;
        float         conf = 0.f;
        int           missedFrames = 0;
        cv::Mat       colorHist;
        VelocityModel vel;
    };

    struct BankEntry {
        int     id;
        cv::Mat colorHist;
        int     framesInBank = 0;
    };

    NativeKoiTracker() : nextId_(1) {}
    int trackCount() const { return (int)tracks_.size(); }

    std::vector<SmoothDetection> update(const std::vector<Detection>& dets,
        const cv::Mat& fullFrame,
        const cv::Point& roiOffset)
    {
        const int nT = (int)tracks_.size();
        const int nD = (int)dets.size();

        std::vector<cv::Mat> detHists(nD);
        for (int d = 0; d < nD; ++d) {
            cv::Rect absBox = dets[d].box;
            absBox.x += roiOffset.x;
            absBox.y += roiOffset.y;
            detHists[d] = computeColorHist(fullFrame, absBox);
        }

        std::vector<std::tuple<float, int, int>> pairs;
        pairs.reserve(nT * nD);

        for (int t = 0; t < nT; ++t) {
            cv::Point2f pred = tracks_[t].vel.predict(
                std::max(1, tracks_[t].missedFrames + 1));

            for (int d = 0; d < nD; ++d) {
                float iou = calcIoU(tracks_[t].box, dets[d].box);
                float colorSim = histSimilarity(tracks_[t].colorHist, detHists[d]);
                float dcx = (dets[d].box.x + dets[d].box.width * 0.5f) - pred.x;
                float dcy = (dets[d].box.y + dets[d].box.height * 0.5f) - pred.y;
                float dist = std::sqrt(dcx * dcx + dcy * dcy);
                float sigma = (float)(std::max(tracks_[t].box.width,
                    tracks_[t].box.height)) * 1.5f + 1.f;
                float motionSim = std::exp(-0.5f * (dist / sigma) * (dist / sigma));
                float score = Config::W_IOU * iou
                    + Config::W_COLOR * colorSim
                    + Config::W_MOTION * motionSim;
                if (score > Config::MATCH_THRESH)
                    pairs.push_back(std::make_tuple(score, t, d));
            }
        }

        std::sort(pairs.begin(), pairs.end(),
            [](const std::tuple<float, int, int>& a,
                const std::tuple<float, int, int>& b) {
                    return std::get<0>(a) > std::get<0>(b); });

        std::vector<bool> detMatched(nD, false);
        std::vector<bool> trkMatched(nT, false);

        for (int pi = 0; pi < (int)pairs.size(); ++pi) {
            int ti = std::get<1>(pairs[pi]);
            int di = std::get<2>(pairs[pi]);
            if (trkMatched[ti] || detMatched[di]) continue;
            tracks_[ti].box = dets[di].box;
            tracks_[ti].conf = dets[di].conf;
            tracks_[ti].missedFrames = 0;
            tracks_[ti].vel.update(dets[di].box);
            if (!detHists[di].empty()) {
                if (tracks_[ti].colorHist.empty())
                    tracks_[ti].colorHist = detHists[di].clone();
                else
                    cv::addWeighted(tracks_[ti].colorHist, 1.0 - Config::HIST_LR,
                        detHists[di], Config::HIST_LR,
                        0.0, tracks_[ti].colorHist);
            }
            trkMatched[ti] = true;
            detMatched[di] = true;
        }

        for (int t = 0; t < nT; ++t)
            if (!trkMatched[t]) tracks_[t].missedFrames++;

        // บันทึกลง ID Bank ก่อนลบ
        for (int t = 0; t < (int)tracks_.size(); ++t) {
            if (tracks_[t].missedFrames == Config::MAX_MISS_DELETE
                && !tracks_[t].colorHist.empty()) {
                BankEntry be;
                be.id = tracks_[t].id;
                be.colorHist = tracks_[t].colorHist.clone();
                be.framesInBank = 0;
                bool found = false;
                for (auto& b : idBank_) {
                    if (b.id == be.id) {
                        b.colorHist = be.colorHist;
                        b.framesInBank = 0;
                        found = true; break;
                    }
                }
                if (!found) idBank_.push_back(be);
            }
        }

        tracks_.erase(
            std::remove_if(tracks_.begin(), tracks_.end(),
                [](const Track& tk) { return tk.missedFrames > Config::MAX_MISS_DELETE; }),
            tracks_.end());

        // ID Bank Re-ID
        for (int d = 0; d < nD; ++d) {
            if (detMatched[d]) continue;
            int   bestBankIdx = -1;
            float bestBankSim = Config::REID_THRESH;
            for (int bi = 0; bi < (int)idBank_.size(); ++bi) {
                float sim = histSimilarity(idBank_[bi].colorHist, detHists[d]);
                if (sim > bestBankSim) { bestBankSim = sim; bestBankIdx = bi; }
            }
            Track tk;
            if (bestBankIdx >= 0) {
                tk.id = idBank_[bestBankIdx].id;
                tk.colorHist = idBank_[bestBankIdx].colorHist.clone();
                idBank_.erase(idBank_.begin() + bestBankIdx);
            }
            else {
                tk.id = nextId_++;
                tk.colorHist = detHists[d].clone();
            }
            tk.box = dets[d].box;  tk.conf = dets[d].conf;
            tk.missedFrames = 0;   tk.vel.update(dets[d].box);
            tracks_.push_back(std::move(tk));
        }

        for (auto& b : idBank_) b.framesInBank++;
        idBank_.erase(
            std::remove_if(idBank_.begin(), idBank_.end(),
                [](const BankEntry& b) { return b.framesInBank > Config::BANK_MAX_AGE; }),
            idBank_.end());

        std::vector<SmoothDetection> out;
        out.reserve(tracks_.size());
        for (auto& tk : tracks_) {
            if (tk.missedFrames > 0) continue;
            Detection tmp{ tk.box, tk.conf };
            out.push_back(SmoothDetection(tmp, tk.id));
        }
        return out;
    }

private:
    static float calcIoU(const cv::Rect& a, const cv::Rect& b) {
        cv::Rect inter = a & b;
        if (inter.width <= 0 || inter.height <= 0) return 0.f;
        float iArea = (float)(inter.width * inter.height);
        float uArea = (float)(a.area() + b.area()) - iArea;
        return uArea > 0.f ? iArea / uArea : 0.f;
    }
    std::vector<Track>     tracks_;
    std::vector<BankEntry> idBank_;
    int                    nextId_;
};

// ================================================================
//  UTILITY
// ================================================================
inline cv::Mat buildGammaLUT(double gamma) {
    cv::Mat lut(1, 256, CV_8UC1);
    uchar* p = lut.ptr();
    double inv = 1.0 / gamma;
    for (int i = 0; i < 256; ++i)
        p[i] = cv::saturate_cast<uchar>(std::pow(i / 255.0, inv) * 255.0);
    return lut;
}

inline cv::Mat applyGamma(const cv::Mat& src, const cv::Mat& lut) {
    cv::Mat dst; cv::LUT(src, lut, dst); return dst;
}

inline std::vector<Detection> filterOverlap(std::vector<Detection> dets, float thresh) {
    if (dets.empty()) return {};
    std::sort(dets.begin(), dets.end(),
        [](const Detection& a, const Detection& b) { return a.conf > b.conf; });
    std::vector<Detection> keep;
    while (!dets.empty()) {
        Detection cur = dets.front(); dets.erase(dets.begin());
        float cArea = (float)(cur.box.width * cur.box.height);
        if (cArea <= 0.f) continue;
        bool dup = false;
        for (const auto& k : keep) {
            cv::Rect inter = cur.box & k.box;
            if (inter.width <= 0 || inter.height <= 0) continue;
            float iArea = (float)(inter.width * inter.height);
            float kArea = (float)(k.box.width * k.box.height);
            float minArea = std::min(cArea, kArea);
            if (minArea > 0.f && iArea / minArea > thresh) { dup = true; break; }
        }
        if (!dup) keep.push_back(cur);
    }
    return keep;
}

// ================================================================
//  YOLO DETECTOR
// ================================================================
class KoiDetector {
public:
    KoiDetector(const std::string& modelPath) : env_(ORT_LOGGING_LEVEL_WARNING, "Koi") {
        Ort::SessionOptions opts;
        opts.SetIntraOpNumThreads(4);
        opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        bool cudaOk = false;
        try {
            OrtCUDAProviderOptions cuda{};
            opts.AppendExecutionProvider_CUDA(cuda);
            cudaOk = true;
        }
        catch (...) {}

        try {
#ifdef _WIN32
            std::wstring wpath(modelPath.begin(), modelPath.end());
            session_ = std::make_unique<Ort::Session>(env_, wpath.c_str(), opts);
#else
            session_ = std::make_unique<Ort::Session>(env_, modelPath.c_str(), opts);
#endif
        }
        catch (const Ort::Exception& e) {
            cudaOk = false;
            Ort::SessionOptions cpuOpts;
            cpuOpts.SetIntraOpNumThreads(4);
            cpuOpts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
#ifdef _WIN32
            std::wstring wpath2(modelPath.begin(), modelPath.end());
            session_ = std::make_unique<Ort::Session>(env_, wpath2.c_str(), cpuOpts);
#else
            session_ = std::make_unique<Ort::Session>(env_, modelPath.c_str(), cpuOpts);
#endif
        }

        Ort::AllocatorWithDefaultOptions alloc;
        inputName_ = session_->GetInputNameAllocated(0, alloc).get();
        outputName_ = session_->GetOutputNameAllocated(0, alloc).get();
        auto shape = session_->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
        modelH_ = (int)shape[2];
        modelW_ = (int)shape[3];
    }

    std::vector<Detection> detect(const cv::Mat& roi, float confThresh) {
        cv::Mat resized;
        cv::resize(roi, resized, cv::Size(modelW_, modelH_));
        resized.convertTo(resized, CV_32F, 1.0 / 255.0);
        cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);

        std::vector<cv::Mat> ch(3);
        cv::split(resized, ch);
        std::vector<float> blob;
        blob.reserve(3 * modelH_ * modelW_);
        for (auto& c : ch)
            blob.insert(blob.end(), (float*)c.datastart, (float*)c.dataend);

        std::array<int64_t, 4> inShape = { 1, 3, modelH_, modelW_ };
        Ort::MemoryInfo mem = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value ortIn = Ort::Value::CreateTensor<float>(
            mem, blob.data(), blob.size(), inShape.data(), inShape.size());

        const char* inN[] = { inputName_.c_str() };
        const char* outN[] = { outputName_.c_str() };
        auto outs = session_->Run(Ort::RunOptions{ nullptr }, inN, &ortIn, 1, outN, 1);

        float* data = outs[0].GetTensorMutableData<float>();
        auto   shape = outs[0].GetTensorTypeAndShapeInfo().GetShape();

        int numAnchors, numFields; bool transposed;
        if (shape[1] < shape[2]) {
            numFields = (int)shape[1]; numAnchors = (int)shape[2]; transposed = true;
        }
        else {
            numAnchors = (int)shape[1]; numFields = (int)shape[2]; transposed = false;
        }

        float sx = (float)roi.cols / modelW_;
        float sy = (float)roi.rows / modelH_;

        std::vector<Detection> dets;
        for (int i = 0; i < numAnchors; ++i) {
            float cx, cy, w, h, conf = 0.f;
            if (transposed) {
                cx = data[0 * numAnchors + i]; cy = data[1 * numAnchors + i];
                w = data[2 * numAnchors + i]; h = data[3 * numAnchors + i];
                for (int c = 4; c < numFields; ++c)
                    conf = std::max(conf, data[c * numAnchors + i]);
            }
            else {
                cx = data[i * numFields + 0]; cy = data[i * numFields + 1];
                w = data[i * numFields + 2]; h = data[i * numFields + 3];
                for (int c = 4; c < numFields; ++c)
                    conf = std::max(conf, data[i * numFields + c]);
            }
            if (conf < confThresh) continue;
            int x1 = std::max(0, (int)((cx - w / 2) * sx));
            int y1 = std::max(0, (int)((cy - h / 2) * sy));
            int bw = std::min((int)(w * sx), roi.cols - x1);
            int bh = std::min((int)(h * sy), roi.rows - y1);
            if (bw <= 0 || bh <= 0) continue;
            dets.push_back({ cv::Rect(x1,y1,bw,bh), conf });
        }
        return dets;
    }

private:
    Ort::Env                      env_;
    std::unique_ptr<Ort::Session> session_;
    std::string                   inputName_, outputName_;
    int                           modelW_ = 640, modelH_ = 640;
};

// ================================================================
//  DRAW HELPERS (optional — ใช้ถ้าต้องการ draw บน cv::Mat ก่อนส่ง)
// ================================================================
static cv::Scalar idColor(int id) {
    float hue = fmod(id * 137.508f, 360.f);
    float h = hue / 60.f; int i = (int)h; float f = h - i, q = 1.f - f;
    float r, g, b;
    switch (i % 6) {
    case 0: r = 1;  g = f;  b = 0;  break; case 1: r = q;  g = 1;  b = 0;  break;
    case 2: r = 0;  g = 1;  b = f;  break; case 3: r = 0;  g = q;  b = 1;  break;
    case 4: r = f;  g = 0;  b = 1;  break; default:r = 1;  g = 0;  b = q;  break;
    }
    return cv::Scalar((int)(b * 200) + 55, (int)(g * 200) + 55, (int)(r * 200) + 55);
}

inline void drawDetections(cv::Mat& frame,
    const std::vector<SmoothDetection>& dets, cv::Point roiOffset)
{
    for (const auto& d : dets) {
        int x1 = (int)d.x1 + roiOffset.x, y1 = (int)d.y1 + roiOffset.y;
        int x2 = (int)d.x2 + roiOffset.x, y2 = (int)d.y2 + roiOffset.y;
        int cx = x1 + (x2 - x1) / 2, cy = y1 + (y2 - y1) / 2;
        cv::Scalar color = idColor(d.id);
        cv::rectangle(frame, { x1 + 1,y1 + 1 }, { x2 + 1,y2 + 1 }, cv::Scalar(0, 0, 0), 2);
        cv::rectangle(frame, { x1,y1 }, { x2,y2 }, color, 2);
        cv::circle(frame, { cx,cy }, 4, cv::Scalar(0, 0, 0), cv::FILLED);
        cv::circle(frame, { cx,cy }, 3, color, cv::FILLED);

        std::ostringstream ss;
        ss << "#" << d.id << "  " << std::fixed << std::setprecision(2) << d.conf
            << "  (" << cx << "," << cy << ")";
        std::string label = ss.str();
        double fontScale = 0.40; int fontFace = cv::FONT_HERSHEY_SIMPLEX, thick = 1, bl = 0;
        cv::Size ts = cv::getTextSize(label, fontFace, fontScale, thick, &bl);
        int ly = std::max(y1 - 3, ts.height + 2);
        cv::rectangle(frame, { x1, ly - ts.height - 2 }, { x1 + ts.width + 4, ly + 2 }, color, cv::FILLED);
        double lum = 0.114 * color[0] + 0.587 * color[1] + 0.299 * color[2];
        cv::Scalar tc = (lum > 128) ? cv::Scalar(0, 0, 0) : cv::Scalar(255, 255, 255);
        cv::putText(frame, label, { x1 + 2, ly }, fontFace, fontScale, tc, thick, cv::LINE_AA);
    }
}