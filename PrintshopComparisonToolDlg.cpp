
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
#include "PrintChecker.hpp"
#include "utils.h"
#include "SGInputDlg.h"


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

static vz::ImgCmp image_compare;
static CString annotate_path("annotation.png");

printcheck::PrintChecker pc;
bool need_to_update = false;
bool images_loaded=false;
PrintshopComparisonToolDlg::PrintshopComparisonToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_PRINTSHOPCOMPARISONTOOL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	NoPages=1;
	curPage=1;
}

void PrintshopComparisonToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER1, threshold_slider);
	DDX_Control(pDX, IDC_SLIDER2, filter_size_slider);
	DDX_Control(pDX, IDC_STATUS, m_Status);
	DDX_Control(pDX, IDC_BUTTON_ORIG, m_BtnOrig);
	DDX_Control(pDX, IDC_BUTTON_SCAN, m_BtnScan);
	DDX_Control(pDX, IDC_BUTTON_PROC, m_BtnProc);
	DDX_Control(pDX, IDC_BUTTON_OPENRESULT, m_BtnOpenResult);
	DDX_Control(pDX, IDC_BUTTON_SET_TH, m_BtnSetTH);
	DDX_Control(pDX, IDC_BUTTON_SIDE_A, m_BtnSideA);
	DDX_Control(pDX, IDC_BUTTON_SIDE_B, m_BtnSideB);

}

