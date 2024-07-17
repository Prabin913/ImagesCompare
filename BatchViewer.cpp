// BatchViewer.cpp : implementation file
//

#include "pch.h"
#include "PrintshopComparisonTool.h"
#include "PrintshopComparisonToolDlg.h"
#include "BatchViewer.h"
#include "afxdialogex.h"
#include "utils.h"


// BatchViewer dialog

// refresh timer
#define TMR_REFRESH				(0x300)
#define TRANSPARENTCOLOR RGB(100, 100, 100)

IMPLEMENT_DYNAMIC(BatchViewer, CDialogEx)

BEGIN_MESSAGE_MAP(BatchViewer, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_USER_BATCH_REFRESH, &BatchViewer::OnBatchRefresh)
	ON_MESSAGE(WM_USER_BATCH_FINISH, &BatchViewer::OnBatchFinish)
	ON_MESSAGE(WM_USER_BATCH_PROCESS, &BatchViewer::OnBatchProcess)
	ON_WM_CONTEXTMENU()
//	ON_COMMAND(ID_BATCH_REMOVESELECTEDITEM, &BatchViewer::OnBatchRemoveSelectedItem)
//	ON_COMMAND(ID_BATCHVIEWER_CLEARBATCH, &BatchViewer::OnBatchviewerClearBatch)
//	ON_COMMAND(ID_BATCHVIEWER_OPTIMIZELIST, &BatchViewer::OnBatchviewerOptimizelist)
//	ON_COMMAND(ID_BATCHVIEWER_RESETSTATUSES, &BatchViewer::OnBatchViewerResetBatchStatuses)
	ON_BN_CLICKED(IDC_BTN_APPLY, &BatchViewer::OnBnClickedBtnApply)
	ON_NOTIFY(NM_CLICK, IDC_LST_BATCHS, &BatchViewer::OnNMClickLstBatchs)
	ON_BN_CLICKED(IDC_BATCHRUN, &BatchViewer::OnBnClickedBatchrun)
	ON_BN_CLICKED(IDC_BATCHSTOP, &BatchViewer::OnBnClickedBatchstop)
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

BOOL BatchViewer::OnEraseBkgnd(CDC *pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);

	CRect clientRect;

	GetClientRect(&clientRect);
	pDC->FillSolidRect(clientRect, TRANSPARENTCOLOR);  // paint background in magenta

	return FALSE;
}
BatchViewer::BatchViewer(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BATCHVIEWER, pParent)
{
	m_pParent = pParent;
}

BatchViewer::~BatchViewer()
{
}

BOOL BatchViewer::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	// Create the font to be used
//	m_font.CreateFont(FONT_SIZE, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, 0,
//		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
//		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
//		DEFAULT_PITCH&FF_SWISS, _T(DEF_FONT));


	m_BtnRun.SetImages(IDB_BN_START, IDB_BN_START_H, IDB_BN_START_P, IDB_BN_START_D);
	m_BtnRun.SetParent(this);
	m_BtnRun.SetCaptionText(L"Run");

	m_BtnStop.SetImages(IDB_BN_STOP, IDB_BN_STOP_H, IDB_BN_STOP_P, IDB_BN_STOP_D);
	m_BtnStop.SetParent(this);
	m_BtnStop.SetCaptionText(L"Stop");

	// Background color to be used
//	m_bkBrush.CreateSolidBrush(DEF_COLOR);

	PrintshopComparisonToolDlg* pMainDlg = (PrintshopComparisonToolDlg*)AfxGetApp()->GetMainWnd();
	pMainDlg->m_myBatchViewer = this;

/*
	m_Batchs.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_Batchs.InsertColumn(0, L"ID", LVCFMT_LEFT, (int)(40 * g_fDPIRate));
	m_Batchs.InsertColumn(1, L"Path", LVCFMT_LEFT, (int)(300 * g_fDPIRate));
	m_Batchs.InsertColumn(2, L"Clip MOB ID", LVCFMT_LEFT, (int)(120 * g_fDPIRate));
	m_Batchs.InsertColumn(3, L"Track Mixing", LVCFMT_LEFT, (int)(70 * g_fDPIRate));
	m_Batchs.InsertColumn(4, L"Language", LVCFMT_LEFT, (int)(120 * g_fDPIRate));
	m_Batchs.InsertColumn(5, L"Entity Type", LVCFMT_LEFT, (int)(70 * g_fDPIRate));
	m_Batchs.InsertColumn(6, L"Status", LVCFMT_LEFT, (int)(100 * g_fDPIRate));

	ReadBatchToUI();
	*/
	SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) ^ WS_EX_LAYERED);
	SetLayeredWindowAttributes(TRANSPARENTCOLOR, 0, LWA_COLORKEY);

	// initialize batch manager
//	m_clsBatchManager.InitBatch();
//	m_clsBatchManager.SetMainWnd(m_pParent);

	// start refresh timer
	SetTimer(TMR_REFRESH, 3000, NULL); // 3s

	return TRUE;
}

