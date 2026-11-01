// SPDX-License-Identifier: MIT
// main.cpp - the CaddieRig application window.
//
// Plain Win32: a tree of what is in the puppet, a property list that edits the
// PMP block behind the selection, an OpenGL viewport and a timeline of the
// motions and scripts. Menus and labels are Korean or English, chosen at run time.
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "audio.h"
#include "document.h"
#include "fxsim.h"
#include "puppetscript.h"
#include "petpose.h"
#include "texture.h"
#include "view.h"
#include "wardrobe.h"
#include <map>

// -- language -----------------------------------------------------------------

static bool g_korean;

static const wchar_t* T(const wchar_t* ko, const wchar_t* en)
{
	return g_korean ? ko : en;
}

static std::wstring Fmt(const wchar_t* fmt, ...)
{
	wchar_t buf[1024];
	va_list va;
	va_start(va, fmt);
	vswprintf_s(buf, fmt, va);
	va_end(va);
	return buf;
}

// English labels for the Korean PMP keys.
static const wchar_t* FieldEnglish(const std::string& key)
{
	static const struct { const char* ko; const wchar_t* en; } map[] = {
		{ "부모텍스쳐", L"Parent texture" }, { "텍스쳐이름", L"Texture name" }, { "텍스쳐파일", L"Texture file" },
		{ "텍스쳐연결", L"Atlas link" }, { "모양(o,ㅁ)", L"Shape (o,box)" }, { "보이기", L"Visible" },
		{ "박스이름", L"Box name" }, { "부모본이름", L"Parent bone" }, { "스크립트0", L"Script 0" },
		{ "스크립트1", L"Script 1" }, { "최소값x", L"Min X" }, { "최소값y", L"Min Y" }, { "최소값z", L"Min Z" },
		{ "최대값x", L"Max X" }, { "최대값y", L"Max Y" }, { "최대값z", L"Max Z" }, { "동작이름", L"Motion name" },
		{ "원본시작시간", L"Source start (s)" }, { "원본종료시간", L"Source end (s)" }, { "다음동작", L"Next motion" },
		{ "연결방법", L"Transition" }, { "연결시간", L"Transition time" }, { "최상위본", L"Apply bone" },
		{ "원본시작프레임", L"Source start frame" }, { "원본종료프레임", L"Source end frame" },
		{ "원본길이사용", L"Use source length" }, { "원본파일", L"Source file" }, { "원본유무", L"Source present" },
		{ "시작프레임", L"Start frame" }, { "종료프레임", L"End frame" }, { "그룹이름", L"Group" },
		{ "메시지", L"Message" }, { "플래그", L"Check flag" }, { "원본프레임", L"Source frame" },
	};
	for (size_t i = 0; i < sizeof(map) / sizeof(map[0]); ++i)
		if (key == map[i].ko)
			return map[i].en;
	return 0;
}

// -- ids ----------------------------------------------------------------------

enum
{
	ID_FILE_NEWASE = 100, ID_FILE_OPEN, ID_FILE_SAVE, ID_FILE_SAVEAS, ID_FILE_EXPORT, ID_FILE_ATTACH, ID_FILE_EXIT,
	ID_EDIT_ADDTEX, ID_EDIT_ADDBOX, ID_EDIT_ADDCENTER, ID_EDIT_ADDSCRIPT, ID_EDIT_IMPORTMOTION, ID_EDIT_DELETE,
	ID_BUILD_REBUILD, ID_BUILD_VALIDATE,
	ID_VIEW_TEX, ID_VIEW_FLAT, ID_VIEW_WIRE, ID_VIEW_SKEL, ID_VIEW_BOXES, ID_VIEW_NORMALS, ID_VIEW_GRID, ID_VIEW_LIT,
	ID_VIEW_FRAME, ID_VIEW_PERSP, ID_VIEW_FRONT, ID_VIEW_SIDE, ID_VIEW_TOP,
	ID_OPT_ROOT, ID_OPT_VER_KEEP, ID_OPT_VER_10, ID_OPT_VER_13, ID_OPT_KO, ID_OPT_EN,
	ID_HELP_FLAGS, ID_HELP_ABOUT,
	ID_TL_PLAY, ID_TL_STOP, ID_TL_PREV, ID_TL_NEXT, ID_TL_LOOP, ID_TL_FOLLOW,
	ID_VIEW_WARDROBE, ID_WARD_RESET,
	ID_FX_SOUND, ID_FX_EFFECTS, ID_FX_BOX, ID_FX_PARTS, ID_FX_FIRE, ID_FX_CLEAR,
	IDC_TREE = 300, IDC_PROPS, IDC_LOG, IDC_TOOLBAR, IDC_STATUS, IDC_TIMELINE, IDC_FRAME_LABEL
};

enum NodeKind { N_GROUP, N_TEXTURE, N_BONE, N_MOTION, N_SCRIPT, N_BOX, N_FACE, N_PART, N_CHUNK, N_FILE };

static LPARAM MakeNode(NodeKind kind, int index)
{
	return (LPARAM)(((int)kind << 20) | (index & 0xfffff));
}
static NodeKind NodeKindOf(LPARAM p) { return (NodeKind)(((int)p >> 20) & 0xff); }
static int NodeIndexOf(LPARAM p) { return (int)p & 0xfffff; }

// -- state --------------------------------------------------------------------

static HINSTANCE g_inst;
static HWND g_main, g_tree, g_props, g_log, g_toolbar, g_status, g_view, g_timeline;
static HWND g_btnPlay, g_btnStop, g_btnPrev, g_btnNext, g_btnLoop, g_btnFollow, g_frameLabel;
static HFONT g_font, g_fontSmall;
static Document g_doc;
static ViewOptions g_opts;
static int g_leftWidth = 300;
static bool g_splitDrag;

static float g_frame;
static bool g_playing;
static DWORD g_lastTick;
static int g_loopStart = 0, g_loopEnd = 0;
static int g_currentMotion = -1;
static float g_zoom = 2.0f;			// pixels per frame
static int g_scroll = 0;			// first visible frame

static int g_exportVer = 0;			// 0 keep, 10, 13
static LPARAM g_selection = MakeNode(N_FILE, 0);

struct PropRow
{
	std::wstring label, value;
	int field;		// index into the block's fields, -1 read only, -2 texture prefix toggle
	wchar_t flag[3];
};
static std::vector<PropRow> g_rows;
static HWND g_editor;
static int g_editorRow;
static WNDPROC g_editorProc;

static void RefreshTree();
static void RefreshProps();
static void UpdateStatus();
static void BuildMenu();
static void SyncChecks();
static void Layout();

// -- log ----------------------------------------------------------------------

static void LogAppend(const std::wstring& line)
{
	int len = GetWindowTextLengthW(g_log);
	if (len > 200000)
	{
		SetWindowTextW(g_log, L"");
		len = 0;
	}
	SendMessageW(g_log, EM_SETSEL, len, len);
	std::wstring s = line + L"\r\n";
	SendMessageW(g_log, EM_REPLACESEL, FALSE, (LPARAM)s.c_str());
}

static void FlushLog(const BuildLog& log)
{
	for (size_t i = 0; i < log.lines.size(); ++i)
		LogAppend(Wide(log.lines[i]));
}

// -- posing and playback ------------------------------------------------------

// -- PuppetScript -----------------------------------------------------------------

class AppFxHost : public FxHost
{
public:
	virtual std::string ResolveAsset(const std::string& name) { return g_doc.ResolveAsset(name); }
	virtual void PlaySound(const std::string& name, float volume);
	virtual FxPetModel* LoadPetModel(const std::string& name) { return (FxPetModel*)g_doc.LoadFxModel(name); }
};

static AppFxHost g_fxHost;
static FxBox g_fx(&g_fxHost);
static bool g_soundOn = true, g_fxOn = true, g_boxFxOn = true, g_partFxOn = false;
static size_t g_fxProblemsShown;
static DWORD g_fxTick;

// CSoundManager opens "sound/<name>.wav"; the matching directory finds it wherever it lives.
void AppFxHost::PlaySound(const std::string& name, float volume)
{
	if (!g_soundOn)
		return;
	std::string path = g_doc.ResolveAsset(name + ".wav");
	if (path.empty() || !AudioPlay(path, volume))
		g_fx.Problem("sound " + name);
}

static WVector BoxCenter(const PetColl& box)
{
	return WVector(box.box[0] + box.box[3], box.box[1] + box.box[4], box.box[2] + box.box[5]) * 0.5f;
}

static int FindSkeletonBone(const std::string& name)
{
	const std::vector<PetBone>& bones = g_doc.Skeleton();
	for (size_t i = 0; i < bones.size(); ++i)
		if (!_stricmp(bones[i].name.c_str(), name.c_str()))
			return (int)i;
	return -1;
}

// A bone by name, else a box: its bone plus w_bound_box_info::spherePivot.
static bool FindAttachPoint(const std::string& name, int slot, FxAttacher* a)
{
	int bone = FindSkeletonBone(name);
	if (bone >= 0)
	{
		a->bone[slot] = bone;
		return true;
	}
	const std::vector<PetColl>& boxes = g_doc.pet.colls;
	for (size_t i = 0; i < boxes.size(); ++i)
	{
		if (boxes[i].name != name)
			continue;
		a->bone[slot] = FindSkeletonBone(boxes[i].bone);
		a->hasPos[slot] = true;
		a->pos[slot] = BoxCenter(boxes[i]);
		return true;
	}
	return false;
}

// CGolfPlayer::ParseBoundingBoxScript: effects that run as long as the model is shown.
static void StartBoxEffects()
{
	g_fx.ClearAll(false);
	if (!g_boxFxOn || !g_fxOn)
		return;
	const std::vector<PetColl>& boxes = g_doc.pet.colls;
	int started = 0, conditional = 0;
	for (size_t i = 0; i < boxes.size(); ++i)
	{
		std::vector<BoxFx> list;
		ParseBoxScript(boxes[i].script0.c_str() + std::string(" ") + boxes[i].script1.c_str(), &list);
		for (size_t k = 0; k < list.size(); ++k)
		{
			// CGolfPlayer attaches a character's box effect only behind a part condition;
			// caddies and props have none to give.
			if (g_doc.IsAvatar() && list[k].parts.empty())
				continue;
			if (!list[k].parts.empty() && !g_partFxOn)
			{
				++conditional;
				continue;
			}
			FxAttacher a;
			a.bone[0] = FindSkeletonBone(boxes[i].bone);
			if (a.bone[0] < 0)
				continue;
			a.hasPos[0] = true;
			a.pos[0] = BoxCenter(boxes[i]);
			if (g_fx.Attach(a, list[k].file, false))
				++started;
		}
	}
	if (started || conditional)
		LogAppend(Fmt(T(L"박스 효과 %d개 시작, 파츠 조건으로 %d개 건너뜀", L"Box effects: %d started, %d skipped for part conditions"), started, conditional));
}

// avatar/visgroup.def: "group" "bone" "bone" ...
static std::map<std::string, std::vector<std::string> > g_visGroups;
static bool g_visGroupsLoaded;

static void LoadVisGroups()
{
	if (g_visGroupsLoaded)
		return;
	g_visGroupsLoaded = true;
	std::string path = g_doc.ResolveAsset("visgroup.def");
	FILE* f = path.empty() ? 0 : OpenUtf8(path.c_str(), L"rb");
	if (!f)
		return;
	char line[2048];
	while (fgets(line, sizeof(line), f))
	{
		std::vector<std::string> tokens;
		const char* p = line;
		while ((p = strchr(p, '"')) != 0)
		{
			const char* e = strchr(p + 1, '"');
			if (!e)
				break;
			tokens.push_back(std::string(p + 1, e - p - 1));
			p = e + 1;
		}
		if (tokens.size() > 1)
			g_visGroups[tokens[0]].assign(tokens.begin() + 1, tokens.end());
	}
	fclose(f);
}

// What *hide, *hidebone, *opa and *ptex have done. Sounds and effects happen once, but
// this state must be right wherever the timeline stops, so seeking replays it.
static float g_opaFrom = 1.0f, g_opaTo = 1.0f, g_opaFrames = 0.0f, g_opaStart = 0.0f;
static int g_pendingMotion = -1;

static void SetBoneHidden(const std::string& bone, bool hidden)
{
	int b = FindSkeletonBone(bone);
	if (b >= 0 && b < (int)g_doc.boneHidden.size())
		g_doc.boneHidden[b] = hidden;
	// The client adds each part's mesh bone to the skeleton (WBone::AttachMeshBone); here
	// they stay with the part.
	for (size_t p = 0; p < g_doc.parts.size(); ++p)
	{
		MeshPart& part = *g_doc.parts[p];
		for (size_t k = 0; k < part.pet->bones.size() && k < part.ownHidden.size(); ++k)
			if (part.boneMap[k] < 0 && !_stricmp(part.pet->bones[k].name.c_str(), bone.c_str()))
				part.ownHidden[k] = hidden;
	}
}

// CGolfPlayer::SetMotion does this at the start of every motion.
static void ResetMotionState()
{
	g_doc.boneHidden.assign(g_doc.boneHidden.size(), false);
	for (size_t p = 0; p < g_doc.parts.size(); ++p)
		g_doc.parts[p]->ownHidden.assign(g_doc.parts[p]->ownHidden.size(), false);
	g_doc.modelHidden = false;
	g_doc.modelAlpha = 1.0f;
	g_opaFrom = g_opaTo = 1.0f;
	g_opaFrames = 0.0f;
}

static void ResetScriptState()
{
	ResetMotionState();
	ViewResetTexParts();
}

static void UpdateScriptAlpha(float frame)
{
	float a = g_opaTo;
	if (g_opaFrames > 0.0f && frame < g_opaStart + g_opaFrames)
		a = g_opaFrom + (g_opaTo - g_opaFrom) * (frame - g_opaStart) / g_opaFrames;
	g_doc.modelAlpha = a < 0.0f ? 0.0f : (a > 1.0f ? 1.0f : a);
}

// The commands that change how the model looks.
static void RunVisualCommand(const ScriptCommand& c, const std::string& motionName, bool ignoreMotion, float frame)
{
	const char* t = c.token.c_str();
	if (!_stricmp(t, "*ptex("))
	{
		size_t i = 0;
		if (c.args.empty() || c.args[0].empty())
			return;
		if (c.args[0][0] == '@')
		{
			if (!ignoreMotion && _stricmp(c.args[0].c_str() + 1, motionName.c_str()))
				return;
			i = 1;
		}
		// A third string, as in "face" "blink1.png" "m06.png", is read by nothing in the client.
		if (i + 1 >= c.args.size())
			return;
		std::string why;
		if (!ViewApplyTexPart(c.args[i], c.args[i + 1], &why))
			g_fx.Problem("*ptex " + c.args[i] + " " + c.args[i + 1] + ": " + why);
	}
	// CGolfPlayer dispatches on neither of these, or on "*opa" without a bracket.
	else if (!_stricmp(t, "*hide") && !g_doc.IsAvatar())
		g_doc.modelHidden = true;
	else if (!_stricmp(t, "*show") && !g_doc.IsAvatar())
		g_doc.modelHidden = false;
	else if (!_stricmp(t, "*hidebone(") || !_stricmp(t, "*showbone("))
	{
		for (size_t i = 0; i < c.args.size(); ++i)
			SetBoneHidden(c.args[i], !_stricmp(t, "*hidebone("));
	}
	else if (!_stricmp(t, "*hidevis(") || !_stricmp(t, "*showvis("))
	{
		LoadVisGroups();
		for (size_t i = 0; i < c.args.size(); ++i)
		{
			std::map<std::string, std::vector<std::string> >::iterator g = g_visGroups.find(c.args[i]);
			if (g == g_visGroups.end())
				continue;
			for (size_t b = 0; b < g->second.size(); ++b)
				SetBoneHidden(g->second[b], !_stricmp(t, "*hidevis("));
		}
	}
	else if (!_stricmp(t, "*opa("))
	{
		// CGolfPlayer: "from to frames", from a @motion gate like the others.
		size_t i = 0;
		if (c.args.empty())
			return;
		if (!c.args[0].empty() && c.args[0][0] == '@')
		{
			if (!ignoreMotion && _stricmp(c.args[0].c_str() + 1, motionName.c_str()))
				return;
			i = 1;
		}
		if (i >= c.args.size())
			return;
		float from = 1.0f, to = 1.0f, time = 0.0f;
		sscanf_s(c.args[i].c_str(), "%f %f %f", &from, &to, &time);
		g_opaFrom = from;
		g_opaTo = time > 0.0f ? to : from;
		g_opaFrames = time;
		g_opaStart = frame;
		UpdateScriptAlpha(frame);
	}
	else if (!_stricmp(t, "*opa") && !g_doc.IsAvatar())
	{
		// CNpc reads this form as a percentage.
		float percent = (float)atof(c.rest.c_str());
		g_opaFrom = g_opaTo = percent * 0.01f;
		g_opaFrames = 0.0f;
		UpdateScriptAlpha(frame);
	}
}

