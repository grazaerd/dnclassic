#ifndef MEM_HPP
#define MEM_HPP

#include <algorithm>
#include <bit>
#include <cstdint>
#include <charconv>
#include <cstring>
#include <format>
#include <processthreadsapi.h>
#include <Psapi.h>
#include <string_view>
#include <TlHelp32.h>
#include <immintrin.h>

#include <synchapi.h>
#include <windef.h>
#include <winbase.h>
#include <winuser.h>

namespace Memory {

    inline const uintptr_t moduleBase = std::bit_cast<uintptr_t>(GetModuleHandleA(nullptr));
    inline MODULEINFO MODINF;

    inline void* hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION, false, GetCurrentProcessId());
	
	template<typename T>
	inline void PatternScan(std::string_view sig, T& address) {
		const uint8_t* start = std::bit_cast<uint8_t*>(moduleBase);
		const std::size_t size = static_cast<std::size_t>(MODINF.SizeOfImage);

		constexpr auto hexToByte = [](char h, char l) noexcept -> uint8_t {
			auto hexVal = [](char c) noexcept -> uint8_t {
				return static_cast<uint8_t>(
					(c >= '0' && c <= '9') ? (c - '0') :
					(c >= 'A' && c <= 'F') ? (c - 'A' + 10) :
					(c >= 'a' && c <= 'f') ? (c - 'a' + 10) :
					0
				);
			};
			return static_cast<uint8_t>((hexVal(h) << 4) | hexVal(l));
		};

		const char* pat = sig.data();

		for (const uint8_t* p = start; p < start + size; ++p) {
			const uint8_t* mem = p;
			const char* s = pat;

			while (true) {
				if (*s == '\0') { address = reinterpret_cast<T>(const_cast<uint8_t*>(p)); return; }
				if (*s == ' ') { ++s; continue; }
				if (*s == '?') { ++mem; s += (*(s + 1) == '?') ? 2 : 1; continue; }

				if (*mem != hexToByte(s[0], s[1])) break;
				++mem; s += 2;
			}
		}

		if constexpr (std::is_pointer_v<T>) {
			address = nullptr;
		} else {
			address = 0;
		}
	}
	inline uint32_t FindRVA(const uintptr_t& module, const void* Address, const uint8_t MinLen, const uint8_t MaxLen) {
		// https://godbolt.org/z/hKrxf8aEE
		const uint32_t RVA = *std::bit_cast<uint32_t*>(std::bit_cast<uintptr_t>(Address) + MinLen);
		return static_cast<uint32_t>((RVA + static_cast<uint32_t>(std::bit_cast<uintptr_t>(Address) + MaxLen)) - module);
	}
	template <typename Type, typename Func>
	inline void IatMod(Type* address, Func fn) {
		DWORD oldProtect;
		VirtualProtectEx(hProcess, static_cast<void*>(address), sizeof(uint64_t), PAGE_EXECUTE_READWRITE, &oldProtect);
		*address = std::bit_cast<uint64_t>(fn);
		VirtualProtectEx(hProcess, static_cast<void*>(address), sizeof(uint64_t), oldProtect, &oldProtect);
	}
}

#endif