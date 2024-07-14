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
    int m_selectedColor;

public:
    void SetUserInput(int);
    void SetColor(int color);

    int GetUserInput() const { return m_userInput; }
    int GetSelectedColor() const { return m_selectedColor; }
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    CComboBox m_Color;
    afx_msg void OnCbnSelchangeComboColors();
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
};
