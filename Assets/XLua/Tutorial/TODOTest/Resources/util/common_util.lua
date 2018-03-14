-- max_digit: 最大位数，超过后尝试使用K, M等后缀
function format_gold(v, max_digit, sep)
    local s = tostring(v)
    local len = s:len()
    local suffix = ""
    local ret = ""
    
    if max_digit ~= nil and len > max_digit then
        if len - 3 <= max_digit then
            suffix = "K"
            len = len - 3
        else
            suffix = "M"
            len = len - 6
        end
        s = s:sub(1, len)
    end
    
    sep = sep or ","
    for i = len, 1, -3 do
        local p = s:sub(math.max(i - 3 + 1, 1), i)
        if ret ~= "" then
            ret = p .. sep .. ret
        else
            ret = p
        end
    end
    
    return ret .. suffix
end

-- 尝试从transform中查找是否有指定类型的组件
-- typeOrTypes：可以是table，也可以是单个type
function TryGetComponent(transform, typeOrTypes)
    if type(typeOrTypes) == "table" then
        local types = typeOrTypes
        for i, t in ipairs(types) do
            local comp = transform:GetComponent(t)
            if comp then
                return comp
            end
        end
    else
        return transform:GetComponent(typeOrTypes)
    end
end

function RemoveAllChildren(transform)
    local destroy = UnityEngine.Object.Destroy
    local n = transform.childCount
    for i=1,n do
        destroy(transform:GetChild(i - 1).gameObject)
    end
end


-- 如果有高频调用的话，要慎用。。
function MakeCallbackVaArg(f, ...)
    local args = {...}
    local function ret(...)
        local insert = table.insert
        local args2 = {table.unpack(args)}
        for i, v in ipairs({...}) do
            insert(args2, v)
        end
        return f(table.unpack(args2))
    end
    return ret
end

-- function textLayoutStartPosition(totl, pos, align)
--     local px
--     if align == const.TEXT_ALIGN_LEFT then
--         px = pos
--     elseif align == const.TEXT_ALIGN_RIGHT then
--         px = pos - totl
--     else
--         px = pos - totl / 2
--     end
--     return px
-- end

function textLayoutTotalLength(text, font_setting, spacing)
    local txtl = #text
    local totl = 0
    for i=1,txtl-1 do
        local ch = text:sub(i, i)
        totl = totl + font_setting[ch].advance
    end
    totl = totl + font_setting[ text:sub(txtl) ].width
    totl = totl + spacing * (txtl - 1)
    return totl
end

function textLayout(text, font_setting, pos, align, spacing)
    local txtl = #text
    if txtl == 0 then
        return {}
    end

    local totl = textLayoutTotalLength(text, font_setting, spacing)
    local px = textLayoutStartPosition(totl, pos, align)
    
    local coords = {}
    local table_insert = table.insert
    for i=1,txtl do
        local ch = text:sub(i, i)
        local setting = font_setting[ch]
        local x = px + setting.width / 2
        local y = -setting.yoffset
        table_insert(coords, x)
        table_insert(coords, y)
        px = px + setting.advance + spacing
    end
    return coords
end

function textLayoutCurve(text, font_setting, pos, align, spacing, radius)
    local coords = textLayout(text, font_setting, 0, align, spacing)
    local cos = math.cos
    local sin = math.sin
    -- Cartesian to polar coordinate
    local n = #text
    local j = 1
    for i=1,n do
        local x = coords[j]
        local y = coords[j + 1]
        local theta = pos + x / radius
        local r = radius - y
        -- x advance to angle advance
        local nx = cos(theta) * r
        local ny = sin(theta) * r
        coords[j] = nx
        coords[j + 1] = ny
        j = j + 2
    end
    return coords
end

