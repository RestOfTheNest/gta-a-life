#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <winsock2.h>
#include <windows.h>
#include <atomic>
#include <mutex>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include "CVector.h"
#include "spsc_queue.h"

// =============================================================================
//  Highway & Logistics Fleet Structures
// =============================================================================

struct HighwayWaypoint {
    float x;
    float y;
    float z;
};

enum class CustomShopType : uint8_t {
    None = 0,
    GasStation,
    Commercial
};

enum class TruckState : uint8_t {
    LOADING = 0,     // At Ocean Docks (Waypoint 0)
    EN_ROUTE = 1,    // Driving along the highway
    IN_TRANSIT = 1,  // Driving along the highway (alias for EN_ROUTE)
    RESTING = 2,     // Sleeping at Bone County Truck Stop (Waypoint 135) or roadside nap
    UNLOADING = 3,   // Unloading cargo at San Fierro Terminal (Waypoint 240)
    BROKEN_DOWN = 4, // Engine failure / roadside SOS breakdown
    DESTROYED = 5,   // Totaled / fatal wreck / written off
    INSPECTION = 6   // Weigh Station (Waypoint 85) & police highway checkpoint
};

enum class ComplianceTier : uint8_t {
    STANDARD = 0,
    WATCHLIST,
    GRAY_MARKET
};

struct LogisticsCompany {
    uint8_t     id;
    const char* name;
    const char* color;
    const char* baseHub;
};

struct LogisticsAsset {
    uint8_t     id;
    const char* name;
    uint32_t    cost;
};

struct LogisticsWarehouseStatus {
    uint32_t fuelStock = 0;
    uint32_t timberStock = 0;
    uint32_t electronicsStock = 0;
    uint32_t foodStock = 0;
    bool     isFuelCritical = false;
};

struct DistrictAABB {
    uint32_t id;
    char name[32];
    float minX, maxX;
    float minY, maxY;
    float baseCrimeRate;
    float wealthModifier;
};
extern DistrictAABB g_districtZones[4];
extern std::atomic<uint32_t> s_playerCurrentDistrict;
int GetDistrictByCoords(float x, float y);
inline const char* GetDistrictSpecialization(uint32_t districtId) {
    switch (districtId) {
        case 0: return "Primary Demand: Food & Ammunition";
        case 1: return "Primary Demand: High-Octane Fuel & Tech";
        case 2: return "Primary Demand: Industrial Lumber & Heavy Fuel";
        case 3: return "Primary Demand: Transit Fuel & Rural Timber";
        default: return "Primary Demand: Balanced General";
    }
}

// =============================================================================
//  District Living Standards & Citizen Real Wage Burden
// =============================================================================

struct DistrictLivingStandards {
    uint32_t id{ 0 };
    char     name[32]{ 0 };
    float    baseWage{ 0.0f };
    float    effectiveWage{ 0.0f };
    float    dynamicBonus{ 0.0f };
    float    fixedLivingCost{ 0.0f };
    float    crimeSurcharge{ 0.0f };
    float    basketCost{ 0.0f };
    float    pollTax{ 0.0f };
    float    totalDeduction{ 0.0f };
    float    netRatio{ 0.0f };
    char     status[32]{ 0 };
    bool     austerity{ false };
    float    homeownershipRate{ 0.30f };
    int32_t  baseRent{ 480 };
    int32_t  baseMaintenance{ 140 };
    float    effectiveHousingCost{ 0.0f };
    int64_t  householdSavingsPool{ 500000 };
    uint32_t consecutiveDeficitCycles{ 0 };
};
extern DistrictLivingStandards g_districtLivingStandards[4];

struct FleecaBankState {
    std::atomic<int64_t>  totalReserves{2500000};
    std::atomic<float>    dynamicInterestRate{0.05f};
    std::atomic<uint32_t> activeBailoutsCount{0};
    std::atomic<uint32_t> bankruptStoresCount{0};
};
extern FleecaBankState g_fleecaBank;

