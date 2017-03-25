#include "stdafx.h"
#include "VideoCapture.h"


VideoCapture::VideoCapture(void * priv) :
    m_nRefCount(1),
    m_pReader(NULL),
    m_pActivate(NULL)
{

    InitializeCriticalSection(&m_critsec);
    m_pActivate = (IMFActivate*)priv;

	this->EnumAttribute();

	this->CreateSourceReader();
}


VideoCapture::~VideoCapture()
{
	EnterCriticalSection(&m_critsec);
	SafeRelease(&m_pReader);
	LeaveCriticalSection(&m_critsec);

	VideoCaptureAttribute* pattr = NULL;
	vector<VideoCaptureAttribute*>::iterator it;
	for (it = m_AttributeList.begin(); it != m_AttributeList.end();)
	{
		pattr = *it;
		it = m_AttributeList.erase(it);
		delete pattr;
	}

    DeleteCriticalSection(&m_critsec);
}


/////////////// Private methods ///////////////

void VideoCapture::EnumAttribute()
{
	HRESULT hr = S_OK;

	IMFMediaSource  *pSource = nullptr;

	hr = m_pActivate->ActivateObject(
		__uuidof(IMFMediaSource),
		(void**)&pSource
		);

	IMFPresentationDescriptor* presentationDescriptor = nullptr;
	hr = pSource->CreatePresentationDescriptor(&presentationDescriptor);
	if (SUCCEEDED(hr))
	{
		DWORD dwCount = 0;
		presentationDescriptor->GetStreamDescriptorCount(&dwCount);
		if (dwCount > 0)
		{
			BOOL bSelect;
			IMFStreamDescriptor *pStreamDescriptor = nullptr;
			hr = presentationDescriptor->GetStreamDescriptorByIndex(0, &bSelect, &pStreamDescriptor);
			if (SUCCEEDED(hr) && bSelect == TRUE)
			{
				IMFMediaTypeHandler *pMediaTypeHandler = nullptr;
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
					IMFMediaType * pMediaType = nullptr;
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

						VideoCaptureAttribute *attribute = new VideoCaptureAttribute();
						attribute->format = subType;
						attribute->stride = lStride;
						attribute->width = uWidth;
						attribute->height = uHeight;
						attribute->fps = uNummerator;
						m_AttributeList.push_back(attribute);

						UINT32 factor = uWidth * uHeight * uNummerator;
						if (factor > maxFactor)
						{
							maxFactor = factor;
							pMediaTypeHandler->SetCurrentMediaType(pMediaType);
							m_BestAttribute.format = subType;
							m_BestAttribute.stride = lStride;
							m_BestAttribute.width = uWidth;
							m_BestAttribute.height = uHeight;
							m_BestAttribute.fps = uNummerator;
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


void VideoCapture::CreateSourceReader()
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
			m_CurrentAttribute.format = subType;
			m_CurrentAttribute.stride = lStride;
			m_CurrentAttribute.width = uWidth;
			m_CurrentAttribute.height = uHeight;
			m_CurrentAttribute.fps = uNummerator;
			SafeRelease(&pMediaType);
		}
		SafeRelease(&pAttributes);
	}

	SafeRelease(&pSource);

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
        {
            // Get the video frame buffer from the sample.
			hr = pSample->GetBufferByIndex(0, &pBuffer);
			if (SUCCEEDED(hr))
            {
                MediaFrame frame(pBuffer, FRAME_TYPE_VIDEO, &m_CurrentAttribute);
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
    if ( (sink != NULL) && (sink->SetSourceAttribute(&m_CurrentAttribute, ATTRIBUTE_TYPE_VIDEO)>=0) )
    {
        m_Sinks.push_back(sink);
    }
    return 0;
}


int VideoCapture::GetSupportAttribute(void* attribute)
{
    *(vector<VideoCaptureAttribute*>**)attribute = &m_AttributeList;
    return (int)m_AttributeList.size();
}


int VideoCapture::SetConfig(void* attribute)
{
	VideoCaptureAttribute* pattr = (VideoCaptureAttribute*)attribute;
	if (pattr==nullptr)
	{
		return -1;
	}

	HRESULT hr = S_OK;

	EnterCriticalSection(&m_critsec);

	CAPTURE_STATUS_E status = this->m_Status;
	if (status == CAPTURE_STATUS_START)
	{
		hr = m_pReader->Flush(MF_SOURCE_READER_FIRST_VIDEO_STREAM);
	}

	bool success = false;

	for (int i = 0; ; i++)
	{
		IMFMediaType   *pMediaType= nullptr;
		hr = m_pReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, i, &pMediaType);
		if (!SUCCEEDED(hr))
		{
			break;
		}

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
		
		if (uWidth==pattr->width && uHeight==pattr->height && uNummerator==pattr->fps && subType==pattr->format)
		{
			hr = m_pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pMediaType);
			if (SUCCEEDED(hr))
			{
				m_CurrentAttribute = *pattr;
				m_CurrentAttribute.stride = lStride;

				vector<Sink*>::iterator it;
				for (it = m_Sinks.begin(); it != m_Sinks.end(); it++)
				{
					Sink* sink = *it;
					sink->SetSourceAttribute(&m_CurrentAttribute, ATTRIBUTE_TYPE_VIDEO);
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

	if (status == CAPTURE_STATUS_START)
	{
		hr = m_pReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL, NULL, NULL);
	}

	LeaveCriticalSection(&m_critsec);

    return 0;
}


int VideoCapture::GetConfig(void* attribute)
{
    if (attribute)
    {
        *(VideoCaptureAttribute*)attribute = m_CurrentAttribute;
    }
    return 0;
}


CAPTURE_STATUS_E VideoCapture::GetStatus()
{
	return m_Status;
}


int VideoCapture::Start()
{
	EnterCriticalSection(&m_critsec);

	m_pReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL, NULL, NULL);

	m_Status = CAPTURE_STATUS_START;

	LeaveCriticalSection(&m_critsec);

    return 0;
}


int VideoCapture::Stop()
{
	EnterCriticalSection(&m_critsec);

	m_pReader->Flush(MF_SOURCE_READER_FIRST_VIDEO_STREAM);

	m_Status = CAPTURE_STATUS_STOP;

	LeaveCriticalSection(&m_critsec);

    return 0;
}

