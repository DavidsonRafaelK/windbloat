#pragma once
#include <string>

// Takes ownership of path, resets its ACL, then recursively deletes it.
// Falls back to MoveFileEx(MOVEFILE_DELAY_UNTIL_REBOOT) for locked files.
// Returns true if the path no longer exists after the call.
bool ForceDeletePath(const std::wstring& path);