struct RetailStoreNode {
    uint32_t id{0};
    uint32_t categoryId{0}; // 0: Food/Grocery, 1: Gas Station, 2: Agro/Produce & Provisions, 3: Tech/Ammu
    uint32_t districtId{0}; // 0: South Central, 1: Downtown, 2: Industrial Port, 3: Country/Highway
    float posX{0.0f};
    float posY{0.0f};
    char name[32]{0};
    std::atomic<uint32_t> staffWagesCycle{600};
    std::atomic<uint32_t> propertyRentCycle{450};
    std::atomic<uint64_t> totalTaxesPaid{0};
    std::atomic<uint32_t> localStock{180};
    std::atomic<uint32_t> maxCapacity{300};
    std::atomic<uint32_t> minThreshold{90};
    std::atomic<uint32_t> batchOrderSize{100};
    std::atomic<int64_t>  capitalBalance{25000};
    std::atomic<uint64_t> totalUnitsSold{0};
    std::atomic<uint64_t> lifetimeRevenue{0};
    std::atomic<uint32_t> windowUnitsSold{0};
    std::atomic<float>    salesVelocityPerMin{0.0f};
    std::atomic<int64_t>  loanDebt{0};
    std::atomic<int32_t>  creditScore{80}; // Range: [0, 100]
    std::atomic<bool>     isBankrupt{false};
    std::atomic<float>    fractionalSalesAccum{0.0f};
    std::atomic<bool>     isUnderRaid{false};
    std::atomic<uint32_t> raidStartMs{0};
    std::atomic<uint32_t> lastDrainMs{0};
    std::atomic<uint32_t> stolenStock{0};
    std::atomic<uint32_t> raidCooldownUntilMs{0};
    std::atomic<bool>     isRansacked{false};

    RetailStoreNode() = default;
    RetailStoreNode(uint32_t storeId, uint32_t catId, float x, float y, const char* storeName, uint32_t wages = 600, uint32_t rent = 450, uint32_t initStock = 180, uint32_t maxCap = 300, uint32_t minThresh = 90, uint32_t batchSize = 100, int64_t initBal = 25000)
        : id(storeId), categoryId(catId), districtId(0), posX(x), posY(y), staffWagesCycle(wages), propertyRentCycle(rent), localStock(initStock), maxCapacity(maxCap), minThreshold(minThresh), batchOrderSize(batchSize), capitalBalance(initBal)
    {
        size_t idx = 0;
        if (storeName) {
            while (storeName[idx] && idx < sizeof(name) - 1) {
                name[idx] = storeName[idx];
                idx++;
            }
        }
        name[idx] = '\0';
    }
    RetailStoreNode(uint32_t storeId, uint32_t catId, uint32_t distId, float x, float y, const char* storeName, uint32_t wages = 600, uint32_t rent = 450, uint32_t initStock = 180, uint32_t maxCap = 300, uint32_t minThresh = 90, uint32_t batchSize = 100, int64_t initBal = 25000)
        : id(storeId), categoryId(catId), districtId(distId), posX(x), posY(y), staffWagesCycle(wages), propertyRentCycle(rent), localStock(initStock), maxCapacity(maxCap), minThreshold(minThresh), batchOrderSize(batchSize), capitalBalance(initBal)
    {
        size_t idx = 0;
        if (storeName) {
            while (storeName[idx] && idx < sizeof(name) - 1) {
                name[idx] = storeName[idx];
                idx++;
            }
        }
        name[idx] = '\0';
    }
    RetailStoreNode(uint32_t storeId, uint32_t catId, uint32_t distId, const char* storeName, uint32_t wages = 600, uint32_t rent = 450, uint32_t initStock = 180, uint32_t maxCap = 300, uint32_t minThresh = 90, uint32_t batchSize = 100, int64_t initBal = 25000)
        : id(storeId), categoryId(catId), districtId(distId), posX(0.0f), posY(0.0f), staffWagesCycle(wages), propertyRentCycle(rent), localStock(initStock), maxCapacity(maxCap), minThreshold(minThresh), batchOrderSize(batchSize), capitalBalance(initBal)
    {
        size_t idx = 0;
        if (storeName) {
            while (storeName[idx] && idx < sizeof(name) - 1) {
                name[idx] = storeName[idx];
                idx++;
            }
        }
        name[idx] = '\0';
    }
    RetailStoreNode(uint32_t storeId, uint32_t catId, const char* storeName, uint32_t wages = 600, uint32_t rent = 450, uint32_t initStock = 180, uint32_t maxCap = 300, uint32_t minThresh = 90, uint32_t batchSize = 100, int64_t initBal = 25000, uint32_t distId = 0)
        : id(storeId), categoryId(catId), districtId(distId), posX(0.0f), posY(0.0f), staffWagesCycle(wages), propertyRentCycle(rent), localStock(initStock), maxCapacity(maxCap), minThreshold(minThresh), batchOrderSize(batchSize), capitalBalance(initBal)
    {
        size_t idx = 0;
        if (storeName) {
            while (storeName[idx] && idx < sizeof(name) - 1) {
                name[idx] = storeName[idx];
                idx++;
            }
        }
        name[idx] = '\0';
    }
};
using RetailStore = RetailStoreNode;

