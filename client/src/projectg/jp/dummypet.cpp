// JP 983: IDummyPet (004d1190–004d4800: construtor 004d37d0, destrutor 004d38c0, as 10 virtuais próprias, os 4 carregadores
// de caddie/taco/mascote, a troca de textura e 8 auxiliares). Fonte reconstruída do 983 (classe sem par no 645; par no KR 839).
// Sessões B29 e B30.
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "dummypet.h"
#include "lobbyjp.h"
#include "shareddoc.h"
#include "commonutil.h"
#include "wpuppet.h"
#include "wbone.h"
#include "wresrcmng.h"
#include "fxinterpreter.h"
#include "mascot.h"
#include "soundmanager.h"
#include "texturepart.h"
#include "gdskinitem.h"
#include <iterator>
#include <ctype.h>
#include <algorithm>

extern const WVector g_vUp983_e42538; // 00e42538 = (0, 1, 0)
const char* GetStrConst(const char* src, char* dst);

// WPuppet (engine, frente A): 00b86cf0 e o acerto da matriz 004d18f0 (cópia fora de linha desta unidade)
class WPuppetJp : public WPuppet
{
public:
	void Animate983(const char* a, const char* b, float time, int c, int d, int e); // 00b86cf0
	void SetMatrix983(const WMatrix& m) { m_mat = m; }
};

// CItemManager do 983: 00a171e0 (registro com o nome em +0x49), FindClub 00a171a0 e FindMascot 00a179d0 (nome em +0xc0)
class CItemManagerPetJp
{
public:
	char* Find1e0_983(unsigned long typeId); // 00a171e0
	char* FindClub983(unsigned long typeId); // 00a171a0
	char* FindMascot983(unsigned long typeId); // 00a179d0
};
#define ITEMMGRPETJP() ((CItemManagerPetJp*)&Doc()->m_itemManager)
#define MASCOTMAP983() (Doc()->m_mascotMap)

struct sDeleteTexPart983
{
	void operator()(sTexturePart* p) const
	{
		if (p)
			delete p;
	}
};

// CGDSkinItem (include/jp/gdskinitem.h): 00542f00 devolve a instância ("TanLines" do CGameDataDB na 1ª chamada)
class CGDSkinItemPetJp
{
public:
	static CGDSkinItem* Instance983(); // 00542f00
};

// caddie dono de uma roupa: os dois campos de 5 bits do typeId da roupa somados
#define CADDIE_OF_SKIN983(id) (((((id) >> 0x15) & 0x1f) + (((id) >> 0x10) & 0x1f)) | 0x1c000000)

IDummyPet::IDummyPet(int kind)
	: m_pPuppet(NULL),
	  m_kind(kind),
	  m_hTex(0),
	  m_pMotion(NULL),
	  m_pTexPart(NULL),
	  m_bInit(false),
	  m_pFx(NULL),
	  m_fxId(0)
{
}

IDummyPet::~IDummyPet()
{
	std::for_each(m_texList.begin(), m_texList.end(), sDeleteTexPart983());
	if (m_pFx)
	{
		delete m_pFx;
		m_pFx = NULL;
	}
	if (m_pPuppet && g_resrcmng)
	{
		g_resrcmng->Release(m_pPuppet, false);
		m_pPuppet = NULL;
	}
	m_bInit = false;
	m_fxId = 0;
	if (g_resrcmng && m_hTex)
	{
		g_resrcmng->Release(m_hTex);
		m_hTex = 0;
	}
}

void IDummyPet::OnScriptFx(const char*& p, bool bSkip, int cmd)
{
}

void IDummyPet::OnScriptShowBone(const char*& p, int cmd)
{
	ScriptShowBonePet983(m_pPuppet, cmd, p);
}

void IDummyPet::OnScriptPlayIf(const char*& p)
{
}

void IDummyPet::OnScriptShowVis(const char*& p, int cmd)
{
	ScriptShowVisPet983(m_pPuppet, cmd == 0xd, p);
}

