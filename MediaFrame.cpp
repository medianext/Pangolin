#include "stdafx.h"
#include "MediaFrame.h"
#include "BufferLock.h"


MediaFrame::MediaFrame():
    m_pData(NULL)
{
}


MediaFrame::MediaFrame(IMFMediaBuffer* pBuffer, FrameType type, int arg1, int arg2, int arg3)
{
    this->m_FrameType = type;
    DWORD bufferSize = 0;
    pBuffer->GetCurrentLength(&bufferSize);
    m_dataSize = bufferSize;

    VideoBufferLock buffer(pBuffer);

    if (this->m_FrameType == FRAME_TYPE_VIDEO)
    {
        LONG lStride = (LONG)arg3;
        // Helper object to lock the video buffer.

        // Lock the video buffer. This method returns a pointer to the first scan line
        // in the image, and the stride in bytes.
        buffer.LockBuffer(arg1, arg2, &m_pData, &lStride);
        m_stride = (int)lStride;
        m_width = arg1;
        m_height = arg2;
    }
    else if (this->m_FrameType == FRAME_TYPE_AUDIO) {
        m_samplerate = arg1;
        m_channels = arg2;
        m_bitwide = arg3;
        buffer.LockBuffer(0, 0, &m_pData, NULL);
    }

}


MediaFrame::~MediaFrame()
{
}