struct VirtualTruck {
    uint32_t    id;
    float       x, y, z;
    float       heading;
    float       speed;
    size_t      currentTargetNode;
    bool        isMaterialized;
    bool        spawnRequested = false;
    uint32_t    spawnPendingTicks = 0;
    uint32_t    physicalHandle; // SCM handle in CPools::ms_pVehiclePool
    TruckState  state = TruckState::EN_ROUTE;
    uint32_t    stateTimer = 0; // Ticks/ms spent in current static state
    uint32_t    blockedSinceMs = 0;
    float       fuel = 100.0f;       // 0.0% to 100.0%
    float       fatigue = 0.0f;     // 0.0% (rested) to 100.0% (exhausted)
    int         cargoType = 0;        // 0: Empty, 1: Timber, 2: Fuel/Oil, 3: Electronics, 4: Food
    int         cargoWeightTons = 0;  // 0 to 25
    int         deliveriesDone = 0;
    uint8_t     companyId = 0;
    const char* driverName = "Driver";
    uint32_t    driverWallet = 0;
    uint32_t    deadlineTicks = 33600; // 28-minute contract deadline (33600 ticks @ 20 Hz)
    bool        deadlinePenalized = false;
    uint32_t    experience = 0;
    uint8_t     skillLevel = 1; // 1: Rookie, 2: Veteran, 3: Master
    ComplianceTier complianceTier = ComplianceTier::STANDARD;
    bool        isOverloaded = false;
    bool        travelForward = true; // True: Outbound towards terminal; False: Inbound back to base
};

// Snapshot item for wait-free publication to Web Worker
struct TruckSnapshotItem {
    uint32_t    id;
    float       x;
    float       y;
    float       z;
    float       heading;
    float       speed;
    bool        isMaterialized;
    TruckState  state;
    float       fuel;
    float       fatigue;
    int         cargoType;
    int         cargoWeightTons;
    int         deliveriesDone;
    uint8_t     companyId;
    const char* driverName;
    uint32_t    driverWallet;
    uint32_t    deadlineTicks;
    uint32_t    experience;
    uint8_t     skillLevel;
};

// Feedback from game thread to worker for materialized vehicles
// Force 64-byte hardware cache line boundary to eliminate false sharing across CPU cores
struct alignas(64) PhysicalTruckFeedback {
    // 4-byte atomic floats (20 bytes)
    std::atomic<float> x{ 0.0f };
    std::atomic<float> y{ 0.0f };
    std::atomic<float> z{ 0.0f };
    std::atomic<float> heading{ 0.0f };
    std::atomic<float> speed{ 0.0f };

    // 4-byte atomic uint32 (12 bytes)
    std::atomic<uint32_t> targetWaypointIndex{ 0 };
    std::atomic<uint32_t> cargoWeightTons{ 15 };
    std::atomic<uint32_t> lastUpdateMs{ 0 };

    // 1-byte atomics (11 bytes)
    std::atomic<bool>     active{ false };
    std::atomic<bool>     brokenDown{ false };
    std::atomic<bool>     destroyed{ false };
    std::atomic<bool>     stopped{ false };
    std::atomic<bool>     isSpawned{ false };
    std::atomic<bool>     hijackedReset{ false };
    std::atomic<bool>     pullOverRequested{ false };
    std::atomic<bool>     isOverloaded{ false };
    std::atomic<bool>     travelForward{ true };
    std::atomic<uint8_t>  companyId{ 0 };
    std::atomic<uint8_t>  cargoType{ 0 };

