// SPDX-License-Identifier: MIT
// main.cpp - the application window, menus, tools and console.
//
//   FairwayED [file.wep|file.gbin] [/root dir] [/lang ko|en] [/exec "cmd;cmd"] [/shot out.png]
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <objidl.h>
#include <gdiplus.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include "scene.h"
#include "wcommon.h"
#include "texture.h"

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

Editor g_ed;

Editor::Editor()
{
	tool = TOOL_SELECT;
	grid = 8.0f;
	snap = false;
	snapGround = false;
	showPuppets = showPoints = showCameras = showAreas = showNodes = showLabels = true;
	showShadow = showFog = showSky = showFx = showScripts = true;
	activeNode = -1;
	surfaceHighlight = -1;
	korean = false;
	main = 0;
	inst = 0;
}

bool Editor::IsSelected(const ElementRef& r) const
{
	return std::find(sel.begin(), sel.end(), r) != sel.end();
}

enum
{
	ID_NEW = 100, ID_OPEN, ID_SAVE, ID_SAVEAS, ID_EXPORT, ID_ROOT, ID_EXIT,
	ID_UNDO, ID_REDO, ID_DUP, ID_DEL, ID_SELALL, ID_DROP, ID_NEWNODE, ID_SETBASE,
	ID_FRAMEALL, ID_FRAMESEL, ID_MAX, ID_SHADE_WIRE, ID_SHADE_FLAT, ID_SHADE_TEX, ID_SHADE_SURF,
	ID_SHOW_PUPPETS, ID_SHOW_POINTS, ID_SHOW_CAMERAS, ID_SHOW_AREAS, ID_SHOW_NODES, ID_SHOW_LABELS, ID_SHOW_SHADOW, ID_SHOW_FOG, ID_SHOW_SKY,
	ID_SHOW_FX, ID_SHOW_SCRIPTS, ID_FX_RESTART, ID_AR_WIND,
	ID_LOOKCAM, ID_FLY,
	ID_CHECK, ID_BAKE, ID_CLEARSHADOW, ID_BUILD,
	ID_GRID1, ID_GRID2, ID_GRID4, ID_GRID8, ID_GRID16, ID_GRID32, ID_SNAP, ID_SNAPGROUND,
	ID_LANG_KO, ID_LANG_EN, ID_ABOUT,
	ID_PT_POINT, ID_PT_SUN, ID_PT_MOON, ID_PT_LIGHT, ID_PT_LIGHT2,
	ID_AR_EVENT, ID_AR_NPC, ID_AR_LAKE, ID_AR_SOUND, ID_AR_FX,
	ID_TOOL_FIRST = 300,
	ID_CONSOLE = 400
};

static HWND g_status, g_log, g_console;
static HWND g_toolButtons[TOOL_COUNT];
static WNDPROC g_consoleOld;
static HFONT g_font;
static std::vector<std::string> g_consoleHistory;
static const int LEFTBAR = 40, RIGHTPANEL = 340, BOTTOM = 124;

// -- log / status --------------------------------------------------------------

void EdLog(const std::string& line)
{
	std::wstring w = Wide(line) + L"\r\n";
	int len = GetWindowTextLengthW(g_log);
	if (len > 60000)
	{
		SendMessageW(g_log, EM_SETSEL, 0, 20000);
		SendMessageW(g_log, EM_REPLACESEL, FALSE, (LPARAM)L"");
		len = GetWindowTextLengthW(g_log);
	}
	SendMessageW(g_log, EM_SETSEL, len, len);
	SendMessageW(g_log, EM_REPLACESEL, FALSE, (LPARAM)w.c_str());
}

void EdFlush(const CourseLog& log)
{
	for (size_t i = 0; i < log.lines.size(); ++i)
		EdLog(log.lines[i]);
}

void EdStatus(int part, const std::wstring& text)
{
	SendMessageW(g_status, SB_SETTEXTW, part, (LPARAM)text.c_str());
}

static void TitleUpdate()
{
	Course& c = g_ed.course;
	std::wstring t = L"FairwayED - ";
	t += c.path.empty() ? std::wstring(L"Untitled") : Wide(PathBaseName(c.path));
	if (c.dirty)
		t += L" *";
	SetWindowTextW(g_ed.main, t.c_str());
}

static void StatusUpdate()
{
	wchar_t buf[128];
	swprintf_s(buf, L"%s: %d", T(L"선택", L"Selected"), (int)g_ed.sel.size());
	EdStatus(0, buf);
	swprintf_s(buf, L"%s %.0f%s%s", T(L"격자", L"Grid"), g_ed.grid, g_ed.snap ? T(L" 스냅", L" snap") : L"", g_ed.snapGround ? T(L", 지면 붙이기", L", ground") : L"");
	EdStatus(2, buf);
	static const wchar_t* const NAMES[TOOL_COUNT] = { L"Select/Move", L"Rotate", L"Scale", L"Add Puppet", L"Add Point", L"Add Camera", L"Add Area", L"Edit Nodes" };
	EdStatus(3, NAMES[g_ed.tool]);
}

void EdSelectionChanged()
{
	PanelsRefreshProps();
	PanelsSyncTreeSelection();
	StatusUpdate();
	ViewsRedraw();
}

void EdSceneChanged()
{
	PanelsRefreshScene();
	PanelsRefreshProps();
	PanelsRefreshHole();
	TitleUpdate();
	StatusUpdate();
	ViewsRedraw();
}

void EdValuesChanged()
{
	TitleUpdate();
	PanelsRefreshProps();
	ViewsRedraw();
}

void EdSetTool(Tool t)
{
	g_ed.tool = t;
	for (int i = 0; i < TOOL_COUNT; ++i)
		SendMessageW(g_toolButtons[i], BM_SETCHECK, i == t ? BST_CHECKED : BST_UNCHECKED, 0);
	for (int i = 0; i < TOOL_COUNT; ++i)
		InvalidateRect(g_toolButtons[i], 0, TRUE);
	StatusUpdate();
	ViewsRedraw();
}

// -- edits ---------------------------------------------------------------------

int EdAddElement(const WepElement& e, const std::string& label)
{
	Course& c = g_ed.course;
	c.Checkpoint(label);
	int at = c.wep.InsertElement(e);
	if (e.IsPuppet())
		c.wep.SyncPuppetList();
	EdSceneChanged();
	return at;
}

void EdDeleteSelection()
{
	Course& c = g_ed.course;
	if (g_ed.sel.empty())
		return;
	c.Checkpoint("Delete");
	std::vector<ElementRef> order = g_ed.sel;
	// Highest index first so earlier erasures don't shift later ones.
	struct { bool operator()(const ElementRef& a, const ElementRef& b) const { return a.index != b.index ? a.index > b.index : a.point > b.point; } } byIndexDown;
	std::sort(order.begin(), order.end(), byIndexDown);
	for (size_t i = 0; i < order.size(); ++i)
	{
		const ElementRef& r = order[i];
		if (r.kind == ElementRef::ELEMENT)
		{
			const WepElement& e = c.wep.elements[r.index];
			if (e.IsGame())
			{
				EdLog("! The GAME element holds par and light; it stays.");
				continue;
			}
			c.wep.elements.erase(c.wep.elements.begin() + r.index);
		}
		else if (r.kind == ElementRef::NODE)
		{
			c.wep.nodes.erase(c.wep.nodes.begin() + r.index);
			if (g_ed.activeNode >= r.index)
				g_ed.activeNode = -1;
		}
		else if (r.kind == ElementRef::NODE_POINT)
		{
			std::vector<WepNodePoint>& pts = c.wep.nodes[r.index].points;
			pts.erase(pts.begin() + r.point);
		}
	}
	g_ed.sel.clear();
	c.wep.SyncPuppetList();
	EdSceneChanged();
}

