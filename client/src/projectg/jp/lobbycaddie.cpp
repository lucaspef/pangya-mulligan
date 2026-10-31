// JP 983: LobbyCaddie (0055fee0–00561b80: construtor 00561180, destrutor 00560c90, criador 00561600, registro 00561670,
// OnPreLoadInit 005617b0, OnProcess 00560d70, OnDisplay 00560290, OnDestroy 005616a0, HandleMsg 00561820 e 11 auxiliares).
// Fonte reconstruída do 983 (classe sem par no 645; par no KR 839). Sessão B28.
#include "minatl.h"
#include "projectg.h"
#include "lobbycaddie.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "inputmanager.h"
#include "polysoup.h"
#include "commonutil.h"
#include "wpuppet.h"
#include "wresrcmng.h"
#include "fxinterpreter.h"

// JP 983: objeto apontado por CSharedDoc+0x11024 (m_pGolfDoc; A729: membro, cl novo 0x10cc4); o WPolySoup do cenário está em +8
struct sLobbyScene983
{
	char m_00[8];
	WPolySoup* m_pSoup; // +0x08
};
#define LOBBYSOUP983() (((sLobbyScene983*)::Doc()->m_pGolfDoc)->m_pSoup)

extern const WVector g_vZero983_f041c4; // 00f041c4

IMPLEMENT_ACTOR(LobbyCaddie, IActor)

LobbyCaddie::LobbyCaddie()
	: m_pet((IDummyPet*)NULL),
	  m_pos(g_vZero983_f041c4),
	  m_bPlace(true),
	  m_typeId(0),
	  m_caddieId(0),
	  m_bVisible(false),
	  m_bFading(false),
	  m_fadeTime(0),
	  m_bLocked(false),
	  m_bDrag(false),
	  m_48(0)
{
	InitializeCriticalSection(&m_csPet);
}

LobbyCaddie::~LobbyCaddie()
{
	DeleteCriticalSection(&m_csPet);
}

void LobbyCaddie::OnAnotherHand(int key, void* param)
{
	m_loader();
	m_bLoaded = true;
}

void LobbyCaddie::RequestLoad983(unsigned long typeId, unsigned long caddieId)
{
	if (m_hand.IsWorking(0))
		return;
	m_reqCaddieId = caddieId;
	m_reqTypeId = typeId;
	m_bLoaded = false;
	m_bRequest = true;
}

void LobbyCaddie::OnDisplay()
{
	if (m_pet)
		m_pet->Display();
}

void LobbyCaddie::SetPoint983(const char* name)
{
	WPolySoup::sPoint* pPoint = LOBBYSOUP983()->FindPoint(name);
	if (pPoint)
		m_pos = pPoint->pos;
}

void LobbyCaddie::SetVisible983(bool bVisible, bool bFade)
{
	if (m_bVisible == bVisible)
		return;
	m_bFading = bFade;
	if (!bFade)
	{
		if (m_pet && m_pet->m_pPuppet)
		{
			m_pet->m_pPuppet->SetAlpha(bVisible ? 0xff : 0);
			m_bVisible = bVisible;
			m_bFading = false;
		}
	}
	else
	{
		m_fadeTime = 0;
	}
}

void LobbyCaddie::Rotate983(float angle)
{
	IDummyPet* pPet = m_pet.get();
	if (pPet)
	{
		WMatrix rot;
		RotMat(angle, 1, rot);
		m_pet->m_matrix = rot * pPet->m_matrix;
	}
}

// 0055fe60–0055fea0: grupos de telas do lobby (B163: faltavam; o robô b163-4 as alcançou no lobby)
bool CLobbyState983::IsGroupB983() const
{
	return m_state == 5 || m_state == 6 || m_state == 7;
}

bool CLobbyState983::IsGroupA983() const
{
	return m_state == 2 || m_state == 3 || m_state == 4;
}

bool CLobbyState983::IsPickable983() const
{
	return m_state == 3 || m_state == 6;
}

bool LobbyCaddie::Fade983(float dt)
{
	if (!m_pet || !m_pet->m_pPuppet)
		return false;
	float from = 0;
	float to = 255.0f;
	if (m_bVisible)
	{
		from = 255.0f;
		to = 0;
	}
	float alpha = ::Fade983(from, to, m_fadeTime, 0.7f);
	m_fadeTime += dt;
	if (m_bVisible)
	{
		if (to >= alpha)
		{
			m_bVisible = false;
			m_bFading = false;
			alpha = to;
		}
	}
	else if (to <= alpha)
	{
		m_bVisible = true;
		m_bFading = false;
		alpha = to;
	}
	m_pet->m_pPuppet->SetAlpha((unsigned char)(int)alpha);
	return m_bVisible;
}

