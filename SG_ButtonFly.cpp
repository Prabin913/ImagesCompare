/*

Secured Globe, Inc. 
**** SG_ButtonFly ****
Based on https://www.codeproject.com/Articles/5344494/Float-like-a-Butterfly-Sting-like-a-ButtonFly
©2022 Secured Globe, Inc.
1501 Broadway Ave, STE 1200
New York 10036, NY
USA
(646)4800506
info@securedglobe.com

*/

#include "pch.h"
#include "SG_ButtonFly.h"
#include <VersionHelpers.h>

#define DPI(xy, dpi) (xy) //MulDiv(xy, dpi, 96)

#define FONT_SIZE	(int)(12 * g_fDPIRate)
#define DEF_FONT	_T("Arial")

extern double g_fDPIRate;

#define CAPTION_HEIGHT		(int)(25 * g_fDPIRate)
#define	CAPTION_GAP			(int)(5 * g_fDPIRate)

int GetDpi(HWND hWnd)
{
	int nDpi = 0;

	if (IsWindows10OrGreater())
	{
		//nDpi = GetDpiForWindow(hWnd);
	}
	else
	{
		const auto hDc = GetDC(hWnd);
		nDpi = GetDeviceCaps(hDc, LOGPIXELSX);
		ReleaseDC(hWnd, hDc);
	}

	return nDpi;
}


IMPLEMENT_DYNAMIC(SG_ButtonFly, CButton)

SG_ButtonFly::SG_ButtonFly()
	: m_images()
	, m_bMouseTrack(TRUE)
	, m_pParent(NULL)
{
	m_clCaptionNormal = RGB(125, 70, 30);
	m_clCaptionHover = RGB(0, 0, 255);

	m_bUseCaption = false;
}

SG_ButtonFly::~SG_ButtonFly()
{
	ReleaseImages();
}

BEGIN_MESSAGE_MAP(SG_ButtonFly, CButton)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEMOVE()
	ON_WM_ENABLE()
END_MESSAGE_MAP()

BOOL SG_ButtonFly::OnEraseBkgnd(CDC* pDC)
{
	return TRUE; // CButton::OnEraseBkgnd(pDC);
}

void SG_ButtonFly::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_state = Press;
	Invalidate();
	//if (m_pParent)
	//	m_pParent->Invalidate();

	CButton::OnLButtonDown(nFlags, point);
}

void SG_ButtonFly::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_state = Hover;
	Invalidate();
	//if (m_pParent)
	//	m_pParent->Invalidate();

	CButton::OnLButtonUp(nFlags, point);
}

void SG_ButtonFly::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bMouseTrack)
	{
		TRACKMOUSEEVENT eventTrack;
		eventTrack.cbSize = sizeof(eventTrack);
		eventTrack.dwFlags = TME_LEAVE | TME_HOVER;
		eventTrack.hwndTrack = m_hWnd;
		eventTrack.dwHoverTime = 1;
		_TrackMouseEvent(&eventTrack);
		m_bMouseTrack = FALSE;
	}

	CButton::OnMouseMove(nFlags, point);
}

void SG_ButtonFly::OnMouseHover(UINT nFlags, CPoint point)
{
	m_state = Hover;
	Invalidate();
	//if (m_pParent)
	//	m_pParent->Invalidate();

	CButton::OnMouseHover(nFlags, point);
}

void SG_ButtonFly::OnMouseLeave()
{
	m_state = Normal;
	m_bMouseTrack = TRUE;
	//Invalidate();
	if (m_pParent)
	{
		CRect w_rtRect;
		GetWindowRect(w_rtRect);
		m_pParent->ScreenToClient(w_rtRect);
		m_pParent->InvalidateRect(w_rtRect);
	}

	CButton::OnMouseLeave();
}

void SG_ButtonFly::OnEnable(BOOL bEnable)
{
	CButton::OnEnable(bEnable);

	if (bEnable)
	{
		m_state = Normal;
		m_bMouseTrack = TRUE;

		//Invalidate();
		if (m_pParent)
		{
			CRect w_rtRect;
			GetWindowRect(w_rtRect);
			m_pParent->ScreenToClient(w_rtRect);
			m_pParent->InvalidateRect(w_rtRect);
		}
	}
}

void SG_ButtonFly::PreSubclassWindow()
{
	ModifyStyle(0, BS_OWNERDRAW);

	CButton::PreSubclassWindow();
}

