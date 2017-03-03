#include "stdafx.h"
#include "Rtmpc.h"


//////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////

Rtmpc::Rtmpc(Codec* codec) :
    m_pCodec(codec),
    m_pRtmp(NULL),
    m_pThread(NULL),
    m_Status(0)
{
}


Rtmpc::~Rtmpc()
{
}


//////////////////////////////////////////////////////////////////////////
// private method
//////////////////////////////////////////////////////////////////////////

int Rtmpc::Connect()
{
    int ret = 0;

    m_pRtmp = RTMP_Alloc();;
    if (m_pRtmp == NULL)
    {
        OutputDebugString(TEXT("RTMP_Alloc failed!\n"));
        return -1;
    }

    RTMP_Init(m_pRtmp);

    ret = RTMP_SetupURL(m_pRtmp, m_Url);
    if (ret==0)
    {
        OutputDebugString(TEXT("Set rtmp url failed!\n"));
        return -2;
    }

    RTMP_EnableWrite(m_pRtmp);

    ret = RTMP_Connect(m_pRtmp, NULL);
    if (ret == 0)
    {
        OutputDebugString(TEXT("Rtmp connect failed!\n"));
        return -2;
    }

    ret = RTMP_ConnectStream(m_pRtmp, 0);
    if (ret == 0)
    {
        OutputDebugString(TEXT("Rtmp connect stream failed!\n"));
        return -3;
    }

    ret = SetChunkSize(1024);
    if (ret != 0)
    {
        OutputDebugString(TEXT("Rtmp set chunksize failed!\n"));
        return -4;
    }

    return 0;
}


int Rtmpc::Disconnect()
{
    if (m_pRtmp)
    {
        RTMP_Close(m_pRtmp);

        RTMP_Free(m_pRtmp);
        m_pRtmp = NULL;
    }

    return 0;
}


int Rtmpc::SetChunkSize(int size)
{
    int maxBodySize = 4;
    RTMPPacket pkt = {0};
    if (RTMPPacket_Alloc(&pkt, maxBodySize) == 0)
    {
        return -1;
    }

    char* ptr  = pkt.m_body;
    char* pend = pkt.m_body + maxBodySize;

    ptr = AMF_EncodeInt32(ptr, pend, size);

    pkt.m_nChannel = 0x02;
    pkt.m_headerType = RTMP_PACKET_SIZE_LARGE;
    pkt.m_packetType = RTMP_PACKET_TYPE_CHUNK_SIZE;
    pkt.m_nInfoField2 = 0;
    pkt.m_hasAbsTimestamp = 0;
    pkt.m_nTimeStamp = 0;
    pkt.m_nBodySize = ptr - pkt.m_body;

    if (RTMP_SendPacket(m_pRtmp, &pkt, 1) == 0) 
    {
        return -1;
    }

    RTMPPacket_Free(&pkt);

    return 0;
}


int Rtmpc::SendMetadata()
{
    int maxBodySize = 256;
    RTMPPacket pkt = { 0 };
    if (RTMPPacket_Alloc(&pkt, maxBodySize) == 0)
    {
        return -1;
    }

    char* ptr = pkt.m_body;
    char* pend = pkt.m_body + maxBodySize;

    const VideoCodecAttribute * pVideoAttr = NULL;
    m_pCodec->GetVideoCodecAttribute(&pVideoAttr);

    AMFObjectProperty prop[] = {
        { { "width",strlen("width") }, AMF_NUMBER, pVideoAttr->width},
        { { "height",strlen("height") }, AMF_NUMBER, pVideoAttr->height },
        { { "framerate",strlen("framerate") }, AMF_NUMBER, pVideoAttr->fps },
        { { "videocodeid",strlen("videocodeid") }, AMF_NUMBER, 7 },
        { { "videodatarate",strlen("videodatarate") }, AMF_NUMBER, pVideoAttr->bitrate },
    };
    AMFObject obj = {
        sizeof(prop)/sizeof(AMFObjectProperty), prop
    };

    AVal val = { "onMetaData", strlen("onMetaData") };
    ptr = AMF_EncodeString(ptr, pend, &val);
    ptr = AMF_EncodeEcmaArray(&obj, ptr, pend);

    pkt.m_nChannel = 0x04;
    pkt.m_headerType = RTMP_PACKET_SIZE_LARGE;
    pkt.m_packetType = RTMP_PACKET_TYPE_INFO;
    pkt.m_nInfoField2 = m_pRtmp->m_stream_id;
    pkt.m_hasAbsTimestamp = 0;
    pkt.m_nTimeStamp = 0;
    pkt.m_nBodySize = ptr - pkt.m_body;

    if (RTMP_SendPacket(m_pRtmp, &pkt, 1) == 0)
    {
        return -1;
    }

    RTMPPacket_Free(&pkt);

    return 0;
}


int Rtmpc::SendVideoHeader()
{
    return 0;
}


int Rtmpc::SendAudioHeader()
{
    return 0;
}


int Rtmpc::SendVideoData()
{
    return 0;
}


int Rtmpc::SendAudioData()
{
    return 0;
}


DWORD WINAPI Rtmpc::RtmpProcessThread(LPVOID lpParam)
{
    Rtmpc* rtmpc = (Rtmpc*)lpParam;

    if (rtmpc->Connect()<0)
    {
        rtmpc->Disconnect();
        rtmpc->m_Status = RTMP_STATUS_STOP;
        return 1;
    }

    rtmpc->SendMetadata();

    while (1)
    {
        if (rtmpc->m_QuitCmd == 1)
        {
            break;
        }

        Sleep(100);
    }

    rtmpc->Disconnect();

    return 0;
}

//////////////////////////////////////////////////////////////////////////
// public method
//////////////////////////////////////////////////////////////////////////

int Rtmpc::SetConfig(char* url)
{
    if ( url==NULL )
    {
        return -1;
    }

    unsigned int len = (unsigned int)strlen(url);
    if (len <= 0 || len >= 256)
    {
        return -1;
    }

    strcpy(m_Url, url);

    return 0;
}


int Rtmpc::GetStatus()
{
    return m_Status;
}


int Rtmpc::Start()
{
    m_QuitCmd = 0;
    m_pThread = CreateThread(NULL, 0, RtmpProcessThread, this, 0, NULL);
    m_Status = RTMP_STATUS_START;
    return 0;
}


int Rtmpc::Stop()
{
    m_Status = RTMP_STATUS_STOP;
    m_QuitCmd = 1;
    WaitForSingleObject(m_pThread, INFINITE);
    return 0;
}
