#include "economy_retail.h"
#include "globals.h"
#include "economy.h"
#include "municipal.h"
#include "CTimer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include <atomic>
#include <chrono>

// =============================================================================
//  Closed-Loop Commodity Market Economy: Retail Stores & Wholesale Pricing
// =============================================================================

RetailStoreNode g_retailStores[20] = {
    // District 0 (South Central / East LS): X in [1800, 2900], Y in [-1850, -900]
    {0, 0, 1930.0f, -1770.0f, "Idlewood 24-7", 400, 300, 180, 300, 90, 100, 25000},
    {1, 0, 2245.0f, -1660.0f, "Ganton Bodega", 400, 300, 180, 300, 90, 100, 25000},
    {2, 1, 1944.5f, -1771.2f, "Idlewood Gas", 500, 450, 180, 300, 90, 100, 25000},
    {3, 2, 2320.0f, -1645.0f, "Willowfield Agro Mart", 550, 500, 180, 300, 90, 100, 25000},
    {4, 3, 2400.0f, -1250.0f, "East LS AmmuTech", 1400, 1600, 180, 300, 90, 100, 25000},

    // District 1 (Downtown & West LS): X in [400, 1800], Y in [-1850, -900]
    {5, 0, 569.0f, -1335.0f, "Rodeo Deli", 400, 300, 180, 300, 90, 100, 25000},
    {6, 0, 1315.0f, -905.0f, "Mulholland Market", 400, 300, 180, 300, 90, 100, 25000},
    {7, 1, 1585.0f, -1670.0f, "Downtown Petrol", 500, 450, 180, 300, 90, 100, 25000},
    {8, 2, 1050.0f, -1200.0f, "Market Fresh Produce", 550, 500, 180, 300, 90, 100, 25000},
    {9, 3, 1365.2f, -1279.8f, "Downtown Tech", 1400, 1600, 180, 300, 90, 100, 25000},

    // District 2 (Industrial Port & Transport): X in [1000, 2900], Y in [-2800, -1850]
    {10, 0, 2255.0f, -2387.0f, "Ocean Docks Diner", 400, 300, 180, 300, 90, 100, 25000},
    {11, 1, 2640.0f, -2115.0f, "Terminal Fuel", 500, 450, 180, 300, 90, 100, 25000},
    {12, 1, 1980.0f, -2490.0f, "LSX Aviation Fuel", 500, 450, 180, 300, 90, 100, 25000},
    {13, 2, 2750.0f, -2400.0f, "Ocean Docks Provisions", 550, 500, 180, 300, 90, 100, 25000},
    {14, 3, 2445.0f, -2547.0f, "Port Radio Depot", 1400, 1600, 180, 300, 90, 100, 25000},

    // District 3 (County / Highways): Surrounding outskirts
    {15, 0, 1260.0f, 250.0f, "Montgomery Grocery", 400, 300, 180, 300, 90, 100, 25000},
    {16, 1, -91.3f, -1170.5f, "Flint County Gas", 500, 450, 180, 300, 90, 100, 25000},
    {17, 1, 660.0f, -560.0f, "Dillimore Diesel", 500, 450, 180, 300, 90, 100, 25000},
    {18, 2, 850.0f, -200.0f, "Red County Agro Silo", 550, 500, 180, 300, 90, 100, 25000},
    {19, 3, 200.0f, -240.0f, "Blueberry Depot", 1400, 1600, 180, 300, 90, 100, 25000}
};

static inline uint32_t GetBaseWholesalePrice(uint32_t catId) {
    switch (catId) {
    case 0:  return 25;  // Food ($25.0)
    case 1:  return 35;  // Fuel ($35.0)
    case 2:  return 28;  // Agro/Produce ($28.0)
    case 3:  return 120; // Tech/Weapons ($120.0)
    default: return 30;
    }
}

