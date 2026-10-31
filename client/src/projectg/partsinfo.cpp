#include "minatl.h"
#include "vc10_layout.h"
// HACK: GetPart goes through GetNumTotalParts only here, so that this object
// (and not extendedeffect) emits the out-of-line GetNumTotalParts
#define RB_GETPART_VIA_TOTAL
#include "partsinfo.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "uccmanager.h"
#include "ucclibrary.h"
#include "wresrcmng.h"
#include "wpuppet.h"
#include "wbone.h"
#include "../shared/localize.h"
#include "../shared/exceptionreport.h"
#include "jp/gdskinitem.h" // 983: Build/SetHairClr983 (CGDSkinItem "TanLines")
#include "jp/dummyplayer.h" // 983: Build (IDummyPlayer dono: m_kind, m_e0)
#include "gdskininstance983.h" // 983 00542f00
#include "commonutil.h" // 983: MakeStr (RefreshSubPets983)
#include <algorithm>
extern "C" void __cdecl jp983_rastro(const char* fmt, ...);

#define RB_STR_D_S "_%d%s"

CPartList::CPartList()
	: m_tidChar(0),
	  m_defPartNum(0),
	  m_partNum(0),
	  m_hairColor(0),
	  m_shirtsColor(0)
{
}

CPartList::~CPartList()
{
}

bool CPartList::Build(const CPartTidList* pTids)
{
	if (!pTids->IsComboValid())
		ItemManager()->GetDefCombo(pTids->m_charTid,
			const_cast<unsigned long*>(pTids->m_tid));

	memset(m_part, 0, sizeof(m_part));

	m_tidChar = pTids->m_charTid;
	m_defPartNum = pTids->m_defPartNum;
	m_partNum = pTids->m_partNum;
	m_hairColor = pTids->m_hairColor;
	m_shirtsColor = pTids->m_shirtsColor;

	for (int i = 0; i < m_partNum; i++)
	{
		if (pTids->m_tid[i])
		{
			IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pTids->m_tid[i]);
			m_part[i].tid = pTids->m_tid[i];
			m_part[i].pPart = pPart;
			m_part[i].uid = pTids->m_uid[i];
		}
	}
	return true;
}

unsigned long CPartList::SetPart(CPartTidList* pTids, unsigned long tid,
	unsigned long uid)
{
	unsigned long charIdx = (tid >> 18) & 0xff;
	if (charIdx == (m_tidChar & 0x3ffffff) &&
		charIdx == (pTids->m_charTid & 0x3ffffff) && IsIdentical(pTids) &&
		pTids->IsComboValid() && SetPart(tid, uid) && CheckSubParts())
	{
		unsigned long mask = 0;
		for (int i = 0; i < m_partNum; i++)
		{
			if (pTids->m_tid[i] != m_part[i].tid ||
				(IsLocalContent(S4_UCC) && m_part[i].tid == tid &&
					IsUccClothes(m_part[i].tid)))
			{
				mask |= 1 << i;
				pTids->m_tid[i] = m_part[i].tid;
				pTids->m_uid[i] = m_part[i].uid; // 983 008af1e0: sem IsLocalContent(S4_GUID_BASE)
			}
		}
		return mask;
	}
	return 0;
}

bool CPartList::SetPart(unsigned long tid, unsigned long uid)
{
	bool bKeep = false;
	unsigned char idx = (unsigned char)((tid >> 13) & 0x1f);

	if (m_part[idx].tid == tid)
	{
		if (uid && m_part[idx].uid && m_part[idx].uid != uid)
		{
		}
		else if (idx < m_defPartNum)
		{
			tid = (((m_tidChar << 5) | idx) << 13) | 0x8000400;
			uid = 0;
		}
		else
		{
			bKeep = true;
		}
	}

	IFF_STRUCT::sPart* pNew = ItemManager()->FindPart(tid);
	if (!pNew)
		return false;

	unsigned long remain = pNew->PosMask;
	for (int i = 0; i < m_partNum; i++)
	{
		if (m_part[i].tid)
		{
			unsigned long overlap = m_part[i].pPart->PosMask & pNew->PosMask;
			if (overlap)
			{
				remain &= ~overlap;
				if (overlap != m_part[i].pPart->PosMask)
				{
					switch ((m_part[i].tid >> 9) & 3)
					{
					case 0:
					case 2:
					{
						unsigned long subTid = m_part[i].tid | 0x200;
						IFF_STRUCT::sPart* pSub =
							ItemManager()->FindPart(subTid);
						if (pSub)
						{
							EmptyList(pSub->PosMask);
							for (int k = 0; k < m_partNum; k++)
							{
								if (pSub->PosMask & (1 << k))
								{
									m_part[k].tid = subTid;
									m_part[k].pPart = pSub;
									m_part[k].uid = 0;
									break;
								}
							}
						}
						else
						{
							unsigned long left =
								(pNew->PosMask ^ m_part[i].pPart->PosMask) &
								m_part[i].pPart->PosMask;
							for (int b = 0; b < m_partNum; b++)
							{
								if (!(left & (0xffffffff << b)))
									break;
								if (left & (1 << b))
								{
									unsigned long defTid =
										(((m_tidChar << 5) | b) << 13) |
										0x8000400;
									IFF_STRUCT::sPart* pDef =
										ItemManager()->FindPart(defTid);
									if (pDef)
									{
										if (pNew->PosMask & pDef->PosMask)
											return false;
										EmptyList(pDef->PosMask);
										m_part[b].tid = defTid;
									}
									else
									{
										m_part[b].tid = 0;
									}
									m_part[b].pPart = pDef;
									m_part[b].uid = 0;
								}
							}
						}
						break;
					}
					}
				}
				EmptyList(overlap);
			}
			if (!remain)
				break;
		}
	}

	if (!bKeep)
	{
		m_part[idx].tid = tid;
		m_part[idx].pPart = pNew;
		m_part[idx].uid = uid;
	}

	unsigned long used = 0;
	for (int i = 0; i < m_partNum; i++)
	{
		if (m_part[i].pPart)
			used |= m_part[i].pPart->PosMask;
	}

	if (!bKeep)
	{
		unsigned long defTid = (((m_tidChar << 5) | idx) << 13) | 0x8000600;
		IFF_STRUCT::sPart* pDef = ItemManager()->FindPart(defTid);
		if (pDef && !(used & pDef->PosMask))
		{
			int j;
			for (j = 0; j < m_defPartNum; j++)
			{
				if (pDef->PosMask & (1 << j))
					break;
			}
			if (m_part[j].tid == 0)
			{
				m_part[j].tid = defTid;
				m_part[j].pPart = pDef;
				m_part[j].uid = 0;
			}
		}
	}

	for (int i = 0; i < m_defPartNum; i++)
	{
		if (!(used & (1 << i)))
		{
			unsigned long defTid = (((m_tidChar << 5) | i) << 13) | 0x8000400;
			IFF_STRUCT::sPart* pDef = ItemManager()->FindPart(defTid);
			if (pDef && !(pDef->PosMask & used))
			{
				m_part[i].tid = defTid;
				m_part[i].pPart = pDef;
				m_part[i].uid = 0;
			}
		}
	}
	return true;
}

