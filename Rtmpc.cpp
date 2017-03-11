#include "stdafx.h"
#include "Rtmpc.h"


//////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////

Rtmpc::Rtmpc(Codec* codec) :
    m_pCodec(codec),
    m_pRtmp(NULL),
    m_pThread(NULL),
    m_Status(0),
	m_bConnected(false),
	m_bNeedKeyframe(true)
{
}


Rtmpc::~Rtmpc()
{
}


//////////////////////////////////////////////////////////////////////////
// private method
//////////////////////////////////////////////////////////////////////////

int Rtmpc::ParseSpsPps(unsigned char* pdata, int size, H264Header* head)
{
	//sps
	int i = 0;
	while (i<size)
	{
		int tmp = i;
		if (size-i>4 && pdata[i++]==0x00 && pdata[i++] == 0x00 && ( pdata[i++] == 0x01 || (pdata[i] == 0x00 && pdata[i++] == 0x01)))
		{
			int start = i;
			while (i<size)
			{
				int tmp2 = i;
				if (size - i>4 && pdata[i++] == 0x00 && pdata[i++] == 0x00 && (pdata[i] == 0x01 || (pdata[i] == 0x00 && pdata[i+1] == 0x01)))
				{
					i = tmp2;
					break;
				}
				else {
					i = tmp2 + 1;
				}
			}

			if ((pdata[start] & 0x1F) == 7)
			{
				head->sps = &pdata[start];
				head->spslen = i - start;
				break;
			}
		}
		else {
			i = tmp + 1;
		}

	}

	//pps
	i = 0;
	while (i < size)
	{
		int tmp = i;
		if (size - i>4 && pdata[i++] == 0x00 && pdata[i++] == 0x00 && (pdata[i++] == 0x01 || (pdata[i] == 0x00 && pdata[i++] == 0x01)))
		{
			int start = i;
			while (i < size)
			{
				int tmp2 = i;
				if (size - i>4 && pdata[i++] == 0x00 && pdata[i++] == 0x00 && (pdata[i] == 0x01 || (pdata[i] == 0x00 && pdata[i + 1] == 0x01)))
				{
					i = tmp2;
					break;
				}
				else {
					i = tmp2 + 1;
				}
			}

			if ((pdata[start] & 0x1F) == 8)
			{
				head->pps = &pdata[start];
				head->ppslen = i - start;
				break;
			}
		}
		else {
			i = tmp + 1;
		}

	}
	return 0;
}


int Rtmpc::DetectVideoData(unsigned char* pdata, int size, unsigned char **ppdata, int* type)
{

	int i = 0;
	while (i < size)
	{
		int tmp = i;
		if (size - i>4 && pdata[i++] == 0x00 && pdata[i++] == 0x00 && (pdata[i++] == 0x01 || (pdata[i] == 0x00 && pdata[i++] == 0x01)))
		{
			int start = i;
			while (i < size)
			{
				int tmp2 = i;
				if (size - i>4 && pdata[i++] == 0x00 && pdata[i++] == 0x00 && (pdata[i] == 0x01 || (pdata[i] == 0x00 && pdata[i + 1] == 0x01)))
				{
					i = tmp2;
					break;
				}
				else {
					i = tmp2 + 1;
				}
			}

//  			if ((pdata[start] & 0x1F) <= 5)
//  			{
				*ppdata = &pdata[start];
				if ((pdata[start] & 0x1F) >= 5)
				{
					*type = 1;
				}
				return i - start;
//			}
		}
		else {
			i = tmp + 1;
		}

	}
	return 0;
}


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

	m_pRtmp->m_outChunkSize = size;

	if (!RTMP_IsConnected(m_pRtmp))
	{
		return -1;
	}

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

	if (!RTMP_IsConnected(m_pRtmp))
	{
		return -1;
	}

    if (RTMP_SendPacket(m_pRtmp, &pkt, 1) == 0)
    {
        return -1;
    }

    RTMPPacket_Free(&pkt);

    return 0;
}


