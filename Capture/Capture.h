#pragma once

#include <string>

class CVideoCapture;
class CRecord;

using namespace std;

typedef void (*UPDATECALLBACK)(int, string, void*);

typedef void (*ONVIDEOSAMPLE)(DWORD , BYTE* , long, void* );
typedef void (*ONAUDIOSAMPLE)(void*, int, void*);

class __declspec(dllexport) CCapture
{
public:
	CCapture(void);
	CCapture(void* pPUser);
	~CCapture(void);

public:
	void getListDev();
	void getFormatDev(string szDeviceName);

	void updateListDev(int nIndex, string szDevName);
	void updateListCaps(int nIndex, string szCap);

	void OnVideoSample(DWORD dwTimeStamp, BYTE* pBuffer, long lBufferSize);
	static void OnAudioSample(void* pData, int nSize, void* pUser);

	void getVideoSize(int* nWidth, int* nHeight);
	void setFormatIndex(int nIndex);

	void setCallback(UPDATECALLBACK device, UPDATECALLBACK caps, ONVIDEOSAMPLE onsVideoSample, ONAUDIOSAMPLE onAudioSample);

	int startCaptureVideo();
	void stopCaptureVideo();

protected:
	void* m_pVideoSource;

	CVideoCapture*	m_pVideoCapture;
	CRecord*		m_pAudioCapture;

	UPDATECALLBACK m_pDeviceUpdate;
	UPDATECALLBACK m_pFormatUpdate;

	ONVIDEOSAMPLE		m_pOnVideoSample;
	ONAUDIOSAMPLE		m_pOnAudioSample;
};

