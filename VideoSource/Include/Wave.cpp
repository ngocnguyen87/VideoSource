#include "StdAfx.h"
#include ".\wave.h"
//#include "..\Include\EvisionSmartStruct.h"


CWave::CWave(void)
{
	m_pcm.cbSize = 0;
	m_pcm.nChannels = 1;
	m_pcm.nSamplesPerSec = 16000;
	m_pcm.wBitsPerSample = 16;
	m_pcm.nBlockAlign = m_pcm.nChannels*m_pcm.wBitsPerSample/8;
	m_pcm.nAvgBytesPerSec = m_pcm.nSamplesPerSec*m_pcm.nBlockAlign;
	m_pcm.wFormatTag = WAVE_FORMAT_PCM;
}

CWave::~CWave(void)
{
}

void CWave::SetWaveFormat(WAVEFORMATEX pcm)
{
	memcpy(&m_pcm, &pcm, sizeof(pcm));
}
/*
 **************************************************
 * Class CRecord implementation
 **************************************************
*/

CRecord* CRecord::m_pRecordInstance = NULL;

CRecord* CRecord::GetInstance()
{
	if(!m_pRecordInstance)
		m_pRecordInstance = new CRecord();

	return m_pRecordInstance;
}

CRecord::CRecord(void)
{
	m_bResetWaveHdr = false;
}

CRecord::~CRecord(void)
{

}