void IDummyPet::ParsePuppetScript(char* script, bool bSkip)
{
	ParseScript(script, bSkip);
}

void IDummyPet::RefreshFx983(bool bAll, int a, int b)
{
	if (bAll)
		m_pFx->DetachFxAll(-1);
	else
		m_pFx->DetachFx(m_fxId);
	m_fxId = m_pFx->ParseBoundingBoxScript_fx(m_pPuppet);
}

void IDummyPet::OnScriptSnd(const char*& p)
{
	static char sndName[0x40];
	p = GetStrConst(p, sndName);
	if (strlen(sndName) != 0 && strlen(sndName) < 64)
	{
		WVector pos = m_matrix.pivot - g_camera.pivot;
		g_audio->PlaySfx(sndName, &pos, 5, NULL, NULL, 0.5f, 200.0f);
	}
}

void IDummyPet::OnScriptHideClub(const char*& p)
{
	CSceneManager::Instance()->SetVisible("\xc5\xac\xb7\xb4", false);
}

void IDummyPet::ScriptProcess(w_motion_data* motion, float from, float to, bool bAll, bool bSkip)
{
	if (motion == NULL)
		return;
	if ((!(from <= motion->e) || from == to) && !bAll)
		return;

	int start, end;
	from *= 30.0f;
	if (bAll)
	{
		start = Ceil(from, 0.03f);
		end = start;
	}
	else
	{
		float limit = motion->e;
		if (limit > to)
			limit = to;
		to = limit * 30.0f;
		start = Ceil(from, 0.03f);
		end = Wabs((float)floor(to) - to) < g_EPSILON ? (int)to - 1 : (int)floor(to);

		if (start == (int)floor(motion->s * 30.0f + 0.5f))
			start++;

		if (start > end)
			return;
	}

	if (m_pPuppet->CheckFrameData(start, end, (int&)from, (int&)to))
	{
		for (int i = (int&)from; i <= (int&)to; i++)
			ParsePuppetScript(m_pPuppet->GetFrameData(i), bSkip);
	}
}

void IDummyPet::SetPosition(const WVector& pos)
{
	m_matrix.Reset();
	m_matrix.ya = g_vUp983_e42538;
	m_matrix.xa = WCrossProduct(m_matrix.ya, m_matrix.za);
	m_matrix.pivot = pos;
	m_pos = pos;
}

void IDummyPet::LookAt983(WVector pos)
{
	WVector dir;
	dir.x = pos.x - m_matrix.pivot.x;
	dir.y = 0;
	dir.z = pos.z - m_matrix.pivot.z;
	WVector front = m_matrix.za * -1.0f;
	float angle = CalcDeltaAngle(dir, front);
	if (Wabs(angle) > 0.0174532924f)
		m_matrix = RotMat(angle, 'y') * m_matrix;
}

bool IDummyPet::LoadClubUi983(IDummyPet* pPet, unsigned long id, bool bNoScene)
{
	if (pPet->m_kind != 2)
		return false;
	char* pItem = ITEMMGRPETJP()->Find1e0_983(id);
	if (pItem == NULL)
		return false;
	pPet->m_pPuppet = g_resrcmng->GetPuppet(MakeStr("%s_ui.pet", pItem + 0x49), false, true, false);
	if (pPet->m_pPuppet == NULL)
		return false;
	pPet->m_pPuppet->CalcBound(NULL, true);
	if (!bNoScene)
	{
		CSceneManagerJp::Instance983()->AddElement983(4, 0, 0, pPet->m_name.c_str(), 0x18, 0);
		CSceneManagerJp::Instance983()->SetPuppet(pPet->m_name.c_str(), pPet->m_pPuppet);
	}
	return true;
}

