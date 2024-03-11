#pragma once

// #include "FfiDynamicEncoderParams.h"
#include "NvEncoderD3D11.h"
#include "alvr_server\bindings.h"
#include "shared/d3drender.h"
#include <functional>
#include <memory>

class VideoEncoder {
public:
  virtual void Initialize() = 0;
  virtual void Shutdown() = 0;

  virtual std::vector<std::vector<uint8_t>>
  Transmit(ID3D11Texture2D *pTexture, uint64_t presentationTime,
           uint64_t targetTimestampNs, bool insertIDR, bool insertSPSPPS,
           FfiDynamicEncoderParams params) = 0;
};
