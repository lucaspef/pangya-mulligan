#pragma once
#include "wmath.h"
#include "wtypes.h"
#include "wresrcmng.h"
#include "wvideo.h"

class WView : public WResource
{
public:
	enum PROJECTION_MODE
	{
		PERSPECTIVE = 0,
		PARALLEL = 1
	};

	WView();

	virtual void BeginScene();
	virtual void Clear(ulong flags, int color);	// 983: + caso 8 (limpa com o tipo 4)
	virtual void EndScene();
	void Update(unsigned long, int);
	virtual void Flush(ulong flags, unsigned char flag983 = 0);	// 983 (00b4fee0): 2º argumento vai como param2 do W_VDEV_FLUSH; nome e padrão provisórios
	virtual void Render();
	virtual void DrawPolygonFan(WTVertex** vertices, int count, int flags,
		bool transform);
	// 983: + nº de coordenadas de textura e material de shader (repassados ao WVideoDev; nomes provisórios)
	virtual void DrawIndexedTriangles(WTVertex* vertices, int vertexCount,
		unsigned short* indices, int indexCount, int flags, int transform,
		int nTexCoord983 = 0, WxShaderMtl* pShaderMtl983 = 0);
	void DrawProjPolygonFan(WTVertex**, int, int);
	virtual bool IsShadowView(void) { return false; }
	virtual void DrawIndexedTrianglesDirect(WTVertex*, int, unsigned short*,
		int, int, int, int = 0, WxShaderMtl* = 0);

	void SetClip(float, float, bool);
	void SetPrevCamera(const WMatrix&);
	void SetCamera(const WMatrix&);
	const WMatrix& GetCamera() const { return camera; }
	const WMatrix& GetInvCamera() const { return invcamera; }
	bool InFrustum(const WSphere& sphere)
	{
		for (int i = 0; i < 6; ++i)
			if (frustum[i] * sphere.pos > sphere.radius)
				return false;
		return true;
	}
#ifndef REBANG_PCH_POOL // HACK: the pool PCH has its own copy (the bytes pool.obj emits), other units keep their IL
	bool InFrustum(const Waabb& box)
	{
		for (int i = 0; i < 6; ++i)
		{
			WVector v(frustum[i].x < 0.0f ? box.max.x : box.min.x,
				frustum[i].y < 0.0f ? box.max.y : box.min.y,
				frustum[i].z < 0.0f ? box.max.z : box.min.z);
			if (frustum[i] * v > 0.0f)
				return false;
		}
		return true;
	}
#endif
	bool InFrustum(const WVector& pos);
#ifdef REBANG_PCH_POOL // HACK: only in the pool PCH, other units keep their IL and name count
	bool InFrustum(const Waabb& box);
#endif
	bool InFrustumSafe(const WSphere& sphere)
	{
		for (int i = 0; i < 6; ++i)
			if (frustumSafe[i] * sphere.pos + sphere.radius >= 0.0f)
				return false;
		return true;
	}
	bool ProcessEffect() { return m_bProcessEffect; }
	bool GetReflective() const;
#ifndef REBANG_PCH_POOL
	void SetReflective(bool);
#endif
	bool SetFogEnable(bool);
	void SetFogState(float, float, unsigned long);
	void SetViewport(float, float);
	WVector2D GetScreenCenter() const;
	void SetFOV(float);
	void DrawLine2D(const _WPOINT&, const _WPOINT&, unsigned long, int);
	void DrawLine2D(const _WPOINT&, const _WPOINT&, unsigned long,
		unsigned long, int);
	void Draw2DTexture(const WRect&, const WRect&, int, unsigned long,
		unsigned long);
	void DrawLine(const WVector&, unsigned long, const WVector&, unsigned long,
		int, float width = 1.0f); // 983 00b50d70: width > 1 = faixa 3D com recorte no plano near
	void DrawAABB(const Waabb&, unsigned long, bool, int);
	void DrawAABB(WMatrix&, const Waabb&, unsigned long, bool, int);
	void DrawOBB(const WMatrix&, unsigned long);
	void DrawOBB(const Wobb&, unsigned long);
	void DrawSphere(const WVector&, float, unsigned long, int, int);
	void DrawSphere(const WSphere&, unsigned long, int, int);
	void SetScreenCenter(float, float);
	void UpdateCamera(void);
	void GetScreen2World(float, float, WVector&, WVector&);
	void xSetLight(unsigned long, const LightSet&);
	LightSet* xGetLight(void);
	void xScaleProjMat(int, int);
	void SetScale(float);
	float GetScale() const { return m_scale; }
#if defined(REBANG_CLIENTSETTING_PCH) || defined(REBANG_GOLFRULE_PCH)
	// HACK: inline here without emitting the COMDAT owned by a later unit.
	__declspec(dllimport) __forceinline float GetFOV_Unmodified() const
	{
		return FOV;
	}
#else
	float GetFOV_Unmodified() const { return FOV; }
#endif
	PROJECTION_MODE GetProjectionMode() const;
	void SetProjectionMode(PROJECTION_MODE);
	WVector Projection(const WVector&);
	void Projection2(WTVertex*, const WVector&);
	// 983 006cc860 (so o C3dFont::Printf usa): z do espaco da camera normalizado por [clip_near, clip_far]
	float GetNormalizedDepth983(const WVector& v) const
	{
		float z = v.y * matrix.ya.z + v.x * matrix.xa.z + v.z * matrix.za.z + matrix.pivot.z;
		return (z - clip_near) / (clip_far - clip_near);
	}
	void SetClippingArea(const WRect&);
	void ResetClippingArea();
	void xDrawIndexedTriangles(const WxBatchState&);

protected:
	void CheckReflectiveAndConvertCullFlag(int& flag) const
	{
		if (m_isReflective)
		{
			// 983: os bits de face (flag1) são 0x200/0x400; no 645 eram 0x400/0x800
			switch (flag & 0x600)
			{
			case 0:
				flag |= 0x400;
				break;
			case 0x400:
				flag &= ~0x400;
				break;
			}
		}
	}

