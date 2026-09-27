#pragma once
#include <cstdint>

class CPed;

namespace GangWars {
    void Init();
    void Update(uint32_t currentMs, CPed* player);
    void Cleanup();
}
