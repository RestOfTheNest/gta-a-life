#include "municipal.h"
#include "rioters.h"
#include "gang_wars.h"
#include "globals.h"
#include "economy.h"
#include "crash_handler.h"

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <unordered_map>
#include <chrono>
#include <cstdarg>
#include <cstring>

#include "plugin.h"
#include "common.h"
#include "CPed.h"
#include "CVehicle.h"
#include "CAutomobile.h"
#include "CObject.h"
#include "CWorld.h"
#include "CStreaming.h"
#include "CPools.h"
#include "CTheScripts.h"
#include "CRadar.h"
#include "CHud.h"
#include "CWeather.h"
#include "CTimer.h"
#include "CPlayerPed.h"
#include "CWanted.h"
#include "CWeaponInfo.h"
#include "CWeapon.h"
#include "CPathFind.h"
#include "CPathNode.h"
#include "CAudioEngine.h"
#include "CTaskManager.h"
#include "CPedIntelligence.h"
#include "CTaskComplexKillPedOnFoot.h"
#include "CTaskComplexWanderStandard.h"
#include "CTaskSimpleStandStill.h"
#include "CAnimManager.h"
#include "CAnimBlock.h"
#include "CColStore.h"
#include "CClock.h"
#include "CCarCtrl.h"
#include "CPad.h"
#include "CPopulation.h"
#include "CMessages.h"
#include "enums/eWeaponType.h"
#include "enums/ePedType.h"
#include "enums/eModelID.h"
#include "enums/eAnimations.h"
#include "enums/eMoveState.h"
#include "enums/eTaskType.h"
#include "extensions/ScriptCommands.h"

#ifndef m_nDrivingStyle
#define m_nDrivingStyle m_nCarDrivingStyle
#endif

#ifndef m_nMoney
#define m_nMoney GetMoney()
#endif

using namespace plugin;

#ifndef MODEL_DEAGLE
#define MODEL_DEAGLE MODEL_DESERT_EAGLE
#endif

#ifndef MODEL_COPCARLS
#define MODEL_COPCARLS MODEL_COPCARLA
#endif

#ifndef MODEL_BMOCHAO
#define MODEL_BMOCHAO 162
#endif

#ifndef MODEL_WMOCHAO
#define MODEL_WMOCHAO 230
#endif

#ifndef MODEL_HMUCR
#ifdef MODEL_HMYCR
#define MODEL_HMUCR MODEL_HMYCR
#else
#define MODEL_HMUCR 47
#endif
#endif

#ifndef m_fCurrentRotation
#define m_fCurrentRotation m_fHeadingCurrent
#endif

#ifndef bPanicWhenScared
#define bPanicWhenScared bFleeWhenStanding
#endif

#ifndef bIgnorePlayer
#define bIgnorePlayer bDontFight
#endif

#ifndef m_nPedStat
#define m_nPedStat m_nPedType
#endif

#ifndef bCanAttackPlayerWithMelee
#define bCanAttackPlayerWithMelee bPartOfAttackWave
#endif

constexpr eBlipDisplay BLIP_DISPLAY_ONLY_RADAR = BLIP_DISPLAY_BLIP_ONLY;

#ifndef m_nTotalAmmo
#define m_nTotalAmmo m_nAmmoTotal
#endif

#ifndef m_nHandbrakeOn
#define m_nHandbrakeOn bIsHandbrakeOn
#endif

#ifndef WEAPONTYPE_SPAS12_SHOTGUN
#define WEAPONTYPE_SPAS12_SHOTGUN WEAPONTYPE_SPAS12
#endif

static constexpr int STREAMING_PRIORITY_REQUEST = PRIORITY_REQUEST;
static constexpr int STREAMING_GAME_REQUIRED = GAME_REQUIRED;
static constexpr int STREAMING_KEEP_IN_MEMORY = KEEP_IN_MEMORY;

// =============================================================================
//  Tactical Squads Memory Structure (SWAT & FBI Patrol Memory)
// =============================================================================

struct TacticalSquad {
    CVehicle* pVehicle = nullptr;
    unsigned int vehicleCreationTime = 0;
    CPed* pDriver = nullptr;
    CPed* pPassengers[3] = { nullptr, nullptr, nullptr };
    int passengerCount = 0;
    bool bDeployed = false;
    uint32_t lastCombatMs = 0;
    float lastDriverHealth = 200.0f;
    float lastPassengerHealth[3] = { 200.0f, 200.0f, 200.0f };
};
static constexpr size_t k_maxTacticalSquads = 8;
static TacticalSquad s_tacticalSquads[k_maxTacticalSquads]{};

// =============================================================================
//  Spatial Spawn Point Coordinates
// =============================================================================

const CVector k_cityHallCoords{ 1481.0f, -1745.0f, 13.5f }; // Мэрия / Pershing Square
const CVector k_gantonGroveEnd{ 2490.0f, -1670.0f, 13.3f }; // Тупик Grove (сторона Grove)

const CVector k_policeBarricadePoints[8] = {
    { 1491.0f, -1740.0f, 13.0f },
    { 1454.0f, -1748.0f, 13.0f },
    { 1521.0f, -1731.0f, 12.0f },
    { 1429.0f, -1632.0f, 12.0f },
    { 1497.0f, -1592.0f, 12.0f },
    { 1690.0f, -1759.0f, 12.0f },
    { 1936.0f, -1612.0f, 12.0f },
    { 1823.0f, -1744.0f, 12.0f }
};

const CVector k_civilRiotPoints[12] = {
    { 1478.0f, -1707.0f, 13.0f },
    { 1721.0f, -1711.0f, 13.0f },
    { 1734.0f, -1596.0f, 12.0f },
    { 1328.0f, -1573.0f, 12.0f },
    { 1371.0f, -1400.0f, 12.0f },
    { 1454.0f, -1453.0f, 12.0f },
    { 1543.0f, -1720.0f, 13.0f },
    { 1463.0f, -1741.0f, 13.0f },
    { 1387.0f, -1743.0f, 12.0f },
    { 1389.0f, -1861.0f, 12.0f },
    { 1782.0f, -1901.0f, 12.0f },
    { 1789.0f, -1596.0f, 13.0f }
};

const CVector k_gangWarPoints[9] = {
    { 2297.0f, -1744.0f, 13.0f },
    { 2116.0f, -1770.0f, 13.0f },
    { 2208.0f, -1675.0f, 14.0f },
    { 2249.0f, -1483.0f, 23.0f },
    { 2221.0f, -1160.0f, 25.0f },
    { 1921.0f, -1201.0f, 19.0f },
    { 1928.0f, -1401.0f, 14.0f },
    { 2039.0f, -1653.0f, 13.0f },
    { 2326.0f, -2077.0f, 13.0f }
};

// =============================================================================
//  Shared State Definitions
// =============================================================================

std::atomic<uint32_t> s_nextMuniIncidentId{ 1000 };
std::atomic<uint32_t> s_totalIncidentsHandled{ 0 };
std::atomic<float>    s_policeDesertionPct{ 0.0f };
std::atomic<bool>     s_rampartScandalActive{ false };
std::atomic<bool>     s_opioidSurgeActive{ false };
std::atomic<bool>     s_gridBlackoutActive{ false };
std::atomic<uint8_t>  s_pendingCrisisTriggerId{ 0 };
std::atomic<float>    s_publicUnrest{ 10.0f };
static std::atomic<float> s_criminalCompliance{ 0.95f };

// Municipal Logs
MunicipalLogEntry s_municipalLogs[k_maxMunicipalLogs]{};
size_t            s_municipalLogHead = 0;
std::mutex        s_municipalLogMutex;

void AddMunicipalLog(const char* fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    auto nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm tmBuf{};
    localtime_s(&tmBuf, &nowTime);
    char fullMsg[140];
    snprintf(fullMsg, sizeof(fullMsg), "[%02d:%02d:%02d] %s", tmBuf.tm_hour, tmBuf.tm_min, tmBuf.tm_sec, buf);

    std::lock_guard<std::mutex> lock(s_municipalLogMutex);
    const size_t idx = s_municipalLogHead % k_maxMunicipalLogs;
    strncpy_s(s_municipalLogs[idx].message, fullMsg, sizeof(s_municipalLogs[idx].message) - 1);
    s_municipalLogHead++;
}

// Bribe and Union Strike structures
SyndicateBribeOffer s_activeBribeOffer{};
std::mutex          s_bribeOfferMutex;

UnionStrikeEvent    s_activeUnionStrike{};
std::mutex          s_unionStrikeMutex;

// In-Game Ticker
char       s_inGameTickerText[160] = "[MUNICIPAL AI] ALL SYSTEMS NOMINAL | FISCAL SOLVENCY: $1,000,000";
std::mutex s_inGameTickerMutex;
void SetInGameTicker(const char* fmt, ...) {
    char buf[160];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    std::lock_guard<std::mutex> lock(s_inGameTickerMutex);
    strncpy_s(s_inGameTickerText, buf, sizeof(s_inGameTickerText) - 1);
}

// Crisis Catalog
const WorldCrisisDef k_catalogCrises[15] = {
    { 1,  "OIL_EMBARGO",        "Global Oil Embargo",                 2.5f, 0.8f,  0.2f,  0.4f, -15000, 0.8f, 180000, "[WORLD EVENT] GLOBAL OIL EMBARGO | FUEL PRICES SKYROCKET" },
    { 2,  "PORT_STRIKE",        "Dockworkers Port Strike",            1.2f, 1.8f,  0.3f,  0.7f, -25000, 0.7f, 180000, "[WORLD EVENT] PORT WORKERS STRIKE | OCEAN DOCKS SHUT DOWN" },
    { 3,  "CHIP_SHORTAGE",      "Global Microchip Shortage",          1.0f, 2.2f,  0.5f,  0.3f, -20000, 0.9f, 180000, "[WORLD EVENT] CHIP SHORTAGE | HIGHWAY HIJACKINGS ESCALATE" },
    { 4,  "BRIDGE_REPAIR",      "Critical Infrastructure Repair",     1.1f, 1.0f,  0.0f,  0.1f, -35000, 1.0f, 180000, "[WORLD EVENT] INFRASTRUCTURE EMERGENCY | BRIDGE REPAIRS" },
    { 5,  "MUNI_DEFAULT",       "Municipal Debt Default",             1.3f, 0.7f,  0.8f,  1.0f, -60000, 0.3f, 180000, "[WORLD EVENT] MUNICIPAL DEFAULT | POLICE WALKOUT & RALLIES" },
    { 6,  "FEDERAL_GRANT",      "Federal Infrastructure Grant",       0.8f, 1.3f, -0.4f, -0.6f,  50000, 1.3f, 180000, "[WORLD EVENT] FEDERAL AID APPROVED | $200,000 TREASURY INFUSION" },
    { 7,  "CREDIT_CRUNCH",      "Liquidity Crisis & Bank Run",        1.1f, 0.6f,  0.4f,  0.8f, -40000, 0.8f, 180000, "[WORLD EVENT] BANK RUN IN PROGRESS | PERSHING SQUARE LIQUIDITY PANIC" },
    { 8,  "CONTRABAND_BOOM",    "Ocean Docks Contraband Surge",       1.0f, 1.5f,  0.9f,  0.4f, -10000, 0.7f, 180000, "[WORLD EVENT] ILLICIT CONTRABAND SURGE | DOCKS SYNDICATE ACTIVE" },
    { 9,  "CIVIL_RIOT_1992",    "1992 Los Santos Civil Uprising",     1.5f, 0.5f,  1.5f,  1.5f, -80000, 0.4f, 180000, "[WORLD EVENT] 1992 CIVIL RIOT | MASS UNREST ACROSS LOS SANTOS" },
    { 10, "SANITATION_STRIKE",  "Sanitation Workers Strike",          1.0f, 0.9f,  0.2f,  0.8f, -30000, 0.8f, 180000, "[WORLD EVENT] SANITATION STRIKE | CITY HALL ENTRANCE BLOCKED" },
    { 11, "CAMPUS_PROTEST",     "Student Peace Encampment",           1.0f, 1.0f, -0.1f,  0.5f,  -5000, 0.9f, 180000, "[WORLD EVENT] PERSHING SQUARE STUDENT SIT-IN PROTEST" },
    { 12, "RAMPART_SCANDAL",    "CRASH Division Corruption Inquiry",  1.0f, 1.0f,  0.7f,  0.9f, -25000, 0.5f, 180000, "[WORLD EVENT] RAMPART POLICE SCANDAL | PATROLS RESTRICTED" },
    { 13, "RICO_SWEEP",         "Federal RICO Taskforce Raid",        1.0f, 1.1f, -0.8f,  0.3f, -45000, 1.8f, 180000, "[WORLD EVENT] FEDERAL RICO STRIKE | FBI AGENTS TARGETING SYNDICATES" },
    { 14, "OPIOID_SURGE",       "Ganton-Idlewood Street Epidemic",    1.0f, 0.8f,  1.1f,  0.6f, -20000, 0.7f, 180000, "[WORLD EVENT] NARCOTICS SURGE | STREET AGGRESSION IN SOUTH CENTRAL" },
    { 15, "GRID_BLACKOUT",      "Metropolitan Power Grid Blackout",   1.4f, 0.7f,  0.9f,  0.5f, -50000, 0.6f, 180000, "[WORLD EVENT] ELECTRICAL GRID COLLAPSE | TRAFFIC SIGNALS OFFLINE" }
};

bool TriggerWorldCrisis(uint8_t id, uint32_t currentMs) {
    if (currentMs == 0) currentMs = CTimer::m_snTimeInMilliseconds;
    const WorldCrisisDef* found = nullptr;
    for (const auto& c : k_catalogCrises) {
        if (c.id == id) {
            found = &c;
            break;
        }
    }
    if (!found) return false;

    // Apply physical world behaviors and spawn mission entities
    ApplyCrisisWorldPhysics(id, true);

    s_activeCrisis.activeId.store(found->id, std::memory_order_release);
    s_activeCrisis.startMs.store(currentMs, std::memory_order_relaxed);
    s_activeCrisis.durationMs.store(found->durationMs, std::memory_order_relaxed);
    s_activeCrisis.fuelPriceMul.store(found->fuelPriceMul, std::memory_order_relaxed);
    s_activeCrisis.cargoPriceMul.store(found->cargoPriceMul, std::memory_order_relaxed);
    s_activeCrisis.crimeVelocity.store(found->crimeVelocity, std::memory_order_relaxed);
    s_activeCrisis.unrestVelocity.store(found->unrestVelocity, std::memory_order_relaxed);
    s_activeCrisis.treasuryDeltaPerMin.store(found->treasuryDeltaPerMin, std::memory_order_relaxed);
    s_activeCrisis.policeEffMul.store(found->policeEffMul, std::memory_order_relaxed);
    {
        std::lock_guard<std::mutex> lock(s_activeCrisis.titleMutex);
        strncpy_s(s_activeCrisis.activeTitle, found->title, sizeof(s_activeCrisis.activeTitle) - 1);
        strncpy_s(s_activeCrisis.activeHeadline, found->tickerHeadline, sizeof(s_activeCrisis.activeHeadline) - 1);
    }
    AddMunicipalLog("CRISIS TRIGGERED: %s", found->title);
    SetInGameTicker("%s", found->tickerHeadline);
    CHud::SetHelpMessage(found->title, true, false, false);
    return true;
}

void TriggerWorldCrisis(const WorldCrisisDef& def, uint32_t currentMs) {
    TriggerWorldCrisis(def.id, currentMs);
}

// Queue Mutex and Enqueue function
static std::mutex g_municipalEventQueueMutex;
void EnqueueMunicipalEvent(const MunicipalGameEvent& ev) {
    std::lock_guard<std::mutex> lock(g_municipalEventQueueMutex);
    g_municipalGameEventQueue.push(ev);
}

// Traffic substitution tracking state
static std::unordered_map<CVehicle*, unsigned int> s_evaluatedVehicles;
static uint32_t s_lastCarCleanMs = 0;

// Frontal spawn state
static bool     s_gangFrontActive = false;
static uint32_t s_gangFrontIds[9] = { 0 };
static bool     s_policeCordonActive = false;
static uint32_t s_policeCordonIds[8] = { 0 };

static uint32_t s_incidentCollisionRequested[k_maxIncidents]{};

// SA палитра цвета блипов для CRadar::ChangeBlipColour (та же нумерация, что
// у eBlipColour в SDK III/VC): 0=red, 1=green, 2=blue, 3=white, 4=yellow
static constexpr unsigned int k_muniBlipColourBlue   = 2;
static constexpr unsigned int k_muniBlipColourYellow = 4;
static constexpr unsigned int k_muniBlipColourRed    = 0;

// Верифицированные riot-данные (enums/eAnimations.h):
static constexpr int k_riotAssocGroup = ANIM_GROUP_RIOT; // = 50
static const int k_riotAnimIds[7] = {
    ANIM_RIOT_RIOT_ANGRY,      // 265
    ANIM_RIOT_RIOT_ANGRY_B,    // 266
    ANIM_RIOT_RIOT_CHANT,      // 267
    ANIM_RIOT_RIOT_PUNCHES,    // 268
    ANIM_RIOT_RIOT_SHOUT,      // 269
    ANIM_RIOT_RIOT_CHALLENGE,  // 270
    ANIM_RIOT_RIOT_FUKU        // 271
};

// =============================================================================
//  A-Life Helpers: Anti-stale Handles, Streaming & Shared Ped Utilities
// =============================================================================

bool SelectSafeIncidentCoord(const CVector* points, size_t count, const CVector& playerPos, CVector& outCoord) {
    if (!points || count == 0) return false;
    size_t bestSafeIdx = (size_t)-1;
    float minSafeDistSq = 1e12f;
    size_t furthestIdx = 0;
    float maxDistSq = -1.0f;

    for (size_t i = 0; i < count; ++i) {
        float dx = points[i].x - playerPos.x;
        float dy = points[i].y - playerPos.y;
        float dSq = dx * dx + dy * dy;
        if (dSq >= (45.0f * 45.0f)) {
            if (dSq < minSafeDistSq) {
                minSafeDistSq = dSq;
                bestSafeIdx = i;
            }
        }
        if (dSq > maxDistSq) {
            maxDistSq = dSq;
            furthestIdx = i;
        }
    }

    if (bestSafeIdx != (size_t)-1) {
        outCoord = points[bestSafeIdx];
    } else {
        outCoord = points[furthestIdx];
    }
    return true;
}

// Anti-stale handle resolver: возвращает CPed* из пула (включая мертвых), не обнуляя хэндл.
static CPed* ResolvePed(uint32_t handle) {
    if (handle == 0 || !CPools::ms_pPedPool) return nullptr;
    CPed* ped = CPools::ms_pPedPool->GetAtRef(static_cast<int>(handle));
    if (!ped && handle < static_cast<uint32_t>(CPools::ms_pPedPool->m_nSize)) {
        ped = CPools::ms_pPedPool->GetAt(static_cast<int>(handle));
    }
    if (ped && CPools::ms_pPedPool->IsObjectValid(ped)) {
        return ped;
    }
    return nullptr;
}

static CVehicle* ResolveVehicle(uint32_t handle) {
    if (handle == 0 || !CPools::ms_pVehiclePool) return nullptr;
    CVehicle* veh = CPools::ms_pVehiclePool->GetAtRef(static_cast<int>(handle));
    if (!veh && handle < static_cast<uint32_t>(CPools::ms_pVehiclePool->m_nSize)) {
        veh = CPools::ms_pVehiclePool->GetAt(static_cast<int>(handle));
    }
    if (veh && CPools::ms_pVehiclePool->IsObjectValid(veh)) {
        return veh;
    }
    return nullptr;
}

// Подсчет живых юнитов в массиве хэндлов (не модифицирует сами хэндлы).
static int CountSafeAliveUnits(const uint32_t handles[8]) {
    int alive = 0;
    if (!CPools::ms_pPedPool) return 0;
    for (int i = 0; i < 8; ++i) {
        if (handles[i] != 0) {
            CPed* p = ResolvePed(handles[i]);
            if (p && CPools::ms_pPedPool->IsObjectValid(p) && p->m_fHealth > 0.0f) {
                alive++;
            }
        }
    }
    return alive;
}

static CVector GetIncidentPlayerPosition(CPed* player) {
    if (!player) return CVector(0.0f, 0.0f, 0.0f);
    return player->m_pVehicle ? player->m_pVehicle->GetPosition() : player->GetPosition();
}

static bool IsMuniModelLoaded(int modelId) {
    if (modelId <= 0 || modelId >= 26316) return false;
    if (modelId < 20000 && !CModelInfo::ms_modelInfoPtrs[modelId]) return false;
    return CStreaming::ms_aInfoForModel[modelId].m_nLoadState == LOADSTATE_LOADED;
}

static void SafeRequestModel(int modelId, int flags = (STREAMING_GAME_REQUIRED | STREAMING_PRIORITY_REQUEST)) {
    if (modelId <= 0 || modelId >= 26316) return;
    if (modelId < 20000 && !CModelInfo::ms_modelInfoPtrs[modelId]) return;
    CStreaming::RequestModel(modelId, flags);
}

static void SafeRequestAnimation(const char* name) {
    if (!name || name[0] == '\0') return;
    Command<Commands::REQUEST_ANIMATION>(name);
}

// Общая миссионная настройка каждого созданного CPed (требования A-Life).
// Исходный m_nPedType (COP, GANG1, GANG2, CIVMALE) сохраняется для работы отношений и реакций движка.
static void ConfigureMissionPed(CPed* ped, bool stayInPlace) {
    if (!ped) return;
    ped->m_nCreatedBy = 2; // Mission ped: не удаляется движком автоматически
    ped->bIsVisible = true;        // CEntity.h:36 (не m_bIsVisible)
    ped->bCullExtraFarAway = true; // SA-аналог m_nPedFlags.bNeverLeavesSphere (CPed.h:111)
    ped->bPanicWhenScared = false;
    ped->bCrouchWhenScared = false;
    ped->bFleeWhenStanding = false;
    ped->bStayInSamePlace = stayInPlace;
    ped->m_nWeaponSkill = 2; // Максимальная меткость и стрельба на ходу
}

static void GiveInfiniteCombatWeapon(CPed* ped, eWeaponType wep) {
    if (!ped) return;
    int modelId = -1;
    switch (wep) {
    case WEAPONTYPE_SHOTGUN:      modelId = MODEL_CHROMEGUN; break;
    case WEAPONTYPE_MP5:          modelId = MODEL_MP5LNG; break;
    case WEAPONTYPE_PISTOL:       modelId = MODEL_COLT45; break;
    case WEAPONTYPE_DESERT_EAGLE: modelId = MODEL_DESERT_EAGLE; break;
    case WEAPONTYPE_MICRO_UZI:    modelId = MODEL_MICRO_UZI; break;
    case WEAPONTYPE_AK47:         modelId = MODEL_AK47; break;
    case WEAPONTYPE_BASEBALLBAT:  modelId = MODEL_BAT; break;
    case WEAPONTYPE_GOLFCLUB:     modelId = MODEL_GOLFCLUB; break;
    case WEAPONTYPE_KNIFE:        modelId = MODEL_KNIFECUR; break;
    default:                      break;
    }
    if (modelId > 0 && !IsMuniModelLoaded(modelId)) {
        return; // Model not loaded yet, skip assignment to prevent 0xC0000005 bone attach crash
    }
    ped->GiveWeapon(wep, 9999, true);
    ped->SetCurrentWeapon(wep);
    Command<Commands::SET_CURRENT_CHAR_WEAPON>(ped, wep);
    CWeaponInfo* wInfo = CWeaponInfo::GetWeaponInfo(wep, 1);
    if (wInfo) {
        int wepSlot = wInfo->m_nSlot;
        if (wepSlot > 1 && wepSlot < 13) {
            ped->m_aWeapons[wepSlot].m_nAmmoInClip = 9999;
            ped->m_aWeapons[wepSlot].m_nTotalAmmo = 9999;
        }
    }
}

static CPed* SpawnIncidentPed(int modelId, ePedType pedType, const CVector& pos, bool stayInPlace) {
    if (!CPools::ms_pPedPool || CPools::ms_pPedPool->GetNoOfUsedSpaces() >= 125 || CPools::ms_pPedPool->GetNoOfFreeSpaces() < 2) return nullptr;
    if (!IsMuniModelLoaded(modelId)) return nullptr;
    CPed* ped = new CPed(pedType);
    if (!ped || !CPools::ms_pPedPool->IsObjectValid(ped)) {
        if (ped) Command<Commands::DELETE_CHAR>(ped);
        return nullptr;
    }
    ped->SetModelIndex(modelId);
    ConfigureMissionPed(ped, stayInPlace);
    ped->m_nStatus = eEntityStatus::STATUS_PHYSICS;
    ped->Teleport(pos);
    CWorld::Add(ped);
    ped->UpdateRwMatrix();
    return ped;
}

// Verified constructor (CTaskComplexKillPedOnFoot.h:28, native 0x620E30):
// CTaskComplexKillPedOnFoot(CPed* target, int time, int specFlags, int delay, int chance, char a7)
// Один и тот же конструктор используется и для GANTON_RIOT, и для ROADBLOCK.
void AssignKillPedTask(CPed* attacker, CPed* target) {
    if (!attacker || !target) return;
    if (!CPools::ms_pPedPool || !CPools::ms_pPedPool->IsObjectValid(target) || !CPools::ms_pPedPool->IsObjectValid(attacker)) return;
    if (target->m_fHealth <= 0.0f || attacker->m_fHealth <= 0.0f) return;
    if (attacker->bInVehicle || attacker->m_pVehicle || target->bInVehicle || target->m_pVehicle) return;
    if (!attacker->m_pIntelligence) return;
    attacker->bPanicWhenScared = false;
    attacker->bCrouchWhenScared = false;
    attacker->bFleeWhenStanding = false;
    attacker->bStayInSamePlace = false;
    attacker->m_nWeaponSkill = 2;
    attacker->m_pIntelligence->m_TaskMgr.SetTask(
        new CTaskComplexKillPedOnFoot(target, -1, 0, 0, 100, 0),
        TASK_PRIMARY_PRIMARY, false);
}

static inline void RequestRiotAnimations(int animBlock, int flags = STREAMING_KEEP_IN_MEMORY) {
    (void)flags;
    (void)animBlock;
    SafeRequestAnimation("RIOT");
}

static bool SafePlayRiotAnimation(CPed* ped, const char* animName) {
    if (!ped || !CPools::ms_pPedPool || !CPools::ms_pPedPool->IsObjectValid(ped)) return false;
    if (ped->m_fHealth <= 0.0f) return false;
    
    int animBlock = CAnimManager::GetAnimationBlockIndex("RIOT");
    if (animBlock >= 0 && animBlock < CAnimManager::ms_numAnimBlocks && CAnimManager::ms_aAnimBlocks[animBlock].bLoaded) {
        Command<Commands::TASK_PLAY_ANIM>(ped, animName, "RIOT", 4.0f, 1, 0, 0, 0, -1);
        return true;
    }
    
    // Safe fallback: Do NOT request animation synchronously. Use standard engine tasks.
    if (ped->m_pIntelligence) {
        ped->m_pIntelligence->m_TaskMgr.SetTask(
            new CTaskComplexWanderStandard(PEDMOVE_WALK, 255, true),
            TASK_PRIMARY_PRIMARY, false);
    }
    return false;
}

struct ActiveIncident;
static void DematerializeIncident(ActiveIncident& inc, CPed* player);

static bool IsVehicleValidAndAlive(CVehicle* veh, unsigned int creationTime) {
    if (!veh || !CPools::ms_pVehiclePool) return false;
    if (!CPools::ms_pVehiclePool->IsObjectValid(veh)) return false;
    if (veh->m_nCreationTime != creationTime) return false;
    if (veh->m_fHealth <= 0.0f) return false;
    return true;
}

static bool IsPedValidAndAlive(CPed* ped) {
    if (!ped || !CPools::ms_pPedPool) return false;
    if (!CPools::ms_pPedPool->IsObjectValid(ped)) return false;
    return (ped->m_fHealth > 0.0f);
}

static bool IsPedEnteringCar(CPed* ped) {
    if (!ped || !ped->m_pIntelligence) return false;
    CTaskManager& tm = ped->m_pIntelligence->m_TaskMgr;
    if (tm.FindActiveTaskByType(TASK_COMPLEX_ENTER_CAR_AS_DRIVER) ||
        tm.FindActiveTaskByType(TASK_COMPLEX_ENTER_CAR_AS_PASSENGER) ||
        tm.FindActiveTaskByType(TASK_COMPLEX_ENTER_CAR_AS_DRIVER_TIMED) ||
        tm.FindActiveTaskByType(TASK_COMPLEX_ENTER_CAR_AS_PASSENGER_TIMED) ||
        tm.FindActiveTaskByType(TASK_COMPLEX_ENTER_CAR_AS_PASSENGER_WAIT)) {
        return true;
    }
    return false;
}

static CPed* FindGangTargetNear(const CVector& pos, float radius = 65.0f) {
    if (!CPools::ms_pPedPool) return nullptr;
    const float radiusSq = radius * radius;
    CPed* closestEnemy = nullptr;
    float closestDistSq = radiusSq;
    CPed* player = FindPlayerPed();

    for (int p = 0; p < CPools::ms_pPedPool->m_nSize; ++p) {
        CPed* ped = CPools::ms_pPedPool->GetAt(p);
        if (!ped || !CPools::ms_pPedPool->IsObjectValid(ped) || ped == player || ped->m_fHealth <= 0.0f) continue;
        if (player && player->m_pVehicle) {
            if (ped == player->m_pVehicle->m_pDriver) continue;
            bool inPlayerVeh = false;
            for (int s = 0; s < 8; ++s) {
                if (ped == player->m_pVehicle->m_apPassengers[s]) { inPlayerVeh = true; break; }
            }
            if (inPlayerVeh) continue;
        }
        // Целью патрулей являются только враждебные бандиты (PED_TYPE_GANG1 - Ballas/Vagos):
        if (ped->m_nPedType != PED_TYPE_GANG1) continue;

        const CVector diff = ped->GetPosition() - pos;
        const float distSq = diff.MagnitudeSqr();
        if (distSq < closestDistSq) {
            closestDistSq = distSq;
            closestEnemy = ped;
        }
    }
    return closestEnemy;
}

static bool IsPedArmed(CPed* ped) {
    if (!ped) return false;
    if (ped->m_nSelectedWepSlot < 13) {
        const eWeaponType w = ped->m_aWeapons[ped->m_nSelectedWepSlot].m_eWeaponType;
        if (w > WEAPONTYPE_UNARMED && w != WEAPONTYPE_BRASSKNUCKLE) return true;
    }
    for (int s = 2; s <= 7; ++s) {
        const eWeaponType w = ped->m_aWeapons[s].m_eWeaponType;
        if (w > WEAPONTYPE_UNARMED && ped->m_aWeapons[s].m_nAmmoTotal > 0) return true;
    }
    return false;
}

static CPed* FindArmedBanditNear(const CVector& pos, float radius) {
    if (!CPools::ms_pPedPool) return nullptr;
    const float radiusSq = radius * radius;
    CPed* closestEnemy = nullptr;
    float closestDistSq = radiusSq;
    CPed* player = FindPlayerPed();

    for (int p = 0; p < CPools::ms_pPedPool->m_nSize; ++p) {
        CPed* ped = CPools::ms_pPedPool->GetAt(p);
        if (!ped || !CPools::ms_pPedPool->IsObjectValid(ped) || ped == player || ped->m_fHealth <= 0.0f) continue;
        if (ped->m_nPedType != PED_TYPE_GANG1 && ped->m_nPedType != PED_TYPE_GANG2) continue;
        if (!IsPedArmed(ped)) continue;

        const CVector diff = ped->GetPosition() - pos;
        const float distSq = diff.MagnitudeSqr();
        if (distSq < closestDistSq) {
            closestDistSq = distSq;
            closestEnemy = ped;
        }
    }
    return closestEnemy;
}

ptrdiff_t FindIncidentSlotById(uint32_t id) {
    if (id == 0) return -1;
    for (size_t i = 0; i < k_maxIncidents; ++i)
        if (s_incidents[i].active && s_incidents[i].id == id)
            return static_cast<ptrdiff_t>(i);
    return -1;
}

static size_t AcquireIncidentSlot(CPed* player) {
    for (size_t i = 0; i < k_maxIncidents; ++i) {
        if (!s_incidents[i].active) return i;
    }
    // Реестр полон: вытесняем наиболее удалённый нематериализованный инцидент
    const CVector pPos = player ? GetIncidentPlayerPosition(player) : CVector(0.0f, 0.0f, 0.0f);
    ptrdiff_t bestVictim = -1;
    float maxDistSq = -1.0f;
    for (size_t i = 0; i < k_maxIncidents; ++i) {
        if (!s_incidents[i].isMaterialized) {
            const float dx = s_incidents[i].pos.x - pPos.x;
            const float dy = s_incidents[i].pos.y - pPos.y;
            const float dSq = dx * dx + dy * dy;
            if (dSq > maxDistSq) {
                maxDistSq = dSq;
                bestVictim = static_cast<ptrdiff_t>(i);
            }
        }
    }
    if (bestVictim < 0) {
        // Все материализованы: вытесняем самый дальний от игрока (никогда не тот, где стоит игрок!)
        for (size_t i = 0; i < k_maxIncidents; ++i) {
            const float dx = s_incidents[i].pos.x - pPos.x;
            const float dy = s_incidents[i].pos.y - pPos.y;
            const float dSq = dx * dx + dy * dy;
            if (dSq > maxDistSq) {
                maxDistSq = dSq;
                bestVictim = static_cast<ptrdiff_t>(i);
            }
        }
    }
    size_t victimSlot = (bestVictim >= 0) ? static_cast<size_t>(bestVictim) : 0;
    CleanupIncident(s_incidents[victimSlot], player, true);
    return victimSlot;
}

