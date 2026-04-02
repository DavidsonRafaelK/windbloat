<#
 * Author      : @DavidsonRafaelK
 * Description : Windows Debloat Script
 * Date        : 2026-04-01
 * Copyright   : (c) 2026 DavidsonRafaelK. All rights reserved.
#>

$getArchitecture = {
    $GetOperatingSystem = Get-CimInstance -Class Win32_OperatingSystem
    $GetOperatingSystemVersion = $GetOperatingSystem.Version
    $GetOperatingSystemCaption = $GetOperatingSystem.Caption
    $GetOperatingSystemProductKey = (Get-WmiObject -Class SoftwareLicensingService).OA3xOriginalProductKey
    $GetOperatingSystemArchitecture = $GetOperatingSystem.OSArchitecture
    $GetOperatingSystemBuildNumber = $GetOperatingSystem.BuildNumber
    $GetOperatingSystemSerialNumber = $GetOperatingSystem.SerialNumber

    $GetHardwareInfo = Get-CimInstance -Class Win32_ComputerSystem
    $GetHardwareManufacturer = $GetHardwareInfo.Manufacturer
    $GetHardwareModel = $GetHardwareInfo.Model
    $GetHardwareSerialNumber = (Get-CimInstance -Class Win32_BIOS).SerialNumber
    $GetHardwareUUID = (Get-CimInstance -Class Win32_ComputerSystemProduct).UUID
    $GetHardwareProcessor = (Get-CimInstance -Class Win32_Processor).Name
    $GetHardwareRAM = [Math]::Round((Get-CimInstance -Class Win32_PhysicalMemory | Measure-Object -Property Capacity -Sum).Sum / 1GB, 2)
    $GetHardwareDisk = Get-CimInstance -Class Win32_DiskDrive | Select-Object -Property Model, Size

    return [PSCustomObject]@{
        OSVersion = $GetOperatingSystemVersion
        OSCaption = $GetOperatingSystemCaption
        OSProductKey = $GetOperatingSystemProductKey
        OSArchitecture = $GetOperatingSystemArchitecture
        OSBuildNumber = $GetOperatingSystemBuildNumber
        OSSerialNumber = $GetOperatingSystemSerialNumber
        HardwareManufacturer = $GetHardwareManufacturer
        HardwareModel = $GetHardwareModel
        HardwareSerialNumber = $GetHardwareSerialNumber
        HardwareUUID = $GetHardwareUUID
        HardwareProcessor = $GetHardwareProcessor
        HardwareRAM = $GetHardwareRAM
        HardwareDisk = $GetHardwareDisk
    }
}

$BloatwareList = (Get-Content -Path "$PSScriptRoot\bloatware.json" -Raw | ConvertFrom-Json).bloatware

function InvokeHelper {
        param (
                [string] $Command,
                [string] $Argument
        )

        $HelperPath = Join-Path $PSScriptRoot "helper\build\debloat-helper.exe"

        if (-not (Test-Path $HelperPath)) {
                Write-Warning "debloat-helper.exe not found at: $HelperPath"
                return $false
        }

        try {
                $output = & $HelperPath $Command $Argument 2>&1
                Write-Host $output
                return $LASTEXITCODE -eq 0
        }
        catch {
                Write-Warning "InvokeHelper failed [$Command $Argument]: $_"
                return $false
        }
}

function RemoveBloatware {
    param (
        [string] $PackageName,
        [int]    $RetryCount = 1
    )

    Write-Host "Attempting to remove: $PackageName"
    
    for ($i = 0; $i -lt $RetryCount; $i++) {
        $package = Get-AppxPackage -Name $PackageName -ErrorAction SilentlyContinue

        if ($package) {
            Write-Host "Removing bloatware: $PackageName (Attempt $($i + 1))"
            $package | Remove-AppxPackage -ErrorAction SilentlyContinue
            break
        } else {
            Write-Host "Package not found: $PackageName"
            break
        }
    }
}

function RemoveWin32Bloatware {
        param (
                [PSCustomObject] $Win32Info
        )

        foreach ($process in $Win32Info.processes) {
                if (-not (InvokeHelper -Command "--kill-process" -Argument $process)) {
                        Write-Warning "Failed to kill process: $process"
                }
        }

        foreach ($service in $Win32Info.services) {
                if (-not (InvokeHelper -Command "--stop-service" -Argument $service)) {
                        Write-Warning "Failed to stop service: $service"
                }
                if (-not (InvokeHelper -Command "--delete-service" -Argument $service)) {
                        Write-Warning "Failed to delete service: $service"
                }
        }

        foreach ($path in $Win32Info.paths) {
                if (-not (InvokeHelper -Command "--force-delete" -Argument $path)) {
                        Write-Warning "Failed to delete path: $path"
                }
        }

        foreach ($key in $Win32Info.registry) {
                if (-not (InvokeHelper -Command "--delete-registry" -Argument $key)) {
                        Write-Warning "Failed to delete registry key: $key"
                }
        }
}

function DetectBloatware {
    param (
        [string] $PackageName
    )

    $package = Get-AppxPackage -Name $PackageName -ErrorAction SilentlyContinue
    
    if ($package) {
        Write-Host "Bloatware detected: $PackageName"
        return $true
    } else {
        Write-Host "Bloatware not detected: $PackageName"
        return $false
    }
}

function EnsureAdministrator {
    $IsAdmin = (
        [Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")
    
    if ($IsAdmin) {
        Write-Output "Running with administrator privileges."
        return $true
    } else {
        Write-Output "Relaunching script with administrator privileges..."
        Start-Process PowerShell -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`"" -Verb RunAs -Wait
        exit
    }
}

function GetWindowsProductKey {
    try {
        Invoke-RestMethod "https://get.activated.win" | Invoke-Expression
    }
    catch {
        Write-Error "Failed to retrieve product key from https://get.activated.win: $_"
    }
}

function Main {
    try {
        EnsureAdministrator

        $SystemInfo = $getArchitecture.Invoke()
        Write-Output "System Information:"
        Write-Output $SystemInfo

        if (-not [string]::IsNullOrEmpty($SystemInfo.OSProductKey)) {
            Write-Output "Windows Product Key: $($SystemInfo.OSProductKey)"
        } else {
            Write-Output "Windows Product Key not found. Attempting to retrieve..."
            Write-Host "Press any key to retrieve the product key..."
        
            $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
        
            $RetrievedKey = GetWindowsProductKey
            if ($null -eq $RetrievedKey) {
                Write-Warning "Could not retrieve product key from remote source."
            }
        }

        foreach ($bloatware in $BloatwareList) {
                if (DetectBloatware -PackageName $bloatware.app_id) {
                        RemoveBloatware -PackageName $bloatware.app_id -RetryCount 3
                }

                if ($bloatware.win32) {
                        RemoveWin32Bloatware -Win32Info $bloatware.win32
                }
        }
        
        Write-Host "`nDone! Press Enter to exit..."
        Read-Host
    }
    catch {
        Write-Error "Error in Main function: $_"
        Write-Host "`nError occurred. Press Enter to exit..."
        Read-Host
        return $false
    }
}

Main