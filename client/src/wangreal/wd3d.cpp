#include <new>
#include <string>
#include "wd3d.h"
#include <cmath>
#include <wutil.h>
#include <algorithm>
WDirect3D::WDirect3D()
{
	m_mainThreadId = 0;
	m_chVertex = 0;
	m_dwOffset = 0;
	m_bufPolySwNum = m_bufPolyHwNum = m_bufSortNum = 0;
	m_texCount1 = 128;
	m_texCount2 = 1;
	m_useHiQualityTex = false;
	for (unsigned i = 0; i < 2048; i++)
		m_texList[i] = TS_VOID;
	m_MaxTextureBlendStages = 0;
	ApplyGamma(1.3f);
	m_fog = false;
	m_fogColor = 0;
	m_fogEnd = m_fogStart = 0;
	m_rs4FlushingEx = 0;
	m_xiVbCount = m_xiIbCount = 1;
	memset(m_xahVbs, 0, sizeof(m_xahVbs));
	memset(m_xahIbs, 0, sizeof(m_xahIbs));
	m_xahVbs[0] = m_xahIbs[0] = 1;
}

WDirect3D::~WDirect3D()
{
	this->WDirect3D::Release();
}

void WDirect3D::Init()
{
	this->m_chVertex = new char[0x100000];
	this->m_devState = W_VDEVSTATE_NORMAL;
	this->m_buf4sort.resize(0x1200);
	this->m_bufPolySw.resize(0x1000);
	this->m_bufPolyHw.resize(0x200);
}

void WDirect3D::Release()
{
	for (unsigned int i = 0; i < 0x800; i++)
	{
		if (this->m_texList[i])
		{
			this->DestroyTexture(i);
			this->m_texList[i] = TS_VOID;
		}
	}
	for (unsigned int i = 1; i < 0x400; i++)
	{
		if (this->m_xahVbs[i])
		{
			this->xReleaseVertexBuffer(i);
			this->m_xahVbs[i] = 0;
		}
	}
	for (int i = 1; i < 0x200; i++)
	{
		if (this->m_xahIbs[i])
		{
			this->xReleaseIndexBuffer(i);
			this->m_xahIbs[i] = 0;
		}
	}
	if (this->m_chVertex)
	{
		delete[] this->m_chVertex;
		this->m_chVertex = 0;
	}
}

// 983: um ModifyVertices só (no 645 eram dois, com um caso por FVF): monta os vértices no formato pedido,
// campo a campo, a partir dos bits do FVF. 10013910
void* WDirect3D::ModifyVertices(const WTVertexList& vl, unsigned int n,
	unsigned long dwVertexTypeDesc, bool store)
{
	unsigned long dwOffset = this->m_dwOffset;
	if ((int)dwOffset >= 0xCCCCC)
	{
		this->Flush(0, true);
		dwOffset = 0;
	}

	char* result = &this->m_chVertex[dwOffset];
	// W4: o 983 grava o FVF ajustado no próprio parâmetro e passa ESTE ao VertexSize (a versão anterior passava o
	// original: sem bits de posição, o passo entre vértices saía 16 B menor que o do 983)
	dwVertexTypeDesc = (dwVertexTypeDesc & D3DFVF_POSITION_MASK)
		? dwVertexTypeDesc
		: dwVertexTypeDesc | D3DFVF_XYZRHW;
	const unsigned long fvf = dwVertexTypeDesc;
	unsigned long size = this->VertexSize(dwVertexTypeDesc);
	unsigned long posType = fvf & D3DFVF_POSITION_MASK;
	unsigned long posSize = posType == D3DFVF_XYZRHW ? 16 : 12;
	unsigned int i;
	char* dst;

	dst = result;
	for (i = 0; i < n; i++, dst += size)
		memcpy(dst, &vl[i], posSize);

	unsigned long pos = posSize;
	if (fvf & D3DFVF_DIFFUSE)
	{
		dst = result + posSize;
		for (i = 0; i < n; i++, dst += size)
			*(unsigned long*)dst = vl[i].diffuse;
		pos = posSize + 4;
	}
	if (fvf & (0x80000000 | D3DFVF_SPECULAR))
	{
		if (posType == D3DFVF_XYZRHW)
		{
			dst = result + pos + 3;
			for (i = 0; i < n; i++, dst += size)
				*(unsigned char*)dst = this->CalcFog(vl[i].z);
		}
		pos += 4;
	}
	unsigned long texCount = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	if (texCount > 0)
	{
		size_t extra = texCount <= 1 ? 0 : texCount * 8 - 8;
		dst = result + pos;
		for (i = 0; i < n; i++, dst += size)
		{
			memcpy(dst, &vl[i].tu, 8);
			memcpy(dst + 8, &vl[i].lu, extra);
		}
	}
	if (store)
		this->m_dwOffset = dwOffset + size * n;
	return result;
}

