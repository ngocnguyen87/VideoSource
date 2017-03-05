// Render.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Render.h"
#include "VideoRender.h"


CRender::CRender()
{
	
}

CRender::CRender(void* pUser)
{
	m_pUser = pUser;
	m_pVideoRender = new CVideoRender(this);
}

CRender::~CRender()
{
	
}

int CRender::InitVideoRender(HWND hWnd)
{
	if(m_pVideoRender)
		return m_pVideoRender->InitDD(hWnd);

	return -1;
}

void CRender::writeData(int nStream, BYTE* pBuffer, int width, int height, unsigned long timestamp)
{
	if(nStream == 1)
	{
		if(m_pVideoRender)
			m_pVideoRender->writeVideoData(pBuffer, width, height, timestamp);
	}
}

void CRender::UpdateOverlay(RECT rect)
{
	if(m_pVideoRender)
		return m_pVideoRender->UpdateOverlay(rect);	
}

