#pragma once
#include <string>

// Finds all running processes matching the given name (case-insensitive)
// and terminates them. Returns the number of processes killed.
int KillProcessByName(const std::wstring& processName);