void WDirect3D::ApplyGamma(float gamma)
{
	for (int i = 0; i < 256; i++)
	{
		this->m_iRefTable[i] = Between<int>(0,
			static_cast<int>(
				pow(static_cast<double>(i) * 0.00390625, 1.0 / gamma) * 256.0),
			255);
	}
}

bool WDirect3D::SetFogEnable(bool enable)
{
	bool old = m_fog;
	m_fog = enable;
	return old;
}

void WDirect3D::SetFogState(float fogStart, float fogEnd, unsigned long color)
{
	this->m_fogStart = fogStart;
	this->m_fogEnd = fogEnd;
	this->m_fogColor = color & 0xFFFFFF;
}

unsigned char WDirect3D::CalcFog(float depth)
{
	if (this->m_clip_near_scale == 0.0f)
	{
		this->m_clip_near_scale = 0.1f;
	}
	float x = this->m_clip_scale_z - depth;
	if (x == 0.0f)
	{
		x = 0.0000001f;
	}
	float d = this->m_clip_near_scale / static_cast<float>(x);
	if (d > this->m_fogEnd)
	{
		return 0;
	}
	if (d <= this->m_fogStart)
		return 255;
	{
		return static_cast<unsigned char>(
			static_cast<int>((this->m_fogEnd - d) * 255.0f /
				(this->m_fogEnd - this->m_fogStart)));
	}
	return -1;
}

inline float WDirect3D::CalcDepth(WTVertex** p, int num)
{
	float n = 0.0f;
	for (int i = 0; i < num; i++)
	{
		n += p[i]->vz;
	}
	return static_cast<float>(n / static_cast<float>(num));
}

// 983: sem os casos de multitextura por software do 645; texCount/pShaderMtl são novos (nomes provisórios)
void WDirect3D::DrawPolygonFan(WTVertex** p, int iType, int iNum, int iType2,
	unsigned long dwVertexTypeDesc, int texCount, WxShaderMtl* pShaderMtl)
{
	WTVertex* t[64];
	D3DPRIMITIVETYPE dptPrimitiveType;

	if (this->m_devState == W_VDEVSTATE_LOST)
	{
		return;
	}

	if (iType & 0x8000000)
	{
		if (iNum == 2)
		{
			dptPrimitiveType = D3DPT_LINESTRIP;
		}
		else
		{
			int i;
			for (i = 0; i < iNum; i++)
				t[i] = p[i];
			t[i] = p[0];
			p = t;
			++iNum;
			dptPrimitiveType = D3DPT_LINESTRIP;
		}
	}
	else
	{
		dptPrimitiveType = D3DPT_TRIANGLEFAN;
	}
	if (!(iType & 0x40000000))
		dwVertexTypeDesc |= D3DFVF_DIFFUSE;
	if (pShaderMtl)
		dwVertexTypeDesc |= texCount << D3DFVF_TEXCOUNT_SHIFT;
	else
		dwVertexTypeDesc |=
			(iType & 0x3F800) ? 0x200 : ((iType & 0x7FF) ? 0x100 : 0);

	if (iType & 0x10000000)
	{
		if (this->m_fog)
		{
			dwVertexTypeDesc |= 0x80000080;
		}
		else
		{
			iType &= ~0x10000000;
		}
	}
	if (iType & 0x400000)
	{
		float pa;
		if ((iType & 0x300000) == 0x300000 || (iType & 0x300000) == 0x100000)
		{
			pa = 0.0;
		}
		else
		{
			pa = this->CalcDepth(p, iNum);
		}
		this->Buffering(iType, iType2, iNum, dwVertexTypeDesc,
			this->ModifyVertices(p, iNum, dwVertexTypeDesc, true),
			dptPrimitiveType, pa, pShaderMtl);
	}
	else
	{
		this->DrawPrimitive(iType, iNum, dwVertexTypeDesc,
			this->ModifyVertices(p, iNum, dwVertexTypeDesc, false),
			dptPrimitiveType, iType2, pShaderMtl);
	}
}

