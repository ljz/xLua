require("app.baseapp")
require("ui.msgqueue")
require("module")

CMainApp = Class("CMainApp" )

-- print("_G>>>>>>>>>>>>>"..tostring(_G))
-- for key, value  in pairs( _G ) do
-- 	print("key == ".. key.. "; value == "..tostring(value))
-- end

package.cpath = package.cpath ..";.bin/?.dll"
function CMainApp:Init()
	print("In CMainApp:Init()")
    CBaseApp.Init(self)
    self.version = "1.0.0" -- 后面会变成一个类
    self.mainUI = nil -- UI 模块
    self.accountInfo = nil
    self.player = nil
    self.config = nil
    self.stateMgr = nil
    self.networkMgr = nil
    self.sceneMgr = nil
    self.tipMgr = nil
    self.player_data = nil
    self.svrTime4Client = os.time()             -- 客户端维护的服务器时间
    self.svrSyncTime = nil                      -- 服务器同步下来的时间
    self.svrTimeElapsed = 0                     -- 用于模拟服务器时间
    self.svrSyncPoint = 0                       -- 服务器端同步时间时的客户端时间戳
    self.lastOKFrameTime = 33                   -- 上一帧的时间(33ms)
    self.svrTimeUpdateOK = false                -- 服务器时间更新问题
    self.loginTime = nil                        -- 客户端登陆时间
    self.lastFocusTime = 0
    self._resMgr = nil
    self._aiDirector = nil
    self._audioMgr = nil
    self._sfxMgr = nil
    self.localCfg = nil
    self.msgQueue = CMsgQueue()
    self.modelMgr = nil
    self.notifyMgr = nil
    self:InitGame()
    self:StartGame()
    -- self:InitEndlessCheck()
end


function CMainApp:InitGame()
	print(" >>================ in CMainApp:InitGame()")
    -- if platform.IsEditor() then
    -- RegGlobalObj("luaprint", print)
    -- RegGlobalObj("print", self.Print)
    -- end
    -- 各种模块初始化
    -- print("Game initializing")
    -- self:_PreInitGame() -- 注册全局函数

    -- self:_InitAccount()
    -- self:_InitNetwork()

    -- self:_InitStateMgr()
    
    -- if not IsClientAgent() then
    self._resMgr = CS.ResManager.instance
    print("CS ============================================= "..tostring(CS))
    print("CS.ResManager === "..tostring(CS.ResManager))
    print("CS.ResManager.instance === "..tostring(CS.ResManager.instance))
    -- print("self._resMgr ============"..self._resMgr)
    --     self._audioMgr = CS.AudioManager.instance
    --     self:_InitTipMgr()
    --     self:_InitLoadingIconMgr()
    self:_InitUI()
    --     self:_InitSceneMgr()
    --     self:_InitAIDirector()
    --     self:_InitSfxManager()
    --     self:_InitModelMgr()
    --     self:_InitNotifyMgr()
    --     self:ConnectGMTool()
    --     self:_InitInlineTextMgr()
    --     self:_InitDecoratorFactory()
    -- end

    -- self:_PostInitGame() -- 注册全局函数

    -- sdktool.StartProcessSDTCheckResultTick()
    
    self:_Init4Log()
end


function CMainApp:_Init4Log()
	print("in CMainApp:_Init4Log(); do nothing")
    -- if IsClientAgent() then
    --     return
    -- end
    
    -- if "true" == self:GetLocalConfig():Get(const.CONFIG.LOG_2_FILE, false) then
    --     CS.UnityEngine.GameObject.Find("logic"):AddComponent(typeof(CS.Pet.MobileLog))
    -- else
    --     CS.Pet.LuaClient.CleanLog()
    -- end
end

function CMainApp:_InitUI()
	Import "ui.uimanager"
	-- require("ui.uimanager")
	-- print(" >>>>>>  in CMainApp:_InitUI")
	self.uimanager = uimanager.CUIManager()
    self.uimanager:Init()
	-- print("self.uimanager ===")
	-- print(self.uimanager)

end


