#include "stdafx.h"
#include "AudioCapture.h"


AudioCapture::AudioCapture(void * priv) :
    m_pActivate(NULL)
{
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
                    AudioCaptureAttribute *attribute = new AudioCaptureAttribute();
                    IMFMediaType * pMediaType = NULL;
                    hr = pMediaTypeHandler->GetMediaTypeByIndex(j, &pMediaType);
                    if (SUCCEEDED(hr))
                    {
                        UINT32 uChannel, nSamplesRate;
                        GUID subType;
                        pMediaType->GetGUID(MF_MT_SUBTYPE, &subType);
                        pMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &uChannel);
                        pMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &nSamplesRate);
                        attribute = new AudioCaptureAttribute();
                        attribute->channel = uChannel;
                        attribute->samplerate = nSamplesRate;
                        m_Attributes.push_back(attribute);
                    }
                    SafeRelease(&pMediaType);
                }
                SafeRelease(&pMediaTypeHandler);
            }
            SafeRelease(&pStreamDescriptor);
        }
    }

    SafeRelease(&presentationDescriptor);

    SafeRelease(&pSource);
}


AudioCapture::~AudioCapture()
{

    SafeRelease(&m_pActivate);
}


int AudioCapture::AddSink(Sink * sink)
{
    if (sink!=NULL)
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
    return 0;
}


int AudioCapture::Stop()
{
    return 0;
}

