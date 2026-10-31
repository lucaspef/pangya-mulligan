#include "minatl.h"
#include <tlhelp32.h> // B1178: watchpoint D486 em todas as threads
#define REBANG_AVATAR_TASK_CPP
#include "avatar_task.h"
#include "actor.h"
#include "fresh.h"
#include "frform.h"
#include "frwndmanager.h"
#include "frdesktop.h"
#include "frgaugebar.h"
#include "clientsetting.h"
#include "mousecursor.h"
#include "scenemanager.h"
#include "inputmanager.h"
#include "chatmsg.h"
#include "quitwindow.h"
#include "messengerdlg.h"
#include "jp/createcharacteroption.h"
#include "jp/refresh/rflobbydesktop.h"
#include "jp/extraresource.h"
#include "fx.h"
#include "gatewayactor.h"
#include "projectg.h"
#include "partsinfo.h"
#include "../shared/localize.h"


// HACK: retain the original header constants in the non-COMDAT .rdata.
static bool IsValidRange(float f)
{
	return f < g_HUGE && f > g_EPSILON;
}

extern Fresh* g_pFresh;
extern int g_iRecvPacketType;

struct sLoadParam
{
	void* pList;
	int total;
	int current;
	bool bFlagC;
	bool bFlagD;
	unsigned char unused[2];
};
extern sLoadParam g_LoadParam;
extern bool g_SALoadingState;
void RequestUccInfo(sCharacterInfo& info);

#include "vc10_layout.h"
// B1470: era DOC_AT cru do VC10 (+0x10594/+0x10595/+0xd8/+0x111d4); no cl 14.44 o CSharedDoc anda -> membros.
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.gameType) == 0x10594 && offsetof(CSharedDoc, m_golfGame.gameTypeName) == 0x10595);
C_ASSERT_VC10(offsetof(CSharedDoc, m_gameTypeInfo) + 2 * sizeof(sGameTypeInfo) == 0xd8 && offsetof(CSharedDoc, m_stateTrade) == 0x111d4);
C_ASSERT_VC10(offsetof(CGolfDoc, m_bReport) == 0x15f && offsetof(CGolfDoc, m_bStopRequest) == 0x191); // 983 007b5a20 caso 0x94

IMPLEMENT_OBJECT(ntAvatarChatTask, CTask)


ntAvatarChatTask::ntAvatarChatTask()
{
	m_bLobbyScreenSize = false;
	m_bQuit = false;
	InitializeCriticalSection(&m_cs);
	new ntAvatarManager;
}

ntAvatarChatTask::~ntAvatarChatTask()
{
	COption::Instance()->gSetAvatarNewBie(0);
	COption::Instance()->vSetMinimumScreenSize(0, 800, 600);
	m_bQuit = true;
	m_anotherHand.WaitUntilWorkDone(9999);
	m_anotherHand.BreakWorkingThread();
	DeleteCriticalSection(&m_cs);
	delete ntAvatarManager::m_pInstance;
}

void ntAvatarChatTask::Load()
{
	m_anotherHand.Work(this, 0, NULL, false, 0);
}

void ntAvatarChatTask::Init(const char* wallPaper)
{
	if (m_pLoadingFresh)
	{
		delete m_pLoadingFresh;
		m_pLoadingFresh = NULL;
	}

	g_pFresh->GetManager()->GetDesktop()->Init(g_view->GetWidth(),
		g_view->GetHeight());
	// 983: sem SetWallPaper (007b3aa0, ret sem argumento); wallPaper ignorado
	CChatMsg::m_pInstance->SetActive(true, true, true);
}

void ntAvatarChatTask::Register()
{
	SetMainActor(AddActor("ntAvatarChatMain", "AvatarChat", 0, ""));
	AddActor("CNPC", "NPC", 0, "");
	if (IsLocalContent((localContentType_t)0x76))
		AddActor("CPrivateTrade", "PrivateTrade", 0, "");
	AddActor(s_gatewayActor[0], s_gatewayActor[1], 0, GetRTTI()->GetName());
}

void ntAvatarChatTask::PreserveBack(const char* name)
{
	CTask::PreserveBack(name);
	CChatMsg::Instance()->Close(true, true);
}

void ntAvatarChatTask::Destroy()
{
	CTask::Destroy();
	delete CQuitWindow::Instance();
}

void ntAvatarChatTask::ShowQuitDialog()
{
	if (CChatMsg::m_pInstance->IsOpened())
		CChatMsg::m_pInstance->Close(true, true);
	else if (CMessengerInfo::Instance()->IsVisible())
		CMessengerInfo::Instance()->Close(false); // 983: o ESC fecha primeiro o mensageiro (00902650/00905710)
	else if (RFLOBBYDESKTOP983 && !RFLOBBYDESKTOP983->m_200 && CQuitWindow::m_pInstance)
		CQuitWindow::m_pInstance->Process();
}

