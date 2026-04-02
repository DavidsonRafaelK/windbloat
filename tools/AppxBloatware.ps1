<#
 * Tool        : AppxBloatware
 * Description : Detects and removes AppX (UWP) bloatware packages.
#>

function Test-AppxBloatware {
        param (
                [string] $PackageName
        )

        $package = Get-AppxPackage -Name $PackageName -ErrorAction SilentlyContinue

        if ($package) {
                Write-Host "Bloatware detected: $PackageName"
                return $true
        }
        else {
                Write-Host "Bloatware not detected: $PackageName"
                return $false
        }
}

function Remove-AppxBloatware {
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
                }
                else {
                        Write-Host "Package not found: $PackageName"
                        break
                }
        }
}
