/*
* Author：王德明
* Email：phight@163.com
* QQ群：220954528
*/

#pragma once

#include "MediaFrame.h"


typedef void(*IMAGE_TRANSFORM_FN)(
	BYTE*       pDest,
	LONG        lDestStride,
	const BYTE* pSrc,
	LONG        lSrcStride,
	DWORD       dwWidthInPixels,
	DWORD       dwHeightInPixels
	);

struct VideoConversionFunction
{
	GUID               subtype;
	IMAGE_TRANSFORM_FN xform;
};


typedef void(*AUDIO_TRANSFORM_FN)(MediaFrame* srcFrame, MediaFrame* dstFrame);

struct AudioConversionFunction
{
	GUID               subtype;
	AUDIO_TRANSFORM_FN xform;
};


enum AttributeType {

	ATTRIBUTE_TYPE_VIDEO = 0,
	ATTRIBUTE_TYPE_AUDIO,
};


class Sink
{

public:
    virtual int SetSourceAttribute(void* attribute, AttributeType type) = 0;
    virtual int SendFrame(MediaFrame * frame) = 0;
};

