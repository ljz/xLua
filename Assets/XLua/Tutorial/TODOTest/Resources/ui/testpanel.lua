Import "ui.uipanel"
local super = uipanel.CUIPanel
CTestPanel = Class("CTestPanel", super)
	
function CTestPanel:Init()
	print("this is in CTestPanel:Init")
	self:BindEvent()
end

function CTestPanel:BindEvent()
	print("this is in CTestPanel:BindEvent")
	self:OnClick("Button", self.OnClickLogin)
end

function CTestPanel:OnClickLogin()
	print(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>call CTestPanel:OnClickLogin")
end

function CTestPanel:Release()
	print("this is in CTestPanel:Release")
end

function CTestPanel:OnShow(args)
	print("this is in CTestPanel:OnShow")
end