void EdDuplicateSelection()
{
	Course& c = g_ed.course;
	if (g_ed.sel.empty())
		return;
	c.Checkpoint("Duplicate");
	std::vector<ElementRef> fresh;
	WVector offset(g_ed.grid * 2.0f, 0.0f, g_ed.grid * 2.0f);
	for (size_t i = 0; i < g_ed.sel.size(); ++i)
	{
		const ElementRef& r = g_ed.sel[i];
		if (r.kind == ElementRef::ELEMENT)
		{
			WepElement e = c.wep.elements[r.index];
			if (e.IsGame() || e.IsGround())
				continue;
			e.Remove("GUID");
			if (e.IsArea())
			{
				WVector lo, hi;
				e.GetBox(&lo, &hi);
				e.SetBox(lo + offset, hi + offset);
			}
			else
			{
				e.SetVec("Pos", e.GetVec("Pos") + offset);
				if (e.IsCamera())
					e.SetVec("Dest", e.GetVec("Dest") + offset);
			}
			fresh.push_back(ElementRef(ElementRef::ELEMENT, c.wep.InsertElement(e)));
		}
		else if (r.kind == ElementRef::NODE)
		{
			WepNode n = c.wep.nodes[r.index];
			n.Remove("GUID");
			for (size_t k = 0; k < n.points.size(); ++k)
			{
				n.points[k].guid.clear();
				n.points[k].Set(n.points[k].pos + offset);
			}
			c.wep.nodes.push_back(n);
			fresh.push_back(ElementRef(ElementRef::NODE, (int)c.wep.nodes.size() - 1));
		}
	}
	g_ed.sel = fresh;
	EdSceneChanged();
	EdSelectionChanged();
}

static void DropSelection()
{
	Course& c = g_ed.course;
	if (g_ed.sel.empty())
		return;
	c.Checkpoint("Drop to ground");
	int moved = 0, missed = 0;
	for (size_t i = 0; i < g_ed.sel.size(); ++i)
	{
		const ElementRef& r = g_ed.sel[i];
		if (r.kind == ElementRef::ELEMENT)
		{
			WepElement& e = c.wep.elements[r.index];
			if (!e.IsPuppet() && !e.IsPoint())
				continue;
			if (e.IsPuppet() && e.IsGround())
				continue;
			WVector p = e.GetVec("Pos");
			if (c.DropToGround(&p))
			{
				e.SetVec("Pos", p);
				++moved;
			}
			else
			{
				++missed;
			}
		}
		else if (r.kind == ElementRef::NODE)
		{
			std::vector<WepNodePoint>& pts = c.wep.nodes[r.index].points;
			for (size_t k = 0; k < pts.size(); ++k)
			{
				WVector p = pts[k].pos;
				if (c.DropToGround(&p))
				{
					pts[k].Set(p);
					++moved;
				}
				else
				{
					++missed;
				}
			}
		}
		else if (r.kind == ElementRef::NODE_POINT)
		{
			WVector p = c.wep.nodes[r.index].points[r.point].pos;
			if (c.DropToGround(&p))
			{
				c.wep.nodes[r.index].points[r.point].Set(p);
				++moved;
			}
		}
	}
	char buf[96];
	sprintf_s(buf, "Dropped %d, %d not above the base model.", moved, missed);
	EdLog(buf);
	EdValuesChanged();
}

// -- files ---------------------------------------------------------------------