-- -- 根据传入的捣蛋结果，生成一份测试用捣蛋数据
-- function createTestWarInfo(name, park)
--     local buildingID
--     for _, buildingData in pairs(park:GetAllBuildings()) do
--         if buildingData:GetLv() > 0 then
--             buildingID = buildingData:GetID()
--         end
--     end
    
--     local warInfo = {
--         debug=true,
--         buildingID=buildingID,
--         gold=1234,
--     }
    
--     if name == "testSpeciality" then
--         warInfo.result = const.TRICK_RESULT.ATTACK_PET_PK_WIN
--         warInfo.attack = {sid=101, slv=1, speciality=1, float=6, energyCost=1, petInfo={species=1, energy=10, natureBias={{2, 40}, {1, 30}, {1, 30}}}}
--         warInfo.defend = {sid=201, slv=1, restraint=1, float=8, buildingHp=3, targetName="[防守方]", energyCost=8, petInfo={species=3, name="[防守宠]", energy=10, natureBias={{1, 20}, {2, 20}, {3, 60}}, gender=2, species=1}}
--     elseif name == "testCard" then
--         warInfo.result = const.TRICK_RESULT.DEFEND_PET_PK_WIN
--         warInfo.attack = {sid=101, slv=1, float=0, energyCost=0, petInfo={species=1, energy=10, natureBias={{1, 20}, {2, 20}, {3, 60}}, lv=2}}
--         warInfo.defend = {sid=201, slv=1, float=0, buildingHp=3, targetName="[防守方]", energyCost=0, petInfo={name="[防守宠]", energy=10, natureBias={{1, 10}, {2, 30}, {3, 60}}, species=2, gender=1, lv=2}}
--     else
--         error(string.format("creating warInfo for %s is not implemented!", name))
--     end
--     return warInfo
-- end

function GetTargetPositionWithOffset(fromPosition, toPosition, offset)
    local dist = Vector3.Distance(fromPosition, toPosition)

    if offset == 0 or offset == nil then 
        return toPosition
    end

    if dist <= offset then
        return fromPosition
    end

    local temp_x = toPosition.x
    local temp_y = toPosition.y
    local temp_z = toPosition.z

    if fromPosition.x < toPosition.x then
        temp_x = (toPosition.x - fromPosition.x) * (dist - offset) / dist + fromPosition.x
    elseif fromPosition.x > toPosition.x then
        temp_x = (fromPosition.x - toPosition.x) * offset / dist + toPosition.x
    end

    if fromPosition.y < toPosition.y then
        temp_y = (toPosition.y - fromPosition.y) * (dist - offset) / dist + fromPosition.y
    elseif fromPosition.y > toPosition.y then
        temp_y = (fromPosition.y - toPosition.y) * offset / dist + toPosition.y
    end

    if fromPosition.z < toPosition.z then
        temp_z = (toPosition.z - fromPosition.z) * (dist - offset) / dist + fromPosition.z
    elseif fromPosition.z > toPosition.z then
        temp_z = (fromPosition.z - toPosition.z) * offset / dist + toPosition.z
    end

    return Vector3(temp_x, temp_y, temp_z)
end

-- position：UI的世界坐标, transform.position
-- canvasRectTransform：defaults to "/LuaCanvas" if not specified
-- groundTransform: defaults to "/Ground" if not specified
function CanvasPositionToGround(position, canvasRectTransform, groundTransform)
    if canvasRectTransform == nil then
        canvasRectTransform = GameObject.Find("/LuaCanvas"):GetComponent("RectTransform")
    end
    if groundTransform == nil then
        groundTransform = GameObject.Find("/Ground").transform
    end
    local cam = canvasRectTransform:Find("Camera"):GetComponent("Camera")
    local scrPos = cam:WorldToScreenPoint(position)
    local groundPos = Camera.main:GetComponent("CameraController"):ScreenToFloor(scrPos, groundTransform.position.y)
    return groundPos
end

