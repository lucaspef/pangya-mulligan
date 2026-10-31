// JP 983: LobbyMascot (00563860–005652d0: construtor 00564fb0, destrutor 00564e60, criador 005650e0, registro 00565150,
// OnPreLoadInit 005648f0, OnProcess 00564340, OnDisplay 00563870, OnDestroy 005643e0, HandleMsg 00565180 e 8 auxiliares; o
// resto da faixa são moldes de map/function; 00565300–00565b40 é da unidade lobbyplayer). Fonte reconstruída do 983 (classe
// sem par no 645; par no KR 839). Sessão B31.
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "lobbymascot.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "polysoup.h"
#include "commonutil.h"
#include "wpuppet.h"
#include "wresrcmng.h"

// JP 983: objeto apontado por CSharedDoc+0x11024 (m_pGolfDoc; A729: membro, cl novo 0x10cc4); o WPolySoup do cenário está em +8
struct sLobbyScene983
{
	char m_00[8];
	WPolySoup* m_pSoup; // +0x08
};
#define LOBBYSOUP983() (((sLobbyScene983*)::Doc()->m_pGolfDoc)->m_pSoup)

extern const WVector g_vZero983_f041c4; // 00f041c4

IMPLEMENT_ACTOR(LobbyMascot, IActor)

LobbyMascot::LobbyMascot()
	: m_pPet(NULL), m_id(0), m_pos(g_vZero983_f041c4), m_bPlace(true), m_48(0)
{
	InitializeCriticalSection(&m_csPet);
}

LobbyMascot::~LobbyMascot()
{
	DeleteCriticalSection(&m_csPet);
}

void LobbyMascot::OnLoad()
{
}

void LobbyMascot::OnDisplay()
{
	if (m_pPet)
		m_pPet->Display();
}

void LobbyMascot::OnAnotherHand(int key, void* param)
{
	m_loader();
	m_bLoaded = true;
}

void LobbyMascot::Request983(unsigned long id)
{
	if (m_hand.IsWorking(0))
		return;
	m_reqId = id;
	m_bLoaded = false;
	m_bReady = true;
}

void LobbyMascot::Rotate983(float angle)
{
	if (m_pPet)
	{
		WMatrix rot;
		RotMat(angle, 1, rot);
		m_pPet->m_matrix = rot * m_pPet->m_matrix;
	}
}

void LobbyMascot::SetPoint983(const char* name)
{
	sLobbyScene983* pScene = (sLobbyScene983*)::Doc()->m_pGolfDoc; // A729: membro (cl novo 0x10cc4)
	WPolySoup::sPoint* pPoint = pScene->m_pSoup->FindPoint(name);
	if (pPoint)
		m_pos = pPoint->pos;
}

bool LobbyMascot::Place983()
{
	if (m_pPet && m_pPet->SetMotion("Default"))
	{
		m_pPet->SetPosition(m_pos);
		m_pPet->m_bMoved = true;
		m_pPet->m_bRotated = true;
		CSceneManagerJp::Instance983()->AddElement983(4, 0, 0, m_pPet->m_name.c_str(), 0x18, 0);
		CSceneManagerJp::Instance983()->SetPuppet(m_pPet->m_name.c_str(), m_pPet->m_pPuppet);
		return true;
	}
	return false;
}

void LobbyMascot::Unload983()
{
	if (m_pPet)
		CSceneManagerJp::Instance983()->DeleteElement(m_pPet->m_name.c_str());
	if (m_pPet)
	{
		delete m_pPet;
		m_pPet = NULL;
	}
	m_pPet = NULL;
	m_bPlace = false;
	m_id = 0;
	m_bLoaded = true;
	if (m_hand.IsWorking(0))
		m_hand.WaitUntilWorkDone((unsigned long)-1);
	g_resrcmng->UnlockAllByThread(m_hand.GetThreadID());
}

