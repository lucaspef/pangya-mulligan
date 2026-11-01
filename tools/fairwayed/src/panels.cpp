// SPDX-License-Identifier: MIT
// panels.cpp - the right-hand side: the scene tree and property grid, the puppet browser,
// the surface table and the hole sheet.
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <algorithm>
#include "areascript.h"
#include "scene.h"
#include "wcommon.h"

enum
{
	IDC_TABS = 5000, IDC_TREE, IDC_PROPS, IDC_PROP_EDIT,
	IDC_BROWSE_FILTER, IDC_BROWSE_LIST, IDC_BROWSE_LOCAL, IDC_BROWSE_PLACE, IDC_BROWSE_REPLACE,
	IDC_SURF_CLASSES, IDC_SURF_TEX, IDC_SURF_ASSIGN, IDC_SURF_ADD, IDC_SURF_DEL, IDC_SURF_SAVE, IDC_SURF_SHOW,
	IDC_HOLE_SLOTS, IDC_HOLE_PLACE, IDC_HOLE_GOTO, IDC_HOLE_SUN, IDC_HOLE_DESC, IDC_HOLE_APPLY, IDC_HOLE_STATS
};

static HWND g_tabs, g_tree, g_props, g_edit;
static HWND g_filter, g_browse, g_local, g_place, g_replace, g_preview;
static HWND g_classes, g_texlist, g_assign, g_addClass, g_delClass, g_saveXml, g_showSurf;
static HWND g_slots, g_holePlace, g_holeGoto, g_sunLabel, g_sun, g_descLabel, g_desc, g_holeApply, g_stats;
static HFONT g_font;
static int g_x, g_y, g_w, g_h;
static int g_tab;
static bool g_syncing;
static std::vector<std::string> g_browseNames;

struct PropRow { std::string key; bool isColor, isFlag; };
static std::vector<PropRow> g_rows;
static int g_editRow = -1;
static WNDPROC g_editOld;

static void Fnt(HWND h)
{
	SendMessageW(h, WM_SETFONT, (WPARAM)g_font, TRUE);
}

static HWND Make(const wchar_t* cls, const wchar_t* text, DWORD style, int id, HWND parent, DWORD ex = 0)
{
	HWND h = CreateWindowExW(ex, cls, text, WS_CHILD | WS_CLIPSIBLINGS | style, 0, 0, 10, 10, parent, (HMENU)(INT_PTR)id, g_ed.inst, 0);
	Fnt(h);
	return h;
}

static void Column(HWND list, int index, const wchar_t* title, int width)
{
	LVCOLUMNW c;
	memset(&c, 0, sizeof(c));
	c.mask = LVCF_TEXT | LVCF_WIDTH;
	c.pszText = (LPWSTR)title;
	c.cx = width;
	SendMessageW(list, LVM_INSERTCOLUMNW, index, (LPARAM)&c);
}

static int AddRow(HWND list, const std::wstring& a, const std::wstring& b)
{
	LVITEMW it;
	memset(&it, 0, sizeof(it));
	it.mask = LVIF_TEXT;
	it.iItem = ListView_GetItemCount(list);
	it.pszText = (LPWSTR)a.c_str();
	int row = (int)SendMessageW(list, LVM_INSERTITEMW, 0, (LPARAM)&it);
	it.iSubItem = 1;
	it.iItem = row;
	it.pszText = (LPWSTR)b.c_str();
	SendMessageW(list, LVM_SETITEMTEXTW, row, (LPARAM)&it);
	return row;
}

static void SubItem(HWND list, int row, int col, const std::wstring& s)
{
	LVITEMW it;
	memset(&it, 0, sizeof(it));
	it.iSubItem = col;
	it.pszText = (LPWSTR)s.c_str();
	SendMessageW(list, LVM_SETITEMTEXTW, row, (LPARAM)&it);
}

// -- labels -------------------------------------------------------------------

std::wstring ElementLabel(const WepElement& e, int index)
{
	wchar_t buf[32];
	swprintf_s(buf, L"[%d] ", index);
	std::wstring s = buf;
	if (e.IsGame())
		return s + L"GAME";
	std::string cls = e.Class();
	if (e.IsPuppet())
		return s + WideCp(PathBaseName(e.Get("Filename"))) + L"  (" + Wide(cls) + L")";
	std::wstring name = WideCp(e.Get("Name"));
	if (name.empty())
		name = Wide(cls);
	else if (e.IsPoint() || e.IsArea())
		name += L"  (" + Wide(cls) + L")";
	return s + name;
}

// -- scene tree ---------------------------------------------------------------

static LPARAM TreeData(ElementRef::Kind kind, int index)
{
	return ((LPARAM)kind << 24) | (index & 0xFFFFFF);
}

static HTREEITEM TreeAdd(HTREEITEM parent, const std::wstring& text, LPARAM data)
{
	TVINSERTSTRUCTW ins;
	memset(&ins, 0, sizeof(ins));
	ins.hParent = parent;
	ins.hInsertAfter = TVI_LAST;
	ins.item.mask = TVIF_TEXT | TVIF_PARAM;
	ins.item.pszText = (LPWSTR)text.c_str();
	ins.item.lParam = data;
	return (HTREEITEM)SendMessageW(g_tree, TVM_INSERTITEMW, 0, (LPARAM)&ins);
}

