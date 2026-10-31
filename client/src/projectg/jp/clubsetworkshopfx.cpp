// JP 983: unidade 0049c120–0049da20 (ClubSetWorkshopFx983, efeitos e sons da oficina de tacos). Sem RTTI, do 839/983, sem
// par no 645; ver include/jp/clubsetworkshopfx.h. Sessão B87 (F3.7.9, 12º lote). Só a classe: 0049c120/0049c380 (newRandomBox2,
// club_init_item_a_anim*) ficam com a unidade da ResetProgress e 0049c430–0049cb30 (atributos do taco pelo CItemManager)
// com outro arquivo. O compilador gera: 0049ccf0/0049cd10 e 0049cce0/0049cd00 (operator[] e size() dos dois tr1::array),
// 0049cb90 (strrchr do C++), 0049cba0 (CFxSpray::Pos), 0049cd30 (WMatrix::operator=), 0049d120/0049d370 (leituras do
// lua_tinker), 0049d550 (`delete` do sWorkshopFx983).
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "wresrcmng.h"
#include "wview.h"
#include "fxbox.h"
#include "fx.h"
#include "soundmanager.h"
#include "lua983.h"
#include "clubsetworkshopfx.h"
#include "fresh.h"
#include "clubsetstat.h"

extern Fresh* g_pFresh; // 00f02340
extern const WRect g_rcZero983; // 00f14668 (0, 0, 0, 0)
extern const WVector g_vZero983_f041c4; // 00f041c4

// CFxBox::Finished (0096d7f0) só é visível na fxbox.cpp: visão com o nome no jp.tsv
class CFxBoxJp983 : public CFxBox
{
public:
	bool Finished();
};

ClubSetWorkshopFx983* ClubSetWorkshopFx983::s_pInstance = NULL;


// 00498550
void ClubSetWorkshopFx983::sSlot983::Clear983()
{
	C_ASSERT_VC10(offsetof(sSlot983, statMax) == 0x32 && offsetof(sSlot983, nextStatMax) == 0x3c && offsetof(sSlot983, clubType) == 0x48);
	guid = 0;
	tid = 0;
	name.clear();
	pBmp = NULL;
	for (int i = 0; i < 5; i++)
	{
		stat[i] = 0;
		statMax[i] = 0;
		nextStatMax[i] = 0;
	}
	clubType = -1;
	level = -1;
	rank = -1;
	typeId = -1;
	workshopParam = -1;
	exp = 0;
	recoveryLeft = 0;
	recoveryMax = 0;
	if (pPreview)
	{
		delete pPreview;
		pPreview = 0;
	}
}

// 004986c0
ClubSetWorkshopFx983::sSlot983::sSlot983()
{
	pPreview = 0;
	Clear983();
}

// 00498460
ClubSetWorkshopFx983::sSlot983::~sSlot983()
{
	Clear983();
}

// 00498100: so o ramo com taco e IFF (o ramo sem eles so recalcula grau a partir dos slots ja gravados)
void ClubSetWorkshopFx983::sSlot983::Recalc983(sItemInfo* pItem, IFF_STRUCT::sClubSet* pClub)
{
	int sum = 0;
	if (!pItem || !pClub)
	{
		for (int i = 0; i < 5; i++)
			sum += stat[i];
		rank = (sum >= 30 && sum <= 59) ? (sum - 30) / 5 : -1;
		level = (sum >= 30 && sum <= 59) ? (sum - 30) % 5 : -1;
	}
	else
	{
		for (int i = 0; i < 5; i++)
		{
			stat[i] = GetClubSlotMax983(i, pClub, pItem, false);
			sum += stat[i];
		}
		clubType = pClub->nWorkshopType983;
		rank = (sum >= 30 && sum <= 59) ? (sum - 30) / 5 : -1;
		level = (rank == 5 || rank == -1) ? -1 : (sum - 30) % 5;
		typeId = pClub->nWorkshopTypeId983;
		workshopParam = pClub->nWorkshopParam983;
	}
	short* mx = statMax;
	CItemManagerStatJp* pMgr = (CItemManagerStatJp*)ItemManager();
	pMgr->GetClubRankBonus983(clubType, rank, &mx[0], &mx[1], &mx[2], &mx[3], &mx[4]);
	for (int i = 0; i < 5; i++)
	{
		if (mx[i] < stat[i])
			mx[i] = stat[i];
	}
	if (rank != 5)
	{
		short* nx = nextStatMax;
		pMgr->GetClubRankBonus983(clubType, rank + 1, &nx[0], &nx[1], &nx[2], &nx[3], &nx[4]);
	}
}

