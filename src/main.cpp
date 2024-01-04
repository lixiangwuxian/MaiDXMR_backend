#include "bitBlt/bitBlt.h"
// #include "nvenc/NvEncoderD3D11.h"
#include <GdiPlus.h>
#include <Windows.h>
#include <d3d11.h>
#include <iostream>
#include <mutex>
#include <synchapi.h>
#include <wrl.h>

#pragma comment(lib, "GdiPlus.lib")

// HBITMAP g_hLastBitmap = NULL;
// std::mutex g_mutex;
// NvEncoderD3D11 *nvEncoder = NULL;
// Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
// Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
// Microsoft::WRL::ComPtr<IDXGIFactory1> pFactory;
// Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter;
// Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexSysMem;

// NV_ENC_INITIALIZE_PARAMS * params = NULL;


int nWidth = 585;
int nHeight = 1050;

using namespace Gdiplus;

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

ID3D11Texture2D *HbitToD3D11Texture2D(ID3D11Device *pDevice, HBITMAP hBitmap) {
  BITMAP bmp;
  GetObject(hBitmap, sizeof(BITMAP), &bmp);
  int width = bmp.bmWidth;
  int height = bmp.bmHeight;
  HDC hdc = CreateCompatibleDC(nullptr);
  SelectObject(hdc, hBitmap);

  // 准备位图数据的缓冲区
  std::vector<BYTE> Buffer(width * height * 4); // 假设是32位位图

  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height; // 顶向下位图
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  // 获取位图数据
  GetDIBits(hdc, hBitmap, 0, height, &Buffer[0], &bmi, DIB_RGB_COLORS);

  // 创建纹理
  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 32位位图通常是这种格式
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;

  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = &Buffer[0];
  initData.SysMemPitch = static_cast<UINT>(width * 4);

  ID3D11Texture2D *pTexture = nullptr;
  HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &pTexture);
  return pTexture;
}

void MyFrameCallback(HBITMAP hBitmap) {
  ID3D11Texture2D *texture =
      HbitToD3D11Texture2D((ID3D11Device *)nvEncoder->GetDevice(), hBitmap);
}

// void EncodeFrame(ID3D11Texture2D *texture) {
//  CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)pFactory.GetAddressOf());
// pFactory->EnumAdapters(0, pAdapter.GetAddressOf());
// D3D11CreateDevice(pAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, 0,
//     NULL, 0, D3D11_SDK_VERSION, pDevice.GetAddressOf(), NULL, pContext.GetAddressOf());
// DXGI_ADAPTER_DESC adapterDesc;
// pAdapter->GetDesc(&adapterDesc);
// char szDesc[80];
// wcstombs(szDesc, adapterDesc.Description, sizeof(szDesc));
// std::cout << "GPU in use: " << szDesc << std::endl;

// D3D11_TEXTURE2D_DESC desc;
// ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
// desc.Width = nWidth;
// desc.Height = nHeight;
// desc.MipLevels = 1;
// desc.ArraySize = 1;
// desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
// desc.SampleDesc.Count = 1;
// desc.Usage = D3D11_USAGE_STAGING;
// desc.BindFlags = 0;
// desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
// pDevice->CreateTexture2D(&desc, NULL, pTexSysMem.GetAddressOf());

// // std::unique_ptr<RGBToNV12ConverterD3D11> pConverter;

// NvEncoderD3D11 enc(pDevice.Get(), nWidth, nHeight,  NV_ENC_BUFFER_FORMAT_ARGB);

// NV_ENC_INITIALIZE_PARAMS initializeParams = { NV_ENC_INITIALIZE_PARAMS_VER };
// NV_ENC_CONFIG encodeConfig = { NV_ENC_CONFIG_VER };
// initializeParams.encodeConfig = &encodeConfig;
// enc.CreateDefaultEncoderParams(&initializeParams, pEncodeCLIOptions->GetEncodeGUID(), pEncodeCLIOptions->GetPresetGUID());

// pEncodeCLIOptions->SetInitParams(&initializeParams,  NV_ENC_BUFFER_FORMAT_ARGB);

// enc.CreateEncoder(&initializeParams);

// // std::ifstream fpBgra(szBgraFilePath, std::ifstream::in | std::ifstream::binary);
// // if (!fpBgra)
// // {
// //     std::ostringstream err;
// //     err << "Unable to open input file: " << szBgraFilePath << std::endl;
// //     throw std::invalid_argument(err.str());
// // }

// // std::ofstream fpOut(szOutFilePath, std::ios::out | std::ios::binary);
// // if (!fpOut)
// // {
// //     std::ostringstream err;
// //     err << "Unable to open output file: " << szOutFilePath << std::endl;
// //     throw std::invalid_argument(err.str());
// // }

// int nSize = nWidth * nHeight * 4;
// std::unique_ptr<uint8_t[]> pHostFrame(new uint8_t[nSize]);
// int nFrame = 0;
//   nvEncoder->EncodeFrame(vPacket);
// }

// NV_ENC_INITIALIZE_PARAMS *SetNVEncParams(int width, int height) {
//   NV_ENC_INITIALIZE_PARAMS *pEncodeParams = new NV_ENC_INITIALIZE_PARAMS();
//   pEncodeParams->encodeWidth = width;
//   pEncodeParams->encodeHeight = height;
//   pEncodeParams->darWidth = width;
//   pEncodeParams->darHeight = height;
//   pEncodeParams->frameRateNum = 120;
//   pEncodeParams->frameRateDen = 1;
//   pEncodeParams->enablePTD = 1;
//   pEncodeParams->reportSliceOffsets = 0;
//   pEncodeParams->enableSubFrameWrite = 0;
//   pEncodeParams->enableExternalMEHints = 0;
//   pEncodeParams->enableMEOnlyMode = 0;
//   pEncodeParams->enableWeightedPrediction = 0;
//   pEncodeParams->encodeGUID = NV_ENC_CODEC_H264_GUID;
//   return pEncodeParams;
// }

int main() {
  params = SetNVEncParams(585, 1050);
  AllocConsole();
  CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)pFactory.GetAddressOf());
  pFactory->EnumAdapters(0, pAdapter.GetAddressOf());
  D3D11CreateDevice(pAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, NULL, 0,
                    D3D11_SDK_VERSION, pDevice.GetAddressOf(), NULL,
                    pContext.GetAddressOf());
  DXGI_ADAPTER_DESC adapterDesc;
  pAdapter->GetDesc(&adapterDesc);
  char szDesc[80];
  wcstombs(szDesc, adapterDesc.Description, sizeof(szDesc));
  std::cout << "GPU in use: " << szDesc << std::endl;

  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
  desc.Width = 585;
  desc.Height = 1050;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_STAGING;
  desc.BindFlags = 0;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  pDevice->CreateTexture2D(&desc, NULL, pTexSysMem.GetAddressOf());
  nvEncoder = new NvEncoderD3D11(*pDevice.GetAddressOf(), 585, 1050,
                                 NV_ENC_BUFFER_FORMAT_NV12);
  nvEncoder->CreateEncoder(SetNVEncParams(585, 1050));
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