void PanelsRefreshScene()
{
	g_syncing = true;
	SendMessageW(g_tree, WM_SETREDRAW, FALSE, 0);
	TreeView_DeleteAllItems(g_tree);
	Course& c = g_ed.course;
	const std::vector<WepElement>& els = c.wep.elements;
	HTREEITEM cams = TreeAdd(TVI_ROOT, T(L"카메라", L"Cameras"), -1);
	HTREEITEM points = TreeAdd(TVI_ROOT, T(L"포인트", L"Points"), -1);
	HTREEITEM areas = TreeAdd(TVI_ROOT, T(L"영역", L"Areas"), -1);
	HTREEITEM nodes = TreeAdd(TVI_ROOT, T(L"노드 리스트", L"Node lists"), -1);
	HTREEITEM puppets = TreeAdd(TVI_ROOT, T(L"퍼펫", L"Puppets"), -1);
	for (size_t i = 0; i < els.size(); ++i)
	{
		const WepElement& e = els[i];
		std::wstring label = ElementLabel(e, (int)i);
		LPARAM data = TreeData(ElementRef::ELEMENT, (int)i);
		if (e.IsGame())
		{
			TVINSERTSTRUCTW ins;
			memset(&ins, 0, sizeof(ins));
			ins.hParent = TVI_ROOT;
			ins.hInsertAfter = TVI_FIRST;
			ins.item.mask = TVIF_TEXT | TVIF_PARAM;
			ins.item.pszText = (LPWSTR)label.c_str();
			ins.item.lParam = data;
			SendMessageW(g_tree, TVM_INSERTITEMW, 0, (LPARAM)&ins);
		}
		else if (e.IsCamera())
			TreeAdd(cams, label, data);
		else if (e.IsPoint())
			TreeAdd(points, label, data);
		else if (e.IsArea())
			TreeAdd(areas, label, data);
		else if (e.IsPuppet())
			TreeAdd(puppets, label, data);
	}
	for (size_t i = 0; i < c.wep.nodes.size(); ++i)
	{
		wchar_t buf[64];
		swprintf_s(buf, L"[%d] ", (int)i);
		std::wstring label = buf + WideCp(c.wep.nodes[i].Get("Name"));
		swprintf_s(buf, L"  (%d)", (int)c.wep.nodes[i].points.size());
		TreeAdd(nodes, label + buf, TreeData(ElementRef::NODE, (int)i));
	}
	TreeView_Expand(g_tree, cams, TVE_EXPAND);
	TreeView_Expand(g_tree, points, TVE_EXPAND);
	TreeView_Expand(g_tree, nodes, TVE_EXPAND);
	TreeView_Expand(g_tree, areas, TVE_EXPAND);
	SendMessageW(g_tree, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(g_tree, 0, TRUE);
	g_syncing = false;
}

static HTREEITEM FindTreeItem(HTREEITEM from, LPARAM data)
{
	for (HTREEITEM i = from; i; i = TreeView_GetNextSibling(g_tree, i))
	{
		TVITEMW it;
		it.mask = TVIF_PARAM;
		it.hItem = i;
		SendMessageW(g_tree, TVM_GETITEMW, 0, (LPARAM)&it);
		if (it.lParam == data)
			return i;
		HTREEITEM child = TreeView_GetChild(g_tree, i);
		if (child)
		{
			HTREEITEM f = FindTreeItem(child, data);
			if (f)
				return f;
		}
	}
	return 0;
}

void PanelsSyncTreeSelection()
{
	if (g_ed.sel.empty())
		return;
	const ElementRef& r = g_ed.sel[0];
	LPARAM data = r.kind == ElementRef::ELEMENT ? TreeData(ElementRef::ELEMENT, r.index) : TreeData(ElementRef::NODE, r.index);
	HTREEITEM item = FindTreeItem(TreeView_GetRoot(g_tree), data);
	if (item)
	{
		g_syncing = true;
		TreeView_SelectItem(g_tree, item);
		g_syncing = false;
	}
}

// -- property grid ------------------------------------------------------------

static WepFields* FieldsOf(const ElementRef& r)
{
	Course& c = g_ed.course;
	if (r.kind == ElementRef::ELEMENT && r.index < (int)c.wep.elements.size())
		return &c.wep.elements[r.index];
	if ((r.kind == ElementRef::NODE || r.kind == ElementRef::NODE_POINT) && r.index < (int)c.wep.nodes.size())
		return &c.wep.nodes[r.index];
	return 0;
}

void PanelsRefreshProps()
{
	SendMessageW(g_props, WM_SETREDRAW, FALSE, 0);
	ListView_DeleteAllItems(g_props);
	g_rows.clear();
	if (!g_ed.sel.empty())
	{
		const ElementRef& r = g_ed.sel[0];
		WepFields* f = FieldsOf(r);
		if (f)
		{
			for (size_t i = 0; i < f->fields.size(); ++i)
			{
				const WepField& field = f->fields[i];
				if (field.key == "Num")
					continue;
				PropRow row;
				row.key = field.key;
				row.isColor = field.key == "Ambient" || field.key == "Diffuse" || field.key == "Ambient2" || field.key == "Shadow";
				row.isFlag = field.key == "FlagShow" || field.key == "NoLOD" || field.key == "ForceOpaque" || field.key == "Show" ||
					field.key == "NoFog" || field.key == "NoAlphaSort" || field.key == "NoShadow";
				g_rows.push_back(row);
				std::wstring value = WideCp(field.value);
				if (row.isFlag)
					value = field.value == "0" ? L"\x2610  0" : L"\x2611  1";
				AddRow(g_props, Wide(field.key), value);
			}
			if (r.kind == ElementRef::NODE_POINT)
			{
				PropRow row;
				row.key = "Npos";
				row.isColor = row.isFlag = false;
				g_rows.push_back(row);
				AddRow(g_props, L"Npos", Wide(g_ed.course.wep.nodes[r.index].points[r.point].text));
			}
			// What the client makes of the script, read-only: the Name row is what gets edited.
			std::vector<ScriptRow> decoded;
			if (r.kind == ElementRef::ELEMENT && g_ed.course.wep.elements[r.index].IsArea())
			{
				AreaScript s = ParseAreaScript(g_ed.course.wep.elements[r.index]);
				decoded = s.rows;
				for (size_t p = 0; p < s.problems.size(); ++p)
				{
					ScriptRow bad;
					bad.label = "Problem";
					bad.value = s.problems[p];
					decoded.push_back(bad);
				}
			}
			else if (r.kind != ElementRef::ELEMENT)
			{
				NodeScript s = ParseNodeScript(f->Get("Name"));
				if (!s.kind.empty())
				{
					ScriptRow use;
					use.label = "Use";
					use.value = s.kind;
					decoded.push_back(use);
				}
				decoded.insert(decoded.end(), s.rows.begin(), s.rows.end());
			}
			for (size_t d = 0; d < decoded.size(); ++d)
			{
				PropRow row;
				row.key = "";
				row.isColor = row.isFlag = false;
				g_rows.push_back(row);
				AddRow(g_props, L"\x25B8 " + Wide(decoded[d].label), Wide(decoded[d].value));
			}
		}
	}
	SendMessageW(g_props, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(g_props, 0, TRUE);
}

// A value goes to every selected thing that has the key.
static void ApplyProp(const std::string& key, const std::string& value)
{
	Course& c = g_ed.course;
	c.Checkpoint("Set " + key);
	for (size_t i = 0; i < g_ed.sel.size(); ++i)
	{
		const ElementRef& r = g_ed.sel[i];
		if (key == "Npos")
		{
			if (r.kind == ElementRef::NODE_POINT)
			{
				WVector v;
				sscanf(value.c_str(), "%f %f %f", &v.x, &v.y, &v.z);
				c.wep.nodes[r.index].points[r.point].Set(v);
			}
			continue;
		}
		WepFields* f = FieldsOf(r);
		if (f && f->Has(key))
			f->Set(key, value);
	}
	if (key == "Filename")
		c.wep.SyncPuppetList();
	if (key == "Ambient" || key == "Diffuse" || key == "Ambient2" || key == "Shadow")
		SceneInvalidateBase();
	if (key == "Name" || key == "Filename" || key == "Class")
		EdSceneChanged();
	else
		EdValuesChanged();
	PanelsRefreshProps();
	ViewsRedraw();
}

static void CommitEdit(bool apply)
{
	if (g_editRow < 0)
		return;
	int row = g_editRow;
	g_editRow = -1;
	wchar_t buf[512];
	GetWindowTextW(g_edit, buf, 512);
	ShowWindow(g_edit, SW_HIDE);
	if (apply && row < (int)g_rows.size())
	{
		std::string value = NarrowCp(buf);
		ApplyProp(g_rows[row].key, value);
	}
}

static LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_KEYDOWN && wp == VK_RETURN)
	{
		CommitEdit(true);
		return 0;
	}
	if (msg == WM_KEYDOWN && wp == VK_ESCAPE)
	{
		CommitEdit(false);
		return 0;
	}
	if (msg == WM_KILLFOCUS)
		CommitEdit(true);
	return CallWindowProcW(g_editOld, hwnd, msg, wp, lp);
}

