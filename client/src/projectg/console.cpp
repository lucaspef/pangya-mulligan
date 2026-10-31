#include "minatl.h"
#include "console.h"
#include "guildactor.h"
#include "powergauge.h"
#include "sky.h"
#include "packet.h"
#include "camerawork.h"
#define REBANG_CONSOLE_CPP // HACK: see quadtree.h
#include "quadtree.h"
#include "headicon.h"
#include "clientsetting.h"
#include "club.h"
#include "wind.h"
#include "polysoup.h"
#include "wvideo.h"
#include "wresrcmng.h"
#include "contentsdoc.h"
#include "autotarget.h"
#include "fresh.h"
#include "lobbymain.h"
#include "jp/clubsetstat.h"
#include "../shared/LuaSystem/luascriptmanager.h"
#include <string.h>
#include <algorithm>
#include <deque>
#include <map>

extern float causticSpeed; // pool.cpp
extern int causticAlpha;

struct sCausticRb
{
	float speed;
	int alpha;
};

#define COMMAND_COUNT 0xc6 // 983: 0xc6 (645: 89)

// members missing in PCH headers: helper classes + /alternatename
class CGolfDocEx
{
public:
	void UpdateBright(float value);
};
class CSceneManagerEx
{
public:
	void DropTest();
};
#pragma comment(linker, \
	"/alternatename:?UpdateBright@CGolfDocEx@@QAEXM@Z=?UpdateBright@CGolfDoc@@QAEXM@Z")
#pragma comment(linker, \
	"/alternatename:?DropTest@CSceneManagerEx@@QAEXXZ=?DropTest@CSceneManager@@QAEXXZ")

sTeamData* __fastcall TEAM(unsigned char index);

// tasks of other units without headers: only their RTTI is needed
class CShopTask
{
public:
	static const WRTTI m_RTTI;
};
class CRealMyRoomTask
{
public:
	static const WRTTI m_RTTI;
};

extern void PerformPowerBarSetting();
bool __fastcall IsMyTurn(bool);
unsigned char __fastcall MyIndex();

struct IRbVObj
{
	virtual void vfunc0();
	virtual void vfunc1();
	virtual void vfunc2();
	virtual void vfunc3();
	virtual void vfunc4();
};

// defined (inline) by shareddoc.cpp
const WVector& GetHoleCupPos(); // 983: devolve referencia
#define MYPLAYER() PLAYER(CSharedDoc::Instance()->m_pGolfDoc->m_currentPlayer)

// bridge to members not declared in the headers yet (offset in the original layout)
#define FIELD(T, p, off) (*(T*)((char*)(p) + (off)))

const float CONSOLE_HEIGHT = 302.0f;
const float CONSOLE_MARGIN = 4.0f;

// unused, but the original emitted CONSOLE_MARGIN and g_HUGE into .rdata
static bool IsInsideConsole(float y)
{
	return y < CONSOLE_HEIGHT && y > CONSOLE_MARGIN && y < g_HUGE;
}

