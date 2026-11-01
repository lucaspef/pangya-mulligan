// SPDX-License-Identifier: MIT
// viewport.cpp - the four UnrealEd-style panes and everything the mouse does in them.
#include <windows.h>
#include <windowsx.h>
#include <GL/gl.h>
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include "areascript.h"
#include "scene.h"
#include "wcommon.h"

static HGLRC g_rc;
static HWND g_preview;
static HDC g_previewDc;
static PuppetAsset* g_previewAsset;
static float g_previewYaw;

enum PaneKind { PANE_TOP, PANE_FRONT, PANE_SIDE, PANE_PERSP, PANE_COUNT };

struct Pane
{
	HWND wnd;
	HDC dc;
	PaneKind kind;
	PaneShade shade;
	WVector center;		// ortho: what the middle of the pane looks at
	float halfHeight;
	WVector eye;		// perspective
	float yaw, pitch;
	float fov;
};

static Pane g_panes[PANE_COUNT];
static int g_active = PANE_PERSP;
static int g_maximized = -1;
static int g_x, g_y, g_w, g_h;
static int g_lookCamera = -1;
static bool g_flying;
static float g_flyAt;
static int g_flyNode;

// -- views --------------------------------------------------------------------

static SceneView ViewOf(const Pane& p)
{
	SceneView v;
	RECT rc;
	GetClientRect(p.wnd, &rc);
	v.width = rc.right > 1 ? rc.right : 1;
	v.height = rc.bottom > 1 ? rc.bottom : 1;
	v.shade = p.shade;
	v.ortho = p.kind != PANE_PERSP;
	v.halfHeight = p.halfHeight;
	v.fov = p.fov;
	switch (p.kind)
	{
	case PANE_TOP:
		v.right = WVector(1, 0, 0);
		v.up = WVector(0, 0, 1);
		v.forward = WVector(0, -1, 0);
		v.eye = p.center;
		break;
	case PANE_FRONT:
		v.right = WVector(1, 0, 0);
		v.up = WVector(0, 1, 0);
		v.forward = WVector(0, 0, 1);
		v.eye = p.center;
		break;
	case PANE_SIDE:
		v.right = WVector(0, 0, 1);
		v.up = WVector(0, 1, 0);
		v.forward = WVector(-1, 0, 0);
		v.eye = p.center;
		break;
	default:
	{
		v.eye = p.eye;
		float cp = (float)cos(p.pitch);
		v.forward = WVector((float)sin(p.yaw) * cp, (float)sin(p.pitch), (float)cos(p.yaw) * cp);
		v.right = WVector(0, 1, 0).Cross(v.forward);
		v.right.Normalize();
		v.up = v.forward.Cross(v.right);
		break;
	}
	}
	return v;
}

static const wchar_t* PaneTitle(PaneKind k)
{
	switch (k)
	{
	case PANE_TOP: return T(L"위 (XZ)", L"Top (XZ)");
	case PANE_FRONT: return T(L"앞 (XY)", L"Front (XY)");
	case PANE_SIDE: return T(L"옆 (ZY)", L"Side (ZY)");
	default: return T(L"3D 카메라", L"3D Camera");
	}
}

static const wchar_t* ShadeTitle(PaneShade s)
{
	switch (s)
	{
	case SHADE_WIRE: return T(L"와이어", L"Wire");
	case SHADE_FLAT: return T(L"단색", L"Flat");
	case SHADE_TEXTURED: return T(L"텍스처", L"Textured");
	default: return T(L"지면 속성", L"Surfaces");
	}
}

static void PaintPane(Pane& p)
{
	PAINTSTRUCT ps;
	BeginPaint(p.wnd, &ps);
	EndPaint(p.wnd, &ps);
	if (!wglMakeCurrent(p.dc, g_rc))
		return;
	SceneView v = ViewOf(p);
	SceneDraw(v);
	wchar_t title[128];
	swprintf_s(title, L"%s  [%s]", PaneTitle(p.kind), ShadeTitle(p.shade));
	if ((int)(&p - g_panes) == g_active)
		glColor3f(1.0f, 0.85f, 0.3f);
	else
		glColor3f(0.75f, 0.75f, 0.75f);
	SceneDrawOverlayText(v, title, 6, 15);
	SwapBuffers(p.dc);
}

void ViewsCaptureInto(HDC dc, int originX, int originY)
{
	for (int i = 0; i < PANE_COUNT; ++i)
	{
		Pane& p = g_panes[i];
		if (!p.wnd || !IsWindowVisible(p.wnd) || !wglMakeCurrent(p.dc, g_rc))
			continue;
		SceneView v = ViewOf(p);
		SceneDraw(v);
		glColor3f(0.75f, 0.75f, 0.75f);
		wchar_t title[128];
		swprintf_s(title, L"%s  [%s]", PaneTitle(p.kind), ShadeTitle(p.shade));
		SceneDrawOverlayText(v, title, 6, 15);
		std::vector<unsigned char> pixels(v.width * v.height * 4);
		glReadBuffer(GL_BACK);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, v.width, v.height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, &pixels[0]);
		BITMAPINFO bi;
		memset(&bi, 0, sizeof(bi));
		bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
		bi.bmiHeader.biWidth = v.width;
		bi.bmiHeader.biHeight = v.height;
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 32;
		POINT at = { 0, 0 };
		ClientToScreen(p.wnd, &at);
		SetDIBitsToDevice(dc, at.x - originX, at.y - originY, v.width, v.height, 0, 0, 0, v.height, &pixels[0], &bi, DIB_RGB_COLORS);
	}
}

