# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

C++/CLI Windows Forms application ที่ detect และ track ปลาคาร์ฟ real-time ด้วย YOLOv8 ONNX model ผ่าน ONNX Runtime + OpenCV

- **Python prototype**: `program/koi-detect.ipynb` (Ultralytics YOLOv8, GPU, `device=0, half=True`)
- **C++ Window Form**: `Koi Fish Motion and Identity Detection/` (ONNX Runtime + OpenCV + C++/CLI)

## Build

เปิดด้วย Visual Studio → Build Configuration: **Release | x64**

```
Solution: Koi Fish Motion and Identity Detection.slnx
Project:  Koi Fish Motion and Identity Detection/Koi Fish Motion and Identity Detection.vcxproj
```

Post-Build Event copy อัตโนมัติ:
- `best50.onnx` → `x64/Release/`
- `C:\libs\onnxruntime\lib\*.dll` → `x64/Release/`

**DLL ที่ต้อง copy ด้วยตนเอง** ไปที่ `x64/Release/` (ไม่อยู่ใน post-build):
- CUDA Runtime: `C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.6\bin\cudart64_12.dll`, `cublas64_12.dll`, `cublasLt64_12.dll`
- cuDNN: `C:\Program Files\NVIDIA\CUDNN\v9.x\bin\12.9\x64\cudnn*.dll`

## External Dependencies (ติดตั้งใน C:\libs\)

| Library | Path | Version |
|---|---|---|
| OpenCV | `C:\libs\opencv\build\` | 4.12.0 |
| ONNX Runtime GPU | `C:\libs\onnxruntime\` | 1.24.3 (gpu, CUDA 12) |

ต้องใช้ **ONNX Runtime GPU** (`onnxruntime-win-x64-gpu-1.24.3.zip`) ไม่ใช่ CPU-only และไม่ใช่ `gpu_cuda13`

## Architecture

### Data Flow
```
VideoCapture → applyGamma → [every N frames] KoiDetector.detect() → NativeKoiTracker.update()
    → drawDetections() → MatHelper::MatToBitmap() → BeginInvoke → PictureBox
    → UpdateFishes() → BeginInvoke → DataGridView / Stats / DetailForm
```

### Key Files

| File | Role |
|---|---|
| `KoiDetectCore.h` | Native C++: YOLOv8 ONNX inference (`KoiDetector`), multi-object tracker (`NativeKoiTracker`), config constants (`Config::`) |
| `DetectionService.h/.cpp` | C++/CLI wrapper: background thread loop, fires managed events (`OnFrameBitmap`, `OnFrameUpdated`, `OnError`) |
| `Mainform.h/.cpp` | UI: wires events, renders PictureBox overlay (trajectories), DataGridView |
| `FishTrack.h` | Managed data class: `FishTrack^`, `FishStatus`, `KoiSpecies` |
| `MatHelper.h` | cv::Mat → System::Drawing::Bitmap^ conversion (row-by-row memcpy) |
| `FishDetailForm.h/.cpp` | Detail popup เมื่อ click ปลาหรือ double-click grid |

### Threading
- **UI thread**: Form, PictureBox, DataGridView, Timer
- **Background thread**: `DetectionService::InferLoop()` — OpenCV + ONNX inference
- Cross-thread calls ใช้ `BeginInvoke` (async) สำหรับ frame updates, `Invoke` (sync) ใช้เฉพาะ error/log

### Config Constants (`KoiDetectCore.h` → `namespace Config`)

```cpp
CONF_THRESH    = 0.7f   // confidence threshold
PROCESS_EVERY  = 2      // detect ทุก N เฟรม (Python ใช้ 2)
MAX_BOX_W/H    = 80     // กรอง bounding box ที่ใหญ่เกิน
GAMMA          = 0.50f  // gamma correction
```

## GPU Setup (สำคัญมาก)

RTX 50-series (Blackwell) ครั้งแรกที่รัน CUDA จะ JIT compile kernels นาน **5-15 นาที** — ห้ามปิดโปรแกรม ครั้งต่อไปเร็วทันทีเพราะ cache

CUDA Provider ใน `KoiDetector` constructor ถ้า fail จะ silent fallback ไป CPU → inference ~600ms/frame

ตรวจสอบว่า GPU ทำงาน: ดู `timing_log.txt` ข้าง exe
- CPU: ~600ms/frame
- GPU: ~5-15ms/frame

## Model

`best50.onnx` — YOLOv8 trained บน koi fish dataset, input 640×640, single class
ต้องอยู่ข้าง `.exe` ใน `x64/Release/`
