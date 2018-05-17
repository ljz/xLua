require("util.res_loader")
Import "ui.uioptions"
Import "util.common_util"

CUIPanel = Class("CUIPanel")
CUIPanel.depth = uioptions.EUIDepth.Normal


function CUIPanel:CUIPanel(name)
	print("in CUIPanel:CUIPanel".. name)
	self.name = name
	self.resLoader = CResLoader()
	self.resLoader:Init()
	self._showArg = nil
	self._closeArg = nil
	-- self.tickObj = clienttick.CTick()
	-- self.tickObj:Init()
	self.platformBackHandle = {}
	self._redDotDrawContext = nil

	self:ReadSetting()
end


function CUIPanel:Init()
	self.listener = {}
	self._prev = nil
    self._next = nil
    self._queued = {} -- queue show panel requests
    self.visible = false
    self.position = nil
    self.listposition = nil
    self.pressTime = nil
    self.callback = nil
    self.CDTime = nil
    self.fxMap = {}
    self.mask = nil
    self.dropList = nil    
end

function CUIPanel:Release()
	-- if self._redDotDrawContext then
 --        self:ClearAllRedDotDrawContext()
 --    end
    -- GetApp():UnregisterAllByObj(self)
    -- if self._opMenu then    -- 在此界面上弹出的菜单
    --     self._opMenu:Release()
    --     self._opMenu = nil
    -- end    
    -- if self.gameObject ~= nil then
    --     CS.UnityEngine.Object.Destroy(self.gameObject)
    --     GetApp():GetResMgr():UnloadGameObject(self.gameObject)
    -- end
    -- if self.tickObj then
    --     self.tickObj:Release()
    --     self.tickObj = nil
    -- end
    -- self.resLoader:Release()
    -- self.gameObject = nil
    -- self.transform = nil
    -- self.anim = nil
    -- self._hidingAnim = nil
    -- self.mgr = nil
    -- self.pressTime = nil
    -- self.position = nil
    -- self.callback = nil
    -- self.CDTime = nil
    -- self.fxMap = nil
    -- self.mask = nil
    -- self.dropList = nil
    -- self:__Release__()
    -- if self.platformBackHandle then
    --     -- body
    --     for k,v in pairs(self.platformBackHandle) do
    --         CS.PlatformInputManager.instance:UnregisterBackButtonHandle(k)
    --         v = nil
    --     end
        
    --     self.platformBackHandle = nil
    -- end
end


function CUIPanel:ReadSetting()
    -- local mgr = GetApp():GetSettingMgr()
    -- self.uiSetting = mgr:GetLine("uiconfi_anim", self.name)
    -- if self.uiSetting ~= nil then
    --     self.openSound = self.uiSetting.OpenSound
    --     self.closeSound = self.uiSetting.CloseSound
    -- end
end

function CUIPanel:Find(path, t)
    -- local trans = self.transform:Find(path)
    -- if not trans then
    --     print("ui Sprite not found >>> ",self.name,path)
    -- end
    -- if t ~= nil then
    --     return trans:GetComponent(t)
    -- else
    --     return trans
    -- end
end

function CUIPanel:IsInUse()
    -- return self._prev ~= nil
end

function CUIPanel:BringToTop()
    -- self.transform:SetAsLastSibling()
end

function CUIPanel:RegTick(callback, args, interval, cnt)
    -- self.tickObj:RegTick(callback, args, interval, cnt)
end

function CUIPanel:DelTick(callback)
    -- self.tickObj:DelTick(callback)
end

function CUIPanel:DelAllTick()
    -- self.tickObj:DelAllTick()
end

function CUIPanel:OnClick(path, func, ...)
    return self:OnUIEvent(
        path, typeof(Button), "onClick", 
        "DEF_UI_CLICK",
        func, self, ...)
end

-- 不使用不定参数，方便后面扩展
-- OnClick则最方便，后面追加的扩展全部使用默认参数
function CUIPanel:OnClickEx(path, func, arg, sound)
    -- return self:OnUIEvent(
    --     path, typeof(Button), "onClick", 
    --     sound, 
    --     func, self, table.unpack(arg or {}))
end

