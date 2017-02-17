#pragma once

#include "Sink.h"
#include "MediaPacket.h"

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
    int SendFrame(MediaFrame * frame, FrameType type);

    int ConfigVideoCodec(VideoCodecAttribute* attribute);
    int ConfigAudioCodec(VideoCodecAttribute* attribute);

    MediaPacket* GetVideoPacket();
    MediaPacket* GetVideoPacket();

private:
    int EncodeVideo(MediaFrame* frame, MediaPacket* packet);
    int EncodeAudio(MediaFrame* frame, MediaPacket* packet);
};

