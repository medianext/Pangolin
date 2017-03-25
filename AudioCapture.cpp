#include "stdafx.h"
#include "AudioCapture.h"


AudioCapture::AudioCapture(void * priv) :
    m_nRefCount(1),
    m_pReader(NULL),
    m_pActivate(NULL)
{

    InitializeCriticalSection(&m_critsec);
	m_pActivate = (IMFActivate*)priv;

	this->EnumAttribute();

	this->CreateSourceReader();
}


AudioCapture::~AudioCapture()
{
	EnterCriticalSection(&m_critsec);
	SafeRelease(&m_pReader);
	LeaveCriticalSection(&m_critsec);

	vector<AudioCaptureAttribute*>::iterator it;
	for (it = m_AttributeList.begin(); it != m_AttributeList.end();)
	{
		AudioCaptureAttribute* pattr = *it;
		it = m_AttributeList.erase(it);
		delete pattr;
	}

    DeleteCriticalSection(&m_critsec);
}


/////////////// Private methods ///////////////

void AudioCapture::EnumAttribute()
{

	HRESULT hr = S_OK;

	IMFMediaSource  *pSource = NULL;

	hr = m_pActivate->ActivateObject(
		__uuidof(IMFMediaSource),
		(void**)&pSource
		);

	IMFPresentationDescriptor* presentationDescriptor;
	hr = pSource->CreatePresentationDescriptor(&presentationDescriptor);
	if (SUCCEEDED(hr))
	{
		DWORD dwCount = 0;
		presentationDescriptor->GetStreamDescriptorCount(&dwCount);
		if (dwCount > 0)
		{
			BOOL bSelect;
			IMFStreamDescriptor *pStreamDescriptor = NULL;
			hr = presentationDescriptor->GetStreamDescriptorByIndex(0, &bSelect, &pStreamDescriptor);
			if (SUCCEEDED(hr) && bSelect == TRUE)
			{
				IMFMediaTypeHandler *pMediaTypeHandler = NULL;
				hr = pStreamDescriptor->GetMediaTypeHandler(&pMediaTypeHandler);
				if (!SUCCEEDED(hr))
				{
					SafeRelease(&pStreamDescriptor);
				}
				UINT32 maxFactor = 0;
				DWORD dwMediaTypeCount = 0;
				hr = pMediaTypeHandler->GetMediaTypeCount(&dwMediaTypeCount);
				for (DWORD j = 0; j < dwMediaTypeCount; j++)
				{
					IMFMediaType * pMediaType = NULL;
					hr = pMediaTypeHandler->GetMediaTypeByIndex(j, &pMediaType);
					if (SUCCEEDED(hr))
					{
						UINT32 uChannel, nSamplesRate, wBitsPerSample, wSamplesPerBlock;
						GUID subType;
						hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &subType);
						hr = pMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &uChannel);
						hr = pMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &nSamplesRate);
						hr = pMediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &wBitsPerSample);
						hr = pMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_BLOCK, &wSamplesPerBlock);

						AudioCaptureAttribute *attribute = new AudioCaptureAttribute();
						attribute->format = subType;
						attribute->channel = uChannel;
						attribute->samplerate = nSamplesRate;
						attribute->bitwide = wBitsPerSample;
						m_AttributeList.push_back(attribute);

						m_BestAttribute.format = subType;
						m_BestAttribute.channel = uChannel;
						m_BestAttribute.samplerate = nSamplesRate;
						m_BestAttribute.bitwide = wBitsPerSample;
					}
					SafeRelease(&pMediaType);
				}
				SafeRelease(&pMediaTypeHandler);
			}
			SafeRelease(&pStreamDescriptor);
		}
		SafeRelease(&presentationDescriptor);
	}

	SafeRelease(&pSource);
}


void AudioCapture::CreateSourceReader()
{
	HRESULT hr = S_OK;

	IMFMediaSource  *pSource = NULL;

	hr = m_pActivate->ActivateObject(
		__uuidof(IMFMediaSource),
		(void**)&pSource
		);

	// Create the IMFSourceReader
	IMFAttributes   *pAttributes = NULL;
	hr = MFCreateAttributes(&pAttributes, 2);
	if (SUCCEEDED(hr))
	{
		hr = pAttributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, TRUE);
		hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);

		hr = MFCreateSourceReaderFromMediaSource(pSource, pAttributes, &m_pReader);
		if (!SUCCEEDED(hr))
		{
		}
		IMFMediaType * pMediaType = NULL;
		hr = m_pReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pMediaType);
		if (SUCCEEDED(hr))
		{

			UINT32 uChannel, nSamplesRate, wBitsPerSample;
			GUID subType;
			hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &subType);
			hr = pMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &uChannel);
			hr = pMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &nSamplesRate);
			hr = pMediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &wBitsPerSample);
			m_CurrentAttribute.format = subType;
			m_CurrentAttribute.channel = uChannel;
			m_CurrentAttribute.samplerate = nSamplesRate;
			m_CurrentAttribute.bitwide = wBitsPerSample;
			SafeRelease(&pMediaType);
		}
		SafeRelease(&pAttributes);
	}

	SafeRelease(&pSource);

}


