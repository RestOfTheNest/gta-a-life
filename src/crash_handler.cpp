#include "crash_handler.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dbghelp.h>
#include <io.h>
#include <cstdio>
#include <cstdint>
#include <ctime>
#include <cmath>
#include <atomic>

#include "plugin.h"
#include "CPed.h"
#include "CVehicle.h"
#include "CPlayerPed.h"
#include "CStreaming.h"
#include "CPools.h"
#include "CTimer.h"
#include "common.h"
#include "municipal.h"

#pragma comment(lib, "dbghelp.lib")

// =============================================================================
//  1. Ring Buffer Flight Recorder
// =============================================================================

static constexpr size_t k_traceBufferSize = 32;
static TraceEvent s_traceBuffer[k_traceBufferSize]{};
static std::atomic<uint32_t> s_traceIndex{ 0 };

void RecordEngineTrace(const char* subsystem, const char* fmt, ...) {
    const uint32_t idx = s_traceIndex.fetch_add(1, std::memory_order_relaxed);
    TraceEvent& ev = s_traceBuffer[idx % k_traceBufferSize];
    ev.timestampMs = GetTickCount();

    if (subsystem) {
        strncpy_s(ev.subsystem, sizeof(ev.subsystem), subsystem, _TRUNCATE);
    } else {
        ev.subsystem[0] = '\0';
    }

    if (fmt) {
        va_list args;
        va_start(args, fmt);
        vsnprintf_s(ev.action, sizeof(ev.action), _TRUNCATE, fmt, args);
        va_end(args);
    } else {
        ev.action[0] = '\0';
    }
}

// =============================================================================
//  2. Telemetry Snapshots & Exception-Guarded Memory Readers
// =============================================================================

struct PlayerTelemetry {
    bool  valid;
    float posX, posY, posZ;
    float speedX, speedY, speedZ;
    float speedMph;
    int   vehicleModel;
    int   interior;
};

struct PoolTelemetry {
    bool         vehValid;
    unsigned int vehUsed;
    int          vehMax;

    bool         pedValid;
    unsigned int pedUsed;
    int          pedMax;

    bool         objValid;
    unsigned int objUsed;
    int          objMax;
};

struct StreamingTelemetry {
    bool         valid;
    unsigned int memoryUsedBytes;
    unsigned int memoryBudgetBytes;
    unsigned int queuedModels;
};

struct MunicipalTelemetry {
    bool   valid;
    float  socialUnrestPct;
    size_t activeIncidentsCount;
    size_t activeRoadblocksCount;
    float  frameDeltaMs;
};