    // Explicit padding to ensure struct fills out complete cache line (64 bytes)
    uint8_t _cachePad[64 - ((sizeof(std::atomic<float>) * 5) + (sizeof(std::atomic<uint32_t>) * 3) + (sizeof(std::atomic<bool>) * 9) + (sizeof(std::atomic<uint8_t>) * 2)) % 64];
};
static_assert(sizeof(PhysicalTruckFeedback) == 64, "PhysicalTruckFeedback must be exactly 64 bytes");

// =============================================================================
//  Custom Logistics Routes Structures
// =============================================================================

struct CustomRouteNode {
    float x = 0.0f;
    float y = 0.0f;
    float z = 10.0f;
};

struct CompanyCustomRoute {
    uint8_t companyId = 0;
    char color[16] = "#38bdf8";
    std::vector<CustomRouteNode> nodes;
    bool active = false;
};

// =============================================================================
//  Municipal & World Event Structures
// =============================================================================

struct MunicipalIncident {
    uint32_t id = 0;
    char     type[32] = { 0 };      // "Gang Shootout", "Civil Riot", "Store Looting", etc.
    char     location[32] = { 0 };  // "Ganton", "Ocean Flats", "Garcia", "Idlewood"
    float    x = 0.0f;
    float    y = 0.0f;
    float    z = 0.0f;
    uint8_t  severity = 1;          // 1: Low, 2: Moderate, 3: High, 4: Critical
    char     status[32] = { 0 };    // "DISPATCHED", "IN_PROGRESS", "RESOLVED"
    uint32_t timerTicks = 0;
    bool     active = false;
};

struct MunicipalLogEntry {
    char message[128] = { 0 };
};

struct SyndicateBribeOffer {
    uint32_t id = 0;
    char     syndicate[48] = { 0 };  // "Da Nang Boys", "San Fierro Rifa", "Russian Syndicate", "Loco Cartel"
    char     description[96] = { 0 };// "Smuggle untraceable military weapons through Ocean Docks"
    int64_t  amount = 0;             // $150,000 to $350,000
    float    crimeDelta = 0.0f;      // +15.0f
    float    unrestDelta = 0.0f;     // +8.0f
    uint32_t expireTicks = 0;        // Ticks until offer expires
    bool     active = false;
};

struct UnionStrikeEvent {
    uint32_t id = 0;
    char     unionName[48] = { 0 };  // "San Andreas Teamsters Union", "Dockworkers Local 402", "Transit Guild"
    char     location[48] = { 0 };   // "Ocean Docks Gate", "Ganton Interchange", "Easter Basin Depot"
    float    x = 0.0f;
    float    y = 0.0f;
    float    z = 0.0f;
    uint32_t durationTicks = 0;      // Seconds active
    bool     active = false;
};

struct WorldCrisisDef {
    uint8_t     id;
    char        tag[32];
    char        title[64];
    float       fuelPriceMul;
    float       cargoPriceMul;
    float       crimeVelocity;
    float       unrestVelocity;
    int32_t     treasuryDeltaPerMin;
    float       policeEffMul;
    uint32_t    durationMs;
    char        tickerHeadline[128];
};

struct ActiveWorldCrisisState {
    std::atomic<uint8_t>  activeId{ 0 }; // 0 = NONE
    std::atomic<uint32_t> startMs{ 0 };
    std::atomic<uint32_t> durationMs{ 0 };
    std::atomic<float>    fuelPriceMul{ 1.0f };
    std::atomic<float>    cargoPriceMul{ 1.0f };
    std::atomic<float>    crimeVelocity{ 0.0f };
    std::atomic<float>    unrestVelocity{ 0.0f };
    std::atomic<int32_t>  treasuryDeltaPerMin{ 0 };
    std::atomic<float>    policeEffMul{ 1.0f };
    char                  activeTitle[64] = { 0 };
    char                  activeHeadline[128] = { 0 };
    std::mutex            titleMutex;
};

// =============================================================================
//  Command & Telemetry & Event Contracts
// =============================================================================

enum class CommandType : uint8_t {
    SetWeather,
    RestoreVitals,
    DisplayNotice
};

struct GameCommand {
    CommandType type;
    int32_t     param;
};

struct TelemetryState {
    float    x;
    float    y;
    float    z;
    float    speed;
    uint32_t timestamp;
};

