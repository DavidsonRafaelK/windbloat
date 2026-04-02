#pragma once
#include <windows.h>
#include <iostream>
#include <string>

inline void LogWin32Error(const wchar_t* context, DWORD code = ::GetLastError())
{
        wchar_t* msg = nullptr;
        FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM     |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, code,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPWSTR>(&msg), 0, nullptr);

        std::wcerr << L"[ERROR] " << context << L": "
                   << (msg ? msg : L"Unknown error")
                   << L" (code " << code << L")\n";

        // FORMAT_MESSAGE_ALLOCATE_BUFFER makes FormatMessage allocate msg via LocalAlloc
        if (msg) LocalFree(msg);
}

struct ScopedHandle {
        HANDLE h;
        explicit ScopedHandle(HANDLE h) : h(h) {}
        ~ScopedHandle() { if (h && h != INVALID_HANDLE_VALUE) CloseHandle(h); }
        operator HANDLE() const { return h; }
        bool valid() const { return h && h != INVALID_HANDLE_VALUE; }
        ScopedHandle(const ScopedHandle&)            = delete;
        ScopedHandle& operator=(const ScopedHandle&) = delete;
};

struct ScopedScHandle {
        SC_HANDLE h;
        explicit ScopedScHandle(SC_HANDLE h) : h(h) {}
        ~ScopedScHandle() { if (h) CloseServiceHandle(h); }
        operator SC_HANDLE() const { return h; }
        bool valid() const { return h != nullptr; }
        ScopedScHandle(const ScopedScHandle&)            = delete;
        ScopedScHandle& operator=(const ScopedScHandle&) = delete;
};

// Do NOT use for predefined keys (HKEY_LOCAL_MACHINE etc.) — only for keys
// opened with RegOpenKeyEx, which must be explicitly closed.
struct ScopedRegKey {
        HKEY h;
        explicit ScopedRegKey(HKEY h = nullptr) : h(h) {}
        ~ScopedRegKey() { if (h) RegCloseKey(h); }
        operator HKEY() const { return h; }
        bool valid() const { return h != nullptr; }
        ScopedRegKey(const ScopedRegKey&)            = delete;
        ScopedRegKey& operator=(const ScopedRegKey&) = delete;
};

struct ScopedFindHandle {
        HANDLE h;
        explicit ScopedFindHandle(HANDLE h) : h(h) {}
        ~ScopedFindHandle() { if (h && h != INVALID_HANDLE_VALUE) FindClose(h); }
        operator HANDLE() const { return h; }
        bool valid() const { return h && h != INVALID_HANDLE_VALUE; }
        ScopedFindHandle(const ScopedFindHandle&)            = delete;
        ScopedFindHandle& operator=(const ScopedFindHandle&) = delete;
};
