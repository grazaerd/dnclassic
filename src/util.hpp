#ifndef UTIL_HPP
#define UTIL_HPP

#include <io.h>
#include <cstdint>
#include <synchapi.h>
#include <handleapi.h>
#include <windows.h>

#if defined(__GNUC__) 
#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#else 
#define likely(x)   (x)
#define unlikely(x) (x)
#endif

#define INF 0xFFFFFFFF
namespace util {
    
inline bool FilePermission(const char* path, std::uint8_t mode) {
    if (_access_s(path, mode) == 0) {
        return true;
    } else {
        return false;
    }
    return false;
}

inline void HighResSleep(double milliseconds) {
    HANDLE timer = CreateWaitableTimerExW(
        NULL, NULL,
        CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
        TIMER_ALL_ACCESS
    );

    if (!timer) {
        Sleep(milliseconds);
        return;
    };

    LARGE_INTEGER due_time;
    due_time.QuadPart = -static_cast<LONGLONG>(milliseconds * 10000.0);

    if (!SetWaitableTimer(timer, &due_time, 0, NULL, NULL, FALSE)) {
        CloseHandle(timer);
        return;
    }

    WaitForSingleObject(timer, INF);
    CloseHandle(timer);
}

}

#endif