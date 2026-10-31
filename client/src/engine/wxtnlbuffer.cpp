#include "wmemblock.inl"
#include "wxtnlbuffer.h"
#include "wmath.h"
#include "wview.h"
#include "wpuppet.h"
#include "wbone.h"
#include "wmesh.h"
#include "wscene.h"
#include <float.h>
#include <new>

WxVb::WxVb(unsigned long dwFVF, unsigned long dwUsage)
{
	xReset(dwFVF, dwUsage);
}

WxVb::~WxVb()
{
	xReset(0, 0);
}

void WxVb::xReset(unsigned long dwFVF, unsigned long dwUsage)
{
	xhVb = 0;
	xnVtxs = 0;
	xdwFVF = dwFVF;
	xdwUsage = dwUsage;
	xpNext = 0;
}

void WxVb::xClear(WVideoDev* pVideo)
{
	if (xpNext)
	{
		xpNext->xClear(pVideo);
		delete xpNext;
		xpNext = 0;
	}
	pVideo->xReleaseVertexBuffer(xhVb);
}

WxIb::WxIb()
{
	xhIb = 0;
	xnIdxs = 0;
	xdwUsage = 0;
}

WxBatch::WxBatch(WBone* pBone, w_mesh* pMesh, unsigned long dwFVF,
	int iBaseVtxIdx, int nVtxs, int iBaseIdxIdx, int nIdxs)
	: xpBone(pBone),
	  xpMesh(pMesh),
	  xiBaseVtxIdx(iBaseVtxIdx),
	  xnVtxs(nVtxs),
	  xiBaseIdxIdx(iBaseIdxIdx),
	  xnIdxs(nIdxs),
	  xdwFVF(dwFVF),
	  xpNext(0)
{
}

void WxBatch::xClear()
{
	if (xpNext)
	{
		xpNext->xClear();
		delete xpNext;
		xpNext = 0;
	}
}

void WxBatch::xCopy(WxBatch** ppOut)
{
	*ppOut = new WxBatch(xpBone, xpMesh, xdwFVF, xiBaseVtxIdx, xnVtxs,
		xiBaseIdxIdx, xnIdxs);
}

void WxBatch::xProcessShader(WView* pView)
{
	if (!xpMesh->bSpec)
		return;

	WVideoDev* pVideo = pView->GetVideoDevice();
	if (!pVideo)
		return;

	int i;
	int iOffset = pVideo->xGetVertexElemOffset(xdwFVF, WX_VELEM_TEX2);
	if (iOffset < 0)
		return;

	BYTE nStride =
		pVideo->xGetStride((int)((unsigned int)xpMesh->xiDrawFlag2 >> 22));
	unsigned char* pData = pVideo->xLockVertexBuffer(
		(int)((unsigned int)xpMesh->xiDrawFlag2 >> 22), xiBaseVtxIdx * nStride,
		xnVtxs * nStride);

	WBone* pBone = xpBone;
	WMatrix mInv = ~pBone->m_matrix;
	if (pBone->m_light.type == 1)
	{
	}
	else if (pBone->m_light.type == 2)
	{
		WVector vLocalE;
		WVector vLocalL;
		WVector vXA;
		WVector vYA;
		WVector vZA;
		WVector* normList =
			xpMesh->myNormalList ? xpMesh->myNormalList : xpMesh->normList;

		vLocalE = pView->GetCamera().pivot * mInv;
		vLocalL = RotVec(pBone->m_light.nearOne, mInv).Normalize();

		for (i = 0; i < xnVtxs; i++, pData += nStride)
		{
			if (vLocalL * normList[i] >= 0.0f)
			{
				*(float*)(pData + iOffset) = *(float*)(pData + iOffset + 4) =
					0.75f;
			}
			else
			{
				vZA = ((xpMesh->vecList[i] - vLocalE).Normalize() + vLocalL)
						  .Normalize();

				if (vZA * normList[i] >= 0.0f)
				{
					*(float*)(pData + iOffset) =
						*(float*)(pData + iOffset + 4) = 0.75f;
				}
				else
				{
					if (WisEqual(vZA.y, -1.0f, g_EPSILON) ||
						WisEqual(vZA.y, 1.0f, g_EPSILON))
					{
						vXA =
							WCrossProduct(WVector::UNIT_POS_X, vZA).Normalize();
						vYA = WCrossProduct(vZA, vXA).Normalize();
					}
					else
					{
						vXA =
							WCrossProduct(WVector::UNIT_POS_Y, vZA).Normalize();
						vYA = WCrossProduct(vZA, vXA).Normalize();
					}

					*(float*)(pData + iOffset) = Wabs(vXA * normList[i]);
					*(float*)(pData + iOffset + 4) = Wabs(vYA * normList[i]);
				}
			}
		}
	}

	pVideo->xUnlockVertexBuffer((int)((unsigned int)xpMesh->xiDrawFlag2 >> 22));
}

WxBatchGrp::WxBatchGrp(WxVb* pVb, WxIb* pIb, int iFlag0, int iFlag1,
	int iBaseVtxIdx, int nVtxs, int iBaseIdxIdx, int nIdxs)
	: xpVb(pVb),
	  xpIb(pIb),
	  xiFlag0(iFlag0),
	  xiFlag1(iFlag1),
	  xiBaseVtxIdx(iBaseVtxIdx),
	  xnVtxs(nVtxs),
	  xiBaseIdxIdx(iBaseIdxIdx),
	  xnIdxs(nIdxs),
	  xpaBatches(0),
	  xpNext(0)
{
}