// Rebuilds the state for a frame from the motion it belongs to.
static void ReplayScriptState(int motion, int frame)
{
	ResetScriptState();
	const PetFile& s = g_doc.ScriptSource();
	const PetFile& a = g_doc.AnimSource();
	bool hasMotion = motion >= 0 && motion < (int)a.motions.size();
	int start = hasMotion ? (int)a.motions[motion].startFrame : 0;
	static const std::string none;
	const std::string& name = hasMotion ? a.motions[motion].name : none;
	for (size_t i = 0; i < s.frames.size(); ++i)
	{
		int f = (int)s.frames[i].frame;
		if (f < start || f > frame)
			continue;
		std::vector<ScriptCommand> cmds;
		ParsePuppetScript(s.frames[i].script, &cmds);
		for (size_t c = 0; c < cmds.size(); ++c)
			RunVisualCommand(cmds[c], name, !hasMotion, (float)f);
	}
	UpdateScriptAlpha((float)frame);
}

// Runs one FRAM message. motionName gates "@motion" effects, as in ParsePuppetScript.
static void RunPuppetScript(const std::string& script, const std::string& motionName, bool ignoreMotion)
{
	std::vector<ScriptCommand> cmds;
	ParsePuppetScript(script, &cmds);
	for (size_t i = 0; i < cmds.size(); ++i)
	{
		const ScriptCommand& c = cmds[i];
		if (IsSoundCommand(c))
		{
			g_fxHost.PlaySound(c.args[0], 1.0f);
			continue;
		}
		RunVisualCommand(c, motionName, ignoreMotion, g_frame);
		if (!_stricmp(c.token.c_str(), "*playif(") && c.args.size() >= 2)
		{
			// CGolfPlayer: switch motion if any of the ';' separated parts is attached.
			std::string list = c.args[0] + ";";
			for (size_t at = 0, semi; (semi = list.find(';', at)) != std::string::npos; at = semi + 1)
			{
				std::string want = Cp949ToUtf8(list.substr(at, semi - at)) + ".mpet";
				for (size_t p = 0; p < g_doc.parts.size(); ++p)
				{
					if (_stricmp(g_doc.parts[p]->name.c_str(), want.c_str()))
						continue;
					const std::vector<PetMotion>& m = g_doc.AnimSource().motions;
					for (size_t k = 0; k < m.size(); ++k)
						if (m[k].name == c.args[1])
							g_pendingMotion = (int)k;
				}
			}
			continue;
		}
		if (!_stricmp(c.token.c_str(), "*fxstop("))
		{
			// No handler in this client's source. The arguments mirror *fx, so it is taken
			// to stop the effect *fx started.
			ScriptCommand asFx = c;
			asFx.token = "*fx(";
			ScriptFx stop;
			if (ParseFxCommand(asFx, &stop))
				g_fx.StopFile(stop.file);
			continue;
		}
		ScriptFx fx;
		if (!g_fxOn || !ParseFxCommand(c, &fx))
			continue;
		if (!ignoreMotion && !fx.motion.empty() && _stricmp(fx.motion.c_str(), motionName.c_str()))
			continue;
		if (!_stricmp(fx.file.c_str(), "*playbbfx"))
		{
			StartBoxEffects();
			continue;
		}
		if ((!fx.parts.empty() && !g_partFxOn) || fx.bones.empty())
			continue;
		FxAttacher a;
		for (size_t b = 0; b < fx.bones.size(); ++b)
			FindAttachPoint(fx.bones[b], (int)b, &a);
		g_fx.Attach(a, fx.file, true);
	}
}

static void ApplyPose()
{
	g_doc.Pose(g_frame / 30.0f);
	InvalidateRect(g_view, 0, FALSE);
}

static int MotionAtFrame(int frame);

static void UpdateFrameLabel()
{
	const PetFile& a = g_doc.AnimSource();
	std::wstring motion;
	if (g_currentMotion >= 0 && g_currentMotion < (int)a.motions.size())
		motion = WideCp(a.motions[g_currentMotion].name);
	SetWindowTextW(g_frameLabel, Fmt(L"%s %d  (%.2fs)  %s", T(L"프레임", L"Frame"), (int)g_frame, g_frame / 30.0f, motion.c_str()).c_str());
}

static void SetFrame(float frame, bool ensureVisible)
{
	float last = g_doc.LastFrame();
	if (frame < 0.0f)
		frame = 0.0f;
	if (frame > last)
		frame = last;
	g_frame = frame;
	if (ensureVisible)
	{
		RECT rc;
		GetClientRect(g_timeline, &rc);
		int visible = (int)(rc.right / g_zoom);
		if ((int)frame < g_scroll || (int)frame > g_scroll + visible - 4)
			g_scroll = (int)frame - visible / 4;
		if (g_scroll < 0)
			g_scroll = 0;
	}
	// Only playback moves the bones smoothly; any other change of frame is a teleport.
	if (!g_playing)
	{
		g_fx.ResetTrails();
		ReplayScriptState(g_currentMotion >= 0 ? g_currentMotion : MotionAtFrame((int)frame), (int)frame);
	}
	else
		UpdateScriptAlpha(frame);
	ApplyPose();
	InvalidateRect(g_timeline, 0, FALSE);
	UpdateFrameLabel();
	UpdateStatus();
}

static int MotionAtFrame(int frame)
{
	const PetFile& a = g_doc.AnimSource();
	for (size_t i = 0; i < a.motions.size(); ++i)
		if (frame >= (int)a.motions[i].startFrame && frame <= (int)a.motions[i].endFrame)
			return (int)i;
	return -1;
}

// CPetBody::ScriptProcess: frames from the first at or after "from" up to, but not
// including, a whole-numbered "to".
static void FireRange(float from, float to)
{
	if (from >= to)
		return;
	int first = (int)ceil(from - 0.03f);
	int last = to == floor(to) ? (int)to - 1 : (int)floor(to);
	const PetFile& s = g_doc.ScriptSource();
	const PetFile& a = g_doc.AnimSource();
	for (size_t i = 0; i < s.frames.size(); ++i)
	{
		int f = (int)s.frames[i].frame;
		if (f < first || f > last)
			continue;
		LogAppend(Fmt(L"[%04d] %s", f, WideCp(s.frames[i].script).c_str()));
		int m = g_currentMotion >= 0 ? g_currentMotion : MotionAtFrame(f);
		RunPuppetScript(s.frames[i].script, m >= 0 && m < (int)a.motions.size() ? a.motions[m].name : std::string(), false);
	}
}

static void SelectMotion(int index, bool jump)
{
	const PetFile& a = g_doc.AnimSource();
	if (index < 0 || index >= (int)a.motions.size())
	{
		g_currentMotion = -1;
		g_loopStart = 0;
		g_loopEnd = (int)g_doc.LastFrame();
		return;
	}
	g_currentMotion = index;
	g_loopStart = (int)a.motions[index].startFrame;
	g_loopEnd = (int)a.motions[index].endFrame;
	if (jump)
		SetFrame((float)g_loopStart, true);
}

static void SetPlaying(bool on)
{
	g_playing = on;
	g_lastTick = GetTickCount();
	SetWindowTextW(g_btnPlay, on ? T(L"일시정지", L"Pause") : T(L"재생", L"Play"));
	if (on)
		SetTimer(g_main, 1, 15, 0);
	else
		KillTimer(g_main, 1);
}

static void PlaybackTick()
{
	DWORD now = GetTickCount();
	float dt = (now - g_lastTick) / 1000.0f;
	g_lastTick = now;
	if (g_doc.LastFrame() <= 0.0f)
	{
		SetPlaying(false);
		return;
	}
	float from = g_frame;
	float next = g_frame + dt * 30.0f;
	int motionBefore = g_currentMotion >= 0 ? g_currentMotion : MotionAtFrame((int)g_frame);
	bool loop = SendMessageW(g_btnLoop, BM_GETCHECK, 0, 0) == BST_CHECKED;
	bool follow = SendMessageW(g_btnFollow, BM_GETCHECK, 0, 0) == BST_CHECKED;
	const PetFile& a = g_doc.AnimSource();
	if (g_loopEnd > g_loopStart && next > g_loopEnd)
	{
		FireRange(from, (float)g_loopEnd);
		int target = -1;
		if (follow && g_currentMotion >= 0 && g_currentMotion < (int)a.motions.size())
		{
			const std::string& name = a.motions[g_currentMotion].next;
			for (size_t i = 0; i < a.motions.size(); ++i)
				if (a.motions[i].name == name)
					target = (int)i;
			if (target == g_currentMotion || (target >= 0 && a.motions[target].endFrame <= a.motions[target].startFrame))
				target = -1;
		}
		if (target >= 0)
		{
			SelectMotion(target, false);
			LogAppend(Fmt(L">> %s", WideCp(a.motions[target].name).c_str()));
			g_fx.ResetTrails();
			ResetMotionState();
			from = next = (float)g_loopStart;
		}
		else if (loop)
		{
			// Looping sequences would otherwise pile up, one per pass.
			g_fx.StopSequences(true);
			g_fx.ResetTrails();
			ResetScriptState();
			from = next = (float)g_loopStart;
		}
		else
		{
			SetFrame((float)g_loopEnd, true);
			SetPlaying(false);
			return;
		}
	}
	// Playing the whole timeline crosses motions without anyone selecting them.
	if (g_currentMotion < 0 && MotionAtFrame((int)next) != motionBefore)
		ResetMotionState();
	FireRange(from, next);
	if (g_pendingMotion >= 0)
	{
		SelectMotion(g_pendingMotion, false);
		g_pendingMotion = -1;
		g_fx.ResetTrails();
		ResetMotionState();
		next = (float)g_loopStart;
	}
	SetFrame(next, true);
}

// Effects keep running while the timeline is paused, as they do in the game.
static void EffectsTick()
{
	DWORD now = GetTickCount();
	float ms = (float)(now - g_fxTick);
	g_fxTick = now;
	AudioUpdate();
	if (!g_fx.Finished() && g_fxOn)
	{
		g_fx.Process(ms, g_doc.boneWorld);
		InvalidateRect(g_view, 0, FALSE);
	}
	for (; g_fxProblemsShown < g_fx.m_problems.size(); ++g_fxProblemsShown)
		LogAppend(Fmt(T(L"! 사용할 수 없음: %s", L"! Could not use: %s"), WideCp(g_fx.m_problems[g_fxProblemsShown]).c_str()));
}

// -- timeline -----------------------------------------------------------------

static const int TL_RULER = 18, TL_MOTION = 34, TL_SCRIPT = 12;

static int FrameAtX(int x)
{
	return g_scroll + (int)(x / g_zoom);
}

