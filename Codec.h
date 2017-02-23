#pragma once

#include "Sink.h"
#include "MediaPacket.h"

#include "x264/x264.h"
#include "fdk-aac/aacenc_lib.h"

#define H264_PROFILE_HIGH 0
#define H264_PROFILE_MAIN 1
#define H264_PROFILE_LOW  2

#define AAC_PROFILE_LC   0
#define AAC_PROFILE_HE   1
#define AAC_PROFILE_HEV2 2

#define CODEC_STATUS_UNINIT 0
#define CODEC_STATUS_STOP   1
#define CODEC_STATUS_PAUSE  2
#define CODEC_STATUS_START  3

struct VideoCodecAttribute 
{
	int profile;
	int width;
	int height;
	int fps;
	int bitrate;
};


struct AudioCodecAttribute
{
	int profile;
	int bitrate;
};


class Codec :
    public Sink
{

public:
    Codec();
    ~Codec();

public:
    int SetSourceAttribute(void* attribute, AttributeType type);
    int SendFrame(MediaFrame * frame);

	int SetVideoCodecAttribute(VideoCodecAttribute* attribute);
	int SetAudioCodecAttribute(AudioCodecAttribute* attribute);

	int Start();
	int Pause();
	int Stop();

    MediaPacket* GetVideoPacket();
    MediaPacket* GetAudioPacket();

private:
	int ConfigVideoCodec();
	int ConfigAudioCodec();

    int EncodeVideo(MediaFrame* frame, MediaPacket* packet);
    int EncodeAudio(MediaFrame* frame, MediaPacket* packet);

private:
	int                      m_Status;

	VideoCaptureAttribute    m_videoSrcAttribute;
	AudioCaptureAttribute    m_audioSrcAttribute;

	VideoCodecAttribute      m_videoAttribute;
	AudioCodecAttribute      m_audioAttribute;

    HANDLE_AACENCODER        m_audioEncoder;
    x264_t*                  m_videoEncoder;
};