enum class AIDirectiveType : uint8_t {
    InterceptTarget,
    SpawnTruck,
    DespawnTruck,
    DisplayHudNotice
};

struct AIDirective {
    AIDirectiveType type;
    uint32_t        truckId;
    float           targetX;
    float           targetY;
    float           targetZ;
    float           heading;
    float           speed;
    uint32_t        directiveId;
    uint32_t        targetWaypointIndex = 0;
    char            noticeMsg[128] = { 0 };
};

struct EconomyEvent {
    enum Type {
        WAGE_STRIKE,
        GANG_ATTACK,
        STORE_PRICE_UPDATE,
        FUEL_CRISIS_UPDATE,
        PLAYER_FINANCE,
        SHOP_PURCHASE
    };
    Type    type;
    int     companyId;
    int     value;
    CVector pos;
};

enum class MunicipalGameEventType : uint8_t {
    None,
    RoadblockPolice,
    StrikeProtest,
    StrikeDisband,
    IncidentSpawn,
    IncidentResolve,
    CurfewDensity,
    BigMessage
};

struct MunicipalGameEvent {
    MunicipalGameEventType type = MunicipalGameEventType::None;
    uint32_t id = 0;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    char text[128] = { 0 };
    uint32_t param = 0;
};

// =============================================================================
//  A-Life Municipal Incidents
// =============================================================================

enum class IncidentType : uint8_t { ROADBLOCK, UNION_STRIKE, GANTON_RIOT };
enum class AnimState : uint8_t { NOT_LOADED, REQUESTED, READY, FAILED };

struct ActiveIncident {
    bool active = false;
    uint32_t id = 0;
    IncidentType type = IncidentType::ROADBLOCK;
    CVector pos{ 0.0f, 0.0f, 0.0f };
    float heading = 0.0f;
    uint32_t blipHandle = 0;
    uint32_t spawnTimeMs = 0;
    uint32_t durationMs = 600000;
    bool isMaterialized = false;
    bool inCombat = false;
    bool provoked = false;
    AnimState animState = AnimState::NOT_LOADED;

    uint32_t vehHandle = 0;
    uint32_t vehHandle2 = 0;
    uint32_t strikeBarrierHandle = 0; // Машина-баррикада забастовщиков
    uint32_t patrolVehHandle = 0;     // Патрульная машина копов у митинга
    uint32_t patrolDriverHandle = 0;  // Водитель патрульной машины
    uint32_t groupAPeds[8] = { 0 };
    uint32_t groupBPeds[8] = { 0 };
    uint32_t lastTargets[8] = { 0 };
    uint32_t lastTargetsB[8] = { 0 };
    bool copsAlerted[8] = { false };
    uint32_t coneHandles[16] = { 0 };
};

constexpr size_t k_maxIncidents = 32;

// =============================================================================
//  Extern Global Queues & State Variables
// =============================================================================

extern SPSCQueue<GameCommand, 256> g_webCommandQueue;
extern SPSCQueue<TelemetryState, 256> g_aiTelemetryQueue;
extern SPSCQueue<AIDirective, 256> g_aiDirectiveQueue;
extern SPSCQueue<EconomyEvent, 256> g_economyEventQueue;
extern SPSCQueue<EconomyEvent, 256> g_shopPurchaseQueue;
extern SPSCQueue<MunicipalGameEvent, 128> g_municipalGameEventQueue;

extern std::atomic<int64_t> s_cityTreasury;
extern std::atomic<float> s_crimeRate;
extern std::atomic<float> s_socialUnrest;
extern std::atomic<bool> s_curfewActive;
extern std::atomic<int> s_emergencyState;
extern std::atomic<bool> s_impeachmentTriggered;
extern std::atomic<bool> s_fuelCrisisActive;

extern std::atomic<float> s_taxSalesGeneral;    // Default: 0.12f (Range: 0.02 - 0.30)
extern std::atomic<float> s_taxSalesLuxury;     // Default: 0.25f (Range: 0.05 - 0.50)
extern std::atomic<float> s_taxHaulerPort;      // Default: 0.15f (Range: 0.05 - 0.40)
extern std::atomic<float> s_taxCorporateWealth; // Default: 0.03f (Range: 0.00 - 0.15)
extern std::atomic<int32_t> s_taxCitizenPoll;   // Master/legacy uniform preset (Range: $0 - $4000)
extern std::atomic<float>   s_districtPollTax[4]; // Granular District Poll Taxes: 0=South Central, 1=Downtown, 2=Port, 3=Rural