static void PaintTimeline(HDC dc, const RECT& rc)
{
	HBRUSH bg = CreateSolidBrush(RGB(212, 208, 200));
	FillRect(dc, &rc, bg);
	DeleteObject(bg);
	SetBkMode(dc, TRANSPARENT);
	HFONT oldFont = (HFONT)SelectObject(dc, g_fontSmall);

	// Ruler
	int step = 1;
	while (step * g_zoom < 50.0f)
		step = step < 5 ? 5 : (step < 10 ? 10 : step * 2 + (step == 10 ? 0 : 0));
	int first = g_scroll / step * step;
	HPEN tick = CreatePen(PS_SOLID, 1, RGB(90, 90, 90));
	HPEN oldPen = (HPEN)SelectObject(dc, tick);
	SetTextColor(dc, RGB(0, 0, 0));
	for (int f = first; ; f += step)
	{
		int x = (int)((f - g_scroll) * g_zoom);
		if (x > rc.right)
			break;
		MoveToEx(dc, x, 10, 0);
		LineTo(dc, x, TL_RULER);
		wchar_t num[16];
		swprintf_s(num, L"%d", f);
		TextOutW(dc, x + 2, 1, num, (int)wcslen(num));
	}
	MoveToEx(dc, 0, TL_RULER, 0);
	LineTo(dc, rc.right, TL_RULER);

	// Motion clips, staggered onto two rows so short neighbours stay readable
	const PetFile& a = g_doc.AnimSource();
	int rowH = TL_MOTION / 2;
	for (size_t i = 0; i < a.motions.size(); ++i)
	{
		const PetMotion& m = a.motions[i];
		int x0 = (int)(((int)m.startFrame - g_scroll) * g_zoom);
		int x1 = (int)(((int)m.endFrame - g_scroll) * g_zoom);
		if (x1 < x0 + 2)
			x1 = x0 + 2;
		if (x1 < 0 || x0 > rc.right)
			continue;
		int top = TL_RULER + 1 + (int)(i % 2) * rowH;
		RECT r = { x0, top, x1, top + rowH - 1 };
		bool current = (int)i == g_currentMotion;
		HBRUSH b = CreateSolidBrush(current ? RGB(255, 214, 90) : (i % 2 ? RGB(150, 180, 214) : RGB(174, 200, 228)));
		FillRect(dc, &r, b);
		DeleteObject(b);
		FrameRect(dc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
		if (x1 - x0 > 20)
		{
			std::wstring name = WideCp(m.name);
			RECT tr = r;
			tr.left += 3;
			if (tr.left < 3)
				tr.left = 3;
			DrawTextW(dc, name.c_str(), (int)name.size(), &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		}
	}

	// Scripts
	int y = TL_RULER + TL_MOTION + 2;
	HPEN scriptPen = CreatePen(PS_SOLID, 1, RGB(170, 40, 40));
	SelectObject(dc, scriptPen);
	for (size_t i = 0; i < a.frames.size(); ++i)
	{
		int x = (int)(((int)a.frames[i].frame - g_scroll) * g_zoom);
		if (x < 0 || x > rc.right)
			continue;
		MoveToEx(dc, x, y, 0);
		LineTo(dc, x, y + TL_SCRIPT);
	}

	// Playhead
	HPEN head = CreatePen(PS_SOLID, 1, RGB(220, 0, 0));
	SelectObject(dc, head);
	int px = (int)((g_frame - g_scroll) * g_zoom);
	MoveToEx(dc, px, 0, 0);
	LineTo(dc, px, rc.bottom);

	SelectObject(dc, oldPen);
	SelectObject(dc, oldFont);
	DeleteObject(tick);
	DeleteObject(scriptPen);
	DeleteObject(head);
}

static LRESULT CALLBACK TimelineProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(hwnd, &ps);
			RECT rc;
			GetClientRect(hwnd, &rc);
			HDC mem = CreateCompatibleDC(dc);
			HBITMAP bmp = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
			HBITMAP old = (HBITMAP)SelectObject(mem, bmp);
			PaintTimeline(mem, rc);
			BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
			SelectObject(mem, old);
			DeleteObject(bmp);
			DeleteDC(mem);
			EndPaint(hwnd, &ps);
			return 0;
		}
	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		SetFocus(hwnd);
		// fall through
	case WM_MOUSEMOVE:
		if ((msg == WM_LBUTTONDOWN || (wp & MK_LBUTTON)) && GetCapture() == hwnd)
		{
			int frame = FrameAtX(GET_X_LPARAM(lp));
			if (frame < 0)
				frame = 0;
			if (msg == WM_LBUTTONDOWN && GET_Y_LPARAM(lp) >= TL_RULER && GET_Y_LPARAM(lp) < TL_RULER + TL_MOTION)
				SelectMotion(MotionAtFrame(frame), false);
			SetFrame((float)frame, false);
		}
		return 0;
	case WM_LBUTTONUP:
		ReleaseCapture();
		return 0;
	case WM_MOUSEWHEEL:
		{
			POINT pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
			ScreenToClient(hwnd, &pt);
			if (GET_KEYSTATE_WPARAM(wp) & MK_SHIFT)
			{
				g_scroll -= (int)(GET_WHEEL_DELTA_WPARAM(wp) / 120 * 60 / g_zoom);
			}
			else
			{
				int anchor = FrameAtX(pt.x);
				g_zoom *= GET_WHEEL_DELTA_WPARAM(wp) > 0 ? 1.3f : 0.77f;
				if (g_zoom < 0.05f) g_zoom = 0.05f;
				if (g_zoom > 24.0f) g_zoom = 24.0f;
				g_scroll = anchor - (int)(pt.x / g_zoom);
			}
			if (g_scroll < 0)
				g_scroll = 0;
			InvalidateRect(hwnd, 0, FALSE);
			return 0;
		}
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

// -- tree -----------------------------------------------------------------------

static HTREEITEM g_treeParts;

static HTREEITEM TreeAdd(HTREEITEM parent, const std::wstring& text, LPARAM data)
{
	TVINSERTSTRUCTW ins;
	memset(&ins, 0, sizeof(ins));
	ins.hParent = parent;
	ins.hInsertAfter = TVI_LAST;
	ins.item.mask = TVIF_TEXT | TVIF_PARAM;
	ins.item.pszText = const_cast<wchar_t*>(text.c_str());
	ins.item.lParam = data;
	return (HTREEITEM)SendMessageW(g_tree, TVM_INSERTITEMW, 0, (LPARAM)&ins);
}

static void RefreshTree()
{
	SendMessageW(g_tree, WM_SETREDRAW, FALSE, 0);
	TreeView_DeleteAllItems(g_tree);
	std::wstring title = g_doc.sourcePath.empty() ? std::wstring(T(L"(새 프로젝트)", L"(no project)")) : Wide(PathBaseName(g_doc.sourcePath));
	HTREEITEM root = TreeAdd(TVI_ROOT, title, MakeNode(N_FILE, 0));

	HTREEITEM tex = TreeAdd(root, Fmt(L"%s (%d)", T(L"텍스쳐", L"Textures"), g_doc.BlockCount(PMP_TEXTURE)), MakeNode(N_GROUP, 0));
	for (int i = 0; i < g_doc.BlockCount(PMP_TEXTURE); ++i)
		TreeAdd(tex, PathBaseName(Wide(g_doc.BlockAt(PMP_TEXTURE, i)->Get("텍스쳐파일")).empty() ? std::string() : g_doc.BlockAt(PMP_TEXTURE, i)->Get("텍스쳐파일")) == std::string() ? L"?" : Wide(PathBaseName(g_doc.BlockAt(PMP_TEXTURE, i)->Get("텍스쳐파일"))), MakeNode(N_TEXTURE, i));

	const std::vector<PetBone>& bones = g_doc.Skeleton();
	HTREEITEM boneRoot = TreeAdd(root, Fmt(L"%s (%d)", T(L"본", L"Bones"), (int)(g_doc.HasSkeleton() ? bones.size() : 0)), MakeNode(N_GROUP, 1));
	if (g_doc.HasSkeleton())
	{
		std::vector<HTREEITEM> items(bones.size());
		for (size_t i = 0; i < bones.size(); ++i)
		{
			HTREEITEM parent = bones[i].parent >= 0 && bones[i].parent < (int)i ? items[bones[i].parent] : boneRoot;
			items[i] = TreeAdd(parent, WideCp(bones[i].name), MakeNode(N_BONE, (int)i));
		}
	}

	HTREEITEM mot = TreeAdd(root, Fmt(L"%s (%d)", T(L"동작", L"Motions"), g_doc.BlockCount(PMP_MOTION)), MakeNode(N_GROUP, 2));
	for (int i = 0; i < g_doc.BlockCount(PMP_MOTION); ++i)
	{
		const PmpBlock* b = g_doc.BlockAt(PMP_MOTION, i);
		TreeAdd(mot, Wide(b->Get("동작이름") + "  [" + b->Get("시작프레임") + "-" + b->Get("종료프레임") + "]"), MakeNode(N_MOTION, i));
	}

	HTREEITEM scr = TreeAdd(root, Fmt(L"%s (%d)", T(L"스크립트", L"Scripts"), g_doc.BlockCount(PMP_SCRIPT)), MakeNode(N_GROUP, 3));
	for (int i = 0; i < g_doc.BlockCount(PMP_SCRIPT); ++i)
	{
		const PmpBlock* b = g_doc.BlockAt(PMP_SCRIPT, i);
		TreeAdd(scr, Wide(b->arg + "  " + b->Get("메시지")), MakeNode(N_SCRIPT, i));
	}

	HTREEITEM box = TreeAdd(root, Fmt(L"%s (%d)", T(L"충돌박스", L"Collision boxes"), g_doc.BlockCount(PMP_BOX)), MakeNode(N_GROUP, 4));
	for (int i = 0; i < g_doc.BlockCount(PMP_BOX); ++i)
		TreeAdd(box, Wide(g_doc.BlockAt(PMP_BOX, i)->Get("박스이름")), MakeNode(N_BOX, i));

	HTREEITEM face = TreeAdd(root, Fmt(L"%s (%d)", T(L"표정", L"Face animations"), (int)g_doc.pet.faces.size()), MakeNode(N_GROUP, 5));
	for (size_t i = 0; i < g_doc.pet.faces.size(); ++i)
		TreeAdd(face, WideCp(g_doc.pet.faces[i].Name()), MakeNode(N_FACE, (int)i));

	HTREEITEM part = TreeAdd(root, Fmt(L"%s (%d)", T(L"파츠", L"Mesh parts"), (int)g_doc.parts.size()), MakeNode(N_GROUP, 6));
	g_treeParts = part;
	for (size_t i = 0; i < g_doc.parts.size(); ++i)
		TreeAdd(part, Wide(g_doc.parts[i]->name), MakeNode(N_PART, (int)i));

	HTREEITEM chunks = TreeAdd(root, T(L"청크", L"Chunks"), MakeNode(N_GROUP, 7));
	for (size_t i = 0; i < g_doc.pet.chunks.size(); ++i)
		TreeAdd(chunks, g_doc.pet.chunks[i].id[0] ? Wide(g_doc.pet.chunks[i].id) : std::wstring(T(L"(읽을 수 없는 끝부분)", L"(undecodable tail)")), MakeNode(N_CHUNK, (int)i));

	TreeView_Expand(g_tree, root, TVE_EXPAND);
	TreeView_Expand(g_tree, tex, TVE_EXPAND);
	TreeView_Expand(g_tree, mot, g_doc.BlockCount(PMP_MOTION) < 200 ? TVE_EXPAND : TVE_COLLAPSE);
	TreeView_Expand(g_tree, box, TVE_COLLAPSE);
	TreeView_EnsureVisible(g_tree, root);
	SendMessageW(g_tree, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(g_tree, 0, TRUE);
}

// The wardrobe changes only this branch, so the rest of the tree keeps its state.
static void RefreshTreeParts()
{
	if (!g_treeParts)
		return;
	if (NodeKindOf(g_selection) == N_PART)
		g_selection = MakeNode(N_FILE, 0);
	HTREEITEM child;
	while ((child = TreeView_GetChild(g_tree, g_treeParts)) != 0)
		TreeView_DeleteItem(g_tree, child);
	for (size_t i = 0; i < g_doc.parts.size(); ++i)
		TreeAdd(g_treeParts, Wide(g_doc.parts[i]->name), MakeNode(N_PART, (int)i));
	std::wstring label = Fmt(L"%s (%d)", T(L"파츠", L"Mesh parts"), (int)g_doc.parts.size());
	TVITEMW it;
	memset(&it, 0, sizeof(it));
	it.mask = TVIF_TEXT;
	it.hItem = g_treeParts;
	it.pszText = const_cast<wchar_t*>(label.c_str());
	SendMessageW(g_tree, TVM_SETITEMW, 0, (LPARAM)&it);
}

// -- properties ------------------------------------------------------------------

static void PropAdd(const std::wstring& label, const std::wstring& value, int field = -1)
{
	PropRow r;
	r.label = label;
	r.value = value;
	r.field = field;
	r.flag[0] = 0;
	g_rows.push_back(r);
}

static bool HasPrefix(const std::string& file, const char* flag)
{
	if (!strcmp(flag, "2#"))
		return file.find("2#") != std::string::npos;
	return NameHasFlag(file, flag);
}

static void AddTextureRows(const PmpBlock& b)
{
	std::string file = PathBaseName(b.Get("텍스쳐파일"));
	std::string resolved = g_doc.ResolveTexture(file);
	PropAdd(T(L"찾은 경로", L"Resolved path"), resolved.empty() ? T(L"(찾을 수 없음)", L"(not found)") : Wide(resolved));
	static const struct { const char* f; const wchar_t* w; const wchar_t* ko; const wchar_t* en; } flags[] = {
		{ "[", L"[", L"[  알파 (0x400000)", L"[  alpha blend (0x400000)" },
		{ "+", L"+", L"+  알파 + 항상 밝게 (0x10)", L"+  alpha, full bright (0x10)" },
		{ "]", L"]", L"]  알파테스트 + 앰비언트만 (4)", L"]  alpha test, ambient only (4)" },
		{ "{", L"{", L"{  알파테스트", L"{  alpha test" },
		{ "~", L"~", L"~  0x20800000 + 알파", L"~  0x20800000 + alpha" },
		{ "}", L"}", L"}  0x21800000 + 알파", L"}  0x21800000 + alpha" },
		{ "@", L"@", L"@  맵 타입 1", L"@  map type 1" },
		{ "!", L"!", L"!  스페큘러 맵 (타입 3)", L"!  specular map (type 3)" },
		{ "2#", L"2#", L"2#  양면 조명 (0x400)", L"2#  two-sided lighting (0x400)" },
	};
	for (size_t i = 0; i < sizeof(flags) / sizeof(flags[0]); ++i)
	{
		PropRow r;
		r.label = g_korean ? flags[i].ko : flags[i].en;
		r.value = HasPrefix(file, flags[i].f) ? L"●" : L"";
		r.field = -2;
		wcscpy_s(r.flag, flags[i].w);
		g_rows.push_back(r);
	}
}

static void BlockToRows(const PmpBlock& b)
{
	if (b.kind == PMP_SCRIPT)
		PropAdd(T(L"프레임", L"Frame"), Wide(b.arg), -3);
	for (size_t i = 0; i < b.fields.size(); ++i)
	{
		const wchar_t* en = FieldEnglish(b.fields[i].key);
		std::wstring label = g_korean || !en ? Wide(b.fields[i].key) : std::wstring(en);
		PropAdd(label, Wide(b.fields[i].value), (int)i);
	}
}

static const wchar_t* KindName(PetKind k)
{
	switch (k)
	{
	case PET_MESH: return T(L".mpet 메시 파츠", L".mpet mesh part");
	case PET_BONE: return T(L".bpet 뼈대", L".bpet skeleton");
	case PET_ANIM: return T(L".apet 애니메이션", L".apet animation");
	default: return T(L".pet 전체", L".pet full");
	}
}

static void RefreshProps()
{
	if (g_editor)
		DestroyWindow(g_editor);
	g_rows.clear();
	NodeKind kind = NodeKindOf(g_selection);
	int idx = NodeIndexOf(g_selection);
	const PetFile& pet = g_doc.pet;
	switch (kind)
	{
	case N_FILE:
		PropAdd(T(L"파일", L"File"), Wide(g_doc.sourcePath));
		PropAdd(T(L"종류", L"Type"), KindName(pet.kind));
		PropAdd(T(L"버전", L"Puppet version"), Fmt(L"%d.%d", pet.verMajor, pet.verMinor));
		PropAdd(T(L"PMP 버전", L"PMPF version"), Wide(g_doc.pmp.version));
		PropAdd(T(L"ASE 이름", L"ASE name"), Wide(g_doc.pmp.aseName));
		PropAdd(T(L"PMP 인코딩", L"PMP encoding"), g_doc.pmp.cp949 ? L"CP949" : L"UTF-8");
		PropAdd(T(L"정점 / 삼각형", L"Vertices / triangles"), Fmt(L"%d / %d", (int)pet.vertices.size(), (int)pet.points.size() / 3));
		PropAdd(T(L"애니메이션 트랙", L"Animation tracks"), Fmt(L"%d", (int)g_doc.AnimSource().anims.size()));
		break;
	case N_TEXTURE:
		if (const PmpBlock* b = g_doc.BlockAt(PMP_TEXTURE, idx))
		{
			BlockToRows(*b);
			if (idx < (int)pet.textures.size())
			{
				const PetTexture& t = pet.textures[idx];
				PropAdd(T(L"그룹", L"Group"), Fmt(L"%d", t.group));
				PropAdd(T(L"디퓨즈", L"Diffuse"), Fmt(L"%08X", t.diffuse));
			}
			AddTextureRows(*b);
		}
		break;
	case N_MOTION:
		if (const PmpBlock* b = g_doc.BlockAt(PMP_MOTION, idx))
			BlockToRows(*b);
		break;
	case N_SCRIPT:
		if (const PmpBlock* b = g_doc.BlockAt(PMP_SCRIPT, idx))
			BlockToRows(*b);
		break;
	case N_BOX:
		if (const PmpBlock* b = g_doc.BlockAt(PMP_BOX, idx))
			BlockToRows(*b);
		break;
	case N_BONE:
		if (g_doc.HasSkeleton() && idx < (int)g_doc.Skeleton().size())
		{
			const PetBone& b = g_doc.Skeleton()[idx];
			PropAdd(T(L"이름", L"Name"), WideCp(b.name));
			PropAdd(T(L"번호", L"ID"), Fmt(L"%d", idx));
			PropAdd(T(L"부모", L"Parent"), b.parent >= 0 ? WideCp(g_doc.Skeleton()[b.parent].name) : L"-");
			PropAdd(L"X", Fmt(L"%.4f  %.4f  %.4f", b.mat.xx, b.mat.yx, b.mat.zx));
			PropAdd(L"Y", Fmt(L"%.4f  %.4f  %.4f", b.mat.xy, b.mat.yy, b.mat.zy));
			PropAdd(L"Z", Fmt(L"%.4f  %.4f  %.4f", b.mat.xz, b.mat.yz, b.mat.zz));
			PropAdd(T(L"위치", L"Pivot"), Fmt(L"%.4f  %.4f  %.4f", b.mat.xm, b.mat.ym, b.mat.zm));
			const PetFile& a = g_doc.AnimSource();
			for (size_t i = 0; i < a.anims.size(); ++i)
			{
				if (a.anims[i].bone < (int)a.bones.size() && a.bones[a.anims[i].bone].name == b.name)
				{
					PropAdd(T(L"위치 키", L"Position keys"), Fmt(L"%d", (int)a.anims[i].pos.size()));
					PropAdd(T(L"회전 키", L"Rotation keys"), Fmt(L"%d", (int)a.anims[i].rot.size()));
					PropAdd(T(L"크기 키", L"Scale keys"), Fmt(L"%d", (int)a.anims[i].scale.size()));
				}
			}
		}
		break;
	case N_FACE:
		if (idx < (int)pet.faces.size())
		{
			PropAdd(T(L"그룹", L"Group"), Fmt(L"%d", pet.faces[idx].group));
			PropAdd(T(L"이름", L"Name"), WideCp(pet.faces[idx].Name()));
			PropAdd(T(L"파일", L"File"), WideCp(pet.faces[idx].File()));
		}
		break;
	case N_PART:
		if (idx < (int)g_doc.parts.size())
		{
			const MeshPart& p = *g_doc.parts[idx];
			PropAdd(T(L"이름", L"Name"), Wide(p.name));
			PropAdd(T(L"정점", L"Vertices"), Fmt(L"%d", (int)p.pet->vertices.size()));
			PropAdd(T(L"삼각형", L"Triangles"), Fmt(L"%d", (int)p.pet->points.size() / 3));
			PropAdd(T(L"스킨 조각", L"Skin pieces"), Fmt(L"%d", (int)p.pet->skins.size()));
			PropAdd(T(L"표시 (더블클릭)", L"Visible (double-click)"), p.visible ? L"●" : L"", -4);
		}
		break;
	case N_CHUNK:
		if (idx < (int)pet.chunks.size())
		{
			ByteBuf out;
			pet.SaveBuffer(&out);
			size_t pos = 0;
			for (int i = 0; i < (int)pet.chunks.size() && pos + 8 <= out.size(); ++i)
			{
				if (!pet.chunks[i].id[0])
					break;
				u32 len;
				memcpy(&len, &out[pos + 4], 4);
				if (i == idx)
				{
					PropAdd(T(L"청크", L"Chunk"), Wide(pet.chunks[i].id));
					PropAdd(T(L"오프셋", L"Offset"), Fmt(L"0x%X", (int)pos));
					PropAdd(T(L"크기", L"Size"), Fmt(L"%u", len));
					break;
				}
				pos += 8 + len;
			}
			if (!pet.chunks[idx].id[0])
				PropAdd(T(L"크기", L"Size"), Fmt(L"%d", (int)pet.chunks[idx].raw.size()));
		}
		break;
	default:
		break;
	}
	ListView_DeleteAllItems(g_props);
	LVITEMW it;
	memset(&it, 0, sizeof(it));
	it.mask = LVIF_TEXT;
	for (size_t i = 0; i < g_rows.size(); ++i)
	{
		it.iItem = (int)i;
		it.iSubItem = 0;
		it.pszText = const_cast<wchar_t*>(g_rows[i].label.c_str());
		SendMessageW(g_props, LVM_INSERTITEMW, 0, (LPARAM)&it);
		it.iSubItem = 1;
		it.pszText = const_cast<wchar_t*>(g_rows[i].value.c_str());
		SendMessageW(g_props, LVM_SETITEMTEXTW, i, (LPARAM)&it);
	}
}

static PmpBlock* SelectedBlock()
{
	int idx = NodeIndexOf(g_selection);
	switch (NodeKindOf(g_selection))
	{
	case N_TEXTURE: return g_doc.BlockAt(PMP_TEXTURE, idx);
	case N_MOTION: return g_doc.BlockAt(PMP_MOTION, idx);
	case N_SCRIPT: return g_doc.BlockAt(PMP_SCRIPT, idx);
	case N_BOX: return g_doc.BlockAt(PMP_BOX, idx);
	default: return 0;
	}
}

static void TitleUpdate()
{
	std::wstring name = g_doc.sourcePath.empty() ? std::wstring(T(L"제목없음", L"Untitled")) : Wide(PathBaseName(g_doc.sourcePath));
	SetWindowTextW(g_main, (name + (g_doc.dirty ? L" *" : L"") + L" - CaddieRig").c_str());
}

static void BlockEdited(PmpBlock* b)
{
	PmpBlockKind kind = b->kind;
	g_doc.ApplyBlockKind(kind);
	if (kind == PMP_BOX)
		StartBoxEffects();
	if (kind == PMP_MOTION || kind == PMP_SCRIPT)
		InvalidateRect(g_timeline, 0, FALSE);
	ApplyPose();
	TitleUpdate();
	// Keep the tree label in step without rebuilding the whole thing.
	HTREEITEM sel = TreeView_GetSelection(g_tree);
	if (sel)
	{
		std::wstring label;
		if (kind == PMP_BOX)
			label = Wide(b->Get("박스이름"));
		else if (kind == PMP_MOTION)
			label = Wide(b->Get("동작이름") + "  [" + b->Get("시작프레임") + "-" + b->Get("종료프레임") + "]");
		else if (kind == PMP_SCRIPT)
			label = Wide(b->arg + "  " + b->Get("메시지"));
		else
			label = Wide(PathBaseName(b->Get("텍스쳐파일")));
		TVITEMW it;
		memset(&it, 0, sizeof(it));
		it.mask = TVIF_TEXT;
		it.hItem = sel;
		it.pszText = const_cast<wchar_t*>(label.c_str());
		SendMessageW(g_tree, TVM_SETITEMW, 0, (LPARAM)&it);
	}
}

static void CommitEditor(bool apply)
{
	if (!g_editor)
		return;
	HWND ed = g_editor;
	g_editor = 0;
	if (apply && g_editorRow >= 0 && g_editorRow < (int)g_rows.size())
	{
		int len = GetWindowTextLengthW(ed);
		std::wstring text(len, L' ');
		if (len)
			GetWindowTextW(ed, &text[0], len + 1);
		PmpBlock* b = SelectedBlock();
		const PropRow& row = g_rows[g_editorRow];
		if (b && text != row.value)
		{
			if (row.field >= 0 && row.field < (int)b->fields.size())
				b->fields[row.field].value = Narrow(text);
			else if (row.field == -3)
			{
				wchar_t num[16];
				swprintf_s(num, L"%04d", _wtoi(text.c_str()));
				b->arg = Narrow(num);
			}
			BlockEdited(b);
		}
	}
	DestroyWindow(ed);
	g_editor = 0;
	RefreshProps();
}

static LRESULT CALLBACK EditorProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_KEYDOWN && wp == VK_RETURN)
	{
		CommitEditor(true);
		return 0;
	}
	if (msg == WM_KEYDOWN && wp == VK_ESCAPE)
	{
		CommitEditor(false);
		return 0;
	}
	if (msg == WM_KILLFOCUS)
	{
		CommitEditor(true);
		return 0;
	}
	return CallWindowProcW(g_editorProc, hwnd, msg, wp, lp);
}

