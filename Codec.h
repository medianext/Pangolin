/*
* Author：王德明
* Email：phight@163.com
* QQ群：220954528
*/

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

#define MAX_VIDEO_FRAME 5
#define MAX_VIDEO_PACKET 20

#define MAX_AUDIO_FRAME 5
#define MAX_AUDIO_PACKET 20

struct VideoCodecAttribute 
{
	int width;
	int height;
	int profile;
	int fps;
	int bitrate;
};


struct AudioCodecAttribute
{
	int channel;
    int samplerate;
	int bitwide;
	int profile;
    int bitrate;
};


struct CodecStatistics {
	uint32_t videoFrameCnt;
	uint32_t videoPacketCnt;
	uint32_t videoLostCnt;
	uint32_t videoDecCnt;
	double   videoDecFps;
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

	int GetCodecStatistics(CodecStatistics* statistics);

    int SetVideoCodecAttribute(VideoCodecAttribute* attribute);
    int GetVideoCodecAttribute(const VideoCodecAttribute** attribute);

    int SetAudioCodecAttribute(AudioCodecAttribute* attribute);
    int GetAudioCodecAttribute(const AudioCodecAttribute** attribute);

	int Start();
	int Pause();
	int Stop();

	int GetVideoPacketCount();
	int GetAudioPacketCount();

    MediaPacket* GetVideoPacket();
    MediaPacket* GetAudioPacket();

private:
	HRESULT ChooseConversionFunction(AttributeType type, REFGUID subtype);

	int InitCodec();
	int UninitCodec();

	int AllocMemory();
	int FreeMemory();

	x264_picture_t* PopVideoPicture();
	void PushVideoPicture(x264_picture_t* pic);

	MediaPacket* PopVideoPacket();
	void PushVideoPacket(MediaPacket* packet);

	MediaFrame* PopAudioFrame();
	void PushAudioFrame(MediaFrame* frame);

	MediaPacket* PopAudioPacket();
	void PushAudioPacket(MediaPacket* packet);

	int ConfigVideoCodec();
	int ConfigAudioCodec();

	static DWORD WINAPI VideoEncodecThread(LPVOID lpParam);
	static DWORD WINAPI AudioEncodecThread(LPVOID lpParam);

private:
	int                      m_Status = 0;
	int                      m_QuitCmd = 0;

	HANDLE                   m_videoThread = NULL;
	HANDLE                   m_audioThread = NULL;

	VideoCaptureAttribute    m_videoSrcAttribute = { 0 };
	AudioCaptureAttribute    m_audioSrcAttribute = { 0 };

	VideoCodecAttribute      m_videoAttribute = { 0 };
	AudioCodecAttribute      m_audioAttribute = { 0 };

    HANDLE_AACENCODER        m_audioEncoder = NULL;
    x264_t*                  m_videoEncoder = NULL;

	IMAGE_TRANSFORM_FN      m_videoConvertFn;
	AUDIO_TRANSFORM_FN      m_audioConvertFn;

	CRITICAL_SECTION        m_vfMtx;
	CRITICAL_SECTION        m_vpMtx;
	CRITICAL_SECTION        m_afMtx;
	CRITICAL_SECTION        m_apMtx;

	queue<x264_picture_t *> videoFrameQueue;
	queue<MediaPacket *> videoPacketQueue;

	queue<MediaFrame *> audioFrameQueue;
	queue<MediaPacket *> audioPacketQueue;

	//Statistics
	uint32_t m_videoLostCnt    = 0;
	uint32_t m_videoDecCnt     = 0;
	double   m_videoDecFps     = 0;

#if REC_CODEC_RAW
    ofstream      m_pcmfile;
    ofstream      m_yuvfile;
#endif
};

