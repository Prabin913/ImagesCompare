
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

#include <iostream>
#include <sstream>
#include "VzImgCmp.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define SELF_REMOVE_STRING  s_2560964179().c_str()
#define CHRNULL_A   '\0'
double g_fDPIRate = 1.0;

// PrintshopComparisonToolDlg dialog
#define DATETIME_BUFFER_SIZE 80
#define BUFFER_SIZE 4096
#ifdef LOG
#define LOG_FILE_NAME L"PrintshopComparisonTool.log"
#endif


PrintshopComparisonToolDlg::PrintshopComparisonToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_PRINTSHOPCOMPARISONTOOL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void PrintshopComparisonToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
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

	if (pWnd == this)
	{
		return m_brBack;
	}
	// Check if the control is a menu
	if (nCtlColor == WM_CTLCOLORMSGBOX)
	{
		// Set the background color for the menu
		pDC->SetBkColor(TX_COLOR2);
		return m_brBack2; // Return a brush with the same color
	}


	switch (pWnd->GetDlgCtrlID())
	{
		pDC->SetTextColor(TX_COLOR2);
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetBkColor(BG_COLOR);
		return m_brBack;
		break;
	default:
	{
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetTextColor(RGB(0, 0, 0));
		return m_tx;
	}
	}

	return hbr;
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



	SetTimer(1000, 500, NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
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


void shutSystemOff(const bool& shutReboot = true)
{
	// Create all required variables
	int vRet = 0;
	bool adjustRet;
	HANDLE hToken = NULL;
	LUID luid;
	TOKEN_PRIVILEGES tp;

	// Get LUID for current boot for current process.
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &luid);

	// Modify and Adjust token privileges for current process.
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	adjustRet = AdjustTokenPrivileges(hToken, false, &tp, sizeof(tp), NULL, 0);

	// Check if token privileges were set.
	if (adjustRet)
	{
		// Initiates system shutdown ( Local system, Shutdown Message, Dwell time, Prompt user, Reboot )
		InitiateSystemShutdownA(NULL, NULL, 0, true, shutReboot);
	}
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
	if (_pdf->needs_password()) {
		CString strErr;
		strErr.Format(_T("%s was protected by password"), pdfFilePath);
		AfxMessageBox(strErr);
	}

	int pages = 0;
	int zoom = 300;
	if (_pdf->good() && _pdf->size() != 0) {
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
	std::cout << "-> reading " << filename << ": ";

	if (filename.empty())
	{
		throw std::invalid_argument("cannot read mage from empty filename");
	}

	cv::Mat mat = cv::imread(filename, cv::IMREAD_COLOR);

	if (mat.channels() != 3 ||
		mat.cols < 10 ||
		mat.rows < 10)
	{
		throw std::runtime_error("failed to read image " + filename);
	}

	std::cout << mat.cols << "x" << mat.rows << std::endl;

	return mat;
}

void PrintshopComparisonToolDlg::CompareImage() {
	vz::ImgCmp image_compare;

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
	image_compare.compare(comparison_image);

	cv::Mat annotation_image = image_compare.annotate();
	CString annotate_path("annotation.png");
	cv::imwrite(ConvertWideCharToMultiByte(annotate_path), annotation_image);
	
	// save path to temporary annotation image
	// so it can be drawn correctly in PrintshopComparisonToolDlg::OnPaint
	m_diffPath = annotate_path;
	DrawImage(GetDlgItem(IDC_PIC_DIFF), m_diffPath);

	UpdateWindow();
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
	CRect rc;
	GetDlgItem(IDC_PIC_ORIG)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	if (rc.PtInRect(point)) {
		CString pdfPath = SelectFileFromDialog(1);
		if (ConvertPDF2IMG(pdfPath)) {
			DrawImage(GetDlgItem(IDC_PIC_ORIG), m_origPath);
		}
		//m_origPath = pdfPath;
		//DrawImage(GetDlgItem(IDC_PIC_ORIG), m_origPath);
		return;
	}

	GetDlgItem(IDC_PIC_SCAN)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	if (rc.PtInRect(point)) {
		m_scanPath = SelectFileFromDialog(0);
		if (!m_scanPath.IsEmpty()) {
			DrawImage(GetDlgItem(IDC_PIC_SCAN), m_scanPath);
			CompareImage();
		}
		return;
	}

	UpdateWindow();

	CDialog::OnLButtonDown(nFlags, point);
}
