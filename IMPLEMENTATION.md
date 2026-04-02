Started from a PowerShell-only AppX debloater and expanded it toward a proper Win32 removal pipeline:

1. Planned the architecture — decided a C++ helper EXE calling Win32 APIs was the right approach for stubborn software (services, locked files, protected dirs), not a kernel driver
2. Expanded bloatware.json — added type field (appx/win32), win32 block with processes/services/install_dirs/registry_keys, and 6 new Win32 targets (Vanguard, McAfee, Norton, HP/Dell tools,
WildTangent)
3. Built the C++ helper skeleton — debloat-helper.exe with 4 modules: process kill, service stop/delete, force filesystem delete, recursive registry cleanup
4. Added RAII + error logging — win32_utils.h with scoped handle wrappers, LogWin32Error using FormatMessageW for readable output
---
What's Next
Immediate (to make it functional):
- Update main.ps1 to read the type field and route win32 entries to debloat-helper.exe — without this, the helper exists but nothing calls it
- Build and smoke-test the C++ helper (cmake + MSVC/MinGW)
Then:
- Fix the duplicate ScopedFindHandle in filesystem.cpp (two FindFirstFile calls were left by mistake)
- Add a --run-uninstall command to the helper for entries that have an uninstall_string
- Test against a real target (Vanguard or WildTangent on a VM is safest)
Longer term:
- Wire the whole feat/kernel branch into dev once main.ps1 integration is done and tested

claude --resume 29859e58-97f8-40d3-ac42-9f6499577349