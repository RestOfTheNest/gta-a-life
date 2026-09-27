#include "globals.h"
#include "gang_wars.h"
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

using namespace plugin;

namespace GangWars {

struct GangPoint {
    float x, y, z;
};

// All 9 user-defined Gang War Coordinates
static const GangPoint k_gangCoords[9] = {
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
constexpr size_t k_totalGangPoints = sizeof(k_gangCoords) / sizeof(k_gangCoords[0]);

struct ActiveGangSite {
    bool     active = false;
    uint32_t blipHandle = 0;
    uint32_t spawnTimeMs = 0;
    uint32_t cooldownUntilMs = 0;
    uint32_t ballasHandles[2] = { 0, 0 };
    uint32_t groveHandles[2] = { 0, 0 };
};

static ActiveGangSite s_gangSites[k_totalGangPoints]{};

static CPed* ResolvePed(uint32_t handle) {
    if (handle == 0 || !CPools::ms_pPedPool) return nullptr;
    CPed* p = CPools::ms_pPedPool->GetAtRef(static_cast<int>(handle));
    return (p && CPools::ms_pPedPool->IsObjectValid(p)) ? p : nullptr;
}

static void CleanupGangSite(size_t idx) {
    if (idx >= k_totalGangPoints) return;
    auto& s = s_gangSites[idx];
    if (CPools::ms_pPedPool) {
        for (size_t i = 0; i < 2; ++i) {
            if (s.ballasHandles[i] != 0) {
                CPed* p = ResolvePed(s.ballasHandles[i]);
                if (p) {
                    if (p->m_pIntelligence) p->m_pIntelligence->m_TaskMgr.FlushImmediately();
                    Command<Commands::CLEAR_CHAR_TASKS_IMMEDIATELY>(p);
                    Command<Commands::DELETE_CHAR>(p);
                }
                s.ballasHandles[i] = 0;
            }
            if (s.groveHandles[i] != 0) {
                CPed* p = ResolvePed(s.groveHandles[i]);
                if (p) {
                    if (p->m_pIntelligence) p->m_pIntelligence->m_TaskMgr.FlushImmediately();
                    Command<Commands::CLEAR_CHAR_TASKS_IMMEDIATELY>(p);
                    Command<Commands::DELETE_CHAR>(p);
                }
                s.groveHandles[i] = 0;
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
    for (size_t i = 0; i < k_totalGangPoints; ++i) {
        CleanupGangSite(i);
        s_gangSites[i].cooldownUntilMs = 0;
    }
}

void Update(uint32_t currentMs, CPed* player) {
    if (!player) return;
    const float curCrime = s_crimeRate.load(std::memory_order_relaxed);

    if (curCrime < 35.0f) {
        for (size_t i = 0; i < k_totalGangPoints; ++i) {
            if (s_gangSites[i].active) CleanupGangSite(i);
        }
        return;
    }

    const CVector pPos = player->GetPosition();
    size_t activeCount = 0;

    for (size_t i = 0; i < k_totalGangPoints; ++i) {
        auto& s = s_gangSites[i];
        if (!s.active) continue;

        const float dx = pPos.x - k_gangCoords[i].x;
        const float dy = pPos.y - k_gangCoords[i].y;
        const float distSq = dx * dx + dy * dy;

        // Despawn if player walks away > 160m
        if (distSq > (160.0f * 160.0f)) {
            CleanupGangSite(i);
            s.cooldownUntilMs = currentMs + 15000;
            continue;
        }

        size_t livingBallas = 0;
        size_t livingGrove = 0;
        for (size_t p = 0; p < 2; ++p) {
            CPed* b = ResolvePed(s.ballasHandles[p]);
            if (b && b->m_fHealth > 0.0f) livingBallas++;

            CPed* g = ResolvePed(s.groveHandles[p]);
            if (g && g->m_fHealth > 0.0f) livingGrove++;
        }

        if (livingBallas == 0 || livingGrove == 0 || (currentMs - s.spawnTimeMs > 180000)) {
            CleanupGangSite(i);
            s.cooldownUntilMs = currentMs + 180000; // 3 min cooldown
            continue;
        }

        activeCount++;
    }

    // Strict cap: maximum 2 concurrent gang shootouts
    if (activeCount >= 2 || curCrime < 50.0f) return;
    if (!CPools::ms_pPedPool || CPools::ms_pPedPool->GetNoOfFreeSpaces() < 12) return;

    for (size_t i = 0; i < k_totalGangPoints && activeCount < 2; ++i) {
        auto& s = s_gangSites[i];
        if (s.active || currentMs < s.cooldownUntilMs) continue;

        const float dx = pPos.x - k_gangCoords[i].x;
        const float dy = pPos.y - k_gangCoords[i].y;
        const float distSq = dx * dx + dy * dy;

        if (distSq >= (45.0f * 45.0f) && distSq <= (110.0f * 110.0f)) {
            CStreaming::RequestModel(102, PRIORITY_REQUEST | GAME_REQUIRED); // Ballas
            CStreaming::RequestModel(105, PRIORITY_REQUEST | GAME_REQUIRED); // Grove
            CStreaming::RequestModel(352, PRIORITY_REQUEST | GAME_REQUIRED); // Micro Uzi

            if (!CStreaming::HasModelLoaded(102) || !CStreaming::HasModelLoaded(105) || !CStreaming::HasModelLoaded(352)) return;

            const int blip = CRadar::SetCoordBlip(BLIP_COORD, CVector(k_gangCoords[i].x, k_gangCoords[i].y, k_gangCoords[i].z), 0, BLIP_DISPLAY_BOTH, nullptr);
            if (blip >= 0) {
                CRadar::SetBlipSprite(blip, RADAR_SPRITE_ENEMYATTACK);
                CRadar::ChangeBlipColour(blip, 0); // Red
                CRadar::ChangeBlipScale(blip, 3);
                s.blipHandle = static_cast<uint32_t>(blip);
            }

            for (size_t p = 0; p < 2; ++p) {
                CVector bPos(k_gangCoords[i].x - 6.0f, k_gangCoords[i].y + (p * 2.5f), k_gangCoords[i].z + 1.0f);
                float gzB = CWorld::FindGroundZForCoord(bPos.x, bPos.y);
                if (gzB > -100.0f) bPos.z = gzB + 1.0f;
                CPed* b = new CPed(PED_TYPE_GANG1);
                if (b && CPools::ms_pPedPool->IsObjectValid(b)) {
                    b->SetModelIndex(102);
                    b->SetPosn(bPos);
                    b->m_nCreatedBy = 2;
                    b->bIsVisible = true;
                    b->m_fHealth = 150.0f;
                    b->GiveWeapon(WEAPONTYPE_MICRO_UZI, 9999, true);
                    b->SetCurrentWeapon(WEAPONTYPE_MICRO_UZI);
                    CWorld::Add(b);
                    s.ballasHandles[p] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(b));
                }

                CVector gPos(k_gangCoords[i].x + 6.0f, k_gangCoords[i].y + (p * 2.5f), k_gangCoords[i].z + 1.0f);
                float gzG = CWorld::FindGroundZForCoord(gPos.x, gPos.y);
                if (gzG > -100.0f) gPos.z = gzG + 1.0f;
                CPed* g = new CPed(PED_TYPE_GANG2);
                if (g && CPools::ms_pPedPool->IsObjectValid(g)) {
                    g->SetModelIndex(105);
                    g->SetPosn(gPos);
                    g->m_nCreatedBy = 2;
                    g->bIsVisible = true;
                    g->m_fHealth = 150.0f;
                    g->GiveWeapon(WEAPONTYPE_MICRO_UZI, 9999, true);
                    g->SetCurrentWeapon(WEAPONTYPE_MICRO_UZI);
                    CWorld::Add(g);
                    s.groveHandles[p] = static_cast<uint32_t>(CPools::ms_pPedPool->GetRef(g));
                }
            }

            for (size_t p = 0; p < 2; ++p) {
                CPed* b = ResolvePed(s.ballasHandles[p]);
                CPed* g = ResolvePed(s.groveHandles[p]);
                if (b && g) {
                    Command<Commands::TASK_KILL_CHAR_ON_FOOT>(b, g);
                    Command<Commands::TASK_KILL_CHAR_ON_FOOT>(g, b);
                }
            }

            s.active = true;
            s.spawnTimeMs = currentMs;
            activeCount++;
        }
    }
}

} // namespace GangWars
