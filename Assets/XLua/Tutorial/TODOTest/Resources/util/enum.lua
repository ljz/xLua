-- 枚举值封装

local rawget = rawget
local table = table
local string = string
local setmetatable = setmetatable

local __AllEnums__ = {}
local __LastEnum__ = nil

function DoEnum(self, key, val)
	local k2v = rawget(self,"__K2V__")
	if rawget(self,"__END__") then -- 枚举已经结束
		--[[
		error(string.format("Enum(%s) ended, can't add new enum(%s) any more.", self.__ClassName__, key))
			return nil
		
		enum 暂时没法做到
		A = Enum("A")
		A.E1
		A.E2
		暂时只能是
		B = Enum("B")
		B.E1 = 0
		B.E2 = 1
		后面在想想能不能把这步复制简化掉
		]]
		if val ~= nil then
			LOGE(string.format("Enum(%s) ended, can't add new enum(%s) any more.", rawget(self,"__ENUMNAME__"), key))
			return nil
		end
		local v = k2v[key]
		if v == nil then
			LOGE(string.format("Enum(%s) doesn't contain enum key(%s)", rawget(self,"__ENUMNAME__"), key))
			return nil
		end
		return v
	end
	
	local v2k = rawget(self,"__V2K__")
	if val ~= nil then -- 重复性检查
		local v = k2v[key]
		if v ~= nil then -- 重复枚举，key 重复
			LOGE(string.format("Duplicated key(%s) of enum(%s), previous value is %s.", key, rawget(self,"__ENUMNAME__"), v))
			return nil
		end
		local k = v2k[val]
		if k ~= nil then -- 重复枚举，val 重复
			LOGE(string.format("Duplicated value(%s) of enum(%s), previous key is %s.", val, rawget(self,"__ENUMNAME__"), k))
			return nil
		end
	else
		local v = k2v[key]
		if v == nil then
			LOGE(string.format("Enum(%s) doesn't contain enum key(%s)", rawget(self,"__ENUMNAME__"), key))
		end
		return v
	end
	
	-- 增加枚举值
	k2v[key] = val
	v2k[val] = key
	local allKey = rawget(self,"__AllKey__")
	if allKey then
		table.insert(allKey,key)
	end
	
	local allValue = rawget(self,"__AllValue__")
	if allValue then
		table.insert(allValue,val)
	end
	
	return val
end

function HasEnum(self, key)
	local k2v = rawget(self,"__K2V__")
	if k2v[key] then
		return true
	end
	return false
end

function HasValue(self, val)
	local v2k = rawget(self,"__V2K__")
	if v2k[val] then
		return true
	end
	return false
end

function Val2Key(self, val)
	local v2k = rawget(self,"__V2K__")
	return v2k[val]
end

function GetKV(self)
	return rawget(self,"__K2V__")
end

function GetVK(self)
	return rawget(self,"__V2K__")
end

function GetAllKey(self)
	return rawget(self,"__AllKey__")
end

function GetAllValue(self)
	return rawget(self,"__AllValue__")
end

function GetEnumName(self)
	return rawget(self,"__ENUMNAME__")
end

-- 只能再模块中使用，不要在函数中调用，这样无法正确设置__END__
function Enum(enumName,bOpenKeyArray,bOpenValueArray)
	if enumName == nil then
		error("Enum(enumName) failed,enumName is nil ")
	end
	
	-- enum 和 env中的enum重名了
	
	local em = __AllEnums__[enumName]
	if em then
		error("Duplicated em: " .. enumName)
		return nil
	end
	
	em = {}
	local metaTable =
	{
		__index = DoEnum,
		__newindex = DoEnum,
	}
	em.__ENUMNAME__ = enumName
	em.__IsEnum__ = true
	em.__CURIDX__ = 0
	em.__K2V__ = {}
	em.__V2K__ = {}
	
	if bOpenKeyArray then
		em.__AllKey__ = {}
	end
	
	if bOpenValueArray then
		em.__AllValue__ = {}
	end
	
	em.__END__ = false
	em.HasEnum = HasEnum
	em.HasValue = HasValue
	em.Val2Key = Val2Key
	em.GetKV = GetKV
	em.GetVK = GetVK
	em.GetAllKey = GetAllKey
	em.GetAllValue = GetAllValue
	em.GetEnumName = GetEnumName
	
	setmetatable(em, metaTable)
	if __LastEnum__ then
		rawset(__LastEnum__,"__END__",true)
	end
	__LastEnum__ = em
	__AllEnums__[enumName] = em
	
	return em
end

function EndEnum()
	if __LastEnum__ then
		rawset(__LastEnum__,"__END__",true)
		__LastEnum__ = nil
	end
end