bool LobbyCaddie::Input983(float dt)
{
	if (!g_bFocus)
		return false;
	unsigned int buttons = (CUiPointer983::Instance983() ? CUiPointer983::Instance983()->GetButtons983() : NULL)->Get983();
	unsigned int released = buttons & 2;
	if (released)
		m_bDrag = false;
	if (m_bLocked || !m_pet)
		return false;
	if (!CUiPointer983::Instance983() || !CUiPointer983::Instance983()->GetButtons983() || !g_pF02340_983 || !FRESHMGR983())
		return false;
	CLobbyState983* pState = CLobbyState983::Instance983();
	if (pState && (pState->IsGroupA983() || pState->IsGroupB983()) && !pState->IsPickable983())
		return false;

	WVector mouse = *CUiPointer983::Instance983()->GetPos983();
	WVector delta = CInputManager::Instance()->GetMouseDelta();
	if (CUiPointer983::Instance983()->IsOverWnd983(mouse.x, mouse.y))
	{
		if (!CLobbyState983::Instance983()->m_bCursorBusy)
			CUiPointer983::Instance983()->SetCursor983(0, 0);
		return false;
	}
	if (!m_bDrag && !IsMouseOverPet983(mouse, m_pos))
	{
		if (!CLobbyState983::Instance983()->m_bCursorBusy)
			CUiPointer983::Instance983()->SetCursor983(0, 0);
		return false;
	}

	if (buttons & 1)
		m_bDrag = true;
	if (m_bDrag)
	{
		CUiPointer983::Instance983()->SetCursor983(0xe, 1);
		if (buttons & 0x100)
		{
			static WVector s_last(0, 0, 0);
			float move = (s_last.x + delta.x) * 0.5;
			s_last = delta;
			move = move * 0.007000000216066837;
			Rotate983(move);
		}
		return true;
	}
	if (released)
	{
		m_bDrag = false;
		return true;
	}
	// clique: toca o movimento "\xb1\xe2\xba\xbb" e volta para o lugar, de frente
	if ((buttons & 4) && m_pet->SetMotion("\xb1\xe2\xba\xbb"))
	{
		m_pet->SetPosition(m_pos);
		m_pet->m_bMoved = true;
		m_pet->m_bRotated = true;
		Rotate983(0.6f);
	}
	CUiPointer983::Instance983()->SetCursor983(1, 1);
	return true;
}

// põe o boneco recém-carregado na cena: movimento de espera, efeito de aparecer, posição, ângulo e elemento da cena
bool LobbyCaddie::Place983()
{
	if (!m_pet || !m_pet->SetMotion("\xb1\xe2\xb4\xd9\xb8\xae\xb1\xe2"))
		return false;
	if (CLobbyState983::Instance983() && !CLobbyState983::Instance983()->IsGroupA983())
		m_pet->m_pFx->PlayFx983(g_lobbyAppearFx983, &m_pos);
	m_pet->SetPosition(m_pos);
	m_pet->m_bMoved = true;
	m_pet->m_bRotated = true;
	m_fadeTime = 0;
	m_bVisible = true;
	m_bFading = false;
	Rotate983(0.6f);
	CSceneManagerJp::Instance983()->AddElement983(4, 0, 0, m_pet->m_name.c_str(), 0x18, 0);
	CSceneManagerJp::Instance983()->SetPuppet(m_pet->m_name.c_str(), m_pet->m_pPuppet);
	return true;
}

void LobbyCaddie::OnProcess(float delta)
{
	if (m_bRequest == true)
	{
		m_bRequest = false;
		if (!m_hand.IsWorking(0))
			m_hand.Work(this, 0, NULL, true, -1);
	}
	if (m_pet && m_bPlace)
	{
		Place983();
		m_bPlace = false;
	}
	if (m_pet)
	{
		m_pet->Process(delta);
		bool bOver = Input983(delta);
		if (CLobbyState983::Instance983())
			CLobbyState983::Instance983()->m_bOverPet = bOver;
	}
	if (m_bFading)
		Fade983(delta);
}

// cria o boneco do caddie `caddieId` (0 ou -1 = o caddie em uso) ou, sem caddie, o do item `typeId`
bool LobbyCaddie::Load983(unsigned long caddieId, unsigned long typeId)
{
	if (m_pet)
		return false;
	IDummyPet* pPet = new IDummyPet(0);
	if (!pPet)
		return false;
	if (caddieId == (unsigned long)-1)
		caddieId = 0;
	if (caddieId == 0 && typeId == 0)
		caddieId = DOCLOBBYJP()->GetCaddieId983();

	sCaddie983* pCaddie = (sCaddie983*)Doc()->FindCaddie983(caddieId);
	if (pCaddie)
	{
		pPet->m_name = MakeStr("LOBBY_CADDIE_%d_%d", DOCLOBBYJP()->GetUid983(), pCaddie->typeId);
		if (IDummyPet::Load(pPet, pCaddie->typeId, pCaddie->skinId, true))
		{
			m_caddieId = caddieId;
			m_typeId = pCaddie->typeId;
			goto loaded;
		}
	}
	else if (typeId)
	{
		pPet->m_name = MakeStr("LOBBY_CADDIE_%d_%d", DOCLOBBYJP()->GetUid983(), typeId);
		unsigned long skinId;
		if (ITEMMGRLOBBYJP()->Find2a0_983(typeId))
		{
			skinId = caddieId; // 983: o próprio argumento (0, salvo caddie pedido e não achado)
		}
		else
		{
			sItem983* pSkin = ITEMMGRLOBBYJP()->FindCadItem983(typeId);
			sItem983* pBase = ITEMMGRLOBBYJP()->Find2a0_983((((typeId >> 0x15) & 0x1f) + ((typeId >> 0x10) & 0x1f)) | 0x1c000000);
			typeId = pBase->typeId;
			skinId = pSkin->typeId;
		}
		if (IDummyPet::Load(pPet, typeId, skinId, true))
		{
			m_typeId = typeId;
			m_caddieId = 0;
			goto loaded;
		}
	}
	delete pPet;
	return false;

loaded:
	EnterCriticalSection(&m_csPet);
	m_pet = std::tr1::shared_ptr<IDummyPet>(pPet);
	m_bPlace = true;
	LeaveCriticalSection(&m_csPet);
	return true;
}

