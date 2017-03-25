#include "stdafx.h"
#include "Capture.h"
#include "VideoCapture.h"
#include "AudioCapture.h"

static UINT32 m_videoCapCnt;
static UINT32 m_audioCapCnt;
static IMFActivate **m_ppVideoDevices;
static IMFActivate **m_ppAudioDevices;


Capture::~Capture()
{
}


int Capture::Init()
{
    HRESULT hr = S_OK;
    IMFAttributes *pAttributes = NULL;

	// Enumerate devices.
    hr = MFCreateAttributes(&pAttributes, 1);
    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
            );
    }

    if (SUCCEEDED(hr))
    {
        hr = MFEnumDeviceSources(pAttributes, &m_ppVideoDevices, &m_videoCapCnt);
    }

    SafeRelease(&pAttributes);

	// Enumerate devices.
    hr = MFCreateAttributes(&pAttributes, 1);
    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID
            );
    }

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

	for (UINT32 i = 0; i < m_videoCapCnt; i++)
	{
		SafeRelease(&m_ppVideoDevices[i]);
	}

	for (UINT32 i = 0; i < m_audioCapCnt; i++)
	{
		SafeRelease(&m_ppAudioDevices[i]);
	}

    return m_audioCapCnt;
}


int Capture::EnumVideoCature(std::vector<CString *> *vCaptureList)
{
    if (vCaptureList != NULL)
    {
        for (UINT32 i = 0; i < m_videoCapCnt; i++)
        {
            WCHAR * szFriendlyName;
            m_ppVideoDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &szFriendlyName, NULL);
			CString* str = new CString;
			str->Format(TEXT("%s"), szFriendlyName);
            vCaptureList->insert(vCaptureList->end(), str);
            CoTaskMemFree(szFriendlyName);
        }
    }

	return (int)m_videoCapCnt;
}


int Capture::EnumAudioCature(std::vector<CString *> *aCaptureList)
{
    if (aCaptureList != NULL)
    {
        for (UINT32 i = 0; i < m_audioCapCnt; i++)
        {
            WCHAR * szFriendlyName;
			m_ppAudioDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &szFriendlyName, NULL);
			CString* str = new CString;
			str->Format(TEXT("%s"), szFriendlyName);
            aCaptureList->insert(aCaptureList->end(), str);
            CoTaskMemFree(szFriendlyName);
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