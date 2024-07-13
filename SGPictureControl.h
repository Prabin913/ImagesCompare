// SGPictureControl.h
#pragma once

#include <afxwin.h>
#include <atlimage.h>

class SGPictureControl : public CStatic
{
public:
    SGPictureControl();
    virtual ~SGPictureControl();

    void LoadImage(const CString& strImageFilePath);
    void SetBorderColor(COLORREF color);
    void SetBorderThickness(int thickness);

protected:
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnOpenImage(); // Handle opening image here
    DECLARE_MESSAGE_MAP()

private:
    void DrawImage(CDC* pDC);
    void DrawBorder(CDC* pDC);
    void ShowContextMenu(CPoint point); // Method to show context menu

    CImage m_image;
    COLORREF m_borderColor;
    CString m_imageFilePath;
    int m_borderThickness;

    CMenu m_Menu; // Context menu object
};