function CUIPanel:OnEndEdit(path, func, ...)
    -- return self:OnUIEvent(
    --     path, typeof(CS.UnityEngine.UI.InputField), "onEndEdit", 
    --     nil,
    --     func, self, ...)
end

function CUIPanel:OnValueChanged(path, func, ...)
    -- return self:OnUIEvent(
    --     path, ON_VALUE_CHANGED_TYPES, "onValueChanged", 
    --     "DEF_UI_CLICK",
    --     func, self, ...)
end

function CUIPanel:OnValueChangedEx(path, func, arg, sound)
    -- if sound == nil then
    --     sound = "DEF_UI_CLICK"
    -- end
    -- return self:OnUIEvent(
    --     path, ON_VALUE_CHANGED_TYPES, "onValueChanged", 
    --     sound,
    --     func, self, table.unpack(arg or {}))
end

function CUIPanel:_SubTransform(pathOrTransform)
    if type(pathOrTransform) == "string" then
        return self.transform:Find(pathOrTransform)
    else
        return pathOrTransform
    end
end

function OnUIEventCallback(sound, f, ...)
    local aux_args = {...}
    local function callback(...)
        -- if sound ~= nil then
        --     GetApp():PlaySound(sound)
        -- end
        local insert = table.insert
        local args = {table.unpack(aux_args)}
        for _, v in ipairs({...}) do
            insert(args, v)
        end
        return f(table.unpack(args))
    end
    return callback
end

function CUIPanel:OnUIEvent(
        path, types, event,
        sound,
        func, ...)
    local transform = nil

    if type(path) == "string" then
        transform = self:_SubTransform(path)
    elseif type(path) == "userdata" then
        if path:GetType() == typeof(RectTransform) or  path:GetType() == typeof(GameObject) or path:GetType() == typeof(Transform) then
            transform = path
        end
    end
    
    if transform == nil then
        error(self.transform.name .. ": can't find child " .. path)
    end
    local comp = common_util.TryGetComponent(transform, types)

    local callback = OnUIEventCallback(sound, func, ...)

    comp[event]:RemoveAllListeners()
    comp[event]:AddListener(callback)
    return comp
end

local function _MakePointerEventCallback(self, f)
    -- return function(data)
    --     f(self, data)
    -- end
end

-- path: 按钮的路径
-- callback_dict：{eventType: callback}
-- eventType为string：PointerDown, PointerUp, ...
-- 详见：https://docs.unity3d.com/ScriptReference/EventSystems.EventTriggerType.html
-- callback为function(self, PointerEventData data)
-- 返回按钮
function CUIPanel:OnPointerEvent(path, callback_map)
    -- local transform = self:_SubTransform(path)   
    -- local gameObject = transform.gameObject
    -- local eventTrigger = gameObject:GetComponent(typeof(EventTrigger))
    -- if eventTrigger == nil then
    --     eventTrigger = gameObject:AddComponent(typeof(EventTrigger))
    -- end
    -- local entry = nil    
    -- for k, v in pairs(callback_map) do
    --     entry = EventTrigger.Entry()
    --     entry.eventID = EventTriggerType[k]
    --     entry.callback:AddListener(_MakePointerEventCallback(self, v))
    --     eventTrigger.triggers:Add(entry)
    -- end
    
    -- return transform:GetComponent(typeof(CS.UnityEngine.UI.Button))
end

function CUIPanel:RemovePointEvent(path, callback_map)
    -- local transform = self:_SubTransform(path)   
    -- local gameObject = transform.gameObject
    -- local eventTrigger = gameObject:GetComponent(typeof(EventTrigger))
    -- if eventTrigger == nil then
    --     return
    -- end
    -- --是否在这个里面
    -- local function IsInCallbackMap(id)
    --     for k, v in pairs(callback_map) do
    --         if k == id then
    --             return true
    --         end
    --     end
    --     return false
    -- end

    -- local i = 1
    -- while i <= #eventTrigger.triggers do
    --     if IsInCallbackMap(eventTrigger.triggers[i].eventID) then
    --         table.remove(eventTrigger.triggers[i])
    --     end
    --     i = i+1
    -- end
    -- if #eventTrigger.triggers == 0 then
    --     UnityEngine.GameObject.Destroy(gameObject:GetComponent(typeof(CS.UnityEngine.EventSystems.EventTrigger)))
    -- end
