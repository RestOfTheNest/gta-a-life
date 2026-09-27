#pragma once
#include <cstdint>

class CPed;

namespace Rioters {
    void Init();
    void Update(uint32_t currentMs, CPed* player);
    void Cleanup();
    void ForceCityHallRiot();
}
