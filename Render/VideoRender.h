#pragma once

#include <ddraw.h>

class CRender;

typedef struct
{
	LPBYTE	lpLum;
	LPBYTE	lpCr;
	LPBYTE	lpCb;
	DWORD	PTS;
	int		width;
	int		height;
} s_pict;

struct _pic // YUV
{
	int nWidth;
	int nHeight;
	DWORD pts;
	unsigned char *pBuff;
	_pic()
	{
		nWidth = 0;
		nHeight = 0;
		pts = 0;
		pBuff = NULL;
	}
};

class CVideoRender
{
public:
	CVideoRender(void);
	CVideoRender(void* pUser);

	~CVideoRender(void);

	int InitDD(HWND hWnd);
	int createOverlaySurface(int nWidth, int nHeight);

	void ClearSurface(LPDIRECTDRAWSURFACE7 lpSurface, int red=0, int green=0, int blue=0);

	void writeVideoData(const unsigned char* pData, int nWidth, int nHeight, unsigned long timeStamp);
	s_pict* CreatePicture(int width, int height);
	int CopyPict2SurYV12(s_pict *lpPict);

	int CreateOfflineSurface(int width,int height);

	void UpdateOverlay(RECT rect);

protected:
	CRender* m_pRender;

	HWND m_hWnd;
	int		m_nWidthPreview;
	int		m_nHeightPreview;

	int		m_nOffSurfaceWidth;
	int		m_nOffSurfaceHeight;

	static LPDIRECTDRAW7			m_lpDirectDraw;
	static LPDIRECTDRAWSURFACE7	m_lpPrimarySurface;
	static LPDIRECTDRAWSURFACE7	m_lpFrontSurface;
	static LPDIRECTDRAWSURFACE7	m_lpBackSurface;
	static LPDIRECTDRAWSURFACE7	m_lpLogSurface;

	LPDIRECTDRAWSURFACE7			m_lpOfflineSurface;	

	HANDLE	m_hMutex;
};

