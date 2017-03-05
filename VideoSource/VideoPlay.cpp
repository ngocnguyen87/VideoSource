// VideoPlay.cpp : implementation file
//

#include "stdafx.h"
#include "VideoSource.h"
#include "VideoPlay.h"
#include "VideoSourceDlg.h"
#include "..\Render\Render.h"

#include "afxdialogex.h"


// CVideoPlay dialog

IMPLEMENT_DYNAMIC(CVideoPlay, CDialog)

CVideoPlay::CVideoPlay(CWnd* pParent /*=NULL*/)
	: CDialog(CVideoPlay::IDD, pParent)
{
}

CVideoPlay::~CVideoPlay()
{
}

void CVideoPlay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVideoPlay, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_MOVING()
END_MESSAGE_MAP()


// CVideoPlay message handlers


BOOL CVideoPlay::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);

	pDC->FillSolidRect(&rect,RGB(0,0,0));

	return TRUE;
}


void CVideoPlay::setParent(void* pParent)
{
	m_pParent = (CVideoSourceDlg*)pParent;
}

void CVideoPlay::OnMoving(UINT fwSide, LPRECT pRect)
{
	CDialog::OnMoving(fwSide, pRect);

	RECT rect;

	GetWindowRect(&rect);

	m_pParent->UpdateOverlay(rect);

	// TODO: Add your message handler code here
}
