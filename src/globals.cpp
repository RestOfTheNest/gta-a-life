#include "globals.h"
#include <cstdio>
#include <cstdarg>
#include <share.h>

// =============================================================================
//  Global Lock-Free SPSC Queues
// =============================================================================

SPSCQueue<GameCommand, 256> g_webCommandQueue;
SPSCQueue<TelemetryState, 256> g_aiTelemetryQueue;
SPSCQueue<AIDirective, 256> g_aiDirectiveQueue;
SPSCQueue<EconomyEvent, 256> g_economyEventQueue;
SPSCQueue<EconomyEvent, 256> g_shopPurchaseQueue;
SPSCQueue<MunicipalGameEvent, 128> g_municipalGameEventQueue;

// =============================================================================
//  Municipal Government & AI Mayor State
// =============================================================================

std::atomic<int64_t> s_cityTreasury{ 1000000 };
std::atomic<float> s_crimeRate{ 25.0f };
std::atomic<float> s_socialUnrest{ 10.0f };
std::atomic<bool> s_curfewActive{ false };
std::atomic<int> s_emergencyState{ 0 };
std::atomic<bool> s_impeachmentTriggered{ false };
std::atomic<bool> s_fuelCrisisActive{ false };

std::atomic<float> s_taxSalesGeneral{ 0.12f };
std::atomic<float> s_taxSalesLuxury{ 0.25f };
std::atomic<float> s_taxHaulerPort{ 0.15f };
std::atomic<float> s_taxCorporateWealth{ 0.03f };
std::atomic<int32_t> s_taxCitizenPoll{ 800 };
std::atomic<float>   s_districtPollTax[4]{ 150.0f, 500.0f, 250.0f, 180.0f };

FleecaBankState g_fleecaBank;

// =============================================================================
//  A-Life Municipal Incidents Registry
// =============================================================================

ActiveIncident s_incidents[k_maxIncidents]{};

// =============================================================================
//  Logistics Fleet & Economy State
// =============================================================================

std::atomic<int64_t> s_companyBalances[3] = { 150000, 150000, 150000 };
std::atomic<int8_t> s_assetOwners[3] = { -1, -1, -1 };

std::atomic<uint32_t> s_sfTimberStock{ 50 };
std::atomic<uint32_t> s_sfFuelStock{ 40 };
std::atomic<uint32_t> s_sfElectronicsStock{ 30 };
std::atomic<uint32_t> s_sfFoodStock{ 60 };
std::atomic<uint32_t> s_portFoodStock{ 120 };
std::atomic<uint32_t> s_portFuelStock{ 120 };

std::atomic<float> s_storePriceMultiplier{ 1.0f };
std::atomic<float> s_fuelPriceMultiplier{ 1.0f };

// =============================================================================
//  Shared Engine Atomics, Feedback Buffers & Crisis State
// =============================================================================

std::atomic<bool> g_running{ false };
std::atomic<uint64_t> g_totalTelemetryProcessed{ 0 };
std::atomic<uint64_t> g_totalDirectivesExecuted{ 0 };
std::atomic<float> g_playerX{ 0.0f };
std::atomic<float> g_playerY{ 0.0f };
std::atomic<float> g_playerPosX{ 0.0f };
std::atomic<float> g_playerPosY{ 0.0f };
std::atomic<float> g_playerPosZ{ 0.0f };
std::atomic<bool>  g_isPlayerValid{ false };
std::atomic<bool> s_portStrikeActive{ false };
std::atomic<bool> s_chipShortageActive{ false };
std::atomic<bool> s_federalGrantActive{ false };
std::atomic<uint32_t> s_companyStrikeTicks[3] = { 0, 0, 0 };
PhysicalTruckFeedback g_physicalFeedback[k_totalTruckCount];
TruckSnapshotItem g_truckSnapshots[2][k_totalTruckCount]{};
std::atomic<uint32_t> g_truckSnapshotActiveIdx{ 0 };
ActiveWorldCrisisState s_activeCrisis;
uint32_t s_retailStoreBlips[20] = { 0 };

// =============================================================================
//  District AABB Spatial Zoning (Los Santos Master Zoning Map)
// =============================================================================