bool IDummyPet::LoadClub983(IDummyPet* pPet, unsigned long id, bool bNoScene)
{
	if (pPet->m_kind != 2)
		return false;
	char* pItem = ITEMMGRPETJP()->FindClub983(id);
	if (pItem == NULL)
		return false;
	pPet->m_pPuppet = g_resrcmng->GetPuppet(MakeStr("%s.pet", pItem + 0xc0), false, true, false);
	if (pPet->m_pPuppet == NULL)
		return false;
	pPet->m_pPuppet->CalcBound(NULL, true);
	if (!bNoScene)
	{
		CSceneManagerJp::Instance983()->AddElement983(4, 0, 0, pPet->m_name.c_str(), 0x18, 0);
		CSceneManagerJp::Instance983()->SetPuppet(pPet->m_name.c_str(), pPet->m_pPuppet);
	}
	return true;
}

void IDummyPet::Display()
{
	if (!m_bMoved || m_pPuppet == NULL)
		return;
	m_pPuppet->Transform(g_view, 1.0f, 0);
	m_pPuppet->UpdateBSphere();
	if (g_view->InFrustum(m_pPuppet->GetBoundSphere()))
	{
		m_pPuppet->UpdateLightSource(&g_lightset, false, NULL);
		WPuppet* pPet = m_pPuppet;
		WView* pView = g_view;
		pPet->Render(pView, true, 1.0f, pView->InFrustumSafe(pPet->GetBoundBox()), false, false);
	}
}

bool IDummyPet::SetMotionData983(const char* name)
{
	if (m_pPuppet == NULL || m_pPuppet->GetRootBone() == NULL)
		return false;
	m_pMotion = m_pPuppet->FindMotionData(name);
	if (m_pMotion == NULL)
	{
		m_bRotated = false;
		return false;
	}
	m_pPuppet->GetRootBone()->SetInvisible(true, false);
	m_motionTime = Min(m_pMotion->s, m_pMotion->e);
	ScriptProcess(m_pMotion, m_pMotion->s, m_pMotion->s, true, false);
	return true;
}

void IDummyPet::Play983(const char* name)
{
	m_5c = 3.14159274f;
	m_bMoved = true;
	m_bRotated = false;
	m_motionTime = 0;
	SetMotionData983(name);
	RefreshFx983(false, 0, 1);
}

bool IDummyPet::SetMotion(const char* name)
{
	m_5c = 3.14159274f;
	m_motionTime = 0;
	m_matrix.Reset();
	m_pTexPart = NULL;
	m_ac = 0;
	m_bMoved = false;
	m_bRotated = false;
	if (m_pFx == NULL)
		m_pFx = new FxInterpreter(NULL, 0);
	if (name)
		SetMotionData983(name);
	RefreshFx983(false, 0, 1);
	m_bInit = true;
	return true;
}

void IDummyPet::Process(float dt)
{
	if (!m_bRotated || m_pMotion == NULL)
		return;
	m_motionTime += dt;
	ScriptProcess(m_pMotion, m_motionTime - dt, m_motionTime, false, false);
	if (m_motionTime > m_pMotion->e)
	{
		if (m_pMotion->s != m_pMotion->e)
			SetMotionData983(m_pMotion->nextmotion);
		else
			m_bRotated = false;
		return;
	}
	((WPuppetJp*)m_pPuppet)->SetMatrix983(m_matrix);
	((WPuppetJp*)m_pPuppet)->Animate983(NULL, NULL, m_motionTime, 0, 0, 0);
	m_pPuppet->UpdateBound(true, true);
}