static bool FileDialog(bool save, const wchar_t* filter, const wchar_t* defExt, std::wstring* path)
{
	wchar_t buf[MAX_PATH];
	wcsncpy_s(buf, path->c_str(), MAX_PATH - 1);
	OPENFILENAMEW ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_ed.main;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = defExt;
	ofn.Flags = OFN_HIDEREADONLY | OFN_NOCHANGEDIR | (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
	if (!(save ? GetSaveFileNameW(&ofn) : GetOpenFileNameW(&ofn)))
		return false;
	*path = buf;
	return true;
}

static bool ConfirmDiscard()
{
	bool dirty = g_ed.course.dirty || g_ed.course.prop.dirty;
	if (!dirty)
		return true;
	return MessageBoxW(g_ed.main, T(L"저장하지 않은 변경 사항을 버릴까요?", L"Discard unsaved changes?"), L"FairwayED", MB_YESNO | MB_ICONQUESTION) == IDYES;
}

static void DocumentChanged()
{
	g_ed.sel.clear();
	g_ed.activeNode = -1;
	g_ed.surfaceHighlight = -1;
	SceneInvalidateBase();
	PanelsRefreshScene();
	PanelsRefreshProps();
	PanelsRefreshHole();
	PanelsRefreshBrowser();
	PanelsRefreshSurfaces();
	ViewsFrameAll();
	// The 3D pane opens where the player's first look at the hole is.
	int cam = g_ed.course.wep.FindElement("", "DefaultCamera");
	if (cam >= 0 && g_ed.course.wep.elements[cam].IsCamera())
		ViewsLookThrough(cam);
	TitleUpdate();
	StatusUpdate();
}

// Makes a puppet the hole's base model. Colours and shadow maps were baked for the old one.
static void SetBaseModel(const std::string& cp949Name)
{
	Course& c = g_ed.course;
	if (cp949Name.empty())
	{
		EdLog("! Pick a puppet in the Puppets tab first.");
		return;
	}
	c.Checkpoint("Set base model");
	std::string file = c.wep.rootDir + PathBaseName(cp949Name);
	bool found = false;
	for (size_t i = 0; i < c.wep.elements.size(); ++i)
	{
		if (c.wep.elements[i].IsPuppet() && c.wep.elements[i].IsGround())
		{
			c.wep.elements[i].Set("Filename", file);
			found = true;
		}
	}
	if (!found)
	{
		WepElement e = WepMakeElement("PUPPET", "WGround");
		e.Set("Filename", file);
		c.wep.elements.push_back(e);
	}
	c.wep.SyncPuppetList();
	c.ComputeBaseMeshes();
	c.cornerColor.clear();
	c.hasSbin = false;
	c.sbin.Clear();
	++c.revision;
	SceneInvalidateBase();
	EdSceneChanged();
	EdLog("Base model set. Build > Light Base Model gives it colours.");
}

static void OpenPath(const std::wstring& path)
{
	CourseLog log;
	HCURSOR old = SetCursor(LoadCursor(0, IDC_WAIT));
	bool ok = g_ed.course.Open(Narrow(path), &log);
	SetCursor(old);
	EdFlush(log);
	if (!ok)
		MessageBoxW(g_ed.main, (Wide(log.lines.empty() ? std::string() : log.lines.back())).c_str(), L"FairwayED", MB_ICONERROR);
	DocumentChanged();
}

static bool SaveProject(bool saveAs)
{
	Course& c = g_ed.course;
	std::wstring path = Wide(c.path);
	if (saveAs || path.empty())
	{
		if (!FileDialog(true, L"FairwayED project (*.wep)\0*.wep\0", L"wep", &path))
			return false;
	}
	std::string error;
	if (!c.SaveWep(Narrow(path), &error))
	{
		MessageBoxW(g_ed.main, Wide(error).c_str(), L"FairwayED", MB_ICONERROR);
		return false;
	}
	EdLog("Saved " + Narrow(path));
	TitleUpdate();
	return true;
}

static void ExportDialog()
{
	Course& c = g_ed.course;
	std::wstring path = c.path.empty() ? std::wstring() : Wide(c.path.substr(0, c.path.rfind('.')) + ".gbin");
	if (!FileDialog(true, L"Compiled hole (*.gbin)\0*.gbin\0", L"gbin", &path))
		return;
	CourseLog log;
	c.Validate(&log);
	if (log.warnings)
		log.Info("Exporting with %d map check warning(s).", log.warnings);
	c.ExportHole(Narrow(path), &log);
	EdFlush(log);
}

static std::string ReadSetting(const wchar_t* name)
{
	HKEY key;
	std::string out;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Retreev\\FairwayED", 0, KEY_READ, &key) == ERROR_SUCCESS)
	{
		wchar_t buf[MAX_PATH];
		DWORD size = sizeof(buf), type;
		if (RegQueryValueExW(key, name, 0, &type, (BYTE*)buf, &size) == ERROR_SUCCESS && type == REG_SZ)
			out = Narrow(buf);
		RegCloseKey(key);
	}
	return out;
}

static void WriteSetting(const wchar_t* name, const std::string& value)
{
	HKEY key;
	if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Retreev\\FairwayED", 0, 0, 0, KEY_WRITE, 0, &key, 0) == ERROR_SUCCESS)
	{
		std::wstring w = Wide(value);
		RegSetValueExW(key, name, 0, REG_SZ, (const BYTE*)w.c_str(), (DWORD)(w.size() + 1) * 2);
		RegCloseKey(key);
	}
}

static void PickRoot()
{
	BROWSEINFOW bi;
	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner = g_ed.main;
	bi.lpszTitle = T(L"게임 data 폴더를 고르세요", L"Choose the game's data folder");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	LPITEMIDLIST list = SHBrowseForFolderW(&bi);
	if (!list)
		return;
	wchar_t buf[MAX_PATH];
	if (SHGetPathFromIDListW(list, buf))
	{
		AreaFxReset();
		g_ed.course.SetDataRoot(Narrow(buf));
		WriteSetting(L"DataRoot", Narrow(buf));
		EdLog("Data folder: " + Narrow(buf));
		SceneInvalidateBase();
		DocumentChanged();
	}
	CoTaskMemFree(list);
}

// -- prompt --------------------------------------------------------------------

static HWND g_promptEdit;
static std::wstring* g_promptValue;
static bool g_promptOk, g_promptDone;

static LRESULT CALLBACK PromptProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_COMMAND:
		if (LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL)
		{
			if (LOWORD(wp) == IDOK)
			{
				wchar_t buf[512];
				GetWindowTextW(g_promptEdit, buf, 512);
				*g_promptValue = buf;
				g_promptOk = true;
			}
			g_promptDone = true;
			return 0;
		}
		break;
	case WM_CLOSE:
		g_promptDone = true;
		return 0;
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

bool EdPrompt(const wchar_t* title, const wchar_t* label, std::wstring* value)
{
	RECT rc;
	GetWindowRect(g_ed.main, &rc);
	HWND dlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"FairwayED_Prompt", title, WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
		(rc.left + rc.right) / 2 - 170, (rc.top + rc.bottom) / 2 - 60, 340, 120, g_ed.main, 0, g_ed.inst, 0);
	HWND s = CreateWindowW(L"STATIC", label, WS_CHILD | WS_VISIBLE, 8, 8, 310, 16, dlg, 0, g_ed.inst, 0);
	g_promptEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", value->c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 8, 26, 310, 22, dlg, 0, g_ed.inst, 0);
	HWND ok = CreateWindowW(L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP, 160, 54, 76, 24, dlg, (HMENU)IDOK, g_ed.inst, 0);
	HWND cancel = CreateWindowW(L"BUTTON", T(L"취소", L"Cancel"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 242, 54, 76, 24, dlg, (HMENU)IDCANCEL, g_ed.inst, 0);
	SendMessageW(s, WM_SETFONT, (WPARAM)g_font, 0);
	SendMessageW(g_promptEdit, WM_SETFONT, (WPARAM)g_font, 0);
	SendMessageW(ok, WM_SETFONT, (WPARAM)g_font, 0);
	SendMessageW(cancel, WM_SETFONT, (WPARAM)g_font, 0);
	SendMessageW(g_promptEdit, EM_SETSEL, 0, -1);
	SetFocus(g_promptEdit);
	EnableWindow(g_ed.main, FALSE);
	g_promptValue = value;
	g_promptOk = g_promptDone = false;
	MSG m;
	while (!g_promptDone && GetMessageW(&m, 0, 0, 0))
	{
		if (!IsDialogMessageW(dlg, &m))
		{
			TranslateMessage(&m);
			DispatchMessageW(&m);
		}
	}
	EnableWindow(g_ed.main, TRUE);
	DestroyWindow(dlg);
	SetActiveWindow(g_ed.main);
	return g_promptOk;
}

// -- console -------------------------------------------------------------------

static void SyncChecks();

// UnrealEd has one, so FairwayED has one. Also what /exec feeds.
void EdRunConsole(const std::string& line)
{
	Course& c = g_ed.course;
	char cmd[64] = "";
	char rest[512] = "";
	sscanf(line.c_str(), "%63s %511[^\n]", cmd, rest);
	std::string op = LowerCp949(cmd);
	if (op.empty())
		return;
	if (op != "exec")
		EdLog("> " + line);
	float a = 0, b = 0, d = 0;
	if (op == "help" || op == "?")
	{
		EdLog("open <file>  save [file]  export <file>  check  bake  undo  redo\n"
			"select <n>|all|none|class <WClass>  goto x y z  frame  drop  delete  duplicate\n"
			"par <n>  move x y z  rot x y z  scale s  tool select|rotate|scale|puppet|point|camera|area|node\n"
			"puppet <file.pet>  ground <file.pet>  point <class> <name>  node <name>  rootdir <path>\n"
			"shade wire|flat|tex|surf  grid <n>  look <camera>  camera x y z yaw pitch  fly  fog  fx  fxrestart  fxrun <frames>  scripts  tab <0-3>  info");
	}
	else if (op == "open")
	{
		OpenPath(Wide(rest));
	}
	else if (op == "save")
	{
		std::string error;
		if (!c.SaveWep(rest[0] ? rest : c.path, &error))
			EdLog("! " + error);
		else
			EdLog("Saved " + c.path);
		TitleUpdate();
	}
	else if (op == "export")
	{
		CourseLog log;
		c.ExportHole(rest, &log);
		EdFlush(log);
	}
	else if (op == "check")
	{
		CourseLog log;
		c.Validate(&log);
		EdFlush(log);
	}
	else if (op == "bake")
	{
		CourseLog log;
		c.BakeBaseColors(&log);
		EdFlush(log);
		SceneInvalidateBase();
		ViewsRedraw();
		TitleUpdate();
	}
	else if (op == "undo" || op == "redo")
	{
		if (op == "undo" ? c.Undo() : c.Redo())
		{
			g_ed.sel.clear();
			EdSceneChanged();
		}
	}
	else if (op == "select")
	{
		std::string what = rest;
		if (what == "none")
			g_ed.sel.clear();
		else if (what == "all" || what.compare(0, 6, "class ") == 0)
		{
			g_ed.sel.clear();
			std::string cls = what == "all" ? std::string() : what.substr(6);
			for (size_t i = 0; i < c.wep.elements.size(); ++i)
			{
				const WepElement& e = c.wep.elements[i];
				if (e.IsGame() || e.IsGround() || (!cls.empty() && e.Class() != cls))
					continue;
				g_ed.sel.push_back(ElementRef(ElementRef::ELEMENT, (int)i));
			}
		}
		else
		{
			int n = atoi(rest);
			if (n >= 0 && n < (int)c.wep.elements.size())
				g_ed.sel.assign(1, ElementRef(ElementRef::ELEMENT, n));
		}
		EdSelectionChanged();
	}
	else if (op == "frame")
	{
		ViewsFrameSelection();
	}
	else if (op == "drop")
	{
		DropSelection();
	}
	else if (op == "delete")
	{
		EdDeleteSelection();
	}
	else if (op == "duplicate")
	{
		EdDuplicateSelection();
	}
	else if (op == "par" && c.wep.Game())
	{
		c.Checkpoint("Set par");
		c.wep.Game()->SetInt("Par", atoi(rest));
		EdValuesChanged();
		PanelsRefreshHole();
	}
	else if ((op == "move" || op == "rot") && sscanf(rest, "%f %f %f", &a, &b, &d) == 3)
	{
		c.Checkpoint(op == "move" ? "Move" : "Rotate");
		for (size_t i = 0; i < g_ed.sel.size(); ++i)
		{
			if (g_ed.sel[i].kind != ElementRef::ELEMENT)
				continue;
			WepElement& e = c.wep.elements[g_ed.sel[i].index];
			if (op == "move" && e.Has("Pos"))
				e.SetVec("Pos", WVector(a, b, d));
			if (op == "rot" && e.Has("Rot"))
				e.SetVec("Rot", WVector(a, b, d), 1);
		}
		EdValuesChanged();
	}
	else if (op == "scale" && sscanf(rest, "%f", &a) == 1)
	{
		c.Checkpoint("Scale");
		for (size_t i = 0; i < g_ed.sel.size(); ++i)
			if (g_ed.sel[i].kind == ElementRef::ELEMENT && c.wep.elements[g_ed.sel[i].index].Has("Scale"))
				c.wep.elements[g_ed.sel[i].index].SetFloat("Scale", a);
		EdValuesChanged();
	}
	else if (op == "tool")
	{
		static const char* const NAMES[TOOL_COUNT] = { "select", "rotate", "scale", "puppet", "point", "camera", "area", "node" };
		for (int i = 0; i < TOOL_COUNT; ++i)
			if (!strcmp(rest, NAMES[i]))
				EdSetTool((Tool)i);
	}
	else if (op == "puppet")
	{
		g_ed.addPuppet = Utf8ToCp949(rest);
		EdSetTool(TOOL_ADD_PUPPET);
	}
	else if (op == "point")
	{
		char cls[64] = "", name[256] = "";
		sscanf(rest, "%63s %255[^\n]", cls, name);
		g_ed.addPointClass = cls;
		g_ed.addPoint = Utf8ToCp949(name);
		EdSetTool(TOOL_ADD_POINT);
	}
	else if (op == "node")
	{
		c.Checkpoint("Add node list");
		c.wep.nodes.push_back(WepMakeNode(Utf8ToCp949(rest)));
		g_ed.activeNode = (int)c.wep.nodes.size() - 1;
		g_ed.sel.assign(1, ElementRef(ElementRef::NODE, g_ed.activeNode));
		EdSetTool(TOOL_NODE);
		EdSceneChanged();
		EdSelectionChanged();
	}
	else if (op == "shade")
	{
		std::string s = rest;
		ViewsSetActiveShade(s == "wire" ? SHADE_WIRE : s == "flat" ? SHADE_FLAT : s == "surf" ? SHADE_SURFACE : SHADE_TEXTURED);
	}
	else if (op == "grid" && sscanf(rest, "%f", &a) == 1 && a > 0.0f)
	{
		g_ed.grid = a;
		StatusUpdate();
		ViewsRedraw();
	}
	else if (op == "look")
	{
		int n = atoi(rest);
		ViewsLookThrough(n >= 0 && n < (int)c.wep.elements.size() && c.wep.elements[n].IsCamera() ? n : -1);
	}
	else if (op == "fx" || op == "scripts")
	{
		bool& flag = op == "fx" ? g_ed.showFx : g_ed.showScripts;
		flag = !flag;
		SyncChecks();
		ViewsRedraw();
	}
	else if (op == "fxrestart")
	{
		AreaFxRestart();
	}
	else if (op == "fxrun")
	{
		// Steps the effects by hand, for a look at them without waiting.
		int steps = atoi(rest);
		for (int i = 0; i < steps; ++i)
			ViewsStepEffects(16.0f);
		char buf[64];
		sprintf_s(buf, "%d particles", AreaFxParticles());
		EdLog(buf);
		ViewsRedraw();
	}
	else if (op == "fog")
	{
		g_ed.showFog = !g_ed.showFog;
		SyncChecks();
		ViewsRedraw();
	}
	else if (op == "camera")
	{
		float yaw = 0, pitch = 0;
		if (sscanf(rest, "%f %f %f %f %f", &a, &b, &d, &yaw, &pitch) == 5)
			ViewsSetCamera(WVector(a, b, d), yaw, pitch);
	}
	else if (op == "fly")
	{
		ViewsFlyThrough(!ViewsFlying());
	}
	else if (op == "ground")
	{
		SetBaseModel(Utf8ToCp949(rest));
	}
	else if (op == "tab")
	{
		PanelsSwitchTab(atoi(rest));
	}
	else if (op == "rootdir")
	{
		c.Checkpoint("Set root");
		c.wep.rootDir = rest;
		TitleUpdate();
	}
	else if (op == "info")
	{
		char buf[256];
		int s = 0;
		for (size_t i = 0; i < c.wep.elements.size(); ++i)
			if (c.wep.elements[i].IsPuppet())
				++s;
		sprintf_s(buf, "%d elements, %d puppets placed, %d puppet files, %d node lists, grid %.0f", (int)c.wep.elements.size(), s, (int)c.wep.puppets.size(), (int)c.wep.nodes.size(), g_ed.grid);
		EdLog(buf);
	}
	else
	{
		EdLog("! Unknown command. Try help.");
	}
}

static LRESULT CALLBACK ConsoleProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static int history = -1;
	if (msg == WM_KEYDOWN && wp == VK_RETURN)
	{
		wchar_t buf[512];
		GetWindowTextW(hwnd, buf, 512);
		SetWindowTextW(hwnd, L"");
		std::string line = Narrow(buf);
		if (!line.empty())
		{
			g_consoleHistory.push_back(line);
			history = -1;
			EdRunConsole(line);
		}
		return 0;
	}
	if (msg == WM_KEYDOWN && (wp == VK_UP || wp == VK_DOWN) && !g_consoleHistory.empty())
	{
		int n = (int)g_consoleHistory.size();
		if (history < 0)
			history = n;
		history += wp == VK_UP ? -1 : 1;
		if (history < 0) history = 0;
		if (history >= n) history = n - 1;
		SetWindowTextW(hwnd, Wide(g_consoleHistory[history]).c_str());
		return 0;
	}
	if (msg == WM_CHAR && wp == VK_RETURN)
		return 0;
	return CallWindowProcW(g_consoleOld, hwnd, msg, wp, lp);
}

