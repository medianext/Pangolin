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


MediaFrame::MediaFrame(IMFMediaBuffer* pBuffer, FrameType type, int arg1, int arg2, int arg3) :
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
        LONG lStride = (LONG)arg3;
        // Helper object to lock the video buffer.

        // Lock the video buffer. This method returns a pointer to the first scan line
		// in the image, and the stride in bytes.
		pBuffer->Lock(&pData, NULL, NULL);
		memcpy(m_pData, pData, m_dataSize);
		pBuffer->Unlock();
		m_width = arg1;
		m_height = arg2;
		if (lStride<0){
			m_stride = 0 - lStride;
		}else{
			m_stride = lStride;
		}
    }
    else if (this->m_FrameType == FRAME_TYPE_AUDIO) {
        m_samplerate = arg1;
        m_channels = arg2;
        m_bitwide = arg3;
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