/*
void BatchViewer::ReadBatchToUI()
{
	vector<SG_Batch> myBatch;

	// read data
	m_clsBatchManager.ReadAllItems(&myBatch);

	for (size_t i = 0; i < myBatch.size(); i++)
	{
		CString w_szInd, w_szClipIdx, w_szEntity, w_szStatus;
		w_szInd.Format(L"%d", i + 1);

		if (myBatch[i].clip_index == -2)
			w_szClipIdx = L"Group";
		else if (myBatch[i].clip_index < 0)
			w_szClipIdx = L"Merge";
		else
			w_szClipIdx.Format(L"%d", myBatch[i].clip_index);

		switch (myBatch[i].entity_type)
		{
		case EntityType::Folder:
			w_szEntity = L"Folder";
			break;
		case EntityType::Avbfile:
			w_szEntity = L"AVB File";
			break;
		case EntityType::Clip:
			w_szEntity = L"Clip";
			break;
		default:
			w_szEntity = L"Unknown";
			break;
		}

		switch (myBatch[i].status)
		{
		case BatchStatus::Ready:
			w_szStatus = BATCH_STATUS_READY;
			break;
		case BatchStatus::Processing:
			w_szStatus = BATCH_STATUS_PROCESSING;
			break;
		case BatchStatus::Mixing:
			w_szStatus = BATCH_STATUS_MIXING;
			break;
		case BatchStatus::Transcribing:
			w_szStatus = BATCH_STATUS_TRANSCRIBING;
			break;
		case BatchStatus::InsertMarker:
			w_szStatus = BATCH_STATUS_MARKERS_INSERTION;
			break;
		case BatchStatus::Finish:
			w_szStatus = BATCH_STATUS_COMPLETED;
			break;
		case BatchStatus::Failed:
			w_szStatus = BATCH_STATUS_FAILED;
			break;
		case BatchStatus::Stopped:
			w_szStatus = BATCH_STATUS_STOPPED;
			break;
		default:
			break;
		}
		m_Batchs.InsertItem(i, w_szInd);
		m_Batchs.SetItemText(i, 1, myBatch[i].file_path);
		m_Batchs.SetItemText(i, 2, myBatch[i].clip_mob_id);
		m_Batchs.SetItemText(i, 3, w_szClipIdx);
		m_Batchs.SetItemText(i, 4, myBatch[i].language);
		m_Batchs.SetItemText(i, 5, w_szEntity);
		m_Batchs.SetItemText(i, 6, w_szStatus);

		if (w_szStatus == BATCH_STATUS_PROCESSING)
			m_Batchs.SetSelectionMark(i);
	}
}
*/
HBRUSH BatchViewer::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd == this)
	{
		return m_bkBrush;
	}

	switch (pWnd->GetDlgCtrlID())
	{
		case IDC_STA_ORIGFILE:
		case IDC_STA_SCANFILE:
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(100,100,100));

			return m_bkBrush;
		default:
			break;
	}
	return hbr;
}

LRESULT BatchViewer::OnBatchRefresh(WPARAM wParam, LPARAM lParam)
{
	// clear list
	m_Batchs.DeleteAllItems();

	// read data and show
	//ReadBatchToUI();

	return S_OK;
}

LRESULT BatchViewer::OnBatchFinish(WPARAM wParam, LPARAM lParam)
{
	// enable controls
	EnableProcessWindow(TRUE);
	m_BtnRun.SetCheck(FALSE);

	return S_OK;
}

LRESULT BatchViewer::OnBatchProcess(WPARAM wParam, LPARAM lParam)
{
	// select current process
	m_Batchs.SetItemState(wParam, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_Batchs.SetFocus();

	return S_OK;
}

void BatchViewer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LST_BATCHS, m_Batchs);
	DDX_Control(pDX, IDC_BATCHRUN, m_BtnRun);
	DDX_Control(pDX, IDC_BATCHSTOP, m_BtnStop);
}

void BatchViewer::PostNcDestroy()
{
	CDialogEx::PostNcDestroy();
	delete this;
}

void BatchViewer::OnContextMenu(CWnd* pWnd, CPoint point)
{
	int CtrlID = pWnd->GetDlgCtrlID();

	if (CtrlID == IDC_LST_BATCHS)
	{
		CMenu* w_mnPopup = m_clipsMenu.GetSubMenu(1);
		int retVal = w_mnPopup->TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y, this);
	}
}

void BatchViewer::ShowStatus(LPCWSTR lpText, ...)
{
	CString sMsg;
	va_list ptr;

	va_start(ptr, lpText);
	sMsg.FormatV(lpText, ptr);
	va_end(ptr);

//	CMatchPMR_AVBMainDlg* pMainDlg = (CMatchPMR_AVBMainDlg*)AfxGetApp()->GetMainWnd();
//	if (pMainDlg)
//		pMainDlg->ShowStatus(sMsg.GetBuffer(), NULL);
	sMsg.ReleaseBuffer();
}

