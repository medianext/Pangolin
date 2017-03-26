/*
* Author：王德明
* Email：phight@163.com
* QQ群：220954528
*/

#pragma once

#include "Codec.h"

#include "rtmp/rtmp.h"


#define RTMP_STATUS_UNINIT 0
#define RTMP_STATUS_STOP   1
#define RTMP_STATUS_PAUSE  2
#define RTMP_STATUS_START  3


struct RtmpStatistics {
	uint32_t videoTotalSnd;
	uint32_t videoBitrate;
	uint32_t audioTotalSnd;
	uint32_t audioBitrate;
};


struct H264Header {
	unsigned char * sps;
	int spslen;
	unsigned char * pps;
	int ppslen;
};


class Rtmpc
{
public:
    Rtmpc(Codec* codec);
    ~Rtmpc();

public:
	int GetRtmpStatistics(RtmpStatistics* statistics);
    int SetConfig(char* url);
    int GetStatus();
    int Start();
    int Stop();

private:
    int Connect();
    int Disconnect();
    int SetChunkSize(int size);
    int SendMetadata();
    int SendVideoHeader(MediaPacket* packet);
    int SendVideoData(MediaPacket* packet);
    int SendAudioHeader(MediaPacket* packet);
    int SendAudioData(MediaPacket* packet);

	int ParseSpsPps(unsigned char* pdata, int size, H264Header* head);
	int DetectVideoData(unsigned char* pdata, int size, unsigned char **ppdata, int* type);

    static DWORD WINAPI RtmpProcessThread(LPVOID lpParam);

private:
    Codec* m_pCodec  = NULL;
    RTMP*  m_pRtmp   = NULL;
    HANDLE m_pThread = NULL;

    int m_QuitCmd = 0;
    int m_Status  = 0;
	bool m_bConnected = false;
	bool m_bNeedKeyframe = true;
	unsigned int m_uFirstTimestamp = 0;

    char m_Url[256];

	RtmpStatistics m_statistics;

#if REC_STREAM
    ofstream    h264file;
    ofstream    aacfile;
#endif
};

