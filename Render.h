/*
* Author：王德明
* Email：phight@163.com
* QQ群：220954528
*/

#pragma once

#include "stdafx.h"
#include "Sink.h"


class Render :
    public Sink
{

public:
    virtual ~Render();

public:
    static void Init(HWND hWnd);
    static void Uninit();
    static Render* GetRender();
};


RECT LetterBoxRect(const RECT& rcSrc, const RECT& rcDst);
RECT CorrectAspectRatio(const RECT& src, const MFRatio& srcPAR);

void TransformImage_RGB32(BYTE* pDst, LONG dstStride, const BYTE* pSrc, LONG srcStride, DWORD dwWidthInPixels, DWORD dwHeightInPixels);
void TransformImage_RGB24(BYTE* pDst, LONG dstStride, const BYTE* pSrc, LONG srcStride, DWORD dwWidthInPixels, DWORD dwHeightInPixels);
void TransformImage_YUY2(BYTE* pDst, LONG dstStride, const BYTE* pSrc, LONG srcStride, DWORD dwWidthInPixels, DWORD dwHeightInPixels);
void TransformImage_I420(BYTE* pDst, LONG dstStride, const BYTE* pSrc, LONG srcStride, DWORD dwWidthInPixels, DWORD dwHeightInPixels);
void TransformImage_NV12(BYTE* pDst, LONG dstStride, const BYTE* pSrc, LONG srcStride, DWORD dwWidthInPixels, DWORD dwHeightInPixels);


extern VideoConversionFunction g_FormatConversions[];
extern const DWORD g_cFormats;