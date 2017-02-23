#pragma once

#include "Sink.h"
#include "MediaPacket.h"

#include "x264/x264.h"
#include "fdk-aac/aacenc_lib.h"

struct VideoCodecAttribute 
{
};


struct AudioCodecAttribute
{
};


class Codec :
    public Sink
{

public:
    Codec();
    ~Codec();

public:
    int SetAttribute(void* attribute);
    int SendFrame(MediaFrame * frame);

    int ConfigVideoCodec(VideoCodecAttribute* attribute);
    int ConfigAudioCodec(VideoCodecAttribute* attribute);

    MediaPacket* GetVideoPacket();
    MediaPacket* GetAudioPacket();

private:
    int EncodeVideo(MediaFrame* frame, MediaPacket* packet);
    int EncodeAudio(MediaFrame* frame, MediaPacket* packet);

private:
    HANDLE_AACENCODER        m_audioEncoder;
    x264_t*                  m_videoEncoder;
};

