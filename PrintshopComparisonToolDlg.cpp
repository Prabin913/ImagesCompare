
// PrintshopComparisonToolDlg.cpp : implementation file
//

#include "pch.h"
#include <stdlib.h>
#include "framework.h"
#include "PrintshopComparisonTool.h"
#include "SGPictureControl.h"
#include "PrintshopComparisonToolDlg.h"
#include "afxdialogex.h"
#include "mupdf/pdf.hpp"
#include "printcheck/PrintChecker.hpp"
#include "utils.h"
#include "SGInputDlg.h"
#include "constants.h"

#include <iostream>
#include <sstream>
#include "VzImgCmp.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
double g_fDPIRate = 1.0;

// PrintshopComparisonToolDlg dialog
#define DATETIME_BUFFER_SIZE 80
#define BUFFER_SIZE 4096

static CString annotate_path("annotation.png");

printcheck::PrintChecker pc;
bool need_to_update = false;
bool images_loaded=false;
PrintshopComparisonToolDlg::PrintshopComparisonToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_PRINTSHOPCOMPARISONTOOL_DIALOG, pParent),
	m_batchMode(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	NoPages=1;
	curPage=1;
}

void PrintshopComparisonToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_THRESHOLD, threshold_slider);
	DDX_Control(pDX, IDC_STATUS, m_Status);
	DDX_Control(pDX, IDC_BUTTON_ORIG, m_BtnOrig);
	DDX_Control(pDX, IDC_BUTTON_SCAN, m_BtnScan);
	DDX_Control(pDX, IDC_BUTTON_PROC, m_BtnProc);
	DDX_Control(pDX, IDC_BUTTON_SETTINGS, m_BtnSetTH);
	DDX_Control(pDX, IDC_BUTTON_SIDE_A, m_BtnSideA);
	DDX_Control(pDX, IDC_BUTTON_SIDE_B, m_BtnSideB);

}

BEGIN_MESSAGE_MAP(PrintshopComparisonToolDlg, CDialog)
	ON_WM_HOTKEY()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_CONTEXTMENU()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_ORIG, &PrintshopComparisonToolDlg::OnBnClickedButtonOrig)
	ON_BN_CLICKED(IDC_BUTTON_SCAN, &PrintshopComparisonToolDlg::OnBnClickedButtonScan)
	ON_BN_CLICKED(IDC_BUTTON_SETTINGS,&PrintshopComparisonToolDlg::OnBnClickedButtonSetTH)
	ON_BN_CLICKED(IDC_BUTTON_PROC, &PrintshopComparisonToolDlg::OnBnClickedButtonProc)
	ON_BN_CLICKED(IDC_BUTTON_SIDE_A, &PrintshopComparisonToolDlg::OnBnClickedButtonSideA)
	ON_BN_CLICKED(IDC_BUTTON_SIDE_B, &PrintshopComparisonToolDlg::OnBnClickedButtonSideB)
END_MESSAGE_MAP()

// PrintshopComparisonToolDlg class implementation
void PrintshopComparisonToolDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	switch (nHotKeyId)
	{
		case 1:
			imshow("error", pc.error());
			break;
		case 2:
			OnBnClickedButtonSetTH();
			break;
		case 3:
			imshow("error-map", pc.errormap());
			break;


	}

	CDialog::OnHotKey(nHotKeyId, nKey1, nKey2);
}
void PrintshopComparisonToolDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

}


void PrintshopComparisonToolDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	return;
	if (pWnd->GetSafeHwnd())
	{
		// Check if pWnd is an SGPictureControl
//		if (SGPictureControl* pPictureControl = dynamic_cast<SGPictureControl*>(pWnd))
		{
			CRect rect;
			pWnd->GetWindowRect(&rect);
			ScreenToClient(&rect);

			// Check if the point is within the control's client area
			if (rect.PtInRect(point))
			{
				ClientToScreen(&point); // Convert point to screen coordinates
				((SGPictureControl*)pWnd)->SendMessage(WM_CONTEXTMENU, (WPARAM)pWnd->GetSafeHwnd(), MAKELPARAM(point.x, point.y));

				return; // Exit after calling the context menu handling
			}
		}
	}

	// Call base class implementation if the point is outside the control's client area
	CDialog::OnContextMenu(pWnd, point);
}
HBRUSH PrintshopComparisonToolDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())
	{
		case IDC_STATIC_TH:
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetBkColor(BG_COLOR);
			return m_tx; // Return a brush with the same color
			break;
		case IDC_SLIDER_THRESHOLD:

		case IDC_STATIC_THR:
		case IDC_STATIC_FILT_SIZE:
		case IDC_STATIC_FL:
			pDC->SetBkMode(OPAQUE);
			pDC->SetBkColor(BG_COLOR);
			return m_tx; // Return a brush with the same color
			break;
		default:
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetBkColor(RGB(0, 0, 0));
			pDC->SetTextColor(RGB(0, 0, 0));
			return m_brBack;
	}

	return m_tx;
}
BOOL PrintshopComparisonToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_BtnOrig.SetImages(IDB_BN_ORIG, IDB_BN_ORIG_H, IDB_BN_ORIG_P, IDB_BN_ORIG_D);
	m_BtnOrig.SetParent(this);
	m_BtnOrig.SetCaptionText(L"Original");

	m_BtnScan.SetImages(IDB_BN_SCAN, IDB_BN_SCAN_H, IDB_BN_SCAN_P, IDB_BN_SCAN_D);
	m_BtnScan.SetParent(this);
	m_BtnScan.SetCaptionText(L"Scan");

	m_BtnProc.SetImages(IDB_BN_PROC, IDB_BN_PROC_H, IDB_BN_PROC_P, IDB_BN_PROC_D);
	m_BtnProc.SetParent(this);
	m_BtnProc.SetCaptionText(L"Process");

	m_BtnSetTH.SetImages(IDB_BN_SETTING, IDB_BN_SETTING_H, IDB_BN_SETTING_P, IDB_BN_SETTING_D);
	m_BtnSetTH.SetParent(this);
	m_BtnSetTH.SetCaptionText(L"Settings");

	// Subclass the picture control
	m_pictureResults1.SubclassDlgItem(IDC_PIC_DIFF1, this);
	m_pictureResults1.SetBorderThickness(10);
	m_pictureResults2.SubclassDlgItem(IDC_PIC_DIFF2, this);
	m_pictureResults2.SetBorderThickness(10);

	m_pictureOrig1.SubclassDlgItem(IDC_PIC_ORIG1,this);
	m_pictureOrig1.SetBorderThickness(10);
	m_pictureOrig2.SubclassDlgItem(IDC_PIC_ORIG2, this);
	m_pictureOrig2.SetBorderThickness(10);


	m_pictureScan1.SubclassDlgItem(IDC_PIC_SCAN1,this);
	m_pictureScan2.SubclassDlgItem(IDC_PIC_SCAN2, this);

	if (!RegisterHotKey(m_hWnd, 1, MOD_CONTROL | MOD_SHIFT, 'E'))
	{
		AfxMessageBox(L"Failed to register hotkey!");
	}
	if (!RegisterHotKey(m_hWnd, 2, MOD_CONTROL | MOD_SHIFT, 'B'))
	{
		AfxMessageBox(L"Failed to register hotkey!");
	}
	if (!RegisterHotKey(m_hWnd, 3, MOD_CONTROL | MOD_SHIFT, 'M'))
	{
		AfxMessageBox(L"Failed to register hotkey!");
	}
	threshold_slider.SetRange(0, 256, TRUE);
	threshold_slider.SetPos(70);
	m_BtnProc.EnableWindow(FALSE);
	m_BtnScan.EnableWindow(FALSE);


	m_CurrentThreshold = 70;
	need_to_update = true;

	UpdateData(FALSE);

	HDC desktopDC = ::GetDC(NULL);
	int horDPI = GetDeviceCaps(desktopDC, LOGPIXELSX);
	int verDPI = GetDeviceCaps(desktopDC, LOGPIXELSY);

	if (horDPI == 120)
		g_fDPIRate = 1.25;
	else if (horDPI == 144)
		g_fDPIRate = 1.5;
	else if (horDPI == 192)
		g_fDPIRate = 2.0;
	else
		g_fDPIRate = 1.0;

	// initialize brush
	m_brBack.CreateSolidBrush(BG_COLOR);
	m_tx.CreateSolidBrush(RGB(255, 255, 255));

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetTitle();
	SetTimer(1000, 500, NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}