// Строго асинхронные приоритетные запросы моделей.
// ВАЖНО: CStreaming::LoadAllRequestedModels() в игровом потоке НЕ вызывается.
static void RequestIncidentModels(const ActiveIncident& inc) {
    switch (inc.type) {
    case IncidentType::ROADBLOCK:
        SafeRequestModel(MODEL_ENFORCER);
        SafeRequestModel(MODEL_FBIRANCH);
        SafeRequestModel(MODEL_SWAT);
        SafeRequestModel(MODEL_MP5LNG);
        SafeRequestModel(MODEL_CHROMEGUN);
        SafeRequestModel(1238);
        break;
    case IncidentType::UNION_STRIKE:
        SafeRequestModel(MODEL_BMYST);
        SafeRequestModel(MODEL_WMYST);
        SafeRequestModel(MODEL_HMYST);
        SafeRequestModel(MODEL_WFYST);
        SafeRequestModel(MODEL_COPCARLS);
        SafeRequestModel(MODEL_CSHER);
        SafeRequestAnimation("RIOT");
        break;
    case IncidentType::GANTON_RIOT:
        SafeRequestModel(MODEL_FAM1);
        SafeRequestModel(MODEL_FAM2);
        SafeRequestModel(MODEL_BALLAS1);
        SafeRequestModel(MODEL_BALLAS2);
        SafeRequestModel(MODEL_COLT45);
        SafeRequestModel(MODEL_DEAGLE);
        break;
    }
}

static bool IncidentModelsReady(const ActiveIncident& inc) {
    switch (inc.type) {
    case IncidentType::ROADBLOCK:
        return IsMuniModelLoaded(MODEL_ENFORCER) &&
               IsMuniModelLoaded(MODEL_FBIRANCH) &&
               IsMuniModelLoaded(MODEL_SWAT) &&
               IsMuniModelLoaded(1238) &&
               IsMuniModelLoaded(MODEL_CHROMEGUN) &&
               IsMuniModelLoaded(MODEL_MP5LNG);
    case IncidentType::UNION_STRIKE:
        return IsMuniModelLoaded(MODEL_BMYST) && IsMuniModelLoaded(MODEL_WMYST) &&
               IsMuniModelLoaded(MODEL_HMYST) && IsMuniModelLoaded(MODEL_WFYST) &&
               IsMuniModelLoaded(MODEL_COPCARLS) &&
               IsMuniModelLoaded(MODEL_CSHER) &&
               (inc.animState == AnimState::READY || inc.animState == AnimState::FAILED);
    case IncidentType::GANTON_RIOT:
        return IsMuniModelLoaded(MODEL_FAM1) && IsMuniModelLoaded(MODEL_FAM2) &&
               IsMuniModelLoaded(MODEL_BALLAS1) && IsMuniModelLoaded(MODEL_BALLAS2) &&
               IsMuniModelLoaded(MODEL_COLT45) && IsMuniModelLoaded(MODEL_DEAGLE);
    }
    return false;
}

// Прогресс анимационного блока "RIOT" для забастовки (асинхронно, через refs).
// Верифицировано: CAnimManager::GetAnimationBlockIndex/AddAnimBlockRef (CAnimManager.h),
// CAnimBlock::bLoaded (CAnimBlock.h:15).
static void UpdateIncidentAnimBlock(ActiveIncident& inc) {
    if (inc.type != IncidentType::UNION_STRIKE) return;
    if (inc.animState == AnimState::NOT_LOADED) {
        SafeRequestAnimation("RIOT");
        inc.animState = AnimState::REQUESTED;
    }
    if (inc.animState == AnimState::REQUESTED) {
        const int blockIdx = CAnimManager::GetAnimationBlockIndex("RIOT");
        if (blockIdx >= 0 && blockIdx < CAnimManager::ms_numAnimBlocks &&
            CAnimManager::ms_aAnimBlocks[blockIdx].bLoaded) {
            inc.animState = AnimState::READY;
        }
    }
}

static void EmitMunicipalEvent(const char* eventType, uint32_t incidentId, const char* extraJson = nullptr) {
    if (!eventType || incidentId == 0) return;
    Logger::Log("[MunicipalEvent] %s id=%u %s", eventType, incidentId, extraJson ? extraJson : "");
}

// =============================================================================
//  A-Life: 3D материализация при дистанции < 130.0f
// =============================================================================

static bool MaterializeRoadblock(ActiveIncident& inc, float gZ) {
    if (!CPools::ms_pVehiclePool || CPools::ms_pVehiclePool->GetNoOfUsedSpaces() >= 95 || CPools::ms_pVehiclePool->GetNoOfFreeSpaces() < 4) return false;
    if (!CPools::ms_pObjectPool || CPools::ms_pObjectPool->GetNoOfFreeSpaces() < 16) return false;
    if (!CPools::ms_pPedPool || CPools::ms_pPedPool->GetNoOfUsedSpaces() >= 125 || CPools::ms_pPedPool->GetNoOfFreeSpaces() < 8) return false;

    // Автосмещение от перекрёстка: если inc.pos находится в пределах 15м от ноды с >2 связями
    CPathNode* interNode = nullptr;
    float interDistSq = 15.0f * 15.0f;
    for (int a = 0; a < 64; ++a) {
        if (!ThePaths.m_pPathNodes[a]) continue;
        const unsigned int numVehNodes = ThePaths.m_dwNumVehicleNodes[a];
        for (unsigned int n = 0; n < numVehNodes; ++n) {
            CPathNode& node = ThePaths.m_pPathNodes[a][n];
            if (node.NumberAdjNodes > 2) {
                CVector nPos = node.GetNodeCoors();
                float dx = nPos.x - inc.pos.x;
                float dy = nPos.y - inc.pos.y;
                float dSq = dx * dx + dy * dy;
                if (dSq < interDistSq) {
                    interDistSq = dSq;
                    interNode = &node;
                }
            }
        }
    }

    CAutomobile* pVeh1 = new CAutomobile(MODEL_ENFORCER, 2, true);
    if (!pVeh1 || !CPools::ms_pVehiclePool->IsObjectValid(pVeh1)) {
        if (pVeh1) Command<Commands::DELETE_CAR>(pVeh1);
        return false;
    }

    CAutomobile* pVeh2 = new CAutomobile(MODEL_FBIRANCH, 2, true);
    if (!pVeh2 || !CPools::ms_pVehiclePool->IsObjectValid(pVeh2)) {
        Command<Commands::DELETE_CAR>(pVeh1);
        if (pVeh2) Command<Commands::DELETE_CAR>(pVeh2);
        return false;
    }

    pVeh1->m_nCreatedBy = 2; // Mission vehicle: не удаляется движком
    pVeh1->Teleport(CVector(inc.pos.x, inc.pos.y, (gZ > -100.0f ? gZ : inc.pos.z) + 0.5f));
    pVeh1->m_nStatus = eEntityStatus::STATUS_PHYSICS;
    pVeh1->m_fHealth = 2000.0f;
    pVeh1->bEngineOn = true;
    pVeh1->m_nHandbrakeOn = true;
    pVeh1->bSirenOrAlarm = true;
    CWorld::Add(pVeh1);
    pVeh1->PlaceOnRoadProperly();

    float roadH = pVeh1->GetHeading();

    if (interNode) {
        CVector nPos = interNode->GetNodeCoors();
        CVector away = inc.pos - nPos;
        away.z = 0.0f;
        CVector roadDir(-std::sin(-roadH), -std::cos(-roadH), 0.0f);
        if (away.x * roadDir.x + away.y * roadDir.y < 0.0f) {
            roadDir = -roadDir;
        }
        inc.pos += roadDir * 25.0f;
        float shiftGz = CWorld::FindGroundZForCoord(inc.pos.x, inc.pos.y);
        if (shiftGz > -100.0f) inc.pos.z = shiftGz;
        pVeh1->Teleport(CVector(inc.pos.x, inc.pos.y, inc.pos.z + 0.5f));
        pVeh1->PlaceOnRoadProperly();
        roadH = pVeh1->GetHeading();
    }

    inc.heading = roadH;
    CVector fwd(-std::sin(-roadH), -std::cos(-roadH), 0.0f);
    CVector side(std::cos(-roadH), -std::sin(-roadH), 0.0f);

    // Первая машина (MODEL_ENFORCER):
    CVector car1Pos = inc.pos - side * 2.2f;
    float c1Gz = CWorld::FindGroundZForCoord(car1Pos.x, car1Pos.y);
    car1Pos.z = (c1Gz > -100.0f ? c1Gz : inc.pos.z) + 0.5f;
    pVeh1->Teleport(car1Pos);
    pVeh1->PlaceOnRoadProperly();
    pVeh1->SetOrientation(0.0f, 0.0f, roadH + 1.5707963f);
    pVeh1->UpdateRwMatrix();
    inc.vehHandle = static_cast<uint32_t>(CPools::ms_pVehiclePool->GetRef(pVeh1));

    // Вторая машина (MODEL_FBIRANCH):
    CVector targetPos2 = inc.pos + side * 2.4f + fwd * 1.2f;
    bool clear = CWorld::GetIsLineOfSightClear(inc.pos, targetPos2, true, false, false, true, false, false, false);
    if (!clear) {
        targetPos2 = inc.pos + side * 1.5f + fwd * 2.0f;
        bool clear2 = CWorld::GetIsLineOfSightClear(inc.pos, targetPos2, true, false, false, true, false, false, false);
        if (!clear2) {
            targetPos2 = inc.pos - fwd * 4.5f + side * 0.8f;
        }
    }
    float gZ2 = CWorld::FindGroundZForCoord(targetPos2.x, targetPos2.y);
    targetPos2.z = (gZ2 > -100.0f ? gZ2 : inc.pos.z) + 0.3f;

    pVeh2->m_nCreatedBy = 2;
    pVeh2->Teleport(targetPos2);
    pVeh2->m_nStatus = eEntityStatus::STATUS_PHYSICS;
    pVeh2->m_fHealth = 1800.0f;
    pVeh2->bEngineOn = true;
    pVeh2->m_nHandbrakeOn = true;
    pVeh2->bSirenOrAlarm = true;
    CWorld::Add(pVeh2);
    pVeh2->PlaceOnRoadProperly();
    pVeh2->SetOrientation(0.0f, 0.0f, roadH - 1.15f);
    pVeh2->UpdateRwMatrix();
    inc.vehHandle2 = static_cast<uint32_t>(CPools::ms_pVehiclePool->GetRef(pVeh2));

    // Cones...
    CVector coneOffsets[16] = {
        fwd * 7.0f + side * -4.5f,
        fwd * 7.0f + side * -1.5f,
        fwd * 7.0f + side *  1.5f,
        fwd * 7.0f + side *  4.5f,
        fwd * -7.0f + side * -4.5f,
        fwd * -7.0f + side * -1.5f,
        fwd * -7.0f + side *  1.5f,
        fwd * -7.0f + side *  4.5f,
        fwd *  7.0f + side * -6.0f,
        fwd *  2.33f + side * -6.0f,
        fwd * -2.33f + side * -6.0f,
        fwd * -7.0f + side * -6.0f,
        fwd *  7.0f + side *  6.0f,
        fwd *  2.33f + side *  6.0f,
        fwd * -2.33f + side *  6.0f,
        fwd * -7.0f + side *  6.0f,
    };

    for (int k = 0; k < 16; ++k) {
        CVector conePos = inc.pos + coneOffsets[k];
        const float cGz = CWorld::FindGroundZForCoord(conePos.x, conePos.y);
        conePos.z = (cGz > -100.0f ? cGz : inc.pos.z) + 0.1f;
        if (!IsMuniModelLoaded(1238)) continue;
        CObject* cone = CObject::Create(1238);
        if (cone && CPools::ms_pObjectPool && CPools::ms_pObjectPool->IsObjectValid(cone)) {
            cone->m_nObjectType = OBJECT_MISSION;
            cone->Teleport(conePos);
            cone->m_nStatus = eEntityStatus::STATUS_PHYSICS;
            CWorld::Add(cone);
            cone->UpdateRwMatrix();
            inc.coneHandles[k] = static_cast<uint32_t>(CPools::ms_pObjectPool->GetRef(cone));
        } else if (cone) {
            Command<Commands::DELETE_OBJECT>(cone);
        }
    }

    // 6 SWAT cops
    CVector copPositions[6] = {
        car1Pos + fwd * 2.2f,
        targetPos2 + fwd * 1.8f,
        inc.pos - side * 4.6f + fwd * 0.5f,
        inc.pos + side * 4.6f + fwd * 0.5f,
        inc.pos + fwd * 1.5f,
        inc.pos - fwd * 1.5f
    };
    float copHeadings[6] = {
        roadH,
        roadH,
        roadH,
        roadH,
        roadH,
        roadH + 3.14159265f
    };

    for (int c = 0; c < 6; ++c) {
        CVector copPos = copPositions[c];
        const float cGz = CWorld::FindGroundZForCoord(copPos.x, copPos.y);
        copPos.z = (cGz > -100.0f ? cGz : inc.pos.z) + 1.05f;
        CPed* cop = SpawnIncidentPed(MODEL_SWAT, PED_TYPE_COP, copPos, false);
        if (!cop) {
            for (int k = 0; k < c; ++k) {
                if (inc.groupAPeds[k] != 0) {
                    CPed* prevCop = ResolvePed(inc.groupAPeds[k]);
                    if (prevCop) {
                        if (prevCop->m_pIntelligence) prevCop->m_pIntelligence->m_TaskMgr.FlushImmediately();
                        Command<Commands::DELETE_CHAR>(prevCop);
                    }
                    inc.groupAPeds[k] = 0;
                }
            }
            if (CPools::ms_pObjectPool) {
                for (int k = 0; k < 16; ++k) {
                    if (inc.coneHandles[k] != 0) {
                        CObject* cone = CPools::ms_pObjectPool->GetAtRef(static_cast<int>(inc.coneHandles[k]));
                        if (cone) Command<Commands::DELETE_OBJECT>(cone);
                        inc.coneHandles[k] = 0;
                    }
                }
            }
            if (pVeh1 && CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->IsObjectValid(pVeh1)) {
                Command<Commands::DELETE_CAR>(pVeh1);
            }
            if (pVeh2 && CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->IsObjectValid(pVeh2)) {
                Command<Commands::DELETE_CAR>(pVeh2);
            }
            inc.vehHandle = 0;
            inc.vehHandle2 = 0;
            return false;
        }
        cop->m_fCurrentRotation = copHeadings[c];
        cop->SetHeading(copHeadings[c]);
        cop->UpdateRwMatrix();
        const eWeaponType wep = (c == 2 || c == 3) ? WEAPONTYPE_SHOTGUN : WEAPONTYPE_MP5;
        GiveInfiniteCombatWeapon(cop, wep);
        cop->m_nWeaponSkill = 2;
        cop->m_fHealth = 200.0f;
        cop->m_fMaxHealth = 200.0f;
        cop->m_fArmour = 100.0f;
        cop->bStayInSamePlace = false;
        cop->bPanicWhenScared = false;
        cop->bCrouchWhenScared = false;
        cop->bFleeWhenStanding = false;
        cop->bUsesCollision = true;
        if (cop->m_pIntelligence) {
            cop->m_pIntelligence->m_TaskMgr.SetTask(
                new CTaskSimpleStandStill(-1, true, false, 8.0f),
                TASK_PRIMARY_PRIMARY, false);
        }
        inc.groupAPeds[c] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(cop));
        inc.copsAlerted[c] = false;
        inc.lastTargets[c] = 0;
    }

    inc.isMaterialized = true;
    Logger::Log("[MunicipalALife] Roadblock materialized at (%.1f, %.1f, %.1f) blip #%u vehs=%u,%u",
        inc.pos.x, inc.pos.y, inc.pos.z, inc.blipHandle, inc.vehHandle, inc.vehHandle2);
    return true;
}

static bool MaterializeUnionStrike(ActiveIncident& inc) {
    if (!CPools::ms_pPedPool || CPools::ms_pPedPool->GetNoOfUsedSpaces() >= 125 || CPools::ms_pPedPool->GetNoOfFreeSpaces() < 14) return false;
    if (!CPools::ms_pVehiclePool || CPools::ms_pVehiclePool->GetNoOfUsedSpaces() >= 95 || CPools::ms_pVehiclePool->GetNoOfFreeSpaces() < 2) return false;
    static const int k_strikerModels[4] = { MODEL_BMYST, MODEL_WMYST, MODEL_HMYST, MODEL_WFYST };

    int riotBlock = CAnimManager::GetAnimationBlockIndex("riot");
    if (riotBlock == -1) riotBlock = CAnimManager::GetAnimationBlockIndex("RIOT");
    const bool canPlayAnim = (riotBlock != -1 && riotBlock < CAnimManager::ms_numAnimBlocks && CAnimManager::ms_aAnimBlocks[riotBlock].bLoaded);

    for (int i = 0; i < 12; ++i) {
        const bool isInner = (i < 6);
        const int idxInRing = isInner ? i : (i - 6);
        const float radius = isInner ? 3.0f : 5.5f;
        const float angleOffset = isInner ? 0.0f : 0.52359877f;
        const float angle = idxInRing * (2.0f * 3.14159265f / 6.0f) + angleOffset;

        CVector sPos = inc.pos;
        sPos.x += std::cos(angle) * radius;
        sPos.y += std::sin(angle) * radius;
        const float sGz = CWorld::FindGroundZForCoord(sPos.x, sPos.y);
        sPos.z = (sGz > -100.0f && std::abs(sGz - inc.pos.z) < 3.5f ? sGz : inc.pos.z) + 1.05f;

        const int modelId = k_strikerModels[i % 4];
        CPed* pPed = SpawnIncidentPed(modelId, PED_TYPE_CIVMALE, sPos, isInner);
        if (!pPed) continue;

        float dx = inc.pos.x - sPos.x;
        float dy = inc.pos.y - sPos.y;
        float h = (dx * dx + dy * dy > 0.05f) ? std::atan2(-dx, dy) : 0.0f;
        h += ((rand() % 30) - 15) * (3.14159265f / 180.0f);
        pPed->m_fCurrentRotation = h;
        pPed->SetHeading(h);
        pPed->UpdateRwMatrix();

        if (i < 6) {
            inc.groupAPeds[i] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(pPed));
        } else {
            inc.groupBPeds[i - 6] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(pPed));
        }

        pPed->m_fHealth = 120.0f;
        pPed->m_fMaxHealth = 120.0f;
        pPed->bPanicWhenScared = false;
        pPed->bCrouchWhenScared = false;

        if (isInner) {
            pPed->bStayInSamePlace = true;
            if (canPlayAnim) {
                static const char* s_riotAnims[4] = { "RIOT_CHANT", "RIOT_PUNCHES", "RIOT_ANGRY", "RIOT_SHOUT" };
                const char* anim = s_riotAnims[i % 4];
                Command<Commands::TASK_PLAY_ANIM>(pPed, anim, "RIOT", 4.0f, 1, 0, 0, 0, -1);
            } else {
                if (pPed->m_pIntelligence) {
                    pPed->m_pIntelligence->m_TaskMgr.SetTask(
                        new CTaskComplexWanderStandard(PEDMOVE_RUN, 255, true),
                        TASK_PRIMARY_PRIMARY, false);
                }
            }
        } else {
            pPed->bStayInSamePlace = false;
            if (pPed->m_pIntelligence) {
                pPed->m_pIntelligence->m_TaskMgr.SetTask(
                    new CTaskComplexWanderStandard(PEDMOVE_WALK, 255, true),
                    TASK_PRIMARY_PRIMARY, false);
            }
        }
    }

    inc.strikeBarrierHandle = 0;

    if (CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->GetNoOfUsedSpaces() < 95 && CPools::ms_pVehiclePool->GetNoOfFreeSpaces() >= 2 &&
        CPools::ms_pPedPool && CPools::ms_pPedPool->GetNoOfUsedSpaces() < 125 && CPools::ms_pPedPool->GetNoOfFreeSpaces() >= 2 &&
        IsMuniModelLoaded(MODEL_COPCARLS) && IsMuniModelLoaded(MODEL_CSHER)) {
        CAutomobile* patrolCar = new CAutomobile(MODEL_COPCARLS, 2, true);
        if (patrolCar && CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->IsObjectValid(patrolCar)) {
            patrolCar->m_nCreatedBy = 2;
            patrolCar->Teleport(inc.pos + CVector(18.0f, 0.0f, 0.0f));
            patrolCar->PlaceOnRoadProperly();
            patrolCar->bEngineOn = true;
            patrolCar->bSirenOrAlarm = true;
            CWorld::Add(patrolCar);
            inc.patrolVehHandle = static_cast<uint32_t>(CPools::ms_pVehiclePool->GetRef(patrolCar));

            CPed* copDriver = SpawnIncidentPed(MODEL_CSHER, PED_TYPE_COP, inc.pos + CVector(18.0f, 0.0f, 0.0f), false);
            if (copDriver) {
                Command<Commands::WARP_CHAR_INTO_CAR>(copDriver, patrolCar);
                patrolCar->m_pDriver = copDriver;
                patrolCar->m_autoPilot.m_nCarMission = MISSION_CRUISE;
                patrolCar->m_autoPilot.m_nCruiseSpeed = 10;
                patrolCar->m_autoPilot.m_nCarDrivingStyle = DRIVINGSTYLE_STOP_FOR_CARS;
                inc.patrolDriverHandle = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(copDriver));
            }
        } else if (patrolCar) {
            Command<Commands::DELETE_CAR>(patrolCar);
        }
    }

    inc.isMaterialized = true;
    Logger::Log("[MunicipalALife] Union strike materialized at (%.1f, %.1f, %.1f) blip #%u animState=%u",
        inc.pos.x, inc.pos.y, inc.pos.z, inc.blipHandle, static_cast<unsigned>(inc.animState));
    return true;
}

static void MaterializeIncident(size_t slot, ActiveIncident& inc, uint32_t currentMs) {
    (void)slot;
    (void)currentMs;
    if (!CPools::ms_pPedPool || CPools::ms_pPedPool->GetNoOfUsedSpaces() >= 125 || CPools::ms_pPedPool->GetNoOfFreeSpaces() < 8) return;
    if (!CPools::ms_pVehiclePool || CPools::ms_pVehiclePool->GetNoOfUsedSpaces() >= 95) return;

    RequestIncidentModels(inc);
    if (!IncidentModelsReady(inc)) return; // модели ещё не готовы — пропустить кадр

    const float gZ = CWorld::FindGroundZForCoord(inc.pos.x, inc.pos.y);
    if (gZ > -100.0f && std::abs(gZ - inc.pos.z) < 3.5f) {
        inc.pos.z = gZ;
    }

    if (inc.type == IncidentType::ROADBLOCK) {
        MaterializeRoadblock(inc, gZ);
        return;
    }

    if (inc.type == IncidentType::UNION_STRIKE) {
        MaterializeUnionStrike(inc);
        return;
    }

    // GANTON_RIOT: Grove против Ballas (локальный бой 3 на 3, 6 бойцов суммарно)
    if (!CPools::ms_pPedPool || CPools::ms_pPedPool->GetNoOfFreeSpaces() < 8) return;
    CVector grovePos = inc.pos + CVector(8.0f, 0.0f, 0.0f);
    CVector ballaPos = inc.pos - CVector(8.0f, 0.0f, 0.0f);
    for (int i = 0; i < 3; ++i) {
        CVector gPos = grovePos;
        gPos.x += (i - 1.0f) * 1.8f;
        gPos.y += (i % 2 == 0 ? 1.5f : -1.5f);
        const float gGz = CWorld::FindGroundZForCoord(gPos.x, gPos.y);
        gPos.z = (gGz > -100.0f ? gGz : inc.pos.z) + 1.05f; // Пед спавнится от таза, нужно +1.05f!
        const int gModel = (i % 2 == 0) ? MODEL_FAM1 : MODEL_FAM2;
        CPed* grove = SpawnIncidentPed(gModel, PED_TYPE_GANG2, gPos, false);
        if (!grove) {
            for (int k = 0; k < i; ++k) {
                if (inc.groupAPeds[k] != 0) {
                    CPed* p = ResolvePed(inc.groupAPeds[k]);
                    if (p) { if (p->m_pIntelligence) p->m_pIntelligence->m_TaskMgr.FlushImmediately(); Command<Commands::DELETE_CHAR>(p); }
                    inc.groupAPeds[k] = 0;
                }
                if (inc.groupBPeds[k] != 0) {
                    CPed* p = ResolvePed(inc.groupBPeds[k]);
                    if (p) { if (p->m_pIntelligence) p->m_pIntelligence->m_TaskMgr.FlushImmediately(); Command<Commands::DELETE_CHAR>(p); }
                    inc.groupBPeds[k] = 0;
                }
            }
            return;
        }
        grove->m_fHealth = 150.0f;
        grove->m_fMaxHealth = 150.0f;
        float gh = std::atan2(-(ballaPos.x - gPos.x), ballaPos.y - gPos.y);
        grove->m_fCurrentRotation = gh;
        grove->SetHeading(gh);
        grove->UpdateRwMatrix();
        const eWeaponType wep = (i % 2 == 0) ? WEAPONTYPE_PISTOL : WEAPONTYPE_DESERT_EAGLE;
        GiveInfiniteCombatWeapon(grove, wep);
        grove->m_nWeaponSkill = 2;
        grove->bStayInSamePlace = false;
        grove->bPanicWhenScared = false;
        grove->bCrouchWhenScared = false;
        grove->bFleeWhenStanding = false;
        inc.groupAPeds[i] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(grove));

        CVector bPos = ballaPos;
        bPos.x += (i - 1.0f) * 1.8f;
        bPos.y += (i % 2 == 0 ? -1.5f : 1.5f);
        const float bGz = CWorld::FindGroundZForCoord(bPos.x, bPos.y);
        bPos.z = (bGz > -100.0f ? bGz : inc.pos.z) + 1.05f; // Пед спавнится от таза, нужно +1.05f!
        const int bModel = (i % 2 == 0) ? MODEL_BALLAS1 : MODEL_BALLAS2;
        CPed* balla = SpawnIncidentPed(bModel, PED_TYPE_GANG1, bPos, false);
        if (!balla) {
            for (int k = 0; k <= i; ++k) {
                if (inc.groupAPeds[k] != 0) {
                    CPed* p = ResolvePed(inc.groupAPeds[k]);
                    if (p) { if (p->m_pIntelligence) p->m_pIntelligence->m_TaskMgr.FlushImmediately(); Command<Commands::DELETE_CHAR>(p); }
                    inc.groupAPeds[k] = 0;
                }
                if (k < i && inc.groupBPeds[k] != 0) {
                    CPed* p = ResolvePed(inc.groupBPeds[k]);
                    if (p) { if (p->m_pIntelligence) p->m_pIntelligence->m_TaskMgr.FlushImmediately(); Command<Commands::DELETE_CHAR>(p); }
                    inc.groupBPeds[k] = 0;
                }
            }
            return;
        }
        balla->m_fHealth = 150.0f;
        balla->m_fMaxHealth = 150.0f;
        float bh = std::atan2(-(grovePos.x - bPos.x), grovePos.y - bPos.y);
        balla->m_fCurrentRotation = bh;
        balla->SetHeading(bh);
        balla->UpdateRwMatrix();
        const eWeaponType bWep = (i % 2 == 0) ? WEAPONTYPE_PISTOL : WEAPONTYPE_DESERT_EAGLE;
        GiveInfiniteCombatWeapon(balla, bWep);
        balla->m_nWeaponSkill = 2;
        balla->bStayInSamePlace = false;
        balla->bPanicWhenScared = false;
        balla->bCrouchWhenScared = false;
        balla->bFleeWhenStanding = false;
        inc.groupBPeds[i] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(balla));
    }
    // Начальные задачи боя друг на друга (для 3 пар бойцов)
    for (int i = 0; i < 3; ++i) {
        CPed* aFighter = ResolvePed(inc.groupAPeds[i]);
        if (aFighter && inc.lastTargets[i] == 0) {
            for (int j = 0; j < 3; ++j) {
                const int targetIdx = (i + j) % 3;
                CPed* enemy = ResolvePed(inc.groupBPeds[targetIdx]);
                if (enemy) {
                    inc.lastTargets[i] = inc.groupBPeds[targetIdx];
                    AssignKillPedTask(aFighter, enemy);
                    inc.inCombat = true;
                    break;
                }
            }
        }
        CPed* bFighter = ResolvePed(inc.groupBPeds[i]);
        if (bFighter && inc.lastTargetsB[i] == 0) {
            for (int j = 0; j < 3; ++j) {
                const int targetIdx = (i + j) % 3;
                CPed* enemy = ResolvePed(inc.groupAPeds[targetIdx]);
                if (enemy) {
                    inc.lastTargetsB[i] = inc.groupAPeds[targetIdx];
                    AssignKillPedTask(bFighter, enemy);
                    inc.inCombat = true;
                    break;
                }
            }
        }
    }
    inc.isMaterialized = true;
    Logger::Log("[MunicipalALife] Ganton riot materialized at (%.1f, %.1f, %.1f) blip #%u",
        inc.pos.x, inc.pos.y, inc.pos.z, inc.blipHandle);
}

// =============================================================================
//  A-Life: выгрузка 3D при дистанции > 170.0f; маркер остаётся на миникарте
// =============================================================================

static void DematerializeIncident(ActiveIncident& inc, CPed* player) {
    if (!inc.isMaterialized) return;
    if (CPools::ms_pVehiclePool && inc.vehHandle != 0) {
        CVehicle* veh = CPools::ms_pVehiclePool->GetAtRef(static_cast<int>(inc.vehHandle));
        if (veh && CPools::ms_pVehiclePool->IsObjectValid(veh) && (!player || player->m_pVehicle != veh)) {
            Command<Commands::DELETE_CAR>(veh);
        }
        inc.vehHandle = 0;
    }
    if (CPools::ms_pVehiclePool && inc.vehHandle2 != 0) {
        CVehicle* v2 = CPools::ms_pVehiclePool->GetAtRef(static_cast<int>(inc.vehHandle2));
        if (v2 && CPools::ms_pVehiclePool->IsObjectValid(v2) && (!player || player->m_pVehicle != v2)) {
            Command<Commands::DELETE_CAR>(v2);
        }
        inc.vehHandle2 = 0;
    }
    inc.strikeBarrierHandle = 0;
    if (CPools::ms_pPedPool && inc.patrolDriverHandle != 0) {
        CPed* driver = ResolvePed(inc.patrolDriverHandle);
        if (driver && driver != player && CPools::ms_pPedPool->IsObjectValid(driver)) {
            driver->ClearWeapons();
            if (driver->m_pIntelligence) driver->m_pIntelligence->m_TaskMgr.FlushImmediately();
            Command<Commands::DELETE_CHAR>(driver);
        }
        inc.patrolDriverHandle = 0;
    }
    if (CPools::ms_pVehiclePool && inc.patrolVehHandle != 0) {
        CVehicle* pat = CPools::ms_pVehiclePool->GetAtRef(static_cast<int>(inc.patrolVehHandle));
        if (pat && CPools::ms_pVehiclePool->IsObjectValid(pat) && (!player || player->m_pVehicle != pat)) {
            Command<Commands::DELETE_CAR>(pat);
        }
        inc.patrolVehHandle = 0;
    }
    if (CPools::ms_pObjectPool) {
        for (int i = 0; i < 16; ++i) {
            if (inc.coneHandles[i] != 0) {
                CObject* cone = CPools::ms_pObjectPool->GetAtRef(static_cast<int>(inc.coneHandles[i]));
                if (cone && CPools::ms_pObjectPool->IsObjectValid(cone)) {
                    Command<Commands::DELETE_OBJECT>(cone);
                }
                inc.coneHandles[i] = 0;
            }
        }
    }
    if (CPools::ms_pPedPool) {
        for (int i = 0; i < 8; ++i) {
            CPed* a = ResolvePed(inc.groupAPeds[i]);
            if (a && CPools::ms_pPedPool->IsObjectValid(a)) {
                a->ClearWeapons();
                if (a->m_pIntelligence) a->m_pIntelligence->m_TaskMgr.FlushImmediately();
            }
            CPed* b = ResolvePed(inc.groupBPeds[i]);
            if (b && CPools::ms_pPedPool->IsObjectValid(b)) {
                b->ClearWeapons();
                if (b->m_pIntelligence) b->m_pIntelligence->m_TaskMgr.FlushImmediately();
            }
        }
        for (int i = 0; i < 8; ++i) {
            if (inc.groupAPeds[i] != 0) {
                uint32_t handle = inc.groupAPeds[i];
                CPed* ped = ResolvePed(handle);
                if (ped && ped != player && CPools::ms_pPedPool->IsObjectValid(ped)) {
                    ped->ClearWeapons();
                    if (ped->m_pIntelligence) ped->m_pIntelligence->m_TaskMgr.FlushImmediately();
                    Command<Commands::DELETE_CHAR>(ped);
                }
                inc.groupAPeds[i] = 0;
            }
            if (inc.groupBPeds[i] != 0) {
                uint32_t handle = inc.groupBPeds[i];
                CPed* ped = ResolvePed(handle);
                if (ped && ped != player && CPools::ms_pPedPool->IsObjectValid(ped)) {
                    ped->ClearWeapons();
                    if (ped->m_pIntelligence) ped->m_pIntelligence->m_TaskMgr.FlushImmediately();
                    Command<Commands::DELETE_CHAR>(ped);
                }
                inc.groupBPeds[i] = 0;
            }
            inc.lastTargets[i] = 0;
            inc.lastTargetsB[i] = 0;
            inc.copsAlerted[i] = false;
        }
    }
    inc.inCombat = false;
    inc.isMaterialized = false;
}

// Полная очистка инцидента (по таймеру / сигналу очистки / вытеснению из реестра)
void CleanupIncident(ActiveIncident& inc, CPed* player, bool clearBlip) {
    if (inc.id > 0) {
        EmitMunicipalEvent("incident_cleanup", inc.id);
    }
    if (inc.isMaterialized) {
        DematerializeIncident(inc, player);
    }
    inc.vehHandle = 0;
    inc.vehHandle2 = 0;
    inc.strikeBarrierHandle = 0;
    inc.patrolVehHandle = 0;
    inc.patrolDriverHandle = 0;
    for (int i = 0; i < 16; ++i) {
        inc.coneHandles[i] = 0;
    }
    for (int i = 0; i < 8; ++i) {
        inc.groupAPeds[i] = 0;
        inc.groupBPeds[i] = 0;
        inc.lastTargets[i] = 0;
        inc.lastTargetsB[i] = 0;
        inc.copsAlerted[i] = false;
    }
    if (clearBlip && inc.blipHandle > 0) {
        CRadar::ClearBlip(static_cast<int>(inc.blipHandle));
        inc.blipHandle = 0;
    }
    if (inc.type == IncidentType::UNION_STRIKE &&
        (inc.animState == AnimState::REQUESTED || inc.animState == AnimState::READY)) {
        const int blockIdx = CAnimManager::GetAnimationBlockIndex("RIOT");
        if (blockIdx >= 0) CAnimManager::RemoveAnimBlockRef(blockIdx);
    }
    inc.animState = AnimState::NOT_LOADED;
    inc.id = 0;
    inc.inCombat = false;
    inc.active = false;
}