// 983: sem funcao propria (inline nos chamadores; so o asm 645 chama). COMDAT = fora das proprias do cmpfunc.
__declspec(noinline) inline void CPartList::EmptyList(unsigned long mask)
{
	for (int i = 0; i < m_partNum; i++)
	{
		if (!(mask & (0xffffffff << i)))
			break;
		if (mask & (1 << i))
		{
			m_part[i].tid = 0;
			m_part[i].pPart = 0;
			m_part[i].uid = 0;
		}
	}
}

bool CPartList::CheckSubParts()
{
	int i;
	for (i = 0; i < m_partNum; i++)
	{
		sPart& rPart = m_part[i];
		unsigned long tid = rPart.tid;
		if (tid && (tid & 0x600) == 0x200)
		{
			int j;
			for (j = 0; j < m_partNum; j++)
			{
				if (m_part[j].pPart && i != j)
				{
					int k;
					for (k = 0; k < 2; k++)
					{
						if (m_part[j].pPart->SubPart[k] == tid)
							break;
					}
					if (k < 2)
						break;
				}
			}
			if (j >= m_partNum)
			{
				if (!SetPart(tid, rPart.uid))
					return false;
			}
			else if (i >= m_defPartNum && rPart.pPart &&
				!IsEmpty(rPart.pPart->PosMask, i))
			{
				rPart.tid = 0;
				rPart.pPart = 0;
				rPart.uid = 0;
			}
		}
	}

	for (i = 0; i < m_partNum; i++)
	{
		if (m_part[i].pPart)
		{
			for (int k = 0; k < 2; k++)
			{
				unsigned long sub = m_part[i].pPart->SubPart[k];
				if (sub)
				{
					unsigned char si = (unsigned char)((sub >> 13) & 0x1f);
					if (si < m_defPartNum)
					{
						if ((m_part[si].tid & 0x600) == 0x400)
						{
							if (!SetPart(sub, m_part[i].uid))
								return false;
						}
					}
					else
					{
						IFF_STRUCT::sPart* pFind = ItemManager()->FindPart(sub);
						if (pFind && IsEmpty(pFind->PosMask, 0xff))
						{
							if (!SetPart(m_part[i].pPart->SubPart[k],
									m_part[i].uid))
								return false;
						}
					}
				}
			}
		}
	}
	return true;
}

bool CPartList::IsEmpty(unsigned long mask, unsigned char skip)
{
	for (int i = 0; i < m_partNum; i++)
	{
		if (m_part[i].pPart && skip != i && (m_part[i].pPart->PosMask & mask))
			return false;
	}
	return true;
}

bool CPartList::SetHairClr(CPartTidList* pTids, unsigned char clr)
{
	if (m_tidChar != pTids->m_charTid)
		return false;
	if (pTids->m_hairColor == clr)
		return false;
	pTids->m_hairColor = clr;
	m_hairColor = clr;
	return true;
}

bool CPartList::SetShirtsClr(CPartTidList* pTids, unsigned char clr)
{
	if (m_tidChar != pTids->m_charTid)
		return false;
	if (pTids->m_shirtsColor == clr)
		return false;
	pTids->m_shirtsColor = clr;
	m_shirtsColor = clr;
	return true;
}

bool CPartList::IsIdentical(CPartTidList* pTids)
{
	if (m_tidChar != pTids->m_charTid || m_defPartNum != pTids->m_defPartNum ||
		m_partNum != pTids->m_partNum)
		return false;

	for (int i = 0; i < m_partNum; i++)
	{
		if (m_part[i].tid != pTids->m_tid[i])
			return false;
	}

	// 983 008aca50: sem o IsLocalContent(S4_GUID_BASE) do 645 (compara os uid sempre)
	for (int i = 0; i < m_partNum; i++)
	{
		if (m_part[i].uid && pTids->m_uid[i] &&
			m_part[i].uid != pTids->m_uid[i])
			return false;
	}
	return true;
}

bool CPartList::FindPart(unsigned long tid) const
{
	for (unsigned char i = 0; i < m_partNum; i++)
	{
		if (GetPart(i)->tid == tid)
			return true;
	}
	return false;
}

CPetFrame::CPetFrame()
	: m_tidChar(0),
	  m_pBonePet(0),
	  m_hairTex(0),
	  m_shirtsTex(0),
	  m_faceTex(0),
	  m_pCurAniPet(0),
	  m_defPartNum(0),
	  m_partNum(0),
	  m_makeupIndex(1),
	  m_bShirtsClr(true),
	  m_bMakeup(false)
{
	memset(m_pPartPet, 0, sizeof(m_pPartPet));
	memset(m_partTex, 0, sizeof(m_partTex));
	memset(m_auxPart, 0, sizeof(m_auxPart));
}

CPetFrame::~CPetFrame()
{
	ReleasePets(7);
}

void CPetFrame::ReleasePets(int flags)
{
	if ((flags & 1) && g_resrcmng && m_pBonePet)
	{
		g_resrcmng->Release(m_pBonePet, false); // 983 008b0e50 (o teste D468 vazava; causa da queda do Lounge = AddBoneKey, D629)
		m_pBonePet = 0;
	}
	if (flags & 1)
		m_addonSubPets983.m_bones.clear(); // 983 008b0e50: esvazia a lista de ossos (+0x340) junto do boneco-base

	if (flags & 2)
	{
		for (std::vector<WPuppet*>::iterator it = m_aniPetList.begin();
			it != m_aniPetList.end(); ++it)
		{
			if (*it)
				g_resrcmng->Release(*it, false);
		}
		m_aniPetList.clear();
	}

	if (flags & 4)
	{
		for (int i = 0; i < m_partNum; i++)
		{
			if (g_resrcmng && m_pPartPet[i])
			{
				g_resrcmng->Release(m_pPartPet[i], false);
				m_pPartPet[i] = 0;
			}
			for (int j = 0; j < 3; j++)
			{
				if (g_resrcmng && m_partTex[i][j])
				{
					g_resrcmng->Release(m_partTex[i][j]);
					m_partTex[i][j] = 0;
				}
			}
		}

		if (m_partNum)
		{
			memset(m_pPartPet, 0, sizeof(m_pPartPet));
			memset(m_partTex, 0, sizeof(m_partTex));
			m_partNum = 0;
			m_defPartNum = 0;
			m_addonSubPets983.ReleaseAll983();
		}

		if (g_resrcmng && m_faceTex)
		{
			g_resrcmng->Release(m_faceTex);
			m_faceTex = 0;
		}
		if (g_resrcmng && m_hairTex)
		{
			g_resrcmng->Release(m_hairTex);
			m_hairTex = 0;
		}
		if (g_resrcmng && m_shirtsTex)
		{
			g_resrcmng->Release(m_shirtsTex);
			m_shirtsTex = 0;
		}
	}
}

