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


//////////////////////////////////////////////////////////////////////////
// private method
//////////////////////////////////////////////////////////////////////////

int Codec::ConfigVideoCodec()
{
	x264_param_t param;
	x264_param_default(&param);
	m_videoEncoder = x264_encoder_open(&param);
	return 0;
}


int Codec::ConfigAudioCodec()
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


//////////////////////////////////////////////////////////////////////////
// inherited method
//////////////////////////////////////////////////////////////////////////

int Codec::SetSourceAttribute(void* attribute, AttributeType type)
{
	if (type == ATTRIBUTE_TYPE_VIDEO)
	{
		VideoCaptureAttribute *pattr = (VideoCaptureAttribute *)attribute;
		m_videoSrcAttribute = *pattr;

	}
	else if (type == ATTRIBUTE_TYPE_AUDIO)
	{
		AudioCaptureAttribute *pattr = (AudioCaptureAttribute *)attribute;
		m_audioSrcAttribute = *pattr;
	}
	return 0;
}


int Codec::SendFrame(MediaFrame * frame)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
// public method
//////////////////////////////////////////////////////////////////////////

int SetVideoCodecAttribute(VideoCodecAttribute* attribute)
{
	return 0;
}


int SetAudioCodecAttribute(AudioCodecAttribute* attribute)
{
	return 0;
}


int Codec::Start()
{
	return 0;
}


int Codec::Pause()
{
	return 0;
}


int Codec::Stop()
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