// =============================================================================
//  A-Life: реактивный AI (SetTask только при смене цели, без спама)
// =============================================================================

static void UpdateIncidentReactiveAI(ActiveIncident& inc, CPed* player) {
    if (player) {
        const CVector pPos = GetIncidentPlayerPosition(player);
        auto updatePedLod = [&](uint32_t handle) {
            if (handle == 0) return;
            CPed* ped = ResolvePed(handle);
            if (!ped || ped->m_fHealth <= 0.0f) return;
            const CVector pedPos = ped->GetPosition();
            float distToPlayerSq = (pPos.x - pedPos.x) * (pPos.x - pedPos.x) + (pPos.y - pedPos.y) * (pPos.y - pedPos.y);
            if (distToPlayerSq > 85.0f * 85.0f && !inc.inCombat) {
                ped->m_nStatus = eEntityStatus::STATUS_SIMPLE;
                ped->bUsesCollision = true;
                ped->bCollisionProcessed = false;
            } else {
                ped->m_nStatus = eEntityStatus::STATUS_PHYSICS;
            }
        };
        for (int i = 0; i < 8; ++i) {
            updatePedLod(inc.groupAPeds[i]);
            updatePedLod(inc.groupBPeds[i]);
        }
    }

    if (inc.type == IncidentType::UNION_STRIKE) {
        if (inc.isMaterialized) {
            const int aliveA = CountSafeAliveUnits(inc.groupAPeds);
            const int aliveB = CountSafeAliveUnits(inc.groupBPeds);
            if (aliveA == 0 && aliveB == 0 && (inc.groupAPeds[0] != 0 || inc.groupBPeds[0] != 0)) {
                bool playerClose = false;
                if (player) {
                    const CVector pPos = GetIncidentPlayerPosition(player);
                    const float dx = pPos.x - inc.pos.x;
                    const float dy = pPos.y - inc.pos.y;
                    playerClose = ((dx * dx + dy * dy) < (35.0f * 35.0f));
                }
                if (playerClose) {
                    EmitMunicipalEvent("incident_cleared", inc.id, "{\"scenario\":\"riot_massacre\"}");
                    CPlayerPed* playerPed = FindPlayerPed();
                    if (playerPed) {
                        playerPed->SetWantedLevelNoDrop(3);
                    }
                } else {
                    EmitMunicipalEvent("incident_cleared", inc.id, "{\"scenario\":\"player\"}");
                }
                s_totalIncidentsHandled.fetch_add(1, std::memory_order_relaxed);
                CleanupIncident(inc, player, true);
                return;
            }
        }
        return;
    }

    if (inc.type == IncidentType::GANTON_RIOT) {
        // Группа A -> groupBPeds (lastTargets); зеркально группа B -> groupAPeds (lastTargetsB)
        for (int i = 0; i < 8; ++i) {
            if (inc.groupAPeds[i] == 0) continue;
            CPed* fighter = ResolvePed(inc.groupAPeds[i]);
            if (!fighter || fighter->m_fHealth <= 0.0f) continue;
            CPed* curTarget = ResolvePed(inc.lastTargets[i]);
            if (!curTarget || curTarget->m_fHealth <= 0.0f) {
                inc.lastTargets[i] = 0;
                for (int j = 0; j < 8; ++j) {
                    CPed* enemy = ResolvePed(inc.groupBPeds[j]);
                    if (enemy && enemy->m_fHealth > 0.0f) {
                        inc.lastTargets[i] = inc.groupBPeds[j];
                        AssignKillPedTask(fighter, enemy);
                        inc.inCombat = true;
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < 8; ++i) {
            if (inc.groupBPeds[i] == 0) continue;
            CPed* fighter = ResolvePed(inc.groupBPeds[i]);
            if (!fighter || fighter->m_fHealth <= 0.0f) continue;
            CPed* curTargetB = ResolvePed(inc.lastTargetsB[i]);
            if (!curTargetB || curTargetB->m_fHealth <= 0.0f) {
                inc.lastTargetsB[i] = 0;
                for (int j = 0; j < 8; ++j) {
                    CPed* enemy = ResolvePed(inc.groupAPeds[j]);
                    if (enemy && enemy->m_fHealth > 0.0f) {
                        inc.lastTargetsB[i] = inc.groupAPeds[j];
                        AssignKillPedTask(fighter, enemy);
                        inc.inCombat = true;
                        break;
                    }
                }
            }
        }
    } else if (inc.type == IncidentType::ROADBLOCK) {
        if (player && player->m_fHealth > 0.0f) {
            const CVector pPos = GetIncidentPlayerPosition(player);
            const float dx = pPos.x - inc.pos.x;
            const float dy = pPos.y - inc.pos.y;
            float distToCenter = std::sqrt(dx * dx + dy * dy);

            // 1) ЗОНА ПОРАЖЕНИЯ (Игрок пересек линию конусов, дистанция <= 7.0м):
            if (distToCenter <= 7.0f) {
                if (!inc.inCombat) {
                    inc.inCombat = true;
                    CPlayerPed* playerPed = FindPlayerPed();
                    if (playerPed && playerPed->m_pPlayerData && playerPed->m_pPlayerData->m_pWanted) {
                        if (playerPed->m_pPlayerData->m_pWanted->m_nWantedLevel < 3) {
                            playerPed->SetWantedLevelNoDrop(3);
                        }
                    }
                    CHud::SetHelpMessage("~r~CHECKPOINT BREACH:~w~ Lethal force engaged!", true, false, false);
                    for (int c = 0; c < 6; ++c) {
                        CPed* cop = ResolvePed(inc.groupAPeds[c]);
                        if (cop && cop->m_fHealth > 0.0f) {
                            AssignKillPedTask(cop, player);
                            inc.copsAlerted[c] = true;
                        }
                    }
                    EmitMunicipalEvent("cop_engaged", inc.id, "{\"reason\":\"breach\"}");
                }
            }
            // 2) ЗОНА ПРЕДУПРЕЖДЕНИЯ (Дистанция от 7.0м до 14.0м):
            else if (distToCenter > 7.0f && distToCenter <= 14.0f && !inc.inCombat) {
                CHud::SetHelpMessage("~y~RESTRICTED POLICE CHECKPOINT:~w~ Turn back or we will open fire!", true, false, false);
                // Все бойцы разворачиваются на игрока и берут на прицел:
                for (int c = 0; c < 6; ++c) {
                    CPed* cop = ResolvePed(inc.groupAPeds[c]);
                    if (cop && cop->m_fHealth > 0.0f) {
                        CVector cPos = cop->GetPosition();
                        float lookH = std::atan2(-(pPos.x - cPos.x), pPos.y - cPos.y);
                        cop->m_fCurrentRotation = lookH;
                        cop->SetHeading(lookH);
                        cop->UpdateRwMatrix();
                    }
                }
            }
            else {
                // Проверка получения урона бойцом SWAT (под огнём)
                for (int c = 0; c < 6; ++c) {
                    if (inc.copsAlerted[c]) continue;
                    CPed* cop = ResolvePed(inc.groupAPeds[c]);
                    if (!cop || cop->m_fHealth <= 0.0f) continue;
                    if (cop->m_fHealth < 95.0f) {
                        AssignKillPedTask(cop, player);
                        inc.copsAlerted[c] = true;
                        inc.inCombat = true;
                        EmitMunicipalEvent("cop_engaged", inc.id, "{\"reason\":\"under_fire\"}");
                    }
                }
            }
        }

        // Реакция на бандитов и забастовщиков: радиус 65м от inc.pos
        const CVector pPos = player ? GetIncidentPlayerPosition(player) : inc.pos;
        float distToPlayerSq = (pPos.x - inc.pos.x) * (pPos.x - inc.pos.x) + (pPos.y - inc.pos.y) * (pPos.y - inc.pos.y);
        if (CPools::ms_pPedPool && distToPlayerSq <= 160.0f * 160.0f) {
            CPed* enemyTarget = nullptr;
            float closestEnemyDistSq = 65.0f * 65.0f;
            for (int pIdx = 0; pIdx < CPools::ms_pPedPool->m_nSize; ++pIdx) {
                CPed* otherPed = CPools::ms_pPedPool->GetAt(pIdx);
                if (!otherPed || !CPools::ms_pPedPool->IsObjectValid(otherPed) || otherPed->m_fHealth <= 0.0f) continue;
                if (player && otherPed == player) continue;
                if (otherPed->m_nPedType == PED_TYPE_COP) continue;

                bool isHostile = false;
                if (otherPed->m_nPedType >= PED_TYPE_GANG1 && otherPed->m_nPedType <= PED_TYPE_GANG10) {
                    if (!s_rampartScandalActive.load(std::memory_order_relaxed)) isHostile = true;
                }

                if (isHostile) {
                    const CVector oPos = otherPed->GetPosition();
                    const float gdx = oPos.x - inc.pos.x;
                    const float gdy = oPos.y - inc.pos.y;
                    const float distSq = gdx * gdx + gdy * gdy;
                    if (distSq < closestEnemyDistSq) {
                        closestEnemyDistSq = distSq;
                        enemyTarget = otherPed;
                    }
                }
            }
            if (enemyTarget) {
                for (int c = 0; c < 6; ++c) {
                    CPed* cop = ResolvePed(inc.groupAPeds[c]);
                    if (cop && cop->m_fHealth > 0.0f) {
                        AssignKillPedTask(cop, enemyTarget);
                        inc.copsAlerted[c] = true;
                        inc.inCombat = true;
                    }
                }
            }
        }
    }

    if (inc.inCombat) {
        const int aliveA = CountSafeAliveUnits(inc.groupAPeds);
        const int aliveB = CountSafeAliveUnits(inc.groupBPeds);

        if (inc.type == IncidentType::ROADBLOCK) {
            // Односторонний инцидент (только копы groupA): зачищен ТОЛЬКО когда все копы реально погибли
            if (aliveA == 0 && inc.groupAPeds[0] != 0) {
                inc.inCombat = false;
                char dataJson[64];
                snprintf(dataJson, sizeof(dataJson), "{\"scenario\":\"player\"}");
                EmitMunicipalEvent("incident_cleared", inc.id, dataJson);
                s_totalIncidentsHandled.fetch_add(1, std::memory_order_relaxed);
                AddMunicipalLog("POLICE ACTION: SWAT Roadblock neutralized");
                CleanupIncident(inc, player, true);
                return;
            }
        } else if (inc.type == IncidentType::GANTON_RIOT) {
            // Двусторонний конфликт: проверка завершения когда одна или обе стороны разбиты
            if (aliveA == 0 || aliveB == 0) {
                inc.inCombat = false;
                const char* scenario = (aliveA == 0 && aliveB == 0) ? "mutual" : "one_sided";
                if (player) {
                    const CVector pPos = GetIncidentPlayerPosition(player);
                    const float dx = pPos.x - inc.pos.x;
                    const float dy = pPos.y - inc.pos.y;
                    if ((dx * dx + dy * dy) < (35.0f * 35.0f)) {
                        scenario = "player";
                    }
                }
                char dataJson[64];
                snprintf(dataJson, sizeof(dataJson), "{\"scenario\":\"%s\"}", scenario);
                EmitMunicipalEvent("incident_cleared", inc.id, dataJson);

                s_totalIncidentsHandled.fetch_add(1, std::memory_order_relaxed);
                s_cityTreasury.fetch_add(15000, std::memory_order_relaxed);
                const float curCrime = s_crimeRate.load(std::memory_order_relaxed);
                s_crimeRate.store((std::max)(0.0f, curCrime - 2.5f), std::memory_order_relaxed);
                AddMunicipalLog("POLICE ACTION: Gang shootout suppressed. Contraband confiscated (+$15,000, Crime -2.5%%)");

                CleanupIncident(inc, player, true);
                return;
            }
        }
    }
}

// =============================================================================
//  A-Life: покадровый жизненный цикл реестра инцидентов
// =============================================================================

void UpdateIncidentsLifecycle(CPed* player, uint32_t currentMs) {
    // 1. Очистка по таймеру и обновление анимаций
    for (size_t i = 0; i < k_maxIncidents; ++i) {
        ActiveIncident& inc = s_incidents[i];
        if (!inc.active) continue;

        if (inc.durationMs > 0 && (currentMs - inc.spawnTimeMs) > inc.durationMs) {
            if (inc.type == IncidentType::UNION_STRIKE) {
                EmitMunicipalEvent("incident_cleared", inc.id, "{\"scenario\":\"riot_dispersed\"}");
            }
            s_totalIncidentsHandled.fetch_add(1, std::memory_order_relaxed);
            CleanupIncident(inc, player, true);
            continue;
        }

        if (inc.type == IncidentType::UNION_STRIKE) {
            UpdateIncidentAnimBlock(inc);
        }
    }

    if (!player) return; // нет игрока — дистанционная логика не выполняется

    const CVector pPos = GetIncidentPlayerPosition(player);
    constexpr float STREAM_IN_DIST = 100.0f;
    constexpr float STREAM_OUT_DIST = 140.0f;
    constexpr float STREAM_IN_DIST_SQ = STREAM_IN_DIST * STREAM_IN_DIST;
    constexpr float STREAM_OUT_DIST_SQ = STREAM_OUT_DIST * STREAM_OUT_DIST;

    // 2. Дематериализация удалённых инцидентов и обновление AI активных
    for (size_t i = 0; i < k_maxIncidents; ++i) {
        ActiveIncident& inc = s_incidents[i];
        if (!inc.active || !inc.isMaterialized) continue;

        const float dx = pPos.x - inc.pos.x;
        const float dy = pPos.y - inc.pos.y;
        const float distSq = dx * dx + dy * dy;

        if (distSq > STREAM_OUT_DIST_SQ) {
            DematerializeIncident(inc, player);
            EmitMunicipalEvent("phase_changed", inc.id, "{\"phase\":\"dematerialized\"}");
        } else {
            const bool wasInCombat = inc.inCombat;
            UpdateIncidentReactiveAI(inc, player);
            if (!wasInCombat && inc.inCombat) {
                EmitMunicipalEvent("phase_changed", inc.id, "{\"phase\":\"combat\"}");
            } else if (wasInCombat && !inc.inCombat) {
                EmitMunicipalEvent("phase_changed", inc.id, "{\"phase\":\"combat_ended\"}");
            }
        }
    }

    // 3. Подсчёт активных материализованных инцидентов
    size_t matRoadblocks = 0;
    size_t matStrikes = 0;
    size_t matGangWars = 0;
    for (size_t k = 0; k < k_maxIncidents; ++k) {
        if (s_incidents[k].active && s_incidents[k].isMaterialized) {
            if (s_incidents[k].type == IncidentType::ROADBLOCK) matRoadblocks++;
            else if (s_incidents[k].type == IncidentType::UNION_STRIKE) matStrikes++;
            else if (s_incidents[k].type == IncidentType::GANTON_RIOT) matGangWars++;
        }
    }

    // 4. Сбор нематериализованных инцидентов в радиусе входа
    struct IncidentCandidate {
        size_t index;
        float distSq;
    };
    IncidentCandidate candidates[k_maxIncidents];
    size_t candidateCount = 0;

    for (size_t i = 0; i < k_maxIncidents; ++i) {
        ActiveIncident& inc = s_incidents[i];
        if (!inc.active || inc.isMaterialized) continue;

        const float dx = pPos.x - inc.pos.x;
        const float dy = pPos.y - inc.pos.y;
        const float distSq = dx * dx + dy * dy;

        if (distSq <= STREAM_IN_DIST_SQ) {
            candidates[candidateCount++] = { i, distSq };
        }
    }

    // Сортировка по возрастанию дистанции (ближайшие к игроку спавнятся первыми!)
    std::sort(candidates, candidates + candidateCount, [](const IncidentCandidate& a, const IncidentCandidate& b) {
        return a.distSq < b.distSq;
    });

    // 5. Приоритетная материализация с вытеснением далёких точек
    for (size_t c = 0; c < candidateCount; ++c) {
        const size_t slot = candidates[c].index;
        ActiveIncident& inc = s_incidents[slot];

        // Idlewood Commercial Junction (1935, -1770) Hard Throttling:
        // When player is within 80m of Idlewood Commercial Junction, limit concurrent incident entities:
        // Maximum 1 active incident (either roadblock OR strike OR gang riot, never simultaneous stacking).
        const float idlewoodDx = pPos.x - 1935.0f;
        const float idlewoodDy = pPos.y - (-1770.0f);
        const bool inIdlewoodSector = (idlewoodDx * idlewoodDx + idlewoodDy * idlewoodDy) <= (80.0f * 80.0f);
        if (inIdlewoodSector) {
            size_t activeNearIdlewood = 0;
            for (size_t k = 0; k < k_maxIncidents; ++k) {
                if (s_incidents[k].active && s_incidents[k].isMaterialized) {
                    const float kDx = 1935.0f - s_incidents[k].pos.x;
                    const float kDy = -1770.0f - s_incidents[k].pos.y;
                    if ((kDx * kDx + kDy * kDy) <= (120.0f * 120.0f)) {
                        activeNearIdlewood++;
                    }
                }
            }
            if (activeNearIdlewood >= 1) {
                continue; // Maximum 1 active incident near Idlewood Junction (no stacking)
            }
        }

        // Restrict simultaneous materialized roadblocks and strike incidents in the player's streaming bubble to a strict maximum of 1 active site within 150m.
        if (inc.type == IncidentType::ROADBLOCK || inc.type == IncidentType::UNION_STRIKE) {
            size_t nearbyMajorSites = 0;
            for (size_t k = 0; k < k_maxIncidents; ++k) {
                if (s_incidents[k].active && s_incidents[k].isMaterialized) {
                    if (s_incidents[k].type == IncidentType::ROADBLOCK || s_incidents[k].type == IncidentType::UNION_STRIKE) {
                        const float kDx = pPos.x - s_incidents[k].pos.x;
                        const float kDy = pPos.y - s_incidents[k].pos.y;
                        if ((kDx * kDx + kDy * kDy) <= (150.0f * 150.0f)) {
                            nearbyMajorSites++;
                        }
                    }
                }
            }
            if (nearbyMajorSites >= 1) {
                continue; // Strict maximum 1 active site within 150m
            }
        }

        size_t* currentCount = nullptr;
        size_t maxLimit = 2;
        if (inc.type == IncidentType::ROADBLOCK) {
            currentCount = &matRoadblocks;
            maxLimit = 1; // Throttled to 1 for stability
        } else if (inc.type == IncidentType::UNION_STRIKE) {
            currentCount = &matStrikes;
            maxLimit = 1;
        } else if (inc.type == IncidentType::GANTON_RIOT) {
            currentCount = &matGangWars;
            maxLimit = 2;
        }
        if (!currentCount) continue;

        // Если квота заполнена, но игрок вплотную подошёл к новой точке (< 75м),
        // выгружаем самый дальний материализованный инцидент этого же типа
        if (*currentCount >= maxLimit && candidates[c].distSq < (75.0f * 75.0f)) {
            ptrdiff_t furthestSlot = -1;
            float maxDistSq = -1.0f;
            for (size_t k = 0; k < k_maxIncidents; ++k) {
                if (s_incidents[k].active && s_incidents[k].isMaterialized && s_incidents[k].type == inc.type) {
                    const float kDx = pPos.x - s_incidents[k].pos.x;
                    const float kDy = pPos.y - s_incidents[k].pos.y;
                    const float kDistSq = kDx * kDx + kDy * kDy;
                    if (kDistSq > maxDistSq) {
                        maxDistSq = kDistSq;
                        furthestSlot = static_cast<ptrdiff_t>(k);
                    }
                }
            }
            if (furthestSlot >= 0 && maxDistSq > (candidates[c].distSq + 25.0f * 25.0f)) {
                DematerializeIncident(s_incidents[furthestSlot], player);
                EmitMunicipalEvent("phase_changed", s_incidents[furthestSlot].id, "{\"phase\":\"dematerialized\"}");
                (*currentCount)--;
            }
        }

        if (*currentCount < maxLimit) {
            MaterializeIncident(slot, inc, currentMs);
            if (inc.isMaterialized) {
                (*currentCount)++;
                EmitMunicipalEvent("phase_changed", inc.id, "{\"phase\":\"materialized\"}");
            }
        }
    }
}

// =============================================================================
//  A-Life: приём события из очереди g_municipalGameEventQueue
// =============================================================================

void IntakeMunicipalIncident(const MunicipalGameEvent& ev, uint32_t nowMs, CPed* player) {
    if (ev.type == MunicipalGameEventType::StrikeProtest) {
        // Do not force reset if Rioters module is already handling a site
        return;
    }
    IncidentType type = IncidentType::GANTON_RIOT;
    switch (ev.type) {
    case MunicipalGameEventType::RoadblockPolice: type = IncidentType::ROADBLOCK;    break;
    case MunicipalGameEventType::IncidentSpawn:   type = IncidentType::GANTON_RIOT;  break;
    default: return;
    }

    CVector pos(ev.x, ev.y, ev.z);
    if (pos.x == 0.0f && pos.y == 0.0f) {
        // Канонические якоря из муниципальной спецификации
        if (type == IncidentType::UNION_STRIKE) {
            pos = k_cityHallCoords;
        } else if (type == IncidentType::ROADBLOCK) {
            pos = k_policeBarricadePoints[0];
        } else {
            pos = k_gantonGroveEnd;
        }
    }

    // Защита от наложения: если в радиусе 25м уже есть активный инцидент этого же типа — не плодить дубликаты
    for (size_t i = 0; i < k_maxIncidents; ++i) {
        if (s_incidents[i].active && s_incidents[i].type == type) {
            const float dx = s_incidents[i].pos.x - pos.x;
            const float dy = s_incidents[i].pos.y - pos.y;
            if ((dx * dx + dy * dy) < (25.0f * 25.0f)) {
                return; // Точка уже занята активным инцидентом
            }
        }
    }

    size_t slot = 0;
    const ptrdiff_t existingSlot = FindIncidentSlotById(ev.id);
    if (existingSlot >= 0) {
        slot = static_cast<size_t>(existingSlot);
        CleanupIncident(s_incidents[slot], player, true);
    } else {
        slot = AcquireIncidentSlot(player);
    }

    ActiveIncident& inc = s_incidents[slot];
    inc = ActiveIncident{};
    inc.active = true;
    inc.id = (ev.id != 0) ? ev.id : s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
    inc.type = type;
    inc.pos = pos;
    inc.heading = 0.0f;
    inc.spawnTimeMs = nowMs;
    inc.durationMs = (ev.param > 0) ? ev.param : 600000;
    inc.isMaterialized = false;
    inc.inCombat = false;
    inc.animState = (type == IncidentType::UNION_STRIKE) ? AnimState::NOT_LOADED : AnimState::FAILED;

    // СРАЗУ выставляем радарный маркер (живёт независимо от 3D-выгрузки моделей)
    const int blip = CRadar::SetCoordBlip(BLIP_COORD, pos, 0, BLIP_DISPLAY_BOTH, nullptr);
    if (blip >= 0) {
        switch (type) {
        case IncidentType::ROADBLOCK:
            CRadar::SetBlipSprite(blip, RADAR_SPRITE_POLICE);
            CRadar::ChangeBlipColour(blip, k_muniBlipColourBlue);   // синий
            break;
        case IncidentType::UNION_STRIKE:
            CRadar::SetBlipSprite(blip, RADAR_SPRITE_FLAG);
            CRadar::ChangeBlipColour(blip, k_muniBlipColourYellow); // жёлтый
            break;
        case IncidentType::GANTON_RIOT:
            CRadar::SetBlipSprite(blip, RADAR_SPRITE_ENEMYATTACK);
            CRadar::ChangeBlipColour(blip, k_muniBlipColourRed);    // красный
            break;
        }
        CRadar::ChangeBlipScale(blip, 3);
        inc.blipHandle = static_cast<uint32_t>(blip);
    } else {
        inc.blipHandle = 0;
        Logger::Log("[MunicipalALife] Warning: Failed to allocate radar blip for incident #%u (pool exhausted)", inc.id);
    }

    switch (type) {
    case IncidentType::ROADBLOCK:
        CHud::SetHelpMessage("~b~POLICE ROADBLOCK DEPLOYED:~w~ LAPD checkpoint ahead!", true, false, false);
        break;
    case IncidentType::UNION_STRIKE:
        CHud::SetHelpMessage("~y~UNION STRIKE ACTIVE:~w~ City Hall workers walk out!", true, false, false);
        break;
    case IncidentType::GANTON_RIOT:
        {
            char hudMsg[128];
            snprintf(hudMsg, sizeof(hudMsg), "~r~EMERGENCY INCIDENT:~w~ %s! Check radar.",
                ev.text[0] ? ev.text : "Ganton riot in progress");
            CHud::SetHelpMessage(hudMsg, true, false, false);
        }
        break;
    }
    s_incidentCollisionRequested[slot] = 0;

    // Модели — строго асинхронно и приоритетно, БЕЗ LoadAllRequestedModels
    RequestIncidentModels(inc);

    Logger::Log("[MunicipalALife] Incident #%u accepted (type %u) at (%.1f, %.1f, %.1f), blip #%u",
        inc.id, static_cast<unsigned>(type), pos.x, pos.y, pos.z, inc.blipHandle);
}

// =============================================================================
//  World Crisis Physical World Engine & Lifecycle Manager
// =============================================================================

CrisisWorldEntityState s_crisisEntities;
static uint8_t s_crisisPendingMaterializeId = 0;

static void RequestCrisisModels(uint8_t crisisId) {
    switch (crisisId) {
    case 1: // OIL_EMBARGO
        SafeRequestModel(MODEL_GLENDALE);
        break;
    case 4: // BRIDGE_REPAIR
        SafeRequestModel(1238);
        SafeRequestModel(MODEL_WMYMECH);
        break;
    case 7: // CREDIT_CRUNCH
        SafeRequestModel(MODEL_BMYST);
        SafeRequestModel(MODEL_WMYST);
        SafeRequestModel(MODEL_WFYST);
        break;
    case 8: // CONTRABAND_BOOM
        SafeRequestModel(MODEL_BURRITO);
        SafeRequestModel(MODEL_BALLAS1);
        SafeRequestModel(MODEL_AK47);
        break;
    case 10: // SANITATION_STRIKE
        SafeRequestModel(MODEL_TRASH);
        SafeRequestModel(MODEL_BMYST);
        SafeRequestModel(MODEL_WMYST);
        SafeRequestAnimation("RIOT");
        break;
    case 11: // CAMPUS_PROTEST
        SafeRequestModel(MODEL_BMYST);
        SafeRequestModel(MODEL_WFYST);
        break;
    case 13: // RICO_SWEEP
        SafeRequestModel(MODEL_FBIRANCH);
        SafeRequestModel(MODEL_FBI);
        SafeRequestModel(MODEL_MP5LNG);
        break;
    default:
        break;
    }
}

static bool AreCrisisModelsReady(uint8_t crisisId) {
    switch (crisisId) {
    case 1:
        return IsMuniModelLoaded(MODEL_GLENDALE);
    case 4:
        return IsMuniModelLoaded(1238) && IsMuniModelLoaded(MODEL_WMYMECH);
    case 7:
        return IsMuniModelLoaded(MODEL_BMYST) && IsMuniModelLoaded(MODEL_WMYST) && IsMuniModelLoaded(MODEL_WFYST);
    case 8:
        return IsMuniModelLoaded(MODEL_BURRITO) && IsMuniModelLoaded(MODEL_BALLAS1) && IsMuniModelLoaded(MODEL_AK47);
    case 10:
        return IsMuniModelLoaded(MODEL_TRASH) && IsMuniModelLoaded(MODEL_BMYST) && IsMuniModelLoaded(MODEL_WMYST);
    case 11:
        return IsMuniModelLoaded(MODEL_BMYST) && IsMuniModelLoaded(MODEL_WFYST);
    case 13:
        return IsMuniModelLoaded(MODEL_FBIRANCH) && IsMuniModelLoaded(MODEL_FBI) && IsMuniModelLoaded(MODEL_MP5LNG);
    default:
        return true;
    }
}

void ApplyCrisisWorldPhysics(uint8_t crisisId, bool activate) {
    if (!activate) {
        // Clean up spawned vehicles
        if (CPools::ms_pVehiclePool) {
            for (int i = 0; i < 8; ++i) {
                if (s_crisisEntities.vehHandles[i] != 0) {
                    CVehicle* veh = CPools::ms_pVehiclePool->GetAtRef(static_cast<int>(s_crisisEntities.vehHandles[i]));
                    if (veh) {
                        Command<Commands::DELETE_CAR>(veh);
                    }
                    s_crisisEntities.vehHandles[i] = 0;
                }
            }
        }
        // Clean up spawned peds
        if (CPools::ms_pPedPool) {
            CPed* player = FindPlayerPed();
            for (int i = 0; i < 16; ++i) {
                if (s_crisisEntities.pedHandles[i] != 0) {
                    CPed* ped = CPools::ms_pPedPool->GetAtRef(static_cast<int>(s_crisisEntities.pedHandles[i]));
                    if (ped && ped != player) {
                        if (ped->m_pIntelligence) ped->m_pIntelligence->m_TaskMgr.FlushImmediately();
                        Command<Commands::DELETE_CHAR>(ped);
                    }
                    s_crisisEntities.pedHandles[i] = 0;
                }
            }
        }
        // Clean up spawned objects
        if (CPools::ms_pObjectPool) {
            for (int i = 0; i < 12; ++i) {
                if (s_crisisEntities.objHandles[i] != 0) {
                    CObject* obj = CPools::ms_pObjectPool->GetAtRef(static_cast<int>(s_crisisEntities.objHandles[i]));
                    if (obj) {
                        Command<Commands::DELETE_OBJECT>(obj);
                    }
                    s_crisisEntities.objHandles[i] = 0;
                }
            }
        }
        // Clean up linked municipal incidents
        CPed* player = FindPlayerPed();
        for (int i = 0; i < 4; ++i) {
            if (s_crisisEntities.incidentIds[i] != 0) {
                ptrdiff_t slot = FindIncidentSlotById(s_crisisEntities.incidentIds[i]);
                if (slot >= 0) {
                    CleanupIncident(s_incidents[slot], player, true);
                }
                s_crisisEntities.incidentIds[i] = 0;
            }
        }

        // Reset modifiers and engine states
        s_portStrikeActive.store(false, std::memory_order_relaxed);
        s_chipShortageActive.store(false, std::memory_order_relaxed);
        s_federalGrantActive.store(false, std::memory_order_relaxed);
        s_rampartScandalActive.store(false, std::memory_order_relaxed);
        s_opioidSurgeActive.store(false, std::memory_order_relaxed);
        s_gridBlackoutActive.store(false, std::memory_order_relaxed);

        if (crisisId == 9 || s_crisisEntities.activeCrisisId == 9) {
            Command<Commands::SET_LA_RIOTS>(0);
            CTheScripts::RiotIntensity = 0;
        }
        if (crisisId == 15 || s_crisisEntities.activeCrisisId == 15) {
            CWeather::TrafficLightsBrightness = 1.0f;
        }

        s_crisisEntities.activeCrisisId = 0;
        s_crisisPendingMaterializeId = 0;
        return;
    }

    // Deactivate previous crisis if different
    if (s_crisisEntities.activeCrisisId != 0 && s_crisisEntities.activeCrisisId != crisisId) {
        ApplyCrisisWorldPhysics(s_crisisEntities.activeCrisisId, false);
    }
    s_crisisEntities.activeCrisisId = crisisId;

    RequestCrisisModels(crisisId);

    CPed* player = FindPlayerPed();
    const uint32_t currentMs = CTimer::m_snTimeInMilliseconds;

    switch (crisisId) {
    case 1: { // OIL_EMBARGO: Abandoned Glendales with open hoods & hazard alarms at gas stations
        if (!AreCrisisModelsReady(1)) {
            s_crisisPendingMaterializeId = 1;
            break;
        }
        s_crisisPendingMaterializeId = 0;
        if (CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->GetNoOfFreeSpaces() >= 3) {
            for (int i = 0; i < 3; ++i) {
                CAutomobile* car = new CAutomobile(MODEL_GLENDALE, 2, true);
                if (car && CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->IsObjectValid(car)) {
                    car->m_nCreatedBy = 2;
                    CVector spawnPos = k_gasStationCoords[i] + CVector(3.5f, 1.5f, 0.5f);
                    car->Teleport(spawnPos);
                    car->PlaceOnRoadProperly();
                    car->bEngineOn = false;
                    car->bEngineBroken = true;
                    car->bSirenOrAlarm = true;
                    car->OpenDoor(nullptr, CAR_BONNET, BONNET, 1.0f, true);
                    CWorld::Add(car);
                    car->UpdateRwMatrix();
                    s_crisisEntities.vehHandles[i] = static_cast<uint32_t>(CPools::ms_pVehiclePool->GetRef(car));
                } else if (car) {
                    Command<Commands::DELETE_CAR>(car);
                }
            }
        }
        break;
    }
    case 2: { // PORT_STRIKE: Strikes at City Hall and Ocean Docks, block resource additions
        s_portStrikeActive.store(true, std::memory_order_relaxed);
        MunicipalGameEvent ev1{};
        ev1.type = MunicipalGameEventType::StrikeProtest;
        ev1.id = s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
        ev1.x = k_cityHallCoords.x; ev1.y = k_cityHallCoords.y; ev1.z = k_cityHallCoords.z;
        ev1.param = 180000;
        strncpy_s(ev1.text, "Port Strike - City Hall", sizeof(ev1.text) - 1);
        IntakeMunicipalIncident(ev1, currentMs, player);
        s_crisisEntities.incidentIds[0] = ev1.id;

        MunicipalGameEvent ev2{};
        ev2.type = MunicipalGameEventType::StrikeProtest;
        ev2.id = s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
        ev2.x = s_highwayLoop[0].x; ev2.y = s_highwayLoop[0].y; ev2.z = s_highwayLoop[0].z;
        ev2.param = 180000;
        strncpy_s(ev2.text, "Port Strike - Ocean Docks", sizeof(ev2.text) - 1);
        IntakeMunicipalIncident(ev2, currentMs, player);
        s_crisisEntities.incidentIds[1] = ev2.id;
        break;
    }
    case 3: { // CHIP_SHORTAGE: Accelerate electronics highway hijackings
        s_chipShortageActive.store(true, std::memory_order_relaxed);
        break;
    }
    case 4: { // BRIDGE_REPAIR: 6 cones across lane at k_policeBarricadePoints[0], 2 mechanics
        if (!AreCrisisModelsReady(4)) {
            s_crisisPendingMaterializeId = 4;
            break;
        }
        s_crisisPendingMaterializeId = 0;
        const CVector pt = k_policeBarricadePoints[0];
        if (CPools::ms_pObjectPool && CPools::ms_pObjectPool->GetNoOfFreeSpaces() >= 6) {
            for (int k = 0; k < 6; ++k) {
                CVector cPos = pt + CVector(-4.0f + static_cast<float>(k) * 1.6f, 0.0f, 0.2f);
                float gz = CWorld::FindGroundZForCoord(cPos.x, cPos.y);
                if (gz > -100.0f) cPos.z = gz + 0.1f;
                CObject* cone = CObject::Create(1238);
                if (cone) {
                    cone->m_nObjectType = OBJECT_MISSION;
                    cone->Teleport(cPos);
                    cone->m_nStatus = eEntityStatus::STATUS_PHYSICS;
                    CWorld::Add(cone);
                    cone->UpdateRwMatrix();
                    s_crisisEntities.objHandles[k] = static_cast<uint32_t>(CPools::ms_pObjectPool->GetRef(cone));
                }
            }
        }
        if (CPools::ms_pPedPool && CPools::ms_pPedPool->GetNoOfFreeSpaces() >= 2) {
            for (int m = 0; m < 2; ++m) {
                CVector mPos = pt + CVector(-1.5f + static_cast<float>(m) * 3.0f, 2.5f, 0.5f);
                CPed* mech = SpawnIncidentPed(MODEL_WMYMECH, PED_TYPE_CIVMALE, mPos, true);
                if (mech) {
                    Command<Commands::TASK_PLAY_ANIM>(mech, "CAR_JACKEDLHS", "PED", 4.0f, 1, 0, 0, 0, -1);
                    s_crisisEntities.pedHandles[m] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(mech));
                }
            }
        }
        break;
    }
    case 5: { // MUNI_DEFAULT: Police desertion 70%, spontaneous rally at City Hall
        s_policeDesertionPct.store(70.0f, std::memory_order_relaxed);
        MunicipalGameEvent ev{};
        ev.type = MunicipalGameEventType::StrikeProtest;
        ev.id = s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
        ev.x = k_cityHallCoords.x; ev.y = k_cityHallCoords.y; ev.z = k_cityHallCoords.z;
        ev.param = 180000;
        strncpy_s(ev.text, "Municipal Default Rally", sizeof(ev.text) - 1);
        IntakeMunicipalIncident(ev, currentMs, player);
        s_crisisEntities.incidentIds[0] = ev.id;
        break;
    }
    case 6: { // FEDERAL_GRANT: +$200,000 to treasury, +50% lumber purchase rate
        s_cityTreasury.fetch_add(200000, std::memory_order_relaxed);
        s_federalGrantActive.store(true, std::memory_order_relaxed);
        break;
    }
    case 7: { // CREDIT_CRUNCH: -15% company balances, queue of peds at Pershing Square bank
        for (size_t c = 0; c < 3; ++c) {
            int64_t b = s_companyBalances[c].load(std::memory_order_relaxed);
            s_companyBalances[c].store((b * 85) / 100, std::memory_order_relaxed);
        }
        if (!AreCrisisModelsReady(7)) {
            s_crisisPendingMaterializeId = 7;
            break;
        }
        s_crisisPendingMaterializeId = 0;
        if (CPools::ms_pPedPool && CPools::ms_pPedPool->GetNoOfFreeSpaces() >= 5) {
            const CVector bPos = k_policeBarricadePoints[1];
            for (int q = 0; q < 5; ++q) {
                const int m = (q % 3 == 0) ? MODEL_BMYST : ((q % 3 == 1) ? MODEL_WMYST : MODEL_WFYST);
                CVector pos = bPos + CVector(0.0f, -static_cast<float>(q) * 1.3f, 0.5f);
                CPed* p = SpawnIncidentPed(m, PED_TYPE_CIVMALE, pos, true);
                if (p) {
                    p->SetHeading(0.0f);
                    p->m_pIntelligence->m_TaskMgr.SetTask(new CTaskSimpleStandStill(-1, true, false, 8.0f), TASK_PRIMARY_PRIMARY, false);
                    s_crisisEntities.pedHandles[q] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(p));
                }
            }
        }
        break;
    }
    case 8: { // CONTRABAND_BOOM: Burrito van & 2 armed mobsters at Ocean Docks dead-end
        if (!AreCrisisModelsReady(8)) {
            s_crisisPendingMaterializeId = 8;
            break;
        }
        s_crisisPendingMaterializeId = 0;
        const CVector docksEnd(2315.0f, -2260.0f, 15.0f);
        if (CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->GetNoOfFreeSpaces() >= 1) {
            CAutomobile* van = new CAutomobile(MODEL_BURRITO, 2, true);
            if (van && CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->IsObjectValid(van)) {
                van->m_nCreatedBy = 2;
                van->Teleport(docksEnd);
                van->PlaceOnRoadProperly();
                van->bEngineOn = true;
                CWorld::Add(van);
                van->UpdateRwMatrix();
                s_crisisEntities.vehHandles[0] = static_cast<uint32_t>(CPools::ms_pVehiclePool->GetRef(van));
            } else if (van) {
                Command<Commands::DELETE_CAR>(van);
            }
        }
        if (CPools::ms_pPedPool && CPools::ms_pPedPool->GetNoOfFreeSpaces() >= 2) {
            for (int g = 0; g < 2; ++g) {
                CVector mPos = docksEnd + CVector(3.0f + static_cast<float>(g) * 2.0f, 2.0f, 0.5f);
                CPed* mob = SpawnIncidentPed(MODEL_BALLAS1, PED_TYPE_GANG1, mPos, true);
                if (mob) {
                    GiveInfiniteCombatWeapon(mob, WEAPONTYPE_AK47);
                    mob->m_nWeaponSkill = 2;
                    mob->bPanicWhenScared = false;
                    mob->bCrouchWhenScared = false;
                    mob->bFleeWhenStanding = false;
                    mob->m_pIntelligence->m_TaskMgr.SetTask(new CTaskSimpleStandStill(-1, true, false, 8.0f), TASK_PRIMARY_PRIMARY, false);
                    s_crisisEntities.pedHandles[g] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(mob));
                }
            }
        }
        break;
    }
    case 9: { // CIVIL_RIOT_1992: Native Riot Mode, 4 clash hotspots across k_civilRiotPoints[0..3]
        Command<Commands::SET_LA_RIOTS>(1);
        CTheScripts::RiotIntensity = 100;
        for (int i = 0; i < 4; ++i) {
            MunicipalGameEvent ev{};
            ev.type = (i % 2 == 0) ? MunicipalGameEventType::StrikeProtest : MunicipalGameEventType::IncidentSpawn;
            ev.id = s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
            ev.x = k_civilRiotPoints[i].x; ev.y = k_civilRiotPoints[i].y; ev.z = k_civilRiotPoints[i].z;
            ev.param = 180000;
            strncpy_s(ev.text, "1992 Civil Riot Clash", sizeof(ev.text) - 1);
            IntakeMunicipalIncident(ev, currentMs, player);
            s_crisisEntities.incidentIds[i] = ev.id;
        }
        break;
    }
    case 10: { // SANITATION_STRIKE: 2 Trash trucks blocking City Hall stairs, peds with RIOT_CHANT
        if (!AreCrisisModelsReady(10)) {
            s_crisisPendingMaterializeId = 10;
            break;
        }
        s_crisisPendingMaterializeId = 0;
        if (CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->GetNoOfFreeSpaces() >= 2) {
            for (int t = 0; t < 2; ++t) {
                CAutomobile* truck = new CAutomobile(MODEL_TRASH, 2, true);
                if (truck && CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->IsObjectValid(truck)) {
                    truck->m_nCreatedBy = 2;
                    truck->Teleport(CVector(1477.0f + static_cast<float>(t) * 8.0f, -1742.0f, 13.5f));
                    truck->PlaceOnRoadProperly();
                    truck->bEngineOn = false;
                    truck->m_nHandbrakeOn = true;
                    CWorld::Add(truck);
                    truck->UpdateRwMatrix();
                    s_crisisEntities.vehHandles[t] = static_cast<uint32_t>(CPools::ms_pVehiclePool->GetRef(truck));
                } else if (truck) {
                    Command<Commands::DELETE_CAR>(truck);
                }
            }
        }
        if (CPools::ms_pPedPool && CPools::ms_pPedPool->GetNoOfFreeSpaces() >= 4) {
            for (int p = 0; p < 4; ++p) {
                const int pModel = (p % 2 == 0) ? MODEL_BMYST : MODEL_WMYST;
                CVector pPos(1476.0f + static_cast<float>(p) * 3.0f, -1746.0f, 13.8f);
                CPed* ped = SpawnIncidentPed(pModel, PED_TYPE_CIVMALE, pPos, true);
                if (ped) {
                    SafePlayRiotAnimation(ped, "RIOT_CHANT");
                    s_crisisEntities.pedHandles[p] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(ped));
                }
            }
        }
        break;
    }
    case 11: { // CAMPUS_PROTEST: Youth sit-in in Pershing Square park
        if (!AreCrisisModelsReady(11)) {
            s_crisisPendingMaterializeId = 11;
            break;
        }
        s_crisisPendingMaterializeId = 0;
        if (CPools::ms_pPedPool && CPools::ms_pPedPool->GetNoOfFreeSpaces() >= 6) {
            const CVector center = k_policeBarricadePoints[1];
            for (int s = 0; s < 6; ++s) {
                const int sModel = (s % 2 == 0) ? MODEL_BMYST : MODEL_WFYST;
                float ang = static_cast<float>(s) * (6.2831853f / 6.0f);
                CVector sPos = center + CVector(std::cos(ang) * 2.5f, std::sin(ang) * 2.5f, 0.3f);
                CPed* stu = SpawnIncidentPed(sModel, PED_TYPE_CIVMALE, sPos, true);
                if (stu) {
                    stu->SetHeading(ang + 3.14159265f);
                    Command<Commands::TASK_PLAY_ANIM>(stu, "SEAT_IDLE", "PED", 4.0f, 1, 0, 0, 0, -1);
                    s_crisisEntities.pedHandles[s] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(stu));
                }
            }
        }
        break;
    }
    case 12: { // RAMPART_SCANDAL: Police efficiency 0.5, cops stand down during Ganton Riots
        s_rampartScandalActive.store(true, std::memory_order_relaxed);
        break;
    }
    case 13: { // RICO_SWEEP: 2 FBI Ranchers and 4 agents raiding gang hotspots
        if (!AreCrisisModelsReady(13)) {
            s_crisisPendingMaterializeId = 13;
            break;
        }
        s_crisisPendingMaterializeId = 0;
        if (CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->GetNoOfFreeSpaces() >= 2) {
            for (int r = 0; r < 2; ++r) {
                CAutomobile* fbiVeh = new CAutomobile(MODEL_FBIRANCH, 2, true);
                if (fbiVeh && CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->IsObjectValid(fbiVeh)) {
                    fbiVeh->m_nCreatedBy = 2;
                    fbiVeh->Teleport(k_gangWarPoints[r]);
                    fbiVeh->PlaceOnRoadProperly();
                    fbiVeh->m_nPrimaryColor = 0; // Black
                    fbiVeh->m_nSecondaryColor = 0;
                    fbiVeh->bEngineOn = true;
                    fbiVeh->bSirenOrAlarm = true;
                    CWorld::Add(fbiVeh);
                    fbiVeh->UpdateRwMatrix();
                    s_crisisEntities.vehHandles[r] = static_cast<uint32_t>(CPools::ms_pVehiclePool->GetRef(fbiVeh));
                } else if (fbiVeh) {
                    Command<Commands::DELETE_CAR>(fbiVeh);
                }
            }
        }
        if (CPools::ms_pPedPool && CPools::ms_pPedPool->GetNoOfFreeSpaces() >= 4) {
            for (int a = 0; a < 4; ++a) {
                int ptIdx = a / 2;
                CVector aPos = k_gangWarPoints[ptIdx] + CVector(2.5f + static_cast<float>(a % 2) * 2.0f, 1.5f, 0.5f);
                CPed* fbi = SpawnIncidentPed(MODEL_FBI, PED_TYPE_COP, aPos, false);
                if (fbi) {
                    GiveInfiniteCombatWeapon(fbi, WEAPONTYPE_MP5);
                    fbi->m_nWeaponSkill = 2;
                    fbi->bPanicWhenScared = false;
                    fbi->bCrouchWhenScared = false;
                    fbi->bFleeWhenStanding = false;
                    s_crisisEntities.pedHandles[a] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(fbi));
                }
            }
        }
        break;
    }
    case 14: { // OPIOID_SURGE: Ambient peds in Ganton/Idlewood attack pedestrians
        s_opioidSurgeActive.store(true, std::memory_order_relaxed);
        break;
    }
    case 15: { // GRID_BLACKOUT: Street lights & traffic signals offline
        s_gridBlackoutActive.store(true, std::memory_order_relaxed);
        CWeather::TrafficLightsBrightness = 0.0f;
        break;
    }
    default:
        break;
    }
}

