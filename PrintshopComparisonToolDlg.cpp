
// PrintshopComparisonToolDlg.cpp : implementation file
//

#include "pch.h"
#include "SplashWnd.h"
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
	DDX_Text(pDX, IDC_STATIC_THR, thr_slider_echo);
	DDX_Text(pDX, IDC_STATIC_FILT_SIZE, filt_slider_echo);
	DDX_Control(pDX, IDC_STATUS, m_Status);
}

BEGIN_MESSAGE_MAP(PrintshopComparisonToolDlg, CDialog)

	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_STN_CLICKED(IDC_PIC_ORIG, &PrintshopComparisonToolDlg::OnStnClickedPicOrig)
	ON_STN_CLICKED(IDC_PIC_SCAN, &PrintshopComparisonToolDlg::OnStnClickedPicScan)
	ON_WM_LBUTTONDOWN()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

// PrintshopComparisonToolDlg class implementation
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
	
	CSplashWnd::EnableSplashScreen();


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
	filt_slider_echo.Format(_T("%d"), filter_size_slider.GetPos());
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
		title.Format(L"Printshop Master - ready to start");
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
		FilterSpec = { (wchar_t*)L"PDF Files(*.pdf)\0*.pdf\0All Files(*.*)\0*.*\0" };
	else
		FilterSpec = { (wchar_t*)L"PNG Files(*.png)\0*.png\0All Files(*.*)\0*.*\0" };
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

bool PrintshopComparisonToolDlg::ConvertPDF2IMG(CString &pdfFilePath) {

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

static vz::ImgCmp image_compare;
static CString annotate_path("annotation.png");

void PrintshopComparisonToolDlg::CompareImage() 
{

	// new method
	
	printcheck::PrintChecker pc;
	std::filesystem::path p1;
	std::filesystem::path p2;
	p1=m_origPath.GetString();
	p2=m_scanPath.GetString();
//	imshow("blended", pc.process(p1,p2));
//	imshow("blended-50", pc.applyLimit(50));
//	imshow("error", pc.error());
//	imshow("error-map", pc.errormap());

	cv::Mat annotation_image = pc.process(p1, p2);
	cv::imwrite(ConvertWideCharToMultiByte(annotate_path), annotation_image);

	// save path to temporary annotation image
	// so it can be drawn correctly in PrintshopComparisonToolDlg::OnPaint
	m_diffPath = annotate_path;
	DrawImage(GetDlgItem(IDC_PIC_DIFF), m_diffPath);

	UpdateWindow();
	


	// old method
	return;

	/*
	image_compare.dilate_and_erode = 3;
	image_compare.resized_image_scale = 0.5;
	image_compare.min_contour_area = 20.0;

	image_compare.set_flag	(vz::ImgCmp::Flags::kDiffColour				);
	//image_compare.set_flag(vz::ImgCmp::Flags::kDiffGreyscale);
	image_compare.set_flag(vz::ImgCmp::Flags::kThresholdTriangle);
	image_compare.set_flag	(vz::ImgCmp::Flags::kDiffOriginalSize		);
	//image_compare.set_flag(vz::ImgCmp::Flags::kDiffResized);
	image_compare.set_flag(vz::ImgCmp::Flags::kAnnotateAddRedBorder);
	image_compare.set_flag(vz::ImgCmp::Flags::kAnnotateAddGreenBorder);
	image_compare.set_flag	(vz::ImgCmp::Flags::kAnnotateOverColour		);
	//image_compare.set_flag(vz::ImgCmp::Flags::kAnnotateOverGrey);
	image_compare.set_flag(vz::ImgCmp::Flags::kDrawContour);
	image_compare.set_flag(vz::ImgCmp::Flags::kDrawRectangle);

	std::string orgFilePath(ConvertWideCharToMultiByte(m_origPath));
	cv::Mat tmp = get_image(orgFilePath);
	cv::Size orgSize = tmp.size();
	image_compare.set_master_image(tmp);

	std::string scabFilePath(ConvertWideCharToMultiByte(m_scanPath));
	
	cv::Mat comparison_image = get_image(scabFilePath);
	image_compare.compare(comparison_image,
		threshold_slider.GetPos(),
		filter_size_slider.GetPos());

	cv::Mat annotation_image = image_compare.annotate();
	cv::imwrite(ConvertWideCharToMultiByte(annotate_path), annotation_image);
	
	// save path to temporary annotation image
	// so it can be drawn correctly in PrintshopComparisonToolDlg::OnPaint
	m_diffPath = annotate_path;
	DrawImage(GetDlgItem(IDC_PIC_DIFF), m_diffPath);

	UpdateWindow();
	*/
}

void PrintshopComparisonToolDlg::OnStnClickedPicOrig()
{

}


void PrintshopComparisonToolDlg::OnStnClickedPicScan()
{
	// TODO: Add your control notification handler code here
}


void PrintshopComparisonToolDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	try
	{
		CRect rc;
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
				CompareImage();
				WriteLogFile(L"Compare completed");
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

void PrintshopComparisonToolDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == (CScrollBar*)&threshold_slider or
		pScrollBar == (CScrollBar*)&filter_size_slider)
	{
		// update threshold and rerun the difference
		int thr = threshold_slider.GetPos();
		int filt_s = filter_size_slider.GetPos();

		thr_slider_echo.Format(_T("%d"), thr);
		filt_slider_echo.Format(_T("%d"), filt_s);

		image_compare.threshold_and_opening(thr, filt_s);
		cv::Mat annotation_image = image_compare.annotate();

		if (not annotation_image.empty()) {
			cv::imwrite(ConvertWideCharToMultiByte(annotate_path), annotation_image);
			m_diffPath = annotate_path;
			DrawImage(GetDlgItem(IDC_PIC_DIFF), m_diffPath);
		}

		UpdateData(FALSE);
	}
	else
	{
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	}
}