void ntAvatarChatTask::Display()
{
	CTask::Display();

	FrDesktop* pDesktop = g_pFresh->GetManager()->GetDesktop();
	if (pDesktop)
		pDesktop->OnDisplay(true, false, true, true);

	if (!CChatMsg::m_pInstance->HasBeenPrinted())
	{
		CChatMsg::m_pInstance->PrintInScreen();
		CChatMsg::m_pInstance->DrawChatWindow();
		CChatMsg::m_pInstance->PrintInChatWindow();
	}

	g_pFresh->OnDisplay(true);
}

void ntAvatarChatTask::PreLoadInit()
{
	for (sActor* pActor = m_actorList.Start(); pActor;
		pActor = m_actorList.Next())
		pActor->actor->OnPreLoadInit();

	IActor::m_defaultSkipLayer = 0;
	IActor::m_pLayer = NULL;

	SetLoadingBackGround(CExtraResource::Instance()->GetLoading(2));
	CChatMsg::m_pInstance->ResetUiPosition();

	new CQuitWindow;

	Doc()->m_golfGame.gameType = 2;
	{
		const char* src = Doc()->m_gameTypeInfo[2].name;
		char* dst = Doc()->m_golfGame.gameTypeName;
		char c;
		do
		{
			c = *src;
			*dst = c;
			src++;
			dst++;
		} while (c);
	}

	for (sActor* pActor = m_actorList.Start(); pActor;
		pActor = m_actorList.Next())
		pActor->actor->OnLoad();

	for (sActor* pActor = m_actorList.Start(); pActor;
		pActor = m_actorList.Next())
		pActor->actor->OnInit();

	if (((m_taskFlag >> 1) & 1) == 0)
	{
		m_taskFlag |= 2;
		OnInitFinished();
	}

	g_LoadParam.total = 1;
	g_LoadParam.current = 0;

	if (m_pProgressGauge)
		m_pProgressGauge->SetRange(0, 100, 0);

	m_pLoadingFresh = new Fresh;
	m_pProgressGauge = NULL;
	m_pLoadingFresh->Init("loading.xml", false,
		m_loadingBackGround != "" ? m_loadingBackGround.c_str() : NULL);
	m_pLoadingFresh->OpenLayout("LOADING", NULL, true);

	if (m_bTip)
	{
		m_pTipForm = LoadTipText(0, 0);
		if (m_pTipForm)
		{
			m_pProgressGauge = DYNAMIC_CAST(FrGaugeBar,
				m_pTipForm->FindChildByName("progress"));
			if (m_pProgressGauge)
				m_pProgressGauge->SetBarSpeed(200.0f);
		}
	}

	if (CMouseCursor::m_pInstance)
	{
		CMouseCursor::m_pInstance->SetActive(true);
		CMouseCursor::m_pInstance->SetMode(2);
	}
}

void ntAvatarChatTask::RestorePreserved()
{
	if (0)
		((ntAvatarChatTask*)0)->ntAvatarChatTask::PreLoadInit();
	if (Doc()->m_golfGame.gameType == 8 && CFx::m_pInstance)
		CFx::m_pInstance->SetActive(true);

	Doc()->m_golfGame.gameType = 2;
	{
		const char* src = Doc()->m_gameTypeInfo[2].name;
		char* dst = Doc()->m_golfGame.gameTypeName;
		char c;
		do
		{
			c = *src;
			*dst = c;
			src++;
			dst++;
		} while (c);
	}

	CTask::RestorePreserved();

	FrDesktop* pDesktop = g_pFresh->GetManager()->GetDesktop();
	pDesktop->SetViewFocus(true);
	pDesktop->Init(g_view->GetWidth(), g_view->GetHeight());
	pDesktop->SetWallPaper(NULL, true);
	CChatMsg::m_pInstance->ResetUiPosition();
}

void ntAvatarChatTask::AddAvatarBurden(const ntAvatarBurden& burden)
{
	EnterCriticalSection(&m_cs);
	m_addList.push_back(burden);
	LeaveCriticalSection(&m_cs);
}

void ntAvatarChatTask::RemoveAvatarBurden(const ntAvatarBurden& burden)
{
	if (!m_bQuit)
	{
		EnterCriticalSection(&m_cs);
		m_removeList.push_back(burden);
		LeaveCriticalSection(&m_cs);
	}
}

void ntAvatarChatTask::DeleteBurdenPet(std::list<ntAvatarBurden>& dst,
	std::list<ntAvatarBurden>& src)
{
	EnterCriticalSection(&m_cs);
	for (std::list<ntAvatarBurden>::iterator srcIt = src.begin();
		srcIt != src.end(); ++srcIt)
		dst.push_back(*srcIt);
	src.clear();
	LeaveCriticalSection(&m_cs);

	std::list<ntAvatarBurden>::iterator it = dst.begin();
	while (it != dst.end())
	{
		if ((*it).m_pPet && (*it).m_pPet != (CPetFrame*)-1)
			delete (*it).m_pPet;
		it = dst.erase(it);
	}
}

