
#include "stdafx.h"
#include "Direct3D9Render.h"

Direct3D9Render::Direct3D9Render(HWND hWnd)
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

    GetClientRect(m_hWnd, &m_rcCanvas);

}


Direct3D9Render::~Direct3D9Render()
{
    SafeRelease(&m_pSwapChain);
    SafeRelease(&m_pDevice);
    SafeRelease(&m_pD3D);
}


//////////////////////////////////////////////////////////////////////////
// private method
//////////////////////////////////////////////////////////////////////////

HRESULT Direct3D9Render::ChooseConversionFunction(REFGUID subtype)
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


HRESULT Direct3D9Render::TestCooperativeLevel()
{
    if (m_pDevice == NULL)
    {
        return E_FAIL;
    }

    HRESULT hr = S_OK;

    // Check the current status of D3D9 device.
    hr = m_pDevice->TestCooperativeLevel();

    switch (hr)
    {
    case D3D_OK:
        break;

    case D3DERR_DEVICELOST:
        hr = S_OK;

    case D3DERR_DEVICENOTRESET:
        //hr = ResetDevice();
        return E_FAIL;
        break;

    default:
        // Some other failure.
        break;
    }

    return hr;
}


//////////////////////////////////////////////////////////////////////////
// public method
//////////////////////////////////////////////////////////////////////////

int Direct3D9Render::SetSourceAttribute(void* attribute, AttributeType type)
{
    if (attribute == NULL)
    {
        return -1;
    }
    VideoCaptureAttribute* attr = (VideoCaptureAttribute*)attribute;

    HRESULT hr = S_OK;

    hr = ChooseConversionFunction(attr->format);
    if (!SUCCEEDED(hr))
    {
        return -1;
    }
    m_subtype = attr->format;

    IDirect3DSwapChain9 *pSwapChain;
    D3DPRESENT_PARAMETERS pp = { 0 };
    pp.BackBufferWidth = attr->width;
    pp.BackBufferHeight = attr->height;
    pp.Windowed = TRUE;
    pp.SwapEffect = D3DSWAPEFFECT_FLIP;
    pp.hDeviceWindow = m_hWnd;
    pp.BackBufferFormat = D3DFMT_X8R8G8B8;
    pp.Flags =
        D3DPRESENTFLAG_VIDEO | D3DPRESENTFLAG_DEVICECLIP |
        D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    pp.BackBufferCount = 2;

    hr = m_pDevice->CreateAdditionalSwapChain(&pp, &pSwapChain);
    if (!SUCCEEDED(hr))
    {
        return -1;
    }
    SafeRelease(&m_pSwapChain);
    m_pSwapChain = pSwapChain;

    //
    RECT rcSrc = { 0, 0, attr->width, attr->height };
    //rcSrc = CorrectAspectRatio(rcSrc, m_PixelAR);
    m_rcDest = LetterBoxRect(rcSrc, m_rcCanvas);

    return 0;
}


int Direct3D9Render::SendFrame(MediaFrame * frame)
{
    DrawFrame(frame);
    return 0;
}


int Direct3D9Render::DrawFrame(MediaFrame * frame)
{

    if (m_pDevice == NULL || m_pSwapChain == NULL || frame == NULL)
    {
        return S_OK;
    }

    HRESULT hr = S_OK;
    D3DLOCKED_RECT lr;

    IDirect3DSurface9 *pSurf = NULL;
    IDirect3DSurface9 *pBB = NULL;

    if (m_convertFn == NULL)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    hr = TestCooperativeLevel();

    if (FAILED(hr)) { goto done; }

    // Get the swap-chain surface.
    hr = m_pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pSurf);

    if (FAILED(hr)) { goto done; }

    // Lock the swap-chain surface.
    hr = pSurf->LockRect(&lr, NULL, D3DLOCK_NOSYSLOCK);

    if (FAILED(hr)) { goto done; }


    // Convert the frame. This also copies it to the Direct3D surface.

    m_convertFn(
        (BYTE*)lr.pBits,
        lr.Pitch,
        frame->m_pData,
        frame->m_stride,
        frame->m_width,
        frame->m_height
        );

    hr = pSurf->UnlockRect();

    if (FAILED(hr)) { goto done; }


    // Color fill the back buffer.
    hr = m_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBB);

    if (FAILED(hr)) { goto done; }

    hr = m_pDevice->ColorFill(pBB, NULL, D3DCOLOR_XRGB(0, 0, 0x80));

    if (FAILED(hr)) { goto done; }


    // Blit the frame.
    hr = m_pDevice->StretchRect(pSurf, NULL, pBB, &m_rcDest, D3DTEXF_LINEAR);

    if (FAILED(hr)) { goto done; }


    // Present the frame.
    hr = m_pDevice->Present(NULL, NULL, NULL, NULL);


done:
    SafeRelease(&pBB);
    SafeRelease(&pSurf);
    return hr;
}