struct
{
	char* name;
	int id;
	char* help;
} command_table[COMMAND_COUNT] = {
	// 983: tabela em 00e16f08 (0xc6 entradas; ids renumerados e ~110 comandos novos em relacao ao 645)
	{ "help", 0x0, "help" },
	{ "quit", 0x1, "quit : terminate client" },
	{ "clear", 0x2, "clear" },
	{ "changegamescreenresolution", 0x3, "changegamescreenresolution [w] [h]" },
	{ "caddie", 0x4, "caddie [serial no.] [level(1-4)]" },
	{ "caddiemotion", 0x5, "caddiemotion [motionname]" },
	{ "map", 0x6, "map [value(0-2)]" },
	{ "hole", 0x7, "hole [value(1-18)][forcechange(1)]" },
	{ "repeat", 0x8, "repeat [value: (1-18) : set repaet, (otherwise) : cancel repeat]" },
	{ "weather", 0x9, "weather [value(0-3)]" },
	{ "heal", 0xa, "heal [value]" },
	{ "clubset", 0xb, "clubset [value(0-3)]" },
	{ "combo", 0xc, "combo [value]" },
	{ "tag", 0xd, "tag" },
	{ "render", 0xe, "render [value(0-3)]" },
	{ "fill", 0xf, "fill [value(0-2)]" },
	{ "fov", 0x10, "fov [value(10-120)]" },
	{ "dist", 0x11, "dist" },
	{ "save", 0x12, "save [filename]" },
	{ "load", 0x13, "load [filename]" },
	{ "wind", 0x14, "wind [value(0-9)] [value(0-360)]" },
	{ "world", 0x15, "world : \xbf\xf9\xb5\xe5\xc0\xc7 \xc5\xa9\xb1\xe2\xb8\xa6 \xbe\xcb\xb7\xc1 \xc1\xdc" },
	{ "cull", 0x16, "cull [value]" },
	{ "timeout", 0x17, "timeout : \xc5\xb8\xc0\xd3 \xbe\xc6\xbf\xf4 \xc5\xe4\xb1\xdb (on:default)" },
	{ "cameracheck", 0x18, "cameracheck : \xc4\xab\xb8\xde\xb6\xf3 \xc3\xbc\xc5\xb7 \xc5\xe4\xb1\xdb (on:default)" },
	{ "slope", 0x19, "slope [value] (1.7:default)" },
	{ "texture", 0x1a, "texture [filename]" },
	{ "effectlevel", 0x1b, "effectlevel [value(0-2)] (1:default)" },
	{ "aveffect", 0x1c, "aveffect [nick] [effect name]" },
	{ "mipfilter", 0x1d, "mipfilter [value(0-5)] (1:default)" },
	{ "mparts", 0x1e, "mparts [part name]" },
	{ "dparts", 0x1f, "dparts [typeid]" },
	{ "hair", 0x20, "hair [typeid]" },
	{ "statistics", 0x21, "statistics" },
	{ "pangya", 0x22, "pangya [value(n)] (1:default)" },
	{ "alpangya", 0x23, "alpangya [value(n)] (1:default)" },
	{ "ball", 0x24, "ball [serial no.]" },
	{ "reloadball", 0x25, "reloadball" },
	{ "scale", 0x26, "scale [value(1-\xa1\xc4)] (1:default)" },
	{ "trace", 0x27, "trace [value] (value=\xc7\xc1\xb7\xb9\xc0\xd3 \xb9\xf8\xc8\xa3)" },
	{ "grid", 0x28, "grid [value]" },
	{ "gridshow", 0x29, "gridshow [0-1]" },
	{ "drop", 0x2a, "drop" },
	{ "award", 0x2b, "award" },
	{ "ai", 0x2c, "ai" },
	{ "key", 0x2d, "key" },
	{ "special", 0x2e, "special [shot ID( Tomahawk:1, Cobra:2, Spike:3 )]" },
	{ "lowest", 0x2f, "lowest" },
	{ "highest", 0x30, "highest" },
	{ "debugline", 0x31, "debugline [value]" },
	{ "god", 0x32, "god" },
	{ "togid", 0x33, "toggle id" },
	{ "power", 0x34, "power value[0-100]" },
	{ "impact", 0x35, "impact value[0-30]" },
	{ "control", 0x36, "control value[0-30]" },
	{ "spin", 0x37, "spin value[0-30]" },
	{ "curve", 0x38, "curve value[0-30]" },
	{ "togglefullscreen", 0x39, "toggle fullscreen" },
	{ "bright", 0x3a, "bright [value(0-1)] (1:default)" },
	{ "holecup", 0x3b, "holecup [-1 - 2] (-1:random, 0-2:specific holecup location)" },
	{ "bounce", 0x3c, "bounce [0-\xa1\xc4] (1:default)" },
	{ "roll", 0x3d, "roll [0-\xa1\xc4] (1:default)" },
	{ "condition", 0x3e, "condition [-50-50] (0:default)" },
	{ "temperature", 0x3f, "temperature" },
	{ "humidity", 0x40, "humidity [0-100]" },
	{ "unbalance", 0x41, "unbalance impact control" },
	{ "green", 0x42, "green [value(0-\xa1\xc4)] (0.2:default)" },
	{ "camvel", 0x43, "camvel [value(0-\xa1\xc4)] (-1:default)" },
	{ "mouse", 0x44, "mouse [value(0.01-\xa1\xc4)] (1.0:default)" },
	{ "transparent", 0x45, "transparent (on:default)" },
	{ "plist", 0x46, "plist : part list of cur character" },
	{ "camtarget", 0x47, "camtarget [value(0-\xa1\xc4)] (100.0:defalut)" },
	{ "paint", 0x48, "paint mesh_index (-1:default)" },
	{ "speed", 0x49, "speed (5.0:default)" },
	{ "listbres", 0x4a, "list bres" },
	{ "lake", 0x4b, "lake speed alpha" },
	{ "showmethemoney", 0x4c, "user pang and cookie is increase" },
	{ "gravity", 0x4d, "gravity modifier [value]" },
	{ "bar", 0x4e, "input number [2~9]" },
	{ "tw cmd all", 0x4f, "tw cmd all : \xbb\xe7\xbf\xeb \xb0\xa1\xb4\xc9\xc7\xd1 \xb8\xf0\xb5\xe7 \xc6\xae\xc0\xa7\xc4\xbf \xc3\xb5\xc1\xa6 \xb8\xed\xb7\xc9\xbe\xee\xb5\xe9 \xb8\xf1\xb7\xcf\xc0\xd4\xb4\xcf\xb4\xd9." },
	{ "tw list", 0x50, "tw list : \xc6\xae\xc0\xa7\xc4\xbf\xb7\xce \xb5\xee\xb7\xcf\xb5\xc8 \xc5\xac\xb7\xa1\xbd\xba \xb8\xf1\xb7\xcf\xc0\xbb \xc7\xa5\xbd\xc3 \xc7\xd5\xb4\xcf\xb4\xd9." },
	{ "tw using", 0x51, "tw using : tw list\xbf\xa1 \xb5\xee\xb7\xcf\xb5\xc8 \xc5\xac\xb7\xa1\xbd\xba\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xd4\xc0\xb8\xb7\xce\xbd\xe1, \xc0\xfc\xc3\xbc \xb8\xed\xb7\xc9\xbe\xee\xb0\xa1 \xbe\xc6\xb4\xd1 \xc7\xd8\xb4\xe7 \xc6\xae\xc0\xa7\xc4\xbf\xc0\xc7 \xb8\xed\xb7\xc9\xbe\xee\xb8\xa6 \xb9\xd9\xb7\xce \xc7\xd8\xbc\xae\xc7\xd5\xb4\xcf\xb4\xd9." },
	{ "calipers", 0x52, "set default callipers [0 ~ 5]" },
	{ "posmask", 0x53, "posmask %d(TypeID), %d(PosMask)" },
	{ "texname", 0x54, "texname %d(TypeID), %d(0:normal tex 1:original tex), %d(tex num : 0 ~ 2), %s(tex name)" },
	{ "petname", 0x55, "petname %d(TypeID), %s(PetName)" },
	{ "vectortype", 0x58, "vectortype [0-1]" },
	{ "vectorspeed", 0x59, "vectorspeed [%d]  default limitspeed 100" },
	{ "vectorheight", 0x5a, "vectorheight [%d] default bound height 30" },
	{ "vectorcount", 0x5b, "vectorcount [%d] default limit bound count 20" },
	{ "luacmd", 0x56, "luacmd [index]  index(0:reload,1:runfile \"test.lua\" 2:peras-only)" },
	{ "lua", 0x57, "lua \"command string\" " },
	{ "togglerendershotstatistics", 0x5c, "toggle render shot statistics" },
	{ "loadshotstatistics", 0x5d, "load shot statistics [normal/natural/all(0/1/2)]" },
	{ "clustershotspot", 0x5e, "cluster shot spot [numCenters(> 0)]" },
	{ "redisposefielditem", 0x5f, "redispose field item" },
	{ "saveshotspotcenter", 0x60, "save shot spot centers [data+summary/summary(0/1)]" },
	{ "screenshotshotspotcenters", 0x61, "screenshot shot spot centers [quality(0-100, default 100)][loadShotStat(0/1, default 1)]" },
	{ "aabb", 0x62, "show aabb Box" },
	{ "svcontrol_set", 0x63, "svcontrol_set [prop]" },
	{ "svcontrol_add", 0x64, "svcontrol_add [prop]" },
	{ "reloadxml", 0x65, "fresh\xbf\xa1\xbc\xad \xbb\xe7\xbf\xeb\xc7\xcf\xb4\xc2 xml \xb4\xd9\xbd\xc3 \xb7\xce\xb5\xf9\xc7\xcf\xb0\xed \xb4\xd9\xc0\xbd \xb9\xf8\xbf\xa1 \xc7\xd8\xb4\xe7 \xb7\xb9\xc0\xcc\xbe\xc6\xbf\xf4\xc0\xbb \xbf\xad \xb6\xa7 \xb9\xdd\xbf\xb5\xb5\xca" },
	{ "reloadbitmap", 0x66, "fresh\xbf\xa1\xbc\xad \xbb\xe7\xbf\xeb\xc7\xcf\xb4\xc2 bitmap cache \xb8\xa6 \xc1\xf6\xbf\xec\xb0\xed \xb4\xd9\xc0\xbd \xb9\xf8\xbf\xa1 \xbf\xe4\xc3\xbb\xc0\xcc \xc0\xd6\xc0\xbb \xb6\xa7 \xb4\xd9\xbd\xc3 \xc0\xd0\xbe\xee\xb5\xe9\xc0\xd3" },
	{ "timecontrol", 0x67, "timecontrol : [0, 1]" },
	{ "hio_holecup_scale", 0x68, "hio_holecup_scale [\xb1\xe2\xba\xbb\xb0\xaa(0)/\xb0\xed\xc1\xa4(1~9)] : \xc8\xa6\xc4\xc5\xc5\xa9\xb1\xe2 \xc1\xf6\xc1\xa4" },
	{ "set_limiteitemlist", 0x69, "set_limiteitemlist : excelItemtable setting" },
	{ "next_levelup_comment", 0x6a, "next_levelup_comment : \xb4\xd9\xc0\xbd \xb7\xb9\xba\xa7 \xc6\xaf\xc0\xfc\xba\xb8\xbb\xf3 \xc1\xa4\xba\xb8" },
	{ "sdscale", 0x6b, "sdscale [petname(pet or bpet)] [bodyscale(> 0)] [headscale(> 0)]" },
	{ "fresh_form", 0x6c, "fresh_form [form_name]" },
	{ "fresh_reload_form", 0x6d, "fresh_reload_form [form_name] : Bitmap\xc0\xc7 \xb8\xae\xbc\xc2\xc0\xcc \xc0\xcf\xbe\xee\xb3\xaa\xb4\xc2 \xb8\xb8\xc5\xad \xb9\xdd\xb5\xe5\xbd\xc3 \xbd\xba\xc5\xb8\xc6\xae \xb8\xf0\xb5\xe5 20\xb9\xf8\xb0\xfa \xb0\xb0\xc0\xba \xb0\xf7\xbf\xa1\xbc\xad \xbb\xe7\xbf\xeb\xc7\xcf\xb1\xe6 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9." },
	{ "hub_dev /?", 0x6e, "hub_dev /? : \xb0\xb3\xb9\xdf\xc0\xda \xb1\xe2\xb4\xc9 \xb5\xb5\xbf\xf2\xb8\xbb <HUB>" },
	{ "hub_dev", 0x6f, "hub_dev %d : \xb0\xb3\xb9\xdf\xc0\xda \xb1\xe2\xb4\xc9 <HUB>" },
	{ "quest_list", 0x70, "quest_list : \xc7\xf6\xc0\xe7 \xba\xb8\xc0\xaf\xc7\xd1 \xc4\xf9\xbd\xba\xc6\xae \xb8\xae\xbd\xba\xc6\xae" },
	{ "quest_accept", 0x71, "quest_accept [UID(%d)] : \xc4\xf9\xbd\xba\xc6\xae \xbc\xf6\xb6\xf4 \xbf\xe4\xc3\xbb" },
	{ "quest_accept_all", 0x72, "quest_accept_all : \xc0\xfc\xc3\xbc \xc4\xf9\xbd\xba\xc6\xae \xbc\xf6\xb6\xf4 \xbf\xe4\xc3\xbb" },
	{ "quest_reward", 0x73, "quest_reward [UID(%d)] : \xc4\xf9\xbd\xba\xc6\xae \xba\xb8\xbb\xf3 \xbf\xe4\xc3\xbb" },
	{ "quest_reward_all", 0x74, "quest_reward_all : \xc0\xfc\xc3\xbc \xc4\xf9\xbd\xba\xc6\xae \xba\xb8\xbb\xf3 \xbf\xe4\xc3\xbb" },
	{ "quest_cancel", 0x75, "quest_cancel [UID(%d)] : \xc4\xf9\xbd\xba\xc6\xae \xc3\xeb\xbc\xd2 \xbf\xe4\xc3\xbb" },
	{ "add_exe_dir", 0x76, "add_exe_dir [full dir] : data\xc6\xfa\xb4\xf5\xbf\xcd sibling\xc7\xd1 \xba\xce\xb0\xa1 \xb5\xa5\xc0\xcc\xc5\xcd \xc6\xfa\xb4\xf5\xb8\xa6 \xc3\xdf\xb0\xa1" },
	{ "ab_dev /?", 0x77, "ab_dev /? : \xb0\xb3\xb9\xdf\xc0\xda \xb1\xe2\xb4\xc9 \xb5\xb5\xbf\xf2\xb8\xbb <2012 \xbe\xc6\xb8\xb0 \xbb\xfd\xc0\xcf\xc0\xcc\xba\xa5\xc6\xae>" },
	{ "ab_dev", 0x78, "ab_dev %d : \xb0\xb3\xb9\xdf\xc0\xda \xb1\xe2\xb4\xc9 <2012 \xbe\xc6\xb8\xb0 \xbb\xfd\xc0\xcf\xc0\xcc\xba\xa5\xc6\xae>" },
	{ "ab_open", 0x79, "ab_open : \xc1\xa4\xba\xb8 \xba\xd2\xb7\xaf\xbf\xc0\xb1\xe2 <2012 \xbe\xc6\xb8\xb0 \xbb\xfd\xc0\xcf\xc0\xcc\xba\xa5\xc6\xae>" },
	{ "ab_accept", 0x7a, "ab_accept : \xc4\xf9\xbd\xba\xc6\xae \xb9\xde\xb1\xe2 <2012 \xbe\xc6\xb8\xb0 \xbb\xfd\xc0\xcf\xc0\xcc\xba\xa5\xc6\xae>" },
	{ "ab_reward", 0x7b, "ab_reward [UID(%d)] : \xc4\xf9\xbd\xba\xc6\xae \xba\xb8\xbb\xf3 <2012 \xbe\xc6\xb8\xb0 \xbb\xfd\xc0\xcf\xc0\xcc\xba\xa5\xc6\xae>" },
	{ "ab_cancel", 0x7c, "ab_cancel : \xc4\xf9\xbd\xba\xc6\xae \xc6\xf7\xb1\xe2 <2012 \xbe\xc6\xb8\xb0 \xbb\xfd\xc0\xcf\xc0\xcc\xba\xa5\xc6\xae>" },
	{ "compose_card", 0x7d, "compose_card : \xc4\xab\xb5\xe5\xc7\xd5\xbc\xba" },
	{ "artifact /?", 0x7e, "artifact /? : \xbe\xc6\xc6\xbc\xc6\xd1\xc6\xae \xb5\xb5\xbf\xf2\xb8\xbb" },
	{ "artifact_status", 0x7f, "artifact_status : \xbe\xc6\xc6\xbc\xc6\xd1\xc6\xae \xbb\xf3\xc5\xc2 \xc8\xae\xc0\xce" },
	{ "artifact_change", 0x80, "artifact_change [\xbe\xc6\xc6\xbc\xc6\xd1\xc6\xae(%d)]: \xbe\xc6\xc6\xbc\xc6\xd1\xc6\xae \xba\xaf\xb0\xe6 ( \xb4\xdc, \xb4\xeb\xb1\xe2\xb7\xeb\xbf\xa1\xbc\xad\xb8\xb8 \xbb\xe7\xbf\xeb \xb0\xa1\xb4\xc9 )" },
	{ "achievement_list", 0x83, "achievement_list : \xc7\xf6\xc0\xe7 \xba\xb8\xc0\xaf\xc7\xd1 \xbe\xf7\xc0\xfb \xb8\xae\xbd\xba\xc6\xae" },
	{ "achievement /?", 0x81, "achievement /? : \xbe\xf7\xc0\xfb \xb5\xb5\xbf\xf2\xb8\xbb" },
	{ "achievement", 0x82, "achievement [%d] : \xbe\xf7\xc0\xfb \xb0\xb3\xb9\xdf\xc0\xda \xb1\xe2\xb4\xc9" },
	{ "achievement_info", 0x84, "achievement_info : \xbe\xf7\xc0\xfb \xc1\xa4\xba\xb8 \xbf\xe4\xc3\xbb" },
	{ "auto_cmd", 0x85, "auto_cmd : \xc0\xda\xb5\xbf \xc4\xbf\xb8\xc7\xb5\xe5 \xbe\xc6\xc0\xcc\xc5\xdb \xbb\xe7\xbf\xeb \xbe\xcb\xb6\xf7" },
	{ "workshop_levelup", 0x86, "workshop_levelup [\xbe\xd7\xbc\xc7\xb9\xf8\xc8\xa3(%d):0~5] : \xbd\xbd\xb7\xd4\xb0\xb3\xc1\xb6 < \xc5\xac\xb7\xb4\xbc\xc2\xb0\xf8\xb9\xe6 >" },
	{ "workshop_levelup_apply", 0x87, "workshop_levelup_apply : \xbd\xbd\xb7\xd4\xb0\xb3\xc1\xb6 \xc8\xae\xc0\xce < \xc5\xac\xb7\xb4\xbc\xc2\xb0\xf8\xb9\xe6 >" },
	{ "workshop_levelup_cancel", 0x88, "workshop_levelup_cancel : \xbd\xbd\xb7\xd4\xb0\xb3\xc1\xb6 \xc3\xeb\xbc\xd2 < \xc5\xac\xb7\xb4\xbc\xc2\xb0\xf8\xb9\xe6 >" },
	{ "workshop_rankup", 0x89, "workshop_rankup [\xbe\xd7\xbc\xc7\xb9\xf8\xc8\xa3(%d):1~5] : \xb7\xa9\xc5\xa9\xbe\xf7 < \xc5\xac\xb7\xb4\xbc\xc2\xb0\xf8\xb9\xe6 >" },
	{ "workshop_change_apply", 0x8a, "workshop_change_apply : \xc5\xac\xb7\xb4\xba\xaf\xc8\xad \xbc\xf6\xb6\xf4 < \xc5\xac\xb7\xb4\xbc\xc2\xb0\xf8\xb9\xe6 >" },
	{ "workshop_change_cancel", 0x8b, "workshop_change_cancel : \xc5\xac\xb7\xb4\xba\xaf\xc8\xad \xc3\xeb\xbc\xd2 < \xc5\xac\xb7\xb4\xbc\xc2\xb0\xf8\xb9\xe6 >" },
	{ "workshop_exp_increment", 0x8c, "workshop_exp_increment [\xc8\xa6\xc8\xbd\xbc\xf6(%d):1~18] : \xbc\xf7\xb7\xc3\xb5\xb5 \xc1\xf6\xb1\xde < \xc5\xac\xb7\xb4\xbc\xc2\xb0\xf8\xb9\xe6 >" },
	{ "workshop_cancel_recovery", 0x8d, "workshop_cancel_recovery : \xbd\xbd\xb7\xd4\xb0\xb3\xc1\xb6 \xc3\xeb\xbc\xd2 \xc8\xbd\xbc\xf6 \xc8\xb8\xba\xb9 < \xc5\xac\xb7\xb4\xbc\xc2\xb0\xf8\xb9\xe6 >" },
	{ "workshop_exp_movement", 0x8e, "workshop_exp_movement %d : \xbc\xf7\xb7\xc3\xb5\xb5 \xc0\xcc\xc0\xfc < \xc5\xac\xb7\xb4\xbc\xc2\xb0\xf8\xb9\xe6 >" },
	{ "workshop_init", 0x8f, "workshop_init [\xbe\xd7\xbc\xc7\xb9\xf8\xc8\xa3(%d):1~2]: \xc3\xca\xb1\xe2\xc8\xad < \xc5\xac\xb7\xb4\xbc\xc2\xb0\xf8\xb9\xe6 >" },
	{ "istodaylogin", 0x90, "istodaylogin : \xb4\xe7\xc0\xcf \xb7\xce\xb1\xd7\xc0\xce \xc8\xae\xc0\xce" },
	{ "nextdaylogin", 0x91, "nextdaylogin : \xb4\xd9\xc0\xbd \xb3\xaf\xb7\xce \xc0\xcc\xb5\xbf" },
	{ "workshop_event_hole_count_info", 0x92, "workshop_event_hole_count_info : \xb0\xb3\xc0\xce, \xc3\xd1 \xc8\xa6 \xc4\xab\xbf\xee\xc6\xae \xc8\xae\xc0\xce <8\xbf\xf9 \xb0\xf8\xb9\xe6 \xc0\xcc\xba\xa5\xc6\xae>" },
	{ "workshop_event_randombox", 0x93, "workshop_event_randombox : \xb7\xa3\xb4\xfd\xb9\xda\xbd\xba \xc1\xf6\xb1\xde \xbf\xe4\xc3\xbb <8\xbf\xf9 \xb0\xf8\xb9\xe6 \xc0\xcc\xba\xa5\xc6\xae>" },
	{ "workshop_event_exp_event_on", 0x94, "workshop_event_exp_event_on : \xbc\xf7\xb7\xc3\xb5\xb5 2\xb9\xe8 \xc8\xbf\xb0\xfa \xc8\xb0\xbc\xba\xc8\xad <8\xbf\xf9 \xb0\xf8\xb9\xe6 \xc0\xcc\xba\xa5\xc6\xae>" },
	{ "workshop_event_play_hole_count", 0x95, "workshop_event_play_hole_count : \xc8\xc7 \xc4\xab\xbf\xee\xc6\xae \xc1\xf5\xb0\xa1 <8\xbf\xf9 \xb0\xf8\xb9\xe6 \xc0\xcc\xba\xa5\xc6\xae>" },
	{ "playerlevel", 0x97, "playerlevel : \xc4\xb3\xb8\xaf\xc5\xcd \xb4\xc9\xb7\xc2\xc4\xa1 \xc8\xae\xc0\xce" },
	{ "gp_enter_lobby", 0x98, "gp_enter_lobby : \xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae \xb7\xce\xba\xf1 \xc0\xd4\xc0\xe5 <\xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae>" },
	{ "gp_leave_lobby", 0x99, "gp_leave_lobby : \xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae \xb7\xce\xba\xf1 \xc5\xf0\xc0\xe5 <\xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae>" },
	{ "gp_move_lobby", 0x9a, "gp_move_lobby [??] : \xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae \xb7\xce\xba\xf1 \xbc\xad\xb9\xf6\xc0\xcc\xb5\xbf <\xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae>" },
	{ "gp_enter_room", 0x9b, "gp_enter_room [\xb1\xd7\xb8\xb5\xc7\xc1\xb8\xae\xb9\xf8\xc8\xa3(%d)] : \xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae \xb7\xeb \xc0\xd4\xc0\xe5 <\xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae>" },
	{ "gp_leave_room", 0x9c, "gp_leave_room [\xb1\xd7\xb8\xb5\xc7\xc1\xb8\xae\xb9\xf8\xc8\xa3(%d)] : \xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae \xb7\xeb \xc5\xf0\xc0\xe5 <\xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae>" },
	{ "lobbyavatar", 0x9d, "lobbyavatar [option] - \xb7\xce\xba\xf1\xbe\xc6\xb9\xd9\xc5\xb8 \xb0\xa8\xc3\xdf\xb1\xe2" },
	{ "gpaward", 0x9e, "gpaward" },
	{ "gpawardreset", 0x9f, "gpawardreset" },
	{ "gpreloadui", 0xa0, "gpreloadui [%s] : \xc6\xc4\xc0\xcf\xc0\xcc\xb8\xa7" },
	{ "gploadcrowd", 0xa1, "gploadcrowd" },
	{ "layout", 0xa2, "layout [%s] : \xb7\xb9\xc0\xcc\xbe\xc6\xbf\xf4 \xc0\xcc\xb8\xa7" },
	{ "gpreserve", 0xa3, "gpreserve [turnidx]" },
	{ "gp_find_ai_info", 0xa4, "gp_find_ai_info [AI\xb9\xf8\xc8\xa3 1 ~ AI\xc7\xc3\xb7\xb9\xc0\xcc\xbe\xee\xbc\xf6 (%d)] : AI\xc1\xa4\xba\xb8 <\xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae>" },
	{ "loadstring", 0x96, "DB\xbf\xa1 \xc0\xd6\xb4\xc2 GameString \xba\xd2\xb7\xaf\xbf\xc0\xb1\xe2" },
	{ "playerlevel", 0x97, "playerlevel : \xc4\xb3\xb8\xaf\xc5\xcd \xb4\xc9\xb7\xc2\xc4\xa1 \xc8\xae\xc0\xce" },
	{ "get_play_assist", 0xa6, "get_play_assist : \xc7\xc3\xb7\xb9\xc0\xcc \xbe\xee\xbd\xc3\xbd\xba\xc6\xae \xbb\xf3\xc5\xc2" },
	{ "play_assist", 0xa7, "play_assist : \xc7\xc3\xb7\xb9\xc0\xcc \xbe\xee\xbd\xc3\xbd\xba\xc6\xae ON/OFF" },
	{ "assist_update_boundary", 0xae, "assist_update_boundary [\xb1\xb8\xb0\xa3 \xbc\xf6]: (\xc3\xd6\xbc\xd2 1 - \xc3\xd6\xb4\xeb 30)\xbd\xba\xc7\xc9, \xc4\xbf\xba\xea \xba\xaf\xb0\xe6\xbd\xc3 \xb4\xd9\xbd\xc3 \xb0\xe8\xbb\xea\xc7\xcf\xb4\xc2 \xb1\xb8\xb0\xa3 \xbc\xb3\xc1\xa4" },
	{ "assist_trajectory_fake_factor_rate", 0xa8, "assist_trajectory_fake_factor_rate [\xbf\xc0\xc2\xf7\xc0\xfb\xbf\xeb \xba\xf1\xc0\xb2] : (1000 = 100%)\xb9\xcc\xb8\xae\xba\xb8\xb1\xe2 \xb1\xcb\xc0\xfb \xb0\xe8\xbb\xea\xbd\xc3 \xb0\xa1\xbb\xea\xb5\xc7\xb4\xc2 \xbf\xc0\xc2\xf7\xb0\xaa\xc0\xc7 \xc0\xfb\xbf\xeb \xba\xf1\xc0\xb2 \xbc\xb3\xc1\xa4" },
	{ "assist_trajectory_fading", 0xa9, "assist_trajectory_fading [\xbd\xc3\xc0\xdb\xc1\xf6\xc1\xa1] [\xb3\xa1\xc1\xf6\xc1\xa1]: \xc7\xc3\xb7\xb9\xc0\xcc \xbe\xee\xbd\xc3\xbd\xba\xc6\xae \xb1\xcb\xc0\xfb \xc6\xe4\xc0\xcc\xb5\xe5\xbe\xc6\xbf\xf4 \xbc\xf6\xc4\xa1 \xc1\xb6\xc1\xa4" },
	{ "assist_trajectory_repetition", 0xaa, "assist_trajectory_repetition [\xc5\xd8\xbd\xba\xc3\xc4 \xb9\xdd\xba\xb9 \xc8\xbd\xbc\xf6]: \xb1\xcb\xc0\xfb\xbb\xf3 \xc5\xd8\xbd\xba\xc3\xc4\xb0\xa1 \xb8\xee\xb9\xf8 \xb9\xdd\xba\xb9\xb5\xc7\xbe\xee \xb3\xaa\xc5\xb8\xb3\xaf\xb0\xcd\xc0\xce\xc1\xf6 \xc1\xb6\xc1\xa4" },
	{ "assist_trajectory_width", 0xab, "assist_trajectory_width [\xb3\xca\xba\xf1\xb0\xaa]: \xb1\xcb\xc0\xfb\xc0\xc7 \xb3\xca\xba\xf1 \xc1\xb6\xc1\xa4  (\xbe\xc6\xc1\xee\xc5\xd8 \xc1\xf6\xb8\xa7 \xb1\xe2\xc1\xd8. \xb3\xca\xba\xf1\xb0\xaa 10 = 1\xb9\xe8)" },
	{ "assist_trajectory_speed", 0xac, "assist_trajectory_speed [\xbc\xd3\xb5\xb5\xb0\xaa]: \xb1\xcb\xc0\xfb \xc5\xd8\xbd\xba\xc3\xc4 \xbe\xd6\xb4\xcf\xb8\xde\xc0\xcc\xbc\xc7 \xbc\xd3\xb5\xb5 \xc1\xb6\xc1\xa4" },
	{ "assist_bounding_spot_range", 0xad, "assist_bounding_spot_range [\xc3\xd6\xbc\xd2] [\xc3\xd6\xb4\xeb] : (\xb9\xdd\xc1\xf6\xb8\xa7\xb0\xaa, 10 = 1\xbe\xdf\xb5\xe5) \xb9\xd9\xbf\xee\xb5\xf9 \xbd\xba\xc6\xcc \xc7\xa5\xbd\xc3\xb9\xfc\xc0\xa7 \xc1\xb6\xc1\xa4" },
	{ "errorcode", 0xaf, "errorcode %d : \xbf\xa1\xb7\xaf \xc4\xda\xb5\xe5 \xc3\xe2\xb7\xc2" },
	{ "get_identity", 0xb0, "get_identity : IDENTITY" },
	{ "is_premium", 0xb1, "is_premium : \xc7\xc1\xb8\xae\xb9\xcc\xbe\xf6 \xbf\xa9\xba\xce" },
	{ "mastery_point_up", 0xb2, "mastery_point_up : \xc4\xb3\xb8\xaf\xc5\xcd \xb8\xb6\xbd\xba\xc5\xcd\xb8\xae \xc6\xf7\xc0\xce\xc6\xae \xbb\xf3\xc7\xe2" },
	{ "mastery_status_up", 0xb3, "mastery_status_up [\xc6\xc4\xbf\xf6(0), \xc4\xc1\xc6\xae\xb7\xd1(1), \xc1\xa4\xc8\xae\xb5\xb5(2), \xbd\xba\xc7\xc9(3), \xc4\xbf\xba\xea(4)] : \xc4\xb3\xb8\xaf\xc5\xcd STATUS \xbb\xf3\xc7\xe2" },
	{ "mastery_status_down", 0xb4, "mastery_status_down [\xc6\xc4\xbf\xf6(0), \xc4\xc1\xc6\xae\xb7\xd1(1), \xc1\xa4\xc8\xae\xb5\xb5(2), \xbd\xba\xc7\xc9(3), \xc4\xbf\xba\xea(4)] : \xc4\xb3\xb8\xaf\xc5\xcd STATUS \xc7\xcf\xc7\xe2" },
	{ "mastery_attach_card", 0xb5, "mastery_attach_card : \xc4\xb3\xb8\xaf\xc5\xcd\xbf\xa1 \xc4\xab\xb5\xe5 \xc0\xe5\xc2\xf8" },
	{ "mastery_attach_bonus_card", 0xb6, "mastery_attach_bonus_card : \xc4\xb3\xb8\xaf\xc5\xcd\xbf\xa1 \xba\xb8\xb3\xca\xbd\xba \xc4\xab\xb5\xe5 \xc0\xe5\xc2\xf8" },
	{ "mastery_detach_card", 0xb7, "mastery_detach_card : \xc4\xb3\xb8\xaf\xc5\xcd\xbf\xa1 \xc4\xab\xb5\xe5 \xc7\xd8\xc1\xa6" },
	{ "mastery_open_dlg", 0xb8, "mastery_open_dlg : \xb8\xb6\xbd\xba\xc5\xcd\xb8\xae\xc3\xa2 \xbf\xc0\xc7\xc2" },
	{ "mp_break", 0xb9, "mp_break : MP\xba\xd0\xc7\xd8 < \xbf\xac\xb1\xdd >" },
	{ "crash", 0xba, "crash [no thread(0), thread(1)]: \xc5\xa9\xb7\xa1\xbd\xc3" },
	{ "arin_dev /?", 0xbb, "arin_dev /? : \xb0\xb3\xb9\xdf\xc0\xda \xb1\xe2\xb4\xc9 \xb5\xb5\xbf\xf2\xb8\xbb <2012 \xbe\xc6\xb8\xb0 \xbb\xfd\xc0\xcf\xc0\xcc\xba\xa5\xc6\xae>" },
	{ "arin_dev", 0xbc, "arin_dev %d : \xb0\xb3\xb9\xdf\xc0\xda \xb1\xe2\xb4\xc9 <2014 \xbe\xc6\xb8\xb0 \xbb\xfd\xc0\xcf\xc0\xcc\xba\xa5\xc6\xae>" },
	{ "arin_reward_1a", 0xbd, "arin_reward_1a : \xbd\xba\xc5\xd7\xc0\xcc\xc1\xf6" "1A \xba\xb8\xbb\xf3 <2014 \xbe\xc6\xb8\xb0 \xbb\xfd\xc0\xcf\xc0\xcc\xba\xa5\xc6\xae>" },
	{ "arin_reward_1b", 0xbe, "arin_reward_1b : \xbd\xba\xc5\xd7\xc0\xcc\xc1\xf6" "1B \xba\xb8\xbb\xf3 <2014 \xbe\xc6\xb8\xb0 \xbb\xfd\xc0\xcf\xc0\xcc\xba\xa5\xc6\xae>" },
	{ "arin_info", 0xbf, "arin_info : \xbd\xba\xc5\xd7\xc0\xcc\xc1\xf6" "2 \xb9\xcc\xbc\xc7 <2014 \xbe\xc6\xb8\xb0 \xbb\xfd\xc0\xcf\xc0\xcc\xba\xa5\xc6\xae>" },
	{ "arin_reward_mission", 0xc0, "arin_reward_mission : \xbd\xba\xc5\xd7\xc0\xcc\xc1\xf6" "2 \xb9\xcc\xbc\xc7 \xba\xb8\xbb\xf3 <2014 \xbe\xc6\xb8\xb0 \xbb\xfd\xc0\xcf\xc0\xcc\xba\xa5\xc6\xae>" },
	{ "arin_reward_2", 0xc1, "arin_reward_2 : \xbd\xba\xc5\xd7\xc0\xcc\xc1\xf6" "2 \xba\xb8\xbb\xf3 <2014 \xbe\xc6\xb8\xb0 \xbb\xfd\xc0\xcf\xc0\xcc\xba\xa5\xc6\xae>" },
	{ "arin_open", 0xc2, "arin_dlg_open" },
	{ "scratch_sync", 0xc3, "scratch_sync" },
	{ "runspeed", 0xc4, "runspeed : chatroom movespeed up ex.)runspeed 1.5" },
	{ "god2", 0xc5, "god2 value - 0:OFF 1:Absolute HoleIn" },
};

