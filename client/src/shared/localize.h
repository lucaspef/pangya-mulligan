#pragma once

enum localContentType_t
{
	S3_BONGDARISHOP = 0x01,
	S3_SCRATCH = 0x02,
	S3_MASCOT = 0x03,
	S3_TRADE = 0x04,
	S3_RECYCLE = 0x05,
	S3_BS_USABLE_TIMES = 0x06,
	S3_BS_4TH_RARE_ITEM = 0x07,
	S3_TRADE_BLOCK = 0x08,
	S3_RECORD_PLAYTIME = 0x09,
	S3_GM_TOOLKIT = 0x0a,
	S3_NEW_LAUNCH_PASSKEY = 0x0d,
	S3_ENCRYPTION_LV1 = 0x0e,
	S3_PARAN_CHANNELING = 0x10,
	S3_SHUFFLE_BONUS = 0x11,
	S3_ENCRYPTION_UPDATELIST = 0x12,
	S3_CADDIE_REPORT = 0x14,
	S3_EVENT_FLAG = 0x23, // 983 (645: 0x22; interpolado)
	S3_ICE_SPA = 0x27, // 983 (645: 0x26; interpolado)
	S3_DISCONNECT_CODE = 0x28, // 983 (645: 0x27; interpolado)
	S3_MATCH_HISTORY = 0x29, // 983 (645: 0x28; interpolado)
	S3_LEVELUP_ITEM = 0x2a, // 983 (645: 0x29; medido)
	S3_HUNDRED_MODE = 0x2c, // 983 (645: 0x2b; interpolado)
	S3_CHECK_SESSION = 0x2d, // 983 (645: 0x2c; interpolado)
	S3_SHOTRESULT_ENCRYPT = 0x2e, // 983 (645: 0x2d; interpolado)
	S3_INTRUSION = 0x2f, // 983 (645: 0x2e; interpolado)
	S3_SAFETEE_ITEM = 0x30, // 983 (645: 0x2f; interpolado)
	S3_AUTOCALIPERS_ITEM = 0x31, // 983 (645: 0x30; interpolado)
	S3_STRENGTH = 0x32, // 983 (645: 0x31; medido)
	S3_CADDIEBONUS = 0x33, // 983 (645: 0x32; interpolado)
	S3_ELFEAR = 0x34, // 983 (645: 0x33; interpolado)
	S3_INSERTGIFT_CHANGE = 0x35, // 983 (645: 0x34; interpolado)
	S3_PASSWORD_MD5 = 0x36, // 983 (645: 0x35; interpolado)
	S3_TUTORIAL_RENEWAL = 0x39, // 983 (645: 0x38; medido)
	S3_MAP_EVENT = 0x3a, // 983 (645: 0x39; interpolado)
	S3_ROOKIE_CHANNEL = 0x3b, // 983 (645: 0x3a; medido)
	S3_TIKI_REPORT = 0x3e, // 983 (645: 0x3d; interpolado)
	S3_ROOKIE_RESTRICT = 0x3f, // 983 (645: 0x3e; medido)
	S3_NEWYEAR_MONEY = 0x41, // 983 (645: 0x40; interpolado)
	S3_ITEMDROP_LOG = 0x45, // 983 (645: 0x44; interpolado)
	S3_ADD_USER_ACTION_LOG = 0x46, // 983 (645: 0x45; interpolado)
	S3_TUTORIAL_SECOND_GRADE = 0x47, // 983 (645: 0x46; interpolado)
	S3_OPEN_LUCKY_POUCH = 0x4b, // 983 (645: 0x4a; interpolado)
	S3_MYSTIC_RING = 0x4c, // 983 (645: 0x4b; medido)
	S3_EVENT_ADDTIONAL_GIVEAWAY = 0x4e, // 983 (645: 0x4d; interpolado)
	S4_PACKET_COMPRESS = 0x4f, // 983 (645: 0x4e; interpolado)
	S4_REPLAY_SYSTEM = 0x50, // 983 (645: 0x4f; medido)
	S4_SPECIALSHOT_RESTRICTION_MODIFY = 0x51, // 983 (645: 0x50; medido)
	S4_CARD_SYSTEM = 0x52, // 983 (645: 0x51; medido)
	S4_GOWITH_FRIEND = 0x54, // 983 (645: 0x53; medido)
	S4_INVITE_FRIEND = 0x55, // 983 (645: 0x54; medido)
	S4_REAL_MYROOM = 0x57, // 983 (645: 0x56; medido)
	S4_UCC = 0x58, // 983 (645: 0x57; medido)
	S4_TREASURE_HUNTER = 0x59, // 983 (645: 0x58; interpolado)
	S4_MATCHING_SYSTEM = 0x5a, // 983 (645: 0x59; medido)
	S4_GHOST = 0x5e, // 983 (645: 0x5d; interpolado)
	S4_GUID_BASE = 0x60, // 983 (645: 0x5f; interpolado)
	S4_MAILBOX_FOR_REAL_MYROOM = 0x61, // 983 (645: 0x60; medido)
	S4_REAL_MYROOM_TEST_ENABLE = 0x62, // 983 (645: 0x61; interpolado)
	S4_OPEN_RECYCLE_SPECIAL_TAB = 0x64, // 983 (645: 0x63; interpolado)
	S4_APPLY_CARD_FAMILYMODE = 0x65, // 983 (645: 0x64; interpolado)
	S4_OFFLINE_SHOP = 0x67, // 983 (645: 0x66; interpolado)
	S4_TREASURE_HUNTER_AD = 0x68, // 983 (645: 0x67; interpolado)
	S4_NEW_GUILD_POINT = 0x69, // 983 (645: 0x68; interpolado)
	S4_TOPPAGE_BTN_LINEUP = 0x6a,
	S4_BALL_SLIDE = 0x6b,
	S4_MASCOT_NO_LIMIT = 0x6c,
	S4_RANDOM_CHAR_MOTION = 0x6d,
	S4_NEW_PANGYAZONE_CALC = 0x6e,
	S4_CARD_SYSTEM_CHANGE = 0x6f,
	S4_ITEM_STORAGE = 0x70, // 983 (645: 0x71; medido)
	S4_FURNITURE_ABILITY = 0x75, // 983 (645: 0x76; interpolado)
	S4NT_VIEW_OFFLINE_GUILD = 0x7a, // 983 (645: 0x7d; medido)
	S4_FIX_ENCHANT = 0x7c, // 983 (645: 0x7f; medido)
	NT_FURNITURESAVE2 = 0x7d, // 983 (645: 0x80; interpolado)
	S4_NT_NICKNAME_CHANGE = 0x7e, // 983 (645: 0x81; medido)
	S4_NT_NICKNAME_CHANGE_2ND = 0x7f, // 983 (645: 0x82; interpolado)
	S4_NT_EVENT_MISSION = 0x80, // 983 (645: 0x83; medido)
	S4_NT_SC_ITEMSLOT = 0x81, // 983 (645: 0x84; interpolado)
	S4_NT_EVENT_BINGO = 0x82, // 983 (645: 0x85; medido)
	S4_HACKINGTOOL = 0x84, // 983 (645: 0x87; medido)
	MAX_LOCAL_CONTENTS = 0xa7 // 983: cmp esi, 0xa7 em IsLocalContent (00a7a8d3); era 0xa4
};

void InitLocalizeSystem();
bool IsLocalCountry(__int64 code);
bool IsLocalContent(localContentType_t code);
int GetLocalCountry();
