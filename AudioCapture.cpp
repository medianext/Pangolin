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
        SafeRelease(&pAttributes);
    }

    SafeRelease(&pSource);
}


AudioCapture::~AudioCapture()
{

    SafeRelease(&m_pActivate);
    SafeRelease(&m_pReader);
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

            // 			hr = pSample->GetBufferByIndex(0, &pBuffer);
            // 
            // 			// Draw the frame.
            // 
            // 			if (SUCCEEDED(hr))
            // 			{
            // 				BYTE * pbScanline0;
            // 				LONG lStride;
            // 
            // 				VideoBufferLock buffer(pBuffer);    // Helper object to lock the video buffer.
            // 
            // 													// Lock the video buffer. This method returns a pointer to the first scan
            // 													// line in the image, and the stride in bytes.
            // 				hr = buffer.LockBuffer(m_videoAttribute.m_uStride, m_videoAttribute.m_uHeight, &pbScanline0, &lStride);
            // 
            // 				if (SUCCEEDED(hr)) {
            // 					hr = m_draw.DrawFrame(pbScanline0, lStride);
            // 
            // 					av_image_fill_pointers(m_srcFrame->data, (AVPixelFormat)m_srcFrame->format, m_srcFrame->height, pbScanline0, m_srcFrame->linesize);
            // 
            // 					ret = sws_scale(m_swsContext, m_srcFrame->data, m_srcFrame->linesize, 0, m_srcFrame->height, m_dstFrame->data, m_dstFrame->linesize);
            // 
            // 
            // 					if (m_bYUVRecordStatus == TRUE)
            // 					{
            // 						int len = av_image_get_buffer_size((AVPixelFormat)m_dstFrame->format, m_dstFrame->width, m_dstFrame->height, 32);
            // 						yuvfile->write((char *)m_dstFrame->data[0], len);
            // 						count++;
            // 						LOG_INFO("write %d byte data\n", len);
            // 					}
            // 
            // 					if (m_bH264RecordStatus == TRUE)
            // 					{
            // 						static BOOL first = TRUE;
            // 						AVPacket pkt;
            // 						av_init_packet(&pkt);
            // 						pkt.data = NULL;    // packet data will be allocated by the encoder
            // 						pkt.size = 0;
            // 						int got_frame;
            // 						ret = avcodec_encode_video2(m_codecContext, &pkt, m_dstFrame, &got_frame);
            // 						if (ret != 0)
            // 						{
            // 							LOG_ERR("avcodec_encode_video2 error with %d !\n", ret);
            // 						}
            // 
            // 						if (got_frame)
            // 						{
            // 							if (first == TRUE)
            // 							{
            // 								if ((pkt.flags & AV_PKT_FLAG_KEY)) {
            // 									first = FALSE;
            // 									LOG_INFO("get first key frame\n");
            // 									h264file->write((char *)pkt.data, pkt.size);
            // 								}
            // 							}
            // 							else {
            // 								h264file->write((char *)pkt.data, pkt.size);
            // 							}
            // 
            // 							LOG_DEBUG("pkt.pts=%lld pkt.dts=%lld pkt.size=%d !\n", pkt.pts, pkt.dts, pkt.size);
            // 							av_packet_unref(&pkt);
            // 						}
            // 
            // 						m_dstFrame->pts++;
            // 					}
            // 				}
            //
            //			}
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