end


function CUIPanel:OnPointerImage(path, callback_map)
    -- local transform = self:_SubTransform(path)
    -- if transform == nil then
    --     error("can't find child transform " .. path)
    -- end
    -- local gameObject = transform.gameObject
    -- local eventTrigger = gameObject:GetComponent(typeof(EventTrigger))
    -- if eventTrigger == nil then
    --     eventTrigger = gameObject:AddComponent(typeof(EventTrigger))
    -- end
    -- local entry = nil    
    -- for k, v in pairs(callback_map) do
    --     entry = EventTrigger.Entry()
    --     entry.eventID = EventTriggerType[k]
    --     entry.callback:AddListener(_MakePointerEventCallback(self, v))
    --     eventTrigger.triggers:Add(entry)
    -- end
    
    -- return transform:GetComponent(typeof(CS.UnityEngine.UI.Image)) or transform:GetComponent("InverseMask")
end

function CUIPanel:OnHoldUIEvent(path, callback1, callback2)
    -- local callback_map = {}
    -- callback_map.PointerDown = callback1
    -- callback_map.PointerUp = callback2
    -- return self:OnPointerEvent(path, callback_map)
end

function CUIPanel:OnPressDown(buttonID)
    -- if self.pressTime == nil then
    --     self.pressTime = GetApp():GetSvrTime()
    -- end
    -- local curTime = GetApp():GetSvrTime()
    -- self.callback = self:MakeCallback(self.OnHoldingButton, self)
    -- self:RegTick(self.callback, buttonID, const.TIPS_HOLDTIME * 1000, 1)
end

function CUIPanel:OnPressUp(buttonID)
    -- self:DelTick(self.callback)
    -- if self.tips ~= nil then
    --     self.tips.gameObject:SetActive(false)
    -- end
    -- local curTime = GetApp():GetSvrTime()
    -- if self.pressTime == nil then
    --     self.pressTime = curTime
    -- end
    -- local deltaTime = curTime - self.pressTime
    -- if deltaTime < const.TIPS_HOLDTIME then
    --     self:OnReleaseButton(buttonID)
    -- else
    --     self:OnReleasedHoldButton(buttonID)
    -- end
    -- self.pressTime = nil
end

function CUIPanel:OnHoldingButton(buttonID)
end

function CUIPanel:OnReleaseButton(buttonID) 
end

function CUIPanel:OnReleasedHoldButton(buttonID)
end

function CUIPanel:OnClickedHelp()
    -- local text
    -- if self.uiSetting ~= nil then
    --     if self.uiSetting.HelpText ~= "" then
    --         text = GetApp():GetTextMgr():GetText(self.uiSetting.HelpText)
    --     end
    -- end
    -- GetApp():GetUI():OpenPanel("help", {text=(text or "help not configured!")})
end

function CUIPanel:SetText(path, text)
    -- local transform = self:_SubTransform(path)
    -- if transform == nil then
    --     error("can't find child transform " .. path)
    -- end    
    -- local component = transform:GetComponent("Text")
    -- component.text = text
    -- return component
end

function CUIPanel:SetImage(path, imagePath)
    -- local transform = self:_SubTransform(path)
    -- if transform == nil then
    --     error("can't find child transform " .. path)
    -- end
    -- return common_util.SetImage(transform, imagePath)
end

function CUIPanel:Update()
end

function CUIPanel:Show()
    if self.visible then
        return
    end
    self:_BecomeVisible()
    self:OnShow(self:_FlushShowArgument())
end

function CUIPanel:Hide()
    -- if not self.visible then
    --     GetApp():GetUI():ClearFromHideList(self);
    --     return
    -- end
    -- self.visible = false

    -- self._hidingAnim = nil
    -- self:EnableTouch(false)
    -- GetApp():PlaySound(self.closeSound)
    -- self:PlayHideAnimation()
end

