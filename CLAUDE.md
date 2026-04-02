# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**windebloat** is a PowerShell script that detects and removes pre-installed Windows bloatware (AppX packages). It also gathers system hardware/OS information and can retrieve a Windows product key via [MAS](https://get.activated.win).

## Running the Script

The script must be run as Administrator. It auto-elevates if not already privileged:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File main.ps1
```

There is no build, lint, or test pipeline.

## Architecture

All logic lives in `main.ps1` (monolithic script). Execution flow:

1. `Main()` is called at the bottom of the file.
2. `EnsureAdministrator` checks privileges and re-launches elevated if needed.
3. `$getArchitecture` scriptblock collects OS/hardware info via CIM/WMI.
4. If no embedded product key is found, `GetWindowsProductKey` fetches and executes a remote activation script via `Invoke-Expression`.
5. Each entry in `bloatware.json` is passed through `DetectBloatware` → `RemoveBloatware` (up to 3 retries).

## Adding Bloatware Targets

Edit `bloatware.json`. Each entry requires:

```json
{
  "name": "Display Name",
  "app_id": "AppX.Package.Name",
  "description": "Why it's bloatware",
  "removable": true,
  "notes": "Any caveats"
}
```

The `app_id` must match the exact package name returned by `Get-AppxPackage`.

## Code Style

Enforced via `.vscode/settings.json`:
- **Formatter:** PowerShell (PSScriptAnalyzer, Stroustrup brace style)
- **Indent:** 8 spaces (tabs rendered as 8)
- **Line endings:** CRLF
- **Format on save:** enabled

## Branching Convention

- `main` — production
- `dev` — integration
- `feat/<feature>` — feature branches
- `hotfix/<fix>` — hotfix branches

Commit messages follow Conventional Commits (`feat:`, `fix:`, `del:`, etc.).
