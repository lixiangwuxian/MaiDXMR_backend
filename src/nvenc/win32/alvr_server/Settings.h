#pragma once

#include "../alvr_common/packet_types.h"
#include <string>

class Settings {
  static Settings m_Instance;
  bool m_loaded;

  Settings();
  virtual ~Settings();

public:
  void Load();
  static Settings &Instance() { return m_Instance; }

  bool IsLoaded() { return m_loaded; }

  int m_codec;
  int m_refreshRate;

  bool m_use10bitEncoder;
  uint32_t m_nvencQualityPreset;
  uint32_t m_rateControlMode;
  bool m_fillerData;
  uint32_t m_entropyCoding;
  uint32_t m_nvencTuningPreset;
  uint32_t m_nvencMultiPass;
  uint32_t m_nvencAdaptiveQuantizationMode;
  int64_t m_nvencLowDelayKeyFrameScale;
  int64_t m_nvencRefreshRate;
  bool m_nvencEnableIntraRefresh;
  int64_t m_nvencIntraRefreshPeriod;
  int64_t m_nvencIntraRefreshCount;
  int64_t m_nvencMaxNumRefFrames;
  int64_t m_nvencGopLength;
  int64_t m_nvencPFrameStrategy;
  int64_t m_nvencRateControlMode;
  int64_t m_nvencRcBufferSize;
  int64_t m_nvencRcInitialDelay;
  int64_t m_nvencRcMaxBitrate;
  int64_t m_nvencRcAverageBitrate;
  bool m_nvencEnableWeightedPrediction;
};