// 00498610 (guid 0 = refaz com o taco atual; nao achou: limpa)
bool ClubSetWorkshopFx983::sSlot983::SetClub983(unsigned long guid)
{
	if (guid == 0)
		guid = this->guid;
	sItemInfo* pItem = FindMyClubSet983(guid);
	if (pItem)
	{
		IFF_STRUCT::sClubSet* pClub = FindClubSetIff983(pItem->tid);
		if (pClub)
		{
			this->guid = guid;
			tid = (int)pItem->tid;
			name = pClub->c.Name; // +0x8
			pBmp = (Bitmap*)g_pFresh->GetBitmap(pClub->c.Icon); // +0x49
			Recalc983(pItem, pClub);
			exp = pItem->clubExp983;
			recoveryMax = pClub->nRecoveryMax983; // +0xec
			recoveryLeft = recoveryMax - pItem->recoveryUsed983;
			return true;
		}
	}
	Clear983();
	return false;
}

// 0049cbb0
void ClubSetWorkshopFx983::ResetSlots983()
{
	for (int i = 0; i < 5; i++)
	{
		if (m_slot[i])
			m_slot[i]->SetClub983(0);
	}
	m_cur = 5;
}

// 0049cbe0
bool ClubSetWorkshopFx983::SetSlotClubM983(int slot, unsigned long guid)
{
	if (slot != -1)
		return m_slot[slot]->SetClub983(guid);
	return false;
}

// 0049cc10
void sWorkshopFx983::Process983(const float& deltaTime)
{
	if (m_pFxBox)
	{
		m_pFxBox->ProcessAttacher(deltaTime * 1000.0);
		m_pFxBox->Process(deltaTime * 1000.0);
	}
}

// 0049cc60
void sWorkshopFx983::Stop983()
{
	m_pFxBox->ClearAll(false);
	m_pSpray = NULL;
	m_pSeq = NULL;
}

// 0049cc80
bool sWorkshopFx983::IsEnd983()
{
	if (m_pFxBox)
		return ((CFxBoxJp983*)m_pFxBox)->Finished();
	return true;
}

// 0049cc90
void sWorkshopFx983::SetRect983(const WRect& rc)
{
	if (m_pFxBox)
		m_pFxBox->SetScreenCenter(rc.x, rc.y);
	m_rc = rc;
}

// 0049cd90
ClubSetWorkshopFx983::ClubSetWorkshopFx983()
{
	s_pInstance = this;
	for (int i = 0; i < 5; i++)
		m_slot[i] = new sSlot983;
	for (int i = 0; i < 8; i++)
		m_sound[i] = NULL;
	for (int i = 0; i < 5; i++)
		m_fx[i] = NULL;
	ResetSlots983();
}

// 0049ce40
void ClubSetWorkshopFx983::Proc983(const float& deltaTime)
{
	for (int i = 0; i < 5; i++)
	{
		if (m_fx[i])
			m_fx[i]->Process983(deltaTime);
	}
}

