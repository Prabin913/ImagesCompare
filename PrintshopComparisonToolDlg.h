
// PrintshopComparisonToolSetupDlg.h : header file
//
#include "SG_ButtonFly.h"

#pragma once
#define BG_COLOR		RGB(215, 217, 239)
#define	TX_COLOR		RGB(25, 19, 31)
#define	TX_COLOR2		RGB(64, 6, 49)
#define	TX_COLOR3		RGB(0, 0, 0)
#define	TX_HOVER_COLOR	RGB(180, 151, 209)


using PasswordCheckCallback = std::function<bool(const std::wstring&)>;

// PrintshopComparisonToolDlg dialog
class PrintshopComparisonToolDlg : public CDialog
{
// Construction
public:
	PrintshopComparisonToolDlg(CWnd* pParent = NULL);	// standard constructor
	bool AutoMode;
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PRINTSHOPCOMPARISONTOOL_DIALOG};
#endif


protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnQuit();
	afx_msg void OnStnClickedPicOrig();
	afx_msg void OnStnClickedPicScan();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	char *ConvertWideCharToMultiByte(const CString &strWideChar);
	bool ConvertPDF2IMG(CString &pdfFilePath);
	void DrawImage(CWnd *pRenderWnd, const CString &strImageFilePath);
	void CompareImage();

public:
	// Add member variable to store the last rect
	CRect m_LastRect;

// Implementation
protected:
	HICON m_hIcon;

private:
	CBrush m_brBack, m_brBack2, m_brBack3;
	CBrush m_tx;

	CString m_origPath, m_scanPath, m_diffPath;
public:
	CSliderCtrl threshold_slider;
	CSliderCtrl filter_size_slider;
	CString thr_slider_echo;
	CString filt_slider_echo;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CStatic m_Status;
};
