#pragma once
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

class SG_ButtonFly : public CButton
{
	DECLARE_DYNAMIC(SG_ButtonFly)

	SG_ButtonFly();
	virtual ~SG_ButtonFly();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnEnable(BOOL bEnable);
	void PreSubclassWindow() override;
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;

public:
	void SetImages(UINT nNormalId, UINT nHoverId, UINT nPressId, UINT nDisableId = 0, LPCTSTR lpszResourceType = _T("PNG"));
	void AutoSize(double p_fDPIRate = 1.0, bool p_bUseCaption = false);
	void SetParent(CWnd* p_pParent);
	void SetCaptionText(LPCTSTR p_szCaption);
	void SetCaptionColor(COLORREF p_clCaptionNormal, COLORREF p_clCaptionHover);
	void SetButtonSize(int p_nWidth, int p_nHeight);
	
protected:
	static void LoadImageFromResource(CImage& img, UINT nId, LPCTSTR lpszResourceType);
	void ReleaseImages();
	enum State
	{
		Normal = 0,
		Hover,
		Press,
		Disable,
	};
	
	CImage		m_images[4];
	State		m_state = Normal;
	BOOL        m_bMouseTrack;
	CWnd*		m_pParent;
	CString		m_szCaption;
	COLORREF	m_clCaptionNormal;
	COLORREF	m_clCaptionHover;
	bool		m_bUseCaption;
};
