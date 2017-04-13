#pragma once


#include <d3d9.h>

#include "stdafx.h"
#include "Render.h"


//////////////////////////////////////////////////////////////////////////
//  
//////////////////////////////////////////////////////////////////////////

class Direct3D9Render :
    public Render
{

public:
    Direct3D9Render(HWND hWnd);
    ~Direct3D9Render();

private:
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

