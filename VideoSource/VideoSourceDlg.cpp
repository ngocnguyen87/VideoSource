
// VideoSourceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VideoSource.h"
#include "VideoSourceDlg.h"
#include "VideoPlay.h"
#include "afxdialogex.h"

#include "..\Capture\Capture.h"
#include "..\Render\Render.h"

#include "FileRecoder.h"
#include "M3U8File.h"

#include "Wave.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CVideoSourceDlg dialog




CVideoSourceDlg::CVideoSourceDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVideoSourceDlg::IDD, pParent)
	, m_nPort(0)
	, m_nBitrate(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CVideoSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_VIDEO, m_hVideoWindow);
	DDX_Control(pDX, IDC_COMBO1, m_cbDeviceList);
	DDX_Control(pDX, IDC_COMBO2, m_cbFormatList);
	DDX_Control(pDX, IDC_IPADDRESS, m_sIpAddress);
	DDX_Text(pDX, IDC_PORT, m_nPort);
	DDX_Text(pDX, IDC_BITRATE, m_nBitrate);
}

BEGIN_MESSAGE_MAP(CVideoSourceDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, &CVideoSourceDlg::OnBnClickedStart)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CVideoSourceDlg::OnCbnSelchangeCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CVideoSourceDlg::OnCbnSelchangeCombo2)
	ON_WM_MOVING()
	ON_BN_CLICKED(IDC_STOP, &CVideoSourceDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_START_STREAM, &CVideoSourceDlg::OnBnClickedStartStream)
	ON_BN_CLICKED(IDC_STOP_STREAM, &CVideoSourceDlg::OnBnClickedStopStream)
	ON_BN_CLICKED(IDC_RECORD, &CVideoSourceDlg::OnBnClickedRecord)
	ON_BN_CLICKED(IDC_STOP_REC, &CVideoSourceDlg::OnBnClickedStopRec)
END_MESSAGE_MAP()


// CVideoSourceDlg message handlers

BOOL CVideoSourceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ShowWindow(SW_MINIMIZE);

	// TODO: Add extra initialization here
	m_nIndex = 1;

	m_pListFile = new M3U8File();
	m_pListFile->setPath("http://192.168.16.102/webtest/");
	m_pListFile->releaseFile();

	m_bIsRecoding = false;

	SetDlgItemText(IDC_IPADDRESS, "192.168.6.112");
	SetDlgItemInt(IDC_PORT, 15000);
	SetDlgItemInt(IDC_BITRATE, 2000);

	SetDlgItemInt(IDC_BITRATE_REC, 2000);

	m_hVideoMutex = CreateMutex(NULL, FALSE, NULL);

	//Init record
	m_pFileRecoder = NULL;
	m_pFileRecoder = new CFileRecorder(this);
	//End record

	//Init Render
	m_hVideoWindow = new CVideoPlay();
	m_hVideoWindow->Create(IDD_DISPLAY);
	m_hVideoWindow->ShowWindow(SW_NORMAL);
	m_hVideoWindow->setParent(this);

	m_pRender = new CRender(this);
	if(m_pRender->InitVideoRender(m_hVideoWindow->m_hWnd) == 0)
	{
		RECT videoRect;

		m_hVideoWindow->GetWindowRect(&videoRect);

		if(m_pRender)
			m_pRender->UpdateOverlay(videoRect);
	}
	//End Render

	//Init Capture
	m_pCapture = new CCapture(this);
	m_pCapture->setCallback(updateListDev, updateListCaps, OnVideoSample, OnAudioSample);
	m_pCapture->getListDev();

	//Focus to first device
	if(m_cbDeviceList.GetCount()>0)
	{
		m_cbDeviceList.SetCurSel(0);

		CString strDevice;
		m_cbDeviceList.GetLBText(0, strDevice);

		string sDevice(strDevice.GetString());
		m_cbFormatList.ResetContent();
		m_pCapture->getFormatDev(sDevice);
		m_cbFormatList.SetCurSel(0);
	}
	//End Capture

	//Init sound Player
	m_pPlay = new CPlay();
	m_pPlay->Open();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVideoSourceDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVideoSourceDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVideoSourceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CVideoSourceDlg::OnBnClickedStart()
{
	int nRet = 0;
	if(m_pCapture)
		nRet = m_pCapture->startCaptureVideo();
}

void CVideoSourceDlg::OnBnClickedStop()
{

	if(m_pCapture)
		m_pCapture->stopCaptureVideo();

	/*if(m_pFileRecoder && m_bIsRecoding)
		m_pFileRecoder->stopRecord();*/

	OnBnClickedStopRec();

	if(m_pListFile)
		m_pListFile->releaseFile(true);
}	

void CVideoSourceDlg::OnBnClickedRecord()
{
	//UpdateData();

	WaitForSingleObject(m_hVideoMutex, INFINITE);

	DWORD processTime = GetTickCount();

	if(!m_bIsRecoding)
	{
		int nWidth;
		int nHeight;

		if(m_pCapture)
			m_pCapture->getVideoSize(&nWidth, &nHeight);

		EncodeParam param;

		memset(m_szFileName, 0, sizeof(m_szFileName));

		sprintf(m_szFileName, "sample%d.ts", m_nIndex);
		m_nIndex++;

		param.sFileName = m_szFileName;
		param.nVideoCodec = CODEC_H264;
		param.nBitrate = 512;//GetDlgItemInt(IDC_BITRATE_REC);
		param.nWidth = nWidth;
		param.nHeight = nHeight;
		param.nFrameRate = 30;

		param.nAudioCodec = CODEC_MP3;
		param.sample_rate = 16000;
		param.bit_per_sample = 16;
		param.bit_rate = 64000;
		param.channels = 1;
		
		m_dwTime = GetTickCount();

		m_pFileRecoder->startRecord(&param);		

		m_bIsRecoding = true;
	}

	processTime = GetTickCount() - processTime;

	TRACE("\nCVideoSourceDlg::OnBnClickedRecord() processTime = %d", processTime);

	ReleaseMutex(m_hVideoMutex);
}


