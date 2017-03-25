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
	vector<AudioCaptureAttribute*> m_AttributeList;
	AudioCaptureAttribute    m_BestAttribute;
	AudioCaptureAttribute    m_CurrentAttribute;
	vector<Sink *>           m_Sinks;

	CAPTURE_STATUS_E         m_Status = CAPTURE_STATUS_STOP;

#if REC_CAPTURE_RAW
    ofstream                 m_file;
#endif

private:
	void EnumAttribute();
	void CreateSourceReader();

public:
    int AddSink(Sink * sink);
    int GetSupportAttribute(void* attribute);
    int SetConfig(void* attribute);
	int GetConfig(void* attribute);
	CAPTURE_STATUS_E GetStatus();
    int Start();
    int Stop();

};

