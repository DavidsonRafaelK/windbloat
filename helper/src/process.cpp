#include "process.h"
#include "win32_utils.h"
#include <windows.h>
#include <tlhelp32.h>
#include <algorithm>

int KillProcessByName(const std::wstring& processName)
{
        int killed = 0;

        ScopedHandle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
        if (!snapshot.valid()) {
                LogWin32Error(L"CreateToolhelp32Snapshot");
                return 0;
        }

        // dwSize must be set before the first Process32First call — Win32 requirement
        PROCESSENTRY32W entry{};
        entry.dwSize = sizeof(entry);

        if (!Process32FirstW(snapshot, &entry)) {
                LogWin32Error(L"Process32FirstW");
                return 0;
        }

        auto toLower = [](std::wstring s) {
                std::transform(s.begin(), s.end(), s.begin(), ::towlower);
                return s;
        };

        do {
                if (toLower(entry.szExeFile) != toLower(processName))
                        continue;

                ScopedHandle hProc(OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID));
                if (!hProc.valid()) {
                        LogWin32Error((L"OpenProcess for " + std::wstring(entry.szExeFile)).c_str());
                        continue;
                }

                if (TerminateProcess(hProc, 1))
                        ++killed;
                else
                        LogWin32Error((L"TerminateProcess for " + std::wstring(entry.szExeFile)).c_str());

        } while (Process32NextW(snapshot, &entry));

        return killed;
}