-- position：世界坐标
-- canvasRectTransform：defaults to "/LuaCanvas" if not specified
function WorldPositionToCanvas(position, canvasRectTransform)
    if canvasRectTransform == nil then
        canvasRectTransform = GameObject.Find("/LuaCanvas"):GetComponent("RectTransform")
    end
    local viewportPos = Camera.main:WorldToViewportPoint(position)
    local scrPos = Vector3((viewportPos.x - 0.5) * canvasRectTransform.sizeDelta.x,
        (viewportPos.y - 0.5) * canvasRectTransform.sizeDelta.y, 0)
    return scrPos
end

-- position: 目标位置的相对坐标
-- aimTransform: 目标位置相对坐标的父类,defaults to "/LuaCanvas" if not specified
-- selectTransform: 选中物体的父类selectTransform
function LocalPositionToLocal(position, selectTransform, aimTransform)
    if aimTransform == nil then
        aimTransform = GameObject.Find("/LuaCanvas"):GetComponent("RectTransform")
    end
    local worldPosition = aimTransform:TransformPoint(position)
    local localPosition = selectTransform:InverseTransformPoint(worldPosition)
    return localPosition
end

function IsObjectReleased(obj)
    return obj.__ClassDef__ == nil
end

function UIFollow(go, transform, offset, followScale)
    local typeObject = typeof(CS.UIFollow)
    local CSUIFollow = go:GetComponent(typeObject)
    if CSUIFollow == nil then
        CSUIFollow = go:AddComponent(typeObject)
    end
    CSUIFollow.target = transform
    CSUIFollow.offset = offset
    CSUIFollow.followScale = followScale
end

-- function GetPetScaleFactor(lv)
--     Import "globalcommon.const"
--     for factor, lvScope in pairs(const.PetShape) do
--         if lv >= lvScope[1] and lv <= lvScope[2] then
--             return factor/100
--         end
--     end
--     return 1
-- end

function IsNilComponent(component)
    return component == nil or component:IsNull()
end

function GetOrAddComponent(gameObject, componentType)
    local component = gameObject:GetComponent(componentType)
    if IsNilComponent(component) then
        component = gameObject:AddComponent(componentType)
    end
    return component
end

function SetImage(transform, imagePath)
    if imagePath == nil or imagePath == "" then
        return
    end
    local uiloader = GetOrAddComponent(transform.gameObject, typeof(CS.UILoader))
    uiloader.path = imagePath
    return uiloader
end

function SetStarSprite(panel, starImg, starNum)
    local settingMgr = GetApp():GetSettingMgr()
    local allKeys = settingMgr:GetAllKeys("park_star_info")
    local setting = nil
    for i,v in ipairs(allKeys) do
        setting = settingMgr:GetLine("park_star_info",v)
        if starNum < setting.StarNum then
            setting = settingMgr:GetLine("park_star_info",v-1)
            panel:SetImage(starImg,setting.Path)
            return
        end
    end

    setting = settingMgr:GetLine("park_star_info",allKeys[#allKeys-1])
    panel:SetImage(starImg,setting.Path)
end

local TOURNAMENT_ENTERKEY_SEP = "|"
function TournamentSerializeEnterKey(playerIDs)
    local s1 = tostring(playerIDs[1])
    local s2 = tostring(playerIDs[2])
    return s1 .. TOURNAMENT_ENTERKEY_SEP .. s2
end

function TournamentDeserializeEnterKey(key)
    local sepIndex = key:find(TOURNAMENT_ENTERKEY_SEP)
    local id1 = tonumber(key:sub(1, sepIndex - 1))
    local id2 = tonumber(key:sub(sepIndex + 1))
    return {id1, id2}
end

function Tablelength(T)
  local count = 0
  for _ in pairs(T) do count = count + 1 end
  return count
end

function GetCardSetting(id)
    return GetApp():GetSettingMgr():GetLine("pet_skill", id)
end

function  GetCardTypeByID(id)
    local s = GetCardSetting(id)
    return s.Type
end