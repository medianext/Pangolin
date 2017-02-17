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
    int Config(void* attribute);
    int Start();
    int Stop();

};

