#include "pch.h"
#include "SGPictureControl.h"

BEGIN_MESSAGE_MAP(SGPictureControl, CStatic)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
END_MESSAGE_MAP()

SGPictureControl::SGPictureControl()
    : m_borderColor(RGB(0, 0, 0))
    , m_borderThickness(10) // Default border thickness
{
}

SGPictureControl::~SGPictureControl()
{
}

void SGPictureControl::LoadImage(const CString& strImageFilePath)
{
    HRESULT hr = m_image.Load(strImageFilePath);
    if (FAILED(hr))
    {
        AfxMessageBox(_T("Failed to load image"));
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

void SGPictureControl::OnPaint()
{
    CPaintDC dc(this);

    // Draw the image and border
    DrawImage(&dc);
    DrawBorder(&dc);
}

BOOL SGPictureControl::OnEraseBkgnd(CDC* pDC)
{
    // Avoid erasing background to reduce flickering
    return TRUE;
}

void SGPictureControl::OnSize(UINT nType, int cx, int cy)
{
    CStatic::OnSize(nType, cx, cy);

    // Redraw the control upon resizing
    Invalidate();
}

void SGPictureControl::DrawImage(CDC* pDC)
{
    if (m_image.IsNull()) return;

    CRect rc;
    GetClientRect(&rc);

    // Adjust the client area for the border thickness
    rc.DeflateRect(m_borderThickness, m_borderThickness);

    // Draw the image inside the adjusted client area
    m_image.StretchBlt(pDC->GetSafeHdc(),
        rc.left, rc.top,           // Starting position adjusted for border
        rc.Width(), rc.Height(),   // Size adjusted for border
        SRCCOPY);
}

void SGPictureControl::DrawBorder(CDC* pDC)
{
    CRect rc;
    GetClientRect(&rc);

    // Draw the border using specified thickness and color
    CPen pen(PS_SOLID, m_borderThickness, m_borderColor);
    CPen* pOldPen = pDC->SelectObject(&pen);
    pDC->SelectStockObject(NULL_BRUSH); // No brush to fill the rectangle
    pDC->Rectangle(&rc);
    pDC->SelectObject(pOldPen);
}
