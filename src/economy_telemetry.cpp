#include "economy_telemetry.h"
#include "globals.h"
#include "economy.h"
#include "municipal.h"

#include <cstdio>
#include <cstring>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>

char g_telemetryJsonBuffer[65536] = {0};
std::atomic<bool> g_telemetryDirty{false};

namespace EconomyTelemetry {

static std::thread s_telemetryWorkerThread;
static std::atomic<bool> s_workerRunning{false};
static std::mutex s_serializationMutex;

void SerializeTelemetryJson() {
    std::lock_guard<std::mutex> lock(s_serializationMutex);

    int raidedStoreId = -1;
    const char *raidedStoreName = "None";
    float rx = 0.0f, ry = 0.0f, rz = 0.0f;
    uint32_t rStock = 0;
    bool rUnderRaid = false;
    for (size_t s = 0; s < 20; ++s) {
        if (g_retailStores[s].isUnderRaid.load(std::memory_order_relaxed)) {
            raidedStoreId = static_cast<int>(s);
            raidedStoreName = g_retailStores[s].name;
            rx = g_retailStores[s].posX;
            ry = g_retailStores[s].posY;
            rz = k_retailStoreGroundZ[s];
            rStock = g_retailStores[s].localStock.load(std::memory_order_relaxed);
            rUnderRaid = true;
            break;
        }
    }

    char *ptr = g_telemetryJsonBuffer;
    size_t remaining = sizeof(g_telemetryJsonBuffer);

    int written = snprintf(
        ptr, remaining,
        "{\"raidedStoreId\":%d,\"raidedStoreName\":\"%s\",\"raidedStoreCoords\":{\"x\":%.1f,\"y\":%.1f,\"z\":%.1f},"
        "\"localStock\":%u,\"isUnderRaid\":%s,\"stores\":[",
        raidedStoreId, raidedStoreName, rx, ry, rz, rStock,
        rUnderRaid ? "true" : "false");
    if (written > 0 && static_cast<size_t>(written) < remaining) {
        ptr += written;
        remaining -= written;
    }

    for (size_t i = 0; i < 20; ++i) {
        written = snprintf(
            ptr, remaining,
            "%s{\"id\":%u,\"cat\":%u,\"dist\":%u,\"x\":%.1f,\"y\":%.1f,\"rate\":%."
            "1f,\"name\":\"%s\",\"stock\":%u,\"cap\":%u,\"bal\":%lld,\"sold\":%llu,"
            "\"rev\":%llu,\"tax\":%llu,\"debt\":%lld,\"score\":%d,\"bankrupt\":%s,"
            "\"isUnderRaid\":%s,\"isRansacked\":%s,"
            "\"thresh\":%u,\"batch\":%u,\"win\":%u}",
            (i > 0 ? "," : ""), g_retailStores[i].id, g_retailStores[i].categoryId,
            g_retailStores[i].districtId, g_retailStores[i].posX,
            g_retailStores[i].posY,
            g_retailStores[i].salesVelocityPerMin.load(std::memory_order_relaxed),
            g_retailStores[i].name,
            g_retailStores[i].localStock.load(std::memory_order_relaxed),
            g_retailStores[i].maxCapacity.load(std::memory_order_relaxed),
            static_cast<long long>(
                g_retailStores[i].capitalBalance.load(std::memory_order_relaxed)),
            static_cast<unsigned long long>(
                g_retailStores[i].totalUnitsSold.load(std::memory_order_relaxed)),
            static_cast<unsigned long long>(
                g_retailStores[i].lifetimeRevenue.load(std::memory_order_relaxed)),
            static_cast<unsigned long long>(
                g_retailStores[i].totalTaxesPaid.load(std::memory_order_relaxed)),
            static_cast<long long>(
                g_retailStores[i].loanDebt.load(std::memory_order_relaxed)),
            static_cast<int>(
                g_retailStores[i].creditScore.load(std::memory_order_relaxed)),
            g_retailStores[i].isBankrupt.load(std::memory_order_relaxed) ? "true"
                                                                         : "false",
            g_retailStores[i].isUnderRaid.load(std::memory_order_relaxed) ? "true"
                                                                          : "false",
            g_retailStores[i].isRansacked.load(std::memory_order_relaxed) ? "true"
                                                                          : "false",
            g_retailStores[i].minThreshold.load(std::memory_order_relaxed),
            g_retailStores[i].batchOrderSize.load(std::memory_order_relaxed),
            g_retailStores[i].windowUnitsSold.load(std::memory_order_relaxed));
        if (written > 0 && static_cast<size_t>(written) < remaining) {
            ptr += written;
            remaining -= written;
        }
    }

    uint32_t distStock[4] = {0, 0, 0, 0};
    uint32_t distCap[4] = {0, 0, 0, 0};
    float distVelocity[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    for (size_t i = 0; i < 20; ++i) {
        const uint32_t d = g_retailStores[i].districtId;
        if (d < 4) {
            distVelocity[d] +=
                g_retailStores[i].salesVelocityPerMin.load(std::memory_order_relaxed);
            if (g_retailStores[i].categoryId == 0 ||
                g_retailStores[i].categoryId == 1) {
                distStock[d] +=
                    g_retailStores[i].localStock.load(std::memory_order_relaxed);
                distCap[d] +=
                    g_retailStores[i].maxCapacity.load(std::memory_order_relaxed);
            }
        }
    }

    const float curCrime = s_crimeRate.load(std::memory_order_relaxed);
    const float curUnrest = s_socialUnrest.load(std::memory_order_relaxed);
    const bool curCurfew = s_curfewActive.load(std::memory_order_relaxed);

    written = snprintf(
        ptr, remaining,
        "],\"districts\":["
        "{\"id\":0,\"name\":\"South "
        "Central\",\"spec\":\"%s\",\"stock\":%u,\"cap\":%u,\"rate\":%.1f},"
        "{\"id\":1,\"name\":\"Downtown\",\"spec\":\"%s\",\"stock\":%u,\"cap\":%u,"
        "\"rate\":%.1f},"
        "{\"id\":2,\"name\":\"Industrial "
        "Port\",\"spec\":\"%s\",\"stock\":%u,\"cap\":%u,\"rate\":%.1f},"
        "{\"id\":3,\"name\":\"Country / "
        "Hwy\",\"spec\":\"%s\",\"stock\":%u,\"cap\":%u,\"rate\":%.1f}"
        "],\"macro\":{\"crime\":%.1f,\"unrest\":%.1f,\"curfew\":%s,"
        "\"playerDist\":%u,\"taxSalesGen\":%.3f,\"taxSalesLux\":%.3f},"
        "\"warehouses\":{\"fuel\":%u,\"timber\":%u,\"elec\":%u,\"food\":%u},"
        "\"port\":{\"food\":%u,\"fuel\":%u},"
        "\"companies\":[{\"bal\":%lld},{\"bal\":%lld},{\"bal\":%lld}],"
        "\"bank\":{\"reserves\":%lld,\"rate\":%.3f,\"bailouts\":%u,\"bankrupts\":"
        "%u},"
        "\"districtEcon\":[",
        GetDistrictSpecialization(0), distStock[0], distCap[0], distVelocity[0],
        GetDistrictSpecialization(1), distStock[1], distCap[1], distVelocity[1],
        GetDistrictSpecialization(2), distStock[2], distCap[2], distVelocity[2],
        GetDistrictSpecialization(3), distStock[3], distCap[3], distVelocity[3],
        curCrime, curUnrest, curCurfew ? "true" : "false",
        s_playerCurrentDistrict.load(std::memory_order_relaxed),
        s_taxSalesGeneral.load(std::memory_order_relaxed),
        s_taxSalesLuxury.load(std::memory_order_relaxed),
        s_sfFuelStock.load(std::memory_order_relaxed),
        s_sfTimberStock.load(std::memory_order_relaxed),
        s_sfElectronicsStock.load(std::memory_order_relaxed),
        s_sfFoodStock.load(std::memory_order_relaxed),
        s_portFoodStock.load(std::memory_order_relaxed),
        s_portFuelStock.load(std::memory_order_relaxed),
        static_cast<long long>(
            s_companyBalances[0].load(std::memory_order_relaxed)),
        static_cast<long long>(
            s_companyBalances[1].load(std::memory_order_relaxed)),
        static_cast<long long>(
            s_companyBalances[2].load(std::memory_order_relaxed)),
        static_cast<long long>(
            g_fleecaBank.totalReserves.load(std::memory_order_relaxed)),
        g_fleecaBank.dynamicInterestRate.load(std::memory_order_relaxed),
        g_fleecaBank.activeBailoutsCount.load(std::memory_order_relaxed),
        g_fleecaBank.bankruptStoresCount.load(std::memory_order_relaxed));
    if (written > 0 && static_cast<size_t>(written) < remaining) {
        ptr += written;
        remaining -= written;
    }

    static const char *const k_dNames[4] = {"South Central", "Downtown",
                                            "Industrial Port", "Country / Rural"};
    static const float k_dWages[4] = {1100.0f, 4200.0f, 2400.0f, 1400.0f};

    for (uint32_t d = 0; d < 4; ++d) {
        const float wage = (g_districtLivingStandards[d].baseWage > 0.0f)
                               ? g_districtLivingStandards[d].baseWage
                               : k_dWages[d];
        const float effWage = (g_districtLivingStandards[d].effectiveWage > 0.0f)
                                  ? g_districtLivingStandards[d].effectiveWage
                                  : wage;
        const char *name = g_districtLivingStandards[d].name[0]
                               ? g_districtLivingStandards[d].name
                               : k_dNames[d];
        const char *status = g_districtLivingStandards[d].status[0]
                                 ? g_districtLivingStandards[d].status
                                 : "STRAINED";
        written =
            snprintf(ptr, remaining,
                     "%s{\"id\":%u,\"name\":\"%s\",\"wage\":%.1f,\"effWage\":%.1f,"
                     "\"bonus\":%.1f,\"housing\":%.1f,\"crime\":%.1f,\"basket\":%."
                     "1f,\"tax\":%.1f,\"pollTax\":%.1f,\"netRatio\":%.3f,"
                     "\"status\":\"%s\",\"austerity\":%s}",
                     (d > 0 ? "," : ""), d, name, wage, effWage,
                     g_districtLivingStandards[d].dynamicBonus,
                     g_districtLivingStandards[d].fixedLivingCost,
                     g_districtLivingStandards[d].crimeSurcharge,
                     g_districtLivingStandards[d].basketCost,
                     g_districtLivingStandards[d].pollTax,
                     s_districtPollTax[d].load(std::memory_order_relaxed),
                     g_districtLivingStandards[d].netRatio, status,
                     g_districtLivingStandards[d].austerity ? "true" : "false");
        if (written > 0 && static_cast<size_t>(written) < remaining) {
            ptr += written;
            remaining -= written;
        }
    }

    written = snprintf(ptr, remaining, "]");
    if (written > 0 && static_cast<size_t>(written) < remaining) {
        ptr += written;
        remaining -= written;
    }

    written = snprintf(ptr, remaining, ",\"roadblocks\":[");
    if (written > 0 && static_cast<size_t>(written) < remaining) { ptr += written; remaining -= written; }

    const uint64_t rbMask = s_activeRoadblockMask.load(std::memory_order_acquire);
    bool firstRb = true;
    for (size_t i = 0; i < k_numArterialChokepoints; ++i) {
        const bool isActive = (rbMask & (1ULL << i)) != 0;
        const auto cpStat = s_arterialChokepoints[i].status.load(std::memory_order_relaxed);
        if (!isActive && cpStat == ChokepointStatus::CLEAR) {
            continue;
        }
        const char* statusStr = (cpStat == ChokepointStatus::CLEARING_BLUE_BLINK) ? "CLEARING" : "BLOCKED";
        uint32_t riotersAlive = 0;
        bool provoked = false;
        GetRoadblockSlotDetails(i, riotersAlive, provoked);

        const uint32_t distId = static_cast<uint32_t>(GetDistrictByCoords(k_chokepointsTable[i].x, k_chokepointsTable[i].y));
        const char* distName = (distId < 4) ? g_districtLivingStandards[distId].name : "San Andreas";
        char locBuf[96];
        snprintf(locBuf, sizeof(locBuf), "%s Arterial #%u", distName, static_cast<unsigned int>(i));

        written = snprintf(ptr, remaining,
            "%s{\"id\":%u,\"x\":%.1f,\"y\":%.1f,\"z\":%.1f,\"active\":true,\"status\":\"%s\",\"location\":\"%s\",\"riotersAlive\":%u,\"provoked\":%s}",
            (firstRb ? "" : ","),
            static_cast<unsigned int>(i),
            k_chokepointsTable[i].x,
            k_chokepointsTable[i].y,
            k_chokepointsTable[i].z,
            statusStr,
            locBuf,
            riotersAlive,
            provoked ? "true" : "false"
        );
        firstRb = false;
        if (written > 0 && static_cast<size_t>(written) < remaining) { ptr += written; remaining -= written; }
    }
    written = snprintf(ptr, remaining, "]");
    if (written > 0 && static_cast<size_t>(written) < remaining) { ptr += written; remaining -= written; }

    written = snprintf(ptr, remaining, "}");
    if (written > 0 && static_cast<size_t>(written) < remaining) {
        ptr += written;
        remaining -= written;
    }

    g_telemetryDirty.store(true, std::memory_order_release);
    g_telemetryJsonLength.store(static_cast<size_t>(ptr - g_telemetryJsonBuffer),
                                std::memory_order_release);
}

static void TelemetrySerializerLoop() {
    while (s_workerRunning.load(std::memory_order_relaxed) && g_running.load(std::memory_order_relaxed)) {
        SerializeTelemetryJson();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void StartTelemetryWorker() {
    if (!s_workerRunning.exchange(true)) {
        s_telemetryWorkerThread = std::thread(TelemetrySerializerLoop);
    }
}

void StopTelemetryWorker() {
    if (s_workerRunning.exchange(false)) {
        if (s_telemetryWorkerThread.joinable()) {
            s_telemetryWorkerThread.join();
        }
    }
}

} // namespace EconomyTelemetry
