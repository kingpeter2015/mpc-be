/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
 *
 * This file is part of MPC-BE.
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

#pragma once

#include <atlcoll.h>
#include <BaseClasses/wxutil.h>
#include "TextFile.h"
#include "GFN.h"

#define DEFSCREENSIZE CSize(384, 288)

typedef enum {TIME, FRAME} tmode; // the meaning of STSEntry::start/end

class STSStyle
{
public:
	CRect		marginRect;		// measured from the sides
	int			scrAlignment;	// 1 - 9: as on the numpad, 0: default
	int			borderStyle;	// 0: outline, 1: opaque box
	double		outlineWidthX, outlineWidthY;
	double		shadowDepthX, shadowDepthY;
	COLORREF	colors[4];		// usually: {primary, secondary, outline/background, shadow}
	BYTE		alpha[4];
	int			charSet;
	CString		fontName;
	double		fontSize;		// height
	double		fontScaleX, fontScaleY;	// percent
	double		fontSpacing;	// +/- pixels
	LONG		fontWeight;
	BYTE		fItalic;
	BYTE		fUnderline;
	BYTE		fStrikeOut;
	int			fBlur;
	double		fGaussianBlur;
	double		fontAngleZ, fontAngleX, fontAngleY;
	double		fontShiftX, fontShiftY;
	int			relativeTo;		// 0: window, 1: video, 2: undefined (~window)

	STSStyle();

	void SetDefault();

	bool operator == (STSStyle& s);
	bool IsFontStyleEqual(STSStyle& s);

	STSStyle& operator = (LOGFONT& lf);

	friend LOGFONTA& operator <<= (LOGFONTA& lfa, STSStyle& s);
	friend LOGFONTW& operator <<= (LOGFONTW& lfw, STSStyle& s);

	friend CString& operator <<= (CString& style, STSStyle& s);
	friend STSStyle& operator <<= (STSStyle& s, CString& style);
};

class CSTSStyleMap : public CAtlMap<CString, STSStyle*, CStringElementTraits<CString> >
{
public:
	CSTSStyleMap() {}
	virtual ~CSTSStyleMap() {
		Free();
	}
	void Free();
};

typedef struct {
	CStringW str;
	bool fUnicode;
	CString style, actor, effect;
	CRect marginRect;
	int layer;
	int start, end;
	int readorder;
} STSEntry;

class STSSegment
{
public:
	int start, end;
	CAtlArray<int> subs;

	STSSegment() {};
	STSSegment(int s, int e) {
		start = s;
		end = e;
	}
	STSSegment(const STSSegment& stss) {
		*this = stss;
	}
	STSSegment& operator = (const STSSegment& stss) {
		if (this != &stss) {
			start = stss.start;
			end = stss.end;
			subs.Copy(stss.subs);
		}
		return *this;
	}
};

class CSimpleTextSubtitle : public CAtlArray<STSEntry>
{
	friend class CSubtitleEditorDlg;

protected:
	CAtlArray<STSSegment> m_segments;
	virtual void OnChanged() {}

public:
	CString m_name;
	LCID m_lcid;
	exttype m_exttype;
	tmode m_mode;
	CTextFile::enc m_encoding;
	CString m_path;

	CSize m_dstScreenSize;
	int m_defaultWrapStyle;
	int m_collisions;
	bool m_fScaledBAS;

	bool m_fUsingAutoGeneratedDefaultStyle;

	CSTSStyleMap m_styles;

	enum EPARCompensationType {
		EPCTDisabled = 0,
		EPCTDownscale = 1,
		EPCTUpscale = 2,
		EPCTAccurateSize = 3
	};

	EPARCompensationType m_ePARCompensationType;
	double m_dPARCompensation;

public:
	CSimpleTextSubtitle();
	virtual ~CSimpleTextSubtitle();

	virtual void Copy(CSimpleTextSubtitle& sts);
	virtual void Empty();

	void Sort(bool fRestoreReadorder = false);
	void CreateSegments();

	void Append(CSimpleTextSubtitle& sts, int timeoff = -1);

	bool Open(CString fn, int CharSet, CString name = _T(""));
	bool Open(CTextFile* f, int CharSet, CString name);
	bool Open(BYTE* data, int len, int CharSet, CString name);
	bool SaveAs(CString fn, exttype et, double fps = -1, CTextFile::enc = CTextFile::ASCII);

	void Add(CStringW str, bool fUnicode, int start, int end, CString style = _T("Default"), CString actor = _T(""), CString effect = _T(""), CRect marginRect = CRect(0,0,0,0), int layer = 0, int readorder = -1);
	STSStyle* CreateDefaultStyle(int CharSet);
	void ChangeUnknownStylesToDefault();
	void AddStyle(CString name, STSStyle* style); // style will be stored and freed in Empty() later
	bool CopyStyles(const CSTSStyleMap& styles, bool fAppend = false);

	bool SetDefaultStyle(STSStyle& s);
	bool GetDefaultStyle(STSStyle& s);

	void ConvertToTimeBased(double fps);
	void ConvertToFrameBased(double fps);

	int TranslateStart(int i, double fps);
	int TranslateEnd(int i, double fps);
	int SearchSub(int t, double fps);

	int TranslateSegmentStart(int i, double fps);
	int TranslateSegmentEnd(int i, double fps);
	const STSSegment* SearchSubs(int t, double fps, /*[out]*/ int* iSegment = NULL, int* nSegments = NULL);
	const STSSegment* GetSegment(int iSegment) {
		return iSegment >= 0 && iSegment < (int)m_segments.GetCount() ? &m_segments[iSegment] : NULL;
	}

	STSStyle* GetStyle(int i);
	bool GetStyle(int i, STSStyle& stss);
	int GetCharSet(int i);
	bool IsEntryUnicode(int i);
	void ConvertUnicode(int i, bool fUnicode);

	CStringA GetStrA(int i, bool fSSA = false);
	CStringW GetStrW(int i, bool fSSA = false);
	CStringW GetStrWA(int i, bool fSSA = false);

	#define GetStr GetStrW

	void SetStr(int i, CStringA str, bool fUnicode /* ignored */);
	void SetStr(int i, CStringW str, bool fUnicode);
};

extern BYTE CharSetList[];
extern TCHAR* CharSetNames[];
extern int CharSetLen;

class CHtmlColorMap : public CAtlMap<CString, DWORD, CStringElementTraits<CString> >
{
public:
	CHtmlColorMap();
};

extern CHtmlColorMap g_colors;
