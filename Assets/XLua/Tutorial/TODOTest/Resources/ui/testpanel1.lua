require("ui.uipanel")

local super = CUIPanel
CTestPanelClass = Class("CTestPanelClass", super)

function CTestPanelClass:Init()
	print("CTestPanelClass:Init")
	super.Init(self)
end

function CTestPanelClass:Release()
	print("CTestPanelClass:Release")
end