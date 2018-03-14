require("util.enum")
-- 一些UI层级，可以根据需求扩展
EUIDepth = Enum("EUIDepth", true)
EUIDepth.Scene = 1  -- 场景中的UI元素，最好就不要用多个canvas会比较耗，最好是创建完以后用UIFollow.cs来每帧更新坐标
EUIDepth.Normal = 2
EUIDepth.Fixed = 3
EUIDepth.Popup = 4

UIDepthOrdered = EUIDepth:GetAllKey()

UIShowType = {
	Normal = 1,    -- 普通窗口，一个一个的堆叠，窗口之间是并存的
	Stacked = 2,   -- 新的窗口出来后，会自动隐藏前一个stacked窗口，关闭后会恢复
	HideOthers = 3, -- 窗口出来后会将其余窗口全部隐藏，关闭后再恢复显示
	Queued = 4,    -- 两个以上同类窗口出现后，下一个窗口会暂缓显示，参数会被序列化存储起来，此窗口结束后方才显示
}

UICloseType = {
	Hide = 1,
	Destory = 2,
}

UIMaskType = {
	None = 1,             -- 无遮罩，点击可穿透
	Semitransparent = 2,  -- 半透明，点击不可穿透
	Transparent = 3,      -- 透明，点击不可穿透
}


-- 支持一些界面的动画
UIAnimType = {
	None = 1,             -- 无动画
	Zoom = 2,             -- 有动画，缩放
	Topin = 3,            -- 有动画，上方进入
	Bottomin = 4,         -- 有动画，下方进入
	--list不能启用layout组件
	Story = 5,            -- 剧情界面动画
	Leftin = 6,           -- 有动画，左边进入
	Rightin = 7,          -- 有动画，右边进入
	
}
UIAnimNames = {"None", "Zoom", "Topin", "Bottomin", "Story", "Leftin", "Rightin"}

--界面组件列表进入的动画
UIListAnim = {
	None = 1,
	Listin = 2,   -- 列表进入，注意列表进入的时候，要求预设下统一存放list路径
	-- list不能启用layout组件
	Textin = 3,
}

UIListAnimNames = {"None", "Listin", "Textin"}

UIMaskColor = {
	Semitransparent = {},
	Transparent = {},
}


--支持一些对象的变化动画
ObjectAnimType = {
	None = 1,
	Enlarge = 2,
	EnlargeMove = 3,
	RollerChange = 4,
	Slider = 5,
	EnergyChange = 6,
	LvUp = 7,
	NumberChange = 8,
}

ObjectAnimTypeNames = {
	"None",
	"Enlarge",
	"EnlargeMove",
	"RollerChange",
	"Slider",
	"EnergyChange",
	"LvUp",
	"NumberChange"
}

ObjectAnimTypeNames = {"None", "Enlarge", "EnlargeMove", "RollerChange",
"Slider", "NumberChange", "LvUp"}

MSGBOX_YES = 1
MSGBOX_NO = 2
MSGBOX_CANCEL = 4
MSGBOX_YESNO = MSGBOX_YES | MSGBOX_NO
MSGBOX_YESCANCEL = MSGBOX_YES | MSGBOX_CANCEL
MSGBOX_ALL = MSGBOX_YES | MSGBOX_NO | MSGBOX_CANCEL



UIClickType = {
	None = 1,
	Self = 2,
	Others = 3,
}
