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

public:
    int SendFrame(MediaFrame * frame);
    int DrawFrame(MediaFrame * frame);

private:
    HWND m_hWnd;

    IDirect3D9              *m_pD3D;
    IDirect3DDevice9        *m_pDevice;
    IDirect3DSwapChain9     *m_pSwapChain;

    D3DPRESENT_PARAMETERS   m_d3dpp;

    D3DFORMAT               m_format;
    UINT                    m_width;
    UINT                    m_height;
};

