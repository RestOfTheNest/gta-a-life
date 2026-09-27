-- =============================================================================
--  tycoon_shop.lua — Interactive Dear ImGui Shop & Service UI for MoonLoader
--  GTA San Andreas v1.0 US · MoonLoader · mimgui Backend
-- =============================================================================

script_name("Tycoon Retail Store & Depot Engine")
script_author("Antigravity Systems")
script_description("Interactive mimgui 20-Store Retail, Supply Depot & Service Network")
script_version("3.0.0")

local imgui = require 'mimgui'
local ffi = require 'ffi'

-- -----------------------------------------------------------------------------
-- §1. Category Configurations & Catalog Specifications
-- -----------------------------------------------------------------------------
local CATEGORY_INFO = {
    [0] = {
        tag = "[FOOD & RATIONS]",
        color = imgui.ImVec4(0.13, 0.77, 0.37, 1.0),
        item_type = "heal",
        item_name = "Combat Rations & Medical Kit",
        item_desc = "Restores CJ Health to 100% and provides emergency sustenance"
    },
    [1] = {
        tag = "[PETROLEUM & FUEL]",
        color = imgui.ImVec4(0.96, 0.62, 0.04, 1.0),
        item_type = "refuel",
        item_name = "High-Octane Refuel & Vehicle Service",
        item_desc = "Repairs vehicle to 1000 HP, fixes chassis, or equips +50% body armor"
    },
    [2] = {
        tag = "[AGRO FOOD & PRODUCE]",
        color = imgui.ImVec4(0.13, 0.77, 0.37, 1.0),
        item_type = "heal",
        item_name = "Wholesale Agricultural Produce & Combat Rations",
        item_desc = "Restores CJ Health to 100% and equips +50% Body Armor"
    },
    [3] = {
        tag = "[TACTICAL TECH & AMMO]",
        color = imgui.ImVec4(0.22, 0.74, 0.97, 1.0),
        item_type = "ammo",
        item_name = "Tactical Micro-UZI & Ballistic Armor",
        item_desc = "Supplies Micro-UZI with 150 rounds and equips 100% Kevlar Body Armor"
    }
}

-- -----------------------------------------------------------------------------
-- §2. Fast Zero-Dependency JSON Parser
-- -----------------------------------------------------------------------------
local function parse_json(str)
    if not str or str == '' then return nil end
    local pos = 1
    local len = #str

    local function skip_whitespace()
        while pos <= len do
            local c = str:sub(pos, pos)
            if c == ' ' or c == '\t' or c == '\n' or c == '\r' then
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
            if c == '\\' then
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
            if c:match('[%d%.%-%+eE]') then
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
        if str:sub(pos, pos) == ']' then
            pos = pos + 1
            return arr
        end
        while pos <= len do
            table.insert(arr, parse_value())
            skip_whitespace()
            local c = str:sub(pos, pos)
            if c == ',' then
                pos = pos + 1
                skip_whitespace()
            elseif c == ']' then
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
        if str:sub(pos, pos) == '}' then
            pos = pos + 1
            return obj
        end
        while pos <= len do
            skip_whitespace()
            if str:sub(pos, pos) ~= '"' then break end
            local key = parse_string()
            skip_whitespace()
            if str:sub(pos, pos) == ':' then
                pos = pos + 1
            end
            skip_whitespace()
            obj[key] = parse_value()
            skip_whitespace()
            local c = str:sub(pos, pos)
            if c == ',' then
                pos = pos + 1
                skip_whitespace()
            elseif c == '}' then
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
        elseif c == '{' then
            return parse_object()
        elseif c == '[' then
            return parse_array()
        elseif c == 't' and str:sub(pos, pos + 3) == 'true' then
            pos = pos + 4
            return true
        elseif c == 'f' and str:sub(pos, pos + 4) == 'false' then
            pos = pos + 5
            return false
        elseif c == 'n' and str:sub(pos, pos + 3) == 'null' then
            pos = pos + 4
            return nil
        else
            return parse_number()
        end
    end

    return parse_value()
end