// -- tool tips -----------------------------------------------------------------

static HWND g_toolTips;

static const wchar_t* ToolTipText(int tool)
{
	switch (tool)
	{
	case TOOL_SELECT: return T(L"선택 / 이동\n클릭으로 선택, 끌어서 이동. Shift: 3D에서 위아래로 이동", L"Select / Move\nClick to select, drag to move. Shift: move vertically in the 3D pane");
	case TOOL_ROTATE: return T(L"회전\n끌어서 Y축 회전. Ctrl: X축, Shift: Z축", L"Rotate\nDrag to turn around Y. Ctrl: X axis, Shift: Z axis");
	case TOOL_SCALE: return T(L"크기\n퍼펫 크기 조절, 영역은 끌어서 크기 변경", L"Scale\nDrag to resize a puppet, or the side of an area");
	case TOOL_ADD_PUPPET: return T(L"퍼펫 추가\n퍼펫 탭에서 고른 뒤 코스를 클릭", L"Add Puppet\nPick one in the Puppets tab, then click on the course");
	case TOOL_ADD_POINT: return T(L"포인트 추가\n코스를 클릭. 종류는 액터 메뉴에서 선택", L"Add Point\nClick on the course. The Actor menu picks the class");
	case TOOL_ADD_CAMERA: return T(L"카메라 추가\n코스를 클릭. 먼 쪽 마름모를 끌어 방향 조절", L"Add Camera\nClick on the course. Drag its far diamond to aim it");
	case TOOL_ADD_AREA: return T(L"영역 추가\n끌어서 상자 그리기. 종류는 액터 메뉴에서 선택", L"Add Area\nDrag to draw the box. The Actor menu picks the class");
	default: return T(L"노드 편집\n클릭으로 선택한 노드 리스트에 점 추가, 점을 끌어 이동", L"Edit Nodes\nClick to add a point to the active node list, drag a point to move it");
	}
}

// A separate step so switching language can reuse it.
static void UpdateToolTips()
{
	for (int i = 0; i < TOOL_COUNT; ++i)
	{
		TOOLINFOW ti;
		memset(&ti, 0, sizeof(ti));
		ti.cbSize = sizeof(ti);
		ti.hwnd = g_ed.main;
		ti.uId = (UINT_PTR)g_toolButtons[i];
		ti.lpszText = (LPWSTR)ToolTipText(i);
		SendMessageW(g_toolTips, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);
	}
}

static void CreateToolTips()
{
	g_toolTips = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, 0, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, g_ed.main, 0, g_ed.inst, 0);
	SendMessageW(g_toolTips, WM_SETFONT, (WPARAM)g_font, 0);
	SendMessageW(g_toolTips, TTM_SETMAXTIPWIDTH, 0, 320);	// also what turns on line breaks
	for (int i = 0; i < TOOL_COUNT; ++i)
	{
		TOOLINFOW ti;
		memset(&ti, 0, sizeof(ti));
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		ti.hwnd = g_ed.main;
		ti.uId = (UINT_PTR)g_toolButtons[i];
		ti.lpszText = (LPWSTR)ToolTipText(i);
		SendMessageW(g_toolTips, TTM_ADDTOOLW, 0, (LPARAM)&ti);
	}
}

