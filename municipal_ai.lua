-- =============================================================================
--  municipal_ai.lua — Municipal A-Life Decision Engine for MoonLoader 5.1
--  GTA San Andreas v1.0 US · MoonLoader · Pure Lua (Zero Dependencies, No FFI)
-- =============================================================================

script_name("Municipal AI")
script_author("Antigravity Systems")
script_description("Municipal A-Life Director & Incident Scheduler via File Bridge")
script_version("1.1.0")

-- -----------------------------------------------------------------------------
-- §1. Paths & Logging
-- -----------------------------------------------------------------------------
local LOG_PATH = "moonloader/municipal_ai.log"
local CONFIG_DIR = "moonloader/config"
local HB_TMP_PATH = "moonloader/config/municipal_heartbeat.tmp"
local HB_FINAL_PATH = "moonloader/config/municipal_heartbeat.json"
local WORLD_STATE_PATH = "moonloader/config/municipal_world_state.json"
local INCIDENTS_STATE_PATH = "moonloader/config/municipal_incidents_state.json"
local CMD_TMP_PATH = "moonloader/config/municipal_commands.tmp"
local CMD_FINAL_PATH = "moonloader/config/municipal_commands.json"
local ECONOMY_STATE_PATH = "moonloader/config/economy_state.json"

local function log(fmt, ...)
    local msg = select("#", ...) > 0 and string.format(fmt, ...) or tostring(fmt)
    local line = string.format("[%s] %s\n", os.date("%Y-%m-%d %H:%M:%S"), msg)
    local f = io.open(LOG_PATH, "a")
    if f then
        f:write(line)
        f:close()
    end
end

-- -----------------------------------------------------------------------------
-- §2. Fast Zero-Dependency JSON Parser (Integrated from tycoon_shop.lua)
-- -----------------------------------------------------------------------------
local function parse_json(str)
    if not str or str == "" then return nil end
    local pos = 1
    local len = #str

    local function skip_whitespace()
        while pos <= len do
            local c = str:sub(pos, pos)
            if c == " " or c == "\t" or c == "\n" or c == "\r" then
                pos = pos + 1
            else
                break
            end
        end
    end

    local parse_value

    local function parse_string()
        pos = pos + 1
        local start = pos
        while pos <= len do
            local c = str:sub(pos, pos)
            if c == "\\" then
                pos = pos + 2
            elseif c == '"' then
                local s = str:sub(start, pos - 1)
                pos = pos + 1
                return s
            else
                pos = pos + 1
            end
        end
        return str:sub(start)
    end

    local function parse_number()
        local start = pos
        while pos <= len do
            local c = str:sub(pos, pos)
            if c:match("[%d%.%-%+eE]") then
                pos = pos + 1
            else
                break
            end
        end
        return tonumber(str:sub(start, pos - 1)) or 0
    end

    local function parse_array()
        pos = pos + 1
        local arr = {}
        skip_whitespace()
        if str:sub(pos, pos) == "]" then
            pos = pos + 1
            return arr
        end
        while pos <= len do
            table.insert(arr, parse_value())
            skip_whitespace()
            local c = str:sub(pos, pos)
            if c == "," then
                pos = pos + 1
                skip_whitespace()
            elseif c == "]" then
                pos = pos + 1
                return arr
            else
                break
            end
        end
        return arr
    end

    local function parse_object()
        pos = pos + 1
        local obj = {}
        skip_whitespace()
        if str:sub(pos, pos) == "}" then
            pos = pos + 1
            return obj
        end
        while pos <= len do
            skip_whitespace()
            if str:sub(pos, pos) ~= '"' then break end
            local key = parse_string()
            skip_whitespace()
            if str:sub(pos, pos) == ":" then
                pos = pos + 1
            end
            skip_whitespace()
            obj[key] = parse_value()
            skip_whitespace()
            local c = str:sub(pos, pos)
            if c == "," then
                pos = pos + 1
                skip_whitespace()
            elseif c == "}" then
                pos = pos + 1
                return obj
            else
                break
            end
        end
        return obj
    end

    parse_value = function()
        skip_whitespace()
        if pos > len then return nil end
        local c = str:sub(pos, pos)
        if c == '"' then
            return parse_string()
        elseif c == "{" then
            return parse_object()
        elseif c == "[" then
            return parse_array()
        elseif c == "t" and str:sub(pos, pos + 3) == "true" then
            pos = pos + 4
            return true
        elseif c == "f" and str:sub(pos, pos + 4) == "false" then
            pos = pos + 5
            return false
        elseif c == "n" and str:sub(pos, pos + 3) == "null" then
            pos = pos + 4
            return nil
        else
            return parse_number()
        end
    end

    return parse_value()