static void ToggleTextureFlag(const wchar_t* flag)
{
	PmpBlock* b = SelectedBlock();
	if (!b)
		return;
	std::string path = b->Get("텍스쳐파일");
	std::string dir = PathDir(path);
	std::string file = PathBaseName(path);
	std::string f = Narrow(flag);
	size_t at = file.find(f);
	if (at != std::string::npos && (f == "2#" || at < 3))
		file.erase(at, f.size());
	else if (f == "2#")
	{
		// goes after the single character flags, as in "[2#i_hair_01-03.jpg"
		size_t ins = 0;
		while (ins < file.size() && strchr("[]{}~+@!", file[ins]))
			++ins;
		file.insert(ins, f);
	}
	else
		file = f + file;
	b->Set("텍스쳐파일", dir.empty() ? file : dir + "/" + file);
	BlockEdited(b);
	RefreshProps();
}

static void PropsDoubleClick(int row)
{
	if (row < 0 || row >= (int)g_rows.size())
		return;
	const PropRow& r = g_rows[row];
	if (r.field == -2)
	{
		ToggleTextureFlag(r.flag);
		return;
	}
	if (r.field == -4)
	{
		int idx = NodeIndexOf(g_selection);
		if (idx < (int)g_doc.parts.size())
		{
			g_doc.parts[idx]->visible = !g_doc.parts[idx]->visible;
			ApplyPose();
			RefreshProps();
		}
		return;
	}
	if (r.field == -1)
		return;
	RECT rc;
	ListView_GetSubItemRect(g_props, row, 1, LVIR_BOUNDS, &rc);
	g_editorRow = row;
	g_editor = CreateWindowExW(0, L"EDIT", r.value.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top + 2, g_props, 0, g_inst, 0);
	SendMessageW(g_editor, WM_SETFONT, (WPARAM)g_font, TRUE);
	g_editorProc = (WNDPROC)SetWindowLongPtrW(g_editor, GWLP_WNDPROC, (LONG_PTR)EditorProc);
	SendMessageW(g_editor, EM_SETSEL, 0, -1);
	SetFocus(g_editor);
}

static void OnTreeSelect(LPARAM data)
{
	g_selection = data;
	NodeKind kind = NodeKindOf(data);
	int idx = NodeIndexOf(data);
	g_opts.selectedBone = kind == N_BONE ? idx : -1;
	g_opts.selectedBox = kind == N_BOX ? idx : -1;
	g_opts.selectedFace = kind == N_FACE ? idx : -1;
	if (kind == N_MOTION)
		SelectMotion(idx, true);
	else if (kind == N_SCRIPT)
	{
		const PmpBlock* b = g_doc.BlockAt(PMP_SCRIPT, idx);
		if (b)
		{
			int frame = atoi(b->arg.c_str());
			if (frame >= 0)
			{
				SelectMotion(MotionAtFrame(frame), false);
				SetFrame((float)frame, true);
			}
		}
	}
	RefreshProps();
	InvalidateRect(g_view, 0, FALSE);
}

// -- wardrobe -----------------------------------------------------------------------

static Wardrobe g_wardrobe;
static HWND g_wardWnd, g_wardExtras, g_wardReset;
static bool g_wardFilling;
static const int WARD_ROWS = 8;
static const int WARD_ROW_H = 26, WARD_W = 560;

struct WardrobeRow
{
	HWND label, part, colour, status, show;
};
static WardrobeRow g_wardRow[WARD_ROWS];

enum { IDW_PART = 2000, IDW_COLOUR = 2100, IDW_SHOW = 2200, IDW_EXTRAS = 2300 };

static const wchar_t* SlotLabel(const std::string& slot)
{
	if (slot == "ts") return T(L"상의", L"Torso");
	if (slot == "pv") return T(L"하의", L"Pelvis");
	if (slot == "am") return T(L"팔", L"Arms");
	if (slot == "lg") return T(L"다리", L"Legs");
	if (slot == "hn") return T(L"손", L"Hands");
	if (slot == "ft") return T(L"신발", L"Feet");
	if (slot == "ha") return T(L"머리", L"Hair");
	if (slot == "fc") return T(L"얼굴", L"Face");
	return L"";
}

static MeshPart* WornPart(int item)
{
	if (item < 0)
		return 0;
	for (size_t p = 0; p < g_doc.parts.size(); ++p)
		if (!_stricmp(g_doc.parts[p]->name.c_str(), g_wardrobe.items[item].file.c_str()))
			return g_doc.parts[p];
	return 0;
}

// Colour numbers with files on disk for any of the part's numbered textures. Every row
// asks again on each change, so the answers stay until the next WardrobeFill.
static std::map<std::string, std::vector<std::string> > g_partVariants;

static const std::vector<std::string>& PartVariants(const MeshPart& part)
{
	std::map<std::string, std::vector<std::string> >::iterator known = g_partVariants.find(part.name);
	if (known != g_partVariants.end())
		return known->second;
	std::vector<std::string>& out = g_partVariants[part.name];
	for (int n = 1; n <= 40; ++n)
	{
		char num[8];
		sprintf_s(num, "%02d", n);
		for (size_t t = 0; t < part.pet->textures.size(); ++t)
		{
			std::string name = TextureVariantName(part.pet->textures[t].Name(), num);
			if (!name.empty() && !g_doc.ResolveAsset(name).empty())
			{
				out.push_back(num);
				break;
			}
		}
	}
	return out;
}

static void ApplyVariant(MeshPart* part, const std::string& variant)
{
	std::vector<std::string> paths(part->pet->textures.size());
	for (size_t t = 0; t < paths.size() && !variant.empty(); ++t)
	{
		std::string name = TextureVariantName(part->pet->textures[t].Name(), variant);
		if (!name.empty())
			paths[t] = g_doc.ResolveAsset(name);
	}
	ViewSetPartVariant(part, paths);
}

static void WardrobeUpdateRows()
{
	for (int r = 0; r < WARD_ROWS && r < (int)g_wardrobe.slots.size(); ++r)
	{
		const WardrobeSlot& s = g_wardrobe.slots[r];
		int by = g_wardrobe.HiddenBy(r);
		std::wstring text;
		if (by >= 0)
			text = Fmt(T(L"%s 때문에 숨김", L"hidden by %s"), Wide(g_wardrobe.items[by].slot + "_" + g_wardrobe.items[by].id).c_str());
		else if (s.selected < 0)
			text = T(L"비어 있음", L"empty");
		SetWindowTextW(g_wardRow[r].status, text.c_str());
		SendMessageW(g_wardRow[r].show, BM_SETCHECK, s.forceShow ? BST_CHECKED : BST_UNCHECKED, 0);
		EnableWindow(g_wardRow[r].show, by >= 0);

		SendMessageW(g_wardRow[r].colour, CB_RESETCONTENT, 0, 0);
		SendMessageW(g_wardRow[r].colour, CB_ADDSTRING, 0, (LPARAM)T(L"원본", L"Orig"));
		SendMessageW(g_wardRow[r].colour, CB_SETCURSEL, 0, 0);
		MeshPart* part = WornPart(s.selected);
		if (part)
		{
			const std::vector<std::string>& variants = PartVariants(*part);
			for (size_t v = 0; v < variants.size(); ++v)
			{
				SendMessageW(g_wardRow[r].colour, CB_ADDSTRING, 0, (LPARAM)Wide(variants[v]).c_str());
				if (variants[v] == s.variant)
					SendMessageW(g_wardRow[r].colour, CB_SETCURSEL, v + 1, 0);
			}
		}
		EnableWindow(g_wardRow[r].colour, part != 0 && SendMessageW(g_wardRow[r].colour, CB_GETCOUNT, 0, 0) > 1);
	}
}

// The controls follow changes made elsewhere: /wear, Attach, Reset.
static void WardrobeShowSelection()
{
	g_wardFilling = true;
	for (int r = 0; r < WARD_ROWS && r < (int)g_wardrobe.slots.size(); ++r)
	{
		const WardrobeSlot& s = g_wardrobe.slots[r];
		int sel = 0;
		for (size_t c = 0; c < s.choices.size(); ++c)
			if (s.choices[c] == s.selected)
				sel = (int)c + 1;
		SendMessageW(g_wardRow[r].part, CB_SETCURSEL, sel, 0);
	}
	int rows = ListView_GetItemCount(g_wardExtras);
	for (int i = 0; i < rows; ++i)
	{
		LVITEMW it;
		memset(&it, 0, sizeof(it));
		it.mask = LVIF_PARAM;
		it.iItem = i;
		SendMessageW(g_wardExtras, LVM_GETITEMW, 0, (LPARAM)&it);
		bool on = g_wardrobe.extraOn[it.lParam];
		if ((ListView_GetCheckState(g_wardExtras, i) != 0) != on)
			ListView_SetCheckState(g_wardExtras, i, on);
	}
	g_wardFilling = false;
}

// Parts changed: match the document, then redo what scripts did to them.
static void WardrobeApplied()
{
	g_wardrobe.Sync(&g_doc, ViewForgetPart);
	RefreshTreeParts();
	for (size_t s = 0; s < g_wardrobe.slots.size(); ++s)
	{
		MeshPart* part = WornPart(g_wardrobe.slots[s].selected);
		if (part && !g_wardrobe.slots[s].variant.empty() && part->textureOverride.empty())
			ApplyVariant(part, g_wardrobe.slots[s].variant);
	}
	if (g_wardWnd)
	{
		WardrobeShowSelection();
		WardrobeUpdateRows();
	}
	g_doc.Pose(g_frame / 30.0f);
	int m = g_currentMotion >= 0 ? g_currentMotion : MotionAtFrame((int)g_frame);
	ReplayScriptState(m, (int)g_frame);
	InvalidateRect(g_view, 0, FALSE);
	UpdateStatus();
}