function CUIPanel:PlayHideAnimation()
    -- local animType
    -- if self.uiSetting == nil then
    --     animType = UIAnimType.None
    -- else
    --     animType = self.uiSetting.CloseAnim
    -- end
    -- if animType == UIAnimType.None then
    --     GetApp():GetUI():OnPanelHideDone(self)
    --     return
    -- end
    -- local function onPanelHideDone()
    --     GetApp():GetUI():OnPanelHideDone(self)
    -- end
    -- animationmgr.CAnimationMgr:HidePanel(self, self.uiSetting, onPanelHideDone)
end

function CUIPanel:Back()
    -- if not self.visible then
    --     return
    -- end
    -- self.visible = false
    -- GetApp().mainUI:OnPanelBackDone(self)
    -- self._hidingAnim = nil
    -- self:EnableTouch(false)
end

function CUIPanel:_BecomeVisible()
    self._hidingAnim = nil
    if self.position == nil and self.transform.position ~= nil then
        self.position = self.transform.position
    end
    self.visible = true
    self:EnableTouch(true)
    -- animationmgr.CAnimationMgr:ShowPanel(self)
    
    self.gameObject:SetActive(true)    
    
    -- GetApp():PlaySound(self.openSound)
end


function CUIPanel:_backBecomeVisible()
    -- self._hidingAnim = nil
    -- if self.position == nil and self.transform.position ~= nil then
    --     self.position = self.transform.position
    -- end
    -- self.visible = true
    -- self.transform.position = self.position
    -- self.transform.localScale = CS.UnityEngine.Vector3(1, 1, 1)
    -- self:EnableTouch(true)
    -- self.gameObject:SetActive(true)
end

    
-- 由于上层Stack的界面隐藏而自动恢复
function CUIPanel:Recover()
    -- if self.visible then
    --     return
    -- end
    -- self:_BecomeVisible()
    -- self:OnRecover(self:_FlushCloseArgument())
end

function CUIPanel:BackRecover()
    -- if self.visible then
    --     return
    -- end
    -- self:_backBecomeVisible()
    -- self:OnRecover(self:_FlushCloseArgument())
end

function CUIPanel:OnRecover(arg)
end
    
function CUIPanel:CheckHideDone()
    -- return self._hidingAnim == nil or self._hidingAnim.time <= 0
end

function CUIPanel:OnShow(arg)
end

function CUIPanel:OnHide()
end

function CUIPanel:EnableTouch(b)
    --[[
    local g = self.gameObject:GetComponent(typeof(CS.UnityEngine.CanvasGroup))
    if g == nil then
        g = self.gameObject:AddComponent(typeof(CS.UnityEngine.CanvasGroup))
    end
    g.interactable = b
    --]]
end

function CUIPanel:LoadSprite(assetPath)
    -- return self:LoadAsset(assetPath, typeof(CS.UnityEngine.Sprite))
end

function CUIPanel:LoadAsset(assetPath, utype)
    -- return self.resLoader:LoadAsset(assetPath, utype)
end

function CUIPanel:SetShowArgument(arg)
    self._showArg = arg
end

function CUIPanel:SetCloseArgument(arg)
    self._closeArg = arg
end

function CUIPanel:_FlushCloseArgument()
    local arg = self._closeArg
    self._closeArg = nil
    return arg
end

function CUIPanel:_FlushShowArgument()
    local arg = self._showArg
    self._showArg = nil
    return arg
end

function CUIPanel:MakeCallback(f, ...)
   -- return common_util.MakeCallbackVaArg(f, ...)
end

-- transform: UnityEngine.Transform
-- offset: UnityEngine.Vector3
function CUIPanel:Follow(transform, offset)
    -- common_util.UIFollow(self.gameObject, transform, offset, true)
end

function CUIPanel:FormatGold(v, max_digit, sep)
    -- return common_util.format_gold(v, max_digit, sep)
end

function CUIPanel:FormatTimeText(timeInSec)
    -- return time_util.GetTimeFormatedStr(timeInSec)
end