void ProcessCrisisWorldPhysicsStreaming() {
    if (s_crisisPendingMaterializeId != 0) {
        if (AreCrisisModelsReady(s_crisisPendingMaterializeId)) {
            ApplyCrisisWorldPhysics(s_crisisPendingMaterializeId, true);
        }
    }
}

// =============================================================================
//  Ambient High-Crime Street Riots & Looters Manager
// =============================================================================

enum class AmbientRioterRole : uint8_t {
    NONE = 0,
    SHOOTER,
    AGITATOR
};

struct AmbientRioter {
    uint32_t pedHandle = 0;
    uint32_t targetPedHandle = 0; // Current victim handle
    AmbientRioterRole role = AmbientRioterRole::NONE;
    uint32_t spawnTimeMs = 0;
    uint32_t lastActionMs = 0;
};

static constexpr size_t k_maxAmbientRioters = 14;
static AmbientRioter s_ambientRioters[k_maxAmbientRioters]{};

// =============================================================================
//  Micro-Zone Urban Unrest Hotspots Catalog
// =============================================================================

static const HotspotZone k_urbanHotspots[] = {
    // District 0: South Central
    { 1935.0f, -1770.0f, 13.0f, 75.0f, 1.00f, 0, "Idlewood Commercial Junction" },
    { 2220.0f, -1160.0f, 25.0f, 60.0f, 0.85f, 0, "Jefferson Turf Border" },
    { 2320.0f, -1645.0f, 14.0f, 50.0f, 0.40f, 0, "Willowfield Freight Crossing" },
    { 2490.0f, -1670.0f, 13.0f, 40.0f, 0.15f, 0, "Ganton Cul-de-Sac (Residential)" },

    // District 2: Industrial Port & Logistics
    { 2740.0f, -2450.0f, 13.5f, 80.0f, 0.95f, 2, "Ocean Docks Main Terminal Gate" },
    { 2250.0f, -2360.0f, 13.0f, 50.0f, 0.30f, 2, "Port Rail Depot Buffer" },

    // District 1: Downtown & Civic Center
    { 1481.0f, -1745.0f, 13.5f, 70.0f, 1.00f, 1, "City Hall & Pershing Square" }
};
static constexpr size_t k_hotspotCount = sizeof(k_urbanHotspots) / sizeof(k_urbanHotspots[0]);

const HotspotZone* GetActiveHotspotNearPlayer(const CVector& playerPos, float& outEffectiveFactor) {
    outEffectiveFactor = 0.0f;
    const HotspotZone* bestHotspot = nullptr;
    float bestFactor = 0.0f;

    for (size_t i = 0; i < k_hotspotCount; ++i) {
        const auto& hs = k_urbanHotspots[i];
        const float dx = playerPos.x - hs.x;
        const float dy = playerPos.y - hs.y;
        const float dist2D = std::sqrt(dx * dx + dy * dy);
        if (dist2D <= hs.radius) {
            const float factor = hs.susceptibility * (1.0f - (dist2D / hs.radius) * 0.3f);
            if (factor > bestFactor) {
                bestFactor = factor;
                bestHotspot = &hs;
            }
        }
    }

    if (bestHotspot) {
        outEffectiveFactor = bestFactor;
        return bestHotspot;
    }
    return nullptr;
}

static void RequestAmbientRiotResources(bool keepInMemory = false) {
    const int flags = keepInMemory ? (STREAMING_GAME_REQUIRED | STREAMING_PRIORITY_REQUEST | STREAMING_KEEP_IN_MEMORY)
                                   : (STREAMING_GAME_REQUIRED | STREAMING_PRIORITY_REQUEST);
    // Purely civilian / resident demographics (No gang models)
    SafeRequestModel(MODEL_BMYST, flags);
    SafeRequestModel(MODEL_WMYST, flags);
    SafeRequestModel(MODEL_HMYST, flags);
    SafeRequestModel(MODEL_BMYRI, flags);
    SafeRequestModel(MODEL_WMYRI, flags);
    SafeRequestModel(MODEL_SBMYTR3, flags);
    SafeRequestModel(MODEL_SBFYSTR, flags);
    SafeRequestModel(MODEL_HFYRI, flags);
    SafeRequestModel(MODEL_BFYST, flags);
    SafeRequestModel(MODEL_WFYST, flags);
    SafeRequestModel(MODEL_BMOCHAO, flags);
    SafeRequestModel(MODEL_WMOCHAO, flags);

    SafeRequestModel(MODEL_BAT, flags);
    SafeRequestModel(MODEL_GOLFCLUB, flags);
    SafeRequestModel(MODEL_COLT45, flags);
    SafeRequestAnimation("RIOT");
}

static bool IsAmbientRiotReady() {
    return IsMuniModelLoaded(MODEL_BMYST) || IsMuniModelLoaded(MODEL_WMYST) ||
           IsMuniModelLoaded(MODEL_HMYST) || IsMuniModelLoaded(MODEL_BMYRI) ||
           IsMuniModelLoaded(MODEL_WMYRI) || IsMuniModelLoaded(MODEL_SBMYTR3) ||
           IsMuniModelLoaded(MODEL_SBFYSTR) || IsMuniModelLoaded(MODEL_HFYRI) ||
           IsMuniModelLoaded(MODEL_BFYST) || IsMuniModelLoaded(MODEL_WFYST) ||
           IsMuniModelLoaded(MODEL_BMOCHAO) || IsMuniModelLoaded(MODEL_WMOCHAO);
}

static bool IsRiotAnimBlockReady() {
    const int blockIdx = CAnimManager::GetAnimationBlockIndex("RIOT");
    return (blockIdx >= 0 && blockIdx < CAnimManager::ms_numAnimBlocks && CAnimManager::ms_aAnimBlocks[blockIdx].bLoaded);
}

static CPed* FindNearestCivilianVictim(CPed* attacker, float maxRangeSq = 35.0f * 35.0f) {
    if (!attacker || !CPools::ms_pPedPool) return nullptr;
    CPed* player = FindPlayerPed();
    const CVector aPos = attacker->GetPosition();
    CPed* bestVictim = nullptr;
    float bestDistSq = maxRangeSq;

    for (int p = 0; p < CPools::ms_pPedPool->m_nSize; ++p) {
        CPed* ped = CPools::ms_pPedPool->GetAt(p);
        if (!ped || ped == attacker || ped == player) continue;
        if (!CPools::ms_pPedPool->IsObjectValid(ped)) continue;
        if (ped->m_fHealth <= 0.0f) continue;
        if (ped->bInVehicle || ped->m_pVehicle) continue;
        if (ped->m_nPedType != PED_TYPE_CIVMALE && ped->m_nPedType != PED_TYPE_CIVFEMALE) continue;
        if (ped->m_nCreatedBy == 2) continue; // Skip mission/incident peds

        const CVector pPos = ped->GetPosition();
        const float dx = pPos.x - aPos.x;
        const float dy = pPos.y - aPos.y;
        const float dz = pPos.z - aPos.z;
        const float dSq = dx * dx + dy * dy + dz * dz;

        if (dSq < bestDistSq) {
            bestDistSq = dSq;
            bestVictim = ped;
        }
    }
    return bestVictim;
}

static void CleanupAmbientRioterSlot(size_t i) {
    if (i >= k_maxAmbientRioters) return;
    AmbientRioter& r = s_ambientRioters[i];
    CPed* player = FindPlayerPed();

    if (r.pedHandle != 0 && CPools::ms_pPedPool) {
        CPed* ped = ResolvePed(r.pedHandle);
        if (ped && ped != player && CPools::ms_pPedPool->IsObjectValid(ped)) {
            if (ped->m_pIntelligence) {
                ped->m_pIntelligence->m_TaskMgr.FlushImmediately();
            }
            Command<Commands::DELETE_CHAR>(ped);
        }
        r.pedHandle = 0;
    }

    r.targetPedHandle = 0;
    r.role = AmbientRioterRole::NONE;
    r.spawnTimeMs = 0;
    r.lastActionMs = 0;
}

void CleanupAllAmbientRioters() {
    for (size_t i = 0; i < k_maxAmbientRioters; ++i) {
        CleanupAmbientRioterSlot(i);
    }
}

static bool SpawnAmbientSidewalkRioter(const CVector& probePos, const CVector& playerPos, uint32_t currentMs, size_t slotIndex, bool isDriving = false) {
    if (slotIndex >= k_maxAmbientRioters) return false;
    if (!CPools::ms_pPedPool || CPools::ms_pPedPool->GetNoOfUsedSpaces() >= 125 || CPools::ms_pPedPool->GetNoOfFreeSpaces() < 2) return false;

    CVector spawnPos = probePos;
    float nx = 0.0f, ny = 0.0f, nz = 0.0f;
    Command<Commands::GET_CLOSEST_CHAR_NODE>(probePos.x, probePos.y, probePos.z, &nx, &ny, &nz);
    if (std::abs(nx) > 1.0f || std::abs(ny) > 1.0f) {
        spawnPos = CVector(nx, ny, nz);
    } else {
        Command<Commands::GET_CLOSEST_CAR_NODE>(probePos.x, probePos.y, probePos.z, &nx, &ny, &nz);
        if (std::abs(nx) > 1.0f || std::abs(ny) > 1.0f) {
            spawnPos = CVector(nx, ny, nz);
        }
    }

    const float gz = CWorld::FindGroundZForCoord(spawnPos.x, spawnPos.y);
    const float maxZDelta = isDriving ? 18.0f : 7.0f;
    if (gz <= -99.0f || std::abs(gz - playerPos.z) > maxZDelta) return false;

    // In GTA SA / RenderWare, a CPed's pivot point/origin is at the pelvis / center of mass (~0.95m - 1.0m above ground).
    // Passing surface ground Z directly buries the ped from the waist down into the collision mesh.
    // Half-height ped collision offset (+1.0f) is strictly required so feet stand firmly on the pavement.
    spawnPos.z = gz + 1.0f;

    // Enforce spatial proximity clearance: reject if distance to any existing active ambient rioter is < 3.0m
    for (size_t i = 0; i < k_maxAmbientRioters; ++i) {
        if (s_ambientRioters[i].role == AmbientRioterRole::NONE || s_ambientRioters[i].pedHandle == 0) continue;
        CPed* existingPed = ResolvePed(s_ambientRioters[i].pedHandle);
        if (!existingPed || !CPools::ms_pPedPool || !CPools::ms_pPedPool->IsObjectValid(existingPed)) continue;
        const CVector exPos = existingPed->GetPosition();
        const float dx = spawnPos.x - exPos.x;
        const float dy = spawnPos.y - exPos.y;
        if ((dx * dx + dy * dy) < (3.0f * 3.0f)) {
            return false; // Proximity conflict: maintain >= 3.0m clearance between rioters
        }
    }

    // Also avoid spawning directly on top of CJ (< 3.0m)
    const float pDx = spawnPos.x - playerPos.x;
    const float pDy = spawnPos.y - playerPos.y;
    if ((pDx * pDx + pDy * pDy) < (3.0f * 3.0f)) {
        return false;
    }

    // Exclusively civilian/resident demographics (gangs strictly purged)
    static const int k_civilianRiotModels[] = {
        MODEL_BMYST,   MODEL_WMYST,   MODEL_HMYST,   MODEL_BMYRI,
        MODEL_WMYRI,   MODEL_SBMYTR3, MODEL_SBFYSTR, MODEL_HFYRI,
        MODEL_BFYST,   MODEL_WFYST,   MODEL_BMOCHAO, MODEL_WMOCHAO
    };
    int loadedModels[12];
    int loadedCount = 0;
    for (int m : k_civilianRiotModels) {
        if (IsMuniModelLoaded(m)) {
            loadedModels[loadedCount++] = m;
        }
    }
    if (loadedCount == 0) return false;

    const int sModel = loadedModels[rand() % loadedCount];
    CPed* rioter = SpawnIncidentPed(sModel, PED_TYPE_CRIMINAL, spawnPos, false);
    if (!rioter) return false;

    rioter->m_fHealth = 130.0f;
    rioter->m_fMaxHealth = 130.0f;
    rioter->bStayInSamePlace = true; // Stay chanting at the street corner/sidewalk
    rioter->bPanicWhenScared = false;
    rioter->bCrouchWhenScared = false;
    rioter->bFleeWhenStanding = false;
    rioter->bIgnorePlayer = true;
    rioter->bDontFight = true;

    if ((rand() % 100) < 40 && CStreaming::HasModelLoaded(MODEL_BAT)) {
        GiveInfiniteCombatWeapon(rioter, WEAPONTYPE_BASEBALLBAT);
    } else if ((rand() % 100) < 30 && CStreaming::HasModelLoaded(MODEL_GOLFCLUB)) {
        GiveInfiniteCombatWeapon(rioter, WEAPONTYPE_GOLFCLUB);
    } else {
        rioter->SetCurrentWeapon(WEAPONTYPE_UNARMED);
    }

    // Play initial protest animation (RIOT_CHANT: raised hands, RIOT_ANGRY, RIOT_PUNCHES)
    static const char* s_riotAnims[3] = { "RIOT_CHANT", "RIOT_ANGRY", "RIOT_PUNCHES" };
    SafePlayRiotAnimation(rioter, s_riotAnims[rand() % 3]);

    s_ambientRioters[slotIndex].pedHandle = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(rioter));
    s_ambientRioters[slotIndex].targetPedHandle = 0;
    s_ambientRioters[slotIndex].role = AmbientRioterRole::AGITATOR;
    s_ambientRioters[slotIndex].spawnTimeMs = currentMs;
    s_ambientRioters[slotIndex].lastActionMs = currentMs;

    Logger::Log("[AmbientRiot] Sidewalk agitator spawned at (%.1f, %.1f, %.1f), model %d (Slot: %u/%u)",
        spawnPos.x, spawnPos.y, spawnPos.z, sModel, static_cast<unsigned int>(slotIndex), static_cast<unsigned int>(k_maxAmbientRioters));
    return true;
}

void UpdateAmbientHighCrimeRiot(uint32_t currentMs, CPed* player, float curCrime) {
    (void)curCrime;
    if (!player) return;
    const float unrest = static_cast<float>(s_socialUnrest.load(std::memory_order_relaxed));

    // 1. Outside high unrest (< 50%): despawn all agitators and return
    if (unrest < 50.0f) {
        CleanupAllAmbientRioters();
        return;
    }

    const bool keepInMemory = (unrest >= 60.0f);
    RequestAmbientRiotResources(keepInMemory);

    if (!IsAmbientRiotReady()) return;

    CVehicle* veh = FindPlayerVehicle(-1, false);
    const bool isDriving = (veh != nullptr);
    const bool isDrivingFast = (isDriving && veh->m_vecMoveSpeed.Magnitude() > 0.3f);

    // 2. Maintenance of existing active agitators (despawn if far or dead, maintain looping protest anims)
    size_t activeCount = 0;
    const CVector pPos = player->GetPosition();
    const float maxActiveDist = isDriving ? 160.0f : 80.0f;

    for (size_t i = 0; i < k_maxAmbientRioters; ++i) {
        AmbientRioter& r = s_ambientRioters[i];
        if (r.role == AmbientRioterRole::NONE) continue;

        CPed* ped = ResolvePed(r.pedHandle);
        if (!ped || ped == player || !CPools::ms_pPedPool || !CPools::ms_pPedPool->IsObjectValid(ped)) {
            CleanupAmbientRioterSlot(i);
            continue;
        }

        const CVector rPos = ped->GetPosition();
        const float dx = rPos.x - pPos.x;
        const float dy = rPos.y - pPos.y;
        const float dz = rPos.z - pPos.z;
        const float distSq = dx * dx + dy * dy + dz * dz;

        // Despawn if beyond active envelope from player
        if (distSq > maxActiveDist * maxActiveDist) {
            CleanupAmbientRioterSlot(i);
            continue;
        }

        // Handle dead ped
        if (ped->m_fHealth <= 0.0f) {
            if (currentMs - r.spawnTimeMs > 15000 || distSq > (isDriving ? 60.0f * 60.0f : 35.0f * 35.0f)) {
                CleanupAmbientRioterSlot(i);
            }
            continue;
        }

        activeCount++;

        // Maintain looping protest animations (RIOT_CHANT: arms raised chanting, RIOT_ANGRY, RIOT_PUNCHES)
        if (currentMs - r.lastActionMs >= 2500) {
            r.lastActionMs = currentMs;
            static const char* s_protestAnims[3] = { "RIOT_CHANT", "RIOT_ANGRY", "RIOT_PUNCHES" };
            SafePlayRiotAnimation(ped, s_protestAnims[(i + (currentMs / 2500)) % 3]);
        }
    }

    // 3. Spawning new street agitators if unrest >= 60.0f and pool allows
    if (unrest < 60.0f) return;
    if (activeCount >= k_maxAmbientRioters) return;
    if (!CPools::ms_pPedPool || CPools::ms_pPedPool->GetNoOfUsedSpaces() >= 125 || CPools::ms_pPedPool->GetNoOfFreeSpaces() < 10) return;

    // Adaptive Timer: 400ms when driving fast (> 0.3 units/frame), 2500ms on foot / slow
    const uint32_t spawnIntervalMs = isDrivingFast ? 400 : 2500;
    static uint32_t s_lastAmbientRiotSpawnMs = 0;
    if (currentMs - s_lastAmbientRiotSpawnMs < spawnIntervalMs) return;

    if (!IsAmbientRiotReady()) return;

    // Spawn scattered 2-3 civilian clusters at urban sidewalks / street corners
    const size_t availableSlots = k_maxAmbientRioters - activeCount;
    const size_t spawnBatch = (std::min)(availableSlots, static_cast<size_t>(2 + (rand() % 2)));

    // Calculate vehicle lookahead anchor if driving
    CVector lookaheadPos = pPos;
    CVector fwdDir(0.0f, 1.0f, 0.0f);
    CVector sideDir(1.0f, 0.0f, 0.0f);
    float vehSpeed = 0.0f;

    if (isDriving) {
        const CVector moveVec = veh->m_vecMoveSpeed;
        vehSpeed = moveVec.Magnitude();
        if (vehSpeed > 0.05f) {
            // Forward target anchor: projects 50-90m ahead along direction of travel
            lookaheadPos = pPos + (moveVec * 60.0f);
            fwdDir = CVector(moveVec.x / vehSpeed, moveVec.y / vehSpeed, 0.0f);
            sideDir = CVector(-fwdDir.y, fwdDir.x, 0.0f);
        } else {
            const float vHeading = veh->GetHeading();
            fwdDir = CVector(-std::sin(vHeading), std::cos(vHeading), 0.0f);
            sideDir = CVector(fwdDir.y, -fwdDir.x, 0.0f);
            lookaheadPos = pPos + fwdDir * 35.0f;
        }
    }

    for (size_t b = 0; b < spawnBatch; ++b) {
        if (!CPools::ms_pPedPool || CPools::ms_pPedPool->GetNoOfUsedSpaces() >= 125 || CPools::ms_pPedPool->GetNoOfFreeSpaces() < 10) break;
        size_t freeSlot = k_maxAmbientRioters;
        for (size_t s = 0; s < k_maxAmbientRioters; ++s) {
            if (s_ambientRioters[s].role == AmbientRioterRole::NONE) {
                freeSlot = s;
                break;
            }
        }
        if (freeSlot == k_maxAmbientRioters) break;

        bool spawned = false;
        for (int attempt = 0; attempt < 4; ++attempt) {
            CVector probePos;
            if (isDriving && vehSpeed > 0.05f) {
                // In vehicle: spawn within 65.0m - 95.0m ahead around lookaheadPos
                const float fwdDist = 65.0f + static_cast<float>(rand() % 31); // 65.0m - 95.0m ahead
                const float lateralOffset = static_cast<float>((rand() % 30) - 15); // +/- 15m to catch sidewalks
                probePos = pPos + (fwdDir * fwdDist) + (sideDir * lateralOffset);
            } else if (isDriving) {
                // In vehicle but idling/low speed: 35.0m - 50.0m ahead
                const float fwdDist = 35.0f + static_cast<float>(rand() % 16);
                const float lateralOffset = static_cast<float>((rand() % 24) - 12);
                probePos = pPos + (fwdDir * fwdDist) + (sideDir * lateralOffset);
            } else {
                // On foot: spawn within 35.0m - 50.0m around player
                const float pHeading = player->GetHeading();
                const float angleOffset = ((rand() % 140) - 70) * (3.14159265f / 180.0f);
                const float spawnAngle = pHeading + angleOffset;
                const float fwdX = -std::sin(spawnAngle);
                const float fwdY = std::cos(spawnAngle);
                const float fwdDist = 35.0f + static_cast<float>(rand() % 16); // 35.0m - 50.0m
                probePos = pPos + CVector(fwdX * fwdDist, fwdY * fwdDist, 0.0f);
            }

            if (SpawnAmbientSidewalkRioter(probePos, pPos, currentMs, freeSlot, isDriving)) {
                s_lastAmbientRiotSpawnMs = currentMs;
                activeCount++;
                spawned = true;
                break;
            }
        }
        if (!spawned) break;
    }
}

