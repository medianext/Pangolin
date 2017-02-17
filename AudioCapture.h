#pragma once

#include "Capture.h"


class AudioCapture :
    public Capture
{

public:
    AudioCapture();
    ~AudioCapture();

public:
    int AddSink(Sink * sink);
    int EnumAttribute(void** attribute);
    int Config(void* attribute);
    int Start();
    int Stop();

};

