#pragma once
#include "encoder.h"
#include "CommonAutoLock.h"

class CVSS45Encoder :
	public CEncoder
{
public:
	CVSS45Encoder(void);
	virtual ~CVSS45Encoder(void);
	static CVSS45Encoder* getInstance();
	static void removeInstance();
	int setting(LPVOID pParam);
	int getVideoSetting(LPVOID pParam);
	int Init();
	int Exit();
	int encode(BYTE* pData, DWORD dwSize, DWORD dwTS);

	int changeBitrate(int nNewBitrate);
	int fastUpdate();
protected:
	void* m_params;
	VSS_SETTING m_Setting;
	static CVSS45Encoder* m_pInstance;

	CCommonKeyLock keyEncoder;
	int m_nExit;
	void* m_hEncoder;
	BOOL m_bFastUpdate;
};
