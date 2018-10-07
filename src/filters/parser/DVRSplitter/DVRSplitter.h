/*
 * (C) 2018 see Authors.txt
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

#include "../BaseSplitter/BaseSplitter.h"

#define DVRSplitterName L"MPC DVR Splitter"
#define DVRSourceName   L"MPC DVR Source"

class __declspec(uuid("69D5E2F8-55D8-41B9-A95E-24A240F5F127"))
CDVRSplitterFilter : public CBaseSplitterFilter
{
	CAutoPtr<CBaseSplitterFileEx> m_pFile;

	__int64 m_startpos = 0;
	__int64 m_endpos   = 0;

	REFERENCE_TIME m_AvgTimePerFrame = 0;
	REFERENCE_TIME m_rtOffsetVideo = INVALID_TIME;
	REFERENCE_TIME m_rtOffsetAudio = INVALID_TIME;

	struct Header {
		DWORD sync, size, pts, dummy;
		REFERENCE_TIME rt;
		bool key_frame;
	};
	bool Sync();
	bool ReadHeader(Header& hdr);

	std::vector<SyncPoint> m_sps;

protected:
	HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

	bool DemuxInit();
	void DemuxSeek(REFERENCE_TIME rt);
	bool DemuxLoop();

public:
	CDVRSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);

	// CBaseFilter

	STDMETHODIMP_(HRESULT) QueryFilterInfo(FILTER_INFO* pInfo);

	// IKeyFrameInfo

	STDMETHODIMP GetKeyFrameCount(UINT& nKFs);
	STDMETHODIMP GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs);
};

class __declspec(uuid("E85619F1-2A32-4CF2-84E0-2F888E458EE1"))
CDVRSourceFilter : public CDVRSplitterFilter
{
public:
	CDVRSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);

	STDMETHODIMP_(HRESULT) QueryFilterInfo(FILTER_INFO* pInfo);
};
