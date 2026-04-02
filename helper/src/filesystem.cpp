#include "filesystem.h"
#include "win32_utils.h"
#include <windows.h>
#include <aclapi.h>

static bool EnableOwnershipPrivileges()
{
        ScopedHandle token([]() {
                HANDLE h = nullptr;
                OpenProcessToken(GetCurrentProcess(),
                        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &h);
                return h;
        }());

        if (!token.valid()) {
                LogWin32Error(L"OpenProcessToken");
                return false;
        }

        // SE_RESTORE_NAME is required alongside SE_TAKE_OWNERSHIP_NAME to allow
        // setting an owner SID that the current user doesn't belong to (e.g. Administrators)
        const wchar_t* privs[] = { SE_TAKE_OWNERSHIP_NAME, SE_RESTORE_NAME };
        for (auto& priv : privs) {
                TOKEN_PRIVILEGES tp{};
                tp.PrivilegeCount = 1;
                if (!LookupPrivilegeValueW(nullptr, priv, &tp.Privileges[0].Luid)) {
                        LogWin32Error((std::wstring(L"LookupPrivilegeValue: ") + priv).c_str());
                        continue;
                }
                tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), nullptr, nullptr);
        }

        return true;
}

static void TakeOwnershipAndGrantAccess(const std::wstring& path)
{
        PSID adminSid = nullptr;
        SID_IDENTIFIER_AUTHORITY ntAuth = SECURITY_NT_AUTHORITY;
        if (!AllocateAndInitializeSid(&ntAuth, 2,
                SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0, &adminSid)) {
                LogWin32Error((L"AllocateAndInitializeSid: " + path).c_str());
                return;
        }

        // Ownership must be set before the DACL, you can't change the DACL
        // on a file you don't own, even as an administrator
        DWORD res = SetNamedSecurityInfoW(
                const_cast<LPWSTR>(path.c_str()),
                SE_FILE_OBJECT,
                OWNER_SECURITY_INFORMATION,
                adminSid, nullptr, nullptr, nullptr);
        if (res != ERROR_SUCCESS)
                LogWin32Error((L"SetNamedSecurityInfo (owner): " + path).c_str(), res);

        EXPLICIT_ACCESS_W ea{};
        ea.grfAccessPermissions = GENERIC_ALL;
        ea.grfAccessMode        = SET_ACCESS;
        ea.grfInheritance       = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea.Trustee.TrusteeForm  = TRUSTEE_IS_SID;
        ea.Trustee.TrusteeType  = TRUSTEE_IS_GROUP;
        ea.Trustee.ptstrName    = reinterpret_cast<LPWSTR>(adminSid);

        PACL newDacl = nullptr;
        SetEntriesInAclW(1, &ea, nullptr, &newDacl);

        // PROTECTED_DACL_SECURITY_INFORMATION prevents the parent directory from
        // re-applying its inherited ACEs and overriding our new DACL
        res = SetNamedSecurityInfoW(
                const_cast<LPWSTR>(path.c_str()),
                SE_FILE_OBJECT,
                DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
                nullptr, nullptr, newDacl, nullptr);
        if (res != ERROR_SUCCESS)
                LogWin32Error((L"SetNamedSecurityInfo (DACL): " + path).c_str(), res);

        if (newDacl)  LocalFree(newDacl);
        if (adminSid) FreeSid(adminSid);
}

static bool DeleteDirectoryRecursive(const std::wstring& dir)
{
        ScopedFindHandle hFind(FindFirstFileW((dir + L"\\*").c_str(), nullptr));

        WIN32_FIND_DATAW fd{};
        ScopedFindHandle hFindReal(FindFirstFileW((dir + L"\\*").c_str(), &fd));
        if (hFindReal.valid()) {
                do {
                        if (wcscmp(fd.cFileName, L".") == 0 ||
                            wcscmp(fd.cFileName, L"..") == 0)
                                continue;

                        std::wstring child = dir + L"\\" + fd.cFileName;
                        TakeOwnershipAndGrantAccess(child);

                        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                                DeleteDirectoryRecursive(child);
                                if (!RemoveDirectoryW(child.c_str()))
                                        MoveFileExW(child.c_str(), nullptr, MOVEFILE_DELAY_UNTIL_REBOOT);
                        } else {
                                SetFileAttributesW(child.c_str(), FILE_ATTRIBUTE_NORMAL);
                                if (!DeleteFileW(child.c_str())) {
                                        // File is locked by a running process, mark for deletion on next boot
                                        if (!MoveFileExW(child.c_str(), nullptr, MOVEFILE_DELAY_UNTIL_REBOOT))
                                                LogWin32Error((L"MoveFileEx (reboot-delete): " + child).c_str());
                                }
                        }
                } while (FindNextFileW(hFindReal, &fd));
        }

        if (!RemoveDirectoryW(dir.c_str())) {
                // Directory may still have reboot-pending children; schedule it too
                MoveFileExW(dir.c_str(), nullptr, MOVEFILE_DELAY_UNTIL_REBOOT);
                return false;
        }
        return true;
}

bool ForceDeletePath(const std::wstring& path)
{
        EnableOwnershipPrivileges();
        TakeOwnershipAndGrantAccess(path);

        DWORD attrs = GetFileAttributesW(path.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES)
                return true;

        if (attrs & FILE_ATTRIBUTE_DIRECTORY)
                return DeleteDirectoryRecursive(path);

        SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
        if (DeleteFileW(path.c_str()))
                return true;

        // nullptr as destination tells MoveFileEx to delete the file on next reboot
        if (!MoveFileExW(path.c_str(), nullptr, MOVEFILE_DELAY_UNTIL_REBOOT)) {
                LogWin32Error((L"MoveFileEx (reboot-delete): " + path).c_str());
                return false;
        }
        return false;
}