void PrintshopComparisonToolDlg::SetTitle()
{
	CString title;
	CString PDF, PNG;
	// Extract file name from m_origPath1
	LPCTSTR origPathFileName = PathFindFileName(m_origPath1);
	if (origPathFileName != nullptr)
	{
		PDF = origPathFileName;
	}

	// Extract file name from m_scanPath1
	LPCTSTR scanPathFileName = PathFindFileName(m_scanPath1);
	if (scanPathFileName != nullptr)
	{
		PNG = scanPathFileName;
	}
	if (PDF != L"" && PNG != L"")
	{
		m_BtnProc.EnableWindow(TRUE);
		m_BtnScan.EnableWindow(TRUE);
		UpdateData(FALSE);


	}
	else
	if (PDF != L"")
	{
		m_BtnProc.EnableWindow(FALSE);
		m_BtnScan.EnableWindow(TRUE);
		UpdateData(FALSE);

		title.Format(L"Printshop Master PDF selected :'%s' PNG not selected yet", PDF);
		m_pictureOrig1.SetBorderColor(RGB(255, 255, 255));
		m_pictureOrig1.SetBorderThickness(7);
		m_pictureOrig2.SetBorderColor(RGB(255, 255, 255));
		m_pictureOrig2.SetBorderThickness(7);
		m_pictureScan1.SetBorderColor(RGB(255, 0, 255));
		m_pictureScan1.SetBorderThickness(10);
		m_pictureScan2.SetBorderColor(RGB(255, 0, 255));
		m_pictureScan2.SetBorderThickness(10);
		m_pictureResults1.SetBorderColor(RGB(255, 255, 255));
		m_pictureResults1.SetBorderThickness(7);
		m_pictureResults2.SetBorderColor(RGB(255, 255, 255));
		m_pictureResults2.SetBorderThickness(7);

	}
	else
	{

		m_BtnProc.EnableWindow(FALSE);
		m_BtnScan.EnableWindow(FALSE);
		UpdateData(FALSE);
		title.Format(L"Printshop Master PDF selected :'%s' PNG selected:'%s'", PDF, PNG);

		m_pictureOrig1.SetBorderColor(RGB(255,0,255));
		m_pictureOrig1.SetBorderThickness(10);
		m_pictureOrig2.SetBorderColor(RGB(255, 0, 255));
		m_pictureOrig2.SetBorderThickness(10);

		m_pictureScan1.SetBorderColor(RGB(255, 255, 255));
		m_pictureScan1.SetBorderThickness(7);
		m_pictureScan2.SetBorderColor(RGB(255, 255, 255));
		m_pictureScan2.SetBorderThickness(7);

		m_pictureResults1.SetBorderColor(RGB(255, 255, 255));
		m_pictureResults1.SetBorderThickness(7);
		m_pictureResults2.SetBorderColor(RGB(255, 255, 255));
		m_pictureResults2.SetBorderThickness(7);

		title.Format(L"Printshop Master (new) - ready to start");
	}
	UpdateTitle(title);
}
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

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