int __fastcall _POSTMSG(const void* sender, const char* target, int message,
	int param1, int param2, int param3, unsigned long time)
{
	return AfxPostMsg(NULL, target, message, param1, param2, param3, time);
}

CConsole::CConsole()
	: m_pMatch(0), m_cmdList(0x20), m_textList(0x20)
{
	m_fSpeed = 5.0f;
}

CConsole::~CConsole()
{
	ShutDown();
}

void CConsole::Init()
{
	m_bsStack[0] = 0;
	m_bBrowsing = false;
	m_pMatch = new char[COMMAND_COUNT * 2];
	for (unsigned int i = 0; i < COMMAND_COUNT * 2; i += 2)
		m_pMatch[i + 1] = 0;
	m_fAlpha = 0;
	m_input[0] = 0;
	m_bComposing = false;
	m_scroll = 0;
	m_state = 1;
	m_pFont = CChatMsg::Instance()->GetMaskedFont();
}

void CConsole::ShutDown()
{
	if (m_pMatch)
	{
		delete[] m_pMatch;
		m_pMatch = 0;
	}
	for (char* p = m_cmdList.Start(); p; p = m_cmdList.Next())
	{
		if (p)
			delete[] p;
	}
	m_cmdList.Reset();
	for (char* p = m_textList.Start(); p; p = m_textList.Next())
	{
		if (p)
			delete[] p;
	}
	m_textList.Reset();
}

void CConsole::Process(float dt)
{
	if (0) // emitted after InputProcess in the original
		InputProcess(dt);
}

void CConsole::InputProcess(float dt)
{
	int key = 0;
	char buf[4];
	buf[0] = 0;
	const char* pch = (const char*)g_ime->GetState(3, 0);
	if (pch)
	{
		int ch = *pch;
	if (ch)
	{
		if (ch == 9)
		{
			key = ch;
		}
		else if (ch == 8 || ch == 13 || ch == 6 || ch == 7 || ch == 2 ||
			ch == 3 || ch == 4 || ch == 5)
		{
			key = ch;
			m_bBrowsing = false;
		}
		else
		{
			if (Custom_IsLeadByte((unsigned char)ch))
			{
				buf[0] = (char)ch;
				buf[1] = (char)(ch >> 8);
				buf[2] = 0;
				PopFromBsStack();
				PopFromBsStack();
				m_bBrowsing = false;
			}
			else
			{
				if (ch < 0x20 || ch > 0x7a)
					goto done;
				buf[0] = (char)ch;
				buf[1] = 0;
				if (ch == ' ')
				{
					ClearBsStack();
					m_bBrowsing = false;
				}
				else
				{
					PopFromBsStack();
					m_bBrowsing = false;
				}
			}
			if ((char)ch)
			{
				if (m_bComposing)
					m_input[strlen(m_input) - 2] = 0;
				strcat(m_input, buf);
				m_input[sizeof(m_input) - 1] = 0;
			}
		}
done:
		m_bComposing = false;
	}
	}
	static int s_prevComp;
	int comp = g_ime->GetState(3, -2);
	if (comp == 0)
	{
		if (m_bComposing)
		{
			m_input[strlen(m_input) - 2] = 0;
			m_bComposing = false;
		}
	}
	else if (comp != s_prevComp)
	{
		s_prevComp = comp;
		buf[0] = (char)comp;
		buf[1] = (char)(comp >> 8);
		buf[2] = 0;
		if (m_bComposing)
			m_input[strlen(m_input) - 2] = 0;
		strcat(m_input, buf);
		m_input[sizeof(m_input) - 1] = 0;
		m_bComposing = true;
	}
	switch (key)
	{
	case 2:
		if (m_textList.GetCount() < 20)
			return;
		m_scroll = m_scroll - 1 > 0 ? m_scroll - 1 : 0;
		break;
	case 3:
		if (m_textList.GetCount() < 20)
			return;
		m_scroll = m_scroll + 1 <= m_textList.GetCount() - 19
			? m_scroll + 1
			: m_textList.GetCount() - 19;
		break;
	case 6:
		if (m_histDir >= 0 && m_cmdList.GetCount() > 0)
		{
			m_histDir = 0;
			if (!m_histPos)
			{
				strcpy(m_input, m_cmdList.End(m_histPos));
				return;
			}
			char* text = m_cmdList.Prev(m_histPos);
			if (m_histPos)
			{
				strcpy(m_input, text);
				return;
			}
			m_histDir = -1;
			m_cmdList.Start(m_histPos);
			return;
		}
		break;
	case 7:
		if (m_histDir <= 0 && m_cmdList.GetCount() > 0)
		{
			m_histDir = 0;
			if (m_histPos)
			{
				char* text = m_cmdList.Next(m_histPos);
				if (m_histPos)
				{
					strcpy(m_input, text);
					return;
				}
				m_histDir = 1;
				m_input[0] = 0;
				return;
			}
		}
		break;
	case 8:
	{
		int len = strlen(m_input);
		if (len > 0 && !m_bComposing)
		{
			if (len > 1 && m_input[len - 1] < 0)
			{
				PushIntoBsStack(m_input[len - 1]);
				PushIntoBsStack(m_input[len - 2]);
				m_input[len - 2] = 0;
				return;
			}
			if (m_input[len - 1] == ' ')
			{
				ClearBsStack();
				m_input[len - 1] = 0;
				return;
			}
			PushIntoBsStack(m_input[len - 1]);
			m_input[len - 1] = 0;
			return;
		}
		break;
	}
	case 9:
	{
		char* word = m_input;
		if (!m_bBrowsing)
		{
			strcpy(m_browse, m_input);
			m_bBrowsing = true;
			for (char* space = strstr(m_input, " "); space;
				space = strstr(word, " "))
			{
				word = space + 1;
				strcpy(m_browse, word);
			}
			for (unsigned int i = 0; i < COMMAND_COUNT * 2; i += 2)
				m_pMatch[i + 1] = 0;
		}
		if (word != m_input)
		{
			char* end = strchr(word, 0);
			if (!isalnum(end[-1]))
			{
				char c;
				do
				{
					c = PopFromBsStack();
					if (!c)
						goto restored;
				} while (!isalnum(c));
				*end = c;
				do
				{
					*++end = c = PopFromBsStack();
				} while (c);
			}
			else
			{
				while ((*end = PopFromBsStack()) != 0)
					end++;
			}
restored:
			m_bBrowsing = false;
			return;
		}
		char name[128];
		if (BrowseCommand(m_browse, name) >= 0)
		{
			strcpy(word, name);
			return;
		}
		break;
	}
	case 13:
	{
		m_bsStack[0] = 0;
		if (strlen(m_input) == 0)
			return;
		char* cmd = new char[strlen(m_input) + 1];
		strcpy(cmd, m_input);
		m_cmdList += cmd;
		AddText(MakeStr(">%s", cmd));
		if (!TweakerCommandProc(m_input))
			Command(_strlwr(m_input));
		m_input[0] = 0;
		int lines = m_textList.GetCount() - 19;
		m_histPos = 0;
		m_histDir = 1;
		m_scroll = lines > 0 ? lines : 0;
		break;
	}
	}
}

