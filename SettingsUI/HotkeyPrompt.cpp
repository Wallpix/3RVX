#include "stdafx.h"
#include "SettingsUI.h"
#include "HotkeyPrompt.h"
#include "afxdialogex.h"

#include "KeyGrabber.h"

IMPLEMENT_DYNAMIC(HotkeyPrompt, CDialog)

HotkeyPrompt::HotkeyPrompt(CWnd* pParent /*=NULL*/) :
CDialog(HotkeyPrompt::IDD, pParent) {

}

HotkeyPrompt::~HotkeyPrompt() {

}

BOOL HotkeyPrompt::OnInitDialog() {
    CDialog::OnInitDialog();
    SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    KeyGrabber::Instance()->SetHwnd(m_hWnd);
    KeyGrabber::Instance()->Grab();
    return TRUE;
}

void HotkeyPrompt::DoDataExchange(CDataExchange* pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, LBL_KEYS, _keys);
    DDX_Control(pDX, BTN_CANCEL, _cancel);
}

BEGIN_MESSAGE_MAP(HotkeyPrompt, CDialog)
    ON_BN_CLICKED(BTN_CANCEL, &HotkeyPrompt::OnBnClickedCancel)
END_MESSAGE_MAP()

void HotkeyPrompt::OnBnClickedCancel() {
    PostMessage(WM_CLOSE, NULL, NULL);
}
