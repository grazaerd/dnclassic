#define WIN32_LEAN_AND_MEAN
// #include <windows.h>
#include <thread>
#include "proxy.hpp"
#include "mem.hpp"
#include <bit>
#include "dxport.hpp"
#include "util.hpp"

HMODULE ourModule = nullptr;

void MainThread() {
	util::HighResSleep(5000); // this is needed for classic
	DXPORT::Initialize();
} 

BOOL DllMain(HMODULE hMod, DWORD dwReason, [[maybe_unused]] LPVOID lpReserved) {
	switch (dwReason) {
		case DLL_PROCESS_ATTACH:
    		K32GetModuleInformation(Memory::hProcess, std::bit_cast<HMODULE>(Memory::moduleBase), &Memory::MODINF, sizeof(MODULEINFO));
			ourModule = hMod;
            Proxy_Attach();
			DisableThreadLibraryCalls(hMod);
			std::thread(MainThread).detach();
			break;
		case DLL_PROCESS_DETACH:
            Proxy_Detach();
			break;
	}
	return TRUE;
}