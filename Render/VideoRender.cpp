#include "StdAfx.h"
#include "VideoRender.h"
#include "Render.h"
#include <iostream>

using namespace std;

LPDIRECTDRAW7			CVideoRender::m_lpDirectDraw			= NULL;
LPDIRECTDRAWSURFACE7	CVideoRender::m_lpPrimarySurface		= NULL;
LPDIRECTDRAWSURFACE7	CVideoRender::m_lpFrontSurface			= NULL;
LPDIRECTDRAWSURFACE7	CVideoRender::m_lpBackSurface			= NULL;
LPDIRECTDRAWSURFACE7	CVideoRender::m_lpLogSurface			= NULL;

DDPIXELFORMAT g_PixelFormats[] = {
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('Y','V','1','2'),0,0,0,0,0},  //YV12
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('Y','U','Y','2'),0,0,0,0,0},  // YUV422	
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16,  0x0000F800, 0x000007E0, 0x0000001F, 0},
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 24,  0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000},   // 24-bit
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32,  0x00FF0000 , 0x0000FF00 , 0x000000FF , 0xFF000000}, //32bits
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32,  0x000000FF , 0x0000FF00 , 0x00FF0000 , 0}, //32bits
	{sizeof(DDPIXELFORMAT), DDPF_ALPHA, 0, 8,  0x00 , 0x00 , 0x00 , 0x000000FF}
};

CVideoRender::CVideoRender(void)
{
}

CVideoRender::CVideoRender(void* pUser)
{
	m_pRender = (CRender*)pUser;

	m_nWidthPreview = 0;
	m_nHeightPreview = 0;

	m_nOffSurfaceWidth = 0;
	m_nOffSurfaceHeight = 0;

	m_lpOfflineSurface = NULL;

	m_hMutex = CreateMutex(NULL,FALSE,NULL);
}

CVideoRender::~CVideoRender(void)
{
}

int CVideoRender::InitDD(HWND hWnd)
{
	HRESULT hr;
	DDSURFACEDESC2 ddsd;

	RECT rWindowRect;
	GetClientRect(hWnd, &rWindowRect);
	m_nWidthPreview = rWindowRect.right;
	m_nHeightPreview = rWindowRect.bottom;

	m_hWnd = hWnd;

	hr = DirectDrawCreateEx(NULL, (VOID**)&m_lpDirectDraw, IID_IDirectDraw7, NULL);
	if(hr != S_OK)
	{
		cout<<"\nCVideoRender::InitDD DirectDrawCreateEx FAILED ";
		return -1;
	}
	
	hr = m_lpDirectDraw->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);
	if(hr != S_OK)
	{
		cout<<"\CVideoRender::InitDD SetCooperativeLevel FAILED ";
		return -1;
	}

	//Create primary surface
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);	
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	hr = m_lpDirectDraw->CreateSurface(&ddsd, &m_lpPrimarySurface, NULL);
	if(hr != S_OK)
	{
		cout<<"\CVideoRender::InitDD CreateSurface m_lpPrimarySurface FAILED ";
		return -1;
	}
	
	hr = createOverlaySurface(m_nWidthPreview, m_nHeightPreview);
	if(hr != S_OK)
	{
		cout<<"\CVideoRender::InitDD createOverlaySurface  FAILED ";
		return -1;
	}

	//UpdateOverlay();

	return 0;
}

int CVideoRender::createOverlaySurface(int nWidth, int nHeight)
{
	//Create Overlay Surface
	HRESULT hr;

	DDSURFACEDESC2 ddsd;
	int buffercount = 2;

	ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd); 
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT |
					DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_FLIP | DDSCAPS_COMPLEX |DDSCAPS_VIDEOMEMORY;
	ddsd.dwBackBufferCount = buffercount;
    ddsd.dwWidth  = nWidth;
    ddsd.dwHeight = nHeight;
	ddsd.ddpfPixelFormat = g_PixelFormats[0];//Create YUV12 Overlay

	hr = m_lpDirectDraw->CreateSurface(&ddsd, &m_lpFrontSurface, NULL);
	if(hr != S_OK)
	{
		ddsd.ddpfPixelFormat = g_PixelFormats[1];
		hr = m_lpDirectDraw->CreateSurface(&ddsd, &m_lpFrontSurface, NULL);
		if(hr != S_OK)
		{
			return -1;
		}
	}

	//Attach BackSuface to OverlaySurface
	//Get back bufferd
	DDSCAPS2 ddscaps;
	ZeroMemory(&ddscaps, sizeof(ddscaps));
	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

	hr = m_lpFrontSurface->GetAttachedSurface(&ddscaps, &m_lpBackSurface);
	if(hr != S_OK)
	{
		cout<<"\n";
		return -1;
	}

	for (int i=0; i<=buffercount; i++)
	{
		ClearSurface(m_lpBackSurface);
		hr = m_lpFrontSurface->Flip(NULL, DDFLIP_WAIT);
	}

	return 0;
}

