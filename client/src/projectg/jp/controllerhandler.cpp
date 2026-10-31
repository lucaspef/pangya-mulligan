// JP 983: unidade 004bab60–004bd8e0 (_gateway::_controller::CControllerHandler). Fonte reconstruída do 983 (classe nova, sem par no
// 645). Sessões B14 e B15: registro, tabela de mensagens, Execute, os quatro virtuais e as 25 mensagens — ver
// out/F3.7/s15/RELATORIO.md. O resto da faixa são instâncias da STL.
#include "controllerhandler.h"
#include "controller.h"
#include "shareddocjp.h"
#include "initdelegate.h"
#include "packet.h"
#include "taskmanager.h"
#include "golfdoc.h"
#include "vc10_layout.h"
#include <algorithm>
C_ASSERT_VC10(offsetof(CSharedDoc, m_pGolfDoc) == 0x11024 && offsetof(CSharedDoc, m_holeOrder) == 0x1051e);

using namespace _gateway;
using namespace _gateway::_controller;

IObject* CControllerHandlerMakeInstance() // 004bd7b0
{
	return new CControllerHandler;
}

struct __sCControllerHandler
{
	__sCControllerHandler() // 004bd820
	{
		ObjectFactory().AddObjectFunctor(CControllerHandlerMakeInstance, "CControllerHandler");
	}
};

const WRTTI CControllerHandler::m_RTTI("CControllerHandler", &IObject::m_RTTI);
static __sCControllerHandler __implCControllerHandler;

static void ReleaseControllerHandler() // 004bab50 (vazia)
{
}

static bool AddReleaseControllerHandler() // 004bb110 (chamada pelo inicializador estático 00cc90d0)
{
	static bool s_bAdded; // 00e74cc1
	if (!s_bAdded)
	{
		CInitDelegate::AddReleaseFunction(ReleaseControllerHandler);
		s_bAdded = true;
	}
	return s_bAdded;
}

static void RegisterControllerHandlerActor() // 004bc970: o ator já nasce criado e fica guardado na fábrica pelo nome
{
	ActorRegistry983().Add(CControllerHandler::m_RTTI.GetName(),
		ObjectFactory().CreateObject(CControllerHandler::m_RTTI.GetName()));
}

static bool AddInitControllerHandler() // 004bcac0 (chamada pelo inicializador estático 00cc90c0)
{
	static bool s_bAdded; // 00e74cc0
	if (!s_bAdded)
	{
		CInitDelegate::AddInitFunction(RegisterControllerHandlerActor);
		s_bAdded = true;
	}
	return s_bAdded;
}

static bool s_bInitAdded = AddInitControllerHandler(); // 00e74cc3
static bool s_bReleaseAdded = AddReleaseControllerHandler(); // 00e74ccc

// índice do jogador local na sala: CGolfDoc +0x189 (m_currentPlayer) via CSharedDoc+0x11024 (m_pGolfDoc)
// B1451: membro em vez de cru (no cl novo o CGolfDoc anda)
C_ASSERT_VC10(offsetof(CGolfDoc, m_currentPlayer) == 0x189);
C_ASSERT_VC10(offsetof(CGolfDoc, m_currentHole) == 0x17c);
#define LocalPlayer983() (::Doc()->m_pGolfDoc->m_currentPlayer) // B1446: +0x11024 = m_pGolfDoc

// cópia embutida da busca de 0062bee0 nas mensagens 0x18 e 0x19 (lá o resultado é int: -1 se não achar)
static inline int FindPlayerByUid(unsigned long uid)
{
	for (int i = 0; i < 4; i++)
	{
		if (uid == ::Doc()->m_userInfo[i].info.dwUID)
			return i;
	}
	return -1;
}

