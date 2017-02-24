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
    m_audioEncoder(NULL)
{
}


Codec::~Codec()
{
    if (m_videoEncoder)
    {
        x264_encoder_close(m_videoEncoder);
        x264_picture_clean(&m_picture);
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

    x264_picture_init(&m_picture);
    ret = x264_picture_alloc(&m_picture, X264_CSP_I420, m_videoAttribute.width, m_videoAttribute.height);

	return 0;
}


int Codec::ConfigAudioCodec()
{
	return 0;
}


int Codec::EncodeVideo(MediaFrame* frame, MediaPacket* packet)
{
    if (m_videoEncoder==NULL || frame==NULL)
    {
        return -1;
    }

	TransformImage_RGB24(
        m_picture.img.plane[0],
        m_picture.img.i_stride[0],
        frame->m_pData,
        frame->m_stride,
        frame->m_width,
        frame->m_height
        );

    x264_picture_t pic_out;
    x264_nal_t *nal;
    int i_nal;
    int i_frame_size = 0;

    i_frame_size = x264_encoder_encode(m_videoEncoder, &nal, &i_nal, &m_picture, &pic_out);

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
    if (frame == NULL) {
        return -1;
    }

    if (frame->m_FrameType==FRAME_TYPE_VIDEO)
    {
        this->EncodeVideo(frame, NULL);
    } 
    else if(frame->m_FrameType == FRAME_TYPE_AUDIO)
    {
        this->EncodeAudio(frame, NULL);
    }

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// public method
//////////////////////////////////////////////////////////////////////////

int Codec::SetVideoCodecAttribute(VideoCodecAttribute* attribute)
{
    if (attribute!=NULL)
    {
        m_videoAttribute = *attribute;
    }
	return 0;
}


int Codec::SetAudioCodecAttribute(AudioCodecAttribute* attribute)
{
    if (attribute == NULL)
    {
        m_audioAttribute = *attribute;
    }
	return 0;
}


int Codec::Start()
{
    ConfigVideoCodec();
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