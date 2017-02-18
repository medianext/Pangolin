#pragma once

#include "Capture.h"


class AudioCapture :
    public Capture,
    public IMFSourceReaderCallback
{

public:
    AudioCapture(void * priv);
    ~AudioCapture();

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFSourceReaderCallback methods
    STDMETHODIMP OnReadSample(
        HRESULT hrStatus,
        DWORD dwStreamIndex,
        DWORD dwStreamFlags,
        LONGLONG llTimestamp,
        IMFSample *pSample
        );

    STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *)
    {
        return S_OK;
    }

    STDMETHODIMP OnFlush(DWORD)
    {
        return S_OK;
    }

private:
    long                    m_nRefCount;        // Reference count.
    CRITICAL_SECTION        m_critsec;

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

