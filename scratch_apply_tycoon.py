import sys

with open('D:/gtaspscqueue/src/main.cpp', 'r', encoding='utf-8', errors='ignore') as f:
    code = f.read()

with open('D:/gtaspscqueue/scratch_logistics.html', 'r', encoding='utf-8') as f:
    logistics_html = f.read()

# 1. Insert k_logisticsHtml definition
logistics_def = '\nstatic const char k_logisticsHtml[] = R"rawlogistics(' + logistics_html + ')rawlogistics";\n'

map_end_target = '</html>)rawmap";'
if map_end_target not in code:
    print("ERROR: map_end_target not found!")
    sys.exit(1)

code = code.replace(map_end_target, map_end_target + '\n' + logistics_def, 1)

# 2. Add /logistics route in HandleHttpClient
route_target = """    else if (path == "/map" || path == "/map.html") {
        const std::string header = "HTTP/1.1 200 OK\\r\\n"
                                   "Content-Type: text/html; charset=utf-8\\r\\n"
                                   "Content-Length: " + std::to_string(sizeof(k_mapHtml) - 1) + "\\r\\n"
                                   "Connection: close\\r\\n\\r\\n";
        SendAll(clientSock, header.c_str(), static_cast<int>(header.size()));
        SendAll(clientSock, k_mapHtml, static_cast<int>(sizeof(k_mapHtml) - 1));
    }"""

route_replacement = """    else if (path == "/map" || path == "/map.html") {
        const std::string header = "HTTP/1.1 200 OK\\r\\n"
                                   "Content-Type: text/html; charset=utf-8\\r\\n"
                                   "Content-Length: " + std::to_string(sizeof(k_mapHtml) - 1) + "\\r\\n"
                                   "Connection: close\\r\\n\\r\\n";
        SendAll(clientSock, header.c_str(), static_cast<int>(header.size()));
        SendAll(clientSock, k_mapHtml, static_cast<int>(sizeof(k_mapHtml) - 1));
    }
    else if (path == "/logistics" || path == "/logistics.html") {
        const std::string header = "HTTP/1.1 200 OK\\r\\n"
                                   "Content-Type: text/html; charset=utf-8\\r\\n"
                                   "Content-Length: " + std::to_string(sizeof(k_logisticsHtml) - 1) + "\\r\\n"
                                   "Connection: close\\r\\n\\r\\n";
        SendAll(clientSock, header.c_str(), static_cast<int>(header.size()));
        SendAll(clientSock, k_logisticsHtml, static_cast<int>(sizeof(k_logisticsHtml) - 1));
    }"""

if route_target not in code:
    print("ERROR: route_target not found!")
    sys.exit(1)

code = code.replace(route_target, route_replacement, 1)

