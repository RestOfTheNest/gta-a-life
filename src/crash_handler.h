#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdint>
#include <cstddef>

// =============================================================================
//  Flight Recorder & Crash Diagnostics Subsystem
// =============================================================================

struct TraceEvent {
    uint32_t timestampMs;
    char     subsystem[24];
    char     action[96];
};

void RecordEngineTrace(const char* subsystem, const char* fmt, ...);

#define ENGINE_TRACE(subsystem, fmt, ...) \
    RecordEngineTrace((subsystem), (fmt), ##__VA_ARGS__)

void GetFormattedLocalTime(char* outBuf, size_t bufSize);
void InstallCrashHandler();
void UninstallCrashHandler();