void PrintshopComparisonToolDlg::OnPaint()
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
		CDialog::OnPaint();

		SHOWORIG1;
		SHOWORIG2;
		SHOWSCAN1;
		SHOWSCAN2;
		SHOWRESULT1;
		SHOWRESULT2;
	}		
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR PrintshopComparisonToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void PrintshopComparisonToolDlg::UpdatePagesStates()
{
	if (NoPages == 1)
	{
		m_BtnSideB.EnableWindow(FALSE);
		m_BtnSideA.EnableWindow(FALSE);

	}
	else
	{
		m_BtnSideB.EnableWindow(TRUE);
		m_BtnSideA.EnableWindow(TRUE);

	}

	if (NoPages == 2 && curPage == 2)
	{
		m_BtnSideB.SetState(TRUE);
		m_BtnSideA.SetState(FALSE);
		m_pictureResults1.ShowWindow(FALSE);
		m_pictureResults2.ShowWindow(TRUE);
		m_pictureOrig1.ShowWindow(FALSE);
		m_pictureOrig2.ShowWindow(TRUE);
		m_pictureScan1.ShowWindow(FALSE);
		m_pictureScan2.ShowWindow(TRUE);
	}
	else
	{
		m_BtnSideA.SetState(TRUE);
		m_BtnSideB.SetState(FALSE);
		m_pictureResults2.ShowWindow(FALSE);
		m_pictureResults1.ShowWindow(TRUE);
		m_pictureOrig2.ShowWindow(FALSE);
		m_pictureOrig1.ShowWindow(TRUE);
		m_pictureScan2.ShowWindow(FALSE);
		m_pictureScan1.ShowWindow(TRUE);

	}

	UpdateData(FALSE);
}
CString PrintshopComparisonToolDlg::GetCurrentPagePath()
{
	if (NoPages == 2)
	{
		if (curPage == 1)
		{
			return m_origPath1;
		}
		else
		{
			return m_origPath2;

		}
	}
	else
	{
		return m_origPath1 ;

	}

}
void PrintshopComparisonToolDlg::UpdateTitle(CString s)
{
	SG_Version ver;
	WCHAR szExeFileName[MAX_PATH];
	GetModuleFileName(NULL, szExeFileName, MAX_PATH);
	SG_GetVersion(szExeFileName, &ver);
	CString CreationDate = GetCreationDateTime();
	CString strver;
	strver.Format(L"Printshop Master version %d.%d.%d.%d Created on %s - %s",
		ver.Major,
		ver.Minor,
		ver.Revision,
		ver.SubRevision,
		CreationDate,s);
	SetWindowText(strver);

}
void PrintshopComparisonToolDlg::OnTimer(UINT_PTR nIDEvent)
{

	CString thr_slider_echo;

	static bool do_once{true};
	if(do_once)
	{ 
		do_once = false;

		UpdateTitle(L"");

		if (m_batchFile != L"")
		{
			NotifyVersionInfo(L"Batch mode", L"Executing " + m_batchFile);

			StartBatchProcessing(m_batchFile);

		}
		else
		{
			NotifyVersionInfo(L"Ready to start", L"Please select an original PDF file");

		}

	}
	if(m_batchMode) return;
	thr_slider_echo.Format(_T("%d"), m_CurrentThreshold);
	GetDlgItem(IDC_STATIC_THR)->SetWindowTextW(thr_slider_echo);

	if (need_to_update && images_loaded)
	{
		CString sCurPage = GetCurrentPagePath();
		UpdatePagesStates();
		CWaitCursor w;
		double diff;
		CString temp;
		temp.Format(L"Threshold set to %d", m_CurrentThreshold);
		NotifyVersionInfo(temp, L"Results will show....\nPress CTLR+SHIFT+E to see error\nPress CTRL + SHIFT + M to see error map\nPress CTRL + SHIFT + B to set Threshold");
		std::string orig_path = ConvertWideCharToMultiByte(sCurPage.GetString());
		std::string scan_path = ConvertWideCharToMultiByte((curPage==2)?m_scanPath2.GetString() : m_scanPath1.GetString());
		pc.process(orig_path, scan_path, &diff);

		ShowResults(m_CurrentThreshold, m_CurrentColor);

		need_to_update = false;
	}
	CDialog::OnTimer(nIDEvent);
}




void PrintshopComparisonToolDlg::OnClose()
{
	// Perform cleanup tasks before closing the dialog
	StopBatchProcessing(); // Ensure batch processing is stopped or completed
	// Unregister hotkeys
	UnregisterHotKey(m_hWnd, 1);
	UnregisterHotKey(m_hWnd, 2);
	UnregisterHotKey(m_hWnd, 3);
	CDialog::OnClose();
}



void PrintshopComparisonToolDlg::OnQuit()
{
	_exit(0);
}


