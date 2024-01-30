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

void MyFrameCallback(HBITMAP hBitmap) {
  sc->ConvertHBitmapToTexture(hBitmap);
}

void MyUdpSend(const std::vector<uint8_t> &data) {
  client->send(data);
}

// void ConvertHBitmapToTexture(HBITMAP hBitmap) {

// //creat device
//   ComPtr<ID3D11Device> pDevice;
//   ComPtr<ID3D11DeviceContext> pContext;
//   ComPtr<IDXGIFactory1> pFactory;
//   ComPtr<IDXGIAdapter> pAdapter;
//   ComPtr<ID3D11Texture2D> pTexSysMem;
//   int iGpu=0;


//     CreateDXGIFactory1(__uuidof(IDXGIFactory1),
//                        (void **)pFactory.GetAddressOf());
//     pFactory->EnumAdapters(iGpu, pAdapter.GetAddressOf());
//     D3D11CreateDevice(pAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, NULL, 0,
//                       D3D11_SDK_VERSION, pDevice.GetAddressOf(), NULL,
//                       pContext.GetAddressOf());


//     int nSize = nWidth * nHeight * 4;
//     std::cout << "ConvertHBitmapToTexture" << std::endl;
//     Gdiplus::GdiplusStartupInput gdiplusStartupInput;
//     ULONG_PTR gdiplusToken;
//     GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
//     Gdiplus::Bitmap *bitmap = Gdiplus::Bitmap::FromHBITMAP(hBitmap, NULL);
//     Gdiplus::BitmapData bitmapData;
//     Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
//     bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB,
//                      &bitmapData);
//     D3D11_TEXTURE2D_DESC desc;
//     desc.Width = nWidth;
//     desc.Height = nHeight;
//     desc.MipLevels = 1;
//     desc.ArraySize = 1;
//     desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//     desc.SampleDesc.Count = 1;
//     desc.SampleDesc.Quality = 0;
//     desc.Usage = D3D11_USAGE_DEFAULT;
//     desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
//     desc.CPUAccessFlags = 0;
//     desc.MiscFlags = 0;
//     // ID3D11DeviceContext *pContext = nullptr;
//     pDevice->GetImmediateContext(&pContext);
//     const NvEncInputFrame *encoderInputFrame = p_enc->GetNextInputFrame();
//     D3D11_MAPPED_SUBRESOURCE map;
//     pContext->Map(pTexSysMem.Get(), D3D11CalcSubresource(0, 0, 1), D3D11_MAP_WRITE, 0, &map);
//     for (int y = 0; y < nHeight; y++)
//     {
//         memcpy((uint8_t *)map.pData + y * map.RowPitch, (uint8_t *)bitmapData.Scan0 + y * nWidth * 4, nWidth * 4);
//     }
//     pContext->Unmap(pTexSysMem.Get(), D3D11CalcSubresource(0, 0, 1));
//         ID3D11Texture2D *pTexBgra = reinterpret_cast<ID3D11Texture2D*>(encoderInputFrame->inputPtr);
//         pContext->CopyResource(pTexBgra, pTexSysMem.Get());
//     p_enc->EncodeFrame(vPacket);
//     nFrame += (int)vPacket.size();
//     for (std::vector<uint8_t> &packet : vPacket) {
//       // fpOut.write(reinterpret_cast<char *>(packet.data()), packet.size());
//       udpSendCallBack(packet);
//       if (nFrame==1){
//         udpSendCallBack(packet);
//       }
//       std::cout << "packet.size():" << packet.size() << std::endl;
//       totalSize += packet.size();
//     }
//     pContext->Release();
//     bitmap->UnlockBits(&bitmapData);
//     delete bitmap;
//     Gdiplus::GdiplusShutdown(gdiplusToken);
//   }

int main() {
  AllocConsole();
  DesktopCapturer capturer;
  while (!capturer.Initialize("Sinmai", MyFrameCallback)) {
    printf("Failed to initialize DesktopCapturer\n");
    Sleep(5000);
  }
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
  while (true) {
    capturer.StartCapture();
    Sleep(1000/120);
  }
  sc->EndCompress();
  return 0;
}