static void SafeCollectPlayerState(PlayerTelemetry* out) {
    if (!out) return;
    out->valid = false;
    __try {
        CPlayerPed* player = FindPlayerPed(-1);
        if (player && CPools::ms_pPedPool && CPools::ms_pPedPool->IsObjectValid(player)) {
            out->valid = true;
            CVector pos = player->GetPosition();
            out->posX = pos.x;
            out->posY = pos.y;
            out->posZ = pos.z;

            out->speedX = player->m_vecMoveSpeed.x;
            out->speedY = player->m_vecMoveSpeed.y;
            out->speedZ = player->m_vecMoveSpeed.z;
            const float spdSq = out->speedX * out->speedX + out->speedY * out->speedY + out->speedZ * out->speedZ;
            out->speedMph = sqrtf(spdSq) * 111.847f; // GTA SA units to mph

            out->interior = static_cast<int>(player->m_nAreaCode);

            CVehicle* veh = FindPlayerVehicle(-1, false);
            if (veh && CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->IsObjectValid(veh)) {
                out->vehicleModel = veh->m_nModelIndex;
            } else {
                out->vehicleModel = -1;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        out->valid = false;
    }
}

static void SafeCollectPoolHealth(PoolTelemetry* out) {
    if (!out) return;
    out->vehValid = false;
    out->pedValid = false;
    out->objValid = false;

    __try {
        if (CPools::ms_pVehiclePool) {
            out->vehMax = CPools::ms_pVehiclePool->m_nSize;
            out->vehUsed = CPools::ms_pVehiclePool->GetNoOfUsedSpaces();
            out->vehValid = true;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        out->vehValid = false;
    }

    __try {
        if (CPools::ms_pPedPool) {
            out->pedMax = CPools::ms_pPedPool->m_nSize;
            out->pedUsed = CPools::ms_pPedPool->GetNoOfUsedSpaces();
            out->pedValid = true;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        out->pedValid = false;
    }

    __try {
        if (CPools::ms_pObjectPool) {
            out->objMax = CPools::ms_pObjectPool->m_nSize;
            out->objUsed = CPools::ms_pObjectPool->GetNoOfUsedSpaces();
            out->objValid = true;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        out->objValid = false;
    }
}

static void SafeCollectStreamingHealth(StreamingTelemetry* out) {
    if (!out) return;
    out->valid = false;
    __try {
        out->memoryUsedBytes = CStreaming::ms_memoryUsed;
        out->memoryBudgetBytes = 50 * 1024 * 1024; // Standard 50MB base streaming memory
        out->queuedModels = CStreaming::ms_numModelsRequested;
        out->valid = true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        out->valid = false;
    }
}

static void SafeCollectMunicipalState(MunicipalTelemetry* out) {
    if (!out) return;
    out->valid = false;
    __try {
        MunicipalDiagnosticTelemetry diag = GetMunicipalDiagnosticTelemetry();
        out->socialUnrestPct = diag.socialUnrestPct;
        out->activeIncidentsCount = diag.activeIncidentsCount;
        out->activeRoadblocksCount = diag.activeRoadblocksCount;
        out->frameDeltaMs = diag.lastFrameDeltaMs;
        out->valid = true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        out->valid = false;
    }
}

// =============================================================================
//  3. Symbolic Resolution & Minidump Generation
// =============================================================================

static LPTOP_LEVEL_EXCEPTION_FILTER s_prevFilter = nullptr;
static LONG s_installed = 0;
static LONG s_inCrashHandler = 0;

void GetFormattedLocalTime(char* outBuf, size_t bufSize) {
    if (!outBuf || bufSize == 0) return;
    SYSTEMTIME st;
    GetLocalTime(&st);
    snprintf(outBuf, bufSize, "%04u-%02u-%02u %02u:%02u:%02u.%03u",
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

static const char* GetExceptionDescription(DWORD code) {
    switch (code) {
    case EXCEPTION_ACCESS_VIOLATION:         return "EXCEPTION_ACCESS_VIOLATION";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
    case EXCEPTION_BREAKPOINT:               return "EXCEPTION_BREAKPOINT";
    case EXCEPTION_DATATYPE_MISALIGNMENT:    return "EXCEPTION_DATATYPE_MISALIGNMENT";
    case EXCEPTION_FLT_DENORMAL_OPERAND:     return "EXCEPTION_FLT_DENORMAL_OPERAND";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
    case EXCEPTION_FLT_INEXACT_RESULT:       return "EXCEPTION_FLT_INEXACT_RESULT";
    case EXCEPTION_FLT_INVALID_OPERATION:    return "EXCEPTION_FLT_INVALID_OPERATION";
    case EXCEPTION_FLT_OVERFLOW:             return "EXCEPTION_FLT_OVERFLOW";
    case EXCEPTION_FLT_STACK_CHECK:          return "EXCEPTION_FLT_STACK_CHECK";
    case EXCEPTION_FLT_UNDERFLOW:            return "EXCEPTION_FLT_UNDERFLOW";
    case EXCEPTION_ILLEGAL_INSTRUCTION:      return "EXCEPTION_ILLEGAL_INSTRUCTION";
    case EXCEPTION_IN_PAGE_ERROR:            return "EXCEPTION_IN_PAGE_ERROR";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "EXCEPTION_INT_DIVIDE_BY_ZERO";
    case EXCEPTION_INT_OVERFLOW:             return "EXCEPTION_INT_OVERFLOW";
    case EXCEPTION_INVALID_DISPOSITION:      return "EXCEPTION_INVALID_DISPOSITION";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
    case EXCEPTION_PRIV_INSTRUCTION:         return "EXCEPTION_PRIV_INSTRUCTION";
    case EXCEPTION_SINGLE_STEP:              return "EXCEPTION_SINGLE_STEP";
    case EXCEPTION_STACK_OVERFLOW:           return "EXCEPTION_STACK_OVERFLOW";
    case 0xE06D7363:                         return "Unhandled C++ Exception (MSVC)";
    default:                                 return "UNKNOWN_EXCEPTION";
    }
}

static void GetModuleInfoForAddr(void* addr, char* outModName, size_t maxModName, uintptr_t& outBase, uintptr_t& outOffset) {
    outModName[0] = '\0';
    outBase = 0;
    outOffset = 0;

    if (!addr) {
        strncpy_s(outModName, maxModName, "NullAddress", _TRUNCATE);
        return;
    }

    MEMORY_BASIC_INFORMATION mbi{};
    if (VirtualQuery(addr, &mbi, sizeof(mbi)) != 0 && mbi.AllocationBase != nullptr) {
        outBase = reinterpret_cast<uintptr_t>(mbi.AllocationBase);
        outOffset = reinterpret_cast<uintptr_t>(addr) - outBase;
        char fullPath[MAX_PATH] = { 0 };
        if (GetModuleFileNameA(reinterpret_cast<HMODULE>(mbi.AllocationBase), fullPath, MAX_PATH) > 0) {
            const char* slash = strrchr(fullPath, '\\');
            if (!slash) slash = strrchr(fullPath, '/');
            const char* fname = slash ? slash + 1 : fullPath;
            strncpy_s(outModName, maxModName, fname, _TRUNCATE);
            return;
        }
    }
    strncpy_s(outModName, maxModName, "UnknownModule", _TRUNCATE);
}

static bool WriteMiniDump(EXCEPTION_POINTERS* pExceptionInfo) {
    HANDLE hFile = CreateFileA(
        "gtasystemcore_crash.dmp",
        GENERIC_WRITE,
        FILE_SHARE_READ,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    MINIDUMP_EXCEPTION_INFORMATION mei{};
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = pExceptionInfo;
    mei.ClientPointers = FALSE;

    BOOL ok = MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MiniDumpNormal,
        pExceptionInfo ? &mei : nullptr,
        nullptr,
        nullptr
    );

    CloseHandle(hFile);
    return ok != FALSE;
}

static LONG WINAPI TopLevelExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo) {
    // Prevent recursive crash handler execution
    if (InterlockedCompareExchange(&s_inCrashHandler, 1, 0) != 0) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // 1. Generate Binary Minidump (.dmp)
    const bool dmpWritten = WriteMiniDump(pExceptionInfo);

    // 2. Write Diagnostic Log (.log)
    FILE* f = nullptr;
    errno_t err = fopen_s(&f, "gtasystemcore_crash.log", "w");
    if (err != 0 || f == nullptr) {
        fopen_s(&f, "gtasystemcore_crash.log", "a");
    }
    if (f != nullptr) {
        char crashTimeStr[64]{ 0 };
        GetFormattedLocalTime(crashTimeStr, sizeof(crashTimeStr));

        PEXCEPTION_RECORD rec = pExceptionInfo ? pExceptionInfo->ExceptionRecord : nullptr;
        PCONTEXT ctx = pExceptionInfo ? pExceptionInfo->ContextRecord : nullptr;

        fprintf(f, "\n");
        fprintf(f, "================================================================================\n");
        fprintf(f, "  GTA SAN ANDREAS FATAL ENGINE CRASH DIAGNOSTIC REPORT\n");
        fprintf(f, "  Timestamp: %s\n", crashTimeStr);
        fprintf(f, "  Process ID: %lu | Thread ID: %lu\n", GetCurrentProcessId(), GetCurrentThreadId());
        fprintf(f, "================================================================================\n\n");

        const DWORD code = rec ? rec->ExceptionCode : 0;
        fprintf(f, "Exception Code:   0x%08X -> %s\n", code, GetExceptionDescription(code));

        void* faultAddr = rec ? rec->ExceptionAddress : nullptr;
        char faultMod[MAX_PATH]{ 0 };
        uintptr_t faultBase = 0;
        uintptr_t faultOffset = 0;
        GetModuleInfoForAddr(faultAddr, faultMod, sizeof(faultMod), faultBase, faultOffset);

        fprintf(f, "Fault Address:    0x%08X\n", static_cast<uint32_t>(reinterpret_cast<uintptr_t>(faultAddr)));
        fprintf(f, "Fault Module:     %s (Base: 0x%08X, Offset: +0x%08X)\n",
            faultMod,
            static_cast<uint32_t>(faultBase),
            static_cast<uint32_t>(faultOffset));

        if (code == EXCEPTION_ACCESS_VIOLATION && rec && rec->NumberParameters >= 2) {
            const ULONG_PTR accessType = rec->ExceptionInformation[0];
            const ULONG_PTR targetAddr = rec->ExceptionInformation[1];
            const char* opName = "access";
            if (accessType == 0) opName = "read from";
            else if (accessType == 1) opName = "write to";
            else if (accessType == 8) opName = "execute (DEP violation) at";

            fprintf(f, "Access Violation: Attempted to %s target address 0x%08X\n",
                opName, static_cast<uint32_t>(targetAddr));
        }

        fprintf(f, "Minidump Status:  %s (gtasystemcore_crash.dmp)\n", dmpWritten ? "SUCCESSFULLY WRITTEN" : "FAILED TO WRITE");

        if (ctx) {
            fprintf(f, "\n--------------------------------------------------------------------------------\n");
            fprintf(f, "                             REGISTER DUMP (x86)                                \n");
            fprintf(f, "--------------------------------------------------------------------------------\n");
            fprintf(f, "EAX: 0x%08X  EBX: 0x%08X  ECX: 0x%08X  EDX: 0x%08X\n", ctx->Eax, ctx->Ebx, ctx->Ecx, ctx->Edx);
            fprintf(f, "ESI: 0x%08X  EDI: 0x%08X  EBP: 0x%08X  ESP: 0x%08X\n", ctx->Esi, ctx->Edi, ctx->Ebp, ctx->Esp);
            fprintf(f, "EIP: 0x%08X  EFLAGS: 0x%08X\n", ctx->Eip, ctx->EFlags);
        }

        // 3. Game Runtime Snapshot (Diagnostic Telemetry)
        fprintf(f, "\n--------------------------------------------------------------------------------\n");
        fprintf(f, "                    GAME RUNTIME SNAPSHOT (DIAGNOSTIC TELEMETRY)                \n");
        fprintf(f, "--------------------------------------------------------------------------------\n");

        PlayerTelemetry playerTelem{};
        SafeCollectPlayerState(&playerTelem);
        if (playerTelem.valid) {
            fprintf(f, "[Player State]\n");
            fprintf(f, "  Position:       (X: %.2f, Y: %.2f, Z: %.2f)\n", playerTelem.posX, playerTelem.posY, playerTelem.posZ);
            fprintf(f, "  Speed Vector:   (Vx: %.4f, Vy: %.4f, Vz: %.4f) -> Velocity: %.1f mph\n",
                playerTelem.speedX, playerTelem.speedY, playerTelem.speedZ, playerTelem.speedMph);
            if (playerTelem.vehicleModel >= 0) {
                fprintf(f, "  Vehicle Model:  %d\n", playerTelem.vehicleModel);
            } else {
                fprintf(f, "  Vehicle Model:  On Foot\n");
            }
            fprintf(f, "  Interior ID:    %d\n", playerTelem.interior);
        } else {
            fprintf(f, "[Player State]    Unavailable (Player ped null or inaccessible)\n");
        }

        PoolTelemetry poolTelem{};
        SafeCollectPoolHealth(&poolTelem);
        fprintf(f, "[Pool Health]\n");
        if (poolTelem.vehValid) {
            const float vehPct = poolTelem.vehMax > 0 ? (static_cast<float>(poolTelem.vehUsed) / poolTelem.vehMax * 100.0f) : 0.0f;
            fprintf(f, "  CVehiclePool:   %u / %d  (%.1f%% utilized)\n", poolTelem.vehUsed, poolTelem.vehMax, vehPct);
        } else {
            fprintf(f, "  CVehiclePool:   Unavailable\n");
        }
        if (poolTelem.pedValid) {
            const float pedPct = poolTelem.pedMax > 0 ? (static_cast<float>(poolTelem.pedUsed) / poolTelem.pedMax * 100.0f) : 0.0f;
            fprintf(f, "  CPedPool:       %u / %d  (%.1f%% utilized)\n", poolTelem.pedUsed, poolTelem.pedMax, pedPct);
        } else {
            fprintf(f, "  CPedPool:       Unavailable\n");
        }
        if (poolTelem.objValid) {
            const float objPct = poolTelem.objMax > 0 ? (static_cast<float>(poolTelem.objUsed) / poolTelem.objMax * 100.0f) : 0.0f;
            fprintf(f, "  CObjectPool:    %u / %d  (%.1f%% utilized)\n", poolTelem.objUsed, poolTelem.objMax, objPct);
        } else {
            fprintf(f, "  CObjectPool:    Unavailable\n");
        }

        StreamingTelemetry streamTelem{};
        SafeCollectStreamingHealth(&streamTelem);
        fprintf(f, "[Streaming Health]\n");
        if (streamTelem.valid) {
            const float usedMb = static_cast<float>(streamTelem.memoryUsedBytes) / (1024.0f * 1024.0f);
            const float budgetMb = static_cast<float>(streamTelem.memoryBudgetBytes) / (1024.0f * 1024.0f);
            const float memPct = budgetMb > 0.0f ? (usedMb / budgetMb * 100.0f) : 0.0f;
            fprintf(f, "  Memory Used:    %.2f MB / %.2f MB  (%.1f%% of cache)\n", usedMb, budgetMb, memPct);
            fprintf(f, "  Queued Models:  %u in request queue\n", streamTelem.queuedModels);
        } else {
            fprintf(f, "  Streaming:      Unavailable\n");
        }

        MunicipalTelemetry muniTelem{};
        SafeCollectMunicipalState(&muniTelem);
        fprintf(f, "[Municipal Subsystem State]\n");
        if (muniTelem.valid) {
            fprintf(f, "  Social Unrest:  %.1f%%\n", muniTelem.socialUnrestPct);
            fprintf(f, "  Incidents:      %zu active\n", muniTelem.activeIncidentsCount);
            fprintf(f, "  Roadblocks:     %zu active\n", muniTelem.activeRoadblocksCount);
            fprintf(f, "  Frame Delta:    %.2f ms\n", muniTelem.frameDeltaMs);
        } else {
            fprintf(f, "  Municipal:      Unavailable\n");
        }

        // 4. Ring Buffer Flight Recorder Breadcrumbs
        fprintf(f, "\n--------------------------------------------------------------------------------\n");
        fprintf(f, "                   FLIGHT RECORDER (LAST 20 BREADCRUMBS)                        \n");
        fprintf(f, "--------------------------------------------------------------------------------\n");
        const uint32_t crashTickMs = GetTickCount();
        const uint32_t totalEvents = s_traceIndex.load(std::memory_order_relaxed);
        if (totalEvents == 0) {
            fprintf(f, "  [Flight recorder empty - no events logged]\n");
        } else {
            const uint32_t count = (totalEvents < 20) ? totalEvents : 20;
            const uint32_t startIdx = totalEvents - count;
            for (uint32_t i = startIdx; i < totalEvents; ++i) {
                const TraceEvent& ev = s_traceBuffer[i % k_traceBufferSize];
                const int32_t relOffsetMs = static_cast<int32_t>(ev.timestampMs) - static_cast<int32_t>(crashTickMs);
                fprintf(f, "  [%08u ms | T%+6d ms] [%-16s] %s\n", ev.timestampMs, relOffsetMs, ev.subsystem, ev.action);
            }
        }

        // 5. Symbolic Callstack Trace
        if (ctx) {
            fprintf(f, "\n--------------------------------------------------------------------------------\n");
            fprintf(f, "                             CALLSTACK TRACE (x86)                              \n");
            fprintf(f, "--------------------------------------------------------------------------------\n");

            HANDLE hProcess = GetCurrentProcess();
            HANDLE hThread = GetCurrentThread();

            SymInitialize(hProcess, nullptr, TRUE);
            SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

            CONTEXT walkCtx = *ctx;
            STACKFRAME64 stackFrame{};
            stackFrame.AddrPC.Offset = walkCtx.Eip;
            stackFrame.AddrPC.Mode = AddrModeFlat;
            stackFrame.AddrFrame.Offset = walkCtx.Ebp;
            stackFrame.AddrFrame.Mode = AddrModeFlat;
            stackFrame.AddrStack.Offset = walkCtx.Esp;
            stackFrame.AddrStack.Mode = AddrModeFlat;

            for (int frame = 0; frame < 20; ++frame) {
                BOOL walkOk = StackWalk64(
                    IMAGE_FILE_MACHINE_I386,
                    hProcess,
                    hThread,
                    &stackFrame,
                    &walkCtx,
                    nullptr,
                    SymFunctionTableAccess64,
                    SymGetModuleBase64,
                    nullptr
                );

                if (!walkOk || stackFrame.AddrPC.Offset == 0) {
                    break;
                }

                const DWORD64 pc = stackFrame.AddrPC.Offset;
                char frameMod[MAX_PATH]{ 0 };
                uintptr_t frameBase = 0;
                uintptr_t frameOffset = 0;
                GetModuleInfoForAddr(reinterpret_cast<void*>(static_cast<uintptr_t>(pc)), frameMod, sizeof(frameMod), frameBase, frameOffset);

                alignas(SYMBOL_INFO) char symBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(char)]{ 0 };
                PSYMBOL_INFO pSym = reinterpret_cast<PSYMBOL_INFO>(symBuffer);
                pSym->SizeOfStruct = sizeof(SYMBOL_INFO);
                pSym->MaxNameLen = MAX_SYM_NAME;

                DWORD64 symDisplacement = 0;
                BOOL hasSym = SymFromAddr(hProcess, pc, &symDisplacement, pSym);

                IMAGEHLP_LINE64 lineInfo{};
                lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                DWORD lineDisplacement = 0;
                BOOL hasLine = SymGetLineFromAddr64(hProcess, pc, &lineDisplacement, &lineInfo);

                if (hasSym && hasLine) {
                    fprintf(f, "  #%02d  0x%08X  [%s + 0x%08X]  %s + 0x%X  (%s:%u)\n",
                        frame,
                        static_cast<uint32_t>(pc),
                        frameMod,
                        static_cast<uint32_t>(frameOffset),
                        pSym->Name,
                        static_cast<uint32_t>(symDisplacement),
                        lineInfo.FileName,
                        lineInfo.LineNumber);
                } else if (hasSym) {
                    fprintf(f, "  #%02d  0x%08X  [%s + 0x%08X]  %s + 0x%X\n",
                        frame,
                        static_cast<uint32_t>(pc),
                        frameMod,
                        static_cast<uint32_t>(frameOffset),
                        pSym->Name,
                        static_cast<uint32_t>(symDisplacement));
                } else {
                    fprintf(f, "  #%02d  0x%08X  [%s + 0x%08X]\n",
                        frame,
                        static_cast<uint32_t>(pc),
                        frameMod,
                        static_cast<uint32_t>(frameOffset));
                }
            }

            SymCleanup(hProcess);
        }

        fprintf(f, "================================================================================\n\n");

        fflush(f);
        int fd = _fileno(f);
        if (fd >= 0) {
            HANDLE hLogFile = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
            if (hLogFile != INVALID_HANDLE_VALUE && hLogFile != nullptr) {
                FlushFileBuffers(hLogFile);
            }
        }
        fclose(f);
    }

    if (s_prevFilter) {
        return s_prevFilter(pExceptionInfo);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void InstallCrashHandler() {
    if (InterlockedCompareExchange(&s_installed, 1, 0) == 0) {
        // 1. Purge legacy crash log from prior game sessions
        DeleteFileA("gtasystemcore_crash.log");
        DeleteFileA("gtasystemcore_crash.dmp");
        DeleteFileA("crash_dump.log");
        DeleteFileA("crash_dump.dmp");

        // 2. Register unhandled top level exception filter
        s_prevFilter = SetUnhandledExceptionFilter(TopLevelExceptionFilter);

        // 3. Write clean session start marker
        FILE* f = nullptr;
        if (fopen_s(&f, "gtasystemcore_session.log", "w") == 0 && f) {
            char timeStr[64];
            GetFormattedLocalTime(timeStr, sizeof(timeStr));
            fprintf(f, "[SESSION START] %s | PID: %lu\n", timeStr, GetCurrentProcessId());
            fclose(f);
        }
    }
}

void UninstallCrashHandler() {
    if (InterlockedCompareExchange(&s_installed, 0, 1) == 1) {
        SetUnhandledExceptionFilter(s_prevFilter);
        s_prevFilter = nullptr;
    }
}