void LobbyMascot::OnProcess(float delta)
{
	if (m_bReady == true)
	{
		m_bReady = false;
		if (!m_hand.IsWorking(0))
			m_hand.Work(this, 0, NULL, true, -1);
	}
	if (m_pPet && m_bPlace)
	{
		Place983();
		m_bPlace = false;
	}
	if (m_pPet)
		m_pPet->Process(delta);
}

void LobbyMascot::OnDestroy()
{
	Unload983();
	m_bPlace = true;
	m_pos = g_vZero983_f041c4;
}

// cria o boneco do mascote `id` do jogador (0 = o que está em uso)
bool LobbyMascot::LoadMascot983(unsigned long id)
{
	if (m_pPet)
		return false;
	IDummyPet* pPet = new IDummyPet(5);
	if (!pPet)
		return false;
	sMascotInfo* pInfo = ::Doc()->GetMyMascotInfo(id ? id : DOCLOBBYJP()->GetMascotId983());
	if (pInfo)
	{
		pPet->m_name = MakeStr("LOBBY_MASCOT_%d_%d", DOCLOBBYJP()->GetUid983(), pInfo->tid);
		if (IDummyPet::LoadMascot983(pPet, pInfo->tid, true))
		{
			EnterCriticalSection(&m_csPet);
			m_pPet = pPet;
			LeaveCriticalSection(&m_csPet);
		}
		return true;
	}
	delete pPet;
	return false;
}

void LobbyMascot::LoadNow983()
{
	m_bPlace = LoadMascot983(m_reqId);
	if (m_bPlace)
		m_id = m_reqId;
}

// corpo do trabalho da outra linha (m_loader): carrega uma vez e cede a vez
bool LobbyMascot::LoadProc983()
{
	if (m_bLoaded == true)
		return false;
	LoadNow983();
	Sleep(15);
	return true;
}

void LobbyMascot::OnPreLoadInit()
{
	_util::WMemFun983<LobbyMascot, bool> fun;
	m_loader = *_util::InitMemFun983(&fun, this, &LobbyMascot::LoadProc983);
	m_bLoaded = false;
	m_bReady = false;
	m_reqId = 0;
}

void LobbyMascot::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case 0x184:
		OnDestroy();
		break;
	case 0x1aa:
		{
			if (WisEqual(m_pos, g_vZero983_f041c4, 0.00001f))
				SetPoint983("mascot");
			unsigned long id = msg.param1 ? msg.param1 : DOCLOBBYJP()->GetMascotId983();
			if (m_id != id)
			{
				m_reqId = id;
				m_bLoaded = false;
				m_bReady = true;
			}
		}
		break;
	case 0x1ab:
		Unload983();
		break;
	case 0x180:
		if (m_pPet)
			m_pPet->RefreshFx983(false, 0, 1); // o 983 empilha 1, 0, 0
		break;
	case 0x1ce:
		if (m_pPet && m_pPet->m_pPuppet)
			m_pPet->m_pPuppet->SetAlpha(0xff);
		break;
	case 0x1cf:
		if (m_pPet && m_pPet->m_pPuppet)
			m_pPet->m_pPuppet->SetAlpha(0);
		break;
	}
}

// layout medido no 983: `new` de 0x210 em 00565103; CAnotherHand em +0x70 (construtor 0056505b) e seção crítica em +0x1f8
struct sLobbyMascotLayout983
{
	C_ASSERT_VC10(sizeof(LobbyMascot) == 0x210);
	C_ASSERT(offsetof(LobbyMascot, m_cache) == 0x20);
	C_ASSERT_VC10(offsetof(LobbyMascot, m_id) == 0x30);
	C_ASSERT_VC10(offsetof(LobbyMascot, m_loader) == 0x50);
	C_ASSERT_VC10(offsetof(LobbyMascot, m_bLoaded) == 0x68);
	C_ASSERT_VC10(offsetof(LobbyMascot, m_hand) == 0x70);
	C_ASSERT_VC10(offsetof(LobbyMascot, m_csPet) == 0x1f8);
};