void WxBatchGrp::xClear()
{
	if (xpNext)
	{
		xpNext->xClear();
		delete xpNext;
		xpNext = 0;
	}
	if (xpaBatches)
	{
		xpaBatches->xClear();
		delete xpaBatches;
		xpaBatches = 0;
	}
}

void WxBatchGrp::xCopy(WxBatchGrp** ppOut)
{
	*ppOut = new WxBatchGrp(xpVb, xpIb, xiFlag0, xiFlag1, xiBaseVtxIdx, xnVtxs,
		xiBaseIdxIdx, xnIdxs);
}

// 983 (00bd27e0)
bool WxBatch::xIsCompatible983(const WxBatch* pOther, bool bPalette) const
{
	w_mesh* pOtherMesh = pOther->xpMesh;
	w_mesh* pMesh = xpMesh;
	if (pMesh->xpShaderMtl983 == pOtherMesh->xpShaderMtl983 &&
		xpBone->m_alpha == pOther->xpBone->m_alpha &&
		pMesh->drawFlag == pOtherMesh->drawFlag &&
		pMesh->xiDrawFlag2 == pOtherMesh->xiDrawFlag2 &&
		xpBone->m_light == pOther->xpBone->m_light)
	{
		if (bPalette)
			return pMesh->palette983 == pOtherMesh->palette983;
		return true;
	}
	return false;
}

// 983 (00bd45b0): buffer do boneco
void WxBatchGrp::xRenderByBatch(WView* pView)
{
	WxBatchState st;
	WxBatch* pStart = 0;
	int nRun = 0;
	for (WxBatch* pBatch = xpaBatches; pBatch; pBatch = pBatch->xpNext)
	{
		if (!pBatch->xpBone || !pBatch->xpMesh ||
			!WBone::IsMeshVisible(pBatch->xpBone, pBatch->xpMesh) ||
			(pView->IsShadowView() &&
				(pBatch->xpMesh->drawFlag & 0x300000) == 0x100000))
		{
			if (pStart)
			{
				xRenderSubset(pView, &st, pStart, nRun);
				pStart = 0;
				nRun = 0;
			}
			continue;
		}
		if ((pBatch->xpMesh->drawFlag & 0x400000) == 0 &&
			(pBatch->xpMesh->xiDrawFlag2 & 0x800))
		{
			if (pStart)
			{
				if (pBatch->xIsCompatible983(pStart, true))
				{
					++nRun;
					continue;
				}
				xRenderSubset(pView, &st, pStart, nRun);
				pStart = 0;
				nRun = 0;
			}
		}
		else if (pStart)
		{
			xRenderSubset(pView, &st, pStart, nRun);
			pStart = 0;
			nRun = 0;
		}
		if (pBatch->xpMesh->xiDrawFlag2 & 0x800)
		{
			st.xnmTransfs = (int)pBatch->xpMesh->palette983->bones.size();
			st.xpapmW = &pBatch->xpMesh->palette983->transf[0];
			st.xpapmO = &pBatch->xpMesh->palette983->orig[0];
		}
		else
		{
			st.xnmTransfs = 0;
			st.xmW = pBatch->xpBone->m_matrix;
		}
		if ((pBatch->xpMesh->drawFlag & 0x400000) == 0 &&
			(pBatch->xpMesh->xiDrawFlag2 & 0x800))
		{
			pStart = pBatch;
			nRun = 1;
		}
		else
			xRenderSubset(pView, &st, pBatch, 1);
	}
	if (pStart)
		xRenderSubset(pView, &st, pStart, nRun);
}

// 983 (00bd47a0): buffer estatico
void WxBatchGrp::xRenderAtOnce(WView* pView)
{
	WxBatchState st;
	WxBatch* pStart = 0;
	int nRun = 0;
	for (WxBatch* pBatch = xpaBatches; pBatch; pBatch = pBatch->xpNext)
	{
		if (!pBatch->xpBone || !pBatch->xpMesh ||
			!WBone::IsMeshVisible(pBatch->xpBone, pBatch->xpMesh) ||
			(pView->IsShadowView() &&
				(pBatch->xpMesh->drawFlag & 0x300000) == 0x100000))
		{
			if (pStart)
			{
				xRenderSubset(pView, &st, pStart, nRun);
				pStart = 0;
				nRun = 0;
			}
		}
		else if ((pBatch->xpMesh->drawFlag & 0x400000) == 0 && pStart)
		{
			if (pBatch->xIsCompatible983(pStart, false))
				++nRun;
			else
			{
				xRenderSubset(pView, &st, pStart, nRun);
				pStart = pBatch;
				nRun = 1;
			}
		}
		else
		{
			if (pStart)
				xRenderSubset(pView, &st, pStart, nRun);
			pStart = pBatch;
			nRun = 1;
		}
	}
	if (pStart)
		xRenderSubset(pView, &st, pStart, nRun);
}

void WxBatchGrp::xRenderSubset(WView* pView, WxBatchState* pState,
	WxBatch* pBatch, int nBatches)
{
	int nVtxs = 0;
	int nIdxs = 0;
	WxBatch* p = pBatch;
	for (int k = 0; k < nBatches; ++k)
	{
		nVtxs += p->xnVtxs;
		nIdxs += p->xnIdxs;
		p = p->xpNext;
	}
	if (pView->ProcessEffect())
	{
		p = pBatch;
		for (int k = nBatches; k > 0; --k)
		{
			p->xProcessShader(pView);
			p = p->xpNext;
		}
	}
	pState->xpShaderMtl = pBatch->xpMesh->xpShaderMtl983;
	pState->xiFlag0 = pBatch->xpMesh->drawFlag;
	pState->xiFlag1 = pBatch->xpMesh->xiDrawFlag2;
	pState->xiBaseVtxIdx = pBatch->xiBaseVtxIdx;
	pState->xiBaseIdxIdx = pBatch->xiBaseIdxIdx;
	pState->xnVtxs = nVtxs;
	pState->xnIdxs = nIdxs;
	::xRender(pView, pState, pBatch->xpBone, pBatch->xpMesh);
}

