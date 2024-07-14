
// PrintshopComparisonToolSetupDlg.h : header file
//
#include "SG_ButtonFly.h"
#pragma once

#include "SGPictureControl.h"

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

	int m_CurrentThreshold;
	int m_CurrentColor;
	int filt_s;
	SGPictureControl 
		m_pictureResults1, m_pictureResults2,
		m_pictureOrig1, m_pictureOrig2,
		m_pictureScan1, m_pictureScan2;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnQuit();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	COLORREF m_borderColor;
	
	char *ConvertWideCharToMultiByte(const CString &strWideChar);
	bool ConvertPDF2IMG(CString &pdfFilePath, int& pages);
	void UpdatePagesStates();
	CString GetCurrentPagePath();
	void UpdateTitle(CString s);



	void SetTitle();

	void DrawImage(CWnd *pRenderWnd, const CString &strImageFilePath);
	void ShowResults(int Threshold, int color);

public:
	// Add member variable to store the last rect
	CRect m_LastRect;

// Implementation
protected:
	HICON m_hIcon;
	int curPage;
	int NoPages;
private:
	CBrush m_brBack, m_brBack2, m_brBack3;
	CBrush m_tx;

	CString 
		m_origPath1, m_origPath2,
		m_scanPath1, m_scanPath2,
		m_diffPath1, m_diffPath2;
public:
	CSliderCtrl threshold_slider;
	CSliderCtrl filter_size_slider;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CStatic m_Status;
	CButton m_BtnOrig;
	CButton m_BtnScan;
	CButton m_BtnProc;
	afx_msg void OnBnClickedButtonOrig();
	afx_msg void OnBnClickedButtonScan();
	CButton m_BtnOpenResult;
	CButton m_BtnSetTH;
	afx_msg void OnBnClickedButtonOpenresult();
	afx_msg void OnBnClickedButtonSetTH();
	afx_msg void OnBnClickedButtonProc();
	CButton m_BtnSideA;
	CButton m_BtnSideB;
	afx_msg void OnBnClickedButtonSideA();
	afx_msg void OnBnClickedButtonSideB();
};
