#pragma once

#include "Codec.h"

#include "rtmp/rtmp.h"


#define RTMP_STATUS_UNINIT 0
#define RTMP_STATUS_STOP   1
#define RTMP_STATUS_PAUSE  2
#define RTMP_STATUS_START  3


class Rtmpc
{
public:
    Rtmpc(Codec* codec);
    ~Rtmpc();

public:
    int SetConfig(char* url);
    int GetStatus();
    int Start();
    int Stop();

private:
    int Connect();
    int Disconnect();
    int SetChunkSize(int size);
    int SendMetadata();
    int SendVideoHeader();
    int SendAudioHeader();
    int SendVideoData();
    int SendAudioData();

    static DWORD WINAPI RtmpProcessThread(LPVOID lpParam);

private:
    Codec* m_pCodec;
    RTMP*  m_pRtmp;
    HANDLE m_pThread;

    int m_QuitCmd;
    int m_Status;
    char m_Url[256];
};

