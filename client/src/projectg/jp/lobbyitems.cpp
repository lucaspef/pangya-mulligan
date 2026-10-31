// JP 983: LobbyItems (00561b80–00563860: construtor 005633f0, destrutor 00562df0, criador 00563680, registro 005636f0,
// OnPreLoadInit 00563750, OnProcess 00562ef0, OnDisplay 005621f0, OnDestroy 005635d0, HandleMsg 005637c0 e 8 auxiliares; o
// resto da faixa são moldes de deque/function). Fonte reconstruída do 983 (classe sem par no 645; par no KR 839). Sessão B30.
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "lobbyitems.h"
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

IMPLEMENT_ACTOR(LobbyItems, IActor)

LobbyItems::LobbyItems()
	: m_60(0)
{
	InitializeCriticalSection(&m_csPet);
	for (int i = 0; i < 2; ++i)
	{
		m_pets[i] = std::tr1::shared_ptr<IDummyPet>((IDummyPet*)NULL);
		m_bPlace[i] = true;
		m_pos[i] = g_vZero983_f041c4;
	}
}

LobbyItems::~LobbyItems()
{
	DeleteCriticalSection(&m_csPet);
}

void LobbyItems::OnLoad()
{
}

void LobbyItems::OnAnotherHand(int key, void* param)
{
	m_loader();
	m_bLoaded = true;
}

void LobbyItems::OnDisplay()
{
	for (int i = 0; i < 2; ++i)
	{
		if (m_pets[i])
			m_pets[i]->Display();
	}
}

void LobbyItems::SetPoint983(int index, const char* name)
{
	WPolySoup::sPoint* pPoint = LOBBYSOUP983()->FindPoint(name);
	if (pPoint)
		m_pos[index] = pPoint->pos;
}

bool LobbyItems::Place983(int index)
{
	if (index == 2 || !m_pets[index])
		return false;
	if (!m_pets[index]->SetMotion(NULL))
		return false;
	m_pets[index]->SetPosition(m_pos[index]);
	m_pets[index]->m_bMoved = true;
	m_pets[index]->m_bRotated = true;
	CSceneManagerJp::Instance983()->AddElement983(4, 0, 0, m_pets[index]->m_name.c_str(), 0x18, 0);
	CSceneManagerJp::Instance983()->SetPuppet(m_pets[index]->m_name.c_str(), m_pets[index]->m_pPuppet);
	return true;
}

void LobbyItems::OnProcess(float delta)
{
	if (!m_queue.empty() && m_bReady == true)
	{
		sLobbyItemReq983& req = m_queue.front();
		m_queue.pop_front();
		m_reqIndex = req.index;
		m_reqId = req.id;
		m_bReady = false;
		if (!m_hand.IsWorking(0))
			m_hand.Work(this, 0, NULL, true, -1);
	}
	for (int i = 0; i < 2; ++i)
	{
		if (m_pets[i])
		{
			if (m_bPlace[i])
			{
				Place983(i);
				m_bPlace[i] = false;
			}
			m_pets[i]->Process(delta);
		}
	}
}

void LobbyItems::Request983(int index, unsigned long id)
{
	sLobbyItemReq983 req;
	req.index = index;
	req.id = id;
	m_queue.push_back(req);
}

// cria o boneco do jogo de tacos `id` do jogador (0 = o que está em uso)
bool LobbyItems::LoadClubSet983(unsigned long id)
{
	if (m_pets[0])
		return false;
	IDummyPet* pPet = new IDummyPet(2);
	if (!pPet)
		return false;
	if (id == 0)
		id = DOCLOBBYJP()->GetClubSetId983();
	sItem983* pClubSet = (sItem983*)Doc()->FindClubSet983(id);
	if (pClubSet)
	{
		char* pItem = ITEMMGRLOBBYJP()->Find1e0_983(pClubSet->typeId);
		if (pItem)
		{
			pPet->m_name = MakeStr("LOBBY_CLUBSET_%d_%d", DOCLOBBYJP()->GetUid983(), pClubSet->typeId);
			if (IDummyPet::LoadClub983(pPet, *(unsigned long*)(pItem + 0xc0), true))
			{
				EnterCriticalSection(&m_csPet);
				m_pets[0] = std::tr1::shared_ptr<IDummyPet>(pPet);
				LeaveCriticalSection(&m_csPet);
			}
			return true;
		}
	}
	delete pPet;
	return false;
}

void LobbyItems::Unload983(int index)
{
	int begin = index;
	int end = index + 1;
	if (index == 2)
	{
		begin = 0;
		end = 2;
	}
	for (int i = begin; i < end; ++i)
	{
		if (m_pets[i])
			CSceneManagerJp::Instance983()->DeleteElement(m_pets[i]->m_name.c_str());
		m_pets[i] = std::tr1::shared_ptr<IDummyPet>((IDummyPet*)NULL);
		m_bPlace[i] = false;
	}
	if (index != 2)
	{
		m_bLoaded = true;
		if (m_hand.IsWorking(0))
			m_hand.WaitUntilWorkDone((unsigned long)-1);
		g_resrcmng->UnlockAllByThread(m_hand.GetThreadID());
	}
}

void LobbyItems::OnDestroy()
{
	Unload983(2);
	for (int i = 0; i < 2; ++i)
	{
		m_bPlace[i] = true;
		m_pos[i] = g_vZero983_f041c4;
	}
	for (size_t i = 0; i < m_queue.size(); ++i)
		m_queue.pop_front();
}

void LobbyItems::LoadNow983(int index)
{
	if (index == 0)
		m_bPlace[0] = LoadClubSet983(m_reqId);
}

// corpo do trabalho da outra linha (m_loader): carrega uma vez e cede a vez
bool LobbyItems::LoadProc983()
{
	if (m_bLoaded == true)
		return false;
	LoadNow983(m_reqIndex);
	Sleep(15);
	return true;
}

void LobbyItems::OnPreLoadInit()
{
	_util::WMemFun983<LobbyItems, bool> fun;
	m_loader = *_util::InitMemFun983(&fun, this, &LobbyItems::LoadProc983);
	m_bLoaded = false;
	m_bReady = false;
	m_reqIndex = 2;
	m_reqId = 0;
}

void LobbyItems::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case 0x184:
		OnDestroy();
		break;
	case 0x1ac:
		if (WisEqual(m_pos[0], g_vZero983_f041c4, 0.00001f))
		{
			SetPoint983(0, "\xbd\xc3\xc0\xdb\xc1\xa1"); // "시작점" (ponto de início) em CP949
			m_pos[0].y += 10.0;
		}
		Request983(0, msg.param1 ? msg.param1 : DOCLOBBYJP()->GetClubSetId983());
		m_bLoaded = false;
		m_bReady = true;
		break;
	case 0x1ad:
		Unload983(0);
		break;
	}
}

// layout medido no 983: `new` de 0x230 em 00563690; CAnotherHand em +0x90 (construtor 005634a6) e seção crítica em +0x218
struct sLobbyItemsLayout983
{
	C_ASSERT_VC10(sizeof(LobbyItems) == 0x230);
	C_ASSERT(offsetof(LobbyItems, m_queue) == 0x48);
	C_ASSERT_VC10(offsetof(LobbyItems, m_loader) == 0x68);
	C_ASSERT_VC10(offsetof(LobbyItems, m_bLoaded) == 0x80);
	C_ASSERT_VC10(offsetof(LobbyItems, m_hand) == 0x90);
	C_ASSERT_VC10(offsetof(LobbyItems, m_csPet) == 0x218);
};