namespace EconomyRetail {

void Init() {
    for (size_t i = 0; i < 20; ++i) {
        g_retailStores[i].districtId = static_cast<uint32_t>(
            GetDistrictByCoords(g_retailStores[i].posX, g_retailStores[i].posY));
    }
    Logger::Log("[EconomyRetail] Initialized 20 retail store nodes across 4 districts");
}

int CalculateRetailStorePrice(uint32_t storeIndex) {
    if (storeIndex >= 20) storeIndex = 0;
    const auto &store = g_retailStores[storeIndex];
    const uint32_t catId = store.categoryId;
    const uint32_t d = (store.districtId < 4) ? store.districtId : 0;
    const uint32_t localStock = store.localStock.load(std::memory_order_relaxed);
    const uint32_t maxCapacity = (std::max)(1u, store.maxCapacity.load(std::memory_order_relaxed));

    float baseWholesale = 25.0f;
    if (catId == 1)      baseWholesale = 35.0f;
    else if (catId == 2) baseWholesale = 28.0f;
    else if (catId == 3) baseWholesale = 120.0f;

    const float wealthMod = g_districtZones[d].wealthModifier;
    const float districtRentFactor = 1.0f + (wealthMod - 1.0f) * 0.50f;

    float retailMargin = 0.30f;
    if (catId == 1)      retailMargin = 0.20f;
    else if (catId == 3) retailMargin = 0.60f;

    float scarcitySurcharge = 1.0f;
    const float bufferThreshold = 0.25f * static_cast<float>(maxCapacity);
    if (static_cast<float>(localStock) < bufferThreshold && bufferThreshold > 0.001f) {
        scarcitySurcharge = 1.0f + 0.50f * ((bufferThreshold - static_cast<float>(localStock)) / bufferThreshold);
    }

    const float inboundTariff = 1.0f + (s_taxHaulerPort.load(std::memory_order_relaxed) * 0.40f);
    const float salesTax = (catId == 3) ? s_taxSalesLuxury.load(std::memory_order_relaxed)
                                        : s_taxSalesGeneral.load(std::memory_order_relaxed);

    float finalPrice = baseWholesale * districtRentFactor * inboundTariff *
                       (1.0f + retailMargin) * scarcitySurcharge *
                       (1.0f + salesTax);
    finalPrice = std::clamp(finalPrice, 5.0f, 2500.0f);

    return static_cast<int>(std::round(finalPrice));
}

int CalculateRetailStorePrice(uint32_t catId, uint32_t currentStock,
                              uint32_t maxCap, float &outScarcityMult,
                              uint32_t &outRequiredStock, uint32_t districtId) {
    outRequiredStock = (catId == 1 || catId == 3) ? 2 : 1;
    const uint32_t cap = (std::max)(1u, maxCap);
    const uint32_t d = (districtId < 4) ? districtId : 0;

    float baseWholesale = 25.0f;
    if (catId == 1)      baseWholesale = 35.0f;
    else if (catId == 2) baseWholesale = 28.0f;
    else if (catId == 3) baseWholesale = 120.0f;

    const float wealthMod = g_districtZones[d].wealthModifier;
    const float districtRentFactor = 1.0f + (wealthMod - 1.0f) * 0.50f;

    float retailMargin = 0.30f;
    if (catId == 1)      retailMargin = 0.20f;
    else if (catId == 3) retailMargin = 0.60f;

    float scarcitySurcharge = 1.0f;
    const float bufferThreshold = 0.25f * static_cast<float>(cap);
    if (static_cast<float>(currentStock) < bufferThreshold && bufferThreshold > 0.001f) {
        scarcitySurcharge = 1.0f + 0.50f * ((bufferThreshold - static_cast<float>(currentStock)) / bufferThreshold);
    }
    outScarcityMult = scarcitySurcharge;

    const float inboundTariff = 1.0f + (s_taxHaulerPort.load(std::memory_order_relaxed) * 0.40f);
    const float salesTax = (catId == 3) ? s_taxSalesLuxury.load(std::memory_order_relaxed)
                                        : s_taxSalesGeneral.load(std::memory_order_relaxed);

    float finalPrice = baseWholesale * districtRentFactor * inboundTariff *
                       (1.0f + retailMargin) * scarcitySurcharge *
                       (1.0f + salesTax);
    finalPrice = std::clamp(finalPrice, 5.0f, 2500.0f);

    return static_cast<int>(std::round(finalPrice));
}

float GetDistrictAverageStorePrice(uint32_t districtId, uint32_t preferredCategory) {
    float totalPrice = 0.0f;
    int count = 0;
    for (size_t i = 0; i < 20; ++i) {
        if (g_retailStores[i].districtId == districtId) {
            bool matches = false;
            if (preferredCategory == 0) {
                matches = (g_retailStores[i].categoryId == 0 || g_retailStores[i].categoryId == 2);
            } else {
                matches = (g_retailStores[i].categoryId == preferredCategory);
            }
            if (matches) {
                int p = CalculateRetailStorePrice(static_cast<uint32_t>(i));
                totalPrice += static_cast<float>(p);
                count++;
            }
        }
    }
    if (count == 0) return 60.0f;
    return totalPrice / static_cast<float>(count);
}

void RecordRetailStoreDirectPurchase(RetailStoreNode &store, int finalPrice, uint32_t unitsSold) {
    const int64_t storeRevenue = static_cast<int64_t>(static_cast<float>(finalPrice) * 0.85f);
    const int64_t tax = static_cast<int64_t>(finalPrice) - storeRevenue;
    store.capitalBalance.fetch_add(storeRevenue, std::memory_order_relaxed);
    s_cityTreasury.fetch_add(tax, std::memory_order_relaxed);
    store.totalTaxesPaid.fetch_add(static_cast<uint64_t>(tax), std::memory_order_relaxed);
    store.totalUnitsSold.fetch_add(unitsSold, std::memory_order_relaxed);
    store.lifetimeRevenue.fetch_add(static_cast<uint64_t>(finalPrice), std::memory_order_relaxed);
}

float GetStoreNominalVelocity(uint32_t catId, uint32_t distId) {
    float baseVelocity = 6.5f; // Cat 0 (Food)
    if (catId == 1)      baseVelocity = 5.5f; // Cat 1 (Fuel)
    else if (catId == 2) baseVelocity = 4.5f; // Cat 2 (Agro/Provisions)
    else if (catId == 3) baseVelocity = 2.0f; // Cat 3 (Tech/Ammo)

    float distMultiplier = 1.0f;
    if (distId == 0) { // South Central / East LS
        if (catId == 0)      distMultiplier = 1.8f;
        else if (catId == 1) distMultiplier = 0.6f;
        else if (catId == 2) distMultiplier = 0.7f;
        else if (catId == 3) distMultiplier = 1.6f;
    } else if (distId == 1) { // Downtown & West LS
        if (catId == 0)      distMultiplier = 1.0f;
        else if (catId == 1) distMultiplier = 1.6f;
        else if (catId == 2) distMultiplier = 0.4f;
        else if (catId == 3) distMultiplier = 1.8f;
    } else if (distId == 2) { // Industrial Port
        if (catId == 0)      distMultiplier = 0.8f;
        else if (catId == 1) distMultiplier = 1.7f;
        else if (catId == 2) distMultiplier = 2.2f;
        else if (catId == 3) distMultiplier = 0.7f;
    } else if (distId == 3) { // County / Highways
        if (catId == 0)      distMultiplier = 1.0f;
        else if (catId == 1) distMultiplier = 2.0f;
        else if (catId == 2) distMultiplier = 1.3f;
        else if (catId == 3) distMultiplier = 0.5f;
    }
    return baseVelocity * distMultiplier;
}

void UpdateRetailStores(uint32_t deltaMs) {
    const float dt = (deltaMs > 0) ? (static_cast<float>(deltaMs) / 1000.0f) : 0.05f;
    const uint32_t playerDist = s_playerCurrentDistrict.load(std::memory_order_relaxed);

    for (uint32_t i = 0; i < 20; ++i) {
        auto &store = g_retailStores[i];
        if (store.isRansacked.load(std::memory_order_relaxed)) {
            store.salesVelocityPerMin.store(0.0f, std::memory_order_relaxed);
            continue;
        }
        const uint32_t catId = store.categoryId;
        const uint32_t distId = store.districtId;

        float calculatedVelocity = GetStoreNominalVelocity(catId, distId);
        if (distId == playerDist) {
            calculatedVelocity *= 1.35f;
        }

        uint32_t currentStock = store.localStock.load(std::memory_order_relaxed);
        uint32_t maxCap = store.maxCapacity.load(std::memory_order_relaxed);

        if (maxCap > 0 && currentStock < static_cast<uint32_t>(static_cast<float>(maxCap) * 0.25f)) {
            float scarcityRatio = static_cast<float>(currentStock) / (static_cast<float>(maxCap) * 0.25f);
            calculatedVelocity *= std::clamp(scarcityRatio, 0.15f, 1.0f);
        }

        store.salesVelocityPerMin.store(calculatedVelocity, std::memory_order_relaxed);

        float unitsSoldDelta = (calculatedVelocity / 60.0f) * dt;
        float currentAccum = store.fractionalSalesAccum.load(std::memory_order_relaxed) + unitsSoldDelta;
        uint32_t wholeUnits = static_cast<uint32_t>(currentAccum);
        if (wholeUnits > 0) {
            currentAccum -= static_cast<float>(wholeUnits);
            store.fractionalSalesAccum.store(currentAccum, std::memory_order_relaxed);

            uint32_t drain = (std::min)(wholeUnits, currentStock);
            if (drain > 0) {
                store.localStock.fetch_sub(drain, std::memory_order_relaxed);
                currentStock -= drain;

                const float mult = (catId == 1) ? s_fuelPriceMultiplier.load(std::memory_order_relaxed)
                                                : s_storePriceMultiplier.load(std::memory_order_relaxed);
                const float taxRate = (catId == 3) ? s_taxSalesLuxury.load(std::memory_order_relaxed)
                                                   : s_taxSalesGeneral.load(std::memory_order_relaxed);
                const float storeMargin = (store.id == 15) ? (1.4f * 1.20f) : 1.4f;
                const uint64_t gross = static_cast<uint64_t>(drain * GetBaseWholesalePrice(catId) * storeMargin * mult);
                const int64_t tax = static_cast<int64_t>(static_cast<float>(gross) * taxRate);

                s_cityTreasury.fetch_add(tax, std::memory_order_relaxed);
                store.totalTaxesPaid.fetch_add(static_cast<uint64_t>(tax), std::memory_order_relaxed);
                store.capitalBalance.fetch_add(static_cast<int64_t>(gross) - tax, std::memory_order_relaxed);
                store.totalUnitsSold.fetch_add(drain, std::memory_order_relaxed);
                store.lifetimeRevenue.fetch_add(gross, std::memory_order_relaxed);
                store.windowUnitsSold.fetch_add(drain, std::memory_order_relaxed);
            }
        } else {
            store.fractionalSalesAccum.store(currentAccum, std::memory_order_relaxed);
        }

        // Wholesale Restock: If localStock <= minThreshold and !isBankrupt:
        if (s_socialUnrest.load(std::memory_order_relaxed) < 60.0f &&
            !store.isUnderRaid.load(std::memory_order_relaxed) &&
            !store.isBankrupt.load(std::memory_order_relaxed) &&
            currentStock <= store.minThreshold.load(std::memory_order_relaxed)) {
            const int64_t curBal = store.capitalBalance.load(std::memory_order_relaxed);
            if (curBal > 2000) {
                const int64_t maxBudget = (curBal > 5000) ? (curBal - 5000) : (curBal - 1000);
                const float mult = (catId == 1) ? s_fuelPriceMultiplier.load(std::memory_order_relaxed)
                                                : s_storePriceMultiplier.load(std::memory_order_relaxed);
                const uint32_t unitWholesale = static_cast<uint32_t>((std::max)(1.0f, GetBaseWholesalePrice(catId) * mult));
                const uint32_t affordableUnits = static_cast<uint32_t>(maxBudget / unitWholesale);

                if (affordableUnits > 0) {
                    const uint32_t batchSize = store.batchOrderSize.load(std::memory_order_relaxed);
                    const uint32_t orderQty = (std::min)(batchSize, affordableUnits);

                    std::atomic<uint32_t> *pCentral = nullptr;
                    if (catId == 0 || catId == 2)      pCentral = &s_sfFoodStock;
                    else if (catId == 1)               pCentral = &s_sfFuelStock;
                    else if (catId == 3)               pCentral = &s_sfElectronicsStock;

                    if (pCentral) {
                        uint32_t currentCentral = pCentral->load(std::memory_order_relaxed);
                        uint32_t acquired = 0;
                        while (true) {
                            uint32_t available = (catId == 1) ? (currentCentral > 15 ? currentCentral - 15 : 0) : currentCentral;
                            if (available == 0) break;
                            acquired = (std::min)(available, orderQty);
                            if (pCentral->compare_exchange_weak(currentCentral, currentCentral - acquired, std::memory_order_relaxed)) {
                                break;
                            }
                        }

                        if (acquired > 0) {
                            const int64_t wholesaleCost = static_cast<int64_t>(acquired * unitWholesale);
                            store.capitalBalance.fetch_sub(wholesaleCost, std::memory_order_relaxed);
                            store.localStock.fetch_add(acquired, std::memory_order_relaxed);
                            if (store.localStock.load(std::memory_order_relaxed) >= 20) {
                                store.isRansacked.store(false, std::memory_order_relaxed);
                            }
                        }
                    }
                }
            }
        }
    }
}

void UpdateStoreRaids(uint32_t currentMs, float unrestPct) {
    static uint32_t s_lastRaidSimMs = 0;
    const uint32_t nowMs = (currentMs != 0) ? currentMs
        : (CTimer::m_snTimeInMilliseconds != 0 ? CTimer::m_snTimeInMilliseconds
           : static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::steady_clock::now().time_since_epoch()).count()));

    if (nowMs - s_lastRaidSimMs < 1000) {
        return;
    }
    s_lastRaidSimMs = nowMs;

    const float unrest = (unrestPct >= 0.0f) ? unrestPct : s_socialUnrest.load(std::memory_order_relaxed);
    const float crime = s_crimeRate.load(std::memory_order_relaxed);
    const float pX = g_isPlayerValid.load(std::memory_order_relaxed)
                         ? g_playerPosX.load(std::memory_order_relaxed)
                         : g_playerX.load(std::memory_order_relaxed);
    const float pY = g_isPlayerValid.load(std::memory_order_relaxed)
                         ? g_playerPosY.load(std::memory_order_relaxed)
                         : g_playerY.load(std::memory_order_relaxed);

    // 1. If social unrest drops below 40%, abort all active raids safely
    if (unrest < 40.0f) {
        for (size_t i = 0; i < 20; ++i) {
            if (g_retailStores[i].isUnderRaid.load(std::memory_order_relaxed)) {
                g_retailStores[i].isUnderRaid.store(false, std::memory_order_relaxed);
                g_retailStores[i].raidStartMs.store(0, std::memory_order_relaxed);
                g_retailStores[i].lastDrainMs.store(0, std::memory_order_relaxed);
                g_retailStores[i].stolenStock.store(0, std::memory_order_relaxed);
                Logger::Log("[Economy] Store Raid on %s aborted: Social unrest pacified below 40%%",
                            g_retailStores[i].name);
            }
        }
        return;
    }

    // 2. Count active raids across the state (Global Cap: Maximum 2 concurrent raids)
    uint32_t activeRaids = 0;
    for (size_t i = 0; i < 20; ++i) {
        if (g_retailStores[i].isUnderRaid.load(std::memory_order_relaxed)) {
            activeRaids++;
        }
    }

    // 3. Roll chance to trigger new raids if high unrest (> 60%) and high crime (> 50%)
    static uint32_t s_lastRaidRollMs = 0;
    if (unrest > 60.0f && crime > 50.0f && activeRaids < 2 && (nowMs - s_lastRaidRollMs >= 12000)) {
        s_lastRaidRollMs = nowMs;
        const int raidChance = (std::min)(65, static_cast<int>(20.0f + (unrest - 60.0f) * 1.5f + (crime - 50.0f) * 0.8f));
        if ((rand() % 100) < raidChance) {
            std::vector<size_t> eligible;
            eligible.reserve(20);
            for (size_t i = 0; i < 20; ++i) {
                if (!g_retailStores[i].isUnderRaid.load(std::memory_order_relaxed) &&
                    !g_retailStores[i].isBankrupt.load(std::memory_order_relaxed) &&
                    !g_retailStores[i].isRansacked.load(std::memory_order_relaxed) &&
                    nowMs >= g_retailStores[i].raidCooldownUntilMs.load(std::memory_order_relaxed) &&
                    g_retailStores[i].localStock.load(std::memory_order_relaxed) > 30) {
                    eligible.push_back(i);
                }
            }
            if (!eligible.empty()) {
                size_t chosen = eligible[rand() % eligible.size()];
                for (size_t idx : eligible) {
                    if (g_retailStores[idx].districtId == 0 && (rand() % 100) < 60) {
                        chosen = idx;
                        break;
                    }
                }
                auto &targetStore = g_retailStores[chosen];
                if ((rand() % 100) < 50) {
                    Logger::Log("[Economy] Police patrol thwarted raid attempt at %s", targetStore.name);
                } else {
                    targetStore.isUnderRaid.store(true, std::memory_order_relaxed);
                    targetStore.raidStartMs.store(nowMs, std::memory_order_relaxed);
                    targetStore.lastDrainMs.store(nowMs, std::memory_order_relaxed);
                    targetStore.stolenStock.store(0, std::memory_order_relaxed);
                    activeRaids++;
                    Logger::Log("[Economy] RAID_IN_PROGRESS: Store #%u (%s) in District %u under attack! (Active raids: %u/2)",
                                targetStore.id, targetStore.name, targetStore.districtId, activeRaids);
                }
            }
        }
    }

    // 4. Background mathematical simulation of active raids (> 80.0m zero physical entities)
    for (size_t i = 0; i < 20; ++i) {
        auto &store = g_retailStores[i];
        if (!store.isUnderRaid.load(std::memory_order_relaxed)) continue;

        const uint32_t startMs = store.raidStartMs.load(std::memory_order_relaxed);
        const uint32_t lastDrain = store.lastDrainMs.load(std::memory_order_relaxed);
        const float dx = store.posX - pX;
        const float dy = store.posY - pY;
        const float distToPlayer = std::sqrt(dx * dx + dy * dy);

        if (nowMs - lastDrain >= 15000) {
            store.lastDrainMs.store(nowMs, std::memory_order_relaxed);
            const uint32_t curStock = store.localStock.load(std::memory_order_relaxed);
            const uint32_t drain = (std::min)(curStock, 15u);
            if (drain > 0) {
                store.localStock.fetch_sub(drain, std::memory_order_relaxed);
                store.stolenStock.fetch_add(drain, std::memory_order_relaxed);
                if (store.localStock.load(std::memory_order_relaxed) == 0) {
                    store.isRansacked.store(true, std::memory_order_relaxed);
                }
            }
            s_cityTreasury.fetch_sub(350, std::memory_order_relaxed);
            Logger::Log("[Economy] Store Raid Drain: %s lost %u stock (total stolen: %u), city treasury drained -$350",
                        store.name, drain, store.stolenStock.load(std::memory_order_relaxed));
        }

        if (nowMs - startMs >= 90000) {
            if (distToPlayer > 80.0f) {
                s_cityTreasury.fetch_sub(2500, std::memory_order_relaxed);
                store.localStock.store(0, std::memory_order_relaxed);
                store.isRansacked.store(true, std::memory_order_relaxed);
                store.raidCooldownUntilMs.store(nowMs + 600000, std::memory_order_relaxed);

                store.isUnderRaid.store(false, std::memory_order_relaxed);
                store.raidStartMs.store(0, std::memory_order_relaxed);
                store.lastDrainMs.store(0, std::memory_order_relaxed);
                store.stolenStock.store(0, std::memory_order_relaxed);

                Logger::Log("[Economy] Store %s was RANSACKED! (Stock drained to 0, 10-min cooldown, -$2500 treasury)",
                            store.name);
            }
        }
    }
}

} // namespace EconomyRetail

