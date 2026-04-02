#include "service.h"
#include "win32_utils.h"
#include <windows.h>

static constexpr DWORD STOP_TIMEOUT_MS = 10000;

bool StopService(const std::wstring& serviceName)
{
        ScopedScHandle scm(OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT));
        if (!scm.valid()) {
                LogWin32Error(L"OpenSCManagerW");
                return false;
        }

        ScopedScHandle svc(OpenServiceW(scm, serviceName.c_str(),
                SERVICE_STOP | SERVICE_QUERY_STATUS));
        if (!svc.valid()) {
                // Not found means it's already gone — treat as success
                if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
                        return true;
                LogWin32Error((L"OpenServiceW: " + serviceName).c_str());
                return false;
        }

        SERVICE_STATUS_PROCESS ssp{};
        DWORD needed = 0;
        QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO,
                reinterpret_cast<LPBYTE>(&ssp), sizeof(ssp), &needed);

        if (ssp.dwCurrentState != SERVICE_STOPPED) {
                SERVICE_STATUS ss{};
                if (!ControlService(svc, SERVICE_CONTROL_STOP, &ss))
                        LogWin32Error((L"ControlService STOP: " + serviceName).c_str());

                // Poll until stopped or timeout — SCM stop is asynchronous
                DWORD elapsed = 0;
                while (elapsed < STOP_TIMEOUT_MS) {
                        QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO,
                                reinterpret_cast<LPBYTE>(&ssp), sizeof(ssp), &needed);
                        if (ssp.dwCurrentState == SERVICE_STOPPED)
                                break;
                        Sleep(500);
                        elapsed += 500;
                }

                if (ssp.dwCurrentState != SERVICE_STOPPED)
                        LogWin32Error((L"Service stop timed out: " + serviceName).c_str(), ERROR_TIMEOUT);
        }

        return ssp.dwCurrentState == SERVICE_STOPPED;
}

bool DeleteService(const std::wstring& serviceName)
{
        ScopedScHandle scm(OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT));
        if (!scm.valid()) {
                LogWin32Error(L"OpenSCManagerW");
                return false;
        }

        ScopedScHandle svc(OpenServiceW(scm, serviceName.c_str(), DELETE));
        if (!svc.valid()) {
                if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
                        return true;
                LogWin32Error((L"OpenServiceW: " + serviceName).c_str());
                return false;
        }

        if (!::DeleteService(svc)) {
                LogWin32Error((L"DeleteService: " + serviceName).c_str());
                return false;
        }

        return true;
}
