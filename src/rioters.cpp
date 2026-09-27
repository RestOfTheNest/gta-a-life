#include "globals.h"
#include "rioters.h"
#include "plugin.h"
#include "common.h"
#include "CPed.h"
#include "CWorld.h"
#include "CStreaming.h"
#include "CPools.h"
#include "CRadar.h"
#include "CHud.h"
#include "CTimer.h"
#include "extensions/ScriptCommands.h"
#include "enums/ePedType.h"
#include "enums/eWeaponType.h"
#include <cmath>

using namespace plugin;

namespace Rioters {

struct RiotPoint {
    float x, y, z;
};

// All 12 user-defined Civil Riot Coordinates
static const RiotPoint k_riotCoords[12] = {
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
constexpr size_t k_totalRiotPoints = sizeof(k_riotCoords) / sizeof(k_riotCoords[0]);

struct ActiveRiotSite {
    bool     active = false;
    uint32_t blipHandle = 0;
    uint32_t spawnTimeMs = 0;
    uint32_t cooldownUntilMs = 0;
    uint32_t pedHandles[4] = { 0, 0, 0, 0 };
};

static ActiveRiotSite s_sites[k_totalRiotPoints]{};

static CPed* ResolvePed(uint32_t handle) {
    if (handle == 0 || !CPools::ms_pPedPool) return nullptr;
    CPed* p = CPools::ms_pPedPool->GetAtRef(static_cast<int>(handle));
    return (p && CPools::ms_pPedPool->IsObjectValid(p)) ? p : nullptr;
}

static void CleanupSite(size_t idx) {
    if (idx >= k_totalRiotPoints) return;
    auto& s = s_sites[idx];
    if (CPools::ms_pPedPool) {
        for (size_t i = 0; i < 4; ++i) {
            if (s.pedHandles[i] != 0) {
                CPed* p = ResolvePed(s.pedHandles[i]);
                if (p) {
                    if (p->m_pIntelligence) p->m_pIntelligence->m_TaskMgr.FlushImmediately();
                    Command<Commands::CLEAR_CHAR_TASKS_IMMEDIATELY>(p);
                    Command<Commands::DELETE_CHAR>(p);
                }
                s.pedHandles[i] = 0;
            }
        }
    }
    if (s.blipHandle != 0) {
        CRadar::ClearBlip(static_cast<int>(s.blipHandle));
        s.blipHandle = 0;
    }
    s.active = false;
}

void Init() {
    Cleanup();
}

void Cleanup() {
    for (size_t i = 0; i < k_totalRiotPoints; ++i) {
        CleanupSite(i);
        s_sites[i].cooldownUntilMs = 0;
    }
}

void ForceCityHallRiot() {
    s_sites[0].cooldownUntilMs = 0;
    if (s_sites[0].active) CleanupSite(0);
}

void Update(uint32_t currentMs, CPed* player) {
    if (!player) return;
    const float curUnrest = s_socialUnrest.load(std::memory_order_relaxed);

    if (curUnrest < 45.0f && !s_curfewActive.load(std::memory_order_relaxed)) {
        for (size_t i = 0; i < k_totalRiotPoints; ++i) {
            if (s_sites[i].active) CleanupSite(i);
        }
        return;
    }

    const CVector pPos = player->GetPosition();
    size_t activeCount = 0;

    // 1. Maintain active sites
    for (size_t i = 0; i < k_totalRiotPoints; ++i) {
        auto& s = s_sites[i];
        if (!s.active) continue;

        const float dx = pPos.x - k_riotCoords[i].x;
        const float dy = pPos.y - k_riotCoords[i].y;
        const float distSq = dx * dx + dy * dy;

        // Despawn if player walks away > 160m
        if (distSq > (160.0f * 160.0f)) {
            CleanupSite(i);
            s.cooldownUntilMs = currentMs + 15000;
            continue;
        }

        // Check living rioters or TTL timeout (180 seconds)
        size_t living = 0;
        for (size_t p = 0; p < 4; ++p) {
            CPed* ped = ResolvePed(s.pedHandles[p]);
            if (ped && ped->m_fHealth > 0.0f) living++;
        }

        if (living == 0 || (currentMs - s.spawnTimeMs > 180000)) {
            CleanupSite(i);
            s.cooldownUntilMs = currentMs + 180000; // 3 min cooldown
            continue;
        }

        activeCount++;
    }

    // 2. Strict cap: maximum 2 concurrent active civil riots across the entire city
    if (activeCount >= 2 || curUnrest < 55.0f) return;
    if (!CPools::ms_pPedPool || CPools::ms_pPedPool->GetNoOfFreeSpaces() < 12) return;

    for (size_t i = 0; i < k_totalRiotPoints && activeCount < 2; ++i) {
        auto& s = s_sites[i];
        if (s.active || currentMs < s.cooldownUntilMs) continue;

        const float dx = pPos.x - k_riotCoords[i].x;
        const float dy = pPos.y - k_riotCoords[i].y;
        const float distSq = dx * dx + dy * dy;

        // Materialize strictly when player approaches within 45m - 110m
        if (distSq >= (45.0f * 45.0f) && distSq <= (110.0f * 110.0f)) {
            CStreaming::RequestModel(21, PRIORITY_REQUEST | GAME_REQUIRED);
            CStreaming::RequestModel(22, PRIORITY_REQUEST | GAME_REQUIRED);
            Command<Commands::REQUEST_ANIMATION>("RIOT");

            if (!CStreaming::HasModelLoaded(21) || !CStreaming::HasModelLoaded(22)) return;

            // Single area radar blip
            const int blip = CRadar::SetCoordBlip(BLIP_COORD, CVector(k_riotCoords[i].x, k_riotCoords[i].y, k_riotCoords[i].z), 0, BLIP_DISPLAY_BOTH, nullptr);
            if (blip >= 0) {
                CRadar::SetBlipSprite(blip, RADAR_SPRITE_RACE);
                CRadar::ChangeBlipColour(blip, 1); // White flag
                CRadar::ChangeBlipScale(blip, 3);
                s.blipHandle = static_cast<uint32_t>(blip);
            }

            // Spawn 4 agitators in formation
            for (size_t p = 0; p < 4; ++p) {
                const float angle = static_cast<float>(p) * (6.28318f / 4.0f);
                CVector sPos(k_riotCoords[i].x + std::cos(angle) * 3.0f, k_riotCoords[i].y + std::sin(angle) * 3.0f, k_riotCoords[i].z);
                float gz = CWorld::FindGroundZForCoord(sPos.x, sPos.y);
                if (gz > -100.0f) sPos.z = gz + 1.0f;

                CPed* ped = new CPed(PED_TYPE_MISSION1);
                if (!ped || !CPools::ms_pPedPool->IsObjectValid(ped)) continue;

                ped->SetModelIndex((p % 2 == 0) ? 21 : 22);
                ped->SetPosn(sPos);
                ped->m_fHeadingCurrent = (angle * 57.2957795f) + 180.0f;
                ped->m_nCreatedBy = 2;
                ped->bIsVisible = true;
                ped->bStayInSamePlace = true;
                ped->m_fHealth = 150.0f;
                ped->m_fMaxHealth = 150.0f;
                CWorld::Add(ped);

                Command<Commands::SET_CHAR_RELATIONSHIP>(ped, 4, PED_TYPE_COP);
                Command<Commands::TASK_PLAY_ANIM>(ped, (p % 2 == 0) ? "RIOT_ANGRY" : "RIOT_CHANT", "RIOT", 4.0f, true, false, false, false, -1);

                s.pedHandles[p] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(ped));
            }

            s.active = true;
            s.spawnTimeMs = currentMs;
            activeCount++;
        }
    }
}

} // namespace Rioters
