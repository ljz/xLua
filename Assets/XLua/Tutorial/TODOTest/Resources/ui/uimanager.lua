require("ui.uioptions")
require("util.lua_util")
Import "ui.wndid"
WNDALL = wndid.WNDALL
CUIManager = Class("CUIManager")

function CUIManager:Init()
	self.panels = {}
	self.id2panel = {}
	self.panelList = {}
    self.depthTop = {}

	self.localIndex = 0 -- for debug
	

	self:_InitUI()
end

function CUIManager:Destroy()
	print("CUIManager:Destroy")
end

function CUIManager:CloseAllPanels()
	for id, panel in pairs(self.id2panel) do
		self:ClosePanel(id)
	end
end

function CUIManager:DestroyAllPanels()
	for id, panel in pairs(self.panels) do
		if panel.Release then
			panel:Release()
		end
	end
	self.panels = nil
end


function CUIManager:_InitUI()
    -- print("CS.GameObject============"..CS.GameObject.Find)
    local GameObject = CS.UnityEngine.GameObject
    -- local Find = GameObject.Find
	self.canvas = CS.UnityEngine.GameObject.Find("LuaCanvas")
	if self.canvas == nil then
		error("Can not find canvas!")
	end

	CS.UnityEngine.Object.DontDestroyOnLoad(self.canvas)
	CS.UnityEngine.Object.DontDestroyOnLoad(CS.UnityEngine.GameObject.Find("EventSystem"))

	self.depthRoots = {}

	for i, depthName in pairs( UIDepthOrdered ) do
		local name = "Depth"..depthName.."Root"
		local go = GameObject(name, typeof(CS.UnityEngine.RectTransform))
		go.transform:SetParent(self.canvas.transform)
		self:_SetFullScreen(go)
		self.depthRoots[i] = go
	end
end

function CUIManager:_SetFullScreen(gameobject)
	local transform = gameobject.transform
	transform.anchorMin = CS.UnityEngine.Vector2.zero
	transform.anchorMax = CS.UnityEngine.Vector2.one
	transform.offsetMin = CS.UnityEngine.Vector2.zero
	transform.offsetMax = CS.UnityEngine.Vector2.zero
end

--还需要支持sync/async/assetbundle
function CUIManager:CreatePanel(name, wndCls, wndInfo)

	--load res
	local assetName = "Assets/res/Prefabs/UI/"..wndInfo[3].. ".prefab"
	local resMgr = GetApp():GetResMgr()
	local go = resMgr:LoadGameObject(assetName)

	--load

    print("wndCls ========="..tostring(wndCls))
    print(type( wndCls ))
    print(":::::")
	local panel = wndCls(name)
	self.localIndex = self.localIndex + 1

	go.name = wndInfo[3]

	go:SetActive(false)
	panel.gameObject = go
	panel.transform = go.transform
	panel.uiid = self.localIndex--估计是可以打印出来看.

	return panel
end

function CUIManager:AddPanel(panel)
    print("in >>>>>>>>>>>>>>>AddPanel")
    print(debug.traceback())
    local name = panel.name
    --该名字的panel表
    local panels = self.panels[name]
    if panels == nil then
        panels = {}
        self.panels[name] = panels
    end
    panel.mgr = self
    table.insert(panels, panel)
    
    self.id2panel[panel.uiid] = panel
end

function CUIManager:RemovePanel(panel)    
    local name = panel.name
    local panels = self.panels[name]
    if panels == nil then
        return
    end
    --遍历该名字的panel表,找到对应的panel对象,删除之.
    for pos, p in ipairs(panels) do
        if p == panel then
            table.remove(panels, pos)
            panel.mgr = nil
            
            self.id2panel[panel.uiid] = nil
            return
        end
    end
end