	void SetFOV_Unmodified(float);
	void CheckViewAndProjTransformUpdateToVideo(void);
	WVector Projection_Perspective(const WVector&);
	WVector Projection_Parallel(const WVector&);
	void Projection2_Perspective(WTVertex*, const WVector&);
	void Projection2_Parallel(WTVertex*, const WVector&);
	void GetScreen2World_Perspective(float, float, WVector&, WVector&);
	void GetScreen2World_Parallel(float, float, WVector&, WVector&);
	void UpdateProjectionTransform_Perspective(void);
	void UpdateProjectionTransform_Parallel(void);
	void UpdateViewTransform(void);
	void UpdateCamera_Perspective(void);
	void UpdateCamera_Parallel(void);
	void ClipPlane(WTVertex*, const WTVertex*, const WTVertex*, int,
		const WPlane&);

	static WVector (WView::* ms_fnProjection[2])(const WVector&);
	static void (WView::* ms_fnProjection2[2])(WTVertex*, const WVector&);
	static void (WView::* ms_fnUpdateProjectionTransform[2])(void);
	static void (WView::* ms_fnUpdateCamera[2])(void);
	static void (
		WView::* ms_fnGetScreen2World[2])(float, float, WVector&, WVector&);

public:
	static const WMatrix4 ms_uvConvMat;

	bool InFrustumSafe(const Waabb& box);
#ifdef REBANG_CLIENTSETTING_PCH
	// HACK: inline here without emitting the COMDATs owned by a later unit.
	__declspec(dllimport) __forceinline float GetWidth() const
	{
		return SCREEN_XS;
	}
	__declspec(dllimport) __forceinline float GetHeight() const
	{
		return SCREEN_YS;
	}
	__declspec(dllimport) __forceinline float GetRatio() const
	{
		return SCREEN_YS / SCREEN_XS;
	}
#else
	__forceinline float GetWidth() const { return SCREEN_XS; }
	__forceinline float GetHeight() const { return SCREEN_YS; }
	float GetRatio() const { return SCREEN_YS / SCREEN_XS; }
#endif
	float GetClipNearValue() const { return clip_near; }
	float GetClipFarValue() const { return clip_far; }
	void ResetScreenCenter()
	{
		SetScreenCenter(SCREEN_XS * 0.5f, SCREEN_YS * 0.5f);
	}
#ifdef REBANG_PCH_GETFOV
	// HACK: original GetFOV is a PCH inline; keep its body at the WView definition.
	float GetFOV() const
	{
		float ratio = GetRatio();
		return ratio * FOV * (4.0f / 3.0f);
	}
#else
	float GetFOV() const;
#endif
	__forceinline WVideoDev* GetVideoDevice() const
	{
		return GetResrcManager()->video;
	}
	float xGetProjScale() const { return proj_scale; }
#ifdef REBANG_PCH_POOL // HACK: only in the pool PCH, other units keep their IL and name count
	void SetReflective(bool b) { m_isReflective = b; }
#endif
	const WxViewState& xGetViewState() const { return m_xViewState; }

