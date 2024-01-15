#include "network/udpClient.h"
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
//192.168.6.197//v50
//192.168.6.109//quest
UDPClient *client= new UDPClient("192.168.6.109",10890);

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
}

void MyUdpSend(const std::vector<uint8_t> &data) {
  client->send(data);
}

int nv_enc_main() {
  try {
    NvEncoderInitParam encodeCLIOptions("",NULL,false);
    int iGpu = 0;
    bool bForceNv12 = false;
    sc=new SimpleEncoder( nWidth, nHeight, &encodeCLIOptions, iGpu,
              bForceNv12,MyUdpSend);
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
  // int FramesToCapture = 1200;
  while (true) {
    capturer.StartCapture();
    Sleep(1000/120);
    // FramesToCapture--;
  }
  sc->EndCompress();
  // nv_enc_thread.join();
  return 0;
}