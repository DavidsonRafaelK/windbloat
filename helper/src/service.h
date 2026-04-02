#pragma once
#include <string>

// Sends a STOP control to the named service and waits for it to stop.
// Returns true if the service stopped (or was already stopped).
bool StopService(const std::wstring& serviceName);

// Marks the named service for deletion via SCM.
// The service must be stopped first. Returns true on success.
bool DeleteService(const std::wstring& serviceName);
