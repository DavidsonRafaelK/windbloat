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

$BloatwareList = (Get-Content -Path "./bloatware.json" -Raw | ConvertFrom-Json).bloatware

function RemoveBloatware {
    param (
        [string] $PackageName,
        [int]    $RetryCount = 1
    )

    Write-Output "Attempting to remove: $PackageName"
    
    for ($i = 0; $i -lt $RetryCount; $i++) {
        $package = Get-AppxPackage -Name $PackageName -ErrorAction SilentlyContinue

        if ($package) {
            Write-Output "Removing bloatware: $PackageName (Attempt $($i + 1))"
            $package | Remove-AppxPackage -ErrorAction SilentlyContinue
        } else {
            Write-Output "Package not found: $PackageName"
            break
        }
    }
}

function DetectBloatware {
    param (
        [string] $PackageName
    )

    $package = Get-AppxPackage -Name $PackageName -ErrorAction SilentlyContinue
    
    if ($package) {
        Write-Output "Bloatware detected: $PackageName"
        return $true
    } else {
        Write-Output "Bloatware not detected: $PackageName"
        return $false
    }
}

function Main {
    try {
        $IsAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")
        DetectAdministrator -IsAdministrator $IsAdmin

        if (-NOT $IsAdmin) {
            Write-Warning "Script requires Administrator privileges to remove packages."
            return
        }

        $SystemInfo = $getArchitecture.Invoke()
        Write-Output "System Information:"
        Write-Output $SystemInfo

        foreach ($bloatware in $BloatwareList) {
            if (DetectBloatware -PackageName $bloatware) {
                RemoveBloatware -PackageName $bloatware -RetryCount 3
            }
        }   
    }
    catch {
        Write-Error "Error in Main function: $_"
    }
}