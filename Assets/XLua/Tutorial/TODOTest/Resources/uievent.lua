--[[
    每一类只能用100个ID，非特殊需求不要占1000个了!!!!
--]]

-- 玩家基本信息的事件
GOLD_CHANGED = 1             -- 参数：endgold, delta
ENERGY_CHANGED = 2           -- 参数: energy, delta
ROLLER_CHANGED = 3           -- 参数：petid滚动滚轮的宠物id
DIAMOND_CHANGED = 4          -- 无参数：钻石数量改变
PORTRAIT_CHANGED = 5         -- 无参数：玩家头像改变
PLAYER_NAME_CHANGED = 6      -- 无参数：玩家昵称改变
PLAYER_VIP_CHANGED = 7       -- 无参数：玩家vip改变
MONEY_CHANGED = 8            -- 参数：moneyID, endMoney, delta


-- 背包系统的事件
ITEM_ADD = 101               -- 添加物品。参数：ItemID, EndCount, Delta
ITEM_DEL = 102               -- 删除物品。参数：ItemID, EndCount, Delta
ITEM_SELL_RESULT = 103       -- 物品出售
ITEM_USE_RESULT = 104        -- 物品使用
ITEM_PILE_OVERFLOW = 105     -- 物品堆叠超过上限

-- 宠物属性信息的事件
VITITEM_USED = 201           -- 活力道具使用
EXPITEM_USED = 202           -- 经验道具使用
BACKDATE_USED = 203          -- 宠物回溯
BACKDATE_FORSKILL = 204      -- 技能刷新
LV_CHANGED = 205               -- 宠物等级改变
NAME_CHANGED = 206            -- 名字改变
VIT_CHANGED = 207             -- 体力改变
PET_EXP_CHANGED = 208         -- 宠物经验改变

ADOPT_APPLY_SUCCESS = 250     -- 领养申请成功
NEW_PET = 251                 -- 新增宠物
ADOPT_APPLY_FINISH = 252      -- 领养宠物结束
PET_HAT_CHANGED = 253         -- 宠物帽子改变

-- 乐园、建筑事件
BUILDING_LEVEL_UP = 300        -- 参数：index, level
PARK_UP           = 301        -- 参数：parkID
BUILDING_FIXED 	  = 302        -- 参数: index
BUILDING_ATTACKED = 303        -- 参数：index, endHp
BUILDING_SELECT   = 304        -- 参数：index
BUILDING_FINISH   = 305        -- 参数：parkID
BUILDING_PREPARE  = 306        -- 参数：index, HideOrShow(bool)
BUIDING_BROKEN    = 307        -- 参数: index
PARK_EXP_CHANGED  = 308        -- 无参数：发生改变乐园经验值的事件，例如建筑物损坏，升级，修复等。
PARK_LEVEL_UP       = 310      -- 参数：level
RECEIVE_PARK_REWARD = 311      -- 无参数

--邮件事件
NEW_MAIL = 401                 -- 参数： mailID
MAIL_CHANGED = 402                -- 参数： mailID
TRADEMAIL_CHANGED = 403           -- 参数： tradeMailID
MAIL_TAKE_APPEND_SUCCESS = 404           -- 参数： mailID

-- 驻守事件
GUARD_CHANGED = 501            -- 参数：buildingID petID
GUARD_FRIEND_COMPLETE = 502    -- 参数：petID, playerID
FRIEND_GUARD_CHANGED = 503             -- 参数：friendGuardPet
GUARD_FRIEND_ERROR = 504               --无参数
FRIEND_GUARD_PET_LEAVE = 505    -- 无参数
CAN_BE_GUARD_INFO_UPDATE = 506  -- 是否可被驻守的信息更新，参数：{{_id: integer, canBeGuard: boolean}, ...}

