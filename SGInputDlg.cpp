#include "pch.h"
#include "resource.h"
#include "SGInputDlg.h"
#include "afxdialogex.h"

//IMPLEMENT_DYNAMIC(CInputNumberDialog, CDialogEx)

CInputNumberDialog::CInputNumberDialog(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_INPUT_NUMBER_DIALOG, pParent), m_userInput(0)
{
}

void CInputNumberDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_NUMBER, m_userInput);
    DDV_MinMaxInt(pDX, m_userInput, 1, 128);
}

BEGIN_MESSAGE_MAP(CInputNumberDialog, CDialogEx)
END_MESSAGE_MAP()

BOOL CInputNumberDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    return TRUE;
}

void CInputNumberDialog::OnOK()
{
    UpdateData(TRUE);
    if (m_userInput < 1 || m_userInput > 128)
    {
        AfxMessageBox(_T("Please enter a number between 1 and 128."));
        return;
    }
    CDialogEx::OnOK();
}
