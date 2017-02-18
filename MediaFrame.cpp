#include "stdafx.h"
#include "MediaFrame.h"
#include "BufferLock.h"


MediaFrame::MediaFrame():
    m_pData(NULL)
{
}


MediaFrame::MediaFrame(IMFMediaBuffer* pBuffer, FrameType type, int arg1, int arg2)
{
    this->m_FrameType = type;
    LONG lStride;
    // Helper object to lock the video buffer.
    VideoBufferLock buffer(pBuffer);

    // Lock the video buffer. This method returns a pointer to the first scan line
    // in the image, and the stride in bytes.
    buffer.LockBuffer(arg1, arg2, &m_pData, &lStride);

}


MediaFrame::~MediaFrame()
{
}