void WDirect3D::DrawIndexedTriangles(WTVertex* p, int pNum,
	unsigned short* fList, int fNum, int iType, int iType2, int texCount,
	WxShaderMtl* pShaderMtl)
{
	if (m_devState != W_VDEVSTATE_LOST)
	{
		unsigned long dwVertexTypeDesc = D3DFVF_XYZ;
		if (!(iType & 0x40000000))
			dwVertexTypeDesc |= D3DFVF_DIFFUSE;
		if (pShaderMtl)
			texCount <<= D3DFVF_TEXCOUNT_SHIFT;
		else
			texCount = (iType & 0x3F800) ? 0x200 : ((iType & 0x7FF) ? 0x100 : 0);
		dwVertexTypeDesc |= texCount;
		if (iType & 0x10000000)
		{
			if (m_fog)
				dwVertexTypeDesc |= 0x80000080;
			else
				iType &= ~0x10000000;
		}
		if (iType & 0x400000)
		{
			WTVertex* t[3];
			WTVertexList vl(t);
			for (int i = 0; i < fNum;)
			{
				t[0] = &p[fList[i++]];
				t[1] = &p[fList[i++]];
				t[2] = &p[fList[i++]];
				float depth;
				if ((iType & 0x300000) == 0x300000 ||
					(iType & 0x300000) == 0x100000)
					depth = 0.0f;
				else
					depth = CalcDepth(t, 3);
				Buffering(iType, iType2, 3, dwVertexTypeDesc,
					ModifyVertices(vl, 3, dwVertexTypeDesc, true),
					D3DPT_TRIANGLEFAN, depth, pShaderMtl);
			}
		}
		else if (!(iType & 0x8000000))
		{
			DrawPrimitiveIndexed(iType, dwVertexTypeDesc,
				ModifyVertices(p, pNum, dwVertexTypeDesc, false), pNum, fList,
				fNum, iType2, pShaderMtl);
		}
		else
		{
			WTVertex* t[4];
			WTVertexList vl(t);
			for (int i = 0; i < fNum;)
			{
				t[0] = &p[fList[i++]];
				t[1] = &p[fList[i++]];
				t[2] = &p[fList[i++]];
				t[3] = t[0];
				DrawPrimitive(iType, 4, dwVertexTypeDesc,
					ModifyVertices(vl, 4, dwVertexTypeDesc, false),
					D3DPT_LINESTRIP, iType2, pShaderMtl);
			}
		}
	}
}

void WDirect3D::DrawBuffered(const w_poly_buffer_sw& bufSw)
{
	this->ApplyCustomRenderState(bufSw.customRenderState);
	this->DrawPrimitive(bufSw.type, bufSw.num, bufSw.dwVertexTypeDesc,
		bufSw.lpvVertices, bufSw.dptPrimitiveType, bufSw.type2,
		bufSw.pShaderMtl);
}