CString SelectFileFromDialog(int type)
{
	OPENFILENAME  ofn;
	wchar_t* FilterSpec;
	if (type == 1)
		FilterSpec = { (wchar_t*)L"PDF Files(*.pdf)\0*.pdf\0PNG Files(*.png)\0*.png\0Tif Files(*.tif)\0*.tif\0All Files(*.*)\0*.*\0" };
	else
		FilterSpec = { (wchar_t*)L"PNG Files(*.png)\0*.png\0JPG Files(*.jpg)\0*.jpg\0All Files(*.*)\0*.*\0" };
	wchar_t* Title{ (wchar_t*)L"Open...." };
	wchar_t szFileName[MAX_PATH];
	wchar_t szFileTitle[MAX_PATH];
	int             Result;

	*szFileName = 0;
	*szFileTitle = 0;

	/* fill in non-variant fields of OPENFILENAME struct. */
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetFocus();
	ofn.lpstrFilter = FilterSpec;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = L"."; // Initial directory.
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrTitle = Title;
	ofn.lpstrDefExt = L".xlsx";

	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if (!GetOpenFileName((LPOPENFILENAME)&ofn))
	{
		return L""; // Failed or cancelled
	}
	else
	{
		return(szFileName);
	}
}

bool PrintshopComparisonToolDlg::ConvertPDF2IMG(CString &pdfFilePath, int &pages) 
{

	char *szSrcFilePath = ConvertWideCharToMultiByte(pdfFilePath);
	pdf *_pdf = new pdf(szSrcFilePath);
	if (!_pdf) return false;

	int count = 1;
	const char *password;
	if (_pdf->needs_password()) 
	{
		CString strErr;
		strErr.Format(_T("%s was protected by password"), pdfFilePath);
		AfxMessageBox(strErr);
		return false;
	}

	int zoom = 300;
	if (_pdf->good() && _pdf->size() != 0) 
	{
		pages = _pdf->size();
	}

	if (pages == 0) return false;

	int from, to;
	from = to = 1;
	if (!_pdf->render(szSrcFilePath, from, to, zoom))
	{
		AfxMessageBox(_T("Failed to render document."));
		return false;
	}
	m_origPath1 = pdfFilePath + _T(".1.png");

	if (pages == 2)
	{
		from = to = 2;
		if (!_pdf->render(szSrcFilePath, from, to, zoom))
		{
			AfxMessageBox(_T("Failed to render page 2 of document."));
			return false;
		}
		m_origPath2 = pdfFilePath + _T(".2.png");
		curPage=1;
		NoPages=2;
		GetDlgItem(IDC_BUTTON_SIDE_A)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_SIDE_B)->EnableWindow(TRUE);

	}
	else
	{
		curPage = 1;
		NoPages = 1;
		GetDlgItem(IDC_BUTTON_SIDE_A)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_SIDE_B)->EnableWindow(FALSE);

	}
	free(szSrcFilePath);

	return true;
}

void PrintshopComparisonToolDlg::DrawImage(CWnd* pRenderWnd, const CString& strImageFilePath)
{
	CWaitCursor w;
	if (strImageFilePath.IsEmpty()) return;

	CRect rc;
	pRenderWnd->GetWindowRect(&rc);
	pRenderWnd->ScreenToClient(&rc); // Convert to client coordinates

	CDC* pDC = pRenderWnd->GetDC();
	if (!pDC) return;

	CImage pngImage;
	HRESULT hr = pngImage.Load(strImageFilePath);
	if (FAILED(hr)) return;

	// Clear the background
	rc.left=11;
	rc.top=11;
	rc.right=rc.right-11;
	rc.bottom=rc.bottom-11;
	pDC->FillSolidRect(&rc, RGB(255, 255, 255));

	// Draw the image
	pngImage.StretchBlt(pDC->GetSafeHdc(), 11, 11, rc.Width()-22, rc.Height()-22, SRCCOPY);

	pRenderWnd->ReleaseDC(pDC);
}