void ViewsRedraw()
{
	for (int i = 0; i < PANE_COUNT; ++i)
		if (g_panes[i].wnd)
			InvalidateRect(g_panes[i].wnd, 0, FALSE);
}

static void ApplyLayout()
{
	int gap = 2;
	int w1 = (g_w - gap) / 2, h1 = (g_h - gap) / 2;
	for (int i = 0; i < PANE_COUNT; ++i)
	{
		if (!g_panes[i].wnd)
			continue;
		if (g_maximized >= 0)
		{
			if (i == g_maximized)
			{
				ShowWindow(g_panes[i].wnd, SW_SHOW);
				MoveWindow(g_panes[i].wnd, g_x, g_y, g_w, g_h, TRUE);
			}
			else
			{
				ShowWindow(g_panes[i].wnd, SW_HIDE);
			}
			continue;
		}
		ShowWindow(g_panes[i].wnd, SW_SHOW);
		// Top | 3D over Front | Side, the layout UnrealEd opens with.
		int col = i == PANE_TOP || i == PANE_FRONT ? 0 : 1;
		int row = i == PANE_TOP || i == PANE_PERSP ? 0 : 1;
		MoveWindow(g_panes[i].wnd, g_x + col * (w1 + gap), g_y + row * (h1 + gap), col ? g_w - w1 - gap : w1, row ? g_h - h1 - gap : h1, TRUE);
	}
}

void ViewsLayout(int x, int y, int w, int h)
{
	g_x = x;
	g_y = y;
	g_w = w;
	g_h = h;
	ApplyLayout();
}

void ViewsMaximizeActive()
{
	g_maximized = g_maximized >= 0 ? -1 : g_active;
	ApplyLayout();
	ViewsRedraw();
}

void ViewsSetActiveShade(PaneShade shade)
{
	g_panes[g_active].shade = shade;
	ViewsRedraw();
}

PaneShade ViewsActiveShade()
{
	return g_panes[g_active].shade;
}

static void FrameBox(const WVector& lo, const WVector& hi)
{
	WVector c = (lo + hi) * 0.5f;
	float r = (hi - lo).Length() * 0.5f;
	if (r < 5.0f)
		r = 5.0f;
	for (int i = 0; i < PANE_COUNT; ++i)
	{
		Pane& p = g_panes[i];
		if (p.kind == PANE_PERSP)
		{
			float dist = r / (float)tan(p.fov * 3.14159265f / 360.0f);
			float cp = (float)cos(p.pitch);
			WVector f((float)sin(p.yaw) * cp, (float)sin(p.pitch), (float)cos(p.yaw) * cp);
			p.eye = c - f * dist;
		}
		else
		{
			p.center = c;
			p.halfHeight = r * 1.05f;
		}
	}
	ViewsRedraw();
}

void ViewsFrameAll()
{
	WVector lo, hi;
	g_ed.course.SceneBounds(&lo, &hi);
	Pane& p = g_panes[PANE_PERSP];
	p.yaw = 0.0f;
	p.pitch = -0.6f;
	FrameBox(lo, hi);
}

void ViewsFrameSelection()
{
	Course& c = g_ed.course;
	bool any = false;
	WVector lo, hi;
	for (size_t i = 0; i < g_ed.sel.size(); ++i)
	{
		const ElementRef& r = g_ed.sel[i];
		WVector a, b;
		if (r.kind == ElementRef::ELEMENT)
		{
			c.ElementBounds(c.wep.elements[r.index], &a, &b);
		}
		else if (r.kind == ElementRef::NODE && !c.wep.nodes[r.index].points.empty())
		{
			a = b = c.wep.nodes[r.index].points[0].pos;
			for (size_t k = 1; k < c.wep.nodes[r.index].points.size(); ++k)
			{
				GrowBox(&a, &b, c.wep.nodes[r.index].points[k].pos);
			}
		}
		else if (r.kind == ElementRef::NODE_POINT)
		{
			a = c.wep.nodes[r.index].points[r.point].pos - WVector(5, 5, 5);
			b = a + WVector(10, 10, 10);
		}
		else
		{
			continue;
		}
		if (!any)
		{
			lo = a;
			hi = b;
			any = true;
		}
		GrowBox(&lo, &hi, a);
		GrowBox(&lo, &hi, b);
	}
	if (any)
		FrameBox(lo, hi);
	else
		ViewsFrameAll();
}

void ViewsLookThrough(int cameraElement)
{
	g_lookCamera = cameraElement;
	if (cameraElement < 0)
	{
		ViewsRedraw();
		return;
	}
	const WepElement& e = g_ed.course.wep.elements[cameraElement];
	Pane& p = g_panes[PANE_PERSP];
	p.eye = e.GetVec("Pos");
	WVector f = e.GetVec("Dest") - p.eye;
	f.Normalize();
	p.yaw = (float)atan2(f.x, f.z);
	p.pitch = (float)asin(f.y);
	p.fov = e.GetFloat("Fov", 60.0f);
	ViewsRedraw();
}

void ViewsSetCamera(const WVector& eye, float yawDegrees, float pitchDegrees)
{
	Pane& p = g_panes[PANE_PERSP];
	p.eye = eye;
	p.yaw = yawDegrees * 3.14159265f / 180.0f;
	p.pitch = pitchDegrees * 3.14159265f / 180.0f;
	g_lookCamera = -1;
	ViewsRedraw();
}

