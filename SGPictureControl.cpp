#include "pch.h"
#include "resource.h"
#include "SGPictureControl.h"

BEGIN_MESSAGE_MAP(SGPictureControl, CStatic)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_CONTEXTMENU_OPENIMAGE, &SGPictureControl::OnOpenImage)
    ON_COMMAND(ID_CONTEXTMENU_SHOWSCAN, &SGPictureControl::OnShowScan) // Map "Show Scan" command
    // Add more as needed
END_MESSAGE_MAP()

SGPictureControl::SGPictureControl()
    : m_borderColor(RGB(0, 0, 0))
    , m_borderThickness(10)
{
    m_Menu.CreatePopupMenu();
    m_Menu.AppendMenu(MF_STRING, ID_CONTEXTMENU_OPENIMAGE, _T("Open image"));
}

SGPictureControl::~SGPictureControl()
{
    if (m_Menu.GetSafeHmenu())
        m_Menu.DestroyMenu();
}

void SGPictureControl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    ShowContextMenu(point);
}

void SGPictureControl::OnOpenImage()
{
    if (!m_imageFilePath.IsEmpty())
    {
        ShellExecute(NULL, _T("OPEN"), m_imageFilePath, NULL, NULL, SW_SHOWNORMAL);
    }
}

void SGPictureControl::OnShowScan()
{
    LoadImage(L"1_align_scanned.png");
    m_Menu.EnableMenuItem(ID_CONTEXTMENU_SHOWSCAN, MF_BYCOMMAND | MF_GRAYED);
    Invalidate();
    UpdateWindow();
}

void SGPictureControl::LoadImage(const CString& strImageFilePath)
{
    if (!m_image.IsNull()) m_image.Destroy();
    HRESULT hr = m_image.Load(strImageFilePath);
    if (FAILED(hr))
    {
        AfxMessageBox(_T("Failed to load image"));
    }
    else
    {
        m_imageFilePath = strImageFilePath;
    }
    Invalidate();
}

void SGPictureControl::SetBorderColor(COLORREF color)
{
    m_borderColor = color;
    Invalidate();
}

void SGPictureControl::SetBorderThickness(int thickness)
{
    m_borderThickness = thickness;
    Invalidate();
}

void SGPictureControl::SetContextMenuItems(const std::vector<std::pair<UINT, CString>>& items)
{
    m_contextMenuItems = items;

    if (m_Menu.GetSafeHmenu())
        m_Menu.DestroyMenu();

    m_Menu.CreatePopupMenu();
    for (const auto& item : m_contextMenuItems)
    {
        m_Menu.AppendMenu(MF_STRING, item.first, item.second);
    }
}

void SGPictureControl::OnPaint()
{
    CPaintDC dc(this);

    DrawImage(&dc);
    DrawBorder(&dc);
}

BOOL SGPictureControl::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

void SGPictureControl::OnSize(UINT nType, int cx, int cy)
{
    CStatic::OnSize(nType, cx, cy);
    Invalidate();
}

void SGPictureControl::DrawImage(CDC* pDC)
{
    if (m_image.IsNull())
    {
        return;
    }

    CRect rc;
    GetClientRect(&rc);
    rc.DeflateRect(m_borderThickness, m_borderThickness);
    m_image.StretchBlt(pDC->GetSafeHdc(), rc.left, rc.top, rc.Width(), rc.Height(), SRCCOPY);
}

void SGPictureControl::DrawBorder(CDC* pDC)
{
    if (!IsWindowVisible())
    {
        return;
    }

    CRect rc;
    GetClientRect(&rc);

    CPen pen(PS_SOLID, m_borderThickness, m_borderColor);
    CPen* pOldPen = pDC->SelectObject(&pen);
    pDC->SelectStockObject(NULL_BRUSH);
    pDC->Rectangle(&rc);
    pDC->SelectObject(pOldPen);
}

void SGPictureControl::ShowContextMenu(CPoint point)
{
    if (m_Menu.GetSafeHmenu() != nullptr)
    {
        m_Menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
    }
}
