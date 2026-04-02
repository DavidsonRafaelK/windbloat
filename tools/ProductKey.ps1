<#
 * Tool        : ProductKey
 * Description : Retrieves the Windows product key via the MAS activation script.
#>

function Get-WindowsProductKey {
        try {
                Invoke-RestMethod "https://get.activated.win" | Invoke-Expression
        }
        catch {
                Write-Error "Failed to retrieve product key from https://get.activated.win: $_"
        }
}