-- 捣蛋事件
FOUND_TARGET = 601               -- 参数：随机捣蛋对象targetInfo
CHANGE_TARGET = 602             -- 参数：新选中的复仇对象targetInfo
CHANGE_DISPATCH = 603                -- 参数：新选中的捣蛋宠物
RECEIVE_REVENGE_LIST = 604            -- 参数： 复仇列表
RECEIVE_FRIEND_LIST = 605            -- 参数： 好友列表
TRICKSCENE_READY = 606                -- 无参数，获取到捣蛋所需数据
TRICK_START = 607                     -- 参数：捣蛋结果类型值，进攻宠物状态，防守宠物状态
ILLEGAL_TARGET = 608                  -- 无参数，选择目标不在可攻击列表（复仇列表好友列表或者陌生人）中
TRICK_WAIT_RESPOND = 609                -- 无参数：客户端请求变更捣蛋对象时等待服务器变更回复
TRICK_WANTED_END = 610                -- 参数：targetID,攻击的时候通缉已经结束了
TRICK_BUILDING_SELECT = 611             -- 参数：buildingIndex
TRICK_END = 612                         --无参数，没有下次捣蛋了（加倍体力滚轮）


--好友事件
NEW_FRIEND = 701 				-- 参数: friendID
REMOVE_FRIEND = 702			-- 参数: friendID
APPEND_FRIEND = 703        	-- 参数：nil
ON_RECOMMEND_FRIEND = 704  	-- 参数: friend列表
AGREE_OVER = 705               -- 参数: friendID, state
REFUSE_OVER = 706				-- 参数: friendID, state
RECEIVE_APPEND_FRIEND = 707    -- 参数: friendID, friendInfo
ADD_BEST_FRIEND = 708          -- 参数: friendID
REMOVE_BEST_FRIEND = 709       -- 参数: friendID
FRIEND_ENERGY = 710            -- 参数: const.FRIEND_EVENT,friendID,energy
ONEKEY_FRIEND_ENERGY = 711     -- 参数: nil
DELFRIENDAPPLICATION = 712     -- 参数：playerID

ON_BEST_FRIEND = 713 -- 切換到密友
ON_NORMAL_FRIEND = 714 -- 切換到普通好友
-- 借钱事件
BORROW_SELECT = 801                 -- 参数：index
BORROW_BALANCE = 802                -- 参数：borrowInfo
BORROW_CAMERA_RESET = 803        
BORROW_NEXT = 804
BORROW_START = 805                  -- 参数：result
BORROW_ESCAPE = 806                 -- 参数：result
BORROW_PET_SMELL = 807              -- 参数：pos
BORROW_PET_DISCOVER = 808           -- 参数：uiTransform
BORROW_RICHINFO_UPDATED = 809
BORROW_END = 810                    -- 无参数，没有下次借钱了（加倍体力滚轮）
NEXT_BORROW_TARGET = 811            -- 无参数，收到下个借钱目标（加倍体力滚轮）

-- 飞艇事件
AIRSHIP_COMP_UP = 901
AIRSHIP_COMP_CHECK_UP = 902
AIRSHIP_COMP_ACCELERATE_UP = 903
AIRSHIP_THROUGH_SCENE = 904

--宠物小窝事件
PETHOUSE_FOSTER_PET_DONE = 1001
PETHOUSE_GETBACK_PET_DONE = 1002
PETHOUSE_LV_UP = 1004

-- SDK
SDK_LOGIN_SUCCESS = 1100

-- 滚轮事件
WHEEL_STOP = 1200               -- 参数: targetGrid
WHEEL_RESULT = 1201             -- 参数：targetGrid, result, times

-- 排行榜事件
BILLBOARD_RECEIVE = 1300        -- 参数：sType

-- 通缉事件
WANTED_SUCCESS = 1400           -- 参数： itemCnt
WANTED_FAIL = 1401