void CConsole::Display()
{
	if (!(Wabs(m_fAlpha) < g_EPSILON))
	{
		WOverlay::DrawBox(g_view,
			WRect(0, 0, g_view->GetWidth(), CONSOLE_HEIGHT), 0,
			(int)(m_fAlpha * 128.0f) << 24, 0.001f);
		int lines = 19 - m_textList.GetCount();
		float y = (float)(lines > 0 ? lines : 0) * 15.0f + 5.0f;
		int alpha = (int)(m_fAlpha * 255.0f);
		int i = 0;
		for (char* text = m_textList.Start(); text && i != m_scroll + 19;
			text = m_textList.Next(), i++)
		{
			if (i >= m_scroll)
			{
				m_pFont->Print(g_view, 6.0f, y + 1.0f, text, 0, alpha << 24,
					NULL);
				m_pFont->Print(g_view, 5.0f, y, text, 0,
					(alpha << 24) | 0xffffff, NULL);
				y += 15.0f;
			}
		}
		float x = m_pFont->Print(g_view, 5.0f, y, MakeStr(">%s", m_input), 0,
			(alpha << 24) | 0xffffff, NULL);
		m_pFont->Flush(g_view);
		WOverlay::DrawBox(g_view, WRect(x + 5.0f, y + 10.0f, 7.0f, 2.0f), 0,
			(alpha << 24) | 0xffffff, 0.001f);
		WOverlay::DrawBox(g_view,
			WRect(g_view->GetWidth() - 12.0f, 5.0f, 10.0f, 292.0f), 0,
			(alpha << 24) | 0x404040, 0.001f);
		int count = m_textList.GetCount();
		if (count > 0 && count > 19)
		{
			WOverlay::DrawBox(g_view,
				WRect(g_view->GetWidth() - 10.0f,
					(float)m_scroll * 292.0f / (float)count + 5.0f, 6.0f,
					5548.0f / (float)count),
				0, (alpha << 24) | 0x707070, 0.001f);
		}
	}
}

bool CConsole::PushIntoBsStack(char ch)
{
	int len = strlen(m_bsStack);
	if (len < 0x7f)
	{
		m_bsStack[len] = ch;
		m_bsStack[len + 1] = 0;
		return true;
	}
	return false;
}

char CConsole::PopFromBsStack()
{
	int len = strlen(m_bsStack);
	if (len == 0)
		return 0;
	char ch = m_bsStack[len - 1];
	m_bsStack[len - 1] = 0;
	return ch;
}

void CConsole::ClearBsStack()
{
	m_bsStack[0] = 0;
}

int CConsole::BrowseCommand(const char* input, char* out)
{
	unsigned int i;
	for (i = 0; i < COMMAND_COUNT * 2; i += 2)
		m_pMatch[i] = 1;
	int len = strlen(input);
	int pos = 0;
	int idx = 0;
	if (0 < len)
	{
		bool bNoMatch;
		do
		{
			bNoMatch = true;
			idx = 0;
			for (i = 0; i < COMMAND_COUNT * sizeof(command_table[0]);
				i += sizeof(command_table[0]), idx += 2)
			{
				if (m_pMatch[idx])
				{
					const char* name = *(char**)((char*)command_table + i);
					if ((int)strlen(name) > pos && input[pos] == name[pos])
						bNoMatch = false;
					else
						m_pMatch[idx] = 0;
				}
			}
		} while (!bNoMatch && (++pos, pos < len));
		idx = pos;
	}
	char* flag = m_pMatch;
	for (i = 0; i < COMMAND_COUNT; i++, flag += 2)
	{
		if (*flag && !flag[1])
			goto found;
	}
	for (i = 0; i < COMMAND_COUNT * 2; i += 2)
		m_pMatch[i + 1] = 0;
found:
	int best = -1;
	int n = 0;
	flag = m_pMatch;
	for (i = 0; i < COMMAND_COUNT; i++)
	{
		if (flag[0] && !flag[1])
		{
			const char* name = command_table[i].name;
			if ((int)strlen(name) <= idx)
			{
				if (out)
					strcpy(out, command_table[n].name);
				return n;
			}
			if (best == -1 || name[idx] < command_table[best].name[idx])
			{
				best = n;
			}
		}
		flag += 2;
		n++;
	}
	if (best != -1)
	{
		m_pMatch[best * 2 + 1] = 1;
		if (out)
			strcpy(out, command_table[best].name);
		return command_table[best].id;
	}
	*out = 0;
	return -1;
}