WxTnLBuffer::WxTnLBuffer()
	: m_xiFlag(0), m_xpaVbs(0)
{
	m_xpaBatchGrps = 0;
	m_xpPet = 0;
	m_xuMaxBoneNum983 = 0;
	m_xpVideo = 0;
	memset(m_xaIbs, 0, sizeof(m_xaIbs));
}

WxTnLBuffer::~WxTnLBuffer()
{
	xClear();
}

void WxTnLBuffer::xClear()
{
	if ((m_xiFlag & 1) == 0 && m_xpaVbs)
	{
		m_xpaVbs->xClear(m_xpVideo);
		if (m_xpaVbs)
		{
			delete m_xpaVbs;
			m_xpaVbs = 0;
		}
	}
	WxIb* pIb = m_xaIbs;
	int n = 32;
	do
	{
		if (pIb->xnIdxs != 0 && (m_xiFlag & 1) == 0)
			m_xpVideo->xReleaseIndexBuffer(pIb->xhIb);
		pIb->xnIdxs = 0;
		pIb->xhIb = 0;
		pIb->xdwUsage = 0;
		++pIb;
		--n;
	} while (n);
	if (m_xpaBatchGrps)
	{
		m_xpaBatchGrps->xClear();
		if (m_xpaBatchGrps)
		{
			delete m_xpaBatchGrps;
			m_xpaBatchGrps = 0;
		}
	}
	m_xuMaxBoneNum983 = 0;
}

void WxTnLBuffer::xCopy(WxTnLBuffer** ppOut, WVideoDev* pVideo)
{
	*ppOut = new WxTnLBuffer;
	(*ppOut)->m_xiFlag |= m_xiFlag | 1;
	(*ppOut)->m_xpVideo = pVideo;
	(*ppOut)->m_xpaVbs = m_xpaVbs;
	for (unsigned int i = 0; i < 32; ++i)
		(*ppOut)->m_xaIbs[i] = m_xaIbs[i];
	WxBatchGrp** ppGrp = &(*ppOut)->m_xpaBatchGrps;
	for (WxBatchGrp* pGrp = m_xpaBatchGrps; pGrp; pGrp = pGrp->xpNext)
	{
		pGrp->xCopy(ppGrp);
		WxBatch** ppBatch = &(*ppGrp)->xpaBatches;
		for (WxBatch* pBatch = pGrp->xpaBatches; pBatch;
			pBatch = pBatch->xpNext)
		{
			pBatch->xCopy(ppBatch);
			ppBatch = &(*ppBatch)->xpNext;
		}
		ppGrp = &(*ppGrp)->xpNext;
	}
}

void WxTnLBuffer::xSetRootBone(WPuppet* pPet)
{
	m_xpPet = pPet;
	xUpdateBones(pPet->m_rootbone);
}

void WxTnLBuffer::xUpdateBones(WBone* pBone)
{
	do
	{
		for (WxBatchGrp* pGrp = m_xpaBatchGrps; pGrp; pGrp = pGrp->xpNext)
		{
			for (WxBatch* pBatch = pGrp->xpaBatches; pBatch;
				pBatch = pBatch->xpNext)
			{
				if (pBatch->xpBone->m_id == pBone->m_id)
				{
					w_mesh* pOld = pBatch->xpBone->m_mesh;
					w_mesh* pNew = pBone->m_mesh;
					if (pOld)
					{
						do
						{
							if (pOld == pBatch->xpMesh)
							{
								pBatch->xpMesh = pNew;
								break;
							}
							pOld = pOld->next;
							pNew = pNew->next;
						} while (pOld);
					}
					pBatch->xpBone = pBone;
				}
			}
		}
		if (pBone->m_next)
			xUpdateBones(pBone->m_next);
		pBone = pBone->m_child;
	} while (pBone);
}

WxBatchGrp* WxTnLBuffer::xGetBatchGrps()
{
	return m_xpaBatchGrps;
}

// 983 (00bd2ae0): maior maxBoneNum das malhas do osso
void __fastcall xMaxBoneNumOfBone983(WPuppet* pPet, WBone* pBone, void* pParam)
{
	unsigned int* pMax = (unsigned int*)pParam;
	for (w_mesh* pMesh = pBone->m_mesh; pMesh; pMesh = pMesh->next)
	{
		if ((unsigned int)pMesh->maxBoneNum > *pMax)
			*pMax = pMesh->maxBoneNum;
	}
}

// 983 (00bd2b10)
void WxTnLBuffer::xUpdateMaxBoneNum983()
{
	unsigned int uMax = 0;
	WPuppet::ForEachBone983(m_xpPet, xMaxBoneNumOfBone983, &uMax);
	m_xuMaxBoneNum983 = uMax;
}

// 983 (00bd5b00): o maior numero de ossos do boneco vai para o FVF (00bd2b10) e o mapa local grupo -> paleta separa os
// grupos de malhas de paletas diferentes.
int WxTnLBuffer::xCreateBuffers(WVideoDev* pVideo, WPuppet* pPet)
{
	tPaletteByGrp983 paletteByGrp;
	m_xpPet = pPet;
	m_xpVideo = pVideo;
	xUpdateMaxBoneNum983();
	return xDetermineProperty(pPet->GetRootBone(), 1, pPet->GetLightMode(),
			   &paletteByGrp) |
		xAllocBuffers(1);
}

