#include "stdafx.h"
#include "Capture.h"


int Capture::EnumVideoCature(std::vector<WCHAR *> *vCaptureList)
{
	HRESULT hr = S_OK;
	UINT32 videoCapCnt;
	IMFActivate **ppVideoDevices = NULL;
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
		hr = MFEnumDeviceSources(pAttributes, &ppVideoDevices, &videoCapCnt);

		if (vCaptureList != NULL)
		{
			for (UINT32 i = 0; i < videoCapCnt; i++)
			{
				WCHAR * szFriendlyName;
				ppVideoDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &szFriendlyName, NULL);
				WCHAR * str = new WCHAR[wcslen(szFriendlyName)];
				StrCpyW(str, szFriendlyName);
				vCaptureList->insert(vCaptureList->end(), str);
				CoTaskMemFree(szFriendlyName);
				SafeRelease(&ppVideoDevices[i]);
			}
		}
	}

	SafeRelease(&pAttributes);

	return (int)videoCapCnt;
}


int Capture::EnumAudioCature(std::vector<WCHAR *> *aCaptureList)
{
	HRESULT hr = S_OK;
	UINT32 audioCapCnt;
	IMFActivate **ppAudioDevices = NULL;
	IMFAttributes *pAttributes = NULL;

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
		hr = MFEnumDeviceSources(pAttributes, &ppAudioDevices, &audioCapCnt);

		if (aCaptureList != NULL)
		{
			for (UINT32 i = 0; i < audioCapCnt; i++)
			{
				WCHAR * szFriendlyName;
				ppAudioDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &szFriendlyName, NULL);
				WCHAR * str = new WCHAR[wcslen(szFriendlyName)];
				StrCpyW(str, szFriendlyName);
				aCaptureList->insert(aCaptureList->end(), str);
				CoTaskMemFree(szFriendlyName);
				SafeRelease(&ppAudioDevices[i]);
			}
		}
	}

	SafeRelease(&pAttributes);

	return audioCapCnt;
}


Capture* Capture::GetVideoCature(int index)
{
    return NULL;
}


Capture* Capture::GetAudioCature(int index)
{
    return NULL;
}