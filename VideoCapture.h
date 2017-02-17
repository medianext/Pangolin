#pragma once

#include "Capture.h"


class VideoCapture :
    public Capture
{

public:
    VideoCapture();
    ~VideoCapture();

public:
    int AddSink(Sink * sink);
    int EnumAttribute(void** attribute);
    int Config(void* attribute);
    int Start();
    int Stop();
};