static void WardrobeFill()
{
	g_partVariants.clear();
	g_wardFilling = true;
	for (int r = 0; r < WARD_ROWS && r < (int)g_wardrobe.slots.size(); ++r)
	{
		const WardrobeSlot& s = g_wardrobe.slots[r];
		SetWindowTextW(g_wardRow[r].label, SlotLabel(s.slot));
		SendMessageW(g_wardRow[r].part, CB_RESETCONTENT, 0, 0);
		SendMessageW(g_wardRow[r].part, CB_ADDSTRING, 0, (LPARAM)T(L"(없음)", L"(none)"));
		for (size_t c = 0; c < s.choices.size(); ++c)
			SendMessageW(g_wardRow[r].part, CB_ADDSTRING, 0, (LPARAM)Wide(g_wardrobe.items[s.choices[c]].file).c_str());
	}
	ListView_DeleteAllItems(g_wardExtras);
	int row = 0;
	LVITEMW it;
	memset(&it, 0, sizeof(it));
	for (size_t i = 0; i < g_wardrobe.items.size(); ++i)
	{
		const WardrobeItem& item = g_wardrobe.items[i];
		if (!item.extra)
			continue;
		std::wstring file = Wide(item.file), slot = Wide(item.slot);
		it.mask = LVIF_TEXT | LVIF_PARAM;
		it.iItem = row;
		it.iSubItem = 0;
		it.lParam = (LPARAM)i;
		it.pszText = const_cast<wchar_t*>(file.c_str());
		SendMessageW(g_wardExtras, LVM_INSERTITEMW, 0, (LPARAM)&it);
		it.mask = LVIF_TEXT;
		it.iSubItem = 1;
		it.pszText = const_cast<wchar_t*>(slot.c_str());
		SendMessageW(g_wardExtras, LVM_SETITEMTEXTW, row, (LPARAM)&it);
		++row;
	}
	WardrobeShowSelection();
	WardrobeUpdateRows();
}

// Rebuilds labels for the current language.
static void WardrobeRebuild()
{
	if (!g_wardWnd)
		return;
	SetWindowTextW(g_wardWnd, Fmt(L"%s - %s", T(L"옷장", L"Wardrobe"), Wide(PathBaseName(g_doc.baseDir)).c_str()).c_str());
	SetWindowTextW(g_wardReset, T(L"기본으로", L"Reset to def"));
	for (int r = 0; r < WARD_ROWS; ++r)
		SetWindowTextW(g_wardRow[r].show, T(L"표시", L"Show"));
	LVCOLUMNW col;
	col.mask = LVCF_TEXT;
	col.pszText = const_cast<wchar_t*>(T(L"추가 파츠", L"Extras"));
	SendMessageW(g_wardExtras, LVM_SETCOLUMNW, 0, (LPARAM)&col);
	col.pszText = const_cast<wchar_t*>(T(L"슬롯", L"Slot"));
	SendMessageW(g_wardExtras, LVM_SETCOLUMNW, 1, (LPARAM)&col);
	WardrobeFill();
}

static LRESULT CALLBACK WardrobeProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_COMMAND:
		{
			int id = LOWORD(wp), code = HIWORD(wp);
			if (g_wardFilling)
				return 0;
			if (id == ID_WARD_RESET)
			{
				for (size_t s = 0; s < g_wardrobe.slots.size(); ++s)
				{
					MeshPart* part = WornPart(g_wardrobe.slots[s].selected);
					if (part && !part->textureOverride.empty())
						ApplyVariant(part, std::string());
				}
				g_wardrobe.ResetToDefaults();
				WardrobeApplied();
			}
			else if (id >= IDW_PART && id < IDW_PART + WARD_ROWS && code == CBN_SELCHANGE)
			{
				WardrobeSlot& s = g_wardrobe.slots[id - IDW_PART];
				int sel = (int)SendMessageW((HWND)lp, CB_GETCURSEL, 0, 0);
				s.selected = sel <= 0 ? -1 : s.choices[sel - 1];
				s.variant.clear();
				WardrobeApplied();
			}
			else if (id >= IDW_COLOUR && id < IDW_COLOUR + WARD_ROWS && code == CBN_SELCHANGE)
			{
				WardrobeSlot& s = g_wardrobe.slots[id - IDW_COLOUR];
				int sel = (int)SendMessageW((HWND)lp, CB_GETCURSEL, 0, 0);
				wchar_t text[16] = L"";
				if (sel > 0)
					SendMessageW((HWND)lp, CB_GETLBTEXT, sel, (LPARAM)text);
				s.variant = sel > 0 ? Narrow(text) : std::string();
				MeshPart* part = WornPart(s.selected);
				if (part)
					ApplyVariant(part, s.variant);
			}
			else if (id >= IDW_SHOW && id < IDW_SHOW + WARD_ROWS && code == BN_CLICKED)
			{
				g_wardrobe.slots[id - IDW_SHOW].forceShow = SendMessageW((HWND)lp, BM_GETCHECK, 0, 0) == BST_CHECKED;
				WardrobeApplied();
			}
			return 0;
		}
	case WM_NOTIFY:
		{
			NMHDR* hdr = (NMHDR*)lp;
			if (hdr->idFrom == IDW_EXTRAS && hdr->code == LVN_ITEMCHANGED && !g_wardFilling)
			{
				NMLISTVIEW* nm = (NMLISTVIEW*)lp;
				if ((nm->uChanged & LVIF_STATE) && ((nm->uNewState ^ nm->uOldState) & LVIS_STATEIMAGEMASK))
				{
					g_wardrobe.extraOn[nm->lParam] = ListView_GetCheckState(g_wardExtras, nm->iItem) != 0;
					WardrobeApplied();
				}
			}
			return 0;
		}
	case WM_CLOSE:
		ShowWindow(hwnd, SW_HIDE);
		SyncChecks();
		return 0;
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

static void WardrobeCreate()
{
	RECT owner;
	GetWindowRect(g_main, &owner);
	int h = WARD_ROWS * WARD_ROW_H + 300;
	g_wardWnd = CreateWindowExW(WS_EX_TOOLWINDOW, L"CaddieRig_Wardrobe", L"", WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN,
		owner.right - WARD_W - 24, owner.top + 90, WARD_W, h, g_main, 0, g_inst, 0);
	for (int r = 0; r < WARD_ROWS; ++r)
	{
		int y = 8 + r * WARD_ROW_H;
		WardrobeRow& row = g_wardRow[r];
		row.label = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, 8, y + 4, 46, 18, g_wardWnd, 0, g_inst, 0);
		row.part = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST, 56, y, 220, 320, g_wardWnd, (HMENU)(INT_PTR)(IDW_PART + r), g_inst, 0);
		row.colour = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST, 280, y, 66, 240, g_wardWnd, (HMENU)(INT_PTR)(IDW_COLOUR + r), g_inst, 0);
		row.status = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, 352, y + 4, 136, 18, g_wardWnd, 0, g_inst, 0);
		row.show = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 490, y + 2, 56, 20, g_wardWnd, (HMENU)(INT_PTR)(IDW_SHOW + r), g_inst, 0);
		SendMessageW(row.label, WM_SETFONT, (WPARAM)g_font, TRUE);
		SendMessageW(row.part, WM_SETFONT, (WPARAM)g_font, TRUE);
		SendMessageW(row.colour, WM_SETFONT, (WPARAM)g_font, TRUE);
		SendMessageW(row.status, WM_SETFONT, (WPARAM)g_font, TRUE);
		SendMessageW(row.show, WM_SETFONT, (WPARAM)g_font, TRUE);
	}
	int y = 8 + WARD_ROWS * WARD_ROW_H + 4;
	g_wardExtras = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, 0, WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
		8, y, WARD_W - 24, 220, g_wardWnd, (HMENU)(INT_PTR)IDW_EXTRAS, g_inst, 0);
	SendMessageW(g_wardExtras, WM_SETFONT, (WPARAM)g_font, TRUE);
	ListView_SetExtendedListViewStyle(g_wardExtras, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	LVCOLUMNW col;
	memset(&col, 0, sizeof(col));
	col.mask = LVCF_TEXT | LVCF_WIDTH;
	col.cx = 380;
	col.pszText = const_cast<wchar_t*>(L"");
	SendMessageW(g_wardExtras, LVM_INSERTCOLUMNW, 0, (LPARAM)&col);
	col.cx = 90;
	SendMessageW(g_wardExtras, LVM_INSERTCOLUMNW, 1, (LPARAM)&col);
	g_wardReset = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 8, y + 226, 110, 24, g_wardWnd, (HMENU)(INT_PTR)ID_WARD_RESET, g_inst, 0);
	SendMessageW(g_wardReset, WM_SETFONT, (WPARAM)g_font, TRUE);
}

static void WardrobeShow(bool show)
{
	if (show && !g_wardrobe.Active())
	{
		LogAppend(T(L"! 옷장은 캐릭터(.bpet)에서만 쓸 수 있습니다", L"! The wardrobe works on a character (.bpet)"));
		return;
	}
	if (g_wardWnd)
		ShowWindow(g_wardWnd, show ? SW_SHOWNOACTIVATE : SW_HIDE);
}

// A character's parts sit next to its .bpet and share the name before the first '_'.
static void WardrobeDocumentChanged()
{
	g_wardrobe.Clear();
	if (g_doc.pet.kind == PET_BONE)
	{
		std::string stem = PathBaseName(g_doc.sourcePath);
		size_t us = stem.find('_');
		if (us != std::string::npos)
			g_wardrobe.Scan(g_doc.baseDir, stem.substr(0, us));
	}
	if (!g_wardrobe.Active())
	{
		if (g_wardWnd)
			ShowWindow(g_wardWnd, SW_HIDE);
		return;
	}
	if (!g_wardWnd)
		WardrobeCreate();
	WardrobeRebuild();
	WardrobeApplied();
	WardrobeShow(true);
}

// -- commands --------------------------------------------------------------------

