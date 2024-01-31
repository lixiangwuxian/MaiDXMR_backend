#include "network/udpClient.h"
// clang-format is bad..
#include "bitBlt/bitBlt.h"
#include "nvenc/win32/VideoEncoderNVENC.h"
#include <GdiPlus.h>
#include <Windows.h>
#include <d3d11.h>
#include <iostream>
#include <wrl.h>
// #include <wrl/client.h>

#define WIN32

#pragma comment(lib, "GdiPlus.lib")
using namespace Gdiplus;

class MainMethod {
public:
  MainMethod(const std::string &ip) {
    p_d3dRender = std::make_shared<CD3DRender>();
    pDevice = p_d3dRender->GetDevice();
    pContext = p_d3dRender->GetContext();
    Shape *shape = p_capturer->Initialize("Sinmai");
    if (shape != nullptr) {
      nHeight = shape->height;
      nWidth = shape->width;
      delete shape;
    }
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
    desc.Width = nWidth;
    desc.Height = nHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pDevice->CreateTexture2D(&desc, NULL, pTexSysMem.GetAddressOf());
    p_capturer = std::unique_ptr<DesktopCapturer>();
    p_enc = std::unique_ptr<VideoEncoderNVENC>(
        new VideoEncoderNVENC(p_d3dRender, nWidth, nHeight));
    p_udpClient = std::unique_ptr<UDPClient>(new UDPClient(ip, 10890));
  }
  ~MainMethod() {
    p_enc->Shutdown();
    p_capturer.release();
    p_d3dRender.reset();
  }
  void Start() {
    while (true) {
      ConvertHBitmapToTexture(p_capturer->StartCapture());
      nFrame++;
      Sleep(1000 / 120);
    }
  }

private:
  void ConvertHBitmapToTexture(HBITMAP hBitmap) {
    int nSize = nWidth * nHeight * 4;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    Gdiplus::Bitmap *bitmap = Gdiplus::Bitmap::FromHBITMAP(hBitmap, NULL);
    Gdiplus::BitmapData bitmapData;
    Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
    bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB,
                     &bitmapData);
    pDevice->GetImmediateContext(&pContext);
    D3D11_MAPPED_SUBRESOURCE map;
    pContext->Map(pTexSysMem.Get(), D3D11CalcSubresource(0, 0, 1),
                  D3D11_MAP_WRITE, 0, &map);
    for (int y = 0; y < nHeight; y++) {
      memcpy((uint8_t *)map.pData + y * map.RowPitch,
             (uint8_t *)bitmapData.Scan0 + y * nWidth * 4, nWidth * 4);
    }
    pContext->Unmap(pTexSysMem.Get(), D3D11CalcSubresource(0, 0, 1));
    std::vector<std::vector<uint8_t>> vPacket = p_enc->Transmit(
        *pTexSysMem.GetAddressOf(), 0, 0, nFrame % 240 == 0, m_params);
    nFrame += (int)vPacket.size();
    for (std::vector<uint8_t> &packet : vPacket) {
      // fpOut.write(reinterpret_cast<char *>(packet.data()), packet.size());
      p_udpClient->send(packet);
      std::cout << "packet.size():" << packet.size() << std::endl;
    }
    pContext->Release();
    bitmap->UnlockBits(&bitmapData);
    delete bitmap;
    delete hBitmap;
    Gdiplus::GdiplusShutdown(gdiplusToken);
  }

private:
  std::unique_ptr<UDPClient> p_udpClient;
  std::string serverIp;

  std::unique_ptr<VideoEncoderNVENC> p_enc;
  std::shared_ptr<CD3DRender> p_d3dRender;
  std::unique_ptr<DesktopCapturer> p_capturer;
  Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexSysMem;
  FfiDynamicEncoderParams m_params;
  int nWidth = 0;
  int nHeight = 0;
  int nFrame = 0;
};

// int nWidth = 585;
// int nHeight = 1050;

// SimpleEncoder *sc;

// 192.168.6.197//v50
// 192.168.6.109//quest
// UDPClient *client = new UDPClient("192.168.6.109", 10890);

// void SaveHBITMAPToFile(HBITMAP hBitmap, LPCWSTR filename) {
//     GdiplusStartupInput gdiplusStartupInput;
//     ULONG_PTR gdiplusToken;
//     GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
//     Bitmap* bitmap = Bitmap::FromHBITMAP(hBitmap, NULL);
//     CLSID bmpClsid;
//     CLSIDFromString(L"{557CF400-1A04-11D3-9A73-0000F81EF32E}", &bmpClsid);
//     bitmap->Save(filename, &bmpClsid, NULL);
//     delete bitmap;
//     GdiplusShutdown(gdiplusToken);
// }

int main() {
  // AllocConsole();
  // DesktopCapturer capturer;
  // while (!capturer.Initialize("Sinmai", MyFrameCallback)) {
  //   printf("Failed to initialize DesktopCapturer\n");
  //   Sleep(5000);
  // }
  // try {
  //   NvEncoderInitParam encodeCLIOptions("", NULL, false);
  //   int iGpu = 0;
  //   bool bForceNv12 = false;
  //   sc = new SimpleEncoder(nWidth, nHeight, &encodeCLIOptions, iGpu,
  //   bForceNv12,
  //                          MyUdpSend);
  // } catch (const std::exception &ex) {
  //   std::cout << ex.what();
  //   exit(1);
  // }
  // while (true) {
  //   capturer.StartCapture();
  //   Sleep(1000 / 120);
  // }
  // sc->EndCompress();
  MainMethod m("192.168.6.197");
  m.Start();
  return 0;
}