#include "stdafx.h"
#include "Render.h"


static Render* instanse;

Render::Render(HWND hWnd)
{
    m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (m_pD3D == NULL)
    {
        delete this;
        return;
    }

    HRESULT hr = S_OK;
    D3DPRESENT_PARAMETERS pp = { 0 };
    D3DDISPLAYMODE mode = { 0 };

    hr = m_pD3D->GetAdapterDisplayMode(
        D3DADAPTER_DEFAULT,
        &mode
        );

    if (FAILED(hr)) { return; }

    hr = m_pD3D->CheckDeviceType(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        mode.Format,
        D3DFMT_X8R8G8B8,
        TRUE    // windowed
        );

    if (FAILED(hr)) { return; }

    pp.BackBufferFormat = D3DFMT_X8R8G8B8;
    pp.SwapEffect = D3DSWAPEFFECT_COPY;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    pp.Windowed = TRUE;
    pp.hDeviceWindow = hWnd;

    hr = m_pD3D->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE,
        &pp,
        &m_pDevice
        );

    if (FAILED(hr)) { return; }

    m_hWnd = hWnd;
    m_d3dpp = pp;

    RECT rect;
    GetClientRect(m_hWnd, &rect);
    m_width = rect.right - rect.left;
    m_height = rect.bottom - rect.top;

    pp.BackBufferWidth = m_width;
    pp.BackBufferHeight = m_height;
    pp.Windowed = TRUE;
    pp.SwapEffect = D3DSWAPEFFECT_FLIP;
    pp.hDeviceWindow = hWnd;
    pp.BackBufferFormat = D3DFMT_X8R8G8B8;
    pp.Flags =
        D3DPRESENTFLAG_VIDEO | D3DPRESENTFLAG_DEVICECLIP |
        D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    pp.BackBufferCount = 2;

    hr = m_pDevice->CreateAdditionalSwapChain(&pp, &m_pSwapChain);

}


Render::~Render()
{
    SafeRelease(&m_pSwapChain);
    SafeRelease(&m_pDevice);
    SafeRelease(&m_pD3D);
}


void Render::Init(HWND hWnd)
{
    if (!instanse) {
        instanse = new Render(hWnd);
    }
}


void Render::Uninit()
{
    if (instanse)
    {
        delete instanse;
    }
}


Render* Render::GetRender()
{
    return instanse;
}


int Render::SendFrame(MediaFrame * frame)
{
    return 0;
}


int Render::DrawFrame(MediaFrame * frame)
{
    return 0;
}