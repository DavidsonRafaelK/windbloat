#include "registry.h"
#include "win32_utils.h"
#include <windows.h>
#include <string>

static HKEY ResolveRoot(const std::wstring& root)
{
        if (root == L"HKLM" || root == L"HKEY_LOCAL_MACHINE")  return HKEY_LOCAL_MACHINE;
        if (root == L"HKCU" || root == L"HKEY_CURRENT_USER")   return HKEY_CURRENT_USER;
        if (root == L"HKCR" || root == L"HKEY_CLASSES_ROOT")   return HKEY_CLASSES_ROOT;
        if (root == L"HKU"  || root == L"HKEY_USERS")          return HKEY_USERS;
        if (root == L"HKCC" || root == L"HKEY_CURRENT_CONFIG") return HKEY_CURRENT_CONFIG;
        return nullptr;
}

static bool DeleteKeyRecursive(HKEY hRoot, const std::wstring& subKey)
{
        HKEY raw = nullptr;
        LONG res = RegOpenKeyExW(hRoot, subKey.c_str(), 0,
                KEY_READ | KEY_WRITE | DELETE, &raw);

        if (res == ERROR_FILE_NOT_FOUND)
                return true;
        if (res != ERROR_SUCCESS) {
                LogWin32Error((L"RegOpenKeyExW: " + subKey).c_str(), res);
                return false;
        }

        ScopedRegKey hKey(raw);

        // Always enumerate index 0: each successful recursive delete shifts
        // the remaining child keys down, so index 0 is always the next unvisited key
        while (true) {
                wchar_t childName[256]{};
                DWORD childLen = 256;
                LONG enumRes = RegEnumKeyExW(hKey, 0, childName, &childLen,
                        nullptr, nullptr, nullptr, nullptr);
                if (enumRes != ERROR_SUCCESS)
                        break;

                DeleteKeyRecursive(hRoot, subKey + L"\\" + childName);
        }

        // ScopedRegKey closes hKey here before we attempt to delete the key itself
        // (an open handle on a key blocks its deletion)
        raw = hKey.h;
        hKey.h = nullptr;
        RegCloseKey(raw);

        // Try both 64-bit and 32-bit registry views to handle WOW64 redirection
        res = RegDeleteKeyExW(hRoot, subKey.c_str(), KEY_WOW64_64KEY, 0);
        if (res != ERROR_SUCCESS && res != ERROR_FILE_NOT_FOUND)
                res = RegDeleteKeyExW(hRoot, subKey.c_str(), KEY_WOW64_32KEY, 0);

        if (res != ERROR_SUCCESS && res != ERROR_FILE_NOT_FOUND) {
                LogWin32Error((L"RegDeleteKeyExW: " + subKey).c_str(), res);
                return false;
        }

        return true;
}

bool DeleteRegistryKey(const std::wstring& keyPath)
{
        auto sep = keyPath.find(L'\\');
        if (sep == std::wstring::npos) {
                LogWin32Error(L"DeleteRegistryKey: invalid key path (no root separator)", ERROR_INVALID_PARAMETER);
                return false;
        }

        HKEY hRoot = ResolveRoot(keyPath.substr(0, sep));
        if (!hRoot) {
                LogWin32Error(L"DeleteRegistryKey: unrecognised root hive", ERROR_INVALID_PARAMETER);
                return false;
        }

        return DeleteKeyRecursive(hRoot, keyPath.substr(sep + 1));
}
