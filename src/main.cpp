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
// adb logcat -s Unity
#define ALVR_DEBUG_LOG

#define WIN32

#define OUTPUT_TO_FILE

#pragma comment(lib, "GdiPlus.lib")
using namespace Gdiplus;

class MainMethod {
public:
  MainMethod(const std::string &ip) {
    setEncoderParam();
    p_capturer = std::unique_ptr<DesktopCapturer>();
    p_capturer.reset(new DesktopCapturer());
    Shape *shape = p_capturer->Initialize("Sinmai");
    if (shape != nullptr) {
      nHeight = shape->height;
      nWidth = shape->width;
      delete shape;
    }
    p_d3dRender = std::make_shared<CD3DRender>();
    p_d3dRender.reset(new CD3DRender());
    p_d3dRender->Initialize(nWidth, nHeight);
    pDevice = p_d3dRender->GetDevice();
    pContext = p_d3dRender->GetContext();
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
    desc.Width = nWidth;
    desc.Height = nHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // todo
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pDevice->CreateTexture2D(&desc, NULL, pTexSysMem.GetAddressOf());
    p_enc = std::unique_ptr<VideoEncoderNVENC>(
        new VideoEncoderNVENC(p_d3dRender, nWidth, nHeight));
    p_enc->Initialize();
    p_udpClient = std::unique_ptr<UDPClient>(new UDPClient(ip, 10890));
#ifdef OUTPUT_TO_FILE
    fpOut.open("output.h264", std::ios::binary);
#endif
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
      Sleep(1000 / 50);
    }
  }

private:
  void ConvertHBitmapToTexture(HBITMAP hBitmap) {
    // SaveHBITMAPToFile(hBitmap, L"test.bmp");
    int nSize = nWidth * nHeight * 4;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    Gdiplus::Bitmap *bitmap = Gdiplus::Bitmap::FromHBITMAP(hBitmap, NULL);
    Gdiplus::BitmapData bitmapData;
    Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
    bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB,
                     &bitmapData);
    D3D11_MAPPED_SUBRESOURCE map;
    pContext->Map(pTexSysMem.Get(), D3D11CalcSubresource(0, 0, 1),
                  D3D11_MAP_WRITE, 0, &map);
    for (int y = 0; y < nHeight; y++) {
      memcpy((uint8_t *)map.pData + map.RowPitch * y,
             (uint8_t *)bitmapData.Scan0 + y * nWidth * 4, nWidth * 4);
    }
    pContext->Unmap(pTexSysMem.Get(), D3D11CalcSubresource(0, 0, 1));
    std::vector<std::vector<uint8_t>> vPacket =
        p_enc->Transmit(*pTexSysMem.GetAddressOf(), 0, 0, nFrame % 2400 == 0,
                        nFrame % 500 == 0, m_params);
    nFrame += (int)vPacket.size();
    for (std::vector<uint8_t> &packet : vPacket) {
#ifdef OUTPUT_TO_FILE
      fpOut.write(reinterpret_cast<char *>(packet.data()), packet.size());
#endif
      p_udpClient->send(packet);
      std::cout << "packet.size():" << packet.size() << std::endl;
    }
    // pContext->Release();
    bitmap->UnlockBits(&bitmapData);
    delete bitmap;
    Gdiplus::GdiplusShutdown(gdiplusToken);
  }

  void setEncoderParam() {
    Settings::Instance().m_codec = ALVR_CODEC_H264;
    Settings::Instance().m_refreshRate = 120;
    Settings::Instance().m_use10bitEncoder = false;
    Settings::Instance().m_nvencQualityPreset = 1;
    Settings::Instance().m_rateControlMode = ALVR_CBR;
    Settings::Instance().m_fillerData = true;
    Settings::Instance().m_entropyCoding = ALVR_CAVLC;
    Settings::Instance().m_nvencRefreshRate = -1;
    Settings::Instance().m_nvencMaxNumRefFrames = -1;
    Settings::Instance().m_nvencGopLength = -1;
    Settings::Instance().m_nvencIntraRefreshPeriod = -1;
    Settings::Instance().m_nvencIntraRefreshCount = -1;
    Settings::Instance().m_nvencPFrameStrategy = -1;
    Settings::Instance().m_nvencMultiPass = NV_ENC_MULTI_PASS_DISABLED;
    Settings::Instance().m_nvencLowDelayKeyFrameScale = -1;
    Settings::Instance().m_nvencAdaptiveQuantizationMode = SpatialAQ;
    Settings::Instance().m_nvencRateControlMode = -1;
    Settings::Instance().m_nvencRcInitialDelay = -1;
    Settings::Instance().m_nvencRcMaxBitrate = -1;
    Settings::Instance().m_nvencRcAverageBitrate = -1;
    Settings::Instance().m_nvencEnableIntraRefresh = true;
    Settings::Instance().m_nvencTuningPreset = NV_ENC_TUNING_INFO_LOW_LATENCY;
    Settings::Instance().m_nvencEnableWeightedPrediction = false;
    m_params.bitrate_bps = 30000000;
    m_params.framerate = 120;
    m_params.updated = 0;
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
  std::ofstream fpOut;
  void static SaveHBITMAPToFile(HBITMAP hBitmap, LPCWSTR filename) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    Bitmap *bitmap = Bitmap::FromHBITMAP(hBitmap, NULL);
    CLSID bmpClsid;
    CLSIDFromString(L"{557CF400-1A04-11D3-9A73-0000F81EF32E}", &bmpClsid);
    bitmap->Save(filename, &bmpClsid, NULL);
    delete bitmap;
    GdiplusShutdown(gdiplusToken);
  }
};

int main() {
  MainMethod m("192.168.36.179");
  m.Start();
  return 0;
}