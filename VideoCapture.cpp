/*
* Author：王德明
* Email：phight@163.com
* QQ群：220954528
*/

#include "stdafx.h"
#include "VideoCapture.h"


VideoCapture::VideoCapture(void * priv) :
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
	hr = pActivate->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &str, NULL);
	m_CaptureLink = str;
	CoTaskMemFree(str);

	this->EnumAttribute(pActivate);
}


VideoCapture::~VideoCapture()
{
	EnterCriticalSection(&m_critsec);
	SafeRelease(&m_pReader);
	if (m_pCurrentAttribute)
	{
		delete m_pCurrentAttribute;
		m_pCurrentAttribute = NULL;
	}
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

void VideoCapture::EnumAttribute(IMFActivate* pActivate)
{
	HRESULT hr = S_OK;

	IMFMediaSource  *pSource = nullptr;

	hr = pActivate->ActivateObject(
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
                        if (IsVideoFormatSupport(subType))
                        {
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
                            attribute->fps = uNummerator / uDenominator;
                            m_AttributeList.push_back(attribute);

                            UINT32 factor = attribute->width * attribute->height * attribute->fps;
                            if (factor > maxFactor)
                            {
                                maxFactor = factor;
                                m_pBestAttribute = attribute;
                            }
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
	IMFAttributes   *pAttributes = NULL;

	SafeRelease(&m_pReader);
	if (m_pCurrentAttribute)
	{
		delete m_pCurrentAttribute;
		m_pCurrentAttribute = NULL;
	}

	hr = MFCreateAttributes(&pAttributes, 2);
	pAttributes->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, m_CaptureLink);
	pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
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


HRESULT VideoCapture::SetConfigInternal(VideoCaptureAttribute* pattr)
{
	HRESULT hr;

	bool success = false;

	for (int i = 0; ; i++)
	{
		IMFMediaType   *pMediaType = nullptr;
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

		if (uWidth == pattr->width && uHeight == pattr->height && (uNummerator / uDenominator) == pattr->fps && subType == pattr->format)
		{
			hr = m_pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pMediaType);
			if (SUCCEEDED(hr))
			{
				if (m_pCurrentAttribute==NULL)
				{
					m_pCurrentAttribute = new VideoCaptureAttribute;
				}
				*m_pCurrentAttribute = *pattr;
				m_pCurrentAttribute->stride = lStride;

				vector<Sink*>::iterator it;
				for (it = m_Sinks.begin(); it != m_Sinks.end(); it++)
				{
					Sink* sink = *it;
					sink->SetSourceAttribute(m_pCurrentAttribute, ATTRIBUTE_TYPE_VIDEO);
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
                MediaFrame frame(pBuffer, FRAME_TYPE_VIDEO, m_pCurrentAttribute);
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
    if ( sink != NULL )
    {
		if (m_pCurrentAttribute)
		{
			sink->SetSourceAttribute(m_pCurrentAttribute, ATTRIBUTE_TYPE_VIDEO);
		}
		else
		{
			sink->SetSourceAttribute(m_pBestAttribute, ATTRIBUTE_TYPE_VIDEO);
		}
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

	SetConfigInternal(pattr);

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
		if (m_pCurrentAttribute)
		{
			*(VideoCaptureAttribute*)attribute = *m_pCurrentAttribute;
		}
		else
		{
			*(VideoCaptureAttribute*)attribute = *m_pBestAttribute;
		}
    }
    return 0;
}


CString VideoCapture::GetName()
{
	return m_CaptureName;
}


CAPTURE_STATUS_E VideoCapture::GetStatus()
{
	return m_Status;
}


int VideoCapture::Start()
{
	EnterCriticalSection(&m_critsec);

	this->CreateSourceReader();

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