void ntAvatarChatTask::KeepUniqueCompleteList(const ntAvatarBurden& burden)
{
	std::list<ntAvatarBurden>::iterator it;
	it = std::find(m_completeList.begin(), m_completeList.end(), burden);
	if (it != m_completeList.end())
	{
		m_removeList.push_back(*it);
		m_completeList.erase(it);
	}
}

extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
// D486 (diagnostico da B, B1176 golfplayer.cpp): watchpoint de hardware (Dr0, escrita, 4 bytes): loga EIP, valor e pilha de cada escrita. D629: o Lounge nao arma mais
static volatile LONG s_wpHits = 0;
static void* s_wpAddr = 0;
static LONG CALLBACK RbD486Veh(PEXCEPTION_POINTERS ep)
{
	if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP && (ep->ContextRecord->Dr6 & 1))
	{
		ep->ContextRecord->Dr6 = 0;
		if (InterlockedIncrement(&s_wpHits) <= 40)
		{
			void* frames[10];
			typedef WORD (WINAPI* CaptureFn)(DWORD, DWORD, void**, DWORD*);
			CaptureFn capture = (CaptureFn)GetProcAddress(GetModuleHandleA("kernel32.dll"), "RtlCaptureStackBackTrace");
			int n = capture ? capture(0, 10, frames, NULL) : 0;
			char line[400];
			int len = _snprintf(line, 120, "D486 wp %p val=%08x eip=%08x tid=%u |", s_wpAddr, *(DWORD*)s_wpAddr, ep->ContextRecord->Eip, GetCurrentThreadId());
			for (int i = 0; i < n && len > 0 && len < 360; i++)
				len += _snprintf(line + len, sizeof(line) - len, " %p", frames[i]);
			jp983_rastro("%s", line);
		}
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	return EXCEPTION_CONTINUE_SEARCH;
}
struct RbD486Arg { HANDLE th; DWORD dr7; DWORD addr; };
static void RbD486SetCtx(HANDLE th, RbD486Arg* a)
{
	SuspendThread(th);
	CONTEXT c;
	c.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	if (GetThreadContext(th, &c))
	{
		c.Dr0 = a->addr;
		c.Dr6 = 0;
		c.Dr7 = a->dr7;
		SetThreadContext(th, &c);
	}
	ResumeThread(th);
}
static volatile LONG s_wpThreads = 0;
static DWORD WINAPI RbD486Setter(LPVOID p)
{
	RbD486Arg* a = (RbD486Arg*)p;
	// B1178: arma o Dr0 em TODAS as threads do processo (o escritor do clubInfo.tid pode nao ser a thread do jogo)
	typedef HANDLE (WINAPI* SnapFn)(DWORD, DWORD);
	typedef BOOL (WINAPI* ThFn)(HANDLE, LPTHREADENTRY32);
	typedef HANDLE (WINAPI* OpenThFn)(DWORD, BOOL, DWORD);
	HMODULE k32 = GetModuleHandleA("kernel32.dll");
	SnapFn snap = (SnapFn)GetProcAddress(k32, "CreateToolhelp32Snapshot");
	ThFn first = (ThFn)GetProcAddress(k32, "Thread32First");
	ThFn next = (ThFn)GetProcAddress(k32, "Thread32Next");
	OpenThFn openTh = (OpenThFn)GetProcAddress(k32, "OpenThread");
	LONG n = 0;
	HANDLE s = (snap && first && next && openTh) ? snap(TH32CS_SNAPTHREAD, 0) : INVALID_HANDLE_VALUE;
	if (s != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 te;
		te.dwSize = sizeof(te);
		for (BOOL ok = first(s, &te); ok; ok = next(s, &te))
		{
			if (te.th32OwnerProcessID != GetCurrentProcessId() || te.th32ThreadID == GetCurrentThreadId())
				continue;
			HANDLE th = openTh(0x0002 | 0x0008 | 0x0010 | 0x0040, FALSE, te.th32ThreadID); // SUSPEND_RESUME|GET/SET_CONTEXT|QUERY
			if (!th)
				continue;
			RbD486SetCtx(th, a);
			CloseHandle(th);
			n++;
		}
		CloseHandle(s);
	}
	if (!n)
		RbD486SetCtx(a->th, a);
	s_wpThreads = n;
	return 0;
}
void RbD486Watch(void* addr, bool on)
{
	static bool veh = false;
	if (!veh)
	{
		typedef PVOID (WINAPI* AddVehFn)(ULONG, PVECTORED_EXCEPTION_HANDLER);
		AddVehFn addVeh = (AddVehFn)GetProcAddress(GetModuleHandleA("kernel32.dll"), "AddVectoredExceptionHandler");
		if (addVeh) addVeh(1, RbD486Veh);
		veh = true;
	}
	if (!on && s_wpAddr && addr != s_wpAddr)
	{
		// B1178: o dtor do ntAvatarChatTask desarmava o Dr0 ja rearmado no clubInfo.tid pelo RegisterClub
		jp983_rastro("D486 watch off ignorado %p (armado %p)", addr, s_wpAddr);
		return;
	}
	s_wpAddr = addr;
	if (on) s_wpHits = 0; // B1176: rearmar na partida (o Lounge pode ter gasto as 40 linhas)
	RbD486Arg a;
	a.th = 0;
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &a.th, 0, FALSE, DUPLICATE_SAME_ACCESS);
	a.dr7 = on ? 0xD0001 : 0;
	a.addr = (DWORD)addr;
	HANDLE h = CreateThread(NULL, 0, RbD486Setter, &a, 0, NULL);
	WaitForSingleObject(h, 2000);
	CloseHandle(h);
	CloseHandle(a.th);
	jp983_rastro("D486 watch %s %p threads=%d main=%u", on ? "on" : "off", addr, (int)s_wpThreads, GetCurrentThreadId());
}