DistrictAABB g_districtZones[4] = {
    { 0, "South Central / East LS", 1800.0f, 2900.0f, -1850.0f,  -900.0f, 65.0f, 0.8f },
    { 1, "Downtown & West LS",       400.0f, 1800.0f, -1850.0f,  -900.0f, 20.0f, 1.4f },
    { 2, "Industrial Port & Transport", 1000.0f, 2900.0f, -2800.0f, -1850.0f, 45.0f, 1.0f },
    { 3, "County / Highways",          0.0f,    0.0f,     0.0f,     0.0f, 25.0f, 0.9f }
};

DistrictLivingStandards g_districtLivingStandards[4]{};

std::atomic<size_t> g_telemetryJsonLength{ 0 };

void InitDistrictHousingDefaults() {
    // D0: South Central
    g_districtLivingStandards[0].homeownershipRate = 0.30f;
    g_districtLivingStandards[0].baseRent = 480;
    g_districtLivingStandards[0].baseMaintenance = 140;
    g_districtLivingStandards[0].householdSavingsPool = 150000;
    g_districtLivingStandards[0].consecutiveDeficitCycles = 0;

    // D1: Downtown & West LS
    g_districtLivingStandards[1].homeownershipRate = 0.65f;
    g_districtLivingStandards[1].baseRent = 2600;
    g_districtLivingStandards[1].baseMaintenance = 800;
    g_districtLivingStandards[1].householdSavingsPool = 1200000;
    g_districtLivingStandards[1].consecutiveDeficitCycles = 0;

    // D2: Industrial Port & Logistics
    g_districtLivingStandards[2].homeownershipRate = 0.35f;
    g_districtLivingStandards[2].baseRent = 1500;
    g_districtLivingStandards[2].baseMaintenance = 400;
    g_districtLivingStandards[2].householdSavingsPool = 350000;
    g_districtLivingStandards[2].consecutiveDeficitCycles = 0;

    // D3: Country / Rural Highways
    g_districtLivingStandards[3].homeownershipRate = 0.75f;
    g_districtLivingStandards[3].baseRent = 650;
    g_districtLivingStandards[3].baseMaintenance = 120;
    g_districtLivingStandards[3].householdSavingsPool = 220000;
    g_districtLivingStandards[3].consecutiveDeficitCycles = 0;
}

std::atomic<uint32_t> s_playerCurrentDistrict{ 3 };

int GetDistrictByCoords(float x, float y) {
    for (int i = 0; i < 3; ++i) {
        const auto& z = g_districtZones[i];
        if (x >= z.minX && x <= z.maxX && y >= z.minY && y <= z.maxY) {
            return static_cast<int>(z.id);
        }
    }
    return 3;
}

// =============================================================================
//  Structured Logger Implementation
// =============================================================================

static FILE*& LogFile() { static FILE* s = nullptr; return s; }
static std::mutex& LogMtx() { static std::mutex m; return m; }

static void LogGetTimeString(char* buf, size_t size) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    snprintf(buf, size, "%02d:%02d:%02d.%03d",
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

void Logger::Init(const char* filename) {
    std::lock_guard<std::mutex> lk(LogMtx());
    if (!LogFile()) {
        LogFile() = _fsopen(filename, "a", _SH_DENYNO);
    }
    if (LogFile()) {
        char timeBuf[32];
        LogGetTimeString(timeBuf, sizeof(timeBuf));
        fprintf(LogFile(), "[%s] === [gtasystemcore.asi] Engine Initialized ===\n", timeBuf);
        fflush(LogFile());
    }
}

void Logger::Log(const char* fmt, ...) {
    std::lock_guard<std::mutex> lk(LogMtx());
    if (!LogFile()) return;

    char timeBuf[32];
    LogGetTimeString(timeBuf, sizeof(timeBuf));
    fprintf(LogFile(), "[%s] ", timeBuf);

    va_list args;
    va_start(args, fmt);
    vfprintf(LogFile(), fmt, args);
    va_end(args);

    fputc('\n', LogFile());
    fflush(LogFile());
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lk(LogMtx());
    if (LogFile()) {
        char timeBuf[32];
        LogGetTimeString(timeBuf, sizeof(timeBuf));
        fprintf(LogFile(), "[%s] === [gtasystemcore.asi] Clean Shutdown ===\n", timeBuf);
        fclose(LogFile());
        LogFile() = nullptr;
    }
}