BEGIN_MESSAGE_MAP(PrintshopComparisonToolDlg, CDialog)
	ON_WM_HOTKEY()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
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
	ON_BN_CLICKED(IDC_BUTTON_OPENRESULT, &PrintshopComparisonToolDlg::OnBnClickedButtonOpenresult)
	ON_BN_CLICKED(IDC_BUTTON_SET_TH,&PrintshopComparisonToolDlg::OnBnClickedButtonSetTH)
	ON_BN_CLICKED(IDC_BUTTON_PROC, &PrintshopComparisonToolDlg::OnBnClickedButtonProc)
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

	if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED)
	{
		GetWindowRect(m_LastRect);

	}
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
		case IDC_SLIDER1:
		case IDC_SLIDER2:

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
	
	// Subclass the picture control
	m_pictureResults.SubclassDlgItem(IDC_PIC_DIFF, this);
	m_pictureResults.SetBorderThickness(10);
	m_pictureOrig.SubclassDlgItem(IDC_PIC_ORIG,this);
	m_pictureOrig.SetBorderThickness(10);
	m_pictureScan.SubclassDlgItem(IDC_PIC_SCAN,this);

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
	filter_size_slider.SetRange(0, 10, TRUE);
	threshold_slider.SetRange(0, 256, TRUE);
	threshold_slider.SetPos(100);
	m_BtnOpenResult.EnableWindow(FALSE);
	m_BtnProc.EnableWindow(FALSE);
	m_BtnScan.EnableWindow(FALSE);


	UpdateData(FALSE);
	thr = 100;
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
	// Extract file name from m_origPath
	LPCTSTR origPathFileName = PathFindFileName(m_origPath);
	if (origPathFileName != nullptr)
	{
		PDF = origPathFileName;
	}

	// Extract file name from m_scanPath
	LPCTSTR scanPathFileName = PathFindFileName(m_scanPath);
	if (scanPathFileName != nullptr)
	{
		PNG = scanPathFileName;
	}
	if (PDF != L"" && PNG != L"")
	{
		m_BtnOpenResult.EnableWindow(TRUE);
		m_BtnProc.EnableWindow(TRUE);
		m_BtnScan.EnableWindow(TRUE);
		UpdateData(FALSE);


	}
	else
	if (PDF != L"")
	{
		m_BtnOpenResult.EnableWindow(FALSE);
		m_BtnProc.EnableWindow(FALSE);
		m_BtnScan.EnableWindow(TRUE);
		UpdateData(FALSE);

		title.Format(L"Printshop Master PDF selected :'%s' PNG not selected yet", PDF);
		m_pictureOrig.SetBorderColor(RGB(255, 255, 255));
		m_pictureOrig.SetBorderThickness(7);
		m_pictureScan.SetBorderColor(RGB(255, 0, 255));
		m_pictureScan.SetBorderThickness(10);
		m_pictureResults.SetBorderColor(RGB(255, 255, 255));
		m_pictureResults.SetBorderThickness(7);

	}
	else
	{

		m_BtnOpenResult.EnableWindow(FALSE);
		m_BtnProc.EnableWindow(FALSE);
		m_BtnScan.EnableWindow(FALSE);
		UpdateData(FALSE);
		title.Format(L"Printshop Master PDF selected :'%s' PNG selected:'%s'", PDF, PNG);

		m_pictureOrig.SetBorderColor(RGB(255,0,255));
		m_pictureOrig.SetBorderThickness(10);
		m_pictureScan.SetBorderColor(RGB(255, 255, 255));
		m_pictureScan.SetBorderThickness(7);
		m_pictureResults.SetBorderColor(RGB(255, 255, 255));
		m_pictureResults.SetBorderThickness(7);

		title.Format(L"Printshop Master (new) - ready to start");
	}
	SetWindowText(title);
}
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

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

		DrawImage(GetDlgItem(IDC_PIC_ORIG), m_origPath);
		DrawImage(GetDlgItem(IDC_PIC_SCAN), m_scanPath);
		DrawImage(GetDlgItem(IDC_PIC_DIFF), m_diffPath);
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
	if (NoPages == 2)
	{
		m_BtnSideB.EnableWindow(TRUE);
		m_BtnSideA.EnableWindow(TRUE);

		if (curPage == 1)
		{
			m_BtnSideA.SetState(TRUE);
			m_BtnSideB.SetState(FALSE);
		}
		else
		{
			m_BtnSideB.SetState(TRUE);
			m_BtnSideA.SetState(FALSE);

		}

	}
	else
	{
		m_BtnSideB.EnableWindow(FALSE);
		m_BtnSideA.EnableWindow(FALSE);
	}
	UpdateData(FALSE);
}
void PrintshopComparisonToolDlg::OnTimer(UINT_PTR nIDEvent)
{

	CString thr_slider_echo;
	CString filt_slider_echo;

	static bool do_once{true};
	if(do_once)
	{ 
		do_once=false;
		NotifyVersionInfo(L"Ready to start", L"Please select an original PDF file");

	}
	thr_slider_echo.Format(_T("%d"), thr);
	filt_slider_echo.Format(_T("%d"), filt_s);
	GetDlgItem(IDC_STATIC_THR)->SetWindowTextW(thr_slider_echo);
	GetDlgItem(IDC_STATIC_FILT_SIZE)->SetWindowTextW(filt_slider_echo);

	if (need_to_update && images_loaded)
	{
		UpdatePagesStates();
		CWaitCursor w;
		double diff;
		image_compare.threshold_and_opening(thr, filt_s);
		CString temp;
		temp.Format(L"Threshold set to %d", thr);
		NotifyVersionInfo(temp, L"Results will show....\nPress CTLR+SHIFT+E to see error\nPress CTRL + SHIFT + M to see error map\nPress CTRL + SHIFT + B to set Threshold");
		std::string orig_path = ConvertWideCharToMultiByte(m_origPath.GetString());
		std::string scan_path = ConvertWideCharToMultiByte(m_scanPath.GetString());
		pc.process(orig_path, scan_path,diff);

		CString stdDiff;
		stdDiff.Format(L"Difference between images is  %.1f%%", diff);
		if (diff == 0.0)
		
		{ 
			m_pictureResults.SetBorderColor(RGB(0, 255, 0));
			m_pictureResults.SetBorderThickness(10);
			NotifyVersionInfo(L"Identical images!", stdDiff);

		}
		else
		{
			m_pictureResults.SetBorderColor(RGB(255, 0, 0));
			m_pictureResults.SetBorderThickness(10);
			NotifyVersionInfo(L"Unidentical images", stdDiff);

		}
		ShowResults(thr);

		need_to_update = false;
	}
	CDialog::OnTimer(nIDEvent);
}




