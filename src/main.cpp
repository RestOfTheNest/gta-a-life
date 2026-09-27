// =============================================================================
//  gtasystemcore.asi — Multi-Subsystem Lock-Free Core Engine for GTA San Andreas
//  GTA San Andreas v1.0 US · Plugin-SDK · MSVC x86 · C++23
// =============================================================================

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "globals.h"
#include "economy.h"
#include "municipal.h"
#include "rioters.h"
#include "web_assets.h"
#include "crash_handler.h"
#include "streaming_fix.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

#include <d3d9.h>
#include "RenderWare.h"

#include "plugin.h"
#include "CPed.h"
#include "CVehicle.h"
#include "CAutomobile.h"
#include "CStreaming.h"
#include "CPools.h"
#include "CWorld.h"
#include "CHud.h"
#include "CWeather.h"
#include "CTimer.h"
#include "CClock.h"
#include "CWanted.h"
#include "CPlayerPed.h"
#include "CVector.h"
#include "CRadar.h"
#include "CTheScripts.h"
#include "CCheckpoints.h"
#include "C3dMarkers.h"
#include "C3dMarker.h"
#include "CCoronas.h"
#include "Patch.h"
#include "common.h"
#include "CPickups.h"
#include "CPickup.h"
#include "enums/eWeaponType.h"
#include "enums/ePedType.h"
#include "enums/eModelID.h"
#include "extensions/ScriptCommands.h"
#include "CFont.h"
#include "CMessages.h"
#include "CCarCtrl.h"
#include "CPopulation.h"
// A-Life Municipal Incidents: verified Plugin-SDK headers (see registry comment)
#include "CTaskManager.h"
#include "CTaskComplexKillPedOnFoot.h"
#include "CTaskComplexWanderStandard.h"
#include "CTaskSimpleStandStill.h"
#include "CAnimManager.h"
#include "CColStore.h"
#include "enums/eAnimations.h"
#include "CObject.h"
#include "CPathFind.h"
#include "CPathNode.h"
#include "CTaskSimpleUseGun.h"
#include "CWeaponInfo.h"
#include "CWeapon.h"
#include "enums/eMoveState.h"

#ifndef bPanicWhenScared
#define bPanicWhenScared bFleeWhenStanding
#endif

#ifndef m_nTotalAmmo
#define m_nTotalAmmo m_nAmmoTotal
#endif

#ifndef WEAPONTYPE_SPAS12_SHOTGUN
#define WEAPONTYPE_SPAS12_SHOTGUN WEAPONTYPE_SPAS12
#endif

#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <cstdint>
#include <cstdio>
#include <share.h>
#include <cstdarg>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <algorithm>
#include <new>
#include <type_traits>

using namespace plugin;
using namespace plugin::scripting;

// =============================================================================
//  §2. Engine Constants & Aliases
// =============================================================================

static constexpr int STREAMING_PRIORITY_REQUEST = PRIORITY_REQUEST;
static constexpr int STREAMING_GAME_REQUIRED = GAME_REQUIRED;

static constexpr eCarDrivingStyle DRIVINGSTYLE_NORMAL = DRIVINGSTYLE_STOP_FOR_CARS;

#ifndef MISSION_GOTO_COORDS_STRAIGHT
#define MISSION_GOTO_COORDS_STRAIGHT MISSION_GOTOCOORDS_STRAIGHT
#endif

#ifndef DRIVING_STYLE_AVOID_CARS
#define DRIVING_STYLE_AVOID_CARS DRIVINGSTYLE_AVOID_CARS
#endif

#ifndef m_vecDestination
#define m_vecDestination m_vecDestinationCoors
#endif

#ifndef m_nDrivingStyle
#define m_nDrivingStyle m_nCarDrivingStyle
#endif

#ifndef m_nHandbrakeOn
#define m_nHandbrakeOn bIsHandbrakeOn
#endif

#ifndef m_nMoney
#define m_nMoney GetMoney()
#endif

static std::atomic<CustomShopType> s_activeShopType{ CustomShopType::None };
static std::atomic<uint8_t>        s_activeShopIndex{ 0 };

// =============================================================================
//  §3. Lock-Free SPSCQueue Primitive
// =============================================================================

// Activity metrics for monitoring
static std::atomic<uint64_t> g_totalWebCommands{ 0 };

// Real-time telemetry cache for web GPS radar & API
static std::atomic<float> g_playerZ{ 0.0f };
static std::atomic<float> g_playerSpeed{ 0.0f };
static std::atomic<float> g_playerHeading{ 0.0f };

// Real-time AI directive cache for web GPS radar & API
static std::atomic<float>    g_lastDirectiveX{ 0.0f };
static std::atomic<float>    g_lastDirectiveY{ 0.0f };
static std::atomic<float>    g_lastDirectiveZ{ 0.0f };
static std::atomic<uint32_t> g_lastDirectiveId{ 0 };

static std::atomic<SOCKET> g_listenSock{ INVALID_SOCKET };
static std::thread         g_webWorkerThread;
static std::thread         g_aiDirectorThread;
// g_municipalDirectorThread removed in Phase 5 (migrated to Lua)


// =============================================================================
//  §7. HTTP Request Handling
// =============================================================================

static bool SendAll(SOCKET s, const char* data, int totalBytes) {
    int bytesSent = 0;
    while (bytesSent < totalBytes) {
        const int res = send(s, data + bytesSent, totalBytes - bytesSent, 0);
        if (res <= 0) return false;
        bytesSent += res;
    }
    return true;
}

struct SocketGuard {
    SOCKET s;
    ~SocketGuard() {
        if (s != INVALID_SOCKET) {
            shutdown(s, SD_BOTH);
            closesocket(s);
            s = INVALID_SOCKET;
        }
    }
};

