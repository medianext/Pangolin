/*
* Author：王德明
* Email：phight@163.com
* QQ群：220954528
*/

#include "stdafx.h"
#include "MediaFrame.h"


MediaFrame::MediaFrame():
    m_pData(NULL),
	m_uTimestamp(0)
{
}


MediaFrame::MediaFrame(FrameType frameType, GUID  subtype, DWORD dataSize)
{
	m_pData = (BYTE*)malloc(dataSize);
	if (m_pData!=NULL)
	{
		m_dataSize = dataSize;
		m_FrameType = frameType;
		m_subtype = subtype;
	}
}


MediaFrame::MediaFrame(IMFMediaBuffer* pBuffer, FrameType type, void* attribute) :
	m_uTimestamp(0)
{
    this->m_FrameType = type;
	DWORD bufferSize = 0;
	BYTE* pData = NULL;
    pBuffer->GetCurrentLength(&bufferSize);
    m_dataSize = bufferSize;
	m_pData = (BYTE*)malloc(m_dataSize);

    if (this->m_FrameType == FRAME_TYPE_VIDEO)
    {
		VideoCaptureAttribute* pattr = (VideoCaptureAttribute*)attribute;
        LONG lStride = (LONG)pattr->stride;
        // Helper object to lock the video buffer.

        // Lock the video buffer. This method returns a pointer to the first scan line
		// in the image, and the stride in bytes.
		pBuffer->Lock(&pData, NULL, NULL);
		memcpy(m_pData, pData, m_dataSize);
		pBuffer->Unlock();
		m_subtype = pattr->format;
		m_width = pattr->width;
		m_height = pattr->height;
		if (lStride<0){
			m_stride = 0 - lStride;
			Inversion();
		}else{
			m_stride = lStride;
		}
    }
	else if (this->m_FrameType == FRAME_TYPE_AUDIO) {
		AudioCaptureAttribute* pattr = (AudioCaptureAttribute*)attribute;
		m_subtype = pattr->format;
        m_samplerate = pattr->samplerate;
        m_channels = pattr->channel;
        m_bitwide = pattr->bitwide;
		pBuffer->Lock(&pData, NULL, NULL);
		memcpy(m_pData, pData, m_dataSize);
		pBuffer->Unlock();
    }

}


MediaFrame::~MediaFrame()
{
	if (m_pData)
	{
		free(m_pData);
	}
}


void MediaFrame::Inversion()
{
	BYTE *pData = (BYTE*)malloc(m_dataSize);

	for (int i = 0; i < m_height; i++)
	{
		BYTE *pDst = &pData[i*m_stride];
		BYTE *pSrc = &m_pData[(m_height - i - 1)*m_stride];
		memcpy(pDst, pSrc, m_stride);
	}
	free(m_pData);
	m_pData = pData;
}