// 983 (00bd5c00): sem mapa (vertices ja transformados, nao ha paleta no desenho).
int WxTnLBuffer::xCreateStaticBuffers(WVideoDev* pVideo, WPuppet** ppPets,
	int nPets)
{
	m_xiFlag |= 2;
	m_xpPet = ppPets[0];
	m_xpVideo = pVideo;
	return xDetermineProperty(m_xpPet->GetRootBone(), nPets,
			   ppPets[0]->GetLightMode(), 0) |
		xAllocBuffers(nPets);
}

// 983 (00bd59c0): o bit de ossos (0x800) vale para todas as malhas do boneco se alguma tiver ossos (m_xuMaxBoneNum983),
// e nunca no buffer estatico.
int WxTnLBuffer::xDetermineProperty(WBone* pBone, int nCopies, int iLightMode,
	tPaletteByGrp983* pPaletteByGrp)
{
	w_mesh* pMesh = pBone->m_mesh;
	if (pMesh)
	{
		do
		{
			// 983: bits do xiDrawFlag2 um abaixo do 645 (ossos 0x800, sem sombra 0x1000, como no xDetermineFVF da dll 983)
			pMesh->xiDrawFlag2 = (pMesh->xiDrawFlag2 & 0x614) |
				(~((unsigned int)iLightMode >> 12) & 0x1000);
			if ((m_xiFlag & 2) == 0 && m_xuMaxBoneNum983 != 0)
				pMesh->xiDrawFlag2 |= 0x800;
			if ((pMesh->xiDrawFlag2 & 0x1000) == 0)
				pMesh->xiDrawFlag2 |= 4;
			WxVb* pVb = 0;
			WxIb* pIb = 0;
			WxBatchGrp* pGrp = 0;
			WxBatch* pBatch = 0;
			xGetMatchingBuffers(&pVb, &pIb, pMesh);
			xGetMatchingBatches(&pGrp, &pBatch, pVb, pIb, pBone, pMesh,
				nCopies, pPaletteByGrp);
			pVb->xnVtxs += pMesh->vtxNum * nCopies;
			pIb->xnIdxs += pMesh->indexNum * nCopies;
			pMesh->xpBatchGrp = pGrp;
			pMesh->xpBatch = pBatch;
			pMesh = pMesh->next;
		} while (pMesh);
	}
	if (pBone->m_next)
		xDetermineProperty(pBone->m_next, nCopies, iLightMode, pPaletteByGrp);
	if (pBone->m_child)
		xDetermineProperty(pBone->m_child, nCopies, iLightMode, pPaletteByGrp);
	return 0;
}

// 983 (00bd3630): FVF pelo material de shader da malha (sobrecarga de 4 argumentos) quando ha um registrado, com o maior
// numero de ossos do boneco; VB com teto de 3000 vertices e IB com teto de 15000 indices (645: sem teto / 9000).
void WxTnLBuffer::xGetMatchingBuffers(WxVb** ppVb, WxIb** ppIb, w_mesh* pMesh)
{
	WxVb* pPrevVb = 0;
	WxIb* pIb = 0;
	unsigned long dwFVF;
	if (pMesh->xpShaderMtl983 && pMesh->xpShaderMtl983->m_shaderId)
		dwFVF = m_xpVideo->xDetermineFVF(pMesh->xpShaderMtl983->m_shaderId,
			pMesh->drawFlag, pMesh->xiDrawFlag2, m_xuMaxBoneNum983);
	else
		dwFVF = m_xpVideo->xDetermineFVF(pMesh->drawFlag, pMesh->xiDrawFlag2,
			m_xuMaxBoneNum983);
	unsigned long dwUsage = m_xpVideo->xDetermineBufferUsage(dwFVF) | 8;
	unsigned long dwVbUsage = dwUsage;
	if (pMesh->bSpec)
		dwVbUsage |= 0x200;
	WxVb* pVb = m_xpaVbs;
	while (pVb)
	{
		if (dwFVF == pVb->xdwFVF && dwVbUsage == pVb->xdwUsage &&
			pVb->xnVtxs + pMesh->vtxNum < 3001)
			break;
		pPrevVb = pVb;
		pVb = pVb->xpNext;
	}
	if (!pVb)
	{
		pVb = new WxVb(dwFVF, dwVbUsage);
		if (pPrevVb)
			pPrevVb->xpNext = pVb;
		else
			m_xpaVbs = pVb;
	}

	for (unsigned int i = 0; i < 32; ++i)
	{
		if (m_xaIbs[i].xnIdxs == 0)
		{
			pIb = &m_xaIbs[i];
			pIb->xdwUsage = dwUsage;
			break;
		}
		if (dwUsage == m_xaIbs[i].xdwUsage &&
			m_xaIbs[i].xnIdxs + pMesh->indexNum < 15001)
		{
			pIb = &m_xaIbs[i];
			break;
		}
	}
	*ppVb = pVb;
	*ppIb = pIb;
}