// uma função do CControlledObject em todos os objetos de todos os jogadores (004bbe40 = o for_each; 004bc630 = esta)
struct FireAll
{
	FireAll(bool bReset, void (CControlledObject::*pFunc)()) : m_bReset(bReset), m_pFunc(pFunc) { }
	void operator()(CControllerDoc::ObjectMap::value_type& v) const
	{
		for (CControllerDoc::ObjectList::iterator it = v.second.begin(); it != v.second.end(); ++it)
		{
			CControlledObject* p = *it;
			if (p)
			{
				if (m_bReset)
					p->On05();
				(p->*m_pFunc)();
			}
		}
	}
	bool m_bReset;
	void (CControlledObject::*m_pFunc)();
};

void FireAllObjects(CControllerDoc::ObjectMap& objects, bool bReset, void (CControlledObject::*pFunc)())
{
	std::for_each(objects.begin(), objects.end(), FireAll(bReset, pFunc));
}

void CControllerHandler::OnLeave()
{
	CControllerDoc* pDoc = Doc983();
	pDoc->FireOn03();
	pDoc->Clear();
}

void CControllerHandler::FireOn07(int player, unsigned long ability, int arg)
{
	if (ability - 1 > eSpecialAbility_Max - 2)
		return;
	CControlledObject* p = Doc983()->GetObjectAt(player, ability);
	if (p)
		p->On07(arg);
}

void CControllerHandler::FireOn08(int player, unsigned long ability, int arg)
{
	if (ability - 1 > eSpecialAbility_Max - 2)
		return;
	CControlledObject* p = Doc983()->GetObjectAt(player, ability);
	if (p)
	{
		p->Set28(true);
		p->On08(arg);
	}
}

bool CControllerHandler::InitializeIfEmpty()
{
	if (m_functionMap.size() == 0)
		return Initialize();
	return true;
}

bool CControllerHandler::OnEnter()
{
	Doc983()->RefreshEnable();
	if (m_functionMap.size() == 0)
		return Initialize();
	return true;
}

void CControllerHandler::OnSetObjects(const MsgObject& msg)
{
	int player = MSGJP(msg).Param2<int>();
	if (player < 4)
		Doc983()->SetObjects(player);
}

void CControllerHandler::OnAddAbility(const MsgObject& msg)
{
	sAbilityMsg* p = MSGJP(msg).Param2<sAbilityMsg*>();
	if (p)
		Doc983()->AddAbility(p->m_player, p->m_ability, p->m_08, p->m_typeId, p->m_value);
}

void CControllerHandler::OnGetObject(const MsgObject& msg)
{
	CControllerDoc* pDoc = Doc983();
	int player = MSGJP(msg).Param2<int>();
	unsigned long ability = MSGJP(msg).Param3<unsigned long>();
	if (ability)
		*(CControlledObject**)msg.param4 = pDoc->GetObjectAt(player, ability);
}

void CControllerHandler::OnObject08(const MsgObject& msg)
{
	CControllerDoc* pDoc = Doc983();
	int player = MSGJP(msg).Param2<int>();
	unsigned long ability = MSGJP(msg).Param3<unsigned long>();
	int arg = MSGJP(msg).Param4<int>();
	if (ability)
	{
		CControlledObject* p = pDoc->GetObjectAt(player, ability);
		if (p)
		{
			p->Set28(true);
			p->On08(arg);
		}
	}
}

void CControllerHandler::OnObject07(const MsgObject& msg)
{
	CControllerDoc* pDoc = Doc983();
	int player = MSGJP(msg).Param2<int>();
	unsigned long ability = MSGJP(msg).Param3<unsigned long>();
	int arg = MSGJP(msg).Param4<int>();
	if (ability)
	{
		CControlledObject* p = pDoc->GetObjectAt(player, ability);
		if (p)
			p->On07(arg);
	}
}

void CControllerHandler::OnProcess(float delta)
{
	CControllerDoc* pDoc = Doc983();
	if (pDoc->GetMode983() == 0x10)
	{
		CControllerDoc::ObjectMap& objects = pDoc->GetObjects();
		std::for_each(objects.begin(), objects.end(), [delta](CControllerDoc::ObjectMap::value_type& v)
		{
			for (CControllerDoc::ObjectList::iterator it = v.second.begin(); it != v.second.end(); ++it)
			{
				if (*it)
					(*it)->Fire06(delta);
			}
		});
	}
}

