#include "stdafx.h"
#include "Capture.h"
#include "VideoCapture.h"
#include "AudioCapture.h"

static UINT32 m_videoCapCnt;
static UINT32 m_audioCapCnt;
static vector<Capture *> videoCaptureList;
static vector<Capture *> audioCaptureList;


Capture::~Capture()
{
}


int Capture::Init()
{
    HRESULT hr = S_OK;
	IMFAttributes *pAttributes = NULL;
	static IMFActivate **m_ppDevices;

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
		hr = MFEnumDeviceSources(pAttributes, &m_ppDevices, &m_videoCapCnt);
		for (UINT32 i = 0; i < m_videoCapCnt; i++)
		{
			videoCaptureList.push_back(new VideoCapture(m_ppDevices[i]));
			SafeRelease(&m_ppDevices[i]);
		}
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
		hr = MFEnumDeviceSources(pAttributes, &m_ppDevices, &m_audioCapCnt);
		for (UINT32 i = 0; i < m_audioCapCnt; i++)
		{
			audioCaptureList.push_back(new AudioCapture(m_ppDevices[i]));
			SafeRelease(&m_ppDevices[i]);
		}
    }

    SafeRelease(&pAttributes);

    return (int)(m_videoCapCnt + m_audioCapCnt);
}


int Capture::Uninit()
{
    HRESULT hr = S_OK;
    IMFAttributes *pAttributes = NULL;

	Capture* capture = NULL;
	vector<Capture*>::iterator it;

	for (it = videoCaptureList.begin(); it != videoCaptureList.end();)
	{
		capture = *it;
		it = videoCaptureList.erase(it);
		delete capture;
	}

	for (it = audioCaptureList.begin(); it != audioCaptureList.end();)
	{
		capture = *it;
		it = audioCaptureList.erase(it);
		delete capture;
	}

    return m_audioCapCnt;
}


int Capture::EnumVideoCature(const std::vector<Capture *> *& captureList)
{
    if (captureList != NULL)
    {
		captureList = &videoCaptureList;
    }

	return (int)m_videoCapCnt;
}


int Capture::EnumAudioCature(const std::vector<Capture *> *& captureList)
{
	if (captureList != NULL)
	{
		captureList = &audioCaptureList;
	}

	return (int)m_audioCapCnt;
}


Capture* Capture::GetVideoCature(int index)
{
    if ((UINT32)index < m_videoCapCnt)
    {
        return videoCaptureList[index];
    }
    return NULL;
}


Capture* Capture::GetAudioCature(int index)
{
    if ((UINT32)index < m_audioCapCnt)
	{
		return audioCaptureList[index];
    }
    return NULL;
}