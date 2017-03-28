/*
* Author：王德明
* Email：phight@163.com
* QQ群：220954528
*/

#include "stdafx.h"
#include "Codec.h"


//-------------------------------------------------------------------
//
// Video Conversion functions
//
//-------------------------------------------------------------------

__forceinline BYTE Clip(int clr)
{
    return (BYTE)(clr < 0 ? 0 : (clr > 255 ? 255 : clr));
}

typedef struct tagYUVQUAD {
	BYTE    y;
	BYTE    u;
	BYTE    v;
} YUVQUAD;

__forceinline YUVQUAD ConvertRGBToYCrCb(
	int r,
	int g,
	int b
	)
{
	YUVQUAD yuv;

	yuv.y = Clip(((66 * r + 129 * g + 25 * b + 128) >> 8) + 16);
	yuv.u = Clip(((112 * r - 94 * g - 18 * b + 128) >> 8) + 128);
	yuv.v = Clip(((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128);

	return yuv;
}


//-------------------------------------------------------------------
// TransformImage_RGB24 
//
// RGB-24 to I420
//-------------------------------------------------------------------

static void TransformImage_RGB24(
    BYTE*       pDst,
    LONG        lDstStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    )
{
	BYTE* lpDstBitsY = pDst;
	BYTE* lpDstBitsCb = lpDstBitsY + (dwHeightInPixels * lDstStride);
	BYTE* lpDstBitsCr = lpDstBitsCb + (dwHeightInPixels * lDstStride) / 4;

	const BYTE* lpRGB = pSrc;

	for (UINT x = 0; x < dwHeightInPixels; x++)
	{
		const BYTE* lpRGBSel = lpRGB;
		for (UINT y = 0; y < dwWidthInPixels; y++)
		{
			int red = *(lpRGBSel++);
			int green = *(lpRGBSel++);
			int blue = *(lpRGBSel++);

			YUVQUAD yuv = ConvertRGBToYCrCb(red, green, blue);
			*(lpDstBitsY++) = yuv.y;
			if (x%2==0 && y%2==0)
			{
				*(lpDstBitsCb++) = yuv.u;
				*(lpDstBitsCr++) = yuv.v;
			}
		}

		lpRGB += lSrcStride;
	}
}

//-------------------------------------------------------------------
// TransformImage_RGB32
//
// RGB-32 to I420
//-------------------------------------------------------------------

static void TransformImage_RGB32(
    BYTE*       pDst,
    LONG        lDstStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    )
{
	BYTE* lpDstBitsY = pDst;
	BYTE* lpDstBitsCb = lpDstBitsY + (dwHeightInPixels * lDstStride);
	BYTE* lpDstBitsCr = lpDstBitsCb + (dwHeightInPixels * lDstStride) / 4;

	const BYTE* lpRGB = pSrc;

	for (UINT x = 0; x < dwHeightInPixels; x++)
	{
		const BYTE* lpRGBSel = lpRGB;
		for (UINT y = 0; y < dwWidthInPixels; y++)
		{
			int red = *(lpRGBSel++);
			int green = *(lpRGBSel++);
			int blue = *(lpRGBSel++);

			YUVQUAD yuv = ConvertRGBToYCrCb(red, green, blue);
			*(lpDstBitsY++) = yuv.y;
			if (x % 2 == 0 && y % 2 == 0)
			{
				*(lpDstBitsCb++) = yuv.u;
				*(lpDstBitsCr++) = yuv.v;
			}
		}

		lpRGB += lSrcStride;
	}
}

//-------------------------------------------------------------------
// TransformImage_YUY2 
//
// YUY2 to I420
//-------------------------------------------------------------------

static void TransformImage_YUY2(
    BYTE*       pDst,
    LONG        lDstStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    )
{
	BYTE* lpDstBitsY = pDst;
	BYTE* lpDstBitsCb = lpDstBitsY + (dwHeightInPixels * lDstStride);
	BYTE* lpDstBitsCr = lpDstBitsCb + (dwHeightInPixels * lDstStride) / 4;

    for (DWORD y = 0; y < dwHeightInPixels; y++)
	{
        WORD *pSrcPel = (WORD*)pSrc;

        for (DWORD x = 0; x < dwWidthInPixels; x+=2)
        {
            // Byte order is U0 Y0 V0 Y1

            int y0 = (int)LOBYTE(pSrcPel[x]);
            int u0 = (int)HIBYTE(pSrcPel[x]);
            int y1 = (int)LOBYTE(pSrcPel[x + 1]);
            int v0 = (int)HIBYTE(pSrcPel[x + 1]);

			*(lpDstBitsY++) = y0;
			*(lpDstBitsY++) = y1;
			if (x % 2 == 0 && y % 2 == 0)
			{
				*(lpDstBitsCb++) = u0;
				*(lpDstBitsCr++) = v0;
			}
        }

        pSrc += lSrcStride;
    }

}


//-------------------------------------------------------------------
// TransformImage_I420
//
// I420 to I420
//-------------------------------------------------------------------

static void TransformImage_I420(
    BYTE* pDst,
    LONG lDstStride,
    const BYTE* pSrc,
    LONG lSrcStride,
    DWORD dwWidthInPixels,
    DWORD dwHeightInPixels
    )
{
    MFCopyImage(pDst, lDstStride, pSrc, lSrcStride, dwWidthInPixels * 3 / 2, dwHeightInPixels);
}


//-------------------------------------------------------------------
// TransformImage_NV12
//
// NV12 to I420
//-------------------------------------------------------------------

static void TransformImage_NV12(
    BYTE* pDst,
    LONG lDstStride,
    const BYTE* pSrc,
    LONG lSrcStride,
    DWORD dwWidthInPixels,
    DWORD dwHeightInPixels
    )
{
    BYTE* lpDstBitsY = pDst;
    BYTE* lpDstBitsCb = lpDstBitsY + (dwHeightInPixels * lDstStride);
    BYTE* lpDstBitsCr = lpDstBitsCb + (dwHeightInPixels * lDstStride) / 4;

    const BYTE* lpSrcBitsY = pSrc;
    const BYTE* lpSrcBitsCb = lpSrcBitsY + (dwHeightInPixels * lDstStride);
    //const BYTE* lpSrcBitsCr = lpSrcBitsCb + 1;

	for (DWORD y = 0; y < dwHeightInPixels; y++)
	{
		WORD *pSrcPel = (WORD*)pSrc;

		for (DWORD x = 0; x < dwWidthInPixels; x++ )
		{

			*(lpDstBitsY++) = *(lpSrcBitsY++);

			if (x % 2 == 0 && y % 2 == 0)
			{
                *(lpDstBitsCb++) =  *(lpSrcBitsCb++);
                *(lpDstBitsCr++) =  *(lpSrcBitsCb++);
			}
		}

		pSrc += lSrcStride;
	}
}


static VideoConversionFunction g_VideoConversions[] =
{
    { MFVideoFormat_RGB32, TransformImage_RGB32 },
    { MFVideoFormat_RGB24, TransformImage_RGB24 },
    { MFVideoFormat_YUY2,  TransformImage_YUY2 },
    { MFVideoFormat_I420,  TransformImage_I420 },
    { MFVideoFormat_NV12,  TransformImage_NV12 }
};

const DWORD   g_cVideoFormats = ARRAYSIZE(g_VideoConversions);


//-------------------------------------------------------------------
//
// Audio Conversion functions
//
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// TransformImage_FLTP
//
// FLTP to S16
//-------------------------------------------------------------------

static void TransformImage_FLTP(MediaFrame* srcFrame, MediaFrame* dstFrame)
{
	short* dstData = (short*)dstFrame->m_pData;
	float* srcData = (float*)srcFrame->m_pData;
	int sampleCnt = dstFrame->m_dataSize * 8 / dstFrame->m_bitwide;
	if (srcFrame->m_channels==dstFrame->m_channels)
    {
        for (int i = 0; i < sampleCnt; i++)
        {
            *dstData++ = (short)((*srcData++) * 32767.0f);
        }
	}else if (srcFrame->m_channels==1 && dstFrame->m_channels==2)
	{
		for (int i = 0; i < sampleCnt; i++)
		{
			*dstData++ = (short)((*srcData++) * 32767.0f);
			*dstData++ = *dstData;
		}
	}else if (srcFrame->m_channels == 2 && dstFrame->m_channels == 1)
	{
	}
}


static AudioConversionFunction g_AudioConversions[] =
{
	{ MFAudioFormat_Float,  TransformImage_FLTP }
};

const DWORD   g_cAudioFormats = ARRAYSIZE(g_AudioConversions);



//////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////

Codec::Codec() :
    m_videoEncoder(NULL),
    m_audioEncoder(NULL),
    m_Status(0)
{
	InitializeCriticalSection(&m_vfMtx);
	InitializeCriticalSection(&m_vpMtx);
	InitializeCriticalSection(&m_afMtx);
	InitializeCriticalSection(&m_apMtx);
}


Codec::~Codec()
{

	this->Stop();

	DeleteCriticalSection(&m_vfMtx);
	DeleteCriticalSection(&m_vpMtx);
	DeleteCriticalSection(&m_afMtx);
	DeleteCriticalSection(&m_apMtx);
}


//////////////////////////////////////////////////////////////////////////
// private method
//////////////////////////////////////////////////////////////////////////

HRESULT Codec::ChooseConversionFunction(AttributeType type, REFGUID subtype)
{
	if (type == ATTRIBUTE_TYPE_VIDEO)
	{
		m_videoConvertFn = NULL;

		for (DWORD i = 0; i < g_cVideoFormats; i++)
		{
			if (g_VideoConversions[i].subtype == subtype)
			{
				m_videoConvertFn = g_VideoConversions[i].xform;
				return S_OK;
			}
		}
		return MF_E_INVALIDMEDIATYPE;

	}else if (type == ATTRIBUTE_TYPE_AUDIO)
	{
		m_audioConvertFn = NULL;

		for (DWORD i = 0; i < g_cAudioFormats; i++)
		{
			if (g_AudioConversions[i].subtype == subtype)
			{
				m_audioConvertFn = g_AudioConversions[i].xform;
				return S_OK;
			}
		}
		return MF_E_INVALIDMEDIATYPE;

	}

	return MF_E_INVALIDTYPE;
}


int Codec::ConfigVideoCodec()
{
    int ret = 0;
	x264_param_t param;

    ret = x264_param_default_preset(&param, "veryfast","zerolatency");

    param.i_threads = 1;
    param.i_csp = X264_CSP_I420;
    param.i_width = m_videoAttribute.width;
    param.i_height = m_videoAttribute.height;
	param.b_vfr_input = 0;
    param.i_fps_num = m_videoAttribute.fps;
    param.i_fps_den = 1;
	param.i_keyint_max = m_videoAttribute.fps;
	param.i_keyint_min = 5;
    param.b_intra_refresh = 0;
    param.b_annexb = 1;

    param.rc.i_rc_method = X264_RC_ABR;
    param.rc.i_bitrate = m_videoAttribute.bitrate;

	switch (m_videoAttribute.profile)
	{
	case 0:
		ret = x264_param_apply_profile(&param, "high");
		break;
	case 1:
		ret = x264_param_apply_profile(&param, "main");
		break;
	case 2:
		ret = x264_param_apply_profile(&param, "baseline");
		break;
	default:
		ret = x264_param_apply_profile(&param, "high");
	}

	m_videoEncoder = x264_encoder_open(&param);

	if (m_videoEncoder)
	{
		x264_encoder_parameters(m_videoEncoder, &param);
		return 0;
	}
	else
	{
		return -1;
	}

}


int Codec::ConfigAudioCodec()
{
	if (aacEncOpen(&m_audioEncoder, 0, m_audioAttribute.channel) != AACENC_OK) {
		OutputDebugString(TEXT("Open AAC encode handle failed!\n"));
        return -1;
    }
    int aot = 2;
    switch (m_audioAttribute.profile)
    {
    case 0: aot = AOT_AAC_LC;  break;
    case 1: aot = AOT_SBR;  break;
    case 2: aot = AOT_PS; break;
    }
    if (aacEncoder_SetParam(m_audioEncoder, AACENC_AOT, aot) != AACENC_OK) {
        OutputDebugString(TEXT("Unable to set the AOT\n"));
        return -1;
    }
    if (aacEncoder_SetParam(m_audioEncoder, AACENC_SAMPLERATE, m_audioAttribute.samplerate) != AACENC_OK) {
        OutputDebugString(TEXT("Unable to set the SAMPLERATE\n"));
        return -1;
    }
    int mode = MODE_1;
    switch (m_audioAttribute.channel) {
    case 1: mode = MODE_1;       break;
    case 2: mode = MODE_2;       break;
    case 3: mode = MODE_1_2;     break;
    case 4: mode = MODE_1_2_1;   break;
    case 5: mode = MODE_1_2_2;   break;
    case 6: mode = MODE_1_2_2_1; break;
    }
    if (aacEncoder_SetParam(m_audioEncoder, AACENC_CHANNELMODE, mode) != AACENC_OK) {
        OutputDebugString(TEXT("Unable to set the channel mode\n"));
        return -1;
    }

    if (aacEncoder_SetParam(m_audioEncoder, AACENC_CHANNELORDER, 1) != AACENC_OK) {
        OutputDebugString(TEXT("Unable to set the channel order\n"));
        return -1;
    }

	if (aacEncoder_SetParam(m_audioEncoder, AACENC_BITRATEMODE, 0) != AACENC_OK) {
		OutputDebugString(TEXT("Unable to set the bitrate mode\n"));
		return -1;
	}

	if (aacEncoder_SetParam(m_audioEncoder, AACENC_BITRATE, m_audioAttribute.bitrate * 1000) != AACENC_OK) {
		OutputDebugString(TEXT("Unable to set the bitrate\n"));
		return -1;
    }

    if (aacEncoder_SetParam(m_audioEncoder, AACENC_TRANSMUX, TT_MP4_ADTS) != AACENC_OK) {
        OutputDebugString(TEXT("Unable to set the transport type\n"));
        return -1;
    }

	if (aacEncEncode(m_audioEncoder, NULL, NULL, NULL, NULL) != AACENC_OK) {
		OutputDebugString(TEXT("Unable to initialize the encoder\n"));
		return 1;
	}
	AACENC_InfoStruct info = { 0 };
	if (aacEncInfo(m_audioEncoder, &info) != AACENC_OK) {
		OutputDebugString(TEXT("Unable to get the encoder info\n"));
		return 1;
	}

	return 0;
}

int Codec::AllocMemory()
{
	return 0;
}


int Codec::FreeMemory()
{
	EnterCriticalSection(&m_vfMtx);
	while (!videoFrameQueue.empty())
	{
		x264_picture_t *pic = videoFrameQueue.front();
		videoFrameQueue.pop();
		x264_picture_clean(pic);
		delete pic;
	}
	LeaveCriticalSection(&m_vfMtx);

	EnterCriticalSection(&m_vpMtx);
	while (!videoPacketQueue.empty())
	{
		MediaPacket* packet = videoPacketQueue.front();
		videoPacketQueue.pop();
		delete packet;
	}
	LeaveCriticalSection(&m_vpMtx);

	EnterCriticalSection(&m_afMtx);
    while (!audioFrameQueue.empty())
    {
        MediaFrame *frame = audioFrameQueue.front();
        audioFrameQueue.pop();
        delete frame;
	}
	LeaveCriticalSection(&m_afMtx);

	EnterCriticalSection(&m_apMtx);
    while (!audioPacketQueue.empty())
    {
        MediaPacket* packet = audioPacketQueue.front();
        audioPacketQueue.pop();
        delete packet;
	}
	LeaveCriticalSection(&m_apMtx);
	return 0;
}


x264_picture_t* Codec::PopVideoPicture()
{
	x264_picture_t *pic = NULL;
	EnterCriticalSection(&m_vfMtx);
	if (!videoFrameQueue.empty())
	{
		pic = videoFrameQueue.front();
		videoFrameQueue.pop();
	}
	LeaveCriticalSection(&m_vfMtx);
	return pic;
}


void Codec::PushVideoPicture(x264_picture_t* pic)
{
	if (pic)
	{
		EnterCriticalSection(&m_vfMtx);
		videoFrameQueue.push(pic);
		LeaveCriticalSection(&m_vfMtx);
	}
}


MediaPacket* Codec::PopVideoPacket()
{
	MediaPacket* packet = NULL;
	EnterCriticalSection(&m_vpMtx);
	if (!videoPacketQueue.empty())
	{
		packet = videoPacketQueue.front();
		videoPacketQueue.pop();
	}
	LeaveCriticalSection(&m_vpMtx);
	return packet;
}


void Codec::PushVideoPacket(MediaPacket* packet)
{
	if (packet!=NULL)
	{
		EnterCriticalSection(&m_vpMtx);
		videoPacketQueue.push(packet);
		LeaveCriticalSection(&m_vpMtx);
	}
}


MediaFrame* Codec::PopAudioFrame()
{
	MediaFrame *frame = NULL;
	EnterCriticalSection(&m_afMtx);
	if (!audioFrameQueue.empty())
	{
		frame = audioFrameQueue.front();
		audioFrameQueue.pop();
	}
	LeaveCriticalSection(&m_afMtx);
	return frame;
}


void Codec::PushAudioFrame(MediaFrame* frame)
{
	if (frame)
	{
		EnterCriticalSection(&m_afMtx);
		audioFrameQueue.push(frame);
		LeaveCriticalSection(&m_afMtx);
	}
}


MediaPacket* Codec::PopAudioPacket()
{
	MediaPacket* packet = NULL;
	EnterCriticalSection(&m_apMtx);
	if (!audioPacketQueue.empty())
	{
		packet = audioPacketQueue.front();
		audioPacketQueue.pop();
	}
	LeaveCriticalSection(&m_apMtx);
	return packet;
}


void Codec::PushAudioPacket(MediaPacket* packet)
{
	if (packet != NULL)
	{
		EnterCriticalSection(&m_apMtx);
		audioPacketQueue.push(packet);
		LeaveCriticalSection(&m_apMtx);
	}
}


int Codec::InitCodec()
{
	ConfigVideoCodec();
	ConfigAudioCodec();
	AllocMemory();
	return 0;
}


int Codec::UninitCodec()
{
	if (m_videoEncoder)
	{
		x264_encoder_close(m_videoEncoder);
		m_videoEncoder = NULL;
	}

	if (m_audioEncoder)
	{
		aacEncClose(&m_audioEncoder);
		m_audioEncoder = NULL;
	}
	FreeMemory();
	return 0;
}


DWORD WINAPI Codec::VideoEncodecThread(LPVOID lpParam)
{
	Codec* codec = (Codec*)lpParam;

	clock_t start = clock();
	clock_t finish;
	int preVideoCnt = 0;

	while (1)
	{
		if (codec->m_QuitCmd==1)
		{ 
			break;
		}

		x264_picture_t* pic = codec->PopVideoPicture();
		if (pic == NULL)
        {
            Sleep(10);
			continue;
        }

#if REC_CODEC_RAW
        if (codec->m_yuvfile.is_open())
        {
            codec->m_yuvfile.write((char*)pic->img.plane[0], codec->m_videoAttribute.width * codec->m_videoAttribute.height * 3 / 2);
        }
#endif

		x264_picture_t pic_out;
		x264_nal_t *nal;
		int i_nal;
		int i_frame_size = 0;

		i_frame_size = x264_encoder_encode(codec->m_videoEncoder, &nal, &i_nal, pic, &pic_out);
		if (i_frame_size > 0)
		{
			//OutputDebugString(TEXT("get video packet.\n"));
			MediaPacket* packet = new MediaPacket(PACKET_TYPE_VIDEO, i_frame_size);
			int size = 0;
			for (int i = 0; i < i_nal; ++i)
			{
				memcpy(&(packet->m_pData[size]), nal[i].p_payload, nal[i].i_payload);
				size += nal[i].i_payload;				
			}

			if (pic_out.b_keyframe)
			{
				packet->m_bKeyframe = true;
			}
			packet->m_uTimestamp = pic_out.i_pts / 1000;

			//Statistics
			codec->m_videoDecCnt++;
			finish = clock();
			if (finish - start >= CLOCKS_PER_SEC)
			{
				double fps = ((double)(codec->m_videoDecCnt - preVideoCnt)) * CLOCKS_PER_SEC / (finish - start);
				codec->m_videoDecFps = fps;
				preVideoCnt = codec->m_videoDecCnt;
				start = finish;
			}

            codec->PushVideoPacket(packet);
		}

		x264_picture_clean(pic);
		delete pic;
	}

	return 0;
}


DWORD WINAPI Codec::AudioEncodecThread(LPVOID lpParam)
{
    Codec* codec = (Codec*)lpParam;
    AudioCodecAttribute attr = codec->m_audioAttribute;

    AACENC_InfoStruct info = { 0 };
    if (aacEncInfo(codec->m_audioEncoder, &info) != AACENC_OK) {
        OutputDebugString(TEXT("Unable to get the encoder info\n"));
        return 1;
    }

    AACENC_BufDesc inbuf = { 0 };
    int in_cursize = 0;
    int in_size = 1024 * attr.channel * attr.bitwide / 8;
    void* pinbuf = malloc(in_size);
    int in_identifier = IN_AUDIO_DATA;
    int in_elem_size = 2;

    inbuf.numBufs = 1;
    inbuf.bufferIdentifiers = &in_identifier;
    inbuf.bufElSizes = &in_elem_size;
    inbuf.bufs = &pinbuf;
    inbuf.bufSizes = &in_size;

    AACENC_BufDesc outbuf = { 0 };
    int out_size = info.maxOutBufBytes;
    void* poutbuf = malloc(out_size);
    if (poutbuf==NULL)
    {
        return -1;
    }
    int out_elem_size = 1;
    int out_identifier = OUT_BITSTREAM_DATA;
    outbuf.numBufs = 1;
    outbuf.bufs = &poutbuf;
    outbuf.bufferIdentifiers = &out_identifier;
    outbuf.bufSizes = &out_size;
    outbuf.bufElSizes = &out_elem_size;

    AACENC_InArgs in_args = { 0 };
    in_args.numInSamples = 1024 * attr.channel;

    AACENC_OutArgs out_args = { 0 };

    unsigned long long timestamp = 0;

    while (1)
    {
        if (codec->m_QuitCmd == 1)
        {
            break;
        }

        MediaFrame* frame = codec->PopAudioFrame();
        if (frame == NULL)
        {
            Sleep(10);
            continue;
        }

#if REC_CODEC_RAW
        if (codec->m_pcmfile.is_open())
        {
            codec->m_pcmfile.write((char*)frame->m_pData, frame->m_dataSize);
        }
#endif

        int pframeleft = frame->m_dataSize;

        while (1)
        {
            if (pframeleft<=0)
            {
                break;
            }

            if (pframeleft < in_size - in_cursize)
            {
                memcpy(&((char*)pinbuf)[in_cursize], &frame->m_pData[frame->m_dataSize - pframeleft], pframeleft);
                in_cursize += pframeleft;
                pframeleft = 0;
                break;
            }
            else
            {
                memcpy(&((char*)pinbuf)[in_cursize], &frame->m_pData[frame->m_dataSize - pframeleft], in_size - in_cursize);
                pframeleft -= in_size - in_cursize;
                in_cursize = 0;
            }

            if (timestamp == 0)
            {
                timestamp = frame->m_uTimestamp;
            }
            else
            {
                timestamp += 1024 * 1000 * 1000 / attr.samplerate;
            }

            AACENC_ERROR err;
            if ((err = aacEncEncode(codec->m_audioEncoder, &inbuf, &outbuf, &in_args, &out_args)) != AACENC_OK) {
                if (err != AACENC_ENCODE_EOF)
                    OutputDebugString(TEXT("Encoding failed\n"));
                break;
            }
            int size = out_args.numOutBytes;
            if (size)
            {
                MediaPacket* packet = new MediaPacket(PACKET_TYPE_AUDIO, size);
                memcpy(packet->m_pData, outbuf.bufs[0], size);
                packet->m_uTimestamp = timestamp / 1000;

                codec->PushAudioPacket(packet);
            }
        }

        delete frame;
    }

    free(pinbuf);
    free(poutbuf);

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
		ChooseConversionFunction(type, pattr->format);

	}
	else if (type == ATTRIBUTE_TYPE_AUDIO)
	{
		AudioCaptureAttribute *pattr = (AudioCaptureAttribute *)attribute;
		m_audioSrcAttribute = *pattr;
		ChooseConversionFunction(type, pattr->format);
	}
	return 0;
}


int Codec::SendFrame(MediaFrame * frame)
{
    if (frame == NULL || m_Status!= CODEC_STATUS_START) {
        return -1;
    }

	int ret = 0;

    if (frame->m_FrameType==FRAME_TYPE_VIDEO)
	{

		if (m_videoConvertFn == NULL)
		{
			return MF_E_INVALIDMEDIATYPE;
		}

		if (videoFrameQueue.size()>=MAX_VIDEO_FRAME || videoPacketQueue.size()>=MAX_VIDEO_PACKET)
		{
			m_videoLostCnt++;
			return -1;
		}

		x264_picture_t* pic = new x264_picture_t;
		if (pic == NULL)
		{
			return -1;
		}
		x264_picture_init(pic);
		ret = x264_picture_alloc(pic, X264_CSP_I420, m_videoAttribute.width, m_videoAttribute.height);
		if (ret != 0)
		{
			delete pic;
			return -1;
		}

		m_videoConvertFn(
			pic->img.plane[0],
			pic->img.i_stride[0],
			frame->m_pData,
			frame->m_stride,
			frame->m_width,
			frame->m_height
			);
		pic->i_pts = frame->m_uTimestamp;

		PushVideoPicture(pic);
    } 
    else if(frame->m_FrameType == FRAME_TYPE_AUDIO)
	{
		if (m_audioConvertFn == NULL)
		{
			return MF_E_INVALIDMEDIATYPE;
		}

		if (audioFrameQueue.size() >= MAX_AUDIO_FRAME || audioPacketQueue.size() >= MAX_AUDIO_PACKET)
		{
			return -1;
		}

		MediaFrame* sample = new MediaFrame(FRAME_TYPE_AUDIO, MFAudioFormat_PCM, frame->m_dataSize * m_audioAttribute.bitwide * m_audioAttribute.channel / (frame->m_bitwide * frame->m_channels));
        sample->m_FrameType = frame->m_FrameType;
		sample->m_uTimestamp = frame->m_uTimestamp;
		sample->m_channels = m_audioAttribute.channel;
		sample->m_bitwide = m_audioAttribute.bitwide; //支持S16格式的音频
		sample->m_samplerate = m_audioAttribute.samplerate;

		m_audioConvertFn(frame, sample);

		PushAudioFrame(sample);
    }

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// public method
//////////////////////////////////////////////////////////////////////////

int Codec::GetCodecStatistics(CodecStatistics* statistics)
{
	if (statistics)
	{
		statistics->videoFrameCnt  = videoFrameQueue.size();
		statistics->videoPacketCnt = videoPacketQueue.size();
		statistics->videoLostCnt   = m_videoLostCnt;
		statistics->videoDecCnt    = m_videoDecCnt;
		statistics->videoDecFps    = m_videoDecFps;
	}
	return 0;
}


int Codec::SetVideoCodecAttribute(VideoCodecAttribute* attribute)
{
    if (attribute != NULL)
    {
        m_videoAttribute = *attribute;
		m_videoAttribute.width = m_videoSrcAttribute.width;
		m_videoAttribute.height = m_videoSrcAttribute.height;
		m_videoAttribute.fps = m_videoSrcAttribute.fps;
    }
	return 0;
}


int Codec::GetVideoCodecAttribute(const VideoCodecAttribute** attribute)
{
    if (attribute != NULL)
    {
        *attribute = &m_videoAttribute;
    }
    return 0;
}


int Codec::SetAudioCodecAttribute(AudioCodecAttribute* attribute)
{
    if (attribute != NULL)
    {
		m_audioAttribute = *attribute;
		m_audioAttribute.channel = m_audioSrcAttribute.channel;
		m_audioAttribute.samplerate = m_audioSrcAttribute.samplerate;
        m_audioAttribute.bitwide = 16;
    }
	return 0;
}


int Codec::GetAudioCodecAttribute(const AudioCodecAttribute** attribute)
{
    if (attribute != NULL)
    {
        *attribute = &m_audioAttribute;
    }
    return 0;
}


int Codec::Start()
{

#if REC_CODEC_RAW
    m_pcmfile.open("codec.pcm", ios::out | ios::binary);
    m_yuvfile.open("codec.yuv", ios::out | ios::binary);
#endif

	InitCodec();
	m_QuitCmd = 0;
    m_videoThread = CreateThread(NULL, 0, VideoEncodecThread, this, 0, NULL);
    m_audioThread = CreateThread(NULL, 0, AudioEncodecThread, this, 0, NULL);
    m_Status = CODEC_STATUS_START;
	return 0;
}


int Codec::Pause()
{
	return 0;
}


int Codec::Stop()
{

    m_Status = CODEC_STATUS_STOP;
	m_QuitCmd = 1;
    WaitForSingleObject(m_videoThread, INFINITE);
    WaitForSingleObject(m_audioThread, INFINITE);
	UninitCodec();

#if REC_CODEC_RAW
    if (m_pcmfile.is_open())
    {
        m_pcmfile.flush();
        m_pcmfile.close();
    }
    if (m_yuvfile.is_open())
    {
        m_yuvfile.flush();
        m_yuvfile.close();
    }
#endif

	return 0;
}


MediaPacket* Codec::GetVideoPacket()
{
    return PopVideoPacket();
}


MediaPacket* Codec::GetAudioPacket()
{
    return PopAudioPacket();
}


int Codec::GetVideoPacketCount()
{
	return (int)videoPacketQueue.size();
}


int Codec::GetAudioPacketCount()
{
	return (int)audioPacketQueue.size();
}

