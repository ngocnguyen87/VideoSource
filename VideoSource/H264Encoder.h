#pragma once

#include <stdint.h>
extern "C"
{
	#include "..\x264\x264.h"
} 


typedef void (*PENCODECALLBACK)(BYTE*, int, DWORD, void*);

typedef struct x264_setting
{
	int nWidth;
	int nHeight;
	int nBitrate;

	PENCODECALLBACK pEncodeCallback;
};


class CH264Encoder
{
public:
	CH264Encoder(void);
	CH264Encoder(void* pUser);

	~CH264Encoder(void);

	int Init(void* pParam);
	void Exit();

	int encode(BYTE* pData, DWORD dwSize, DWORD dwTS);

	void fastUpdate(){ m_bFastupdate = true;};

protected:
	void* m_pVideoSrc;

	x264_t* m_hEncoder;
	x264_param_t param;

	BYTE*	m_pBuffer;
	int		m_nBufferSize;

	PENCODECALLBACK m_pEncodeCallback;

	bool	m_bFastupdate;

	HANDLE m_hMutex;
};