void CControllerHandler::OnSetSubObjects(const MsgObject& msg)
{
	CControllerDoc* pDoc = Doc983();
	if (!pDoc->IsUsable983())
		return;
	unsigned char player = MSGJP(msg).Param2<unsigned char>();
	const sCharacterInfo* pInfo = MSGJP(msg).Param3<const sCharacterInfo*>();
	pDoc->RemoveSubObjects(player);
	pDoc->SetSubObjects(player, pInfo);
	CControllerDoc::ObjectMap& objects = pDoc->GetSubObjects();
	CControllerDoc::ObjectMap::iterator it = objects.find(player);
	if (it != objects.end())
	{
		for (CControllerDoc::ObjectList::iterator i = it->second.begin(); i != it->second.end(); ++i)
		{
			if (*i)
				(*i)->Fire11();
		}
	}
}

void CControllerHandler::OnDisplay()
{
	CControllerDoc::ObjectMap& objects = Doc983()->GetObjects();
	FireAllObjects(objects, false, &CControlledObject::Fire12);
}

void CControllerHandler::OnFire04()
{
	// só quando CGolfDoc +0x17c (m_currentHole) é igual ao byte CSharedDoc+0x1051e (m_holeOrder[0])
	CSharedDoc* pDoc = ::Doc();
	if (pDoc->m_pGolfDoc->m_currentHole == pDoc->m_holeOrder[0]) // B1446/B1451: +0x11024/+0x17c/+0x1051e
	{
		CControllerDoc::ObjectMap& objects = Doc983()->GetObjects();
		FireAllObjects(objects, true, &CControlledObject::Fire04);
	}
}

void CControllerHandler::OnFire05()
{
	CControllerDoc::ObjectMap& objects = Doc983()->GetObjects();
	FireAllObjects(objects, true, &CControlledObject::Fire05);
}

void CControllerHandler::OnFire08()
{
	CControllerDoc::ObjectMap& objects = Doc983()->GetObjects();
	FireAllObjects(objects, false, &CControlledObject::Fire08);
}

void CControllerHandler::OnFire10(const MsgObject& msg)
{
	int arg = MSGJP(msg).Param2<unsigned long>();
	CControllerDoc::ObjectMap& objects = Doc983()->GetObjects();
	std::for_each(objects.begin(), objects.end(), [arg](CControllerDoc::ObjectMap::value_type& v)
	{
		for (CControllerDoc::ObjectList::iterator it = v.second.begin(); it != v.second.end(); ++it)
		{
			if (*it)
				(*it)->Fire10(arg);
		}
	});
}

void CControllerHandler::OnFire09()
{
	CControllerDoc::ObjectMap& objects = Doc983()->GetObjects();
	FireAllObjects(objects, false, &CControlledObject::Fire09);
}

void CControllerHandler::OnFire13()
{
	CControllerDoc::ObjectMap& objects = Doc983()->GetObjects();
	std::for_each(objects.begin(), objects.end(), [](CControllerDoc::ObjectMap::value_type& v)
	{
		for (CControllerDoc::ObjectList::iterator it = v.second.begin(); it != v.second.end(); ++it)
		{
			if (*it)
				(*it)->Fire13();
		}
	});
}

void CControllerHandler::OnPacket13(const MsgObject& msg)
{
	sAbilityPacket data;
	MSGJP(msg).Param2<WReceivedPacket*>()->DecodeBuffer(&data, sizeof(data));
	unsigned char player = Doc()->FindPlayerByUid983(data.m_uid);
	if (player == 0xff)
		return;
	if (data.m_uid != ::Doc()->m_myInfo.info.dwUID)
		FireOn08(player, data.m_ability, data.m_arg);
	else
		FireOn07(player, data.m_ability, data.m_arg);
}

