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
#include <shdispid.h>

#include <intrin.h>

HRESULT GetLocationFromView(IShellBrowser* psb, PWSTR* ppszLocation);
void ProcessOneWindow(IUnknown* punk);
int start();
int findit(const wchar_t* path, wchar_t** str, int len, int* ascend);

class CCoInitialize
{
public:
	CCoInitialize() : m_hr(CoInitialize(NULL)) { }
	~CCoInitialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
	operator HRESULT() const { return m_hr; }
	HRESULT m_hr;
};

#ifdef __cplusplus
extern "C" {
#endif

int __declspec(dllexport) find_sort(const wchar_t* path, int& cookie, int& ascend)
{
	if (!path) return -1;
	start();
	return 0;
}

int __declspec(dllexport) take2(const wchar_t* path, wchar_t **str, int len, int& ascend)
{
	if (!path) return -1;
	return findit(path, str, len, &ascend);
}

#ifdef __cplusplus
}
#endif


int start()
{
	CCoInitialize init;
	CComPtr<IShellWindows> spShellWindows;
	spShellWindows.CoCreateInstance(CLSID_ShellWindows);

	CComPtr<IUnknown> spunkEnum;
	spShellWindows->_NewEnum(&spunkEnum);
	CComQIPtr<IEnumVARIANT> spev(spunkEnum);

	for (CComVariant svar; spev->Next(1, &svar, nullptr) == S_OK; svar.Clear())
	{
		ProcessOneWindow(svar.pdispVal);
	}

	return 0;
}

HRESULT GetLocationFromView(IShellBrowser* psb, PWSTR* ppszLocation)
{
	HRESULT hr;

	*ppszLocation = nullptr;
	CComPtr<IShellView> spsv;

	hr = psb->QueryActiveShellView(&spsv);
	if (FAILED(hr)) return hr;

	CComQIPtr<IPersistIDList> sppidl(spsv);
	if (!sppidl) return E_FAIL;

	CComHeapPtr<ITEMIDLIST_ABSOLUTE> spidl;
	hr = sppidl->GetIDList(&spidl);

	if (FAILED(hr)) return hr;

	CComPtr<IShellItem> spsi;
	hr = SHCreateItemFromIDList(spidl, IID_PPV_ARGS(&spsi));

	if (FAILED(hr)) return hr;

	hr = spsi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, ppszLocation);

	return hr;
}


void ProcessOneWindow(IUnknown* punk)
{
	CComPtr<IShellBrowser> spsb;
	if (FAILED(IUnknown_QueryService(punk, SID_STopLevelBrowser,
		IID_PPV_ARGS(&spsb)))) return;

	CComPtr<IShellView> spsv;
	if (FAILED(spsb->QueryActiveShellView(&spsv))) return;

	CComQIPtr<IFolderView2> spfv(spsv);
	if (!spfv) return;

	CComHeapPtr<WCHAR> spszLocation;
	if (FAILED(GetLocationFromView(spsb, &spszLocation))) return;

	printf("Location = %ls\n", static_cast<PCWSTR>(spszLocation));

	int cColumns;
	if (FAILED(spfv->GetSortColumnCount(&cColumns))) return;
	if (cColumns > 10) cColumns = 10;

	SORTCOLUMN rgColumns[10]; // arbitrary number
	spfv->GetSortColumns(rgColumns, cColumns);

	for (int i = 0; i < cColumns; i++) {
		PCWSTR pszDir = rgColumns[0].direction > 0 ? L"ascending"
			: L"descending";
		PCWSTR pszName;
		CComHeapPtr<WCHAR> spszName;
		WCHAR szName[PKEYSTR_MAX];
		if (SUCCEEDED(PSGetNameFromPropertyKey(rgColumns[0].propkey,
			&spszName))) {
			pszName = spszName;
		}
		else {
			PSStringFromPropertyKey(rgColumns[0].propkey,
				szName, ARRAYSIZE(szName));
			pszName = szName;
		}
		printf("Column = %ls, direction = %ls\n", pszName, pszDir);
	}
}

void ProcessOneWindow2(IUnknown* punk, const WCHAR *target, WCHAR **store, int len, int *ascend)
{
	CComPtr<IShellBrowser> spsb;
	if (FAILED(IUnknown_QueryService(punk, SID_STopLevelBrowser,
		IID_PPV_ARGS(&spsb)))) return;

	CComPtr<IShellView> spsv;
	if (FAILED(spsb->QueryActiveShellView(&spsv))) return;

	CComQIPtr<IFolderView2> spfv(spsv);
	if (!spfv) return;

	CComHeapPtr<WCHAR> spszLocation;
	if (FAILED(GetLocationFromView(spsb, &spszLocation))) return;

	if (StrCmpW(spszLocation, target) != 0)
		return;

	int cColumns;
	if (FAILED(spfv->GetSortColumnCount(&cColumns))) return;
	if (cColumns > 10) cColumns = 10;

	SORTCOLUMN rgColumns[10]; // arbitrary number
	spfv->GetSortColumns(rgColumns, cColumns);

	for (int i = 0; i < cColumns; i++) 
	{
//		PCWSTR pszDir =
		*ascend = rgColumns[0].direction > 0 ? 1 : 0; //L"ascending" : L"descending";

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
		//printf("Column = %ls, direction = %ls\n", pszName, pszDir);
	}
}

int start2(const wchar_t *target, wchar_t **store, int len, int*ascend)
{
	CCoInitialize init;
	CComPtr<IShellWindows> spShellWindows;
	spShellWindows.CoCreateInstance(CLSID_ShellWindows);

	CComPtr<IUnknown> spunkEnum;
	spShellWindows->_NewEnum(&spunkEnum);
	CComQIPtr<IEnumVARIANT> spev(spunkEnum);

	for (CComVariant svar; spev->Next(1, &svar, nullptr) == S_OK; svar.Clear())
	{
		ProcessOneWindow2(svar.pdispVal, target, store, len, ascend);
	}

	return 0;
}



int findit(const wchar_t* path, wchar_t** str, int len, int* ascend)
{
	return start2(path, str, len, ascend);
}


/*
int test1()
{
	IServiceProvider* psp;
	IShellBrowser* psb;

	//CComQIPtr<IServiceProvider> psp(m_pWebBrowser2);
	//CComPtr<IShellBrowser>	psb;
	if (psp)
		psp->QueryService(SID_STopLevelBrowser, IID_IShellBrowser, (LPVOID*)& psb);
	if (psb)
	{
		IShellView* psv;
		if (SUCCEEDED(psb->QueryActiveShellView(&psv)))
		{
			IFolderView2* pfv;
			if (SUCCEEDED(psv->QueryInterface(IID_IFolderView2, (void**)& pfv)))
			{
				int pcColumns;
				HRESULT res = pfv->GetSortColumnCount(&pcColumns);

				if (SUCCEEDED(res) && pcColumns > 0)
				{
					SORTCOLUMN* sc = (SORTCOLUMN*)malloc(sizeof(SORTCOLUMN) * pcColumns);
					res = pfv->GetSortColumns(sc, pcColumns);
					if (SUCCEEDED(res))
					{
						SORTDIRECTION dir = sc->direction;
						PROPERTYKEY key = sc->propkey;
					}
					free(sc);
				}

				pfv->Release();
			}
			psv->Release();
		}
	}


}
*/

