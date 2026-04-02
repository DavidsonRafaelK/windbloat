# windebloat — Codebase Reference

## Purpose
PowerShell script that detects and removes Windows bloatware (AppX + Win32).
Also collects system hardware/OS info and can retrieve a Windows product key via MAS.

## Entry Point
`main.ps1` — monolithic script. `Main()` is called at the bottom.

## Execution Flow
1. `EnsureAdministrator` — checks privileges, re-launches elevated if needed
2. `$getArchitecture` scriptblock — collects OS/hardware info via CIM/WMI
3. `GetWindowsProductKey` — fetches activation script from MAS if no embedded key found
4. Iterates `bloatware.json` → `DetectBloatware` → `RemoveBloatware` (3 retries)

## bloatware.json Schema
Each entry:
```json
{
  "name":        "Display Name",
  "type":        "appx" | "win32",
  "app_id":      "AppX.Package.Name" | null,
  "description": "Why it's bloatware",
  "removable":   true | false,
  "notes":       "Any caveats",
  "win32": {
    "processes":        ["proc.exe"],
    "services":         ["svcName"],
    "install_dirs":     ["%ProgramFiles%\\Vendor\\App"],
    "registry_keys":    ["HKLM\\SOFTWARE\\Vendor\\App"],
    "uninstall_string": "path\\to\\uninstaller.exe" | null
  } | null
}
```

## C++ Helper (helper/)
`debloat-helper.exe` — handles Win32 removal that PowerShell can't do cleanly.

| Module | File | API used |
|--------|------|----------|
| Process kill | process.cpp | `CreateToolhelp32Snapshot`, `TerminateProcess` |
| Service control | service.cpp | `OpenSCManager`, `ControlService`, `DeleteService` |
| Force file delete | filesystem.cpp | `SetNamedSecurityInfo`, `DeleteFile`, `MoveFileEx` |
| Registry cleanup | registry.cpp | `RegOpenKeyExW`, `RegDeleteKeyExW` |
| Shared utils | win32_utils.h | RAII handles, `LogWin32Error` via `FormatMessageW` |

CLI interface:
```
debloat-helper.exe --kill-process    <process.exe>
debloat-helper.exe --stop-service    <service-name>
debloat-helper.exe --delete-service  <service-name>
debloat-helper.exe --force-delete    <path>
debloat-helper.exe --delete-registry <HKLM\...\Key>
```

Build: `cmake` in `helper/`, outputs `helper/build/debloat-helper.exe`.

## Current Branch State (feat/kernel)
- `bloatware.json` — expanded with `type` + `win32` blocks; 6 Win32 targets added
- `helper/` — C++ helper fully written, not yet wired into `main.ps1`
- `main.ps1` — still only handles AppX; Win32 routing not implemented yet

## Pending Work
- [ ] Update `main.ps1` to call `debloat-helper.exe` for `"type": "win32"` entries
- [ ] Add `--run-uninstall` command to helper for entries with `uninstall_string`
- [ ] Fix duplicate `FindFirstFile` call in `filesystem.cpp` (`DeleteDirectoryRecursive`)
- [ ] Build + smoke-test helper against a real target (VM recommended)
- [ ] Merge `feat/kernel` → `dev` after integration is tested

## Branching Convention
- `main` — production
- `dev` — integration
- `feat/<feature>` — feature branches
- `hotfix/<fix>` — hotfix branches