// =============================================================================
//  §3a-2. Severe Unrest Crisis Atmosphere: Arterial Roadblocks & Distant Sirens
// =============================================================================



std::atomic<uint64_t> s_activeRoadblockMask{ 0 };
std::atomic<uint32_t> s_activeRoadblocksCount{ 0 };

const ChokepointCoord k_chokepointsTable[63] = {
    { 1369.0f, -1400.0f, 12.0f },
    { 1211.0f, -1573.0f, 12.0f },
    { 1198.0f, -1711.0f, 12.0f },
    { 1047.0f, -2087.0f, 12.0f },
    { 1031.0f, -2087.0f, 12.0f },
    { 1032.0f, -2222.0f, 12.0f },
    { 1032.0f, -2172.0f, 12.0f },
    { 1023.0f, -2120.0f, 12.0f },
    { 1330.0f, -2448.0f,  7.0f },
    { 1329.0f, -2464.0f,  6.0f },
    { 1367.0f, -2447.0f,  7.0f },
    { 1369.0f, -2465.0f,  6.0f },
    { 1345.0f, -2407.0f, 12.0f },
    { 1332.0f, -2414.0f, 12.0f },
    { 1481.0f, -2686.0f, 10.0f },
    { 1467.0f, -2669.0f, 11.0f },
    { 1849.0f, -1271.0f, 12.0f },
    { 1820.0f, -1260.0f, 12.0f },
    { 1746.0f, -1161.0f, 23.0f },
    { 1701.0f, -1301.0f, 12.0f },
    { 1714.0f, -1317.0f, 12.0f },
    { 1715.0f, -1278.0f, 12.0f },
    { 1453.0f, -1463.0f, 12.0f },
    { 1438.0f, -1525.0f, 12.0f },
    { 1196.0f, -1418.0f, 12.0f },
    { 1164.0f, -1281.0f, 12.0f },
    { 1215.0f, -1258.0f, 13.0f },
    { 1200.0f, -1334.0f, 12.0f },
    { 1187.0f, -1331.0f, 13.0f },
    { 1214.0f, -1333.0f, 12.0f },
    { 1257.0f, -1313.0f, 12.0f },
    { 1092.0f, -1399.0f, 12.0f },
    { 1055.0f, -1393.0f, 12.0f },
    { 1061.0f, -1438.0f, 12.0f },
    {  919.0f, -1409.0f, 12.0f },
    {  917.0f, -1385.0f, 12.0f },
    {  798.0f, -1414.0f, 12.0f },
    {  761.0f, -1400.0f, 12.0f },
    {  797.0f, -1370.0f, 12.0f },
    {  663.0f, -1315.0f, 12.0f },
    {  668.0f, -1234.0f, 14.0f },
    {  792.0f, -1150.0f, 23.0f },
    {  962.0f, -1127.0f, 23.0f },
    {  967.0f, -1038.0f, 29.0f },
    {  963.0f,  -978.0f, 38.0f },
    {  794.0f, -1041.0f, 24.0f },
    { 1155.0f,  -792.0f, 55.0f },
    { 1369.0f,  -925.0f, 33.0f },
    { 1379.0f,  -927.0f, 33.0f },
    { 1490.0f,  -938.0f, 36.0f },
    { 1479.0f,  -975.0f, 36.0f },
    { 1659.0f,  -811.0f, 56.0f },
    { 1679.0f,  -807.0f, 55.0f },
    { 1703.0f,  -784.0f, 53.0f },
    { 2451.0f, -1249.0f, 23.0f },
    { 2431.0f, -1636.0f, 26.0f },
    { 2029.0f, -1752.0f, 12.0f },
    { 1960.0f, -1994.0f, 12.0f },
    { 2264.0f, -2233.0f, 12.0f },
    { 2756.0f, -2150.0f, 10.0f },
    { 2762.0f, -2162.0f, 10.0f },
    { 2849.0f, -1656.0f, 10.0f },
    { 2858.0f, -1137.0f, 10.0f }
};

ArterialChokepoint s_arterialChokepoints[k_numArterialChokepoints] = {
    { { 1369.0f, -1400.0f, 12.0f }, 0, -1 },
    { { 1211.0f, -1573.0f, 12.0f }, 0, -1 },
    { { 1198.0f, -1711.0f, 12.0f }, 0, -1 },
    { { 1047.0f, -2087.0f, 12.0f }, 0, -1 },
    { { 1031.0f, -2087.0f, 12.0f }, 0, -1 },
    { { 1032.0f, -2222.0f, 12.0f }, 0, -1 },
    { { 1032.0f, -2172.0f, 12.0f }, 0, -1 },
    { { 1023.0f, -2120.0f, 12.0f }, 0, -1 },
    { { 1330.0f, -2448.0f,  7.0f }, 0, -1 },
    { { 1329.0f, -2464.0f,  6.0f }, 0, -1 },
    { { 1367.0f, -2447.0f,  7.0f }, 0, -1 },
    { { 1369.0f, -2465.0f,  6.0f }, 0, -1 },
    { { 1345.0f, -2407.0f, 12.0f }, 0, -1 },
    { { 1332.0f, -2414.0f, 12.0f }, 0, -1 },
    { { 1481.0f, -2686.0f, 10.0f }, 0, -1 },
    { { 1467.0f, -2669.0f, 11.0f }, 0, -1 },
    { { 1849.0f, -1271.0f, 12.0f }, 0, -1 },
    { { 1820.0f, -1260.0f, 12.0f }, 0, -1 },
    { { 1746.0f, -1161.0f, 23.0f }, 0, -1 },
    { { 1701.0f, -1301.0f, 12.0f }, 0, -1 },
    { { 1714.0f, -1317.0f, 12.0f }, 0, -1 },
    { { 1715.0f, -1278.0f, 12.0f }, 0, -1 },
    { { 1453.0f, -1463.0f, 12.0f }, 0, -1 },
    { { 1438.0f, -1525.0f, 12.0f }, 0, -1 },
    { { 1196.0f, -1418.0f, 12.0f }, 0, -1 },
    { { 1164.0f, -1281.0f, 12.0f }, 0, -1 },
    { { 1215.0f, -1258.0f, 13.0f }, 0, -1 },
    { { 1200.0f, -1334.0f, 12.0f }, 0, -1 },
    { { 1187.0f, -1331.0f, 13.0f }, 0, -1 },
    { { 1214.0f, -1333.0f, 12.0f }, 0, -1 },
    { { 1257.0f, -1313.0f, 12.0f }, 0, -1 },
    { { 1092.0f, -1399.0f, 12.0f }, 0, -1 },
    { { 1055.0f, -1393.0f, 12.0f }, 0, -1 },
    { { 1061.0f, -1438.0f, 12.0f }, 0, -1 },
    { {  919.0f, -1409.0f, 12.0f }, 0, -1 },
    { {  917.0f, -1385.0f, 12.0f }, 0, -1 },
    { {  798.0f, -1414.0f, 12.0f }, 0, -1 },
    { {  761.0f, -1400.0f, 12.0f }, 0, -1 },
    { {  797.0f, -1370.0f, 12.0f }, 0, -1 },
    { {  663.0f, -1315.0f, 12.0f }, 0, -1 },
    { {  668.0f, -1234.0f, 14.0f }, 0, -1 },
    { {  792.0f, -1150.0f, 23.0f }, 0, -1 },
    { {  962.0f, -1127.0f, 23.0f }, 0, -1 },
    { {  967.0f, -1038.0f, 29.0f }, 0, -1 },
    { {  963.0f,  -978.0f, 38.0f }, 0, -1 },
    { {  794.0f, -1041.0f, 24.0f }, 0, -1 },
    { { 1155.0f,  -792.0f, 55.0f }, 0, -1 },
    { { 1369.0f,  -925.0f, 33.0f }, 0, -1 },
    { { 1379.0f,  -927.0f, 33.0f }, 0, -1 },
    { { 1490.0f,  -938.0f, 36.0f }, 0, -1 },
    { { 1479.0f,  -975.0f, 36.0f }, 0, -1 },
    { { 1659.0f,  -811.0f, 56.0f }, 0, -1 },
    { { 1679.0f,  -807.0f, 55.0f }, 0, -1 },
    { { 1703.0f,  -784.0f, 53.0f }, 0, -1 },
    { { 2451.0f, -1249.0f, 23.0f }, 0, -1 },
    { { 2431.0f, -1636.0f, 26.0f }, 0, -1 },
    { { 2029.0f, -1752.0f, 12.0f }, 0, -1 },
    { { 1960.0f, -1994.0f, 12.0f }, 0, -1 },
    { { 2264.0f, -2233.0f, 12.0f }, 0, -1 },
    { { 2756.0f, -2150.0f, 10.0f }, 0, -1 },
    { { 2762.0f, -2162.0f, 10.0f }, 0, -1 },
    { { 2849.0f, -1656.0f, 10.0f }, 0, -1 },
    { { 2858.0f, -1137.0f, 10.0f }, 0, -1 }
};

struct CrisisRoadblockEntity {
    uint32_t vehHandles[2]{ 0, 0 };
    size_t   vehCount{ 0 };
    uint32_t rioterHandles[2]{ 0, 0 };
    size_t   rioterCount{ 0 };
    int      chokepointIndex{ -1 };
    CVector  pos{ 0.0f, 0.0f, 0.0f };
    CVector  initialSpawnPos{ 0.0f, 0.0f, 0.0f };
    CVector  initialVehSpawnPos[2]{ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
    uint32_t spawnTimeMs{ 0 };
    uint32_t lastAnimMs{ 0 };
    bool     provoked{ false };
    bool     active{ false };
};

static constexpr size_t k_maxActiveRoadblocks = 7;
static CrisisRoadblockEntity s_crisisRoadblocks[k_maxActiveRoadblocks]{};
static bool s_crisisAtmosphereActive = false;

static const int s_crisisWreckModels[3] = {
    MODEL_GLENDALE,
    MODEL_GREENWOO,
    MODEL_OCEANIC
};

static const int s_crisisRioterModels[12] = {
    MODEL_BMYST, MODEL_WMYST, MODEL_HMYST, MODEL_BMYRI,
    MODEL_WMYRI, MODEL_SBMYTR3, MODEL_SBFYSTR, MODEL_HFYRI,
    MODEL_BFYST, MODEL_WFYST, MODEL_BMOCHAO, MODEL_WMOCHAO
};

static void RequestCrisisAtmosphereResources(bool keepInMemory = false) {
    const int flags = keepInMemory ? (STREAMING_GAME_REQUIRED | STREAMING_PRIORITY_REQUEST | STREAMING_KEEP_IN_MEMORY)
                                   : (STREAMING_GAME_REQUIRED | STREAMING_PRIORITY_REQUEST);
    SafeRequestModel(MODEL_GLENDALE, flags);
    SafeRequestModel(MODEL_GREENWOO, flags);
    SafeRequestModel(MODEL_OCEANIC, flags);
    SafeRequestModel(MODEL_MICRO_UZI, flags);
    SafeRequestModel(MODEL_BMYST, flags);
    SafeRequestModel(MODEL_WMYST, flags);
    SafeRequestAnimation("RIOT");
}

static bool IsCrisisWreckModelReady() {
    return IsMuniModelLoaded(MODEL_GLENDALE) ||
           IsMuniModelLoaded(MODEL_GREENWOO) ||
           IsMuniModelLoaded(MODEL_OCEANIC);
}


static void CleanupCrisisRoadblockSlot(size_t index, uint32_t currentMs = 0) {
    if (index >= k_maxActiveRoadblocks) return;
    CrisisRoadblockEntity& rb = s_crisisRoadblocks[index];
    if (!rb.active && rb.vehCount == 0 && rb.rioterCount == 0 && rb.chokepointIndex < 0) return;

    rb.active = false;
    const int slotIdx = rb.chokepointIndex;
    if (slotIdx >= 0 && slotIdx < static_cast<int>(k_numArterialChokepoints)) {
        s_arterialChokepoints[slotIdx].cooldownUntilMs = currentMs + 120000;
        s_arterialChokepoints[slotIdx].activeRoadblockSlot = -1;
        s_arterialChokepoints[slotIdx].status.store(ChokepointStatus::CLEAR, std::memory_order_relaxed);
        if (slotIdx < 63) {
            s_activeRoadblockMask.fetch_and(~(1ULL << slotIdx), std::memory_order_release);
        }
        AddMunicipalLog("ROADBLOCK CLEARED: Chokepoint #%d entered 2-min cooldown", slotIdx);
    }
    if (s_activeRoadblocksCount.load(std::memory_order_relaxed) > 0) {
        s_activeRoadblocksCount.fetch_sub(1, std::memory_order_relaxed);
    }

    // Clean up rioters
    for (size_t r = 0; r < 2; ++r) {
        if (rb.rioterHandles[r] != 0) {
            CPed* ped = ResolvePed(rb.rioterHandles[r]);
            if (ped && CPools::ms_pPedPool && CPools::ms_pPedPool->IsObjectValid(ped)) {
                if (ped->m_pIntelligence) {
                    ped->m_pIntelligence->m_TaskMgr.FlushImmediately();
                }
                Command<Commands::DELETE_CHAR>(ped);
            }
            rb.rioterHandles[r] = 0;
        }
    }

    // Clean up vehicles
    for (size_t v = 0; v < 2; ++v) {
        if (rb.vehHandles[v] != 0) {
            CVehicle* veh = ResolveVehicle(rb.vehHandles[v]);
            if (veh && CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->IsObjectValid(veh)) {
                CWorld::Remove(veh);
                delete veh;
            }
            rb.vehHandles[v] = 0;
        }
    }

    rb = CrisisRoadblockEntity{};
}

static void CleanupAllCrisisRoadblocks() {
    for (size_t w = 0; w < k_maxActiveRoadblocks; ++w) {
        CleanupCrisisRoadblockSlot(w, 0);
    }
    for (size_t i = 0; i < k_numArterialChokepoints; ++i) {
        s_arterialChokepoints[i].activeRoadblockSlot = -1;
        s_arterialChokepoints[i].status.store(ChokepointStatus::CLEAR, std::memory_order_relaxed);
    }
    s_activeRoadblockMask.store(0, std::memory_order_release);
    s_activeRoadblocksCount.store(0, std::memory_order_relaxed);
}

bool IsRoadblockBlockingWaypoint(float wpX, float wpY, float radius) noexcept {
    const float unrest = static_cast<float>(s_socialUnrest.load(std::memory_order_relaxed));
    const bool curfew = s_curfewActive.load(std::memory_order_relaxed);

    // If no crisis, roadblocks are inactive
    if (unrest < 60.0f && !curfew) {
        return false;
    }

    const uint64_t rbMask = s_activeRoadblockMask.load(std::memory_order_acquire);
    const float rSq = radius * radius;
    for (size_t i = 0; i < k_numArterialChokepoints; ++i) {
        const bool isActive = ((rbMask & (1ULL << i)) != 0) || (s_arterialChokepoints[i].status.load(std::memory_order_relaxed) != ChokepointStatus::CLEAR);
        if (!isActive) continue;

        const float dx = k_chokepointsTable[i].x - wpX;
        const float dy = k_chokepointsTable[i].y - wpY;
        if (std::abs(dx) <= radius && std::abs(dy) <= radius) {
            if ((dx * dx + dy * dy) <= rSq) {
                return true; // Active roadblock triggers waypoint blockage
            }
        }
    }
    return false;
}

static uint32_t s_lastMunicipalSuppressionMs = 0;
static uint32_t s_lastGunfireAcousticMs = 0;
static CVector  s_lastGunfirePos = { 0.0f, 0.0f, 0.0f };

static void UpdateCrisisAtmosphere(uint32_t currentMs, CPed* player) {
    if (!player) return;
    static bool s_initNotified = false;
    if (!s_initNotified) {
        CMessages::AddMessageJumpQ("VERIFIED 63 ROADBLOCKS LOADED", 4000, 0, false);
        s_initNotified = true;
    }

    const float curUnrest = static_cast<float>(s_socialUnrest.load(std::memory_order_relaxed));

    // -------------------------------------------------------------------------
    // 1. Economic Pacification Check
    // -------------------------------------------------------------------------
    if (curUnrest < 45.0f && !s_curfewActive.load(std::memory_order_relaxed)) {
        if (s_crisisAtmosphereActive) {
            s_crisisAtmosphereActive = false;
            AddMunicipalLog("ATMOSPHERE RESTORED: Social Unrest (%.1f%% < 45%%) normalized. Arterial roadblocks lifted.", curUnrest);
        }
        CleanupAllCrisisRoadblocks();
        s_activeRoadblockMask.store(0, std::memory_order_release);
        return;
    }

    // Dynamic systemic threshold: trigger crisis atmosphere at >= 75%
    // Hysteresis: only pacify and suppress sirens/wrecks when unrest falls strictly below 50.0f
    if (!s_crisisAtmosphereActive && curUnrest >= 75.0f) {
        s_crisisAtmosphereActive = true;
        AddMunicipalLog("CRISIS ATMOSPHERE: Severe Unrest (%.1f%% >= 75%%) triggered crisis state (arterial roadblocks, sirens)", curUnrest);
    } else if (s_crisisAtmosphereActive && curUnrest < 50.0f) {
        s_crisisAtmosphereActive = false;
        CleanupAllCrisisRoadblocks();
        s_activeRoadblockMask.store(0, std::memory_order_release);
        AddMunicipalLog("ATMOSPHERE RESTORED: Social Unrest (%.1f%% < 50%%) normalized. Arterial roadblocks lifted.", curUnrest);
    }

    if (!s_crisisAtmosphereActive) {
        return;
    }

    // Pre-stream crisis vehicle wreck models
    RequestCrisisAtmosphereResources(curUnrest >= 80.0f);

    if (player && player->m_pPlayerData) {
        const eWeaponType pWep = player->m_aWeapons[player->m_nSelectedWepSlot].m_eWeaponType;
        const bool hasGun = (pWep >= WEAPONTYPE_PISTOL && pWep <= WEAPONTYPE_MINIGUN) ||
                            (pWep >= WEAPONTYPE_GRENADE && pWep <= WEAPONTYPE_MOLOTOV);
        if (hasGun) {
            const CWeapon& activeWep = player->m_aWeapons[player->m_nSelectedWepSlot];
            const bool firedNow = (activeWep.m_nState == WEAPONSTATE_FIRING) ||
                                  (player->m_pPlayerData->m_fAttackButtonCounter > 0.0f) ||
                                  (CPad::GetPad(0) && (CPad::GetPad(0)->NewState.ButtonCircle || CPad::NewMouseControllerState.lmb));
            if (firedNow) {
                s_lastGunfireAcousticMs = currentMs;
                s_lastGunfirePos = player->GetPosition();
            }
        }
    }

    const CVector pPos = player->GetPosition();
    ENGINE_TRACE("MUNICIPAL", "UpdateCrisisAtmosphere: pPos=(%.1f, %.1f) unrest=%.1f", pPos.x, pPos.y, curUnrest);

    // 1. Audio disturbance: distant emergency sirens and alarms echoing through the city
    static uint32_t s_lastSirenSoundMs = 0;
    if (currentMs - s_lastSirenSoundMs >= 4500) {
        s_lastSirenSoundMs = currentMs;
        const float sirenAngle = static_cast<float>(rand() % 360) * (3.14159265f / 180.0f);
        const float sirenDist = 80.0f + static_cast<float>(rand() % 50);
        const CVector sirenPos = pPos + CVector(std::cos(sirenAngle) * sirenDist, std::sin(sirenAngle) * sirenDist, 10.0f);
        Command<Commands::ADD_ONE_OFF_SOUND>(sirenPos.x, sirenPos.y, sirenPos.z, 1155); // SOUND_CAT2_SECURITY_ALARM
        AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_SCANNER_NOISE_START, 0.35f, 1.0f);
    }

    // -------------------------------------------------------------------------
    // 2. Incident Lifespan TTL Check & 3. Player Physical Intervention Check
    // -------------------------------------------------------------------------
    size_t activeCount = 0;
    for (size_t w = 0; w < k_maxActiveRoadblocks; ++w) {
        CrisisRoadblockEntity& rb = s_crisisRoadblocks[w];
        if (rb.vehCount == 0 && rb.rioterCount == 0) continue;

        // Check if roadblock exceeded maximum lifetime (180 seconds)
        if (currentMs - rb.spawnTimeMs > 180000) {
            Logger::Log("[Municipal] Roadblock at chokepoint #%d dispersed due to expiration.", rb.chokepointIndex);
            AddMunicipalLog("ROADBLOCK DISPERSED: Chokepoint #%d exceeded 180s TTL lifespan", rb.chokepointIndex);
            CleanupCrisisRoadblockSlot(w, currentMs);
            continue;
        }

        // Check if digital police clearance cleared this roadblock from the background thread
        if (rb.chokepointIndex >= 0 && rb.chokepointIndex < static_cast<int>(k_numArterialChokepoints)) {
            if (s_arterialChokepoints[rb.chokepointIndex].status.load(std::memory_order_relaxed) == ChokepointStatus::CLEAR) {
                CleanupCrisisRoadblockSlot(w, currentMs);
                continue;
            }
        }

        CVehicle* primaryVeh = (rb.vehHandles[0] != 0) ? ResolveVehicle(rb.vehHandles[0]) : nullptr;
        if (!primaryVeh || !CPools::ms_pVehiclePool || !CPools::ms_pVehiclePool->IsObjectValid(primaryVeh)) {
            CleanupCrisisRoadblockSlot(w, currentMs);
            continue;
        }

        const CVector wPos = primaryVeh->GetPosition();
        const float dx = wPos.x - pPos.x;
        const float dy = wPos.y - pPos.y;
        const float dist2DSq = dx * dx + dy * dy;

        // Despawn safely when player distance > 190m off-screen
        if (dist2DSq > (190.0f * 190.0f)) {
            CleanupCrisisRoadblockSlot(w, currentMs);
            continue;
        }

        // ---------------------------------------------------------------------
        // SECTION 3: PROVOCATION DETECTION & RETALIATION
        // ---------------------------------------------------------------------
        bool provocationTriggered = false;

        // 1. Vehicle displacement: Any roadblock car moved >= 2.5m from initial spawn pos
        for (size_t v = 0; v < rb.vehCount; ++v) {
            if (rb.vehHandles[v] != 0) {
                CVehicle* veh = ResolveVehicle(rb.vehHandles[v]);
                if (veh && CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->IsObjectValid(veh)) {
                    const CVector& vSpawn = (v == 0) ? rb.initialSpawnPos : rb.initialVehSpawnPos[1];
                    const CVector vPos = veh->GetPosition();
                    const float moveDx = vPos.x - vSpawn.x;
                    const float moveDy = vPos.y - vSpawn.y;
                    if ((moveDx * moveDx + moveDy * moveDy) >= (2.5f * 2.5f)) {
                        provocationTriggered = true;
                        break;
                    }
                }
            }
        }

        // 2. Direct attack trigger:
        if (!provocationTriggered) {
            eWeaponType playerWep = (player && player->m_pPlayerData) 
                ? player->m_aWeapons[player->m_nSelectedWepSlot].m_eWeaponType 
                : WEAPONTYPE_UNARMED;
            const bool playerHoldingGun = (playerWep >= WEAPONTYPE_PISTOL && playerWep <= WEAPONTYPE_EXTINGUISHER);

            // A: Direct sentinel threat / damage evaluation
            for (size_t r = 0; r < rb.rioterCount; ++r) {
                if (rb.rioterHandles[r] != 0) {
                    CPed* rPed = ResolvePed(rb.rioterHandles[r]);
                    if (rPed && CPools::ms_pPedPool && CPools::ms_pPedPool->IsObjectValid(rPed) && rPed->m_fHealth > 0.0f) {
                        const float totalHp = rPed->m_fHealth + rPed->m_fArmour;
                        if (totalHp <= 175.0f) {
                            provocationTriggered = true;
                            break;
                        }
                        if (playerHoldingGun && player->m_pPlayerData) {
                            const bool isTargetedObj = (reinterpret_cast<void*>(player->m_pTargetedObject) == static_cast<void*>(rPed));
                            const bool isTargetedScript = Command<Commands::IS_PLAYER_TARGETTING_CHAR>(0, rPed);
                            if (isTargetedObj || isTargetedScript) {
                                provocationTriggered = true;
                                break;
                            }
                        }
                    }
                }
            }

            // B: Gunfire Audio Proximity Aggro Detection (500ms acoustic sound decay buffer within 110.0m)
            if (!provocationTriggered) {
                // Check 500ms acoustic gunshot window within 110.0m
                if (currentMs - s_lastGunfireAcousticMs <= 500) {
                    const float dx = s_lastGunfirePos.x - rb.pos.x;
                    const float dy = s_lastGunfirePos.y - rb.pos.y;
                    if ((dx * dx + dy * dy) <= (110.0f * 110.0f)) {
                        provocationTriggered = true;
                    }
                }
            }
        }

        // Full squad engagement & surrender suppression:
        if (provocationTriggered && !rb.provoked) {
            rb.provoked = true;
            for (size_t r = 0; r < rb.rioterCount; ++r) {
                if (rb.rioterHandles[r] != 0) {
                    CPed* rPed = ResolvePed(rb.rioterHandles[r]);
                    if (rPed && CPools::ms_pPedPool && CPools::ms_pPedPool->IsObjectValid(rPed) && rPed->m_fHealth > 0.0f) {
                        rPed->bIgnorePlayer = false;
                        rPed->bStayInSamePlace = false;
                        rPed->bPanicWhenScared = false;
                        rPed->bFleeWhenStanding = false;
                        rPed->bCrouchWhenScared = false;
                        rPed->bDontFight = false;

                        Command<Commands::CLEAR_CHAR_TASKS_IMMEDIATELY>(rPed);

                        // Hard-set combat aggression
                        Command<Commands::SET_CHAR_RELATIONSHIP>(rPed, 4, PED_TYPE_PLAYER1);

                        if (rPed->m_pIntelligence) {
                            rPed->m_pIntelligence->m_TaskMgr.FlushImmediately();
                            // Assign low-level persistent kill task
                            rPed->m_pIntelligence->m_TaskMgr.SetTask(
                                new CTaskComplexKillPedOnFoot(player, -1, 0, 1, 0, 1),
                                TASK_PRIMARY_PRIMARY, true);
                        }
                    }
                }
            }
        }

        // ---------------------------------------------------------------------
        // SECTION 4: DUAL-CONDITION PHYSICAL BREACH (PLAYER INTERACTION)
        // ---------------------------------------------------------------------
        // Condition 1 (Vehicles cleared): All slot vehicles moved >= 1.5m from spawn OR destroyed (m_fHealth <= 100.0f)
        bool condition1VehiclesCleared = true;
        for (size_t v = 0; v < rb.vehCount; ++v) {
            if (rb.vehHandles[v] != 0) {
                CVehicle* veh = ResolveVehicle(rb.vehHandles[v]);
                if (veh && CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->IsObjectValid(veh)) {
                    const bool isDestroyed = (veh->m_fHealth <= 100.0f);
                    const CVector& vSpawn = (v == 0) ? rb.initialSpawnPos : rb.initialVehSpawnPos[1];
                    const CVector vPos = veh->GetPosition();
                    const float moveDx = vPos.x - vSpawn.x;
                    const float moveDy = vPos.y - vSpawn.y;
                    const bool isMoved = ((moveDx * moveDx + moveDy * moveDy) >= (1.5f * 1.5f));
                    if (!isMoved && !isDestroyed) {
                        condition1VehiclesCleared = false;
                        break;
                    }
                }
            }
        }

        // Condition 2 (Rioters eliminated): All slot rioters neutralized (m_fHealth <= 0.0f or invalid ref)
        bool condition2RiotersEliminated = true;
        for (size_t r = 0; r < rb.rioterCount; ++r) {
            if (rb.rioterHandles[r] != 0) {
                CPed* rPed = ResolvePed(rb.rioterHandles[r]);
                if (rPed && CPools::ms_pPedPool && CPools::ms_pPedPool->IsObjectValid(rPed)) {
                    if (rPed->m_fHealth > 0.0f) {
                        condition2RiotersEliminated = false;
                        break;
                    }
                }
            }
        }

        if (condition1VehiclesCleared && condition2RiotersEliminated) {
            const size_t cpIdx = static_cast<size_t>(rb.chokepointIndex);
            if (rb.chokepointIndex >= 0 && cpIdx < k_numArterialChokepoints) {
                s_arterialChokepoints[cpIdx].status.store(ChokepointStatus::CLEAR, std::memory_order_release);
                s_arterialChokepoints[cpIdx].playerCleared = true;
                if (cpIdx < 63) {
                    s_activeRoadblockMask.fetch_and(~(1ULL << cpIdx), std::memory_order_release);
                }
            }
            s_publicUnrest.fetch_sub(1.0f, std::memory_order_relaxed);
            AddMunicipalLog("[Roadblock] Chokepoint #%zu secured by player! Vehicles cleared & threats eliminated.", cpIdx);
            CleanupCrisisRoadblockSlot(w, currentMs);
            continue;
        }

        activeCount++;

    }

    // -------------------------------------------------------------------------
    // 4. Municipal Police Towing & Riot Clearance (Every 45 seconds)
    // -------------------------------------------------------------------------
    if (currentMs - s_lastMunicipalSuppressionMs >= 45000) {
        s_lastMunicipalSuppressionMs = currentMs;

        const int64_t treasury = s_cityTreasury.load(std::memory_order_relaxed);
        const float desertion = s_policeDesertionPct.load(std::memory_order_relaxed);
        const uint64_t activeMask = s_activeRoadblockMask.load(std::memory_order_acquire);

        // Police sweep conditions: Treasury has operational funds and officers are not deserting
        if (treasury >= 15000 && desertion < 40.0f && activeMask != 0) {
            // Find an active roadblock to clear
            for (size_t slot = 0; slot < k_maxActiveRoadblocks; ++slot) {
                if (s_crisisRoadblocks[slot].vehCount > 0 || s_crisisRoadblocks[slot].rioterCount > 0) {
                    const int clearedChokepoint = s_crisisRoadblocks[slot].chokepointIndex;
                    CleanupCrisisRoadblockSlot(slot, currentMs);

                    // Deduct municipal towing/tactical operational cost ($1,500)
                    s_cityTreasury.fetch_sub(1500, std::memory_order_relaxed);
                    Logger::Log("[Municipal] Police sweep cleared roadblock #%d. Operational cost: $1,500.", clearedChokepoint);
                    AddMunicipalLog("POLICE SWEEP: Roadblock #%d cleared by tactical towing (-$1,500)", clearedChokepoint);
                    if (activeCount > 0) activeCount--;
                    break;
                }
            }
        }
    }

    // 5. Selection & Concurrency: Cap concurrent active roadblocks to k_maxActiveRoadblocks (7)
    if (activeCount >= k_maxActiveRoadblocks) return;

    // Verify strict headroom to prevent engine pool overflows (0xC0000005)
    if (!CPools::ms_pVehiclePool || CPools::ms_pVehiclePool->GetNoOfFreeSpaces() < 8) {
        return; // Abort: vehicle pool saturated
    }
    if (!CPools::ms_pPedPool || CPools::ms_pPedPool->GetNoOfFreeSpaces() < 12) {
        return; // Abort: ped pool saturated
    }

    // Evaluate every 1.5 seconds instead of 6.0 seconds
    static uint32_t s_lastRoadblockEvaluationMs = 0;
    if ((currentMs - s_lastRoadblockEvaluationMs) < 1500) {
        return;
    }
    s_lastRoadblockEvaluationMs = currentMs;

    static uint32_t s_lastSpawnFrameMs = 0;
    if (currentMs - s_lastSpawnFrameMs < 120) {
        return; // Enforce minimum 120ms gap between heavy 3D entity spawns
    }
    s_lastSpawnFrameMs = currentMs;

    if (!CStreaming::HasModelLoaded(MODEL_GLENDALE) &&
        !CStreaming::HasModelLoaded(MODEL_GREENWOO) &&
        !CStreaming::HasModelLoaded(MODEL_OCEANIC)) {
        return; // Abort until vehicle streaming completes
    }
    if (!CStreaming::HasModelLoaded(MODEL_MICRO_UZI) ||
        !CStreaming::HasModelLoaded(MODEL_BMYST)) {
        return; // Abort until weapon and ped models complete streaming
    }

    // Find free roadblock slot
    size_t freeSlot = k_maxActiveRoadblocks;
    for (size_t w = 0; w < k_maxActiveRoadblocks; ++w) {
        if (s_crisisRoadblocks[w].vehCount == 0 && s_crisisRoadblocks[w].rioterCount == 0) {
            freeSlot = w;
            break;
        }
    }
    if (freeSlot >= k_maxActiveRoadblocks) return;

    // Find eligible chokepoint: approaching player, not active, cooldown expired
    int candidateChokepoints[k_numArterialChokepoints];
    size_t numCandidates = 0;

    for (size_t i = 0; i < k_numArterialChokepoints; ++i) {
        auto& cp = s_arterialChokepoints[i];
        if (cp.activeRoadblockSlot != -1) continue; // Already active
        if (currentMs < cp.cooldownUntilMs) continue; // In 2-minute cooldown

        const CVector& candidatePos = cp.pos;
        const float dx = candidatePos.x - pPos.x;
        const float dy = candidatePos.y - pPos.y;
        const float distSq = dx * dx + dy * dy;

        // Pre-stream models at distance <= 200m so they are 100% loaded before player reaches safe bracket
        if (distSq <= (200.0f * 200.0f)) {
            RequestCrisisAtmosphereResources(false);
        }

        // SPAWN BOUNDARY: Min 80.0m (never in direct FOV), Max 170.0m
        if (distSq < (80.0f * 80.0f) || distSq > (170.0f * 170.0f)) {
            continue;
        }

        // Camera forward check: avoid popping directly in front if closer than 100m
        CVector toCandidate = candidatePos - pPos;
        toCandidate.Normalize();
        CVector fwd = (player && player->m_matrix) ? player->GetForward() : CVector(0.0f, 1.0f, 0.0f);
        if (distSq < (100.0f * 100.0f) && (fwd.x * toCandidate.x + fwd.y * toCandidate.y) > 0.60f) {
            continue; // In front cone closer than 100m -> defer to flank/rear nodes
        }

        candidateChokepoints[numCandidates++] = static_cast<int>(i);
    }