void ntAvatarChatTask::Process(float delta)
{
	if (g_input->GetDown("ESCAPE", true) == 1 &&
		!g_pFresh->GetManager()->HasEscKeyWindow())
	{
		if (Doc()->m_stateTrade != 0)
		{
			CChatMsg::m_pInstance->AddChatMsg(
				std::string(
					K2L_Compatibility("\xbb\xf3\xc1\xa1 \xc0\xcc\xbf\xeb\xc1\xdf\xbf\xa1\xb4\xc2 \xb4\xeb\xc8\xad\xb9\xe6\xbf\xa1\xbc\xad \xb3\xaa\xb0\xa5\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.")),
				0xffff00, true, false);
		}
		else
		{
			ShowQuitDialog();
		}
	}

	std::list<ntAvatarBurden> list;
	EnterCriticalSection(&m_cs);
	for (std::list<ntAvatarBurden>::iterator srcIt = m_completeList.begin();
		srcIt != m_completeList.end(); ++srcIt)
		list.push_back(*srcIt);
	m_completeList.clear();
	LeaveCriticalSection(&m_cs);

	for (std::list<ntAvatarBurden>::iterator it = list.begin();
		it != list.end(); ++it)
	{
		ntAvatarBurden& burden = *it;
		ntAvatar* pAvatar =
			ntAvatarManager::m_pInstance->FindAvatarByGuid(burden.m_guid);
		if (pAvatar)
		{
			if (pAvatar->m_pPetFrame)
			{
				RemoveAvatarBurden(ntAvatarBurden(pAvatar->m_guid,
					pAvatar->m_key, pAvatar->m_charInfo, pAvatar->m_pPetFrame));
				pAvatar->m_pPetFrame = NULL;
			}
			pAvatar->m_burden = burden;
			pAvatar->m_bBurdenLoaded = true;
		}
		else
		{
			RemoveAvatarBurden(burden);
		}
	}

	CTask::Process(delta);
	g_pFresh->OnProcess(delta);
	CChatMsg::m_pInstance->ProcessWindow(delta);

	FrWnd* pFocus = g_pFresh->GetManager()->GetKeyFocused();
	g_pFresh->GetManager()->SetExclusiveKey(
		pFocus && pFocus->IsVisible() ? true : false);
}

