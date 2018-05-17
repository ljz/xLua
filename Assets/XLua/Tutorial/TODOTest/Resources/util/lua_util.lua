-- File: lua_util.lua

local typeFunc = type
local tostring = tostring
local tonumber = tonumber
local pairs = pairs
local ipairs = ipairs
local assert = assert

function GetClsByClsStr(strModule, strCls)
    print("strModule ===============")
    print(strModule)
    print("strCls ===================")
    print(strCls)
    -- local mod = require(strModule)
    local mod = Import(strModule)
    if mod ~= nil then
        return mod[strCls]
    end
    return nil
end

function IsValidNumber(val)
    if "number" ~= typeFunc(val) then
        return false
    end
    if 0 * val ~= 0 then
        return false
    end
    return true
end

function table.len(tbl)
    assert("table" == typeFunc(tbl))
    local cnt = 0
    for k, v in pairs(tbl) do
        cnt = cnt + 1
    end
    return cnt
end

function table.print(tbl)
    if "table" ~= typeFunc(tbl) then
        return false
    end
    for k, v in pairs(tbl) do
        print(k, "=", v)
    end
end

function table.get(self, field, defV)
    if nil == self then
        return defV
    end
    local val = self[field]
    if val == nil then
        return defV
    end
    return val
end

function table.keys(tbl)
    local keys = {}
    for k, v in pairs(tbl) do
        keys[#keys + 1] = k
    end
    return keys
end

function table.shuffle(array)
    local funcRand = math.random
    for n = #array, 1, -1 do
        local k = funcRand(n)
        array[n], array[k] = array[k], array[n]
    end
    return array
end

function table.isarray(self)
    if "table" ~= typeFunc(self) then
        return false
    end
    local cnt = 0
    for k, v in pairs(self) do
        if "number" ~= typeFunc(k) then
            return false
        end
        cnt = cnt + 1
    end
    local curKey = 1
    local iCnt = 0
    for k, v in ipairs(self) do
        if k ~= curKey  then
            return false
        end
        curKey = curKey + 1
        iCnt = iCnt + 1
    end
    return cnt == iCnt
end

function table.merge(tblA, tblB)
    -- 返回 tbleA
    for k, v in pairs(tblB) do
        assert("number" == typeFunc(v))
        local val = tblA[k]
        if nil == val then
            val = 0
        else
            assert("number" == typeFunc(val))
        end
        tblA[k] = val + v
    end
    return tblA
end

function table.toStrKey(tbl,keyTbl)
    if nil == tbl then
        return nil
    end
    
    local tmp = keyTbl or {}
    for k, v in pairs(tbl) do
        tmp[tostring(k)] = v
    end
    return tmp
end

function table.toNumKey(tbl)
    if nil == tbl then
        return nil
    end
    local tmp = {}
    for k, v in pairs(tbl) do
        tmp[tonumber(k)] = v
    end
    return tmp
end

function table.randomPickWeight(tbl, weightIdx, retIdx)
    -- tbl 需要外面构造好成一个累加权重数组
    local weightSum = tbl[#tbl]
    if weightIdx ~= nil then
        weightSum = weightSum[weightIdx]
    end
    local rand = math.random(weightSum)
    local w = nil
    for i, item in ipairs(tbl) do
        if weightIdx == nil then
            w = item
        else
            w = item[weightIdx]
        end
        if rand <= w then
            if nil == retIdx then
                return i
            end
            return item[retIdx] -- 返回 ID
        end
    end
    LOGWF("randomPickWeight reach where should never reach!")
    if nil == retIdx then
        return #tbl
    end
    return tbl[#tbl][retIdx]
end

function table.copy(tbl)
    local newTbl = {}
    for k, v in pairs(tbl) do
        newTbl[k] = v
    end
    return newTbl
end

function deepcopy(val)
    if type(val) == "table" then
        local newVal = {}
        for k, v in pairs(val) do
            newVal[k] = deepcopy(v)
        end
        return newVal
    else
        return val
    end
end

function table.deepcopy(tbl)
    return deepcopy(tbl)
end

function string.split(str, delimeter)
    if "string" ~= typeFunc(str) then
        return {}
    end
    local ret = {}
    local cnt = 1
    for token in string.gmatch(str, "([^" .. delimeter .. "]+)") do
        ret[cnt] = token
        cnt = cnt + 1
    end
    return ret
end

function string.startswith(str, prefix)
    if typeFunc(str) ~= "string" or typeFunc(prefix) ~= "string" then
        LOGE("invalid arguments for string.startswith(string, string)")
        return false
    end
    if #prefix > #str then
        return false
    end
    return str:sub(1, #prefix) == prefix
end

function string.endswith(str, suffix)
    if typeFunc(str) ~= "string" or typeFunc(suffix) ~= "string" then
        LOGE("invalid arguments for string.endswith(string, string)")
        return false
    end
    if #suffix > #str then
        return false
    end
    return str:sub(#str - #suffix + 1) == suffix
end

function string.strip(str)
    if "string" ~= typeFunc(str) then
        return str
    end
    local start = 1
    local stop = nil
    local len = string.len(str)
    local sub = string.sub
    for i = 1, len do
        if ' ' ~= sub(str, i, i) then
            start = i
            break
        end
    end
    for i = len, 1, -1 do
        if ' ' ~= sub(str, i, i) then
            stop = i
            break
        end
    end
    return sub(str, start, stop)
end