void CVideoRender::ClearSurface(LPDIRECTDRAWSURFACE7 lpSurface, int red, int green, int blue)
{
	if (lpSurface == NULL)
		return;

	DDPIXELFORMAT ddpf;
	DDBLTFX ddbltfx;
	HRESULT hr;

	ddbltfx.dwSize = sizeof(ddbltfx);
	
	memset(&ddpf, 0, sizeof(ddpf));
	ddpf.dwSize = sizeof(ddpf);
	lpSurface->GetPixelFormat(&ddpf);

	if (ddpf.dwFourCC == MAKEFOURCC('Y', 'V', '1', '2'))
	{
		DDSURFACEDESC2 ddsd;

		ddsd.dwSize = sizeof(ddsd);	
		hr = lpSurface->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		if (hr != 0)
		{
			if(hr==DDERR_SURFACELOST)
			{
				lpSurface->Restore();
				return;
			}
			else
				return;
				//ASSERT(0);
			return;
		}
		DWORD y, u, v;
		y = 299*red + 587*green + 114*blue;
		u = -147*red - 289*green + 437*blue;
		v = 615*red - 515*green - 100*blue;

		y /= 1000;
		u /= 1000;
		v /= 1000;

		LPBYTE lpSurfaceData = LPBYTE(ddsd.lpSurface);
		memset(lpSurfaceData, y, ddsd.lPitch*ddsd.dwHeight);
		lpSurfaceData += ddsd.lPitch*ddsd.dwHeight;

		memset(lpSurfaceData, u+0x74, ddsd.lPitch*ddsd.dwHeight/4);
		lpSurfaceData += ddsd.lPitch*ddsd.dwHeight/4;

		memset(lpSurfaceData, v+0x74, ddsd.lPitch*ddsd.dwHeight/4);
		lpSurface->Unlock(NULL);
	}
	else if (ddpf.dwFourCC == MAKEFOURCC('Y', 'U', 'Y', '2'))
	{ 
		ddbltfx.dwFillColor = 0x007F007F;
		hr = lpSurface->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx);

		if(hr!=0)
		{
			if(hr==DDERR_SURFACELOST)
				lpSurface->Restore();
			else
			{
				//ASSERT(0);
				return;
			}
		}
	}
	else
	{
		ddbltfx.dwFillColor = 0;
		hr = lpSurface->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx);
		if(hr!=0)
		{
			if(hr==DDERR_SURFACELOST)
				lpSurface->Restore();
			else
			{
				//ASSERT(0);
				return;
			}
		}
	}	
}

void CVideoRender::writeVideoData(const unsigned char* pData, int nWidth, int nHeight, unsigned long timeStamp)
{
	WaitForSingleObject(m_hMutex, INFINITE);

	int ret = 0;
	HRESULT hr;

	int framesize = nWidth * nHeight;

	LPBYTE pY = (LPBYTE)pData;
	LPBYTE pCr = pY + framesize;
	LPBYTE pCb = pCr + framesize / 4;

	s_pict* pict = NULL;
	pict = CreatePicture(nWidth, nHeight);

	int index = 0;
	for(int i = 0; i < nHeight; i++)
	{
		memcpy(pict->lpLum + index, pY, nWidth);
		pY += nWidth;
		index += nWidth;
	}
	index = 0;
	for (int i = 0; i < nHeight/2; i++)
	{
		memcpy(pict->lpCr + index, pCr, nWidth/2);
		memcpy(pict->lpCb + index, pCb, nWidth/2);
		index += nWidth/2;
		pCr += nWidth/2;
		pCb += nWidth/2;
	}

	//Copy Picture to Offline Surface
	ret = CopyPict2SurYV12(pict);
	if(ret == -1)
	{
		return;
	}

	//Attach Offline Surface to BackBuffer
	RECT rDesRect;
	RECT rSrcRect;
		
	DDBLTFX ddbltfx;
	memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwDDFX = DDBLTFX_NOTEARING;

	SetRect(&rDesRect, 1, 1, m_nWidthPreview, m_nHeightPreview);

	hr = m_lpBackSurface->Blt(NULL, m_lpOfflineSurface, NULL, DDBLT_ASYNC | DDBLT_DDFX, &ddbltfx);

	hr=m_lpFrontSurface->Flip(NULL,DDFLIP_WAIT);

	delete pict->lpLum;
	pict->lpLum = NULL;

	delete pict;
	pict = NULL;

	ReleaseMutex(m_hMutex);
}

