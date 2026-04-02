<#
 * Tool        : SystemInfo
 * Description : Collects OS and hardware information via CIM/WMI.
#>

function Get-SystemInfo {
        $OS = Get-CimInstance -Class Win32_OperatingSystem
        $HW = Get-CimInstance -Class Win32_ComputerSystem

        return [PSCustomObject]@{
                OSVersion        = $OS.Version
                OSCaption        = $OS.Caption
                OSProductKey     = (Get-WmiObject -Class SoftwareLicensingService).OA3xOriginalProductKey
                OSArchitecture   = $OS.OSArchitecture
                OSBuildNumber    = $OS.BuildNumber
                OSSerialNumber   = $OS.SerialNumber
                Manufacturer     = $HW.Manufacturer
                Model            = $HW.Model
                SerialNumber     = (Get-CimInstance -Class Win32_BIOS).SerialNumber
                UUID             = (Get-CimInstance -Class Win32_ComputerSystemProduct).UUID
                Processor        = (Get-CimInstance -Class Win32_Processor).Name
                RAM              = [Math]::Round(
                        (Get-CimInstance -Class Win32_PhysicalMemory |
                                Measure-Object -Property Capacity -Sum).Sum / 1GB, 2)
                Disks            = Get-CimInstance -Class Win32_DiskDrive |
                        Select-Object -Property Model, Size
        }
}
