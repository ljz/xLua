CMono = Class("CMono")

function CMono:Init()
    self.curTickID = 0
    self.delayedCalls = {}  -- {{tickID1, time1, fun1, arg1}, {tickID2, time2, fun2, arg2}, ...}
end

function CMono:Release()
    self:ClearDelayedCalls()
    self:__Release__()
end

function CMono:GetTickObj()
    error("This method should be overrided!")
end

function CMono:Invoke(funOrMethodName, t, args)
	-- """
	-- 发射,可以理解为创建定时器,过t这么多毫秒之后调以参数args调用funOrMethodName
	-- 加入列表而已.
	-- 返回tickID
	-- """
    local fun
    if type(funOrMethodName) == "function" then
        fun = funOrMethodName
    else
        fun = self[methodName]
    end
    if fun == nil then
        return nil
    end
    --延迟调用的队列,会把funOrMethodName对象放到队列中.
    local delayedCalls = self.delayedCalls
    --不能延迟0毫秒,这里预防传入的参数为0的情况下出错.
    if t == 0 then
        t = 0.001
    end
    --时间戳:即真正调用funormethodName的时刻.
    --当前时间+ 需要延迟的时间t
    local timestamp = Time.time + t
    -- find a insert position
    local insertPos = 1
    --遍历当前的延迟调用函数队列,找到插入的位置.确保插入之后队列中从前往后是按照时间先后排序的.
    --如果比所有的都大呢?或者说刚开始的时候列表为空.
    for i = 1, #delayedCalls do
        if timestamp < delayedCalls[i][2] then
            insertPos = i
            break
        end
        --add by ljz
        if i == #delayedCalls then
        	insertPos = #delayedCalls
        end
    end
    --每一个定时函数加入的时候需要有一个ID,相当于加入的顺序:1、2、3、4......
    -- add entry
    local tickID = self:_GetNextTickID()
    --插入延迟函数列表,存放的是一个定时函数的信息的集合,内容有:{tickID, 真正调用的时间, 函数对象, 参数}
    table.insert(delayedCalls, insertPos, {tickID, timestamp, fun, args})


    --比所有的都大或者刚开始的时候列表为空,就进入下面这一个判断条件.调用self._ScheduleDelayedCall(),这个函数的含义,从名字上我不能感受出来.弄清楚,然后改之!!!
    -- start a new tick if needed(这是干啥?)

    --前面的就好比给冲锋枪压入一颗子弹,每压入一颗子弹之后都会去检查有没有人在开枪,如果没有就叫机枪手上班了.
    if insertPos == 1 then
    	--开始真正的调用.
        self:_ScheduleDelayedCall()--计划/安排延迟调用.
    end
    return tickID
end

function CMono:CancelInvoke(invokeID)
    local entry = self:_RemoveDelayedCallEntry(invokeID)
    if #self.delayedCalls == 0 then
        self:GetTickObj():DelTick(self._OnDelayedCall)
    end
end

function CMono:ClearDelayedCalls()
    if #self.delayedCalls > 0 then
        self:GetTickObj():DelTick(self._OnDelayedCall)
    end
    self.delayedCalls = {}
end

function CMono:_GetNextTickID()
   self.curTickID = self.curTickID + 1
   return self.curTickID
end

-- remove a delayed call entry and do nothing else
function CMono:_RemoveDelayedCallEntry(invokeID)
    local q = self.delayedCalls
    for i, v in ipairs(q) do
        if v[1] == invokeID then
            local entry = table.remove(q, i)
            return entry
        end
    end
end

-- call & remove all overdue calls
function CMono:_FlushDelayedCall()
	-- """
	-- 超过时间的都执行掉,然后从
	-- """
    local q = self.delayedCalls
    local now = Time.time
    local n = #q
    while n > 0 do
        local entry = q[1]
        local tickID = entry[1]
        local timestamp = entry[2]
        if now >= timestamp then
            self:_RemoveDelayedCallEntry(tickID)--从table中移除.
            local fun = entry[3]
            local args = entry[4]
            if args == nil then
                fun()
            else
                fun(table.unpack(args))
            end
            n = n - 1
        else
            break
        end
    end
end

--计划延迟调用.说白了就是创建一个定时器去调用_OnDelayedCall
function CMono:_ScheduleDelayedCall()
    local q = self.delayedCalls
    if #q == 0 then--没有需要调用的函数
        return
    end
    local timestamp = q[1][2]

    --使用RegTick创建一个定时器, 去调用self._OnDelayedCall.
    --self._OnDelayedCall才是真正去延时调用的地方.前面那个Invoke只是加入列表,就好比给冲锋枪压入子弹.
    --选择第一个,因为第一个的时间是最短的.
    self:GetTickObj():RegTick(self._OnDelayedCall, self, 
        (timestamp - Time.time) * 1000, 1)
end

function CMono:_OnDelayedCall()
    self:_FlushDelayedCall()---- call & remove all overdue calls,调用需要执行的,并且从列表中移除.
    --再次启动计划延迟调用.如此循环,以达到永久不断的执行延迟列表中的函数.
    self:_ScheduleDelayedCall()
end

function WaitForSeconds(t)
    local due = Time.time + t
    local fn = function ()
        return Time.time >= due
    end
    return {Queue="WaitForSeconds", fn=fn}
end

null = {
    Queue="null", fn=function () return true end,
}






