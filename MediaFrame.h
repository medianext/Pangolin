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
    // common filed
    FrameType m_FrameType;
    GUID  m_subtype;
    BYTE  *m_pData;
    DWORD m_dataSize;

    // video specific filed
    int m_stride;
    int m_width;
    int m_height;

    // audio specific filed
    int m_samplerate;
    int m_channels;
    int m_bitwide;
};

