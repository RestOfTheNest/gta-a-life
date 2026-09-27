#pragma once

#include "globals.h"
#include "economy_retail.h"
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <atomic>

class CPed;
class CAutomobile;

// =============================================================================
//  Accurate Highway Waypoints & Logistics Fleet Specifications
// =============================================================================

extern const HighwayWaypoint s_highwayLoop[];
extern const size_t s_waypointCount;
extern const HighwayWaypoint* const k_highwayWaypoints;

constexpr int MODEL_LINERUNNER = 514;
constexpr int MODEL_ROADTRAIN  = 515;
constexpr int MODEL_TANKER     = 403;
extern const int k_truckModels[3];

extern const LogisticsCompany k_companies[3];
extern const LogisticsAsset k_assets[3];
extern const char* const k_driverNames[30];

extern const CVector k_gasStationCoords[3];
extern const CVector k_shopCoords[2];
extern VirtualTruck s_fleet[k_totalTruckCount];

// =============================================================================
//  Economy, Logistics & MoonLoader Functions
// =============================================================================

using RetailStore = RetailStoreNode;

void AIDirectorLoop();
void UpdateStoreRaidsSimulation();
void ExportMoonLoaderJsonState();
void ProcessMoonLoaderRequests(CPed* player);
void EnsureMunicipalConfigDir();
void PullTruckToRoadside(CAutomobile* veh, float offsetDist = 3.5f);
float GetDistrictAverageStorePrice(uint32_t districtId, uint32_t preferredCategory = 0);
int CalculateRetailStorePrice(uint32_t storeIndex);
int CalculateRetailStorePrice(uint32_t catId, uint32_t currentStock, uint32_t maxCap, float& outScarcityMult, uint32_t& outRequiredStock, uint32_t districtId);

// Physical Trucks Lifecycle & Directives Management
void ProcessTruckDirectives();
void UpdatePhysicalTrucks(CPed* player);
void CleanupAllPhysicalTrucks();
bool IsAreaClearOfVehicles(const CVector& pos, float radius);

// =============================================================================
//  Custom Logistics Routes Management
// =============================================================================

extern std::recursive_mutex g_customRoutesMutex;
extern CompanyCustomRoute g_customRoutes[3];
inline CompanyCustomRoute (&g_companyRoutes)[3] = g_customRoutes;
inline CompanyCustomRoute (&s_feederRoutes)[3] = g_customRoutes;

void InitCustomRoutes();
bool LoadCustomRoutesJson();
bool SaveCustomRoutesToFile();
bool SaveSingleCustomRoute(uint8_t companyId, const char* color, const std::vector<CustomRouteNode>& nodes);
std::string ExportCustomRoutesJson();
bool ParseRouteJsonPayload(const char* body, uint8_t& outCompanyId, std::string& outColor, std::vector<CustomRouteNode>& outNodes);

