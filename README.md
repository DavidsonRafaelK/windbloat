# windebloat

A PowerShell debloater for Windows that removes pre-installed AppX (UWP) and Win32 bloatware, collects system information, and retrieves your Windows product key.

## Features

- **Remove AppX Bloatware** – Uninstall pre-installed UWP apps
- **Remove Win32 Bloatware** – Remove traditional Windows applications via registry, services, and filesystem operations
- **System Info** – Collect OS version, hardware details via WMI/CIM
- **Windows Product Key** – Retrieve your Windows license key via [MAS](https://get.activated.win)
- **Auto-Elevate** – Automatically runs with Administrator privileges if needed

## Requirements

- Windows 10 or later
- Administrator privileges (script auto-elevates)
- PowerShell 5.1 or later

## Usage

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File main.ps1
```

## Building the C++ Helper

The project includes a C++ helper binary for low-level Win32 operations:

```bash
cmake -S helper -B helper/build
cmake --build helper/build --config Release
```

**Requirements:** CMake and a C++17-capable compiler (MSVC recommended)

**Output:** `helper/build/debloat-helper.exe`

## Project Structure

- `main.ps1` – Main orchestrator script
- `tools/` – Individual tool scripts for each operation
- `bloatware.json` – Bloatware definitions and removal targets
- `helper/` – C++ helper source for Win32 operations

## License

MIT