// 983 (00bd57d0): xiFlag1 comparado so nos 13 bits de baixo; com mapa, o grupo tambem tem de ser da paleta da malha.
void WxTnLBuffer::xGetMatchingBatches(WxBatchGrp** ppGrp, WxBatch** ppBatch,
	WxVb* pVb, WxIb* pIb, WBone* pBone, w_mesh* pMesh, int nPets,
	tPaletteByGrp983* pPaletteByGrp)
{
	WxBatch* pBatch = 0;
	WxBatchGrp* pPrev = 0;
	WxBatchGrp* pGrp = m_xpaBatchGrps;
	while (pGrp)
	{
		if (pGrp->xpVb == pVb && pGrp->xpIb == pIb &&
			pGrp->xiFlag0 == pMesh->drawFlag &&
			((pGrp->xiFlag1 ^ pMesh->xiDrawFlag2) & 0x1fff) == 0 &&
			(!pPaletteByGrp || (*pPaletteByGrp)[pGrp] == pMesh->palette983))
			break;
		pPrev = pGrp;
		pGrp = pGrp->xpNext;
	}
	if (!pGrp)
	{
		pGrp = new WxBatchGrp(pVb, pIb, pMesh->drawFlag,
			pMesh->xiDrawFlag2 & 0x1fff, -1, 0, -1, 0);
		if (pPaletteByGrp)
			(*pPaletteByGrp)[pGrp] = pMesh->palette983;
		if (pPrev)
			pPrev->xpNext = pGrp;
		else
			m_xpaBatchGrps = pGrp;
	}
	WxBatch* pTail = pGrp->xpaBatches;
	if (!pTail)
		pTail = 0;
	else
		while (pTail->xpNext)
			pTail = pTail->xpNext;
	for (int i = 0; i < nPets; i++)
	{
		WxBatch* pNextBatch = pBatch;
		pBatch = new WxBatch(pBone, pMesh, pVb->xdwFVF, -1, 0, -1, 0);
		pBatch->xpNext = pNextBatch;
		if (!pTail)
			pGrp->xpaBatches = pBatch;
		else
			pTail->xpNext = pBatch;
	}
	*ppGrp = pGrp;
	*ppBatch = pBatch;
}

void WxTnLBuffer::xSortBatches()
{
}

int WxTnLBuffer::xAllocBuffers(int nCopies)
{
	int iBase = 0;
	for (WxVb* pVb = m_xpaVbs; pVb; pVb = pVb->xpNext)
	{
		pVb->xhVb = m_xpVideo->xCreateVertexBuffer(pVb->xnVtxs, pVb->xdwFVF,
			pVb->xdwUsage);
		for (WxBatchGrp* pGrp = m_xpaBatchGrps; pGrp; pGrp = pGrp->xpNext)
		{
			if (pGrp->xpVb == pVb)
			{
				pGrp->xiFlag1 |= pVb->xhVb << 22;
				WxBatch* pBatch = pGrp->xpaBatches;
				pGrp->xiBaseVtxIdx = iBase;
				for (; pBatch; pBatch = pBatch->xpNext)
				{
					pBatch->xiBaseVtxIdx = iBase;
					xCount(true, pGrp, pBatch, m_xpPet->m_rootbone);
					int nVtxs = pBatch->xnVtxs;
					if (nCopies > 1)
					{
						int iOff = iBase + nVtxs;
						int k = nCopies - 1;
						do
						{
							pBatch = pBatch->xpNext;
							pBatch->xiBaseVtxIdx = iOff;
							iOff += nVtxs;
							--k;
							pBatch->xnVtxs = nVtxs;

						} while (k);
					}
					pGrp->xnVtxs += nVtxs * nCopies;
					iBase += nVtxs * nCopies;
				}
			}
		}
		iBase = 0;
	}
	int iBaseIdx = 0;
	for (int i = 0; i < 32; ++i)
	{
		WxIb* pIb = &m_xaIbs[i];
		if (pIb->xnIdxs > 0)
		{
			pIb->xhIb =
				m_xpVideo->xCreateIndexBuffer(pIb->xnIdxs, pIb->xdwUsage);
			for (WxBatchGrp* pGrp = m_xpaBatchGrps; pGrp; pGrp = pGrp->xpNext)
			{
				if (pGrp->xpIb == pIb)
				{
					pGrp->xiFlag1 |= (pIb->xhIb & 0x1ff) << 13; // 983: como em 00bd2c86
					WxBatch* pBatch = pGrp->xpaBatches;
					pGrp->xiBaseIdxIdx = iBaseIdx;
					for (; pBatch; pBatch = pBatch->xpNext)
					{
						pBatch->xiBaseIdxIdx = iBaseIdx;
						xCount(false, pGrp, pBatch, m_xpPet->m_rootbone);
						int nIdxs = pBatch->xnIdxs;
						if (nCopies > 1)
						{
							int iOff = iBaseIdx + nIdxs;
							int k = nCopies - 1;
							do
							{
								pBatch = pBatch->xpNext;
								pBatch->xiBaseIdxIdx = iOff;
								iOff += nIdxs;
								--k;
								pBatch->xnIdxs = nIdxs;

							} while (k);
						}
						pGrp->xnIdxs += nIdxs * nCopies;
						iBaseIdx += nIdxs * nCopies;
					}
				}
			}
			iBaseIdx = 0;
		}
	}
	return 0;
}

void WxTnLBuffer::xCount(bool bVtxs, WxBatchGrp* pGrp, WxBatch* pBatch,
	WBone* pBone)
{
	{
		for (w_mesh* pMesh = pBone->m_mesh; pMesh; pMesh = pMesh->next)
		{
			if (pMesh->xpBatch == pBatch)
			{
				if (bVtxs)
					pBatch->xnVtxs += pMesh->vtxNum;
				else
					pBatch->xnIdxs += pMesh->indexNum;
				break;
			}
		}
		if (pBone->m_next)
			xCount(bVtxs, pGrp, pBatch, pBone->m_next);
		if (pBone->m_child)
			xCount(bVtxs, pGrp, pBatch, pBone->m_child);
	}
}

