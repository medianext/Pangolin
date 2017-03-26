/*
* Author：王德明
* Email：phight@163.com
* QQ群：220954528
*/

#pragma once

#include "Sink.h"


class Render :
    public Sink
{

public:
    static void Init(HWND hWnd);
    static void Uninit();
    static Render* GetRender();
    ~Render();
private:
    Render(HWND hWnd);
    HRESULT TestCooperativeLevel();
    HRESULT ChooseConversionFunction(REFGUID subtype);

public:
    int SetSourceAttribute(void* attribute, AttributeType type);
    int SendFrame(MediaFrame * frame);

    int DrawFrame(MediaFrame * frame);

private:
    HWND m_hWnd;

    IDirect3D9              *m_pD3D;
    IDirect3DDevice9        *m_pDevice;
    IDirect3DSwapChain9     *m_pSwapChain;

    D3DPRESENT_PARAMETERS   m_d3dpp;

    RECT                    m_rcCanvas;
    RECT                    m_rcDest;

    GUID                    m_subtype;
    D3DFORMAT               m_format;
    
    IMAGE_TRANSFORM_FN      m_convertFn;
};