static void PropDoubleClick(int row)
{
	if (row < 0 || row >= (int)g_rows.size() || g_ed.sel.empty())
		return;
	const PropRow& p = g_rows[row];
	WepFields* f = FieldsOf(g_ed.sel[0]);
	if (!f || p.key.empty())
		return;
	if (p.isFlag)
	{
		ApplyProp(p.key, f->Get(p.key) == "0" ? "1" : "0");
		return;
	}
	if (p.isColor)
	{
		int r = 128, g = 128, b = 128;
		sscanf(f->Get(p.key).c_str(), "%d %d %d", &r, &g, &b);
		static COLORREF custom[16];
		CHOOSECOLORW cc;
		memset(&cc, 0, sizeof(cc));
		cc.lStructSize = sizeof(cc);
		cc.hwndOwner = g_ed.main;
		cc.rgbResult = RGB(r, g, b);
		cc.lpCustColors = custom;
		cc.Flags = CC_RGBINIT | CC_FULLOPEN;
		if (ChooseColorW(&cc))
		{
			char buf[32];
			sprintf_s(buf, "%d %d %d", GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult));
			ApplyProp(p.key, buf);
		}
		return;
	}
	RECT rc;
	ListView_GetSubItemRect(g_props, row, 1, LVIR_BOUNDS, &rc);
	std::wstring value = p.key == "Npos" ? Wide(g_ed.course.wep.nodes[g_ed.sel[0].index].points[g_ed.sel[0].point].text) : WideCp(f->Get(p.key));
	g_editRow = row;
	SetWindowTextW(g_edit, value.c_str());
	POINT tl = { rc.left, rc.top };
	ClientToScreen(g_props, &tl);
	ScreenToClient(GetParent(g_edit), &tl);
	SetWindowPos(g_edit, HWND_TOP, tl.x, tl.y, rc.right - rc.left, rc.bottom - rc.top + 2, SWP_SHOWWINDOW);
	SendMessageW(g_edit, EM_SETSEL, 0, -1);
	SetFocus(g_edit);
}

// -- browser ------------------------------------------------------------------

