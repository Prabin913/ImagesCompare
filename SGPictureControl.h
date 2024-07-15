#pragma once

class SGPictureControl : public CStatic
{
public:
    SGPictureControl();
    virtual ~SGPictureControl();

    void LoadImage(const CString& strImageFilePath);
    void SetBorderColor(COLORREF color);
    void SetBorderThickness(int thickness);
    void SetContextMenuItems(const std::vector<std::pair<UINT, CString>>& items);

protected:
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint point);
    afx_msg void OnOpenImage();
    afx_msg void OnShowScan(); // Declare handler for "Show Scan"
    DECLARE_MESSAGE_MAP()

private:
    void DrawImage(CDC* pDC);
    void DrawBorder(CDC* pDC);
    void ShowContextMenu(CPoint point);

    CMenu m_Menu;
    CString m_imageFilePath;
    CImage m_image;
    COLORREF m_borderColor;
    int m_borderThickness;
    std::vector<std::pair<UINT, CString>> m_contextMenuItems;
};