/////////////// IUnknown methods ///////////////

ULONG AudioCapture::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}


ULONG AudioCapture::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}


HRESULT AudioCapture::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(AudioCapture, IMFSourceReaderCallback),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


/////////////// IMFSourceReaderCallback methods ///////////////

HRESULT AudioCapture::OnReadSample(
    HRESULT hrStatus,
    DWORD /* dwStreamIndex */,
    DWORD /* dwStreamFlags */,
    LONGLONG llTimestamp,
    IMFSample *pSample      // Can be NULL
    )
{
    HRESULT hr = S_OK;
    IMFMediaBuffer *pBuffer = NULL;

    static int count = 0;

    int ret = 0;

    EnterCriticalSection(&m_critsec);

    if (FAILED(hrStatus))
    {
        hr = hrStatus;
    }

    if (SUCCEEDED(hr))
    {
        if (pSample)
        {            // Get the video frame buffer from the sample.
            hr = pSample->GetBufferByIndex(0, &pBuffer);
            if (SUCCEEDED(hr))
            {
                MediaFrame frame(pBuffer, FRAME_TYPE_AUDIO, m_CurrentAttribute.samplerate, m_CurrentAttribute.channel, m_CurrentAttribute.bitwide);
                frame.m_subtype = m_CurrentAttribute.format;
				frame.m_uTimestamp = llTimestamp / 10;

                if (SUCCEEDED(hr)) {
                    vector<Sink*>::iterator iter = m_Sinks.begin();
                    for (; iter != m_Sinks.end(); ++iter)
                        (*iter)->SendFrame(&frame);

#if REC_CAPTURE_RAW
                    if (m_file.is_open())
                    {
                        m_file.write((char*)frame.m_pData, frame.m_dataSize);
                    }
#endif
                }
            }
        }

    }

    // Request the next frame.
    if (SUCCEEDED(hr))
    {
        hr = m_pReader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            0,
            NULL,   // actual
            NULL,   // flags
            NULL,   // timestamp
            NULL    // sample
            );
    }

    SafeRelease(&pBuffer);

    LeaveCriticalSection(&m_critsec);
    return hr;
}


/////////////// Capture methods ///////////////

int AudioCapture::AddSink(Sink * sink)
{
    if ((sink != NULL) && (sink->SetSourceAttribute(&m_CurrentAttribute, ATTRIBUTE_TYPE_AUDIO) >= 0))
    {
        m_Sinks.push_back(sink);
    }
    return 0;
}


int AudioCapture::GetSupportAttribute(void* attribute)
{
    *(vector<AudioCaptureAttribute*>**)attribute = &m_AttributeList;
    return (int)m_AttributeList.size();
}


int AudioCapture::SetConfig(void* attribute)
{
    return 0;
}


int AudioCapture::GetConfig(void* attribute)
{
    if (attribute)
    {
        *(AudioCaptureAttribute*)attribute = m_CurrentAttribute;
    }
    return 0;
}


CAPTURE_STATUS_E AudioCapture::GetStatus()
{
	return m_Status;
}


int AudioCapture::Start()
{
	if (m_Status== CAPTURE_STATUS_START)
	{
		return 0;
	}

	EnterCriticalSection(&m_critsec);

#if REC_CAPTURE_RAW
	if (m_file.is_open())
	{
		m_file.close();
	}
    m_file.open("capture.pcm", ios::out | ios::binary);
#endif

	m_pReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, NULL, NULL, NULL, NULL);

	m_Status = CAPTURE_STATUS_START;

	LeaveCriticalSection(&m_critsec);

    return 0;
}


int AudioCapture::Stop()
{
	EnterCriticalSection(&m_critsec);

#if REC_CAPTURE_RAW
    if (m_file.is_open())
    {
        m_file.flush();
        m_file.close();
    }
#endif

	m_pReader->Flush(MF_SOURCE_READER_FIRST_AUDIO_STREAM);

	m_Status = CAPTURE_STATUS_STOP;

	LeaveCriticalSection(&m_critsec);

    return 0;
}

