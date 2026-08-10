#include <cstdlib>
#include <stdio.h>
#include <unknwn.h>
#include <bit>
#include <lm.h>
#include <stdio.h>
#include "proxy.hpp"

#define _WSTDIO_S_DEFINED
#define PLUGIN_API extern "C" __declspec(dllexport)

using DirectDrawCreateEx_ptr = HRESULT(WINAPI*)(GUID* lpGuid, LPVOID* lplpDD, REFIID iid, IUnknown* pUnkOuter);
DirectDrawCreateEx_ptr DirectDrawCreateEx_orig = nullptr;

PLUGIN_API HRESULT WINAPI DirectDrawCreateEx(GUID* lpGuid, LPVOID* lplpDD, REFIID iid, IUnknown* pUnkOuter) {
    return DirectDrawCreateEx_orig(lpGuid, lplpDD, iid, pUnkOuter);
}

HMODULE origModule = nullptr;

bool Proxy_Attach()
{
	extern HMODULE ourModule;

	WCHAR modulePath[MAX_PATH] = { 0 };
	if (!GetSystemDirectoryW(modulePath, _countof(modulePath))) {
		return false;
	}

	WCHAR ourModulePath[MAX_PATH] = { 0 };
	GetModuleFileNameW(ourModule, ourModulePath, _countof(ourModulePath));

	WCHAR exeName[MAX_PATH] = { 0 };
	WCHAR extName[MAX_PATH] = { 0 };
	_wsplitpath_s(ourModulePath, nullptr, 0, nullptr, 0, exeName, MAX_PATH, extName, MAX_PATH);

	swprintf_s(modulePath, MAX_PATH, L"%ws\\%ws%ws", modulePath, exeName, extName);

	origModule = LoadLibraryW(modulePath);
	if (!origModule) {
		return false;
	}
    DirectDrawCreateEx_orig = std::bit_cast<DirectDrawCreateEx_ptr>(GetProcAddress(origModule, "DirectDrawCreateEx"));
	return true;
}

void Proxy_Detach()
{
	if (!origModule) {
		return;
	}

	FreeLibrary(origModule);
	origModule = nullptr;
}