int CVideoRender::CopyPict2SurYV12(s_pict *lpPict)
{
	DDSURFACEDESC2 ddsd;
	HRESULT hr;
	LPBYTE lpSurfaceData;
	int i;
	int width = lpPict->width, height = lpPict->height;
	int lPitch;

	if ((m_nOffSurfaceWidth != width) || (m_nOffSurfaceHeight != height))
		CreateOfflineSurface(width, height);

	if (m_lpOfflineSurface == NULL)
	{
		return -1;
	}

	//_pic srcPic;
	//srcPic.nWidth = width;
	//srcPic.nHeight = height;
	//srcPic.pBuff = lpPict->lpLum;

	LPBYTE lpLum = lpPict->lpLum;
	LPBYTE lpCr = lpPict->lpCr;
	LPBYTE lpCb = lpPict->lpCb;

	ddsd.dwSize = sizeof(ddsd);
	hr = m_lpOfflineSurface->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR |
									DDLOCK_WAIT|DDLOCK_WRITEONLY, NULL);
	if (hr != 0)
	{
		if(hr==DDERR_SURFACELOST)
		{
			//Restore();
			cout<<"\nSurfacelost";
		}
		return -1;

	}

	lPitch=ddsd.lPitch;
	lpSurfaceData = LPBYTE(ddsd.lpSurface);

	for (i=m_nOffSurfaceHeight; i>0; i--)
	{
		memcpy(lpSurfaceData,lpLum,width);
		lpSurfaceData+=lPitch;
		lpLum+=width;
	}

	width /= 2;
	height /= 2;
	lPitch/=2;
	LPBYTE lpSurfaceData2=lpSurfaceData + lPitch*m_nOffSurfaceHeight/2;

	for (i=m_nOffSurfaceHeight/2; i>0; i--)
	{
		memcpy(lpSurfaceData,lpCb,width);		
		memcpy(lpSurfaceData2,lpCr,width);

		lpSurfaceData+=lPitch;
		lpSurfaceData2+=lPitch;
		lpCb+=width;
		lpCr+=width;
	}

	m_lpOfflineSurface->Unlock(NULL);
}

int CVideoRender::CreateOfflineSurface(int width,int height)
{		 
	DDSURFACEDESC2 ddsd;
	HRESULT hr;
	
	if (m_lpOfflineSurface)
		m_lpOfflineSurface->Release();
	m_lpOfflineSurface = NULL;

	//create offline screen
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	ddsd.dwWidth = width;
	ddsd.dwHeight = height;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.ddpfPixelFormat = g_PixelFormats[0];
	hr = m_lpDirectDraw->CreateSurface(&ddsd, &m_lpOfflineSurface, NULL);

	if (FAILED(hr))
	{
		return -1;
	}


	m_nOffSurfaceHeight = height;
	m_nOffSurfaceWidth = width;

	return 0;
}

s_pict* CVideoRender::CreatePicture(int width, int height)
{
	s_pict* pPict;
	pPict = new s_pict;
	pPict->lpLum = new BYTE[width*height*3/2];
	pPict->lpCr = pPict->lpLum + width*height;
	pPict->lpCb = pPict->lpCr + width*height/4;
	pPict->width = width;
	pPict->height = height;
	pPict->PTS = 0;

	return pPict;
}

void CVideoRender::UpdateOverlay(RECT rect)
{
	//RECT rect;
	//::GetClientRect(m_hWnd, &rect);
	//::ClientToScreen(m_hWnd, (LPPOINT)(&rect.left));
	//::ClientToScreen(m_hWnd, (LPPOINT)(&rect.right));

	

	if (m_lpFrontSurface)
	{
		DDOVERLAYFX overlayFx;

		ZeroMemory(&overlayFx, sizeof(overlayFx));
		overlayFx.dwSize = sizeof(overlayFx);
		overlayFx.dckDestColorkey.dwColorSpaceHighValue = RGB(0, 0, 0);
		overlayFx.dckDestColorkey.dwColorSpaceLowValue = RGB(0,0,0);

/*		float ratio = (float)(rect.right - rect.left) / (float)(rect.bottom - rect.top);
		if (ratio < 16.0f/9.0f)
		{
			int newH = (rect.right - rect.left) * 9 / 16;
			int delta = (rect.bottom - rect.top - newH) / 2;
			rect.top += delta;
			rect.bottom -= (delta + 1);
		}
		else
		{
			int newW = (rect.bottom - rect.top) * 16 / 9;
			int delta = (rect.right - rect.left - newW) / 2;
			rect.left += delta;
			rect.right -= (delta + 1);
		}	*/

		HRESULT hr = m_lpFrontSurface->UpdateOverlay(NULL, m_lpPrimarySurface,&rect,
									DDOVER_SHOW | DDOVER_KEYDESTOVERRIDE, &overlayFx);
		
		if(hr!=0)
		{
		}
	}
}