void CControllerHandler::OnLoad(const MsgObject& msg)
{
	FILE* fp = MSGJP(msg).Param2<FILE*>();
	CControllerDoc::ObjectList* pList = Doc983()->GetObjectList(LocalPlayer983());
	if (pList)
	{
		unsigned long count = 0;
		fread(&count, 4, 1, fp);
		for (unsigned long i = 0; i < count; i++)
		{
			unsigned long ability = 0;
			fread(&ability, 4, 1, fp);
			CControlledObject* p = pList->at(ability);
			if (p)
				p->Load(fp);
		}
	}
}

void CControllerHandler::OnLoaded()
{
	CControlledObject::SetActive983(true);
	CControllerDoc* pDoc = Doc983();
	pDoc->FireOn09(LocalPlayer983());
	pDoc->Purge(LocalPlayer983());
}

void CControllerHandler::OnPacket14(const MsgObject& msg)
{
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	unsigned long uid = pPacket->Decode4();
	unsigned long value = pPacket->Decode4();
	unsigned char flag = pPacket->Decode1();
	unsigned char player = Doc()->FindPlayerByUid983(uid);
	if (player == 0xff)
		return;
	sAuxpartPacket data;
	data.m_player = player;
	data.m_value = value;
	data.m_uid = uid;
	data.m_flag = flag;
	if (::Doc()->m_myInfo.info.dwUID != uid)
		FireOn08(player, eSpecialAbility_ConditionalAuxpart, (int)&data);
	else
		FireOn07(player, eSpecialAbility_ConditionalAuxpart, (int)&data);
}

void CControllerHandler::OnPacket15(const MsgObject& msg)
{
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	pPacket->Decode4();
	unsigned long uid = pPacket->Decode4();
	unsigned char flag = pPacket->Decode1();
	float value = 0.0f;
	pPacket->DecodeBuffer(&value, 4);
	unsigned char player = Doc()->FindPlayerByUid983(uid);
	if (player == 0xff)
		return;
	sWindPacket data;
	data.m_value = value;
	data.m_flag = flag;
	if (::Doc()->m_myInfo.info.dwUID != uid)
		FireOn08(player, eSpecialAbility_WindDirection, (int)&data);
	else
		FireOn07(player, eSpecialAbility_WindDirection, (int)&data);
}

void CControllerHandler::OnPacket16(const MsgObject& msg)
{
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	int value = pPacket->Decode4();
	unsigned long uid = pPacket->Decode4();
	unsigned char player = Doc()->FindPlayerByUid983(uid);
	if (player == 0xff)
		return;
	if (Doc983()->GetObjectAt(player, eSpecialAbility_WindVolume))
		FireOn08(player, eSpecialAbility_WindVolume, value);
}

void CControllerHandler::OnPacket18(const MsgObject& msg)
{
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	pPacket->Decode4();
	int player = FindPlayerByUid(pPacket->Decode4());
	if (player == -1)
		return;
	if (Doc983()->GetObjectAt(player, eSpecialAbility_AutoMiracleSign))
		AfxGetTask()->GetActor("GuideLine") << MsgObject(NULL, 0x214, 1, 0xff9eeaff, 0, 0, 0);
}

void CControllerHandler::OnPacket19(const MsgObject& msg)
{
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	pPacket->Decode4();
	int player = FindPlayerByUid(pPacket->Decode4());
	if (player == -1)
		return;
	if (Doc983()->GetObjectAt(player, eSpecialAbility_MiracleSignSkill))
		AfxGetTask()->GetActor("GuideLine") << MsgObject(NULL, 0x214, 1, 0xff9eeaff, 0, 0, 0);
}

void CControllerHandler::OnSave(const MsgObject& msg)
{
	CControllerDoc* pDoc = Doc983();
	FILE* fp = MSGJP(msg).Param2<FILE*>();
	CControllerDoc::ObjectList* pList = pDoc->GetObjectList(LocalPlayer983());
	if (pList)
	{
		int count = pDoc->CountKept(pList);
		fwrite(&count, 4, 1, fp);
		std::for_each(pList->begin(), pList->end(), [fp](CControlledObject* p)
		{
			if (p && p->On11())
				p->Save(fp);
		});
	}
}