extern ActiveIncident s_incidents[k_maxIncidents];

extern std::atomic<int64_t> s_companyBalances[3];
extern std::atomic<int8_t> s_assetOwners[3];
extern std::atomic<uint32_t> s_sfTimberStock, s_sfFuelStock, s_sfElectronicsStock, s_sfFoodStock;
extern std::atomic<uint32_t> s_portFoodStock, s_portFuelStock;
extern std::atomic<float> s_storePriceMultiplier, s_fuelPriceMultiplier;

constexpr size_t k_truckCount = 24;
constexpr size_t k_feederTruckCount = 6;
constexpr size_t k_totalTruckCount = k_truckCount + k_feederTruckCount; // 30 (24 Highway Interstate + 6 Local Feeders)

extern std::atomic<bool> g_running;
extern std::atomic<uint64_t> g_totalTelemetryProcessed;
extern std::atomic<uint64_t> g_totalDirectivesExecuted;
extern std::atomic<float> g_playerX, g_playerY;
extern std::atomic<float> g_playerPosX;
extern std::atomic<float> g_playerPosY;
extern std::atomic<float> g_playerPosZ;
extern std::atomic<bool>  g_isPlayerValid;
extern std::atomic<bool> s_portStrikeActive;
extern std::atomic<bool> s_chipShortageActive;
extern std::atomic<bool> s_federalGrantActive;
extern std::atomic<uint32_t> s_companyStrikeTicks[3];
extern PhysicalTruckFeedback g_physicalFeedback[k_totalTruckCount];
extern TruckSnapshotItem g_truckSnapshots[2][k_totalTruckCount];
extern std::atomic<uint32_t> g_truckSnapshotActiveIdx;
extern ActiveWorldCrisisState s_activeCrisis;
// Retail stores registry (20 stores statewide across 4 districts)
// Accessible remapped store nodes:
// Store #5  (Marina/Market): (569.0f, -1335.0f, 13.0f)
// Store #10 (Ocean Docks):   (2255.0f, -2387.0f, 17.0f)
// Store #14 (Docks/Terminal):(2445.0f, -2547.0f, 17.0f)
extern RetailStoreNode g_retailStores[20];
inline constexpr float k_retailStoreGroundZ[20] = {
    13.3f, // 0: Idlewood 24-7
    13.5f, // 1: Ganton Bodega
    13.3f, // 2: Idlewood Gas
    14.0f, // 3: Willowfield Timber
    23.8f, // 4: East LS AmmuTech
    13.0f, // 5: Rodeo Deli
    35.0f, // 6: Mulholland Market
    13.5f, // 7: Downtown Petrol
    16.8f, // 8: Market Hardware
    13.5f, // 9: Downtown Tech
    17.0f, // 10: Ocean Docks Diner
    2.3f,  // 11: Terminal Fuel
    13.5f, // 12: LSX Aviation Fuel
    2.3f,  // 13: Ocean Docks Timber
    17.0f, // 14: Port Radio Depot
    19.5f, // 15: Montgomery Grocery
    2.1f,  // 16: Flint County Gas
    16.3f, // 17: Dillimore Diesel
    19.0f, // 18: Red County Timber
    2.5f   // 19: Blueberry Depot
};
extern uint32_t s_retailStoreBlips[20];
extern char g_telemetryJsonBuffer[65536];
extern std::atomic<size_t> g_telemetryJsonLength;
extern std::atomic<bool> g_telemetryDirty;

void InitDistrictHousingDefaults();

int CalculateRetailStorePrice(uint32_t storeIndex);
int CalculateRetailStorePrice(uint32_t catId, uint32_t currentStock, uint32_t maxCap, float& outScarcityMult, uint32_t& outRequiredStock, uint32_t districtId = 0);
void RecordRetailStoreDirectPurchase(RetailStoreNode& store, int finalPrice, uint32_t unitsSold);

// =============================================================================
//  Structured Logger
// =============================================================================

class Logger {
public:
    static void Init(const char* filename);
    static void Log(const char* fmt, ...);
    static void Shutdown();
};
