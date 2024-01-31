#include <Windows.h>
#include <string>

struct Shape {
  int width;
  int height;
};

class DesktopCapturer {
public:
  DesktopCapturer()
      : hWindow(nullptr), hdcWindow(nullptr), hdcMemory(nullptr),
        hBitmap(nullptr) {}

  // 初始化，根据窗口标题找到窗口并获取句柄
  Shape *Initialize(const std::string &windowTitle) {
    hWindow = FindWindow(nullptr, windowTitle.c_str());
    if (!hWindow) {
      return nullptr;
    }

    // 获取窗口和内存的设备上下文
    hdcWindow = GetDC(hWindow);
    hdcMemory = CreateCompatibleDC(hdcWindow);

    RECT rc;
    GetClientRect(hWindow, &rc);
    int width = (rc.right - rc.left) / 2;
    int height = rc.bottom - rc.top;

    return new Shape{width, height};
  }

  // 开始捕获
  HBITMAP StartCapture() {
    if (!hWindow || !hdcWindow || !hdcMemory) {
      return nullptr;
    }

    RECT rc;
    GetClientRect(hWindow, &rc);
    int width = (rc.right - rc.left) / 2;
    int height = rc.bottom - rc.top;

    // 创建兼容的位图
    hBitmap = CreateCompatibleBitmap(hdcWindow, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMemory, hBitmap);

    // 捕获指定窗口的内容
    if (BitBlt(hdcMemory, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY)) {
    }

    // // 清理
    SelectObject(hdcMemory, hOldBitmap);
    return hBitmap;
  }

  // 清理资源
  ~DesktopCapturer() {
    if (hdcMemory) {
      DeleteDC(hdcMemory);
    }

    if (hdcWindow) {
      ReleaseDC(hWindow, hdcWindow);
    }
  }

private:
  HWND hWindow;    // 窗口句柄
  HDC hdcWindow;   // 窗口的设备上下文
  HDC hdcMemory;   // 内存设备上下文，用于绘制位图
  HBITMAP hBitmap; // 捕获到的位图
};