void ViewsFlyThrough(bool on)
{
	g_flying = false;
	if (!on)
		return;
	const std::vector<WepNode>& nodes = g_ed.course.wep.nodes;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i].Get("Name").find("auto") != std::string::npos && nodes[i].points.size() >= 2)
		{
			g_flying = true;
			g_flyAt = 0.0f;
			g_flyNode = (int)i;
			return;
		}
	}
	EdLog("! No auto node with two points to fly along.");
}

bool ViewsFlying()
{
	return g_flying;
}

// -- picking ------------------------------------------------------------------

static float DistToSegment(float px, float py, float ax, float ay, float bx, float by)
{
	float dx = bx - ax, dy = by - ay;
	float l = dx * dx + dy * dy;
	float t = l > 0.0f ? ((px - ax) * dx + (py - ay) * dy) / l : 0.0f;
	if (t < 0.0f) t = 0.0f;
	if (t > 1.0f) t = 1.0f;
	float qx = ax + dx * t - px, qy = ay + dy * t - py;
	return (float)sqrt(qx * qx + qy * qy);
}

static ElementRef Pick(const SceneView& v, int x, int y, bool allowGround)
{
	Course& c = g_ed.course;
	const std::vector<WepElement>& els = c.wep.elements;
	float best = 8.0f;
	ElementRef found;
	float sx, sy;
	for (size_t i = 0; i < els.size(); ++i)
	{
		const WepElement& e = els[i];
		if ((e.IsPoint() && g_ed.showPoints) || (e.IsCamera() && g_ed.showCameras))
		{
			WVector pos = e.GetVec("Pos");
			if (e.IsPoint() && pos.y == 0.0f)
				c.StandOnGround(&pos);
			if (SceneProject(v, pos, &sx, &sy))
			{
				float d = (float)sqrt((sx - x) * (sx - x) + (sy - y) * (sy - y));
				if (d < best)
				{
					best = d;
					found = ElementRef(ElementRef::ELEMENT, (int)i);
				}
			}
			if (e.IsCamera() && SceneProject(v, e.GetVec("Dest"), &sx, &sy))
			{
				float d = (float)sqrt((sx - x) * (sx - x) + (sy - y) * (sy - y));
				if (d < best)
				{
					best = d;
					found = ElementRef(ElementRef::ELEMENT, (int)i, 1);
				}
			}
		}
		else if (e.IsArea() && g_ed.showAreas)
		{
			WVector lo, hi;
			e.GetBox(&lo, &hi);
			static const int ed[12][2] = { {0,1},{1,3},{3,2},{2,0},{4,5},{5,7},{7,6},{6,4},{0,4},{1,5},{2,6},{3,7} };
			for (int k = 0; k < 12; ++k)
			{
				float ax, ay, bx, by;
				int w0 = ed[k][0], w1 = ed[k][1];
				if (SceneProject(v, WVector(w0 & 1 ? hi.x : lo.x, w0 & 2 ? hi.y : lo.y, w0 & 4 ? hi.z : lo.z), &ax, &ay) &&
					SceneProject(v, WVector(w1 & 1 ? hi.x : lo.x, w1 & 2 ? hi.y : lo.y, w1 & 4 ? hi.z : lo.z), &bx, &by))
				{
					float d = DistToSegment((float)x, (float)y, ax, ay, bx, by);
					if (d < best)
					{
						best = d;
						found = ElementRef(ElementRef::ELEMENT, (int)i);
					}
				}
			}
		}
	}
	if (g_ed.showNodes)
	{
		const std::vector<WepNode>& nodes = c.wep.nodes;
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			float px = 0, py = 0;
			bool have = false;
			for (size_t k = 0; k < nodes[i].points.size(); ++k)
			{
				if (!SceneProject(v, nodes[i].points[k].pos, &sx, &sy))
				{
					have = false;
					continue;
				}
				float d = (float)sqrt((sx - x) * (sx - x) + (sy - y) * (sy - y));
				if (g_ed.tool == TOOL_NODE && d < best + 2.0f)
				{
					best = d;
					found = ElementRef(ElementRef::NODE_POINT, (int)i, (int)k);
				}
				if (have && g_ed.tool != TOOL_NODE)
				{
					float s = DistToSegment((float)x, (float)y, px, py, sx, sy);
					if (s < best - 3.0f)
					{
						best = s + 3.0f;
						found = ElementRef(ElementRef::NODE, (int)i);
					}
				}
				px = sx;
				py = sy;
				have = true;
			}
		}
	}
	if (found.kind != ElementRef::NONE)
		return found;
	if (g_ed.showPuppets)
	{
		WVector o, d;
		SceneRay(v, x, y, &o, &d);
		TerrainHit h = c.CastRay(o, d, false);
		if (h.hit)
		{
			if (h.element >= 0)
				return ElementRef(ElementRef::ELEMENT, h.element);
			if (allowGround)
			{
				for (size_t i = 0; i < els.size(); ++i)
					if (els[i].IsPuppet() && els[i].IsGround())
						return ElementRef(ElementRef::ELEMENT, (int)i);
			}
		}
	}
	return ElementRef();
}

// -- dragging -----------------------------------------------------------------

static float Snap(float v)
{
	if (!g_ed.snap || g_ed.grid <= 0.0f)
		return v;
	return (float)floor(v / g_ed.grid + 0.5f) * g_ed.grid;
}

struct DragItem
{
	ElementRef ref;
	WVector pos, dest, lo, hi, rot;
	float scale;
	std::vector<WVector> nodePoints;
};

enum DragMode { DRAG_NONE, DRAG_ITEMS, DRAG_BOX, DRAG_AREA_NEW, DRAG_PAN, DRAG_LOOK, DRAG_PAN3D };