// 983 008b23f0: o rosto passa pelo CGDSkinItem ("TanLines": marcas de bronzeado pelas pecas vestidas) quando nao ha dono ou
// quando o IDummyPlayer dono nao e o do podio (m_kind 1) e tem m_e0; o mesmo sinal vai ao SetHairClr983. O fim (malhas, mascara)
// e o EndBuild983 fora de linha.
bool CPetFrame::Build(const CPartTidList* pTids, unsigned long* pAux, bool bAni,
	bool bTnL, int nFace, IDummyPlayer* pOwner)
{
	char szBpet[128];
	char szApet[128];
	char szFace[256];
	CGDSkinItem* pSkinItem = CGDSkinItemPetJp::Instance983();
	std::string face;

	if (!m_partList.Build(pTids))
		return false;

	bool bSame = (pTids->m_charTid == m_tidChar);
	if (!bSame)
		ReleasePets(7);

	m_tidChar = pTids->m_charTid;
	m_defPartNum = pTids->m_defPartNum;
	m_partNum = pTids->m_partNum;

	IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(m_tidChar);
	if (!pChar)
		return false;

	sprintf(szBpet, "%s.bpet", pChar->Data);
	if (!bSame || !m_pBonePet)
	{
		m_pBonePet = g_resrcmng->GetPuppet(szBpet, false, false, false);
		if (!m_pBonePet)
		{
			g_exceptionReport.Crash();
			return false;
		}
	}

	sprintf(szApet, "%s.apet", pChar->Data);
	AddAniPet(szApet);
	SetPartPets(-1);

	if (pAux)
		memcpy(m_auxPart, pAux, sizeof(m_auxPart));
	else
		memset(m_auxPart, 0, sizeof(m_auxPart));

	if (!pOwner || (pOwner->m_kind != 1 && pOwner->m_e0))
	{
		pSkinItem->Replace("face", pChar->FaceTex, &face, (const sSkinParts983*)pTids);
		strcpy(szFace, face.c_str());
		sprintf(strrchr(szFace, '.'), RB_STR_D_S, nFace,
			strrchr(face.c_str(), '.'));
		m_faceTex = g_resrcmng->LoadTexture(face.c_str(), 0, 0, szFace);
	}
	else
	{
		strcpy(szFace, pChar->FaceTex);
		sprintf(strrchr(szFace, '.'), RB_STR_D_S, nFace,
			strrchr(pChar->FaceTex, '.'));
		m_faceTex = g_resrcmng->LoadTexture(pChar->FaceTex, 0, 0, szFace);
	}
	w_texlist* pTex = g_resrcmng->FindTexture(pChar->FaceTex);
	if (pTex)
		m_pBonePet->ChangeTextureByHandle(m_faceTex, pTex->texhandle, 0);

	EndBuild983();

	if (bTnL)
		m_pBonePet->xUpdateTnLBuffers(true);

	if (!bAni)
	{
		ReleasePets(2);
	}
	else
	{
		if (!pOwner)
			SetHairClr983(const_cast<CPartTidList*>(pTids), pTids->m_hairColor, 0, true);
		else if (pOwner->m_kind == 1 || !pOwner->m_e0)
			SetHairClr983(const_cast<CPartTidList*>(pTids), pTids->m_hairColor, 0, false);
		else
			SetHairClr983(const_cast<CPartTidList*>(pTids), pTids->m_hairColor, 0, true);
		SetShirtsClr(0, pTids->m_shirtsColor);
		m_bShirtsClr = true;
	}
	return true;
}

// 983 008b2370 (nome nosso): fim do Build - malhas e mascara como no 645, depois o WBoneSet de +0x28 fica com uma chave por osso
// do esqueleto, os ganchos de +0x58 sao avisados e o boneco e transformado uma vez
void CPetFrame::EndBuild983()
{
	C_ASSERT_VC10(sizeof(WBoneSet) == 0x30);
	C_ASSERT_VC10(offsetof(CPetFrame, m_hierObservers983) == 0x58);
	C_ASSERT_VC10(sizeof(sAddonSubPets983) == 0x24);
	C_ASSERT_VC10(offsetof(sAddonSubPets983, m_pAddonParts983) == 0x1c);
	C_ASSERT(sizeof(sSubPetDef983) == 0x120);
	C_ASSERT(sizeof(sSubPetDef983) == sizeof(IFF983::sAddonPart));
	C_ASSERT_VC10(sizeof(CPetFrame) == 0x358);
	// D872: a visao de petframe.h (REBANG_PETFRAME_*) depende destes valores; no cl 14.44 andam -0xc
	C_ASSERT_VC10(offsetof(CPetFrame, m_makeupIndex) == 0x70 && offsetof(CPetFrame, m_partList) == 0x1f8);
#if defined(_MSC_VER) && _MSC_VER > 1600
	C_ASSERT(offsetof(CPetFrame, m_makeupIndex) == 0x64 && offsetof(CPetFrame, m_partList) == 0x1ec);
	C_ASSERT(sizeof(CPetFrame) == 0x340);
#endif
	C_ASSERT(sizeof(CPartList) == 0x128);
	m_pBonePet->EndMeshBoneWork();
	ApplyHideMask(true);
	int count = 0;
	m_pBonePet->m_rootbone->CountBones(count);
	m_boneSet983.Clear();
	m_boneSet983.m_data.resize(count);
	for (std::vector<HierarchyChangeObserver*>::iterator it = m_hierObservers983.begin(); it != m_hierObservers983.end(); ++it)
		(*it)->OnHierarchyChange(this);
	m_pBonePet->Transform(NULL, 1.0f, 0);
}

void CPetFrame::AddObserver983(HierarchyChangeObserver* pObserver)
{
	if (std::find(m_hierObservers983.begin(), m_hierObservers983.end(), pObserver) == m_hierObservers983.end())
		m_hierObservers983.push_back(pObserver);
}

void LoadPartTex983(const char (*pTex)[40], const char (*pOrg)[40], int* tex, int* org); // 983 008acbc0 (mais abaixo)

// 983 008af370: o miolo do laco do SetPartPets do 645 (troca de texturas / de malhas do pet do slot e as texturas do slot)
bool CPetFrame::LoadPartPet983(const char* szPet, WPuppet** ppPet, int* partTex, int* tex, int* org, int hShirts)
{
	bool bShirtsClr = false;
	if (szPet)
	{
		if (*ppPet && stricmp((*ppPet)->GetPuppetName(), szPet) == 0)
		{
			for (int j = 0; j < 3; j++)
				m_pBonePet->ChangeTextureByHandle(tex[j], org[j], *ppPet);
		}
		else
		{
			WPuppet* pNew = g_resrcmng->GetPuppet(szPet, false, true, false);
			if (pNew)
			{
				if (m_pBonePet)
				{
					m_pBonePet->ReplaceMeshBones(*ppPet, pNew, tex[0], org[0]);
					for (int j = 1; j < 3; j++)
					{
						if (tex[j] && org[j])
							m_pBonePet->ChangeTextureByHandle(tex[j], org[j], pNew);
					}
					if (*ppPet)
						g_resrcmng->Release(*ppPet, false);
					*ppPet = pNew;
				}
			}
			else
			{
				CTask* pTask = AfxGetTask();
				if (pTask)
				{
					CGolfDoc* pDoc = (CGolfDoc*)pTask->GetDocument(); // 983 +0x68 = m_modelNameList
					if (pDoc)
					{
						char* pName = new char[strlen(szPet) + 1];
						strcpy(pName, szPet);
						pDoc->m_modelNameList.AddItem(pName, pName, false);
					}
				}
			}
		}

		for (int j = 0; j < 3; j++)
		{
			if (org[j] == hShirts && org[j] == tex[j])
				bShirtsClr = true;
			if (g_resrcmng && org[j])
			{
				g_resrcmng->Release(org[j]);
				org[j] = 0;
			}
		}
	}
	else
	{
		m_pBonePet->ReplaceMeshBones(*ppPet, 0, 0, 0);
		if (g_resrcmng && *ppPet)
		{
			g_resrcmng->Release(*ppPet, false);
			*ppPet = 0;
		}
	}

	for (int j = 0; j < 3; j++)
	{
		if (g_resrcmng && partTex[j])
		{
			g_resrcmng->Release(partTex[j]);
			partTex[j] = 0;
		}
		partTex[j] = tex[j];
	}
	return bShirtsClr;
}

