#pragma once

enum FrameType {

    FRAME_TYPE_VIDEO = 0,
    FRAME_TYPE_AUDIO,
};


class MediaFrame
{
public:
    MediaFrame();
    MediaFrame(IMFMediaBuffer*, FrameType type, int arg1, int arg2);
    ~MediaFrame();

private:
    FrameType m_FrameType;
    BYTE  *m_pData;
};

