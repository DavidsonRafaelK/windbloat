<#
 * Tool        : Win32Bloatware
 * Description : Removes Win32 bloatware by delegating to debloat-helper.exe.
 *               Requires $script:DebloatHelperPath to be set by main.ps1 before dot-sourcing.
#>

function Invoke-DebloatHelper {
        param (
                [string] $Command,
                [string] $Argument
        )

        if (-not (Test-Path $script:DebloatHelperPath)) {
                Write-Warning "debloat-helper.exe not found at: $script:DebloatHelperPath"
                return $false
        }

        try {
                $output = & $script:DebloatHelperPath $Command $Argument 2>&1
                Write-Host $output
                return $LASTEXITCODE -eq 0
        }
        catch {
                Write-Warning "Invoke-DebloatHelper failed [$Command $Argument]: $_"
                return $false
        }
}

function Remove-Win32Bloatware {
        param (
                [PSCustomObject] $Win32Info
        )

        foreach ($process in $Win32Info.processes) {
                if (-not (Invoke-DebloatHelper -Command "--kill-process" -Argument $process)) {
                        Write-Warning "Failed to kill process: $process"
                }
        }

        foreach ($service in $Win32Info.services) {
                if (-not (Invoke-DebloatHelper -Command "--stop-service" -Argument $service)) {
                        Write-Warning "Failed to stop service: $service"
                }
                if (-not (Invoke-DebloatHelper -Command "--delete-service" -Argument $service)) {
                        Write-Warning "Failed to delete service: $service"
                }
        }

        foreach ($path in $Win32Info.install_dirs) {
                if (-not (Invoke-DebloatHelper -Command "--force-delete" -Argument $path)) {
                        Write-Warning "Failed to delete path: $path"
                }
        }

        foreach ($key in $Win32Info.registry_keys) {
                if (-not (Invoke-DebloatHelper -Command "--delete-registry" -Argument $key)) {
                        Write-Warning "Failed to delete registry key: $key"
                }
        }
}
