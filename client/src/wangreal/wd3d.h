#pragma once
#include <vector>
#include <wdevice.h>
#include "wvideo.h"
#include "wvideo.inl"
#include <d3d9.h>
#include <string>

class WDirect3D : public WVideoDev
{
public:
	WDirect3D(const WDirect3D&);
	WDirect3D();
	virtual ~WDirect3D();
	virtual wVDevState GetDeviceState() const { return this->m_devState; }
	virtual int GetBackBufferBpp() const { return this->m_BackBufBpp; }
	virtual void DrawPolygonFan(WTVertex**, int, int, int, unsigned long, int,
		WxShaderMtl*);
	virtual void DrawIndexedTriangles(WTVertex*, int, unsigned short*, int, int,
		int, int, WxShaderMtl*);
	virtual int CreateTexture(tagBITMAPINFO*, int, int = 0); // padrão provisório: chamadas do wd3d8.cpp do 645 (F2.5)
	virtual void UpdateTexture(int, tagBITMAPINFO*, void*, unsigned long);
	virtual int UploadCompressedTexture(void*, unsigned int, int);
	virtual bool SetFogEnable(bool);
	virtual void SetFogState(float, float, unsigned long);
	virtual bool Reset(bool, int, int, int, int, int, long, int) = 0;
	virtual int GetBufferingMeshNum() const { return this->m_bufPolySwNum; }

protected:
	void Buffering(const WxViewState&, const WxBatchState&);
	void Buffering(int, int, int, unsigned long, void*, _D3DPRIMITIVETYPE,
		float, WxShaderMtl*);
	void Release();
	void Init();
	int GetTextureNum(int);
	// 983: + material de shader no fim (0 = sem material)
	virtual void DrawPrimitive(int, int, unsigned long, void*,
		_D3DPRIMITIVETYPE, int, WxShaderMtl* = 0) = 0;
	virtual void DrawPrimitiveIndexed(int, unsigned long, void*, int,
		unsigned short*, int, int, WxShaderMtl* = 0) = 0;
	virtual bool SetRenderState4Flushing(int) = 0;
	// 983: Flush e FlushMultiPass ganharam um bool (desenhar todos os opacos, ou só os de depth < 0.2);
	// FlushAlways não existe mais
	virtual void Flush(unsigned long, bool = true);
	virtual void FlushEqual();
	virtual void FlushMultiPass(unsigned long, bool);
	virtual void FlushOnePass(unsigned long);
	virtual int UploadCompressedTextureSurface(void*, unsigned int, int) = 0;
	virtual void CreateTextureSurface(unsigned long, unsigned long, int, int,
		int, int, int, int = 0) = 0;
	virtual void UpdateTextureSurface(int, tagBITMAPINFO*, void*, int) = 0;
	virtual void DestroyTexture(int);
	virtual bool SupportRenderTargetFormat() const = 0;
	virtual bool IsSupportedDisplayMode(bool, int, int, int) = 0;
	virtual void SetViewPort(unsigned long, unsigned long, unsigned long,
		unsigned long) = 0;
	void* ModifyVertices(const WTVertexList&, unsigned int, unsigned long, bool);
	virtual unsigned long VertexSize(unsigned long);

