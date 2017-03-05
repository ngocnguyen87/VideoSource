#pragma once


typedef void (*PVSOFTCALLBACKVFUNCTION)(unsigned char*,int,unsigned long, int, void*);
typedef void (*PVSOFT_LOG)( void *, int i_level, const char *psz);


#define VSS_BALINE_PROFILE 66
#define VSS_MAIN_PROFILE 77
#define VSS_HIGHT_PROFILE 100

#define VSS_ENTROPY_CAVLC 0
#define VSS_ENTROPY_CABAC 1

#define VSS_RATE_CONTROL_VBR 0
#define VSS_RATE_CONTROL_CBR 1

#define VSS_FRAME_RATE_FULL 1 //30 for NTSC,25 for PAL
#define VSS_FRAME_RATE_15 2 //15 for all
#define VSS_FRAME_RATE_10 3 //10 frame/s
#define VSS_FRAME_RATE_5 6

#define VSS_COLOR_PAL 0
#define VSS_COLOR_NTSC 1


typedef struct VSS_SETTING
{
	int nProfile; //66 baseline, 77 main
	int nLevel; //Chi ho tro tu 1 den 4.2

	int nEntropyMode; //CAVLC or CABAC. Note rang CABAC khong ho trong trong baseline
	int nRateControlMode; //0: CBR,1 VBR
	int nSenceDectect; //1 enable, 0 disable

	int nBitrate; //Tinh theo kilobit

	int nContentEncoder; //1: Content 0:Source

	//lien quan den cac thu khac
	int nWidth;  //frame width
	int nHeight; //frame height
	int nIdrPicture; //khoang cach giua 2 frame I

	int nFrameRate; // 0 full, 15: 1, 10:2,5:3
	int nColor;  //0 PAL,1 NTSC
	int nSource; //0: Composite, 1: SVideo

	int nSendIp;
	int nSendPort;

}VSS_SETTING;

#include "LockData.h"

typedef struct _Packet
{
	BYTE* pData1;
	int   nSize1;
	BYTE* pData2;
	int   nSize2;
	__int64 dwTS;
	int   fEndOfGroup;
}Packet,*PPacket;

class  CircleBuffer
{
	BYTE* m_pBuffer;
	int m_nSize;
	int m_nWrite;
	int m_nSpace;
public:
	CircleBuffer() : m_pBuffer(NULL)
	{
	}

	virtual ~CircleBuffer()
	{
		if(m_pBuffer)
			delete []m_pBuffer;
	}

	int InitBuffer(int nSize)
	{
		if(m_pBuffer)
			delete []m_pBuffer;
		m_pBuffer = new BYTE[nSize];
		if(!m_pBuffer)
			return -1;
		m_nSpace = m_nSize = nSize;
		m_nWrite = 0;
		return 0;
	}

	int PushData(BYTE* pData, int nSize, PPacket pPkg)
	{
		if(!m_pBuffer)
			return -1;

		if(m_nSpace < nSize)
			return -1;
		pPkg->nSize1 = pPkg->nSize2 = 0;
		pPkg->pData1 = pPkg->pData2 = 0;
		//chunk 1
		int len1 = m_nSize - m_nWrite;
		if(len1 > nSize)
			len1 = nSize;
		pPkg->nSize1 = len1;
		pPkg->pData1 = m_pBuffer + m_nWrite;

		memcpy(pPkg->pData1, pData, pPkg->nSize1);
		nSize -= pPkg->nSize1;
		pData += pPkg->nSize1;

		if(nSize)
		{
			pPkg->nSize2 = nSize;
			pPkg->pData2 = m_pBuffer;
			memcpy(pPkg->pData2, pData, pPkg->nSize2);
		}

		m_nWrite = (m_nWrite + pPkg->nSize1 + pPkg->nSize2) % m_nSize;
		m_nSpace -= (pPkg->nSize1 + pPkg->nSize2);
		return 0;
	}

	void PopData(PPacket pPkg)
	{
		m_nSpace += (pPkg->nSize1 + pPkg->nSize2);
	}
};

class  CEncoder
{
protected:
	int m_nLogLevel;
	BOOL m_bIsInitialize;
	//static int m_cEncoders;

	LPVOID m_pUser;
	PVSOFTCALLBACKVFUNCTION m_pEncodedCallback;
	PVSOFT_LOG m_pLog;

#ifdef _USE_AUTOLOCK
	CMyKeyLock m_cSettingLock;
#endif

protected:
	void Log(int nLevel, char* psz, ...);

public:
	CEncoder(void);
	virtual ~CEncoder(void);

	virtual int setting(LPVOID pParam) { return 0; }
	virtual int getVideoSetting(LPVOID pParam) { return 0; }
	virtual int Init()=0;
	virtual int Exit() {return 0;};
	virtual int encode(BYTE* pData, DWORD dwSize, DWORD dwTS) {return 0; }

	virtual int changeBitrate(int nNewBitrate) { return 0; }
	virtual int fastUpdate() { return 0; }


	void SetLogLevel(int lvl) { m_nLogLevel = lvl; }
	void SetCallback(PVSOFTCALLBACKVFUNCTION pCallback, PVSOFT_LOG pLog, LPVOID pUser)
	{
		m_pEncodedCallback = pCallback;
		m_pLog = pLog;
		m_pUser = pUser;
	}

};