static DragMode g_drag;
static std::vector<DragItem> g_items;
static bool g_dragMoved;
static bool g_dragShift;
static int g_downX, g_downY, g_lastX, g_lastY;
static WVector g_dragPlaneStart;
static WVector g_areaStart;
static int g_areaElement;
static int g_boxX, g_boxY;
static bool g_rmbMoved;

static void BeginItemDrag()
{
	Course& c = g_ed.course;
	g_items.clear();
	for (size_t i = 0; i < g_ed.sel.size(); ++i)
	{
		DragItem it;
		it.ref = g_ed.sel[i];
		it.scale = 1.0f;
		if (it.ref.kind == ElementRef::ELEMENT)
		{
			const WepElement& e = c.wep.elements[it.ref.index];
			if (e.IsGame())
				continue;
			it.pos = e.GetVec("Pos");
			it.dest = e.GetVec("Dest");
			it.rot = e.GetVec("Rot");
			it.scale = e.GetFloat("Scale", 1.0f);
			if (e.IsArea())
				e.GetBox(&it.lo, &it.hi);
		}
		else if (it.ref.kind == ElementRef::NODE)
		{
			const WepNode& n = c.wep.nodes[it.ref.index];
			for (size_t k = 0; k < n.points.size(); ++k)
				it.nodePoints.push_back(n.points[k].pos);
		}
		else if (it.ref.kind == ElementRef::NODE_POINT)
		{
			it.pos = c.wep.nodes[it.ref.index].points[it.ref.point].pos;
		}
		g_items.push_back(it);
	}
}

static void ApplyMove(const WVector& delta)
{
	Course& c = g_ed.course;
	for (size_t i = 0; i < g_items.size(); ++i)
	{
		DragItem& it = g_items[i];
		if (it.ref.kind == ElementRef::ELEMENT)
		{
			WepElement& e = c.wep.elements[it.ref.index];
			if (e.IsArea())
			{
				e.SetBox(it.lo + delta, it.hi + delta);
			}
			else if (e.IsCamera() && it.ref.point == 1)
			{
				e.SetVec("Dest", it.dest + delta);
			}
			else if (e.IsCamera())
			{
				e.SetVec("Pos", it.pos + delta);
				e.SetVec("Dest", it.dest + delta);
			}
			else
			{
				WVector p(Snap(it.pos.x + delta.x), Snap(it.pos.y + delta.y), Snap(it.pos.z + delta.z));
				if (e.IsPoint() && it.pos.y == 0.0f && delta.y == 0.0f)
					p.y = 0.0f;
				else if (g_ed.snapGround && delta.y == 0.0f && !e.IsGround())
					c.DropToGround(&p);
				e.SetVec("Pos", p);
			}
		}
		else if (it.ref.kind == ElementRef::NODE)
		{
			WepNode& n = c.wep.nodes[it.ref.index];
			for (size_t k = 0; k < n.points.size() && k < it.nodePoints.size(); ++k)
				n.points[k].Set(it.nodePoints[k] + delta);
		}
		else if (it.ref.kind == ElementRef::NODE_POINT)
		{
			WVector p(Snap(it.pos.x + delta.x), Snap(it.pos.y + delta.y), Snap(it.pos.z + delta.z));
			c.wep.nodes[it.ref.index].points[it.ref.point].Set(p);
		}
	}
}

static void ApplyRotate(float degrees, int axis)
{
	Course& c = g_ed.course;
	float a = g_ed.snap ? (float)floor(degrees / 15.0f + 0.5f) * 15.0f : degrees;
	for (size_t i = 0; i < g_items.size(); ++i)
	{
		DragItem& it = g_items[i];
		if (it.ref.kind != ElementRef::ELEMENT)
			continue;
		WepElement& e = c.wep.elements[it.ref.index];
		if (!e.IsPuppet())
			continue;
		WVector r = it.rot;
		if (axis == 0) r.x += a;
		else if (axis == 2) r.z += a;
		else r.y += a;
		e.SetVec("Rot", r, 1);
	}
}

static void ApplyScale(float factor, const WVector& axisMove)
{
	Course& c = g_ed.course;
	for (size_t i = 0; i < g_items.size(); ++i)
	{
		DragItem& it = g_items[i];
		if (it.ref.kind != ElementRef::ELEMENT)
			continue;
		WepElement& e = c.wep.elements[it.ref.index];
		if (e.IsPuppet())
		{
			e.SetFloat("Scale", max(0.01f, it.scale * factor));
		}
		else if (e.IsArea())
		{
			// The side the drag started on moves; the opposite one stays.
			WVector mid = (it.lo + it.hi) * 0.5f;
			WVector lo = it.lo, hi = it.hi;
			if (g_dragPlaneStart.x >= mid.x) hi.x += axisMove.x; else lo.x += axisMove.x;
			if (g_dragPlaneStart.y >= mid.y) hi.y += axisMove.y; else lo.y += axisMove.y;
			if (g_dragPlaneStart.z >= mid.z) hi.z += axisMove.z; else lo.z += axisMove.z;
			if (lo.x > hi.x) std::swap(lo.x, hi.x);
			if (lo.y > hi.y) std::swap(lo.y, hi.y);
			if (lo.z > hi.z) std::swap(lo.z, hi.z);
			e.SetBox(lo, hi);
		}
	}
}

