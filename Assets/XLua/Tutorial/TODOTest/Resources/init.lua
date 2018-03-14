
--[[
	游戏入口函数
	从这里开始lua逻辑
]]
require("app.main")

print("游戏入口函数, 从这里开始lua逻辑")
print(" in init.lua >>> game enter function, here start lua function")
function init()
	-- print(">>>>>>>>><<<<<<<<<<<<<<<<engine init")
	require("module")
	require("classdefine")
    -- Import "globalcommon.const"

    -- if const.startDebuggerOnInit then
        -- Import "globalcommon.debug.mobdebug"
        -- mobdebug.start(const.debugCfg.host, const.debugCfg.port)
    -- end
    
    -- Import "common.traceback"
	-- Import "common.gcmgr"
	-- print("import platform result >>>>>>>>>>>>>>>>")

	Import "common.platform"
	-- Import "network.networkutil"

    -- Import "util.fileutil"
   	Import "util.unity_const"
    -- Import "scene.transitcloud"
    -- Import "common.debug.stacktraceplus"
    -- print("platform ------------------------"..tostring(platform))
    -- for i, v in pairs( platform ) do
    -- 	print("i ===".. i)
    -- 	print("v ==="..tostring(v))
    -- end
    -- print("2333333333333333333333333") 
    -- print("platform.IsEditor--------------------------"..tostring(platform.IsEditor))
    -- if platform.IsEditor() then
    --     RegGlobalObj("ISDEBUG", true)
    -- else
    --     RegGlobalObj("ISDEBUG", false)
    -- end
    RegGlobalObj("ISDEBUG", true)

    
    unity_const.InitUnityConst()
	-- fileutil.InitFilePath()
	-- gcmgr.InitGCParams()
	
	-- local baseCycle = 60
	-- SysEngine.EngineInit(baseCycle)
end



-- loadfile is lua function
_G["SysLoader"] = {LoadScript = loadfile}



function start()
	print("start")
    -- if const.startDebuggerOnTraceback then
    --     debug.traceback = errorMessageWithDebugger
    --     RegGlobalObj("__JOYLUCK_TRACKBACK__", errorMessageWithDebugger)
    -- else
    --     debug.traceback = StackTracePlus.stacktrace
    --     RegGlobalObj("__JOYLUCK_TRACKBACK__", StackTracePlus.stacktrace)
    -- end
	
	Import "app.main"
	print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Import Finish")
	main.Init()
	local uimanager = GetApp():GetUIManager()
	uimanager:OpenPanel("testpanel")
	
	-- 初始化SDK
	-- Import "client.sdk.sdkmgr"
	-- sdkmgr.InitSDKMgr()
	
	-- 调整音量,取值范围[0,1]
	-- GetApp():GetAudioMgr():SetGroupVolume("music", 0.4)
	--GetApp():GetAudioMgr():SetGroupVolume("sfx", 1)

	-- 本地设置
	-- GetApp():LocalSettingInit()
	-- sdk 登录
	-- 登录成功后，patch 更新
	-- 进入游戏
end


function HookRun( func,...)
	local Status,Msg = xpcall(func,traceback.__JOYLUCK_TRACKBACK__,...)
	if not Status then
		_MyTbProcess(Msg)
	end
end

function _MyTbProcess(Msg)
	Import "globalcommon.const"
	--LOGE(Msg) traceback里边已经有捕获了
	if const.ISDEBUG or not IsPublicServer() then
		Import "client.gm.tb_notifition"
		tb_notifition._ProcessTB(Msg)
	end
end

function OnTickUpdate()
	GetApp():OnUpdate()
end

function FixedUpdate()
    GetApp():OnFixedUpdate()
end

function LateUpdate()
    GetApp():OnLateUpdate()
end

function OnGMRun(svrType,serverID,destPlayerID,cmdText,isCode)
	Import "client.gm.gm_client"
	local cmdData = {
		["SvrType"] = svrType,
		["DestServerID"] = serverID,
		["DestPlayerID"] = destPlayerID,
		["CmdText"] = cmdText,
		["IsCode"] = isCode,
	}
	GetApp():GetGmMgr():RunGM(cmdData)
end

function XLuaLogCallBack(InType,InMsg)
	print(InType,InMsg)
end

function OnApplicationFocus(hasFocus)
	GetApp():OnFocus(hasFocus)
end

function OnApplicationPause(pauseStatus)
	GetApp():OnPause(pauseStatus)
end

-- 游戏退出前调用
function OnApplicationQuit()
	GetApp():OnQuit()
end

function OnUpdateNetworkState(InType)
	local networkMgr = GetApp():GetNetworkMgr()
	if networkMgr == nil then
		return
	end
	networkMgr:UpdateNetworkState(InType)
end

function IsClientAgent()
	if __IsClientAgent__ then
		return true
	end
    return false
end

function IsClient()
	if CS or IsClientAgent() then
		return true	
	end
    return false
end

function dofile()
end

function loadfile()
end

function load()
end

function loadstring()
end

function errorMessageWithDebugger(message)
    Import "globalcommon.debug.mobdebug"
    mobdebug.start(const.debugCfg.host, const.debugCfg.port)
    mobdebug.pause()
    return message
end