char *PrintshopComparisonToolDlg::ConvertWideCharToMultiByte(const CString &strWideChar) {
	int strLen = WideCharToMultiByte(CP_UTF8, 0, strWideChar, -1, NULL, 0, NULL, NULL);
	char *szString = (char *)malloc(strLen + 1);
	memset(szString, 0, strLen + 1);
	WideCharToMultiByte(CP_UTF8, 0, strWideChar, -1, szString, strLen, NULL, NULL);

	return szString;
}

cv::Mat get_image(const std::string & filename)
{
	CWaitCursor w;
	WriteLogFile(L"reading file: %S",filename.c_str());
	
	if (filename.empty())
	{
		WriteLogFile(L"failed to empty file name %S", filename.c_str());
	}

	cv::Mat mat = cv::imread(filename, cv::IMREAD_COLOR);

	if (mat.channels() != 3 ||
		mat.cols < 10 ||
		mat.rows < 10)
	{
		WriteLogFile(L"failed to read image %S",filename.c_str());
	}
	WriteLogFile(L"%d x %d",mat.cols,mat.rows);

	return mat;
}

void PrintshopComparisonToolDlg::ShowResults(int Threshold, int color)
{
	static bool ongoing{ false };
	if (ongoing) return;
	if (!images_loaded) return;

	ongoing = true;
	CWaitCursor w;

	double diff = 0.0;
	cv::Mat annotated;

	try
	{
		annotated = pc.applyLimit(Threshold, color, &diff);

		CString stdDiff;
		stdDiff.Format(L"Difference between images is  %.1f%%", diff);

		if (diff == 0.0)
		{
			if (NoPages == 2 && curPage == 2)
			{
				m_pictureResults2.SetBorderColor(RGB(0, 255, 0));
				m_pictureResults2.SetBorderThickness(10);
				NotifyVersionInfo(L"Identical images!", stdDiff);
			}
			else
			{
				m_pictureResults1.SetBorderColor(RGB(0, 255, 0));
				m_pictureResults1.SetBorderThickness(10);
				NotifyVersionInfo(L"Identical images!", stdDiff);
			}
		}
		else
		{
			if (NoPages == 2 && curPage == 2)
			{
				m_pictureResults2.SetBorderColor(RGB(255, 0, 0));
				m_pictureResults2.SetBorderThickness(10);
				NotifyVersionInfo(L"Unidentical images", stdDiff);
			}
			else
			{
				m_pictureResults1.SetBorderColor(RGB(255, 0, 0));
				m_pictureResults1.SetBorderThickness(10);
				NotifyVersionInfo(L"Unidentical images", stdDiff);
			}
		}

		cv::imwrite(ConvertWideCharToMultiByte(annotate_path), annotated);

		if (NoPages == 2 && curPage == 2)
		{
			m_diffPath2 = annotate_path;
			SHOWRESULT2;
		}
		else
		{
			m_diffPath1 = annotate_path;
			SHOWRESULT1;
		}

		UpdateWindow();
	}
	catch (const cv::Exception& ex)
	{
		CString errMsg;
		errMsg.Format(L"OpenCV exception in ShowResults: %S", ex.what());
		AfxMessageBox(errMsg);
		WriteLogFile(errMsg);
	}
	catch (const std::exception& ex)
	{
		CString errMsg;
		errMsg.Format(L"Standard exception in ShowResults: %S", ex.what());
		AfxMessageBox(errMsg);
		WriteLogFile(errMsg);
	}
	catch (...)
	{
		AfxMessageBox(L"Unknown exception occurred in ShowResults.");
		WriteLogFile(L"Unknown exception occurred in ShowResults.");
	}

	ongoing = false;
}



void PrintshopComparisonToolDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{

	if (pScrollBar == (CScrollBar*)&threshold_slider)
	{
		static int last_th = -1;
		static int last_fs = -1;

		// update threshold and rerun the difference
		int current_thr = threshold_slider.GetPos();
		if (current_thr != last_th)
		{
			m_CurrentThreshold = current_thr;
			last_th = m_CurrentThreshold;
			ShowResults(m_CurrentThreshold, m_CurrentColor);
		}

		
	}
	else
	{
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	}
}






