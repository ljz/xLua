Import "ui.uipanel"
Import "uievent"


local super = uipanel.CUIPanel
CTestPanel = Class("CTestPanel", super)
	
function CTestPanel:Init()
	print("this is in CTestPanel:Init")
	self:BindEvent()
end

function CTestPanel:BindEvent()
	print("this is in CTestPanel:BindEvent")
	self:OnClick("Button", self.OnClickLogin)
	self:OnClick("Button_fb", self.OnClickLoginFB)
	self:OnClick("Button_gg", self.OnClickLoginGG)
	GetApp():Register(uievent.UI_TEST_EVENT, self, self.OnReceiveMsg)

end

function CTestPanel:OnReceiveMsg(msg)
	print("msg======="..msg)
end

function CTestPanel:OnClickLogin()
	print(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>call CTestPanel:OnClickLogin")
	GetApp():Dispatch(uievent.UI_TEST_EVENT, "hello, i am test event")
end

function CTestPanel:OnClickLoginFB()
	print(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>call CTestPanel:OnClickLoginFB")
end

function CTestPanel:OnClickLoginGG()
	print(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>call CTestPanel:OnClickLoginGG")
end


function CTestPanel:Release()
	print("this is in CTestPanel:Release")
end

function CTestPanel:OnShow(args)
	print("this is in CTestPanel:OnShow")
	local rapidjson = require("rapidjson")
	local t = rapidjson.decode('{"a":123}')
	print(t.a)
	t.a = 456
	local s = rapidjson.encode(t)
	print("json", s)
end