// -- menu ----------------------------------------------------------------------

static void Item(HMENU m, int id, const wchar_t* text)
{
	AppendMenuW(m, MF_STRING, id, text);
}

static void Sep(HMENU m)
{
	AppendMenuW(m, MF_SEPARATOR, 0, 0);
}

static void BuildMenu()
{
	HMENU bar = CreateMenu();
	HMENU f = CreatePopupMenu();
	Item(f, ID_NEW, T(L"새 홀\tCtrl+N", L"New Hole\tCtrl+N"));
	Item(f, ID_OPEN, T(L"열기...\tCtrl+O", L"Open...\tCtrl+O"));
	Item(f, ID_SAVE, T(L"저장\tCtrl+S", L"Save\tCtrl+S"));
	Item(f, ID_SAVEAS, T(L"다른 이름으로 저장...", L"Save As..."));
	Sep(f);
	Item(f, ID_EXPORT, T(L"GBIN으로 내보내기...\tF7", L"Export .gbin and .sbin...\tF7"));
	Sep(f);
	Item(f, ID_ROOT, T(L"게임 data 폴더...", L"Game Data Folder..."));
	Sep(f);
	Item(f, ID_EXIT, T(L"끝내기", L"Exit"));
	AppendMenuW(bar, MF_POPUP, (UINT_PTR)f, T(L"파일", L"&File"));

	HMENU e = CreatePopupMenu();
	Item(e, ID_UNDO, T(L"실행 취소\tCtrl+Z", L"Undo\tCtrl+Z"));
	Item(e, ID_REDO, T(L"다시 실행\tCtrl+Y", L"Redo\tCtrl+Y"));
	Sep(e);
	Item(e, ID_DUP, T(L"복제\tCtrl+D", L"Duplicate\tCtrl+D"));
	Item(e, ID_DEL, T(L"삭제\tDel", L"Delete\tDel"));
	Item(e, ID_SELALL, T(L"모두 선택\tCtrl+A", L"Select All\tCtrl+A"));
	Item(e, ID_DROP, T(L"지면에 붙이기\tEnd", L"Drop to Ground\tEnd"));
	Sep(e);
	Item(e, ID_NEWNODE, T(L"새 노드 리스트...", L"New Node List..."));
	Item(e, ID_SETBASE, T(L"퍼펫 탭의 선택을 지면으로", L"Use Browser Puppet as Base Model"));
	AppendMenuW(bar, MF_POPUP, (UINT_PTR)e, T(L"편집", L"&Edit"));

	HMENU v = CreatePopupMenu();
	Item(v, ID_FRAMEALL, T(L"전체 보기\tHome", L"Frame All\tHome"));
	Item(v, ID_FRAMESEL, T(L"선택 보기\tF", L"Frame Selection\tF"));
	Item(v, ID_MAX, T(L"창 최대화\tF11", L"Maximize Pane\tF11"));
	Sep(v);
	Item(v, ID_SHADE_WIRE, T(L"와이어\t1", L"Wireframe\t1"));
	Item(v, ID_SHADE_FLAT, T(L"단색\t2", L"Flat\t2"));
	Item(v, ID_SHADE_TEX, T(L"텍스처\t3", L"Textured\t3"));
	Item(v, ID_SHADE_SURF, T(L"지면 속성\t4", L"Surface Classes\t4"));
	Sep(v);
	Item(v, ID_SHOW_PUPPETS, T(L"퍼펫", L"Puppets"));
	Item(v, ID_SHOW_POINTS, T(L"포인트", L"Points"));
	Item(v, ID_SHOW_CAMERAS, T(L"카메라", L"Cameras"));
	Item(v, ID_SHOW_AREAS, T(L"영역", L"Areas"));
	Item(v, ID_SHOW_NODES, T(L"노드", L"Nodes"));
	Item(v, ID_SHOW_LABELS, T(L"이름표", L"Labels"));
	Sep(v);
	Item(v, ID_SHOW_SHADOW, T(L"그림자 맵", L"Shadow Maps"));
	Item(v, ID_SHOW_FOG, T(L"안개", L"Fog"));
	Item(v, ID_SHOW_SKY, T(L"하늘", L"Sky"));
	Item(v, ID_SHOW_FX, T(L"영역 이펙트", L"Area Effects"));
	Item(v, ID_FX_RESTART, T(L"이펙트 다시 시작", L"Restart Effects"));
	Item(v, ID_SHOW_SCRIPTS, T(L"스크립트 표시", L"Script Overlays"));
	Sep(v);
	Item(v, ID_LOOKCAM, T(L"선택한 카메라로 보기\tC", L"Look Through Selected Camera\tC"));
	Item(v, ID_FLY, T(L"auto 노드 따라 비행\tF5", L"Fly Along Auto Node\tF5"));
	AppendMenuW(bar, MF_POPUP, (UINT_PTR)v, T(L"보기", L"&View"));

	HMENU a = CreatePopupMenu();
	Item(a, ID_PT_POINT, L"WPoint");
	Item(a, ID_PT_SUN, L"WSun");
	Item(a, ID_PT_MOON, L"WMoon");
	Item(a, ID_PT_LIGHT, L"WGlobalLight");
	Item(a, ID_PT_LIGHT2, L"WSecondaryGlobalLight");
	Sep(a);
	Item(a, ID_AR_EVENT, L"WEventArea");
	Item(a, ID_AR_NPC, L"WNPC");
	Item(a, ID_AR_LAKE, L"WLake");
	Item(a, ID_AR_WIND, L"WWindArea");
	Item(a, ID_AR_SOUND, L"WSoundArea");
	Item(a, ID_AR_FX, L"WFxArea");
	AppendMenuW(bar, MF_POPUP, (UINT_PTR)a, T(L"액터", L"&Actor"));

	HMENU b = CreatePopupMenu();
	Item(b, ID_CHECK, T(L"맵 검사\tF8", L"Map Check\tF8"));
	Item(b, ID_BAKE, T(L"지면 조명 굽기", L"Light Base Model"));
	Item(b, ID_CLEARSHADOW, T(L"그림자 맵 지우기", L"Clear Shadow Maps"));
	Sep(b);
	Item(b, ID_BUILD, T(L"GBIN 빌드\tF7", L"Build .gbin\tF7"));
	AppendMenuW(bar, MF_POPUP, (UINT_PTR)b, T(L"빌드", L"&Build"));

	HMENU g = CreatePopupMenu();
	Item(g, ID_GRID1, L"1");
	Item(g, ID_GRID2, L"2");
	Item(g, ID_GRID4, L"4");
	Item(g, ID_GRID8, L"8");
	Item(g, ID_GRID16, L"16");
	Item(g, ID_GRID32, L"32");
	Sep(g);
	Item(g, ID_SNAP, T(L"격자에 맞추기\tG", L"Snap to Grid\tG"));
	Item(g, ID_SNAPGROUND, T(L"움직이면 지면에 붙이기\tH", L"Keep on Ground While Moving\tH"));
	AppendMenuW(bar, MF_POPUP, (UINT_PTR)g, T(L"격자", L"&Grid"));

	HMENU h = CreatePopupMenu();
	Item(h, ID_LANG_KO, L"\xD55C\xAD6D\xC5B4");
	Item(h, ID_LANG_EN, L"English");
	Sep(h);
	Item(h, ID_ABOUT, T(L"FairwayED 정보", L"About FairwayED"));
	AppendMenuW(bar, MF_POPUP, (UINT_PTR)h, T(L"도움말", L"&Help"));

	HMENU old = GetMenu(g_ed.main);
	SetMenu(g_ed.main, bar);
	if (old)
		DestroyMenu(old);
}

static void CheckItem(int id, bool on)
{
	CheckMenuItem(GetMenu(g_ed.main), id, on ? MF_CHECKED : MF_UNCHECKED);
}

