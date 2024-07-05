
// PrintshopComparisonToolDlg.cpp : implementation file
//

#include "pch.h"
#include <stdlib.h>
#include "framework.h"
#include "PrintshopComparisonTool.h"
#include "PrintshopComparisonToolDlg.h"
#include "afxdialogex.h"
#include "mupdf/pdf.hpp"
#include "PrintChecker.hpp"
#include "utils.h"

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

PrintshopComparisonToolDlg::PrintshopComparisonToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_PRINTSHOPCOMPARISONTOOL_DIALOG, pParent)
	, thr_slider_echo(_T(""))
	, filt_slider_echo(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void PrintshopComparisonToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER1, threshold_slider);
	DDX_Control(pDX, IDC_SLIDER2, filter_size_slider);
	DDX_Control(pDX, IDC_STATUS, m_Status);
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
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
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
			imshow("blended-50", pc.applyLimit(50));
			break;
		case 3:
			imshow("error-map", pc.errormap());
			break;


	}

	CDialog::OnHotKey(nHotKeyId, nKey1, nKey2);
}
void PrintshopComparisonToolDlg::OnSize(UINT nType, int cx, int cy)
{

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
		case IDC_SLIDER1:
		case IDC_SLIDER2:
		case IDC_STATIC_THR:
		case IDC_STATIC_FILT_SIZE:
		case IDC_STATIC_TH:
		case IDC_STATIC_FL:
		/*  IDC_STATIC_FL - bad IDC_STATIC_TH - good*/
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

	// initialize sliders
	threshold_slider.SetRange(0, 256, TRUE);
	threshold_slider.SetPos(100);
	thr_slider_echo.Format(_T("%d"), threshold_slider.GetPos());

	filter_size_slider.SetRange(0, 10, TRUE);
	filter_size_slider.SetPos(3);

	thr = filter_size_slider.GetPos();
	filt_s = filter_size_slider.GetPos();

	filt_slider_echo.Format(_T("%d"), filter_size_slider.GetPos());
	thr_slider_echo.Format(_T("%d"), thr);
	filt_slider_echo.Format(_T("%d"), filt_s);

	UpdateData(FALSE);
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
		title.Format(L"Printshop Master PDF selected :'%s' PNG selected:'%s'", PDF, PNG);
	}
	else
	if (PDF != L"")
	{
		title.Format(L"Printshop Master PDF selected :'%s' PNG not selected yet", PDF);

	}
	else
	{
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



void PrintshopComparisonToolDlg::OnTimer(UINT_PTR nIDEvent)
{

	// TODO: Add your message handler code here and/or call default
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

	if (need_to_update && !mouse_down)
	{
		CWaitCursor w;
		image_compare.threshold_and_opening(thr, filt_s);
		CString temp;
		temp.Format(L"Threshold set to %d", thr);
		NotifyVersionInfo(temp, L"Results will show....\nPress CTLR+SHIFT+E to see error\nPress CTRL + SHIFT + M to see error map\nPress CTRL + SHIFT + B to see blended - 50");
		std::string orig_path = ConvertWideCharToMultiByte(m_origPath.GetString());
		std::string scan_path = ConvertWideCharToMultiByte(m_scanPath.GetString());
		pc.process(orig_path, scan_path);

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
		FilterSpec = { (wchar_t*)L"PDF Files(*.pdf)\0*.pdf\0PNG Files(*.png)\0*.png\0Tiff Files(*.tiff)\0*.tiff\0All Files(*.*)\0*.*\0" };
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

bool PrintshopComparisonToolDlg::ConvertPDF2IMG(CString &pdfFilePath) 
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
	}

	int pages = 0;
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

	free(szSrcFilePath);

	m_origPath = pdfFilePath + _T(".1.png");
	SetTitle();

	return true;
}

void PrintshopComparisonToolDlg::DrawImage(CWnd *pRenderWnd, const CString &strImageFilePath)
{
	CWaitCursor w;
	if (strImageFilePath.IsEmpty()) return;

	CRect rc;
	pRenderWnd->GetWindowRect(&rc);

	CImage pngImage;
	pngImage.Load(strImageFilePath);
	pngImage.StretchBlt(pRenderWnd->GetDC()->GetSafeHdc(), CRect(0, 0, rc.Width(), rc.Height()));
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
	CWaitCursor w;
	auto annotated = pc.applyLimit(Threshold);
	cv::imwrite(ConvertWideCharToMultiByte(annotate_path), annotated);

	// save path to temporary annotation image
	// so it can be drawn correctly in PrintshopComparisonToolDlg::OnPaint
	m_diffPath = annotate_path;
	DrawImage(GetDlgItem(IDC_PIC_DIFF), m_diffPath);

	UpdateWindow();


}
void PrintshopComparisonToolDlg::CompareImage() 
{
	need_to_update=true;
	return;

}


void PrintshopComparisonToolDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	mouse_down = true;
	try
	{
		CRect rc;
		GetDlgItem(IDC_PIC_DIFF)->GetWindowRect(&rc);
		ScreenToClient(&rc);
		if (rc.PtInRect(point))
		{
			ShellExecute(NULL, L"OPEN", m_diffPath.GetString(), NULL, L"", TRUE);
			return;
		}
		GetDlgItem(IDC_PIC_ORIG)->GetWindowRect(&rc);
		ScreenToClient(&rc);
		if (rc.PtInRect(point))
		{
			WriteLogFile(L"User clicked PDF area");

			CString pdfPath = SelectFileFromDialog(1);
			WriteLogFile(L"Selected pdf file: '%s'", pdfPath.GetString());
			if (ConvertPDF2IMG(pdfPath))
			{

				DrawImage(GetDlgItem(IDC_PIC_ORIG), m_origPath);
				NotifyVersionInfo(L"Original file loaded and converted", L"Now please select a scanned image");

			}
			//m_origPath = pdfPath;
			//DrawImage(GetDlgItem(IDC_PIC_ORIG), m_origPath);
			return;
		}

		GetDlgItem(IDC_PIC_SCAN)->GetWindowRect(&rc);
		ScreenToClient(&rc);
		if (rc.PtInRect(point))
		{
			WriteLogFile(L"Using clicked PNG area");

			m_scanPath = SelectFileFromDialog(0);
			if (!m_scanPath.IsEmpty())
			{
				DrawImage(GetDlgItem(IDC_PIC_SCAN), m_scanPath);
				WriteLogFile(L"Starting to compare %s and %s", m_origPath.GetString(), m_scanPath.GetString());
				SetTitle();
				need_to_update =true;
			}
			return;
		}

		UpdateWindow();

	}
	catch (const std::exception& e)
	{
		// Log any unhandled exception that may occur during
		// the image comparison.
		WriteLogFile(L"%S: %S", typeid(e).name(), e.what());
	}

	CDialog::OnLButtonDown(nFlags, point);
}
void PrintshopComparisonToolDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	mouse_down = false;
	CDialog::OnLButtonDown(nFlags, point);
}


void PrintshopComparisonToolDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == (CScrollBar*)&threshold_slider ||
		pScrollBar == (CScrollBar*)&filter_size_slider)
	{
		// update threshold and rerun the difference
		thr = threshold_slider.GetPos();
		filt_s = filter_size_slider.GetPos();

		need_to_update = true;

	}
	else
	{
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	}
}


