#include "streaming_fix.h"
#include <windows.h>
#include <plugin.h>
#include <CStreaming.h>

namespace StreamingFix {

// Target: 1024 MB streaming memory limit
static constexpr unsigned int k_TargetStreamingBytes = 1024 * 1024 * 1024; // 1 GB

static void PatchEngineStreamingLimit() {
    // 1. Patch the hardcoded 50MB assignment inside CStreaming::Init() at 0x5B8E6A
    // Instruction: mov dword ptr ds:[08A5A80h], 3200000h (50MB) -> Change immediate operand to 1024MB
    auto* pInitLimit = reinterpret_cast<unsigned int*>(0x5B8E6A);
    if (pInitLimit) {
        DWORD oldProtect;
        if (VirtualProtect(pInitLimit, sizeof(unsigned int), PAGE_EXECUTE_READWRITE, &oldProtect)) {
            *pInitLimit = k_TargetStreamingBytes;
            VirtualProtect(pInitLimit, sizeof(unsigned int), oldProtect, &oldProtect);
        }
    }

    // 2. Patch the actual active limit variable in memory (0x8A5A80)
    auto* pMemoryAvailable = reinterpret_cast<unsigned int*>(0x8A5A80);
    if (pMemoryAvailable) {
        DWORD oldProtect;
        if (VirtualProtect(pMemoryAvailable, sizeof(unsigned int), PAGE_EXECUTE_READWRITE, &oldProtect)) {
            *pMemoryAvailable = k_TargetStreamingBytes;
            VirtualProtect(pMemoryAvailable, sizeof(unsigned int), oldProtect, &oldProtect);
        }
    }

    // DO NOT TOUCH 0x8E4CB4 or 0x8E4CB8! Those track memory usage!
    CStreaming::ms_memoryAvailable = k_TargetStreamingBytes;
}

void Init() {
    PatchEngineStreamingLimit();
}

size_t GetMemoryLimit() {
    return CStreaming::ms_memoryAvailable;
}

size_t GetMemoryUsed() {
    return CStreaming::ms_memoryUsed;
}

void Update() {
    // Keep enforcing the 1024MB limit in case the engine reset it
    if (CStreaming::ms_memoryAvailable < k_TargetStreamingBytes) {
        PatchEngineStreamingLimit();
    }
}

} // namespace StreamingFix
