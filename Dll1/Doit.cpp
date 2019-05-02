/*
 * ExplorerSortOrder: This implementation is Copyright (C) 2019 by Kevin Routley.
 * Based on Raymond Chen's "The Old New Thing" blog entries.
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
//#include <shdispid.h>

#include <intrin.h>
#include <winerror.h>

HRESULT GetLocationFromView(IShellBrowser* psb, PWSTR* ppszLocation);
//void ProcessOneWindow(IUnknown* punk);
//int start();
int findit(const wchar_t* target, wchar_t** store, int len, int* ascend);

// Raymond Chen: "The Old New Thing". Auto-cleanup of smart pointers.
class CCoInitialize
{
public:
	CCoInitialize() : m_hr(CoInitialize(nullptr)) { }
	~CCoInitialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
	operator HRESULT() const { return m_hr; }
	HRESULT m_hr;
};

#ifdef __cplusplus
extern "C" {
#endif

int __declspec(dllexport) take2(const wchar_t* path, wchar_t **str, int len, int& ascend)
{
	if (!path) 
		return -1;
	return findit(path, str, len, &ascend);
}

#ifdef __cplusplus
}
#endif


//int start()
//{
//	CCoInitialize init;
//	CComPtr<IShellWindows> spShellWindows;
//	spShellWindows.CoCreateInstance(CLSID_ShellWindows);
//
//	CComPtr<IUnknown> spunkEnum;
//	spShellWindows->_NewEnum(&spunkEnum);
//	CComQIPtr<IEnumVARIANT> spev(spunkEnum);
//
//	for (CComVariant svar; spev->Next(1, &svar, nullptr) == S_OK; svar.Clear())
//	{
//		ProcessOneWindow(svar.pdispVal);
//	}
//
//	return 0;
//}

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


//void ProcessOneWindow(IUnknown* punk)
//{
//	CComPtr<IShellBrowser> spsb;
//	if (FAILED(IUnknown_QueryService(punk, SID_STopLevelBrowser,
//		IID_PPV_ARGS(&spsb)))) return;
//
//	CComPtr<IShellView> spsv;
//	if (FAILED(spsb->QueryActiveShellView(&spsv))) return;
//
//	CComQIPtr<IFolderView2> spfv(spsv);
//	if (!spfv) return;
//
//	CComHeapPtr<WCHAR> spszLocation;
//	if (FAILED(GetLocationFromView(spsb, &spszLocation))) return;
//
//	printf("Location = %ls\n", static_cast<PCWSTR>(spszLocation));
//
//	int cColumns;
//	if (FAILED(spfv->GetSortColumnCount(&cColumns))) return;
//	if (cColumns > 10) cColumns = 10;
//
//	SORTCOLUMN rgColumns[10]; // arbitrary number
//	spfv->GetSortColumns(rgColumns, cColumns);
//
//	for (int i = 0; i < cColumns; i++) {
//		PCWSTR pszDir = rgColumns[0].direction > 0 ? L"ascending"
//			: L"descending";
//		PCWSTR pszName;
//		CComHeapPtr<WCHAR> spszName;
//		WCHAR szName[PKEYSTR_MAX];
//		if (SUCCEEDED(PSGetNameFromPropertyKey(rgColumns[0].propkey,
//			&spszName))) {
//			pszName = spszName;
//		}
//		else {
//			PSStringFromPropertyKey(rgColumns[0].propkey,
//				szName, ARRAYSIZE(szName));
//			pszName = szName;
//		}
//		printf("Column = %ls, direction = %ls\n", pszName, pszDir);
//	}
//}

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

	// TODO: how can explorer be sorted on more than one column?
	// TODO: this returns the LAST sort column
	for (int i = 0; i < cColumns; i++) 
	{
		// Sort direction
		*ascend = rgColumns[0].direction > 0 ? 1 : 0;

		// Sort column as a string
		PCWSTR pszName;
		CComHeapPtr<WCHAR> spszName;
		WCHAR szName[PKEYSTR_MAX];
		if (SUCCEEDED(PSGetNameFromPropertyKey(rgColumns[0].propkey, store))) 
		{
			pszName = spszName;
		}
		else 
		{
			PSStringFromPropertyKey(rgColumns[0].propkey, szName, ARRAYSIZE(szName));
			pszName = szName;
		}
	}
}

int findit(const wchar_t* target, wchar_t** store, int len, int* ascend)
{
	CCoInitialize init;
	CComPtr<IShellWindows> spShellWindows;
	const HRESULT hr = spShellWindows.CoCreateInstance(CLSID_ShellWindows);
	if (FAILED(hr)) return hr;

	CComPtr<IUnknown> spunkEnum;
	spShellWindows->_NewEnum(&spunkEnum);
	CComQIPtr<IEnumVARIANT> spev(spunkEnum);

	// Iterate thru all active Explorer windows to try to find the one with the target path
	for (CComVariant svar; spev->Next(1, &svar, nullptr) == S_OK; svar.Clear())
	{
		ProcessOneWindow2(svar.pdispVal, target, store, len, ascend);
	}

	return SEVERITY_SUCCESS;
}