void WDirect3D::DrawBuffered(const w_poly_buffer_hw& bufHw)
{
	this->ApplyCustomRenderState(bufHw.customRenderState);
	this->xDrawIndexedPrimitive(bufHw.VState, bufHw.BState);
}

int __cdecl WDirect3D::ComparePolyDepth(const void* elem1, const void* elem2)
{
	const w_poly_buffer* fElem1 =
		*static_cast<const w_poly_buffer* const*>(elem1);
	const w_poly_buffer* fElem2 =
		*static_cast<const w_poly_buffer* const*>(elem2);

	if (fElem1->depth > fElem2->depth)
	{
		return -1;
	}

	if (fElem1->depth < fElem2->depth)
	{
		return 1;
	}

	return fElem1 >= fElem2 ? 1 : -1;
}

void WDirect3D::Flush(unsigned long flag, bool all)
{
	this->FlushRenderPrimitive();
	ClearVertices();
	if (this->m_devState != W_VDEVSTATE_LOST)
	{
		this->BeginUsingCustomRenderState();
		this->FlushEqual();
		this->FlushRenderPrimitive();
		if (this->m_bufSortNum > 0)
		{
			if (flag & 1)
			{
				this->FlushMultiPass(flag, all);
			}
			else
			{
				this->FlushOnePass(flag);
			}
			this->FlushRenderPrimitive();
		}
		this->EndUsingCustomRenderState();
	}
	this->ClearCustomRenderStateSnapShotList();

	if (m_bufPolySwNum == m_bufPolySw.size())
	{
		m_bufPolySw.clear();
		m_bufPolySw.resize(m_bufPolySwNum * 1.5);
	}
	if (m_bufPolyHwNum == m_bufPolyHw.size())
	{
		m_bufPolyHw.clear();
		m_bufPolyHw.resize(m_bufPolyHwNum * 1.5);
	}
	if (GetSortBufferHwSize() + GetSortBufferSwSize() != GetSortBufferSize())
	{
		m_buf4sort.clear();
		m_buf4sort.resize(GetSortBufferHwSize() + GetSortBufferSwSize());
	}
	this->m_bufSortNum = 0;
	this->m_bufPolyHwNum = 0;
	this->m_bufPolySwNum = 0;
}

// 983: estados que o flush impõe aos materiais de shader (ids do material: 4 = teste de alfa, 5 = função,
// 6 = referência, 7 = escrita de Z). Funções locais da unidade; nomes provisórios. 10013bb0..10013ca0
static void ForceOpaquePass(WxShaderMtl* mtl)
{
	WxShaderMtlState* alphaTest = mtl->GetBoolParam(4);
	WxShaderMtlState* alphaFunc = mtl->GetIntParam(5);
	WxShaderMtlState* alphaRef = mtl->GetIntParam(6);
	alphaTest->Force(1);
	alphaFunc->Force(D3DCMP_EQUAL);
	alphaRef->Force(0xff);
}

static void UnforceOpaquePass(WxShaderMtl* mtl)
{
	WxShaderMtlState* alphaTest = mtl->GetBoolParam(4);
	WxShaderMtlState* alphaFunc = mtl->GetIntParam(5);
	WxShaderMtlState* alphaRef = mtl->GetIntParam(6);
	alphaTest->Unforce();
	alphaFunc->Unforce();
	alphaRef->Unforce();
}

static void ForceBlendPass(WxShaderMtl* mtl, bool opaqueDrawn)
{
	WxShaderMtlState* alphaTest = mtl->GetBoolParam(4);
	WxShaderMtlState* alphaFunc = mtl->GetIntParam(5);
	WxShaderMtlState* alphaRef = mtl->GetIntParam(6);
	WxShaderMtlState* zWrite = mtl->GetBoolParam(7);
	alphaTest->Force(1);
	alphaFunc->Force(opaqueDrawn ? D3DCMP_LESS : D3DCMP_LESSEQUAL);
	alphaRef->Force(0xff);
	zWrite->Force(0);
}