bool IDummyPet::LoadMascot983(IDummyPet* pPet, unsigned long id, bool bNoScene)
{
	if (pPet->m_kind != 5)
		return false;
	char* pItem = ITEMMGRPETJP()->FindMascot983(id);
	if (pItem == NULL)
		return false;
	pPet->m_pPuppet = g_resrcmng->GetPuppet(MakeStr("%s.pet", pItem + 0xc0), false, true, false);
	if (pPet->m_pPuppet == NULL)
		return false;
	int index = 0;
	for (std::map<unsigned int, sMascotInfo>::const_iterator it = MASCOTMAP983().begin();
		it != MASCOTMAP983().end(); ++it, ++index)
	{
		if (it->second.tid == id)
		{
			char* pMascot = ITEMMGRPETJP()->FindMascot983(id);
			if (pMascot)
			{
				if (pMascot[0x125])
					pPet->m_hTex = CMascot::ChangeMascotMsgInTex(it->second.szMsg,
						(const IFF_STRUCT::sMascot*)pMascot, pPet->m_pPuppet, MakeStr("ui%02d", index));
				break;
			}
		}
	}
	pPet->m_pPuppet->CalcBound(NULL, true);
	if (!bNoScene)
	{
		CSceneManagerJp::Instance983()->AddElement983(4, 0, 0, pPet->m_name.c_str(), 0x18, 0);
		CSceneManagerJp::Instance983()->SetPuppet(pPet->m_name.c_str(), pPet->m_pPuppet);
	}
	return true;
}

bool IDummyPet::ParsePTex983(w_motion_data* motion, std::string* pName, std::string* pTexture, const char*& p)
{
	char cond[64];
	char name[64];
	char texName[1024];
	cond[0] = 0;
	texName[0] = 0;
	p = GetStrConst(p, cond);
	if (strlen(cond) == 0 || strlen(cond) >= 64)
		return false;
	if (cond[0] == '@')
	{
		if (motion == NULL || strcmpi(cond + 1, motion->name) != 0)
			return false;
		p = GetStrConst(p, name);
		if (strlen(name) == 0 || strlen(name) >= 64)
			return false;
	}
	else
	{
		strcpy(name, cond);
	}
	p = GetStrConst(p, texName);
	if (strlen(texName) == 0 || strlen(texName) >= 1024)
		return false;
	CGDSkinItem* pSkinItem = CGDSkinItemPetJp::Instance983();
	std::string texture;
	pSkinItem->Replace(name, texName, &texture, (const sSkinParts983*)NULL);
	*pName = name;
	*pTexture = texture.c_str();
	return true;
}

void IDummyPet::ChangeTexture983(const char* name, const char* texture)
{
	std::list<sTexturePart*>::iterator it = std::find_if(m_texList.begin(), m_texList.end(), sFindTexPart(texture));
	if (it != m_texList.end())
	{
		if (*it != m_pTexPart)
			m_pTexPart = *it;
	}
	else
	{
		sTexturePart* pTexPart = new sTexturePart;
		strcpy(pTexPart->name, name);
		strcpy(pTexPart->texture, texture);
		m_texList.push_back(pTexPart);
		m_pTexPart = pTexPart;
	}
	if (m_pTexPart == NULL)
		return;
	if (m_kind == 0xc)
	{
		if (m_pPuppet == NULL)
			return;
		int hName = g_resrcmng->LoadTexture(name, 0, 0, NULL);
		if (hName == 0)
			return;
		int hTexture = g_resrcmng->LoadTexture(texture, 0, 0, NULL);
		if (hTexture == 0)
			return;
		m_pPuppet->ChangeTextureByHandle(hTexture, hName, m_pPuppet);
	}
	else
	{
		if (m_pPuppet == NULL)
			return;
		if (m_pTexPart->pBitmap == NULL)
			m_pPuppet->ChangeTexturePart(m_pTexPart->name, m_pTexPart->texture, &m_pTexPart->pBitmap, &m_pTexPart->rect);
		else
			m_pPuppet->ChangeTexturePart(m_pTexPart->name, m_pTexPart->pBitmap, m_pTexPart->rect);
	}
}

void IDummyPet::OnScriptPTex(const char*& p)
{
	std::string name;
	std::string texture;
	if (ParsePTex983(m_pMotion, &name, &texture, p))
		ChangeTexture983(name.c_str(), texture.c_str());
}