void LobbyCaddie::Unload983(bool bWait)
{
	if (m_pet)
		CSceneManagerJp::Instance983()->DeleteElement(m_pet->m_name.c_str());
	m_pet.reset();
	m_pet = std::tr1::shared_ptr<IDummyPet>((IDummyPet*)NULL);
	m_bPlace = false;
	m_typeId = 0;
	m_caddieId = 0;
	m_bLoaded = true;
	if (m_hand.IsWorking(0))
		m_hand.WaitUntilWorkDone((unsigned long)-1);
	g_resrcmng->UnlockAllByThread(m_hand.GetThreadID());
}

void LobbyCaddie::OnDestroy()
{
	Unload983(false);
	m_bPlace = true;
	m_pos = g_vZero983_f041c4;
}

void LobbyCaddie::LoadNow983()
{
	AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x53, 0, 0, 0, 0, 0);
	Load983(m_reqCaddieId, m_reqTypeId);
	AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x54, 0, 0, 0, 0, 0);
}

// corpo do trabalho da outra linha (m_loader): carrega uma vez e cede a vez
bool LobbyCaddie::LoadProc983()
{
	if (m_bLoaded == true)
		return false;
	LoadNow983();
	Sleep(15);
	return true;
}

void LobbyCaddie::OnPreLoadInit()
{
	_util::WMemFun983<LobbyCaddie, bool> fun;
	m_loader = *_util::InitMemFun983(&fun, this, &LobbyCaddie::LoadProc983);
	m_bLoaded = false;
	m_bRequest = false;
	m_reqCaddieId = 0;
	m_reqTypeId = 0;
}

void LobbyCaddie::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case 0x180:
		if (m_pet)
			m_pet->RefreshFx983(false, 0, 1);
		break;
	case 0x184:
		OnDestroy();
		break;
	case 0x193:
		{
			unsigned long caddieId = msg.param1 ? *(unsigned long*)msg.param1 : 0;
			int param2 = msg.param2;
			if (caddieId != DOCLOBBYJP()->GetCaddieId983())
			{
				HandleMsg(MsgObject(NULL, 0x1a6, 0, 0, 0, 0, 0));
				if (caddieId)
					HandleMsg(MsgObject(NULL, 0x1a5, (int)&caddieId, 0, 0, 0, 0));
				AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x49, 0, param2, 0, 0, 0);
			}
		}
		break;
	case 0x1a5:
		if (WisEqual(m_pos, g_vZero983_f041c4, 0.00001f))
			SetPoint983("Caddie");
		if (msg.param1)
			RequestLoad983(0, *(unsigned long*)msg.param1);
		else
			RequestLoad983(0, DOCLOBBYJP()->GetCaddieId983());
		break;
	case 0x1a6:
		Unload983(false);
		break;
	case 0x1a7:
		{
			unsigned long typeId = msg.param1 ? *(unsigned long*)msg.param1 : 0;
			if (m_typeId != typeId)
			{
				if (typeId == 0)
				{
					sCaddie983* pCaddie = (sCaddie983*)Doc()->FindCaddie983(DOCLOBBYJP()->GetCaddieId983());
					if (pCaddie)
						typeId = pCaddie->typeId;
				}
				if (m_typeId)
					Unload983(true);
				if (typeId != 0 && typeId != (unsigned long)-1)
					RequestLoad983(typeId, 0);
			}
		}
		break;
	case 0x1a8:
		Unload983(true);
		break;
	case 0x1a9:
		HandleMsg(MsgObject(NULL, 0x1a6, 0, 0, 0, 0, 0));
		HandleMsg(MsgObject(NULL, 0x1a5, (int)DOCLOBBYJP()->GetCaddieIdPtr983(), 0, 0, 0, 0));
		break;
	case 0x1ce:
	case 0x1cf:
		if (m_pet && m_pet->m_pPuppet)
			m_pet->m_pPuppet->SetAlpha(0xff);
		break;
	case 0x2ae:
		m_bLocked = true;
		break;
	case 0x2af:
		m_bLocked = false;
		break;
	}
}
