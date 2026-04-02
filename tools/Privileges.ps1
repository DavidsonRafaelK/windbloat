<#
 * Tool        : Privileges
 * Description : Ensures the script is running with administrator privileges.
#>

function Confirm-Administrator {
        $IsAdmin = (
                [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
        ).IsInRole([Security.Principal.WindowsBuiltInRole]"Administrator")

        if ($IsAdmin) {
                Write-Output "Running with administrator privileges."
                return $true
        }
        else {
                Write-Output "Relaunching script with administrator privileges..."
                Start-Process PowerShell -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`"" -Verb RunAs -Wait
                exit
        }
}
