#include "stdafx.h"
#include "VideoCapture.h"


VideoCapture::VideoCapture(void * priv) :
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
                    VideoCaptureAttribute *attribute = new VideoCaptureAttribute();
                    IMFMediaType * pMediaType = NULL;
                    hr = pMediaTypeHandler->GetMediaTypeByIndex(j, &pMediaType);
                    if (SUCCEEDED(hr))
                    {
                        UINT32 uWidth, uHeight, uNummerator, uDenominator;
                        LONG lStride;
                        GUID subType;
                        pMediaType->GetGUID(MF_MT_SUBTYPE, &subType);
                        MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &uWidth, &uHeight);
                        MFGetAttributeRatio(pMediaType, MF_MT_FRAME_RATE, &uNummerator, &uDenominator);
                        hr = pMediaType->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*)&lStride);
                        if (FAILED(hr))
                        {
                            hr = MFGetStrideForBitmapInfoHeader(subType.Data1, uWidth, &lStride);
                        }
                        attribute = new VideoCaptureAttribute();
                        attribute->format = subType;
                        attribute->stride = lStride;
                        attribute->width = uWidth;
                        attribute->height = uHeight;
                        attribute->fps = uNummerator;
                        m_Attributes.push_back(attribute);
                        UINT32 factor = uWidth * uHeight * uNummerator;
                        if (factor > maxFactor)
                        {
                            maxFactor = factor;
                            pMediaTypeHandler->SetCurrentMediaType(pMediaType);
                            m_attribute.format = subType;
                            m_attribute.stride = lStride;
                            m_attribute.width = uWidth;
                            m_attribute.height = uHeight;
                            m_attribute.fps = uNummerator;
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
        hr = m_pReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pMediaType);
        if (SUCCEEDED(hr))
        {
            UINT32 uWidth, uHeight, uNummerator, uDenominator;
            LONG lStride;
            GUID subType;
            pMediaType->GetGUID(MF_MT_SUBTYPE, &subType);
            MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &uWidth, &uHeight);
            MFGetAttributeRatio(pMediaType, MF_MT_FRAME_RATE, &uNummerator, &uDenominator);
            hr = pMediaType->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*)&lStride);
            if (FAILED(hr))
            {
                hr = MFGetStrideForBitmapInfoHeader(subType.Data1, uWidth, &lStride);
            }
            m_attribute.format = subType;
            m_attribute.stride = lStride;
            m_attribute.width = uWidth;
            m_attribute.height = uHeight;
            m_attribute.fps = uNummerator;
            char formatName[5] = { 0 };
            formatName[0] = ((char *)(&subType.Data1))[0];
            formatName[1] = ((char *)(&subType.Data1))[1];
            formatName[2] = ((char *)(&subType.Data1))[2];
            formatName[3] = ((char *)(&subType.Data1))[3];
            SafeRelease(&pMediaType);
        }
        SafeRelease(&pAttributes);
    }

    SafeRelease(&pSource);
}


VideoCapture::~VideoCapture()
{

    SafeRelease(&m_pActivate);
    SafeRelease(&m_pReader);
    DeleteCriticalSection(&m_critsec);
}


/////////////// IUnknown methods ///////////////

ULONG VideoCapture::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}


ULONG VideoCapture::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}


HRESULT VideoCapture::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(VideoCapture, IMFSourceReaderCallback),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


/////////////// IMFSourceReaderCallback methods ///////////////

HRESULT VideoCapture::OnReadSample(
    HRESULT hrStatus,
    DWORD /* dwStreamIndex */,
    DWORD /* dwStreamFlags */,
    LONGLONG /* llTimestamp */,
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
        {
            // Get the video frame buffer from the sample.
			hr = pSample->GetBufferByIndex(0, &pBuffer);
			if (SUCCEEDED(hr))
			{
                MediaFrame frame(pBuffer, FRAME_TYPE_VIDEO, m_attribute.width, m_attribute.height, m_attribute.stride);
                frame.m_subtype = m_attribute.format;
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
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
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

int VideoCapture::AddSink(Sink * sink)
{
    if ( (sink != NULL) && (sink->SetSourceAttribute(&m_attribute, ATTRIBUTE_TYPE_VIDEO)>=0) )
    {
        m_Sinks.push_back(sink);
    }
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

    HRESULT hr = m_pReader->ReadSample(
        (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        0,
        NULL,
        NULL,
        NULL,
        NULL
        );
    return 0;
}


int VideoCapture::Stop()
{
    return 0;
}