int Rtmpc::SendVideoHeader(unsigned char * pdata, int size)
{
	if (pdata == NULL|| size <=0)
	{
		return -1;
	}

	H264Header head = { 0 };
	if (ParseSpsPps(pdata, size, &head) < 0)
	{
		return -2;
	}

	int maxBodySize = 100;
	RTMPPacket pkt = { 0 };
	if (RTMPPacket_Alloc(&pkt, maxBodySize) == 0)
	{
		return -1;
	}

	char* ptr = pkt.m_body;
	char* pend = pkt.m_body + maxBodySize;

	*ptr++ = 0x17;
	*ptr++ = 0x00;
	*ptr++ = 0x00;
	*ptr++ = 0x00;
	*ptr++ = 0x00;

	*ptr++ = 0x01;
	*ptr++ = head.sps[1];
	*ptr++ = head.sps[2];
	*ptr++ = head.sps[3];
	*ptr++ = 0xff;

	//sps
	*ptr++ = 0xE1;
	*ptr++ = (head.spslen >> 8) & 0xff;
	*ptr++ = head.spslen & 0xff;
	memcpy(ptr, head.sps, head.spslen);
	ptr += head.spslen;

	//pps
	*ptr++ = 0x01;
	*ptr++ = (head.ppslen >> 8) & 0xff;
	*ptr++ = head.ppslen & 0xff;
	memcpy(ptr, head.pps, head.ppslen);
	ptr += head.ppslen;

	pkt.m_nChannel = 0x04;
	pkt.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	pkt.m_packetType = RTMP_PACKET_TYPE_VIDEO;
	pkt.m_nInfoField2 = 0;
	pkt.m_hasAbsTimestamp = 0;
	pkt.m_nTimeStamp = 0;
	pkt.m_nBodySize = ptr - pkt.m_body;

	if (!RTMP_IsConnected(m_pRtmp))
	{
		return -1;
	}

	if (RTMP_SendPacket(m_pRtmp, &pkt, 1) == 0)
	{
		return -1;
	}

	RTMPPacket_Free(&pkt);

    return 0;
}


int Rtmpc::SendAudioHeader()
{
    return 0;
}


int Rtmpc::SendVideoData(MediaPacket* packet)
{
	if (packet==NULL || packet->m_pData == NULL || packet->m_dataSize <= 0)
	{
		return -1;
	}

	unsigned char* prealdata = NULL;
	int realsize = 0;
	int type = 0;

	realsize = DetectVideoData(packet->m_pData, packet->m_dataSize, &prealdata, &type);
	if (prealdata==NULL)
	{
		return -2;
	}

	int maxBodySize = packet->m_dataSize + 9;
	RTMPPacket pkt = { 0 };
	if (RTMPPacket_Alloc(&pkt, maxBodySize) == 0)
	{
		return -3;
	}

	char* ptr = pkt.m_body;
	char* pend = pkt.m_body + maxBodySize;

	if (type){
		*ptr++ = 0x17;
	}
	else {
		*ptr++ = 0x27;
	}
	*ptr++ = 0x01;
	*ptr++ = 0x00;
	*ptr++ = 0x00;
	*ptr++ = 0x00;

	*ptr++ = (realsize >> 24) & 0xff;
	*ptr++ = (realsize >> 16) & 0xff;
	*ptr++ = (realsize >> 8) & 0xff;
	*ptr++ = realsize & 0xff;

	memcpy(ptr, prealdata, realsize);

	ptr += realsize;

	pkt.m_nChannel = 0x04;
	pkt.m_headerType = RTMP_PACKET_SIZE_LARGE;
	pkt.m_packetType = RTMP_PACKET_TYPE_VIDEO;
	pkt.m_nInfoField2 = 0;
	pkt.m_hasAbsTimestamp = 0;
	pkt.m_nTimeStamp = packet->m_uTimestamp - m_uFirstTimestamp;
	pkt.m_nBodySize = ptr - pkt.m_body;

	if (!RTMP_IsConnected(m_pRtmp))
	{
		return -4;
	}

	if (RTMP_SendPacket(m_pRtmp, &pkt, 1) == 0)
	{
		return -5;
	}

	RTMPPacket_Free(&pkt);

	return 0;
}


