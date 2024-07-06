
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

	int thr;
	int filt_s;
	SGPictureControl m_pictureResults,m_pictureOrig,m_pictureScan;
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
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	COLORREF m_borderColor;
	bool mouse_down = false;

	char *ConvertWideCharToMultiByte(const CString &strWideChar);
	bool ConvertPDF2IMG(CString &pdfFilePath);
	void SetTitle();

	void DrawImage(CWnd *pRenderWnd, const CString &strImageFilePath);
	void ShowResults(int Threshold);
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
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CStatic m_Status;
	afx_msg void OnDblClickSlider1(NMHDR* pNMHDR, LRESULT* pResult);
};
