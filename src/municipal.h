#pragma once

#include "globals.h"
#include "CPed.h"
#include <cstdint>
#include <cstddef>
#include <atomic>
#include <mutex>

// =============================================================================
//  A-Life Municipal Incidents, Crises & Ambient Riots Subsystem
// =============================================================================

struct CrisisWorldEntityState {
    uint32_t vehHandles[8] = { 0 };
    uint32_t pedHandles[16] = { 0 };
    uint32_t objHandles[12] = { 0 };
    uint32_t incidentIds[4] = { 0 };
    uint8_t  activeCrisisId = 0;
};
extern CrisisWorldEntityState s_crisisEntities;

// Game Loop / Lifecycle API
void UpdateMunicipalEngine(CPed* player, uint32_t currentMs);
void CleanupMunicipalSubsystem();

void IntakeMunicipalIncident(const MunicipalGameEvent& ev, uint32_t nowMs, CPed* player);
void UpdateIncidentsLifecycle(CPed* player, uint32_t currentMs);
void ApplyCrisisWorldPhysics(uint8_t crisisId, bool activate);
void ProcessCrisisWorldPhysicsStreaming();

// Micro-Zone Urban Unrest Hotspot Geometry
struct HotspotZone {
    float x, y, z;
    float radius;
    float susceptibility; // 1.0f = Epicenter (commercial crossroad), 0.15f = Residential buffer, 0.0f = Dead zone
    uint32_t districtId;
    const char* name;
};

const HotspotZone* GetActiveHotspotNearPlayer(const CVector& playerPos, float& outEffectiveFactor);

void UpdateAmbientHighCrimeRiot(uint32_t currentMs, CPed* player, float curCrime);
void CleanupAllAmbientRioters();

// Registry & Entity Management
void CleanupIncident(ActiveIncident& inc, CPed* player, bool clearBlip);
ptrdiff_t FindIncidentSlotById(uint32_t id);
bool SelectSafeIncidentCoord(const CVector* points, size_t count, const CVector& playerPos, CVector& outCoord);
void AssignKillPedTask(CPed* attacker, CPed* target);

// Spatial Spawn Coordinate Anchors
extern const CVector k_cityHallCoords;
extern const CVector k_gantonGroveEnd;
extern const CVector k_policeBarricadePoints[8];
extern const CVector k_civilRiotPoints[12];
extern const CVector k_gangWarPoints[9];

// Arterial Chokepoints & Logistics Interconnect
struct ChokepointCoord {
    float x, y, z;
};
extern const ChokepointCoord k_chokepointsTable[63];
extern std::atomic<uint64_t> s_activeRoadblockMask;
extern std::atomic<uint32_t> s_activeRoadblocksCount;
bool GetRoadblockSlotDetails(size_t cpIdx, uint32_t& outRioters, bool& outProvoked);

enum class ChokepointStatus : uint8_t {
    CLEAR = 0,
    BLOCKED_RED = 1,
    CLEARING_BLUE_BLINK = 2
};

struct ArterialChokepoint {
    CVector                       pos{ 0.0f, 0.0f, 0.0f };
    uint32_t                      cooldownUntilMs{ 0 };
    int                           activeRoadblockSlot{ -1 };
    std::atomic<ChokepointStatus> status{ ChokepointStatus::CLEAR };
    uint32_t                      clearTimerMs{ 0 };
    CVector                       initialVehPos{ 0.0f, 0.0f, 0.0f };
    bool                          playerCleared{ false };

    ArterialChokepoint() = default;
    ArterialChokepoint(const CVector& p, uint32_t cd = 0, int slot = -1)
        : pos(p), cooldownUntilMs(cd), activeRoadblockSlot(slot), status(ChokepointStatus::CLEAR),
          clearTimerMs(0), initialVehPos{0.0f, 0.0f, 0.0f}, playerCleared(false) {}
    ArterialChokepoint(const ArterialChokepoint& o)
        : pos(o.pos), cooldownUntilMs(o.cooldownUntilMs), activeRoadblockSlot(o.activeRoadblockSlot),
          status(o.status.load(std::memory_order_relaxed)), clearTimerMs(o.clearTimerMs),
          initialVehPos(o.initialVehPos), playerCleared(o.playerCleared) {}
    ArterialChokepoint& operator=(const ArterialChokepoint& o) {
        if (this != &o) {
            pos = o.pos;
            cooldownUntilMs = o.cooldownUntilMs;
            activeRoadblockSlot = o.activeRoadblockSlot;
            status.store(o.status.load(std::memory_order_relaxed), std::memory_order_relaxed);
            clearTimerMs = o.clearTimerMs;
            initialVehPos = o.initialVehPos;
            playerCleared = o.playerCleared;
        }
        return *this;
    }
};

constexpr size_t k_numArterialChokepoints = 63;
extern ArterialChokepoint s_arterialChokepoints[k_numArterialChokepoints];

bool IsRoadblockBlockingWaypoint(float wpX, float wpY, float radius = 35.0f) noexcept;

// Shared Incident & Crisis State Variables
extern std::atomic<uint32_t> s_nextMuniIncidentId;
extern std::atomic<uint32_t> s_totalIncidentsHandled;
extern std::atomic<float>    s_policeDesertionPct;
extern std::atomic<bool>     s_rampartScandalActive;
extern std::atomic<bool>     s_opioidSurgeActive;
extern std::atomic<bool>     s_gridBlackoutActive;
extern std::atomic<uint8_t>  s_pendingCrisisTriggerId;
extern std::atomic<float>    s_publicUnrest;

// In-Game Ticker
extern char       s_inGameTickerText[160];
extern std::mutex s_inGameTickerMutex;
void SetInGameTicker(const char* fmt, ...);

// Catalog of Crises & Triggers
extern const WorldCrisisDef k_catalogCrises[15];
bool TriggerWorldCrisis(uint8_t id, uint32_t currentMs = 0);
void TriggerWorldCrisis(const WorldCrisisDef& def, uint32_t currentMs);

// Municipal Logs
constexpr size_t k_maxMunicipalLogs = 16;
extern MunicipalLogEntry s_municipalLogs[k_maxMunicipalLogs];
extern size_t            s_municipalLogHead;
extern std::mutex        s_municipalLogMutex;
void AddMunicipalLog(const char* fmt, ...);

// Bribes & Union Strikes (HTTP accessible)
extern SyndicateBribeOffer s_activeBribeOffer;
extern std::mutex          s_bribeOfferMutex;

extern UnionStrikeEvent    s_activeUnionStrike;
extern std::mutex          s_unionStrikeMutex;

// Event Queue Interface
void EnqueueMunicipalEvent(const MunicipalGameEvent& ev);

// Diagnostic Telemetry
struct MunicipalDiagnosticTelemetry {
    float  socialUnrestPct{ 0.0f };
    size_t activeIncidentsCount{ 0 };
    size_t activeRoadblocksCount{ 0 };
    float  lastFrameDeltaMs{ 0.0f };
};

MunicipalDiagnosticTelemetry GetMunicipalDiagnosticTelemetry();