// 983 008b1220
void sAddonSubPets983::CollectBones983(WPuppet* pBonePet)
{
	m_bones.clear();
	for (SubPetMap::iterator it = m_subPets.begin(); it != m_subPets.end(); ++it)
	{
		if (it->second.pDef && (it->second.pDef->tid & 0xfc000000) == 0x4000000 && pBonePet && it->second.pPet)
		{
			for (WBone* pMesh = it->second.pPet->FindMeshBone(NULL); pMesh; pMesh = it->second.pPet->FindMeshBone(pMesh))
			{
				WBone* pBone = pBonePet->FindBone(pMesh->GetBoneName());
				if (pBone)
					m_bones.push_back(pBone);
			}
		}
	}
}

// 983 008b1840: para cada registro do AddonPart.iff com o tid (no grupo do personagem) acha uma entrada da chave livre,
// de outro tid ou ja dele (senao cria) e aponta para o registro; sem bAll solta as entradas da chave de outro tid
sAddonSubPets983::SubPetRange sAddonSubPets983::Collect983(bool bAll, unsigned long key, unsigned long tid)
{
	SubPetRange range(m_subPets.end(), m_subPets.end());
	if (!m_pAddonParts983 || (!bAll && !m_bAddonParts983))
		return range;

	range = m_subPets.equal_range(key);
	std::pair<std::multimap<unsigned int, IFF983::sAddonPart>::iterator, std::multimap<unsigned int, IFF983::sAddonPart>::iterator>
		addons = m_pAddonParts983->equal_range(tid);
	for (std::multimap<unsigned int, IFF983::sAddonPart>::iterator a = addons.first; a != addons.second; ++a)
	{
		sSubPetDef983* pDef = (sSubPetDef983*)&a->second;
		SubPetMap::iterator it;
		for (it = range.first; it != range.second; ++it)
		{
			if (!it->second.pDef || tid != it->second.pDef->tid || it->second.pDef == pDef)
				break;
		}
		if (it == range.second)
		{
			it = m_subPets.insert(SubPetMap::value_type(key, sSubPet983()));
			range = m_subPets.equal_range(key);
		}
		it->second.pDef = pDef;
	}

	if (!bAll)
	{
		for (SubPetMap::iterator it = range.first; it != range.second; ++it)
		{
			if (it->second.pDef && it->second.pDef->tid != tid)
				it->second.pDef = NULL;
		}
	}
	return range;
}

// 983 008af2d0
bool sAddonSubPets983::HasSubPet983(const char* name)
{
	for (SubPetMap::iterator it = m_subPets.begin(); it != m_subPets.end(); ++it)
	{
		if (it->second.pPet && stricmp(it->second.pPet->GetPuppetName(), name) == 0)
			return true;
	}
	return false;
}

// 983 008b0d50: solta o boneco e as 3 texturas de cada sub-peca e esvazia o mapa
void sAddonSubPets983::ReleaseAll983()
{
	for (SubPetMap::iterator it = m_subPets.begin(); it != m_subPets.end(); ++it)
	{
		if (g_resrcmng && it->second.pPet)
		{
			g_resrcmng->Release(it->second.pPet, false);
			it->second.pPet = NULL;
		}
		for (int j = 0; j < 3; j++)
		{
			if (g_resrcmng && it->second.tex[j])
			{
				g_resrcmng->Release(it->second.tex[j]);
				it->second.tex[j] = 0;
			}
		}
	}
	m_subPets.clear(); // 983: o mesmo laco termina esvaziando o mapa
}

// 983 008b1620
sAddonSubPets983::~sAddonSubPets983()
{
}

// 983 008b1750
sAddonSubPets983::sAddonSubPets983()
{
	m_pAddonParts983 = NULL;
	m_bAddonParts983 = false;
}

// 983 008b1bc0
void CPetFrame::RefreshSubPets983(bool bAll, unsigned long key, unsigned long tid)
{
	sAddonSubPets983::SubPetRange range = m_addonSubPets983.Collect983(bAll, key, tid);
	for (sAddonSubPets983::SubPetMap::iterator it = range.first; it != range.second; ++it)
	{
		int tex[3];
		int org[3];
		memset(tex, 0, sizeof(tex));
		memset(org, 0, sizeof(org));
		const char* pszPet = NULL;
		if (it->second.pDef)
		{
			pszPet = MakeStr("%s.mpet", it->second.pDef->Data);
			LoadPartTex983(it->second.pDef->Tex, it->second.pDef->OrgTex, tex, org);
		}
		LoadPartPet983(pszPet, &it->second.pPet, it->second.tex, tex, org, 0);
	}
	if (bAll)
		m_addonSubPets983.CollectBones983(m_pBonePet);
}

