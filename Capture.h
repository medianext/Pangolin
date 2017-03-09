#pragma once

#include "Sink.h"

struct VideoCaptureAttribute {
    GUID format;
    int width;
    int stride;
    int height;
    int fps;
};

struct AudioCaptureAttribute {
    GUID format;
    int channel;
    int samplerate;
    int samplesize;
    int bitwide;
};

class Capture
{

public:
	virtual ~Capture();

public:
    static int Init();
    static int Uninit();
    static int EnumVideoCature(std::vector<CString *> *vCaptureList);
    static int EnumAudioCature(std::vector<CString *> *aCaptureList);
    static Capture* GetVideoCature(int index);
    static Capture* GetAudioCature(int index);

public:
    virtual int AddSink(Sink * sink) = 0;
    virtual int EnumAttribute(void* attribute) = 0;
    virtual int SetConfig(void* attribute) = 0;
    virtual int GetConfig(void* attribute) = 0;
    virtual int Start() = 0;
    virtual int Stop() = 0;
};

