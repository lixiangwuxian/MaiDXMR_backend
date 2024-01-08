#include "bitBlt/bitBlt.h"
#include "nvenc/AppEncD3D11/AppEncD3D11.h"
#include "nvenc/AppEncD3D11/NvEncoder/NvEncoderD3D11.h"
#include "nvenc/AppEncD3D11/NvEncoder/nvEncodeAPI.h"
#include "nvenc/Common/AppEncUtils.h"
#include "nvenc/Utils/NvCodecUtils.h"
#include "nvenc/Utils/NvEncoderCLIOptions.h"
#include <GdiPlus.h>
#include <Windows.h>
#include <d3d11.h>
#include <iostream>
#include <mutex>
#include <synchapi.h>
#include <wingdi.h>
#include <wrl.h>
#include <wrl/client.h>
#include <thread>

#define WIN32


#pragma comment(lib, "GdiPlus.lib")

int nWidth = 585;
int nHeight = 1050;

SimpleEncoder *sc;
using namespace Gdiplus;

void SaveHBITMAPToFile(HBITMAP hBitmap, LPCWSTR filename) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    Bitmap* bitmap = Bitmap::FromHBITMAP(hBitmap, NULL);
    CLSID bmpClsid;
    CLSIDFromString(L"{557CF400-1A04-11D3-9A73-0000F81EF32E}", &bmpClsid);
    bitmap->Save(filename, &bmpClsid, NULL);
    delete bitmap;
    GdiplusShutdown(gdiplusToken);
}

void MyFrameCallback(HBITMAP hBitmap) {
  sc->ConvertHBitmapToTexture(hBitmap);
  // SaveHBITMAPToFile(hBitmap, L"test.bmp");
}



/**
 *  This sample application illustrates encoding of frames in ID3D11Texture2D
 * textures. There are 2 modes of operation demonstrated in this application. In
 * the default mode application reads RGB data from file and copies it to D3D11
 * textures obtained from the encoder using NvEncoder::GetNextInputFrame() and
 * the RGB texture is submitted to NVENC for encoding. In the second case
 * ("-nv12" option) the application converts RGB textures to NV12 textures using
 * DXVA's VideoProcessBlt API call and the NV12 texture is submitted for
 * encoding.
 */
int nv_enc_main() {
  char szOutFilePath[256] = "out.h264";
  try {
    NvEncoderInitParam encodeCLIOptions;
    int iGpu = 0;
    bool bForceNv12 = false;
    sc=new SimpleEncoder( nWidth, nHeight, &encodeCLIOptions, iGpu,
              bForceNv12);
  } catch (const std::exception &ex) {
    std::cout << ex.what();
    exit(1);
  }
  return 0;
}




int main() {
  AllocConsole();
  DesktopCapturer capturer;
  while (!capturer.Initialize("Sinmai", MyFrameCallback)) {
    printf("Failed to initialize DesktopCapturer\n");
    return 0;
  }
  // std::thread nv_enc_thread(nv_enc_main);
  // nv_enc_thread.detach();
  nv_enc_main();
  int FramesToCapture = 1200;
  while (FramesToCapture>0) {
    capturer.StartCapture();
    Sleep(4);
    FramesToCapture--;
  }
  sc->EndCompress();
  // nv_enc_thread.join();
  return 0;
}