function CUIManager:OpenPanel(name, arg)
    local wndInfo = WNDALL[name]
    assert(wndInfo ~= nil, "NO window info named[" .. name .. "] FOUND!")

    local wndCls = GetClsByClsStr(wndInfo[1], wndInfo[2])
    assert(wndCls ~= nil, string.format("Load window cls fail! Module:[%s], cls:[%s]", wndInfo[1], wndInfo[2]))

    local panel = nil
    --单列,先不处理,后面优化再加.
    -- if wndCls.singleton then
    --     panel = self:_GetPanelByName(name)
    --     if panel ~= nil then
    --         -- print("OpenPanel: " .. name .. " , Singleton brought to top!")
    --         if panel == self.depthTop[panel.depth] then
    --             return true,panel
    --         end
    --         panel:BringToTop()
    --         self:ListRemove(panel)
    --     end
    -- end
    
    --这种情况是lru,lru是什么意思?看不懂,先不管.先跑最常规的一种.
    -- if panel == nil then
    --     panel = self.lru:Get(name)
    --     if panel ~= nil then
    --         self:AddPanel(panel)
    --     end
    -- end

    --创建一个,并且添加到一个表
    --这种情况需要初始化
    local needInit = false
    if panel == nil then
        panel = self:CreatePanel(name, wndCls, wndInfo)
        self:AddPanel(panel)
        needInit = true
    end
    
    --设置显示参数

    panel:SetShowArgument(arg)
    print("panel.depth ============")
    panel.transform:SetParent(self.depthRoots[panel.depth].transform, false)
    -- 设置完parent以后再进行Init
    if needInit then
        panel:Init()
        assert(panel.name ~= nil, "请调用基类的Init")
    end
    -- self:ListAppend(panel)
   

    -- 新面板创建,通知小红点模块.
    -- if needInit then
    --     GetApp():Dispatch(uievent.NEW_PANEL_CREATE, panel.gameObject.name)
    -- end
   
    --创建遮照,先不管. 
    -- local cfg = panel.uiSetting
    -- if cfg ~= nil then
    --     if cfg.MaskType ~= UIMaskType.None then
    --         self:PushMask(panel)
    --     end
    -- end
    --panel的显示类型,是栈类型.
    if panel.showType == UIShowType.Stacked then
        -- local prev = panel._prev
        -- local next_ = panel._next
        -- if prev.showType == panel.showType then
        --     self:_backPanel(prev)
        -- end
        -- if next_ == nil or next_.showType ~= panel.showType then
        --     self:_showPanel(panel)
        -- end
    --panel的显示类型是隐藏其他界面的类型.
    elseif panel.showType == UIShowType.HideOthers then
        -- local cur = panel
        -- while cur._prev ~= nil do
        --     if cur._prev.visible == true then
        --         self:_hidePanel(cur._prev)
        --         table.insert(self.hideList, cur._prev)
        --     end
        --     local mask = self.screenMasks[cur._prev.uiid]
        --     if mask then
        --         mask:SetActive(false)
        --     end
        --     cur = cur._prev
        -- end
        -- self:_showPanel(panel)
    else
        self:_showPanel(panel)
    end
    if cfg ~= nil then
    	--添加点击探测,估计是点击的时候出现声音之类的.
        self:AddClickDetect(panel, cfg.ClickType)
    end
    
    return panel.uiid,panel
end

function CUIManager:ListAppend(e)
	local depth = e.depth
	while self.depthTop[depth] == nil do
		depth = depth - 1
	end

	local prev = self.depthTop[depth]
	assert(prev ~= nil, "can not be nil , setienl's always there!")
	e._next = prev._next
	if prev._next ~= nil then
		prev._next._prev = e
	end
	prev._next = e
	e._prev = prev
	self.depthTop[e.depth] = e
end

function CUIManager:_showPanel(panel)
	-- self.hidePendings[panel] = nil--隐藏期间的界面列表.
	panel:Show()
	--通知小红点系统
	-- GetApp():Dispatch(uievent.NEW_PANEL_OPEN, panel.gameObject.name)
end

function CUIManager:AddClickDetect(panel, ClickType)
	-- if ClickType == uioptions.UIClickType.Self then
	-- 	--给界面一个EventTrigger组件,界面就可以点击了.
 --        local trigger = panel.gameObject:AddComponent(typeof(EventTrigger))
 --        local entry = EventTrigger.Entry()--创建一个条目
 --        --条目的类型为PointerClick
 --        entry.eventID = EventTriggerType.PointerClick
 --        --条目的回调函数是这个.
 --        entry.callback:AddListener(_MakePointerEventCallback(panel, self.ClickCallback))
 --        --把这个条目加到trigger组件中.
 --        trigger.triggers:Add(entry)
 --    elseif ClickType == uioptions.UIClickType.Others then
 --        local trigger = self.screenMasks[panel.uiid].gameObject:AddComponent(typeof(EventTrigger))
 --        local entry = EventTrigger.Entry()
 --        entry.eventID = EventTriggerType.PointerClick
 --        entry.callback:AddListener(_MakePointerEventCallback(panel, self.ClickCallback))
 --        trigger.triggers:Add(entry)
 --    end
end

function CUIManager:ClickCallback(panel, data)
	print("CUIManager:ClickCallback")
	self:ClosePanel(panel.name)
end


function CUIManager:ClosePanel(nameOrID, arg)

	local panel
	if type(nameOrID) == "number" then
		panel = self:_GetPanelByID(nameOrID)
	elseif type(nameOrID) == "string" then
		panel = self:_GetPanelByName(nameOrID)
	else
		return
	end
	self:_DoClosePanel(panel, arg)
end

function CUIManager:_DoClosePanel(panel, arg)
	if panel == nil then return end

	local unityObjectName = panel.gameObject.name
	local luaName = panel.name
	print("Close panel", unityObjectName, luaName)

	-- self:RemovePanel(panel)
	-- self:ListRemove(panel)

	panel:Release()



end

function CUIManager:_GetPanelByID(uiid)
	return self.id2panel[uiid]
end

function CUIManager:_GetPanelByName(name)
	local panels = self.panels[name]
	if panels == nil then
		return 
	end

	local n = #panels
	if n == 0 then
		return
	end

	return panels[n]
end

function CUIManager:OnUpdate()
	if not self.hidePendings then
		return
	end

	for panel, _ in pairs( self.hidePendings ) do
		if not panel:IsHideDone() then
			self:OnPanelHideDone(panel)
			self.hidePendings[panel] = nil
		end
	end
	return true, nil
end

--创建点击回调函数
local function _MakePointerEventCallback(panel, func)
	return function(data)
		func(self, panel, data)
	end
end



