# 3. Update /api/status serialization in HandleHttpClient
status_target = """        // Read active truck snapshot wait-free
        const uint32_t snapIdx = g_truckSnapshotActiveIdx.load(std::memory_order_acquire);
        std::string trucksJson = "[";
        for (size_t i = 0; i < k_truckCount; ++i) {
            const auto& t = g_truckSnapshots[snapIdx][i];
            const char* cargoStr = (t.cargoType == 1 ? "Timber" : (t.cargoType == 2 ? "Fuel/Oil" : (t.cargoType == 3 ? "Electronics" : (t.cargoType == 4 ? "Food" : "Empty"))));
            const float roundedFuel = std::round(t.fuel * 10.0f) / 10.0f;
            const float roundedFatigue = std::round(t.fatigue * 10.0f) / 10.0f;
            char itemBuf[384];
            snprintf(itemBuf, sizeof(itemBuf),
                "%s{\\"id\\":%u,\\"x\\":%.1f,\\"y\\":%.1f,\\"z\\":%.1f,\\"heading\\":%.1f,\\"speed\\":%.1f,\\"materialized\\":%s,\\"state\\":\\"%s\\","
                "\\"fuel\\":%.1f,\\"fatigue\\":%.1f,\\"cargo\\":\\"%s\\",\\"cargo_tons\\":%d,\\"deliveries\\":%d}",
                (i > 0 ? "," : ""),
                t.id, t.x, t.y, t.z, t.heading, t.speed,
                (t.isMaterialized ? "true" : "false"),
                (t.state == TruckState::LOADING ? "LOADING" : (t.state == TruckState::RESTING ? "RESTING" : (t.state == TruckState::UNLOADING ? "UNLOADING" : "EN_ROUTE"))),
                roundedFuel, roundedFatigue, cargoStr, t.cargoWeightTons, t.deliveriesDone
            );
            trucksJson += itemBuf;
        }
        trucksJson += "]";

        char headerJsonBuf[1024];
        snprintf(headerJsonBuf, sizeof(headerJsonBuf),
            "{"
            "\\"status\\":\\"ok\\","
            "\\"company_balance\\":%llu,"
            "\\"pos\\":{\\"x\\":%.2f,\\"y\\":%.2f,\\"z\\":%.2f},"
            "\\"heading\\":%.2f,"
            "\\"speed\\":%.3f,"
            "\\"aiDirective\\":{\\"x\\":%.2f,\\"y\\":%.2f,\\"z\\":%.2f,\\"id\\":%u},"
            "\\"stats\\":{\\"web\\":%llu,\\"telemetry\\":%llu,\\"directives\\":%llu},"
            "\\"webCommandsTotal\\":%llu,"
            "\\"telemetryTotal\\":%llu,"
            "\\"directivesTotal\\":%llu,"
            "\\"trucks\\":",
            static_cast<unsigned long long>(s_companyBalance.load(std::memory_order_relaxed)),
            g_playerX.load(std::memory_order_relaxed),
            g_playerY.load(std::memory_order_relaxed),
            g_playerZ.load(std::memory_order_relaxed),
            g_playerHeading.load(std::memory_order_relaxed),
            g_playerSpeed.load(std::memory_order_relaxed),
            g_lastDirectiveX.load(std::memory_order_relaxed),
            g_lastDirectiveY.load(std::memory_order_relaxed),
            g_lastDirectiveZ.load(std::memory_order_relaxed),
            g_lastDirectiveId.load(std::memory_order_relaxed),
            g_totalWebCommands.load(std::memory_order_relaxed),
            g_totalTelemetryProcessed.load(std::memory_order_relaxed),
            g_totalDirectivesExecuted.load(std::memory_order_relaxed),
            g_totalWebCommands.load(std::memory_order_relaxed),
            g_totalTelemetryProcessed.load(std::memory_order_relaxed),
            g_totalDirectivesExecuted.load(std::memory_order_relaxed)
        );"""