void CControllerHandler::OnObject10(const MsgObject& msg)
{
	int arg = MSGJP(msg).Param2<int>();
	CControllerDoc::ObjectList* pList = Doc983()->GetObjectList(0);
	if (pList)
	{
		for (CControllerDoc::ObjectList::iterator it = pList->begin(), end = pList->end(); it != end; ++it)
		{
			if (*it)
				(*it)->On10(arg);
		}
	}
}

void CControllerHandler::OnTurn(const MsgObject& msg)
{
	CControllerDoc* pDoc = Doc983();
	int arg = MSGJP(msg).Param2<int>();
	CControllerDoc::ObjectMap& objects = pDoc->GetObjects();
	for (CControllerDoc::ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
	{
		unsigned char player = it->first;
		std::for_each(it->second.begin(), it->second.end(), [player, arg](CControlledObject* p)
		{
			if (p)
			{
				p->On06(arg);
				if (player == arg)
					p->Fire07(arg);
			}
		});
	}
}

void CControllerHandler::OnFireAll()
{
	CControllerDoc::ObjectMap& objects = Doc983()->GetObjects();
	for (CControllerDoc::ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it)
	{
		unsigned char player = it->first;
		std::for_each(it->second.begin(), it->second.end(), [player](CControlledObject* p)
		{
			if (p)
			{
				float zero = 0.0f;
				p->Fire04();
				p->Fire05();
				p->Fire06(zero);
				p->Fire07(player);
			}
		});
	}
}

void CControllerHandler::OnSnapshot(const MsgObject& msg)
{
	unsigned char player = MSGJP(msg).Param2<unsigned char>();
	CControllerDoc::ObjectMap& objects = Doc983()->GetObjects();
	CControllerDoc::ObjectMap::iterator it = objects.find(player);
	if (it != objects.end())
	{
		CControllerDoc::ObjectList list(it->second);
		for (CControllerDoc::ObjectList::iterator i = list.begin(); i != list.end(); ++i)
		{
			if (*i)
				(*i)->Snapshot();
		}
	}
}

bool CControllerHandler::Execute(const MsgObject& msg)
{
	SetFunctionType(msg.param1);
	return Exec<CControllerHandler, const MsgObject&>(this, msg);
}

bool CControllerHandler::Initialize()
{
	AddFunctionType(0x00, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnSetObjects);
	AddFunctionType(0x01, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnSetSubObjects);
	AddFunctionType(0x02, FUNCTIONTYPE_VOID, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnFire04);
	AddFunctionType(0x03, FUNCTIONTYPE_VOID, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnFire05);
	AddFunctionType(0x04, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnTurn);
	AddFunctionType(0x05, FUNCTIONTYPE_VOID, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnFire08);
	AddFunctionType(0x06, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnFire10);
	AddFunctionType(0x07, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnSnapshot);
	AddFunctionType(0x08, FUNCTIONTYPE_VOID, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnFire09);
	AddFunctionType(0x10, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnGetObject);
	AddFunctionType(0x0f, FUNCTIONTYPE_VOID, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnFireAll);
	AddFunctionType(0x11, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnObject08);
	AddFunctionType(0x12, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnObject07);
	AddFunctionType(0x13, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnPacket13);
	AddFunctionType(0x09, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnAddAbility);
	AddFunctionType(0x0a, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnSave);
	AddFunctionType(0x0b, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnLoad);
	AddFunctionType(0x0c, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnObject10);
	AddFunctionType(0x0d, FUNCTIONTYPE_VOID, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnLoaded);
	AddFunctionType(0x14, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnPacket14);
	AddFunctionType(0x15, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnPacket15);
	AddFunctionType(0x16, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnPacket16);
	AddFunctionType(0x17, FUNCTIONTYPE_VOID, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnFire13);
	AddFunctionType(0x18, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnPacket18);
	AddFunctionType(0x19, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CControllerHandler::*)())&CControllerHandler::OnPacket19);
	return true;
}
