#include "stdafx.h"
#include "Codec.h"


Codec::Codec()
{
}


Codec::~Codec()
{
}


int Codec::SendFrame(MediaFrame * frame)
{
    return 0;
}


int Codec::ConfigVideoCodec(VideoCodecAttribute* attribute)
{
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