int WxTnLBuffer::xFillBuffers()
{
	for (WxVb* pVb = m_xpaVbs; pVb; pVb = pVb->xpNext)
		xFillVertexBuffer(*pVb);
	WxIb* pIb = m_xaIbs;
	int n = 32;
	do
	{
		if (pIb->xnIdxs != 0)
			xFillIndexBuffer(*pIb);
		++pIb;
		--n;
	} while (n);
	return 0;
}

void WxTnLBuffer::xFillVertexBuffer(const WxVb& vb)
{
	unsigned char* pBuf = m_xpVideo->xLockVertexBuffer(vb.xhVb, 0, 0);
	for (WxBatchGrp* pGrp = m_xpaBatchGrps; pGrp; pGrp = pGrp->xpNext)
	{
		if (pGrp->xpVb == &vb)
		{
			int nStride = m_xpVideo->VertexSize(pGrp->xpVb->xdwFVF);
			for (WxBatch* pBatch = pGrp->xpaBatches; pBatch;
				pBatch = pBatch->xpNext)
				xFillVertexBufferByBone(pBuf + pBatch->xiBaseVtxIdx * nStride,
					pGrp, pBatch, pBatch->xpBone);
		}
	}
	m_xpVideo->xUnlockVertexBuffer(vb.xhVb);
}

void WxTnLBuffer::xFillVertexBufferByBone(unsigned char* pBuf, WxBatchGrp* pGrp,
	WxBatch* pBatch, WBone* pBone)
{
	{
		for (w_mesh* pMesh = pBone->m_mesh; pMesh; pMesh = pMesh->next)
		{
			if (pMesh->xpBatch == pBatch)
			{
				pMesh->xiDrawFlag2 |= pGrp->xpVb->xhVb << 22;
				pMesh->xiBaseVtxIdx = pBatch->xiBaseVtxIdx;
				xFillVertexBufferByMesh(pBuf, pGrp, pBatch, pMesh);
				break;
			}
		}
		if (pBone->m_next)
			xFillVertexBufferByBone(pBuf, pGrp, pBatch, pBone->m_next);
		if (pBone->m_child)
			xFillVertexBufferByBone(pBuf, pGrp, pBatch, pBone->m_child);
	}
}