void PrintshopComparisonToolDlg::OnBnClickedButtonOrig()
{
	int pages;

	WriteLogFile(L"User clicked PDF area");
	CString pdfPath;
	if (m_batchMode)
	{
		pdfPath = m_origPath1;
	}
	else
	{
		DeleteFile(m_origPath1);
		m_origPath1 = L"";
		DeleteFile(m_origPath2);
		m_origPath2 = L"";
		pdfPath = SelectFileFromDialog(1);
	}
	if (pdfPath.IsEmpty()) return;
	WriteLogFile(L"Selected pdf file: '%s'", pdfPath.GetString());

	// Check if the selected file is already a .png file
	if (pdfPath.Right(4).CompareNoCase(L".png") == 0)
	{
		// If the file is already a .png, use it directly
		m_origPath1 = pdfPath;
		m_origPath2 = pdfPath; // Assuming the second path is the same for simplicity
		pages = 1; // Assume 1 page for .png file

		CString strPages;
		strPages.Format(L"File has %d page(s)", pages);
		SHOWORIG1;
		SHOWORIG2;

		NotifyVersionInfo(L"Original PNG file loaded. " + strPages, L"Now please select a scanned image");
	}
	else
	{
		// If the file is not a .png, proceed with conversion
		if (ConvertPDF2IMG(pdfPath, pages))
		{
			CString strPages;
			strPages.Format(L"File has %d pages", pages);
			SHOWORIG1;
			SHOWORIG2;

			NotifyVersionInfo(L"Original file loaded and converted. " + strPages, L"Now please select a scanned image");
		}
	}
	SetTitle();

}


void PrintshopComparisonToolDlg::OnBnClickedButtonScan()
{
	WriteLogFile(L"User clicked PNG area");

	if (NoPages == 2 && curPage == 2)
	{
		if (m_batchMode)
		{

		}
		else
		{
			m_scanPath2 = SelectFileFromDialog(0);
		}
		if (!m_scanPath2.IsEmpty())
		{
			if (m_scanPath2.Right(3).MakeUpper() == L"PDF")
			{
				MessageBox(L"Scanned image can't be a PDF");
				return;
			}
			SHOWSCAN2;

			SetTitle();
		}

	}
	else
	{
		if (m_batchMode)
		{

		}
		else
		{
			m_scanPath1 = SelectFileFromDialog(0);
		}
		if (!m_scanPath1.IsEmpty())
		{
			if (m_scanPath1.Right(3).MakeUpper() == L"PDF")
			{
				MessageBox(L"Scanned image can't be a PDF");
				return;
			}
			SHOWSCAN1;

			SetTitle();
		}

	}
	return;
}


void PrintshopComparisonToolDlg::OnBnClickedButtonOpenresult()
{
	if(NoPages ==2 && curPage ==2)
		ShellExecute(NULL, L"OPEN", m_diffPath2.GetString(), NULL, L"", TRUE);
	else
		ShellExecute(NULL, L"OPEN", m_diffPath1.GetString(), NULL, L"", TRUE);
}
void PrintshopComparisonToolDlg::OnBnClickedButtonSetTH()
{
	CInputNumberDialog dlg;
	dlg.SetUserInput(m_CurrentThreshold = threshold_slider.GetPos());
	dlg.SetColor(m_CurrentColor);
	dlg.DoModal();
	int userInput = dlg.GetUserInput();
	m_CurrentColor = dlg.GetSelectedColor();
	if (userInput >= 1 && userInput <= 256)
	{
		// Update the slider position
		threshold_slider.SetPos(userInput);
		UpdateData(FALSE);
		m_CurrentThreshold = userInput;
		ShowResults(m_CurrentThreshold, m_CurrentColor);
		return;
	}

}


