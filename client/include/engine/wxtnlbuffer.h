#pragma once
#include "wtypes.h"
#include <map>

class WBone;
class WPuppet;
class WVideoDev;
class WView;
struct WxBatch;
struct WxBatchState;
struct w_mesh;
struct WBonePalette983;

struct WxVb
{
	int xhVb;
	int xnVtxs;
	ulong xdwFVF;
	ulong xdwUsage;
	WxVb* xpNext;

	WxVb(ulong dwFVF, ulong dwUsage);
	~WxVb();
	void xReset(ulong dwFVF, ulong dwUsage);
	void xClear(WVideoDev* pVideo);
};

struct WxIb
{
	int xhIb;
	int xnIdxs;
	ulong xdwUsage;

	WxIb();
};

struct WxBatch
{
	WBone* xpBone;
	w_mesh* xpMesh;
	int xiBaseVtxIdx;
	int xnVtxs;
	int xiBaseIdxIdx;
	int xnIdxs;
	unsigned long xdwFVF;
	WxBatch* xpNext;

	WxBatch(WBone* pBone, w_mesh* pMesh, unsigned long dwFVF, int iBaseVtxIdx,
		int nVtxs, int iBaseIdxIdx, int nIdxs);
	void xClear();
	void xCopy(WxBatch** ppOut);
	void xProcessShader(WView* pView);
	// 983 (00bd27e0): mesmo material de shader, alpha, drawFlag, xiDrawFlag2 e luz do osso (e a mesma paleta de ossos
	// se bPalette): os dois lotes podem ir num desenho so. Nome meu.
	bool xIsCompatible983(const WxBatch* pOther, bool bPalette) const;
};

struct WxBatchGrp
{
	WxVb* xpVb;
	WxIb* xpIb;
	int xiFlag0;
	int xiFlag1;
	int xiBaseVtxIdx;
	int xnVtxs;
	int xiBaseIdxIdx;
	int xnIdxs;
	WxBatch* xpaBatches;
	WxBatchGrp* xpNext;

	WxBatchGrp(WxVb* pVb, WxIb* pIb, int iFlag0, int iFlag1, int iBaseVtxIdx,
		int nVtxs, int iBaseIdxIdx, int nIdxs);
	void xClear();
	void xCopy(WxBatchGrp** ppOut);
	void xRenderSubset(WView* pView, WxBatchState* pState, WxBatch* pBatch,
		int nBatches);
	// 983: cada grupo desenha com um WxBatchState proprio. xRenderByBatch (00bd45b0) = buffer do boneco: malha sem ossos
	// com a matriz do osso, malha com ossos (xiDrawFlag2 & 0x800) com as matrizes da paleta dela, juntando lotes
	// compativeis de mesma paleta; xRenderAtOnce (00bd47a0) = buffer estatico (vertices ja transformados), juntando
	// lotes compativeis. Os do 645 (estado de fora, ordem, lista de matrizes do boneco por nome) sairam.
	void xRenderByBatch(WView* pView);
	void xRenderAtOnce(WView* pView);
};

void __fastcall xRender(WView* pView, WxBatchState* pState, WBone* pBone,
	w_mesh* pMesh);

class WxTnLBuffer
{
public:
	// 983 (00bd5b00): mapa local do xCreateBuffers, grupo -> paleta de ossos das malhas dele (no 00bd57d0 um grupo do boneco
	// so recebe malhas da mesma paleta). Nome meu.
	typedef std::map<WxBatchGrp*, WBonePalette983*> tPaletteByGrp983;

	WxTnLBuffer();
	~WxTnLBuffer();

	void xCopy(WxTnLBuffer** ppOut, WVideoDev* pVideo);
	WxBatchGrp* xGetBatchGrps();
	void xUpdateStaticBufferInfo(WBone* pSrc, WBone* pDst, int iIdx);
	void xUpdateStaticBufferInfo(WPuppet** ppPets, int nPets);
	void xSetRootBone(WPuppet* pPet);
	void xClear();
	int xCreateBuffers(WVideoDev* pVideo, WPuppet* pPet);
	int xCreateStaticBuffers(WVideoDev* pVideo, WPuppet** ppPets, int nPets);
	void xRender(WView* pView);
	int xFillBuffers();

private:
	void xUpdateBones(WBone* pBone);
	void xGetMatchingBatches(WxBatchGrp** ppGrp, WxBatch** ppBatch, WxVb* pVb,
		WxIb* pIb, WBone* pBone, w_mesh* pMesh, int nPets,
		tPaletteByGrp983* pPaletteByGrp);	// 983 (00bd57d0): mapa = 0 no buffer estatico
	void xSortBatches();
	void xCount(bool bVtxs, WxBatchGrp* pGrp, WxBatch* pBatch, WBone* pBone);
	void xFillIndexBufferByBone(unsigned char* pBuf, WxBatchGrp* pGrp,
		WxBatch* pBatch, WBone* pBone);
	void xGetMatchingBuffers(WxVb** ppVb, WxIb** ppIb, w_mesh* pMesh);
	int xAllocBuffers(int nCopies);
	void xFillIndexBuffer(const WxIb& ib);
	int xDetermineProperty(WBone* pBone, int nCopies, int iLightMode,
		tPaletteByGrp983* pPaletteByGrp);	// 983 (00bd59c0)
	void xFillVertexBufferByMesh(unsigned char* pBuf, WxBatchGrp* pGrp,
		WxBatch* pBatch, w_mesh* pMesh);
	void xFillVertexBufferByBone(unsigned char* pBuf, WxBatchGrp* pGrp,
		WxBatch* pBatch, WBone* pBone);
	void xFillVertexBuffer(const WxVb& vb);
	void xUpdateMaxBoneNum983();	// 00bd2b10 (nome meu)

	int m_xiFlag;
	WxVb* m_xpaVbs;
	WxIb m_xaIbs[32];
	WxBatchGrp* m_xpaBatchGrps;
	WPuppet* m_xpPet;
	unsigned int m_xuMaxBoneNum983;	// +0x190 (983; nome meu): maior w_mesh::maxBoneNum do boneco (00bd2b10)
	WVideoDev* m_xpVideo;			// +0x194 (983; era +0x190)
};