// 0049ce70
void ClubSetWorkshopFx983::PlaySound983(unsigned int index)
{
	if (index < m_sound.size() && m_sound[index])
		g_audio->PlaySfx(m_sound[index], NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 0049cec0
void ClubSetWorkshopFx983::StopFxM983(unsigned int index)
{
	if (index < m_fx.size() && m_fx[index])
		m_fx[index]->Stop983();
}

// 0049cee0
bool ClubSetWorkshopFx983::IsFxEndM983(unsigned int index)
{
	if (index < m_fx.size() && m_fx[index])
		return m_fx[index]->IsEnd983();
	return true;
}

// 0049cf10
void ClubSetWorkshopFx983::DeleteSlots983()
{
	for (int i = 0; i < 5; i++)
	{
		if (m_slot[i])
		{
			delete m_slot[i];
			m_slot[i] = NULL;
		}
	}
	ResetSlots983();
}

// 0049cf50
void sWorkshopFx983::SetRectPtr983(const WRect* pRect)
{
	SetRect983(*pRect);
}

// 0049cf60: câmera, campo de visão e corte próprios; devolve os da tela
void sWorkshopFx983::Draw983()
{
	if (m_pFxBox && !IsEnd983())
	{
		bool bFog = g_view->SetFogEnable(false);
		g_view->SetCamera(m_camera);
		float oldNear = g_view->clip_near;
		WView::PROJECTION_MODE mode = g_view->GetProjectionMode();
		float oldFar = g_view->clip_far;
		float oldFov = g_view->GetFOV();
		g_view->SetProjectionMode((WView::PROJECTION_MODE)0);
		g_view->SetFOV(1.5707964f);
		g_view->SetClip(0.5f, 50.0f, false);
		g_view->SetClippingArea(m_rc);
		g_view->UpdateCamera();
		m_pFxBox->Display(g_view);
		g_view->Flush(1, 1);
		g_view->SetProjectionMode(mode);
		g_view->SetClip(oldNear, oldFar, false);
		g_view->SetFOV(oldFov);
		WRect rc;
		rc.x = 0.0f;
		rc.y = 0.0f;
		rc.w = g_view->SCREEN_XS;
		rc.h = g_view->SCREEN_YS;
		g_view->SetClippingArea(rc);
		g_view->SetScreenCenter(g_view->SCREEN_XS * 0.5, g_view->SCREEN_YS * 0.5);
		g_view->UpdateCamera();
		g_view->SetFogEnable(bFog);
	}
}

// 004ce4c0..004ce740: acessos estaticos (repassam ao s_pInstance; sem ele devolvem o neutro)
bool __fastcall ClubSetWorkshopFx983::SetSlotClub983(int slot, unsigned long guid)
{
	return s_pInstance ? s_pInstance->SetSlotClubM983(slot, guid) : false;
}

ClubSetWorkshopFx983::sSlot983* __fastcall ClubSetWorkshopFx983::GetSlot983(int slot)
{
	if (s_pInstance && s_pInstance->m_slot[slot] && s_pInstance->m_slot[slot]->guid)
		return s_pInstance->m_slot[slot];
	return NULL;
}

int __fastcall ClubSetWorkshopFx983::GetSlotLevel983(int slot)
{
	sSlot983* p = GetSlot983(slot);
	return p ? p->level : -1;
}

int __fastcall ClubSetWorkshopFx983::GetSlotRank983(int slot)
{
	sSlot983* p = GetSlot983(slot);
	return p ? p->rank : -1;
}

int __fastcall ClubSetWorkshopFx983::GetSlotCancelCount983(int slot)
{
	sSlot983* p = GetSlot983(slot);
	return p ? p->recoveryLeft : 0;
}

int __fastcall ClubSetWorkshopFx983::GetSlotCancelMax983(int slot)
{
	sSlot983* p = GetSlot983(slot);
	return p ? p->recoveryMax : 0;
}

int __fastcall ClubSetWorkshopFx983::GetSlotStat983(int slot, int stat)
{
	sSlot983* p = GetSlot983(slot);
	if (p && stat != -1)
		return p->stat[stat];
	return 0;
}

// 00497e70
int ClubSetWorkshopFx983::sSlot983::GetStatMax983(int st, bool b)
{
	if (st == -1)
		return 0;
	if (b && rank < 4)
		return nextStatMax[st];
	return statMax[st];
}

int __fastcall ClubSetWorkshopFx983::GetSlotStatMax983(int slot, int stat, bool b)
{
	sSlot983* p = GetSlot983(slot);
	return p ? p->GetStatMax983(stat, b) : 0;
}

int __fastcall ClubSetWorkshopFx983::GetSlotExp983(int slot)
{
	sSlot983* p = GetSlot983(slot);
	return p ? p->exp : 0;
}

int __fastcall ClubSetWorkshopFx983::GetSlotTypeId983(int slot)
{
	sSlot983* p = GetSlot983(slot);
	return p ? p->typeId : -1;
}

int __fastcall ClubSetWorkshopFx983::GetSlotField58_983(int slot)
{
	sSlot983* p = GetSlot983(slot);
	return p ? p->workshopParam : -1;
}

bool __fastcall ClubSetWorkshopFx983::ClearSlot983(int slot)
{
	sSlot983* p = GetSlot983(slot);
	if (!p)
		return false;
	p->SetClub983(0);
	return true;
}

bool __fastcall ClubSetWorkshopFx983::ResetSlot983(int slot)
{
	sSlot983* p = GetSlot983(slot);
	if (!p)
		return false;
	p->Clear983();
	p->SetClub983(0);
	return true;
}

// 00498720: previa do slot com +1 no atributo escolhido (b); cria o pPreview na primeira vez
ClubSetWorkshopFx983::sSlot983* ClubSetWorkshopFx983::sSlot983::GetPreview983(int st, bool b)
{
	if (st == -1)
		return NULL;
	if (!pPreview)
		pPreview = new sSlot983;
	sSlot983* pv = pPreview;
	if (!pv)
		return NULL;
	pv->SetClub983(guid);
	if (b)
		pv->stat[st] += 1;
	pv->Recalc983(NULL, NULL);
	return pv;
}

ClubSetWorkshopFx983::sSlot983* __fastcall ClubSetWorkshopFx983::GetSlotPreview983(int slot, int stat, bool b)
{
	sSlot983* p = GetSlot983(slot);
	return p ? p->GetPreview983(stat, b) : NULL;
}

bool __fastcall ClubSetWorkshopFx983::StartFx983(int index)
{
	if (!s_pInstance)
		return false;
	s_pInstance->PlaySound983(index);
	return true;
}

bool __fastcall ClubSetWorkshopFx983::SetFxRect983(int index, const WRect* pRect)
{
	if (!s_pInstance)
		return false;
	s_pInstance->SetFxRectM983(index, pRect);
	return true;
}

bool __fastcall ClubSetWorkshopFx983::HoldFx983(int index)
{
	if (!s_pInstance)
		return false;
	s_pInstance->PlayFxM983(index);
	return true;
}

bool __fastcall ClubSetWorkshopFx983::DrawFx983(int index)
{
	if (!s_pInstance)
		return false;
	s_pInstance->DrawFxM983(index);
	return true;
}

bool __fastcall ClubSetWorkshopFx983::StopFx983(int index)
{
	if (!s_pInstance)
		return false;
	s_pInstance->StopFxM983(index);
	return true;
}

bool __fastcall ClubSetWorkshopFx983::IsFxEnd983(int index)
{
	if (s_pInstance)
		s_pInstance->IsFxEndM983(index);
	return false; // 983 004ce752: xor al, al sempre; true aqui fechava o resultado sem 0x165/0x166 (E473)
}

// 0049d160
ClubSetWorkshopFx983* ClubSetWorkshopFx983::Create983()
{
	return new ClubSetWorkshopFx983;
}

// 0049d1d0
void ClubSetWorkshopFx983::SetFxRectM983(unsigned int index, const WRect* pRect)
{
	if (index < m_fx.size() && m_fx[index])
		m_fx[index]->SetRectPtr983(pRect);
}

// 0049d1f0
void ClubSetWorkshopFx983::DrawFxM983(unsigned int index)
{
	if (index < m_fx.size() && m_fx[index])
		m_fx[index]->Draw983();
}

// 0049d210
void sWorkshopFx983::Play983()
{
	if (m_bSpr)
	{
		if (!m_pSpray)
		{
			m_pSpray = m_pFxBox->LoadSpray(m_name.c_str(), NULL);
			if (m_pSpray)
			{
				CFxSpray* p = m_pSpray;
				p->m_attachX = (float)(int)(float)(m_rc.x + m_rc.w * 0.5);
				m_pSpray->m_attachY = (float)(int)(float)(m_rc.y + m_rc.h * 0.5);
				m_pSpray->m_pos = g_vZero983_f041c4;
				m_pFxBox->AddSpray(m_pSpray);
			}
		}
	}
	else if (!m_pSeq)
	{
		m_pSeq = m_pFxBox->LoadSequence(m_name.c_str(), false, NULL);
		if (m_pSeq)
		{
			CFxSequence* p = m_pSeq;
			p->m_attachX = (float)(int)(float)(m_rc.x + m_rc.w * 0.5);
			m_pSeq->m_attachY = (float)(int)(float)(m_rc.y + m_rc.h * 0.5);
			m_pSeq->m_pos = g_vZero983_f041c4;
			m_pFxBox->AddSequence(m_pSeq);
		}
	}
}

// 0049d3c0: para o efeito da vez e toca o pedido
void ClubSetWorkshopFx983::PlayFxM983(unsigned int index)
{
	if (m_cur < 5 && m_fx[m_cur])
		m_fx[m_cur]->Stop983();
	if (index < m_fx.size() && m_fx[index])
		m_fx[index]->Play983();
}

// 0049d400
sWorkshopFx983::~sWorkshopFx983()
{
	if (m_pFxBox)
	{
		m_pFxBox->ClearAll(false);
		if (m_pFxBox)
		{
			delete m_pFxBox;
			m_pFxBox = NULL;
		}
	}
}

// 0049d580
ClubSetWorkshopFx983::~ClubSetWorkshopFx983()
{
	s_pInstance = NULL;
	for (int i = 0; i < 5; i++)
	{
		if (m_fx[i])
		{
			delete m_fx[i];
			m_fx[i] = NULL;
		}
	}
	DeleteSlots983();
}

// 0049d5e0
sWorkshopFx983::sWorkshopFx983(const char* filename)
{
	m_camera = WMatrix::IDENTITY;
	m_pFxBox = new CPvsFxBox;
	m_pFxBox->Init(g_resrcmng);
	m_pFxBox->SetCamera(&g_camera);
	m_pFxBox->m_frameTime = 33.333332f;
	float scale = g_view->SCREEN_YS / 600.0;
	float dist = scale * -30.0;
	m_camera.pivot = m_camera.za * dist;
	m_rc = g_rcZero983;
	m_name = filename;
	m_pSpray = NULL;
	m_pSeq = NULL;
	m_bSpr = false;
	const char* ext = strrchr(filename, '.');
	if (!ext || strlen(ext) < 4)
		return;
	if (!_stricmp(ext + 1, "seq"))
	{
		m_bSpr = false;
		return;
	}
	if (!_stricmp(ext + 1, "spr"))
		m_bSpr = true;
}

// 0049d7e0
void ClubSetWorkshopFx983::Load983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("clubset_workshop.lua");
	lua_system::lua_table_helper tbl = script->GetTable("clubset_workshop_ui_main");
	m_sound[0] = LUA983_GET(tbl, const char*, "sound_levelup_try");
	m_sound[1] = LUA983_GET(tbl, const char*, "sound_levelup_result");
	m_sound[2] = LUA983_GET(tbl, const char*, "sound_levelup_complete");
	m_sound[3] = LUA983_GET(tbl, const char*, "sound_rankup_try");
	m_sound[4] = LUA983_GET(tbl, const char*, "sound_rankup_result");
	m_sound[5] = LUA983_GET(tbl, const char*, "sound_rankup_complete");
	m_sound[6] = LUA983_GET(tbl, const char*, "sound_clubchange");
	m_sound[7] = LUA983_GET(tbl, const char*, "sound_clubmanage");
	m_fx[0] = new sWorkshopFx983(LUA983_GET(tbl, char*, "fx_levelup_filename"));
	m_fx[1] = new sWorkshopFx983(LUA983_GET(tbl, char*, "fx_rankup_filename"));
	m_fx[2] = new sWorkshopFx983(LUA983_GET(tbl, char*, "fx_clubchange_filename"));
	m_fx[3] = new sWorkshopFx983(LUA983_GET(tbl, char*, "fx_clubmanage_filename"));
	m_fx[4] = new sWorkshopFx983(LUA983_GET(tbl, char*, "fx_clubmanage_filename"));
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script); // D608: 983 c1f510 (adiado), nao c1fe30
}

// 0049da00
void ClubSetWorkshopFx983::Init983()
{
	Load983();
}

C_ASSERT_VC10(sizeof(sWorkshopFx983) == 0x6c);
C_ASSERT(sizeof(ClubSetWorkshopFx983) == 0x4c);
C_ASSERT_VC10(sizeof(ClubSetWorkshopFx983::sSlot983) == 0x6c);