void ntAvatarChatTask::OnAnotherHand(int key, void* pParam)
{
	// HACK: retain the original singleton COMDAT without changing the worker bytes.
	if (0)
		ntAvatarManager::IsInstantiated();
	std::list<ntAvatarBurden> list;

	while (!m_bQuit)
	{
		EnterCriticalSection(&m_cs);
		for (std::list<ntAvatarBurden>::iterator srcIt = m_addList.begin();
			srcIt != m_addList.end(); ++srcIt)
			list.push_back(*srcIt);
		m_addList.clear();
		LeaveCriticalSection(&m_cs);

		if (list.size() > 1)
		{
			list.reverse();
			list.sort();
			list.unique();
		}

		std::list<ntAvatarBurden>::iterator it = list.begin();
		while (!m_bQuit && it != list.end())
		{
			if ((*it).m_pPet == (CPetFrame*)-1)
			{
				(*it).m_pPet = NULL;
				EnterCriticalSection(&m_cs);
				KeepUniqueCompleteList(*it);
				LeaveCriticalSection(&m_cs);
				unsigned long guid = (*it).m_guid;
				it = list.erase(it);
				// D516: depois do erase o it pode ser end(); (*end()) le o no sentinela (lixo do heap) e o erase(end()) quebra a lista
				if (it != list.end() && (*it).m_guid == guid)
					it = list.erase(it);
			}
			else
			{
				++it;
			}
		}

		if (ntAvatarManager::m_pInstance->m_pMyAvatar)
		{
			it = list.begin();
			while (!m_bQuit && it != list.end())
			{
				ntAvatarBurden& burden = *it;
				ntAvatar* pAvatar =
					ntAvatarManager::m_pInstance->FindAvatarByGuid(
						burden.m_guid);
				if (ntAvatarManager::m_pInstance->m_pMyAvatar == pAvatar)
				{
					if (burden.Load())
					{
						EnterCriticalSection(&m_cs);
						KeepUniqueCompleteList(burden);
						m_completeList.push_back(burden);
						LeaveCriticalSection(&m_cs);
					}
					it = list.erase(it);
					g_LoadParam.current = 1;
					g_LoadParam.bFlagD = true;
					if (!g_SALoadingState)
						g_SALoadingState = true;
				}
				else
				{
					++it;
				}
			}
		}

		it = list.begin();
		while (!m_bQuit && it != list.end())
		{
			ntAvatarBurden& burden = *it;
			if (burden.Load())
			{
				EnterCriticalSection(&m_cs);
				KeepUniqueCompleteList(burden);
				m_completeList.push_back(burden);
				LeaveCriticalSection(&m_cs);
				if (!m_bQuit)
					Sleep(15);
			}
			it = list.erase(it);
		}

		DeleteBurdenPet(list, m_removeList);
		if (m_bQuit)
			break;
		Sleep(500);
	}

	DeleteBurdenPet(list, m_removeList);
	DeleteBurdenPet(list, m_completeList);
}

// 983: ntAvatar::m_pos fica em +0x164 = +0xfc do nosso (sem a base AvatarBase de 0x68, A224); o +0x164 cru caia no
// m_burden.m_info (D618: o filtro de 256 do chat 0x40 media lixo)
static inline const WVector& AvatarPos983(const ntAvatar* p)
{
	return p->m_pos;
}

