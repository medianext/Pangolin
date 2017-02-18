#include "stdafx.h"
#include "Capture.h"
#include "VideoCapture.h"
#include "AudioCapture.h"

static UINT32 m_videoCapCnt;
static UINT32 m_audioCapCnt;
static IMFActivate **m_ppVideoDevices;
static IMFActivate **m_ppAudioDevices;

int Capture::Init()
{
    HRESULT hr = S_OK;
    IMFAttributes *pAttributes = NULL;

    hr = MFCreateAttributes(&pAttributes, 1);
    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
            );
    }

    // Enumerate devices.
    if (SUCCEEDED(hr))
    {
        hr = MFEnumDeviceSources(pAttributes, &m_ppVideoDevices, &m_videoCapCnt);
    }

    SafeRelease(&pAttributes);

    hr = MFCreateAttributes(&pAttributes, 1);
    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID
            );
    }

    // Enumerate devices.
    if (SUCCEEDED(hr))
    {
        hr = MFEnumDeviceSources(pAttributes, &m_ppAudioDevices, &m_audioCapCnt);
    }

    SafeRelease(&pAttributes);

    return (int)(m_videoCapCnt + m_audioCapCnt);
}


int Capture::Uninit()
{
    HRESULT hr = S_OK;
    IMFAttributes *pAttributes = NULL;


    return m_audioCapCnt;
}


int Capture::EnumVideoCature(std::vector<WCHAR *> *vCaptureList)
{
    if (vCaptureList != NULL)
    {
        for (UINT32 i = 0; i < m_videoCapCnt; i++)
        {
            WCHAR * szFriendlyName;
            m_ppVideoDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &szFriendlyName, NULL);
            WCHAR * str = new WCHAR[wcslen(szFriendlyName)];
            StrCpyW(str, szFriendlyName);
            vCaptureList->insert(vCaptureList->end(), str);
            CoTaskMemFree(szFriendlyName);
            SafeRelease(&m_ppVideoDevices[i]);
        }
    }

	return (int)m_videoCapCnt;
}


int Capture::EnumAudioCature(std::vector<WCHAR *> *aCaptureList)
{
    if (aCaptureList != NULL)
    {
        for (UINT32 i = 0; i < m_audioCapCnt; i++)
        {
            WCHAR * szFriendlyName;
            m_ppAudioDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &szFriendlyName, NULL);
            WCHAR * str = new WCHAR[wcslen(szFriendlyName)];
            StrCpyW(str, szFriendlyName);
            aCaptureList->insert(aCaptureList->end(), str);
            CoTaskMemFree(szFriendlyName);
            SafeRelease(&m_ppAudioDevices[i]);
        }
    }

	return m_audioCapCnt;
}


Capture* Capture::GetVideoCature(int index)
{
    if ((UINT32)index < m_videoCapCnt)
    {
        return new VideoCapture(m_ppVideoDevices[0]);
    }
    return NULL;
}


Capture* Capture::GetAudioCature(int index)
{
    if ((UINT32)index < m_audioCapCnt)
    {
        return new AudioCapture(m_ppAudioDevices[0]);
    }
    return NULL;
}