-- -----------------------------------------------------------------------------
-- §3. State & IPC Variables
-- -----------------------------------------------------------------------------
local show_shop = imgui.new.bool(false)
local active_store = nil
local economy_stores = {}
local player_cash_cached = 0
local closed_manually = false

local feedback_text = ""
local feedback_timer = 0
local feedback_is_error = false

local function set_feedback(msg, is_error)
    feedback_text = msg
    feedback_is_error = is_error or false
    feedback_timer = os.clock() + 3.5
end

local function read_economy_state()
    local f = io.open("moonloader/config/economy_state.json", "r")
    if not f then return end
    local content = f:read("*a")
    f:close()
    if content and #content > 0 then
        local data = parse_json(content)
        if data then
            if data.stores and type(data.stores) == 'table' then
                economy_stores = data.stores
            end
            if data.player and data.player.cash then
                player_cash_cached = tonumber(data.player.cash) or 0
            end
        end
    end
end

local function send_purchase_request(store, item_type)
    local cost = store.finalPrice or 0
    local req = string.format('{"store_id": %d, "item_type": "%s", "cost": %d, "timestamp": %d}',
        store.id, item_type, cost, os.time())
    local f = io.open("moonloader/config/economy_requests.json", "w")
    if f then
        f:write(req)
        f:close()
        set_feedback(string.format(">> Purchase submitted for $%d!", cost), false)
    else
        set_feedback(">> Failed to write purchase request!", true)
    end
end

-- -----------------------------------------------------------------------------
-- §4. Open & Close Handlers
-- -----------------------------------------------------------------------------
local function open_shop(store)
    active_store = store
    read_economy_state()
    feedback_text = ""
    show_shop[0] = true
    closed_manually = false
    setPlayerControl(PLAYER_HANDLE, false)
end

local function close_shop()
    active_store = nil
    show_shop[0] = false
    closed_manually = true
    setPlayerControl(PLAYER_HANDLE, true)
end