bool IDummyPet::Load(IDummyPet* pPet, unsigned long typeId, unsigned long skinId, bool bNoScene)
{
	if (pPet->m_kind != 0)
		return false;
	char* pCaddie = (char*)ITEMMGRLOBBYJP()->Find2a0_983(typeId);
	if (pCaddie == NULL)
		return false;
	if (skinId)
	{
		sItem983* pSkin = ITEMMGRLOBBYJP()->FindCadItem983(skinId);
		if (pSkin == NULL)
			return false;
		char* pBodyTex = (char*)pSkin + 0xe8;
		std::string file(pBodyTex);
		std::string lower;
		std::transform(file.begin(), file.end(), std::back_inserter(lower), tolower);
		int pos = (int)lower.find("_ex_");
		if (lower == "caddie_quma_ex_01.jpg")
			file = "quma_ex_02.pet";
		else if (lower == "caddie_dol_ex_01.jpg")
			file = "dol_ex_02.pet";
		else if (lower == "pipin3_bd_01-01.jpg")
			file = "pipin_ex_02.pet";
		else if (lower == "[2#caddie_cadie_004.jpg")
			file = "cadie_ex_02.pet";
		else if (lower == "[2#ho_body_001_02.jpg")
			file = "ho_mesh_ex02.pet";
		else if (lower == "[2#ho_body_002_04.jpg")
			file = "ho_mesh_ex03.pet";
		else if (lower == "caddie_maid_body_001.jpg")
		{
			if (CADDIE_OF_SKIN983(pSkin->typeId) == 0x1c00001f)
			{
				if ((pSkin->typeId & 0x1fff) < 3)
					file = "maid_1.pet";
				else
					file = "maid_13.pet";
			}
			else
			{
				if ((pSkin->typeId & 0x1fff) < 3)
					file = "maid.pet";
				else
					file = "maid_12.pet";
			}
		}
		else if (lower == "caddie_maid_body_002.jpg")
		{
			if (CADDIE_OF_SKIN983(pSkin->typeId) == 0x1c00001f)
			{
				if ((pSkin->typeId & 0x1fff) < 3)
					file = "maid2_1.pet";
				else
					file = "maid2_3.pet";
			}
			else
			{
				if ((pSkin->typeId & 0x1fff) < 3)
					file = "maid2.pet";
				else
					file = "maid2_2.pet";
			}
		}
		else if (lower == "caddie_maid_body_003.jpg")
		{
			if (CADDIE_OF_SKIN983(pSkin->typeId) == 0x1c00001f)
			{
				if ((pSkin->typeId & 0x1fff) < 3)
					file = "maid3_1.pet";
				else
					file = "maid3_3.pet";
			}
			else
			{
				if ((pSkin->typeId & 0x1fff) < 3)
					file = "maid3.pet";
				else
					file = "maid3_2.pet";
			}
		}
		else if (lower == "[2#cien_body-01.jpg")
		{
			switch (CADDIE_OF_SKIN983(pSkin->typeId))
			{
			case 0x1c000023:
				file = "Cien_Rare.pet";
				break;
			case 0x1c000022:
				file = "Cien_Secret.pet";
				break;
			case 0x1c000021:
				file = "Cien_Armor.pet";
				break;
			}
		}
		else if (lower == "[2#cien_body-02.jpg")
		{
			switch (CADDIE_OF_SKIN983(pSkin->typeId))
			{
			case 0x1c000023:
				file = "Cien_Rare_W.pet";
				break;
			case 0x1c000022:
				file = "Cien_Secret_W.pet";
				break;
			case 0x1c000021:
				file = "Cien_Armor_W.pet";
				break;
			}
		}
		else if (lower == "[2#cien_body-03.jpg")
		{
			if (CADDIE_OF_SKIN983(pSkin->typeId) == 0x1c000023)
				file = "Cien_Rare_S.pet";
		}
		else if (lower == "[2#cien_body-04.jpg")
		{
			switch (CADDIE_OF_SKIN983(pSkin->typeId))
			{
			case 0x1c000022:
				file = "Cien_Secret_WS.pet";
				break;
			case 0x1c000021:
				file = "Cien_Armor_WS.pet";
				break;
			}
		}
		else if (lower == "[2#cien_elemental_01.jpg")
		{
			switch (CADDIE_OF_SKIN983(pSkin->typeId))
			{
			case 0x1c000023:
				file = "Cien_Rare_e.pet";
				break;
			case 0x1c000022:
				file = "Cien_Secret_e.pet";
				break;
			case 0x1c000021:
				file = "Cien_Armor_e.pet";
				break;
			}
		}
		else if (lower == "[2#karen_003.jpg")
		{
			switch (CADDIE_OF_SKIN983(pSkin->typeId))
			{
			case 0x1c000025:
			case 0x1c000029:
				file = "karen.pet";
				break;
			case 0x1c000027:
			case 0x1c00002b:
				file = "karen_ex_n.pet";
				break;
			case 0x1c000026:
			case 0x1c000028:
			case 0x1c00002a:
			case 0x1c00002c:
				file = "karen_r_n.pet";
				break;
			}
		}
		else if (lower == "2#karen_p_001.jpg" || lower == "2#karen_r_p_001.jpg" || lower == "2#karen_ex_p_001.jpg")
		{
			switch (CADDIE_OF_SKIN983(pSkin->typeId))
			{
			case 0x1c000025:
			case 0x1c000029:
				file = "karen_p.pet";
				break;
			case 0x1c000027:
			case 0x1c00002b:
				file = "karen_ex_p.pet";
				break;
			case 0x1c000026:
			case 0x1c000028:
			case 0x1c00002a:
			case 0x1c00002c:
				file = "karen_r_p.pet";
				break;
			}
		}
		else if (pos != -1)
		{
			// "..._ex_NN.jpg": o .pet é o trecho da 1ª letra minúscula até o "_ex"
			size_t i;
			for (i = 0; i < lower.size(); ++i)
			{
				char c = lower[i];
				if (c >= 'a' && c <= 'z')
					break;
			}
			file.assign(lower.begin() + i, lower.begin() + pos + 3);
			file += ".pet";
			pPet->m_pPuppet = g_resrcmng->GetPuppet(file.c_str(), false, true, false);
			goto loaded;
		}
		else
		{
			pPet->m_pPuppet = g_resrcmng->GetPuppet(MakeStr("%s_ex.pet", pCaddie + 0xc4), false, true, false);
			goto loaded;
		}
		pPet->m_pPuppet = g_resrcmng->GetPuppet(file.c_str(), false, true, false);
loaded:
		if (pPet->m_pPuppet == NULL)
			return false;
		if (strlen((char*)pSkin + 0xc0) != 0)
			pPet->ChangeTexture983("face", (char*)pSkin + 0xc0);
		if (strlen(pBodyTex) != 0)
			pPet->ChangeTexture983("body", pBodyTex);
	}
	else
	{
		pPet->m_pPuppet = g_resrcmng->GetPuppet(MakeStr("%s.pet", pCaddie + 0xc4), false, true, false);
	}
	pPet->m_pPuppet->CalcBound(NULL, true);
	if (!bNoScene)
	{
		CSceneManagerJp::Instance983()->AddElement983(4, 0, 0, pPet->m_name.c_str(), 0x18, 0);
		CSceneManagerJp::Instance983()->SetPuppet(pPet->m_name.c_str(), pPet->m_pPuppet);
	}
	return true;
}

// layout medido no 983: `new` de 0xc4 em 0056128c, `new` de 0x94 em 004d3a95
C_ASSERT_VC10(sizeof(IDummyPet) == 0xc4);
C_ASSERT_VC10(offsetof(IDummyPet, m_matrix) == 0x2c);
C_ASSERT_VC10(offsetof(IDummyPet, m_pMotion) == 0x94);
C_ASSERT_VC10(offsetof(IDummyPet, m_texList) == 0x9c);
C_ASSERT_VC10(offsetof(IDummyPet, m_pTexPart) == 0xa8);
C_ASSERT_VC10(offsetof(IDummyPet, m_pFx) == 0xb4);
C_ASSERT(sizeof(sTexturePart) == 0x94);