void BatchViewer::EnableProcessWindow(BOOL p_bEnable)
{

//	CMatchPMR_AVBMainDlg *pMain = (CMatchPMR_AVBMainDlg*)AfxGetApp()->GetMainWnd();
//	BatchViewer* pBatchViewerDlg = pMain->m_pBatchV;

	// enable/disable batch viewer
//	GetDlgItem(IDC_LST_BATCHS)->EnableWindow(p_bEnable); // don't disable that
	GetDlgItem(IDC_CMB_LANG)->EnableWindow(p_bEnable);
	GetDlgItem(IDC_BTN_APPLY)->EnableWindow(p_bEnable);
	GetDlgItem(IDC_BATCHRUN)->EnableWindow(p_bEnable);
	GetDlgItem(IDC_BATCHSTOP)->EnableWindow(!p_bEnable);

	//pBatchViewerDlg->GetDlgItem(IDC_STA_ORIGFILE)->SetWindowText(L"");

	//pBatchViewerDlg->GetDlgItem(IDC_STA_SCANFILE)->SetWindowText(L"");

	// enable/disable main window
//	((CMatchPMR_AVBMainDlg*)m_pParentWnd)->EnableControls(p_bEnable);
//	((CMatchPMR_AVBMainDlg*)m_pParentWnd)->m_pGeneralDlg->EnableControls(p_bEnable);
//	((CMatchPMR_AVBMainDlg*)m_pParentWnd)->m_pTransDlg->EnableControls(p_bEnable);
//	((CMatchPMR_AVBMainDlg*)m_pParentWnd)->m_pMarkerDlg->EnableControls(p_bEnable);
}

void BatchViewer::RemoveSelectedItem()
{
	// update data
	UpdateData(TRUE);

	// get current selection
	int w_nCurIdx = m_Batchs.GetSelectionMark();
	if (w_nCurIdx < 0)
	{
		return;
	}
	/*
	wstring w_szAvbPath;
	wstring w_szMobID;
	SG_Batch w_stItem = { 0 };
	w_szAvbPath = m_Batchs.GetItemText(w_nCurIdx, 1);
	w_szMobID = m_Batchs.GetItemText(w_nCurIdx, 2);
	wcscpy(w_stItem.file_path, w_szAvbPath.c_str());
	wcscpy(w_stItem.clip_mob_id, w_szMobID.c_str());
	m_clsBatchManager.RemoveItem(w_stItem);
	*/
	ShowStatus(L"Current task was removed");

	// update batch viewer
	PostMessage(WM_USER_BATCH_REFRESH, 0, 0);
}

void BatchViewer::OnBatchRemoveSelectedItem()
{
	RemoveSelectedItem();
}

void BatchViewer::OnBatchViewerResetBatchStatuses()
{
	//m_clsBatchManager.ResetBatchStatuses();
}

void BatchViewer::OnBatchviewerClearBatch()
{
	//m_clsBatchManager.ClearAllItems();
	ShowStatus(L"Batch was cleared");

	// update batch viewer
	PostMessage(WM_USER_BATCH_REFRESH, 0, 0);
}

void BatchViewer::OnBatchviewerOptimizelist()
{
	//m_clsBatchManager.OptimizeBatch();
	ShowStatus(L"Batch was optimized");

	// update batch viewer
	PostMessage(WM_USER_BATCH_REFRESH, 0, 0);
}

void BatchViewer::OnBnClickedBtnApply()
{
	UpdateData(TRUE);

	// get current selection
	int w_nCurIdx = m_Batchs.GetSelectionMark();
	if (w_nCurIdx < 0)
		return;

	// get language
	CString w_szLang;
	m_cmbLang.GetWindowText(w_szLang);


	// update batch viewer
	PostMessage(WM_USER_BATCH_REFRESH, 0, 0);
}


void BatchViewer::OnNMClickLstBatchs(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	UpdateData(TRUE);

	// get current selection
	int w_nCurIdx = m_Batchs.GetSelectionMark();
	if (w_nCurIdx < 0)
	{
		*pResult = 0;
		return;
	}

	CString w_szLang;

	*pResult = 0;
}

void BatchViewer::OnBnClickedBatchrun()
{
}


void BatchViewer::OnBnClickedBatchstop()
{
	//m_clsBatchManager.StopBatchThread();
	//m_BtnRun.SetSelected(FALSE);
}


void BatchViewer::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent != TMR_REFRESH)
		return CDialogEx::OnTimer(nIDEvent);

	if (!IsWindowVisible())
		return CDialogEx::OnTimer(nIDEvent);

	//// get current index
	//int w_nCurIdx = m_Batchs.GetSelectionMark();

	//// clear list
	//m_Batchs.DeleteAllItems();

	//// read data and show
	//ReadBatchToUI();

	//// select last item
	//if (w_nCurIdx >= 0)
	//	m_Batchs.SetItemState(w_nCurIdx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	CDialogEx::OnTimer(nIDEvent);
}


BOOL BatchViewer::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		wchar_t c = pMsg->wParam;
		if (c == VK_DELETE)
		{
			RemoveSelectedItem();
			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