int Rtmpc::SendAudioData(MediaPacket* packet)
{
    return 0;
}


DWORD WINAPI Rtmpc::RtmpProcessThread(LPVOID lpParam)
{
    Rtmpc* rtmpc = (Rtmpc*)lpParam;
	int ret = 0;

    if (rtmpc->Connect()<0)
    {
        rtmpc->Disconnect();
        rtmpc->m_Status = RTMP_STATUS_STOP;
        return 1;
    }

    ret = rtmpc->SendMetadata();
	if (ret < 0)
	{
		OutputDebugString(TEXT("Rtmpc send metadata failed!\n"));
	}

	bool bFirst = true;

    while (1)
    {
        if (rtmpc->m_QuitCmd == 1)
        {
            break;
        }

        // Send video
		MediaPacket* videoPacket = rtmpc->m_pCodec->GetVideoPacket();
		if (videoPacket ==NULL)
		{
            if (bFirst)
            {
                Sleep(10);
                continue;
            }
            else
            {
                goto DoAudio;
            }
		}

		if (bFirst && videoPacket->m_bKeyframe)
		{
			//rtmpc->SendAudioHeader();
			rtmpc->m_uFirstTimestamp = videoPacket->m_uTimestamp;
			bFirst = false;
		}else if (bFirst && !videoPacket->m_bKeyframe)
		{
			delete videoPacket;
			continue;
		}

#if REC_STREAM
        if (rtmpc->h264file.is_open())
        {
            rtmpc->h264file.write((char *)videoPacket->m_pData, videoPacket->m_dataSize);
        }
#endif

		if (rtmpc->m_bNeedKeyframe && !videoPacket->m_bKeyframe)
		{
			delete videoPacket;
			continue;
		}else if (rtmpc->m_bNeedKeyframe && videoPacket->m_bKeyframe)
		{
			rtmpc->m_bNeedKeyframe = false;
		}

        if (videoPacket->m_bKeyframe)
        {
            ret = rtmpc->SendVideoHeader(videoPacket->m_pData, videoPacket->m_dataSize);
            if (ret < 0)
            {
                OutputDebugString(TEXT("Rtmpc send video head failed!\n"));
            }
        }

		ret = rtmpc->SendVideoData(videoPacket);
		if (ret < 0)
		{
			OutputDebugString(TEXT("Rtmpc send video data failed!\n"));
		}

        delete videoPacket;

        // Send audio
DoAudio:
        MediaPacket* audioPacket = rtmpc->m_pCodec->GetAudioPacket();
        if (audioPacket == NULL)
        {
            Sleep(10);
            continue;
        }

#if REC_STREAM
        if (rtmpc->aacfile.is_open())
        {
            rtmpc->aacfile.write((char *)audioPacket->m_pData, audioPacket->m_dataSize);
        }
#endif
        
        delete audioPacket;
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

#if REC_STREAM
    char filename[256];
    SYSTEMTIME time;
    GetLocalTime(&time);
    snprintf(filename, 256, "video[%04d%02d%02d-%02d%02d%02d].h264", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
    h264file.open(filename, ios::binary | ios::out);
    snprintf(filename, 256, "audio[%04d%02d%02d-%02d%02d%02d].aac", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
    aacfile.open(filename, ios::binary | ios::out);
#endif

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

#if REC_STREAM
    if (h264file.is_open())
    {
        h264file.flush();
        h264file.close();
    }
    if (aacfile.is_open())
    {
        aacfile.flush();
        aacfile.close();
    }
#endif

    return 0;
}