void PrintshopComparisonToolDlg::OnClose()
{
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
	m_origPath = pdfFilePath + _T(".1.png");

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

	SetTitle();

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


void PrintshopComparisonToolDlg::ShowResults(int Threshold)
{
	if(!images_loaded) return;
	CWaitCursor w;

	auto annotated = pc.applyLimit(Threshold);
	cv::imwrite(ConvertWideCharToMultiByte(annotate_path), annotated);

	// save path to temporary annotation image
	// so it can be drawn correctly in PrintshopComparisonToolDlg::OnPaint
	m_diffPath = annotate_path;
	DrawImage(GetDlgItem(IDC_PIC_DIFF), m_diffPath);

	UpdateWindow();


}




void PrintshopComparisonToolDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{

	if (pScrollBar == (CScrollBar*)&threshold_slider ||
		pScrollBar == (CScrollBar*)&filter_size_slider)
	{
		static int last_th = -1;
		static int last_fs = -1;

		// update threshold and rerun the difference
		int current_thr = threshold_slider.GetPos();
		if (current_thr != last_th)
		{
			thr = current_thr;
			need_to_update = true;
			last_th = thr;
		}

		int current_filt_s = filter_size_slider.GetPos();
		if (current_filt_s != last_fs)
		{
			filt_s = current_filt_s;
			need_to_update = true;
			last_fs = filt_s;
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
	DeleteFile(m_origPath);
	m_origPath = L"";
	DeleteFile(m_origPath2);
	m_origPath2 = L"";

	WriteLogFile(L"User clicked PDF area");

	CString pdfPath = SelectFileFromDialog(1);
	if(pdfPath == L"") return;
	WriteLogFile(L"Selected pdf file: '%s'", pdfPath.GetString());
	if (ConvertPDF2IMG(pdfPath,pages))
	{
		CString strPages;
		strPages.Format(L"File has %d pages",pages);
		DrawImage(GetDlgItem(IDC_PIC_ORIG), m_origPath);
		NotifyVersionInfo(L"Original file loaded and converted. "+strPages, L"Now please select a scanned image");

	}
}


void PrintshopComparisonToolDlg::OnBnClickedButtonScan()
{
	WriteLogFile(L"Using clicked PNG area");

	m_scanPath = SelectFileFromDialog(0);
	if (!m_scanPath.IsEmpty())
	{
		DrawImage(GetDlgItem(IDC_PIC_SCAN), m_scanPath);
		SetTitle();
	}
	return;
}


void PrintshopComparisonToolDlg::OnBnClickedButtonOpenresult()
{
	ShellExecute(NULL, L"OPEN", m_diffPath.GetString(), NULL, L"", TRUE);
}
void PrintshopComparisonToolDlg::OnBnClickedButtonSetTH()
{
	CInputNumberDialog dlg;
	dlg.DoModal();
	int userInput = dlg.GetUserInput();
	if (userInput >= 1 && userInput <= 128)
	{
		// Update the slider position
		threshold_slider.SetPos(userInput);
		UpdateData(FALSE);
		thr = userInput;
		need_to_update = true;
		return;
	}

}


void PrintshopComparisonToolDlg::OnBnClickedButtonProc()
{
	WriteLogFile(L"Starting to compare %s and %s", m_origPath.GetString(), m_scanPath.GetString());
	images_loaded = true;
	m_pictureOrig.SetBorderColor(RGB(255, 255, 255));
	m_pictureOrig.SetBorderThickness(7);
	m_pictureScan.SetBorderColor(RGB(255, 255, 255));
	m_pictureScan.SetBorderThickness(7);
	m_pictureResults.SetBorderColor(RGB(255, 255, 255));
	m_pictureResults.SetBorderThickness(10);

	need_to_update = true;
}
