#include <windows.h>
#include <iostream>
#include <string>
#include <algorithm>

#include "process.h"
#include "service.h"
#include "filesystem.h"
#include "registry.h"
#include "win32_utils.h"

static void PrintUsage()
{
        std::wcout
                << L"debloat-helper.exe <command> <argument>\n\n"
                << L"Commands:\n"
                << L"  --kill-process    <process.exe>           Kill all processes matching name\n"
                << L"  --stop-service    <service-name>          Stop a Windows service\n"
                << L"  --delete-service  <service-name>          Delete a Windows service\n"
                << L"  --force-delete    <path>                  Force-delete file or directory\n"
                << L"  --delete-registry <HKLM\\\\...\\\\Key>    Recursively delete registry key\n"
                << L"\nExit codes: 0 = success, 1 = failure, 2 = bad usage\n";
}

static std::wstring ExpandEnvVars(const std::wstring& input)
{
        wchar_t buf[MAX_PATH]{};
        ExpandEnvironmentStringsW(input.c_str(), buf, MAX_PATH);
        return buf;
}

int wmain(int argc, wchar_t* argv[])
{
        // Last-resort guard: catch any unhandled C++ exceptions (e.g. std::bad_alloc)
        // that slip through the Win32 error-code checks in each module
        try {
                if (argc < 3) {
                        PrintUsage();
                        return 2;
                }

                std::wstring cmd = argv[1];
                std::wstring arg = ExpandEnvVars(argv[2]);

                std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::towlower);

                if (cmd == L"--kill-process") {
                        int n = KillProcessByName(arg);
                        std::wcout << L"Killed " << n << L" process(es): " << arg << L"\n";
                        return 0;
                }

                if (cmd == L"--stop-service") {
                        bool ok = StopService(arg);
                        std::wcout << (ok ? L"Stopped" : L"Failed to stop") << L" service: " << arg << L"\n";
                        return ok ? 0 : 1;
                }

                if (cmd == L"--delete-service") {
                        bool ok = DeleteService(arg);
                        std::wcout << (ok ? L"Deleted" : L"Failed to delete") << L" service: " << arg << L"\n";
                        return ok ? 0 : 1;
                }

                if (cmd == L"--force-delete") {
                        bool ok = ForceDeletePath(arg);
                        std::wcout << (ok ? L"Deleted" : L"Scheduled for reboot-delete") << L": " << arg << L"\n";
                        return 0;
                }

                if (cmd == L"--delete-registry") {
                        bool ok = DeleteRegistryKey(arg);
                        std::wcout << (ok ? L"Deleted" : L"Failed to delete") << L" registry key: " << arg << L"\n";
                        return ok ? 0 : 1;
                }

                PrintUsage();
                return 2;
        }
        catch (const std::exception& ex) {
                std::wcerr << L"[FATAL] Unhandled exception: "
                           << ex.what() << L"\n";
                return 1;
        }
        catch (...) {
                std::wcerr << L"[FATAL] Unknown unhandled exception\n";
                return 1;
        }
}