int ntAvatarChatTask::OnPacket(WReceivedPacket& packet)
{
	unsigned short type = packet.Decode2();

	g_iRecvPacketType = type;

	switch (type)
	{
	case 0x48:
	{
		static sSlotInfo slotInfo;
		static sCharacterInfo charInfo;

		unsigned char sub = packet.Decode1();
		unsigned short index = packet.Decode2();
		if (index != 0xffff)
			return 1;

		std::list<sSlotInfo>* pList = &Doc()->m_slotList;

		switch (sub)
		{
		case 0:
		case 5:
		{
			if (sub == 0)
				pList->clear();

			int count = packet.Decode1();
			for (; count > 0; count--)
			{
				packet.DecodeBuffer(&slotInfo, sizeof(sSlotInfo));	// 983: 0x15c (era 0x152 do 645)
				packet.DecodeBuffer(&charInfo, sizeof(sCharacterInfo));	// 983: 0x201 (era 0x1bc do 645)
				if (IsLocalContent(S4_UCC))
					RequestUccInfo(charInfo);
				pList->push_back(slotInfo);
				GetActor("AvatarChat") << MsgObject(NULL, M983(0xe4), (int)&slotInfo,
					(int)&charInfo, 0, 0, 0);
			}
			break;
		}

		case 7:
		{
			packet.Decode1();
			packet.DecodeBuffer(&slotInfo, sizeof(sSlotInfo));	// 983: 0x15c (era 0x152 do 645)
			packet.DecodeBuffer(&charInfo, sizeof(sCharacterInfo));	// 983: 0x201 (era 0x1bc do 645)
			if (IsLocalContent(S4_UCC))
				RequestUccInfo(charInfo);
			std::list<sSlotInfo>::iterator it;
			it = std::find(pList->begin(), pList->end(), slotInfo.dwGuid);
			if (it == pList->end())
			{
				pList->push_back(slotInfo);
				GetActor("AvatarChat") << MsgObject(NULL, M983(0xe4), (int)&slotInfo,
					(int)&charInfo, 0, 0, 0);
			}
			break;
		}

		case 1:
		{
			packet.DecodeBuffer(&slotInfo, sizeof(sSlotInfo));	// 983: 0x15c (era 0x152 do 645)
			packet.DecodeBuffer(&charInfo, sizeof(sCharacterInfo));	// 983: 0x201 (era 0x1bc do 645)
			if (IsLocalContent(S4_UCC))
				RequestUccInfo(charInfo);
			std::list<sSlotInfo>::iterator it;
			it = std::find(pList->begin(), pList->end(), slotInfo.dwGuid);
			if (it == pList->end())
			{
				pList->push_back(slotInfo);
				GetActor("AvatarChat") << MsgObject(NULL, M983(0xe4), (int)&slotInfo,
					(int)&charInfo, 0, 0, 0);
			}
			break;
		}

		case 2:
		{
			unsigned long guid = packet.Decode4();
			std::list<sSlotInfo>::iterator it;
			it = std::find(pList->begin(), pList->end(), guid);
			if (it != pList->end())
			{
				pList->erase(it);
				GetActor("AvatarChat")
					<< MsgObject(NULL, M983(0xe5), guid, 0, 0, 0, 0);
			}
			break;
		}

		case 3:
		{
			unsigned long guid = packet.Decode4();
			packet.DecodeBuffer(&slotInfo, sizeof(sSlotInfo));	// 983: 0x15c (era 0x152 do 645)
			std::list<sSlotInfo>::iterator it;
			it = std::find(pList->begin(), pList->end(), guid);
			if (it != pList->end())
			{
				GetActor("AvatarChat")
					<< MsgObject(NULL, M983(0xe6), guid, (int)&slotInfo, 0, 0, 0);
			}
			break;
		}
		}

		Doc()->m_roomInfo.nUserNum = (unsigned char)pList->size();
		break;
	}

	case 0x7c:
	{
		unsigned long guid = packet.Decode4();
		unsigned short value = packet.Decode2();
		GetActor("AvatarChat") << MsgObject(NULL, M983(0x5f), guid, value, 0, 0, 0);
		break;
	}

	case 0x81:
	{
		std::string text;
		packet.DecodeStr(text);
		text +=
			K2L_Compatibility("\xb4\xd4\xc0\xbb \xb0\xad\xc1\xa6 \xc5\xf0\xc0\xe5\xbd\xc3\xc5\xb0\xb7\xc1 \xc7\xd5\xb4\xcf\xb4\xd9.\n\xc2\xf9\xbc\xba\xc7\xcf\xbd\xca\xb4\xcf\xb1\xee?");
		unsigned long guid = packet.Decode4();
		GetActor("AvatarChat")
			<< MsgObject(NULL, M983(0x31), (int)text.c_str(), guid, 0, 0, 0);
		break;
	}

	case 0x41:
	{
		std::string text;
		packet.DecodeStr(text);
		GetActor("avatarChatMain")
			<< MsgObject(NULL, M983(0x3), (int)text.c_str(), 0xffff7878, 0, 0, 0);
		break;
	}

	case 0x40:
	{
		std::string name;
		std::string msg;
		unsigned char kind = packet.Decode1();
		std::string text;
		bool bSelf = kind >> 7;
		bool bBang = false;
		bool bFlag3 = false;
		unsigned long color = 0;
		kind &= 0x7f;

		packet.DecodeStr(name);
		packet.DecodeStr(msg);
		Doc()->m_chatManager.FilteringHack(msg.c_str(), false);

		switch (kind)
		{
		case 0:
		{
			ntAvatar* pAvatar =
				ntAvatarManager::Instance()->FindAvatarByNickName(name.c_str());
			if (pAvatar && ntAvatarManager::Instance()->m_pMyAvatar &&
				(AvatarPos983(pAvatar) -
					AvatarPos983(ntAvatarManager::Instance()->m_pMyAvatar))
						.Magnitude() > 256.0f)
				return 1;

			if (strcmp(name.c_str(), Doc()->m_myInfo.info.sNick) == 0)
			{
				color = 0xffffcc00;
			}
			else
			{
				std::list<std::string>::iterator ignoreIt =
					std::find(Doc()->m_ignoreList.begin(),
						Doc()->m_ignoreList.end(), name);
				if (ignoreIt != Doc()->m_ignoreList.end())
					return 1;
				color = 0xffffffff;
			}

			if (*msg.c_str() == '!')
				bBang = true;

			pAvatar =
				ntAvatarManager::Instance()->FindAvatarByNickName(name.c_str());
			if (pAvatar)
				pAvatar->SetBalloonText(msg.c_str());

			bFlag3 = true;
			if (bSelf)
			{
				color = 0xff6eecfb;
			}
			else
			{
				pAvatar = ntAvatarManager::Instance()->FindAvatarByNickName(
					name.c_str());
				if (pAvatar)
					pAvatar->SetBalloonText(msg.c_str());

				// 983 (007b5a20): frase com comando do avatar ("/" + comando, 004d0b10) vai como 0x115;
				// senao 0x6d, e nada se o autor e o proprio jogador. Argumentos: (nome, frase).
				unsigned int cmdIndex = 0;
				if (CreateCharacterOption::Instance()->FindSetCommand983(msg.c_str(), &cmdIndex))
				{
					GetActor("AvatarChat") << MsgObject(NULL, 0x115,
						(int)name.c_str(), (int)msg.c_str(), 0, 0, 0);
				}
				else if (strcmp(name.c_str(), Doc()->m_myInfo.info.sNick) != 0)
				{
					GetActor("AvatarChat") << MsgObject(NULL, M983(0x60),
						(int)name.c_str(), (int)msg.c_str(), 0, 0, 0);
				}
			}

			text = name + " : \b" + msg;
			break;
		}

		case 1:
			text = K2L_Compatibility("\xbe\xcb\xb8\xb2 : ") + name +
				K2L_Compatibility(" \xb4\xd4\xc0\xba \xc7\xf6\xc0\xe7 \xc3\xa4\xb3\xce\xbf\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			color = 0xffff7878;
			break;

		case 5:
			text = K2L_Compatibility("\xbe\xcb\xb8\xb2 : ") + name +
				K2L_Compatibility(" \xb4\xd4\xc0\xba \xc7\xf6\xc0\xe7 \xbc\xad\xb9\xf6\xbf\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			color = 0xffff7878;
			break;

		case 2:
			text = K2L_Compatibility("\xbe\xcb\xb8\xb2 : ") + name +
				K2L_Compatibility(" \xb4\xd4\xc0\xcc \xc5\xf0\xc0\xe5\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9.");
			color = 0xffff7878;
			break;

		case 3:
			text =
				K2L_Compatibility("\xb0\xad\xc5\xf0\xbf\xa1 \xb0\xfc\xc7\xd1 \xc5\xf5\xc7\xa5\xb0\xa1 \xba\xce\xb0\xe1\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.");
			color = 0xffff7878;
			break;

		case 4:
			text = K2L_Compatibility("\xbe\xcb\xb8\xb2 : ") + name +
				K2L_Compatibility(" \xb4\xd4\xc0\xba \xc7\xf6\xc0\xe7 \xb1\xd3\xbc\xd3\xb8\xbb\xc0\xbb \xbc\xf6\xbd\xc5\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			color = 0xffff7878;
			break;

		case 6:
			text = K2L_Compatibility("\xbe\xcb\xb8\xb2 : ") + name +
				K2L_Compatibility(" \xb4\xd4\xc0\xba \xc7\xf6\xc0\xe7 \xc1\xa2\xbc\xd3\xc1\xdf\xc0\xcc \xbe\xc6\xb4\xd5\xb4\xcf\xb4\xd9.");
			color = 0xffff7878;
			break;

		case 7:
			text = K2L_Compatibility("\xbe\xcb\xb8\xb2 : ") + msg;
			color = 0xffff7878;
			break;

		case 10:
		{
			unsigned long count = packet.Decode4();
			text = K2L_Compatibility("\xbe\xcb\xb8\xb2 : ") +
				std::string(MakeStr(
					K2L_Compatibility("%s\xb4\xd4\xb2\xb2\xbc\xad %d \xc6\xce \xc1\xd6\xb8\xd3\xb4\xcf\xb8\xa6 \xc8\xb9\xb5\xe6\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."),
					name.c_str(), count));
			color = 0xffff7878;
			break;
		}

		default:
			text = name + " : " + msg;
			break;
		}

		if (kind != 0 || (!bBang && Doc()->m_pGolfDoc->m_bShowChat))
			CChatMsg::Instance()->AddChatMsg(text, color, true, bFlag3);
		break;
	}

	case 0x78:
	{
		unsigned long guid = packet.Decode4();
		unsigned char value = packet.Decode1();
		GetActor("AvatarChat") << MsgObject(NULL, M983(0x29), guid, value, 0, 0, 0);
		break;
	}

	case 0x84:
	{
		unsigned char flags = packet.Decode1();
		unsigned char high = flags >> 7;
		flags &= 0x7f;
		bool bSent = flags == 0;
		unsigned long color = 0xffd2a2ff;
		if (high)
			color = 0xff6eecfb;

		std::string name;
		std::string msg;
		std::string text;
		packet.DecodeStr(name);
		packet.DecodeStr(msg);

		if (bSent)
		{
			text = Doc()->m_myInfo.info.sNick;
			text += ">";
			text += name;
		}
		else
		{
			if (!COption::Instance()->gIsWhisperAllowed())
				break;

			std::list<std::string>::iterator ignoreIt = std::find(
				Doc()->m_ignoreList.begin(), Doc()->m_ignoreList.end(), name);
			if (ignoreIt != Doc()->m_ignoreList.end())
				break;

			unsigned long dummy;
			GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0x3f), (int)&dummy, 0, 0, 0, 0);
			GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0x38), (int)name.c_str(), 0, 0, 0, 0);
			text = name + ">" + Doc()->m_myInfo.info.sNick;
		}

		Doc()->m_chatManager.FilteringHack(msg.c_str(), false);
		text += " : \b'";
		text += msg;
		text += "'";

		if (AfxGetTask()->IsRecieveWhisper())
		{
			CChatMsg::Instance()->AddChatMsg(text, color, true, true);
		}
		else
		{
			WSendPacket sendPacket(0xde);
			sendPacket.EncodeStr(name);
			sendPacket.Send((eSendTo)0);
		}
		break;
	}

	case 0x8e:
	{
		unsigned long guid = packet.Decode4();
		unsigned char value = packet.Decode1();
		GetActor("AvatarChat") << MsgObject(NULL, M983(0x3d), guid, value, 0, 0, 0);
		break;
	}

	case 0x94:
	{
		CGolfDoc* pGolfDoc = Doc()->m_pGolfDoc;
		pGolfDoc->m_bStopRequest = packet.Decode1() == 1; // 983 007b5a20: +0x191 (o cru +0x17d do 645 caia no m_unused169)
		if (!Doc()->m_pGolfDoc->m_bReport) // 983 +0x15f (o cru +0x14f do 645 era m_cameraData)
			break;

		if (Doc()->m_pGolfDoc->m_bStopRequest)
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetMessage(
				K2L_Compatibility("\xc7\xf6\xc0\xe7 \xb0\xd4\xc0\xd3\xbf\xa1 \xb4\xeb\xc7\xd8\xbc\xad \xc0\xcc\xb9\xcc \xbd\xc5\xb0\xed\xb0\xa1 \xc1\xa2\xbc\xf6\xb5\xc7\xbe\xee \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			pForm->Open(NULL, 4);
			Doc()->m_pGolfDoc->m_bReport = false;
		}
		else
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetMessage(
				K2L_Compatibility("\xbd\xc5\xb0\xed\xb0\xa1 \xc1\xa4\xbb\xf3\xc0\xfb\xc0\xb8\xb7\xce \xc1\xa2\xbc\xf6 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.\n\n\x09\x09\x09\x09\x09\x09\x09\x09\x09  \xc6\xaf\xc1\xa4 \xbb\xf3\xb4\xeb\xc0\xc7 \xb4\xeb\xc8\xad\xb8\xa6 \xc2\xf7\xb4\xdc \xc7\xcf\xb7\xc1\xb8\xe9 \xbb\xf3\xb4\xeb \xc1\xa4\xba\xb8\xc3\xa2\xc0\xbb \xb8\xb6\xbf\xec\xbd\xba \xb4\xf5\xba\xed \xc5\xac\xb8\xaf \xc8\xc4 \\c0xffff0000\\c[\xb4\xeb\xc8\xad \xc2\xf7\xb4\xdc \xbe\xc6\xc0\xcc\xc4\xdc]\\c0xff000000\\c\xc0\xbb \xbc\xb1\xc5\xc3 \xc7\xcf\xbd\xc3\xb8\xe9 \xb5\xcb\xb4\xcf\xb4\xd9."),
				false);
			pForm->Open(NULL, 0);
		}
		break;
	}

	case 0x9e:
	{
		unsigned char weather = packet.Decode1();
		packet.Decode1();
		packet.Decode1();
		CSceneManager::Instance()->SetWeather(weather, false);
		break;
	}

	case 0xc4:
	{
		unsigned long id = packet.Decode4();
		unsigned char sub = packet.Decode1();

		switch (sub)
		{
		case 4:
		{
			unsigned long a;
			unsigned long b;
			unsigned long c;
			packet.DecodeBuffer(&a, 4);
			packet.DecodeBuffer(&b, 4);
			packet.DecodeBuffer(&c, 4);
			GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0x5b), id, sub, (int)&a, (int)&b, (int)&c);
			break;
		}

		case 5:
		{
			unsigned long value = packet.Decode4();
			GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0x5b), id, sub, value, 0, 0);
			break;
		}

		case 6:
		{
			unsigned long a;
			unsigned long b;
			unsigned long c;
			packet.DecodeBuffer(&a, 4);
			packet.DecodeBuffer(&b, 4);
			packet.DecodeBuffer(&c, 4);
			GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0x5b), id, sub, (int)&a, (int)&b, (int)&c);
			break;
		}

		case 7:
		case 10: // 983 007b6b52: mesmo corpo do 7 (movimento de conjunto)
		{
			std::string text;
			packet.DecodeStr(text);
			GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0x5b), id, sub, (int)text.c_str(), 0, 0);
			break;
		}

		case 8:
		{
			unsigned long value = packet.Decode4();
			GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0x5b), id, sub, value, 0, 0);
			break;
		}

		case 9:
			GetActor("AvatarChat") << MsgObject(NULL, M983(0x5b), id, sub, 0, 0, 0);
			break;
		}
		break;
	}

	case 0x191:
		break;

	case 0x196:
	{
		unsigned long guid = packet.Decode4();
		ntAvatar* pAvatar =
			ntAvatarManager::m_pInstance->FindAvatarByGuid(guid);
		if (!pAvatar)
			break;

		eSPAVILITYTYPE spType = (eSPAVILITYTYPE)0;
		do
		{
			SpecialPrizeItem& item = pAvatar->GetMySpItem(spType);
			unsigned long value = 0;
			packet.DecodeBuffer(&value, 4);
			spType = (eSPAVILITYTYPE)(spType + 1);
			item.m_value = value;
		} while ((int)spType < 5);
		break;
	}

	default:
		return OnPacketCommon(packet);
	}

	return 1;
}