// 983 008b1ce0: o do 645 com as texturas pelo sPartTex983 (UCC) e o pet de cada slot pelo LoadPartPet983; sub-pecas
// do personagem (m_addonSubPets983) recarregadas por slot e, com mask -1, todas
void CPetFrame::SetPartPets(unsigned long mask)
{
	int hShirts = 0;
	m_bShirtsClr = false;
	if (mask == (unsigned long)-1)
	{
		m_bMakeup = false;
		m_makeupIndex = 1;
		m_addonSubPets983.m_pAddonParts983 = ItemManager()->FindAddonPartGroup983(m_tidChar);
		RefreshSubPets983(true, m_tidChar, m_tidChar);
	}

	IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(m_tidChar);
	if (pChar->ShirtsTex[0])
		hShirts = g_resrcmng->LoadTexture(pChar->ShirtsTex, 0, 0, 0);

	for (int i = 0; i < m_partNum; i++)
	{
		if (!(mask & (0xffffffff << i)))
			break;
		if (!(mask & (1 << i)))
			continue;

		int org[3];
		int tex[3];
		memset(tex, 0, sizeof(tex));
		memset(org, 0, sizeof(org));
		const char* pszPet = NULL;
		char szPet[32];
		IFF_STRUCT::sPart* pPart = m_partList.GetPart(i)->pPart;
		if (pPart)
		{
			if ((pPart->c.TypeId & 0x1800) == 0x1000)
			{
				m_makeupIndex = i;
				m_bMakeup = true;
			}
			else if (pPart->PosMask & 2)
			{
				m_makeupIndex = 1;
				m_bMakeup = false;
			}
			sprintf(szPet, "%s.mpet", pPart->Data);
			pszPet = szPet;
			sPartTex983 parts;
			sPartTex983::Load(pPart, m_partList.GetPart(i)->uid, &parts);
			memcpy(tex, parts.tex, sizeof(tex));
			memcpy(org, parts.org, sizeof(org));
		}
		m_bShirtsClr = LoadPartPet983(pszPet, &m_pPartPet[i], m_partTex[i], tex, org, hShirts);
		RefreshSubPets983(false, i, m_partList.GetPart(i)->tid);
	}

	int idxA = -1;
	int idxB = -1;
	switch (pChar->c.TypeId & 0x3ffffff)
	{
	case 2:
	case 7:
		idxA = 0x13;
		idxB = 1;
		break;
	case 4:
		idxA = 0x14;
		idxB = 1;
		break;
	}

	bool bHasA = false;
	bool bFlagB = false;
	IFF_STRUCT::sPart* pPart;
	if (idxA >= 0 && m_partList.GetPart(idxA) && m_partList.GetPart(idxA)->tid)
		bHasA = true;
	if (idxB >= 0 && m_partList.GetPart(idxB) &&
		(m_partList.GetPart(idxB)->tid & 0x1800) == 0x1000)
		bFlagB = true;

	if (bHasA)
	{
		if (idxB >= 0)
		{
			m_pBonePet->ReplaceMeshBones(m_pPartPet[idxB], 0, 0, 0);
			if (g_resrcmng && m_pPartPet[idxB])
			{
				g_resrcmng->Release(m_pPartPet[idxB], false);
				m_pPartPet[idxB] = 0;
			}
		}
	}
	else if (idxB >= 0 && m_partList.GetPart(idxB) &&
		(pPart = m_partList.GetPart(idxB)->pPart))
	{
		char szPet[32] = "";
		int tex[3];
		int org[3];
		sprintf(szPet, "%s.mpet", pPart->Data);
		for (int j = 0; j < 3; j++)
		{
			tex[j] = 0;
			org[j] = 0;
			if (pPart->Tex[j][0] && pPart->OrgTex[j][0])
			{
				tex[j] = g_resrcmng->LoadTexture(pPart->Tex[j], 0, 0, 0);
				org[j] = g_resrcmng->LoadTexture(pPart->OrgTex[j], 0, 0, 0);
			}
		}
		WPuppet* pNew = g_resrcmng->GetPuppet(szPet, false, true, false);
		if (pNew)
		{
			m_pBonePet->ReplaceMeshBones(m_pPartPet[idxB], pNew, tex[0],
				org[0]);
			for (int j = 1; j < 3; j++)
			{
				if (tex[j] && org[j])
					m_pBonePet->ChangeTextureByHandle(tex[j], org[j], pNew);
			}
			if (m_pPartPet[idxB])
				g_resrcmng->Release(m_pPartPet[idxB], false);
			m_pPartPet[idxB] = pNew;
		}
	}

	if (bFlagB)
	{
		m_makeupIndex = idxB;
		m_bMakeup = true;
		if (g_resrcmng && m_faceTex)
		{
			g_resrcmng->Release(m_faceTex);
			m_faceTex = 0;
		}
		char szFace[256];
		strcpy(szFace, pChar->FaceTex);
		sprintf(strrchr(szFace, '.'), "_%d%s", idxB,
			strrchr(pChar->FaceTex, '.'));
		m_faceTex = g_resrcmng->LoadTexture(pChar->FaceTex, 0, 0, szFace);
		w_texlist* pTex = g_resrcmng->FindTexture(pChar->FaceTex);
		if (pTex)
			m_pBonePet->ChangeTextureByHandle(m_faceTex, pTex->texhandle, 0);
	}

	if (g_resrcmng && hShirts)
		g_resrcmng->Release(hShirts);
}

char* CPetFrame::GetMakeupTexture()
{
	return m_partList.GetPart(m_makeupIndex)->pPart->Tex[0];
}

WPuppet* CPetFrame::GetBonePet()
{
	return m_pBonePet;
}

bool CPetFrame::Refresh(CPartTidList* pTids)
{
	if (!ResetFace(pTids))
		return false;
	return SetMakeup() ? true : false;
}

bool CPetFrame::ResetFace(CPartTidList* pTids)
{
	IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(m_tidChar);
	if (!pChar)
		return false;
	g_resrcmng->ChangeTexturePart(m_faceTex, pChar->FaceTex, 0, 0);
	SetHairClr983(pTids, pTids->m_hairColor, 0, true); // 983 008b0900
	return true;
}

void CPetFrame::AddAniPet(char* name)
{
	for (std::vector<WPuppet*>::iterator it = m_aniPetList.begin();
		it != m_aniPetList.end(); ++it)
	{
		if (stricmp((*it)->GetPuppetName(), name) == 0)
			return;
	}
	WPuppet* pPet = g_resrcmng->GetPuppet(name, false, true, false);
	if (pPet)
		m_aniPetList.push_back(pPet);
}

std::vector<WPuppet*>& CPetFrame::GetAniPetList()
{
	return m_aniPetList;
}

w_motion_data* CPetFrame::FindMotionData(const char* name, bool bSetCur,
	WPuppet** ppPet)
{
	for (std::vector<WPuppet*>::iterator it = m_aniPetList.begin();
		it != m_aniPetList.end(); ++it)
	{
		w_motion_data* pData = (*it)->FindMotionData(name);
		if (pData)
		{
			if (ppPet)
				*ppPet = *it;
			if (bSetCur)
				m_pCurAniPet = *it;
			return pData;
		}
	}
	return 0;
}

int CPetFrame::GetMotionCountIncludeName(const char* include,
	const char* exclude)
{
	int n = 0;
	for (std::vector<WPuppet*>::iterator it = m_aniPetList.begin();
		it != m_aniPetList.end(); ++it)
	{
		n = (*it)->GetMotionCountIncludeName(include, exclude);
		if (n > 0)
			return n;
	}
	return n;
}

WPuppet* CPetFrame::GetCurAniPet()
{
	return m_pCurAniPet;
}

// 983 008acc90
void CPetFrame::ApplyBoneSet983(WBoneSet* pSet)
{
	m_pBonePet->ApplyBones983(pSet);
}

// 983 008acdf0: o GetBones + ApplyBones do 645 numa chamada (WPuppet::Animate983 no boneco da animacao atual, filtro = boneco
// dos ossos); sem conjunto, usa o do proprio quadro (+0x28) e o aplica no boneco dos ossos (00b7b180)
void CPetFrame::Animate983(WBoneSet* pSet, const char* name, float time)
{
	if (pSet)
	{
		m_pCurAniPet->Animate983(pSet, name, time, 0, NULL, m_pBonePet);
		return;
	}
	m_pCurAniPet->Animate983((WBoneSet*)&m_boneSet983, name, time, 0, NULL, m_pBonePet);
	m_pBonePet->ApplyBones983((WBoneSet*)&m_boneSet983);
}

void CPetFrame::Transform(WView* pView, float scale, int flags)
{
	m_pBonePet->Transform(pView, scale, flags);
}

void CPetFrame::UpdateBound(bool bTransform)
{
	m_pBonePet->UpdateBound(bTransform, false);
}

void CPetFrame::UpdateLightSource(LightSet* pLight, bool bAdd, WScene* pScene)
{
	m_pBonePet->UpdateLightSource(pLight, bAdd, pScene);
}

void CPetFrame::SetAlpha(unsigned char alpha)
{
	m_pBonePet->SetAlpha(alpha);
}