static void SyncChecks()
{
	CheckItem(ID_SHOW_PUPPETS, g_ed.showPuppets);
	CheckItem(ID_SHOW_POINTS, g_ed.showPoints);
	CheckItem(ID_SHOW_CAMERAS, g_ed.showCameras);
	CheckItem(ID_SHOW_AREAS, g_ed.showAreas);
	CheckItem(ID_SHOW_NODES, g_ed.showNodes);
	CheckItem(ID_SHOW_LABELS, g_ed.showLabels);
	CheckItem(ID_SHOW_SHADOW, g_ed.showShadow);
	CheckItem(ID_SHOW_FOG, g_ed.showFog);
	CheckItem(ID_SHOW_SKY, g_ed.showSky);
	CheckItem(ID_SHOW_FX, g_ed.showFx);
	CheckItem(ID_SHOW_SCRIPTS, g_ed.showScripts);
	CheckItem(ID_SNAP, g_ed.snap);
	CheckItem(ID_SNAPGROUND, g_ed.snapGround);
	CheckItem(ID_LANG_KO, g_ed.korean);
	CheckItem(ID_LANG_EN, !g_ed.korean);
	static const struct { int id; float size; } GRIDS[] = { { ID_GRID1, 1 }, { ID_GRID2, 2 }, { ID_GRID4, 4 }, { ID_GRID8, 8 }, { ID_GRID16, 16 }, { ID_GRID32, 32 } };
	for (size_t i = 0; i < sizeof(GRIDS) / sizeof(GRIDS[0]); ++i)
		CheckItem(GRIDS[i].id, g_ed.grid == GRIDS[i].size);
	PaneShade s = ViewsActiveShade();
	CheckItem(ID_SHADE_WIRE, s == SHADE_WIRE);
	CheckItem(ID_SHADE_FLAT, s == SHADE_FLAT);
	CheckItem(ID_SHADE_TEX, s == SHADE_TEXTURED);
	CheckItem(ID_SHADE_SURF, s == SHADE_SURFACE);
}

void EdCommand(int id)
{
	Course& c = g_ed.course;
	if (id >= ID_TOOL_FIRST && id < ID_TOOL_FIRST + TOOL_COUNT)
	{
		EdSetTool((Tool)(id - ID_TOOL_FIRST));
		return;
	}
	bool* toggle = 0;
	switch (id)
	{
	case ID_NEW:
		if (ConfirmDiscard())
		{
			c.New();
			DocumentChanged();
			EdLog("New hole. Set a WGround puppet from the Puppets tab to give it a base model.");
		}
		break;
	case ID_OPEN:
		if (ConfirmDiscard())
		{
			std::wstring path;
			if (FileDialog(false, L"Course files (*.wep;*.gbin)\0*.wep;*.gbin\0All files (*.*)\0*.*\0", L"wep", &path))
				OpenPath(path);
		}
		break;
	case ID_SAVE: SaveProject(false); break;
	case ID_SAVEAS: SaveProject(true); break;
	case ID_EXPORT:
	case ID_BUILD: ExportDialog(); break;
	case ID_ROOT: PickRoot(); break;
	case ID_EXIT: SendMessageW(g_ed.main, WM_CLOSE, 0, 0); break;
	case ID_UNDO:
	case ID_REDO:
		if (id == ID_UNDO ? c.Undo() : c.Redo())
		{
			g_ed.sel.clear();
			EdSceneChanged();
		}
		break;
	case ID_DUP: EdDuplicateSelection(); break;
	case ID_DEL: EdDeleteSelection(); break;
	case ID_SELALL: EdRunConsole("select all"); break;
	case ID_DROP: DropSelection(); break;
	case ID_NEWNODE:
	{
		std::wstring name = L"ib0";
		if (EdPrompt(T(L"새 노드 리스트", L"New node list"), T(L"이름 (outline, ib, ob, auto, river, sea, wave, npc, camera...)", L"Name (outline, ib, ob, auto, river, sea, wave, npc, camera...)"), &name))
			EdRunConsole("node " + Narrow(name));
		break;
	}
	case ID_SETBASE: SetBaseModel(g_ed.addPuppet); break;
	case ID_FRAMEALL: ViewsFrameAll(); break;
	case ID_FRAMESEL: ViewsFrameSelection(); break;
	case ID_MAX: ViewsMaximizeActive(); break;
	case ID_SHADE_WIRE: ViewsSetActiveShade(SHADE_WIRE); break;
	case ID_SHADE_FLAT: ViewsSetActiveShade(SHADE_FLAT); break;
	case ID_SHADE_TEX: ViewsSetActiveShade(SHADE_TEXTURED); break;
	case ID_SHADE_SURF: ViewsSetActiveShade(SHADE_SURFACE); break;
	case ID_SHOW_PUPPETS: toggle = &g_ed.showPuppets; break;
	case ID_SHOW_POINTS: toggle = &g_ed.showPoints; break;
	case ID_SHOW_CAMERAS: toggle = &g_ed.showCameras; break;
	case ID_SHOW_AREAS: toggle = &g_ed.showAreas; break;
	case ID_SHOW_NODES: toggle = &g_ed.showNodes; break;
	case ID_SHOW_LABELS: toggle = &g_ed.showLabels; break;
	case ID_SHOW_SHADOW: toggle = &g_ed.showShadow; break;
	case ID_SHOW_FOG: toggle = &g_ed.showFog; break;
	case ID_SHOW_SKY: toggle = &g_ed.showSky; break;
	case ID_SHOW_FX: toggle = &g_ed.showFx; break;
	case ID_SHOW_SCRIPTS: toggle = &g_ed.showScripts; break;
	case ID_FX_RESTART: AreaFxRestart(); break;
	case ID_AR_WIND:
		g_ed.addAreaClass = "WWindArea";
		EdSetTool(TOOL_ADD_AREA);
		break;
	case ID_LOOKCAM:
		if (!g_ed.sel.empty() && g_ed.sel[0].kind == ElementRef::ELEMENT && c.wep.elements[g_ed.sel[0].index].IsCamera())
			ViewsLookThrough(g_ed.sel[0].index);
		break;
	case ID_FLY: ViewsFlyThrough(!ViewsFlying()); break;
	case ID_CHECK:
	{
		CourseLog log;
		c.Validate(&log);
		EdFlush(log);
		break;
	}
	case ID_BAKE:
	{
		CourseLog log;
		HCURSOR old = SetCursor(LoadCursor(0, IDC_WAIT));
		c.BakeBaseColors(&log);
		SetCursor(old);
		EdFlush(log);
		SceneInvalidateBase();
		ViewsRedraw();
		TitleUpdate();
		break;
	}
	case ID_CLEARSHADOW:
	{
		CourseLog log;
		c.ClearShadowMaps(&log);
		EdFlush(log);
		ViewsRedraw();
		TitleUpdate();
		break;
	}
	case ID_GRID1: g_ed.grid = 1; break;
	case ID_GRID2: g_ed.grid = 2; break;
	case ID_GRID4: g_ed.grid = 4; break;
	case ID_GRID8: g_ed.grid = 8; break;
	case ID_GRID16: g_ed.grid = 16; break;
	case ID_GRID32: g_ed.grid = 32; break;
	case ID_SNAP: toggle = &g_ed.snap; break;
	case ID_SNAPGROUND: toggle = &g_ed.snapGround; break;
	case ID_PT_POINT: case ID_PT_SUN: case ID_PT_MOON: case ID_PT_LIGHT: case ID_PT_LIGHT2:
	{
		static const char* const NAMES[] = { "WPoint", "WSun", "WMoon", "WGlobalLight", "WSecondaryGlobalLight" };
		g_ed.addPointClass = NAMES[id - ID_PT_POINT];
		g_ed.addPoint.clear();
		EdSetTool(TOOL_ADD_POINT);
		break;
	}
	case ID_AR_EVENT: case ID_AR_NPC: case ID_AR_LAKE: case ID_AR_SOUND: case ID_AR_FX:
	{
		static const char* const NAMES[] = { "WEventArea", "WNPC", "WLake", "WSoundArea", "WFxArea" };
		g_ed.addAreaClass = NAMES[id - ID_AR_EVENT];
		EdSetTool(TOOL_ADD_AREA);
		break;
	}
	case ID_LANG_KO:
	case ID_LANG_EN:
		g_ed.korean = id == ID_LANG_KO;
		WriteSetting(L"Language", g_ed.korean ? "ko" : "en");
		BuildMenu();
		PanelsLanguage();
		UpdateToolTips();
		PanelsRefreshScene();
		PanelsRefreshHole();
		ViewsRedraw();
		break;
	case ID_ABOUT:
		MessageBoxW(g_ed.main, T(L"FairwayED\n\n왕리얼 엔진 맵 에디터 WangED에서 영감을 받아 상상으로 만든 프로그램입니다. 엔트리브와 관계없습니다.",
			L"FairwayED\n\nAn imagined course editor inspired by WangED, the WangReal Engine map editor. Not Ntreev's code and no connection to Ntreev."), L"FairwayED", MB_ICONINFORMATION);
		break;
	}
	if (toggle)
	{
		*toggle = !*toggle;
		ViewsRedraw();
	}
	SyncChecks();
	StatusUpdate();
}

