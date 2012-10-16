//-----------------------------------------------------------------------------
// File: main.cpp
//
// Desc: Contains global data and DllMain.
//
// Copyright (C) 1999-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#include "common.hpp"


HMODULE g_hModule = NULL;
long g_cComponents = 0;
long g_cServerLocks = 0;

//exported fns

//can unload?
STDAPI DllCanUnloadNow()
{
	if ((g_cComponents == 0)	 && (g_cServerLocks == 0))
	{
		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}


//get class factory
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{

	HRESULT hr = S_OK;

	//check which class it is
	if ((rclsid != CLSID_CDirectInputActionFramework)
	    && (rclsid != CLSID_CDIDeviceActionConfigPage)
	   )
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	//create the appropriate class factory
	IClassFactory* pFact = NULL;

	if (rclsid == CLSID_CDirectInputActionFramework)
		pFact = new CFactory();

	if (rclsid == CLSID_CDIDeviceActionConfigPage)
		pFact = new CPageFactory();


	if (pFact == NULL)
		return E_OUTOFMEMORY;

	hr = pFact->QueryInterface(riid, ppv);
	pFact->Release();

	return hr;
}


//dll module information
BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch(dwReason)
	{
		case DLL_PROCESS_ATTACH:
			g_hModule = (HMODULE)hModule;
			CFlexWnd::RegisterWndClass((HINSTANCE)hModule);

			break;

		case DLL_PROCESS_DETACH:
			CFlexWnd::UnregisterWndClass((HINSTANCE)hModule);
			break;
	}

	return TRUE;
}


//server registration
STDAPI DllRegisterServer()
{
	HRESULT hr1 = S_OK, hr2 = S_OK, hr3 = S_OK;

	hr1 = RegisterServer(g_hModule, CLSID_CDirectInputActionFramework, _T("CLSID_CDirectInputActionFramework"), _T("DIACTFRM"), _T("DIACTFRM.1"));
	hr2 = RegisterServer(g_hModule, CLSID_CDIDeviceActionConfigPage, _T("CLSID_CDIDeviceActionConfigPage"), _T("DIACTFRM"), _T("DIACTFRM.1"));

	if (FAILED(hr1))
		return hr1;

	if (FAILED(hr2))
		return hr2;

	if (FAILED(hr3))
		return hr3;

	return S_OK;
}

//server unregistration
STDAPI DllUnregisterServer()
{
	HRESULT hr1 = S_OK, hr2 = S_OK, hr3 = S_OK;

	hr1 = UnregisterServer(CLSID_CDirectInputActionFramework, _T("DIACTFRM"), _T("DIACTFRM.1"));
	hr2 = UnregisterServer(CLSID_CDIDeviceActionConfigPage, _T("DIACTFRM"), _T("DIACTFRM.1"));

	if (FAILED(hr1))
		return hr1;

	if (FAILED(hr2))
		return hr2;

	if (FAILED(hr3))
		return hr3;

	return S_OK;
}