static void HandleHttpClient(SOCKET clientSock) {
    SocketGuard sockGuard{ clientSock };
    std::vector<char> reqBuf(65536);
    int bytesReceived = recv(clientSock, reqBuf.data(), static_cast<int>(reqBuf.size() - 1), 0);
    if (bytesReceived <= 0) {
        return;
    }
    reqBuf[bytesReceived] = '\0';

    size_t contentLen = 0;
    const char* clHdr = strstr(reqBuf.data(), "Content-Length:");
    if (!clHdr) clHdr = strstr(reqBuf.data(), "content-length:");
    if (clHdr) {
        contentLen = static_cast<size_t>(std::strtoul(clHdr + 15, nullptr, 10));
    }

    const char* dblCrLf = strstr(reqBuf.data(), "\r\n\r\n");
    if (dblCrLf && contentLen > 0) {
        size_t headerLen = (dblCrLf + 4) - reqBuf.data();
        size_t bodyBytesSoFar = (bytesReceived > static_cast<int>(headerLen)) ? (bytesReceived - headerLen) : 0;
        while (bodyBytesSoFar < contentLen && bytesReceived < static_cast<int>(reqBuf.size() - 1)) {
            int more = recv(clientSock, reqBuf.data() + bytesReceived, static_cast<int>(reqBuf.size() - 1 - bytesReceived), 0);
            if (more <= 0) break;
            bytesReceived += more;
            reqBuf[bytesReceived] = '\0';
            bodyBytesSoFar = bytesReceived - headerLen;
        }
    }

    const bool isGet = (strncmp(reqBuf.data(), "GET ", 4) == 0);
    const bool isPost = (strncmp(reqBuf.data(), "POST ", 5) == 0);
    if (!isGet && !isPost) {
        const char resp405[] = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        send(clientSock, resp405, static_cast<int>(sizeof(resp405) - 1), 0);
        return;
    }

    char* pathStart = isGet ? (reqBuf.data() + 4) : (reqBuf.data() + 5);
    char* pathEnd = strchr(pathStart, ' ');
    if (!pathEnd) {
        return;
    }
    *pathEnd = '\0';
    const std::string path(pathStart);
    const char* body = nullptr;
    if (isPost) {
        dblCrLf = strstr(pathEnd + 1, "\r\n\r\n");
        if (dblCrLf) {
            body = dblCrLf + 4;
        }
    }

    if (path == "/" || path == "/index.html") {
        const std::string header = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/html; charset=utf-8\r\n"
                                   "Content-Length: " + std::to_string(sizeof(k_indexHtml) - 1) + "\r\n"
                                   "Connection: close\r\n\r\n";
        SendAll(clientSock, header.c_str(), static_cast<int>(header.size()));
        SendAll(clientSock, k_indexHtml, static_cast<int>(sizeof(k_indexHtml) - 1));
    }
    else if (path == "/map" || path == "/map.html") {
        const std::string header = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/html; charset=utf-8\r\n"
                                   "Content-Length: " + std::to_string(sizeof(k_mapHtml) - 1) + "\r\n"
                                   "Connection: close\r\n\r\n";
        SendAll(clientSock, header.c_str(), static_cast<int>(header.size()));
        SendAll(clientSock, k_mapHtml, static_cast<int>(sizeof(k_mapHtml) - 1));
    }
    else if (path == "/logistics" || path == "/logistics.html") {
        const std::string header = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/html; charset=utf-8\r\n"
                                   "Content-Length: " + std::to_string(sizeof(k_logisticsHtml) - 1) + "\r\n"
                                   "Connection: close\r\n\r\n";
        SendAll(clientSock, header.c_str(), static_cast<int>(header.size()));
        SendAll(clientSock, k_logisticsHtml, static_cast<int>(sizeof(k_logisticsHtml) - 1));
    }
    else if (path == "/municipal" || path == "/municipal.html") {
        const std::string header = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/html; charset=utf-8\r\n"
                                   "Content-Length: " + std::to_string(sizeof(k_municipalHtml) - 1) + "\r\n"
                                   "Connection: close\r\n\r\n";
        SendAll(clientSock, header.c_str(), static_cast<int>(header.size()));
        SendAll(clientSock, k_municipalHtml, static_cast<int>(sizeof(k_municipalHtml) - 1));
    }
    else if (isPost && path.rfind("/api/routes/save", 0) == 0) {
        uint8_t cId = 0;
        std::string color;
        std::vector<CustomRouteNode> nodes;
        bool parsed = ParseRouteJsonPayload(body, cId, color, nodes);
        bool saved = false;
        if (parsed) {
            saved = SaveSingleCustomRoute(cId, color.c_str(), nodes);
        }
        std::string json;
        if (saved) {
            json = "{\"status\":\"ok\",\"message\":\"Route deployed successfully\",\"companyId\":" + std::to_string(cId) + ",\"nodeCount\":" + std::to_string(nodes.size()) + "}";
        } else {
            json = "{\"status\":\"error\",\"message\":\"Failed to save or parse route payload\"}";
        }
        const std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " +
                                 std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (path.rfind("/api/routes/load", 0) == 0) {
        const std::string json = ExportCustomRoutesJson();
        const std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " +
                                 std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (path.rfind("/api/weather", 0) == 0) {
        int weatherId = 1;
        const size_t idPos = path.find("id=");
        if (idPos != std::string::npos) weatherId = std::atoi(path.c_str() + idPos + 3);

        const GameCommand cmd{ CommandType::SetWeather, weatherId };
        g_webCommandQueue.push(cmd);
        g_totalWebCommands.fetch_add(1, std::memory_order_relaxed);

        const std::string json = "{\"status\":\"ok\",\"type\":\"SetWeather\",\"param\":" + std::to_string(weatherId) + "}";
        const std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " +
                                 std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (path.rfind("/api/vitals", 0) == 0) {
        const GameCommand cmd{ CommandType::RestoreVitals, 0 };
        g_webCommandQueue.push(cmd);
        g_totalWebCommands.fetch_add(1, std::memory_order_relaxed);

        const std::string json = "{\"status\":\"ok\",\"type\":\"RestoreVitals\",\"param\":0}";
        const std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " +
                                 std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (path.rfind("/api/notice", 0) == 0) {
        int noticeId = 0;
        const size_t idPos = path.find("id=");
        if (idPos != std::string::npos) noticeId = std::atoi(path.c_str() + idPos + 3);

        const GameCommand cmd{ CommandType::DisplayNotice, noticeId };
        g_webCommandQueue.push(cmd);
        g_totalWebCommands.fetch_add(1, std::memory_order_relaxed);

        const std::string json = "{\"status\":\"ok\",\"type\":\"DisplayNotice\",\"param\":" + std::to_string(noticeId) + "}";
        const std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " +
                                 std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (isPost && (path.rfind("/api/municipal/taxes", 0) == 0 || path.rfind("/api/policy", 0) == 0)) {
        float sg = s_taxSalesGeneral.load(std::memory_order_relaxed);
        float sl = s_taxSalesLuxury.load(std::memory_order_relaxed);
        float hp = s_taxHaulerPort.load(std::memory_order_relaxed);
        float cw = s_taxCorporateWealth.load(std::memory_order_relaxed);
        int32_t cp = s_taxCitizenPoll.load(std::memory_order_relaxed);

        if (body) {
            auto extractFloat = [](const char* json, const char* key, float defVal) -> float {
                const char* p = strstr(json, key);
                if (!p) return defVal;
                const char* colon = strchr(p, ':');
                if (!colon) return defVal;
                return static_cast<float>(std::strtod(colon + 1, nullptr));
            };
            auto extractInt = [](const char* json, const char* key, int32_t defVal) -> int32_t {
                const char* p = strstr(json, key);
                if (!p) return defVal;
                const char* colon = strchr(p, ':');
                if (!colon) return defVal;
                return static_cast<int32_t>(std::strtol(colon + 1, nullptr, 10));
            };

            if (strstr(body, "\"salesGeneral\"")) sg = (std::clamp)(extractFloat(body, "\"salesGeneral\"", sg), 0.02f, 0.30f);
            if (strstr(body, "\"salesLuxury\"")) sl = (std::clamp)(extractFloat(body, "\"salesLuxury\"", sl), 0.05f, 0.50f);
            if (strstr(body, "\"haulerPort\"")) hp = (std::clamp)(extractFloat(body, "\"haulerPort\"", hp), 0.05f, 0.40f);
            if (strstr(body, "\"corporateWealth\"")) cw = (std::clamp)(extractFloat(body, "\"corporateWealth\"", cw), 0.00f, 0.15f);
            if (strstr(body, "\"citizenPoll\"")) cp = (std::clamp)(extractInt(body, "\"citizenPoll\"", cp), 0, 4000);

            const char* dt = strstr(body, "\"districtTaxes\"");
            if (!dt) dt = strstr(body, "\"districtPollTaxes\"");
            if (dt) {
                const char* bracket = strchr(dt, '[');
                if (bracket) {
                    char* endPtr = nullptr;
                    const char* cur = bracket + 1;
                    for (int d = 0; d < 4; ++d) {
                        float val = static_cast<float>(std::strtod(cur, &endPtr));
                        if (endPtr && endPtr != cur) {
                            val = (std::clamp)(val, 0.0f, 1500.0f);
                            s_districtPollTax[d].store(val, std::memory_order_relaxed);
                            cur = endPtr;
                            const char* comma = strchr(cur, ',');
                            if (comma) cur = comma + 1;
                        }
                    }
                }
            } else if (strstr(body, "\"citizenPoll\"")) {
                s_districtPollTax[0].store((std::clamp)(static_cast<float>(cp) * 0.1875f, 0.0f, 1500.0f), std::memory_order_relaxed);
                s_districtPollTax[1].store((std::clamp)(static_cast<float>(cp) * 0.6250f, 0.0f, 1500.0f), std::memory_order_relaxed);
                s_districtPollTax[2].store((std::clamp)(static_cast<float>(cp) * 0.3125f, 0.0f, 1500.0f), std::memory_order_relaxed);
                s_districtPollTax[3].store((std::clamp)(static_cast<float>(cp) * 0.2250f, 0.0f, 1500.0f), std::memory_order_relaxed);
            }
        }

        s_taxSalesGeneral.store(sg, std::memory_order_relaxed);
        s_taxSalesLuxury.store(sl, std::memory_order_relaxed);
        s_taxHaulerPort.store(hp, std::memory_order_relaxed);
        s_taxCorporateWealth.store(cw, std::memory_order_relaxed);
        s_taxCitizenPoll.store(cp, std::memory_order_relaxed);

        AddMunicipalLog("TAX REFORM: Mayor updated municipal and district tax policy matrix");

        char respBuf[768];
        snprintf(respBuf, sizeof(respBuf),
            "{\"status\":\"ok\",\"message\":\"Tax policy updated\",\"salesGeneral\":%.3f,\"salesLuxury\":%.3f,\"haulerPort\":%.3f,\"corporateWealth\":%.3f,\"citizenPoll\":%d,\"districtTaxes\":[%.1f,%.1f,%.1f,%.1f]}",
            sg, sl, hp, cw, cp,
            s_districtPollTax[0].load(std::memory_order_relaxed),
            s_districtPollTax[1].load(std::memory_order_relaxed),
            s_districtPollTax[2].load(std::memory_order_relaxed),
            s_districtPollTax[3].load(std::memory_order_relaxed));
        const std::string json(respBuf);
        const std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " +
                                 std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (!isPost && (path.rfind("/api/municipal/taxes", 0) == 0 || path.rfind("/api/policy", 0) == 0)) {
        const float sg = s_taxSalesGeneral.load(std::memory_order_relaxed);
        const float sl = s_taxSalesLuxury.load(std::memory_order_relaxed);
        const float hp = s_taxHaulerPort.load(std::memory_order_relaxed);
        const float cw = s_taxCorporateWealth.load(std::memory_order_relaxed);
        const int32_t cp = s_taxCitizenPoll.load(std::memory_order_relaxed);

        char respBuf[768];
        snprintf(respBuf, sizeof(respBuf),
            "{\"status\":\"ok\",\"salesGeneral\":%.3f,\"salesLuxury\":%.3f,\"haulerPort\":%.3f,\"corporateWealth\":%.3f,\"citizenPoll\":%d,\"districtTaxes\":[%.1f,%.1f,%.1f,%.1f]}",
            sg, sl, hp, cw, cp,
            s_districtPollTax[0].load(std::memory_order_relaxed),
            s_districtPollTax[1].load(std::memory_order_relaxed),
            s_districtPollTax[2].load(std::memory_order_relaxed),
            s_districtPollTax[3].load(std::memory_order_relaxed));
        const std::string json(respBuf);
        const std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " +
                                 std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (path.rfind("/api/municipal/veto", 0) == 0) {
        s_cityTreasury.fetch_add(25000, std::memory_order_relaxed);
        s_emergencyState.store(0, std::memory_order_relaxed);
        AddMunicipalLog("MAYOR VETO: Overrode crisis, restored municipal authority (+ $25,000)");
        const std::string json = "{\"status\":\"ok\",\"message\":\"Mayor veto executed in-memory.\"}";
        const std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " +
                                 std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (path.rfind("/api/municipal/curfew", 0) == 0) {
        const bool currentCurfew = s_curfewActive.load(std::memory_order_relaxed);
        const bool newCurfew = !currentCurfew;
        s_curfewActive.store(newCurfew, std::memory_order_relaxed);

        MunicipalGameEvent ev{};
        ev.type = MunicipalGameEventType::CurfewDensity;
        ev.param = newCurfew ? 1 : 0;
        EnqueueMunicipalEvent(ev);

        AddMunicipalLog(newCurfew ? "CURFEW ENACTED: Police decree issued, streets cleared" : "CURFEW LIFTED: Civil traffic restored");
        const std::string json = std::string("{\"status\":\"ok\",\"curfew_active\":") + (newCurfew ? "true" : "false") + "}";
        const std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " +
                                 std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (path.rfind("/api/municipal/bribe/accept", 0) == 0) {
        {
            std::lock_guard<std::mutex> lock(s_bribeOfferMutex);
            if (s_activeBribeOffer.active) {
                s_cityTreasury.fetch_add(s_activeBribeOffer.amount, std::memory_order_relaxed);
                float curCrime = s_crimeRate.load(std::memory_order_relaxed);
                s_crimeRate.store((std::min)(100.0f, curCrime + s_activeBribeOffer.crimeDelta), std::memory_order_relaxed);
                s_activeBribeOffer.active = false;
                AddMunicipalLog("BRIBE ACCEPTED: Secret funds deposited to treasury (+ $%lld)", static_cast<long long>(s_activeBribeOffer.amount));
            }
        }
        const std::string json = "{\"status\":\"ok\",\"message\":\"Bribe accepted in-memory.\"}";
        const std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " +
                                 std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (path.rfind("/api/municipal/bribe/reject", 0) == 0) {
        {
            std::lock_guard<std::mutex> lock(s_bribeOfferMutex);
            if (s_activeBribeOffer.active) {
                s_activeBribeOffer.active = false;
                AddMunicipalLog("BRIBE REJECTED: Mayor refused corrupt syndicate offer");
            }
        }
        const std::string json = "{\"status\":\"ok\",\"message\":\"Bribe rejected in-memory.\"}";
        const std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " +
                                 std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (path.rfind("/api/municipal/strike/subsidize", 0) == 0) {
        s_cityTreasury.fetch_sub(50000, std::memory_order_relaxed);
        float curUnrest = s_socialUnrest.load(std::memory_order_relaxed);
        s_socialUnrest.store((std::max)(0.0f, curUnrest - 20.0f), std::memory_order_relaxed);
        {
            std::lock_guard<std::mutex> lock(s_unionStrikeMutex);
            s_activeUnionStrike.active = false;
        }
        MunicipalGameEvent ev{};
        ev.type = MunicipalGameEventType::StrikeDisband;
        EnqueueMunicipalEvent(ev);
        AddMunicipalLog("UNION SUBSIDY: $50,000 paid to strike fund. Workers dispersed.");
        const std::string json = "{\"status\":\"ok\",\"message\":\"Union subsidized and disbanded.\"}";
        const std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " +
                                 std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (path.rfind("/api/municipal/debug", 0) == 0) {
        bool authorized = (path.find("debug_key=mayor_sec_8941_auth") != std::string::npos);
        if (!authorized) {
            const std::string json = "{\"status\":\"error\",\"message\":\"403 Forbidden: Invalid or missing debug_key.\"}";
            const std::string resp = "HTTP/1.1 403 Forbidden\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " +
                                     std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
            SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
            return;
        }

        std::string action = "none";
        const size_t actPos = path.find("action=");
        if (actPos != std::string::npos) {
            const size_t actEnd = path.find('&', actPos);
            action = path.substr(actPos + 7, (actEnd == std::string::npos ? std::string::npos : actEnd - (actPos + 7)));
        }

        float value = 0.0f;
        const size_t valPos = path.find("value=");
        if (valPos != std::string::npos) {
            value = static_cast<float>(std::atof(path.c_str() + valPos + 6));
        }

        if (action == "spawn_all_crisis") {
            MunicipalGameEvent resolveEv{};
            resolveEv.type = MunicipalGameEventType::IncidentResolve;
            resolveEv.id = 0;
            EnqueueMunicipalEvent(resolveEv);

            for (size_t i = 0; i < 8; ++i) {
                MunicipalGameEvent ev{};
                ev.type = MunicipalGameEventType::RoadblockPolice;
                ev.id = s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
                ev.x = k_policeBarricadePoints[i].x; ev.y = k_policeBarricadePoints[i].y; ev.z = k_policeBarricadePoints[i].z;
                ev.param = 300000;
                strncpy_s(ev.text, "SWAT Roadblock", sizeof(ev.text) - 1);
                EnqueueMunicipalEvent(ev);
            }
            for (size_t i = 0; i < 12; ++i) {
                MunicipalGameEvent ev{};
                ev.type = MunicipalGameEventType::StrikeProtest;
                ev.id = s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
                ev.x = k_civilRiotPoints[i].x; ev.y = k_civilRiotPoints[i].y; ev.z = k_civilRiotPoints[i].z;
                ev.param = 300000;
                strncpy_s(ev.text, "Civil Protest", sizeof(ev.text) - 1);
                EnqueueMunicipalEvent(ev);
            }
            for (size_t i = 0; i < 9; ++i) {
                MunicipalGameEvent ev{};
                ev.type = MunicipalGameEventType::IncidentSpawn;
                ev.id = s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
                ev.x = k_gangWarPoints[i].x; ev.y = k_gangWarPoints[i].y; ev.z = k_gangWarPoints[i].z;
                ev.param = 300000;
                strncpy_s(ev.text, "Gang Shootout", sizeof(ev.text) - 1);
                EnqueueMunicipalEvent(ev);
            }
            s_crimeRate.store(85.0f, std::memory_order_relaxed);
            s_socialUnrest.store(75.0f, std::memory_order_relaxed);
            AddMunicipalLog("ALERT: Total State Crisis deployed. 29 Hotspots active across Los Santos.");
        } else if (action == "spike_crime") {
            s_crimeRate.store(75.0f, std::memory_order_relaxed);
            AddMunicipalLog("DEBUG: Crime rate spiked to 75%");
        } else if (action == "set_crime") {
            value = std::clamp(value, 0.0f, 100.0f);
            s_crimeRate.store(value, std::memory_order_relaxed);
            AddMunicipalLog("DEBUG: Crime rate manually set to %.1f%%", value);
        } else if (action == "set_unrest") {
            value = std::clamp(value, 0.0f, 100.0f);
            s_socialUnrest.store(value, std::memory_order_relaxed);
            s_publicUnrest.store(value, std::memory_order_relaxed);
            AddMunicipalLog("DEBUG: Social unrest manually set to %.1f%%", value);
        } else if (action == "bankruptcy") {
            s_cityTreasury.store(0, std::memory_order_relaxed);
            AddMunicipalLog("DEBUG: Treasury emptied ($0)");
        } else if (action == "inject_budget") {
            s_cityTreasury.fetch_add(100000, std::memory_order_relaxed);
            AddMunicipalLog("DEBUG: Injected +$100,000");
        } else if (action == "drain_budget") {
            s_cityTreasury.fetch_sub(100000, std::memory_order_relaxed);
            AddMunicipalLog("DEBUG: Drained -$100,000");
        } else if (action == "spawn_roadblock") {
            static size_t s_dbgRoadblockIdx = 0;
            const CVector pt = k_policeBarricadePoints[s_dbgRoadblockIdx % 8];
            s_dbgRoadblockIdx++;
            MunicipalGameEvent ev{};
            ev.type = MunicipalGameEventType::RoadblockPolice;
            ev.id = s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
            ev.x = pt.x; ev.y = pt.y; ev.z = pt.z;
            ev.param = 600000;
            strncpy_s(ev.text, "Police Roadblock", sizeof(ev.text) - 1);
            EnqueueMunicipalEvent(ev);
            AddMunicipalLog("DEBUG: Spawned SWAT Roadblock at (%.1f, %.1f)", pt.x, pt.y);
        } else if (action == "spawn_strike" || action == "union_riot" || action == "strike") {
            Rioters::ForceCityHallRiot();
            AddMunicipalLog("DEBUG: Force Triggered City Hall Riot / Strike");
        } else if (action == "spawn_incident") {
            MunicipalGameEvent ev{};
            ev.type = MunicipalGameEventType::IncidentSpawn;
            ev.id = s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
            ev.x = k_gantonGroveEnd.x; ev.y = k_gantonGroveEnd.y; ev.z = k_gantonGroveEnd.z;
            ev.param = 600000;
            strncpy_s(ev.text, "Ganton Riot", sizeof(ev.text) - 1);
            EnqueueMunicipalEvent(ev);
            AddMunicipalLog("DEBUG: Spawned Ganton Riot incident");
        } else if (action == "trigger_crisis") {
            uint8_t crisisToTrigger = 1;
            const size_t cidPos = path.find("crisis_id=");
            if (cidPos != std::string::npos) {
                int parsed = atoi(path.c_str() + cidPos + 10);
                if (parsed >= 1 && parsed <= 15) crisisToTrigger = static_cast<uint8_t>(parsed);
            }
            s_pendingCrisisTriggerId.store(crisisToTrigger, std::memory_order_relaxed);
            AddMunicipalLog("DEBUG: Queued Crisis ID %u", crisisToTrigger);
        } else if (action == "resolve_crisis") {
            s_activeCrisis.durationMs.store(0, std::memory_order_relaxed);
            AddMunicipalLog("CRISIS RESOLUTION QUEUED: Via debug request.");
        } else {
            AddMunicipalLog(("DEBUG: Action '" + action + "' processed").c_str());
        }

        const std::string json = "{\"status\":\"ok\",\"action\":\"" + action + "\",\"executed_in_memory\":true}";
        const std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " +
                                 std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (isPost && path.rfind("/api/municipal", 0) == 0) {
        std::string action = "none";
        if (body) {
            if (const char* actP = strstr(body, "\"action\"")) {
                if (const char* colon = strchr(actP, ':')) {
                    if (const char* q1 = strchr(colon, '"')) {
                        if (const char* q2 = strchr(q1 + 1, '"')) {
                            action.assign(q1 + 1, q2 - (q1 + 1));
                        }
                    }
                }
            }
        }
        if (action == "none") {
            const size_t actPos = path.find("action=");
            if (actPos != std::string::npos) {
                const size_t actEnd = path.find('&', actPos);
                action = path.substr(actPos + 7, (actEnd == std::string::npos ? std::string::npos : actEnd - (actPos + 7)));
            }
        }

        if (action == "spawn_all_crisis") {
            MunicipalGameEvent resolveEv{};
            resolveEv.type = MunicipalGameEventType::IncidentResolve;
            resolveEv.id = 0;
            EnqueueMunicipalEvent(resolveEv);

            for (size_t i = 0; i < 8; ++i) {
                MunicipalGameEvent ev{};
                ev.type = MunicipalGameEventType::RoadblockPolice;
                ev.id = s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
                ev.x = k_policeBarricadePoints[i].x; ev.y = k_policeBarricadePoints[i].y; ev.z = k_policeBarricadePoints[i].z;
                ev.param = 300000;
                strncpy_s(ev.text, "SWAT Roadblock", sizeof(ev.text) - 1);
                EnqueueMunicipalEvent(ev);
            }
            for (size_t i = 0; i < 12; ++i) {
                MunicipalGameEvent ev{};
                ev.type = MunicipalGameEventType::StrikeProtest;
                ev.id = s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
                ev.x = k_civilRiotPoints[i].x; ev.y = k_civilRiotPoints[i].y; ev.z = k_civilRiotPoints[i].z;
                ev.param = 300000;
                strncpy_s(ev.text, "Civil Protest", sizeof(ev.text) - 1);
                EnqueueMunicipalEvent(ev);
            }
            for (size_t i = 0; i < 9; ++i) {
                MunicipalGameEvent ev{};
                ev.type = MunicipalGameEventType::IncidentSpawn;
                ev.id = s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
                ev.x = k_gangWarPoints[i].x; ev.y = k_gangWarPoints[i].y; ev.z = k_gangWarPoints[i].z;
                ev.param = 300000;
                strncpy_s(ev.text, "Gang Shootout", sizeof(ev.text) - 1);
                EnqueueMunicipalEvent(ev);
            }
            s_crimeRate.store(85.0f, std::memory_order_relaxed);
            s_socialUnrest.store(75.0f, std::memory_order_relaxed);
            AddMunicipalLog("ALERT: Total State Crisis deployed. 29 Hotspots active across Los Santos.");
        } else if (action == "veto") {
            s_cityTreasury.fetch_add(25000, std::memory_order_relaxed);
            s_emergencyState.store(0, std::memory_order_relaxed);
            AddMunicipalLog("MAYOR VETO: Overrode crisis (+ $25,000)");
        } else if (action == "curfew") {
            bool nextCurfew = !s_curfewActive.load(std::memory_order_relaxed);
            s_curfewActive.store(nextCurfew, std::memory_order_relaxed);
            MunicipalGameEvent ev{};
            ev.type = MunicipalGameEventType::CurfewDensity;
            ev.param = nextCurfew ? 1 : 0;
            EnqueueMunicipalEvent(ev);
            AddMunicipalLog(nextCurfew ? "CURFEW ENACTED: Police decree issued" : "CURFEW LIFTED: Civil traffic restored");
        } else if (action == "bribe_accept") {
            std::lock_guard<std::mutex> lock(s_bribeOfferMutex);
            if (s_activeBribeOffer.active) {
                s_cityTreasury.fetch_add(s_activeBribeOffer.amount, std::memory_order_relaxed);
                float curCrime = s_crimeRate.load(std::memory_order_relaxed);
                s_crimeRate.store((std::min)(100.0f, curCrime + s_activeBribeOffer.crimeDelta), std::memory_order_relaxed);
                s_activeBribeOffer.active = false;
                AddMunicipalLog("BRIBE ACCEPTED: Shadow funds deposited to treasury");
            }
        } else if (action == "bribe_reject") {
            std::lock_guard<std::mutex> lock(s_bribeOfferMutex);
            if (s_activeBribeOffer.active) {
                s_activeBribeOffer.active = false;
                AddMunicipalLog("BRIBE REJECTED: Mayor refused syndicate bribe");
            }
        } else if (action == "strike_subsidize") {
            s_cityTreasury.fetch_sub(50000, std::memory_order_relaxed);
            float curUnrest = s_socialUnrest.load(std::memory_order_relaxed);
            s_socialUnrest.store((std::max)(0.0f, curUnrest - 20.0f), std::memory_order_relaxed);
            {
                std::lock_guard<std::mutex> lock(s_unionStrikeMutex);
                s_activeUnionStrike.active = false;
            }
            MunicipalGameEvent ev{};
            ev.type = MunicipalGameEventType::StrikeDisband;
            EnqueueMunicipalEvent(ev);
            AddMunicipalLog("UNION SUBSIDY: $50,000 paid to strike fund. Workers dispersed.");
        } else if (action == "spike_crime") {
            s_crimeRate.store(75.0f, std::memory_order_relaxed);
            AddMunicipalLog("DEBUG: Crime rate spiked to 75%");
        } else if (action == "set_crime") {
            float val = 0.0f;
            if (body) {
                if (const char* vP = strstr(body, "\"value\"")) {
                    if (const char* col = strchr(vP, ':')) {
                        val = static_cast<float>(std::atof(col + 1));
                    }
                }
            }
            val = std::clamp(val, 0.0f, 100.0f);
            s_crimeRate.store(val, std::memory_order_relaxed);
            AddMunicipalLog("DEBUG: Crime rate set via POST to %.1f%%", val);
        } else if (action == "set_unrest") {
            float val = 0.0f;
            if (body) {
                if (const char* vP = strstr(body, "\"value\"")) {
                    if (const char* col = strchr(vP, ':')) {
                        val = static_cast<float>(std::atof(col + 1));
                    }
                }
            }
            val = std::clamp(val, 0.0f, 100.0f);
            s_socialUnrest.store(val, std::memory_order_relaxed);
            s_publicUnrest.store(val, std::memory_order_relaxed);
            AddMunicipalLog("DEBUG: Social unrest set via POST to %.1f%%", val);
        } else if (action == "bankruptcy") {
            s_cityTreasury.store(0, std::memory_order_relaxed);
            AddMunicipalLog("DEBUG: Treasury emptied ($0)");
        } else if (action == "inject_budget") {
            s_cityTreasury.fetch_add(100000, std::memory_order_relaxed);
            AddMunicipalLog("DEBUG: Injected +$100,000");
        } else if (action == "drain_budget") {
            s_cityTreasury.fetch_sub(100000, std::memory_order_relaxed);
            AddMunicipalLog("DEBUG: Drained -$100,000");
        } else if (action == "spawn_roadblock") {
            static size_t s_postRoadblockIdx = 0;
            const CVector pt = k_policeBarricadePoints[s_postRoadblockIdx % 8];
            s_postRoadblockIdx++;
            MunicipalGameEvent ev{};
            ev.type = MunicipalGameEventType::RoadblockPolice;
            ev.id = s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
            ev.x = pt.x; ev.y = pt.y; ev.z = pt.z;
            ev.param = 600000;
            strncpy_s(ev.text, "Police Roadblock", sizeof(ev.text) - 1);
            EnqueueMunicipalEvent(ev);
            AddMunicipalLog("WEB POST: Spawned SWAT Roadblock");
        } else if (action == "spawn_strike" || action == "union_riot" || action == "strike") {
            Rioters::ForceCityHallRiot();
            AddMunicipalLog("WEB POST: Force Triggered City Hall Riot / Strike");
        } else if (action == "spawn_incident") {
            MunicipalGameEvent ev{};
            ev.type = MunicipalGameEventType::IncidentSpawn;
            ev.id = s_nextMuniIncidentId.fetch_add(1, std::memory_order_relaxed);
            ev.x = k_gantonGroveEnd.x; ev.y = k_gantonGroveEnd.y; ev.z = k_gantonGroveEnd.z;
            ev.param = 600000;
            strncpy_s(ev.text, "Ganton Riot", sizeof(ev.text) - 1);
            EnqueueMunicipalEvent(ev);
            AddMunicipalLog("WEB POST: Spawned Ganton Riot incident");
        } else if (action == "trigger_crisis") {
            uint8_t crisisToTrigger = 1;
            if (body) {
                if (const char* cidP = strstr(body, "\"crisis_id\"")) {
                    if (const char* colon = strchr(cidP, ':')) {
                        int parsed = atoi(colon + 1);
                        if (parsed >= 1 && parsed <= 15) crisisToTrigger = static_cast<uint8_t>(parsed);
                    }
                }
            }
            s_pendingCrisisTriggerId.store(crisisToTrigger, std::memory_order_relaxed);
            AddMunicipalLog("WEB POST: Queued Crisis ID %u", crisisToTrigger);
        } else if (action == "resolve_crisis") {
            s_activeCrisis.durationMs.store(0, std::memory_order_relaxed);
            AddMunicipalLog("CRISIS RESOLUTION QUEUED: Via web post.");
        } else {
            AddMunicipalLog(("WEB POST: Action '" + action + "' processed").c_str());
        }

        const std::string json = "{\"status\":\"ok\",\"action\":\"" + action + "\",\"executed_in_memory\":true}";
        const std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " +
                                 std::to_string(json.size()) + "\r\nConnection: close\r\n\r\n" + json;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (path.rfind("/api/municipal", 0) == 0) {
        const int64_t treasury = s_cityTreasury.load(std::memory_order_relaxed);
        const float crimeRate = s_crimeRate.load(std::memory_order_relaxed);
        const float socialUnrest = s_socialUnrest.load(std::memory_order_relaxed);
        const uint32_t totalHandled = s_totalIncidentsHandled.load(std::memory_order_relaxed);
        const bool curfewActive = s_curfewActive.load(std::memory_order_relaxed);
        const int emState = s_emergencyState.load(std::memory_order_relaxed);
        const bool impeachment = s_impeachmentTriggered.load(std::memory_order_relaxed);
        const uint32_t fuelStock = s_sfFuelStock.load(std::memory_order_relaxed);
        const bool fuelCrit = s_fuelCrisisActive.load(std::memory_order_relaxed);
        const uint32_t currentMs = CTimer::m_snTimeInMilliseconds;

        // Serialize s_incidents registry directly from memory
        std::string incidentsJson = "[";
        bool firstInc = true;
        for (size_t i = 0; i < k_maxIncidents; ++i) {
            const auto& inc = s_incidents[i];
            if (!inc.active) continue;

            int expiresInMs = 0;
            if (inc.durationMs > 0) {
                const uint32_t elapsed = currentMs - inc.spawnTimeMs;
                if (inc.durationMs > elapsed) {
                    expiresInMs = static_cast<int>(inc.durationMs - elapsed);
                }
            }

            const char* typeStr = "Gang Shootout";
            const char* locStr = "Los Santos";
            uint8_t sev = 3;
            if (inc.type == IncidentType::ROADBLOCK) {
                typeStr = "SWAT Roadblock";
                locStr = "Pershing Square";
                sev = 2;
            } else if (inc.type == IncidentType::UNION_STRIKE) {
                typeStr = "Civil Strike";
                locStr = "City Hall";
                sev = 4;
            } else if (inc.type == IncidentType::GANTON_RIOT) {
                typeStr = "Gang Shootout";
                locStr = "East Los Santos";
                sev = 3;
            }

            const char* statusStr = inc.inCombat ? "COMBAT" : (inc.isMaterialized ? "ACTIVE" : "DISPATCHED");

            char incBuf[512];
            snprintf(incBuf, sizeof(incBuf),
                "%s{\"id\":%u,\"type\":\"%s\",\"location\":\"%s\",\"severity\":%u,\"status\":\"%s\","
                "\"slot\":%d,\"x\":%.2f,\"y\":%.2f,\"z\":%.2f,\"radius\":12.00,\"active\":true,"
                "\"materialized\":%s,\"in_combat\":%s,\"expires_in_ms\":%d}",
                (firstInc ? "" : ","),
                inc.id,
                typeStr,
                locStr,
                sev,
                statusStr,
                static_cast<int>(i),
                inc.pos.x, inc.pos.y, inc.pos.z,
                inc.isMaterialized ? "true" : "false",
                inc.inCombat ? "true" : "false",
                expiresInMs
            );
            incidentsJson += incBuf;
            firstInc = false;
        }
        incidentsJson += "]";

        std::string logsJson = "[";
        {
            std::lock_guard<std::mutex> lock(s_municipalLogMutex);
            const size_t total = (s_municipalLogHead < k_maxMunicipalLogs) ? s_municipalLogHead : k_maxMunicipalLogs;
            bool first = true;
            for (size_t i = 0; i < total; ++i) {
                const size_t idx = (s_municipalLogHead - 1 - i) % k_maxMunicipalLogs;
                char escapedMsg[256];
                size_t ep = 0;
                for (const char* p = s_municipalLogs[idx].message; *p && ep < sizeof(escapedMsg) - 2; ++p) {
                    if (*p == '"' || *p == '\\') escapedMsg[ep++] = '\\';
                    escapedMsg[ep++] = *p;
                }
                escapedMsg[ep] = '\0';

                char logBuf[512];
                snprintf(logBuf, sizeof(logBuf), "%s{\"message\":\"%s\"}", (first ? "" : ","), escapedMsg);
                logsJson += logBuf;
                first = false;
            }
        }
        logsJson += "]";

        char bribeBuf[512];
        {
            std::lock_guard<std::mutex> lock(s_bribeOfferMutex);
            snprintf(bribeBuf, sizeof(bribeBuf),
                "{\"active\":%s,\"id\":%u,\"syndicate\":\"%s\",\"desc\":\"%s\",\"amount\":%lld,\"crime_delta\":%.1f,\"unrest_delta\":%.1f,\"expires\":%u}",
                (s_activeBribeOffer.active ? "true" : "false"),
                s_activeBribeOffer.id,
                s_activeBribeOffer.syndicate,
                s_activeBribeOffer.description,
                static_cast<long long>(s_activeBribeOffer.amount),
                s_activeBribeOffer.crimeDelta,
                s_activeBribeOffer.unrestDelta,
                s_activeBribeOffer.expireTicks
            );
        }

        char strikeBuf[512];
        {
            std::lock_guard<std::mutex> lock(s_unionStrikeMutex);
            snprintf(strikeBuf, sizeof(strikeBuf),
                "{\"active\":%s,\"id\":%u,\"union\":\"%s\",\"location\":\"%s\",\"duration\":%u}",
                (s_activeUnionStrike.active ? "true" : "false"),
                s_activeUnionStrike.id,
                s_activeUnionStrike.unionName,
                s_activeUnionStrike.location,
                s_activeUnionStrike.durationTicks
            );
        }

        char crisisBuf[512];
        {
            const uint8_t cId = s_activeCrisis.activeId.load(std::memory_order_relaxed);
            const bool cActive = (cId != 0);
            uint32_t timeLeftSec = 0;
            if (cActive) {
                const uint32_t startMs = s_activeCrisis.startMs.load(std::memory_order_relaxed);
                const uint32_t durMs = s_activeCrisis.durationMs.load(std::memory_order_relaxed);
                const uint32_t elapsed = (currentMs >= startMs) ? (currentMs - startMs) : 0;
                if (durMs > elapsed) {
                    timeLeftSec = (durMs - elapsed) / 1000;
                }
            }
            char tTitle[64] = { 0 };
            char tHeadline[128] = { 0 };
            {
                std::lock_guard<std::mutex> lock(s_activeCrisis.titleMutex);
                strncpy_s(tTitle, s_activeCrisis.activeTitle, sizeof(tTitle) - 1);
                strncpy_s(tHeadline, s_activeCrisis.activeHeadline, sizeof(tHeadline) - 1);
            }
            char escTitle[128];
            size_t ep1 = 0;
            for (const char* p = tTitle; *p && ep1 < sizeof(escTitle) - 2; ++p) {
                if (*p == '"' || *p == '\\') escTitle[ep1++] = '\\';
                escTitle[ep1++] = *p;
            }
            escTitle[ep1] = '\0';

            char escHeadline[256];
            size_t ep2 = 0;
            for (const char* p = tHeadline; *p && ep2 < sizeof(escHeadline) - 2; ++p) {
                if (*p == '"' || *p == '\\') escHeadline[ep2++] = '\\';
                escHeadline[ep2++] = *p;
            }
            escHeadline[ep2] = '\0';

            snprintf(crisisBuf, sizeof(crisisBuf),
                "{\"active\":%s,\"id\":%u,\"title\":\"%s\",\"headline\":\"%s\",\"time_left_sec\":%u}",
                (cActive ? "true" : "false"),
                static_cast<unsigned int>(cId),
                escTitle,
                escHeadline,
                timeLeftSec
            );
        }

        const float sg = s_taxSalesGeneral.load(std::memory_order_relaxed);
        const float sl = s_taxSalesLuxury.load(std::memory_order_relaxed);
        const float hp = s_taxHaulerPort.load(std::memory_order_relaxed);
        const float cw = s_taxCorporateWealth.load(std::memory_order_relaxed);
        const int32_t cp = s_taxCitizenPoll.load(std::memory_order_relaxed);

        char metaBuf[1536];
        snprintf(metaBuf, sizeof(metaBuf),
            "{\"status\":\"ok\","
            "\"treasury\":%lld,"
            "\"crime_rate\":%.2f,"
            "\"social_unrest\":%.2f,"
            "\"is_emergency\":%s,"
            "\"curfew_active\":%s,"
            "\"emergency_state\":%d,"
            "\"impeachment_triggered\":%s,"
            "\"fuel_crisis\":%s,"
            "\"logistics\":{\"fuel_stock\":%u,\"is_critical\":%s},"
            "\"taxes\":{\"salesGeneral\":%.3f,\"salesLuxury\":%.3f,\"haulerPort\":%.3f,\"corporateWealth\":%.3f,\"citizenPoll\":%d,\"districtTaxes\":[%.1f,%.1f,%.1f,%.1f]},"
            "\"bribe_offer\":%s,"
            "\"strike_event\":%s,"
            "\"world_crisis\":%s,"
            "\"total_handled\":%u,"
            "\"incidents\":",
            static_cast<long long>(treasury),
            crimeRate,
            socialUnrest,
            (treasury <= 0 || crimeRate >= 75.0f) ? "true" : "false",
            curfewActive ? "true" : "false",
            emState,
            impeachment ? "true" : "false",
            (fuelCrit ? "true" : "false"),
            fuelStock, (fuelCrit ? "true" : "false"),
            sg, sl, hp, cw, cp,
            s_districtPollTax[0].load(std::memory_order_relaxed),
            s_districtPollTax[1].load(std::memory_order_relaxed),
            s_districtPollTax[2].load(std::memory_order_relaxed),
            s_districtPollTax[3].load(std::memory_order_relaxed),
            bribeBuf, strikeBuf, crisisBuf,
            totalHandled
        );

        const std::string fullJson = std::string(metaBuf) + incidentsJson + ",\"decisions_log\":" + logsJson + "}";
        const std::string resp = "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: application/json\r\n"
                                 "Access-Control-Allow-Origin: *\r\n"
                                 "Content-Length: " + std::to_string(fullJson.size()) + "\r\n"
                                 "Connection: close\r\n\r\n" + fullJson;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    // Route /api/status: Returns real-time coordinates, heading, speed, AI directive, stats & truck fleet
    else if (path.rfind("/api/status", 0) == 0) {
        // Read active truck snapshot wait-free
        const uint32_t snapIdx = g_truckSnapshotActiveIdx.load(std::memory_order_acquire);
        std::string trucksJson = "[";
        for (size_t i = 0; i < k_totalTruckCount; ++i) {
            const auto& t = g_truckSnapshots[snapIdx][i];
            const char* cargoStr = (t.cargoType == 1 ? "Timber" : (t.cargoType == 2 ? "Fuel/Oil" : (t.cargoType == 3 ? "Electronics" : (t.cargoType == 4 ? "Food" : "Empty"))));
            const char* stateStr = (t.state == TruckState::LOADING ? "LOADING" : (t.state == TruckState::RESTING ? "RESTING" : (t.state == TruckState::UNLOADING ? "UNLOADING" : (t.state == TruckState::BROKEN_DOWN ? "BROKEN_DOWN" : (t.state == TruckState::DESTROYED ? "DESTROYED" : (t.state == TruckState::INSPECTION ? "INSPECTION" : "EN_ROUTE"))))));
            const float roundedFuel = std::round(t.fuel * 10.0f) / 10.0f;
            const float roundedFatigue = std::round(t.fatigue * 10.0f) / 10.0f;
            const uint8_t cId = t.companyId % 3;
            char itemBuf[512];
            snprintf(itemBuf, sizeof(itemBuf),
                "%s{\"id\":%u,\"x\":%.1f,\"y\":%.1f,\"z\":%.1f,\"heading\":%.1f,\"speed\":%.1f,\"materialized\":%s,\"state\":\"%s\","
                "\"destroyed\":%s,"
                "\"fuel\":%.1f,\"fatigue\":%.1f,\"cargo\":\"%s\",\"cargo_tons\":%d,\"deliveries\":%d,"
                "\"company_id\":%u,\"company_name\":\"%s\",\"company_color\":\"%s\",\"driver_name\":\"%s\",\"driver_wallet\":%u,\"deadline_ticks\":%u,"
                "\"xp\":%u,\"level\":%u}",
                (i > 0 ? "," : ""),
                t.id, t.x, t.y, t.z, t.heading, t.speed,
                (t.isMaterialized ? "true" : "false"),
                stateStr,
                (t.state == TruckState::DESTROYED ? "true" : "false"),
                roundedFuel, roundedFatigue, cargoStr, t.cargoWeightTons, t.deliveriesDone,
                cId,
                k_companies[cId].name,
                k_companies[cId].color,
                (t.driverName ? t.driverName : "Driver"),
                t.driverWallet,
                t.deadlineTicks,
                t.experience,
                t.skillLevel
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
                "%s{\"id\":%u,\"name\":\"%s\",\"balance\":%lld,\"color\":\"%s\",\"baseHub\":\"%s\",\"ownedAssets\":%d,\"truckCount\":8}",
                (c > 0 ? "," : ""),
                k_companies[c].id, k_companies[c].name,
                static_cast<long long>(s_companyBalances[c].load(std::memory_order_relaxed)),
                k_companies[c].color, k_companies[c].baseHub, ownedAssets
            );
            companiesJson += cBuf;
        }
        companiesJson += "]";

        std::string assetsJson = "[";
        for (size_t a = 0; a < 3; ++a) {
            char aBuf[256];
            snprintf(aBuf, sizeof(aBuf),
                "%s{\"id\":%u,\"name\":\"%s\",\"cost\":%u,\"owner\":%d}",
                (a > 0 ? "," : ""),
                k_assets[a].id,
                k_assets[a].name,
                k_assets[a].cost,
                static_cast<int>(s_assetOwners[a].load(std::memory_order_relaxed))
            );
            assetsJson += aBuf;
        }
        assetsJson += "]";

        char sfSuppliesBuf[256];
        snprintf(sfSuppliesBuf, sizeof(sfSuppliesBuf),
            "{\"timber\":%u,\"fuel\":%u,\"electronics\":%u,\"food\":%u}",
            s_sfTimberStock.load(std::memory_order_relaxed),
            s_sfFuelStock.load(std::memory_order_relaxed),
            s_sfElectronicsStock.load(std::memory_order_relaxed),
            s_sfFoodStock.load(std::memory_order_relaxed)
        );

        char portSuppliesBuf[256];
        snprintf(portSuppliesBuf, sizeof(portSuppliesBuf),
            "{\"food\":%u,\"fuel\":%u}",
            s_portFoodStock.load(std::memory_order_relaxed),
            s_portFuelStock.load(std::memory_order_relaxed)
        );

        char headerJsonBuf[2048];
        snprintf(headerJsonBuf, sizeof(headerJsonBuf),
            "{"
            "\"status\":\"ok\","
            "\"companies\":%s,"
            "\"assets\":%s,"
            "\"sf_supplies\":%s,"
            "\"port_supplies\":%s,"
            "\"store_multiplier\":%.2f,"
            "\"fuel_multiplier\":%.2f,"
            "\"company_balance\":%lld,"
            "\"pos\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f},"
            "\"heading\":%.2f,"
            "\"speed\":%.3f,"
            "\"aiDirective\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f,\"id\":%u},"
            "\"stats\":{\"web\":%llu,\"telemetry\":%llu,\"directives\":%llu},"
            "\"webCommandsTotal\":%llu,"
            "\"telemetryTotal\":%llu,"
            "\"directivesTotal\":%llu,"
            "\"trucks\":",
            companiesJson.c_str(),
            assetsJson.c_str(),
            sfSuppliesBuf,
            portSuppliesBuf,
            s_storePriceMultiplier.load(std::memory_order_relaxed),
            s_fuelPriceMultiplier.load(std::memory_order_relaxed),
            static_cast<long long>(s_companyBalances[0].load(std::memory_order_relaxed)),
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
        );

        const std::string fullJson = std::string(headerJsonBuf) + trucksJson + "}";
        const std::string resp = "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: application/json\r\n"
                                 "Access-Control-Allow-Origin: *\r\n"
                                 "Content-Length: " + std::to_string(fullJson.size()) + "\r\n"
                                 "Connection: close\r\n\r\n" + fullJson;
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else if (path.rfind("/telemetry", 0) == 0) {
        size_t jsonLen = strlen(g_telemetryJsonBuffer);
        const char* pData = (jsonLen > 0) ? g_telemetryJsonBuffer : "{}";
        if (jsonLen == 0) jsonLen = 2;
        const std::string resp = "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: application/json\r\n"
                                 "Access-Control-Allow-Origin: *\r\n"
                                 "Content-Length: " + std::to_string(jsonLen) + "\r\n"
                                 "Connection: close\r\n\r\n" + std::string(pData, jsonLen);
        SendAll(clientSock, resp.c_str(), static_cast<int>(resp.size()));
    }
    else {
        const char resp404[] = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        send(clientSock, resp404, static_cast<int>(sizeof(resp404) - 1), 0);
    }
}

// =============================================================================
//  §8. Background HTTP Server Loop
// =============================================================================

static void WebWorkerLoop() {
    Logger::Log("[WebWorker] Winsock2 server starting on 0.0.0.0:8080...");

    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return;

    const SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET) {
        WSACleanup();
        return;
    }

    const BOOL opt = TRUE;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(8080);

    if (bind(listenSock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR ||
        listen(listenSock, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listenSock);
        WSACleanup();
        return;
    }

    g_listenSock.store(listenSock, std::memory_order_release);

    while (g_running.load(std::memory_order_relaxed)) {
        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(listenSock, &readFds);

        timeval tv{ 0, 50000 };
        const int selectRes = select(0, &readFds, nullptr, nullptr, &tv);

        if (selectRes > 0 && FD_ISSET(listenSock, &readFds)) {
            sockaddr_in clientAddr{};
            int clientLen = sizeof(clientAddr);
            const SOCKET clientSock = accept(listenSock, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
            if (clientSock != INVALID_SOCKET) {
                const DWORD rcvTimeout = 1000;
                setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&rcvTimeout), sizeof(rcvTimeout));
                HandleHttpClient(clientSock);
            }
        } else if (selectRes == SOCKET_ERROR) {
            break;
        }
    }

    g_listenSock.store(INVALID_SOCKET, std::memory_order_release);
    closesocket(listenSock);
    WSACleanup();
}

// =============================================================================
//  §9b. Background Municipal Director (Deprecated in Phase 5 — Migrated to Lua)
// =============================================================================

static void MunicipalDirectorLoop() {
    // Deprecated: Municipal A-Life director & incident auto-generation migrated to Lua (Phase 5)
}

// =============================================================================
//  §10. Main Game Loop Coordinator
// =============================================================================

static bool     s_blipsRegistered = false;


// Native Pickup & Shop Marker Suppression:
// Globally suppress vanilla script checkpoints, 3D spheres, and native shop pickups in all interiors and triggers
static void SuppressNativeShopPickups() {
    // 1. Suppress Script Spheres (wireframe 3D spheres created by main.scm at shop counters)
    if (CTheScripts::ScriptSphereArray) {
        for (unsigned int i = 0; i < 16; ++i) {
            CTheScripts::ScriptSphereArray[i].bUsed = 0;
        }
    }

    // 2. Suppress Script Checkpoints (main.scm script checkpoint array)
    if (CTheScripts::ScriptCheckpointArray) {
        for (unsigned int i = 0; i < 20; ++i) {
            CTheScripts::ScriptCheckpointArray[i].bUsed = 0;
        }
    }

    // 3. Suppress 3D Markers (red glowing cylinder at Ammu-Nation / shop counters)
    for (unsigned int i = 0; i < 32; ++i) {
        C3dMarker& marker = C3dMarkers::m_aMarkerArray[i];
        if (marker.m_bIsUsed && ((marker.m_nIdentifier >= 1 && marker.m_nIdentifier <= 20) || (marker.m_nIdentifier >= 1000 && marker.m_nIdentifier < 2050))) {
            continue; // Protect custom 20 retail store 3D markers (IDs 1..20)
        }
        if (marker.m_bIsUsed && marker.m_mat.pos.z > 900.0f) {
            marker.m_bIsUsed = false;
            marker.m_nType = 0;
        }
    }

    // 4. Suppress Native Shop Checkpoints / Cylinders (CCheckpoint array at counters)
    if (CCheckpoints::m_aCheckPtArray) {
        for (unsigned int i = 0; i < 32; ++i) {
            CCheckpoint& cp = CCheckpoints::m_aCheckPtArray[i];
            if (cp.m_bIsUsed) {
                if (cp.m_nIdentifier >= 200 && cp.m_nIdentifier < 220) {
                    continue; // Protect custom retail store checkpoints (IDs 200..219)
                }
                // Check if inside interior space (Z > 900.0f)
                if (cp.m_vecPosition.z > 900.0f) {
                    cp.m_bIsUsed = false;
                    cp.m_nType = 0;
                    cp.m_bMustBeRenderedThisFrame = false;
                }
            }
        }
    }

    // 4. Suppress Native Pickups (weapons, body armor, health pickups in shops and around triggers)
    if (CPickups::aPickUps) {
        for (unsigned int i = 0; i < MAX_NUM_PICKUPS; ++i) {
            CPickup& pickup = CPickups::aPickUps[i];
            if (pickup.m_nPickupType == PICKUP_NONE) continue;

            const CVector pickPos = pickup.GetPosn();
            bool inRange = false;

            // All pickups inside interior space (Z > 900.0f)
            if (pickPos.z > 900.0f) {
                inRange = true;
            }

            // Suppress native pickups around all 20 retail stores (10.0m radius)
            if (!inRange) {
                for (size_t s = 0; s < 20; ++s) {
                    const float dx = pickPos.x - g_retailStores[s].posX;
                    const float dy = pickPos.y - g_retailStores[s].posY;
                    if ((dx * dx + dy * dy) <= (10.0f * 10.0f)) {
                        inRange = true;
                        break;
                    }
                }
            }

            if (inRange) {
                pickup.Remove();
                pickup.m_nPickupType = PICKUP_NONE;
                pickup.m_nFlags.bDisabled = 1;
            }
        }
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        InstallCrashHandler();
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        UninstallCrashHandler();
    }
    return TRUE;
}

class GTASystemCorePlugin {
public:
    GTASystemCorePlugin() {
        InstallCrashHandler();
        Logger::Init("gtasystemcore.log");
        EnsureMunicipalConfigDir();

        // Global native patch: disable CTheScripts::DrawScriptSpheres (0x4810E0)
        // Completely suppresses rendering of native 3D wireframe spheres
        plugin::patch::PutRetn(0x4810E0);

        Events::initEngineEvent += []() {
            StreamingFix::Init();
        };
        Events::initGameEvent += []() {
            StreamingFix::Init();
        };
        StreamingFix::Init();

        Events::drawHudEvent += []() {
            char tickerCopy[160];
            {
                std::lock_guard<std::mutex> lock(s_inGameTickerMutex);
                strncpy_s(tickerCopy, s_inGameTickerText, sizeof(tickerCopy) - 1);
            }

            if (tickerCopy[0] != '\0') {
                CFont::SetOrientation(ALIGN_CENTER);
                CFont::SetBackground(false, false);
                CFont::SetProportional(true);
                CFont::SetFontStyle(FONT_SUBTITLES);
                CFont::SetDropShadowPosition(1);
                CFont::SetDropColor(CRGBA(0, 0, 0, 220));

                const int emergency = s_emergencyState.load(std::memory_order_relaxed);
                const bool curfew = s_curfewActive.load(std::memory_order_relaxed);
                const bool impeached = s_impeachmentTriggered.load(std::memory_order_relaxed);

                if (impeached || emergency >= 3) {
                    CFont::SetColor(CRGBA(255, 60, 60, 255));
                } else if (emergency == 2 || curfew) {
                    CFont::SetColor(CRGBA(255, 180, 40, 255));
                } else if (emergency == 1) {
                    CFont::SetColor(CRGBA(255, 230, 80, 255));
                } else {
                    CFont::SetColor(CRGBA(56, 189, 248, 230));
                }

                const float scaleX = 0.35f * static_cast<float>(RsGlobal.maximumWidth) / 640.0f;
                const float scaleY = 0.70f * static_cast<float>(RsGlobal.maximumHeight) / 448.0f;
                CFont::SetScale(scaleX, scaleY);
                CFont::SetWrapx(static_cast<float>(RsGlobal.maximumWidth) - 20.0f);
                CFont::SetCentreSize(static_cast<float>(RsGlobal.maximumWidth));

                const float posX = static_cast<float>(RsGlobal.maximumWidth) * 0.5f;
                const float posY = 8.0f * static_cast<float>(RsGlobal.maximumHeight) / 448.0f;
                CFont::PrintString(posX, posY, tickerCopy);
            }
        };

        Events::gameProcessEvent += []() {
            StreamingFix::Update();

            if (!g_running.load(std::memory_order_relaxed)) {
                g_running.store(true, std::memory_order_release);
                InitCustomRoutes();
                g_webWorkerThread  = std::thread(WebWorkerLoop);
                g_aiDirectorThread = std::thread(AIDirectorLoop);
            }

            // Register permanent radar blips on plugin initialization
            if (!s_blipsRegistered) {
                s_blipsRegistered = true;
                for (size_t i = 0; i < 20; ++i) {
                    auto& store = g_retailStores[i];
                    const int handle = CRadar::SetCoordBlip(
                        BLIP_COORD,
                        CVector(store.posX, store.posY, k_retailStoreGroundZ[i]),
                        0,
                        BLIP_DISPLAY_BOTH,
                        nullptr
                    );
                    s_retailStoreBlips[i] = static_cast<uint32_t>(handle);

                    int sprite = RADAR_SPRITE_BURGERSHOT;
                    switch (store.categoryId) {
                    case 0: sprite = RADAR_SPRITE_BURGERSHOT; break;
                    case 1: sprite = RADAR_SPRITE_TRUCK; break;
                    case 2: sprite = RADAR_SPRITE_BURGERSHOT; break; // Agro Food & Produce
                    case 3: sprite = RADAR_SPRITE_AMMUGUN; break;
                    default: sprite = RADAR_SPRITE_BURGERSHOT; break;
                    }
                    CRadar::SetBlipSprite(handle, sprite);
                    CRadar::ChangeBlipScale(handle, 2);
                }
                Logger::Log("[Radar] Registered permanent blips for 20 Retail Stores.");
            }

            const uint32_t currentMs = CTimer::m_snTimeInMilliseconds;

            CPed* player = FindPlayerPed(0);
            if (player && CPools::ms_pPedPool && CPools::ms_pPedPool->IsObjectValid(player)) {
                CVector pos = player->GetPosition();
                g_playerPosX.store(pos.x, std::memory_order_relaxed);
                g_playerPosY.store(pos.y, std::memory_order_relaxed);
                g_playerPosZ.store(pos.z, std::memory_order_relaxed);
                g_isPlayerValid.store(true, std::memory_order_relaxed);
            } else {
                g_isPlayerValid.store(false, std::memory_order_relaxed);
            }

            // Принудительная выгрузка нативных скриптов магазинов оружия
            for (CRunningScript* script = CTheScripts::pActiveScripts; script;) {
                CRunningScript* nextScript = script->m_pNext;
                if (script->m_szName[0] != '\0') {
                    if (_strnicmp(script->m_szName, "ammu", 4) == 0 || _strnicmp(script->m_szName, "gun", 3) == 0) {
                        script->RemoveScriptFromList(&CTheScripts::pActiveScripts);
                        script->AddScriptToList(&CTheScripts::pIdleScripts);
                        script->m_bIsActive = false;
                    }
                }
                script = nextScript;
            }

            // Suppress native shop pickups and 3D markers every frame (ammu.sc restores markers continuously)
            SuppressNativeShopPickups();

            // Periodic MoonLoader State Export (every 500ms)
            static uint32_t s_lastMoonExportMs = 0;
            if (currentMs - s_lastMoonExportMs >= 500) {
                s_lastMoonExportMs = currentMs;
                ExportMoonLoaderJsonState();
            }

            // Periodic MoonLoader Purchase Requests Processing (every 100ms)
            static uint32_t s_lastMoonRequestMs = 0;
            if (currentMs - s_lastMoonRequestMs >= 100) {
                s_lastMoonRequestMs = currentMs;
                ProcessMoonLoaderRequests(player);
            }

            // =================================================================
            //  C++ Municipal Simulation Core (In-Game Thread)
            // =================================================================
            try {
                UpdateMunicipalEngine(player, currentMs);
            } catch (const std::exception& ex) {
                Logger::Log("[MunicipalEngine] Non-fatal exception caught: %s", ex.what());
            } catch (...) {
                Logger::Log("[MunicipalEngine] Non-fatal unknown exception caught in frame tick");
            }

            if (player) {
                CVector pos = player->m_pVehicle ? player->m_pVehicle->GetPosition() : player->GetPosition();
                CVector vel = player->m_pVehicle ? player->m_pVehicle->m_vecMoveSpeed : player->m_vecMoveSpeed;
                float headingRad = player->m_pVehicle ? player->m_pVehicle->GetHeading() : player->m_fHeadingCurrent;
                float headingDeg = headingRad * (180.0f / 3.14159265358979323846f);
                while (headingDeg < 0.0f) headingDeg += 360.0f;
                while (headingDeg >= 360.0f) headingDeg -= 360.0f;

                g_playerX.store(pos.x, std::memory_order_relaxed);
                g_playerY.store(pos.y, std::memory_order_relaxed);
                g_playerZ.store(pos.z, std::memory_order_relaxed);
                g_playerSpeed.store(vel.Magnitude(), std::memory_order_relaxed);
                g_playerHeading.store(headingDeg, std::memory_order_relaxed);

                // Throttle sending telemetry to AI Director (every 300 ms)
                static uint32_t s_lastTelemetryMs = 0;
                if (currentMs - s_lastTelemetryMs >= 300) {
                    s_lastTelemetryMs = currentMs;

                    const TelemetryState telemetry{
                        .x = pos.x,
                        .y = pos.y,
                        .z = pos.z,
                        .speed = vel.Magnitude(),
                        .timestamp = currentMs
                    };
                    g_aiTelemetryQueue.push(telemetry);
                }
            }

            UpdatePhysicalTrucks(player);

            // Drain Budget for web commands: up to 2 per frame
            constexpr size_t k_webDrainBudget = 2;
            size_t webDrained = 0;
            GameCommand cmd{};
            while (webDrained < k_webDrainBudget && g_webCommandQueue.pop(cmd)) {
                webDrained++;
                switch (cmd.type) {
                case CommandType::SetWeather:
                    CWeather::ForceWeatherNow(static_cast<short>(cmd.param));
                    break;
                case CommandType::RestoreVitals:
                    if (player) {
                        player->m_fHealth = player->m_fMaxHealth;
                        player->m_fArmour = 100.0f;
                        CHud::SetHelpMessage("~g~HEALTH & ARMOR RESTORED!~w~", true, false, false);
                    }
                    break;
                case CommandType::DisplayNotice:
                    CHud::SetHelpMessage("~g~Command Processed!~w~", true, false, false);
                    break;
                }
            }

            ProcessTruckDirectives();


            // =================================================================
            //  Retail Stores 3D Downward Cone Markers & Proximity Trigger
            // =================================================================
            bool inAnyShopZone = false;
            if (player) {
                const CVector pPos = player->m_pVehicle ? player->m_pVehicle->GetPosition() : player->GetPosition();

                for (size_t i = 0; i < 20; ++i) {
                    auto& store = g_retailStores[i];
                    const float dx = pPos.x - store.posX;
                    const float dy = pPos.y - store.posY;
                    const float dist2d = std::sqrt(dx * dx + dy * dy);

                    if (dist2d < 45.0f) {
                        uint8_t r = 34, g = 197, b = 94;
                        switch (store.categoryId) {
                        case 0:
                        case 2: r = 34;  g = 197; b = 94;  break; // Cat 0 & Cat 2 (Food/Agro): Green (34, 197, 94)
                        case 1: r = 245; g = 158; b = 11;  break; // Cat 1 (Fuel): Orange (245, 158, 11)
                        case 3: r = 56;  g = 189; b = 248; break; // Cat 3 (Tech/Ammo): Cyan (56, 189, 248)
                        default: r = 34; g = 197; b = 94;  break;
                        }

                        // Render the static yellow entry cone pointing downward
                        CVector markerPos(store.posX, store.posY, k_retailStoreGroundZ[i] + 1.2f);
                        C3dMarkers::PlaceMarker(
                            static_cast<unsigned int>(i + 1),
                            MARKER3D_ARROW,
                            markerPos,
                            1.2f,
                            r, g, b, 220,
                            0, 0.0f, 0,
                            0.0f, 0.0f, -1.0f,
                            false
                        );

                        // Render static ground corona glow at street level
                        CCoronas::RegisterCorona(
                            static_cast<unsigned int>(100 + i),
                            nullptr,
                            r, g, b, 220,
                            CVector(store.posX, store.posY, k_retailStoreGroundZ[i] + 0.15f),
                            1.5f,
                            50.0f,
                            CORONATYPE_SHINYSTAR,
                            FLARETYPE_NONE,
                            false, false, 0, 0.0f, false, 0.5f, 0, 15.0f, false, false
                        );
                    }

                    // Proximity trigger (radius <= 3.5f)
                    if (dist2d <= 3.5f) {
                        if (store.isRansacked.load(std::memory_order_relaxed) || store.localStock.load(std::memory_order_relaxed) == 0) {
                            CHud::SetHelpMessage("~r~Store Ransacked!~w~", true, false, false);
                        } else {
                            s_activeShopType.store(store.categoryId == 1 ? CustomShopType::GasStation : CustomShopType::Commercial, std::memory_order_release);
                            s_activeShopIndex.store(static_cast<uint8_t>(i), std::memory_order_release);
                            inAnyShopZone = true;
                            break;
                        }
                    }
                }
            }

            if (!inAnyShopZone) {
                s_activeShopType.store(CustomShopType::None, std::memory_order_release);
            }

            // CJ Real Estate Empire & Vehicle Tuning Controls:
            // F4: Buy unowned logistics facility for $50,000 to earn permanent recurring dividends
            if (GetAsyncKeyState(VK_F4) & 1) {
                if (player && player->m_pPlayerData) {
                    bool processed = false;
                    for (size_t a = 0; a < 3; ++a) {
                        if (s_assetOwners[a].load(std::memory_order_relaxed) == -1) {
                            if (player->m_pPlayerData->m_nMoney >= 50000) {
                                player->m_pPlayerData->m_nMoney -= 50000;
                                s_assetOwners[a].store(99, std::memory_order_release); // 99 = CJ / Player
                                char acqMsg[128];
                                snprintf(acqMsg, sizeof(acqMsg), "~g~EMPIRE EXPANSION: CJ acquired %s for $50,000!~w~", k_assets[a].name);
                                CHud::SetHelpMessage(acqMsg, true, false, false);
                                const CVector pPos = player->GetPosition();
                                Command<Commands::REPORT_MISSION_AUDIO_EVENT_AT_POSITION>(pPos.x, pPos.y, pPos.z, 0x1058);
                                processed = true;
                                break;
                            } else {
                                CHud::SetHelpMessage("~r~INSUFFICIENT FUNDS! Need $50,000 for facility acquisition.~w~", true, false, false);
                                processed = true;
                                break;
                            }
                        }
                    }
                    if (!processed) {
                        CHud::SetHelpMessage("~y~All logistics facilities are already owned!~w~", true, false, false);
                    }
                }
            }

            // F7: Custom Vehicle Tuning (Reinforced 2000 HP Chassis & 10x NOS) for $1,000
            if (GetAsyncKeyState(VK_F7) & 1) {
                if (player && player->m_pVehicle && player->m_pPlayerData) {
                    if (player->m_pPlayerData->m_nMoney >= 1000) {
                        player->m_pPlayerData->m_nMoney -= 1000;
                        player->m_pVehicle->m_fHealth = 2000.0f;
                        Command<Commands::ADD_VEHICLE_MOD>(player->m_pVehicle, 1010); // 10x Nitrous upgrade
                        CHud::SetHelpMessage("~g~VEHICLE TUNED: Reinforced 2000 HP Chassis & 10x NOS Installed!~w~", true, false, false);
                    } else {
                        CHud::SetHelpMessage("~r~INSUFFICIENT FUNDS! Need $1,000 for vehicle tuning.~w~", true, false, false);
                    }
                }
            }
        };

    }

    ~GTASystemCorePlugin() {
        if (g_running.exchange(false, std::memory_order_acq_rel)) {
            const SOCKET s = g_listenSock.exchange(INVALID_SOCKET, std::memory_order_acq_rel);
            if (s != INVALID_SOCKET) {
                closesocket(s);
            }

            WSACleanup();

            if (g_webWorkerThread.joinable()) g_webWorkerThread.join();
            if (g_aiDirectorThread.joinable()) g_aiDirectorThread.join();

            // Clean up registered radar blips
            for (size_t i = 0; i < 20; ++i) {
                if (s_retailStoreBlips[i] != 0) {
                    CRadar::ClearBlip(static_cast<int>(s_retailStoreBlips[i]));
                    s_retailStoreBlips[i] = 0;
                }
            }

            CleanupMunicipalSubsystem();

            CleanupAllPhysicalTrucks();
        }
        Logger::Shutdown();
    }

} g_systemCorePlugin;