BOOL CRecord::Open(const char* szDevice)
{
	//Get Audio input device
	int nNumberDevice = waveInGetNumDevs();
	UINT nDevice;

	char m_sSoundRecording[50];
	memset(&m_sSoundRecording, 0, sizeof(m_sSoundRecording));
	if(szDevice != NULL)
	{
		strcpy(m_sSoundRecording, szDevice);

		WAVEINCAPS wc;
		memset (&wc, 0, sizeof (wc));

		for(int i=0; i< nNumberDevice; i++)
		{
			waveInGetDevCaps(i, &wc, sizeof(WAVEINCAPS));
			CHAR *szDeviceName = wc.szPname;
			//szDeviceName.TrimRight(" ");
			if(strstr(m_sSoundRecording, (char*)szDeviceName) != NULL)
			{
				nDevice = i;
				break;
			}
		}

	}
	else
		nDevice = WAVE_MAPPER;

	MMRESULT re = waveInOpen(&m_hWaveIn, nDevice, &m_pcm,
							 (DWORD)&waveInProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
	if (re != 0)
		return false;
	return true;
}

BOOL CRecord::Start(void)
{
	MMRESULT re;
	for (int i=0; i<WAVEHDR_NUM; i++)
	{
		m_pWhdr[i] = new WAVEHDR;
		m_pWhdr[i]->dwFlags = 0;
		m_pWhdr[i]->dwBufferLength = RECORD_BUFFER_SIZE;
		m_pWhdr[i]->lpData = new char[m_pWhdr[i]->dwBufferLength];
		re = waveInPrepareHeader(m_hWaveIn, m_pWhdr[i], sizeof(WAVEHDR));
		re = waveInAddBuffer(m_hWaveIn, m_pWhdr[i], sizeof(WAVEHDR));
	}
	re = waveInStart(m_hWaveIn);
	if (re != 0)
		return false;
	m_bRunThread = true;
	//m_hRunThread = AfxBeginThread(waveHeaderThread, this);
	return true;
}

BOOL CRecord::Stop(void)
{
	m_bRunThread = false;
	WaitForSingleObject(m_hRunThread, INFINITE);
	waveInStop(m_hWaveIn);
	Sleep(100);
	for(int i=0; i<WAVEHDR_NUM; i++)
	{
		waveInUnprepareHeader(m_hWaveIn, m_pWhdr[i], sizeof(WAVEHDR));
		delete m_pWhdr[i]->lpData;
	}

	waveInClose(m_hWaveIn);
	
	return true;
}

BOOL CRecord::Finish(void)
{
	return true;
}

int CRecord::setSoundInput(int nDevice)
{
	if (m_hWaveIn)
	{
		Stop();
	}
	MMRESULT result;
	m_pcm.nSamplesPerSec = 16000;
	result = waveInOpen(&m_hWaveIn, nDevice, &m_pcm,
		(DWORD)&waveInProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
	Start();

	return result;
}

UINT CRecord::waveHeaderThread(void* pUser)
{
	CRecord* pTHIS = (CRecord*)pUser;

	while(pTHIS->m_bRunThread)
	{
		for (int i=0; i<WAVEHDR_NUM; i++)
		{
			//TRACE("\n pWhdr->dwFlags = %d", pTHIS->m_pWhdr[i]->dwFlags);
			if(pTHIS->m_pWhdr[i]->dwFlags == WHDR_DONE)
			{
				waveInUnprepareHeader(pTHIS->m_hWaveIn, pTHIS->m_pWhdr[i], sizeof(WAVEHDR));
				waveInPrepareHeader(pTHIS->m_hWaveIn, pTHIS->m_pWhdr[i], sizeof(WAVEHDR));
				waveInAddBuffer(pTHIS->m_hWaveIn, pTHIS->m_pWhdr[i], sizeof(WAVEHDR));			
			}
		}
	
		Sleep(1);
	}

	return 0;
}

void CRecord::HandleMicPlug(bool bType)
{
	if(bType)//Mic plug in
	{
		
		//Thong bao thiet lap lai mic control
	}
}

void CRecord::GetCurrentDevice(const char* szDevice)
{
	//if(m_hWaveIn)
	//{
	//	UINT nDevice = 0;
	//	waveInGetID(m_hWaveIn, &nDevice);

	//	WAVEINCAPS wc;
	//	memset (&wc, 0, sizeof (wc));

	//	for(int i=0; i< waveInGetNumDevs(); i++)
	//	{
	//		waveInGetDevCaps(i, &wc, sizeof(WAVEINCAPS));
	//		
	//		if(wc.wMid)
	//		{
	//			TRACE("Get device");
	//		}
	//		
	//		//CString szDeviceName = CString(wc.szPname);
	//		//szDeviceName.TrimRight(" ");
	//		//if(strcmp(m_sSoundRecording, szDeviceName.GetBuffer()) == NULL)
	//		//{
	//		//	nDevice = i;
	//		//	break;
	//		//}
	//	}
	//}
}
/*
 **************************************************
 * Record callback function
 **************************************************
*/
void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	LPWAVEHDR pWhdr = (WAVEHDR*)dwParam1;
	CRecord *pRecord = (CRecord*)dwInstance;
	MMRESULT re;
	switch (uMsg)
	{
	case WIM_OPEN:
		{
			break;
		}
	case WIM_DATA:
		{
			if(pRecord->pWaveInData)
				pRecord->pWaveInData(pWhdr->lpData, pWhdr->dwBytesRecorded, pRecord->m_pUser);	
			//TRACE("\n pWhdr->dwFlags = %d", pWhdr->dwFlags);
			pWhdr->dwFlags = WHDR_DONE;
			
			re = waveInUnprepareHeader(hwi, pWhdr, sizeof(WAVEHDR));
			re = waveInPrepareHeader(hwi, pWhdr, sizeof(WAVEHDR));
			re = waveInAddBuffer(hwi, pWhdr, sizeof(WAVEHDR));
			break;
		}
	case WIM_CLOSE:
		{
			break;
		}
	default:
		break;
	}
}

/*
 **************************************************
 * Class CPlay implementation
 **************************************************
*/
CPlay* CPlay::m_pPlayInstance = NULL;

CPlay* CPlay::GetInstance()
{
	if(!m_pPlayInstance)
		m_pPlayInstance = new CPlay();

	return m_pPlayInstance;
}

CPlay::CPlay(void)
{
	MMRESULT re;
	for (int i=0; i<WAVEHDR_NUM; i++)
	{
		m_pWhdr[i] = new WAVEHDR;
		m_pWhdr[i]->dwFlags = WHDR_DONE;
		m_pWhdr[i]->dwBufferLength = PLAY_BUFFER_SIZE;
		m_pWhdr[i]->lpData = new char[m_pWhdr[i]->dwBufferLength];
	}

	//pContext = NULL;

	pavcodec_register_all = NULL;
	pavcodec_init = NULL;

	//paudio_resample_init = NULL;
	//paudio_resample = NULL;
	//paudio_resample_close = NULL;

}

CPlay::~CPlay(void)
{
	waveOutReset(m_hWaveOut);
	waveOutClose(m_hWaveOut);
	for (int i=0; i<WAVEHDR_NUM; i++)
	{
		delete [] m_pWhdr[i]->lpData;
		delete m_pWhdr[i];
	}
}

void CPlay::setSoundOutput(int nDevice)
{
	MMRESULT result;

	m_pcm.nSamplesPerSec = 24000;
	result = waveOutOpen(&m_hWaveOut, nDevice, &m_pcm,
							(DWORD)waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION);

	//if(result != 0)
	//	PTRACE(1,"CPlay::setSoundOutput. Could not open device");
}

bool CPlay::SetOutVolume(USHORT left, USHORT right)
{
	DWORD value = 0;
	UCHAR* p = (UCHAR *)&value;
	if(!m_hWaveOut)
		return false;
	if(left > 100)
		left = 100;
	if(right > 100)
		right = 100;
	USHORT l = ((float)left / 100) * 0xffff;
	USHORT r = ((float)right / 100) * 0xffff;
	memcpy(p, &l, sizeof(USHORT));
	memcpy(p + sizeof(USHORT), &r, sizeof(USHORT));
	HRESULT hr = waveOutSetVolume(m_hWaveOut, value);
	return true;
}


BOOL CPlay::Open(void)
{
	int nNumberDevice = waveOutGetNumDevs();
	int nDevice = WAVE_MAPPER;

	
	WAVEOUTCAPS wc;
	memset (&wc, 0, sizeof (wc));

	char m_sSoundOutput[50];
	
	//for(int i=0; i<nNumberDevice; i++)
	//{
	//	waveOutGetDevCaps(i, &wc, sizeof(WAVEOUTCAPS));
	//	if(strstr(wc.szPname, m_sSoundOutput) != NULL)
	//	{
	//		nDevice = i;
	//		break;
	//	}

	//}

	//m_pcm.nSamplesPerSec = 24000;
	

	MMRESULT re = waveOutOpen(&m_hWaveOut, nDevice, &m_pcm,
							(DWORD)waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
	if (re != 0)
	{
		SetOpen(FALSE);
		return false;
	}

	InitAVCodec();
	//pContext = paudio_resample_init(1, 1, 24000, 16000);

	SetOpen(TRUE);

	return true;
}

BOOL CPlay::Start(void *pData, int size)
{
	MMRESULT re; 
	for (int i=0; i<WAVEHDR_NUM; i++)
	{
		if (m_pWhdr[i]->dwFlags & WHDR_DONE)
		{
			m_pWhdr[i]->dwBufferLength = size;
			memcpy(m_pWhdr[i]->lpData, pData, size);

			//Resample((short*)pData, (short*)m_pWhdr[i]->lpData, size/2);
			//m_pWhdr[i]->dwBufferLength = size*3/2;

			re = waveOutPrepareHeader(m_hWaveOut, m_pWhdr[i], sizeof(WAVEHDR));
			re = waveOutWrite(m_hWaveOut, m_pWhdr[i], sizeof(WAVEHDR));
			if (re != 0)
			{
				ResetWaveHeader();
				return false;
			}
			return true;
		}
	}
	return false;
}

void CPlay::ResetWaveHeader()
{
	for (int i=0; i<WAVEHDR_NUM; i++)
	{
		m_pWhdr[i]->dwFlags = WHDR_DONE;
	}
}

BOOL CPlay::Finish(void)
{
	return true;
}

/*
 **************************************************
 * Play callback function
 **************************************************
*/
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	LPWAVEHDR pHdr = (WAVEHDR*)dwParam1;
	MMRESULT re;
	switch (uMsg)
	{
	case WOM_OPEN:
	case WOM_DONE:
			//re = waveOutUnprepareHeader(hwo, pHdr, sizeof(WAVEHDR));
			//break;
	case WOM_CLOSE:
	default:
		break;
	}
}


void CPlay::InitAVCodec()
{
	//HMODULE hModuleAvcodec;
	//HMODULE hModuleAvutil;

	//hModuleAvcodec = LoadLibrary("avcodec-51.dll");
	//if(hModuleAvcodec==NULL)
	//{
	//	AfxMessageBox("Thu muc chay hien tai khong co avcodec-51.dll");
	//	return;
	//}

	//hModuleAvutil = LoadLibrary("avutil-49.dll");
	//if(hModuleAvutil==NULL)
	//{
	//	AfxMessageBox("Thu muc chay hien tai khong co avutil-49.dll");
	//	return;
	//}

	//pavcodec_register_all = (void (*)(void))GetProcAddress(hModuleAvcodec,"avcodec_register_all");
	//pavcodec_init = (void (*)(void))GetProcAddress(hModuleAvcodec,"avcodec_init");

	//paudio_resample_init = (ReSampleContext* (*)(int, int, int, int))GetProcAddress(hModuleAvcodec,"audio_resample_init");
	//paudio_resample = (int (*)(ReSampleContext*, short *, short *, int))GetProcAddress(hModuleAvcodec,"audio_resample");
	//paudio_resample_close = (void (*)(ReSampleContext *))GetProcAddress(hModuleAvcodec,"audio_resample_close");


	//pavcodec_init();
	//pavcodec_register_all();
}

int CPlay::Resample(short *pSrc, short *pDst, int n_samples)
{
	//return paudio_resample(pContext, pDst, pSrc, n_samples);
	return 0;
}
