#include "stdafx.h"
#include "VideoCapture.h"


VideoCapture::VideoCapture(void * priv) :
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
                    VideoCaptureAttribute *attribute = new VideoCaptureAttribute();
                    IMFMediaType * pMediaType = NULL;
                    hr = pMediaTypeHandler->GetMediaTypeByIndex(j, &pMediaType);
                    if (SUCCEEDED(hr))
                    {
                        UINT32 uWidth, uHeight, uNummerator, uDenominator;
                        GUID subType;
                        pMediaType->GetGUID(MF_MT_SUBTYPE, &subType);
                        MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &uWidth, &uHeight);
                        MFGetAttributeRatio(pMediaType, MF_MT_FRAME_RATE, &uNummerator, &uDenominator);
                        attribute = new VideoCaptureAttribute();
                        attribute->width = uWidth;
                        attribute->height = uHeight;
                        attribute->fps = uNummerator;
                        m_Attributes.push_back(attribute);
                        UINT32 factor = uWidth * uHeight * uNummerator;
                        if (factor > maxFactor)
                        {
                            maxFactor = factor;
                            pMediaTypeHandler->SetCurrentMediaType(pMediaType);
                        }
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


VideoCapture::~VideoCapture()
{

    SafeRelease(&m_pActivate);
}


int VideoCapture::AddSink(Sink * sink)
{
    return 0;
}


int VideoCapture::EnumAttribute(void* attribute)
{
    *(vector<VideoCaptureAttribute*>**)attribute = &m_Attributes;
    return (int)m_Attributes.size();
}


int VideoCapture::Config(void* attribute)
{
    return 0;
}


int VideoCapture::Start()
{
    return 0;
}


int VideoCapture::Stop()
{
    return 0;
}

