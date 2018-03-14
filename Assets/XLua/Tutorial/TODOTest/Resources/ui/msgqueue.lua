require("ui.msgcallback")
local assert = assert
local ipairs = ipairs
local insertF = table.insert
local removeF = table.remove
local pairs = pairs

CMsgQueue = Class("CMsgQueue")

function CMsgQueue:CMsgQueue()
    self.events = {} -- {event: callback}
    self.obj2CbMap = {}
end

function CMsgQueue:Register(event, obj, func, ...)
    assert(event)
    local callbacks = self:_setDefault(event)
    local cb = CMsgCallback()
    cb:Init(event, obj, func, ...)

    local SafeEquals = CMsgCallback.Equals
    for i, _cb in ipairs(callbacks) do
        if SafeEquals(cb, _cb) then
            return
        end
    end
    insertF(callbacks, cb)
    
    local cbs = self.obj2CbMap[obj]
    if cbs == nil then
        cbs = {}
        self.obj2CbMap[obj] = cbs
    end
    insertF(cbs, cb)
end

function CMsgQueue:Unregister(event, obj, func)
    assert(event)
    local callbacks = self:_setDefault(event)
    local SafeEquals = CMsgCallback.Equals
    
    for i, cb in ipairs(callbacks) do
        if SafeEquals(cb, obj, func) then
            cb:Release()
            return
        end
    end
end

function CMsgQueue:UnregisterAllByObj(obj)
    if obj == nil then
        return
    end
    
    local allCallback = self.obj2CbMap[obj]
    if allCallback == nil then
        return
    end
    self.obj2CbMap[obj] = nil
    
    local SafeRelease = self.ReleaseCallbackInner    
    local count = #allCallback
    for i = 1, count do
        local callback = allCallback[i]
        allCallback[i] = nil
        SafeRelease(self, callback)
    end
end

function CMsgQueue:ReleaseCallbackInner(InCallback)
    if InCallback == nil or InCallback.Release == nil then
        return
    end
    InCallback:Release()
end

function CMsgQueue:SendMsg(event, ...)
    assert(event)
    local callbacks = self:_setDefault(event)
    local i = 1
    local n = #callbacks    -- 本次回调的过程中新增的callback不会被调用
    local on = n            -- old `n`
    local SafeInvoke = CMsgCallback.Invoke
    while i <= n do
        local cb = callbacks[i]
        if SafeInvoke(cb, ...) then
            i = i + 1
        else
            callbacks[i] = callbacks[n]
            callbacks[n] = cb
            n = n - 1
        end
    end
    local nn = #callbacks   -- new `n`
    if on > n then  -- safe release callbacks
        local SafeRelease = self.ReleaseCallbackInner
        for i = n + 1, on do
            SafeRelease(self, callbacks[i])
            callbacks[i] = nil
        end
    end
    -- shift newly added callbacks
    if nn > on then
        for i = 1, nn - on do
            callbacks[n + i] = callbacks[on + i]
            callbacks[on + i] = nil
        end
    end
end

function CMsgQueue:_setDefault(event)
    local callbacks = self.events[event]
    if callbacks == nil then
        callbacks = {}
        self.events[event] = callbacks
    end
    return callbacks
end

function CMsgQueue:printCallbacksForEvent(event)
    local callbacks = self:_setDefault(event)
    print ("Callbacks for event", event)
    for i, callback in ipairs(callbacks) do
        local func = callback.func
        local obj = callback.obj
        if obj ~= nil then
            obj = obj()
        end
        print (func, obj)
    end
end

function CMsgQueue:Clear()
    self.events = {}
    self.obj2CbMap = {}
end