// 983 (00bd3990): indice de osso no vertice = posicao do osso na paleta da malha (WBonePalette983::IndexOf),
// matrizes da paleta (GetMatrix), ate 8 camadas de UV (TEX1..TEX8 do FVF). O m_id e a lista de matrizes
// do boneco por nome (mpetName) do 645 sairam.
void WxTnLBuffer::xFillVertexBufferByMesh(unsigned char* pBuf, WxBatchGrp* pGrp,
	WxBatch* pBatch, w_mesh* pMesh)
{
	unsigned long dwFVF = pGrp->xpVb->xdwFVF;
	WBonePalette983* pPalette = pMesh->palette983;
	int nStride = m_xpVideo->VertexSize(dwFVF);
	int iOff = 0;
	int nBlendSize = m_xpVideo->xGetBlendWeightSize(dwFVF);
	bool bNormal = m_xpVideo->xHasVertexElem(dwFVF, WX_VELEM_NORMAL);
	bool bColor = m_xpVideo->xHasVertexElem(dwFVF, WX_VELEM_DIFFUSE);
	int nLayers = 0;
	for (int k = 0; k < 8; ++k)
	{
		if (!m_xpVideo->xHasVertexElem(dwFVF, (wWxVertexElem)(WX_VELEM_TEX1 + k)))
			break;
		++nLayers;
	}
	const WMatrix* pMat;
	if ((m_xiFlag & 2) == 0)
	{
		if (pMesh->rigidNum < pMesh->vtxNum)
			pMat = pPalette->GetMatrix(pBatch->xpBone);
		else
			pMat = &WMatrix::IDENTITY;
	}
	else
	{
		pMat = &pBatch->xpBone->m_matrix;
	}
	WMatrix mRot;
	if (bNormal)
		pMat->GetRotMatrix(&mRot);
	else
		mRot.Reset();
	WVector* pNorm =
		pMesh->myNormalList ? pMesh->myNormalList : pMesh->normList;
	int i = 0;
	for (; i < pMesh->vtxNum; ++i)
	{
		if (!(pMesh->xiDrawFlag2 & 0x800))
		{
			if (!(m_xiFlag & 2))
				memcpy(pBuf + iOff, &pMesh->vecList[i], sizeof(WVector));
			else
				*(WVector*)(pBuf + iOff) = pMesh->vecList[i] * *pMat;
			iOff += 12;
		}
		else
		{
			if (i < pMesh->rigidNum)
				*(WVector*)(pBuf + iOff) = pMesh->vecList[i] *
					*pPalette->GetMatrix(pMesh->boneList[i]);
			else
				*(WVector*)(pBuf + iOff) = pMesh->vecList[i] * *pMat;
			iOff += 0x10 + nBlendSize;
		}
		if (bNormal)
		{
			if (!(pMesh->xiDrawFlag2 & 0x800))
			{
				if (!(m_xiFlag & 2))
					memcpy(pBuf + iOff, &pNorm[i], sizeof(WVector));
				else
					*(WVector*)(pBuf + iOff) = pNorm[i] * mRot;
			}
			else if (i < pMesh->rigidNum)
			{
				WMatrix mBone;
				pPalette->GetMatrix(pMesh->boneList[i])->GetRotMatrix(&mBone);
				*(WVector*)(pBuf + iOff) = pNorm[i] * mBone;
			}
			else
				*(WVector*)(pBuf + iOff) = pNorm[i] * mRot;
			iOff += 12;
		}
		if (bColor)
		{
			if (pMesh->vtxColorPtrList)
				*(unsigned long*)(pBuf + iOff) = *pMesh->vtxColorPtrList[i];
			else
				*(unsigned long*)(pBuf + iOff) = pMesh->vtxColorList[i];
			iOff += 4;
		}
		for (int k = 0; k < nLayers; ++k)
		{
			*(w_uv983*)(pBuf + iOff) = (*pMesh->uv)[k][i];
			iOff += 8;
		}
	}
	if (pMesh->xiDrawFlag2 & 0x800)
	{
		int iVtx = 0;
		int iDst = 12;
		int iBone = 0;
		float afWeights[3];
		if (pMesh->blendedRigidNum > 0)
		{
			do
			{
				unsigned long uIdx = 0;
				memset(afWeights, 0, sizeof(afWeights));
				float fTotal = 0.0f;
				int j = 0;
				for (; fTotal < 0.999f && j < 3; ++j, ++iBone)
				{
					if (pMesh->blendedBoneList[iBone])
					{
						uIdx |= pPalette->IndexOf(pMesh->blendedBoneList[iBone])
							<< (j * 8);
						afWeights[j] = pMesh->blendWeightList[iBone];
						fTotal += pMesh->blendWeightList[iBone];
					}
				}
				if (j == 3 && fTotal < 0.999f && pMesh->blendedBoneList[iBone])
				{
					uIdx |= pPalette->IndexOf(pMesh->blendedBoneList[iBone]) << 24;
					++iBone;
				}

				memcpy(&pBuf[iDst], afWeights, nBlendSize);
				*(unsigned int*)(&pBuf[iDst] + nBlendSize) = uIdx;
				iDst += nStride;
				++iVtx;
			} while (iVtx < pMesh->blendedRigidNum);
		}
		afWeights[0] = 1.0f;
		afWeights[2] = 0.0f;
		afWeights[1] = 0.0f;
		if ((int)nBlendSize > 0)
		{
			if (iVtx < pMesh->rigidNum)
			{
				do
				{
					unsigned int uIdx = pPalette->IndexOf(pMesh->boneList[iVtx]);
					memcpy(&pBuf[iDst], afWeights, nBlendSize);
					*(unsigned int*)(pBuf + nBlendSize + iDst) = uIdx;
					++iVtx;
					iDst += nStride;
				} while (iVtx < pMesh->rigidNum);
			}
			if (pMesh->rigidNum < pMesh->vtxNum)
			{
				unsigned int uIdx = pPalette->IndexOf(pBatch->xpBone);
				if (iVtx < pMesh->vtxNum)
				{
					do
					{
						memcpy(&pBuf[iDst], afWeights, nBlendSize);
						*(unsigned int*)(pBuf + nBlendSize + iDst) = uIdx;
						++iVtx;
						iDst += nStride;
					} while (iVtx < pMesh->vtxNum);
				}
			}
		}
		else
		{
			if (iVtx < pMesh->rigidNum)
			{
				do
				{
					*(unsigned int*)(pBuf + nBlendSize + iDst) =
						pPalette->IndexOf(pMesh->boneList[iVtx]);
					++iVtx;
					iDst += nStride;
				} while (iVtx < pMesh->rigidNum);
			}
			if (pMesh->rigidNum < pMesh->vtxNum)
			{
				unsigned int uIdx = pPalette->IndexOf(pBatch->xpBone);
				if (iVtx < pMesh->vtxNum)
				{
					do
					{
						*(unsigned int*)(pBuf + iDst + nBlendSize) = uIdx;
						++iVtx;
						iDst += nStride;
					} while (iVtx < pMesh->vtxNum);
				}
			}
		}
	}
}

void WxTnLBuffer::xFillIndexBuffer(const WxIb& ib)
{
	unsigned char* pBuf = m_xpVideo->xLockIndexBuffer(ib.xhIb, 0, 0);
	for (WxBatchGrp* pGrp = m_xpaBatchGrps; pGrp; pGrp = pGrp->xpNext)
	{
		if (pGrp->xpIb == &ib)
		{
			for (WxBatch* pBatch = pGrp->xpaBatches; pBatch;
				pBatch = pBatch->xpNext)
				xFillIndexBufferByBone(pBuf + pBatch->xiBaseIdxIdx * 2, pGrp,
					pBatch, pBatch->xpBone);
		}
	}
	m_xpVideo->xUnlockIndexBuffer(ib.xhIb);
}

void WxTnLBuffer::xFillIndexBufferByBone(unsigned char* pBuf, WxBatchGrp* pGrp,
	WxBatch* pBatch, WBone* pBone)
{
	{
		unsigned char* pDst = pBuf;
		for (w_mesh* pMesh = pBone->m_mesh; pMesh; pMesh = pMesh->next)
		{
			if (pMesh->xpBatch == pBatch)
			{
				pMesh->xiDrawFlag2 |= (pGrp->xpIb->xhIb & 0x1ff) << 13; // 983 00bd2c86
				pMesh->xiBaseIdxIdx = pBatch->xiBaseIdxIdx;
				for (int i = 0; i < pMesh->indexNum; ++i, pDst += 2)
					*(unsigned short*)pDst =
						(unsigned short)(pMesh->indexList[i] +
							pMesh->xiBaseVtxIdx);
				break;
			}
		}
		if (pBone->m_next)
			xFillIndexBufferByBone(pBuf, pGrp, pBatch, pBone->m_next);
		if (pBone->m_child)
			xFillIndexBufferByBone(pBuf, pGrp, pBatch, pBone->m_child);
	}
}

