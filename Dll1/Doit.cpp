/*
 * ExplorerSortOrder: This implementation is Copyright (C) 2019 by Kevin Routley.
 * Based on Raymond Chen's "The Old New Thing" blog entries.
 * 
 * Given a path to a target file/folder, determines the sort order of the Explorer
 * Window containing the target. Returns the sort direction and the sort column
 * as a string.
 */
#include "pch.h"

#include <windows.h>
#include <ole2.h>
#include <iostream>
#include <shlobj.h>
#include <atlbase.h>
#include <atlalloc.h>
#include <objbase.h>

#include <shobjidl.h>
#include <ShlGuid.h>

#include <intrin.h>
#include <winerror.h>
#include <strsafe.h>

// Raymond Chen: "The Old New Thing". Auto-cleanup of smart pointers.
class CCoInitialize
{
public:
	CCoInitialize() : m_hr(CoInitialize(nullptr)) { }
	~CCoInitialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
	operator HRESULT() const { return m_hr; }
	HRESULT m_hr;
};

// Windows Explorer specific version: get the current location of the view
HRESULT GetLocationFromView(IShellBrowser* psb, PWSTR* ppszLocation)
{
	*ppszLocation = nullptr;
	CComPtr<IShellView> spsv;

	// TODO: have we already done this? Can it be passed in?
	HRESULT hr = psb->QueryActiveShellView(&spsv);
	if (FAILED(hr)) return hr;

	CComQIPtr<IPersistIDList> sppidl(spsv);
	if (!sppidl) return E_FAIL;

	// Get the pidl, which tells us what Explorer is looking at
	CComHeapPtr<ITEMIDLIST_ABSOLUTE> spidl;
	hr = sppidl->GetIDList(&spidl);

	if (FAILED(hr)) return hr;

	CComPtr<IShellItem> spsi;
	hr = SHCreateItemFromIDList(spidl, IID_PPV_ARGS(&spsi));

	if (FAILED(hr)) return hr;

	// Get the human readable version
	hr = spsi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, ppszLocation);

	return hr;
}

void ProcessOneWindow2(IUnknown* punk, const WCHAR *target, WCHAR **store, int len, int *ascend)
{
	// 1. Ask for the top-level browser
	CComPtr<IShellBrowser> spsb;
	if (FAILED(IUnknown_QueryService(punk, SID_STopLevelBrowser,
		IID_PPV_ARGS(&spsb)))) return;

	// 2. Ask for the active shell view
	CComPtr<IShellView> spsv;
	if (FAILED(spsb->QueryActiveShellView(&spsv))) return;

	// 3. Convert to an IFolderView2
	CComQIPtr<IFolderView2> spfv(spsv);
	if (!spfv) return;

	// Any failure up to this point suggests the window doesn't support sorting.

	CComHeapPtr<WCHAR> spszLocation;
	if (FAILED(GetLocationFromView(spsb, &spszLocation))) return;

	// Does the location match the target (case-insensitive match)?
	if (StrCmpIW(spszLocation, target) != 0) return;

	int cColumns;
	if (FAILED(spfv->GetSortColumnCount(&cColumns))) return;
	if (cColumns > 10) cColumns = 10;

	SORTCOLUMN rgColumns[10]; // arbitrary number
	spfv->GetSortColumns(rgColumns, cColumns);

	// TODO: explorer can be sorted on more than one column using Shift+Click
	// TODO: this returns the LAST sort column
	for (int i = 0; i < cColumns; i++) 
	{
		// Sort direction
		*ascend = rgColumns[0].direction > 0 ? 1 : 0;

		// Sort column as a string
		CComHeapPtr<WCHAR> spszName;
		WCHAR szName[PKEYSTR_MAX];
		if (SUCCEEDED(PSGetNameFromPropertyKey(rgColumns[0].propkey, &spszName))) 
		{
			StringCbCopyNW(*store, len, spszName, len);
		}
		else 
		{
			// TODO when/how do we hit this path?
			PSStringFromPropertyKey(rgColumns[0].propkey, szName, ARRAYSIZE(szName));
			StringCbCopyNW(*store, len, szName, len);
		}
	}
}

// Declare the external function without C++ decoration
#ifdef __cplusplus
extern "C" {
#endif

int __declspec(dllexport) GetExplorerSortOrder(const wchar_t* target, wchar_t** store, int len, int& ascend)
{
	if (!target) return E_INVALIDARG; // no target path

	CCoInitialize init;
	CComPtr<IShellWindows> spShellWindows;
	const HRESULT hr = spShellWindows.CoCreateInstance(CLSID_ShellWindows);
	if (FAILED(hr)) return hr;

	CComPtr<IUnknown> spunkEnum;
	spShellWindows->_NewEnum(&spunkEnum);
	CComQIPtr<IEnumVARIANT> spev(spunkEnum);

	// Iterate thru all active Explorer windows to try to find the one with the target path
	ascend = -1;
	for (CComVariant svar; spev->Next(1, &svar, nullptr) == S_OK; svar.Clear())
	{
		ProcessOneWindow2(svar.pdispVal, target, store, len, &ascend);
	}

	if (ascend < 0)
		return E_FAIL;
	return SEVERITY_SUCCESS;
}

#ifdef __cplusplus
}
#endif
