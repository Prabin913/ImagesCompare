#include "pch.h"
#include "resource.h"
#include "SGInputDlg.h"
#include "afxdialogex.h"
#include "constants.h"

// IMPLEMENT_DYNAMIC(CInputNumberDialog, CDialogEx)

CInputNumberDialog::CInputNumberDialog(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_INPUT_NUMBER_DIALOG, pParent), m_userInput(0), m_selectedColor(-1)
{
}

void CInputNumberDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_NUMBER, m_userInput);
    DDV_MinMaxInt(pDX, m_userInput, 1, 256);
    DDX_Control(pDX, IDC_COMBO_COLORS, m_Color);
}

BEGIN_MESSAGE_MAP(CInputNumberDialog, CDialogEx)
    ON_CBN_SELCHANGE(IDC_COMBO_COLORS, &CInputNumberDialog::OnCbnSelchangeComboColors)
    ON_WM_DRAWITEM()
END_MESSAGE_MAP()

BOOL CInputNumberDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_Color.InsertString(COLOR_PURPLE, COLOR_PURPLE_S);
    m_Color.InsertString(COLOR_BLUE, COLOR_BLUE_S);
    m_Color.InsertString(COLOR_RED, COLOR_RED_S);
    m_Color.SetCurSel(m_selectedColor);

    UpdateData(FALSE);
    return TRUE;
}

void CInputNumberDialog::OnOK()
{
    UpdateData(TRUE);
    if (m_userInput < 1 || m_userInput > 256)
    {
        AfxMessageBox(_T("Please enter a number between 1 and 256."));
        return;
    }
    CDialogEx::OnOK();
}

void CInputNumberDialog::SetUserInput(int userInput)
{
    m_userInput = userInput;
}

void CInputNumberDialog::SetColor(int color)
{
    m_selectedColor = color;
}

void CInputNumberDialog::OnCbnSelchangeComboColors()
{
    SetColor(m_Color.GetCurSel());
}

void CInputNumberDialog::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    if (nIDCtl == IDC_COMBO_COLORS && lpDrawItemStruct->itemID != -1)
    {
        CDC dc;
        dc.Attach(lpDrawItemStruct->hDC);

        // Save the original DC state
        int savedDC = dc.SaveDC();

        COLORREF textColor;
        switch (lpDrawItemStruct->itemID)
        {
            case COLOR_PURPLE:
                textColor = RGB(128, 0, 128);
                break;
            case COLOR_BLUE:
                textColor = RGB(0, 0, 255);
                break;
            case COLOR_RED:
                textColor = RGB(255, 0, 0);
                break;
            default:
                textColor = RGB(0, 0, 0);
                break;
        }

        // Set the text and background colors
        if (lpDrawItemStruct->itemState & ODS_SELECTED)
        {
            dc.SetTextColor(RGB(255, 255, 255));
            dc.SetBkColor(textColor);
            dc.FillSolidRect(&lpDrawItemStruct->rcItem, textColor);
        }
        else
        {
            dc.SetTextColor(textColor);
            dc.SetBkColor(RGB(255, 255, 255));
            dc.FillSolidRect(&lpDrawItemStruct->rcItem, RGB(255, 255, 255));
        }

        // Draw the text
        CString itemText;
        m_Color.GetLBText(lpDrawItemStruct->itemID, itemText);
        dc.DrawText(itemText, &lpDrawItemStruct->rcItem, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        // Restore the original DC state
        dc.RestoreDC(savedDC);
        dc.Detach();
    }
    else
    {
        CDialogEx::OnDrawItem(nIDCtl, lpDrawItemStruct);
    }
}
