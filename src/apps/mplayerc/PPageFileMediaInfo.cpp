/*
 * $Id$
 *
 * Copyright (C) 2012 Sergey "Exodus8" (rusguy6@gmail.com)
 *
 * This file is part of MPC-BE.
 * YOU CANNOT USE THIS FILE WITHOUT AUTHOR PERMISSION!
 *
 * MPC-BE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-BE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "mplayerc.h"
#include "MainFrm.h"
#include "PPageFileMediaInfo.h"
#include "../../DSUtil/WinAPIUtils.h"
#include <MediaInfo/MediaInfo.h>

using namespace MediaInfoLib;

MediaInfoLib::String mi_get_lang_file()
{
	CString path;
	GetModuleFileName(NULL, path.GetBuffer(_MAX_PATH), _MAX_PATH);
	path.ReleaseBuffer();
	path = path.Left(path.ReverseFind('\\') + 1);

	HINSTANCE mpcres = NULL;
	int lang = AfxGetAppSettings().iLanguage;

	if (lang) {
		mpcres = LoadLibrary(path + CMPlayerCApp::GetSatelliteDll(lang));
	}

	if (mpcres) {
		HRSRC hRes = FindResource(mpcres, MAKEINTRESOURCE(IDB_MEDIAINFO_LANGUAGE), _T("FILE"));

		if (hRes) {
			HANDLE lRes = LoadResource(mpcres, hRes);
			int size = SizeofResource(mpcres, hRes);

			wchar_t* wstr = new wchar_t[size];
			MultiByteToWideChar(CP_UTF8, 0, (char*)LockResource(lRes), size, wstr, size);

			UnlockResource(lRes);
			FreeResource(lRes);
			FreeLibrary(mpcres);

			return (MediaInfoLib::String)wstr;
		}
	}

	return (MediaInfoLib::String)_T("  Config_Text_ColumnSize;30");
}

// CPPageFileMediaInfo dialog

IMPLEMENT_DYNAMIC(CPPageFileMediaInfo, CPropertyPage)
CPPageFileMediaInfo::CPPageFileMediaInfo(CString fn, IFilterGraph* pFG)
	: CPropertyPage(CPPageFileMediaInfo::IDD, CPPageFileMediaInfo::IDD)
	, m_fn(fn)
	, m_pFG(pFG)
	, m_pCFont(NULL)
{
}

CPPageFileMediaInfo::~CPPageFileMediaInfo()
{
	delete m_pCFont;
	m_pCFont = NULL;
}

void CPPageFileMediaInfo::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MIEDIT, m_mediainfo);
}

BEGIN_MESSAGE_MAP(CPPageFileMediaInfo, CPropertyPage)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// CPPageFileMediaInfo message handlers

static WNDPROC OldControlProc;

static LRESULT CALLBACK ControlProc(HWND control, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_KEYDOWN) {
		if ((LOWORD(wParam)== 'A' || LOWORD(wParam) == 'a') && GetKeyState(VK_CONTROL) < 0) {
			CEdit *pEdit = (CEdit*)CWnd::FromHandle(control);
			pEdit->SetSel(0, pEdit->GetWindowTextLength(), TRUE);
			return 0;
		}
	}

	return CallWindowProc(OldControlProc, control, message, wParam, lParam);
}

BOOL CPPageFileMediaInfo::OnInitDialog()
{
	__super::OnInitDialog();

	if (!m_pCFont) {
		m_pCFont = DNew CFont;
	}

	if (!m_pCFont) {
		return TRUE;
	}

	MediaInfo MI;

	MI.Open(((CMainFrame*)AfxGetMyApp()->GetMainWnd())->m_strFnFull.GetString());
	MI.Option(_T("Complete"));
	MI.Option(_T("Language"), mi_get_lang_file());
	MI_Text = MI.Inform().c_str();
	MI.Close();

	if (!MI_Text.Find(_T("Unable to load"))) {
		MI_Text = _T("");
	}

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;

	LPCTSTR fonts[] = { _T("Lucida Console"), _T("Courier New"), _T("") };
	int fonts_size[] = { -10, -11, -11 };

	UINT i = 0;
	BOOL success;

	do {
		_tcscpy_s(lf.lfFaceName, fonts[i]);
		lf.lfHeight = fonts_size[i];
		success = IsFontInstalled(fonts[i]) && m_pCFont->CreateFontIndirect(&lf);
		i++;
	} while (!success && i < _countof(fonts));

	m_mediainfo.SetFont(m_pCFont);
	m_mediainfo.SetWindowText(MI_Text);

	OldControlProc = (WNDPROC)SetWindowLongPtr(m_mediainfo.m_hWnd, GWLP_WNDPROC, (LONG_PTR)ControlProc);

	return TRUE;
}

void CPPageFileMediaInfo::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	if (bShow) {
		GetParent()->GetDlgItem(IDC_BUTTON_MI)->ShowWindow(SW_SHOW);
	} else {
		GetParent()->GetDlgItem(IDC_BUTTON_MI)->ShowWindow(SW_HIDE);
	}
}