	void ApplyGamma(float);
	unsigned int GetSortBufferSize() const { return this->m_buf4sort.size(); }
	unsigned int GetSortBufferSwSize() const
	{
		return this->m_bufPolySw.size();
	}
	unsigned int GetSortBufferHwSize() const
	{
		return this->m_bufPolyHw.size();
	}
	int m_iRefTable[256];
	int m_MaxTextureBlendStages;
	int m_BackBufBpp;

private:
	char* m_chVertex;
	unsigned long m_dwOffset;
	void ClearVertices() { m_dwOffset = 0; }
	virtual void FlushRenderPrimitive() { }
	unsigned char CalcFog(float);
public:
	struct w_poly_buffer
	{
		bool bHw;
		float depth;
		bool bOpaqueDrawn; // 983: campo novo (sizeof 8 -> 12): já desenhado no passe opaco do FlushMultiPass
	};
	struct w_poly_buffer_sw : public w_poly_buffer
	{
		int type;
		int type2;
		int num;
		unsigned long dwVertexTypeDesc;
		void* lpvVertices;
		_D3DPRIMITIVETYPE dptPrimitiveType;
		void* customRenderState;
		WxShaderMtl* pShaderMtl; // 983: campo novo em +0x28 (sizeof 36 -> 44)
	};
	struct w_poly_buffer_hw : public w_poly_buffer
	{
		WxViewState VState;
		WxBatchState BState;
		void* customRenderState;
	};

private:
	std::vector<w_poly_buffer*> m_buf4sort;
	std::vector<w_poly_buffer_sw> m_bufPolySw;
	std::vector<w_poly_buffer_hw> m_bufPolyHw;
	void DrawBuffered(const w_poly_buffer_hw&);
	void DrawBuffered(const w_poly_buffer_sw&);
	virtual void* SnapShotCustomRenderState() = 0;
	virtual void ClearCustomRenderStateSnapShotList() = 0;
	virtual void ApplyCustomRenderState(const void*) = 0;
	static int __cdecl ComparePolyDepth(const void*, const void*);
	enum enumTexState
	{
		TS_VOID = 0,
		TS_CREATED = 1,
		TS_UPDATED = 2,
	};
	int m_texCount1;
	int m_texCount2;
	enumTexState m_texList[2048];
	float CalcDepth(WTVertex**, int);

protected:
	int m_bufPolySwNum;
	int m_bufPolyHwNum;
	int m_bufSortNum;
	wVDevState m_devState;
	bool m_fog;
	float m_fogStart;
	float m_fogEnd;
	unsigned long m_fogColor;
	bool m_useHiQualityTex;
	int m_rs4FlushingEx; // 983: campo novo em +0x2480 (escrito por SetRenderState4Flushing)

public:
	virtual void xCreateVertexBuffer(int, int, unsigned long,
		unsigned long) = 0;
	virtual int xCreateVertexBuffer(int, unsigned long, unsigned long);
	virtual void xCreateIndexBuffer(int, int, unsigned long) = 0;
	virtual int xCreateIndexBuffer(int, unsigned long);
	virtual void xReleaseVertexBuffer(int);
	virtual void xReleaseIndexBuffer(int);
	virtual void xDrawIndexedTriangles(const WxViewState&, const WxBatchState&);

private:
	virtual void xDrawIndexedPrimitive(const WxViewState&,
		const WxBatchState&) = 0;
	int xGetVbHandle();
	int xGetIbHandle();
	int m_xiVbCount;
	int m_xiIbCount;
	int m_xahVbs[1024];
	int m_xahIbs[512]; // 983: 512 (645: 256)
};

// 983: calculado pelos bits do FVF (no 645 era uma tabela de casos); 0x10015c70
inline unsigned long WDirect3D::VertexSize(unsigned long dwVertexTypeDesc)
{
	unsigned long size = 0;
	switch (dwVertexTypeDesc & D3DFVF_POSITION_MASK)
	{
	case D3DFVF_XYZRHW:
		size = 16;
		break;
	case D3DFVF_XYZB1:
		size = 16;
		break;
	case D3DFVF_XYZ:
		size = 12;
		break;
	case D3DFVF_XYZB2:
		size = 20;
		break;
	case D3DFVF_XYZB3:
		size = 24;
		break;
	case D3DFVF_XYZB4:
		size = 28;
		break;
	}
	if (dwVertexTypeDesc & D3DFVF_NORMAL)
		size += 12;
	if (dwVertexTypeDesc & D3DFVF_DIFFUSE)
		size += 4;
	if (dwVertexTypeDesc & D3DFVF_SPECULAR)
		size += 4;
	size += ((dwVertexTypeDesc & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT) * 8;
	return size;
}