int CConsole::TweakerCommandProc(const char* cmd)
{
	static tweaker* s_pUsingTweaker;
	if (!stricmp(cmd, "tw cmd all"))
	{
		std::vector<std::string> list;
		tweaker_database::instance()->get_tweaker_command_all(list);
		if (list.empty())
		{
			AddText(
				"\xbb\xe7\xbf\xeb \xb0\xa1\xb4\xc9\xc7\xd1 \xc6\xae\xc0\xa7\xc4\xbf \xb8\xed\xb7\xc9\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			return 1;
		}
		std::vector<std::string>::iterator it;
		for (it = list.begin(); it != list.end(); ++it)
			AddText((*it).c_str());
		return 1;
	}
	if (!stricmp(cmd, "tw list"))
	{
		std::vector<std::string> list;
		tweaker_database::instance()->get_tweaker_names(list);
		if (list.empty())
		{
			AddText(
				"\xbb\xe7\xbf\xeb \xb0\xa1\xb4\xc9\xc7\xd1 \xc6\xae\xc0\xa7\xc4\xbf\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			return 1;
		}
		std::vector<std::string>::iterator it;
		for (it = list.begin(); it != list.end(); ++it)
			AddText((*it).c_str());
		return 1;
	}
	if (!strncmp(cmd, "tw using", 8))
	{
		char className[128] = "";
		char instName[128] = "";
		_snscanf(cmd + 9, 127, "%s %s", className, instName);
		std::string name(className);
		name += " ";
		name += instName;
		s_pUsingTweaker = tweaker_database::instance()->get_tweaker(name);
		if (s_pUsingTweaker)
		{
			AddText(
				"\xc0\xd4\xb7\xc2\xb5\xc8 \xc6\xae\xc0\xa7\xc4\xbf\xb8\xa6 \xbd\xc3\xc0\xdb\xc7\xd5\xb4\xcf\xb4\xd9. \xb4\xd9\xc0\xbd\xc0\xba "
				"\xbf\xb9\xbe\xe0\xb5\xc8 \xb8\xed\xb7\xc9\xbe\xee\xb5\xe9 \xc0\xd4\xb4\xcf\xb4\xd9.");
			std::vector<std::string> list;
			s_pUsingTweaker->get_command_list(list, false);
			std::vector<std::string>::iterator it;
			for (it = list.begin(); it != list.end(); ++it)
				AddText((*it).c_str());
		}
		else
		{
			AddText(
				"\xc6\xae\xc0\xa7\xc4\xbf\xb0\xa1 \xc1\xbe\xb7\xe1\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.(tw list\xb8\xa6 \xc5\xeb\xc7\xd8 "
				"\xc8\xae\xc0\xce\xc7\xcf\xbd\xc5 \xc8\xc4 \xc0\xd4\xb7\xc2\xc7\xcf\xbc\xbc\xbf\xe4.)");
		}
		return 1;
	}
	if (s_pUsingTweaker)
	{
		if (tweaker_database::instance()->is_exist_tweaker(s_pUsingTweaker))
		{
			bool exist = s_pUsingTweaker->is_exist_command(cmd);
			if (exist)
			{
				std::string result = s_pUsingTweaker->run_command(cmd);
				AddText(result.c_str());
				return 1;
			}
		}
		else
		{
			AddText(
				"\xc0\xcc\xc0\xfc \xbb\xe7\xbf\xeb\xc0\xbb \xbc\xb1\xbe\xf0\xc7\xd1 \xc6\xae\xc0\xa7\xc4\xbf\xb0\xa1 \xb4\xf5\xc0\xcc\xbb\xf3 "
				"\xc0\xaf\xc8\xbf\xc7\xcf\xc1\xf6 \xbe\xca\xbe\xc6\xbc\xad \xc0\xda\xb5\xbf \xc1\xbe\xb7\xe1\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.");
			s_pUsingTweaker = NULL;
		}
	}
	else if (tweaker_database::instance()->is_exist_command(cmd))
	{
		std::string result = tweaker_database::instance()->send_command(cmd);
		AddText(result.c_str());
		return 1;
	}
	return 0;
}

// 983: sorteio por peso dos comandos de GM (só no console.cpp; tabela {soma, mapa soma-1 -> valor}).
// Add = 0071f8f0/0071f930/0071f980/0071f9d0, Pick = 0071e720/0071e750/0071e790/0071e7d0 (rand() % soma, lower_bound),
// vazio = 0071b810/0071b820/0071b830. Pick sem itens divide por zero no 983 (compose_card não confere): igual aqui.
template <class T>
struct CConsoleRandom983
{
	unsigned int total;
	std::map<unsigned int, T> table;

	CConsoleRandom983() : total(0) {}
	void Add(const T& value, unsigned int weight)
	{
		if (weight == 0)
			return;
		total += weight;
		table.insert(std::make_pair(total - 1, value));
	}
	bool Empty() const { return table.empty(); }
	T Pick() const { return table.lower_bound(rand() % total)->second; }
};

struct sConsoleCard983 // {typeid, uid} da carta/item sorteado
{
	unsigned long tid;
	unsigned long uid;
};

struct sConsoleMPBreak983 // 00718290: {typeid, uid, quantidade}
{
	unsigned long tid;
	unsigned long uid;
	int count;
};

// 00a19530: membro do CItemManager sem nome no 645 (true = o item pode virar MP); mesma visão da B (alchemympbreakdlg.cpp, jp.tsv)
class CItemManagerJp983 : public CItemManager
{
public:
	bool IsMPBreakItem983(unsigned long typeId);
};

// 007181e0: raridade de carta que entra na alquimia (0..2)
static bool IsComposeCardRare983(int rare)
{
	return rare >= 0 && rare <= 2;
}

// 0071b7b0: custo em pang de uma carta na alquimia (pela raridade)
static __int64 GetComposeCardPang983(unsigned long tid)
{
	IFF_STRUCT::sCard* pCard = CSharedDoc::Instance()->m_itemManager.FindCard(tid);
	if (pCard == NULL || !IsComposeCardRare983(pCard->RareType))
		return 0;
	switch (pCard->RareType)
	{
	case 0:
		return 1000;
	case 1:
		return 2000;
	case 2:
		return 5000;
	}
	return 0;
}

// 00720430: cartas do jogador que entram na alquimia, peso 10 cada
static CConsoleRandom983<unsigned long> MakeComposeCardTable983()
{
	CConsoleRandom983<unsigned long> table;
	std::list<sCards>& cards = CSharedDoc::Instance()->m_cardList;
	for (std::list<sCards>::iterator it = cards.begin(); it != cards.end(); ++it)
	{
		IFF_STRUCT::sCard* pCard = CSharedDoc::Instance()->m_itemManager.FindCard(it->tid);
		if (pCard && IsComposeCardRare983(pCard->RareType))
			table.Add(it->tid, 10);
	}
	return table;
}

// 00718210: carta de personagem (tipo 0, 1 ou 5 nos bits 22..25 do typeid)
static bool IsMasteryCard983(unsigned long tid)
{
	unsigned long type = (tid >> 22) & 0xf;
	return type <= 1 || type == 5;
}

// 00718250: carta que vai no slot bônus (tipo 0 ou 1)
static bool IsMasteryBonusCard983(unsigned long tid)
{
	return ((tid >> 22) & 0xf) <= 1;
}

// 007198d0: item que tira carta do personagem (tabela 00d1f380 com um typeid)
static const unsigned long s_cardRemover983[] = { 0x1a0000c2 };
static bool IsCardRemoverItem983(unsigned long tid)
{
	if ((tid & 0xfc000000) != 0x18000000)
		return false;
	for (int i = 0; i < 1; i++)
	{
		if (tid == s_cardRemover983[i])
			return true;
	}
	return false;
}

// 00719880: missão (grupo 0x1e do typeid) de subtipo 0 (bits 22..25)
static bool IsQuestSubtype0_983(unsigned long tid)
{
	if ((tid & 0xfc000000) != 0x78000000)
		return false;
	return (tid & 0x3c00000) == 0;
}

// 007198a0: missão (grupo 0x1e do typeid) de subtipo 1 ou 2
static bool IsQuestSubtype12_983(unsigned long tid)
{
	if ((tid & 0xfc000000) != 0x78000000)
		return false;
	return ((tid >> 22) & 0xf) - 1 <= 1;
}

// QuestItem.iff +0xc8 (IFF983::sQuestItem ainda sem campos): 1 nas missões "ab" (2012 Arin)
static bool IsAbQuestItem983(unsigned long tid)
{
	IFF983::sQuestItem* pItem = CSharedDoc::Instance()->m_itemManager.FindQuestItem983(tid);
	return pItem && *(unsigned long*)(pItem->raw + 0xc8 - 8) == 1;
}

// 00720530: cartas do jogador para pôr no personagem, peso 10 cada
static CConsoleRandom983<sConsoleCard983> MakeMasteryCardTable983()
{
	CConsoleRandom983<sConsoleCard983> table;
	std::list<sCards>& cards = CSharedDoc::Instance()->m_cardList;
	for (std::list<sCards>::iterator it = cards.begin(); it != cards.end(); ++it)
	{
		if (IsMasteryCard983(it->tid))
		{
			sConsoleCard983 card = { it->tid, it->uid };
			table.Add(card, 10);
		}
	}
	return table;
}

// 00720610: cartas do jogador para o slot bônus, peso 10 cada
static CConsoleRandom983<sConsoleCard983> MakeMasteryBonusCardTable983()
{
	CConsoleRandom983<sConsoleCard983> table;
	std::list<sCards>& cards = CSharedDoc::Instance()->m_cardList;
	for (std::list<sCards>::iterator it = cards.begin(); it != cards.end(); ++it)
	{
		if (IsMasteryBonusCard983(it->tid))
		{
			sConsoleCard983 card = { it->tid, it->uid };
			table.Add(card, 10);
		}
	}
	return table;
}

// 007206f0: itens que tiram carta, peso 10 cada
static CConsoleRandom983<sConsoleCard983> MakeCardRemoverTable983()
{
	CConsoleRandom983<sConsoleCard983> table;
	std::list<sItemInfo>& items = CSharedDoc::Instance()->m_myItemList;
	for (std::list<sItemInfo>::iterator it = items.begin(); it != items.end(); ++it)
	{
		if (IsCardRemoverItem983(it->tid))
		{
			sConsoleCard983 card = { it->tid, it->guid };
			table.Add(card, 10);
		}
	}
	return table;
}

// slots de carta do personagem por tipo (00d1f3d8..00d1f3fc); slot n = sCharacterInfo::card plano [n - 1]
static const unsigned long s_cardSlotBonus0[] = { 4 };
static const unsigned long s_cardSlotBonus1[] = { 8 };
static const unsigned long s_cardSlot0[] = { 1, 2, 3 };
static const unsigned long s_cardSlot1[] = { 5, 6 };
static const unsigned long s_cardSlot5[] = { 9, 10 };
static unsigned long FindEmptyCardSlot983(sCharacterInfo* ch, const unsigned long* slots, int count)
{
	for (int i = 0; i < count; i++)
	{
		if (ch->card[0][slots[i] - 1] == 0)
			return slots[i];
	}
	return 0;
}

// 0071fce0 (0071f500 + 0071ec80): itens do jogador que viram MP, embaralhados
static std::deque<sConsoleMPBreak983> MakeMPBreakList983()
{
	std::deque<sConsoleMPBreak983> list;
	std::list<sItemInfo>& items = CSharedDoc::Instance()->m_myItemList;
	for (std::list<sItemInfo>::iterator it = items.begin(); it != items.end(); ++it)
	{
		if (((CItemManagerJp983*)&CSharedDoc::Instance()->m_itemManager)->IsMPBreakItem983(it->tid))
		{
			sConsoleMPBreak983 item = { it->tid, it->guid, it->Common[0] };
			list.push_back(item);
		}
	}
	std::random_shuffle(list.begin(), list.end());
	return list;
}

// 00497f60 (outra unidade, sem nome): grau do taco pela soma dos slots (30..59 -> 0..5; fora disso -1)
static int GetClubGradeBySlotSum983(int sum)
{
	if (sum < 30 || sum > 59)
		return -1;
	return (sum - 30) / 5;
}

// workshop_levelup/rankup: 1º taco do jogador fora o equipado que ainda não está no grau 5 (0 se nenhum)
static unsigned long FindWorkshopClub983()
{
	CSharedDoc* doc = CSharedDoc::Instance();
	for (std::map<unsigned int, sItemInfo>::iterator it = doc->m_clubSetMap.begin(); it != doc->m_clubSetMap.end(); ++it)
	{
		if (it->second.guid == doc->m_myInfo.userEquip.guidClubSet)
			continue;
		IFF_STRUCT::sClubSet* pClub = doc->m_itemManager.FindClubSet(it->second.tid);
		short* up = it->second.slotUp983;
		int sum = pClub->Slot[0] + pClub->Slot[1] + pClub->Slot[2] + pClub->Slot[3] + pClub->Slot[4] +
			up[0] + up[1] + up[2] + up[3] + up[4];
		if (GetClubGradeBySlotSum983(sum) != 5)
			return it->second.guid;
	}
	return 0;
}

// B1319: o 983 (00721420) le/grava o CGolfDoc em +0x158 (0x2c), +0x170 (0x32), +0x178 (0x4e), +0x180 (0x17), +0x198/+0x19c (0x41)
// e o sUserInfo em idx * 0x31c6 + 0x3922 (0x2c); o nosso usava os crus do 645 (+0x148/+0x160/+0x164/+0x16c/+0x184/+0x188,
// idx * 0xb92 + 0x1323): o 0x41 gravava float por cima de m_shotTime/m_shotPhase/m_currentPlayer.
C_ASSERT_VC10(offsetof(CGolfDoc, m_gameMode) == 0x158 && offsetof(CGolfDoc, m_bGodMode) == 0x170);
C_ASSERT_VC10(offsetof(CGolfDoc, m_debugBarValue983) == 0x178 && offsetof(CGolfDoc, m_remainTime) == 0x180);
C_ASSERT_VC10(offsetof(CGolfDoc, m_impactScale) == 0x198 && offsetof(CGolfDoc, m_controlScale) == 0x19c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame) + offsetof(sGolfGame, gameType) == 0x10594 && offsetof(CSharedDoc, m_golfGame) + offsetof(sGolfGame, shotTimeLimit) == 0x1069c); // B1485: ex-crus CSharedDoc+0x10594/+0x1069c
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + 0x72 == 0x3922 && sizeof(sUserInfo) == 0x31c6); // B1485: ex-cru CSharedDoc+idx*0x31c6+0x3922 (AUTO PLAYER)
void CConsole::Command(const char* cmd)
	{
		char buf[0x20];
		for (unsigned char i = 0; i < COMMAND_COUNT; i++)
		{
			int len = strlen(command_table[i].name);
			if (strncmp(cmd, command_table[i].name, len) ||
				(cmd[len] != ' ' && cmd[len] != 0))
				continue;
			switch (command_table[i].id)
			{
			case 0x49:
			{
				float speed;
				sscanf((cmd + len + 1), "%f", &speed);
				if (speed < 1.0f)
				{
					BadArgument(0x49);
					return;
				}
				m_fSpeed = speed;
				return;
			}
			case 0x48:
			{
				int index = -1;
				sscanf((cmd + len + 1), "%d", &index);
				GetPVS().PaintIndex(index);
				return;
			}
			case 0x47:
			{
				int target = -1;
				sscanf((cmd + len + 1), "%d", &target);
				if (target <= 0)
				{
					BadArgument(0x47);
					return;
				}
				CTaskManager::Instance()->PostMsg(NULL, "GolfRule", M983(0x9d),
					target, 0, 0, 0);
				return;
			}
			case 0x46:
			{
				CPartTidList& tid =
					GOLFDOC()->GetPartTidList(GOLFDOC()->m_currentPlayer);
				std::map<unsigned int, IFF_STRUCT::sPart>& parts =
					CSharedDoc::Instance()->m_itemManager.m_PartMap;
				FILE* fp = fopen("PartList.txt", "wt");
				int n = 1;
				std::map<unsigned int, IFF_STRUCT::sPart>::iterator it;
				for (it = parts.begin(); it != parts.end(); ++it)
				{
					unsigned int t = (*it).second.c.TypeId;
					if ((((t >> 0x12 & 0xff) ^ tid.m_charTid) & 0x3ffffff) ==
							0 &&
						(t & 0x600) == 0)
					{
						fprintf(fp, "%d(%2d): %s\n", t, n, (*it).second.c.Name);
						AddText(MakeStr("%d(%2d): %s", (*it).second.c.TypeId, n,
							(*it).second.c.Name));
						n++;
					}
				}
				std::map<unsigned int, IFF_STRUCT::sAuxPart>& auxParts =
					CSharedDoc::Instance()->m_itemManager.m_AuxPartMap;
				std::map<unsigned int, IFF_STRUCT::sAuxPart>::iterator it2;
				for (it2 = auxParts.begin(); it2 != auxParts.end(); ++it2)
				{
					fprintf(fp, "%d(%2d): %s\n", (*it2).second.c.TypeId, n,
						(*it2).second.c.Name);
					AddText(MakeStr("%d(%2d): %s", (*it2).second.c.TypeId, n,
						(*it2).second.c.Name));
					n++;
				}
				fclose(fp);
				return;
			}
			case 0x45:
			{
				AddText(CSceneManager::Instance()->ToggleTransparent()
						? "Transparent ON"
						: "Transparent OFF");
				return;
			}
			case 0x44:
			{
				float fb;
				fb = 0;
				sscanf((cmd + len + 1), "%f", &fb);
				if (fb < 0.01f)
				{
					BadArgument(0x44);
					return;
				}
				if (0)
					g_input->SetActive(false);
				g_input->SetSensitivity(fb); // B1486: era +0xe4 cru (cl 14.44 +0xe0)
				return;
			}
			case 0x43:
			{
				int a;
				a = -1;
				sscanf((cmd + len + 1), "%d", &a);
				if (a < 0)
				{
					BadArgument(0x43);
					return;
				}
				CTaskManager::Instance()->PostMsg(NULL, "GolfRule", M983(0x9b), a, 0,
					0, 0);
				return;
			}
			case 0x42:
			{
				float fb;
				fb = -1.0f;
				sscanf((cmd + len + 1), "%f", &fb);
				if (fb < 0.0f)
				{
					BadArgument(0x42);
					return;
				}
				AddText(MakeStr("%.2f->%.2f", (double)GetPVS().GetGreenFactor(),
					(double)fb));
				GetPVS().SetGreenFactor(fb);
				GetPVS().CalcGreenPlane();
				return;
			}
			case 0x41:
			{
				float fb;
				float fc;
				sscanf((cmd + len + 1), "%f %f", &fb, &fc);
				if (fb > 0.0f)
				{
					AddText(MakeStr("impact : %.1f->%.1f",
						GOLFDOC()->m_impactScale, fb));
					GOLFDOC()->m_impactScale = fb;
					GolfClub().SetAccuracy();
				}
				if (fc <= 0.0f)
					return;
				AddText(MakeStr("control : %.1f->%.1f",
					GOLFDOC()->m_controlScale, fc));
				GOLFDOC()->m_controlScale = fc;
				return;
			}
			case 0x3e:
			{
				float fb;
				sscanf((cmd + len + 1), "%f", &fb);
				if (fb < -50.0f || fb > 50.0f)
				{
					BadArgument(0x3e);
					return;
				}
				GetPVS().SetCondition(fb);
				CTaskManager::Instance()->PostMsg(NULL, "Screen", M983(0x1a3), 0, 0,
					0, 0);
				return;
			}
			case 0x3c:
			{
				float fb;
				sscanf((cmd + len + 1), "%f", &fb);
				if (fb < 0.0f)
				{
					BadArgument(0x3c);
					return;
				}
				GetPVS().SetBounceFactor(fb);
				return;
			}
			case 0x3d:
			{
				float fb;
				sscanf((cmd + len + 1), "%f", &fb);
				if (fb < 0.0f)
				{
					BadArgument(0x3d);
					return;
				}
				GetPVS().SetRollFactor(fb);
				return;
			}
			case 0x3b:
			{
				int a;
				a = -1;
				sscanf((cmd + len + 1), "%d", &a);
				if (a < 0 || a > 3)
				{
					BadArgument(0x3b);
					return;
				}
				for (unsigned char h = 0; h < 18; h++)
					CSharedDoc::Instance()->m_courseMap[h] =
						a == 0 ? rand() % 3 : a - 1;
				return;
			}
			case 0x3a:
			{
				float fb;
				sscanf((cmd + len + 1), "%f", &fb);
				if (fb < 0.0f || fb > 1.0f)
					BadArgument(0x3a);
				else
					((CGolfDocEx*)GOLFDOC())->UpdateBright(fb);
				return;
			}
			case 0x34:
			{
				int b;
				sscanf((cmd + len + 1), "%d", &b);
				if (b >= 0 && b <= 100)
				{
					AddText(MakeStr("power : Lv%d->Lv%d",
						(int)MYPLAYER()->power + 15, b));
					b -= 15;
					MYPLAYER()->power = b;
					CTaskManager::Instance()->PostMsg(NULL, "Spot", M983(0xf5), 0, 0,
						0, 0);
					GolfClub().SetAccuracy();
				}
				else
					BadArgument(0x34);
				return;
			}
			case 0x35:
			{
				int b;
				sscanf((cmd + len + 1), "%d", &b);
				if (b >= 0 && b <= 30)
				{
					AddText(MakeStr("impact : Lv%d->Lv%d",
						(int)MYPLAYER()->accuracy, b));
					MYPLAYER()->accuracy = b;
					GolfClub().SetAccuracy();
				}
				else
					BadArgument(0x35);
				return;
			}
			case 0x36:
			{
				int b;
				sscanf((cmd + len + 1), "%d", &b);
				if (b >= 0 && b <= 30)
				{
					AddText(MakeStr("control : Lv%d->Lv%d",
						(int)MYPLAYER()->control, b));
					MYPLAYER()->control = b;
				}
				else
					BadArgument(0x36);
				return;
			}
			case 0x37:
			{
				int b;
				sscanf((cmd + len + 1), "%d", &b);
				if (b >= 0 && b <= 30)
				{
					AddText(
						MakeStr("spin : Lv%d->Lv%d", (int)MYPLAYER()->spin, b));
					MYPLAYER()->spin = b;
					CTaskManager::Instance()->PostMsg(NULL, "Spin", 0x271a, 0,
						0, 0, 0);
					CTaskManager::Instance()->PostMsg(NULL, "Spin", M983(0x163), 0, 0,
						0, 0);
				}
				else
					BadArgument(0x37);
				return;
			}
			case 0x38:
			{
				int b;
				sscanf((cmd + len + 1), "%d", &b);
				if (b >= 0 && b <= 30)
				{
					AddText(MakeStr("curve : Lv%d->Lv%d",
						(int)MYPLAYER()->curve, b));
					MYPLAYER()->curve = b;
					CTaskManager::Instance()->PostMsg(NULL, "Spin", 0x271a, 0,
						0, 0, 0);
					CTaskManager::Instance()->PostMsg(NULL, "Spin", M983(0x163), 0, 0,
						0, 0);
				}
				else
					BadArgument(0x38);
				return;
			}
			case 0x39:
				COption::Instance()->vToggleFullscreen();
				return;
			case 0x33:
				if (!CHeadIcon::Instance())
					return;
				CHeadIcon::Instance()->Toggle();
				return;
			case 0x32:
			{
				CGolfDoc* doc = CSharedDoc::Instance()->m_pGolfDoc;
				doc->m_bGodMode = !doc->m_bGodMode;
				PerformPowerBarSetting();
				AddText(CSharedDoc::Instance()->m_pGolfDoc->m_bGodMode
						? "You are the God now! (~+_+)~"
						: "You are a man. T_T");
				return;
			}
			case 0x31:
			{
				int a;
				sscanf((cmd + len + 1), "%d", &a);
				if (!CSceneManager::Instance())
					return;
				FIELD(int, CSceneManager::Instance(), sizeof(CSceneManager) - 4) = a; // 983 00721420: m_debugDraw (+0x120; cl 14.44 +0x104 = ultimo dword da visao reduzida)
				return;
			}
			case 0x2f:
				if (!CSceneManager::Instance())
					return;
				FIELD(unsigned int, CSceneManager::Instance(), sizeof(CSceneManager) - 4) ^= 0x20; // 983: bit 5
				return;
			case 0x30:
				if (!CSceneManager::Instance())
					return;
				FIELD(unsigned int, CSceneManager::Instance(), sizeof(CSceneManager) - 4) ^= 0x40; // 983: bit 6
				return;
			case 0x2d:
				g_input->IncludeKey(NULL);
				return;
			case 0x2c:
			{
				if (!IsMyTurn(true))
					return;
				if ((GOLFDOC()->m_gameMode & 0xfffffbfd) == 0)
					return;
				if (CSharedDoc::Instance()->m_playingMode != 1)
					return;
				unsigned char idx;
				if (WNetworkSystem::Instance()->IsConnected(
						(WNetworkSystem::eNetUnit)0) == 0)
					idx = GOLFDOC()->m_currentPlayer;
				else
					idx = MyIndex();
				AddText((*((unsigned char*)&CSharedDoc::Instance()->m_userInfo[idx] + 0x72) &
							1) != 0
						? "AUTO PLAYER OFF"
						: "AUTO PLAYER ON");
				CTaskManager::Instance()->PostMsg(NULL, "GolfRule", M983(0x90), idx,
					0, 0, 0);
				return;
			}
			case 0x2a:
				((CSceneManagerEx*)CSceneManager::Instance())->DropTest();
				return;
			case 0x28:
			{
				int a;
				sscanf((cmd + len + 1), "%d", &a);
				CTaskManager::Instance()->PostMsg(NULL, "Grid", M983(0x1d3), a, 0, 0,
					0);
				return;
			}
			case 0x29:
			{
				int b;
				sscanf((cmd + len + 1), "%d", &b);
				if (b == 1)
				{
					CTaskManager::Instance()->PostMsg(NULL, "Grid", M983(0x163), 0, 0,
						0, 0);
					return;
				}
				CTaskManager::Instance()->PostMsg(NULL, "Grid", M983(0x164), 0, 0, 0,
					0);
				return;
			}
			case 0x27:
			{
				int b;
				sscanf((cmd + len + 1), "%d", &b);
				if (b < 0 || b > GolfBall().m_renderNum)
					BadArgument(0x27);
				else
					GolfBall().m_pos = GolfBall().PreData(b)->pos;
				return;
			}
			case 0x21:
				CSharedDoc::Instance()
					->m_pGolfDoc->m_pPolySoup->MakeStatistics();
				return;
			case 0x1a:
			{
				sscanf((cmd + len + 1), "%s", buf);
				if (!g_resrcmng->FindTexture(buf))
					AddText("Such Texture Is Not Found");
				return;
			}
			case 0x19:
			{
				float fb;
				fb = 0;
				sscanf((cmd + len + 1), "%f", &fb);
				if (fb < 0.0f)
				{
					BadArgument(0x19);
					return;
				}
				GetPVS().m_vec21d8_983.x = fb; // B1482: ex-FIELD(float, &GetPVS(), 0x21d8) (cl 14.44 desloca o membro)
				return;
			}
			case 0x16:
			{
				float fc;
				if (FIELD(int, &COption::Instance()->Opt983(), 0x318) != 0) // B1486: COption+0x3e4 (VC10) = m_opt+0x318, POD; o COption anda -4 no cl 14.44
				{
					AddText("SoftRender is required");
					return;
				}
				sscanf((cmd + len + 1), "%f", &fc);
				CSharedDoc::Instance()
					->m_pGolfDoc->m_pPolySoup->SetCullDistance(fc * 3.2f);
				return;
			}
			case 0x15:
			{
				// 983 007226d8: aabb do modelo em +0x98 (a visao local lia +0x60, do 645)
				WPolySoup::sModel* m = GOLFDOC()->m_pPolySoup->FindModel(NULL);
				WVector mn = m->aabb.min;
				WVector mx = m->aabb.max;
				AddText(MakeStr("%.0f %.0f %.0f", (mx.x - mn.x) * 0.3125f,
					(mx.y - mn.y) * 0.3125f, (mx.z - mn.z) * 0.3125f));
				return;
			}
			case 0x14:
			{
				int a;
				float fb;
				float dir = Wind().GetGlobalDirection();
				a = sscanf((cmd + len + 1), "%f %f", &fb, &dir);
				if (fb > 10.0f)
				{
					BadArgument(0x14);
					return;
				}
				Wind().SetIntensity(floorf(fb));
				Wind().SetDirection(a == 2 ? dir * g_DEGTORAD : dir);
				CSharedDoc::Instance()
					->m_pGolfDoc->m_pPolySoup->UpdateBgFxWind();
				IRbVObj* weather = (IRbVObj*)CSceneManager::Instance()->m_pWeather; // B1486: era +0xb0 cru (cl 14.44 +0xac)
				if (weather == NULL)
					return;
				weather->vfunc4();
				return;
			}
			case 0x12:
			{
				char str[0x20];
				sscanf((cmd + len + 1), "%s", str);
				CTaskManager::Instance()->PostMsg(NULL, "GolfRule", M983(0x85),
					(int)str, 0, 0, 0);
				return;
			}
			case 0x13:
			{
				char* file = new char[0x20];
				sscanf((cmd + len + 1), "%s", file);
				if (fopen(MakeStr("%s.sav", file), "rb"))
				{
					CTaskManager::Instance()->PostMsg(NULL, "GolfRule", M983(0x86),
						(int)file, 0, 0, 0);
					return;
				}
				AddText("No Save File");
				return;
			}
			case 0x11:
			{
				int a;
				for (a = 0; a < GOLFDOC()->m_playerNum; a++)
				{
					if (GOLFDOC()->GetPlayer(a)->state != 0)
						sprintf(buf, "%d : -", a);
					else
					{
						WVector pin = GetHoleCupPos();
						float dx = pin.x - GOLFDOC()->GetPlayer(a)->pos.x;
						float dz = pin.z - GOLFDOC()->GetPlayer(a)->pos.z;
						float dist = sqrtf(dx * dx + dz * dz);
						sprintf(buf, "%d : %.2fy(%.2f)", a, dist * 0.3125f,
							dist);
					}
					AddText(buf);
				}
				return;
			}
			case 0x0:
				for (unsigned int k = 0xc; k < 0x42c; k += 0xc)
					AddText(*(char**)((char*)&command_table[0].help + k));
				return;
			case 0x2:
				for (char* p = m_textList.Start(); p; p = m_textList.Next())
				{
					if (p)
						delete[] p;
				}
				m_textList.Reset();
				m_scroll = 0;
				return;
			case 0x4:
			{
				int serial, level;
				unsigned int item;
				int n =
					sscanf((cmd + len + 1), "%d %d %d", &serial, &level, &item);
				if (n >= 3 && n != -1 && level >= 1 && level <= 4)
				{
					unsigned int tid;
					if (serial == -1)
						tid = 0;
					else
					{
						tid = serial | 0x1c000000;
						CItemManager* im =
							&CSharedDoc::Instance()->m_itemManager;
						if (im->FindCaddie(tid) == NULL)
							goto bad_caddie;
						if (tid != 0)
						{
							if (item == 0)
								goto post_caddie;
							if ((((item >> 0x15 & 0x1f) ^ tid) & 0x3ffffff) !=
								0)
								goto bad_caddie;
						}
					}
					if (item == 0 ||
						CSharedDoc::Instance()->m_itemManager.FindCadItem(
							item) != NULL)
					{
post_caddie:
						_POSTMSG(this, "Caddie", 0x4a, tid, level - 1, item, 0);
						return;
					}
				}
bad_caddie:
				AddText(
					"caddie [serial no.] [level(1-4)] [caddie item typeid]");
				return;
			}
			case 0x6:
			{
				int a;
				a = -1;
				sscanf((cmd + len + 1), "%d", &a);
				if (a == -1)
				{
					for (int k = 0; k <
						CSharedDoc::Instance()->m_itemManager.GetCourseNum();
						k++)
					{
						if (k < CSharedDoc::Instance()
									->m_itemManager.GetCourseNum())
						{
							char line[0x1c];
							sprintf(line, "%d:%s", k,
								CSharedDoc::Instance()
									->m_itemManager
									.FindCourse(
										(k > 0x7f && k != 0xfd ? k - 0x80 : k) |
										0x28000000)
									->c.Name);
							AddText(line);
						}
					}
					return;
				}
				if (a == GetCurMap())
					return;
				if (SetCurMap((unsigned char)a))
				{
					g_audio->StopSfx(NULL);
					for (int h = 1; h <= 18; h++)
					{
						GOLFDOC()->GetHoleData((unsigned char)h).playerNum = 0; // B1487: era +0x19 cru (sHoleData POD, mesmo layout no cl 14.44)
						GOLFDOC()->GetHoleData((unsigned char)h).result = 3; // +0x1e
						GOLFDOC()->m_pPolySoup->LoadMapCheckData(
							(unsigned char)h);
					}
					CTaskManager::Instance()->PostMsg(NULL, "GolfBg", M983(0x140), 0,
						0, 0, 0);
					CTaskManager::Instance()->PostMsg(NULL, "GolfBg", M983(0x13e), 0,
						0, 0, 0);
					return;
				}
				AddText("map [serial no.]");
				return;
			}
			case 0x7:
			{
				int a;
				sscanf((cmd + len + 1), "%d", &a);
				if (a == CSharedDoc::Instance()->m_pGolfDoc->m_currentHole)
					return;
				if (a >= 1 && a <= 0x12)
				{
					g_audio->StopSfx(NULL);
					CSharedDoc::Instance()->m_pGolfDoc->m_currentHole =
						(unsigned char)a;
					CTaskManager::Instance()->PostMsg(NULL, "GolfBg", M983(0x140), 0,
						0, 0, 0);
				}
				else
					AddText("Invalid Argument");
				return;
			}
			case 0x9:
			{
				int a;
				sscanf((cmd + len + 1), "%d", &a);
				if (a > 3)
				{
					BadArgument(0x9);
					return;
				}
				CTaskManager::Instance()->PostMsg(NULL, "GolfBg", M983(0x142), a, 0,
					0, 0);
				CTaskManager::Instance()->PostMsg(NULL, "GolfBg", M983(0x13d), a, 0,
					0, 0);
				if (a < 2)
				{
					CSharedDoc::Instance()->m_pGolfDoc->m_pSky->GenerateStars(
						-1);
					CSharedDoc::Instance()
						->m_pGolfDoc->m_pSky->GenerateLensFlare();
					CSharedDoc::Instance()->m_pGolfDoc->m_pSky->GenerateMoons();
				}
				CTaskManager::Instance()->PostMsg(NULL, "Screen", M983(0x1a3), 0, 0,
					0, 0);
				return;
			}
			case 0xa:
			{
				int a;
				sscanf((cmd + len + 1), "%d", &a);
				return;
			}
			case 0xb:
			{
				int a;
				a = 0;
				sscanf((cmd + len + 1), "%d", &a);
				CTaskManager::Instance()->PostMsg(NULL, "Player", M983(0x11c),
					a | 0x10000000, 1, 0, 0);
				return;
			}
			case 0xc:
			{
				float fb;
				sscanf((cmd + len + 1), "%f", &fb);
				if (((char)CSharedDoc::Instance()->m_golfGame.gameType) == 1)
				{
					TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)
						->gauge = Between(0.0f,
						TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)->gauge +
							fb,
						CSharedDoc::Instance()->GetComboGaugeLimit(
							PLAYER(GOLFDOC()->m_currentPlayer)->team));
					return;
				}
				PLAYER(GOLFDOC()->m_currentPlayer)->gauge = Between(0.0f,
					PLAYER(GOLFDOC()->m_currentPlayer)->gauge + fb,
					CSharedDoc::Instance()->GetComboGaugeLimit(
						GOLFDOC()->m_currentPlayer));
				return;
			}
			case 0xd:
				CSceneManager::Instance()->ShowTag();
				return;
			case 0xe:
			{
				int b;
				if (FIELD(int, &COption::Instance()->Opt983(), 0x318) != 0)
				{
					sscanf((cmd + len + 1), "%d", &b);
					g_resrcmng->VideoReference()->Command((wVDevMessage)0x21, b,
						0);
					return;
				}
				sscanf((cmd + len + 1), "%d", &b);
				if (b < 0 || b > 3)
				{
					BadArgument(0xe);
					return;
				}
				CTaskManager::Instance()->PostMsg(NULL, "GolfBg", M983(0x118), b, 0,
					0, 0);
				CTaskManager::Instance()->PostMsg(NULL, "Player", M983(0x118), b, 0,
					0, 0);
				CTaskManager::Instance()->PostMsg(NULL, "Caddie", M983(0x118), b, 0,
					0, 0);
				return;
			}
			case 0xf:
			{
				int a;
				int b;
				a = sscanf((cmd + len + 1), "%d", &b);
				if (a != 1 || b < 0 || b >= 3)
					AddText("fill [value(0-2)]");
				else
					g_resrcmng->VideoReference()->Command((wVDevMessage)0x21, b,
						0);
				return;
			}
			case 0x10:
			{
				int a;
				sscanf((cmd + len + 1), "%d", &a);
				if (a < 10 || a > 0x78)
					BadArgument(0x10);
				else
					g_view->SetFOV((float)a * g_DEGTORAD);
				return;
			}
			case 0x17:
				if ((*(int*)&CSharedDoc::Instance()->m_golfGame.shotTimeLimit) == 0)
				{
					(*(int*)&CSharedDoc::Instance()->m_golfGame.shotTimeLimit) = 40000;
					CSharedDoc::Instance()->m_pGolfDoc->m_remainTime = 40000;
					AddText("TIME LIMIT ON");
					return;
				}
				(*(int*)&CSharedDoc::Instance()->m_golfGame.shotTimeLimit) = 0;
				AddText("TIME LIMIT OFF");
				return;
			case 0x18:
				AddText(MakeStr(" CameraChecking: %s",
					CCameraWork::Instance()->ToggleCameraChecking() ? "on"
																	: "off"));
				return;
			case 0x1b:
			{
				int b;
				sscanf((cmd + len + 1), "%d", &b);
				if (b < 0 || b > 2)
				{
					BadArgument(0x1b);
					return;
				}
				COption::Instance()->vSetParticleLevel(b);
				return;
			}
			case 0x1d:
			{
				int a;
				int b;
				a = sscanf((cmd + len + 1), "%d", &b);
				if (a != 0 && b >= 0 && b < 6)
				{
					g_resrcmng->VideoReference()->Command((wVDevMessage)0x22, b,
						0);
					return;
				}
				BadArgument(0x1d);
				AddText("0: D3DTEXF_NONE");
				AddText("1: D3DTEXF_POINT");
				AddText("2: D3DTEXF_LINEAR");
				AddText("3: D3DTEXF_ANISOTROPIC");
				AddText("4: D3DTEXF_FLATCUBIC");
				AddText("5: D3DTEXF_GAUSSIANCUBIC");
				return;
			}
			case 0x1e:
			{
				char name[0x100] = "";
				char str[0x20];
				sscanf(cmd, "%s %s", str, name);
				if (name[0] == '\0')
				{
					BadArgument(0x1e);
					return;
				}
				int index = atoi(name);
				CPartTidList& tid =
					GOLFDOC()->GetPartTidList(GOLFDOC()->m_currentPlayer);
				CSharedDoc* sd = CSharedDoc::Instance();
				int n = 1;
				std::map<unsigned int, IFF_STRUCT::sPart>::iterator it;
				for (it = sd->m_itemManager.m_PartMap.begin();
					it != sd->m_itemManager.m_PartMap.end(); ++it)
				{
					unsigned int t = (*it).second.c.TypeId;
					if ((tid.m_charTid & 0x3ffffff) == (t >> 0x12 & 0xff) &&
						(t & 0x600) == 0)
					{
						if (n == index)
						{
							CTaskManager::Instance()->PostMsg(NULL, "Player",
								M983(0x117), (*it).second.c.TypeId, 0, 0, 0);
							return;
						}
						n++;
					}
				}
				std::map<unsigned int, IFF_STRUCT::sAuxPart>::iterator it2;
				for (it2 = sd->m_itemManager.m_AuxPartMap.begin();
					it2 != sd->m_itemManager.m_AuxPartMap.end(); ++it2)
				{
					if (n == index)
					{
						CTaskManager::Instance()->PostMsg(NULL, "Player", M983(0x117),
							(*it2).second.c.TypeId, 0, 0, 0);
						return;
					}
					n++;
				}
				AddText(MakeStr("can't find %s", name));
				return;
			}
			case 0x1f:
			{
				char str[0x20];
				char arg[0x20] = "";
				sscanf(cmd, "%s %s", str, arg);
				if (arg[0] == '\0')
				{
					BadArgument(0x1f);
					return;
				}
				unsigned long tidv = strtoul(arg, NULL, 10);
				CPartTidList& tid =
					GOLFDOC()->GetPartTidList(GOLFDOC()->m_currentPlayer);
				std::map<unsigned int, IFF_STRUCT::sPart>& parts =
					CSharedDoc::Instance()->m_itemManager.m_PartMap;
				std::map<unsigned int, IFF_STRUCT::sAuxPart>& auxParts =
					CSharedDoc::Instance()->m_itemManager.m_AuxPartMap;
				std::map<unsigned int, IFF_STRUCT::sPart>::iterator it;
				for (it = parts.begin(); it != parts.end(); ++it)
				{
					if ((*it).second.c.TypeId == tidv)
					{
						if (((((*it).second.c.TypeId >> 0x12 & 0xff) ^
								 tid.m_charTid) &
								0x3ffffff) == 0)
						{
							CTaskManager::Instance()->PostMsg(NULL, "Player",
								M983(0x117), (*it).second.c.TypeId, 0, 0, 0);
							return;
						}
						AddText(MakeStr("wrong character's typeid : %d", tidv));
					}
				}
				std::map<unsigned int, IFF_STRUCT::sAuxPart>::iterator it2;
				for (it2 = auxParts.begin(); it2 != auxParts.end(); ++it2)
				{
					if ((*it2).second.c.TypeId == tidv)
					{
						CTaskManager::Instance()->PostMsg(NULL, "Player", M983(0x117),
							(*it2).second.c.TypeId, 0, 0, 0);
						return;
					}
				}
				AddText(MakeStr("can't find %s", arg));
				return;
			}
			case 0x22:
			{
				int a;
				a = -1;
				sscanf((cmd + len + 1), "%d", &a);
				if (a < 1)
				{
					BadArgument(0x22);
					return;
				}
				AddText(MakeStr(" Pangya Area: x%d", a));
				CPowerGauge::Instance()->SetDebugPangYaArea((float)a);
				return;
			}
			case 0x23:
			{
				int a;
				a = -1;
				sscanf((cmd + len + 1), "%d", &a);
				if (a < 1)
				{
					BadArgument(0x23);
					return;
				}
				AddText(MakeStr("AirNight Lucky Pangya Area: %d", a));
				CPowerGauge::Instance()->SetDebugAirNightLuckyPangYaArea(
					(float)a);
				return;
			}
			case 0x24:
			{
				int a;
				int n = sscanf(cmd + len, "%d", &a);
				if (n == 0 || n == -1)
				{
					AddText("ball [serial no.]");
					return;
				}
				int tid = a | 0x14000000;
				CTaskManager::Instance()->PostMsg(NULL, "Ball", M983(0x4c), tid, 0, 0,
					0);
				CTaskManager::Instance()->PostMsg(NULL, "Screen", M983(0x4c), tid, 0,
					0, 0);
				return;
			}
			case 0x26:
			{
				int a;
				a = 0;
				sscanf((cmd + len + 1), "%d", &a);
				if (a < 1)
				{
					BadArgument(0x26);
					return;
				}
				AddText(MakeStr("Picture Scale = x%d", a));
				CTaskManager::Instance()->PostMsg(NULL, "GolfRule", M983(0x8c), a, 0,
					0, 0);
				return;
			}
			case 0x2b:
				CTaskManager::Instance()->PostMsg(NULL, "GolfRule", M983(0x78),
					0x20000, 0, 0, 0);
				return;
			case 0x2e:
			{
				int a;
				a = 0;
				sscanf((cmd + len + 1), "%d", &a);
				if (a == 1)
					a = 0x10;
				else if (a == 2)
					a = 0x20;
				else if (a == 3)
					a = 0x40;
				CTaskManager::Instance()->PostMsg(NULL, "GolfRule", M983(0x91), a, 0,
					0, 0);
				CPowerGauge::Instance()->SetDebugPangYaArea(10.0f);
				AddText(MakeStr(" Pangya Area: x%d", 10));
				if (((char)CSharedDoc::Instance()->m_golfGame.gameType) == 1)
					TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)
						->gauge = Between(0.0f,
						TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)->gauge +
							100.0f,
						CSharedDoc::Instance()->GetComboGaugeLimit(
							PLAYER(GOLFDOC()->m_currentPlayer)->team));
				else
					PLAYER(GOLFDOC()->m_currentPlayer)->gauge = Between(0.0f,
						PLAYER(GOLFDOC()->m_currentPlayer)->gauge + 100.0f,
						CSharedDoc::Instance()->GetComboGaugeLimit(
							GOLFDOC()->m_currentPlayer));
				AddText(MakeStr(" Combo: +%d", 100));
				return;
			}
			case 0x4a:
			{
				std::string path;
				std::map<std::string, resBitmap_t> bres(
					g_pFresh->ListBres(path));
				int total = 0;
				const char* line = MakeStr("[%s]", path.c_str());
				Print(line);
				for (std::map<std::string, resBitmap_t>::iterator it =
						 bres.begin();
					it != bres.end(); ++it)
				{
					line = MakeStr("%-30s: %d bytes", (*it).second.name.c_str(),
						(*it).second.size);
					Print(line);
					total += (*it).second.size;
				}
				line = MakeStr("total %d bytes loaded", total);
				Print(line);
				return;
			}
			case 0x4b:
				sscanf((cmd + len + 1), "%f %d", &causticSpeed,
					&causticAlpha);
				if (causticAlpha < 0xff)
					return;
				causticAlpha = 0xff;
				return;
			case 0x4c:
			{
				WSendPacket packet(0x89); // JP 983 (s63): 0x86 no KR 645
				packet.Send((eSendTo)0);
				return;
			}
			case 0x4d:
			{
				float fb;
				sscanf((cmd + len + 1), "%f", &fb);
				GetPVS().GravityModifier(1.0f / fb);
				return;
			}
			case 0x4e:
			{
				int a;
				a = 0;
				sscanf((cmd + len + 1), "%d", &a);
				switch (a)
				{
				case 2:
					CSharedDoc::Instance()->m_pGolfDoc->m_debugBarValue983 =
						138.0f;
					return;
				case 3:
					CSharedDoc::Instance()->m_pGolfDoc->m_debugBarValue983 =
						138.5f;
					return;
				case 4:
					CSharedDoc::Instance()->m_pGolfDoc->m_debugBarValue983 =
						139.0f;
					return;
				case 5:
					CSharedDoc::Instance()->m_pGolfDoc->m_debugBarValue983 =
						139.5f;
					return;
				case 6:
					CSharedDoc::Instance()->m_pGolfDoc->m_debugBarValue983 =
						140.0f;
					return;
				case 7:
					CSharedDoc::Instance()->m_pGolfDoc->m_debugBarValue983 =
						140.5f;
					return;
				case 8:
					CSharedDoc::Instance()->m_pGolfDoc->m_debugBarValue983 =
						141.5f;
					return;
				case 9:
					CSharedDoc::Instance()->m_pGolfDoc->m_debugBarValue983 =
						142.0f;
					return;
				default:
					CSharedDoc::Instance()->m_pGolfDoc->m_debugBarValue983 = 0;
					return;
				}
			}
			case 0x53:
			{
				CTask* task = CTaskManager::Instance() == NULL
					? NULL
					: CTaskManager::Instance()->GetCurrentTask();
				if (IS_EXACTKINDOF(CShopTask, task) ||
					IS_EXACTKINDOF(CRealMyRoomTask, task))
				{
					unsigned int tidv = 0;
					unsigned int mask = 0;
					sscanf((cmd + len + 1), "%d %d", &tidv, &mask);
					task->SendMsgToMainActor(
						MsgObject(NULL, M983(0x266), tidv, 0, 0, 0, 0));
					AddText(CSharedDoc::Instance()
							->m_itemManager.ChangePartsPosmask(tidv, mask));
					task->SendMsgToMainActor(
						MsgObject(NULL, M983(0x267), tidv, 0, 0, 0, 0));
				}
				else
					AddText(
						"You can use this function only in the shop and myroom");
				return;
			}
			case 0x54:
			{
				CTask* task = CTaskManager::Instance() == NULL
					? NULL
					: CTaskManager::Instance()->GetCurrentTask();
				if (!(IS_EXACTKINDOF(CShopTask, task) ||
						IS_EXACTKINDOF(CRealMyRoomTask, task)))
				{
					AddText(
						"You can use this function only in the shop and myroom");
					return;
				}
				unsigned int tidv = 0;
				unsigned int texType = 0;
				unsigned int texNum = 0;
				char name[0x100] = "";
				sscanf((cmd + len + 1), "%d %d %d %s", &tidv, &texType, &texNum,
					name);
				task->SendMsgToMainActor(
					MsgObject(NULL, M983(0x266), tidv, 0, 0, 0, 0));
				AddText(
					CSharedDoc::Instance()->m_itemManager.ChangePartsTexName(
						tidv, texType, texNum, name));
				task->SendMsgToMainActor(
					MsgObject(NULL, M983(0x267), tidv, 0, 0, 0, 0));
				return;
			}
			case 0x55:
			{
				CTask* task = CTaskManager::Instance() == NULL
					? NULL
					: CTaskManager::Instance()->GetCurrentTask();
				if (!(IS_EXACTKINDOF(CShopTask, task) ||
						IS_EXACTKINDOF(CRealMyRoomTask, task)))
					return;
				char name[0x100] = "";
				unsigned int tidv = 0;
				sscanf((cmd + len + 1), "%d %s", &tidv, name);
				task->SendMsgToMainActor(
					MsgObject(NULL, M983(0x266), tidv, 0, 0, 0, 0));
				AddText(
					CSharedDoc::Instance()->m_itemManager.ChangePartsPetName(
						tidv, name));
				task->SendMsgToMainActor(
					MsgObject(NULL, M983(0x267), tidv, 0, 0, 0, 0));
				return;
			}
			case 0x57:
				if (lua_system::lua_script_manger::GetInstance()->RunString(
						(cmd + len + 1)))
					AddText("lua code Success!");
				return;
			case 0x56:
			{
				int b;
				sscanf((cmd + len + 1), "%d", &b);
				// cases in the order of the original's code (it decides the order of the literals)
				switch (b)
				{
				default:
					AddText("luacmd exec error. plz check luacmd index");
					return;
				case 2:
				{
					if (lua_system::lua_script_manger::GetInstance()->RunString(
							"dofile(\"test.lua\")"))
						AddText("luafile load ok. [test.lua]");
					CGimmickContainer* gimmick = NULL;
					CContentsDoc::Instance()->GetContainer(
						(localContentType_t)0x9b, gimmick);
					gimmick->LoadFromScript("test_table");
					return;
				}
				case 1:
					if (lua_system::lua_script_manger::GetInstance()->RunString(
							"dofile(\"test.lua\")"))
						AddText("luafile load ok. [test.lua]");
					return;
				case 0:
					lua_system::lua_script_manger::GetInstance()->ReloadAll();
					AddText("luafile Reload All.");
					return;
				}
			}
			case 0x58:
			{
				int b;
				CVectorSlideTestContainer* container = NULL;
				CContentsDoc::Instance()->GetContainer((localContentType_t)0x9a,
					container);
				if (!container)
					return;
				sscanf((cmd + len + 1), "%d", &b);
				container->SetNewTypeVectorSlide(b != 0);
				return;
			}
			case 0x59:
			{
				int b;
				CVectorSlideTestContainer* container = NULL;
				CContentsDoc::Instance()->GetContainer((localContentType_t)0x9a,
					container);
				if (!container)
					return;
				sscanf((cmd + len + 1), "%d", &b);
				container->SetLimitSpeed(b);
				return;
			}
			case 0x5a:
			{
				int b;
				CVectorSlideTestContainer* container = NULL;
				CContentsDoc::Instance()->GetContainer((localContentType_t)0x9a,
					container);
				if (!container)
					return;
				sscanf((cmd + len + 1), "%d", &b);
				container->SetBoundHeight(b);
				return;
			}
			case 0x5b:
			{
				int b;
				CVectorSlideTestContainer* container = NULL;
				CContentsDoc::Instance()->GetContainer((localContentType_t)0x9a,
					container);
				if (!container)
					return;
				sscanf((cmd + len + 1), "%d", &b);
				container->SetLimitVectorSlideCount(b);
				return;
			}
			// PKT (frente C, lote 33): comandos novos do 983 que mandam pacote
			case 0x68: // hio_holecup_scale (983 00721560)
			{
				unsigned int scale = -1;
				sscanf((cmd + len + 1), "%d", &scale);
				if (scale > 9)
					return;
				WSendPacket packet(0x8f);
				packet.Encode2(0x1a);
				packet.Encode4(scale);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x6f: // hub_dev (983 00724939)
			{
				unsigned int n = -1;
				sscanf((cmd + len + 1), "%d", &n);
				if (n - 1 > 0x86)
					return;
				WSendPacket packet(0x14c);
				packet.Encode4(n);
				packet.Send((eSendTo)0);
				return;
			}
			// missões (983 0x14d-0x14f; lista CSharedDoc+0x384c): pacote {n, n × uid}
			case 0x71: // quest_accept (983 007249c7): só a missão em START (1)
			{
				unsigned long uid = 0;
				sscanf((cmd + len + 1), "%d", &uid);
				std::list<_quest::CQuestItem>& quests = CSharedDoc::Instance()->m_quests983;
				std::list<_quest::CQuestItem>::iterator it = std::find_if(quests.begin(), quests.end(),
					[uid](const _quest::CQuestItem& q) { return q.GetUid983() == uid; }); // 00718120/0071dec0
				if (it == quests.end())
					return;
				if (!it->IsState983(1))
					return;
				WSendPacket packet(0x14d);
				int count = 0;
				int mark = packet.GetMark();
				packet.Encode4(count);
				packet.Encode4(uid);
				count++;
				packet.EncodeData(mark, (char*)&count, 4);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x72: // quest_accept_all (983 00724ad0)
			{
				WSendPacket packet(0x14d);
				int count = 0;
				int mark = packet.GetMark();
				packet.Encode4(count);
				std::list<_quest::CQuestItem>& quests = CSharedDoc::Instance()->m_quests983;
				for (std::list<_quest::CQuestItem>::iterator it = quests.begin(); it != quests.end(); ++it)
				{
					if (it->IsState983(1))
					{
						packet.Encode4(it->GetUid983());
						count++;
					}
				}
				packet.EncodeData(mark, (char*)&count, 4);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x73: // quest_reward (983 00724b76): só a missão em COMPLETE (4)
			{
				unsigned long uid = 0;
				sscanf((cmd + len + 1), "%d", &uid);
				std::list<_quest::CQuestItem>& quests = CSharedDoc::Instance()->m_quests983;
				std::list<_quest::CQuestItem>::iterator it = std::find_if(quests.begin(), quests.end(),
					[uid](const _quest::CQuestItem& q) { return q.GetUid983() == uid; }); // 00718150/0071df10
				if (it == quests.end())
					return;
				if (!it->IsState983(4))
					return;
				WSendPacket packet(0x14e);
				int count = 0;
				int mark = packet.GetMark();
				packet.Encode4(count);
				packet.Encode4(uid);
				count++;
				packet.EncodeData(mark, (char*)&count, 4);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x74: // quest_reward_all (983 00724c0f)
			{
				WSendPacket packet(0x14e);
				int count = 0;
				int mark = packet.GetMark();
				packet.Encode4(count);
				std::list<_quest::CQuestItem>& quests = CSharedDoc::Instance()->m_quests983;
				for (std::list<_quest::CQuestItem>::iterator it = quests.begin(); it != quests.end(); ++it)
				{
					if (it->IsState983(4))
					{
						packet.Encode4(it->GetUid983());
						count++;
					}
				}
				packet.EncodeData(mark, (char*)&count, 4);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x75: // quest_cancel (983 00724cb5): qualquer estado
			{
				unsigned long uid = 0;
				sscanf((cmd + len + 1), "%d", &uid);
				std::list<_quest::CQuestItem>& quests = CSharedDoc::Instance()->m_quests983;
				std::list<_quest::CQuestItem>::iterator it = std::find_if(quests.begin(), quests.end(),
					[uid](const _quest::CQuestItem& q) { return q.GetUid983() == uid; }); // 00718180/0071df60
				if (it == quests.end())
					return;
				WSendPacket packet(0x14f);
				int count = 0;
				int mark = packet.GetMark();
				packet.Encode4(count);
				packet.Encode4(uid);
				count++;
				packet.EncodeData(mark, (char*)&count, 4);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x78: // ab_dev (983 00724d9a)
			{
				int n = -1;
				sscanf((cmd + len + 1), "%d", &n);
				if (n != 1)
					return;
				WSendPacket packet(0x150);
				packet.Encode4(n);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x79: // ab_open (983 00724dec)
			{
				WSendPacket packet(0x151);
				packet.Send((eSendTo)0);
				return;
			}
			// missões "ab" (983 0x152-0x154): só as de QuestItem.iff +0xc8 == 1
			case 0x7a: // ab_accept (983 00724e27): em START e fora dos subtipos 1/2
			{
				WSendPacket packet(0x152);
				int count = 0;
				int mark = packet.GetMark();
				packet.Encode4(count);
				std::list<_quest::CQuestItem>& quests = CSharedDoc::Instance()->m_quests983;
				for (std::list<_quest::CQuestItem>::iterator it = quests.begin(); it != quests.end(); ++it)
				{
					if (it->IsState983(1) && !IsQuestSubtype12_983(it->GetTypeId983()) &&
						IsAbQuestItem983(it->GetTypeId983()))
					{
						packet.Encode4(it->GetUid983());
						count++;
					}
				}
				packet.EncodeData(mark, (char*)&count, 4);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x7b: // ab_reward (983 00724f0e): uma, em COMPLETE
			{
				unsigned long uid = 0;
				sscanf((cmd + len + 1), "%d", &uid);
				std::list<_quest::CQuestItem>& quests = CSharedDoc::Instance()->m_quests983;
				std::list<_quest::CQuestItem>::iterator it = std::find_if(quests.begin(), quests.end(),
					[uid](const _quest::CQuestItem& q) { return q.GetUid983() == uid; }); // 007181b0/0071dfb0
				if (it == quests.end())
					return;
				if (!it->IsState983(4) || !IsAbQuestItem983(it->GetTypeId983()))
					return;
				WSendPacket packet(0x153);
				int count = 0;
				int mark = packet.GetMark();
				packet.Encode4(count);
				packet.Encode4(uid);
				count++;
				packet.EncodeData(mark, (char*)&count, 4);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x7c: // ab_cancel (983 00724fd7): fora de START e fora do subtipo 0
			{
				WSendPacket packet(0x154);
				int count = 0;
				int mark = packet.GetMark();
				packet.Encode4(count);
				std::list<_quest::CQuestItem>& quests = CSharedDoc::Instance()->m_quests983;
				for (std::list<_quest::CQuestItem>::iterator it = quests.begin(); it != quests.end(); ++it)
				{
					if (!it->IsState983(1) && !IsQuestSubtype0_983(it->GetTypeId983()) &&
						IsAbQuestItem983(it->GetTypeId983()))
					{
						packet.Encode4(it->GetUid983());
						count++;
					}
				}
				packet.EncodeData(mark, (char*)&count, 4);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x7d: // compose_card (983 007250bf): 3 cartas sorteadas e o custo somado
			{
				CConsoleRandom983<unsigned long> table = MakeComposeCardTable983();
				WSendPacket packet(0x155);
				__int64 pang = 0;
				int mark = packet.GetMark();
				packet.Encode8(pang);
				for (int i = 3; i; i--)
				{
					unsigned long tid = table.Pick();
					packet.Encode4(tid);
					pang += GetComposeCardPang983(tid);
				}
				packet.EncodeData(mark, (char*)&pang, 8);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x80: // artifact_change (983 00725236)
			{
				unsigned int n = 0;
				sscanf((cmd + len + 1), "%d", &n);
				if (n > 0x10)
					return;
				// artefato n (1-16) = typeid 0x1a0001a4 + 2 * (n - 1); 0 tira o artefato
				unsigned long tid = n ? 0x1a0001a2 + n * 2 : 0;
				WSendPacket packet(0xa); // mudar sala: 1 campo (0xd = artefato)
				packet.Encode2(0xffff);
				unsigned char count = 0;
				int mark = packet.GetMark();
				packet.Encode1(count);
				packet.Encode1(0xd);
				packet.Encode4(tid);
				count++;
				packet.EncodeData(mark, (char*)&count, 1);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x84: // achievement_info (983 007253e2)
			{
				WSendPacket packet(0x157);
				packet.Encode4(CSharedDoc::Instance()->m_myInfo.info.dwUID);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x85: // auto_cmd (983 00725416)
			{
				WSendPacket packet(0x156);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x86: // workshop_levelup (983 00725432)
			{
				int n = 0;
				sscanf((cmd + len + 1), "%d", &n);
				unsigned long tid;
				unsigned short slot;
				switch (n)
				{
				case 1: case 2: case 3: case 4: case 5:
					tid = 0x7c800026;
					slot = (unsigned short)n;
					break;
				default:
					tid = 0x1a00020f;
					slot = 1;
					break;
				}
				unsigned long club = 0;
				if (!CSharedDoc::Instance()->m_clubSetMap.empty())
					club = FindWorkshopClub983();
				if (tid == 0 || slot == 0 || club == 0)
					return;
				WSendPacket packet(0x164);
				packet.Encode4(tid);
				packet.Encode2(slot);
				packet.Encode4(club);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x87: // workshop_levelup_apply (983 0072560b)
			{
				WSendPacket packet(0x165);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x88: // workshop_levelup_cancel (983 00725627)
			{
				WSendPacket packet(0x166);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x89: // workshop_rankup (983 00725643)
			{
				int n = 0;
				sscanf((cmd + len + 1), "%d", &n);
				unsigned short slot;
				switch (n)
				{
				case 1:
					slot = 0;
					break;
				case 2:
					slot = 1;
					break;
				case 3:
					slot = 2;
					break;
				case 4:
					slot = 3;
					break;
				case 5:
					slot = 5; // 983: 5 (não 4)
					break;
				default:
					return;
				}
				unsigned long club = FindWorkshopClub983();
				if (club == 0)
					return;
				WSendPacket packet(0x167);
				packet.Encode4(0x7c800041);
				packet.Encode2(slot);
				packet.Encode4(club);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x8a: // workshop_change_apply (983 007257d5)
			{
				WSendPacket packet(0x168);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x8b: // workshop_change_cancel (983 007257f1)
			{
				WSendPacket packet(0x169);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x8c: // workshop_exp_increment (983 0072580d)
			{
				unsigned int holes = 0;
				sscanf((cmd + len + 1), "%d", &holes);
				if (holes - 1 > 0x11)
					return;
				WSendPacket packet(0x16a);
				packet.Encode1((unsigned char)holes);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x8d: // workshop_cancel_recovery (983 0072586f)
			{
				WSendPacket packet(0x16b);
				packet.Encode4(0x1a000210);
				packet.Encode4(0x2630d3e1);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x8e: // workshop_exp_movement (983 007258ab)
			{
				int n = 0;
				sscanf((cmd + len + 1), "%d", &n);
				WSendPacket packet(0x16c);
				packet.Encode4(0x1a000211);
				packet.Encode4(0x2630d3e1);
				packet.Encode4(0x2630c7aa);
				packet.Encode4(n);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x8f: // workshop_init (983 0072594c)
			{
				unsigned long club = CSharedDoc::Instance()->m_myInfo.userEquip.guidClubSet;
				int n = 0;
				sscanf((cmd + len + 1), "%d", &n);
				unsigned long tid;
				if (n == 1)
					tid = 0x1a000247;
				else if (n == 2)
					tid = 0x1a00024b;
				else
					return;
				if (club == 0)
					return;
				WSendPacket packet(0x16d);
				packet.Encode4(tid);
				packet.Encode4(club);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x90: // istodaylogin (983 007259d8)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
				WSendPacket packet(0x16e);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x91: // nextdaylogin (983 00725a2b)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
				WSendPacket packet(0x170);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x92: // workshop_event_hole_count_info (983 00725a7e)
			{
				WSendPacket packet(0x172);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x93: // workshop_event_randombox (983 00725a9a)
			{
				int n = 0;
				sscanf((cmd + len + 1), "%d", &n);
				WSendPacket packet(0x173);
				packet.Encode4(n);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x94: // workshop_event_exp_event_on (983 00725aec)
			{
				WSendPacket packet(0x174);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x95: // workshop_event_play_hole_count (983 00725b08)
			{
				int n = 0;
				sscanf((cmd + len + 1), "%d", &n);
				WSendPacket packet(0x175);
				packet.Encode4(n);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x98: // gp_enter_lobby (983 00725e8c)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
				WSendPacket packet(0x176);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x99: // gp_leave_lobby (983 00725edf)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
				WSendPacket packet(0x177);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x9b: // gp_enter_room (983 00725f32)
			{
				int gp = 0;
				sscanf((cmd + len + 1), "%d", &gp);
				if (gp == 0)
					return;
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
				WSendPacket packet(0x179);
				packet.Encode4(gp);
				packet.Send((eSendTo)0);
				return;
			}
			case 0x9c: // gp_leave_room (983 00725fc8)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
				WSendPacket packet(0x17a); // mesmo registro do S5::UTIL::SendExitGPRoom983 (o numero do help nao e lido)
				packet.Encode1(0);
				packet.Encode2(0xffff);
				packet.Encode8(0);
				packet.Encode8(0);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xa7: // play_assist (983 007263a3)
			{
				WSendPacket packet(0x184);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xb2: // mastery_point_up (983 007266ad)
			{
				sCharacterInfo* ch = CSharedDoc::Instance()->FindChar983(0xffffffff);
				if (ch == NULL)
					return;
				WSendPacket packet(0x187);
				packet.Encode4(ch->tid);
				packet.Encode4(ch->guid);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xb3: // mastery_status_up (983 007266fd)
			{
				int stat = -1;
				sscanf((cmd + len + 1), "%d", &stat);
				if (stat < 0 || stat > 4) // 0 forca, 1 controle, 2 precisao, 3 spin, 4 curva
					return;
				sCharacterInfo* ch = CSharedDoc::Instance()->FindChar983(0xffffffff);
				if (ch == NULL)
					return;
				WSendPacket packet(0x188);
				packet.Encode4(stat);
				packet.EncodeBuffer(ch, 0x201);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xb4: // mastery_status_down (983 00726788)
			{
				int stat = -1;
				sscanf((cmd + len + 1), "%d", &stat);
				if (stat < 0 || stat > 4) // 0 forca, 1 controle, 2 precisao, 3 spin, 4 curva
					return;
				sCharacterInfo* ch = CSharedDoc::Instance()->FindChar983(0xffffffff);
				if (ch == NULL)
					return;
				WSendPacket packet(0x189);
				packet.Encode4(stat);
				packet.EncodeBuffer(ch, 0x201);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xb5: // mastery_attach_card (983 007267f0): carta sorteada no 1º slot livre do tipo dela
			{
				CConsoleRandom983<sConsoleCard983> table = MakeMasteryCardTable983();
				if (table.Empty())
					return;
				sCharacterInfo* ch = CSharedDoc::Instance()->FindChar983(0xffffffff);
				if (ch == NULL)
					return;
				sConsoleCard983 card = table.Pick();
				WSendPacket packet(0x18a);
				packet.Encode4(ch->tid);
				packet.Encode4(ch->guid);
				packet.Encode4(card.tid);
				packet.Encode4(card.uid);
				unsigned long slot = 0;
				switch ((card.tid >> 22) & 0xf)
				{
				case 0:
					slot = FindEmptyCardSlot983(ch, s_cardSlot0, 3);
					break;
				case 1:
					slot = FindEmptyCardSlot983(ch, s_cardSlot1, 2);
					break;
				case 5:
					slot = FindEmptyCardSlot983(ch, s_cardSlot5, 2);
					break;
				}
				if (slot == 0)
					return;
				packet.Encode4(slot);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xb6: // mastery_attach_bonus_card (983 0072693a)
			{
				CConsoleRandom983<sConsoleCard983> table = MakeMasteryBonusCardTable983();
				if (table.Empty())
					return;
				sCharacterInfo* ch = CSharedDoc::Instance()->FindChar983(0xffffffff);
				if (ch == NULL)
					return;
				sConsoleCard983 card = table.Pick();
				WSendPacket packet(0x18b);
				packet.Encode4(ch->tid);
				packet.Encode4(ch->guid);
				packet.Encode4(card.tid);
				packet.Encode4(card.uid);
				unsigned long slot = 0;
				switch ((card.tid >> 22) & 0xf)
				{
				case 0:
					slot = FindEmptyCardSlot983(ch, s_cardSlotBonus0, 1);
					break;
				case 1:
					slot = FindEmptyCardSlot983(ch, s_cardSlotBonus1, 1);
					break;
				}
				if (slot == 0)
					return;
				packet.Encode4(slot);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xb7: // mastery_detach_card (983 00726a6a): item sorteado tira a carta do 1º slot ocupado
			{
				CConsoleRandom983<sConsoleCard983> table = MakeCardRemoverTable983();
				if (table.Empty())
					return;
				sCharacterInfo* ch = CSharedDoc::Instance()->FindChar983(0xffffffff);
				if (ch == NULL)
					return;
				sConsoleCard983 item = table.Pick();
				WSendPacket packet(0x18c);
				packet.Encode4(ch->tid);
				packet.Encode4(ch->guid);
				packet.Encode4(item.tid);
				packet.Encode4(item.uid);
				int slot = 0;
				for (int i = 0; i < 12; i++)
				{
					if (ch->card[0][i] != 0)
					{
						slot = i + 1;
						break;
					}
				}
				if (slot == 0)
					return;
				packet.Encode4(slot);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xb9: // mp_break (983 00726b85): até 5 itens embaralhados, quantidade 1..4 sorteada
			{
				std::deque<sConsoleMPBreak983> items = MakeMPBreakList983();
				if (items.empty())
					return;
				WSendPacket packet(0x18d);
				int count = 0;
				int mark = packet.GetMark();
				packet.Encode4(count);
				for (std::deque<sConsoleMPBreak983>::iterator it = items.begin(); it != items.end(); ++it)
				{
					int n = it->count;
					if (n == 0)
						continue;
					int r = rand() % 5;
					if (r < 1)
						n = 1;
					else if (r <= n)
						n = r;
					packet.Encode4(it->tid);
					packet.Encode4(it->uid);
					packet.Encode4(n);
					if ((unsigned int)++count >= 5)
						break;
				}
				packet.EncodeData(mark, (char*)&count, 4);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xbc: // arin_dev (983 00726d1b)
			{
				int n = -1;
				sscanf((cmd + len + 1), "%d", &n);
				switch (n)
				{
				case 0: case 1: case 10: case 11: case 12: case 21: case 22: case 23: case 24:
					break;
				default:
					return;
				}
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
				WSendPacket packet(0x18f);
				packet.Encode4(n);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xbd: // arin_reward_1a (983 00726dc1)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
				WSendPacket packet(0x190);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xbe: // arin_reward_1b (983 00726e2a)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
				WSendPacket packet(0x191);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xbf: // arin_info (983 00726e93)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
				WSendPacket packet(0x192);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xc0: // arin_reward_mission (983 00726efc)
			{
				unsigned int n = 0;
				sscanf((cmd + len + 1), "%d", &n);
				if (n < 0x15 || n > 0x19)
					return;
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
				WSendPacket packet(0x193);
				packet.Encode4(n);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xc1: // arin_reward_2 (983 00726f9d)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
				WSendPacket packet(0x194);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xc2: // arin_open (983 00727006)
			{
				WSendPacket packet(0x192);
				packet.Send((eSendTo)0);
				return;
			}
			case 0xc3: // scratch_sync (983 00727038)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
				WSendPacket packet(0x195);
				packet.Send((eSendTo)0);
				return;
			}
			}
			break;
		}
		AddText(
			"Bad Command. Type 'help' -> list command or 'tweaker' -> list tweaker command");
	}


void CConsole::BadArgument(int index)
{
	// 983 (0071ed20): procura a entrada pelo id (passo 0xc) e copia o help para uma std::string
	std::string help;
	for (int i = 0; i < COMMAND_COUNT; i++)
	{
		if (command_table[i].id == index)
		{
			help.assign(command_table[i].help);
			break;
		}
	}
	AddText(MakeStr("Bad Argument : %s", help.c_str()));
}

void CConsole::AddText(const char* text)
{
	char* p = new char[strlen(text) + 1];
	strcpy(p, text);
	m_textList += p;
	m_scroll = m_textList.GetCount() - 19;
}
