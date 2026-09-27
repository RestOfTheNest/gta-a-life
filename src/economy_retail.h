#pragma once
#include <cstdint>
#include <cstddef>
#include "globals.h"

using RetailStoreState = RetailStoreNode;

extern RetailStoreNode g_retailStores[20];
inline RetailStoreNode (&k_retailStores)[20] = g_retailStores;

namespace EconomyRetail {
    void Init();
    void UpdateRetailStores(uint32_t deltaMs);
    void UpdateStoreRaids(uint32_t currentMs, float unrestPct);

    int CalculateRetailStorePrice(uint32_t storeIndex);
    int CalculateRetailStorePrice(uint32_t catId, uint32_t currentStock, uint32_t maxCap, float &outScarcityMult, uint32_t &outRequiredStock, uint32_t districtId = 0);
    float GetDistrictAverageStorePrice(uint32_t districtId, uint32_t preferredCategory = 0);
    float GetStoreNominalVelocity(uint32_t catId, uint32_t distId);
    void RecordRetailStoreDirectPurchase(RetailStoreNode &store, int finalPrice, uint32_t unitsSold);
}

float GetStoreNominalVelocity(uint32_t catId, uint32_t distId);
