
// PrintshopComparisonToolSetupDlg.h : header file
//
#include "SG_ButtonFly.h"
#pragma once

#include "SGPictureControl.h"

#include "BatchViewer.h"

#define DRAWIMAGE(ID,path,member) 	DrawImage(GetDlgItem(ID), path);if (PathFileExists(path)) member.LoadImage(path)
#define SHOWORIG1 DRAWIMAGE(IDC_PIC_ORIG1, m_origPath1,m_pictureOrig1)
#define SHOWORIG2 DRAWIMAGE(IDC_PIC_ORIG2, m_origPath2, m_pictureOrig2)
#define SHOWSCAN1 DRAWIMAGE(IDC_PIC_SCAN1, m_scanPath1, m_pictureScan1);\
	m_pictureScan1.SetContextMenuItems({\
		{ ID_CONTEXTMENU_OPENIMAGE, _T("Open image") },\
		{ ID_CONTEXTMENU_SHOWSCAN, _T("Show aligned") }\
	})

#define SHOWSCAN2 DRAWIMAGE(IDC_PIC_SCAN2, m_scanPath2, m_pictureScan2);\
	m_pictureScan2.SetContextMenuItems({\
		{ ID_CONTEXTMENU_OPENIMAGE, _T("Open image") },\
		{ ID_CONTEXTMENU_SHOWSCAN, _T("Show aligned") }\
	})
#define SHOWRESULT1 DRAWIMAGE(IDC_PIC_DIFF1, m_diffPath1, m_pictureResults1)
#define SHOWRESULT2 DRAWIMAGE(IDC_PIC_DIFF2, m_diffPath2, m_pictureResults2)

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
	afx_msg void OnEnChangeEditOrig();
	afx_msg void OnEnChangeEditScan();

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
	BatchViewer *m_myBatchViewer;
	// Batch mode
	bool m_batchMode;
	CString m_batchFile;
	std::thread m_batchThread;
	std::atomic<bool> m_stopBatchThread;

	// Batch processing functions
	void StartBatchProcessing(const CString& batchFilePath);
	void StopBatchProcessing();
	void BatchProcess(const CString& batchFilePath);
	void SetThreshold(int val);

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
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CStatic m_Status;
	SG_ButtonFly m_BtnOrig;
	SG_ButtonFly m_BtnScan;
	SG_ButtonFly m_BtnProc;
	SG_ButtonFly m_BtnBatch;
	afx_msg void OnBnClickedButtonOrig();
	afx_msg void OnBnClickedButtonScan();
	SG_ButtonFly m_BtnSetTH;
	afx_msg void OnBnClickedButtonOpenresult();
	afx_msg void OnBnClickedButtonSetTH();
	afx_msg void OnBnClickedButtonProc();
	afx_msg void OnBnClickedToggleBatch();
	void ShowHideBatchViewer();

	CButton m_BtnSideA;
	CButton m_BtnSideB;
	afx_msg void OnBnClickedButtonSideA();
	afx_msg void OnBnClickedButtonSideB();
	BatchViewer* m_pBatchV = NULL;

};
