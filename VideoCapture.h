#pragma once

#include "Capture.h"


class VideoCapture :
    public Capture
{

public:
    VideoCapture(void * priv);
    ~VideoCapture();

private:
    IMFActivate				*m_pActivate;
    IMFSourceReader         *m_pReader;
    vector<VideoCaptureAttribute*> m_Attributes;
    vector<Sink *>           m_Sinks;

public:
    int AddSink(Sink * sink);
    int EnumAttribute(void* attribute);
    int Config(void* attribute);
    int Start();
    int Stop();
};

