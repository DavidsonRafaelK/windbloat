<#
 * Author      : @DavidsonRafaelK
 * Description : Windows Debloat Script
 * Date        : 2026-04-01
 * Copyright   : (c) 2026 DavidsonRafaelK. All rights reserved.
#>

$script:DebloatHelperPath = Join-Path $PSScriptRoot "helper\build\debloat-helper.exe"

. "$PSScriptRoot\tools\Privileges.ps1"
. "$PSScriptRoot\tools\SystemInfo.ps1"
. "$PSScriptRoot\tools\ProductKey.ps1"
. "$PSScriptRoot\tools\AppxBloatware.ps1"
. "$PSScriptRoot\tools\Win32Bloatware.ps1"

$BloatwareList = (Get-Content -Path "$PSScriptRoot\bloatware.json" -Raw | ConvertFrom-Json).bloatware

function Main {
        try {
                Confirm-Administrator

                $SystemInfo = Get-SystemInfo
                Write-Output "System Information:"
                Write-Output $SystemInfo

                if (-not [string]::IsNullOrEmpty($SystemInfo.OSProductKey)) {
                        Write-Output "Windows Product Key: $($SystemInfo.OSProductKey)"
                }
                else {
                        Write-Output "Windows Product Key not found. Attempting to retrieve..."
                        Write-Host "Press any key to retrieve the product key..."
                        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

                        $RetrievedKey = Get-WindowsProductKey
                        if ($null -eq $RetrievedKey) {
                                Write-Warning "Could not retrieve product key from remote source."
                        }
                }

                foreach ($bloatware in $BloatwareList) {
                        if ($bloatware.app_id -and (Test-AppxBloatware -PackageName $bloatware.app_id)) {
                                Remove-AppxBloatware -PackageName $bloatware.app_id -RetryCount 3
                        }

                        if ($bloatware.win32) {
                                Remove-Win32Bloatware -Win32Info $bloatware.win32
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
