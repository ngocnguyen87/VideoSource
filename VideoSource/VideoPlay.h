#pragma once

class CVideoSourceDlg;
// CVideoPlay dialog

class CVideoPlay : public CDialog
{
	DECLARE_DYNAMIC(CVideoPlay)

public:
	CVideoPlay(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoPlay();

// Dialog Data
	enum { IDD = IDD_DISPLAY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);

	void setParent(void* pUser);

	CVideoSourceDlg* m_pParent;
};