end

-- -----------------------------------------------------------------------------
-- §3. Municipal State M & Telemetry Readers (Phase 1 & Phase 3)
-- -----------------------------------------------------------------------------
local M = {
    treasury = 1000000,
    crime_rate = 25.0,
    social_unrest = 10.0,
    curfew_active = false,
    impeachment = false,
    fuel_crisis = false,
    total_handled = 0,
    next_incident_id = 1000,
    active_commands = {},
    spawn_cooldowns = {
        roadblock = 0,
        union_strike = 0,
        ganton_riot = 0,
    },
}

local s_world_state = nil
local s_incidents_state = nil
local s_logged_world_state_init = false
local s_logged_incidents_state_init = false

--- Reads world state exported by ASI engine (Phase 1)
function readWorldState()
    local f = io.open(WORLD_STATE_PATH, "r")
    if not f then return nil end
    local content = f:read("*a")
    f:close()
    if not content or content == "" then return nil end

    local data = parse_json(content)
    if data and data.player and data.metrics and data.game_time then
        s_world_state = data
        M.crime_rate = tonumber(data.metrics.crime_rate) or M.crime_rate
        M.social_unrest = tonumber(data.metrics.unrest_level) or M.social_unrest
        M.treasury = tonumber(data.metrics.treasury_balance) or M.treasury

        if not s_logged_world_state_init then
            s_logged_world_state_init = true
            log("[telemetry] World state connected: player=(%.1f, %.1f, %.1f) in_veh=%s wanted=%d crime=%.1f%% unrest=%.1f%% treasury=$%d time=%02d:%02d",
                data.player.x, data.player.y, data.player.z,
                tostring(data.player.in_vehicle),
                data.player.wanted_level,
                data.metrics.crime_rate,
                data.metrics.unrest_level,
                data.metrics.treasury_balance,
                data.game_time.hours,
                data.game_time.minutes)
        end
        return data
    end
    return nil
end

