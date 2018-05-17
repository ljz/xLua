require("ui.weakref")


CMsgCallback = Class("CMsgCallback")

function CMsgCallback:Init(event,obj, func, ...)
    self.event = event
    self.args = table.pack(...)
    self.func = func
    self.argNum = self.args.n
    if obj then
        self.obj = weakref(obj)
    end
end

function CMsgCallback:Release()
    self.event = nil
    self.args = nil
    self.func = nil
    self.argNum = nil
    self.obj = nil
    self:__Release__()
end

function CMsgCallback:GetEvent()
    return self.event
end

function CMsgCallback:GetArgs()
    return self.args
end

function CMsgCallback:GetFunc()
    return self.func
end

function CMsgCallback:GetObj()
    return self.obj
end

function CMsgCallback:IsMethod()
    return self.obj ~= nil
end

function CMsgCallback:Invoke(...)
    --假如执行期间被释放也返回
    if self == nil then
        return false
    end
    if self.func == nil then
        return false
    end
    local obj
    if self:IsMethod() then
        obj = self.obj()
        if obj == nil then
            return false
        end
    end
    
    local origCnt = self.argNum
    local extraCnt = select("#", ...)
    self.args["n"] = nil
    -- extra arguments
    if extraCnt > 0 then
        self:AppendArgs(extraCnt, ...)
    end
    if obj ~= nil then
        self.func(obj, table.unpack(self.args, 1, origCnt + extraCnt))
    else
        self.func(table.unpack(self.args, 1, origCnt + extraCnt))
    end
    -- 有可能调用完以后这个callback已经被Release了，WTF
    if self.args == nil then
        return false
    end
    -- remove extra arguments
    if extraCnt > 0 then
        self:RemoveArgs(origCnt, extraCnt)
    end
    return true
end

-- CMsgCallback object / (obj, func)
function CMsgCallback:Equals(arg1, arg2)
    local obj = arg1
    local func = arg2

    if arg2 == nil then
        obj = arg1.obj
        func = arg1.func
    end

    if self.func ~= func then
        return false
    end
    if self.obj == nil then
        return obj == nil
    end
    return self.obj() == obj
end

function CMsgCallback:AppendArgs(argCnt, ...)
    local insert = table.insert
    local args = {...}
    local idx = self.argNum
    local cbArgs = self.args
    for i = 1, argCnt do
        cbArgs[idx + i] = args[i]
    end
end

function CMsgCallback:RemoveArgs(origCnt, extraCnt)
    local cbArgs = self.args
    for i = origCnt + 1, origCnt + extraCnt do
        cbArgs[i] = nil
    end
end