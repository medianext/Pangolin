#include "stdafx.h"
#include "AudioCapture.h"


AudioCapture::AudioCapture(void * priv) :
    m_nRefCount(1),
    m_pReader(NULL),
    m_pActivate(NULL)
{

     InitializeCriticalSection(&m_critsec);
    m_pActivate = (IMFActivate*)priv;

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
                        m_Attributes.push_back(attribute);

                        m_attribute.format = subType;
                        m_attribute.channel = uChannel;
                        m_attribute.samplerate = nSamplesRate;
                        m_attribute.bitwide = wBitsPerSample;
                    }
                    SafeRelease(&pMediaType);
                }
                SafeRelease(&pMediaTypeHandler);
            }
            SafeRelease(&pStreamDescriptor);
        }
    }

    SafeRelease(&presentationDescriptor);

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

            UINT32 uChannel, nSamplesRate, wBitsPerSample, wSamplesPerBlock;
            GUID subType;
            hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &subType);
            hr = pMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &uChannel);
            hr = pMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &nSamplesRate);
            hr = pMediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &wBitsPerSample);
            hr = pMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_BLOCK, &wSamplesPerBlock);
            m_attribute.format = subType;
            m_attribute.channel = uChannel;
            m_attribute.samplerate = nSamplesRate;
            m_attribute.bitwide = wBitsPerSample;
            SafeRelease(&pMediaType);
        }
        SafeRelease(&pAttributes);
    }

    SafeRelease(&pSource);
}


AudioCapture::~AudioCapture()
{
	EnterCriticalSection(&m_critsec);
	SafeRelease(&m_pReader);
	LeaveCriticalSection(&m_critsec);

	vector<AudioCaptureAttribute*>::iterator it;
	for (it = m_Attributes.begin(); it != m_Attributes.end();)
	{
		AudioCaptureAttribute* pattr = *it;
		it = m_Attributes.erase(it);
		delete pattr;
	}

    DeleteCriticalSection(&m_critsec);
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
                MediaFrame frame(pBuffer, FRAME_TYPE_AUDIO, m_attribute.samplerate, m_attribute.channel, m_attribute.bitwide);
                frame.m_subtype = m_attribute.format;
				frame.m_uTimestamp = llTimestamp / 10;

                if (SUCCEEDED(hr)) {
                    vector<Sink*>::iterator iter = m_Sinks.begin();
                    for (; iter != m_Sinks.end(); ++iter)
                        (*iter)->SendFrame(&frame);
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
    if ((sink != NULL) && (sink->SetSourceAttribute(&m_attribute, ATTRIBUTE_TYPE_AUDIO) >= 0))
    {
        m_Sinks.push_back(sink);
    }
    return 0;
}


int AudioCapture::EnumAttribute(void* attribute)
{
    *(vector<AudioCaptureAttribute*>**)attribute = &m_Attributes;
    return (int)m_Attributes.size();
}


int AudioCapture::Config(void* attribute)
{
    return 0;
}


int AudioCapture::Start()
{

    HRESULT hr = m_pReader->ReadSample(
        (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        0,
        NULL,
        NULL,
        NULL,
        NULL
        );
    return 0;
}


int AudioCapture::Stop()
{
    return 0;
}

