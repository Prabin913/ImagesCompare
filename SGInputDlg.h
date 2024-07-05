#pragma once

class CInputNumberDialog : public CDialogEx
{
public:
    CInputNumberDialog(CWnd* pParent = nullptr);

    enum { IDD = IDD_INPUT_NUMBER_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    DECLARE_MESSAGE_MAP()

private:
    int m_userInput;

public:
    int GetUserInput() const { return m_userInput; }
    virtual BOOL OnInitDialog();
    virtual void OnOK();
};

