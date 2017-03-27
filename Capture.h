/*
* Author：王德明
* Email：phight@163.com
* QQ群：220954528
*/

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
    int bitwide;
};

enum CAPTURE_STATUS_E
{
	CAPTURE_STATUS_STOP = 0,
	CAPTURE_STATUS_START
};

class Capture
{

public:
	virtual ~Capture();

public:
    static int Init();
    static int Uninit();
    static int EnumVideoCature(const std::vector<Capture *> *& vCaptureList);
    static int EnumAudioCature(const std::vector<Capture *> *& aCaptureList);
    static Capture* GetVideoCature(int index);
    static Capture* GetAudioCature(int index);

public:
    virtual int AddSink(Sink * sink) = 0;
    virtual int GetSupportAttribute(void* attribute) = 0;
    virtual int SetConfig(void* attribute) = 0;
	virtual int GetConfig(void* attribute) = 0;
	virtual CString GetName() = 0;
	virtual CAPTURE_STATUS_E GetStatus() = 0;
    virtual int Start() = 0;
    virtual int Stop() = 0;
};

bool IsVideoFormatSupport(const GUID &guid);
bool IsAudioFormatSupport(GUID &guid);
LPCTSTR GetFormatName(GUID &guid);
