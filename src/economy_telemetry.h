#pragma once
#include <cstdint>
#include <cstddef>

namespace EconomyTelemetry {
    void StartTelemetryWorker();
    void StopTelemetryWorker();
    void SerializeTelemetryJson();
}