void PrintshopComparisonToolDlg::OnBnClickedButtonProc()
{
	if (NoPages == 2)
	{
		if (curPage == 1)
		{
			if(m_origPath1 == L"" || m_scanPath1 == L"") return;
			WriteLogFile(L"Starting to compare %s and %s", m_origPath1.GetString(), m_scanPath1.GetString());

		}
		else
		{
			if (m_origPath2 == L"" || m_scanPath2 == L"") return;

			WriteLogFile(L"Starting to compare %s and %s", m_origPath2.GetString(), m_scanPath2.GetString());
		}
	}
	else
	{ 
		if (m_origPath1 == L"" || m_scanPath1 == L"") return;
		WriteLogFile(L"Starting to compare %s and %s", m_origPath1.GetString(), m_scanPath1.GetString());
	}

	images_loaded = true;
	m_pictureOrig1.SetBorderColor(RGB(255, 255, 255));
	m_pictureOrig1.SetBorderThickness(7);
	m_pictureOrig2.SetBorderColor(RGB(255, 255, 255));
	m_pictureOrig2.SetBorderThickness(7);

	m_pictureScan1.SetBorderColor(RGB(255, 255, 255));
	m_pictureScan1.SetBorderThickness(7);
	m_pictureScan2.SetBorderColor(RGB(255, 255, 255));
	m_pictureScan2.SetBorderThickness(7);

	m_pictureResults1.SetBorderColor(RGB(255, 255, 255));
	m_pictureResults1.SetBorderThickness(10);
	m_pictureResults2.SetBorderColor(RGB(255, 255, 255));
	m_pictureResults2.SetBorderThickness(10);

	if (m_batchMode)
	{
		CString sCurPage = GetCurrentPagePath();
		UpdatePagesStates();
		double diff;
		CString temp;
		temp.Format(L"Threshold set to %d", m_CurrentThreshold);
		NotifyVersionInfo(temp, L"Results will show....\nPress CTLR+SHIFT+E to see error\nPress CTRL + SHIFT + M to see error map\nPress CTRL + SHIFT + B to set Threshold");
		std::string orig_path = ConvertWideCharToMultiByte(sCurPage.GetString());
		std::string scan_path = ConvertWideCharToMultiByte((curPage == 2) ? m_scanPath2.GetString() : m_scanPath1.GetString());
		pc.process(orig_path, scan_path, &diff);
		ShowResults(m_CurrentThreshold, m_CurrentColor);

	}
	else
		need_to_update = true;
}


void PrintshopComparisonToolDlg::OnBnClickedButtonSideA()
{
	if (NoPages != 1)
	{
		curPage=1;
		UpdatePagesStates();
	}
}


void PrintshopComparisonToolDlg::OnBnClickedButtonSideB()
{
	if (NoPages != 1)
	{
		curPage = 2;
		UpdatePagesStates();
	}
}
// Batch mode
void PrintshopComparisonToolDlg::StartBatchProcessing(const CString& batchFilePath)
{
	m_batchMode = true;
	m_stopBatchThread = false;
	m_batchThread = std::thread(&PrintshopComparisonToolDlg::BatchProcess, this, batchFilePath);
}

void PrintshopComparisonToolDlg::StopBatchProcessing()
{
	m_stopBatchThread = true;
	if (m_batchThread.joinable())
	{
		m_batchThread.join();
	}
	m_batchMode = false;
}

void PrintshopComparisonToolDlg::BatchProcess(const CString& batchFilePath)
{
	std::wifstream batchFile(batchFilePath);
	if (!batchFile.is_open())
	{
		WriteLogFile(L"Failed to open batch file: %s", batchFilePath.GetString());
		return;
	}

	std::wstring origPath, scanPath;

	try
	{
		while (batchFile >> origPath >> scanPath)
		{
			// Set the paths
			m_origPath1 = origPath.c_str();
			m_scanPath1 = scanPath.c_str();
			if (!PathFileExists(m_origPath1) ||
				!PathFileExistsW(m_scanPath1))
			{
				WriteLogFile(L"Missing files in batch file");
				m_stopBatchThread = true;
			}
			if (m_stopBatchThread)
			{
				break;
			}

			// Set the orig file
			OnBnClickedButtonOrig();

			// Set the scan file
			OnBnClickedButtonScan();

			// Process the images
			OnBnClickedButtonProc();

			// Sleep for a short while to simulate user interaction
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
	catch (const std::exception& ex)
	{
		WriteLogFile(L"Exception in BatchProcess: %hs", ex.what());
	}
	catch (...)
	{
		WriteLogFile(L"Unknown exception in BatchProcess");
	}

	// Ensure the batch mode flag is cleared
	m_batchMode = false;
}