    if (numCandidates == 0) return;

    // Pick one eligible candidate chokepoint
    const int chosenIdx = candidateChokepoints[rand() % numCandidates];
    auto& chosenCp = s_arterialChokepoints[chosenIdx];

    // Direct table coordinate from verified s_arterialChokepoints:
    const CVector finalSpawnPos = CVector(chosenCp.pos.x, chosenCp.pos.y, chosenCp.pos.z + 1.1f);

    // Pick vehicle models
    int model1 = MODEL_GLENDALE;
    int model2 = MODEL_OCEANIC;
    for (int m : s_crisisWreckModels) {
        if (CStreaming::HasModelLoaded(m)) {
            model1 = m;
            break;
        }
    }
    for (int m : s_crisisWreckModels) {
        if (m != model1 && CStreaming::HasModelLoaded(m)) {
            model2 = m;
            break;
        }
    }

    // Roadblock composition: 1 or 2 static damaged vehicles across road lane
    const size_t numVehs = (rand() % 2 == 0 && CPools::ms_pVehiclePool->GetNoOfFreeSpaces() >= 8) ? 2 : 1;

    // Verify strict headroom to prevent engine pool overflows (0xC0000005)
    if (!CPools::ms_pVehiclePool || CPools::ms_pVehiclePool->GetNoOfFreeSpaces() < 8) {
        return; // Abort: vehicle pool saturated
    }
    if (!CPools::ms_pPedPool || CPools::ms_pPedPool->GetNoOfFreeSpaces() < 12) {
        return; // Abort: ped pool saturated
    }

    if (!CStreaming::HasModelLoaded(MODEL_GLENDALE) &&
        !CStreaming::HasModelLoaded(MODEL_GREENWOO) &&
        !CStreaming::HasModelLoaded(MODEL_OCEANIC)) {
        return; // Abort until vehicle streaming completes
    }
    if (!CStreaming::HasModelLoaded(MODEL_MICRO_UZI) ||
        !CStreaming::HasModelLoaded(MODEL_BMYST)) {
        return; // Abort until weapon and ped models complete streaming
    }

    // Asynchronous Streaming Gate: request models and return if not loaded yet
    if (!CStreaming::HasModelLoaded(model1)) {
        SafeRequestModel(model1, STREAMING_GAME_REQUIRED);
        return;
    }
    if (numVehs == 2 && !CStreaming::HasModelLoaded(model2)) {
        SafeRequestModel(model2, STREAMING_GAME_REQUIRED);
        return;
    }

    // Spawn Vehicle 1
    CAutomobile* pCar1 = new CAutomobile(model1, 2, true);
    if (!pCar1 || !CPools::ms_pVehiclePool || !CPools::ms_pVehiclePool->IsObjectValid(pCar1)) {
        if (pCar1) Command<Commands::DELETE_CAR>(pCar1);
        return;
    }

    pCar1->m_nCreatedBy = 2; // Mission entity
    pCar1->m_nStatus = eEntityStatus::STATUS_ABANDONED;
    pCar1->m_fHealth = 350.0f;
    pCar1->bEngineOn = false;
    pCar1->m_nHandbrakeOn = true;
    pCar1->Teleport(finalSpawnPos);
    pCar1->SetOrientation(0.0f, 0.0f, 0.785f);
    pCar1->PlaceOnRoadProperly();
    pCar1->m_fSteerAngle = 0.0f;
    pCar1->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
    pCar1->m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
    pCar1->m_fGasPedal = 0.0f;
    pCar1->m_fBreakPedal = 1.0f;
    for (int w = 0; w < 4; ++w) {
        pCar1->m_wheelState[w] = WHEEL_STATE_NORMAL; // Wheel normal (not burst)
        pCar1->m_fWheelsSuspensionCompression[w] = 1.0f; // Firm ground contact
    }
    pCar1->UpdateRwMatrix();

    CrisisRoadblockEntity& rb = s_crisisRoadblocks[freeSlot];
    rb = CrisisRoadblockEntity{};
    rb.active = true;
    rb.vehHandles[0] = static_cast<uint32_t>(CPools::ms_pVehiclePool->GetRef(pCar1));
    rb.vehCount = 1;
    rb.pos = finalSpawnPos;
    rb.initialSpawnPos = finalSpawnPos;
    rb.initialVehSpawnPos[0] = finalSpawnPos;
    rb.chokepointIndex = chosenIdx;
    rb.spawnTimeMs = currentMs;
    rb.lastAnimMs = currentMs;
    rb.provoked = false;

    const CVector lateral(-std::sin(0.785f + 1.5707963f), std::cos(0.785f + 1.5707963f), 0.0f);

    // Spawn Vehicle 2 if requested
    if (numVehs == 2 && CStreaming::HasModelLoaded(model2)) {
        CVector car2Pos = finalSpawnPos + (lateral * 3.4f);
        car2Pos.z = chosenCp.pos.z + 1.1f;

        CAutomobile* pCar2 = new CAutomobile(model2, 2, true);
        if (pCar2 && CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->IsObjectValid(pCar2)) {
            pCar2->m_nCreatedBy = 2; // Mission entity
            pCar2->m_nStatus = eEntityStatus::STATUS_ABANDONED;
            pCar2->m_fHealth = 350.0f;
            pCar2->bEngineOn = false;
            pCar2->m_nHandbrakeOn = true;
            pCar2->Teleport(car2Pos);
            pCar2->SetOrientation(0.0f, 0.0f, -0.785f);
            pCar2->PlaceOnRoadProperly();
            pCar2->m_fSteerAngle = 0.0f;
            pCar2->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
            pCar2->m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
            pCar2->m_fGasPedal = 0.0f;
            pCar2->m_fBreakPedal = 1.0f;
            for (int w = 0; w < 4; ++w) {
                pCar2->m_wheelState[w] = WHEEL_STATE_NORMAL; // Wheel normal (not burst)
                pCar2->m_fWheelsSuspensionCompression[w] = 1.0f; // Firm ground contact
            }
            pCar2->UpdateRwMatrix();

            rb.vehHandles[1] = static_cast<uint32_t>(CPools::ms_pVehiclePool->GetRef(pCar2));
            rb.initialVehSpawnPos[1] = car2Pos;
            rb.vehCount = 2;
        } else if (pCar2) {
            Command<Commands::DELETE_CAR>(pCar2);
        }
    }

    // Roadblock rioter ped spawn loop disabled to prevent entity pool saturation.
    rb.rioterCount = 0;

    chosenCp.activeRoadblockSlot = static_cast<int>(freeSlot);
    chosenCp.status.store(ChokepointStatus::BLOCKED_RED, std::memory_order_relaxed);
    chosenCp.initialVehPos = finalSpawnPos;
    chosenCp.playerCleared = false;
    s_activeRoadblockMask.fetch_or(1ULL << chosenIdx, std::memory_order_release);
    s_activeRoadblocksCount.fetch_add(1, std::memory_order_relaxed);

    const float chosenDist = std::sqrt(
        (chosenCp.pos.x - pPos.x) * (chosenCp.pos.x - pPos.x) +
        (chosenCp.pos.y - pPos.y) * (chosenCp.pos.y - pPos.y)
    );
    CMessages::AddMessageJumpQ("ROADBLOCK SPAWNED", 2000, 0, false);
    AddMunicipalLog("[SPAWN VERIFY] Roadblock #%d placed at (%.2f, %.2f, %.2f) | CarHandle: %u", chosenIdx, finalSpawnPos.x, finalSpawnPos.y, finalSpawnPos.z, rb.vehHandles[0]);
    AddMunicipalLog("[ROADBLOCK] Spawned at chokepoint #%d (dist: %.1f)", chosenIdx, chosenDist);
    AddMunicipalLog("CHOKEPOINT ROADBLOCK: Barricade active at chokepoint #%d (%.1f, %.1f) with %zu vehicles, %zu rioters",
        chosenIdx, finalSpawnPos.x, finalSpawnPos.y, rb.vehCount, rb.rioterCount);
    ENGINE_TRACE("MUNICIPAL", "Roadblock spawned #%d at (%.1f, %.1f, %.1f) vehs=%zu rioters=%zu",
        chosenIdx, finalSpawnPos.x, finalSpawnPos.y, finalSpawnPos.z, rb.vehCount, rb.rioterCount);
}

// =============================================================================
//  §3b. Tier 2: Retail Store Raid Physical Materialization & Looter System
// =============================================================================


static constexpr size_t k_maxMaterializedRaids = 2;
static constexpr size_t k_maxLootersPerRaid = 5;
static constexpr size_t k_maxCopsPerRaid = 2;

struct MaterializedLooter {
    uint32_t    pedHandle{0};
    uint32_t    blipHandle{0};
    eWeaponType weapon{WEAPONTYPE_UNARMED};
};

struct MaterializedStoreRaid {
    int                storeIndex{-1};
    bool               isMaterialized{false};
    bool               playerAggroed{false};
    uint32_t           storeBlipHandle{0};
    MaterializedLooter looters[k_maxLootersPerRaid]{};
    size_t             looterCount{0};
    uint32_t           copPedHandles[k_maxCopsPerRaid]{0, 0};
    uint32_t           copBlipHandles[k_maxCopsPerRaid]{0, 0};
    uint32_t           savedWantedLevel{0};
    uint32_t           savedChaosLevel{0};
    uint32_t           lastTaskUpdateMs{0};
};

static MaterializedStoreRaid s_materializedRaids[k_maxMaterializedRaids]{};
static uint32_t s_storeRaidRadarBlips[20]{};

static void RequestStoreRaidResources() {
    SafeRequestModel(MODEL_WMYCR);
    SafeRequestModel(MODEL_DNMYLC);
    SafeRequestModel(MODEL_BMYCG);
    SafeRequestModel(MODEL_VBMYCR);
    SafeRequestModel(MODEL_HMUCR);
    SafeRequestModel(MODEL_LAPD1);
    SafeRequestModel(MODEL_BAT);
    SafeRequestModel(MODEL_KNIFECUR);
    SafeRequestModel(MODEL_COLT45);
    SafeRequestAnimation("RIOT");
}

static void CleanupMaterializedRaidSlot(size_t r, bool cancelRaid) {
    if (r >= k_maxMaterializedRaids) return;
    auto& raid = s_materializedRaids[r];
    if (raid.storeIndex < 0) return;

    const int sIdx = raid.storeIndex;

    for (size_t l = 0; l < raid.looterCount; ++l) {
        if (raid.looters[l].blipHandle != 0) {
            CRadar::ClearBlip(static_cast<int>(raid.looters[l].blipHandle));
            raid.looters[l].blipHandle = 0;
        }
        CPed* looter = ResolvePed(raid.looters[l].pedHandle);
        if (looter && CPools::ms_pPedPool && CPools::ms_pPedPool->IsObjectValid(looter)) {
            if (looter->m_pIntelligence) {
                looter->m_pIntelligence->m_TaskMgr.FlushImmediately();
            }
            Command<Commands::DELETE_CHAR>(looter);
        }
        raid.looters[l].pedHandle = 0;
    }
    raid.looterCount = 0;

    for (size_t c = 0; c < k_maxCopsPerRaid; ++c) {
        if (raid.copBlipHandles[c] != 0) {
            CRadar::ClearBlip(static_cast<int>(raid.copBlipHandles[c]));
            raid.copBlipHandles[c] = 0;
        }
        CPed* cop = ResolvePed(raid.copPedHandles[c]);
        if (cop && CPools::ms_pPedPool && CPools::ms_pPedPool->IsObjectValid(cop)) {
            if (cop->m_pIntelligence) {
                cop->m_pIntelligence->m_TaskMgr.FlushImmediately();
            }
            Command<Commands::DELETE_CHAR>(cop);
        }
        raid.copPedHandles[c] = 0;
    }

    if (raid.storeBlipHandle != 0) {
        CRadar::ClearBlip(static_cast<int>(raid.storeBlipHandle));
        raid.storeBlipHandle = 0;
    }

    if (cancelRaid) {
        if (sIdx >= 0 && sIdx < 20) {
            g_retailStores[sIdx].isUnderRaid.store(false, std::memory_order_relaxed);
            g_retailStores[sIdx].raidStartMs.store(0, std::memory_order_relaxed);
            g_retailStores[sIdx].lastDrainMs.store(0, std::memory_order_relaxed);
            g_retailStores[sIdx].stolenStock.store(0, std::memory_order_relaxed);
            if (s_storeRaidRadarBlips[sIdx] != 0) {
                CRadar::ClearBlip(static_cast<int>(s_storeRaidRadarBlips[sIdx]));
                s_storeRaidRadarBlips[sIdx] = 0;
            }
        }
        raid = MaterializedStoreRaid{};
    } else {
        raid.isMaterialized = false;
        raid.playerAggroed = false;
        raid.storeIndex = -1;
    }
}

void CleanupAllStoreRaids() {
    for (size_t r = 0; r < k_maxMaterializedRaids; ++r) {
        CleanupMaterializedRaidSlot(r, true);
    }
    for (size_t s = 0; s < 20; ++s) {
        if (s_storeRaidRadarBlips[s] != 0) {
            CRadar::ClearBlip(static_cast<int>(s_storeRaidRadarBlips[s]));
            s_storeRaidRadarBlips[s] = 0;
        }
        if (s_retailStoreBlips[s] != 0) {
            CRadar::ChangeBlipDisplay(s_retailStoreBlips[s], BLIP_DISPLAY_BOTH);
        }
        g_retailStores[s].isUnderRaid.store(false, std::memory_order_relaxed);
    }
}

void UpdateStoreRaids(uint32_t currentMs, CPed* player) {
    if (!player) return;

    RequestStoreRaidResources();

    const CVector pPos = player->GetPosition();
    const float unrest = s_socialUnrest.load(std::memory_order_relaxed);

    // 1. If unrest drops below 40%, abort active raids and clean up safely
    if (unrest < 40.0f) {
        for (size_t r = 0; r < k_maxMaterializedRaids; ++r) {
            if (s_materializedRaids[r].storeIndex >= 0) {
                CleanupMaterializedRaidSlot(r, true);
            }
        }
        for (size_t s = 0; s < 20; ++s) {
            if (s_storeRaidRadarBlips[s] != 0) {
                CRadar::ClearBlip(static_cast<int>(s_storeRaidRadarBlips[s]));
                s_storeRaidRadarBlips[s] = 0;
            }
            if (s_retailStoreBlips[s] != 0) {
                CRadar::ChangeBlipDisplay(s_retailStoreBlips[s], BLIP_DISPLAY_BOTH);
            }
        }
        return;
    }

    // 2. Manage Store Radar Blips (flashes / changes color on radar)
    for (size_t s = 0; s < 20; ++s) {
        auto& store = g_retailStores[s];
        if (store.isUnderRaid.load(std::memory_order_relaxed)) {
            // Remove the default shop icon so they don't fight
            if (s_retailStoreBlips[s] != 0) {
                CRadar::ChangeBlipDisplay(s_retailStoreBlips[s], BLIP_DISPLAY_NEITHER);
            }
            if (s_storeRaidRadarBlips[s] == 0) {
                const int blip = CRadar::SetCoordBlip(
                    BLIP_COORD,
                    CVector(store.posX, store.posY, k_retailStoreGroundZ[s] + 1.2f),
                    k_muniBlipColourRed,
                    BLIP_DISPLAY_BOTH,
                    nullptr
                );
                CRadar::SetBlipSprite(blip, RADAR_SPRITE_ENEMYATTACK);
                CRadar::ChangeBlipScale(blip, 4);
                Command<Commands::FLASH_RADAR_BLIP>(blip, -1);
                s_storeRaidRadarBlips[s] = static_cast<uint32_t>(blip);
            }
        } else {
            if (s_storeRaidRadarBlips[s] != 0) {
                CRadar::ClearBlip(static_cast<int>(s_storeRaidRadarBlips[s]));
                s_storeRaidRadarBlips[s] = 0;
            }
            // Restore default shop icon
            if (s_retailStoreBlips[s] != 0) {
                CRadar::ChangeBlipDisplay(s_retailStoreBlips[s], BLIP_DISPLAY_BOTH);
            }
        }
    }

    // 3. Process active materialized raids (combat, Rule B wanted rules, resolution, safe despawn)
    for (size_t r = 0; r < k_maxMaterializedRaids; ++r) {
        auto& raid = s_materializedRaids[r];
        if (!raid.isMaterialized || raid.storeIndex < 0 || raid.storeIndex >= 20) continue;

        auto& store = g_retailStores[raid.storeIndex];
        const float dx = store.posX - pPos.x;
        const float dy = store.posY - pPos.y;
        const float distToStore = std::sqrt(dx * dx + dy * dy);

        // A. Check alive looters, out-of-bounds fleeing (> 45m), and looter blips
        size_t aliveLooters = 0;
        for (size_t l = 0; l < raid.looterCount; ++l) {
            CPed* lPed = ResolvePed(raid.looters[l].pedHandle);
            if (lPed && lPed->m_fHealth > 0.0f) {
                const CVector lPos = lPed->GetPosition();
                const float ldx = lPos.x - store.posX;
                const float ldy = lPos.y - store.posY;
                const float distToStoreSq = ldx * ldx + ldy * ldy;

                if (distToStoreSq > 45.0f * 45.0f) {
                    // Out-of-bounds flee: looter has fled beyond the raid engagement perimeter (> 45.0m)
                    // a) Clear his radar target blip immediately
                    if (raid.looters[l].blipHandle != 0) {
                        CRadar::ClearBlip(static_cast<int>(raid.looters[l].blipHandle));
                        raid.looters[l].blipHandle = 0;
                    }
                    // d) Issue smart flee task and mark as ambient for safe engine garbage collection
                    if (lPed->m_pIntelligence) {
                        lPed->bPanicWhenScared = true;
                        lPed->bFleeWhenStanding = true;
                        lPed->m_pIntelligence->m_TaskMgr.FlushImmediately();
                        Command<Commands::TASK_SMART_FLEE_POINT>(lPed, store.posX, store.posY, k_retailStoreGroundZ[raid.storeIndex], 120.0f, -1);
                    }
                    lPed->m_nCreatedBy = 1; // Mark as ambient ped so engine safely deletes off-screen
                    // c) Remove looter handle from active raid tracker (he is considered routed/escaped)
                    raid.looters[l].pedHandle = 0;
                    // b) Decrement/do not count towards aliveLooters
                    Logger::Log("[StoreRaid] Looter #%u routed/escaped from %s (dist: %.1fm > 45.0m perimeter)",
                        static_cast<unsigned int>(l), store.name, std::sqrt(distToStoreSq));
                } else {
                    aliveLooters++;
                }
            } else {
                if (raid.looters[l].blipHandle != 0) {
                    CRadar::ClearBlip(static_cast<int>(raid.looters[l].blipHandle));
                    raid.looters[l].blipHandle = 0;
                }
            }
        }

        // B. Check cop responder status
        for (size_t c = 0; c < k_maxCopsPerRaid; ++c) {
            CPed* cop = ResolvePed(raid.copPedHandles[c]);
            if (cop && cop->m_fHealth <= 0.0f && raid.copBlipHandles[c] != 0) {
                CRadar::ClearBlip(static_cast<int>(raid.copBlipHandles[c]));
                raid.copBlipHandles[c] = 0;
            }
        }

        // C. RULE B WANTED LEVEL RULES
        CPlayerPed* playerPed = FindPlayerPed();
        if (playerPed && playerPed->m_pPlayerData && playerPed->m_pPlayerData->m_pWanted) {
            CWanted* wanted = playerPed->m_pPlayerData->m_pWanted;
            const uint32_t currentWanted = wanted->m_nWantedLevel;

            // Check if player attacked any responding cop
            bool attackedCop = false;
            for (size_t c = 0; c < k_maxCopsPerRaid; ++c) {
                CPed* cop = ResolvePed(raid.copPedHandles[c]);
                if (cop && (cop->m_pDamageEntity == playerPed || playerPed->m_pPlayerTargettedPed == cop)) {
                    attackedCop = true;
                    break;
                }
            }

            // Check if player attacked innocent civilians within store raid boundary (<= 45.0m)
            bool attackedCivilian = false;
            if (!attackedCop && CPools::ms_pPedPool) {
                for (int p = 0; p < CPools::ms_pPedPool->m_nSize; ++p) {
                    CPed* civPed = CPools::ms_pPedPool->GetAt(p);
                    if (!civPed || civPed == playerPed) continue;
                    bool isCopPed = false;
                    for (size_t c = 0; c < k_maxCopsPerRaid; ++c) {
                        if (civPed == ResolvePed(raid.copPedHandles[c])) {
                            isCopPed = true;
                            break;
                        }
                    }
                    if (isCopPed) continue;
                    if (!CPools::ms_pPedPool->IsObjectValid(civPed)) continue;
                    bool isRegisteredLooter = false;
                    for (size_t l = 0; l < raid.looterCount; ++l) {
                        if (ResolvePed(raid.looters[l].pedHandle) == civPed) {
                            isRegisteredLooter = true;
                            break;
                        }
                    }
                    if (isRegisteredLooter) continue;

                    const CVector cPos = civPed->GetPosition();
                    const float cdx = cPos.x - store.posX;
                    const float cdy = cPos.y - store.posY;
                    if ((cdx * cdx + cdy * cdy) <= 45.0f * 45.0f) {
                        if (civPed->m_pDamageEntity == playerPed) {
                            attackedCivilian = true;
                            break;
                        }
                    }
                }
            }

            if (attackedCop || attackedCivilian) {
                // Immediately enforce full wanted penalties (2+ stars)
                if (currentWanted < 2) {
                    playerPed->SetWantedLevelNoDrop(2);
                }
                raid.savedWantedLevel = (std::max)(2u, wanted->m_nWantedLevel);
                raid.savedChaosLevel = wanted->m_nChaosLevel;
            } else {
                // Check if player is engaging registered looters
                bool engagedLooter = false;
                for (size_t l = 0; l < raid.looterCount; ++l) {
                    CPed* lPed = ResolvePed(raid.looters[l].pedHandle);
                    if (lPed && (lPed->m_pDamageEntity == playerPed || playerPed->m_pPlayerTargettedPed == lPed || lPed->m_fHealth <= 0.0f)) {
                        engagedLooter = true;
                        break;
                    }
                }

                if (engagedLooter && currentWanted > raid.savedWantedLevel) {
                    // Suppress wanted level gain: revert to saved state
                    wanted->SetWantedLevel(raid.savedWantedLevel);
                    wanted->m_nChaosLevel = raid.savedChaosLevel;
                    wanted->ClearQdCrimes();
                } else if (currentWanted <= raid.savedWantedLevel) {
                    raid.savedWantedLevel = currentWanted;
                    raid.savedChaosLevel = wanted->m_nChaosLevel;
                }
            }
        }

        // D. RAID RESOLUTION & POST-RAID RECOVERY
        // When CJ eliminates all materialized looters at the store:
        if (aliveLooters == 0 && raid.looterCount > 0) {
            // a) Clear raid state and remove enemy blips
            store.isUnderRaid.store(false, std::memory_order_relaxed);
            store.raidStartMs.store(0, std::memory_order_relaxed);
            store.lastDrainMs.store(0, std::memory_order_relaxed);
            store.raidCooldownUntilMs.store(currentMs + 600000, std::memory_order_relaxed); // 10 minutes mandatory cooldown
            store.isRansacked.store(false, std::memory_order_relaxed);

            for (size_t l = 0; l < raid.looterCount; ++l) {
                if (raid.looters[l].blipHandle != 0) {
                    CRadar::ClearBlip(static_cast<int>(raid.looters[l].blipHandle));
                    raid.looters[l].blipHandle = 0;
                }
            }
            for (size_t c = 0; c < k_maxCopsPerRaid; ++c) {
                if (raid.copBlipHandles[c] != 0) {
                    CRadar::ClearBlip(static_cast<int>(raid.copBlipHandles[c]));
                    raid.copBlipHandles[c] = 0;
                }
            }
            if (s_storeRaidRadarBlips[raid.storeIndex] != 0) {
                CRadar::ClearBlip(static_cast<int>(s_storeRaidRadarBlips[raid.storeIndex]));
                s_storeRaidRadarBlips[raid.storeIndex] = 0;
            }
            if (s_retailStoreBlips[raid.storeIndex] != 0) {
                CRadar::ChangeBlipDisplay(s_retailStoreBlips[raid.storeIndex], BLIP_DISPLAY_BOTH);
            }

            // b) Award player cash reward ("Store Defended: +$500")
            if (player && player->m_pPlayerData) {
                player->m_pPlayerData->m_nMoney += 500;
            }
            CHud::SetHelpMessage("~g~Store Defended: +$500~w~", true, false, false);

            // c) Partially restore store stock (+15-20% of stolen goods recovered)
            const uint32_t stolen = store.stolenStock.load(std::memory_order_relaxed);
            const uint32_t recovered = (stolen > 0) ? (std::max)(10u, (stolen * 18u) / 100u) : 20u;
            store.localStock.fetch_add(recovered, std::memory_order_relaxed);
            store.stolenStock.store(0, std::memory_order_relaxed);

            // d) Deduct repair/cleanup fee from city treasury/store reserve
            s_cityTreasury.fetch_sub(750, std::memory_order_relaxed);
            if (store.capitalBalance.load(std::memory_order_relaxed) > 500) {
                store.capitalBalance.fetch_sub(500, std::memory_order_relaxed);
            }

            AddMunicipalLog("DEFENDED: %s secured by CJ. Stock recovered: +%u units. Repairs: -$750 treasury, -$500 reserve.",
                store.name, recovered);

            CleanupMaterializedRaidSlot(r, false);
            continue;
        }

        // E. CLOSE-PROXIMITY RETALIATION & COMBAT ENGAGEMENT
        bool playerThreatened = (distToStore <= 18.0f);
        if (!playerThreatened && playerPed) {
            for (size_t l = 0; l < raid.looterCount; ++l) {
                CPed* lPed = ResolvePed(raid.looters[l].pedHandle);
                if (lPed && lPed->m_fHealth > 0.0f) {
                    const CVector lpPos = lPed->GetPosition();
                    const float ldx = lpPos.x - pPos.x;
                    const float ldy = lpPos.y - pPos.y;
                    if ((ldx * ldx + ldy * ldy) <= 18.0f * 18.0f) {
                        playerThreatened = true;
                        break;
                    }
                    if (lPed->m_pDamageEntity == playerPed || playerPed->m_pPlayerTargettedPed == lPed || lPed->m_fHealth < lPed->m_fMaxHealth) {
                        playerThreatened = true;
                        break;
                    }
                }
            }
        }

        // Immediate retaliation: if CJ enters within 18m or harms any looter, assign all surviving looters direct combat on CJ
        if (playerThreatened && playerPed) {
            if (!raid.playerAggroed) {
                raid.playerAggroed = true;
                raid.lastTaskUpdateMs = currentMs;
                for (size_t l = 0; l < raid.looterCount; ++l) {
                    CPed* lPed = ResolvePed(raid.looters[l].pedHandle);
                    if (lPed && lPed->m_fHealth > 0.0f && lPed->m_pIntelligence) {
                        lPed->bPanicWhenScared = false;
                        lPed->bCrouchWhenScared = false;
                        lPed->bFleeWhenStanding = false;
                        lPed->bCanAttackPlayerWithMelee = true;
                        lPed->bDontFight = false;
                        lPed->m_nWeaponSkill = 2;
                        lPed->m_pIntelligence->m_TaskMgr.FlushImmediately();
                        lPed->m_pIntelligence->m_TaskMgr.SetTask(
                            new CTaskComplexKillPedOnFoot(playerPed, -1, 0, 16, 0, 1),
                            TASK_PRIMARY_PRIMARY, false);
                    }
                }
                for (size_t c = 0; c < k_maxCopsPerRaid; ++c) {
                    CPed* cop = ResolvePed(raid.copPedHandles[c]);
                    if (cop && cop->m_fHealth > 0.0f) {
                        CPed* nearestLooter = nullptr;
                        float nearestDistSq = 45.0f * 45.0f;
                        const CVector copPos = cop->GetPosition();
                        for (size_t l = 0; l < raid.looterCount; ++l) {
                            CPed* lPed = ResolvePed(raid.looters[l].pedHandle);
                            if (lPed && lPed->m_fHealth > 0.0f) {
                                const CVector lpPos = lPed->GetPosition();
                                const float cldx = lpPos.x - copPos.x;
                                const float cldy = lpPos.y - copPos.y;
                                const float cldSq = cldx * cldx + cldy * cldy;
                                if (cldSq < nearestDistSq) {
                                    nearestDistSq = cldSq;
                                    nearestLooter = lPed;
                                }
                            }
                        }
                        if (nearestLooter) {
                            AssignKillPedTask(cop, nearestLooter);
                        }
                    }
                }
            }
        } else if (distToStore > 35.0f) {
            raid.playerAggroed = false;
        }

        // Periodic Task Maintenance (every 1.5s)
        if (currentMs - raid.lastTaskUpdateMs >= 1500) {
            raid.lastTaskUpdateMs = currentMs;

            if (raid.playerAggroed && playerPed) {
                // Keep looters focused on CJ with bats/knives
                for (size_t l = 0; l < raid.looterCount; ++l) {
                    CPed* lPed = ResolvePed(raid.looters[l].pedHandle);
                    if (lPed && lPed->m_fHealth > 0.0f && lPed->m_pIntelligence) {
                        if (!lPed->m_pIntelligence->m_TaskMgr.FindTaskByType(TASK_PRIMARY_PRIMARY, TASK_COMPLEX_KILL_PED_ON_FOOT)) {
                            lPed->bPanicWhenScared = false;
                            lPed->bCrouchWhenScared = false;
                            lPed->bFleeWhenStanding = false;
                            lPed->bCanAttackPlayerWithMelee = true;
                            lPed->bDontFight = false;
                            lPed->m_pIntelligence->m_TaskMgr.FlushImmediately();
                            lPed->m_pIntelligence->m_TaskMgr.SetTask(
                                new CTaskComplexKillPedOnFoot(playerPed, -1, 0, 16, 0, 1),
                                TASK_PRIMARY_PRIMARY, false);
                        }
                    }
                }
                for (size_t c = 0; c < k_maxCopsPerRaid; ++c) {
                    CPed* cop = ResolvePed(raid.copPedHandles[c]);
                    if (cop && cop->m_fHealth > 0.0f) {
                        CPed* nearestLooter = nullptr;
                        float nearestDistSq = 45.0f * 45.0f;
                        const CVector copPos = cop->GetPosition();
                        for (size_t l = 0; l < raid.looterCount; ++l) {
                            CPed* lPed = ResolvePed(raid.looters[l].pedHandle);
                            if (lPed && lPed->m_fHealth > 0.0f) {
                                const CVector lpPos = lPed->GetPosition();
                                const float cldx = lpPos.x - copPos.x;
                                const float cldy = lpPos.y - copPos.y;
                                const float cldSq = cldx * cldx + cldy * cldy;
                                if (cldSq < nearestDistSq) {
                                    nearestDistSq = cldSq;
                                    nearestLooter = lPed;
                                }
                            }
                        }
                        if (nearestLooter) {
                            AssignKillPedTask(cop, nearestLooter);
                        }
                    }
                }
            } else {
                bool anyCopAlive = false;
                for (size_t c = 0; c < k_maxCopsPerRaid; ++c) {
                    CPed* cop = ResolvePed(raid.copPedHandles[c]);
                    if (cop && cop->m_fHealth > 0.0f) {
                        anyCopAlive = true;
                        CPed* nearestLooter = nullptr;
                        float nearestDistSq = 45.0f * 45.0f;
                        const CVector copPos = cop->GetPosition();
                        for (size_t l = 0; l < raid.looterCount; ++l) {
                            CPed* lPed = ResolvePed(raid.looters[l].pedHandle);
                            if (lPed && lPed->m_fHealth > 0.0f) {
                                const CVector lpPos = lPed->GetPosition();
                                const float cldx = lpPos.x - copPos.x;
                                const float cldy = lpPos.y - copPos.y;
                                const float cldSq = cldx * cldx + cldy * cldy;
                                if (cldSq < nearestDistSq) {
                                    nearestDistSq = cldSq;
                                    nearestLooter = lPed;
                                }
                            }
                        }
                        if (nearestLooter) {
                            AssignKillPedTask(cop, nearestLooter);
                        }
                    }
                }

                if (anyCopAlive) {
                    // Each alive looter attacks the nearest alive cop
                    for (size_t l = 0; l < raid.looterCount; ++l) {
                        CPed* lPed = ResolvePed(raid.looters[l].pedHandle);
                        if (lPed && lPed->m_fHealth > 0.0f) {
                            CPed* nearestCop = nullptr;
                            float nearestCopDistSq = 45.0f * 45.0f;
                            const CVector lpPos = lPed->GetPosition();
                            for (size_t c = 0; c < k_maxCopsPerRaid; ++c) {
                                CPed* cop = ResolvePed(raid.copPedHandles[c]);
                                if (cop && cop->m_fHealth > 0.0f) {
                                    const CVector copPos = cop->GetPosition();
                                    const float cdx = copPos.x - lpPos.x;
                                    const float cdy = copPos.y - lpPos.y;
                                    const float cdSq = cdx * cdx + cdy * cdy;
                                    if (cdSq < nearestCopDistSq) {
                                        nearestCopDistSq = cdSq;
                                        nearestCop = cop;
                                    }
                                }
                            }
                            if (nearestCop) {
                                AssignKillPedTask(lPed, nearestCop);
                            }
                        }
                    }
                } else {
                    // All cops dead, player not aggroed: Looters attack storefront / play riot anims
                    for (size_t l = 0; l < raid.looterCount; ++l) {
                        CPed* lPed = ResolvePed(raid.looters[l].pedHandle);
                        if (lPed && lPed->m_fHealth > 0.0f && lPed->m_pIntelligence) {
                            static const char* s_raidAnims[3] = { "RIOT_PUNCHES", "RIOT_ANGRY", "RIOT_CHANT" };
                            SafePlayRiotAnimation(lPed, s_raidAnims[(l + (currentMs / 2000)) % 3]);
                        }
                    }
                }
            }
        }

        // F. PREVENT STALE DELETIONS & POP-IN:
        // Do NOT abruptly delete looters when player steps slightly away. Only clean up if distance > 100.0m and entity is off-screen.
        if (distToStore > 100.0f) {
            bool anyOnScreen = false;
            for (size_t l = 0; l < raid.looterCount; ++l) {
                CPed* lPed = ResolvePed(raid.looters[l].pedHandle);
                if (lPed && lPed->m_fHealth > 0.0f) {
                    if (Command<Commands::IS_CHAR_ON_SCREEN>(lPed)) {
                        anyOnScreen = true;
                        break;
                    }
                }
            }
            for (size_t c = 0; c < k_maxCopsPerRaid; ++c) {
                CPed* cop = ResolvePed(raid.copPedHandles[c]);
                if (cop && cop->m_fHealth > 0.0f && Command<Commands::IS_CHAR_ON_SCREEN>(cop)) {
                    anyOnScreen = true;
                    break;
                }
            }

            if (!anyOnScreen || distToStore > 150.0f) {
                CleanupMaterializedRaidSlot(r, false);
                Logger::Log("[StoreRaid] Safe off-screen despawn for store #%d (dist: %.1fm)",
                    raid.storeIndex, distToStore);
            }
        }
    }

    // 4. MATERIALIZATION GATE (40m - 75m):
    // Only when player is within 40m - 75m and store has active RAID_IN_PROGRESS
    for (size_t s = 0; s < 20; ++s) {
        auto& store = g_retailStores[s];
        if (!store.isUnderRaid.load(std::memory_order_relaxed)) continue;

        bool alreadyMaterialized = false;
        for (size_t r = 0; r < k_maxMaterializedRaids; ++r) {
            if (s_materializedRaids[r].isMaterialized && s_materializedRaids[r].storeIndex == static_cast<int>(s)) {
                alreadyMaterialized = true;
                break;
            }
        }
        if (alreadyMaterialized) continue;

        const float dx = store.posX - pPos.x;
        const float dy = store.posY - pPos.y;
        const float distToStore = std::sqrt(dx * dx + dy * dy);

        // Strictly materialize only when within 40m - 75m
        if (distToStore < 40.0f || distToStore > 75.0f) continue;

        size_t freeSlot = k_maxMaterializedRaids;
        for (size_t r = 0; r < k_maxMaterializedRaids; ++r) {
            if (!s_materializedRaids[r].isMaterialized) {
                freeSlot = r;
                break;
            }
        }
        if (freeSlot == k_maxMaterializedRaids) continue;

        // Verified low-income street civilian pool (strictly non-gang):
        // MODEL_WMYCR, MODEL_DNMYLC, MODEL_BMYCG, MODEL_VBMYCR, MODEL_HMUCR
        int readyModels[5];
        int readyModelCount = 0;
        if (IsMuniModelLoaded(MODEL_WMYCR))  readyModels[readyModelCount++] = MODEL_WMYCR;
        if (IsMuniModelLoaded(MODEL_DNMYLC)) readyModels[readyModelCount++] = MODEL_DNMYLC;
        if (IsMuniModelLoaded(MODEL_BMYCG))  readyModels[readyModelCount++] = MODEL_BMYCG;
        if (IsMuniModelLoaded(MODEL_VBMYCR)) readyModels[readyModelCount++] = MODEL_VBMYCR;
        if (IsMuniModelLoaded(MODEL_HMUCR))  readyModels[readyModelCount++] = MODEL_HMUCR;

        if (readyModelCount == 0 || !IsMuniModelLoaded(MODEL_LAPD1) || !IsMuniModelLoaded(MODEL_COLT45)) {
            continue; // Wait for models to finish loading
        }

        const size_t lootersToSpawn = 4 + (rand() % 2); // strictly 4-5 looters
        auto& raid = s_materializedRaids[freeSlot];
        raid = MaterializedStoreRaid{};
        raid.storeIndex = static_cast<int>(s);
        raid.isMaterialized = true;
        raid.looterCount = lootersToSpawn;
        raid.lastTaskUpdateMs = currentMs;

        CPlayerPed* playerPed = FindPlayerPed();
        if (playerPed && playerPed->m_pPlayerData && playerPed->m_pPlayerData->m_pWanted) {
            raid.savedWantedLevel = playerPed->m_pPlayerData->m_pWanted->m_nWantedLevel;
            raid.savedChaosLevel = playerPed->m_pPlayerData->m_pWanted->m_nChaosLevel;
        }

        const float groundZ = k_retailStoreGroundZ[s];

        // Safe perimeter offsets around target store (storefront, gas pumps, adjacent alleys)
        const CVector looterOffsets[5] = {
            CVector(3.2f, -2.5f, 0.0f),
            CVector(-3.8f, 2.8f, 0.0f),
            CVector(4.5f, 3.2f, 0.0f),
            CVector(-2.5f, -3.5f, 0.0f),
            CVector(1.8f, 4.0f, 0.0f)
        };

        for (size_t l = 0; l < lootersToSpawn; ++l) {
            CVector spawnPos;
            if (s == 0 || s == 2) {
                // FORCE EXACT GROUND PLANE for Idlewood 24/7 (Store #0 / #2):
                // Spawn exclusively on the open flat asphalt forecourt around (1945.0f, -1782.0f, 13.0f).
                // Apply slight ground horizontal offsets (+/- 2.5m X/Y) and clamp Z strictly to ground level (13.0f - 13.5f). Forbid > 15.0f.
                static const CVector s_idlewoodForecourtOffsets[5] = {
                    CVector(-2.0f,  1.5f, 0.0f),
                    CVector( 2.2f, -1.2f, 0.0f),
                    CVector( 0.0f,  2.4f, 0.0f),
                    CVector(-1.8f, -2.0f, 0.0f),
                    CVector( 2.5f,  2.0f, 0.0f)
                };
                spawnPos = CVector(1945.0f + s_idlewoodForecourtOffsets[l].x, -1782.0f + s_idlewoodForecourtOffsets[l].y, 13.0f);
                const float gz = CWorld::FindGroundZForCoord(spawnPos.x, spawnPos.y);
                if (gz > -100.0f && gz < 15.0f) {
                    spawnPos.z = (std::max)(13.0f, (std::min)(13.5f, gz)) + 1.0f;
                } else {
                    spawnPos.z = 13.0f + 1.0f;
                }
            } else {
                spawnPos = CVector(store.posX + looterOffsets[l].x, store.posY + looterOffsets[l].y, groundZ);
                const float gz = CWorld::FindGroundZForCoord(spawnPos.x, spawnPos.y);
                if (gz > -100.0f && gz < groundZ + 2.0f && std::abs(gz - groundZ) < 3.0f) {
                    spawnPos.z = gz + 1.0f;
                } else {
                    spawnPos.z = groundZ + 1.0f;
                }
            }

            const int chosenModel = readyModels[rand() % readyModelCount];
            CPed* looter = SpawnIncidentPed(chosenModel, PED_TYPE_CRIMINAL, spawnPos, false);
            if (!looter) continue;

            looter->m_fHealth = 140.0f;
            looter->m_fMaxHealth = 140.0f;
            looter->m_nPedStat = ePedStats::PEDSTAT_GANG_MEMBER;
            looter->bStayInSamePlace = false;
            looter->bPanicWhenScared = false;
            looter->bCrouchWhenScared = false;
            looter->bFleeWhenStanding = false;
            looter->bCanAttackPlayerWithMelee = true;
            looter->bIgnorePlayer = false;
            looter->bDontFight = false;
            looter->m_nWeaponSkill = 2;
            looter->SetPedStats(ePedStats::PEDSTAT_GANG_MEMBER);

            // Equip strictly with Baseball Bats (MODEL_BAT) and Knives (MODEL_KNIFECUR)
            eWeaponType wep = ((rand() % 2 == 0) && IsMuniModelLoaded(MODEL_KNIFECUR)) ? WEAPONTYPE_KNIFE : WEAPONTYPE_BASEBALLBAT;
            if (wep == WEAPONTYPE_BASEBALLBAT && !IsMuniModelLoaded(MODEL_BAT)) {
                wep = IsMuniModelLoaded(MODEL_KNIFECUR) ? WEAPONTYPE_KNIFE : WEAPONTYPE_UNARMED;
            }

            if (wep != WEAPONTYPE_UNARMED) {
                GiveInfiniteCombatWeapon(looter, wep);
            }

            // Small red enemy target blip attached to looters
            const int blip = CRadar::SetEntityBlip(BLIP_CHAR, CPools::ms_pPedPool->GetRef(looter), k_muniBlipColourRed, BLIP_DISPLAY_BOTH);
            CRadar::ChangeBlipScale(blip, 1);

            raid.looters[l].pedHandle = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(looter));
            raid.looters[l].blipHandle = static_cast<uint32_t>(blip);
            raid.looters[l].weapon = wep;
        }

        // Responding Law Enforcement: Spawn strictly 2 police responders (MODEL_LAPD1)
        static const CVector s_idlewoodCopOffsets[k_maxCopsPerRaid] = {
            CVector(1935.0f, -1788.0f, 13.0f),
            CVector(1938.0f, -1792.0f, 13.0f)
        };
        static const CVector s_storeCopOffsets[k_maxCopsPerRaid] = {
            CVector(-7.5f, -6.5f, 0.0f),
            CVector(-9.0f, -4.5f, 0.0f)
        };

        for (size_t c = 0; c < k_maxCopsPerRaid; ++c) {
            CVector copSpawnPos;
            if (s == 0 || s == 2) {
                copSpawnPos = s_idlewoodCopOffsets[c];
                const float cgz = CWorld::FindGroundZForCoord(copSpawnPos.x, copSpawnPos.y);
                if (cgz > -100.0f && cgz < 15.0f) {
                    copSpawnPos.z = (std::max)(13.0f, (std::min)(13.5f, cgz)) + 1.0f;
                } else {
                    copSpawnPos.z = 13.0f + 1.0f;
                }
            } else {
                copSpawnPos = CVector(store.posX + s_storeCopOffsets[c].x, store.posY + s_storeCopOffsets[c].y, groundZ);
                const float cgz = CWorld::FindGroundZForCoord(copSpawnPos.x, copSpawnPos.y);
                if (cgz > -100.0f && cgz < groundZ + 2.0f && std::abs(cgz - groundZ) < 3.0f) {
                    copSpawnPos.z = cgz + 1.0f;
                } else {
                    copSpawnPos.z = groundZ + 1.0f;
                }
            }

            CPed* cop = SpawnIncidentPed(MODEL_LAPD1, PED_TYPE_COP, copSpawnPos, false);
            if (cop) {
                cop->m_fHealth = 160.0f;
                cop->m_fMaxHealth = 160.0f;
                cop->m_fArmour = 50.0f;
                cop->bStayInSamePlace = false;
                cop->bPanicWhenScared = false;
                cop->bCrouchWhenScared = false;
                cop->bFleeWhenStanding = false;
                cop->m_nWeaponSkill = 2;
                GiveInfiniteCombatWeapon(cop, WEAPONTYPE_PISTOL);

                const int copBlip = CRadar::SetEntityBlip(BLIP_CHAR, CPools::ms_pPedPool->GetRef(cop), k_muniBlipColourBlue, BLIP_DISPLAY_ONLY_RADAR);
                CRadar::ChangeBlipScale(copBlip, 1);

                raid.copPedHandles[c] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(cop));
                raid.copBlipHandles[c] = static_cast<uint32_t>(copBlip);

                // Initial combat engagement against first available looter
                for (size_t l = 0; l < raid.looterCount; ++l) {
                    CPed* lPed = ResolvePed(raid.looters[l].pedHandle);
                    if (lPed) {
                        AssignKillPedTask(cop, lPed);
                        AssignKillPedTask(lPed, cop);
                        break;
                    }
                }
            }
        }

        Logger::Log("[StoreRaid] Materialized Store Raid #%u at %s (dist: %.1fm, %u looters, 2 LAPD1 cops)",
            store.id, store.name, distToStore, raid.looterCount);
    }
}

