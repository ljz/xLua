print("in platform 1111111111111")
 EPlatformType = {
    OSXEditor = 0,
    OSXPlayer = 1,
    WindowsPlayer = 2,
    OSXWebPlayer = 3,
    OSXDashboardPlayer = 4,
    WindowsWebPlayer = 5,
    WindowsEditor = 7,
    IPhonePlayer = 8,
    PS3 = 9,
    XBOX360 = 10,
    Android = 11,
    NaCl = 12,
    LinuxPlayer = 13,
    FlashPlayer = 15,
    LinuxEditor = 16,
    WebGLPlayer = 17,
    MetroPlayerX86 = 18,
    WSAPlayerX86 = 18,
    MetroPlayerX64 = 19,
    WSAPlayerX64 = 19,
    MetroPlayerARM = 20,
    WSAPlayerARM = 20,
    WP8Player = 21,
    BB10Player = 22,
    BlackBerryPlayer = 22,
    TizenPlayer = 23,
    PSP2 = 24,
    PS4 = 25,
    PSM = 26,
    XboxOne = 27,
    SamsungTVPlayer = 28,
    WiiU = 30,
    tvOS = 31,
    Switch = 32
}

local DefaultType = -1
print("in platform 2222222222222222222222")
gIsMobile = DefaultType
gIsEditor = DefaultType
gPlatformType = DefaultType
print("in platform 33333333333333333333333333")
function IsMobile()
    if IsClientAgent() then
        return false
    end
	if gIsMobile == DefaultType then
		gIsMobile = CS.UnityEngine.Application.isMobilePlatform
	end
	return gIsMobile
end

print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>zhe li ding yi le IsEditor")

function IsEditor()
    if IsClientAgent() then
        return false
    end
	if gIsEditor == DefaultType then
		gIsEditor = CS.UnityEngine.Application.isEditor
	end
	return gIsEditor
end

function GetPlatformType()
    if IsClientAgent() then
        return EPlatformType.WindowsEditor
    end
	
    if gPlatformType == DefaultType then
		gPlatformType = CS.Pet.LuaClient.GetPlatformType()
	end
	return gPlatformType
end

function IsAndroid()
    if IsClientAgent() then
        return false
    end
	if GetPlatformType() == EPlatformType.Android then
		return true
	end
	return false
end

function IsApple()
    if IsClientAgent() then
        return false
    end
    if GetPlatformType() == EPlatformType.IPhonePlayer then
		return true
	end
	return false
end

function IsOpenSDK( )
    return IsMobile() and IsUseSDK()
end
