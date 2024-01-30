/*
 * Copyright 2017-2018 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#include "../Common/AppEncUtils.h"
#include "../Utils/Logger.h"
#include "../Utils/NvCodecUtils.h"
#include "NvEncoder/NvEncoderD3D11.h"
#include <GdiPlus.h>
#include <Gdipluspixelformats.h>
#include <d3d11.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <synchapi.h>
#include <unordered_map>
#include <wingdi.h>
#include <wrl.h>


using Microsoft::WRL::ComPtr;

simplelogger::Logger *logger =
    simplelogger::LoggerFactory::CreateConsoleLogger();

typedef void (*UdpSendCallBack)(const std::vector<uint8_t> &);
class RGBToNV12ConverterD3D11 {
public:
  RGBToNV12ConverterD3D11(ID3D11Device *pDevice, ID3D11DeviceContext *pContext,
                          int nWidth, int nHeight)
      : pD3D11Device(pDevice), pD3D11Context(pContext) {
    pD3D11Device->AddRef();
    pD3D11Context->AddRef();

    pTexBgra = NULL;
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
    desc.Width = nWidth;
    desc.Height = nHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    desc.CPUAccessFlags = 0;
    ck(pDevice->CreateTexture2D(&desc, NULL, &pTexBgra));

    ck(pDevice->QueryInterface(__uuidof(ID3D11VideoDevice),
                               (void **)&pVideoDevice));
    ck(pContext->QueryInterface(__uuidof(ID3D11VideoContext),
                                (void **)&pVideoContext));

    D3D11_VIDEO_PROCESSOR_CONTENT_DESC contentDesc = {
        D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE,
        {1, 1},
        desc.Width,
        desc.Height,
        {1, 1},
        desc.Width,
        desc.Height,
        D3D11_VIDEO_USAGE_PLAYBACK_NORMAL};
    ck(pVideoDevice->CreateVideoProcessorEnumerator(
        &contentDesc, &pVideoProcessorEnumerator));

    ck(pVideoDevice->CreateVideoProcessor(pVideoProcessorEnumerator, 0,
                                          &pVideoProcessor));
    D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputViewDesc = {
        0, D3D11_VPIV_DIMENSION_TEXTURE2D, {0, 0}};
    ck(pVideoDevice->CreateVideoProcessorInputView(
        pTexBgra, pVideoProcessorEnumerator, &inputViewDesc, &pInputView));
  }

  ~RGBToNV12ConverterD3D11() {
    for (auto &it : outputViewMap) {
      ID3D11VideoProcessorOutputView *pOutputView = it.second;
      pOutputView->Release();
    }

    pInputView->Release();
    pVideoProcessorEnumerator->Release();
    pVideoProcessor->Release();
    pVideoContext->Release();
    pVideoDevice->Release();
    pTexBgra->Release();
    pD3D11Context->Release();
    pD3D11Device->Release();
  }
  void ConvertRGBToNV12(ID3D11Texture2D *pRGBSrcTexture,
                        ID3D11Texture2D *pDestTexture) {
    pD3D11Context->CopyResource(pTexBgra, pRGBSrcTexture);
    ID3D11VideoProcessorOutputView *pOutputView = nullptr;
    auto it = outputViewMap.find(pDestTexture);
    if (it == outputViewMap.end()) {
      D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC outputViewDesc = {
          D3D11_VPOV_DIMENSION_TEXTURE2D};
      ck(pVideoDevice->CreateVideoProcessorOutputView(
          pDestTexture, pVideoProcessorEnumerator, &outputViewDesc,
          &pOutputView));
      outputViewMap.insert({pDestTexture, pOutputView});
    } else {
      pOutputView = it->second;
    }

    D3D11_VIDEO_PROCESSOR_STREAM stream = {TRUE, 0,    0,          0,
                                           0,    NULL, pInputView, NULL};
    ck(pVideoContext->VideoProcessorBlt(pVideoProcessor, pOutputView, 0, 1,
                                        &stream));
    return;
  }

private:
  ID3D11Device *pD3D11Device = NULL;
  ID3D11DeviceContext *pD3D11Context = NULL;
  ID3D11VideoDevice *pVideoDevice = NULL;
  ID3D11VideoContext *pVideoContext = NULL;
  ID3D11VideoProcessor *pVideoProcessor = NULL;
  ID3D11VideoProcessorInputView *pInputView = NULL;
  ID3D11VideoProcessorOutputView *pOutputView = NULL;
  ID3D11Texture2D *pTexBgra = NULL;
  ID3D11VideoProcessorEnumerator *pVideoProcessorEnumerator = nullptr;
  std::unordered_map<ID3D11Texture2D *, ID3D11VideoProcessorOutputView *>
      outputViewMap;
};

class SimpleEncoder {
public:
  ComPtr<ID3D11Device> pDevice;
  // ComPtr<ID3D11DeviceContext> pContext;
  ComPtr<IDXGIFactory1> pFactory;
  ComPtr<IDXGIAdapter> pAdapter;
  ComPtr<ID3D11Texture2D> pTexSysMem;
  std::string szOutFilePath = "out.h264";
  ID3D11Texture2D *pSourceTex = NULL;
  NvEncoderD3D11 *p_enc;
  std::mutex mutex;
  UdpSendCallBack udpSendCallBack;
  // Gdiplus::Bitmap *bitmap;
  // Gdiplus::BitmapData bitmapData;
  int totalSize = 0;
  bool endCompress = false;
  // std::vector<std::vector<uint8_t>> vPacket;
  int nFrame = 0;
  std::ofstream fpOut;
  int nWidth;
  int nHeight;
  SimpleEncoder(int nWidth, int nHeight, NvEncoderInitParam *pEncodeCLIOptions,
                int iGpu, bool bForceNv12, UdpSendCallBack udpSendCallBack) {
    this->nWidth = nWidth;
    this->nHeight = nHeight;
    this->udpSendCallBack = udpSendCallBack;
    CreateDXGIFactory1(__uuidof(IDXGIFactory1),
                       (void **)pFactory.GetAddressOf());
    pFactory->EnumAdapters(iGpu, pAdapter.GetAddressOf());

    ID3D11DeviceContext *pContext = nullptr;
    D3D11CreateDevice(pAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, NULL, 0,
                      D3D11_SDK_VERSION, pDevice.GetAddressOf(), NULL,
                      &pContext);

    DXGI_ADAPTER_DESC adapterDesc;
    pAdapter->GetDesc(&adapterDesc);
    char szDesc[80];
    wcstombs(szDesc, adapterDesc.Description, sizeof(szDesc));
    std::cout << "GPU in use: " << szDesc << std::endl;
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
    ck(pDevice->CreateTexture2D(&desc, NULL, pTexSysMem.GetAddressOf()));
    fpOut = std::ofstream(szOutFilePath, std::ios::out | std::ios::binary);

    p_enc = new NvEncoderD3D11(pDevice.Get(), nWidth, nHeight,
                               bForceNv12 ? NV_ENC_BUFFER_FORMAT_NV12
                                          : NV_ENC_BUFFER_FORMAT_ARGB);

    NV_ENC_INITIALIZE_PARAMS initializeParams = {NV_ENC_INITIALIZE_PARAMS_VER};
    NV_ENC_CONFIG encodeConfig = {NV_ENC_CONFIG_VER};
    initializeParams.encodeConfig = &encodeConfig;
    p_enc->CreateDefaultEncoderParams(&initializeParams,
                                      pEncodeCLIOptions->GetEncodeGUID(),
                                      pEncodeCLIOptions->GetPresetGUID());

    pEncodeCLIOptions->SetInitParams(&initializeParams,
                                     bForceNv12 ? NV_ENC_BUFFER_FORMAT_NV12
                                                : NV_ENC_BUFFER_FORMAT_ARGB);

    p_enc->CreateEncoder(&initializeParams);
    if (!fpOut) {
      std::ostringstream err;
      err << "Unable to open output file: " << szOutFilePath << std::endl;
      throw std::invalid_argument(err.str());
    }
  }

  void EndCompress() {
    pSourceTex = NULL;
    endCompress = true;

    fpOut.close();
    p_enc->DestroyEncoder();
    std::cout << "Total frames encoded: " << nFrame << std::endl
              << "Total bytes output: " << totalSize << std::endl
              << "Saved in file " << szOutFilePath << std::endl;
  }
  void ConvertHBitmapToTexture(HBITMAP hBitmap) {

    std::vector<std::vector<uint8_t>> vPacket;
    int nSize = nWidth * nHeight * 4;
    std::cout << "ConvertHBitmapToTexture" << std::endl;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    Gdiplus::Bitmap *bitmap = Gdiplus::Bitmap::FromHBITMAP(hBitmap, NULL);
    Gdiplus::BitmapData bitmapData;
    Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
    bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB,
                     &bitmapData);


    ID3D11DeviceContext *pContext = nullptr;
    pDevice->GetImmediateContext(&pContext);
    D3D11_MAPPED_SUBRESOURCE map;
    ck(pContext->Map(pTexSysMem.Get(), D3D11CalcSubresource(0, 0, 1),
                     D3D11_MAP_WRITE, 0, &map));
    for (int y = 0; y < nHeight; y++) {
      memcpy((uint8_t *)map.pData + y * map.RowPitch,
             (uint8_t *)bitmapData.Scan0 + y * nWidth * 4, nWidth * 4);
    }
    pContext->Unmap(pTexSysMem.Get(), D3D11CalcSubresource(0, 0, 1));

    const NvEncInputFrame *encoderInputFrame = p_enc->GetNextInputFrame();
    ID3D11Texture2D *pTexBgra =
        reinterpret_cast<ID3D11Texture2D *>(encoderInputFrame->inputPtr);
    pContext->CopyResource(pTexBgra, pTexSysMem.Get());
    p_enc->EncodeFrame(vPacket);


    send_frame(vPacket);
    pContext->Release();
    bitmap->UnlockBits(&bitmapData);
    delete bitmap;
    Gdiplus::GdiplusShutdown(gdiplusToken);
  }
  void send_frame(std::vector<std::vector<uint8_t>> vPacket){
    nFrame += (int)vPacket.size();
    for (std::vector<uint8_t> &packet : vPacket) {
      // fpOut.write(reinterpret_cast<char *>(packet.data()), packet.size());
      udpSendCallBack(packet);
      if (nFrame == 1) {
        udpSendCallBack(packet);
      }
      std::cout << "packet.size():" << packet.size() << std::endl;
      totalSize += packet.size();
    }
  }
};