void CVideoSourceDlg::OnBnClickedStopRec()
{
	WaitForSingleObject(m_hVideoMutex, INFINITE);

	DWORD processTime = GetTickCount();

	if(m_bIsRecoding)
	{
		m_pFileRecoder->stopRecord();

		m_bIsRecoding = false;

		//Add to PlayList
		if(m_pListFile)
			m_pListFile->addFile(m_szFileName);
	}

	processTime = GetTickCount() - processTime;

	TRACE("\nCVideoSourceDlg::OnBnClickedStopRec() processTime = %d", processTime);

	ReleaseMutex(m_hVideoMutex);
}

void CVideoSourceDlg::updateListDev(int index, string szDevName, void* pUser)
{
	CVideoSourceDlg* pThis = (CVideoSourceDlg*)pUser;
	pThis->m_cbDeviceList.InsertString(index, CString(szDevName.c_str()));
}

void CVideoSourceDlg::updateListCaps(int index, string szCap, void* pUser)
{
	CVideoSourceDlg* pThis = (CVideoSourceDlg*)pUser;
	CString str;
	str  = szCap.c_str();
	pThis->m_cbFormatList.InsertString(index, str);	
}

void CVideoSourceDlg::OnVideoSample(DWORD dwTimeStamp, BYTE* pBuffer, long lBufferSize, void* pUser)
{
	CVideoSourceDlg* pThis = (CVideoSourceDlg*)pUser;
	
	//Preview
	pThis->displayFrame(pBuffer, lBufferSize, dwTimeStamp);

}

void CVideoSourceDlg::displayFrame(BYTE* pBuffer, long lBufferSize, DWORD dwTimeStamp)
{
	int nWidth, nHeight;

	if(m_pCapture)
		m_pCapture->getVideoSize(&nWidth, &nHeight);

	if(m_pRender)
		m_pRender->writeData(VIDEO_STREAM, pBuffer, nWidth, nHeight, dwTimeStamp);   

	if(GetTickCount() - m_dwTime >= 10000)//10s thi ngat
	{
		OnBnClickedStopRec();

		//Tao file playlist moi
		if(m_pListFile)
			m_pListFile->releaseFile();

		OnBnClickedRecord();
	}

	WaitForSingleObject(m_hVideoMutex, INFINITE);

	if(m_pFileRecoder && m_bIsRecoding)
		m_pFileRecoder->writeData(VIDEO_STREAM, pBuffer, lBufferSize, dwTimeStamp);

	ReleaseMutex(m_hVideoMutex);
}

void CVideoSourceDlg::OnAudioSample(void* pData, int nSize, void* pUser)
{
	CVideoSourceDlg* pThis = (CVideoSourceDlg*)pUser;

	//pThis->m_pPlay->Start(pData, nSize);

	pThis->writeAudioData(pData, nSize);

}

void CVideoSourceDlg::writeAudioData(void* pData, int nSize)
{
	if(m_pFileRecoder)
		m_pFileRecoder->writeData(AUDIO_STREAM, pData, nSize, 0);
}


void CVideoSourceDlg::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
	CString strDevice;
	int nIndex = 0;
	m_cbDeviceList.GetLBText(nIndex, strDevice);

	string szDeviceName(strDevice.GetString());
	m_cbFormatList.ResetContent();

	m_pCapture->getFormatDev(szDeviceName);

	m_cbFormatList.SetCurSel(0);
}


void CVideoSourceDlg::OnCbnSelchangeCombo2()
{
	// TODO: Add your control notification handler code here
	int nIndex = m_cbFormatList.GetCurSel();
	m_pCapture->setFormatIndex(nIndex);
}


void CVideoSourceDlg::OnMoving(UINT fwSide, LPRECT pRect)
{
	CDialogEx::OnMoving(fwSide, pRect);

	// TODO: Add your message handler code here
}

void CVideoSourceDlg::UpdateOverlay(RECT videoRect)
{
	if(m_pRender)
		m_pRender->UpdateOverlay(videoRect);
}


void CVideoSourceDlg::OnBnClickedStartStream()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	DWORD addr;
	m_sIpAddress.GetAddress(addr);
	m_RtpTransmitter.AddSendAddr(htonl(addr), m_nPort);

	int nBitRate = GetDlgItemInt(IDC_BITRATE);

	int nWidth, nHeight;
	if(m_pCapture)
		m_pCapture->getVideoSize(&nWidth, &nHeight);

	m_bReadyEncode = true;

}


void CVideoSourceDlg::OnBnClickedStopStream()
{
	// TODO: Add your control notification handler code here
	m_bReadyEncode = false;
}

void CVideoSourceDlg::UpdatePlayListFile()
{

}