static void UnforceBlendPass(WxShaderMtl* mtl)
{
	WxShaderMtlState* alphaTest = mtl->GetBoolParam(4);
	WxShaderMtlState* alphaFunc = mtl->GetIntParam(5);
	WxShaderMtlState* alphaRef = mtl->GetIntParam(6);
	WxShaderMtlState* zWrite = mtl->GetBoolParam(7);
	alphaTest->Unforce();
	alphaFunc->Unforce();
	alphaRef->Unforce();
	zWrite->Unforce();
}

// 983: lote de hardware pendente no passe opaco; lotes seguidos com o mesmo estado viram um só desenho.
// Classe local do FlushMultiPass (as funções dela têm convenção de chamada própria no 983); nomes provisórios.
// 10012ed0..10012ef0, 100149e0, 100149f0, 10014e30
// 983: reescrito. Passe 1 (opacos): tudo que não é 0x1800000, na ordem de chegada, com teste de alfa == 0xff nos
// materiais e lotes de hardware juntados; passe 2: tudo, ordenado por profundidade, com mistura. 100150c0
void WDirect3D::FlushMultiPass(unsigned long flag, bool all)
{
	class WPendingBatch
	{
	public:
		WPendingBatch() { Clear(); }
		void Clear() { m_pending = false; }
		bool IsPending() const { return m_pending; }
		w_poly_buffer_hw* Get() { return &m_buf; }
		void Set(const w_poly_buffer_hw& buf)
		{
			m_pending = true;
			m_buf = buf;
		}
		bool Append(const w_poly_buffer_hw& buf)
		{
			if (IsPending())
			{
				if (m_buf.VState == buf.VState &&
					m_buf.BState.IsFollowedBy(buf.BState) &&
					m_buf.customRenderState == buf.customRenderState)
				{
					m_buf.BState.xnVtxs += buf.BState.xnVtxs;
					m_buf.BState.xnIdxs += buf.BState.xnIdxs;
					return true;
				}
				return false;
			}
			Set(buf);
			return true;
		}

	private:
		bool m_pending;
		w_poly_buffer_hw m_buf;
	};
	WPendingBatch pending;
	// uma variável só para os dois passes (mesma posição na pilha no 983)
	bool bLast = this->SetRenderState4Flushing(1);

	int i;
	for (i = 0; i < this->m_bufSortNum; i++)
	{
		w_poly_buffer* buf = m_buf4sort[i];
		if (!buf->bHw)
		{
			w_poly_buffer_sw* swPolyBuf = (w_poly_buffer_sw*)buf;
			if (bLast)
			{
				swPolyBuf->type |= 0x80000000;
			}
			else
			{
				swPolyBuf->type &= ~0x80000000;
			}
			if (!(swPolyBuf->type & 0x01800000) &&
				(all || swPolyBuf->depth < 0.2f))
			{
				if (pending.IsPending())
				{
					w_poly_buffer_hw* hwPending = pending.Get();
					this->DrawBuffered(*hwPending);
					if (hwPending->BState.xpShaderMtl)
						UnforceOpaquePass(hwPending->BState.xpShaderMtl);
					pending.Clear();
				}
				swPolyBuf->bOpaqueDrawn = true;
				if (swPolyBuf->pShaderMtl)
					ForceOpaquePass(swPolyBuf->pShaderMtl);
				this->DrawBuffered(*swPolyBuf);
				if (swPolyBuf->pShaderMtl)
					UnforceOpaquePass(swPolyBuf->pShaderMtl);
			}
		}
		else
		{
			w_poly_buffer_hw* hwPolyBuf = (w_poly_buffer_hw*)buf;
			if (bLast)
			{
				hwPolyBuf->BState.xiFlag0 |= 0x80000000;
			}
			else
			{
				hwPolyBuf->BState.xiFlag0 &= ~0x80000000;
			}
			if (!(hwPolyBuf->BState.xiFlag0 & 0x01800000))
			{
				hwPolyBuf->bOpaqueDrawn = true;
				if (hwPolyBuf->BState.xpShaderMtl && !pending.IsPending())
					ForceOpaquePass(hwPolyBuf->BState.xpShaderMtl);
				if (!pending.Append(*hwPolyBuf))
				{
					w_poly_buffer_hw* hwPending = pending.Get();
					this->DrawBuffered(*hwPending);
					if (hwPending->BState.xpShaderMtl)
						UnforceOpaquePass(hwPending->BState.xpShaderMtl);
					pending.Set(*hwPolyBuf);
					if (hwPolyBuf->BState.xpShaderMtl)
						ForceOpaquePass(hwPolyBuf->BState.xpShaderMtl);
				}
			}
		}
	}
	if (pending.IsPending())
	{
		w_poly_buffer_hw* hwPending = pending.Get();
		this->DrawBuffered(*hwPending);
		if (hwPending->BState.xpShaderMtl)
			UnforceOpaquePass(hwPending->BState.xpShaderMtl);
		pending.Clear();
	}

	this->FlushRenderPrimitive();
	this->ApplyCustomRenderState(0);
	this->SetRenderState4Flushing(2);
	if (this->m_bufSortNum > 0)
		qsort(&this->m_buf4sort[0], this->m_bufSortNum, 4, ComparePolyDepth);
	this->SetRenderState4Flushing(3);

	bLast = true;
	for (i = 0; i < this->m_bufSortNum; i++)
	{
		w_poly_buffer* buf = m_buf4sort[i];
		if (buf->bOpaqueDrawn != bLast)
		{
			if (buf->bOpaqueDrawn)
				this->SetRenderState4Flushing(4);
			else
				this->SetRenderState4Flushing(5);
			bLast = buf->bOpaqueDrawn;
		}
		WxShaderMtl* mtl;
		if (!buf->bHw)
		{
			w_poly_buffer_sw* swPolyBuf = (w_poly_buffer_sw*)buf;
			swPolyBuf->type |= 0x20000000;
			if (swPolyBuf->pShaderMtl)
				ForceBlendPass(swPolyBuf->pShaderMtl, buf->bOpaqueDrawn);
			this->DrawBuffered(*swPolyBuf);
			mtl = swPolyBuf->pShaderMtl;
		}
		else
		{
			w_poly_buffer_hw* hwPolyBuf = (w_poly_buffer_hw*)buf;
			hwPolyBuf->BState.xiFlag0 |= 0x20000000;
			if (hwPolyBuf->BState.xpShaderMtl)
				ForceBlendPass(hwPolyBuf->BState.xpShaderMtl, buf->bOpaqueDrawn);
			this->DrawBuffered(*hwPolyBuf);
			mtl = hwPolyBuf->BState.xpShaderMtl;
		}
		if (mtl)
			UnforceBlendPass(mtl);
	}

	this->FlushRenderPrimitive();
	this->ApplyCustomRenderState(0);
	this->SetRenderState4Flushing(6);
	this->SetRenderState4Flushing(0);
}