void PanelsRefreshBrowser()
{
	wchar_t filter[128];
	GetWindowTextW(g_filter, filter, 128);
	std::string f = LowerCp949(NarrowCp(filter));
	bool local = SendMessageW(g_local, BM_GETCHECK, 0, 0) == BST_CHECKED;
	g_browseNames.clear();
	if (local)
	{
		const std::vector<std::string>& list = g_ed.course.wep.puppets;
		for (size_t i = 0; i < list.size(); ++i)
			g_browseNames.push_back(PathBaseName(list[i]));
	}
	else
	{
		g_browseNames = g_ed.course.AllPuppetNames();
	}
	SendMessageW(g_browse, WM_SETREDRAW, FALSE, 0);
	SendMessageW(g_browse, LB_RESETCONTENT, 0, 0);
	std::vector<std::string> shown;
	for (size_t i = 0; i < g_browseNames.size(); ++i)
	{
		if (!f.empty() && LowerCp949(g_browseNames[i]).find(f) == std::string::npos)
			continue;
		shown.push_back(g_browseNames[i]);
		SendMessageW(g_browse, LB_ADDSTRING, 0, (LPARAM)WideCp(g_browseNames[i]).c_str());
	}
	g_browseNames.swap(shown);
	SendMessageW(g_browse, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(g_browse, 0, TRUE);
}

static std::string BrowseSelection()
{
	int i = (int)SendMessageW(g_browse, LB_GETCURSEL, 0, 0);
	return i >= 0 && i < (int)g_browseNames.size() ? g_browseNames[i] : std::string();
}

// -- surfaces -----------------------------------------------------------------

static std::vector<std::string> g_surfTextures;	// CP949, what the rows of the texture list hold

void PanelsRefreshSurfaces()
{
	Course& c = g_ed.course;
	SendMessageW(g_classes, WM_SETREDRAW, FALSE, 0);
	ListView_DeleteAllItems(g_classes);
	const XmlNode* d = c.prop.Declare();
	if (d)
	{
		for (size_t i = 0; i < d->children.size(); ++i)
		{
			const XmlNode& k = d->children[i];
			int row = AddRow(g_classes, WideCp(k.Attr("name")), WideCp(k.Attr("sort")));
			wchar_t ground[32];
			swprintf_s(ground, L"%S", PropGroundName(PropGroundType(c.prop.Field((int)i, "pet"))));
			SubItem(g_classes, row, 2, Wide(c.prop.Field((int)i, "bound")));
			SubItem(g_classes, row, 3, Wide(c.prop.Field((int)i, "roll")));
			SubItem(g_classes, row, 4, Wide(c.prop.Field((int)i, "min")));
			SubItem(g_classes, row, 5, Wide(c.prop.Field((int)i, "max")));
			SubItem(g_classes, row, 6, Wide(c.prop.Field((int)i, "pass")));
			SubItem(g_classes, row, 7, ground);
			SubItem(g_classes, row, 8, WideCp(c.prop.Field((int)i, "pet")));
		}
	}
	SendMessageW(g_classes, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(g_classes, 0, TRUE);

	SendMessageW(g_texlist, WM_SETREDRAW, FALSE, 0);
	ListView_DeleteAllItems(g_texlist);
	g_surfTextures.clear();
	PuppetAsset* base = c.BasePuppet();
	std::vector<std::string> all = c.UsedTextures();
	for (size_t i = 0; i < all.size(); ++i)
	{
		int cls = c.prop.ClassOfTexture(all[i]);
		const XmlNode* k = c.prop.ClassAt(cls);
		bool inBase = base && std::find(base->textureNames.begin(), base->textureNames.end(), all[i]) != base->textureNames.end();
		int row = AddRow(g_texlist, WideCp(all[i]), k ? WideCp(k->Attr("name")) : std::wstring(L"-"));
		SubItem(g_texlist, row, 2, inBase ? T(L"지면", L"ground") : T(L"오브젝트", L"object"));
		g_surfTextures.push_back(all[i]);
	}
	SendMessageW(g_texlist, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(g_texlist, 0, TRUE);
}

static int SelectedRow(HWND list)
{
	return (int)SendMessageW(list, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
}

static void PropTouched()
{
	Course& c = g_ed.course;
	c.prop.SyncCounts();
	c.prop.dirty = true;
	SceneInvalidateBase();
	PanelsRefreshSurfaces();
	ViewsRedraw();
}

// Moves textures out of whatever class holds them and into this one.
static void AssignTexture(const std::string& tex, int classIndex)
{
	XmlNode* d = g_ed.course.prop.Declare();
	if (!d || classIndex < 0 || classIndex >= (int)d->children.size())
		return;
	for (size_t i = 0; i < d->children.size(); ++i)
	{
		XmlNode* list = d->children[i].Child("textures");
		if (!list)
			continue;
		for (size_t k = 0; k < list->children.size(); ++k)
		{
			if (!_stricmp(list->children[k].text.c_str(), tex.c_str()))
			{
				list->children.erase(list->children.begin() + k);
				break;
			}
		}
		if (list->children.empty())
			d->children[i].RemoveChild("textures");
	}
	XmlNode& target = d->children[classIndex];
	XmlNode* list = target.Child("textures");
	if (!list)
	{
		XmlNode fresh;
		fresh.name = "textures";
		target.children.push_back(fresh);
		list = &target.children.back();
	}
	XmlNode item;
	item.name = "item";
	item.text = tex;
	list->children.push_back(item);
}

static void EditClassCell(int row, int col)
{
	XmlNode* d = g_ed.course.prop.Declare();
	if (!d || row < 0 || row >= (int)d->children.size())
		return;
	XmlNode& k = d->children[row];
	static const char* const FIELDS[] = { "name", "sort", "bound", "roll", "min", "max", "pass", "", "pet" };
	if (col < 0 || col > 8 || col == 7)
		return;
	std::string field = FIELDS[col];
	std::wstring value = col < 2 ? WideCp(k.Attr(field)) : WideCp(g_ed.course.prop.Field(row, field));
	std::wstring title = Wide(field);
	if (!EdPrompt(title.c_str(), title.c_str(), &value))
		return;
	std::string text = NarrowCp(value);
	if (col < 2)
		k.SetAttr(field, text);
	else
		k.SetChildText(field, text);
	PropTouched();
}

// -- hole sheet ---------------------------------------------------------------

static const wchar_t* SlotRole(int slot)
{
	switch (slot)
	{
	case 0: return T(L"티 (보통)", L"Tee, normal");
	case 1: return T(L"티 (DiffFlag 2)", L"Tee, DiffFlag 2");
	case 2: case 3: case 4: return T(L"핀 (보통)", L"Pin, normal");
	default: return T(L"핀 (DiffFlag 2)", L"Pin, DiffFlag 2");
	}
}

void PanelsRefreshHole()
{
	Course& c = g_ed.course;
	SendMessageW(g_slots, WM_SETREDRAW, FALSE, 0);
	ListView_DeleteAllItems(g_slots);
	for (int s = 0; s < Course::HoleSlotCount(); ++s)
	{
		int row = AddRow(g_slots, WideCp(Course::HoleSlotName(s)), SlotRole(s));
		int e = c.FindHoleSlot(s);
		if (e >= 0)
		{
			WVector p = c.wep.elements[e].GetVec("Pos");
			wchar_t buf[64];
			swprintf_s(buf, L"%.2f, %.2f", p.x, p.z);
			SubItem(g_slots, row, 2, buf);
		}
		else
		{
			SubItem(g_slots, row, 2, T(L"없음", L"not placed"));
		}
	}
	SendMessageW(g_slots, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(g_slots, 0, TRUE);
	XmlNode* entry = c.prop.Loaded() ? c.prop.WepEntry(Utf8ToCp949(c.baseName + ".wep")) : 0;
	SetWindowTextW(g_sun, entry ? WideCp(entry->ChildText("sunlight")).c_str() : L"");
	SetWindowTextW(g_desc, entry ? WideCp(entry->ChildText("desc")).c_str() : L"");
	EnableWindow(g_sun, entry != 0);
	EnableWindow(g_desc, entry != 0);
	EnableWindow(g_holeApply, entry != 0);

	int puppets = 0, tris = 0, unresolved = 0;
	for (size_t i = 0; i < c.wep.elements.size(); ++i)
	{
		const WepElement& e = c.wep.elements[i];
		if (!e.IsPuppet())
			continue;
		++puppets;
		PuppetAsset* a = c.Puppet(e.Get("Filename"));
		if (a->loaded)
			tris += (int)a->pet.points.size() / 3;
		else
			++unresolved;
	}
	const WepElement* game = c.wep.Game();
	wchar_t stats[512];
	swprintf_s(stats, g_ed.korean ?
		L"파 %d\r\n퍼펫 %d개 (없는 파일 %d개), 삼각형 %d개\r\n노드 리스트 %d개\r\n컴파일된 파일: %s (버전 0x%X)\r\n그림자 맵: %s, 안개: %s\r\n지면 속성표: %s" :
		L"Par %d\r\n%d puppets (%d unresolved), %d triangles\r\n%d node lists\r\nCompiled hole: %s (version 0x%X)\r\nShadow maps: %s, fog: %s\r\nSurface table: %s",
		game ? game->GetInt("Par") : 0, puppets, unresolved, tris, (int)c.wep.nodes.size(),
		c.hasSource ? L"yes" : L"no", c.hasSource ? c.source.version : 0, c.hasSbin ? L"yes" : L"no", c.hasFog ? L"yes" : L"no",
		c.prop.Loaded() ? L"yes" : L"no");
	SetWindowTextW(g_stats, stats);
}

// -- layout -------------------------------------------------------------------

static void ShowGroup(HWND* list, int count, bool show)
{
	for (int i = 0; i < count; ++i)
		ShowWindow(list[i], show ? SW_SHOW : SW_HIDE);
}

void PanelsSwitchTab(int tab)
{
	g_tab = tab;
	TabCtrl_SetCurSel(g_tabs, tab);
	CommitEdit(false);
	HWND scene[] = { g_tree, g_props };
	HWND browse[] = { g_filter, g_browse, g_local, g_place, g_replace, g_preview };
	HWND surf[] = { g_classes, g_texlist, g_assign, g_addClass, g_delClass, g_saveXml, g_showSurf };
	HWND hole[] = { g_slots, g_holePlace, g_holeGoto, g_sunLabel, g_sun, g_descLabel, g_desc, g_holeApply, g_stats };
	ShowGroup(scene, 2, tab == 0);
	ShowGroup(browse, 6, tab == 1);
	ShowGroup(surf, 7, tab == 2);
	ShowGroup(hole, 9, tab == 3);
	if (tab == 1 && !SendMessageW(g_browse, LB_GETCOUNT, 0, 0))
		PanelsRefreshBrowser();
	if (tab == 2)
		PanelsRefreshSurfaces();
	if (tab == 3)
		PanelsRefreshHole();
}

void PanelsLayout(int x, int y, int w, int h)
{
	g_x = x;
	g_y = y;
	g_w = w;
	g_h = h;
	MoveWindow(g_tabs, x, y, w, 24, TRUE);
	int top = y + 26, cw = w, ch = h - 26;
	int treeH = ch * 55 / 100;
	MoveWindow(g_tree, x, top, cw, treeH, TRUE);
	MoveWindow(g_props, x, top + treeH + 2, cw, ch - treeH - 2, TRUE);
	ListView_SetColumnWidth(g_props, 1, cw - 96 - GetSystemMetrics(SM_CXVSCROLL));

	MoveWindow(g_filter, x, top, cw - 70, 22, TRUE);
	MoveWindow(g_local, x + cw - 66, top, 66, 22, TRUE);
	int previewH = cw * 3 / 4;
	MoveWindow(g_browse, x, top + 24, cw, ch - 24 - previewH - 30, TRUE);
	MoveWindow(g_preview, x, top + ch - previewH - 28, cw, previewH, TRUE);
	MoveWindow(g_place, x, top + ch - 26, cw / 2 - 1, 24, TRUE);
	MoveWindow(g_replace, x + cw / 2 + 1, top + ch - 26, cw / 2 - 1, 24, TRUE);

	int classH = ch * 45 / 100;
	MoveWindow(g_classes, x, top, cw, classH, TRUE);
	int btn = (cw - 6) / 4;
	MoveWindow(g_addClass, x, top + classH + 2, btn, 24, TRUE);
	MoveWindow(g_delClass, x + btn + 2, top + classH + 2, btn, 24, TRUE);
	MoveWindow(g_showSurf, x + (btn + 2) * 2, top + classH + 2, btn, 24, TRUE);
	MoveWindow(g_saveXml, x + (btn + 2) * 3, top + classH + 2, btn, 24, TRUE);
	MoveWindow(g_texlist, x, top + classH + 28, cw, ch - classH - 56, TRUE);
	MoveWindow(g_assign, x, top + ch - 26, cw, 24, TRUE);

	MoveWindow(g_slots, x, top, cw, 190, TRUE);
	MoveWindow(g_holePlace, x, top + 192, cw / 2 - 1, 24, TRUE);
	MoveWindow(g_holeGoto, x + cw / 2 + 1, top + 192, cw / 2 - 1, 24, TRUE);
	MoveWindow(g_sunLabel, x, top + 224, 80, 18, TRUE);
	MoveWindow(g_sun, x + 80, top + 222, cw - 80, 22, TRUE);
	MoveWindow(g_descLabel, x, top + 250, 80, 18, TRUE);
	MoveWindow(g_desc, x + 80, top + 248, cw - 80, 22, TRUE);
	MoveWindow(g_holeApply, x, top + 274, cw, 24, TRUE);
	MoveWindow(g_stats, x, top + 302, cw, ch - 302, TRUE);
}

void PanelsLanguage()
{
	TCITEMW t;
	t.mask = TCIF_TEXT;
	const wchar_t* tabs[4] = { T(L"장면", L"Scene"), T(L"퍼펫", L"Puppets"), T(L"지면 속성", L"Surfaces"), T(L"홀", L"Hole") };
	for (int i = 0; i < 4; ++i)
	{
		t.pszText = (LPWSTR)tabs[i];
		SendMessageW(g_tabs, TCM_SETITEMW, i, (LPARAM)&t);
	}
	SetWindowTextW(g_local, T(L"이 홀만", L"Hole only"));
	SetWindowTextW(g_place, T(L"배치 도구", L"Place tool"));
	SetWindowTextW(g_replace, T(L"선택 교체", L"Replace selected"));
	SetWindowTextW(g_addClass, T(L"추가", L"Add"));
	SetWindowTextW(g_delClass, T(L"삭제", L"Delete"));
	SetWindowTextW(g_saveXml, T(L"저장", L"Save XML"));
	SetWindowTextW(g_showSurf, T(L"보기", L"Show"));
	SetWindowTextW(g_assign, T(L"선택한 텍스처를 선택한 속성에 넣기", L"Assign selected texture to selected class"));
	SetWindowTextW(g_holePlace, T(L"배치", L"Place"));
	SetWindowTextW(g_holeGoto, T(L"가기", L"Go to"));
	SetWindowTextW(g_sunLabel, T(L"햇빛", L"Sunlight"));
	SetWindowTextW(g_descLabel, T(L"설명", L"Description"));
	SetWindowTextW(g_holeApply, T(L"속성표에 적용", L"Apply to surface table"));
}

void PanelsCreate(HWND parent)
{
	g_font = CreateFontW(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"Gulim");
	g_tabs = Make(WC_TABCONTROLW, L"", WS_VISIBLE | WS_CLIPSIBLINGS, IDC_TABS, parent);
	TCITEMW t;
	t.mask = TCIF_TEXT;
	t.pszText = (LPWSTR)L"";
	for (int i = 0; i < 4; ++i)
		SendMessageW(g_tabs, TCM_INSERTITEMW, i, (LPARAM)&t);

	g_tree = Make(WC_TREEVIEWW, L"", WS_VISIBLE | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS, IDC_TREE, parent, WS_EX_CLIENTEDGE);
	g_props = Make(WC_LISTVIEWW, L"", WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, IDC_PROPS, parent, WS_EX_CLIENTEDGE);
	ListView_SetExtendedListViewStyle(g_props, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	Column(g_props, 0, L"Property", 92);
	Column(g_props, 1, L"Value", 200);
	g_edit = Make(L"EDIT", L"", ES_AUTOHSCROLL | WS_BORDER, IDC_PROP_EDIT, parent);
	g_editOld = (WNDPROC)SetWindowLongPtrW(g_edit, GWLP_WNDPROC, (LONG_PTR)EditProc);

	g_filter = Make(L"EDIT", L"", ES_AUTOHSCROLL, IDC_BROWSE_FILTER, parent, WS_EX_CLIENTEDGE);
	g_local = Make(L"BUTTON", L"", BS_AUTOCHECKBOX, IDC_BROWSE_LOCAL, parent);
	g_browse = Make(L"LISTBOX", L"", WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT, IDC_BROWSE_LIST, parent, WS_EX_CLIENTEDGE);
	g_place = Make(L"BUTTON", L"", BS_PUSHBUTTON, IDC_BROWSE_PLACE, parent);
	g_replace = Make(L"BUTTON", L"", BS_PUSHBUTTON, IDC_BROWSE_REPLACE, parent);
	g_preview = ViewsPreviewCreate(parent);

	g_classes = Make(WC_LISTVIEWW, L"", LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, IDC_SURF_CLASSES, parent, WS_EX_CLIENTEDGE);
	ListView_SetExtendedListViewStyle(g_classes, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	Column(g_classes, 0, L"Class", 86);
	Column(g_classes, 1, L"Sort", 46);
	Column(g_classes, 2, L"Bound", 42);
	Column(g_classes, 3, L"Roll", 38);
	Column(g_classes, 4, L"Min", 36);
	Column(g_classes, 5, L"Max", 36);
	Column(g_classes, 6, L"Pass", 34);
	Column(g_classes, 7, L"Ground", 60);
	Column(g_classes, 8, L"Pet", 80);
	g_texlist = Make(WC_LISTVIEWW, L"", LVS_REPORT | LVS_SHOWSELALWAYS, IDC_SURF_TEX, parent, WS_EX_CLIENTEDGE);
	ListView_SetExtendedListViewStyle(g_texlist, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	Column(g_texlist, 0, L"Texture", 150);
	Column(g_texlist, 1, L"Class", 100);
	Column(g_texlist, 2, L"Used by", 60);
	g_assign = Make(L"BUTTON", L"", BS_PUSHBUTTON, IDC_SURF_ASSIGN, parent);
	g_addClass = Make(L"BUTTON", L"", BS_PUSHBUTTON, IDC_SURF_ADD, parent);
	g_delClass = Make(L"BUTTON", L"", BS_PUSHBUTTON, IDC_SURF_DEL, parent);
	g_saveXml = Make(L"BUTTON", L"", BS_PUSHBUTTON, IDC_SURF_SAVE, parent);
	g_showSurf = Make(L"BUTTON", L"", BS_PUSHBUTTON, IDC_SURF_SHOW, parent);

	g_slots = Make(WC_LISTVIEWW, L"", LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, IDC_HOLE_SLOTS, parent, WS_EX_CLIENTEDGE);
	ListView_SetExtendedListViewStyle(g_slots, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	Column(g_slots, 0, L"Point", 70);
	Column(g_slots, 1, L"Role", 110);
	Column(g_slots, 2, L"Position", 120);
	g_holePlace = Make(L"BUTTON", L"", BS_PUSHBUTTON, IDC_HOLE_PLACE, parent);
	g_holeGoto = Make(L"BUTTON", L"", BS_PUSHBUTTON, IDC_HOLE_GOTO, parent);
	g_sunLabel = Make(L"STATIC", L"", 0, 0, parent);
	g_sun = Make(L"EDIT", L"", ES_AUTOHSCROLL, IDC_HOLE_SUN, parent, WS_EX_CLIENTEDGE);
	g_descLabel = Make(L"STATIC", L"", 0, 0, parent);
	g_desc = Make(L"EDIT", L"", ES_AUTOHSCROLL, IDC_HOLE_DESC, parent, WS_EX_CLIENTEDGE);
	g_holeApply = Make(L"BUTTON", L"", BS_PUSHBUTTON, IDC_HOLE_APPLY, parent);
	g_stats = Make(L"EDIT", L"", ES_MULTILINE | ES_READONLY | WS_VSCROLL, IDC_HOLE_STATS, parent, WS_EX_CLIENTEDGE);
	PanelsLanguage();
	PanelsSwitchTab(0);
}

// -- messages -----------------------------------------------------------------

bool PanelsNotify(NMHDR* hdr)
{
	Course& c = g_ed.course;
	switch (hdr->idFrom)
	{
	case IDC_TABS:
		if (hdr->code == TCN_SELCHANGE)
		{
			PanelsSwitchTab(TabCtrl_GetCurSel(g_tabs));
			return true;
		}
		break;
	case IDC_TREE:
		if (hdr->code == TVN_SELCHANGEDW && !g_syncing)
		{
			NMTREEVIEWW* n = (NMTREEVIEWW*)hdr;
			LPARAM d = n->itemNew.lParam;
			if (d == -1)
				return true;
			ElementRef::Kind kind = (ElementRef::Kind)((d >> 24) & 0xFF);
			int index = (int)(d & 0xFFFFFF);
			g_ed.sel.assign(1, ElementRef(kind, index));
			if (kind == ElementRef::NODE)
				g_ed.activeNode = index;
			g_syncing = true;
			EdSelectionChanged();
			g_syncing = false;
			return true;
		}
		if (hdr->code == NM_DBLCLK)
		{
			ViewsFrameSelection();
			return true;
		}
		break;
	case IDC_PROPS:
		if (hdr->code == NM_DBLCLK)
		{
			PropDoubleClick(((NMITEMACTIVATE*)hdr)->iItem);
			return true;
		}
		break;
	case IDC_SURF_CLASSES:
		if (hdr->code == NM_DBLCLK)
		{
			NMITEMACTIVATE* a = (NMITEMACTIVATE*)hdr;
			EditClassCell(a->iItem, a->iSubItem);
			return true;
		}
		if (hdr->code == LVN_ITEMCHANGED)
		{
			NMLISTVIEW* n = (NMLISTVIEW*)hdr;
			if (n->uChanged & LVIF_STATE)
			{
				int sel = SelectedRow(g_classes);
				if (sel != g_ed.surfaceHighlight)
				{
					g_ed.surfaceHighlight = sel;
					ViewsRedraw();
				}
			}
			return true;
		}
		break;
	case IDC_HOLE_SLOTS:
		if (hdr->code == NM_DBLCLK)
		{
			int s = ((NMITEMACTIVATE*)hdr)->iItem;
			int e = s >= 0 ? c.FindHoleSlot(s) : -1;
			if (e >= 0)
			{
				g_ed.sel.assign(1, ElementRef(ElementRef::ELEMENT, e));
				EdSelectionChanged();
				ViewsFrameSelection();
			}
			return true;
		}
		break;
	}
	return false;
}

bool PanelsCommand(int id, int code)
{
	Course& c = g_ed.course;
	switch (id)
	{
	case IDC_BROWSE_FILTER:
		if (code == EN_CHANGE)
			PanelsRefreshBrowser();
		return true;
	case IDC_BROWSE_LOCAL:
		PanelsRefreshBrowser();
		return true;
	case IDC_BROWSE_LIST:
		if (code == LBN_SELCHANGE)
		{
			g_ed.addPuppet = BrowseSelection();
			ViewsPreviewSet(g_ed.addPuppet);
		}
		else if (code == LBN_DBLCLK)
		{
			g_ed.addPuppet = BrowseSelection();
			EdSetTool(TOOL_ADD_PUPPET);
		}
		return true;
	case IDC_BROWSE_PLACE:
		g_ed.addPuppet = BrowseSelection();
		if (!g_ed.addPuppet.empty())
			EdSetTool(TOOL_ADD_PUPPET);
		return true;
	case IDC_BROWSE_REPLACE:
	{
		std::string name = BrowseSelection();
		if (name.empty())
			return true;
		std::string file = c.wep.PuppetFile(name);
		c.Checkpoint("Replace puppet");
		bool any = false;
		for (size_t i = 0; i < g_ed.sel.size(); ++i)
		{
			if (g_ed.sel[i].kind == ElementRef::ELEMENT && c.wep.elements[g_ed.sel[i].index].IsPuppet())
			{
				c.wep.elements[g_ed.sel[i].index].Set("Filename", file);
				any = true;
			}
		}
		if (any)
		{
			c.wep.SyncPuppetList();
			EdSceneChanged();
		}
		return true;
	}
	case IDC_SURF_ASSIGN:
	{
		int cls = SelectedRow(g_classes);
		if (cls < 0)
			return true;
		for (int row = -1; (row = (int)SendMessageW(g_texlist, LVM_GETNEXTITEM, row, LVNI_SELECTED)) >= 0;)
			if (row < (int)g_surfTextures.size())
				AssignTexture(g_surfTextures[row], cls);
		PropTouched();
		return true;
	}
	case IDC_SURF_ADD:
	{
		XmlNode* d = c.prop.Declare();
		if (!d)
			return true;
		std::wstring name = L"new";
		if (EdPrompt(T(L"새 속성", L"New class"), T(L"이름", L"Name"), &name))
		{
			XmlNode k;
			k.name = "class";
			k.SetAttr("name", NarrowCp(name));
			k.SetAttr("sort", "\xC1\xF6\xB8\xE9");	// 지면
			k.SetChildText("bound", "0.5");
			k.SetChildText("roll", "0.5");
			k.SetChildText("min", "90.0");
			k.SetChildText("max", "100.0");
			k.SetChildText("pet", "rought.pet");
			d->children.push_back(k);
			PropTouched();
		}
		return true;
	}
	case IDC_SURF_DEL:
	{
		XmlNode* d = c.prop.Declare();
		int row = SelectedRow(g_classes);
		if (d && row >= 0 && row < (int)d->children.size() && d->children[row].Attr("name") != "default")
		{
			d->children.erase(d->children.begin() + row);
			g_ed.surfaceHighlight = -1;
			PropTouched();
		}
		return true;
	}
	case IDC_SURF_SAVE:
	{
		if (!c.prop.Loaded())
			return true;
		std::string error;
		if (c.prop.Save(c.prop.path, &error))
		{
			c.prop.dirty = false;
			EdLog("Saved " + c.prop.path);
		}
		else
		{
			EdLog("! " + error);
		}
		return true;
	}
	case IDC_SURF_SHOW:
		ViewsSetActiveShade(ViewsActiveShade() == SHADE_SURFACE ? SHADE_TEXTURED : SHADE_SURFACE);
		return true;
	case IDC_HOLE_PLACE:
	{
		int s = SelectedRow(g_slots);
		if (s < 0)
			return true;
		int e = c.FindHoleSlot(s);
		if (e >= 0)
		{
			g_ed.sel.assign(1, ElementRef(ElementRef::ELEMENT, e));
			EdSelectionChanged();
			EdLog("That slot is placed; drag it in a viewport.");
			return true;
		}
		g_ed.addPoint = Course::HoleSlotName(s);
		g_ed.addPointClass = "WPoint";
		EdSetTool(TOOL_ADD_POINT);
		EdLog("Click on the course to place the point.");
		return true;
	}
	case IDC_HOLE_GOTO:
	{
		int s = SelectedRow(g_slots);
		int e = s >= 0 ? c.FindHoleSlot(s) : -1;
		if (e >= 0)
		{
			g_ed.sel.assign(1, ElementRef(ElementRef::ELEMENT, e));
			EdSelectionChanged();
			ViewsFrameSelection();
		}
		return true;
	}
	case IDC_HOLE_APPLY:
	{
		XmlNode* entry = c.prop.Loaded() ? c.prop.WepEntry(Utf8ToCp949(c.baseName + ".wep")) : 0;
		if (!entry)
			return true;
		wchar_t buf[256];
		GetWindowTextW(g_sun, buf, 256);
		entry->SetChildText("sunlight", NarrowCp(buf));
		GetWindowTextW(g_desc, buf, 256);
		if (buf[0])
			entry->SetChildText("desc", NarrowCp(buf));
		else
			entry->RemoveChild("desc");
		c.prop.dirty = true;
		EdLog("Hole entry changed; save it from the Surfaces tab.");
		return true;
	}
	}
	return false;
}