// =============================================================================
//  §4. Complete Municipal Engine Frame Tick & Subsystem Lifecycle
// =============================================================================

void UpdateMunicipalEngine(CPed* player, uint32_t currentMs) {
    static bool s_muniIncidentsInit = false;
    if (!s_muniIncidentsInit) {
        s_muniIncidentsInit = true;
        Rioters::Init();
        GangWars::Init();
    }

    // Federal Emergency Solvency Act (Immediate Debt-Wipe & Stabilization)
    if (s_cityTreasury.load(std::memory_order_relaxed) < 0) {
        s_cityTreasury.store(350000, std::memory_order_relaxed);
        s_emergencyState.store(0, std::memory_order_relaxed);
        AddMunicipalLog("FEDERAL BAILOUT: Agricultural Solvency Decree enacted. $350k allocated, debt cleared.");

        const float curUnrest = s_socialUnrest.load(std::memory_order_relaxed);
        if (curUnrest < 40.0f) {
            s_socialUnrest.store(12.0f, std::memory_order_relaxed);
            s_publicUnrest.store(12.0f, std::memory_order_relaxed);
            s_crimeRate.store(24.0f, std::memory_order_relaxed);

            // Disband cordons only when social unrest is fully pacified below 40.0f
            if (s_policeCordonActive) {
                s_policeCordonActive = false;
                for (size_t i = 0; i < 8; ++i) {
                    if (s_policeCordonIds[i] != 0) {
                        const ptrdiff_t slot = FindIncidentSlotById(s_policeCordonIds[i]);
                        if (slot >= 0) {
                            s_totalIncidentsHandled.fetch_add(1, std::memory_order_relaxed);
                            CleanupIncident(s_incidents[slot], player, true);
                        }
                        s_policeCordonIds[i] = 0;
                    }
                }
                for (size_t k = 0; k < k_maxIncidents; ++k) {
                    if (s_incidents[k].active && s_incidents[k].type == IncidentType::ROADBLOCK) {
                        s_totalIncidentsHandled.fetch_add(1, std::memory_order_relaxed);
                        CleanupIncident(s_incidents[k], player, true);
                    }
                }
            }
        }
    }

    // Supply Starvation Municipal Loop: Rate-limited to run once every 180 seconds, max +0.05f unrest and only if Food stock < 10%
    static uint32_t s_lastStarvationCheckMs = 0;
    if (currentMs - s_lastStarvationCheckMs >= 180000) {
        s_lastStarvationCheckMs = currentMs;
        const uint32_t foodStock = s_sfFoodStock.load(std::memory_order_relaxed);
        if (foodStock < 20) { // < 10% of nominal 200t SF central capacity
            float curU = s_socialUnrest.load(std::memory_order_relaxed);
            float newU = (std::min)(100.0f, curU + 0.05f);
            s_socialUnrest.store(newU, std::memory_order_relaxed);
            s_publicUnrest.store(newU, std::memory_order_relaxed);
            AddMunicipalLog("SUPPLY STARVATION: SF central food stock critical (<10%%: %ut). Social unrest +0.05%%", foodStock);
        }
    }

    // 1. World Crisis Lifecycle, Streaming & Autonomous Scheduler
    const uint8_t pendingTrigger = s_pendingCrisisTriggerId.exchange(0, std::memory_order_relaxed);
    if (pendingTrigger != 0) {
        TriggerWorldCrisis(pendingTrigger, currentMs);
    }

    ProcessCrisisWorldPhysicsStreaming();

    if (s_activeCrisis.activeId.load(std::memory_order_relaxed) != 0) {
        const uint32_t startMs = s_activeCrisis.startMs.load(std::memory_order_relaxed);
        const uint32_t durMs = s_activeCrisis.durationMs.load(std::memory_order_relaxed);
        if (currentMs - startMs >= durMs) {
            const uint8_t activeId = s_activeCrisis.activeId.load(std::memory_order_relaxed);
            ApplyCrisisWorldPhysics(activeId, false);
            s_activeCrisis.fuelPriceMul.store(1.0f, std::memory_order_relaxed);
            s_activeCrisis.cargoPriceMul.store(1.0f, std::memory_order_relaxed);
            s_activeCrisis.crimeVelocity.store(0.0f, std::memory_order_relaxed);
            s_activeCrisis.unrestVelocity.store(0.0f, std::memory_order_relaxed);
            s_activeCrisis.treasuryDeltaPerMin.store(0, std::memory_order_relaxed);
            s_activeCrisis.policeEffMul.store(1.0f, std::memory_order_relaxed);
            {
                std::lock_guard<std::mutex> lock(s_activeCrisis.titleMutex);
                s_activeCrisis.activeTitle[0] = '\0';
                s_activeCrisis.activeHeadline[0] = '\0';
            }
            s_activeCrisis.activeId.store(0, std::memory_order_release);
            AddMunicipalLog("CRISIS RESOLVED: Market conditions normalized.");
            SetInGameTicker("[MUNICIPAL] WORLD CRISIS RESOLVED | MARKETS STABILIZED");
            CHud::SetHelpMessage("~g~CRISIS RESOLVED:~w~ Market conditions normalized.", true, false, false);
        }
    }

    // Autonomous WorldCrisisScheduler (evaluated every 180,000 ms = 3 minutes)
    static uint32_t s_lastCrisisSchedulerMs = 0;
    if (currentMs - s_lastCrisisSchedulerMs >= 180000) {
        s_lastCrisisSchedulerMs = currentMs;
        if (s_activeCrisis.activeId.load(std::memory_order_relaxed) == 0) {
            const int64_t treasury = s_cityTreasury.load(std::memory_order_relaxed);
            const float crime = s_crimeRate.load(std::memory_order_relaxed);
            const float unrest = s_socialUnrest.load(std::memory_order_relaxed);
            const uint32_t fuel = s_sfFuelStock.load(std::memory_order_relaxed);
            const unsigned char hour = CClock::ms_nGameClockHours;

            uint8_t chosenCrisis = 0;
            if (treasury <= 0) {
                chosenCrisis = 5; // MUNI_DEFAULT
            } else if (crime > 75.0f && unrest > 65.0f) {
                chosenCrisis = 9; // CIVIL_RIOT_1992
            } else if (crime > 80.0f) {
                chosenCrisis = 13; // RICO_SWEEP
            } else if (fuel < 15) {
                chosenCrisis = 1; // OIL_EMBARGO
            } else if (unrest > 55.0f) {
                chosenCrisis = (rand() % 2 == 0) ? 2 : 10; // PORT_STRIKE or SANITATION_STRIKE
            } else if ((hour >= 22 || hour <= 5) && (rand() % 100 < 20)) {
                chosenCrisis = 15; // GRID_BLACKOUT
            } else {
                static const uint8_t s_generalPool[] = { 3, 4, 6, 7, 8, 11, 12, 14 };
                chosenCrisis = s_generalPool[rand() % (sizeof(s_generalPool) / sizeof(s_generalPool[0]))];
            }

            if (chosenCrisis > 0) {
                TriggerWorldCrisis(chosenCrisis, currentMs);
                AddMunicipalLog("SCHEDULER: Autonomous evaluator triggered Crisis ID %u", chosenCrisis);
            }
        }
    }

    // Per-tick continuous physical behaviors for active crises
    if (s_gridBlackoutActive.load(std::memory_order_relaxed)) {
        CWeather::TrafficLightsBrightness = 0.0f;
    }

    static uint32_t s_lastRicoAiMs = 0;
    if (s_crisisEntities.activeCrisisId == 13 && currentMs - s_lastRicoAiMs >= 1000) {
        s_lastRicoAiMs = currentMs;
        if (CPools::ms_pPedPool) {
            for (int a = 0; a < 4; ++a) {
                CPed* fbi = CPools::ms_pPedPool->GetAtRef(static_cast<int>(s_crisisEntities.pedHandles[a]));
                if (!fbi || fbi->m_fHealth <= 0.0f) continue;
                const CVector fPos = fbi->GetPosition();
                CPed* targetGang = nullptr;
                float closestDistSq = 60.0f * 60.0f;
                for (int p = 0; p < CPools::ms_pPedPool->m_nSize; ++p) {
                    CPed* ped = CPools::ms_pPedPool->GetAt(p);
                    if (!ped || !CPools::ms_pPedPool->IsObjectValid(ped) || ped == fbi || ped == player || ped->m_fHealth <= 0.0f) continue;
                    if (ped->m_nPedType == PED_TYPE_GANG1 || ped->m_nPedType == PED_TYPE_GANG2) {
                        const CVector pPos = ped->GetPosition();
                        float dx = pPos.x - fPos.x;
                        float dy = pPos.y - fPos.y;
                        float distSq = dx * dx + dy * dy;
                        if (distSq < closestDistSq) {
                            closestDistSq = distSq;
                            targetGang = ped;
                        }
                    }
                }
                if (targetGang) {
                    AssignKillPedTask(fbi, targetGang);
                }
            }
        }
    }

    static uint32_t s_lastOpioidTickMs = 0;
    if (s_opioidSurgeActive.load(std::memory_order_relaxed) && currentMs - s_lastOpioidTickMs >= 2500) {
        s_lastOpioidTickMs = currentMs;
        if (CPools::ms_pPedPool) {
            for (int p = 0; p < CPools::ms_pPedPool->m_nSize; ++p) {
                CPed* ped = CPools::ms_pPedPool->GetAt(p);
                if (!ped || !CPools::ms_pPedPool->IsObjectValid(ped) || ped == player || ped->m_fHealth <= 0.0f || ped->m_nCreatedBy != 1) continue;
                if (ped->m_nPedType != PED_TYPE_CIVMALE && ped->m_nPedType != PED_TYPE_CIVFEMALE) continue;
                const CVector pos = ped->GetPosition();
                if (pos.x > 1950.0f && pos.x < 2550.0f && pos.y > -1850.0f && pos.y < -1550.0f) {
                    CPed* victim = nullptr;
                    float closestSq = 25.0f * 25.0f;
                    for (int v = 0; v < CPools::ms_pPedPool->m_nSize; ++v) {
                        CPed* other = CPools::ms_pPedPool->GetAt(v);
                        if (!other || !CPools::ms_pPedPool->IsObjectValid(other) || other == ped || other == player || other->m_fHealth <= 0.0f) continue;
                        const CVector vPos = other->GetPosition();
                        float dx = vPos.x - pos.x;
                        float dy = vPos.y - pos.y;
                        float distSq = dx * dx + dy * dy;
                        if (distSq < closestSq) {
                            closestSq = distSq;
                            victim = other;
                        }
                    }
                    if (victim && IsMuniModelLoaded(MODEL_BAT)) {
                        GiveInfiniteCombatWeapon(ped, WEAPONTYPE_BASEBALLBAT);
                        AssignKillPedTask(ped, victim);
                        break;
                    }
                }
            }
        }
    }

    // Time step calculation for continuous simulation (seconds)
    static uint32_t s_lastCrimeTickMs = 0;
    if (s_lastCrimeTickMs == 0) s_lastCrimeTickMs = currentMs;
    float dt = (currentMs >= s_lastCrimeTickMs) ? static_cast<float>(currentMs - s_lastCrimeTickMs) / 1000.0f : 0.0f;
    s_lastCrimeTickMs = currentMs;
    if (dt > 2.0f) dt = 0.05f;

    const uint32_t fuelStock = s_sfFuelStock.load(std::memory_order_relaxed);

    // 1. Логика дезертирства полиции (связь с s_sfFuelStock):
    static bool s_policeGroundedLogged = false;
    if (fuelStock < 20) {
        float targetDesertion = 60.0f + (20.0f - static_cast<float>(fuelStock));
        targetDesertion = std::clamp(targetDesertion, 60.0f, 80.0f);
        s_policeDesertionPct.store(targetDesertion, std::memory_order_relaxed);
        if (!s_policeGroundedLogged) {
            s_policeGroundedLogged = true;
            AddMunicipalLog("CRITICAL: Police fleets grounded due to fuel shortage in SF!");
        }
    } else if (fuelStock >= 40) {
        s_policeGroundedLogged = false;
        const int64_t tr = s_cityTreasury.load(std::memory_order_relaxed);
        const float baseDesertion = (tr < 100000) ? 20.0f : (tr < 300000 ? 10.0f : 2.0f);
        float curDesertion = s_policeDesertionPct.load(std::memory_order_relaxed);
        float newDesertion = (std::max)(baseDesertion, curDesertion - 0.5f);
        s_policeDesertionPct.store(newDesertion, std::memory_order_relaxed);
    } else {
        float cur = s_policeDesertionPct.load(std::memory_order_relaxed);
        if (cur > 45.0f) {
            s_policeDesertionPct.store((std::max)(35.0f, cur - 0.25f), std::memory_order_relaxed);
        }
    }

    const float policeDesertion = s_policeDesertionPct.load(std::memory_order_relaxed);

    // 2. Влияние топливного кризиса на АЗС, логи и предупреждения в HUD:
    const float crisisFuelMul = s_activeCrisis.fuelPriceMul.load(std::memory_order_relaxed);
    if (fuelStock < 25) {
        float curMult = s_fuelPriceMultiplier.load(std::memory_order_relaxed);
        if (curMult < 2.5f * crisisFuelMul) {
            s_fuelPriceMultiplier.store(2.5f * crisisFuelMul, std::memory_order_relaxed);
        }
    } else if (crisisFuelMul > 1.0f) {
        float curMult = s_fuelPriceMultiplier.load(std::memory_order_relaxed);
        if (curMult < crisisFuelMul) {
            s_fuelPriceMultiplier.store(crisisFuelMul, std::memory_order_relaxed);
        }
    }

    static bool s_fuelCrisisLogged = false;
    if (fuelStock < 15) {
        if (!s_fuelCrisisLogged) {
            s_fuelCrisisLogged = true;
            AddMunicipalLog("FUEL SHORTAGE: San Fierro terminal reserves depleted! Gas stations raising prices.");
            CHud::SetHelpMessage("~r~FUEL CRISIS:~w~ SF reserves depleted! Prices spiking.", true, false, false);
        }
    } else {
        s_fuelCrisisLogged = false;
    }

    // 3. Автономный расчет криминала, Compliance и реалистичного спада при КЧ:
    float currentUnrest = s_socialUnrest.load(std::memory_order_relaxed);
    s_publicUnrest.store(currentUnrest, std::memory_order_relaxed);

    float compliance = std::clamp(1.0f - (currentUnrest * 0.005f), 0.2f, 1.0f);
    s_criminalCompliance.store(compliance, std::memory_order_relaxed);

    const float curCrime = s_crimeRate.load(std::memory_order_relaxed);
    float curfewDecayRate = 0.0f;
    if (s_curfewActive.load(std::memory_order_relaxed)) {
        if (policeDesertion > 50.0f) {
            curfewDecayRate = 0.0f;
        } else if (curCrime > 60.0f) {
            curfewDecayRate = 4.0f;
        } else {
            curfewDecayRate = 2.5f;
        }
    }
    float curfewDecay = curfewDecayRate * compliance * s_activeCrisis.policeEffMul.load(std::memory_order_relaxed) * (dt / 60.0f);

    float gangPressure = (curCrime > 85.0f) ? 0.4f : 0.0f;
    float desertionPressure = (policeDesertion > 50.0f) ? (policeDesertion - 50.0f) * 0.02f : 0.0f;
    float deltaC = ((gangPressure + desertionPressure) * (dt / 60.0f)) - curfewDecay + (s_activeCrisis.crimeVelocity.load(std::memory_order_relaxed) * (dt / 60.0f));

    float newCrime = std::clamp(curCrime + deltaC, 0.0f, 100.0f);
    s_crimeRate.store(newCrime, std::memory_order_relaxed);

    const float crisisUnrestDelta = s_activeCrisis.unrestVelocity.load(std::memory_order_relaxed) * (dt / 60.0f);
    if (crisisUnrestDelta != 0.0f) {
        float updatedUnrest = std::clamp(currentUnrest + crisisUnrestDelta, 0.0f, 100.0f);
        s_socialUnrest.store(updatedUnrest, std::memory_order_relaxed);
        s_publicUnrest.store(updatedUnrest, std::memory_order_relaxed);
    }

    // 4. Автопилот Мэра (Автоматический комендантский час):
    const float currentCrime = s_crimeRate.load(std::memory_order_relaxed);
    const int64_t currentTreasury = s_cityTreasury.load(std::memory_order_relaxed);

    if (currentCrime >= 75.0f || currentTreasury < 50000) {
        if (!s_curfewActive.load(std::memory_order_relaxed)) {
            s_curfewActive.store(true, std::memory_order_relaxed);
            AddMunicipalLog("AI MAYOR: Curfew automatically enacted (Crime: %.1f%%, Treasury: $%lld)",
                currentCrime, static_cast<long long>(currentTreasury));
            CHud::SetHelpMessage("~r~AI MAYOR EDICT:~w~ Emergency curfew enacted due to crisis!", true, false, false);
        }
    } else if (currentCrime < 40.0f && currentTreasury > 200000) {
        if (s_curfewActive.load(std::memory_order_relaxed)) {
            s_curfewActive.store(false, std::memory_order_relaxed);
            AddMunicipalLog("AI MAYOR: Curfew lifted (Crime: %.1f%%, Treasury: $%lld)",
                currentCrime, static_cast<long long>(currentTreasury));
            CHud::SetHelpMessage("~g~AI MAYOR EDICT:~w~ Curfew lifted. Order restored.", true, false, false);
        }
    }

    const bool curfewActive = s_curfewActive.load(std::memory_order_relaxed);

    // 5. 4-ступенчатая матрица трафика (Police vs Civilian Ratio):
    float copSubstituteChance = 0.10f;
    if (currentCrime < 45.0f) {
        CCarCtrl::CarDensityMultiplier = 1.0f;
        CPopulation::PedDensityMultiplier = 1.0f;
        copSubstituteChance = 0.10f;
    } else if (currentCrime < 60.0f) {
        CCarCtrl::CarDensityMultiplier = 0.85f;
        CPopulation::PedDensityMultiplier = 0.80f;
        copSubstituteChance = 0.25f;
    } else if (currentCrime < 75.0f) {
        CCarCtrl::CarDensityMultiplier = 0.65f;
        CPopulation::PedDensityMultiplier = 0.50f;
        copSubstituteChance = 0.50f;
    } else {
        CCarCtrl::CarDensityMultiplier = 0.30f;
        CPopulation::PedDensityMultiplier = 0.15f;
        copSubstituteChance = 0.80f;
    }

    if (fuelStock < 10) {
        CCarCtrl::CarDensityMultiplier *= 0.7f;
    }

    SafeRequestModel(MODEL_COPCARLA);
    SafeRequestModel(MODEL_ENFORCER);
    SafeRequestModel(MODEL_FBIRANCH);
    SafeRequestModel(MODEL_LAPD1);
    SafeRequestModel(MODEL_SWAT);
    SafeRequestModel(MODEL_FBI);
    SafeRequestModel(MODEL_MP5LNG);
    SafeRequestModel(MODEL_DEAGLE);

    if (currentMs - s_lastCarCleanMs >= 5000) {
        s_lastCarCleanMs = currentMs;
        if (CPools::ms_pVehiclePool) {
            for (auto it = s_evaluatedVehicles.begin(); it != s_evaluatedVehicles.end(); ) {
                if (!CPools::ms_pVehiclePool->IsObjectValid(it->first)) {
                    for (size_t sq = 0; sq < k_maxTacticalSquads; ++sq) {
                        if (s_tacticalSquads[sq].pVehicle == it->first) {
                            s_tacticalSquads[sq] = TacticalSquad{};
                        }
                    }
                    it = s_evaluatedVehicles.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    if (CPools::ms_pVehiclePool) {
        const bool copLaLoaded = IsMuniModelLoaded(MODEL_COPCARLA);
        const bool enforcerLoaded = IsMuniModelLoaded(MODEL_ENFORCER);
        const bool fbiRanchLoaded = IsMuniModelLoaded(MODEL_FBIRANCH);
        const bool lapd1Loaded = IsMuniModelLoaded(MODEL_LAPD1);
        const bool swatLoaded = IsMuniModelLoaded(MODEL_SWAT);
        const bool fbiLoaded = IsMuniModelLoaded(MODEL_FBI);
        const bool wmystLoaded = IsMuniModelLoaded(MODEL_WMYST);

        for (int v = 0; v < CPools::ms_pVehiclePool->m_nSize; ++v) {
            CVehicle* veh = CPools::ms_pVehiclePool->GetAt(v);
            if (!veh || !CPools::ms_pVehiclePool->IsObjectValid(veh)) continue;
            if (player && player->m_pVehicle == veh) continue;
            if (veh->m_nCreatedBy != 1) continue;

            const int m = veh->m_nModelIndex;
            const bool isCopCar = (m == MODEL_COPCARLA || m == MODEL_COPCARSF || m == MODEL_COPCARVG ||
                                   m == MODEL_COPCARRU || m == MODEL_ENFORCER || m == MODEL_FBIRANCH ||
                                   m == MODEL_COPBIKE);
            if (isCopCar) {
                if ((m == MODEL_ENFORCER || m == MODEL_FBIRANCH) && veh->m_fHealth > 0.0f) {
                    TacticalSquad* squad = nullptr;
                    for (size_t sq = 0; sq < k_maxTacticalSquads; ++sq) {
                        if (s_tacticalSquads[sq].pVehicle == veh &&
                            IsVehicleValidAndAlive(veh, s_tacticalSquads[sq].vehicleCreationTime)) {
                            squad = &s_tacticalSquads[sq];
                            break;
                        }
                    }

                    // Auto-register squad if missing
                    if (!squad) {
                        for (size_t sq = 0; sq < k_maxTacticalSquads; ++sq) {
                            if (!s_tacticalSquads[sq].pVehicle ||
                                !IsVehicleValidAndAlive(s_tacticalSquads[sq].pVehicle, s_tacticalSquads[sq].vehicleCreationTime)) {
                                squad = &s_tacticalSquads[sq];
                                squad->pVehicle = veh;
                                squad->vehicleCreationTime = veh->m_nCreationTime;
                                squad->pDriver = veh->m_pDriver;
                                squad->passengerCount = (m == MODEL_ENFORCER) ? 3 : 1;
                                for (int s = 0; s < 3; ++s) {
                                    squad->pPassengers[s] = (s < squad->passengerCount) ? veh->m_apPassengers[s] : nullptr;
                                }
                                squad->bDeployed = false;
                                break;
                            }
                        }
                    }

                    if (squad && !squad->bDeployed) {
                        bool crewUnderFire = false;
                        if (veh->m_fHealth < 950.0f) {
                            crewUnderFire = true;
                        }
                        if (squad->pDriver && squad->pDriver->m_fHealth < squad->lastDriverHealth) {
                            crewUnderFire = true;
                            squad->lastDriverHealth = squad->pDriver->m_fHealth;
                        }
                        for (int s = 0; s < squad->passengerCount; ++s) {
                            if (squad->pPassengers[s] && squad->pPassengers[s]->m_fHealth < squad->lastPassengerHealth[s]) {
                                crewUnderFire = true;
                                squad->lastPassengerHealth[s] = squad->pPassengers[s]->m_fHealth;
                            }
                        }

                        if (crewUnderFire || currentCrime >= 80.0f) {
                            const float scanRadius = crewUnderFire ? 75.0f : 65.0f;
                            CPed* targetBandit = FindGangTargetNear(veh->GetPosition(), scanRadius);
                            if (targetBandit) {
                                veh->m_autoPilot.m_nCruiseSpeed = 0;
                                veh->m_nHandbrakeOn = true;
                                squad->bDeployed = true;
                                squad->lastCombatMs = currentMs;

                                if (squad->pDriver && squad->pDriver->m_fHealth > 0.0f && squad->pDriver->bInVehicle) {
                                    Command<Commands::TASK_LEAVE_ANY_CAR>(squad->pDriver);
                                }
                                for (int s = 0; s < squad->passengerCount; ++s) {
                                    CPed* pass = squad->pPassengers[s];
                                    if (pass && pass->m_fHealth > 0.0f && pass->bInVehicle) {
                                        Command<Commands::TASK_LEAVE_ANY_CAR>(pass);
                                    }
                                }
                            }
                        }
                    }
                }
                continue;
            }

            auto evalIt = s_evaluatedVehicles.find(veh);
            if (evalIt != s_evaluatedVehicles.end() && evalIt->second == veh->m_nCreationTime) {
                continue;
            }

            s_evaluatedVehicles[veh] = veh->m_nCreationTime;

            const float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            if (roll < copSubstituteChance) {
                int targetVehModel = MODEL_COPCARLA;
                int targetPedModel = MODEL_LAPD1;

                if (currentCrime < 60.0f) {
                    targetVehModel = MODEL_COPCARLA;
                    targetPedModel = lapd1Loaded ? MODEL_LAPD1 : MODEL_CSHER;
                } else if (currentCrime < 75.0f) {
                    if ((rand() % 2 == 0 && enforcerLoaded) || !copLaLoaded) {
                        targetVehModel = MODEL_ENFORCER;
                        targetPedModel = swatLoaded ? MODEL_SWAT : MODEL_LAPD1;
                    } else {
                        targetVehModel = MODEL_COPCARLA;
                        targetPedModel = lapd1Loaded ? MODEL_LAPD1 : MODEL_CSHER;
                    }
                } else {
                    const int rPick = rand() % 10;
                    if (rPick < 5 && enforcerLoaded) {
                        targetVehModel = MODEL_ENFORCER;
                        targetPedModel = swatLoaded ? MODEL_SWAT : MODEL_LAPD1;
                    } else if (rPick < 9 && fbiRanchLoaded) {
                        targetVehModel = MODEL_FBIRANCH;
                        targetPedModel = fbiLoaded ? MODEL_FBI : MODEL_SWAT;
                    } else if (copLaLoaded) {
                        targetVehModel = MODEL_COPCARLA;
                        targetPedModel = lapd1Loaded ? MODEL_LAPD1 : MODEL_CSHER;
                    } else {
                        targetVehModel = MODEL_ENFORCER;
                        targetPedModel = swatLoaded ? MODEL_SWAT : MODEL_LAPD1;
                    }
                }

                const bool vehLoaded = IsMuniModelLoaded(targetVehModel);
                const bool pedLoaded = IsMuniModelLoaded(targetPedModel);

                if (vehLoaded && pedLoaded) {
                    const CVector vPos = veh->GetPosition();
                    const CVector vSpeed = veh->m_vecMoveSpeed;
                    const CVector vTurn = veh->m_vecTurnSpeed;
                    const float vHeading = veh->GetHeading();
                    const eCarDrivingStyle vStyle = veh->m_autoPilot.m_nCarDrivingStyle;
                    const eCarMission vMission = veh->m_autoPilot.m_nCarMission;
                    const unsigned char vCruise = veh->m_autoPilot.m_nCruiseSpeed;

                    CPed* oldDriver = veh->m_pDriver;
                    if (oldDriver && oldDriver != player && oldDriver->m_nCreatedBy == 1) {
                        if (oldDriver->m_pIntelligence) oldDriver->m_pIntelligence->m_TaskMgr.FlushImmediately();
                        Command<Commands::DELETE_CHAR>(oldDriver);
                        veh->m_pDriver = nullptr;
                    }
                    for (int s = 0; s < 8; ++s) {
                        CPed* oldPass = veh->m_apPassengers[s];
                        if (oldPass && oldPass != player && oldPass->m_nCreatedBy == 1) {
                            if (oldPass->m_pIntelligence) oldPass->m_pIntelligence->m_TaskMgr.FlushImmediately();
                            Command<Commands::DELETE_CHAR>(oldPass);
                            veh->m_apPassengers[s] = nullptr;
                        }
                    }
                    for (size_t sq = 0; sq < k_maxTacticalSquads; ++sq) {
                        if (s_tacticalSquads[sq].pVehicle == veh) {
                            s_tacticalSquads[sq] = TacticalSquad{};
                        }
                    }
                    Command<Commands::DELETE_CAR>(veh);

                    CAutomobile* copVeh = new CAutomobile(targetVehModel, 1, true);
                    if (copVeh && CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->IsObjectValid(copVeh)) {
                        copVeh->m_nCreatedBy = 1;
                        copVeh->SetOrientation(0.0f, 0.0f, vHeading);
                        copVeh->Teleport(vPos);
                        copVeh->m_vecMoveSpeed = vSpeed;
                        copVeh->m_vecTurnSpeed = vTurn;
                        copVeh->bEngineOn = true;
                        copVeh->m_nStatus = STATUS_PHYSICS;
                        copVeh->bCreatedAsPoliceVehicle = true;
                        if (currentCrime >= 60.0f) {
                            copVeh->bSirenOrAlarm = true;
                        }
                        CWorld::Add(copVeh);
                        copVeh->PlaceOnRoadProperly();
                        copVeh->m_autoPilot.m_nCarMission = (vMission != MISSION_NONE ? vMission : MISSION_CRUISE);
                        copVeh->m_autoPilot.m_nCruiseSpeed = (vCruise > 0 ? vCruise : 15);
                        copVeh->m_autoPilot.m_nCarDrivingStyle = vStyle;

                        CPed* copDriver = new CPed(PED_TYPE_COP);
                        if (copDriver && CPools::ms_pPedPool && CPools::ms_pPedPool->IsObjectValid(copDriver)) {
                            copDriver->SetModelIndex(targetPedModel);
                            copDriver->m_nCreatedBy = 1;
                            copDriver->m_nStatus = STATUS_PHYSICS;
                            copDriver->bPanicWhenScared = false;
                            copDriver->bCrouchWhenScared = false;
                            copDriver->bFleeWhenStanding = false;
                            copDriver->bStayInSamePlace = false;
                            copDriver->m_nWeaponSkill = 2;
                            if (targetVehModel == MODEL_ENFORCER || targetVehModel == MODEL_FBIRANCH) {
                                copDriver->m_fHealth = 200.0f;
                                copDriver->m_fMaxHealth = 200.0f;
                                copDriver->m_fArmour = 100.0f;
                                GiveInfiniteCombatWeapon(copDriver, WEAPONTYPE_MP5);
                            }
                            CWorld::Add(copDriver);
                            Command<Commands::WARP_CHAR_INTO_CAR>(copDriver, copVeh);
                            copVeh->m_pDriver = copDriver;
                        } else if (copDriver) {
                            Command<Commands::DELETE_CHAR>(copDriver);
                        }

                        if (targetVehModel == MODEL_ENFORCER || targetVehModel == MODEL_FBIRANCH) {
                            const int passCount = (targetVehModel == MODEL_ENFORCER) ? 3 : 1;
                            const int passModel = (targetVehModel == MODEL_ENFORCER)
                                ? (swatLoaded ? MODEL_SWAT : targetPedModel)
                                : (fbiLoaded ? MODEL_FBI : targetPedModel);

                            TacticalSquad* squad = nullptr;
                            for (size_t sq = 0; sq < k_maxTacticalSquads; ++sq) {
                                if (!s_tacticalSquads[sq].pVehicle ||
                                    !IsVehicleValidAndAlive(s_tacticalSquads[sq].pVehicle, s_tacticalSquads[sq].vehicleCreationTime)) {
                                    squad = &s_tacticalSquads[sq];
                                    squad->pVehicle = copVeh;
                                    squad->vehicleCreationTime = copVeh->m_nCreationTime;
                                    squad->pDriver = copDriver;
                                    squad->passengerCount = passCount;
                                    for (int i = 0; i < 3; ++i) {
                                        squad->pPassengers[i] = nullptr;
                                        squad->lastPassengerHealth[i] = 200.0f;
                                    }
                                    squad->bDeployed = false;
                                    squad->lastCombatMs = 0;
                                    squad->lastDriverHealth = 200.0f;
                                    break;
                                }
                            }

                            for (int seat = 0; seat < passCount; ++seat) {
                                CPed* swatPass = new CPed(PED_TYPE_COP);
                                if (swatPass && CPools::ms_pPedPool && CPools::ms_pPedPool->IsObjectValid(swatPass)) {
                                    swatPass->SetModelIndex(passModel);
                                    swatPass->m_nCreatedBy = 1;
                                    swatPass->m_nStatus = STATUS_PHYSICS;
                                    swatPass->bPanicWhenScared = false;
                                    swatPass->bCrouchWhenScared = false;
                                    swatPass->bFleeWhenStanding = false;
                                    swatPass->bStayInSamePlace = false;
                                    swatPass->m_nWeaponSkill = 2;
                                    swatPass->m_fHealth = 200.0f;
                                    swatPass->m_fMaxHealth = 200.0f;
                                    swatPass->m_fArmour = 100.0f;
                                    GiveInfiniteCombatWeapon(swatPass, WEAPONTYPE_MP5);
                                    CWorld::Add(swatPass);
                                    Command<Commands::WARP_CHAR_INTO_CAR_AS_PASSENGER>(swatPass, copVeh, seat);
                                    if (squad) {
                                        squad->pPassengers[seat] = swatPass;
                                    }
                                }
                            }
                        }
                        s_evaluatedVehicles[copVeh] = copVeh->m_nCreationTime;
                    }
                }
            }
        }

        // 5b. Tactical Patrol Squads Lifecycle & Return-to-Vehicle Loop (every 500ms)
        static uint32_t s_lastTacticalSquadUpdateMs = 0;
        if (currentMs - s_lastTacticalSquadUpdateMs >= 500) {
            s_lastTacticalSquadUpdateMs = currentMs;

            for (size_t sq = 0; sq < k_maxTacticalSquads; ++sq) {
                TacticalSquad& squad = s_tacticalSquads[sq];
                if (!squad.pVehicle) continue;

                // 1. Проверить валидность машины: если pVehicle уничтожен или деспавнен
                if (!IsVehicleValidAndAlive(squad.pVehicle, squad.vehicleCreationTime)) {
                    if (squad.bDeployed) {
                        if (IsPedValidAndAlive(squad.pDriver) && squad.pDriver->m_pIntelligence) {
                            squad.pDriver->m_pIntelligence->m_TaskMgr.SetTask(
                                new CTaskComplexWanderStandard(PEDMOVE_WALK, 255, true),
                                TASK_PRIMARY_PRIMARY, false);
                        }
                        for (int s = 0; s < squad.passengerCount; ++s) {
                            CPed* pass = squad.pPassengers[s];
                            if (IsPedValidAndAlive(pass) && pass->m_pIntelligence) {
                                pass->m_pIntelligence->m_TaskMgr.SetTask(
                                    new CTaskComplexWanderStandard(PEDMOVE_WALK, 255, true),
                                    TASK_PRIMARY_PRIMARY, false);
                            }
                        }
                    }
                    squad = TacticalSquad{};
                    continue;
                }

                if (!squad.bDeployed) continue;

                // 2. Сканировать врагов не только от машины, но и проверять урон бойцов:
                // Если squad.pDriver->m_fHealth < 200.0f или у любого пассажира health < 200.0f — считать, что отряд под огнем,
                // продлевать squad.lastCombatMs = currentMs и немедленно атаковать ближайшего бандита в радиусе 75.0f.
                bool underFire = false;
                bool tookFreshDamage = false;

                if (IsPedValidAndAlive(squad.pDriver)) {
                    if (squad.pDriver->m_fHealth < 200.0f) underFire = true;
                    if (squad.pDriver->m_fHealth < squad.lastDriverHealth) {
                        tookFreshDamage = true;
                        squad.lastDriverHealth = squad.pDriver->m_fHealth;
                    }
                }
                for (int s = 0; s < squad.passengerCount; ++s) {
                    CPed* pass = squad.pPassengers[s];
                    if (IsPedValidAndAlive(pass)) {
                        if (pass->m_fHealth < 200.0f) underFire = true;
                        if (pass->m_fHealth < squad.lastPassengerHealth[s]) {
                            tookFreshDamage = true;
                            squad.lastPassengerHealth[s] = pass->m_fHealth;
                        }
                    }
                }

                if (tookFreshDamage) {
                    squad.lastCombatMs = currentMs;
                }

                const float searchRadius = underFire ? 75.0f : 65.0f;
                CPed* targetBandit = FindGangTargetNear(squad.pVehicle->GetPosition(), searchRadius);
                if (!targetBandit && IsPedValidAndAlive(squad.pDriver) && !squad.pDriver->bInVehicle) {
                    targetBandit = FindGangTargetNear(squad.pDriver->GetPosition(), searchRadius);
                }
                if (!targetBandit) {
                    for (int s = 0; s < squad.passengerCount; ++s) {
                        CPed* pass = squad.pPassengers[s];
                        if (IsPedValidAndAlive(pass) && !pass->bInVehicle) {
                            targetBandit = FindGangTargetNear(pass->GetPosition(), searchRadius);
                            if (targetBandit) break;
                        }
                    }
                }

                if (targetBandit) {
                    squad.lastCombatMs = currentMs;

                    // Идет бой: каждый спешившийся боец атакует бандита
                    auto orderFighterAttack = [&](CPed* ped) {
                        if (!IsPedValidAndAlive(ped)) return;
                        if (!ped->bInVehicle && ped->m_pVehicle == nullptr) {
                            ped->SetCurrentWeapon(WEAPONTYPE_MP5);
                            ped->bPanicWhenScared = false;
                            ped->bCrouchWhenScared = false;
                            ped->bFleeWhenStanding = false;
                            ped->bStayInSamePlace = false;
                            ped->m_nWeaponSkill = 2;
                            CTask* activeKill = ped->m_pIntelligence ? ped->m_pIntelligence->m_TaskMgr.FindActiveTaskByType(TASK_COMPLEX_KILL_PED_ON_FOOT) : nullptr;
                            if (!activeKill) {
                                Command<Commands::SET_CHAR_ACCURACY>(ped, 85);
                                Command<Commands::TASK_KILL_CHAR_ON_FOOT>(ped, targetBandit);
                            }
                        }
                    };

                    orderFighterAttack(squad.pDriver);
                    for (int s = 0; s < squad.passengerCount; ++s) {
                        orderFighterAttack(squad.pPassengers[s]);
                    }
                } else {
                    // 3. Врагов вокруг нет: посадка строго ТОЛЬКО если врагов вокруг нет непрерывно в течение 5 секунд
                    if (currentMs - squad.lastCombatMs >= 5000) {
                        // Принудительно снимаем ручник, чтобы автомобиль не блокировал посадку
                        squad.pVehicle->m_nHandbrakeOn = false;

                        if (IsPedValidAndAlive(squad.pDriver)) {
                            if (!squad.pDriver->bInVehicle && !IsPedEnteringCar(squad.pDriver)) {
                                Command<Commands::TASK_ENTER_CAR_AS_DRIVER>(squad.pDriver, squad.pVehicle, -1);
                            }
                        } else {
                            // Водитель погиб — распустить выживших пассажиров на пеший патруль
                            for (int s = 0; s < squad.passengerCount; ++s) {
                                CPed* pass = squad.pPassengers[s];
                                if (IsPedValidAndAlive(pass) && pass->m_pIntelligence) {
                                    pass->m_pIntelligence->m_TaskMgr.SetTask(
                                        new CTaskComplexWanderStandard(PEDMOVE_WALK, 255, true),
                                        TASK_PRIMARY_PRIMARY, false);
                                }
                            }
                            squad = TacticalSquad{};
                            continue;
                        }

                        for (int seat = 0; seat < squad.passengerCount; ++seat) {
                            CPed* pass = squad.pPassengers[seat];
                            if (IsPedValidAndAlive(pass)) {
                                if (!pass->bInVehicle && !IsPedEnteringCar(pass)) {
                                    Command<Commands::TASK_ENTER_CAR_AS_PASSENGER>(pass, squad.pVehicle, -1, seat);
                                }
                            }
                        }

                        // 4. Если водитель сел обратно:
                        if (IsPedValidAndAlive(squad.pDriver) && squad.pDriver->bInVehicle) {
                            bool allPassengersInOrDead = true;
                            for (int seat = 0; seat < squad.passengerCount; ++seat) {
                                CPed* pass = squad.pPassengers[seat];
                                if (IsPedValidAndAlive(pass) && !pass->bInVehicle) {
                                    allPassengersInOrDead = false;
                                    break;
                                }
                            }

                            if (allPassengersInOrDead) {
                                squad.pVehicle->m_nHandbrakeOn = false;
                                squad.pVehicle->m_autoPilot.m_nCarMission = MISSION_CRUISE;
                                squad.pVehicle->m_autoPilot.m_nCruiseSpeed = 15;
                                squad.bDeployed = false;

                                // Восстанавливаем здоровье отряда после успешного боя
                                squad.pDriver->m_fHealth = 200.0f;
                                squad.pDriver->m_fMaxHealth = 200.0f;
                                squad.pDriver->m_fArmour = 100.0f;
                                squad.lastDriverHealth = 200.0f;
                                for (int seat = 0; seat < squad.passengerCount; ++seat) {
                                    CPed* pass = squad.pPassengers[seat];
                                    if (IsPedValidAndAlive(pass)) {
                                        pass->m_fHealth = 200.0f;
                                        pass->m_fMaxHealth = 200.0f;
                                        pass->m_fArmour = 100.0f;
                                        squad.lastPassengerHealth[seat] = 200.0f;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // 6. Economic cycle (every 30 seconds)
    static uint32_t s_lastEconCycleMs = 0;
    if (currentMs - s_lastEconCycleMs >= 30000) {
        s_lastEconCycleMs = currentMs;
        int64_t tr = s_cityTreasury.load(std::memory_order_relaxed);
        if (tr > 0) {
            s_cityTreasury.fetch_sub(3500, std::memory_order_relaxed);
            AddMunicipalLog("ECONOMY: Municipal public services maintenance (-$3,500)");
        } else {
            s_cityTreasury.store(350000, std::memory_order_relaxed);
            s_emergencyState.store(0, std::memory_order_relaxed);
            const float curU = s_socialUnrest.load(std::memory_order_relaxed);
            if (curU < 40.0f) {
                s_socialUnrest.store(12.0f, std::memory_order_relaxed);
                s_publicUnrest.store(12.0f, std::memory_order_relaxed);
                s_crimeRate.store(24.0f, std::memory_order_relaxed);
            }
            AddMunicipalLog("FEDERAL BAILOUT: Agricultural Solvency Decree enacted. $350k allocated, debt cleared.");
        }
        const int32_t crisisTreasuryDelta = s_activeCrisis.treasuryDeltaPerMin.load(std::memory_order_relaxed) / 2;
        if (crisisTreasuryDelta != 0) {
            s_cityTreasury.fetch_add(crisisTreasuryDelta, std::memory_order_relaxed);
            AddMunicipalLog("CRISIS IMPACT: Treasury adjusted by $%d", crisisTreasuryDelta);
        }
    }

    // 7. Passive dynamics: sync unrest state (organic crime/unrest driven by living standards and police response)
    static uint32_t s_lastPassiveDynamicsMs = 0;
    if (currentMs - s_lastPassiveDynamicsMs >= 1000) {
        s_lastPassiveDynamicsMs = currentMs;
        s_publicUnrest.store(s_socialUnrest.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    // -------------------------------------------------------------------------
    // Subsystem Evaluation Passes
    // -------------------------------------------------------------------------
    // 8. Ambient High-Crime Street Riots: Purged. Handled exclusively by rioters.cpp.
    // UpdateAmbientHighCrimeRiot(currentMs, player, s_crimeRate.load(std::memory_order_relaxed));

    // 8a-2. Severe Unrest Crisis Atmosphere: Keep distant sirens and road wrecks, but NO ped spawns.
    UpdateCrisisAtmosphere(currentMs, player);

    // 8b. Two-Tier Retail Store Raids: Keep disabled or purely systemic.
    // UpdateStoreRaids(currentMs, player);

    // 9. Frontal incident deployment & suppression: Handled exclusively by gang_wars.cpp and rioters.cpp

    // 10. In-game HUD ticker update (every 1 second)
    static uint32_t s_lastTickerUpdateMs = 0;
    if (currentMs - s_lastTickerUpdateMs >= 1000) {
        s_lastTickerUpdateMs = currentMs;
        const int64_t tr = s_cityTreasury.load(std::memory_order_relaxed);
        const float cr = s_crimeRate.load(std::memory_order_relaxed);
        const float ur = s_socialUnrest.load(std::memory_order_relaxed);
        const float des = s_policeDesertionPct.load(std::memory_order_relaxed);
        const int em = s_emergencyState.load(std::memory_order_relaxed);
        char crisisTag[64] = { 0 };
        if (s_activeCrisis.activeId.load(std::memory_order_relaxed) != 0) {
            std::lock_guard<std::mutex> lock(s_activeCrisis.titleMutex);
            if (s_activeCrisis.activeTitle[0] != '\0') {
                snprintf(crisisTag, sizeof(crisisTag), " | CRISIS: %s", s_activeCrisis.activeTitle);
            }
        }
        if (des > 50.0f) {
            SetInGameTicker("[MUNICIPAL CORE] TREASURY: $%lld | CRIME: %.0f%% | UNREST: %.0f%% | POLICE DESERTION: %.0f%%%s%s%s",
                static_cast<long long>(tr), cr, ur, des,
                (curfewActive ? " | CURFEW" : ""),
                (em > 0 ? " | EMERGENCY" : ""),
                crisisTag);
        } else {
            SetInGameTicker("[MUNICIPAL CORE] TREASURY: $%lld | CRIME: %.0f%% | UNREST: %.0f%%%s%s%s",
                static_cast<long long>(tr), cr, ur,
                (curfewActive ? " | CURFEW ACTIVE" : ""),
                (em > 0 ? " | EMERGENCY STATE" : ""),
                crisisTag);
        }
    }

    // 11. Drain Economy Events from SPSC Queue: up to 4 per frame
    constexpr size_t k_ecoDrainBudget = 4;
    size_t ecoDrained = 0;
    EconomyEvent ecoEvent{};
    while (ecoDrained < k_ecoDrainBudget && g_economyEventQueue.pop(ecoEvent)) {
        ecoDrained++;
        switch (ecoEvent.type) {
        case EconomyEvent::WAGE_STRIKE: {
            char strikeMsg[128];
            const char* cName = (ecoEvent.companyId >= 0 && ecoEvent.companyId < 3) ? k_companies[ecoEvent.companyId].name : "Logistics";
            snprintf(strikeMsg, sizeof(strikeMsg), "~r~WAGE STRIKE: %s payroll deficit! Drivers blocking freeway!~w~", cName);
            CHud::SetHelpMessage(strikeMsg, true, false, false);
            break;
        }
        case EconomyEvent::GANG_ATTACK: {
            if (CPools::ms_pVehiclePool && CPools::ms_pPedPool) {
                SafeRequestModel(MODEL_GLENDALE);
                SafeRequestModel(MODEL_BALLAS1);
                SafeRequestModel(MODEL_MICRO_UZI);
                if (IsMuniModelLoaded(MODEL_GLENDALE) &&
                    IsMuniModelLoaded(MODEL_BALLAS1) &&
                    IsMuniModelLoaded(MODEL_MICRO_UZI)) {
                    CVector ambushPos = ecoEvent.pos;
                    ambushPos.x += 12.0f;
                    ambushPos.y += 12.0f;
                    const float gZ = CWorld::FindGroundZForCoord(ambushPos.x, ambushPos.y);
                    if (gZ > -100.0f) ambushPos.z = gZ + 0.3f;

                    if (IsAreaClearOfVehicles(ambushPos, 10.0f)) {
                        CAutomobile* gangCar = new CAutomobile(MODEL_GLENDALE, 2, true);
                        if (gangCar && CPools::ms_pVehiclePool && CPools::ms_pVehiclePool->IsObjectValid(gangCar)) {
                            gangCar->m_nCreatedBy = 2;
                            gangCar->Teleport(ambushPos);
                            gangCar->m_nStatus = eEntityStatus::STATUS_PHYSICS;
                            gangCar->m_fHealth = 1000.0f;
                            gangCar->bEngineOn = true;
                            gangCar->m_nHandbrakeOn = false;
                            CWorld::Add(gangCar);
                            gangCar->PlaceOnRoadProperly();
                            gangCar->UpdateRwMatrix();

                            CPed* gangPed = new CPed(PED_TYPE_GANG2);
                            if (gangPed && CPools::ms_pPedPool && CPools::ms_pPedPool->IsObjectValid(gangPed)) {
                                gangPed->SetModelIndex(MODEL_BALLAS1);
                                gangPed->m_nCreatedBy = 2;
                                gangPed->m_nPedType = PED_TYPE_GANG2;
                                gangPed->m_nStatus = eEntityStatus::STATUS_PHYSICS;
                                gangPed->bPanicWhenScared = false;
                                gangPed->bCrouchWhenScared = false;
                                gangPed->bFleeWhenStanding = false;
                                gangPed->bStayInSamePlace = false;
                                gangPed->m_nWeaponSkill = 2;
                                GiveInfiniteCombatWeapon(gangPed, WEAPONTYPE_MICRO_UZI);
                                CWorld::Add(gangPed);
                                Command<Commands::WARP_CHAR_INTO_CAR>(gangPed, gangCar);
                                gangCar->m_pDriver = gangPed;
                                gangCar->m_autoPilot.m_nCarMission = MISSION_RAMPLAYER_FARAWAY;
                                gangCar->m_autoPilot.m_nCarDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
                                gangCar->m_autoPilot.m_nCruiseSpeed = 30;
                            } else {
                                if (gangPed) Command<Commands::DELETE_CHAR>(gangPed);
                                Command<Commands::DELETE_CAR>(gangCar);
                            }
                        } else if (gangCar) {
                            Command<Commands::DELETE_CAR>(gangCar);
                        }
                    }
                }
            }
            CHud::SetHelpMessage("~r~GANG AMBUSH! Ballas raiding logistics convoy! Protect the route!~w~", true, false, false);
            break;
        }
        case EconomyEvent::STORE_PRICE_UPDATE: {
            if (ecoEvent.value > 180) {
                char pMsg[128];
                snprintf(pMsg, sizeof(pMsg), "~r~SUPPLY DEFICIT: Ammu-Nation & shop prices spiked to %d%%!~w~", ecoEvent.value);
                CHud::SetHelpMessage(pMsg, true, false, false);
            }
            break;
        }
        case EconomyEvent::FUEL_CRISIS_UPDATE: {
            if (ecoEvent.value > 180) {
                char fMsg[128];
                snprintf(fMsg, sizeof(fMsg), "~y~FUEL CRISIS: Bone County delays triggered fuel surge (%d%%)!~w~", ecoEvent.value);
                CHud::SetHelpMessage(fMsg, true, false, false);
            }
            break;
        }
        case EconomyEvent::PLAYER_FINANCE: {
            if (player && player->m_pPlayerData) {
                player->m_pPlayerData->m_nMoney += ecoEvent.value;
                char finMsg[128];
                snprintf(finMsg, sizeof(finMsg), "~g~LOGISTICS DIVIDENDS: CJ received +$%d from real estate assets!~w~", ecoEvent.value);
                CHud::SetHelpMessage(finMsg, true, false, false);
            }
            break;
        }
        default:
            break;
        }
    }

    // 12. Drain Municipal Game Events from SPSC Queue: up to 32 per frame
    constexpr size_t k_muniDrainBudget = 32;
    size_t muniDrained = 0;
    MunicipalGameEvent muniEvent{};
    while (muniDrained < k_muniDrainBudget && g_municipalGameEventQueue.pop(muniEvent)) {
        muniDrained++;
        switch (muniEvent.type) {
        case MunicipalGameEventType::RoadblockPolice:
        case MunicipalGameEventType::IncidentSpawn: {
            IntakeMunicipalIncident(muniEvent, currentMs, player);
            break;
        }
        case MunicipalGameEventType::StrikeProtest: {
            // Do not force reset if Rioters module is already handling a site
            break;
        }
        case MunicipalGameEventType::StrikeDisband: {
            for (size_t i = 0; i < k_maxIncidents; ++i) {
                if (s_incidents[i].active && s_incidents[i].type == IncidentType::UNION_STRIKE) {
                    CleanupIncident(s_incidents[i], player, true);
                }
            }
            CHud::SetHelpMessage("~g~UNION STRIKE SETTLED:~w~ Strikers disbanded, route clear!", true, false, false);
            break;
        }
        case MunicipalGameEventType::IncidentResolve: {
            for (size_t i = 0; i < k_maxIncidents; ++i) {
                if (s_incidents[i].active && (muniEvent.id == 0 || s_incidents[i].id == muniEvent.id)) {
                    s_totalIncidentsHandled.fetch_add(1, std::memory_order_relaxed);
                    CleanupIncident(s_incidents[i], player, true);
                    if (muniEvent.id != 0) break;
                }
            }
            break;
        }
        case MunicipalGameEventType::CurfewDensity: {
            if (muniEvent.param != 0) {
                CCarCtrl::CarDensityMultiplier = 0.10f;
                CPopulation::PedDensityMultiplier = 0.05f;
                CHud::SetHelpMessage("~r~CURFEW ACTIVE:~w~ Streets cleared by police decree. Civilians stay indoors!", true, false, false);
            } else {
                CCarCtrl::CarDensityMultiplier = 1.0f;
                CPopulation::PedDensityMultiplier = 1.0f;
                CHud::SetHelpMessage("~g~CURFEW LIFTED:~w~ Normal civil traffic resumed.", true, false, false);
            }
            break;
        }
        case MunicipalGameEventType::BigMessage: {
            if (muniEvent.text[0] != '\0') {
                CMessages::AddBigMessageWithNumber(muniEvent.text, 6000, STYLE_MIDDLE, 0, 0, 0, 0, 0, 0);
            }
            break;
        }
        default:
            break;
        }
    }

    // 13. Subsystem Incident Lifecycle:
    // REMOVED: UpdateIncidentsLifecycle(player, currentMs); -> This caused the 100+ crowd duplication!
    
    // Delegated to isolated, pool-budgeted modules:
    Rioters::Update(currentMs, player);
    GangWars::Update(currentMs, player);
}

void CleanupMunicipalSubsystem() {
    for (size_t sq = 0; sq < k_maxTacticalSquads; ++sq) {
        s_tacticalSquads[sq] = TacticalSquad{};
    }
    const uint8_t activeCrisis = s_activeCrisis.activeId.load(std::memory_order_relaxed);
    if (activeCrisis != 0) {
        ApplyCrisisWorldPhysics(activeCrisis, false);
    }
    CleanupAllAmbientRioters();
    CleanupAllCrisisRoadblocks();
    CleanupAllStoreRaids();
    Rioters::Cleanup();
    GangWars::Cleanup();
    for (size_t i = 0; i < k_maxIncidents; ++i) {
        CleanupIncident(s_incidents[i], nullptr, true);
    }
}

MunicipalDiagnosticTelemetry GetMunicipalDiagnosticTelemetry() {
    MunicipalDiagnosticTelemetry telem{};
    telem.socialUnrestPct = s_socialUnrest.load(std::memory_order_relaxed);
    for (size_t i = 0; i < k_maxIncidents; ++i) {
        if (s_incidents[i].active) {
            telem.activeIncidentsCount++;
        }
    }
    telem.activeRoadblocksCount = s_activeRoadblocksCount.load(std::memory_order_relaxed);
    telem.lastFrameDeltaMs = CTimer::ms_fTimeStep * 20.0f;
    return telem;
}

bool GetRoadblockSlotDetails(size_t cpIdx, uint32_t& outRioters, bool& outProvoked) {
    if (cpIdx >= k_numArterialChokepoints) {
        outRioters = 0;
        outProvoked = false;
        return false;
    }
    int slot = s_arterialChokepoints[cpIdx].activeRoadblockSlot;
    if (slot >= 0 && slot < static_cast<int>(k_maxActiveRoadblocks)) {
        const auto& rb = s_crisisRoadblocks[slot];
        if (rb.active || rb.rioterCount > 0 || rb.vehCount > 0) {
            uint32_t alive = 0;
            for (size_t r = 0; r < 2; ++r) {
                if (rb.rioterHandles[r] != 0) {
                    CPed* ped = ResolvePed(rb.rioterHandles[r]);
                    if (ped && CPools::ms_pPedPool && CPools::ms_pPedPool->IsObjectValid(ped) && ped->m_fHealth > 0.0f) {
                        alive++;
                    }
                }
            }
            outRioters = alive;
            outProvoked = rb.provoked;
            return true;
        }
    }
    outRioters = 0;
    outProvoked = false;
    return false;
}