void CPetFrame::SetAlpha(char* name, unsigned char alpha)
{
	m_pBonePet->SetAlpha(name, alpha);
}

void CPetFrame::Render(WView* pView, bool bTransform, float scale, bool b0,
	bool b1, bool b2)
{
	m_pBonePet->Render(pView, bTransform, scale, b0, b1, b2);
}

// 983 008af620 (*ifmpet do ParsePuppetScript 00875c80 e do script do pet 005aca50): pets das pecas e, depois, as sub-pecas do
// AddonPart (008af2d0; ex. asas g_wing/Wing_Feather do brinco 081a4801). COMDAT; endereco guardado abaixo (exhibition.cpp liga por /alternatename).
__declspec(noinline) bool CPetFrame::HasMPet(char* name)
{
	for (int i = 0; i < m_partNum; i++)
	{
		if (m_pPartPet[i] && stricmp(m_pPartPet[i]->GetPuppetName(), name) == 0)
			return true;
	}
	return m_addonSubPets983.HasSubPet983(name);
}

void CPetFrame::SetInvisible(bool bInvisible)
{
	if (m_pBonePet)
	{
		m_pBonePet->GetRootBone()->SetInvisible(true, bInvisible);
		if (!bInvisible)
			ApplyHideMask(true);
	}
}

// 983 008b28b0
unsigned long CPetFrame::SetPart(CPartTidList* pTids, unsigned long tid,
	unsigned long uid, bool bHairTid)
{
	ApplyHideMask(false);
	unsigned long mask = m_partList.SetPart(pTids, tid, uid);
	if (mask)
	{
		SetPartPets(mask);
		if (!bHairTid)
			tid = 0;
		SetHairClr983(0, pTids->m_hairColor, tid, true);
		SetShirtsClr(0, pTids->m_shirtsColor);
		m_bShirtsClr = true;
		EndBuild983();
		return mask;
	}
	return 0;
}

// 983 008acb20
void sPartTex983::Clear()
{
	tex[0] = 0;
	tex[1] = 0;
	tex[2] = 0;
	org[0] = 0;
	org[1] = 0;
	org[2] = 0;
}

// 983 008acb40
void sPartTex983::Apply(WPuppet* pPuppet)
{
	for (unsigned int i = 0; i < 3; i++)
	{
		if (tex[i] && org[i])
			pPuppet->ChangeTextureByHandle(tex[i], org[i], 0);
	}
}

// 983 008acb80
void sPartTex983::Release()
{
	for (int i = 0; i < 3; i++)
	{
		if (g_resrcmng && org[i])
		{
			g_resrcmng->Release(org[i]);
			org[i] = 0;
		}
	}
}

// 983 008acbc0: o LOAD_PART_TEXTURES do 645 fora de linha
void LoadPartTex983(const char (*pTex)[40], const char (*pOrg)[40], int* tex, int* org)
{
	for (int j = 0; j < 3; j++)
	{
		if (pTex[j][0] && pOrg[j][0])
		{
			tex[j] = g_resrcmng->LoadTexture(pTex[j], 0, 0, 0);
			org[j] = g_resrcmng->LoadTexture(pOrg[j], 0, 0, 0);
		}
	}
}

// 983 008b05a0 (o trecho UCC do SetPartPets do 645; so S4_UCC, sem o S4_GUID_BASE; o indice e copiado depois do IsDownloadComplete)
void __stdcall sPartTex983::Load(IFF_STRUCT::sPart* pPart, unsigned long uid, sPartTex983* pOut)
{
	pOut->Clear();
	if (IsLocalContent(S4_UCC) && IsUccClothes(pPart->c.TypeId))
	{
		char uccIdx[9];
		memset(uccIdx, 0, sizeof(uccIdx));
		sUccClothes* pUcc = UccManager()->FindClothes(uid);
		if (pUcc)
		{
			if (UccManager()->IsDownloadComplete(pUcc))
			{
				strncpy(uccIdx, pUcc->uccIndex, 9);
				pOut->tex[0] = g_resrcmng->LoadTexture(
					GenerateTextureName_F(pPart->c.TypeId, uccIdx).c_str(), 0, 0, 0);
				pOut->org[0] = g_resrcmng->LoadTexture(pPart->Tex[0], 0, 0, 0);
				pOut->tex[1] = g_resrcmng->LoadTexture(
					GenerateTextureName_B(pPart->c.TypeId, uccIdx).c_str(), 0, 0, 0);
				pOut->org[1] = g_resrcmng->LoadTexture(pPart->Tex[1], 0, 0, 0);
				if (pOut->tex[0] == 0)
					pOut->tex[0] = g_resrcmng->LoadTexture(pPart->Tex[0], 0, 0, 0);
				if (pOut->tex[1] == 0)
					pOut->tex[1] = g_resrcmng->LoadTexture(pPart->Tex[1], 0, 0, 0);
				return;
			}
		}
		else
		{
			sItemInfo* pItem = GetMyItemInfo(uid);
			if (pItem)
				strncpy(uccIdx, pItem->UccIndex, 9);

			std::list<sItemInfo>::iterator it;
			for (it = Doc()->m_uccItemList.begin(); it != Doc()->m_uccItemList.end(); ++it)
			{
				if (it->tid == pPart->c.TypeId && strlen(uccIdx) &&
					strcmp(it->UccIndex, uccIdx) == 0 && !(it->status & 1))
					break;
			}
			if (it != Doc()->m_uccItemList.end())
			{
				pOut->tex[0] = g_resrcmng->LoadTexture(
					GenerateTextureName_F(pPart->c.TypeId, uccIdx).c_str(), 0, 0, 0);
				pOut->org[0] = g_resrcmng->LoadTexture(pPart->Tex[0], 0, 0, 0);
				pOut->tex[1] = g_resrcmng->LoadTexture(
					GenerateTextureName_B(pPart->c.TypeId, uccIdx).c_str(), 0, 0, 0);
				pOut->org[1] = g_resrcmng->LoadTexture(pPart->Tex[1], 0, 0, 0);
				return;
			}
		}
	}
	LoadPartTex983(pPart->Tex, pPart->OrgTex, pOut->tex, pOut->org);
}

// 983 008ad8e0
sPartTex983::sPartTex983()
{
	Clear();
}

// 983 008b0960: chamado pela AvatarBase (0047e4f0) e por quem acha a roupa UCC no CUccManager (004d7e83 etc.)
void CPetFrame::SetPart983(const sUccItem* pUcc)
{
	sPartTex983 parts;
	IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pUcc->typeId);
	sPartTex983::Load(pPart, pUcc->id, &parts);
	parts.Apply(m_pBonePet);
	parts.Release();
}

bool CPetFrame::SetAuxPart(unsigned char idx, unsigned long id)
{
	if (idx >= 5)
		return false;
	m_auxPart[idx] = id;
	return true;
}

bool CPetFrame::FindAuxPart(unsigned long id)
{
	for (int i = 0; i < 5; i++)
	{
		if (m_auxPart[i] == id)
			return true;
	}
	return false;
}

// 983 00e26ed0: 16 entradas (personagens 0..14 + 0); a do 645 tinha 10 e os tipos 10..14 liam o FIXHAIR::exTable
// (SetHairClr/SetHairClr983 008af755 indexam por m_charTid & 0x3ffffff)
int g_iIndexWigColor[16] = { 16, 16, 16, 16, 16, 20, 14, 16, 16, 16, 16, 14, 14, 16, 16 };

