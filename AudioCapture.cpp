/*
* Author：王德明
* Email：phight@163.com
* QQ群：220954528
*/

#include "stdafx.h"
#include "AudioCapture.h"


AudioCapture::AudioCapture(void * priv) :
    m_nRefCount(1),
    m_pReader(NULL)
{
	HRESULT hr = S_OK;

	InitializeCriticalSection(&m_critsec);
	IMFActivate* pActivate = (IMFActivate*)priv;

	WCHAR * str;
	hr = pActivate->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &str, NULL);
	m_CaptureName = str;
	CoTaskMemFree(str);
	hr = pActivate->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID, &str, NULL);
	m_CapturePoint = str;
	CoTaskMemFree(str);

	this->EnumAttribute(pActivate);
}


AudioCapture::~AudioCapture()
{
	EnterCriticalSection(&m_critsec);
	SafeRelease(&m_pReader);
	if (m_pCurrentAttribute)
	{
		delete m_pCurrentAttribute;
		m_pCurrentAttribute = NULL;
	}
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

void AudioCapture::EnumAttribute(IMFActivate* pActivate)
{

	HRESULT hr = S_OK;

	IMFMediaSource  *pSource = NULL;

	hr = pActivate->ActivateObject(
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
						UINT32 uChannel, nSamplesRate, wBitsPerSample;
						GUID subType;
						hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &subType);
                        if (IsAudioFormatSupport(subType))
                        {
                            hr = pMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &uChannel);
                            hr = pMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &nSamplesRate);
                            hr = pMediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &wBitsPerSample);

                            AudioCaptureAttribute *attribute = new AudioCaptureAttribute();
                            attribute->format = subType;
                            attribute->channel = uChannel;
                            attribute->samplerate = nSamplesRate;
                            attribute->bitwide = wBitsPerSample;
                            m_AttributeList.push_back(attribute);

                            m_pBestAttribute = attribute;
                        }
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
	IMFAttributes   *pAttributes = NULL;

	SafeRelease(&m_pReader);
	if (m_pCurrentAttribute)
	{
		delete m_pCurrentAttribute;
		m_pCurrentAttribute = NULL;
	}

	hr = MFCreateAttributes(&pAttributes, 2);
	pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
	pAttributes->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID, m_CapturePoint);
	hr = MFCreateDeviceSource(pAttributes, &pSource);
	SafeRelease(&pAttributes);

	// Create the IMFSourceReader
	hr = MFCreateAttributes(&pAttributes, 2);
	if (SUCCEEDED(hr))
	{
		hr = pAttributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, TRUE);
		hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);

		hr = MFCreateSourceReaderFromMediaSource(pSource, pAttributes, &m_pReader);
		if (!SUCCEEDED(hr))
		{
		}

		SetConfigInternal(m_pBestAttribute);

		SafeRelease(&pAttributes);
	}

	SafeRelease(&pSource);

}


HRESULT AudioCapture::SetConfigInternal(AudioCaptureAttribute* pattr)
{
	HRESULT hr;

	bool success = false;

	for (int i = 0; ; i++)
	{
		IMFMediaType   *pMediaType = nullptr;
		hr = m_pReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, i, &pMediaType);
		if (!SUCCEEDED(hr))
		{
			break;
		}

		UINT32 uChannel, nSamplesRate, wBitsPerSample;
		GUID subType;
		hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &subType);
		hr = pMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &uChannel);
		hr = pMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &nSamplesRate);
		hr = pMediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &wBitsPerSample);

		if (uChannel == pattr->channel && nSamplesRate == pattr->samplerate && wBitsPerSample == pattr->bitwide && subType == pattr->format)
		{
			hr = m_pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, pMediaType);
			if (SUCCEEDED(hr))
			{
				if (m_pCurrentAttribute == NULL)
				{
					m_pCurrentAttribute = new AudioCaptureAttribute;
				}
				*m_pCurrentAttribute = *pattr;

				vector<Sink*>::iterator it;
				for (it = m_Sinks.begin(); it != m_Sinks.end(); it++)
				{
					Sink* sink = *it;
					sink->SetSourceAttribute(m_pCurrentAttribute, ATTRIBUTE_TYPE_AUDIO);
				}

				success = true;
			}
			else
			{
				OutputDebugString(TEXT("IMFSourceReader::SetCurrentMediaType failed\n"));
			}
		}

		SafeRelease(&pMediaType);

		if (success)
		{
			break;
		}

	}

	return hr;
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
                MediaFrame frame(pBuffer, FRAME_TYPE_AUDIO, m_pCurrentAttribute);
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
    if (sink != NULL )
	{
		if (m_pCurrentAttribute)
		{
			sink->SetSourceAttribute(m_pCurrentAttribute, ATTRIBUTE_TYPE_AUDIO);
		}
		else
		{
			sink->SetSourceAttribute(m_pBestAttribute, ATTRIBUTE_TYPE_AUDIO);
		}
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
		if (m_pCurrentAttribute)
		{
			*(AudioCaptureAttribute*)attribute = *m_pCurrentAttribute;
		}
		else
		{
			*(AudioCaptureAttribute*)attribute = *m_pBestAttribute;
		}
    }
    return 0;
}


CString AudioCapture::GetName()
{
	return m_CaptureName;
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

	this->CreateSourceReader();

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

