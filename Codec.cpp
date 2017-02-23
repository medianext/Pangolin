#include "stdafx.h"
#include "Codec.h"


Codec::Codec() :
    m_videoEncoder(NULL),
    m_audioEncoder(NULL)
{
}


Codec::~Codec()
{
    if (m_videoEncoder)
    {
        x264_encoder_close(m_videoEncoder);
    }

    if (m_audioEncoder)
    {
        aacEncClose(&m_audioEncoder);
    }
}


int Codec::SetAttribute(void* attribute)
{
    return 0;
}


int Codec::SendFrame(MediaFrame * frame)
{
    return 0;
}


int Codec::ConfigVideoCodec(VideoCodecAttribute* attribute)
{
    x264_param_t param;
    x264_param_default(&param);
    m_videoEncoder = x264_encoder_open(&param);
    return 0;
}


int Codec::ConfigAudioCodec(VideoCodecAttribute* attribute)
{
    return 0;
}


int Codec::EncodeVideo(MediaFrame* frame, MediaPacket* packet)
{
    return 0;
}


int Codec::EncodeAudio(MediaFrame* frame, MediaPacket* packet)
{
    return 0;
}



MediaPacket* Codec::GetVideoPacket()
{
    return NULL;
}


MediaPacket* Codec::GetAudioPacket()
{
    return NULL;
}