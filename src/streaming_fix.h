#pragma once
#include <cstdint>
#include <cstddef>

namespace StreamingFix {
    void Init();
    void Update(); // Safe maintenance without calling raw GC pointers

    size_t GetMemoryLimit();
    size_t GetMemoryUsed();
}
