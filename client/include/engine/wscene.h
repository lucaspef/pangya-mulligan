#pragma once
#include <stddef.h>
#include "wmath.h"

struct LightSet
{
	unsigned long type;
	WVector nearOne;
	unsigned long diffuse;
	unsigned long ambient;
	unsigned long ambient2;

	LightSet()
	{
		type = 0;
		diffuse = 0;
		ambient = 0;
		ambient2 = 0xffffff;
		nearOne.x = nearOne.y = nearOne.z = 0.0f;
	}
	void operator=(const LightSet& other)
	{
		memcpy(this, &other, sizeof(LightSet));
	}
	// 983: novo (10012930)
	bool operator==(const LightSet& other) const
	{
		if (type == other.type &&
			WisEqual(nearOne, other.nearOne, g_EPSILON) &&
			diffuse == other.diffuse && ambient == other.ambient &&
			ambient2 == other.ambient2)
			return true;
		return false;
	}
};

struct WxShaderMtl;

struct WxBatchState
{
	WxShaderMtl* xpShaderMtl; // 983: campo novo em +0 (material de shader; nome provisório); sizeof 92 -> 96
	int xiFlag0;
	int xiFlag1;
	int xiBaseVtxIdx;
	int xnVtxs;
	int xiBaseIdxIdx;
	int xnIdxs;
	ulong xdwDiffuse;
	WMatrix xmW;
	float xfDepth;
	int xnmTransfs;
	const WMatrix* const* xpapmW;
	const WMatrix* const* xpapmO;

	WxBatchState()
	{
		xpShaderMtl = 0;
		xiFlag0 = 0;
		xiFlag1 = 0;
		xiBaseVtxIdx = 0;
		xnVtxs = 0;
		xiBaseIdxIdx = 0;
		xnIdxs = 0;
		xdwDiffuse = 0xffffffff;
		xfDepth = g_HUGE;
		xnmTransfs = 0;
		xpapmW = 0;
		xpapmO = 0;
		xmW.Reset();
	}
	// 983: novo (10012a80): "other" continua este lote (mesmos estados, vértices e índices em sequência);
	// usado para juntar lotes no FlushMultiPass. Nome provisório.
	bool IsFollowedBy(const WxBatchState& other) const
	{
		if (xpShaderMtl == other.xpShaderMtl && xiFlag0 == other.xiFlag0 &&
			xiFlag1 == other.xiFlag1 &&
			xiBaseVtxIdx + xnVtxs == other.xiBaseVtxIdx &&
			xiBaseIdxIdx + xnIdxs == other.xiBaseIdxIdx &&
			xdwDiffuse == other.xdwDiffuse && xnmTransfs == other.xnmTransfs)
		{
			if (xnmTransfs == 0)
			{
				if (xmW != other.xmW)
					return false;
			}
			else if (xpapmW != other.xpapmW || xpapmO != other.xpapmO)
				return false;
			return true;
		}
		return false;
	}
};

// A337: contrato com a wangreal.dll do 983 (ApplyMtlParams 100098a0 / xApplyShaderSysParams 1003c750: sem.5 = xdwDiffuse
// em +0x1c; sem.6/7/8 = LightSet +4/+0x10/+0x14)
static_assert(offsetof(LightSet, nearOne) == 0x4 && offsetof(LightSet, diffuse) == 0x10 &&
	offsetof(LightSet, ambient) == 0x14 && offsetof(LightSet, ambient2) == 0x18 && sizeof(LightSet) == 0x1c, "LightSet 983");
static_assert(offsetof(WxBatchState, xdwDiffuse) == 0x1c && offsetof(WxBatchState, xmW) == 0x20 &&
	offsetof(WxBatchState, xfDepth) == 0x50 && sizeof(WxBatchState) == 0x60, "WxBatchState 983");

struct WxViewState
{
	LightSet xLight;
	WMatrix4 xmView;
	WMatrix4 xmProj;
	// 983: novo (10012990). As matrizes são comparadas com elas mesmas no original (sempre iguais): mantido.
	bool operator==(const WxViewState& other) const
	{
		if (xLight == other.xLight && WisEqual(xmView, xmView, g_EPSILON) &&
			WisEqual(xmProj, xmProj, g_EPSILON))
			return true;
		return false;
	}
};

class WScene
{
public:
	virtual ~WScene();
	virtual LightSet* GetLightSet(const WVector& pos);
	virtual float CollTest(const WVector&, const WVector&, WPlane*, float);
};
