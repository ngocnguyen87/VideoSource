#pragma once
#include "MMSystem.h"
//#include <avcodec.h>

#define WAVEHDR_NUM	50
#define RECORD_BUFFER_SIZE	640
#define PLAY_BUFFER_SIZE 2048

typedef void (*PWAVECALLBACKFUNC)(void *pData, int size, void* lpUser);
class CWave
{
public:
	CWave(void);
	virtual ~CWave(void);
	void SetWaveFormat(WAVEFORMATEX pcm);
	void	*m_pUser;
	LPWAVEHDR			m_pWhdr[WAVEHDR_NUM];
	WAVEFORMATEX		m_pcm;
};

/*
 **************************************************
 * Class CRecord definition
 * Writen by: sonltsdtc
 **************************************************
*/

class CRecord : public CWave
{
public:
	CRecord(void);
	virtual ~CRecord(void);

	static CRecord* GetInstance();
	BOOL Open(const char* szDevice = NULL);
	BOOL Start(void);
	BOOL Stop(void);
	BOOL Finish(void);
	void SetWIMDATACallback(PWAVECALLBACKFUNC pCBFunc, void *lpUser)
	{
		m_pUser = lpUser;
		pWaveInData = pCBFunc;
	}
	int setSoundInput(int nDevice);
	void HandleMicPlug(bool bType);
	void GetCurrentDevice(const char* szDevice);

	static UINT	waveHeaderThread(void* pUser);

	static CRecord*		m_pRecordInstance;
	HWAVEIN				m_hWaveIn;
	PWAVECALLBACKFUNC	pWaveInData;
	bool				m_bMuteAudio;
	bool				m_bResetWaveHdr;
	bool				m_bRunThread;
	HANDLE				m_hRunThread;
};

/*
 **************************************************
 * Class CPlay definition
 * Writen by: sonltsdtc
 **************************************************
*/

class CPlay : public CWave
{
public:
	CPlay(void);
	virtual ~CPlay(void);

	static CPlay* GetInstance();
	void setSoundOutput(int nDevice);
	bool SetOutVolume(USHORT left, USHORT right);
	BOOL Open(void);
	BOOL Start(void *pData, int size);
	BOOL Finish(void);
	BOOL IsOpen(void) {return m_bIsOpen;};
	void SetOpen(BOOL bOpen){m_bIsOpen = bOpen;};
	void SetWOMDONECallback(PWAVECALLBACKFUNC pCBFunc, void *lpUser)
	{
		m_pUser = lpUser;
		pWaveOutDone = pCBFunc;
	}
	void ResetWaveHeader();

	static CPlay*		m_pPlayInstance;
	HWAVEOUT			m_hWaveOut;
	PWAVECALLBACKFUNC	pWaveOutDone;

	BOOL				m_bIsOpen;

	//avcodec
	void InitAVCodec();


	void (*pavcodec_register_all)(void);
	void (*pavcodec_init)(void);

	//ReSampleContext* (*paudio_resample_init) (int output_channels, int input_channels, int output_rate, int input_rate);
	//int (*paudio_resample) (ReSampleContext *s, short *output, short *input, int nb_samples);
	//void (*paudio_resample_close) (ReSampleContext *s);

	int Resample(short *pSrc, short *pDst, int n_samples);

	//ReSampleContext *pContext;
	short m_pout_buffer[2048];
	//end avcodec

};

void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