// The plane a drag moves in: the pane's own for ortho, flat through the start for 3D.
static bool PlanePoint(const SceneView& v, int x, int y, const WVector& through, WVector* out)
{
	WVector o, d;
	SceneRay(v, x, y, &o, &d);
	if (v.ortho)
	{
		float t = (through - o).Dot(v.forward) / d.Dot(v.forward);
		*out = o + d * t;
		return true;
	}
	if (fabs(d.y) < 1e-4f)
		return false;
	float t = (through.y - o.y) / d.y;
	if (t <= 0.0f || t > 8000.0f)
		return false;
	*out = o + d * t;
	return true;
}

// Where a click lands: on the base model when it hits, on the pane's plane otherwise.
static WVector PlacePoint(const SceneView& v, int x, int y)
{
	Course& c = g_ed.course;
	WVector o, d;
	SceneRay(v, x, y, &o, &d);
	TerrainHit h = c.CastRay(o, d, true);
	if (h.hit)
		return WVector(Snap(h.pos.x), h.pos.y, Snap(h.pos.z));
	WVector p;
	PlanePoint(v, x, y, WVector(), &p);
	return WVector(Snap(p.x), Snap(p.y), Snap(p.z));
}

static void PlaceElement(const SceneView& v, int x, int y)
{
	Course& c = g_ed.course;
	WVector at = PlacePoint(v, x, y);
	switch (g_ed.tool)
	{
	case TOOL_ADD_PUPPET:
	{
		if (g_ed.addPuppet.empty())
		{
			EdLog("! Pick a puppet in the Puppets tab first.");
			PanelsSwitchTab(1);
			return;
		}
		WepElement e = WepMakeElement("PUPPET", "WObject");
		e.Set("Filename", c.wep.PuppetFile(g_ed.addPuppet));
		e.SetVec("Pos", at);
		int index = EdAddElement(e, "Add puppet");
		g_ed.sel.assign(1, ElementRef(ElementRef::ELEMENT, index));
		break;
	}
	case TOOL_ADD_POINT:
	{
		std::string cls = g_ed.addPointClass.empty() ? "WPoint" : g_ed.addPointClass;
		WepElement e = WepMakeElement("POINT", cls.c_str());
		e.Set("Name", g_ed.addPoint);
		// Tee and pin points are stored at y = 0; only x and z are read.
		bool slot = false;
		for (int s = 0; s < Course::HoleSlotCount(); ++s)
			if (g_ed.addPoint == Course::HoleSlotName(s))
				slot = true;
		e.SetVec("Pos", slot ? WVector(at.x, 0.0f, at.z) : at);
		int index = EdAddElement(e, "Add point");
		g_ed.sel.assign(1, ElementRef(ElementRef::ELEMENT, index));
		if (slot)
		{
			g_ed.addPoint.clear();
			EdSetTool(TOOL_SELECT);
			PanelsRefreshHole();
		}
		break;
	}
	case TOOL_ADD_CAMERA:
	{
		WepElement e = WepMakeElement("CAMERA", "WCamera");
		char name[32];
		int n = 1;
		for (size_t i = 0; i < c.wep.elements.size(); ++i)
			if (c.wep.elements[i].IsCamera())
				++n;
		sprintf_s(name, "\xC4\xAB\xB8\xDE\xB6\xF3%d", n);	// 카메라n
		e.Set("Name", name);
		e.SetVec("Pos", at + WVector(0, 20, 0));
		e.SetVec("Dest", at + WVector(0, 10, 80));
		int index = EdAddElement(e, "Add camera");
		g_ed.sel.assign(1, ElementRef(ElementRef::ELEMENT, index));
		break;
	}
	default:
		break;
	}
	EdSelectionChanged();
}

static void BeginNewArea(const SceneView& v, int x, int y)
{
	WVector at = PlacePoint(v, x, y);
	WepElement e = MakeAreaElement(g_ed.addAreaClass.empty() ? "WEventArea" : g_ed.addAreaClass);
	e.SetBox(at, at + WVector(1, 20, 1));
	g_areaStart = at;
	g_areaElement = EdAddElement(e, "Add area");
	g_ed.sel.assign(1, ElementRef(ElementRef::ELEMENT, g_areaElement));
	EdSelectionChanged();
	g_drag = DRAG_AREA_NEW;
}

static void ContextMenu(HWND hwnd, int x, int y)
{
	HMENU m = CreatePopupMenu();
	AppendMenuW(m, MF_STRING, 1, T(L"복제\tCtrl+D", L"Duplicate\tCtrl+D"));
	AppendMenuW(m, MF_STRING, 2, T(L"삭제\tDel", L"Delete\tDel"));
	AppendMenuW(m, MF_STRING, 3, T(L"지면에 붙이기\tEnd", L"Drop to Ground\tEnd"));
	AppendMenuW(m, MF_STRING, 4, T(L"선택 보기\tF", L"Frame Selection\tF"));
	AppendMenuW(m, MF_SEPARATOR, 0, 0);
	AppendMenuW(m, MF_STRING, 5, T(L"속성 탭 열기", L"Show Properties"));
	POINT pt = { x, y };
	ClientToScreen(hwnd, &pt);
	int cmd = TrackPopupMenu(m, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, g_ed.main, 0);
	DestroyMenu(m);
	switch (cmd)
	{
	case 1: EdDuplicateSelection(); break;
	case 2: EdDeleteSelection(); break;
	case 3: EdRunConsole("drop"); break;
	case 4: ViewsFrameSelection(); break;
	case 5: PanelsSwitchTab(0); break;
	}
}

// -- window -------------------------------------------------------------------