-- -----------------------------------------------------------------------------
-- §5. mimgui Frame Handler
-- -----------------------------------------------------------------------------
imgui.OnFrame(
    function()
        return show_shop[0]
    end,
    function(player)
        player.HideCursor = not show_shop[0]

        if not active_store then return end

        local sw, sh = getScreenResolution()
        imgui.SetNextWindowSize(imgui.ImVec2(540, 380), imgui.Cond.FirstUseEver)
        imgui.SetNextWindowPos(imgui.ImVec2(sw * 0.5 - 270, sh * 0.5 - 190), imgui.Cond.FirstUseEver)

        local cat_info = CATEGORY_INFO[active_store.cat] or CATEGORY_INFO[0]
        local current_cash = getPlayerMoney()
        if current_cash == 0 and player_cash_cached > 0 then
            current_cash = player_cash_cached
        end

        local title = string.format("%s###StorePurchaseWindow", active_store.name)

        -- Window Styling
        local style = imgui.GetStyle()
        style.WindowRounding = 8.0
        style.FrameRounding = 5.0
        style.ItemSpacing = imgui.ImVec2(10.0, 8.0)
        style.WindowPadding = imgui.ImVec2(16.0, 16.0)

        if imgui.Begin(title, show_shop, imgui.WindowFlags.NoCollapse) then
            -- Header Tag
            imgui.TextColored(cat_info.color, cat_info.tag)
            imgui.SameLine()
            imgui.TextColored(imgui.ImVec4(0.70, 0.70, 0.70, 1.0), string.format("| District #%d", active_store.dist or 0))
            imgui.SameLine()
            imgui.TextColored(imgui.ImVec4(0.25, 0.90, 0.40, 1.0), string.format("| CJ Funds: $%d", current_cash))

            imgui.Separator()

            -- Feedback banner
            if os.clock() < feedback_timer and feedback_text ~= "" then
                if feedback_is_error then
                    imgui.TextColored(imgui.ImVec4(0.95, 0.30, 0.30, 1.0), feedback_text)
                else
                    imgui.TextColored(imgui.ImVec4(0.30, 0.95, 0.40, 1.0), feedback_text)
                end
                imgui.Separator()
            end

            -- Inventory & Pricing Grid
            local stock = active_store.stock or 0
            local cap = math.max(1, active_store.cap or 1)
            local stock_pct = math.floor((stock / cap) * 100)
            local is_bankrupt = (active_store.isBankrupt == true)
            local price = active_store.finalPrice or 0
            local can_afford = (current_cash >= price)

            imgui.TextColored(imgui.ImVec4(0.85, 0.85, 0.90, 1.0), "Supply Stock:")
            imgui.SameLine()
            if is_bankrupt then
                imgui.TextColored(imgui.ImVec4(0.95, 0.25, 0.25, 1.0), "BANKRUPT (Trading Suspended)")
            elseif stock == 0 then
                imgui.TextColored(imgui.ImVec4(0.95, 0.25, 0.25, 1.0), "OUT OF STOCK (0 units)")
            else
                imgui.TextColored(imgui.ImVec4(0.25, 0.90, 0.40, 1.0), string.format("%d / %d units (%d%% Available)", stock, cap, stock_pct))
            end

            imgui.TextColored(imgui.ImVec4(0.85, 0.85, 0.90, 1.0), "Scarcity-Adjusted Price:")
            imgui.SameLine()
            imgui.TextColored(can_afford and imgui.ImVec4(0.25, 0.90, 0.40, 1.0) or imgui.ImVec4(0.95, 0.30, 0.30, 1.0), string.format("$%d", price))

            imgui.Spacing()
            imgui.Separator()
            imgui.Spacing()

            -- Product Showcase
            imgui.TextColored(imgui.ImVec4(0.95, 0.95, 0.95, 1.0), cat_info.item_name)
            imgui.TextColored(imgui.ImVec4(0.60, 0.65, 0.75, 1.0), cat_info.item_desc)

            imgui.Spacing()
            imgui.Spacing()

            -- Purchase Action Button
            local can_buy = can_afford and (stock > 0) and not is_bankrupt
            local btn_text = string.format("Buy for $%d", price)

            if can_buy then
                if imgui.Button(btn_text, imgui.ImVec2(160, 36)) then
                    send_purchase_request(active_store, cat_info.item_type)
                    read_economy_state()
                end
            else
                local reason = "Insufficient Funds"
                if is_bankrupt then
                    reason = "Store Bankrupt"
                elseif stock <= 0 then
                    reason = "Out of Stock"
                end
                imgui.TextColored(imgui.ImVec4(0.95, 0.35, 0.35, 1.0), string.format("[Purchase Unavailable: %s]", reason))
            end

            imgui.Spacing()
            imgui.Separator()
            imgui.Spacing()

            if imgui.Button("Close Menu [ESC]", imgui.ImVec2(140, 28)) then
                close_shop()
            end

            imgui.End()
        end

        if not show_shop[0] then
            close_shop()
        end
    end
)

-- -----------------------------------------------------------------------------
-- §6. Main Background Proximity Engine
-- -----------------------------------------------------------------------------
function main()
    while not isPlayerPlaying(PLAYER_HANDLE) do
        wait(100)
    end

    read_economy_state()
    local last_read_time = os.clock()

    while true do
        wait(0)

        -- Reload state every 400ms
        if os.clock() - last_read_time >= 0.4 then
            last_read_time = os.clock()
            read_economy_state()
        end

        local px, py, pz = getCharCoordinates(PLAYER_PED)
        local closest_store = nil
        local min_dist = 99999.0

        if economy_stores and #economy_stores > 0 then
            for _, store in ipairs(economy_stores) do
                local dx = px - store.x
                local dy = py - store.y
                local dz = pz - store.z
                local dist = math.sqrt(dx * dx + dy * dy)
                if dist < min_dist then
                    min_dist = dist
                    if dist <= 3.5 and math.abs(dz) <= 15.0 then
                        closest_store = store
                    end
                end
            end
        end

        if show_shop[0] then
            if wasKeyPressed(0x1B) or min_dist > 6.0 then
                close_shop()
            end
        else
            if closest_store then
                printHelpString("Press ~y~E~w~ to open " .. closest_store.name)
                if wasKeyPressed(0x45) then
                    open_shop(closest_store)
                end
            end
        end
    end
end