// -- tool buttons --------------------------------------------------------------

static void DrawToolGlyph(HDC dc, const RECT& rc, int tool, COLORREF color)
{
	HPEN pen = CreatePen(PS_SOLID, 2, color);
	HBRUSH brush = CreateSolidBrush(color);
	HGDIOBJ oldPen = SelectObject(dc, pen);
	HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
	int cx = (rc.left + rc.right) / 2, cy = (rc.top + rc.bottom) / 2;
	switch (tool)
	{
	case TOOL_SELECT:
	{
		POINT p[] = { { cx - 7, cy - 9 }, { cx - 7, cy + 7 }, { cx - 3, cy + 3 }, { cx, cy + 9 }, { cx + 3, cy + 8 }, { cx, cy + 2 }, { cx + 5, cy + 1 } };
		SelectObject(dc, brush);
		Polygon(dc, p, 7);
		break;
	}
	case TOOL_ROTATE:
		Arc(dc, cx - 9, cy - 9, cx + 9, cy + 9, cx + 9, cy - 3, cx + 3, cy - 9);
		MoveToEx(dc, cx + 9, cy - 3, 0);
		LineTo(dc, cx + 5, cy - 5);
		MoveToEx(dc, cx + 9, cy - 3, 0);
		LineTo(dc, cx + 10, cy - 8);
		break;
	case TOOL_SCALE:
		Rectangle(dc, cx - 9, cy - 1, cx + 1, cy + 9);
		MoveToEx(dc, cx - 2, cy + 2, 0);
		LineTo(dc, cx + 8, cy - 8);
		MoveToEx(dc, cx + 8, cy - 8, 0);
		LineTo(dc, cx + 2, cy - 8);
		MoveToEx(dc, cx + 8, cy - 8, 0);
		LineTo(dc, cx + 8, cy - 2);
		break;
	case TOOL_ADD_PUPPET:
		// A tree: the thing placed most.
		MoveToEx(dc, cx, cy + 9, 0);
		LineTo(dc, cx, cy + 2);
		SelectObject(dc, brush);
		Ellipse(dc, cx - 7, cy - 9, cx + 7, cy + 4);
		break;
	case TOOL_ADD_POINT:
	{
		POINT p[] = { { cx, cy - 9 }, { cx + 8, cy }, { cx, cy + 9 }, { cx - 8, cy } };
		Polygon(dc, p, 4);
		SelectObject(dc, brush);
		Ellipse(dc, cx - 2, cy - 2, cx + 3, cy + 3);
		break;
	}
	case TOOL_ADD_CAMERA:
		SelectObject(dc, brush);
		Rectangle(dc, cx - 9, cy - 5, cx + 3, cy + 6);
		{
			POINT p[] = { { cx + 3, cy }, { cx + 9, cy - 5 }, { cx + 9, cy + 6 } };
			Polygon(dc, p, 3);
		}
		break;
	case TOOL_ADD_AREA:
		Rectangle(dc, cx - 9, cy - 5, cx + 5, cy + 9);
		MoveToEx(dc, cx - 9, cy - 5, 0);
		LineTo(dc, cx - 4, cy - 9);
		LineTo(dc, cx + 9, cy - 9);
		LineTo(dc, cx + 9, cy + 5);
		LineTo(dc, cx + 5, cy + 9);
		MoveToEx(dc, cx + 9, cy - 9, 0);
		LineTo(dc, cx + 5, cy - 5);
		break;
	default:
		MoveToEx(dc, cx - 9, cy + 7, 0);
		LineTo(dc, cx - 3, cy - 5);
		LineTo(dc, cx + 3, cy + 3);
		LineTo(dc, cx + 9, cy - 8);
		SelectObject(dc, brush);
		Rectangle(dc, cx - 11, cy + 5, cx - 7, cy + 9);
		Rectangle(dc, cx - 5, cy - 7, cx - 1, cy - 3);
		Rectangle(dc, cx + 1, cy + 1, cx + 5, cy + 5);
		Rectangle(dc, cx + 7, cy - 10, cx + 11, cy - 6);
		break;
	}
	SelectObject(dc, oldBrush);
	SelectObject(dc, oldPen);
	DeleteObject(pen);
	DeleteObject(brush);
}

// -- layout / window -----------------------------------------------------------

