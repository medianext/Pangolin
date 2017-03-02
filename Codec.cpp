#include "stdafx.h"
#include "Codec.h"


//-------------------------------------------------------------------
//
// Conversion functions
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
	yuv.u = Clip(((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128);
	yuv.v = Clip(((112 * r - 94 * g - 18 * b + 128) >> 8) + 128);

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
// YUY2 to RGB-32
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

        for (DWORD x = 0; x < dwWidthInPixels; x += 2)
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
    const BYTE* lpSrcBitsCr = lpSrcBitsCb + 1;

	for (DWORD y = 0; y < dwHeightInPixels; y++)
	{
		WORD *pSrcPel = (WORD*)pSrc;

		for (DWORD x = 0; x < dwWidthInPixels; x += 2)
		{

			*(lpDstBitsY++) = *(lpSrcBitsY++);

			if (x % 2 == 0 && y % 2 == 0)
			{
				*(lpDstBitsCb++) = *(lpSrcBitsCb++);
				*(lpDstBitsCr++) = *(lpSrcBitsCb++);
			}
		}

		pSrc += lSrcStride;
	}
}


typedef void(*IMAGE_TRANSFORM_FN)(
    BYTE*       pDest,
    LONG        lDestStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    );
// Static table of output formats and conversion functions.
struct ConversionFunction
{
    GUID               subtype;
    IMAGE_TRANSFORM_FN xform;
};


static ConversionFunction g_FormatConversions[] =
{
    { MFVideoFormat_RGB32, TransformImage_RGB32 },
    { MFVideoFormat_RGB24, TransformImage_RGB24 },
    { MFVideoFormat_YUY2,  TransformImage_YUY2 },
    { MFVideoFormat_I420,  TransformImage_I420 },
    { MFVideoFormat_NV12,  TransformImage_NV12 }
};

const DWORD   g_cFormats = ARRAYSIZE(g_FormatConversions);


//////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////

Codec::Codec() :
    m_videoEncoder(NULL),
    m_audioEncoder(NULL),
    m_Status(0)
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

HRESULT Codec::ChooseConversionFunction(REFGUID subtype)
{
	m_convertFn = NULL;

	for (DWORD i = 0; i < g_cFormats; i++)
	{
		if (g_FormatConversions[i].subtype == subtype)
		{
			m_convertFn = g_FormatConversions[i].xform;
			return S_OK;
		}
	}

	return MF_E_INVALIDMEDIATYPE;
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
    param.i_fps_num = m_videoAttribute.fps;
    param.i_fps_den = 1;
    param.i_keyint_max = m_videoAttribute.fps;
    param.b_intra_refresh = 1;
    param.b_annexb = 1;

    param.rc.i_rc_method = X264_RC_CRF;
    param.rc.i_bitrate = m_videoAttribute.bitrate;

    ret = x264_param_apply_profile(&param, "high");

	m_videoEncoder = x264_encoder_open(&param);

    x264_encoder_parameters(m_videoEncoder, &param);

	return 0;
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
    case 0: aot = 2;  break;
    case 1: aot = 5;  break;
    case 2: aot = 29; break;
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

	if (aacEncoder_SetParam(m_audioEncoder, AACENC_BITRATEMODE, 0) != AACENC_OK) {
		OutputDebugString(TEXT("Unable to set the bitrate mode\n"));
		return -1;
	}

	if (aacEncoder_SetParam(m_audioEncoder, AACENC_BITRATE, m_audioAttribute.bitrate) != AACENC_OK) {
		OutputDebugString(TEXT("Unable to set the bitrate\n"));
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
	while (!videoFrameQueue.empty())
	{
		x264_picture_t *pic = videoFrameQueue.front();
		videoFrameQueue.pop();
		x264_picture_clean(pic);
		delete pic;
	}

	while (!videoPacketQueue.empty())
	{
		MediaPacket* packet = videoPacketQueue.front();
		videoPacketQueue.pop();
		delete packet;
    }

    while (!audioFrameQueue.empty())
    {
        MediaFrame *frame = audioFrameQueue.front();
        audioFrameQueue.pop();
        delete frame;
    }

    while (!audioPacketQueue.empty())
    {
        MediaPacket* packet = audioPacketQueue.front();
        audioPacketQueue.pop();
        delete packet;
    }
	return 0;
}


x264_picture_t* Codec::PopVideoPicture()
{
	x264_picture_t *pic = NULL;
	if (!videoFrameQueue.empty())
	{
		pic = videoFrameQueue.front();
		videoFrameQueue.pop();
	}
	return pic;
}


void Codec::PushVideoPicture(x264_picture_t* pic)
{
	if (pic)
	{
		videoFrameQueue.push(pic);
	}
}


MediaPacket* Codec::PopVideoPacket()
{
	MediaPacket* packet = NULL;
	if (!videoPacketQueue.empty())
	{
		packet = videoPacketQueue.front();
		videoPacketQueue.pop();
	}
	return packet;
}


void Codec::PushVideoPacket(MediaPacket* packet)
{
	if (packet!=NULL)
	{
		videoPacketQueue.push(packet);
	}
}


MediaFrame* Codec::PopAudioFrame()
{
    MediaFrame *frame = NULL;
	if (!audioFrameQueue.empty())
	{
		frame = audioFrameQueue.front();
		audioFrameQueue.pop();
	}
	return frame;
}


void Codec::PushAudioFrame(MediaFrame* frame)
{
	if (frame)
	{
		audioFrameQueue.push(frame);
	}
}


MediaPacket* Codec::PopAudioPacket()
{
	MediaPacket* packet = NULL;
	if (!audioPacketQueue.empty())
	{
		packet = audioPacketQueue.front();
		audioPacketQueue.pop();
	}
	return packet;
}


void Codec::PushAudioPacket(MediaPacket* packet)
{
	if (packet != NULL)
	{
		audioPacketQueue.push(packet);
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
    aacEncClose(&m_audioEncoder);
    x264_encoder_close(m_videoEncoder);
	FreeMemory();
	return 0;
}


DWORD WINAPI Codec::VideoEncodecThread(LPVOID lpParam)
{
	Codec* codec = (Codec*)lpParam;

	while (1)
	{
		if (codec->m_QuitCmd==1)
		{ 
			break;
		}

		x264_picture_t* pic = codec->PopVideoPicture();
		if (pic == NULL)
		{
			continue;
		}

		x264_picture_t pic_out;
		x264_nal_t *nal;
		int i_nal;
		int i_frame_size = 0;

		i_frame_size = x264_encoder_encode(codec->m_videoEncoder, &nal, &i_nal, pic, &pic_out);
		if (i_frame_size > 0)
		{
			OutputDebugString(TEXT("get video packet.\n"));
			MediaPacket* packet = new MediaPacket(PACKET_TYPE_VIDEO, i_frame_size);
			int size = 0;
			for (int i = 0; i < i_nal; ++i)
			{
				memcpy(&(packet->m_pData[size]), nal[i].p_payload, nal[i].i_payload);
				size += nal[i].i_payload;
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

    AACENC_InfoStruct info = { 0 };
    if (aacEncInfo(codec->m_audioEncoder, &info) != AACENC_OK) {
        OutputDebugString(TEXT("Unable to get the encoder info\n"));
        return 1;
    }

    AACENC_BufDesc inbuf = { 0 };
    void* pinbuf = NULL;
    int in_size = info.maxOutBufBytes;
    int in_identifier = IN_AUDIO_DATA;
    int in_elem_size = 2;

    inbuf.numBufs = 1;
    inbuf.bufferIdentifiers = &in_identifier;
    inbuf.bufElSizes = &in_elem_size;
    inbuf.bufs = &pinbuf;
    inbuf.bufSizes = &in_size;

    AACENC_BufDesc outbuf = { 0 };
    void* poutbuf = malloc(info.maxOutBufBytes);
    if (poutbuf==NULL)
    {
        return -1;
    }
    int out_size = info.maxOutBufBytes;
    int out_elem_size = 1;
    int out_identifier = OUT_BITSTREAM_DATA;
    outbuf.numBufs = 1;
    outbuf.bufs = &poutbuf;
    outbuf.bufferIdentifiers = &out_identifier;
    outbuf.bufSizes = &out_size;
    outbuf.bufElSizes = &out_elem_size;

    AACENC_InArgs in_args = { 0 };
    AACENC_OutArgs out_args = { 0 };

    AudioCodecAttribute attr = codec->m_audioAttribute;

    while (1)
    {
        if (codec->m_QuitCmd == 1)
        {
            break;
        }

        MediaFrame* frame = codec->PopAudioFrame();
        if (frame == NULL)
        {
            continue;
        }

        pinbuf = frame->m_pData;
        in_size = frame->m_dataSize;

        in_args.numInSamples = inbuf.bufSizes[0] / (attr.channel * attr.bitwide / 8);

        AACENC_ERROR err;
        if ((err = aacEncEncode(codec->m_audioEncoder, &inbuf, &outbuf, &in_args, &out_args)) != AACENC_OK) {
            if (err == AACENC_ENCODE_EOF)
                break;
            OutputDebugString(TEXT("Encoding failed\n"));
            break;
        }
        int size = out_args.numOutBytes;
        if (size)
        {
            MediaPacket* packet = new MediaPacket(PACKET_TYPE_AUDIO, size);
            memcpy(packet->m_pData, outbuf.bufs[0], size);
            codec->PushAudioPacket(packet);
        }
        free(frame->m_pData);
        delete frame;
    }

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
		ChooseConversionFunction(pattr->format);

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
    if (frame == NULL || m_Status!= CODEC_STATUS_START) {
        return -1;
    }

	int ret = 0;

    if (frame->m_FrameType==FRAME_TYPE_VIDEO)
	{

		if (m_convertFn == NULL)
		{
			return MF_E_INVALIDMEDIATYPE;
		}

		if (videoFrameQueue.size()>=MAX_VIDEO_FRAME || videoPacketQueue.size()>=MAX_VIDEO_PACKET)
		{
			OutputDebugString(TEXT("buffer is full!\n"));
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

		m_convertFn(
			pic->img.plane[0],
			pic->img.i_stride[0],
			frame->m_pData,
			frame->m_stride,
			frame->m_width,
			frame->m_height
			);

		PushVideoPicture(pic);
    } 
    else if(frame->m_FrameType == FRAME_TYPE_AUDIO)
	{

		if (audioFrameQueue.size() >= MAX_AUDIO_FRAME || audioPacketQueue.size() >= MAX_AUDIO_PACKET)
		{
			OutputDebugString(TEXT("buffer is full!\n"));
			return -1;
		}

        MediaFrame* sample = new MediaFrame();
        sample->m_pData = (BYTE*)malloc(frame->m_dataSize);
        sample->m_dataSize = frame->m_dataSize;

        memcpy(sample->m_pData, frame->m_pData, frame->m_dataSize);

		PushAudioFrame(sample);
    }

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// public method
//////////////////////////////////////////////////////////////////////////

int Codec::SetVideoCodecAttribute(VideoCodecAttribute* attribute)
{
    if (attribute != NULL)
    {
        m_videoAttribute = *attribute;
    }
	return 0;
}


int Codec::SetAudioCodecAttribute(AudioCodecAttribute* attribute)
{
    if (attribute != NULL)
    {
        m_audioAttribute = *attribute;
        m_audioAttribute.bitwide = m_audioSrcAttribute.bitwide;
    }
	return 0;
}


int Codec::Start()
{
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
	return 0;
}


MediaPacket* Codec::GetVideoPacket()
{
    return PopVideoPacket();
}


MediaPacket* Codec::GetAudioPacket()
{
    return NULL;
}


int Codec::GetVideoPacketCount()
{
	return (int)videoPacketQueue.size();
}


int Codec::GetAudioPacketCount()
{
	return (int)audioPacketQueue.size();
}

