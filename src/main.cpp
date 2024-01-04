#include "bitBlt/bitBlt.h"
#include <GdiPlus.h>
#include <Windows.h>
#include <d3d11.h>
#include <iostream>
#include <mutex>
#include <synchapi.h>
#include <wrl.h>

#pragma comment(lib, "GdiPlus.lib")




int nWidth = 585;
int nHeight = 1050;

using namespace Gdiplus;

void MyFrameCallback(HBITMAP hBitmap) {
}


int main() {
  AllocConsole();
  DesktopCapturer capturer;
  while (!capturer.Initialize("Sinmai", MyFrameCallback)) {
    printf("Failed to initialize DesktopCapturer\n");
    return 0;
  }
  while (1) {
    capturer.StartCapture();
    Sleep(8);
  }
  return 0;
}