// 983: ordena aqui (no 645 o Flush ordenava antes) e desenha tudo com mistura. 10013d80
void WDirect3D::FlushOnePass(unsigned long flag)
{
	if (this->m_bufSortNum > 0)
		qsort(&this->m_buf4sort[0], this->m_bufSortNum, 4, ComparePolyDepth);

	for (int i = 0; i < this->m_bufSortNum; i++)
	{
		if (!m_buf4sort[i]->bHw)
		{
			w_poly_buffer_sw* swPolyBuffer = (w_poly_buffer_sw*)m_buf4sort[i];
			swPolyBuffer->type = 0x20000000 | (swPolyBuffer->type & ~0xA0000000);
			DrawBuffered(*swPolyBuffer);
		}
		else
		{
			w_poly_buffer_hw* hwPolyBuffer = (w_poly_buffer_hw*)m_buf4sort[i];
			hwPolyBuffer->BState.xiFlag0 =
				0x20000000 | (hwPolyBuffer->BState.xiFlag0 & ~0xA0000000);
			DrawBuffered(*hwPolyBuffer);
		}
	}

	this->ApplyCustomRenderState(0);
	this->FlushRenderPrimitive();
}

void WDirect3D::FlushEqual()
{
	int i;
	for (i = 0; i < this->m_bufPolySwNum; i++)
	{
		w_poly_buffer_sw& swPolyBuf = this->m_bufPolySw[i];
		if ((swPolyBuf.type & 0x300000) == 0x100000)
		{
			this->DrawBuffered(swPolyBuf);
		}
	}
	for (i = 0; i < this->m_bufPolyHwNum; i++)
	{
		w_poly_buffer_hw& hwPolyBuf = this->m_bufPolyHw[i];
		if ((hwPolyBuf.BState.xiFlag0 & 0x300000) == 0x100000)
		{
			this->DrawBuffered(hwPolyBuf);
		}
	}
	this->ApplyCustomRenderState(0);
}