// =============================================================================
//  Global Scope Compatibility Wrappers
// =============================================================================

int CalculateRetailStorePrice(uint32_t storeIndex) {
    return EconomyRetail::CalculateRetailStorePrice(storeIndex);
}

int CalculateRetailStorePrice(uint32_t catId, uint32_t currentStock,
                              uint32_t maxCap, float &outScarcityMult,
                              uint32_t &outRequiredStock, uint32_t districtId) {
    return EconomyRetail::CalculateRetailStorePrice(catId, currentStock, maxCap, outScarcityMult, outRequiredStock, districtId);
}

float GetDistrictAverageStorePrice(uint32_t districtId, uint32_t preferredCategory) {
    return EconomyRetail::GetDistrictAverageStorePrice(districtId, preferredCategory);
}

void RecordRetailStoreDirectPurchase(RetailStoreNode &store, int finalPrice, uint32_t unitsSold) {
    EconomyRetail::RecordRetailStoreDirectPurchase(store, finalPrice, unitsSold);
}

void UpdateStoreRaidsSimulation() {
    EconomyRetail::UpdateStoreRaids(CTimer::m_snTimeInMilliseconds, s_socialUnrest.load(std::memory_order_relaxed));
}

float GetStoreNominalVelocity(uint32_t catId, uint32_t distId) {
    return EconomyRetail::GetStoreNominalVelocity(catId, distId);
}