static bool FileDialog(bool save, const wchar_t* filter, const wchar_t* defExt, std::wstring* path)
{
	wchar_t buf[1024];
	wcscpy_s(buf, path->c_str());
	OPENFILENAMEW ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_main;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = 1024;
	ofn.lpstrDefExt = defExt;
	ofn.Flags = OFN_HIDEREADONLY | OFN_NOCHANGEDIR | (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
	BOOL ok = save ? GetSaveFileNameW(&ofn) : GetOpenFileNameW(&ofn);
	if (ok)
		*path = buf;
	return ok != 0;
}

static bool ConfirmDiscard()
{
	if (!g_doc.dirty)
		return true;
	return MessageBoxW(g_main, T(L"저장하지 않은 변경 내용을 버릴까요?", L"Discard unsaved changes?"), L"CaddieRig", MB_YESNO | MB_ICONQUESTION) == IDYES;
}

static void WardrobeDocumentChanged();

static void DocumentChanged()
{
	g_frame = 0.0f;
	g_scroll = 0;
	g_currentMotion = -1;
	g_selection = MakeNode(N_FILE, 0);
	g_opts.selectedBone = g_opts.selectedBox = g_opts.selectedFace = -1;
	g_loopStart = 0;
	g_loopEnd = (int)g_doc.LastFrame();
	if (g_loopEnd > 0)
	{
		RECT rc;
		GetClientRect(g_timeline, &rc);
		// Start zoomed so a screenful holds at most a few hundred frames.
		g_zoom = (float)rc.right / (g_loopEnd < 400 ? (g_loopEnd + 10) : 400);
	}
	g_doc.Pose(0.0f);
	// A dozen boxes aid the eye; a character's seventy hide the body.
	g_opts.boxes = g_doc.pet.colls.size() <= 12;
	SyncChecks();
	AudioStopAll();
	StartBoxEffects();
	g_doc.modelHidden = false;
	g_doc.modelAlpha = 1.0f;
	g_visGroupsLoaded = false;
	g_visGroups.clear();
	ReplayScriptState(MotionAtFrame(0), 0);
	WardrobeDocumentChanged();
	RefreshTree();
	RefreshProps();
	ViewFrameAll();
	TitleUpdate();
	UpdateFrameLabel();
	UpdateStatus();
	InvalidateRect(g_timeline, 0, FALSE);
}

static void OpenPath(const std::wstring& path)
{
	if (!ConfirmDiscard())
		return;
	SetPlaying(false);
	BuildLog log;
	std::string p = Narrow(path);
	size_t dot = path.rfind(L'.');
	std::wstring ext = dot == std::wstring::npos ? std::wstring() : path.substr(dot);
	HCURSOR old = SetCursor(LoadCursor(0, IDC_WAIT));
	if (!_wcsicmp(ext.c_str(), L".pmp"))
		g_doc.OpenProject(p, &log);
	else if (!_wcsicmp(ext.c_str(), L".ase") || !_wcsicmp(ext.c_str(), L".ani"))
		g_doc.NewFromAse(p, &log);
	else
		g_doc.OpenPuppet(p, &log);
	SetCursor(old);
	FlushLog(log);
	DocumentChanged();
}

static const wchar_t OPEN_FILTER[] =
	L"Puppet / Project (*.pmp;*.pet;*.mpet;*.bpet;*.apet;*.ase)\0*.pmp;*.pet;*.mpet;*.bpet;*.apet;*.ase\0"
	L"PMP Project (*.pmp)\0*.pmp\0Puppet (*.pet;*.mpet;*.bpet;*.apet)\0*.pet;*.mpet;*.bpet;*.apet\0ASCII Scene Export (*.ase)\0*.ase\0All files\0*.*\0";

static bool SaveProject(bool saveAs)
{
	std::wstring path = Wide(g_doc.projectPath);
	if (path.empty() || saveAs)
	{
		if (path.empty() && !g_doc.sourcePath.empty())
		{
			path = Wide(g_doc.sourcePath);
			size_t dot = path.rfind(L'.');
			if (dot != std::wstring::npos)
				path.erase(dot);
			path += L".pmp";
		}
		if (!FileDialog(true, L"PMP Project (*.pmp)\0*.pmp\0", L"pmp", &path))
			return false;
	}
	std::string error;
	if (!g_doc.SaveProject(Narrow(path), &error))
	{
		LogAppend(Wide("! " + error));
		return false;
	}
	LogAppend(Fmt(L"%s %s", T(L"저장:", L"Saved"), path.c_str()));
	TitleUpdate();
	return true;
}

static void DoExport()
{
	std::wstring path = Wide(g_doc.sourcePath);
	size_t dot = path.rfind(L'.');
	if (dot != std::wstring::npos)
		path.erase(dot);
	path += Wide(PetKindExt(g_doc.pet.kind));
	if (!FileDialog(true, L"Puppet (*.pet)\0*.pet\0Mesh puppet (*.mpet)\0*.mpet\0Bone puppet (*.bpet)\0*.bpet\0Animation puppet (*.apet)\0*.apet\0", L"pet", &path))
		return;
	BuildLog log;
	g_doc.ExportPuppet(Narrow(path), g_exportVer / 10, g_exportVer % 10, &log);
	FlushLog(log);
}

// Small modal prompt. Windows has no ready-made one.
static std::wstring g_promptResult;
static bool g_promptDone, g_promptOk;
static HWND g_promptEdit;

static LRESULT CALLBACK PromptProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_COMMAND:
		if (LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL)
		{
			g_promptOk = LOWORD(wp) == IDOK;
			if (g_promptOk)
			{
				int len = GetWindowTextLengthW(g_promptEdit);
				g_promptResult.assign(len, L' ');
				if (len)
					GetWindowTextW(g_promptEdit, &g_promptResult[0], len + 1);
			}
			g_promptDone = true;
		}
		return 0;
	case WM_CLOSE:
		g_promptOk = false;
		g_promptDone = true;
		return 0;
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

static bool Prompt(const wchar_t* title, const wchar_t* label, std::wstring* value)
{
	RECT owner;
	GetWindowRect(g_main, &owner);
	int w = 460, h = 130;
	HWND dlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"CaddieRig_Prompt", title, WS_POPUP | WS_CAPTION | WS_SYSMENU,
		(owner.left + owner.right - w) / 2, (owner.top + owner.bottom - h) / 2, w, h, g_main, 0, g_inst, 0);
	HWND st = CreateWindowExW(0, L"STATIC", label, WS_CHILD | WS_VISIBLE, 10, 10, 430, 18, dlg, 0, g_inst, 0);
	g_promptEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", value->c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 10, 30, 430, 22, dlg, 0, g_inst, 0);
	HWND ok = CreateWindowExW(0, L"BUTTON", T(L"확인", L"OK"), WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP, 270, 60, 80, 26, dlg, (HMENU)IDOK, g_inst, 0);
	HWND no = CreateWindowExW(0, L"BUTTON", T(L"취소", L"Cancel"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 360, 60, 80, 26, dlg, (HMENU)IDCANCEL, g_inst, 0);
	SendMessageW(st, WM_SETFONT, (WPARAM)g_font, TRUE);
	SendMessageW(g_promptEdit, WM_SETFONT, (WPARAM)g_font, TRUE);
	SendMessageW(ok, WM_SETFONT, (WPARAM)g_font, TRUE);
	SendMessageW(no, WM_SETFONT, (WPARAM)g_font, TRUE);
	EnableWindow(g_main, FALSE);
	ShowWindow(dlg, SW_SHOW);
	SetFocus(g_promptEdit);
	SendMessageW(g_promptEdit, EM_SETSEL, 0, -1);
	g_promptDone = false;
	MSG m;
	while (!g_promptDone && GetMessageW(&m, 0, 0, 0))
	{
		if (!IsDialogMessageW(dlg, &m))
		{
			TranslateMessage(&m);
			DispatchMessageW(&m);
		}
	}
	EnableWindow(g_main, TRUE);
	DestroyWindow(dlg);
	SetForegroundWindow(g_main);
	if (g_promptOk)
		*value = g_promptResult;
	return g_promptOk;
}

static void ImportMotionDialog()
{
	std::wstring path;
	if (!FileDialog(false, L"Motion (*.ani;*.ase)\0*.ani;*.ase\0All files\0*.*\0", L"ani", &path))
		return;
	std::wstring name = Wide(PathBaseName(Narrow(path)));
	size_t dot = name.rfind(L'.');
	if (dot != std::wstring::npos)
		name.erase(dot);
	if (!Prompt(T(L"동작 가져오기", L"Import motion"), T(L"동작 이름:", L"Motion name:"), &name))
		return;
	BuildLog log;
	bool ok = g_doc.ImportMotion(Narrow(path), Narrow(name), &log);
	FlushLog(log);
	if (ok)
	{
		g_loopEnd = (int)g_doc.LastFrame();
		RefreshTree();
		SelectMotion(g_doc.BlockCount(PMP_MOTION) - 1, true);
		TitleUpdate();
	}
}

static void AddScriptDialog()
{
	std::wstring msg = L"*snd(\"\")";
	if (!Prompt(T(L"스크립트 추가", L"Add script"), Fmt(T(L"프레임 %d 메시지:", L"Message at frame %d:"), (int)g_frame).c_str(), &msg))
		return;
	g_doc.AddScript((int)g_frame, Narrow(msg));
	RefreshTree();
	TitleUpdate();
	InvalidateRect(g_timeline, 0, FALSE);
}

static void DeleteSelected()
{
	int idx = NodeIndexOf(g_selection);
	PmpBlockKind kind;
	switch (NodeKindOf(g_selection))
	{
	case N_TEXTURE: kind = PMP_TEXTURE; break;
	case N_BOX: kind = PMP_BOX; break;
	case N_SCRIPT: kind = PMP_SCRIPT; break;
	case N_MOTION: kind = PMP_MOTION; break;
	default:
		return;
	}
	g_doc.RemoveBlock(kind, idx);
	g_selection = MakeNode(N_FILE, 0);
	g_opts.selectedBox = -1;
	RefreshTree();
	RefreshProps();
	TitleUpdate();
	InvalidateRect(g_timeline, 0, FALSE);
	InvalidateRect(g_view, 0, FALSE);
}

static void SaveSettings()
{
	HKEY key;
	if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Retreev\\CaddieRig", 0, 0, 0, KEY_WRITE, 0, &key, 0) != ERROR_SUCCESS)
		return;
	std::wstring root = Wide(g_doc.dataRoot);
	RegSetValueExW(key, L"DataRoot", 0, REG_SZ, (const BYTE*)root.c_str(), (DWORD)((root.size() + 1) * 2));
	DWORD ko = g_korean ? 1 : 0;
	RegSetValueExW(key, L"Korean", 0, REG_DWORD, (const BYTE*)&ko, 4);
	RegCloseKey(key);
}

static void LoadSettings()
{
	g_korean = PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_KOREAN;
	HKEY key;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Retreev\\CaddieRig", 0, KEY_READ, &key) != ERROR_SUCCESS)
		return;
	wchar_t root[1024];
	DWORD size = sizeof(root);
	if (RegQueryValueExW(key, L"DataRoot", 0, 0, (BYTE*)root, &size) == ERROR_SUCCESS)
		g_doc.SetDataRoot(Narrow(root));
	DWORD ko, kosize = 4;
	if (RegQueryValueExW(key, L"Korean", 0, 0, (BYTE*)&ko, &kosize) == ERROR_SUCCESS)
		g_korean = ko != 0;
	RegCloseKey(key);
}

static void PickDataRoot()
{
	std::wstring root = Wide(g_doc.dataRoot);
	if (Prompt(T(L"텍스쳐 검색 폴더", L"Texture search folder"), T(L"텍스쳐를 찾을 게임 data 폴더:", L"Game data folder to search for textures:"), &root))
	{
		g_doc.SetDataRoot(Narrow(root));
		ViewRefreshTextures();
		SaveSettings();
		RefreshProps();
	}
}

static void SetLanguage(bool korean)
{
	g_korean = korean;
	SaveSettings();
	BuildMenu();
	SyncChecks();
	RefreshTree();
	RefreshProps();
	Layout();
	UpdateFrameLabel();
	UpdateStatus();
	SetWindowTextW(g_btnPlay, g_playing ? T(L"일시정지", L"Pause") : T(L"재생", L"Play"));
	SetWindowTextW(g_btnStop, T(L"정지", L"Stop"));
	WardrobeRebuild();
	SetWindowTextW(g_btnLoop, T(L"반복", L"Loop"));
	SetWindowTextW(g_btnFollow, T(L"다음 동작 따라가기", L"Follow next motion"));
	LVCOLUMNW col;
	col.mask = LVCF_TEXT;
	col.pszText = const_cast<wchar_t*>(T(L"속성", L"Property"));
	SendMessageW(g_props, LVM_SETCOLUMNW, 0, (LPARAM)&col);
	col.pszText = const_cast<wchar_t*>(T(L"값", L"Value"));
	SendMessageW(g_props, LVM_SETCOLUMNW, 1, (LPARAM)&col);
}

static void OnCommand(int id)
{
	switch (id)
	{
	case ID_FILE_NEWASE:
		{
			std::wstring path;
			if (FileDialog(false, L"ASCII Scene Export (*.ase)\0*.ase\0All files\0*.*\0", L"ase", &path))
				OpenPath(path);
		}
		break;
	case ID_FILE_OPEN:
		{
			std::wstring path;
			if (FileDialog(false, OPEN_FILTER, L"pmp", &path))
				OpenPath(path);
		}
		break;
	case ID_FILE_SAVE: SaveProject(false); break;
	case ID_FILE_SAVEAS: SaveProject(true); break;
	case ID_FILE_EXPORT: DoExport(); break;
	case ID_FILE_ATTACH:
		{
			std::wstring path;
			if (FileDialog(false, L"Mesh puppet (*.mpet)\0*.mpet\0", L"mpet", &path))
			{
				std::string p = Narrow(path);
				if (g_wardrobe.Active() && !_stricmp(PathDir(p).c_str(), g_doc.baseDir.c_str()) && g_wardrobe.Wear(PathBaseName(p)))
					WardrobeApplied();
				else
				{
					BuildLog log;
					g_doc.AttachPart(p, &log);
					FlushLog(log);
					RefreshTreeParts();
					SetFrame(g_frame, false);
				}
			}
		}
		break;
	case ID_FILE_EXIT:
		SendMessageW(g_main, WM_CLOSE, 0, 0);
		break;
	case ID_EDIT_ADDTEX:
		{
			std::wstring path;
			if (FileDialog(false, L"Images (*.jpg;*.png;*.tga;*.dds;*.bmp)\0*.jpg;*.png;*.tga;*.dds;*.bmp\0", L"jpg", &path))
			{
				g_doc.AddTexture(Narrow(path));
				RefreshTree();
				TitleUpdate();
			}
		}
		break;
	case ID_EDIT_ADDBOX:
		if (NodeKindOf(g_selection) == N_BONE)
		{
			g_doc.AddBoxForBone(NodeIndexOf(g_selection));
			RefreshTree();
			TitleUpdate();
			InvalidateRect(g_view, 0, FALSE);
		}
		else
			LogAppend(T(L"! 먼저 본을 선택하세요", L"! Select a bone first"));
		break;
	case ID_EDIT_ADDCENTER:
		g_doc.AddCenterBox();
		RefreshTree();
		TitleUpdate();
		InvalidateRect(g_view, 0, FALSE);
		break;
	case ID_EDIT_ADDSCRIPT: AddScriptDialog(); break;
	case ID_EDIT_IMPORTMOTION: ImportMotionDialog(); break;
	case ID_EDIT_DELETE: DeleteSelected(); break;
	case ID_BUILD_REBUILD:
		{
			if (!g_doc.hasGeometrySource)
			{
				LogAppend(T(L"! 원본 ASE가 없어 다시 빌드할 수 없습니다", L"! No ASE source to rebuild from"));
				break;
			}
			SetPlaying(false);
			BuildLog log;
			g_doc.Rebuild(&log);
			FlushLog(log);
			RefreshTree();
			RefreshProps();
			SetFrame(g_frame, false);
			g_doc.dirty = true;
			TitleUpdate();
		}
		break;
	case ID_BUILD_VALIDATE:
		{
			BuildLog log;
			std::vector<std::string> found;
			for (size_t i = 0; i < g_doc.pet.textures.size(); ++i)
			{
				std::string name = Cp949ToUtf8(g_doc.pet.textures[i].Name());
				if (!g_doc.ResolveTexture(name).empty())
					found.push_back(name);
			}
			ValidatePuppet(g_doc.pmp, g_doc.pet, g_doc.baseDir.c_str(), &log, &found);
			FlushLog(log);
		}
		break;
	case ID_VIEW_TEX: g_opts.shade = SHADE_TEXTURED; break;
	case ID_VIEW_FLAT: g_opts.shade = SHADE_FLAT; break;
	case ID_VIEW_WIRE: g_opts.shade = SHADE_WIRE; break;
	case ID_VIEW_SKEL: g_opts.skeleton = !g_opts.skeleton; break;
	case ID_VIEW_BOXES: g_opts.boxes = !g_opts.boxes; break;
	case ID_VIEW_NORMALS: g_opts.normals = !g_opts.normals; break;
	case ID_VIEW_GRID: g_opts.grid = !g_opts.grid; break;
	case ID_VIEW_LIT: g_opts.lit = !g_opts.lit; break;
	case ID_VIEW_FRAME: ViewFrameAll(); break;
	case ID_VIEW_PERSP: ViewSetPreset(0); break;
	case ID_VIEW_FRONT: ViewSetPreset(1); break;
	case ID_VIEW_SIDE: ViewSetPreset(2); break;
	case ID_VIEW_TOP: ViewSetPreset(3); break;
	case ID_OPT_ROOT: PickDataRoot(); break;
	case ID_OPT_VER_KEEP: g_exportVer = 0; break;
	case ID_OPT_VER_10: g_exportVer = 10; break;
	case ID_OPT_VER_13: g_exportVer = 13; break;
	case ID_OPT_KO: SetLanguage(true); break;
	case ID_OPT_EN: SetLanguage(false); break;
	case ID_HELP_FLAGS:
		MessageBoxW(g_main, T(
			L"텍스쳐 파일 이름의 앞 세 글자가 그리기 설정을 정합니다 (WPuppet::LoadPET_Texture).\n\n"
			L"[  알파 블렌드 (_mask 파일 사용)\n+  알파 블렌드 + 항상 밝게\n]  알파 테스트 + 앰비언트만\n{  알파 테스트\n~  0x20800000 + 알파\n}  0x21800000 + 알파\n@  맵 타입 1\n!  스페큘러 맵\n2#  양면 조명 (이름 어디에나)\n\n"
			L"플래그가 없고 '텍스쳐연결'이 1이면 256x256 아틀라스에 합쳐집니다.\n'!' 맵은 합쳐지지 않으며 .dds도 마찬가지입니다.",
			L"The first three characters of a texture file name choose how it draws (WPuppet::LoadPET_Texture).\n\n"
			L"[  alpha blend (alpha from a _mask file)\n+  alpha blend, full bright\n]  alpha test, ambient light only\n{  alpha test\n~  0x20800000 + alpha\n}  0x21800000 + alpha\n@  map type 1\n!  specular map\n2#  two-sided lighting (anywhere in the name)\n\n"
			L"With no flag and 'Atlas link' = 1 the texture is packed into a 256x256 atlas page.\n'!' maps and .dds files are never packed."),
			T(L"텍스쳐 이름 규칙", L"Texture name flags"), MB_OK | MB_ICONINFORMATION);
		break;
	case ID_HELP_ABOUT:
		MessageBoxW(g_main, T(
			L"CaddieRig\n\nNtreev의 Pangya 모델 도구 PuppetMasterG에서 영감을 받아 상상으로 만든 참고용 프로그램입니다.\n"
			L"원본 코드가 아니며 Ntreev와 관계가 없습니다.\n\n남아 있는 PMP/ASE/PET 파일과 클라이언트 로더를 바탕으로 만들었습니다.",
			L"CaddieRig\n\nA reference tool inspired by PuppetMasterG, which Ntreev used to author Pangya's puppets.\n"
			L"This is not Ntreev's code and is not affiliated with Ntreev.\n\nBuilt from surviving PMP/ASE/PET files and the client's loader."),
			T(L"CaddieRig 정보", L"About CaddieRig"), MB_OK | MB_ICONINFORMATION);
		break;
	case ID_TL_PLAY:
		if (g_doc.LastFrame() > 0.0f)
			SetPlaying(!g_playing);
		break;
	case ID_TL_STOP:
		SetPlaying(false);
		g_fx.ClearAll(true);
		AudioStopAll();
		SetFrame((float)(g_currentMotion >= 0 ? g_loopStart : 0), true);
		break;
	case ID_TL_PREV:
		SetPlaying(false);
		SetFrame((float)((int)g_frame - 1), true);
		break;
	case ID_TL_NEXT:
		SetPlaying(false);
		FireRange((float)(int)g_frame, (float)((int)g_frame + 1));
		SetFrame((float)((int)g_frame + 1), true);
		break;
	case ID_VIEW_WARDROBE:
		WardrobeShow(!IsWindowVisible(g_wardWnd));
		break;
	case ID_FX_SOUND:
		g_soundOn = !g_soundOn;
		AudioSetMuted(!g_soundOn);
		break;
	case ID_FX_EFFECTS:
		g_fxOn = !g_fxOn;
		g_opts.effects = g_fxOn;
		StartBoxEffects();
		InvalidateRect(g_view, 0, FALSE);
		break;
	case ID_FX_BOX:
		g_boxFxOn = !g_boxFxOn;
		StartBoxEffects();
		InvalidateRect(g_view, 0, FALSE);
		break;
	case ID_FX_PARTS:
		g_partFxOn = !g_partFxOn;
		StartBoxEffects();
		InvalidateRect(g_view, 0, FALSE);
		break;
	case ID_FX_FIRE:
		{
			NodeKind kind = NodeKindOf(g_selection);
			int idx = NodeIndexOf(g_selection);
			if (kind == N_SCRIPT)
			{
				const PmpBlock* b = g_doc.BlockAt(PMP_SCRIPT, idx);
				if (b)
					RunPuppetScript(Utf8ToCp949(b->Get("메시지")), std::string(), true);
			}
			else if (kind == N_BOX)
			{
				const PmpBlock* b = g_doc.BlockAt(PMP_BOX, idx);
				if (b)
				{
					std::vector<BoxFx> list;
					ParseBoxScript(Utf8ToCp949(b->Get("스크립트0") + " " + b->Get("스크립트1")), &list);
					for (size_t i = 0; i < list.size(); ++i)
					{
						FxAttacher a;
						if (FindAttachPoint(Utf8ToCp949(b->Get("박스이름")), 0, &a))
							g_fx.Attach(a, list[i].file, true);
					}
					if (list.empty())
						LogAppend(T(L"이 박스에는 효과 스크립트가 없습니다", L"This box has no effect script"));
				}
			}
			else
				LogAppend(T(L"! 스크립트나 충돌박스를 선택하세요", L"! Select a script or a collision box"));
		}
		break;
	case ID_FX_CLEAR:
		g_fx.ClearAll(true);
		AudioStopAll();
		InvalidateRect(g_view, 0, FALSE);
		break;
	default:
		return;
	}
	if (id >= ID_VIEW_TEX && id <= ID_VIEW_LIT)
	{
		InvalidateRect(g_view, 0, FALSE);
	}
	SyncChecks();
}

// -- menus, toolbar, layout --------------------------------------------------------

static HMENU g_menu;

static void Item(HMENU m, int id, const wchar_t* text)
{
	AppendMenuW(m, MF_STRING, id, text);
}

static void BuildMenu()
{
	HMENU old = g_menu;
	g_menu = CreateMenu();
	HMENU file = CreatePopupMenu();
	Item(file, ID_FILE_NEWASE, T(L"ASE에서 새 프로젝트...\tCtrl+N", L"New Project from ASE...\tCtrl+N"));
	Item(file, ID_FILE_OPEN, T(L"열기...\tCtrl+O", L"Open...\tCtrl+O"));
	Item(file, ID_FILE_ATTACH, T(L"메시 파츠 붙이기 (.mpet)...", L"Attach Mesh Part (.mpet)..."));
	AppendMenuW(file, MF_SEPARATOR, 0, 0);
	Item(file, ID_FILE_SAVE, T(L"프로젝트 저장\tCtrl+S", L"Save Project\tCtrl+S"));
	Item(file, ID_FILE_SAVEAS, T(L"다른 이름으로 프로젝트 저장...", L"Save Project As..."));
	Item(file, ID_FILE_EXPORT, T(L"퍼펫 내보내기 (.pet)...\tCtrl+E", L"Export Puppet (.pet)...\tCtrl+E"));
	AppendMenuW(file, MF_SEPARATOR, 0, 0);
	Item(file, ID_FILE_EXIT, T(L"끝내기", L"Exit"));

	HMENU edit = CreatePopupMenu();
	Item(edit, ID_EDIT_IMPORTMOTION, T(L"동작 가져오기 (.ani)...", L"Import Motion (.ani)..."));
	Item(edit, ID_EDIT_ADDSCRIPT, T(L"현재 프레임에 스크립트 추가...", L"Add Script at Current Frame..."));
	AppendMenuW(edit, MF_SEPARATOR, 0, 0);
	Item(edit, ID_EDIT_ADDTEX, T(L"텍스쳐 추가...", L"Add Texture..."));
	Item(edit, ID_EDIT_ADDBOX, T(L"선택한 본에 박스 맞추기", L"Fit Box to Selected Bone"));
	Item(edit, ID_EDIT_ADDCENTER, T(L"center 박스 추가", L"Add \"center\" Box"));
	AppendMenuW(edit, MF_SEPARATOR, 0, 0);
	Item(edit, ID_EDIT_DELETE, T(L"선택 항목 삭제\tDel", L"Delete Selected Item\tDel"));

	HMENU build = CreatePopupMenu();
	Item(build, ID_BUILD_REBUILD, T(L"다시 빌드 (ASE + ANI)\tF7", L"Rebuild (ASE + ANI)\tF7"));
	Item(build, ID_BUILD_VALIDATE, T(L"검사\tF8", L"Validate\tF8"));

	HMENU view = CreatePopupMenu();
	Item(view, ID_VIEW_TEX, T(L"텍스쳐", L"Textured"));
	Item(view, ID_VIEW_FLAT, T(L"단색", L"Flat"));
	Item(view, ID_VIEW_WIRE, T(L"와이어프레임", L"Wireframe"));
	AppendMenuW(view, MF_SEPARATOR, 0, 0);
	Item(view, ID_VIEW_SKEL, T(L"뼈대", L"Skeleton"));
	Item(view, ID_VIEW_BOXES, T(L"충돌박스", L"Collision Boxes"));
	Item(view, ID_VIEW_NORMALS, T(L"노멀", L"Normals"));
	Item(view, ID_VIEW_GRID, T(L"격자", L"Grid"));
	Item(view, ID_VIEW_LIT, T(L"조명", L"Lighting"));
	Item(view, ID_VIEW_WARDROBE, T(L"옷장\tF9", L"Wardrobe\tF9"));
	AppendMenuW(view, MF_SEPARATOR, 0, 0);
	Item(view, ID_VIEW_FRAME, T(L"모델에 맞추기\tF", L"Frame Model\tF"));
	Item(view, ID_VIEW_PERSP, T(L"원근", L"Perspective"));
	Item(view, ID_VIEW_FRONT, T(L"정면", L"Front"));
	Item(view, ID_VIEW_SIDE, T(L"측면", L"Side"));
	Item(view, ID_VIEW_TOP, T(L"윗면", L"Top"));

	HMENU script = CreatePopupMenu();
	Item(script, ID_FX_SOUND, T(L"소리 재생", L"Play Sounds"));
	Item(script, ID_FX_EFFECTS, T(L"효과 재생", L"Play Effects"));
	Item(script, ID_FX_BOX, T(L"충돌박스 효과", L"Collision Box Effects"));
	Item(script, ID_FX_PARTS, T(L"파츠 조건(&&id) 효과도 재생", L"Also Play Part-Conditional (&&id) Effects"));
	AppendMenuW(script, MF_SEPARATOR, 0, 0);
	Item(script, ID_FX_FIRE, T(L"선택 항목 실행\tF5", L"Fire Selected Script or Box\tF5"));
	Item(script, ID_FX_CLEAR, T(L"동작 효과 지우기\tF6", L"Clear Motion Effects\tF6"));

	HMENU opt = CreatePopupMenu();
	Item(opt, ID_OPT_ROOT, T(L"텍스쳐 검색 폴더...", L"Texture Search Folder..."));
	AppendMenuW(opt, MF_SEPARATOR, 0, 0);
	Item(opt, ID_OPT_VER_KEEP, T(L"내보내기 버전: 원본 유지", L"Export Version: Keep Source"));
	Item(opt, ID_OPT_VER_10, T(L"내보내기 버전: 1.0", L"Export Version: 1.0"));
	Item(opt, ID_OPT_VER_13, T(L"내보내기 버전: 1.3", L"Export Version: 1.3"));
	AppendMenuW(opt, MF_SEPARATOR, 0, 0);
	Item(opt, ID_OPT_KO, L"한국어");
	Item(opt, ID_OPT_EN, L"English");

	HMENU help = CreatePopupMenu();
	Item(help, ID_HELP_FLAGS, T(L"텍스쳐 이름 규칙", L"Texture Name Flags"));
	Item(help, ID_HELP_ABOUT, T(L"CaddieRig 정보", L"About CaddieRig"));

	AppendMenuW(g_menu, MF_POPUP, (UINT_PTR)file, T(L"파일(&F)", L"&File"));
	AppendMenuW(g_menu, MF_POPUP, (UINT_PTR)edit, T(L"편집(&E)", L"&Edit"));
	AppendMenuW(g_menu, MF_POPUP, (UINT_PTR)build, T(L"빌드(&B)", L"&Build"));
	AppendMenuW(g_menu, MF_POPUP, (UINT_PTR)view, T(L"보기(&V)", L"&View"));
	AppendMenuW(g_menu, MF_POPUP, (UINT_PTR)script, T(L"스크립트(&S)", L"&Script"));
	AppendMenuW(g_menu, MF_POPUP, (UINT_PTR)opt, T(L"설정(&O)", L"&Options"));
	AppendMenuW(g_menu, MF_POPUP, (UINT_PTR)help, T(L"도움말(&H)", L"&Help"));
	SetMenu(g_main, g_menu);
	if (old)
		DestroyMenu(old);

	// Toolbar: text buttons, rebuilt with the language
	if (g_toolbar)
	{
		int n = (int)SendMessageW(g_toolbar, TB_BUTTONCOUNT, 0, 0);
		while (n-- > 0)
			SendMessageW(g_toolbar, TB_DELETEBUTTON, 0, 0);
		static const struct { int id; BYTE style; const wchar_t* ko; const wchar_t* en; } tb[] = {
			{ ID_FILE_OPEN, BTNS_BUTTON, L"열기", L"Open" },
			{ ID_FILE_SAVE, BTNS_BUTTON, L"저장", L"Save" },
			{ ID_BUILD_REBUILD, BTNS_BUTTON, L"빌드", L"Build" },
			{ ID_FILE_EXPORT, BTNS_BUTTON, L"내보내기", L"Export" },
			{ ID_BUILD_VALIDATE, BTNS_BUTTON, L"검사", L"Validate" },
			{ 0, BTNS_SEP, L"", L"" },
			{ ID_VIEW_TEX, BTNS_CHECKGROUP, L"텍스쳐", L"Textured" },
			{ ID_VIEW_FLAT, BTNS_CHECKGROUP, L"단색", L"Flat" },
			{ ID_VIEW_WIRE, BTNS_CHECKGROUP, L"와이어", L"Wire" },
			{ 0, BTNS_SEP, L"", L"" },
			{ ID_VIEW_SKEL, BTNS_CHECK, L"뼈대", L"Skeleton" },
			{ ID_VIEW_BOXES, BTNS_CHECK, L"박스", L"Boxes" },
			{ ID_VIEW_NORMALS, BTNS_CHECK, L"노멀", L"Normals" },
			{ ID_VIEW_GRID, BTNS_CHECK, L"격자", L"Grid" },
			{ 0, BTNS_SEP, L"", L"" },
			{ ID_VIEW_FRAME, BTNS_BUTTON, L"맞추기", L"Frame" },
			{ ID_VIEW_FRONT, BTNS_BUTTON, L"정면", L"Front" },
			{ ID_VIEW_SIDE, BTNS_BUTTON, L"측면", L"Side" },
			{ ID_VIEW_TOP, BTNS_BUTTON, L"윗면", L"Top" },
			{ ID_VIEW_PERSP, BTNS_BUTTON, L"원근", L"Persp" },
		};
		for (size_t i = 0; i < sizeof(tb) / sizeof(tb[0]); ++i)
		{
			TBBUTTON b;
			memset(&b, 0, sizeof(b));
			b.iBitmap = tb[i].style == BTNS_SEP ? 6 : I_IMAGENONE;
			b.idCommand = tb[i].id;
			b.fsState = TBSTATE_ENABLED;
			b.fsStyle = (BYTE)(tb[i].style | (tb[i].style == BTNS_SEP ? 0 : BTNS_AUTOSIZE | BTNS_SHOWTEXT));
			b.iString = tb[i].style == BTNS_SEP ? 0 : (INT_PTR)(g_korean ? tb[i].ko : tb[i].en);
			SendMessageW(g_toolbar, TB_ADDBUTTONSW, 1, (LPARAM)&b);
		}
		SendMessageW(g_toolbar, TB_AUTOSIZE, 0, 0);
	}
}

static void CheckItem(int id, bool on)
{
	CheckMenuItem(g_menu, id, on ? MF_CHECKED : MF_UNCHECKED);
	if (g_toolbar)
		SendMessageW(g_toolbar, TB_CHECKBUTTON, id, on);
}

static void SyncChecks()
{
	CheckItem(ID_VIEW_TEX, g_opts.shade == SHADE_TEXTURED);
	CheckItem(ID_VIEW_FLAT, g_opts.shade == SHADE_FLAT);
	CheckItem(ID_VIEW_WIRE, g_opts.shade == SHADE_WIRE);
	CheckItem(ID_VIEW_SKEL, g_opts.skeleton);
	CheckItem(ID_VIEW_BOXES, g_opts.boxes);
	CheckItem(ID_VIEW_NORMALS, g_opts.normals);
	CheckItem(ID_VIEW_GRID, g_opts.grid);
	CheckItem(ID_VIEW_LIT, g_opts.lit);
	CheckItem(ID_OPT_VER_KEEP, g_exportVer == 0);
	CheckItem(ID_OPT_VER_10, g_exportVer == 10);
	CheckItem(ID_OPT_VER_13, g_exportVer == 13);
	CheckMenuItem(g_menu, ID_VIEW_WARDROBE, g_wardWnd && IsWindowVisible(g_wardWnd) ? MF_CHECKED : MF_UNCHECKED);
	CheckItem(ID_FX_SOUND, g_soundOn);
	CheckItem(ID_FX_EFFECTS, g_fxOn);
	CheckItem(ID_FX_BOX, g_boxFxOn);
	CheckItem(ID_FX_PARTS, g_partFxOn);
	CheckItem(ID_OPT_KO, g_korean);
	CheckItem(ID_OPT_EN, !g_korean);
}

static void UpdateStatus()
{
	if (!g_status)
		return;
	int verts = 0, tris = 0;
	for (size_t i = 0; i < g_doc.parts.size(); ++i)
	{
		if (g_doc.parts[i]->visible)
		{
			verts += (int)g_doc.parts[i]->pet->vertices.size();
			tris += (int)g_doc.parts[i]->pet->points.size() / 3;
		}
	}
	std::wstring a = Fmt(T(L"본 %d  정점 %d  삼각형 %d", L"Bones %d  Verts %d  Tris %d"), g_doc.HasSkeleton() ? (int)g_doc.Skeleton().size() : 0, verts, tris);
	std::wstring b = Fmt(L"%d / %d", (int)g_frame, (int)g_doc.LastFrame());
	std::wstring c = T(L"왼쪽: 회전  오른쪽: 이동  휠: 확대", L"LMB: orbit  RMB: pan  Wheel: zoom");
	SendMessageW(g_status, SB_SETTEXTW, 0, (LPARAM)a.c_str());
	SendMessageW(g_status, SB_SETTEXTW, 1, (LPARAM)b.c_str());
	SendMessageW(g_status, SB_SETTEXTW, 2, (LPARAM)c.c_str());
}

static void Layout()
{
	if (!g_status)
		return;
	RECT rc;
	GetClientRect(g_main, &rc);
	SendMessageW(g_toolbar, TB_AUTOSIZE, 0, 0);
	SendMessageW(g_status, WM_SIZE, 0, 0);
	RECT tr, sr;
	GetWindowRect(g_toolbar, &tr);
	GetWindowRect(g_status, &sr);
	int top = tr.bottom - tr.top;
	int bottom = rc.bottom - (sr.bottom - sr.top);
	int parts[3] = { rc.right * 40 / 100, rc.right * 55 / 100, -1 };
	SendMessageW(g_status, SB_SETPARTS, 3, (LPARAM)parts);

	int left = g_leftWidth;
	int h = bottom - top;
	int treeH = h * 58 / 100;
	MoveWindow(g_tree, 0, top, left, treeH, TRUE);
	MoveWindow(g_props, 0, top + treeH + 2, left, h - treeH - 2, TRUE);
	ListView_SetColumnWidth(g_props, 0, left * 44 / 100);
	ListView_SetColumnWidth(g_props, 1, left - left * 44 / 100 - GetSystemMetrics(SM_CXVSCROLL) - 4);

	int x = left + 4;
	int w = rc.right - x;
	int logH = h * 15 / 100;
	if (logH < 70)
		logH = 70;
	int controlsH = 26;
	int timelineH = TL_RULER + TL_MOTION + TL_SCRIPT + 8;
	int viewH = h - logH - controlsH - timelineH - 4;
	MoveWindow(g_view, x, top, w, viewH, TRUE);
	int y = top + viewH + 2;
	int bx = x;
	MoveWindow(g_btnPrev, bx, y, 30, 23, TRUE); bx += 32;
	MoveWindow(g_btnPlay, bx, y, 70, 23, TRUE); bx += 72;
	MoveWindow(g_btnNext, bx, y, 30, 23, TRUE); bx += 32;
	MoveWindow(g_btnStop, bx, y, 56, 23, TRUE); bx += 62;
	MoveWindow(g_btnLoop, bx, y + 2, 60, 20, TRUE); bx += 64;
	MoveWindow(g_btnFollow, bx, y + 2, 150, 20, TRUE); bx += 156;
	MoveWindow(g_frameLabel, bx, y + 4, rc.right - bx - 4, 18, TRUE);
	y += controlsH;
	MoveWindow(g_timeline, x, y, w, timelineH, TRUE);
	y += timelineH + 2;
	MoveWindow(g_log, x, y, w, bottom - y, TRUE);
}

static HWND MakeButton(const wchar_t* text, int id, DWORD style)
{
	HWND b = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | style, 0, 0, 10, 10, g_main, (HMENU)(INT_PTR)id, g_inst, 0);
	SendMessageW(b, WM_SETFONT, (WPARAM)g_font, TRUE);
	return b;
}