void WDirect3D::Buffering(int type, int type2, int num,
	unsigned long dwVertexTypeDesc, void* p, D3DPRIMITIVETYPE dptPrimitiveType,
	float depth, WxShaderMtl* pShaderMtl)
{
	w_poly_buffer_sw* swPolyBuf = &this->m_bufPolySw[this->m_bufPolySwNum++];
	type2 &= ~0x800;
	swPolyBuf->type2 = type2;
	swPolyBuf->num = num;
	swPolyBuf->bHw = false;
	swPolyBuf->type = type;
	swPolyBuf->dwVertexTypeDesc = dwVertexTypeDesc;
	swPolyBuf->lpvVertices = p;
	swPolyBuf->dptPrimitiveType = dptPrimitiveType;
	swPolyBuf->customRenderState = this->SnapShotCustomRenderState();
	swPolyBuf->bOpaqueDrawn = false;
	swPolyBuf->pShaderMtl = pShaderMtl;

	if ((type & 0x300000) != 0x100000)
	{
		swPolyBuf->depth = depth;
		this->m_buf4sort[this->m_bufSortNum++] = swPolyBuf;
	}

	int bufCapacity = this->m_bufPolySw.size();
	if (this->m_bufPolySwNum == bufCapacity)
	{
		this->Flush(1, true);
	}
}

void WDirect3D::Buffering(const WxViewState& viewState,
	const WxBatchState& batchState)
{
	w_poly_buffer_hw& hwPolyBuf = this->m_bufPolyHw[this->m_bufPolyHwNum++];
	hwPolyBuf.bHw = true;
	memcpy(&hwPolyBuf.VState, &viewState, sizeof hwPolyBuf.VState);
	memcpy(&hwPolyBuf.BState, &batchState, sizeof hwPolyBuf.BState);
	hwPolyBuf.customRenderState = this->SnapShotCustomRenderState();
	hwPolyBuf.bOpaqueDrawn = false;

	if ((batchState.xiFlag0 & 0x300000) != 0x100000)
	{
		hwPolyBuf.depth = batchState.xfDepth;
		this->m_buf4sort[this->m_bufSortNum++] = &hwPolyBuf;
	}

	if (this->m_bufPolyHwNum == static_cast<int>(this->m_bufPolyHw.size()))
	{
		this->Flush(1, true);
	}
}

int WDirect3D::GetTextureNum(int stage)
{
	if (stage)
	{
		for (int i = 0; this->m_texList[this->m_texCount2] && i < 127; ++i)
		{
			this->m_texCount2 =
				this->m_texCount2 >= 127 ? 1 : this->m_texCount2 + 1;
		}
		this->m_texList[this->m_texCount2] = TS_CREATED;
		return this->m_texCount2;
	}
	for (int i = 0; this->m_texList[this->m_texCount1] && i < 1920; ++i)
	{
		this->m_texCount1 =
			this->m_texCount1 >= 2047 ? 128 : this->m_texCount1 + 1;
	}
	this->m_texList[this->m_texCount1] = TS_CREATED;
	return this->m_texCount1;
}

