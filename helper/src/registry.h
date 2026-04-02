#pragma once
#include <string>

// Recursively deletes the given registry key.
// keyPath format: "HKLM\\SOFTWARE\\Vendor\\Product"
// Supported roots: HKLM, HKCU, HKCR, HKU, HKCC
// Returns true if the key no longer exists after the call.
bool DeleteRegistryKey(const std::wstring& keyPath);