static void CreateChildren()
{
	g_toolbar = CreateWindowExW(0, TOOLBARCLASSNAMEW, 0, WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS | CCS_TOP,
		0, 0, 0, 0, g_main, (HMENU)IDC_TOOLBAR, g_inst, 0);
	SendMessageW(g_toolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
	SendMessageW(g_toolbar, TB_SETBITMAPSIZE, 0, MAKELONG(0, 0));
	SendMessageW(g_toolbar, WM_SETFONT, (WPARAM)g_font, TRUE);
	g_status = CreateWindowExW(0, STATUSCLASSNAMEW, 0, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, g_main, (HMENU)IDC_STATUS, g_inst, 0);
	SendMessageW(g_status, WM_SETFONT, (WPARAM)g_font, TRUE);

	g_tree = CreateWindowExW(WS_EX_CLIENTEDGE, WC_TREEVIEWW, 0, WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS,
		0, 0, 0, 0, g_main, (HMENU)IDC_TREE, g_inst, 0);
	SendMessageW(g_tree, WM_SETFONT, (WPARAM)g_font, TRUE);

	g_props = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, 0, WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL | WS_CLIPCHILDREN,
		0, 0, 0, 0, g_main, (HMENU)IDC_PROPS, g_inst, 0);
	SendMessageW(g_props, WM_SETFONT, (WPARAM)g_font, TRUE);
	ListView_SetExtendedListViewStyle(g_props, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	LVCOLUMNW col;
	memset(&col, 0, sizeof(col));
	col.mask = LVCF_TEXT | LVCF_WIDTH;
	col.cx = 120;
	col.pszText = const_cast<wchar_t*>(T(L"속성", L"Property"));
	SendMessageW(g_props, LVM_INSERTCOLUMNW, 0, (LPARAM)&col);
	col.pszText = const_cast<wchar_t*>(T(L"값", L"Value"));
	SendMessageW(g_props, LVM_INSERTCOLUMNW, 1, (LPARAM)&col);

	g_opts.skeleton = false;
	g_opts.boxes = true;
	g_opts.normals = false;
	g_opts.grid = true;
	g_opts.lit = true;
	g_opts.selectedBone = g_opts.selectedBox = g_opts.selectedFace = -1;
	g_opts.effects = true;
	g_view = ViewCreate(g_main, g_inst, &g_doc, &g_opts);
	ViewSetFx(&g_fx);
	g_fxTick = GetTickCount();
	SetTimer(g_main, 2, 25, 0);

	g_btnPrev = MakeButton(L"<", ID_TL_PREV, 0);
	g_btnPlay = MakeButton(T(L"재생", L"Play"), ID_TL_PLAY, 0);
	g_btnNext = MakeButton(L">", ID_TL_NEXT, 0);
	g_btnStop = MakeButton(T(L"정지", L"Stop"), ID_TL_STOP, 0);
	g_btnLoop = MakeButton(T(L"반복", L"Loop"), ID_TL_LOOP, BS_AUTOCHECKBOX);
	SendMessageW(g_btnLoop, BM_SETCHECK, BST_CHECKED, 0);
	g_btnFollow = MakeButton(T(L"다음 동작 따라가기", L"Follow next motion"), ID_TL_FOLLOW, BS_AUTOCHECKBOX);
	g_frameLabel = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 10, 10, g_main, (HMENU)IDC_FRAME_LABEL, g_inst, 0);
	SendMessageW(g_frameLabel, WM_SETFONT, (WPARAM)g_font, TRUE);

	g_timeline = CreateWindowExW(WS_EX_CLIENTEDGE, L"CaddieRig_Timeline", L"", WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, g_main, (HMENU)IDC_TIMELINE, g_inst, 0);
	g_log = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
		0, 0, 10, 10, g_main, (HMENU)IDC_LOG, g_inst, 0);
	SendMessageW(g_log, WM_SETFONT, (WPARAM)g_font, TRUE);
}

static LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CREATE:
		g_main = hwnd;
		return 0;
	case WM_SIZE:
		Layout();
		return 0;
	case WM_COMMAND:
		OnCommand(LOWORD(wp));
		return 0;
	case WM_TIMER:
		if (wp == 1)
			PlaybackTick();
		else if (wp == 2)
			EffectsTick();
		return 0;
	case WM_NOTIFY:
		{
			NMHDR* hdr = (NMHDR*)lp;
			if (hdr->idFrom == IDC_TREE && hdr->code == TVN_SELCHANGEDW)
				OnTreeSelect(((NMTREEVIEWW*)lp)->itemNew.lParam);
			else if (hdr->idFrom == IDC_PROPS && hdr->code == NM_DBLCLK)
				// The list view takes the focus back after this returns, which would
				// close an editor opened here at once.
				PostMessageW(hwnd, WM_APP, ((NMITEMACTIVATE*)lp)->iItem, 0);
			return 0;
		}
	case WM_APP:
		PropsDoubleClick((int)wp);
		return 0;
	case WM_DROPFILES:
		{
			wchar_t path[1024];
			if (DragQueryFileW((HDROP)wp, 0, path, 1024))
				OpenPath(path);
			DragFinish((HDROP)wp);
			return 0;
		}
	case WM_LBUTTONDOWN:
		if (GET_X_LPARAM(lp) >= g_leftWidth - 2 && GET_X_LPARAM(lp) <= g_leftWidth + 6)
		{
			g_splitDrag = true;
			SetCapture(hwnd);
		}
		return 0;
	case WM_MOUSEMOVE:
		if (g_splitDrag)
		{
			int x = GET_X_LPARAM(lp);
			if (x < 160) x = 160;
			if (x > 700) x = 700;
			g_leftWidth = x;
			Layout();
		}
		else if (GET_X_LPARAM(lp) >= g_leftWidth - 2 && GET_X_LPARAM(lp) <= g_leftWidth + 6)
			SetCursor(LoadCursor(0, IDC_SIZEWE));
		return 0;
	case WM_LBUTTONUP:
		if (g_splitDrag)
		{
			g_splitDrag = false;
			ReleaseCapture();
		}
		return 0;
	case WM_CLOSE:
		if (ConfirmDiscard())
			DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

static void HandleAccelerator(const MSG& m)
{
	// Function keys and the Ctrl shortcuts, unless a text field has the focus.
	if (m.message != WM_KEYDOWN)
		return;
	bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	wchar_t cls[32];
	GetClassNameW(GetFocus(), cls, 32);
	bool typing = !_wcsicmp(cls, L"Edit");
	if (ctrl)
	{
		switch (m.wParam)
		{
		case 'N': OnCommand(ID_FILE_NEWASE); break;
		case 'O': OnCommand(ID_FILE_OPEN); break;
		case 'S': OnCommand(ID_FILE_SAVE); break;
		case 'E': OnCommand(ID_FILE_EXPORT); break;
		}
		return;
	}
	if (m.wParam == VK_F9)
		OnCommand(ID_VIEW_WARDROBE);
	else if (m.wParam == VK_F5)
		OnCommand(ID_FX_FIRE);
	else if (m.wParam == VK_F6)
		OnCommand(ID_FX_CLEAR);
	else if (m.wParam == VK_F7)
		OnCommand(ID_BUILD_REBUILD);
	else if (m.wParam == VK_F8)
		OnCommand(ID_BUILD_VALIDATE);
	if (typing)
		return;
	switch (m.wParam)
	{
	case 'F': OnCommand(ID_VIEW_FRAME); break;
	case VK_SPACE: OnCommand(ID_TL_PLAY); break;
	case VK_LEFT:
		if (GetFocus() != g_tree)
			OnCommand(ID_TL_PREV);
		break;
	case VK_RIGHT:
		if (GetFocus() != g_tree)
			OnCommand(ID_TL_NEXT);
		break;
	case VK_DELETE: OnCommand(ID_EDIT_DELETE); break;
	}
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int show)
{
	g_inst = inst;
	g_opts.shade = SHADE_TEXTURED;
	OleInitialize(0);
	INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_TREEVIEW_CLASSES | ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES };
	InitCommonControlsEx(&icc);
	TextureStartup();

	int argc = 0;
	wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	LoadSettings();

	// Command line: file [/root dir] [/lang ko|en] [/motion name] [/frame n] [/attach file.mpet]
	//                    [/skeleton] [/boxes] [/wire] [/flat] [/play] [/mute] [/partfx] [/wear part.mpet] [/nowardrobe]
	std::wstring openFile, motionArg, attachArg;
	std::vector<std::wstring> wearArgs;
	bool hideWardrobe = false;
	int frameArg = -1;
	bool skeletonArg = false, playArg = false, boxesArg = false;
	for (int i = 1; i < argc; ++i)
	{
		if (!wcscmp(argv[i], L"/root") && i + 1 < argc)
			g_doc.SetDataRoot(Narrow(argv[++i]));
		else if (!wcscmp(argv[i], L"/lang") && i + 1 < argc)
			g_korean = !wcscmp(argv[++i], L"ko");
		else if (!wcscmp(argv[i], L"/motion") && i + 1 < argc)
			motionArg = argv[++i];
		else if (!wcscmp(argv[i], L"/frame") && i + 1 < argc)
			frameArg = _wtoi(argv[++i]);
		else if (!wcscmp(argv[i], L"/attach") && i + 1 < argc)
			attachArg = argv[++i];
		else if (!wcscmp(argv[i], L"/skeleton"))
			skeletonArg = true;
		else if (!wcscmp(argv[i], L"/boxes"))
			boxesArg = true;
		else if (!wcscmp(argv[i], L"/wire"))
			g_opts.shade = SHADE_WIRE;
		else if (!wcscmp(argv[i], L"/flat"))
			g_opts.shade = SHADE_FLAT;
		else if (!wcscmp(argv[i], L"/play"))
			playArg = true;
		else if (!wcscmp(argv[i], L"/mute"))
			g_soundOn = false;
		else if (!wcscmp(argv[i], L"/wear") && i + 1 < argc)
			wearArgs.push_back(argv[++i]);
		else if (!wcscmp(argv[i], L"/nowardrobe"))
			hideWardrobe = true;
		else if (!wcscmp(argv[i], L"/partfx"))
			g_partFxOn = true;
		else
			openFile = argv[i];
	}

	g_font = CreateFontW(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"Gulim");
	g_fontSmall = CreateFontW(-11, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"Gulim");

	WNDCLASSW wc;
	memset(&wc, 0, sizeof(wc));
	wc.lpfnWndProc = MainProc;
	wc.hInstance = inst;
	wc.hIcon = LoadIconW(inst, MAKEINTRESOURCEW(1));
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = L"CaddieRig_Main";
	RegisterClassW(&wc);
	wc.lpfnWndProc = TimelineProc;
	wc.hIcon = 0;
	wc.hbrBackground = 0;
	wc.lpszClassName = L"CaddieRig_Timeline";
	RegisterClassW(&wc);
	wc.lpfnWndProc = WardrobeProc;
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = L"CaddieRig_Wardrobe";
	RegisterClassW(&wc);
	wc.lpfnWndProc = PromptProc;
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = L"CaddieRig_Prompt";
	RegisterClassW(&wc);
	ViewRegisterClass(inst);

	g_main = CreateWindowExW(WS_EX_ACCEPTFILES, L"CaddieRig_Main", L"CaddieRig", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, 1280, 860, 0, 0, inst, 0);
	CreateChildren();
	BuildMenu();
	SyncChecks();
	Layout();
	if (skeletonArg)
		g_opts.skeleton = true;
	SyncChecks();
	DocumentChanged();
	LogAppend(T(L"CaddieRig - PMP, ASE, PET 파일을 끌어다 놓으세요", L"CaddieRig - drop a PMP, ASE or PET file here"));
	ShowWindow(g_main, show);
	UpdateWindow(g_main);

	if (!openFile.empty())
		OpenPath(openFile);
	if (!attachArg.empty())
	{
		std::string p = Narrow(attachArg);
		if (!g_wardrobe.Active() || !g_wardrobe.Wear(PathBaseName(p)))
		{
			BuildLog log;
			g_doc.AttachPart(p, &log);
			FlushLog(log);
			RefreshTreeParts();
		}
		else
			WardrobeApplied();
		SetFrame(g_frame, false);
	}
	for (size_t i = 0; i < wearArgs.size(); ++i)
		if (g_wardrobe.Active() && g_wardrobe.Wear(Narrow(wearArgs[i])))
			WardrobeApplied();
	if (hideWardrobe && g_wardWnd)
		ShowWindow(g_wardWnd, SW_HIDE);
	if (!motionArg.empty())
	{
		std::string want = Utf8ToCp949(Narrow(motionArg));
		const PetFile& a = g_doc.AnimSource();
		for (size_t i = 0; i < a.motions.size(); ++i)
			if (a.motions[i].name == want)
				SelectMotion((int)i, true);
	}
	if (frameArg >= 0)
		SetFrame((float)frameArg, true);
	if (boxesArg)
	{
		g_opts.boxes = true;
		SyncChecks();
	}
	if (playArg)
		SetPlaying(true);

	MSG m;
	while (GetMessageW(&m, 0, 0, 0))
	{
		HandleAccelerator(m);
		if (g_wardWnd && IsDialogMessageW(g_wardWnd, &m))
			continue;
		TranslateMessage(&m);
		DispatchMessageW(&m);
	}
	AudioStopAll();
	TextureShutdown();
	OleUninitialize();
	return 0;
}