function CUIPanel:FormatText2Clock(textComponent, time, callback, ...)
    -- if not textComponent.text then
    --     print("uipanel ==>node type wrong, not text component")
    --     return
    -- end
    -- if type(time) ~= "number" then
    --     print("uipanel ==>time formation wrong, not time stamp in secs", time)
    --     return
    -- end
    -- self.CDTime = time
    -- textComponent.text = time_util.GetTimeFormatedStr(self.CDTime,"%d%s%d%s")
    -- if time <= 0 then
    --     return
    -- end
    -- self.CDTimeCallback = self:MakeCallback(self.ColdDownCallback, 
    --     self, 
    --     textComponent, 
    --     callback,
    --     ...)
    -- self:RegTick(self.CDTimeCallback, nil, 1000, -1)
end

function CUIPanel:ColdDownCallback(textComponent, callback, ...)
    -- self.CDTime = self.CDTime - 1
    -- textComponent.text = time_util.GetTimeFormatedStr(self.CDTime,"%d%s%d%s")
    -- if self.CDTime <= 0 then
    --     self.CDTime = nil
    --     self:DelTick(self.CDTimeCallback)
    --     if callback then
    --         callback(...)
    --     end
    -- end
end

function CUIPanel:DelText2Clock()
    -- self:DelTick(self.CDTimeCallback)
end

function CUIPanel:OnCreateTips(text, showtime, deltaHeight, inverse)
    -- local prefab_path = "Prefabs/ui/text_tips.prefab"
    -- local prefab = self:LoadAsset(prefab_path)
    -- local camera = CS.UnityEngine.GameObject.Find("Camera"):GetComponent("Camera")
    -- local pos = camera:ScreenToWorldPoint(CS.UnityEngine.Input.mousePosition)
    -- if self.mTips ~= nil then
    --     UnityEngine.Object.Destroy(self.mTips)
    -- end
    -- if type(deltaHeight) ~= "number" then
    --     deltaHeight = 0
    -- end
    -- self.mTips = UnityEngine.Object.Instantiate(prefab, Vector3(pos[0]+5, pos[1] +deltaHeight, 0), CS.UnityEngine.Quaternion.identity, self.transform)
    -- local tipTrans = self.mTips.transform:Find("text")
    -- tipTrans:GetComponent("Text").text = text
    -- self.mTips:GetComponent("Text").text = text
    -- if inverse == true then
    --     self.mTips.transform.position = Vector3(pos[0] - 5 - tipTrans.sizeDelta.x, pos[1] +deltaHeight, 0)
    -- end
    -- if showtime ~= nil then
    --     GetApp():RegTick(self.OnDestroyTips, self, 1000*showtime, 1)
    -- end
end

function CUIPanel:OnDestroyTips()
    -- if self.mTips ~= nil then
    --     CS.UnityEngine.Object.Destroy(self.mTips)
    --     self.mTips = nil
    -- end
end

function CUIPanel:GetBuildingSpritePath(buildingID, buildingLv, isbroken)
    -- if not buildingID then
    --     return "gui/panel/badmanlist_pop/bad_icon_05.png"
    -- end
    -- local parkID = math.floor(buildingID / 100)
    -- if parkID == 0 then
    --     return "gui/panel/badmanlist_pop/bad_icon_05.png"
    -- end
    -- local idx = math.floor(buildingID % 100)
    -- local buildingPath = string.format("gui/icon/build/%d/build_%d_%d.png", parkID, idx, buildingLv)
    -- if isbroken then
    --     buildingPath = string.format("Prefabs/Garden/%d/res/Building_%d_%d_broke.png", parkID, idx, buildingLv)
    -- end
    -- return buildingPath
end

