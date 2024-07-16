#pragma once


// BatchViewer dialog

#define WM_USER_BATCH_REFRESH			(WM_USER + 0x200)
#define WM_USER_BATCH_FINISH			(WM_USER + 0x201)
#define	WM_USER_BATCH_PROCESS			(WM_USER + 0x202)

#define BATCH_STATUS_READY				L"Ready"
#define BATCH_STATUS_PROCESSING			L"Processing"
#define BATCH_STATUS_COMPLETED			L"Completed"
#define BATCH_STATUS_FAILED				L"Failed"
#define BATCH_STATUS_STOPPED			L"Stopped"

class BatchViewer : public CDialogEx
{
	DECLARE_DYNAMIC(BatchViewer)

public:
	BatchViewer(CWnd* pParent = nullptr);   // standard constructor
	virtual ~BatchViewer();
	virtual BOOL OnInitDialog();
	//void ReadBatchToUI();
	void PostNcDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	LRESULT OnBatchRefresh(WPARAM wParam, LPARAM lParam);
	LRESULT OnBatchFinish(WPARAM wParam, LPARAM lParam);
	LRESULT OnBatchProcess(WPARAM wParam, LPARAM lParam);

	void ShowStatus(LPCWSTR lpText, ...);
	void EnableProcessWindow(BOOL p_bEnable);

	enum { IDD = IDD_BATCHVIEWER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_Batchs;

private:
	CBrush  m_bkBrush;
	CFont m_font;
	CWnd* m_pParent = NULL;

	//BatchManager m_clsBatchManager;

	// context menu
	CMenu m_clipsMenu;
private:

	BITMAP  m_Bitmap;		// Struct to hold info about the bitmap
	HBITMAP m_hBitmap;		// Handle of the bitmap

public:
	SG_ButtonFly m_BtnRun;
	SG_ButtonFly m_BtnStop;
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	void RemoveSelectedItem();
	afx_msg void OnBatchRemoveSelectedItem();
	afx_msg void OnBatchViewerResetBatchStatuses();
	afx_msg void OnBatchviewerClearBatch();
	afx_msg void OnBatchviewerOptimizelist();

	CComboBox m_cmbLang;
	afx_msg void OnBnClickedBtnApply();
	afx_msg void OnNMClickLstBatchs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedBatchrun();
	afx_msg void OnBnClickedBatchstop();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL PreTranslateMessage(MSG* pMsg);
};