function CMainApp.Print(...)
    local info = debug.getinfo(2)
    if info then
        local path = info.short_src
        local size = #path
        if size > 4 then
            local pos = 1
            for i = 1, size do
                local p = string.find(path, "%.", pos)
                if nil == p then
                    break
                end
                pos = p + 1
            end
            path = string.sub(path, pos, size - 2)
        end
        luaprint(string.format("[%s.lua@L%s]", path, info.currentline), ...)
    else
        luaprint("[.lua@]", ...)
    end
end

function CMainApp:CreateTickObj()
    if self.tickObj then
        return
    end
    self.tickObj = clienttick.CTick()
    self.tickObj:Init()
end

function CMainApp:GetSceneMgr()
    return self.sceneMgr
end

function CMainApp:GetResMgr()
    return self._resMgr
end

function CMainApp:GetAIDirector()
    return self._aiDirector
end

function CMainApp:GetDecoratorFactory()
    return self._decoratorFactory
end

function CMainApp:GetNetworkMgr()
    return self.networkMgr
end

function CMainApp:GetAudioMgr()
    return self._audioMgr
end

function CMainApp:GetSfxMgr()
    return self._sfxMgr
end

function CMainApp:GetTipMgr()
    return self.tipMgr
end

function CMainApp:GetModelMgr()
    return self.modelMgr
end

function CMainApp:GetLoadingIconMgr()
    return self.loadingIconMgr
end


function CMainApp:GetNotifyMgr()
    return self.notifyMgr
end

function CMainApp:PlaySound(logic_name,cfg_name)
    if logic_name == nil or logic_name == "" then
        return
    end
    
    if self._audioMgr == nil then
        return
    end
    
    if cfg_name == nil then
        -- body
        cfg_name = "sound_def"
    end

    -- local setting = self:GetSettingMgr():GetLine(cfg_name, logic_name)
    -- self._audioMgr:Play(setting.Path)
end


function createPlaySoundCallback(snd)
    local function play()
        GetApp():PlaySound(snd)
    end
    return play
end

function CMainApp:PlaySoundSequence(seqConfig)
    if seqConfig == nil or #seqConfig == 0 then
        return
    end
    local seq = CS.DG.Tweening.DOTween.Sequence()
    for i = 1,#seqConfig,2 do
        local t = seqConfig[i]
        local snd = seqConfig[i + 1]
        seq:InsertCallback(t, createPlaySoundCallback(snd))
    end
end

function CMainApp:StartGame()
    --print("Game start")
    if self.stateMgr then
    	self.stateMgr:SetCurState(STATE.LOGO)
    else
    	print("you should init stateMgr before")
    end
end

function CMainApp:GetPlayer()
    return self.player
end

function CMainApp:GetUI()
    return self.mainUI
end

function CMainApp:GetUIManager()
	if self.uimanager == nil then
		self:_InitUI()
	end
	return self.uimanager 
end


function CMainApp:GetChatDisplayMgr( ... )
    return self._chatDisplayMgr
end


function CMainApp:ShowMsgbox(params)
    return self.mainUI:OpenPanel("msgbox",params)
end


function CMainApp:Register(event, obj, func, ...)
    self.msgQueue:Register(event, obj, func, ...)
end

function CMainApp:Unregister(event, obj, func)
    self.msgQueue:Unregister(event, obj, func)
end

function CMainApp:UnregisterAllByObj(obj)
    self.msgQueue:UnregisterAllByObj(obj)
end

function CMainApp:Dispatch(event, ...)
    self.msgQueue:SendMsg(event, ...)
end






local gApp = nil
function Init()
	gApp = CMainApp()
	gApp:Init()
end

function GetApp()
	return gApp
end

RegGlobalObj("GetApp", GetApp)

local function Start()

	-- local firstObj = CS.UnityEngine.GameObject("first UnityEngine GameObject created by lua")
	-- print(firstObj)

	--test the same level file require
	-- require("testrequiresamelevelfile")
	-- print("test the same level file require")
	-- local test = CSubClass()
	-- print(test)
	-- test.test()
	-- test.tests2()

	-- print("--test diff level file require")
	-- require("ui.uipanel")
	-- local testpanel = CTestPanelClass("testpanel")
	-- print(testpanel)
	-- testpanel:Init()

	Init()
end

Start()