int WDirect3D::UploadCompressedTexture(void* pSrc, size_t srcSize, int type)
{
	int result = this->UploadCompressedTextureSurface(pSrc, srcSize, type);
	this->m_texList[result] = TS_UPDATED;
	return result;
}

// 983: terceiro argumento novo, repassado ao CreateTextureSurface (1 = textura de render em D3DFMT_A16B16G16R16F)
int WDirect3D::CreateTexture(LPBITMAPINFO src, int type, int bFloat16Fmt983)
{
	int texNum = GetTextureNum((static_cast<unsigned int>(type) >> 29) & 1);
	if (!(type & 0x1000))
	{
		CreateTextureSurface(src->bmiHeader.biWidth, src->bmiHeader.biHeight,
			((type & 0x40000000) | 0x20000000U) >> 29, type & 0x1e000, texNum,
			src->bmiHeader.biBitCount >= 32
				? 2
				: (static_cast<unsigned int>(type) >> 11) & 1,
			m_useHiQualityTex || (type & 0x80000000) ? 24 : 16, bFloat16Fmt983);
		m_texList[texNum] = TS_UPDATED;
	}
	return texNum;
}

void WDirect3D::UpdateTexture(int texHandle, LPBITMAPINFO src, void* data,
	unsigned long type)
{
	if (this->m_texList[texHandle] == 1)
	{
		this->CreateTextureSurface(src->bmiHeader.biWidth,
			src->bmiHeader.biHeight, 1, 0, texHandle, 0, 16, 0);
		this->m_texList[texHandle] = TS_UPDATED;
	}
	this->UpdateTextureSurface(texHandle, src, data, type);
}

void WDirect3D::DestroyTexture(int texHandle)
{
	this->m_texList[texHandle] = TS_VOID;
}

int WDirect3D::xCreateVertexBuffer(int numVertices, unsigned long dwFvf,
	unsigned long dwUsage)
{
	int hVb = this->xGetVbHandle();
	this->xCreateVertexBuffer(hVb, numVertices, dwFvf, dwUsage);
	return hVb;
}

int WDirect3D::xCreateIndexBuffer(int numIndices, unsigned long dwUsage)
{
	int hIb = this->xGetIbHandle();
	this->xCreateIndexBuffer(hIb, numIndices, dwUsage);
	return hIb;
}

int WDirect3D::xGetVbHandle()
{
	for (unsigned int i = 1; this->m_xahVbs[this->m_xiVbCount] && i < 0x3ff;
		++i)
	{
		this->m_xiVbCount =
			static_cast<unsigned int>(this->m_xiVbCount) >= 0x400
			? 1
			: this->m_xiVbCount + 1;
	}
	this->m_xahVbs[this->m_xiVbCount] = 1;
	return this->m_xiVbCount;
}

int WDirect3D::xGetIbHandle()
{
	for (int i = 1; this->m_xahIbs[this->m_xiIbCount] && i < 0x1ff; ++i)
	{
		this->m_xiIbCount =
			this->m_xiIbCount >= 0x200 ? 1 : this->m_xiIbCount + 1;
	}
	this->m_xahIbs[this->m_xiIbCount] = 1;
	return this->m_xiIbCount;
}

void WDirect3D::xReleaseVertexBuffer(int hVb)
{
	this->m_xahVbs[hVb] = 0;
}

void WDirect3D::xReleaseIndexBuffer(int hIb)
{
	this->m_xahIbs[hIb] = 0;
}

void WDirect3D::xDrawIndexedTriangles(const WxViewState& viewState,
	const WxBatchState& batchState)
{
	if (this->m_devState == W_VDEVSTATE_LOST)
	{
		return;
	}
	if (batchState.xiFlag0 & 0x400000)
	{
		this->Buffering(viewState, batchState);
	}
	else
	{
		this->xDrawIndexedPrimitive(viewState, batchState);
	}
}