function CUIPanel:ApplyFxToModel(fxId,modelObj)
    -- body
    -- if self.fxMap[fxId] ~= nil then
    --     -- body
    --     self.fxMap[fxId]:SetActive(false)
    --     self.fxMap[fxId]:SetActive(true)
    -- else
    --     local mgr = GetApp():GetSettingMgr()
    --     local cfg = mgr:GetLine("sfx_type", fxId)
    --     if cfg == nil then
    --         -- body
    --         error("fx not exist! "..fxId)
    --         return
    --     end

    --     local mountPoint = modelObj.transform:Find(cfg.MountPoint)
    --     if mountPoint == nil then
    --         -- body
    --         mountPoint = self.transform;
    --     end

    --     local effect = UnityEngine.Object.Instantiate(self:LoadAsset(cfg.Path))
    --     effect.transform.parent = mountPoint
    --     effect.layer = CS.UnityEngine.LayerMask.NameToLayer("UI")
    --     local allChild = effect:GetComponentsInChildren(typeof(Transform))
    --     for i = 0, allChild.Length - 1 do--Set Layer to Default
    --         allChild[i].gameObject.layer = CS.UnityEngine.LayerMask.NameToLayer("UI")
    --     end
                
    --     effect.transform.localPosition = Vector3(cfg.Position[1],cfg.Position[2],cfg.Position[3])
    --     effect.transform.localRotation = Quaternion.identity
    --     effect.transform.localScale = Vector3(cfg.Scale[1],cfg.Scale[2],cfg.Scale[3])
    --     self.fxMap[fxId] = effect
    -- end
end

function CUIPanel:_CreatePlatformBackClickHandle()
    -- local callback = function ()
    --     if not self.visible then
    --         -- body
    --         return;
    --     elseif not GetApp():GetUI():IsInTop(self) then
    --         return;
    --     end

    --     for k,v in pairs(self.platformBackHandle) do
    --         print(self.name .. "call back")
    --         v(self)
    --     end    
    -- end
    -- return callback
end

function CUIPanel:RegisterPlatformBackClick(onBackBtnClick)
    -- body
    -- assert(#self.platformBackHandle == 0,"platform handle exist!")
    -- local id = CS.PlatformInputManager.instance:RegisterBackButtonHandle(self:_CreatePlatformBackClickHandle())
    -- self.platformBackHandle[id] = onBackBtnClick
end

function CUIPanel:GetOpMenu()
    -- if self._opMenu == nil then
    --     Import "client.ui.common.op_menu"
    --     self._opMenu = op_menu.COpMenu(self)
    -- end
    -- return self._opMenu
end

function CUIPanel:DrawRedDotWithContext(tagName, context)
    --print ("[RedDotMgr] DrawRedDotWithContext", tagName, context)
    -- local reddotmgr = GetApp():GetPlayer():GetRedDotMgr()
    -- if self._redDotDrawContext == nil then
    --     self._redDotDrawContext = {}
    -- end
    -- self._redDotDrawContext[tagName] = context
    -- reddotmgr:SetDrawContext(tagName, context)
    -- if context then
    --     reddotmgr:DrawNode(tagName)
    -- end
end

function CUIPanel:ClearAllRedDotDrawContext()
    -- local reddotmgr = GetApp():GetPlayer():GetRedDotMgr()
    -- for tagName, _ in pairs(self._redDotDrawContext) do
    --     reddotmgr:SetDrawContext(tagName, nil)
    -- end
    -- self._redDotDrawContext = nil
end

function CUIPanel:LoadItemSprite(itemID)
    -- local mgr = GetApp():GetSettingMgr()
    -- local cfg = mgr:GetLine("commonitem_itemlist", itemID)
    -- if cfg == nil then
    --     cfg = mgr:GetLine("coinitem_itemlist", itemID)
    -- end
    -- local itemSprite = self:LoadSprite(cfg.smallIcon or cfg.icon)
    -- return itemSprite
end

--在自定宽度的text里面显示字符串，太长的加"..."
function CUIPanel:SetTextByWidth(textNode, textContent, width)
    -- local textComponent = textNode:GetComponent("Text")
    -- if textComponent == nil then
    --     return
    -- end
    -- if textContent == nil or textContent =="" then
    --     textComponent.text = ""
    -- end
    -- if width == nil then 
    --     textComponent.text = textContent
    -- end

    -- local generator = textComponent.cachedTextGenerator
    -- local settings = textComponent:GetGenerationSettings(Vector2(width, textNode:GetComponent("RectTransform").rect.height))

    -- generator:Populate(textContent, settings)

    -- local generatedLenth = generator.characterCount
    -- local stringLength = string_util.GetStringLen(textContent)
    -- if stringLength > generatedLenth then --如果字符串太长
    --     textComponent.text = string_util.GetMaxLenString(textContent, generatedLenth - 1) .. "..."
    -- else
    --     textComponent.text = textContent
    -- end
end


























