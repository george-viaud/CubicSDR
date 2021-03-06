#pragma once

#include <atomic>

#include "ThreadQueue.h"
#include "DemodulatorMgr.h"
#include "SDRDeviceInfo.h"
#include "AppConfig.h"

#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Device.hpp>


class SDRThreadIQData: public ReferenceCounter {
public:
    long long frequency;
    long long sampleRate;
    bool dcCorrected;
    int numChannels;
    std::vector<liquid_float_complex> data;

    SDRThreadIQData() :
            frequency(0), sampleRate(DEFAULT_SAMPLE_RATE), dcCorrected(true), numChannels(0) {

    }

    SDRThreadIQData(long long bandwidth, long long frequency, std::vector<signed char> * /* data */) :
            frequency(frequency), sampleRate(bandwidth) {

    }

    ~SDRThreadIQData() {

    }
};

typedef ThreadQueue<SDRThreadIQData *> SDRThreadIQDataQueue;

class SDRThread : public IOThread {
private:
    void init();
    void deinit();
    void readStream(SDRThreadIQDataQueue* iqDataOutQueue);
    void readLoop();

public:
    SDRThread();
    ~SDRThread();
    enum SDRThreadState { SDR_THREAD_MESSAGE, SDR_THREAD_INITIALIZED, SDR_THREAD_TERMINATED, SDR_THREAD_FAILED };
    
    void run();

    SDRDeviceInfo *getDevice();
    void setDevice(SDRDeviceInfo *dev);
    int getOptimalElementCount(long long sampleRate, int fps);
    int getOptimalChannelCount(long long sampleRate);
    
    void setFrequency(long long freq);
    long long getFrequency();
    
    void lockFrequency(long long freq);
    bool isFrequencyLocked();
    void unlockFrequency();
    
    void setOffset(long long ofs);
    long long getOffset();
    
    void setSampleRate(int rate);
    int getSampleRate();

    void setPPM(int ppm);
    int getPPM();
    
    void setAGCMode(bool mode);
    bool getAGCMode();

    void setIQSwap(bool swap);
    bool getIQSwap();

    void setGain(std::string name, float value);
    float getGain(std::string name);
    
    void writeSetting(std::string name, std::string value);
    std::string readSetting(std::string name);
    
    void setStreamArgs(SoapySDR::Kwargs streamArgs);
    
protected:
    void updateGains();
    void updateSettings();
    SoapySDR::Kwargs combineArgs(SoapySDR::Kwargs a, SoapySDR::Kwargs b);

    SoapySDR::Stream *stream;
    SoapySDR::Device *device;
    void *buffs[1];
    ReBuffer<SDRThreadIQData> buffers;
    SDRThreadIQData inpBuffer;
    SDRThreadIQData overflowBuffer;
    int numOverflow;
    std::atomic<DeviceConfig *> deviceConfig;
    std::atomic<SDRDeviceInfo *> deviceInfo;
    
    std::mutex setting_busy;
    std::map<std::string, std::string> settings;
    std::map<std::string, bool> settingChanged;

    std::atomic<uint32_t> sampleRate;
    std::atomic_llong frequency, offset, lock_freq;
    std::atomic_int ppm, numElems, mtuElems, numChannels;
    std::atomic_bool hasPPM, hasHardwareDC;
    std::atomic_bool agc_mode, rate_changed, freq_changed, offset_changed,
        ppm_changed, device_changed, agc_mode_changed, gain_value_changed, setting_value_changed, frequency_locked, frequency_lock_init, iq_swap;

    std::mutex gain_busy;
    std::map<std::string, float> gainValues;
    std::map<std::string, bool> gainChanged;
    
    SoapySDR::Kwargs streamArgs;
};
