
// VideoSourceDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <string>
#include "PictureCtrl.h"
#include "RtpTransmitter.h"
#include <iostream>
#include "afxcmn.h"
#include "resource.h"
#include "M3U8File.h"

using namespace std;

#define AUDIO_STREAM 0
#define VIDEO_STREAM 1

class CCapture;
class CRender;
class CVideoPlay;
class CFileRecorder;
class CPlay;
class M3U8File;

// CVideoSourceDlg dialog
class CVideoSourceDlg : public CDialogEx
{
// Construction
public:
	CVideoSourceDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_VIDEOSOURCE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


public:
	
	static void updateListDev(int nIndex, string szDevName, void* pUser);
	static void updateListCaps(int nIndex, string szCap, void* pUser);

	static void OnVideoSample(DWORD dwTimeStamp, BYTE* pBuffer, long lBufferSize, void* pUser);
	static void OnAudioSample(void* pData, int nSize, void* pUser);

	void displayFrame(BYTE* pBuffer, long lBufferSize, DWORD dwTimeStamp);
	void UpdateOverlay(RECT videoRect);

	void writeAudioData(void* pData, int nSize);

	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);

	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnCbnSelchangeCombo2();

	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedStartStream();
	afx_msg void OnBnClickedStopStream();

	afx_msg void OnBnClickedRecord();
	afx_msg void OnBnClickedStopRec();

	void UpdatePlayListFile();


public:

	CCapture*			m_pCapture;
	CRender*			m_pRender;
	CVideoPlay*			m_hVideoWindow;
	CFileRecorder*		m_pFileRecoder;

	CPlay*				m_pPlay;

	M3U8File*			m_pListFile;

	RtpTransmitter m_RtpTransmitter;

	bool m_bReadyEncode;
	bool m_bIsRecoding;

	CComboBox m_cbDeviceList;
	CComboBox m_cbFormatList;

	CIPAddressCtrl m_sIpAddress;

	int m_nPort;
	int m_nBitrate;

	HANDLE	m_hVideoMutex;
	
	DWORD m_dwTime;

	//For playlist
	int m_nIndex;
	char m_szFileName[20];
};
