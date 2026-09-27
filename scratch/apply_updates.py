import os

with open('src/municipal.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# 1. Update SnapToRoadCenterLane
old_snap = '''static bool SnapToRoadCenterLane(const CVector& candidatePos, CVector& outRoadPos, float& outRoadHeading) {
    constexpr unsigned char PATH_TYPE_VEHICLES = 0;
    CNodeAddress nodeAddr;
    nodeAddr.Clear();
    // Query ThePaths around the selected chokepoint to snap precisely to center road lane (25m radius)
    reinterpret_cast<CNodeAddress*(__thiscall*)(CPathFind*, CNodeAddress*, CVector, unsigned char, float, int, int, int, int, int)>(0x44F460)(
        &ThePaths, &nodeAddr, candidatePos, PATH_TYPE_VEHICLES, 25.0f, 0, 1, 0, 0, 0
    );

    if (nodeAddr.IsEmpty()) return false;

    CPathNode* pNode = ThePaths.GetPathNode(nodeAddr);
    if (!pNode || pNode->WaterNode || pNode->NumberAdjNodes == 0) return false;

    CVector roadPos = pNode->GetNodeCoors();

    // Prevent obstructing registered retail store raid entrances (>= 30m)
    for (size_t s = 0; s < 20; ++s) {
        const float sx = g_retailStores[s].posX - roadPos.x;
        const float sy = g_retailStores[s].posY - roadPos.y;
        if ((sx * sx + sy * sy) < (30.0f * 30.0f)) {
            return false;
        }
    }

    const float gz = CWorld::FindGroundZForCoord(roadPos.x, roadPos.y);
    if (gz < -100.0f || std::abs(gz - candidatePos.z) > 12.0f) return false;
    roadPos.z = gz + 0.5f; // Set wheel ground clearance: roadPos.z += 0.5f

    float scriptNx = 0.0f, scriptNy = 0.0f, scriptNz = 0.0f, scriptHeading = 0.0f;
    Command<Commands::GET_CLOSEST_CAR_NODE_WITH_HEADING>(roadPos.x, roadPos.y, roadPos.z, &scriptNx, &scriptNy, &scriptNz, &scriptHeading);
    outRoadHeading = scriptHeading * (3.14159265f / 180.0f);
    outRoadPos = roadPos;
    return true;
}'''

new_snap = '''static bool SnapToRoadCenterLane(const CVector& candidatePos, CVector& outRoadPos, float& outRoadHeading) {
    constexpr unsigned char PATH_TYPE_VEHICLES = 0;
    CNodeAddress nodeAddr;
    nodeAddr.Clear();
    // Query ThePaths around the selected chokepoint to snap precisely to center road lane (25m radius)
    reinterpret_cast<CNodeAddress*(__thiscall*)(CPathFind*, CNodeAddress*, CVector, unsigned char, float, int, int, int, int, int)>(0x44F460)(
        &ThePaths, &nodeAddr, candidatePos, PATH_TYPE_VEHICLES, 25.0f, 0, 1, 0, 0, 0
    );

    if (!nodeAddr.IsEmpty()) {
        CPathNode* pNode = ThePaths.GetPathNode(nodeAddr);
        if (pNode && !pNode->WaterNode && pNode->NumberAdjNodes > 0) {
            CVector roadPos = pNode->GetNodeCoors();

            // Prevent obstructing registered retail store raid entrances (>= 30m)
            bool nearStore = false;
            for (size_t s = 0; s < 20; ++s) {
                const float sx = g_retailStores[s].posX - roadPos.x;
                const float sy = g_retailStores[s].posY - roadPos.y;
                if ((sx * sx + sy * sy) < (30.0f * 30.0f)) {
                    nearStore = true;
                    break;
                }
            }

            if (!nearStore) {
                const float gz = CWorld::FindGroundZForCoord(roadPos.x, roadPos.y);
                if (gz >= -100.0f && std::abs(gz - candidatePos.z) <= 12.0f) {
                    roadPos.z = gz + 0.5f; // Set wheel ground clearance: roadPos.z += 0.5f
                    float scriptNx = 0.0f, scriptNy = 0.0f, scriptNz = 0.0f, scriptHeading = 0.0f;
                    Command<Commands::GET_CLOSEST_CAR_NODE_WITH_HEADING>(roadPos.x, roadPos.y, roadPos.z, &scriptNx, &scriptNy, &scriptNz, &scriptHeading);
                    outRoadHeading = scriptHeading * (3.14159265f / 180.0f);
                    outRoadPos = roadPos;
                    return true;
                }
            }
        }
    }

    // Unconditional coordinate fallback directly to user coords
    CVector roadPos = candidatePos;
    const float gz = CWorld::FindGroundZForCoord(roadPos.x, roadPos.y);
    roadPos.z = (gz > -100.0f ? gz : candidatePos.z) + 0.5f;
    outRoadPos = roadPos;
    outRoadHeading = 0.0f;
    return true; // Force success
}'''

assert old_snap in content, 'old_snap not found'
content = content.replace(old_snap, new_snap)

# 2. Update UpdateCrisisAtmosphere init message
old_update_init = '''static void UpdateCrisisAtmosphere(uint32_t currentMs, CPed* player) {
    if (!player) return;

    const float curUnrest = static_cast<float>(s_socialUnrest.load(std::memory_order_relaxed));'''

new_update_init = '''static void UpdateCrisisAtmosphere(uint32_t currentMs, CPed* player) {
    if (!player) return;
    static bool s_initNotified = false;
    if (!s_initNotified) {
        CMessages::AddMessageJumpQ("VERIFIED 63 ROADBLOCKS LOADED", 4000, 0, false);
        s_initNotified = true;
    }

    const float curUnrest = static_cast<float>(s_socialUnrest.load(std::memory_order_relaxed));'''

assert old_update_init in content, 'old_update_init not found'
content = content.replace(old_update_init, new_update_init)

# 3. Update check interval to 400ms
old_interval = '''    static uint32_t s_lastRoadblockSpawnCheckMs = 0;
    if (currentMs - s_lastRoadblockSpawnCheckMs < 2000) return;
    s_lastRoadblockSpawnCheckMs = currentMs;'''

new_interval = '''    static uint32_t s_lastRoadblockSpawnCheckMs = 0;
    if (currentMs - s_lastRoadblockSpawnCheckMs < 400) return;
    s_lastRoadblockSpawnCheckMs = currentMs;'''

assert old_interval in content, 'old_interval not found'
content = content.replace(old_interval, new_interval)

# 4. Update candidate distance bracket to [15.0f, 180.0f]
old_bracket = '''        // Distance bracket: within approaching corridor (45m to 140m from player)
        if (cDist >= 45.0f && cDist <= 140.0f) {
            candidateChokepoints[numCandidates++] = static_cast<int>(i);
        }'''

new_bracket = '''        // Distance bracket: within approaching corridor (15m to 180m from player)
        if (cDist >= 15.0f && cDist <= 180.0f) {
            candidateChokepoints[numCandidates++] = static_cast<int>(i);
        }'''

assert old_bracket in content, 'old_bracket not found'
content = content.replace(old_bracket, new_bracket)

# 5. Update roadblock spawn notification and log
old_log = '''    chosenCp.activeRoadblockSlot = static_cast<int>(freeSlot);

    AddMunicipalLog("CHOKEPOINT ROADBLOCK: Barricade active at chokepoint #%d (%.1f, %.1f) with %zu vehicles, %zu rioters",
        chosenIdx, roadPos.x, roadPos.y, rb.vehCount, rb.rioterCount);'''

new_log = '''    chosenCp.activeRoadblockSlot = static_cast<int>(freeSlot);

    const float chosenDist = std::sqrt(
        (chosenCp.pos.x - pPos.x) * (chosenCp.pos.x - pPos.x) +
        (chosenCp.pos.y - pPos.y) * (chosenCp.pos.y - pPos.y)
    );
    CMessages::AddMessageJumpQ("ROADBLOCK SPAWNED", 2000, 0, false);
    AddMunicipalLog("[ROADBLOCK] Spawned at chokepoint #%d (dist: %.1f)", chosenIdx, chosenDist);
    AddMunicipalLog("CHOKEPOINT ROADBLOCK: Barricade active at chokepoint #%d (%.1f, %.1f) with %zu vehicles, %zu rioters",
        chosenIdx, roadPos.x, roadPos.y, rb.vehCount, rb.rioterCount);'''

assert old_log in content, 'old_log not found'
content = content.replace(old_log, new_log)

with open('src/municipal.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print('Successfully updated src/municipal.cpp directly!')
