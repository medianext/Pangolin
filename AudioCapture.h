#pragma once

#include "Capture.h"


class AudioCapture :
    public Capture
{

public:
    AudioCapture(void * priv);
    ~AudioCapture();

private:
    IMFActivate				*m_pActivate;
    IMFSourceReader         *m_pReader;
    vector<AudioCaptureAttribute*> m_Attributes;
    vector<Sink *>           m_Sinks;

public:
    int AddSink(Sink * sink);
    int EnumAttribute(void* attribute);
    int Config(void* attribute);
    int Start();
    int Stop();

};