-- boss
BOSS_FIRE_SUCCESS = 1500
BOSS_FIRE = 1501
BOSS_ON_FIRE = 1502
BOSS_REFRESH = 1503
TRADE_REFRESHTIME = 1504
BOSS_RING_STOP = 1507
BOSS_ON_RING_STOP = 1508
BOSS_CHANGE_NATURE = 1509
BOSS_END = 1510
BOSS_EXIST = 1511
BOSS_ERROR = 1512
BOSS_REWARD_RECEIVED = 1513
BOSS_ADD_REWARD = 1514
BOSS_SUMMONED = 1515
BOSS_END_BEFORE_FIRE = 1516

--交易
TRADE_PURCHASELIST = 1600 --TRADE_PURCHASELIST到达
TRADE_BUYRESULT = 1601
TRADE_REFRESHTIME = 1602

ADVENTURE_START = 1700 --开始探险
CAN_PETEXPLORE = 1701 --可以探险
RESET_PETEXPLORE = 1702 -- 重置探險
-- 拼图
PUZZLE_PIECE_CHANGED = 1800 -- 某一个碎片改变（安上/卸下）。参数：Index
PUZZLE_COMPLETE = 1801  -- 拼满了
PUZZLE_REWARD_GIVEN = 1802  -- 奖励已经领取

-- 红点
NEW_PANEL_OPEN = 1900   -- 新面板开启。参数：panel
NEW_PANEL_CREATE = 1901    -- 新面板创建。参数：panel

PANEL_CLOSE = 1902      -- 旧面板关闭，参数：rootName

--新手引导
GUIDE_NEXT = 2000
GUIDE_END = 2001
GUIDE_REQUEST_END = 2002

SHOW_FRIEND_SEARCH_BUTTON = 2100
AIRSHIP_UP_END_NOT_BY_DIAMOND = 2011 --飞艇升级倒计时结束（非使用钻石完成）

--剧情系统
STORY_STATE_CHANGED = 2200

--交配
MATING_FRIENDS_RECEIVE = 2300
MATING_STRANGER_RECEIVE = 2301
MATING_REQUEST_SEND = 2302
MATING_RESULT = 2303
MATING_CHANGED_STATE = 2304

WORK_SUCCESS = 2400
ADD_WORKPET = 2401
GET_WORK_MONEY = 2402
WORK_GOLD_REACH_LIMIT = 2403

TOURNAMENT_STATE = 2500
TOURNAMENT_CHANGE_MATCH_PET = 2501
TOURNAMENT_GAMEEND = 2502
TOURNAMENT_PHASEEND = 2503
TOURNAMENT_PHASECHANGED = 2504
TOURNAMENT_DRAWCARD = 2505
TOURNAMENT_STARTSETCARD =2506
TOURNAMENT_SETCARD = 2507
TOURNAMENT_UNSETCARD =2508
TOURNAMENT_SETTYPECARD =2509
TOURNAMENT_UNSETTYPECARD = 2510
TOURNAMENT_LOCKCARD =2511
TOURNAMENT_VERSUSRESULT = 2512
TOURNAMENT_FINISH = 2513
TOURNAMENT_ENTERBATTLE = 2514
TOURNAMENT_GAMESTART = 2515
TOURNAMENT_RANK = 2516
TOURNAMENT_MATCH_RESULT = 2517
TOURNAMENT_PLAY_PET_ANIMATION = 2518
TOURNAMENT_STOP_PET_ANIMATION = 2519
TOURNAMENT_INTERGRAL = 2520
TOURNAMENT_SELF_RANK = 2521
TOURNAMENT_INTERGRAL = 2522
TOURNAMENT_UPDATEUI = 2523


CHAT_MSG_RECEIVE = 2600
CHAT_NEW_MSG = 2601
CHAT_DISPLAY_MSG_LIST = 2602
CHAT_ENTER_CHANNEL = 2603
CHAT_WORLD_RECEIVE = 2696
CHAT_GUILD_RECEIVE = 2697
CHAT_PERSONAL_RECEIVE = 2698
CHAT_BROCAST_RECEIVE = 2699


--test
UI_TEST_EVENT = 20018

--[[
    每一类只能用100个ID，非特殊需求不要占1000个了!!!!
    请保持此注释在文件尾!!!!
--]]
--打工系统