void SG_ButtonFly::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	auto pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	if (lpDrawItemStruct->itemState & ODS_DISABLED)
	{
		m_state = Disable;
	}

	CDC memDc;
	CBitmap bmp;
	CFont w_fCaption;

	auto&& image = m_images[m_state];
	if (image.IsNull())
	{
		pDC->DrawText(_T("No Image"), &lpDrawItemStruct->rcItem, DT_CENTER | DT_VCENTER);
		return;
	}

	const auto nDpi = GetDpi(lpDrawItemStruct->hwndItem);
	//const auto nWidth = DPI(image.GetWidth(), nDpi);
	//const auto nHeight = DPI(image.GetHeight(), nDpi);
	unsigned int nWidth = lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left;
	unsigned int nHeight = lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top;
	unsigned int nIconDrawSize = min(nWidth, nHeight);
	unsigned int nIconWidth = nIconDrawSize;
	unsigned int nIconHeight = (unsigned int)(nIconDrawSize * (image.GetHeight() / image.GetWidth()));

	memDc.CreateCompatibleDC(pDC);
	bmp.CreateCompatibleBitmap(pDC, nWidth, nHeight);
	// make font
	w_fCaption.CreateFont(FONT_SIZE, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH & FF_SWISS, DEF_FONT);
	memDc.SelectObject(&bmp);

	//image.StretchBlt(memDc, 0, 0, nIconWidth, nIconHeight, 0, 0, image.GetWidth(), image.GetHeight(), SRCCOPY);
	image.Draw(memDc, 0, 0, nIconWidth, nIconHeight, 0, 0, image.GetWidth(), image.GetHeight());

	BLENDFUNCTION bf;
	bf.AlphaFormat = AC_SRC_ALPHA;
	bf.BlendFlags = 0;
	bf.BlendOp = AC_SRC_OVER;
	bf.SourceConstantAlpha = 255;
	int w_nLeftPos = (int)((lpDrawItemStruct->rcItem.right - nWidth) / 2);
	pDC->AlphaBlend(w_nLeftPos, 0, nWidth, nHeight, & memDc, 0, 0, nWidth, nHeight, bf);
	//pDC->AlphaBlend(0, 0, nWidth, nHeight, &memDc, 0, 0, nWidth, nHeight, bf);

	if ((!m_szCaption.IsEmpty()) && (m_bUseCaption))
	{
		pDC->SelectObject(&w_fCaption);
		pDC->SetBkMode(TRANSPARENT);
		if (m_state == Hover)
			pDC->SetTextColor(m_clCaptionHover);
		else
			pDC->SetTextColor(m_clCaptionNormal);
		pDC->DrawText(m_szCaption, m_szCaption.GetLength(),
			CRect(nIconWidth + CAPTION_GAP, (nHeight - CAPTION_HEIGHT) / 2,
				(int)((nWidth - CAPTION_GAP) * g_fDPIRate),
				CAPTION_HEIGHT), DT_VCENTER);
	}
}

void SG_ButtonFly::SetImages(UINT nNormalId, UINT nHoverId, UINT nPressId, UINT nDisableId, LPCTSTR lpszResourceType)
{
	ReleaseImages();
	LoadImageFromResource(m_images[Normal], nNormalId, lpszResourceType);
	LoadImageFromResource(m_images[Hover], nHoverId, lpszResourceType);
	LoadImageFromResource(m_images[Press], nPressId, lpszResourceType);
	nDisableId = nDisableId == 0 ? nNormalId : nDisableId;
	LoadImageFromResource(m_images[Disable], nDisableId, lpszResourceType);
	// AutoSize();
}

void SG_ButtonFly::LoadImageFromResource(CImage& img, UINT nId, LPCTSTR lpszResourceType)
{
	const static auto hInstance = AfxGetInstanceHandle();
	const auto hResource = ::FindResource(hInstance, MAKEINTRESOURCE(nId), lpszResourceType);
	if (hResource == nullptr)
	{
		return;
	}

	const auto hResData = LoadResource(hInstance, hResource);
	if (hResData == nullptr)
	{
		return;
	}

	const auto lpResource = LockResource(hResData);
	if (lpResource == nullptr)
	{
		return;
	}

	const auto dwResourceSize = SizeofResource(hInstance, hResource);
	const auto hMem = GlobalAlloc(GMEM_FIXED, dwResourceSize);
	if (hMem == nullptr)
	{
		return;
	}

	const auto lpMem = static_cast<LPBYTE>(GlobalLock(hMem));
	if (lpMem == nullptr)
	{
		GlobalFree(hMem);
		return;
	}

	memcpy(lpMem, lpResource, dwResourceSize);
	const auto pStream = SHCreateMemStream(lpMem, dwResourceSize);
	if (pStream != nullptr)
	{
		if (img.Load(pStream) == S_OK && _tcsicmp(lpszResourceType, _T("PNG")) == 0 && img.GetBPP() == 32)
		{
			for (auto i = 0; i < img.GetWidth(); i++)
			{
				for (auto j = 0; j < img.GetHeight(); j++)
				{
					const auto pColor = reinterpret_cast<LPBYTE>(img.GetPixelAddress(i, j));
					pColor[0] = pColor[0] * pColor[3] / 255;
					pColor[1] = pColor[1] * pColor[3] / 255;
					pColor[2] = pColor[2] * pColor[3] / 255;
				}
			}
		}
	}

	GlobalUnlock(lpMem);
	GlobalFree(hMem);
}

void SG_ButtonFly::AutoSize(double p_fDPIRate/* = 1.0*/, bool p_bUseCaption/* = false*/)
{
	auto&& image = m_images[Normal];
	if (image.IsNull())
	{
		return;
	}

	const auto nDpi = GetDpi(m_hWnd);

	if (p_bUseCaption)
		SetWindowPos(nullptr, -1, -1, DPI(image.GetWidth(), nDpi), DPI(image.GetHeight(), nDpi) + 100, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	else
		SetWindowPos(nullptr, -1, -1, DPI(image.GetWidth(), nDpi), DPI(image.GetHeight(), nDpi), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	//SetWindowPos(nullptr, -1, -1, (int)(image.GetWidth() * p_fDPIRate), (int)(image.GetHeight() * p_fDPIRate), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	m_bUseCaption = p_bUseCaption;
}

void SG_ButtonFly::SetParent(CWnd* p_pParent)
{
	m_pParent = p_pParent;
}

void SG_ButtonFly::SetCaptionText(LPCTSTR p_szCaption)
{
	m_szCaption = p_szCaption;

	m_bUseCaption = true;
}

void SG_ButtonFly::SetCaptionColor(COLORREF p_clCaptionNormal, COLORREF p_clCaptionHover)
{
	m_clCaptionNormal = p_clCaptionNormal;
	m_clCaptionHover = p_clCaptionHover;
}

void SG_ButtonFly::SetButtonSize(int p_nWidth, int p_nHeight)
{
	SetWindowPos(nullptr, -1, -1, p_nWidth, p_nHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void SG_ButtonFly::ReleaseImages()
{
	for (auto&& image : m_images)
	{
		image.Destroy();
	}
}