// Moving children one at a time lets a growing pane paint over the right-hand controls, and
// MoveWindow then carries those pixels along with them. So nothing draws until all are placed.
static void Layout()
{
	bool visible = IsWindowVisible(g_ed.main) != 0;
	if (visible)
		SendMessageW(g_ed.main, WM_SETREDRAW, FALSE, 0);
	RECT rc;
	GetClientRect(g_ed.main, &rc);
	SendMessageW(g_status, WM_SIZE, 0, 0);
	RECT sr;
	GetWindowRect(g_status, &sr);
	int statusH = sr.bottom - sr.top;
	int parts[4] = { 110, 340, 500, -1 };
	SendMessageW(g_status, SB_SETPARTS, 4, (LPARAM)parts);
	int h = rc.bottom - statusH;
	int w = rc.right;
	for (int i = 0; i < TOOL_COUNT; ++i)
		MoveWindow(g_toolButtons[i], 2, 2 + i * 36, LEFTBAR - 4, 34, TRUE);
	int vw = w - LEFTBAR - RIGHTPANEL - 2;
	ViewsLayout(LEFTBAR, 0, vw > 10 ? vw : 10, h - BOTTOM - 2 > 10 ? h - BOTTOM - 2 : 10);
	MoveWindow(g_log, LEFTBAR, h - BOTTOM, vw, BOTTOM - 24, TRUE);
	MoveWindow(g_console, LEFTBAR, h - 22, vw, 22, TRUE);
	PanelsLayout(w - RIGHTPANEL, 0, RIGHTPANEL, h);
	// WM_SETREDRAW TRUE would also make a window that isn't shown yet count as visible.
	if (visible)
	{
		SendMessageW(g_ed.main, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(g_ed.main, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
	}
}

static LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_SIZE:
		if (g_status)
			Layout();
		return 0;
	case WM_COMMAND:
	{
		int id = LOWORD(wp), code = HIWORD(wp);
		if (PanelsCommand(id, code))
			return 0;
		if (lp && id >= ID_TOOL_FIRST && id < ID_TOOL_FIRST + TOOL_COUNT)
		{
			EdSetTool((Tool)(id - ID_TOOL_FIRST));
			return 0;
		}
		if (!lp || code == BN_CLICKED)
			EdCommand(id);
		return 0;
	}
	case WM_DRAWITEM:
	{
		DRAWITEMSTRUCT* d = (DRAWITEMSTRUCT*)lp;
		int tool = d->CtlID - ID_TOOL_FIRST;
		if (tool >= 0 && tool < TOOL_COUNT)
		{
			bool active = g_ed.tool == tool;
			FillRect(d->hDC, &d->rcItem, GetSysColorBrush(active ? COLOR_3DSHADOW : COLOR_BTNFACE));
			DrawEdge(d->hDC, &d->rcItem, active || (d->itemState & ODS_SELECTED) ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT);
			DrawToolGlyph(d->hDC, d->rcItem, tool, active ? RGB(255, 255, 255) : RGB(30, 30, 60));
			return TRUE;
		}
		break;
	}
	case WM_NOTIFY:
		if (PanelsNotify((NMHDR*)lp))
			return 0;
		break;
	case WM_TIMER:
		ViewsTick();
		return 0;
	case WM_DROPFILES:
	{
		wchar_t path[MAX_PATH];
		if (DragQueryFileW((HDROP)wp, 0, path, MAX_PATH) && ConfirmDiscard())
			OpenPath(path);
		DragFinish((HDROP)wp);
		return 0;
	}
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

static bool HandleKey(const MSG& m)
{
	if (m.message != WM_KEYDOWN)
		return false;
	bool ctrl = GetKeyState(VK_CONTROL) < 0;
	wchar_t cls[32] = L"";
	GetClassNameW(m.hwnd, cls, 32);
	bool typing = !_wcsicmp(cls, L"EDIT");
	bool viewFocus = !wcscmp(cls, L"FairwayED_Pane");
	int id = 0;
	switch (m.wParam)
	{
	case 'N': if (ctrl) id = ID_NEW; break;
	case 'O': if (ctrl) id = ID_OPEN; break;
	case 'S': if (ctrl) id = ID_SAVE; break;
	case 'Z': if (ctrl && !typing) id = ID_UNDO; break;
	case 'Y': if (ctrl && !typing) id = ID_REDO; break;
	case 'D': if (ctrl && !typing) id = ID_DUP; break;
	case 'A': if (ctrl && !typing) id = ID_SELALL; break;
	case VK_F5: id = ID_FLY; break;
	case VK_F7: id = ID_EXPORT; break;
	case VK_F8: id = ID_CHECK; break;
	case VK_F11: id = ID_MAX; break;
	case VK_DELETE: if (!typing) id = ID_DEL; break;
	case VK_HOME: if (!typing) id = ID_FRAMEALL; break;
	case VK_END: if (!typing) id = ID_DROP; break;
	case 'F': if (viewFocus && !ctrl) id = ID_FRAMESEL; break;
	case 'G': if (viewFocus && !ctrl) id = ID_SNAP; break;
	case 'H': if (viewFocus && !ctrl) id = ID_SNAPGROUND; break;
	case 'C': if (viewFocus && !ctrl) id = ID_LOOKCAM; break;
	case '1': if (viewFocus) id = ID_SHADE_WIRE; break;
	case '2': if (viewFocus) id = ID_SHADE_FLAT; break;
	case '3': if (viewFocus) id = ID_SHADE_TEX; break;
	case '4': if (viewFocus) id = ID_SHADE_SURF; break;
	case VK_ESCAPE:
		if (viewFocus)
		{
			EdSetTool(TOOL_SELECT);
			return true;
		}
		break;
	}
	if (!id)
		return false;
	EdCommand(id);
	return true;
}

// Writes this window to a PNG. Only its own contents: controls paint themselves into the
// bitmap and the panes are read back from OpenGL, so whatever covers it stays out of the file.
static void SaveShot(const std::wstring& path)
{
	RECT rc;
	GetWindowRect(g_ed.main, &rc);
	int w = rc.right - rc.left, h = rc.bottom - rc.top;
	HDC screen = GetDC(0);
	HDC mem = CreateCompatibleDC(screen);
	HBITMAP bmp = CreateCompatibleBitmap(screen, w, h);
	HGDIOBJ old = SelectObject(mem, bmp);
	PrintWindow(g_ed.main, mem, 0);
	ViewsCaptureInto(mem, rc.left, rc.top);
	SelectObject(mem, old);
	{
		Gdiplus::Bitmap image(bmp, 0);
		CLSID png;
		CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &png);
		image.Save(path.c_str(), &png, 0);
	}
	DeleteObject(bmp);
	DeleteDC(mem);
	ReleaseDC(0, screen);
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int show)
{
	g_ed.inst = inst;
	OleInitialize(0);
	INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_TREEVIEW_CLASSES | ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_TAB_CLASSES };
	InitCommonControlsEx(&icc);
	TextureStartup();

	std::string root = ReadSetting(L"DataRoot");
	g_ed.korean = ReadSetting(L"Language") == "ko";
	int argc = 0;
	wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	std::wstring openFile, execArg, shotArg;
	for (int i = 1; i < argc; ++i)
	{
		if (!wcscmp(argv[i], L"/root") && i + 1 < argc)
		{
			root = Narrow(argv[++i]);
		}
		else if (!wcscmp(argv[i], L"/lang") && i + 1 < argc)
		{
			g_ed.korean = !wcscmp(argv[++i], L"ko");
		}
		else if (!wcscmp(argv[i], L"/exec") && i + 1 < argc)
		{
			execArg = argv[++i];
		}
		else if (!wcscmp(argv[i], L"/shot") && i + 1 < argc)
		{
			shotArg = argv[++i];
		}
		else
		{
			openFile = argv[i];
		}
	}
	g_ed.course.SetDataRoot(root);
	g_font = CreateFontW(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"Gulim");

	WNDCLASSW wc;
	memset(&wc, 0, sizeof(wc));
	wc.lpfnWndProc = MainProc;
	wc.hInstance = inst;
	wc.hIcon = LoadIconW(inst, MAKEINTRESOURCEW(1));
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = L"FairwayED_Main";
	RegisterClassW(&wc);
	wc.lpfnWndProc = PromptProc;
	wc.hIcon = 0;
	wc.lpszClassName = L"FairwayED_Prompt";
	RegisterClassW(&wc);
	ViewsRegister(inst);

	g_ed.main = CreateWindowExW(WS_EX_ACCEPTFILES, L"FairwayED_Main", L"FairwayED", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, 1440, 920, 0, 0, inst, 0);
	g_status = CreateWindowExW(0, STATUSCLASSNAMEW, L"", WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, g_ed.main, 0, inst, 0);
	SendMessageW(g_status, WM_SETFONT, (WPARAM)g_font, 0);
	for (int i = 0; i < TOOL_COUNT; ++i)
		g_toolButtons[i] = CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 0, 0, 10, 10, g_ed.main, (HMENU)(INT_PTR)(ID_TOOL_FIRST + i), inst, 0);
	CreateToolTips();
	g_log = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL, 0, 0, 10, 10, g_ed.main, 0, inst, 0);
	SendMessageW(g_log, WM_SETFONT, (WPARAM)g_font, 0);
	g_console = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 10, 10, g_ed.main, (HMENU)ID_CONSOLE, inst, 0);
	SendMessageW(g_console, WM_SETFONT, (WPARAM)g_font, 0);
	g_consoleOld = (WNDPROC)SetWindowLongPtrW(g_console, GWLP_WNDPROC, (LONG_PTR)ConsoleProc);
	ViewsCreate(g_ed.main);
	PanelsCreate(g_ed.main);
	BuildMenu();
	SyncChecks();
	Layout();
	ShowWindow(g_ed.main, show);
	UpdateWindow(g_ed.main);
	SetTimer(g_ed.main, 1, 16, 0);
	EdLog("FairwayED ready. Type help below for the console.");
	if (root.empty())
		EdLog("! No game data folder set. File > Game Data Folder, or /root, lets puppets and textures resolve.");
	if (!openFile.empty())
		OpenPath(openFile);
	else
	{
		g_ed.course.New();
		DocumentChanged();
	}
	if (!execArg.empty())
	{
		std::string all = Narrow(execArg);
		size_t at = 0;
		while (at <= all.size())
		{
			size_t e = all.find(';', at);
			if (e == std::string::npos)
				e = all.size();
			EdRunConsole(all.substr(at, e - at));
			at = e + 1;
		}
	}
	if (!shotArg.empty())
	{
		ViewsRedraw();
		// Let everything paint once first.
		DWORD until = GetTickCount() + 1500;
		MSG pump;
		while (GetTickCount() < until)
		{
			while (PeekMessageW(&pump, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&pump);
				DispatchMessageW(&pump);
			}
			Sleep(10);
		}
		SaveShot(shotArg);
		DestroyWindow(g_ed.main);
	}

	MSG m;
	while (GetMessageW(&m, 0, 0, 0))
	{
		if (HandleKey(m))
			continue;
		TranslateMessage(&m);
		DispatchMessageW(&m);
	}
	TextureShutdown();
	return 0;
}
