// Capture.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Capture.h"
#include "VideoCapture.h"
#include "Wave.h"


CCapture::CCapture(void)
{
}

CCapture::CCapture(void* pUser)
{
	m_pVideoSource = pUser;

	//Init Video Capture
	m_pVideoCapture = new CVideoCapture();
	m_pVideoCapture->setParent(this);

	//Init Audio Capture
	m_pAudioCapture = new CRecord();
	m_pAudioCapture->SetWIMDATACallback(OnAudioSample, this);
	m_pAudioCapture->Open(NULL);
	
}


CCapture::~CCapture(void)
{
	if(m_pAudioCapture)
		m_pAudioCapture->Stop();
}

void CCapture::getListDev()
{
	if(m_pVideoCapture)
		m_pVideoCapture->getListDevice();
}

void CCapture::getFormatDev(string szDeviceName)
{
	if(m_pVideoCapture)
		m_pVideoCapture->getListDeviceCaps(szDeviceName);
}

void CCapture::getVideoSize(int* nWidth, int* nHeight)
{
	if(m_pVideoCapture)
		m_pVideoCapture->getVideoSize(nWidth, nHeight);
}

void CCapture::setFormatIndex(int nIndex)
{
	if(m_pVideoCapture)
		m_pVideoCapture->setFormatIndex(nIndex);
}

void CCapture::setCallback(UPDATECALLBACK device, UPDATECALLBACK caps, ONVIDEOSAMPLE onVideoSample, ONAUDIOSAMPLE onAudioSample)
{
	m_pDeviceUpdate = device;
	m_pFormatUpdate = caps;
	m_pOnVideoSample = onVideoSample;
	m_pOnAudioSample = onAudioSample;
}

int CCapture::startCaptureVideo()
{
	if(m_pVideoCapture)
		m_pVideoCapture->startCapture();

	if(m_pAudioCapture)
		m_pAudioCapture->Start();

	return 0;
}

void CCapture::stopCaptureVideo()
{
	if(m_pVideoCapture)
		m_pVideoCapture->stopCapture();
}

void CCapture::updateListDev(int index, string szDevName)
{
	if(m_pDeviceUpdate)
		m_pDeviceUpdate(index, szDevName, m_pVideoSource);
}

void CCapture::updateListCaps(int index, string szCap)
{
	if(m_pFormatUpdate)
		m_pFormatUpdate(index, szCap, m_pVideoSource);
}

void CCapture::OnVideoSample(DWORD dwTimeStamp, BYTE* pBuffer, long lBufferSize)
{
	if(m_pOnVideoSample)
		m_pOnVideoSample(dwTimeStamp, pBuffer, lBufferSize, m_pVideoSource);
}

void CCapture::OnAudioSample(void* pData, int nSize, void* pUser)
{
	CCapture* pThis = (CCapture*)pUser;

	if(pThis->m_pOnAudioSample)
		pThis->m_pOnAudioSample(pData, nSize, pThis->m_pVideoSource);
}