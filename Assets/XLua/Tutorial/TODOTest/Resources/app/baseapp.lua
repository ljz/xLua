-- Import "globalcommon.template.alltemplatemgr"

CBaseApp = Class("CBaseApp", CSingleton)

function CBaseApp:Init()
	self.tickObj = nil
	self.allTemplateMgr = nil
	self.textMgr = nil
	self.profilerMgr = nil
	-- self.settingMgr = settingmgr.CSettingMgr()
	-- self.settingMgr:Init()
	self.gmMgr = nil
	self.refreshMgr = nil
	self.textFilter = nil
	self.petExtraAttrMgr = nil
end

function CBaseApp:Release()
	if self.settingMgr then
		self.settingMgr:Release()
		self.settingMgr = nil
	end
	
	if self.tickObj then
		self.tickObj:Release()
		self.tickObj = nil
	end
	
	if self.profilerMgr then
		self.profilerMgr:Release()
		self.profilerMgr = nil
	end
	
	if self.gmMgr then
		self.gmMgr:Release()
		self.gmMgr = nil
	end
	
	if self.refreshMgr then
		self.refreshMgr:Release()
		self.refreshMgr = nil
	end

	if self.textFilter then
		self.textFilter:Release()
		self.textFilter = nil
	end

	if self.petExtraAttrMgr then
        self.petExtraAttrMgr:Release()
        self.petExtraAttrMgr = nil
    end
	
	self:__Release__()
end


--初始化统计
function CBaseApp:InitStatistics()
	-- if SysLuaEngine == nil or SysLuaEngine.InitStatistics == nil then
	-- 	return
	-- end
	
	--local filePath = string.format("log/statistics_%s_%s",self:GetServerType(),self:GetServerID())
	--SysLuaEngine.InitStatistics(filePath)
end

function CBaseApp:SysLog(logTb)
end

function CBaseApp:InitEndlessCheck()
	-- if SysLuaEngine and SysLuaEngine.OpenFrameCountCheck then
	-- 	SysLuaEngine.OpenFrameCountCheck()
 --    	SysLuaEngine.SetLimitFrameCount(300 * 10000)
	-- end

	-- if SysEngine and SysEngine.OpenFrameCountCheck then
	-- 	SysEngine.OpenFrameCountCheck()
 --    	SysEngine.SetLimitFrameCount(300 * 10000)
	-- end
end

function CBaseApp:CreateTickObj()
	LOGW("CBaseApp:CreateTickObj() should be override !!!!!!!")
end

function CBaseApp:GetTickObj()
	if self.tickObj then
		return self.tickObj
	end
	self:CreateTickObj()
	return self.tickObj
end

function CBaseApp:RegTick(InCallBack,InArgTable,InInterval,InCount)
	local tickObj = self:GetTickObj()
	if tickObj == nil then
		return nil
	end
	return tickObj:RegTick(InCallBack,InArgTable,InInterval,InCount)
end

function CBaseApp:DelTick(InCallBack)
	local tickObj = self:GetTickObj()
	if tickObj == nil then
		return
	end
	tickObj:DelTick(InCallBack)
end

function CBaseApp:DelAllTick()
	local tickObj = self:GetTickObj()
	if tickObj == nil then
		return
	end
	tickObj:DelAllTick()
end

function CBaseApp:IsServer()
	return false
end

-- function CBaseApp:GetAllTemplateMgr()
-- 	if self.allTemplateMgr == nil then
-- 		self.allTemplateMgr = alltemplatemgr.CAllTemplateMgr()
-- 		self.allTemplateMgr:Init()
-- 	end
-- 	return self.allTemplateMgr
-- end

function CBaseApp:GetSettingMgr()
    return self.settingMgr
end

function CBaseApp:GetTextMgr()
	-- if self.textMgr == nil then
	-- 	Import "globalcommon.text.textmgr"
	-- 	self.textMgr = textmgr.CTextMgr()
 --    	self.textMgr:Init()
 --    end
    -- return self.textMgr
end

function CBaseApp:GetProfilerMgr()
	-- if self.profilerMgr == nil then
	-- 	Import "globalcommon.perf.profilermgr"
	-- 	self.profilerMgr = profilermgr.CProfilerMgr()
 --    	self.profilerMgr:Init()
 --    end
 --    return self.profilerMgr
end

function CBaseApp:GetRefreshMgr()
    return self.refreshMgr
end

function CBaseApp:GetTextFilter()
	if self.textFilter then
		return self.textFilter
	end

	-- Import "globalcommon.text.textfilter"
 --    self.textFilter = textfilter.CTextFilter()
 --    self.textFilter:Init()
 --    return self.textFilter
end

function CBaseApp:GetPetExtraAttrMgr()
	-- if self.petExtraAttrMgr == nil then
	-- 	Import "globalcommon.gameplay.petextraattrmgr"
	-- 	self.petExtraAttrMgr = petextraattrmgr.CPetExtraAttrMgr()
 --    	self.petExtraAttrMgr:Init()
 --    end
    return self.petExtraAttrMgr
end