--- Reads active incidents registry snapshot exported by ASI engine (Phase 1)
function readIncidentsState()
    local f = io.open(INCIDENTS_STATE_PATH, "r")
    if not f then return nil end
    local content = f:read("*a")
    f:close()
    if not content or content == "" then return nil end

    local data = parse_json(content)
    if data and type(data) == "table" then
        s_incidents_state = data
        if not s_logged_incidents_state_init then
            s_logged_incidents_state_init = true
            log("[telemetry] Incidents state connected: %d slots tracked in registry", #data)
        end
        return data
    end
    return nil
end

--- Reads reactive incident feedback events from ASI engine (Phase 4 stub)
function readEventsFeedback()
    return nil
end

-- -----------------------------------------------------------------------------
-- §4. Incident Logic Helpers & Presets (Phase 3)
-- -----------------------------------------------------------------------------
local TYPE_NUM_MAP = {
    roadblock = 0,
    union_strike = 1,
    ganton_riot = 2,
}

local function countActiveByType(incidents_state, type_str)
    local count = 0
    local target_num = TYPE_NUM_MAP[type_str]

    if incidents_state and type(incidents_state) == "table" then
        for _, inc in ipairs(incidents_state) do
            if inc.active then
                if inc.type == type_str or (target_num ~= nil and inc.type == target_num) then
                    count = count + 1
                end
            end
        end
    end

    -- Include commands pending in active_commands
    for _, cmd in ipairs(M.active_commands) do
        if cmd.command == "spawn_incident" and cmd.type == type_str then
            count = count + 1
        end
    end

    return count
end

local function isPositionClear(incidents_state, x, y, min_dist)
    local min_dist_sq = min_dist * min_dist

    if incidents_state and type(incidents_state) == "table" then
        for _, inc in ipairs(incidents_state) do
            if inc.active and inc.pos then
                local ix = inc.pos.x or 0.0
                local iy = inc.pos.y or 0.0
                local dx = ix - x
                local dy = iy - y
                if (dx * dx + dy * dy) < min_dist_sq then
                    return false
                end
            end
        end
    end

    for _, cmd in ipairs(M.active_commands) do
        if cmd.command == "spawn_incident" and cmd.x and cmd.y then
            local dx = cmd.x - x
            local dy = cmd.y - y
            if (dx * dx + dy * dy) < min_dist_sq then
                return false
            end
        end
    end

    return true
end

local ROADBLOCK_PRESETS = {
    { name = "Ganton",   x = 2480.0, y = -1660.0, z = 13.0 },
    { name = "Idlewood", x = 2080.0, y = -1760.0, z = 13.5 },
    { name = "Downtown", x = 1450.0, y = -1650.0, z = 13.5 },
}

local function chooseRoadblockPreset(incidents_state)
    for _, preset in ipairs(ROADBLOCK_PRESETS) do
        if isPositionClear(incidents_state, preset.x, preset.y, 15.0) then
            return preset
        end
    end
    return nil
end

local function get_fuel_stock()
    local f = io.open(ECONOMY_STATE_PATH, "r")
    if f then
        local content = f:read("*a")
        f:close()
        if content and content ~= "" then
            local data = parse_json(content)
            if data and data.commodities and data.commodities.fuel then
                return tonumber(data.commodities.fuel) or 40
            end
        end
    end
    return 40
end

local function isPlayerTooClose(x, y, min_dist)
    if s_world_state and s_world_state.player then
        local px = s_world_state.player.x or 0.0
        local py = s_world_state.player.y or 0.0
        local dx = px - x
        local dy = py - y
        return (dx * dx + dy * dy) < (min_dist * min_dist)
    end
    return false
end

local function getPlayerDist(x, y)
    if s_world_state and s_world_state.player then
        local px = s_world_state.player.x or 0.0
        local py = s_world_state.player.y or 0.0
        local dx = px - x
        local dy = py - y
        return math.sqrt(dx * dx + dy * dy)
    end
    return 9999.0
end

-- -----------------------------------------------------------------------------
-- §5. Incident Decision Engine (evaluateSpawn & evaluateCleanup)
-- -----------------------------------------------------------------------------
local function evaluateSpawn(current_ms)
    if not s_world_state or not s_incidents_state then
        return
    end

    -- Priority 1: GANTON_RIOT (crime_rate > 75, max 1, cooldown 180s)
    if M.crime_rate > 75.0 then
        local count = countActiveByType(s_incidents_state, "ganton_riot")
        if count >= 1 then
            log("[eval] SKIP ganton_riot: max active reached (%d/1)", count)
        elseif current_ms < M.spawn_cooldowns.ganton_riot then
            local rem = math.ceil((M.spawn_cooldowns.ganton_riot - current_ms) / 1000)
            log("[eval] SKIP ganton_riot: on cooldown (%d sec left)", rem)
        else
            local pos = { x = 2490.0, y = -1670.0, z = 13.3 }
            if not isPositionClear(s_incidents_state, pos.x, pos.y, 15.0) then
                log("[eval] SKIP ganton_riot: location occupied")
            elseif isPlayerTooClose(pos.x, pos.y, 15.0) then
                log("[eval] SKIP ganton_riot: player too close (%.1fm < 15.0m)", getPlayerDist(pos.x, pos.y))
            else
                local id = M.next_incident_id
                M.next_incident_id = M.next_incident_id + 1
                table.insert(M.active_commands, {
                    command = "spawn_incident",
                    type = "ganton_riot",
                    id = id,
                    x = pos.x,
                    y = pos.y,
                    z = pos.z,
                    duration_ms = 180000,
                })
                M.spawn_cooldowns.ganton_riot = current_ms + (180 * 1000)
                log("[eval] SPAWN ganton_riot id=%u pos=(%.1f, %.1f, %.1f) (crime=%.1f%% > 75%%)",
                    id, pos.x, pos.y, pos.z, M.crime_rate)
                return
            end
        end
    end

    -- Priority 2: UNION_STRIKE (social_unrest > 50, max 1, cooldown 120s)
    if M.social_unrest > 50.0 then
        local count = countActiveByType(s_incidents_state, "union_strike")
        if count >= 1 then
            log("[eval] SKIP union_strike: max active reached (%d/1)", count)
        elseif current_ms < M.spawn_cooldowns.union_strike then
            local rem = math.ceil((M.spawn_cooldowns.union_strike - current_ms) / 1000)
            log("[eval] SKIP union_strike: on cooldown (%d sec left)", rem)
        else
            local pos = { x = 1481.0, y = -1745.0, z = 13.5 }
            if not isPositionClear(s_incidents_state, pos.x, pos.y, 15.0) then
                log("[eval] SKIP union_strike: location occupied")
            elseif isPlayerTooClose(pos.x, pos.y, 15.0) then
                log("[eval] SKIP union_strike: player too close (%.1fm < 15.0m)", getPlayerDist(pos.x, pos.y))
            else
                local id = M.next_incident_id
                M.next_incident_id = M.next_incident_id + 1
                table.insert(M.active_commands, {
                    command = "spawn_incident",
                    type = "union_strike",
                    id = id,
                    x = pos.x,
                    y = pos.y,
                    z = pos.z,
                    duration_ms = 120000,
                })
                M.spawn_cooldowns.union_strike = current_ms + (120 * 1000)
                log("[eval] SPAWN union_strike id=%u pos=(%.1f, %.1f, %.1f) (unrest=%.1f%% > 50%%)",
                    id, pos.x, pos.y, pos.z, M.social_unrest)
                return
            end
        end
    end

    -- Priority 3: ROADBLOCK (crime_rate > 60 AND fuel_stock > 15, max 3, cooldown 30s)
    local fuel_stock = get_fuel_stock()
    if M.crime_rate > 60.0 and fuel_stock > 15 then
        local count = countActiveByType(s_incidents_state, "roadblock")
        if count >= 3 then
            log("[eval] SKIP roadblock: max active reached (%d/3)", count)
        elseif current_ms < M.spawn_cooldowns.roadblock then
            local rem = math.ceil((M.spawn_cooldowns.roadblock - current_ms) / 1000)
            log("[eval] SKIP roadblock: on cooldown (%d sec left)", rem)
        else
            local preset = chooseRoadblockPreset(s_incidents_state)
            if not preset then
                log("[eval] SKIP roadblock: no clear presets available")
            elseif isPlayerTooClose(preset.x, preset.y, 15.0) then
                log("[eval] SKIP roadblock (%s): player too close (%.1fm < 15.0m)",
                    preset.name, getPlayerDist(preset.x, preset.y))
            else
                local id = M.next_incident_id
                M.next_incident_id = M.next_incident_id + 1
                table.insert(M.active_commands, {
                    command = "spawn_incident",
                    type = "roadblock",
                    id = id,
                    x = preset.x,
                    y = preset.y,
                    z = preset.z,
                    duration_ms = 60000,
                })
                M.spawn_cooldowns.roadblock = current_ms + (30 * 1000)
                log("[eval] SPAWN roadblock (%s) id=%u pos=(%.1f, %.1f, %.1f) (crime=%.1f%% > 60%%, fuel=%d > 15)",
                    preset.name, id, preset.x, preset.y, preset.z, M.crime_rate, fuel_stock)
                return
            end
        end
    end
end

local function isCleanupPending(id)
    for _, cmd in ipairs(M.active_commands) do
        if cmd.command == "cleanup_incident" and cmd.id == id then
            return true
        end
    end
    return false
end

local function evaluateCleanup()
    if not s_incidents_state or type(s_incidents_state) ~= "table" then
        return
    end

    for _, inc in ipairs(s_incidents_state) do
        if inc.active and inc.id and inc.id > 0 then
            local is_roadblock = (inc.type == 0 or inc.type == "roadblock")
            local is_union = (inc.type == 1 or inc.type == "union_strike")

            if is_roadblock and M.crime_rate < 40.0 then
                if not isCleanupPending(inc.id) then
                    table.insert(M.active_commands, {
                        command = "cleanup_incident",
                        id = inc.id,
                    })
                    M.total_handled = M.total_handled + 1
                    log("[eval] CLEANUP roadblock id=%u (crime_rate=%.1f%% < 40%%)", inc.id, M.crime_rate)
                end
            elseif is_union and M.social_unrest < 30.0 then
                if not isCleanupPending(inc.id) then
                    table.insert(M.active_commands, {
                        command = "cleanup_incident",
                        id = inc.id,
                    })
                    M.total_handled = M.total_handled + 1
                    log("[eval] CLEANUP union_strike id=%u (social_unrest=%.1f%% < 30%%)", inc.id, M.social_unrest)
                end
            end
        end
    end
end

-- -----------------------------------------------------------------------------
-- §6. Command Dispatcher (writeCommandsFile)
-- -----------------------------------------------------------------------------
local function writeCommandsFile()
    if not M.active_commands or #M.active_commands == 0 then
        return
    end

    local entries = {}
    for _, cmd in ipairs(M.active_commands) do
        if cmd.command == "spawn_incident" then
            table.insert(entries, string.format(
                '    {\n      "command": "spawn_incident",\n      "type": "%s",\n      "id": %u,\n      "x": %.2f,\n      "y": %.2f,\n      "z": %.2f,\n      "duration_ms": %u\n    }',
                cmd.type, cmd.id, cmd.x, cmd.y, cmd.z, cmd.duration_ms or 60000
            ))
        elseif cmd.command == "cleanup_incident" then
            table.insert(entries, string.format(
                '    {\n      "command": "cleanup_incident",\n      "id": %u\n    }',
                cmd.id
            ))
        end
    end

    if #entries == 0 then
        M.active_commands = {}
        return
    end

    local json_str = string.format(
        '{\n  "protocol_version": 1,\n  "commands": [\n%s\n  ]\n}\n',
        table.concat(entries, ",\n")
    )

    local f = io.open(CMD_TMP_PATH, "w")
    if f then
        f:write(json_str)
        f:close()
        os.remove(CMD_FINAL_PATH)
        os.rename(CMD_TMP_PATH, CMD_FINAL_PATH)
        log("[dispatch] Dispatched %d command(s) to ASI via municipal_commands.json", #entries)
        M.active_commands = {}
    else
        log("[dispatch] ERROR: Failed to open %s for writing", CMD_TMP_PATH)
    end
end

--- External command injection helper
function writeCommands(commands_table)
    if type(commands_table) == "table" then
        for _, cmd in ipairs(commands_table) do
            table.insert(M.active_commands, cmd)
        end
        writeCommandsFile()
        return true
    end
    return false
end

-- -----------------------------------------------------------------------------
-- §7. Heartbeat Transport
-- -----------------------------------------------------------------------------
local s_heartbeat_count = 0
local s_last_hb_log_time = 0

local function get_timestamp_ms()
    if getGameTimer then
        local gt = getGameTimer()
        if gt and gt >= 0 then
            return math.floor(gt)
        end
    end
    return math.floor(os.clock() * 1000)
end

local function send_heartbeat(now_ms)
    local content = string.format('{"timestamp_ms": %d, "status": "active"}\n', now_ms)

    local f = io.open(HB_TMP_PATH, "w")
    if f then
        f:write(content)
        f:close()
        os.remove(HB_FINAL_PATH)
        os.rename(HB_TMP_PATH, HB_FINAL_PATH)
    end

    s_heartbeat_count = s_heartbeat_count + 1
    if s_heartbeat_count <= 5 then
        log("[hb] updated: timestamp_ms=%d (count=%d)", now_ms, s_heartbeat_count)
    elseif now_ms - s_last_hb_log_time >= 10000 then
        s_last_hb_log_time = now_ms
        log("[hb] updated: timestamp_ms=%d (count=%d)", now_ms, s_heartbeat_count)
    end
end

-- -----------------------------------------------------------------------------
-- §8. Main Script Lifecycle
-- -----------------------------------------------------------------------------
function main()
    log("[init] Municipal AI started successfully (Phase 3 Decision Engine Online)")

    local last_tick = 0

    while true do
        wait(0)

        local current_ms = get_timestamp_ms()

        -- Main Municipal Engine Tick (every 500ms)
        if current_ms - last_tick >= 500 then
            last_tick = current_ms

            -- 1. Heartbeat to ASI watchdog
            local ok_hb, err_hb = pcall(send_heartbeat, current_ms)
            if not ok_hb then
                log("[ERROR] send_heartbeat failed: %s", tostring(err_hb))
            end

            -- 2. Read World State
            local ok_ws, err_ws = pcall(readWorldState)
            if not ok_ws then
                log("[ERROR] readWorldState failed: %s", tostring(err_ws))
            end

            -- 3. Read Incidents State
            local ok_is, err_is = pcall(readIncidentsState)
            if not ok_is then
                log("[ERROR] readIncidentsState failed: %s", tostring(err_is))
            end

            -- 4. Evaluate Spawns
            local ok_sp, err_sp = pcall(evaluateSpawn, current_ms)
            if not ok_sp then
                log("[ERROR] evaluateSpawn failed: %s", tostring(err_sp))
            end

            -- 5. Evaluate Cleanups
            local ok_cl, err_cl = pcall(evaluateCleanup)
            if not ok_cl then
                log("[ERROR] evaluateCleanup failed: %s", tostring(err_cl))
            end

            -- 6. Write Commands to ASI
            local ok_wc, err_wc = pcall(writeCommandsFile)
            if not ok_wc then
                log("[ERROR] writeCommandsFile failed: %s", tostring(err_wc))
            end
        end
    end
end
