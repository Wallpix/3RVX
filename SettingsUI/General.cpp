#include "stdafx.h"
#include "SettingsUI.h"
#include "General.h"
#include "afxdialogex.h"

#include "Settings.h"
#include "SkinInfo.h"

#define KEY_NAME L"3RVX"
#define STARTUP_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"

IMPLEMENT_DYNAMIC(General, CPropertyPage)

General::General() :
CPropertyPage(General::IDD) {

}

General::~General() {

}

void General::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, CHK_STARTUP, _startup);
    DDX_Control(pDX, CHK_NOTIFY, _notify);
    DDX_Control(pDX, CHK_SOUNDS, _sounds);
    DDX_Control(pDX, CMB_LANG, _lang);
    DDX_Control(pDX, GRP_SKIN, _skinGrp);
    DDX_Control(pDX, GRP_BEHAVIOR, _behaviorGrp);
    DDX_Control(pDX, GRP_LANGUAGE, _languageGrp);
    DDX_Control(pDX, LBL_AUTHOR, _author);
    DDX_Control(pDX, CMB_SKIN, _skins);
    DDX_Control(pDX, BTN_WEBSITE, _website);
}

BOOL General::OnApply() {
    OutputDebugString(L"-> General\n");
    Settings *settings = Settings::Instance();
    RunOnStartup(CHECKED(_startup));
    settings->NotifyIconEnabled(CHECKED(_notify));
    settings->SoundEffectsEnabled(CHECKED(_sounds));

    int skinIdx = _skins.GetCurSel();
    CString skinName;
    _skins.GetLBText(skinIdx, skinName);
    settings->CurrentSkin((LPCWSTR) skinName);

    UIUtils::SaveSettings(*this);
    return CPropertyPage::OnApply();
}

BOOL General::OnCommand(WPARAM wParam, LPARAM lParam) {
    SetModified();
    return CPropertyPage::OnCommand(wParam, lParam);
}

void General::LoadSettings() {
    Settings *settings = Settings::Instance();
    _startup.SetCheck(RunOnStartup());
    _notify.SetCheck(settings->NotifyIconEnabled());
    _sounds.SetCheck(settings->SoundEffectsEnabled());

    /* Determine which skins are available */
    std::list<CString> skins = FindSkins(Settings::SkinDir().c_str());
    for (CString skin : skins) {
        _skins.AddString(skin);
    }

    /* Update the combo box with the current skin */
    std::wstring current = settings->SkinName();
    int idx = _skins.SelectString(0, current.c_str());
    if (idx == CB_ERR) {
        _skins.SelectString(0, DEFAULT_SKIN);
    }
    LoadSkinInfo();

    /* Populate the language box */
    std::list<CString> languages = FindLanguages(
        settings->LanguagesDir().c_str());

    for (CString language : languages) {
        _lang.AddString(language);
    }
    std::wstring currentLang = settings->LanguageName();
    _lang.SelectString(0, currentLang.c_str());
}

BOOL General::OnInitDialog() {
    CPropertyPage::OnInitDialog();
    LoadSettings();
    return TRUE;
}

std::list<CString> General::FindSkins(CString dir) {
    std::list<CString> skins;

    CFileFind ff;
    dir += L"\\*";
    BOOL result = ff.FindFile(dir);
    while (result) {
        result = ff.FindNextFile();
        if (ff.IsDots()) {
            continue;
        }

        if (ff.IsDirectory()) {
            CFileFind inDir;
            CString dirPath = ff.GetFilePath();
            dirPath += L"\\skin.xml";
            if (inDir.FindFile(dirPath)) {
                /* We found a skin XML file; add the skin dir to our list. */
                skins.push_back(ff.GetFileName());
            }
        }
    }

    return skins;
}

std::list<CString> General::FindLanguages(CString dir) {
    std::list<CString> languages;

    CFileFind ff;
    dir += L"\\*.xml";
    BOOL result = ff.FindFile(dir);
    while (result) {
        result = ff.FindNextFile();
        if (ff.IsDots() || ff.IsDirectory()) {
            continue;
        }

        /* Even though we asked for *xml files, FindNextFile() will still
         * return results that start with .xml (like .xmlblah) */
        CString ext = ff.GetFileName().Right(3);
        if (ext == L"xml") {
            languages.push_back(ff.GetFileTitle());
        }
    }

    return languages;
}

void General::LoadSkinInfo() {
    CString selectedSkin;
    int selIdx = _skins.GetCurSel();
    _skins.GetLBText(selIdx, selectedSkin);

    std::wstring skinXML
        = Settings::Instance()->SkinXML((LPCWSTR) selectedSkin);
    SkinInfo s(skinXML);

    CString authorText(L"Author:");
    authorText.Append(L" ");
    authorText.Append(s.Author().c_str());
    _author.SetWindowTextW(authorText);

    _url = s.URL();
    _website.EnableWindow((_url != L""));
}
bool General::RunOnStartup() {
    CRegKey rk;
    int result = rk.Open(HKEY_CURRENT_USER, STARTUP_KEY, KEY_READ);
    if (result == ERROR_SUCCESS) {
        CString str;
        ULONG bufLen = 1024;
        LPTSTR buf = str.GetBufferSetLength(bufLen);

        int queryResult = rk.QueryStringValue(KEY_NAME, buf, &bufLen);
        if (queryResult == ERROR_SUCCESS) {
            return true;
        }
    }
    return false;
}

void General::RunOnStartup(bool enable) {
    std::wstring path = Settings::AppDir();
    CString exePath(path.c_str());
    exePath.Append(L"\\3RVX.exe");

    CRegKey rk;
    int result = rk.Open(HKEY_CURRENT_USER, STARTUP_KEY, KEY_WRITE);
    if (result == ERROR_SUCCESS) {
        if (enable) {
            rk.SetStringValue(KEY_NAME, exePath, REG_SZ);
        } else {
            rk.DeleteValue(KEY_NAME);
        }
    }
}

BEGIN_MESSAGE_MAP(General, CPropertyPage)
    ON_BN_CLICKED(BTN_WEBSITE, &General::OnBnClickedWebsite)
    ON_CBN_SELCHANGE(CMB_SKIN, &General::OnCbnSelchangeSkin)
END_MESSAGE_MAP()

void General::OnBnClickedWebsite() {
    ShellExecute(NULL, L"open", _url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void General::OnCbnSelchangeSkin() {
    LoadSkinInfo();
}