FIXHAIR::sFixItem FIXHAIR::exTable[3] = {
	{ 0, 0x8006031, "m_hair_02-01.jpg"   },
	{ 0, 0x8006032, "m_hair_02-01.jpg"   },
	{ 5, 0x8168009, "2#e_hair_07-07.jpg" },
};

int __fastcall IsFixedHairStyle(unsigned long tid)
{
	for (int i = 0; i < 3; i++)
	{
		if (FIXHAIR::exTable[i].tid == tid)
			return i;
	}
	return -1;
}

bool CPetFrame::SetHairClr(CPartTidList* pTids, unsigned char clr)
{
	bool bFixed = false;
	char szFix[40] = "";
	char szWig[50] = "";

	if (pTids)
	{
		for (int i = 0; i < pTids->m_partNum; i++)
		{
			int idx = IsFixedHairStyle(pTids->m_tid[i]);
			if (idx != -1)
			{
				bFixed = true;
				strcpy(szWig, FIXHAIR::exTable[idx].texture.c_str());
			}
			if (((pTids->m_tid[i] >> 13) & 0x1f) ==
				(unsigned long)g_iIndexWigColor[pTids->m_charTid & 0x3ffffff])
			{
				IFF_STRUCT::sPart* pPart =
					ItemManager()->FindPart(pTids->m_tid[i]);
				if (pPart)
				{
					int hOrg =
						g_resrcmng->LoadTexture(pPart->OrgTex[0], 0, 0, 0);
					if (bFixed && szWig[0])
						strncpy(szFix, szWig, 39);
					else
						strcpy(szFix, pPart->OrgTex[0]);
					char* p = strrchr(szFix, '.');
					if (!p)
						return false;
					p -= 2;
					int n = atoi(p);
					if (bFixed)
						sprintf(p, "%02d.jpg", n);
					else
						sprintf(p, "%02d.jpg", clr + n);
					int hNew = g_resrcmng->LoadTexture(szFix, 0, 0, 0);
					m_pBonePet->ChangeTextureByHandle(hNew, hOrg, 0);
				}
				break;
			}
		}
		if (m_tidChar != pTids->m_charTid)
			return false;
	}

	IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(m_tidChar);
	int hOrg;
	if (bFixed == true && szWig[0])
		hOrg = g_resrcmng->LoadTexture(szWig, 0, 0, 0);
	else
		hOrg = g_resrcmng->LoadTexture(pChar->HairTex, 0, 0, 0);
	strcpy(szFix, pChar->HairTex);
	if (bFixed == true && szWig[0])
		strncpy(szFix, szWig, 39);
	char* p = strrchr(szFix, '.');
	if (!p)
		return false;
	p -= 2;
	int n = atoi(p);
	if (bFixed == true)
		sprintf(p, "%02d.jpg", n);
	else
		sprintf(p, "%02d.jpg", clr + n);
	int hNew = g_resrcmng->LoadTexture(szFix, 0, 0, 0);
	m_pBonePet->ChangeTextureByHandle(hNew, hOrg, 0);
	if (g_resrcmng && hOrg)
		g_resrcmng->Release(hOrg);
	if (m_hairTex)
		g_resrcmng->Release(m_hairTex);
	m_hairTex = hNew;

	if ((pChar->c.TypeId & 0x3ffffff) > 2)
	{
		char szTmp[40];
		char szHr[40];
		strcpy(szTmp, pChar->FaceTex);
		*strrchr(szTmp, '.') = 0;
		sprintf(szHr, "%s_hr%02d.tga", szTmp, clr + 1);
		Bitmap* pBmp1 = g_resrcmng->LoadBitmap(pChar->FaceTex, 0, false);
		Bitmap* pBmp2 = g_resrcmng->LoadBitmap(szHr, 0, false);
		if (pBmp1)
		{
			if (pBmp2)
			{
				Bitmap* pComb = g_resrcmng->Combine(pBmp1, pBmp2);
				tagRECT rc;
				rc.left = 0;
				rc.top = 0;
				rc.right = pComb->Width();
				rc.bottom = pComb->Height();
				g_resrcmng->ChangeTexturePart(m_faceTex, pComb, rc);
				delete pBmp1;
				delete pBmp2;
				delete pComb;
			}
			else
			{
				delete pBmp1;
			}
		}
		else if (pBmp2)
		{
			delete pBmp2;
		}

		w_texlist* pTex = g_resrcmng->FindTexture(pChar->FaceTex);
		if (pTex)
			m_pBonePet->ChangeTextureByHandle(m_faceTex, pTex->texhandle, 0);
	}
	return true;
}

