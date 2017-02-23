#pragma once

enum FrameType {

    FRAME_TYPE_VIDEO = 0,
    FRAME_TYPE_AUDIO,
};


class MediaFrame
{
public:
    MediaFrame();
    MediaFrame(IMFMediaBuffer*, FrameType type, int arg1, int arg2, int arg3);
    ~MediaFrame();

public:
    FrameType m_FrameType;
    GUID  m_subtype;
    BYTE  *m_pData;
    int m_stride;
    int m_width;
    int m_height;
};