status_replacement = """        // Read active truck snapshot wait-free
        const uint32_t snapIdx = g_truckSnapshotActiveIdx.load(std::memory_order_acquire);
        std::string trucksJson = "[";
        for (size_t i = 0; i < k_truckCount; ++i) {
            const auto& t = g_truckSnapshots[snapIdx][i];
            const char* cargoStr = (t.cargoType == 1 ? "Timber" : (t.cargoType == 2 ? "Fuel/Oil" : (t.cargoType == 3 ? "Electronics" : (t.cargoType == 4 ? "Food" : "Empty"))));
            const char* stateStr = (t.state == TruckState::LOADING ? "LOADING" : (t.state == TruckState::RESTING ? "RESTING" : (t.state == TruckState::UNLOADING ? "UNLOADING" : (t.state == TruckState::BROKEN_DOWN ? "BROKEN_DOWN" : "EN_ROUTE"))));
            const float roundedFuel = std::round(t.fuel * 10.0f) / 10.0f;
            const float roundedFatigue = std::round(t.fatigue * 10.0f) / 10.0f;
            const uint8_t cId = t.companyId % 3;
            char itemBuf[512];
            snprintf(itemBuf, sizeof(itemBuf),
                "%s{\\"id\\":%u,\\"x\\":%.1f,\\"y\\":%.1f,\\"z\\":%.1f,\\"heading\\":%.1f,\\"speed\\":%.1f,\\"materialized\\":%s,\\"state\\":\\"%s\\","
                "\\"fuel\\":%.1f,\\"fatigue\\":%.1f,\\"cargo\\":\\"%s\\",\\"cargo_tons\\":%d,\\"deliveries\\":%d,"
                "\\"company_id\\":%u,\\"company_name\\":\\"%s\\",\\"company_color\\":\\"%s\\",\\"driver_name\\":\\"%s\\",\\"driver_wallet\\":%u}",
                (i > 0 ? "," : ""),
                t.id, t.x, t.y, t.z, t.heading, t.speed,
                (t.isMaterialized ? "true" : "false"),
                stateStr,
                roundedFuel, roundedFatigue, cargoStr, t.cargoWeightTons, t.deliveriesDone,
                cId, k_companies[cId].name, k_companies[cId].color,
                (t.driverName ? t.driverName : "Driver"), t.driverWallet
            );
            trucksJson += itemBuf;
        }
        trucksJson += "]";

        std::string companiesJson = "[";
        for (size_t c = 0; c < 3; ++c) {
            char cBuf[256];
            int ownedAssets = 0;
            for (size_t a = 0; a < 3; ++a) {
                if (s_assetOwners[a].load(std::memory_order_relaxed) == static_cast<int8_t>(c)) ownedAssets++;
            }
            snprintf(cBuf, sizeof(cBuf),
                "%s{\\"id\\":%u,\\"name\\":\\"%s\\",\\"balance\\":%llu,\\"color\\":\\"%s\\",\\"baseHub\\":\\"%s\\",\\"ownedAssets\\":%d,\\"truckCount\\":5}",
                (c > 0 ? "," : ""),
                k_companies[c].id, k_companies[c].name,
                static_cast<unsigned long long>(s_companyBalances[c].load(std::memory_order_relaxed)),
                k_companies[c].color, k_companies[c].baseHub, ownedAssets
            );
            companiesJson += cBuf;
        }
        companiesJson += "]";

        std::string assetsJson = "[";
        for (size_t a = 0; a < 3; ++a) {
            char aBuf[256];
            snprintf(aBuf, sizeof(aBuf),
                "%s{\\"id\\":%u,\\"name\\":\\"%s\\",\\"cost\\":%u,\\"ownerId\\":%d}",
                (a > 0 ? "," : ""),
                k_assets[a].id, k_assets[a].name, k_assets[a].cost,
                s_assetOwners[a].load(std::memory_order_relaxed)
            );
            assetsJson += aBuf;
        }
        assetsJson += "]";

        char headerJsonBuf[2048];
        snprintf(headerJsonBuf, sizeof(headerJsonBuf),
            "{"
            "\\"status\\":\\"ok\\","
            "\\"companies\\":%s,"
            "\\"assets\\":%s,"
            "\\"company_balance\\":%llu,"
            "\\"pos\\":{\\"x\\":%.2f,\\"y\\":%.2f,\\"z\\":%.2f},"
            "\\"heading\\":%.2f,"
            "\\"speed\\":%.3f,"
            "\\"aiDirective\\":{\\"x\\":%.2f,\\"y\\":%.2f,\\"z\\":%.2f,\\"id\\":%u},"
            "\\"stats\\":{\\"web\\":%llu,\\"telemetry\\":%llu,\\"directives\\":%llu},"
            "\\"webCommandsTotal\\":%llu,"
            "\\"telemetryTotal\\":%llu,"
            "\\"directivesTotal\\":%llu,"
            "\\"trucks\\":",
            companiesJson.c_str(),
            assetsJson.c_str(),
            static_cast<unsigned long long>(s_companyBalances[0].load(std::memory_order_relaxed)),
            g_playerX.load(std::memory_order_relaxed),
            g_playerY.load(std::memory_order_relaxed),
            g_playerZ.load(std::memory_order_relaxed),
            g_playerHeading.load(std::memory_order_relaxed),
            g_playerSpeed.load(std::memory_order_relaxed),
            g_lastDirectiveX.load(std::memory_order_relaxed),
            g_lastDirectiveY.load(std::memory_order_relaxed),
            g_lastDirectiveZ.load(std::memory_order_relaxed),
            g_lastDirectiveId.load(std::memory_order_relaxed),
            g_totalWebCommands.load(std::memory_order_relaxed),
            g_totalTelemetryProcessed.load(std::memory_order_relaxed),
            g_totalDirectivesExecuted.load(std::memory_order_relaxed),
            g_totalWebCommands.load(std::memory_order_relaxed),
            g_totalTelemetryProcessed.load(std::memory_order_relaxed),
            g_totalDirectivesExecuted.load(std::memory_order_relaxed)
        );"""

if status_target not in code:
    print("ERROR: status_target not found!")
    sys.exit(1)

code = code.replace(status_target, status_replacement, 1)

with open('D:/gtaspscqueue/src/main.cpp', 'w', encoding='utf-8') as f:
    f.write(code)

print("SUCCESS: Stage 1 (HTML, Routes, /api/status) applied!")