// 983 008af6b0: o SetHairClr do 983 (o de cima e o do 645, ainda chamado pelas unidades nao relidas). Diferencas: a instancia do
// CGDSkinItem e pega logo no inicio; grava a cor em pTids; com bSkin o rosto base do "_hr" vem do CGDSkinItem::Apply("face",
// cur = hairTid); os tipos de personagem 10, 11, 12 e 14 nao combinam o "_hr".
bool CPetFrame::SetHairClr983(CPartTidList* pTids, unsigned char clr, unsigned long hairTid, bool bSkin)
{
	CGDSkinItem* pSkinItem = CGDSkinItemPetJp::Instance983();
	std::string face;
	bool bFixed = false;
	char szFix[40] = "";
	char szWig[50] = "";

	if (pTids)
	{
		for (int i = 0; i < pTids->m_partNum; i++)
		{
			int idx = IsFixedHairStyle(pTids->m_tid[i]);
			if (idx != -1)
			{
				bFixed = true;
				strcpy(szWig, FIXHAIR::exTable[idx].texture.c_str());
			}
			if (((pTids->m_tid[i] >> 13) & 0x1f) ==
				(unsigned long)g_iIndexWigColor[pTids->m_charTid & 0x3ffffff])
			{
				IFF_STRUCT::sPart* pPart =
					ItemManager()->FindPart(pTids->m_tid[i]);
				if (pPart)
				{
					int hOrg =
						g_resrcmng->LoadTexture(pPart->OrgTex[0], 0, 0, 0);
					if (bFixed && szWig[0])
						strncpy(szFix, szWig, 39);
					else
						strcpy(szFix, pPart->OrgTex[0]);
					char* p = strrchr(szFix, '.');
					if (!p)
						return false;
					p -= 2;
					int n = atoi(p);
					if (bFixed)
						sprintf(p, "%02d.jpg", n);
					else
						sprintf(p, "%02d.jpg", clr + n);
					int hNew = g_resrcmng->LoadTexture(szFix, 0, 0, 0);
					m_pBonePet->ChangeTextureByHandle(hNew, hOrg, 0);
				}
				break;
			}
		}
		if (m_tidChar != pTids->m_charTid)
			return false;
		if (pTids->m_hairColor != clr)
			pTids->m_hairColor = clr;
	}

	IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(m_tidChar);
	int hOrg;
	if (bFixed == true && szWig[0])
		hOrg = g_resrcmng->LoadTexture(szWig, 0, 0, 0);
	else
		hOrg = g_resrcmng->LoadTexture(pChar->HairTex, 0, 0, 0);
	strcpy(szFix, pChar->HairTex);
	if (bFixed == true && szWig[0])
		strncpy(szFix, szWig, 39);
	char* p = strrchr(szFix, '.');
	if (!p)
		return false;
	p -= 2;
	int n = atoi(p);
	if (bFixed == true)
		sprintf(p, "%02d.jpg", n);
	else
		sprintf(p, "%02d.jpg", clr + n);
	int hNew = g_resrcmng->LoadTexture(szFix, 0, 0, 0);
	m_pBonePet->ChangeTextureByHandle(hNew, hOrg, 0);
	if (g_resrcmng && hOrg)
		g_resrcmng->Release(hOrg);
	if (m_hairTex)
		g_resrcmng->Release(m_hairTex);
	m_hairTex = hNew;

	if ((pChar->c.TypeId & 0x3ffffff) > 2)
	{
		char szTmp[40];
		char szHr[40];
		if (!bSkin)
		{
			strcpy(szTmp, pChar->FaceTex);
		}
		else
		{
			pSkinItem->Apply("face", pChar->FaceTex, &face, (const sSkinParts983*)pTids, hairTid);
			strcpy(szTmp, pChar->FaceTex);
		}
		*strrchr(szTmp, '.') = 0;
		sprintf(szHr, "%s_hr%02d.tga", szTmp, clr + 1);
		unsigned long type = pChar->c.TypeId & 0x3ffffff;
		if (type != 10 && type != 11 && type != 12 && type != 14)
		{
			Bitmap* pBmp1 = g_resrcmng->LoadBitmap(bSkin ? face.c_str() : pChar->FaceTex, 0, false);
			Bitmap* pBmp2 = g_resrcmng->LoadBitmap(szHr, 0, false);
			if (pBmp1)
			{
				if (pBmp2)
				{
					Bitmap* pComb = g_resrcmng->Combine(pBmp1, pBmp2);
					tagRECT rc;
					rc.left = 0;
					rc.top = 0;
					rc.right = pComb->Width();
					rc.bottom = pComb->Height();
					g_resrcmng->ChangeTexturePart(m_faceTex, pComb, rc);
					delete pBmp1;
					delete pBmp2;
					delete pComb;
				}
				else
				{
					delete pBmp1;
				}
			}
			else if (pBmp2)
			{
				delete pBmp2;
			}
		}

		w_texlist* pTex = g_resrcmng->FindTexture(pChar->FaceTex);
		if (pTex)
			m_pBonePet->ChangeTextureByHandle(m_faceTex, pTex->texhandle, 0);
	}
	return true;
}


bool CPetFrame::SetShirtsClr(CPartTidList* pTids, unsigned char clr)
{
	if (!m_bShirtsClr)
		return true;

	if (pTids)
	{
		if (m_tidChar != pTids->m_charTid)
			return false;
		if (m_partList.m_tidChar != pTids->m_charTid)
			return false;
		if (pTids->m_shirtsColor == clr)
			return false;
		pTids->m_shirtsColor = clr;
		m_partList.m_shirtsColor = clr;
	}

	IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(m_tidChar);
	if (!pChar->ShirtsTex[0])
		return false;

	int hOrg = g_resrcmng->LoadTexture(pChar->ShirtsTex, 0, 0, 0);
	char* pDot = strrchr(pChar->ShirtsTex, '.');
	if (!pDot)
		return false;
	pDot--;
	*pDot = '1' + clr;
	int hNew = g_resrcmng->LoadTexture(pChar->ShirtsTex, 0, 0, 0);
	m_pBonePet->ChangeTextureByHandle(hNew, hOrg, 0);
	*pDot = '1';
	if (g_resrcmng && hOrg)
		g_resrcmng->Release(hOrg);
	if (m_shirtsTex)
		g_resrcmng->Release(m_shirtsTex);
	m_shirtsTex = hNew;
	return true;
}

bool CPetFrame::SetMakeup()
{
	if (m_bMakeup)
	{
		if (!m_pBonePet->ChangeTexturePart(m_faceTex, GetMakeupTexture(), 0, 0))
			return false;
	}
	return true;
}

void CPetFrame::CalcBVolume()
{
	if (m_pBonePet)
	{
		std::vector<WPuppet*>::iterator it = m_aniPetList.begin();
		if (it == m_aniPetList.end())
		{
			m_pBonePet->CalcBound(0, true);
		}
		else
		{
			m_pBonePet->CalcBound(true, *it, true, 0, 0);
			for (++it; it != m_aniPetList.end(); ++it)
				m_pBonePet->CalcBound(false, *it, true, 0, 0);
		}
	}
}

void CPetFrame::EndSetPart()
{
	m_pBonePet->EndMeshBoneWork();
	ApplyHideMask(true);
}

void CPetFrame::ApplyHideMask(bool bHide)
{
	// HACK: unused buffer, the original has a /GS cookie here
	char szUnused[64];
	for (int i = 0; i < m_partNum; i++)
	{
		if (m_partList.m_part[i].tid)
		{
			IFF_STRUCT::sPart* pPart = m_partList.m_part[i].pPart;
			if (!pPart || !pPart->HideMask)
				continue;
			for (int j = 0; j < m_partNum; j++)
			{
				if ((pPart->HideMask & (1 << j)) && i != j &&
					m_partList.m_part[j].tid && m_partList.m_part[j].pPart &&
					m_pPartPet[j])
				{
					for (WBone* pBone = m_pPartPet[j]->FindMeshBone(0); pBone;
						pBone = m_pPartPet[j]->FindMeshBone(pBone))
					{
						WBone* pFound =
							m_pBonePet->FindBone(pBone->GetBoneName());
						if (pFound)
							pFound->SetInvisible(false, bHide);
					}
				}
			}
		}
	}
}

bool CPetFrame::ChangeTexturePart(char* name, char* src, Bitmap** ppBmp,
	tagRECT* pRc)
{
	w_face_animation* pAnim = FindFaceAnim(name);
	if (!pAnim)
		return false;
	return g_resrcmng->ChangeTexturePart(pAnim->filename, src, ppBmp, pRc);
}

void CPetFrame::ChangeTexturePart(char* name, Bitmap* pBmp, const tagRECT& rc)
{
	w_face_animation* pAnim = FindFaceAnim(name);
	if (pAnim)
		g_resrcmng->ChangeTexturePart(pAnim->filename, pBmp, rc);
}

void CPetFrame::ChangeTexture(char* name, char* tex)
{
	int h1 = g_resrcmng->LoadTexture(name, 0, 0, 0);
	int h2 = g_resrcmng->LoadTexture(tex, 0, 0, 0);
	if (h1 && h2)
		m_pBonePet->ChangeTextureByHandle(h2, h1, 0);
}

w_face_animation* CPetFrame::FindFaceAnim(char* name)
{
	w_face_animation* pAnim = 0;
	for (int i = 0; i < m_partNum; i++)
	{
		if (m_pPartPet[i])
		{
			pAnim = m_pPartPet[i]->FindFaceAnim(name);
			if (pAnim)
				break;
		}
	}
	return pAnim;
}

bool (CPetFrame::* const g_rbKeepHasMPet)(char*) = &CPetFrame::HasMPet;
