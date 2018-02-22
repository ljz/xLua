-- Lua类定义

local typeFunc = type
local string = string
local stringFormat = string.format
local tostring = tostring
local setmetatable = setmetatable
local getmetatable = getmetatable
local rawset = rawset

function ClsToString(self)
    return stringFormat("<Cls:%s:%s>", self.__ClassName__, tostring(getmetatable(self)))
end

function ClassRelease(Inst)
    -- 解除对类的引用、删除元表,每个类对象删除的时候最好都调用
    if Inst == nil then
        return
    end
    
    rawset(Inst,"__ClassDef__",nil)
    rawset(Inst,"__IsInst__",nil)
    local InstMT = getmetatable(Inst)
    if InstMT then
        rawset(InstMT,"__index",nil)
        setmetatable(Inst,nil)
    end
end

function DefaultCtor(self, ...)
    local base = self.__Super__
    if base then
        local className = base.__ClassName__
        local super = base[className]
        if super then
            super(self, ...)
        end
    end
end

function ClassNew(InClassDef,...)
    local Inst = {}
    local InstMT = {
        __index = InClassDef,
        __tostring = ClsToString
    }
    
    setmetatable(Inst,InstMT)
    
    rawset(Inst,"__ClassDef__",InClassDef)
    rawset(Inst,"__IsInst__",true)

    -- 调用构造函数
    local className = InClassDef.__ClassName__
    local constructFunc = InClassDef[className]
    if constructFunc then
        constructFunc(Inst,...)
    else
        DefaultCtor(Inst,...)
    end
    
    rawset(Inst,"__Release__",ClassRelease)
    return Inst
end

local function ClassNewSingle(InClassDef, ...)
    if InClassDef.__instance ~= nil then
        return InClassDef.__instance
    end
    
    local inst = {}
    rawset(inst,"__SINGLETON_INST__",true)

    InClassDef.__instance = inst
    setmetatable(inst, {__index = InClassDef, __tostring = ClsToString})
    inst.__ClassDef__ = InClassDef
    local constructFunc = inst[InClassDef.__ClassName__]
    if constructFunc ~= nil then
        constructFunc(inst, ...)
    else
        DefaultCtor(inst, ...)
    end
    
    -- singleton 不能简单 release，加个引用计数？
    --cls.__Release__ = ClassRelease
    return inst
end

function Class(InClassName, baseCls)
    if InClassName == nil then
        LOGE("InClassName can't be nil")
        return nil
    end

    local single = false -- singleton 基类
    if baseCls ~= nil then
        if baseCls.__SINGLETON__ ~= nil then
            single = true
        end
    end

    local ClassMT
    if single then
        ClassMT = {
            __index = baseCls,
            __call = ClassNewSingle,
            __tostring = ClsToString
        }
    else
        ClassMT = {
            __index = baseCls,
            __call = ClassNew,
            __tostring = ClsToString
        }
    end
    local ClassDef = setmetatable({}, ClassMT)
    ClassDef.__Super__ = baseCls
    ClassDef.__ClassName__ = InClassName
    ClassDef.__index = ClassDef
    if single then
        ClassDef.__SINGLETON__ = true
    end
    
    return ClassDef
end

-- 全局唯一单例基类
local clsMetaTable = 
{
    __call = ClassNewSingle,
    __tostring = ClsToString
}
CSingleton = setmetatable({}, clsMetaTable)
CSingleton.__Super__ = {}
CSingleton.__ClassName__ = "CSingleton"
CSingleton.__SINGLETON__ = true

-- 获取对象类型
function GetObjectType(InObj)
    local objType = typeFunc(InObj)
    if objType == "function" then
        -- 函数
        return objType
    end
    
    if objType ~= "table" then
        -- Lua其他的原生类型
        return objType
    end
    
    local rg = rawget
    if rg(InObj,"__SINGLETON__") then
        --单例
        return "singleton" 
    end
    
    if rg(InObj,"__IsInst__") then
        -- 类对象
        return "obj" 
    end

    if rg(InObj,"__SINGLETON_INST__") then
        -- 单例对象
        return "obj" 
    end

    if rg(InObj,"__ClassName__") then
        -- 类
        return "class"
    end
    
    if rg(InObj,"__IsEnum__") then
        -- 枚举
        return "enum"
    end
    
    if rg(InObj,"__MODULENAME__") then
        -- 模块
        return "module"
    end
    
    return objType
end