void WxTnLBuffer::xUpdateStaticBufferInfo(WPuppet** ppPets, int nPets)
{
	WBone* pRoot = ppPets[0]->m_rootbone;
	for (int i = 1; i < nPets; ++i)
		xUpdateStaticBufferInfo(pRoot, ppPets[i]->m_rootbone, i);
}

void WxTnLBuffer::xUpdateStaticBufferInfo(WBone* pSrc, WBone* pDst, int iIdx)
{
	{
		WBone* pFound = pDst->FindBone(pSrc->m_name, 0);
		if (pFound)
		{
			w_mesh* pMeshSrc = pSrc->m_mesh;
			for (w_mesh* pMeshDst = pFound->m_mesh; pMeshSrc && pMeshDst;
				pMeshDst = pMeshDst->next)
			{
				WxBatchGrp* pGrp = pMeshSrc->xpBatchGrp;
				WxBatch* pBatch = pMeshSrc->xpBatch;
				for (int i = 0; i < iIdx; ++i)
					pBatch = pBatch->xpNext;
				pMeshDst->xpBatchGrp = pGrp;
				pMeshDst->xiDrawFlag2 = pGrp->xiFlag1;
				pMeshDst->xpBatch = pBatch;
				pMeshDst->xiBaseVtxIdx = pBatch->xiBaseVtxIdx;
				pMeshDst->xiBaseIdxIdx = pBatch->xiBaseIdxIdx;
				pBatch->xpBone = pFound;
				pBatch->xpMesh = pMeshDst;
				pMeshSrc = pMeshSrc->next;
			}
		}
		if (pSrc->m_next)
			xUpdateStaticBufferInfo(pSrc->m_next, pDst, iIdx);
		if (pSrc->m_child)
			xUpdateStaticBufferInfo(pSrc->m_child, pDst, iIdx);
	}
}

// xUpdateMatrixIdx (645) nao existe no 983: o indice de osso vem da paleta da malha (WBonePalette983), ver xFillVertexBufferByMesh

// 983 (00bd4a30): cada grupo com o seu estado (as matrizes dos ossos vem da paleta de cada malha)
void WxTnLBuffer::xRender(WView* pView)
{
	WxBatchGrp* pGrp = m_xpaBatchGrps;
	if (m_xiFlag & 2)
	{
		for (; pGrp; pGrp = pGrp->xpNext)
			pGrp->xRenderAtOnce(pView);
	}
	else
	{
		for (; pGrp; pGrp = pGrp->xpNext)
			pGrp->xRenderByBatch(pView);
	}
}

// 983 (00bd2dc0): as matrizes originais ja vem no estado (paleta); profundidade do lote = a da malha (+0xd4)
// se ela tiver uma, senao a da caixa, normalizada entre clip_near e clip_far
void __fastcall xRender(WView* pView, WxBatchState* pState, WBone* pBone,
	w_mesh* pMesh)
{
	if (pBone && pMesh)
	{
		if (pState->xiFlag1 & 4)
		{
			LightSet* pLight = pView->xGetLight();
			pState->xdwDiffuse =
				((unsigned long)pBone->m_alpha << 24) | pLight->ambient2;
		}
		else
		{
			pState->xdwDiffuse =
				((unsigned long)pBone->m_alpha << 24) | 0xffffff;
		}
		if (pState->xiFlag0 & 0x400000)
		{
			if (pMesh->f_d4_983 != -FLT_MAX)
				pState->xfDepth = pMesh->f_d4_983;
			else
			{
				WVector v[8] = {
					WVector(pMesh->localAabb.min.x, pMesh->localAabb.min.y,
						pMesh->localAabb.min.z),
					WVector(pMesh->localAabb.max.x, pMesh->localAabb.min.y,
						pMesh->localAabb.min.z),
					WVector(pMesh->localAabb.max.x, pMesh->localAabb.min.y,
						pMesh->localAabb.max.z),
					WVector(pMesh->localAabb.min.x, pMesh->localAabb.min.y,
						pMesh->localAabb.max.z),
					WVector(pMesh->localAabb.min.x, pMesh->localAabb.max.y,
						pMesh->localAabb.min.z),
					WVector(pMesh->localAabb.max.x, pMesh->localAabb.max.y,
						pMesh->localAabb.min.z),
					WVector(pMesh->localAabb.max.x, pMesh->localAabb.max.y,
						pMesh->localAabb.max.z),
					WVector(pMesh->localAabb.min.x, pMesh->localAabb.max.y,
						pMesh->localAabb.max.z),
				};
				float fMin = g_HUGE;
				float fMax = -g_HUGE;
				WMatrix mWorld;
				if (pMesh->aabbBone)
					mWorld = pMesh->aabbBone->m_matrix * pView->GetInvCamera();
				else
					mWorld = pBone->m_matrix * pView->GetInvCamera();
				for (int i = 0; i < 8; ++i)
				{
					v[i].z = TransformZ(v[i], mWorld);
					if (v[i].z < fMin)
						fMin = v[i].z;
					if (v[i].z > fMax)
						fMax = v[i].z;
				}
				float fDepth = fMax * 0.3f + fMin * 0.7f;
				pState->xfDepth = (fDepth - pView->clip_near) /
					(pView->clip_far - pView->clip_near);
			}
		}
	}
	pView->xDrawIndexedTriangles(*pState);
}