static LRESULT CALLBACK PaneProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	int idx = -1;
	for (int i = 0; i < PANE_COUNT; ++i)
		if (g_panes[i].wnd == hwnd)
			idx = i;
	if (idx < 0)
		return DefWindowProcW(hwnd, msg, wp, lp);
	Pane& p = g_panes[idx];
	Course& c = g_ed.course;
	switch (msg)
	{
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		PaintPane(p);
		return 0;
	case WM_MOUSEWHEEL:
	{
		float n = GET_WHEEL_DELTA_WPARAM(wp) / 120.0f;
		if (p.kind == PANE_PERSP)
		{
			SceneView v = ViewOf(p);
			p.eye = p.eye + v.forward * (n * max(8.0f, g_ed.grid * 8.0f));
		}
		else
		{
			p.halfHeight *= (float)pow(0.85, n);
			if (p.halfHeight < 2.0f)
				p.halfHeight = 2.0f;
		}
		InvalidateRect(hwnd, 0, FALSE);
		return 0;
	}
	case WM_LBUTTONDBLCLK:
		g_active = idx;
		{
			int x = GET_X_LPARAM(lp), y = GET_Y_LPARAM(lp);
			SceneView v = ViewOf(p);
			ElementRef hit = Pick(v, x, y, false);
			if (y < 22 || hit.kind == ElementRef::NONE)
				ViewsMaximizeActive();
			else
				PanelsSwitchTab(0);
		}
		return 0;
	case WM_LBUTTONDOWN:
	{
		SetFocus(hwnd);
		SetCapture(hwnd);
		g_active = idx;
		int x = GET_X_LPARAM(lp), y = GET_Y_LPARAM(lp);
		g_downX = g_lastX = x;
		g_downY = g_lastY = y;
		g_dragMoved = false;
		g_dragShift = (wp & MK_SHIFT) != 0;
		SceneView v = ViewOf(p);
		if (g_ed.tool >= TOOL_ADD_PUPPET && g_ed.tool <= TOOL_ADD_CAMERA)
		{
			PlaceElement(v, x, y);
			g_drag = DRAG_NONE;
			return 0;
		}
		if (g_ed.tool == TOOL_ADD_AREA)
		{
			BeginNewArea(v, x, y);
			return 0;
		}
		ElementRef hit = Pick(v, x, y, (wp & MK_ALT) != 0 || GetKeyState(VK_MENU) < 0);
		if (g_ed.tool == TOOL_NODE && hit.kind != ElementRef::NODE_POINT && g_ed.activeNode >= 0 && g_ed.activeNode < (int)c.wep.nodes.size())
		{
			c.Checkpoint("Add node point");
			WepNodePoint np;
			np.Set(PlacePoint(v, x, y));
			WepNode& n = c.wep.nodes[g_ed.activeNode];
			n.points.push_back(np);
			g_ed.sel.assign(1, ElementRef(ElementRef::NODE_POINT, g_ed.activeNode, (int)n.points.size() - 1));
			EdSelectionChanged();
			hit = g_ed.sel[0];
		}
		if (hit.kind == ElementRef::NONE)
		{
			if (!g_dragShift)
			{
				g_ed.sel.clear();
				EdSelectionChanged();
			}
			g_drag = v.ortho ? DRAG_BOX : DRAG_NONE;
			g_boxX = x;
			g_boxY = y;
			return 0;
		}
		if (g_dragShift)
		{
			std::vector<ElementRef>::iterator f = std::find(g_ed.sel.begin(), g_ed.sel.end(), hit);
			if (f != g_ed.sel.end())
				g_ed.sel.erase(f);
			else
				g_ed.sel.push_back(hit);
		}
		else if (!g_ed.IsSelected(hit))
		{
			g_ed.sel.assign(1, hit);
		}
		if (hit.kind == ElementRef::NODE || hit.kind == ElementRef::NODE_POINT)
			g_ed.activeNode = hit.index;
		EdSelectionChanged();
		BeginItemDrag();
		WVector through;
		if (hit.kind == ElementRef::ELEMENT)
			through = c.wep.elements[hit.index].IsArea() ? WVector() : c.wep.elements[hit.index].GetVec("Pos");
		else if (hit.kind == ElementRef::NODE_POINT)
			through = c.wep.nodes[hit.index].points[hit.point].pos;
		if (hit.kind == ElementRef::ELEMENT && c.wep.elements[hit.index].IsArea())
		{
			WVector lo, hi;
			c.wep.elements[hit.index].GetBox(&lo, &hi);
			through = (lo + hi) * 0.5f;
		}
		g_dragPlaneStart = through;
		PlanePoint(v, x, y, through, &g_dragPlaneStart);
		g_drag = DRAG_ITEMS;
		return 0;
	}
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		SetFocus(hwnd);
		SetCapture(hwnd);
		g_active = idx;
		g_lastX = g_downX = GET_X_LPARAM(lp);
		g_lastY = g_downY = GET_Y_LPARAM(lp);
		g_rmbMoved = false;
		if (p.kind == PANE_PERSP)
		{
			g_drag = msg == WM_RBUTTONDOWN ? DRAG_LOOK : DRAG_PAN3D;
			if (g_lookCamera >= 0)
				g_lookCamera = -1;
		}
		else
		{
			g_drag = DRAG_PAN;
		}
		return 0;
	case WM_MOUSEMOVE:
	{
		int x = GET_X_LPARAM(lp), y = GET_Y_LPARAM(lp);
		SceneView v = ViewOf(p);
		// Coordinates for the status bar.
		{
			WVector pos;
			if (PlanePoint(v, x, y, WVector(), &pos))
			{
				wchar_t text[64];
				swprintf_s(text, L"%.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
				EdStatus(1, text);
			}
		}
		if (g_drag == DRAG_NONE)
			return 0;
		int dx = x - g_lastX, dy = y - g_lastY;
		g_lastX = x;
		g_lastY = y;
		if (abs(x - g_downX) + abs(y - g_downY) > 2)
			g_rmbMoved = true;
		switch (g_drag)
		{
		case DRAG_PAN:
		{
			float s = 2.0f * p.halfHeight / v.height;
			p.center = p.center - v.right * (dx * s) + v.up * (dy * s);
			break;
		}
		case DRAG_LOOK:
			p.yaw += dx * 0.005f;
			p.pitch -= dy * 0.005f;
			if (p.pitch > 1.55f) p.pitch = 1.55f;
			if (p.pitch < -1.55f) p.pitch = -1.55f;
			break;
		case DRAG_PAN3D:
		{
			float s = 0.4f;
			p.eye = p.eye - v.right * (dx * s) + v.up * (dy * s);
			break;
		}
		case DRAG_BOX:
			g_dragMoved = true;
			break;
		case DRAG_AREA_NEW:
		{
			WVector now;
			if (PlanePoint(v, x, y, g_areaStart, &now))
			{
				WVector lo(min(g_areaStart.x, now.x), min(g_areaStart.y, now.y), min(g_areaStart.z, now.z));
				WVector hi(max(g_areaStart.x, now.x), max(g_areaStart.y, now.y), max(g_areaStart.z, now.z));
				if (hi.y - lo.y < 1.0f)
					hi.y = lo.y + 20.0f;
				c.wep.elements[g_areaElement].SetBox(lo, hi);
				EdValuesChanged();
			}
			break;
		}
		case DRAG_ITEMS:
		{
			if (!g_dragMoved && abs(x - g_downX) + abs(y - g_downY) < 3)
				break;
			if (!g_dragMoved)
			{
				c.Checkpoint(g_ed.tool == TOOL_ROTATE ? "Rotate" : g_ed.tool == TOOL_SCALE ? "Scale" : "Move");
				g_dragMoved = true;
			}
			if (g_ed.tool == TOOL_ROTATE)
			{
				int axis = GetKeyState(VK_CONTROL) < 0 ? 0 : GetKeyState(VK_SHIFT) < 0 ? 2 : 1;
				ApplyRotate((x - g_downX) * 0.75f, axis);
			}
			else if (g_ed.tool == TOOL_SCALE)
			{
				WVector now;
				WVector delta;
				if (PlanePoint(v, x, y, g_dragPlaneStart, &now))
					delta = now - g_dragPlaneStart;
				ApplyScale((float)pow(1.01, (x - g_downX) - (y - g_downY)), delta);
			}
			else if (p.kind == PANE_PERSP && GetKeyState(VK_SHIFT) < 0)
			{
				float s = 0.25f;
				ApplyMove(WVector(0, -(y - g_downY) * s, 0));
			}
			else
			{
				WVector now;
				if (PlanePoint(v, x, y, g_dragPlaneStart, &now))
					ApplyMove(now - g_dragPlaneStart);
			}
			EdValuesChanged();
			break;
		}
		default:
			break;
		}
		// Moving a pane's own camera changes nothing the others draw.
		if (g_drag == DRAG_PAN || g_drag == DRAG_LOOK || g_drag == DRAG_PAN3D)
			InvalidateRect(hwnd, 0, FALSE);
		else
			ViewsRedraw();
		return 0;
	}
	case WM_LBUTTONUP:
	{
		ReleaseCapture();
		if (g_drag == DRAG_BOX && g_dragMoved)
		{
			int x0 = min(g_boxX, GET_X_LPARAM(lp)), x1 = max(g_boxX, GET_X_LPARAM(lp));
			int y0 = min(g_boxY, GET_Y_LPARAM(lp)), y1 = max(g_boxY, GET_Y_LPARAM(lp));
			SceneView v = ViewOf(p);
			for (size_t i = 0; i < c.wep.elements.size(); ++i)
			{
				const WepElement& e = c.wep.elements[i];
				if (e.IsGame() || e.IsGround())
					continue;
				WVector at = e.IsArea() ? WVector() : e.GetVec("Pos");
				if (e.IsArea())
				{
					WVector lo, hi;
					e.GetBox(&lo, &hi);
					at = (lo + hi) * 0.5f;
				}
				float sx, sy;
				if (SceneProject(v, at, &sx, &sy) && sx >= x0 && sx <= x1 && sy >= y0 && sy <= y1)
				{
					ElementRef r(ElementRef::ELEMENT, (int)i);
					if (!g_ed.IsSelected(r))
						g_ed.sel.push_back(r);
				}
			}
			EdSelectionChanged();
		}
		if (g_drag == DRAG_ITEMS && g_dragMoved)
			EdSelectionChanged();
		g_drag = DRAG_NONE;
		ViewsRedraw();
		return 0;
	}
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	{
		ReleaseCapture();
		bool menu = msg == WM_RBUTTONUP && !g_rmbMoved;
		g_drag = DRAG_NONE;
		if (menu)
			ContextMenu(hwnd, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
		return 0;
	}
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

static LRESULT CALLBACK PreviewProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_ERASEBKGND)
		return 1;
	if (msg == WM_PAINT)
	{
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		if (g_rc && wglMakeCurrent(g_previewDc, g_rc))
		{
			RECT rc;
			GetClientRect(hwnd, &rc);
			SceneDrawPreview(g_previewAsset, g_previewYaw, rc.right > 1 ? rc.right : 1, rc.bottom > 1 ? rc.bottom : 1);
			SwapBuffers(g_previewDc);
		}
		return 0;
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

static void SetFormat(HDC dc)
{
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	int pf = ChoosePixelFormat(dc, &pfd);
	SetPixelFormat(dc, pf, &pfd);
}

void ViewsRegister(HINSTANCE inst)
{
	WNDCLASSW wc;
	memset(&wc, 0, sizeof(wc));
	wc.style = CS_OWNDC | CS_DBLCLKS;
	wc.lpfnWndProc = PaneProc;
	wc.hInstance = inst;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.lpszClassName = L"FairwayED_Pane";
	RegisterClassW(&wc);
	wc.lpfnWndProc = PreviewProc;
	wc.lpszClassName = L"FairwayED_Preview";
	RegisterClassW(&wc);
}

void ViewsCreate(HWND parent)
{
	static const PaneShade shades[PANE_COUNT] = { SHADE_WIRE, SHADE_WIRE, SHADE_WIRE, SHADE_TEXTURED };
	for (int i = 0; i < PANE_COUNT; ++i)
	{
		Pane& p = g_panes[i];
		p.kind = (PaneKind)i;
		p.shade = shades[i];
		p.halfHeight = 400.0f;
		p.yaw = 0.0f;
		p.pitch = -0.5f;
		p.fov = 60.0f;
		p.eye = WVector(0, 200, -500);
		p.wnd = CreateWindowExW(WS_EX_CLIENTEDGE, L"FairwayED_Pane", L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
			0, 0, 10, 10, parent, 0, g_ed.inst, 0);
		p.dc = GetDC(p.wnd);
		SetFormat(p.dc);
		if (!g_rc)
			g_rc = wglCreateContext(p.dc);
	}
	wglMakeCurrent(g_panes[0].dc, g_rc);
}

HWND ViewsPreviewCreate(HWND parent)
{
	g_preview = CreateWindowExW(WS_EX_CLIENTEDGE, L"FairwayED_Preview", L"", WS_CHILD | WS_CLIPSIBLINGS, 0, 0, 10, 10, parent, 0, g_ed.inst, 0);
	g_previewDc = GetDC(g_preview);
	SetFormat(g_previewDc);
	return g_preview;
}

void ViewsPreviewSet(const std::string& name)
{
	g_previewAsset = name.empty() ? 0 : g_ed.course.Puppet(name);
	if (g_preview)
		InvalidateRect(g_preview, 0, FALSE);
}

void ViewsStepEffects(float ms)
{
	SceneView v = ViewOf(g_panes[PANE_PERSP]);
	if (wglMakeCurrent(g_panes[PANE_PERSP].dc, g_rc))
		AreaFxTick(ms, v.eye, v.forward);
}

// A timer: fly keys while the 3D camera is held, the fly-through, the preview's turntable,
// and the effects, which only the 3D pane shows.
void ViewsTick()
{
	static DWORD last = GetTickCount();
	DWORD now = GetTickCount();
	float dt = (now - last) / 1000.0f;
	last = now;
	if (dt > 0.2f)
		dt = 0.2f;
	Pane& fxPane = g_panes[PANE_PERSP];
	if (g_ed.showFx && IsWindowVisible(fxPane.wnd))
	{
		ViewsStepEffects(dt * 1000.0f);
		if (AreaFxParticles() > 0)
			InvalidateRect(fxPane.wnd, 0, FALSE);
	}
	if (g_previewAsset && g_preview && IsWindowVisible(g_preview))
	{
		g_previewYaw += dt * 30.0f;
		InvalidateRect(g_preview, 0, FALSE);
	}
	Pane& p = g_panes[PANE_PERSP];
	if (g_drag == DRAG_LOOK)
	{
		SceneView v = ViewOf(p);
		float speed = (GetKeyState(VK_SHIFT) < 0 ? 400.0f : 120.0f) * dt;
		bool moved = false;
		WVector step;
		if (GetKeyState('W') < 0) { step = step + v.forward; moved = true; }
		if (GetKeyState('S') < 0) { step = step - v.forward; moved = true; }
		if (GetKeyState('D') < 0) { step = step + v.right; moved = true; }
		if (GetKeyState('A') < 0) { step = step - v.right; moved = true; }
		if (GetKeyState('E') < 0) { step = step + WVector(0, 1, 0); moved = true; }
		if (GetKeyState('Q') < 0) { step = step - WVector(0, 1, 0); moved = true; }
		if (moved)
		{
			p.eye = p.eye + step * speed;
			InvalidateRect(p.wnd, 0, FALSE);
		}
	}
	if (g_flying)
	{
		const std::vector<WepNodePoint>& pts = g_ed.course.wep.nodes[g_flyNode].points;
		g_flyAt += dt * 50.0f;
		float remaining = g_flyAt;
		size_t k = 0;
		for (; k + 1 < pts.size(); ++k)
		{
			float len = (pts[k + 1].pos - pts[k].pos).Length();
			if (remaining < len)
				break;
			remaining -= len;
		}
		if (k + 1 >= pts.size())
		{
			g_flying = false;
			return;
		}
		WVector a = pts[k].pos, b = pts[k + 1].pos;
		float len = (b - a).Length();
		WVector at = a + (b - a) * (len > 0.0f ? remaining / len : 0.0f);
		g_ed.course.DropToGround(&at);
		p.eye = at + WVector(0, 14, 0);
		WVector f = b - a;
		f.Normalize();
		p.yaw = (float)atan2(f.x, f.z);
		p.pitch = -0.1f;
		InvalidateRect(p.wnd, 0, FALSE);
	}
}