	void xConvScreenRectByProjScale(WRect& rc) const
	{
		if (proj_scale > 1.0f)
		{
			float xs = SCREEN_XS;
			rc.x = (rc.x - (left + 0.5f) * xs) * proj_scale;
			float ys = SCREEN_YS;
			rc.y = (rc.y - (top + 0.5f) * ys) * proj_scale;
			rc.w = rc.w * proj_scale;
			rc.h = rc.h * proj_scale;
		}
	}

protected:
	const WMatrix& GetLastCamera() { return lastcam; }
	float CalcSZ(float);

public:
	WMatrix camera;
	WMatrix invcamera;

protected:
	WMatrix matrix;
	WPlane frustum[6];
	WPlane frustumSafe[6];
	float scalex;
	float scaley;

public: // lidos direto pela regua do Hole-in-One (ProjectG fontinfo, 983 0052bf60: view+0x184/+0x188)
	float m_center_x;
	float m_center_y;

public:
	float clip_near;
	float clip_far;

protected:
	float clip_scaled_near;
	float clip_scaled_far;

public: // read directly by CGolfDoc::MakeTopViewCameraMatrix (ProjectG)
	float SCREEN_XS;
	float SCREEN_YS;

protected:
	unsigned int m_cliptype;
	bool m_fastclip;
	WRect m_clipArea;
	float clip_scale_z;
	float clip_near_scale;

public: // idem
	float FOV;

protected:
	WVector m_temp[512];
	WMatrix lastcam;
	int update;
	float proj_scale;
	float left;
	float right;
	float top;
	float bottom;
	bool m_bProcessEffect;
	bool m_bDisableFog;
	bool m_isReflective;
	WxViewState m_xViewState;
	bool m_xNeedToUpdateViewTransfToVideo;
	bool m_xNeedToUpdateProjTransfToVideo;
	WView::PROJECTION_MODE m_projMode;
	float m_scale;
	WPlane frustumByCam[6];

public:
	__forceinline virtual ~WView();
};

#ifdef REBANG_PCH_POOL
inline bool WView::InFrustum(const Waabb& box)
{
	const WPlane* p = frustum;
	for (int i = 0; i < 6; ++i, ++p)
	{
		WVector v(p->x < 0.0f ? box.max.x : box.min.x,
			p->y < 0.0f ? box.max.y : box.min.y,
			p->z < 0.0f ? box.max.z : box.min.z);
		if (p->normal * v + p->dis > 0.0f)
			return false;
	}
	return true;
}
#endif

class WViewOrth : public WView
{
public:
	virtual ~WViewOrth() { }
};

inline bool WView::GetReflective() const
{
	return m_isReflective;
}

inline WView::PROJECTION_MODE WView::GetProjectionMode() const
{
	return m_projMode;
}

WMatrix __cdecl MakeCameraMatrix(const WVector& eye, const WVector& at);

inline unsigned long ULblend(unsigned long c1, unsigned long c2,
	unsigned char t)
{
	return ((((c1 & 0xff00ff) * (255 - t) + (c2 & 0xff00ff) * t) >> 8) &
			   0xff00ff) |
		((((c1 >> 8) & 0xff00ff) * (255 - t) + ((c2 >> 8) & 0xff00ff) * t) &
			0xff00ff00);
}
