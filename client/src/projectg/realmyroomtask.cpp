#include "minatl.h"
#include "vc10_layout.h"
#include "realmyroomtask.h"
#include "partsinfo.h"
#include "wlist.h"
class cModel;
void RequestUccInfo(sMailIncludeItem& item);
#include "actor.h"
#include "avatar_task.h"
#include "clientsetting.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frform.h"
#include "frdesktop.h"
#include "quitwindow.h"
#include "mousecursor.h"
#include "frgaugebar.h"
#include "shareddoc.h"
#include "../shared/globalnetworkdefine.h"
#include "s5/utilities.h"
#include "guildactor.h"
#include "gatewayactor.h"
#include "projectg.h"
#include "../shared/localize.h"
#include "jp/myroomwidget.h"

// The original retains these math constants in the unit's plain .rdata.
static bool IsValidRange(float value)
{
	return value < g_HUGE && value > g_EPSILON;
}

#ifndef REBANG_SEMANTIC_ONLY
// HACK: the linker keeps these three literals in avatar_task.obj /
// realmyroommain.obj, whose naked functions hold them as local statics.
// A literal emitted here would be picked instead and move .rdata.
// clang-format off
extern "C" const char rb_strLoadingXml[];
extern "C" const char rb_strLoading[];
extern "C" const char rb_strItemBuff[];
#pragma comment(linker, "/alternatename:_rb_strLoadingXml=?rb_str_NBFBDAF@?3??PreLoadInit@ntAvatarChatTask@@MAEXXZ@4QBDB")
#pragma comment(linker, "/alternatename:_rb_strLoading=?rb_C_07BMPLLLFL_LOADING_AA@?3??PreLoadInit@ntAvatarChatTask@@MAEXXZ@4QBDB")
#pragma comment(linker, "/alternatename:_rb_strItemBuff=?rb_C_08CPBGJOJF_ItemBuff_AA@?2??OnProcess@CRealMyRoomMain@@UAEXM@Z@4QBDB")
// clang-format on
#else
static const char rb_strLoadingXml[] = "loading.xml";
static const char rb_strLoading[] = "LOADING";
static const char rb_strItemBuff[] = "ItemBuff";
#endif

extern Fresh* g_pFresh;
extern int g_iRecvPacketType;

struct sLoadParam
{
	unsigned char m_unused0[4];
	int m_nState;
	int m_nLoading;
	unsigned char m_unusedC;
	bool m_bLoaded;
};

extern sLoadParam g_LoadParam;

IMPLEMENT_OBJECT(CRealMyRoomTask, CTask)

CRealMyRoomTask::CRealMyRoomTask()
	: m_bExit(false)
{
	InitializeCriticalSection(&m_csBurden);

	new CRealMyRoom_AvatarManager;
	new CRealMyRoomData;
	new CCameraWork;
	if (!CTransferableItem::IsInstantiated())
		new CTransferableItem; // o 983 o cria em CreateSingletonClasses
	if (IsLocalContent(S4_ITEM_STORAGE))
		new CItemStorage;

	IActor::m_defaultSkipLayer = 0;
	IActor::m_pLayer = NULL;
}

CRealMyRoomTask::~CRealMyRoomTask()
{
	COption::Instance()->gSetAvatarNewBie(0); // 645 +0x1ac (983 +0x1d4)
	m_bExit = true;
	m_anotherHand.WaitUntilWorkDone(9999);
	m_anotherHand.BreakWorkingThread();
	DeleteCriticalSection(&m_csBurden);

	if (CRealMyRoom_AvatarManager::IsInstantiated())
		delete CRealMyRoom_AvatarManager::Instance();
	if (CRealMyRoomData::IsInstantiated())
		delete CRealMyRoomData::Instance();
	delete CCameraWork::Instance();
	if (IsLocalContent(S4_ITEM_STORAGE))
		delete CItemStorage::Instance();
}

void CRealMyRoomTask::Register()
{
	SetMainActor(AddActor("CRealMyRoomMain", "RealMyRoom", 0, ""));
	AddActor("CCamera", "Camera", 0x20, "");
	AddActor("CItemBuff", rb_strItemBuff, 0, "");
	AddActor(s_guildActor[0], s_guildActor[1], 0, "");
	AddActor(s_gatewayActor[0], s_gatewayActor[1], 0, GetRTTI()->GetName());

	if (AfxGetPreservedTask())
	{
		IActor* pTutorial = AfxGetPreservedTask()->GetActor("TutorialMain");
		if (pTutorial)
		{
			const char* className = NULL;
			const char* actorName = NULL;
			pTutorial << MsgObject(0, M983(0x14e), (int)&className, (int)&actorName,
				0, 0, 0);
			AddActor(className, actorName, 0, "");
		}
	}
}

void CRealMyRoomTask::PreLoadInit()
{
	sActor* pActor = m_actorList.Start();
	while (pActor)
	{
		pActor->actor->OnPreLoadInit();
		pActor = m_actorList.Next();
	}

	if (!IS_EXACTKINDOF(ntAvatarChatTask,
			CTaskManager::Instance()->GetPreservedTask()))
		new CQuitWindow;

	SetGameType(8);

	pActor = m_actorList.Start();
	while (pActor)
	{
		pActor->actor->OnLoad();
		pActor = m_actorList.Next();
	}

	pActor = m_actorList.Start();
	while (pActor)
	{
		pActor->actor->OnInit();
		pActor = m_actorList.Next();
	}

	if (!IsInited())
	{
		// through a reference: the original sets the bit in memory instead of
		// reusing the value IsInited() loaded
		unsigned long& flag = m_taskFlag;
		flag |= 2;
		OnInitFinished();
	}

	g_LoadParam.m_nState = 1;
	g_LoadParam.m_nLoading = 0;
	if (m_pProgressGauge)
		m_pProgressGauge->SetRange(0, 100, 0);

	m_pLoadingFresh = new Fresh;
	m_pProgressGauge = NULL;
	if (m_loadingBackGround != "")
		m_pLoadingFresh->Init(rb_strLoadingXml, false,
			m_loadingBackGround.c_str());
	else
		m_pLoadingFresh->Init(rb_strLoadingXml, false, NULL);
	m_pLoadingFresh->OpenLayout(rb_strLoading, NULL, true);
	if (CMouseCursor::Instance())
	{
		CMouseCursor::Instance()->SetActive(true);
		CMouseCursor::Instance()->SetMode(2);
	}
}

void CRealMyRoomTask::Load()
{
	m_anotherHand.Work(this, 0, NULL, false, 0);
	g_LoadParam.m_nLoading = 1;
	g_LoadParam.m_bLoaded = true;
}

void CRealMyRoomTask::Init(const char* wallPaper)
{
	if (m_pLoadingFresh)
	{
		delete m_pLoadingFresh;
		m_pLoadingFresh = NULL;
	}

	g_pFresh->GetManager()->GetDesktop()->Init(g_view->GetWidth(),
		g_view->GetHeight());
	g_pFresh->GetManager()->GetDesktop()->SetWallPaper(wallPaper, true);

	SetWhisper(false);
}

void CRealMyRoomTask::OnAnotherHand(int key, void* pParam)
{
	typedef std::list<CRealMyRoom_AvatarBurden>::iterator BurdenIter;
	std::list<CRealMyRoom_AvatarBurden> work;

	while (!m_bExit)
	{
		EnterCriticalSection(&m_csBurden);
		for (BurdenIter it = m_burdenQueue.begin(); it != m_burdenQueue.end();
			++it)
			work.push_back(*it);
		m_burdenQueue.clear();
		LeaveCriticalSection(&m_csBurden);

		if (CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar)
		{
			// the local avatar's burdens go first
			for (BurdenIter it = work.begin(); !m_bExit && it != work.end();)
			{
				if (CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar ==
					CRealMyRoom_AvatarManager::Instance()->FindAvatarByGuid(
						(*it).m_guid))
				{
					if ((*it).LoadAvatarBurden())
					{
						EnterCriticalSection(&m_csBurden);
						m_burdenList.push_back(*it);
						LeaveCriticalSection(&m_csBurden);
					}
					it = work.erase(it);
				}
				else
					++it;
			}
		}

		for (BurdenIter it = work.begin(); !m_bExit && it != work.end();)
		{
			if ((*it).LoadAvatarBurden())
			{
				EnterCriticalSection(&m_csBurden);
				m_burdenList.push_back(*it);
				LeaveCriticalSection(&m_csBurden);
				if (!m_bExit)
					Sleep(15);
			}
			it = work.erase(it);
		}

		DeleteBurdenPet(work, m_burdenDelete);
		if (!m_bExit)
			Sleep(500);
	}

	DeleteBurdenPet(work, m_burdenDelete);
	DeleteBurdenPet(work, m_burdenList);
}

void CRealMyRoomTask::Process(float delta)
{
	typedef std::list<CRealMyRoom_AvatarBurden>::iterator BurdenIter;
	std::list<CRealMyRoom_AvatarBurden> loaded;

	EnterCriticalSection(&m_csBurden);
	BurdenIter pending;
	for (pending = m_burdenList.begin(); pending != m_burdenList.end();
		++pending)
		loaded.push_back(*pending);
	m_burdenList.clear();
	LeaveCriticalSection(&m_csBurden);

	for (BurdenIter it = loaded.begin(); it != loaded.end(); ++it)
	{
		// CRealMyRoom_Avatar layout: +0x148 burden, +0x3d8
		// loaded flag, +0x660 changed flag.
		CRealMyRoom_AvatarBurden& burden = *it;
		unsigned char* avatar =
			(unsigned char*)CRealMyRoom_AvatarManager::Instance()
				->FindAvatarByGuid(burden.m_guid);
		if (avatar)
		{
			if (*(int*)(avatar + 0x660) != 0)
			{
				*(int*)(avatar + 0x660) = 0;
				RemoveAvatarBurden(
					*(CRealMyRoom_AvatarBurden*)(avatar + 0x148));
			}
			*(CRealMyRoom_AvatarBurden*)(avatar + 0x148) = burden;
			avatar[0x3d8] = 1;
		}
		else
		{
			RemoveAvatarBurden(burden);
		}
	}

	CTask::Process(delta);
	g_pFresh->OnProcess(delta);

	FrWnd* focused = g_pFresh->GetManager()->GetKeyFocused();
	if (focused && focused->IsVisible())
		g_pFresh->GetManager()->SetExclusiveKey(true);
	else
		g_pFresh->GetManager()->SetExclusiveKey(false);
}

void CRealMyRoomTask::RestorePreserved()
{
	g_pFresh->GetManager()->GetDesktop()->SetViewFocus(true);

	SetGameType(2);

	float height = g_view->GetHeight();
	float width = g_view->GetWidth();
	g_pFresh->GetManager()->GetDesktop()->Init(width, height);
	g_pFresh->GetManager()->GetDesktop()->SetWallPaper(NULL, true);
	CTask::RestorePreserved();
}

void CRealMyRoomTask::Destroy()
{
	CTask::Destroy();

	if (!IS_EXACTKINDOF(ntAvatarChatTask,
			CTaskManager::Instance()->GetPreservedTask()))
		delete CQuitWindow::Instance();
}

void CRealMyRoomTask::Display()
{
	CTask::Display();
	if (g_pFresh->GetManager()->GetDesktop())
		g_pFresh->GetManager()->GetDesktop()->OnDisplay(true, false, true, true);
	g_pFresh->OnDisplay(true);
}

void CRealMyRoomTask::AddAvatarBurden(const CRealMyRoom_AvatarBurden& burden)
{
	EnterCriticalSection(&m_csBurden);
	m_burdenQueue.push_back(burden);
	LeaveCriticalSection(&m_csBurden);
}

void CRealMyRoomTask::RemoveAvatarBurden(const CRealMyRoom_AvatarBurden& burden)
{
	if (!m_bExit)
	{
		EnterCriticalSection(&m_csBurden);
		m_burdenDelete.push_back(burden);
		LeaveCriticalSection(&m_csBurden);
	}
}

void CRealMyRoomTask::DeleteBurdenPet(
	std::list<CRealMyRoom_AvatarBurden>& loaded,
	std::list<CRealMyRoom_AvatarBurden>& pending)
{
	typedef std::list<CRealMyRoom_AvatarBurden>::iterator BurdenIter;

	EnterCriticalSection(&m_csBurden);
	for (BurdenIter it = pending.begin(); it != pending.end(); ++it)
		loaded.push_back(*it);
	pending.clear();
	LeaveCriticalSection(&m_csBurden);

	for (BurdenIter it = loaded.begin(); it != loaded.end();)
	{
		if ((*it).m_pPetFrame)
			delete (*it).m_pPetFrame;
		it = loaded.erase(it);
	}
}

// Raw CSharedDoc offsets not yet named in shareddoc.h
#define DOC_I64(off) (*(__int64*)((char*)Doc() + (off)))
#define DOC_U32(off) (*(unsigned int*)((char*)Doc() + (off)))
// A734: crus 0x10d60/0x10d7c/0x10777 do CSharedDoc -> membros (no cl novo o STL do VC10 desloca)
C_ASSERT_VC10(offsetof(CSharedDoc, m_giftName) == 0x10d60);
C_ASSERT_VC10(offsetof(CSharedDoc, m_giftIndex) == 0x10d7c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_bTreasureResult) == 0x10777);
#define DOC_U8(off) (*(unsigned char*)((char*)Doc() + (off)))

// PKT (frente C, lote 11): os números de mensagem passados a estas duas macros eram os do KR 645 escritos direto (o
// tools/f35/msgaplica.py não os pegou) enquanto quem recebe (realmyroommainui.cpp) já usa M983(...); agora vão por M983.
// PKT (frente C, lote 12): no 983 (005a8360) nenhuma mensagem vai por GetActor("RealMyRoom") (o texto nem aparece no
// despachante): todas saem pela tarefa ao ator principal (vtable+0x30 = SendMsgToMainActor, 56 chamadas) - aqui o
// ator principal ja e o "RealMyRoom" (Register), entao o efeito e o mesmo e continua certo quando o my room virar widget.
#define SEND_REALMYROOM(msg, p1, p2, p3, p4, p5) \
	(AfxGetTask()->SendMsgToMainActor( \
		MsgObject(0, (msg), (p1), (p2), (p3), (p4), (p5))))

#define NOTIFY_FORM() \
	CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL)

#define SEND_TASK_REALMYROOM(msg, p1, p2, p3, p4, p5) \
	SEND_REALMYROOM(msg, p1, p2, p3, p4, p5)

// VC10: sem a especialização do construtor de std::pair do VC7.1

// HACK: The original reads these CSharedDoc fields through inline accessors
// (the scratch-register rotation of cases 0x13e and 0x181 only matches this
// way). dllimport + __forceinline: the only form that never emits an
// out-of-line copy (the real accessors belong to a PCH header).
extern "C" void _ReadWriteBarrier(void);
#pragma intrinsic(_ReadWriteBarrier)

__declspec(dllimport) __forceinline unsigned long GetMyUID()
{
	return Doc()->m_myInfo.info.dwUID;
}

__declspec(dllimport) __forceinline const char* GetMyLoginID()
{
	return Doc()->m_myInfo.info.sID;
}

// PKT (frente C, lote 11; C65/C68): objeto de 00e78ca8 = RFLobbyDesktop da frente B (include/jp/refresh/rflobbydesktop.h,
// guardado em CLobbyState983::ms_pInstance); 005fbe40 = RFLobbyDesktop::Refresh983 (corpo da B, B132).
// C72: visao local trocada pelo cabecalho da B (C68.1 atendido na B135).
#include "jp/refresh/rflobbydesktop.h"
// PKT (frente C, lote 11; C66): caso 0x129 do 983 (005a947d) - os premios da bolsa vao para a lista estatica de 005a8160
// (classe e corpos da C: include/packets/pouchprizelist983.h, src/projectg/jp/pktviews983.cpp).
#include "packets/pouchprizelist983.h"

// PKT (frente C, lote 39): no 983 nao ha CRealMyRoomTask (sem RTTI); este despachante e o [6] da vtable da CMyRoomWidget
// (005a8360; pedidos B52/B53). A widget e aberta por cima do lobby e o CTask::OnPacketCommon passa o pacote a ela antes de
// tudo (009c20d7): o que nao e do my room volta com 0 (005aabdd), sem o OnPacketCommon daqui. O `this` so e o dono dos
// "notify" (this + 8 = FrCmdTarget, como no 983 005a937b / 005aa2ca).
int CMyRoomWidget::OnPacket(WReceivedPacket& packet)
{
	packet.Reset();
	g_iRecvPacketType = packet.Decode2();

	switch (g_iRecvPacketType)
	{
	case 0x168: // JP 983 (s63): 0x16e no KR 645; caso 005a8860 (DB 0x15c + DB 0x201)
	{
		static sSlotInfo slotInfo;
		static sCharacterInfo charInfo;

		std::list<sSlotInfo>& slotList = Doc()->m_slotList;

		packet.DecodeBuffer(&slotInfo, sizeof(sSlotInfo));
		packet.DecodeBuffer(&charInfo, sizeof(sCharacterInfo));

		if (!IS_EXACTKINDOF(ntAvatarChatTask,
				CTaskManager::Instance()->GetPreservedTask()))
		{
			std::list<sSlotInfo>::iterator it = Doc()->m_slotList.begin();
			std::list<sSlotInfo>::iterator end = Doc()->m_slotList.end();
			for (; it != end; ++it)
			{
				if (sSlotInfo(*it).dwGuid == slotInfo.dwGuid)
					break;
			}
			if (it == Doc()->m_slotList.end())
				slotList.push_back(slotInfo);
		}

		SEND_REALMYROOM(M983(0xe4), (int)&slotInfo, (int)&charInfo, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0xb6):
	{
		unsigned char count = packet.Decode1();
		if (count >= 0x80)
			count = 0x80;

		unsigned long list[0x80];
		for (unsigned char i = 0; i < count; ++i)
			list[i] = packet.Decode4();

		SEND_REALMYROOM(M983(0xd5), (int)list, count, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x13d):
	{
		if (!IsLocalContent((localContentType_t)0x5c /*983: era 0x5b; interpolado*/))
			return 1;

		unsigned char flag1 = packet.Decode1();
		unsigned char flag2 = packet.Decode1();
		unsigned short count = packet.Decode2();
		for (unsigned short i = 0; i < count; ++i)
		{
			sGiftInfo gift;
			packet.DecodeBuffer(&gift, sizeof(gift));
			Doc()->m_chatManager.FilteringHack(gift.sMsg, false);
			Doc()->m_giftList.push_back(gift);
		}
		*((unsigned char*)&CSharedDoc::m_cardPackSlot) = flag1;
		*((unsigned char*)&CSharedDoc::m_cardPackItem) = flag2;
		*(unsigned short*)((char*)&CSharedDoc::m_rmrTabValue) = count;
		*((unsigned char*)&CSharedDoc::m_bRmrTabPending) = 1;
		break;
	}

	case PKT645_SEM_983(0x78):
	{
		unsigned char flag = packet.Decode1();
		unsigned short perPage = packet.Decode2();
		unsigned short pageCount = packet.Decode2();
		unsigned short total = packet.Decode2();

		if (flag == 1)
		{
			(*(unsigned int*)&CSharedDoc::m_cardPackPageNum) = perPage;
			(*(unsigned int*)&CSharedDoc::m_cardPackSelPageNum) = pageCount;
			(*(unsigned int*)&CSharedDoc::m_cardPackCurPage) = total;

			Doc()->m_giftList.clear();
			for (unsigned short i = 0; i < pageCount; ++i)
			{
				sGiftInfo gift;
				packet.DecodeBuffer(&gift, sizeof(sGiftInfo));
				Doc()->m_chatManager.FilteringHack(gift.sMsg, false);
				Doc()->m_giftList.push_back(gift);
			}

			*((unsigned char*)&CSharedDoc::m_bCardPackRebuild) = 1;
			return 1;
		}

		int pages = total / perPage;
		if (perPage * pages < total)
			++pages;

		if (pages)
		{
			WSendPacket send((enumClientPacket)0x95); // JP 983 (s63): 0x92 no KR 645
			send.Encode4(pages);
			send.Send(TO_GAME);
			return 1;
		}
		else
		{
			WSendPacket send((enumClientPacket)0x95); // JP 983 (s63): 0x92 no KR 645
			send.Encode4(1);
			send.Send(TO_GAME);
			return 1;
		}
	}

	case PKT645_SEM_983(0xda):
	{
		unsigned long guid = packet.Decode4();
		SEND_REALMYROOM(M983(0xd1), guid, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0xd1), guid, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x79):
	{
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0xc7), 0, 0, 0, 0, 0);

		switch (packet.Decode4())
		{
		case 1:
			SEND_REALMYROOM(M983(0x23),
				(int)"\xbc\xb1\xb9\xb0 \xbf\xc5\xb1\xe2\xb1\xe2\xb0\xa1 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9",
				0, 0, 0, 0);
			break;

		case 0:
		{
			unsigned long guid = packet.Decode4();
			int param1 = packet.Decode4();
			unsigned long typeId = packet.Decode4();
			unsigned short count = packet.Decode2();
			unsigned char extra = packet.Decode1();

			int msg;
			switch (typeId >> 26)
			{
			case 1:
				msg = M983(0xc8);
				break;
			case 2:
				msg = M983(0xca);
				break;
			case 7:
				msg = M983(0xcb);
				break;
			case 8:
				msg = M983(0xcc);
				count *= 0x18;
				break;
			case 4:
				msg = M983(0xcf);
				break;
			case 0xe:
				msg = M983(0xd0);
				break;
			case 5:
			case 6:
			case 0x1c:
			case 0x1d:
				msg = M983(0xce);
				break;
			case 0xf:
				msg = M983(0xc9);
				break;
			case 0x10:
				msg = M983(0xcd);
				break;
			case 0x1f:
				msg = M983(0xe0);
				break;
			}

			SEND_REALMYROOM(msg, param1, typeId, count, extra, 0);
			SEND_REALMYROOM(M983(0xd1), guid, 0, 0, 0, 0);
			break;
		}

		case 3:
			SEND_REALMYROOM(M983(0x23),
				(int)"\xc0\xdf\xb8\xf8\xb5\xc8 \xbb\xf3\xc7\xb0 \xc4\xda\xb5\xe5\xc0\xd4\xb4\xcf\xb4\xd9",
				0, 0, 0, 0);
			break;

		case 4:
			SEND_REALMYROOM(M983(0x23),
				(int)"\xc0\xcc\xb9\xcc \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xd4\xb4\xcf\xb4\xd9",
				0, 0, 0, 0);
			break;

		case 0xc:
			SEND_REALMYROOM(M983(0x23),
				(int)"\xc7\xd8\xb4\xe7 \xc4\xb3\xb5\xf0\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9",
				0, 0, 0, 0);
			break;

		case 0xe:
			SEND_REALMYROOM(M983(0x23),
				(int)"\xc7\xd8\xb4\xe7 \xc4\xb3\xb8\xaf\xc5\xcd\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9",
				0, 0, 0, 0);
			break;

		case 0xd:
			SEND_REALMYROOM(M983(0x23),
				(int)"\xc8\xde\xb0\xa1\xc1\xdf\xc0\xce \xc4\xb3\xb5\xf0\xc0\xd4\xb4\xcf\xb4\xd9. \xc0\xe7\xb0\xed\xbf\xeb \xc8\xc4 \xbc\xb1\xb9\xb0 \xbf\xc5\xb1\xe2\xb1\xe2\xb8\xa6 \xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4",
				0, 0, 0, 0);
			break;

		case 5:
		{
			unsigned long guid = packet.Decode4();
			unsigned char count = packet.Decode1();
			sBuyItemResult result;

			for (int i = 0; i < count; ++i)
			{
				packet.DecodeBuffer(&result, sizeof(sBuyItemResult));

				int msg;
				switch (result.Typeid >> 26)
				{
				case 1:
					msg = M983(0xc8);
					break;
				case 2:
					msg = M983(0xca);
					break;
				case 7:
					msg = M983(0xcb);
					break;
				case 4:
					msg = M983(0xcf);
					break;
				case 0xe:
					msg = M983(0xd0);
					break;
				case 5:
					msg = M983(0xce);
					break;
				case 6:
					msg = M983(0xce);
					break;
				case 0x1c:
					msg = M983(0xce);
					break;
				}

				SEND_REALMYROOM(msg, result.guid, result.Typeid, 0, 0, 0);
			}

			SEND_REALMYROOM(M983(0xd1), guid, 0, 0, 0, 0);
			break;
		}

		case 6:
			SEND_REALMYROOM(M983(0x23),
				(int)"\xbc\xb1\xb9\xb0\xc0\xcc \xc1\xb8\xc0\xe7\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9",
				0, 0, 0, 0);
			break;

		case 0x10:
			SEND_REALMYROOM(M983(0x23),
				(int)"\xb1\xe2\xb0\xa3\xc1\xa6 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbb\xe7\xbf\xeb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9",
				0, 0, 0, 0);
			break;

		case 0x11:
			SEND_REALMYROOM(M983(0x23),
				(int)"\xb0\xb9\xbc\xf6\xc1\xa6 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbb\xe7\xbf\xeb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9",
				0, 0, 0, 0);
			break;
		}
		break;
	}

	case 0x69:
	{
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);

		if (packet.Decode1() != 5)
			SEND_REALMYROOM(M983(0x23),
				(int)K2L_Compatibility("\xc8\xaf\xba\xd2\xc0\xcc \xbf\xcf\xb7\xe1\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);

		else
			SEND_REALMYROOM(M983(0x23),
				(int)K2L_Compatibility("\xb9\xb0\xb0\xc7\xc0\xbb \xc8\xaf\xba\xd2\xb9\xde\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);
		for (int count = packet.Decode1(); count > 0; --count)
		{
			sBuyItemResult result;
			packet.DecodeBuffer(&result, sizeof(sBuyItemResult));

			int msg;
			switch (result.Typeid >> 26)
			{
			case 1:
				msg = M983(0xd6);
				break;
			case 2:
				msg = M983(0xd7);
				break;
			case 7:
				msg = M983(0xd8);
				break;
			case 6:
				msg = M983(0xd9);
				break;
			case 4:
				msg = M983(0xda);
				break;
			case 5:
				msg = M983(0xdb);
				break;
			case 0xe:
				msg = M983(0xdc);
				break;
			default:
				continue;
			}

			SEND_REALMYROOM(msg, result.guid, result.Typeid, 0, 0, 0);
		}

		Doc()->m_myInfo.stat.i64Pang = packet.Decode8();
		Doc()->m_bonusCash = packet.Decode8();
		Doc()->RefreshAllCookie();
		if (RFLobbyDesktop::Instance983()) // 983 005a865c
			RFLobbyDesktop::Instance983()->Refresh983();
		break;
	}

	case PKT645_SEM_983(0xa4):
	{
		struct
		{
			__int64 pang;
			__int64 cash;
			unsigned long id;
			unsigned long unused;
		} result;
		packet.DecodeBuffer(&result, sizeof(result));

		Doc()->m_myInfo.stat.i64Pang += result.pang;
		Doc()->m_bonusCash += result.cash;
		Doc()->RefreshAllCookie();
		SEND_REALMYROOM(M983(0xd1), result.id, 0, 0, 0, 0);
		break;
	}

	case 0xa5:
	{
		// PKT (frente C, lote 48): 983 005a8674 - os sub-codigos 2 e 1 dividem a cauda (pang + 005fbe40, 005a87ef); outro codigo
		// so manda a mensagem
		int type = packet.Decode1();
		bool known = true;
		switch (type)
		{
		case 2:
		{
			unsigned char flags = packet.Decode1();
			unsigned char index = packet.Decode1();
			unsigned long guid = packet.Decode4();
			CSharedDoc* doc = Doc();
			if (flags & 1)
			{
				std::map<unsigned int, sItemInfo>::iterator it =
					doc->m_clubSetMap.find(guid);
				if (it != doc->m_clubSetMap.end())
					(*it).second.Common[index]--;
			}
			else
			{
				std::map<unsigned int, sCharacterInfo>::iterator it =
					doc->m_charMap.find(guid);
				if (it != doc->m_charMap.end())
					(*it).second.PCL[index]--;
			}
			break;
		}
		case 1:
		{
			unsigned char flags = packet.Decode1();
			unsigned char index = packet.Decode1();
			unsigned long guid = packet.Decode4();
			CSharedDoc* doc = Doc();
			if (flags & 1)
			{
				std::map<unsigned int, sItemInfo>::iterator it =
					doc->m_clubSetMap.find(guid);
				if (it != doc->m_clubSetMap.end())
					(*it).second.Common[index]++;
			}
			else
			{
				std::map<unsigned int, sCharacterInfo>::iterator it =
					doc->m_charMap.find(guid);
				if (it != doc->m_charMap.end())
					(*it).second.PCL[index]++;
			}
			break;
		}
		default:
			known = false;
			break;
		}
		if (known)
		{
			__int64 pang = packet.Decode8();
			Doc()->m_myInfo.stat.i64Pang -= pang;
			// PKT (frente C, lote 11): 983 005a8808
			if (RFLobbyDesktop::Instance983())
				RFLobbyDesktop::Instance983()->Refresh983();
		}

		SEND_REALMYROOM(M983(0xd2), type, 0, 0, 0, 0);
		break;
	}

	case 0xcd:
	{
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);

		switch (packet.Decode1())
		{
		case 0:
		{
			unsigned long guid = packet.Decode4();
			int param = packet.Decode4();
			int count = packet.Decode2();
			SEND_REALMYROOM(M983(0xde), param, guid, count, 0, 0);
			break;
		}

		case 1:
		case 2:
		case 3:
		case 4:
			SEND_REALMYROOM(M983(0x23),
				(int)K2L_Compatibility("\xc1\xb6\xc7\xd5\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
			break;

		case 5:
			SEND_REALMYROOM(M983(0x23),
				(int)K2L_Compatibility("\xc3\xe0\xc7\xcf\xc7\xd5\xb4\xcf\xb4\xd9. \xc0\xcc\xba\xa5\xc6\xae\xbf\xa1 \xb4\xe7\xc3\xb7\xb5\xc7\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9. \xc0\xda\xbc\xbc\xc7\xd1 \xbb\xe7\xc7\xd7\xc0\xba \xc8\xa8\xc6\xe4\xc0\xcc\xc1\xf6\xb8\xa6 \xc2\xfc\xc1\xb6\xc7\xcf\xbc\xbc\xbf\xe4."),
				0, 0, 0, 0);
			break;

		case 6:
			SEND_REALMYROOM(M983(0x23),
				(int)K2L_Compatibility("\xc0\xcc\xba\xa5\xc6\xae\xbf\xa1 \xb4\xe7\xc3\xb7\xc0\xcc \xbe\xc8\xb5\xc7\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
			break;
		}
		break;
	}

	case 0xe2:
	{
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);

		unsigned char result = packet.Decode1();
		unsigned long mascotGuid = packet.Decode4();

		switch (result)
		{
		case 1:
		case 2:
			SEND_REALMYROOM(M983(0x23),
				(int)K2L_Compatibility("\xb8\xde\xbc\xbc\xc1\xf6 \xba\xaf\xb0\xe6\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
			break;

		case 3:
			SEND_REALMYROOM(M983(0x23),
				(int)K2L_Compatibility("\xb1\xdd\xbe\xd7\xc0\xcc \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);
			break;

		case 4:
		{
			SEND_REALMYROOM(M983(0x23),
				(int)K2L_Compatibility("\xb8\xde\xbc\xbc\xc1\xf6\xb0\xa1 \xba\xaf\xb0\xe6 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);

			std::string message;
			packet.DecodeStr(message);
			Doc()->m_myInfo.stat.i64Pang = packet.Decode8();

			sMascotInfo* mascot = Doc()->GetMyMascotInfo(mascotGuid);
			if (mascot)
			{
				memset(mascot->szMsg, 0, sizeof(mascot->szMsg));
				strncpy(mascot->szMsg, message.c_str(), 0x1d);
			}

			SEND_REALMYROOM(M983(0x4e), mascot->guid, (int)mascot, 0, 0, 0);
			if (RFLobbyDesktop::Instance983()) // 983 005a8e23
				RFLobbyDesktop::Instance983()->Refresh983();
			break;
		}
		}
		break;
	}

	case PKT645_SEM_983(0x181):
	{
		if (!NET()->IsConnected(WNetworkSystem::NET_MSN))
			return 1;

		WSendPacket send(0x27);
		send.EncodeStr(GetMyLoginID());
		send.EncodeStr(GetMyLoginID());
		send.Send(TO_MSN);
		Doc()->m_giftName = "";
		(unsigned int&)Doc()->m_giftIndex = 0xffffffff;
		break;
	}

	case 0x12c:
	{
		if (!IsLocalContent(S4_REAL_MYROOM))
			return 1;

		unsigned long value = packet.Decode4();
		SEND_REALMYROOM(M983(0x228), 0x12c /*983 005a8e3f; era 0x124 do 645*/, value, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x125):
	{
		if (!IsLocalContent(S4_REAL_MYROOM))
			return 1;

		Doc()->m_furnitureList.clear();
		unsigned long result = packet.Decode4();
		if (result == 1)
		{
			int count = packet.Decode2();
			((WList<cModel*>*)((char*)CRealMyRoomData::Instance() + VC10_OFS_BO(0x2c)))
				->Reset();
			sFurniture_List furniture;
			for (int i = 0; i < count; ++i)
			{
				memset(&furniture, 0, sizeof(furniture));
				packet.DecodeBuffer(&furniture, 0x1b);
				CRealMyRoomData::Instance()->AddModel(furniture);
			}
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(0, M983(0x22f), 0, 0, 0, 0, 0));
			if (count > 0 && Doc()->m_myInfo.info.dwUID == Doc()->m_replayTargetUID)
				CRealMyRoomData::Instance()->SaveModel(true);
		}
		else
		{
			SEND_REALMYROOM(M983(0x228), 0x125, result, 0, 0, 0);
		}
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x13f):
	{
		unsigned char result = packet.Decode1();
		SEND_REALMYROOM(M983(0x228), 0x13f, result, 0, 0, 0);
		SEND_REALMYROOM(M983(0x244), 1, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x13e):
	{
		if (NET()->IsConnected(WNetworkSystem::NET_MSN))
		{
			WSendPacket send(0x27);
			send.Encode4(GetMyUID());
			send.Encode4((unsigned int&)Doc()->m_giftIndex);
			send.Send(TO_MSN);
			(unsigned int&)Doc()->m_giftIndex = 0xffffffff;
			Doc()->m_giftName = "";
		}

		unsigned char notified = 0;
		int count = packet.Decode4();
		sMailIncludeItem item;
		for (int n = count; n > 0; --n)
		{
			packet.DecodeBuffer(&item, sizeof(item));
			if (item.iCount == 0)
				continue;

			if (notified == 0)
				SEND_REALMYROOM(M983(0x242), item.dwTID, (int)&notified, 0, 0, 0);

			short amount = (short)item.iCount;
			switch (item.dwTID >> 26)
			{
			case 6:
			{
				for (std::list<sItemInfo>::iterator it =
						 Doc()->m_myItemList.begin();
					it != Doc()->m_myItemList.end(); ++it)
				{
					if (it->guid == item.dwIDX)
					{
						if (it->tid == item.dwTID)
						{
							if (item.iCount < it->Common[0])
								it->Common[0] -= amount;
							else if (it->Common[0] == item.iCount)
								Doc()->m_myItemList.erase(it);
						}
						break;
					}
				}
				break;
			}

			case 5:
			{
				for (std::list<sItemInfo>::iterator it =
						 Doc()->m_ballList.begin();
					it != Doc()->m_ballList.end(); ++it)
				{
					if (it->guid == item.dwIDX)
					{
						if (it->tid == item.dwTID)
						{
							if (item.iCount < it->Common[0])
								it->Common[0] -= amount;
							else if (it->Common[0] == item.iCount)
								Doc()->m_ballList.erase(it);
						}
						break;
					}
				}
				break;
			}

			case 2:
				if (IsLocalContent(S4_UCC) && IsUccClothes(item.dwTID))
				{
					for (std::list<sItemInfo>::iterator it =
							 Doc()->m_uccItemList.begin();
						it != Doc()->m_uccItemList.end(); ++it)
					{
						if (it->guid == item.dwIDX && it->tid == item.dwTID)
						{
							Doc()->m_uccItemList.erase(it);
							break;
						}
					}
				}
				else
				{
					for (std::list<sItemInfo>::iterator it =
							 Doc()->m_partsList.begin();
						it != Doc()->m_partsList.end(); ++it)
					{
						if (it->guid == item.dwIDX)
						{
							if (it->tid == item.dwTID)
								Doc()->m_partsList.erase(it);
							break;
						}
					}
				}
				break;

			case 4:
			{
				for (std::map<unsigned int, sItemInfo>::iterator it =
						 Doc()->m_clubSetMap.begin();
					it != Doc()->m_clubSetMap.end(); ++it)
				{
					if ((*it).second.guid == item.dwIDX)
					{
						if ((*it).second.tid == item.dwTID)
							Doc()->m_clubSetMap.erase(it);
						break;
					}
				}
				break;
			}

			case 0x1c:
			{
				for (std::list<sItemInfo>::iterator it =
						 Doc()->m_auxPartList.begin();
					it != Doc()->m_auxPartList.end(); ++it)
				{
					if (it->guid == item.dwIDX)
					{
						if (it->tid == item.dwTID)
						{
							if (item.iCount < it->Common[0])
								it->Common[0] -= amount;
							else if (it->Common[0] == item.iCount)
								Doc()->m_auxPartList.erase(it);
						}
						break;
					}
				}
				break;
			}

			case 7:
			{
				std::map<unsigned int, sCaddieInfo>::iterator it =
					Doc()->m_caddieMap.find(item.dwIDX);
				if (it != Doc()->m_caddieMap.end())
					Doc()->m_caddieMap.erase(it);
				break;
			}

			case 0x1f:
			{
				for (std::list<sCards>::iterator it = Doc()->m_cardList.begin();
					it != Doc()->m_cardList.end(); it++)
				{
					if ((*it).tid == item.dwTID)
					{
						(*it).cardCount -= item.iCount;
						if ((*it).cardCount < 1)
							Doc()->m_cardList.erase(it);
						break;
					}
				}
				break;
			}
			}
		}

		if (notified != 0)
			SEND_REALMYROOM(M983(0xe1), 0, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0x225), 0x13e, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x141):
	{
		int result = packet.Decode1();
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0x228), 0x13f, result, 0, 0, 0);
		SEND_REALMYROOM(M983(0x244), 1, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x140):
	{
		static sMailBoxListForPage mailPage;
		static sMailInfoBrief mailBrief;

		mailPage.iCurPage = packet.Decode4();
		mailPage.iTotalPage = packet.Decode4();
		mailPage.iTotalCount = packet.Decode4();
		mailPage.m_lMailList.clear();
		for (int i = 0; i < mailPage.iTotalCount; ++i)
		{
			memset(&mailBrief, '0', sizeof(mailBrief));
			packet.DecodeBuffer(&mailBrief, sizeof(mailBrief));
			mailPage.m_lMailList.push_back(mailBrief);
			if (IsLocalContent(S4_UCC))
				RequestUccInfo(mailBrief.item);
		}
		CTransferableItem::Instance()->SetMailInfoPage(mailPage);
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0x223), 0, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0x244), 1, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x143):
	{
		int result = packet.Decode1();
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0x228), 0x13f, result, 0, 0, 0);
		SEND_REALMYROOM(M983(0x244), 1, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x142):
	{
		static sMailInfo mailInfo;
		static sMailIncludeItem mailItem;

		mailInfo.dwIndex = packet.Decode4();
		strncpy(mailInfo.szFromNick, packet.DecodeStr().c_str(), 0x16);
		strncpy(mailInfo.szDate, packet.DecodeStr().c_str(), 0x20);
		strncpy(mailInfo.szContent, packet.DecodeStr().c_str(), 800);
		mailInfo.bHaveItem = packet.Decode1() == 1;
		mailInfo.m_lMailItemList.clear();
		if (mailInfo.bHaveItem)
		{
			for (int n = packet.Decode4(); n > 0; --n)
			{
				packet.DecodeBuffer(&mailItem, sizeof(mailItem));
				mailInfo.m_lMailItemList.push_back(mailItem);
				if (IsLocalContent(S4_UCC))
					RequestUccInfo(mailItem);
			}
		}
		CTransferableItem::Instance()->SetMailInfo(mailInfo);
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0x224), 0, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0x244), 1, 0, 0, 0, 0);
		break;
	}

	// PKT (frente C, lote 12): o despachante do 983 (005a8360) nao tem 0x164/0x165 - os dois vao pelo default ao
	// OnPacketCommon (009d21dd/009d225a, ja escritos em taskmanager.cpp); os rotulos daqui pegavam os pacotes antes dele.
	case PKT645_SEM_983(0x15c): // era 0x164 (s63)
	{
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0x226), 0, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0x244), 1, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x15d): // era 0x165 (s63)
	{
		int value = packet.Decode4();
		SEND_REALMYROOM(M983(0x228), 0x165 /*983; era 0x15d*/, value, 0, 0, 0);
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0x244), 1, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x145):
	{
		int result = packet.Decode1();
		SEND_REALMYROOM(M983(0x228), 0x145, result, 0, 0, 0);
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0x244), 1, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x144):
	{
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0x244), 1, 0, 0, 0, 0);

		int count = packet.Decode4();
		sMailIncludeItem mail;
		sCaddieInfo caddie;
		sCards card;
		sFurniture_List entry;
		for (int n = 0; n < count; ++n)
		{
			packet.DecodeBuffer(&mail, sizeof(mail));
			int amount = mail.iCount < 1 ? 1 : mail.iCount;
			unsigned long tid = mail.dwTID;
			// HACK: no code; keeps idx/time/type as real register variables
			// loaded after the tid compare (PDB lines 1619-1634 hold no code).
			_ReadWriteBarrier();
			unsigned long idx = mail.dwIDX;
			int time = mail.iTimeCount;
			unsigned char type = mail.btItemType;
			if (tid == 0x1a000010)
				continue;

			switch (tid >> 26)
			{
			case 2:
			{
				if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) &&
					IsUccClothes(tid))
				{
					WSendPacket send((enumClientPacket)0xb9);
					send.Encode1(1);
					send.Encode4(idx);
					send.Encode1(1);
					send.Send(TO_GAME);
				}
				else
				{
					sItemInfo item;
					memset(&item, 0, sizeof(item));
					item.guid = idx;
					item.tid = tid;
					item.ItemType = type;
					item.IsValid = 1;
					item.HourRemain = time / 60 / 24;
					Doc()->AddMyPartsList(item);
				}
				break;
			}
			case 7:
			{
				memset(&caddie, 0, sizeof(caddie));
				caddie.guid = idx;
				caddie.tid = tid;
				if (tid == 0x1c000001 || tid == 0x1c000002 ||
					tid == 0x1c000003 || tid == 0x1c000007)
				{
					caddie.Remain_Date = (unsigned short)(time / 60 / 24);
					caddie.Rent_flag = 1;
				}
				else
				{
					caddie.Remain_Date = 0;
					caddie.Rent_flag = 0;
				}
				if (tid >= 0x1c00000c && tid <= 0x1c00000e)
				{
					caddie.Level = type;
					caddie.Exp = mail.iTimeCount;
				}

				std::map<unsigned int, sCaddieInfo>::iterator it =
					Doc()->m_caddieMap.find(mail.dwIDX);
				if (it != Doc()->m_caddieMap.end())
					it->second.Remain_Date = caddie.Remain_Date;
				else
					Doc()->m_caddieMap.insert(
						std::map<unsigned int, sCaddieInfo>::value_type(
							mail.dwIDX, caddie));
				break;
			}
			case 8:
			{
				int days = time / 60;
				std::map<unsigned int, sCaddieInfo>::iterator it =
					Doc()->m_caddieMap.find(mail.dwIDX);
				if (it != Doc()->m_caddieMap.end())
				{
					sCaddieInfo& info = it->second;
					info.tidPart = tid;
					info.Remain_Partdate = (unsigned short)days;
					if (info.Remain_Date < days)
						info.Remain_Date = (unsigned short)days;
				}
				break;
			}
			case 6:
			{
				sItemInfo item;
				memset(&item, 0, sizeof(item));
				item.guid = mail.dwIDX;
				item.tid = tid;
				item.ItemType = type;
				item.Common[0] = (short)amount;
				item.IsValid = 1;
				if (item.ItemType != 0)
				{
					if (item.ItemType == 3)
						item.HourRemain = mail.iTimeCount;
					else if (item.ItemType == 4)
						item.HourRemain = mail.iTimeCount * 24;
					else if (item.ItemType == 5)
						item.HourRemain = mail.iTimeCount * 720;
					else if (item.ItemType == 2)
						item.HourRemain = mail.iTimeCount / 60;
				}

				std::list<sItemInfo>::iterator it;
				for (it = Doc()->m_myItemList.begin();
					it != Doc()->m_myItemList.end(); ++it)
				{
					if (it->guid == mail.dwIDX)
					{
						it->Common[0] = (short)amount;
						break;
					}
				}
				if (it == Doc()->m_myItemList.end())
				{
					if (IsLocalContent((localContentType_t)0x14) &&
						tid == 0x1a000042)
					{
						item.Common[1] = packet.Decode2();
						item.Common[2] = packet.Decode2();
						item.HourRemain = 0xa8;
						_SYSTEMTIME date;
						packet.DecodeBuffer(&date, sizeof(date));
						item.ItemDate = date;
						item.ItemType = 2;
					}
					Doc()->m_myItemList.push_back(item);
				}
				break;
			}
			case 5:
			{
				std::list<sItemInfo>::iterator it;
				for (it = Doc()->m_ballList.begin();
					it != Doc()->m_ballList.end(); ++it)
				{
					if (it->guid == idx)
					{
						it->Common[0] = (short)amount;
						break;
					}
				}
				if (it == Doc()->m_ballList.end())
				{
					sItemInfo item;
					memset(&item, 0, sizeof(item));
					item.guid = idx;
					item.tid = tid;
					item.Common[0] = (short)amount;
					item.ItemType = 0;
					item.IsValid = 1;
					Doc()->m_ballList.push_back(item);
				}
				break;
			}
			case 3:
			case 4:
			{
				sItemInfo item;
				memset(&item, 0, sizeof(item));
				item.guid = mail.dwIDX;
				item.tid = tid;
				memcpy(item.Common, mail.soCom, sizeof(item.Common));
				item.IsValid = 1;
				Doc()->m_clubSetMap.insert(
					std::map<unsigned int, sItemInfo>::value_type(mail.dwIDX,
						item));
				break;
			}
			case 0x1d:
			{
				if (ItemManager()->FindQuestDrop(tid) != 0)
				{
					std::list<sItemInfo>::iterator it;
					for (it = Doc()->m_questDropList.begin();
						it != Doc()->m_questDropList.end(); ++it)
					{
						if (it->guid == idx)
						{
							it->Common[0] += 1;
							break;
						}
					}
					if (it == Doc()->m_questDropList.end())
					{
						sItemInfo item;
						memset(&item, 0, sizeof(item));
						item.guid = idx;
						item.tid = tid;
						item.Common[0] = 1;
						Doc()->m_questDropList.push_back(item);
					}
				}
				break;
			}
			case 0x10:
			{
				sMascotInfo mascot;
				memset(&mascot, 0, sizeof(mascot));
				mascot.guid = mail.dwIDX;
				strcpy(mascot.szMsg, "PANGYA!");
				mascot.tid = tid;
				int days = mail.iTimeCount / 60;
				mascot.Remain_Date = (unsigned short)days;

				_SYSTEMTIME now;
				GetLocalTime(&now);
				FILETIME ft;
				SystemTimeToFileTime(&now, &ft);
				*(__int64*)&ft += (__int64)days * 36000000000i64;
				FileTimeToSystemTime(&ft, &now);
				mascot.endDate = now;
				Doc()->AddMascot(&mascot);
				break;
			}
			case 0x1c:
			{
				sItemInfo item;
				memset(&item, 0, sizeof(item));
				item.guid = mail.dwIDX;
				item.tid = tid;
				item.ItemType = type;
				item.Common[0] = (short)amount;
				item.IsValid = 1;
				if (item.ItemType != 0)
				{
					if (item.ItemType == 3)
						item.HourRemain = mail.iTimeCount;
					else if (item.ItemType == 4)
						item.HourRemain = mail.iTimeCount * 24;
					else if (item.ItemType == 5)
						item.HourRemain = mail.iTimeCount * 720;
					else if (item.ItemType == 2)
						item.HourRemain = mail.iTimeCount / 60;
				}

				std::list<sItemInfo>::iterator it;
				for (it = Doc()->m_auxPartList.begin();
					it != Doc()->m_auxPartList.end(); ++it)
				{
					if (it->guid == mail.dwIDX)
					{
						it->Common[0] = (short)amount;
						break;
					}
				}
				if (it == Doc()->m_auxPartList.end())
					Doc()->m_auxPartList.push_back(item);
				break;
			}
			case 0x1f:
			{
				if (ItemManager()->FindCard(tid) != 0)
				{
					memset(&card, 0, sizeof(card));
					card.uid = idx;
					card.tid = tid;
					card.cardCount = amount;
					card.valid = 1;

					bool found = false;
					std::list<sCards>::iterator it;
					for (it = Doc()->m_cardList.begin();
						it != Doc()->m_cardList.end(); ++it)
					{
						if ((*it).tid == tid)
						{
							(*it).cardCount = amount;
							found = true;
							break;
						}
					}
					if (!found)
						Doc()->m_cardList.push_back(card);
				}
				break;
			}
			case 0xe:
			{
				sItemInfo item;
				memset(&item, 0, sizeof(item));
				item.ItemType = type;
				item.guid = mail.dwIDX;
				item.tid = tid;
				item.IsValid = 1;
				item.HourRemain = mail.iTimeCount * 24;
				Doc()->m_cutinList.push_back(item);
				break;
			}
			case 0x12:
			{
				IFF_STRUCT::sFurniture* furniture =
					ItemManager()->FindFurniture(tid);
				if (furniture != 0)
				{
					memset(&entry, 0, sizeof(entry));
					entry.id = idx;
					entry.typeId = tid;
					entry.x = furniture->Pos[0];
					entry.y = furniture->Pos[1];
					entry.z = furniture->Pos[2];
					entry.r = furniture->Pos[3];
					entry.bArrange = 0;
					Doc()->AddFurniture(entry);
				}
				break;
			}
			case 1:
				continue;
			}
		}

		AfxGetTask()->SendMsgToMainActor(MsgObject(0, M983(0x23),
			(int)"\xc6\xed\xc1\xf6\xbf\xa1 \xc6\xf7\xc7\xd4\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb8\xb6\xc0\xcc\xb7\xeb\xc0\xb8\xb7\xce \xc0\xcc\xb5\xbf\xbd\xc3\xc4\xd7\xbd\xc0\xb4\xcf\xb4\xd9.",
			0, 0, 0, 0));
		SEND_REALMYROOM(M983(0x226), 0, 0, 0, 0, 0);
		SEND_REALMYROOM(M983(0xe1), 0, 0, 0, 0, 0);
		break;
	}

	case 0x11a:
	{
		if (!IsLocalContent(S3_CADDIE_REPORT))
			return 1;

		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);
		Doc()->m_caddieReportList.clear();

		int count = packet.Decode4();
		_SYSTEMTIME report;
		packet.DecodeBuffer(&report, sizeof(report));

		if (count == 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(0, M983(0x23),
				(int)K2L_Compatibility("\xc6\xbc\xc5\xb0 \xb8\xae\xc6\xf7\xc6\xae \xbf\xad\xb6\xf7\xbf\xa1 \xbd\xc7\xc6\xd0 \xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9. "),
				0, 0, 0, 0));
			break;
		}
		if (count == -2)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(0, M983(0x23),
				(int)K2L_Compatibility("\xb5\xa5\xc0\xcc\xc5\xb8\xb0\xa1 \xc0\xaf\xc8\xbf\xc7\xcf\xc1\xf6 \xbe\xca\xc0\xba \xc6\xbc\xc5\xb0 \xb8\xae\xc6\xf7\xc6\xae \xc0\xd4\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
			break;
		}
		if (count == -1)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(0, M983(0x23),
				(int)K2L_Compatibility("\xb3\xaf\xc2\xa5\xb0\xa1 \xc0\xaf\xc8\xbf\xc7\xcf\xc1\xf6 \xbe\xca\xc0\xba \xc6\xbc\xc5\xb0 \xb8\xae\xc6\xf7\xc6\xae \xc0\xd4\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
			break;
		}

		if (IsLocalContent(S4_TREASURE_HUNTER))
			Doc()->m_bTreasureResult = 1;

		unsigned long itemGuid = 0;
		SEND_TASK_REALMYROOM(M983(0x204), (int)&itemGuid, 0, 0, 0, 0);

		std::list<sItemInfo>::iterator it = Doc()->m_myItemList.begin();
		while (it != Doc()->m_myItemList.end() && it->guid != itemGuid)
			++it;
		if (it == Doc()->m_myItemList.end())
			return 0;

		for (int i = 0; i < count; ++i)
		{
			if (--it->Common[0] <= 0)
			{
				Doc()->m_myItemList.erase(it);
				SEND_REALMYROOM(M983(0xdd), 0, 0, 0, 0, 0);
				break;
			}
		}

		sCaddieReportData data;
		for (int n = count; n > 0; --n)
		{
			packet.DecodeBuffer(&data, sizeof(data));
			Doc()->m_caddieReportList.push_back(data);
		}

		Doc()->BuildTikiReportList();
		SEND_REALMYROOM(M983(0x205), (int)&report, 0, 0, 0, 0);
		break;
	}

	case 0x129:
	{
		// PKT (frente C, lote 11): 983 005a92da. Sucesso: o servidor manda a quantidade que sobrou da bolsa (o 645 descontava 1)
		// e os premios vao para a lista de 005a8160, avisada pela mensagem 0x279 (so do 983); o 645 montava o texto do premio
		// aqui. Erro: codigo em byte (era dword) e a janela "notify" so abre nos codigos com texto.
		// PKT (frente C, lote 48): o bloco do erro (1) vem antes do sucesso (0), como no 983
		switch (packet.Decode1())
		{
		case 1:
		{
			SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);
			// tabela 005aadb4: 2, 4 e os de fora (> 5) nao mostram nada
			const char* text = NULL;
			switch (packet.Decode1())
			{
			case 0:
				text = K2L_Compatibility(
					"\xc0\xdf\xb8\xf8\xb5\xc8 \xba\xb9\xc1\xd6\xb8\xd3\xb4\xcf \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xd4\xb4\xcf\xb4\xd9. \xb0\xfc\xb8\xae\xc0\xda\xbf\xa1\xb0\xd4 \xb9\xae\xc0\xc7 \xc7\xcf\xbc\xbc\xbf\xe4.");
				break;
			case 1:
				text = K2L_Compatibility(
					"\xbc\xad\xb9\xf6\xc0\xc7 \xbf\xe4\xc3\xbb\xc0\xcc \xb8\xb9\xbd\xc0\xb4\xcf\xb4\xd9. \xc0\xe1\xbd\xc3 \xc8\xc4 \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5 \xc7\xd8 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0.");
				break;
			case 3:
			{
				const char* __fastcall GetGameString(int key); // 00540a10 (include/jp/gamestringmanager.h)
				text = GetGameString(0x30f34);
				break;
			}
			case 5:
				text = K2L_Compatibility("\xbd\xc3\xbd\xba\xc5\xdb \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9.");
				break;
			}
			if (text)
			{
				FrForm* form = NOTIFY_FORM();
				form->SetMessage(text, false);
				form->Open(NULL, 3);
			}
			break;
		}

		case 0:
		{
			SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);

			unsigned long tid = packet.Decode4();
			int remain = packet.Decode4();

			std::list<sItemInfo>::iterator it = std::find(
				Doc()->m_myItemList.begin(), Doc()->m_myItemList.end(), tid);
			if (it != Doc()->m_myItemList.end())
			{
				it->Common[0] = (short)remain;
				if (it->Common[0] < 1)
					Doc()->m_myItemList.erase(it);
			}

			int count = packet.Decode4();
			RbPouchPrizeList983::Instance983()->Clear983();
			RbPouchPrizeList983::Instance983()->SetTypeId983(tid);
			sPouchPrize prize;
			for (; count > 0; --count)
			{
				packet.DecodeBuffer(&prize, 0x14);
				RbPouchPrizeList983::Instance983()->Add983(&prize);
			}
			SEND_REALMYROOM(0x279 /*so do 983*/, 0, 0, 0, 0, 0);
			break;
		}
		}
		break;
	}

	case 0x19d:
	{
		// PKT (frente C, lote 11): 983 005a95e6 - textos por K2L_Compatibility (cada pedaco), mensagens do premio pelo tipo do
		// item na tabela do 983 (005aaddc), sem o pacote de lista de correio/presente do 645, aviso de presente ligado e 0xce
		SEND_REALMYROOM(M983(0x2), 0, 0, 0, 0, 0);

		unsigned long type = packet.Decode4();
		unsigned long count = packet.Decode4();
		unsigned long tid = packet.Decode4();
		unsigned long amount = packet.Decode4();
		char buf[256] = "";
		const char* fail =
			"\xbb\xf3\xc0\xda \xbf\xad\xb1\xe2\xb0\xa1 \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9.";

		switch (type)
		{
		case 0:
		{
			IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(tid);
			if (common == 0)
			{
				char err[64] = "";
				strcat(err, K2L_Compatibility(
					"\xc0\xdf\xb8\xf8\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb \xc5\xb8\xc0\xd4ID \xc0\xd4\xb4\xcf\xb4\xd9."));
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(0, M983(0x23), (int)err, 0, 0, 0, 0));
				SEND_REALMYROOM(M983(0xdd), count, 1, 0, 0, 0);
				return 1;
			}

			// 983 005a9855: (tid >> 26) - 2 na tabela 005aaddc; 2 e 5 com a quantidade, 6 com as bolsas de pang/experiencia,
			// 0x10 sem quantidade, os outros com a quantidade so se ela passar de 1
			const char* got = K2L_Compatibility(
				"\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb9\xde\xc0\xb8\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9.");
			switch (tid >> 26)
			{
			case 2:
			case 5:
				sprintf(buf, K2L_Compatibility(
					"\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb %d \xb0\xb3 \xb9\xde\xc0\xb8\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."), amount);
				break;
			case 6:
				if (tid == 0x1a000010)
					sprintf(buf, K2L_Compatibility(
						"\xc6\xce \xc1\xd6\xb8\xd3\xb4\xcf(%d)\xb8\xa6 \xb9\xde\xc0\xb8\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."), amount);
				else if (tid == 0x1a00015d)
					sprintf(buf, K2L_Compatibility(
						"\xb0\xe6\xc7\xe8\xc4\xa1 \xc1\xd6\xb8\xd3\xb4\xcf(%d)\xb8\xa6 \xb9\xde\xc0\xb8\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."), amount);
				else if (amount > 1)
					sprintf(buf, K2L_Compatibility(
						"\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb %d \xb0\xb3 \xb9\xde\xc0\xb8\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."), amount);
				else
					sprintf(buf, got);
				break;
			case 0x10:
				sprintf(buf, got);
				break;
			default:
				if (amount > 1)
					sprintf(buf, K2L_Compatibility(
						"\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb %d \xb0\xb3 \xb9\xde\xc0\xb8\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."), amount);
				else
					strcpy(buf, got);
				break;
			}

			AfxGetTask()->SendMsgToMainActor(
				MsgObject(0, M983(0x26), (int)buf, tid, 0, 0, 0));
			SEND_REALMYROOM(M983(0xdd), count, 1, 0, 0, 0);

			// 983 005a9a20: so esvazia a lista do correio (CSharedDoc+0x378c, 005a7b70); nenhum pacote (o 645 pedia a lista de novo)
			if (IsLocalContent((localContentType_t)0x61 /*983: era 0x60; medido*/))
				Doc()->m_mailList.clear();

			AfxGetTask()->SendMsgToMainActor(MsgObject(0, M983(0x23),
				(int)K2L_Compatibility("\xbb\xf3\xc7\xb0\xc0\xcc \xbf\xec\xc6\xed\xc0\xb8\xb7\xce \xc0\xfc\xb4\xde\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9. \xbf\xec\xc6\xed\xc7\xd4\xc0\xbb \xc8\xae\xc0\xce\xc7\xcf\xbc\xbc\xbf\xe4."),
				0, 0, 0, 0));
			// 983 005a9abf: CSharedDoc+0x10d80 = 1 (m_bRefreshCamera no cabecalho; e o aviso de presente, que o giftalarmdlg.cpp
			// desliga) e a mensagem 0xce (so do 983) com 1
			C_ASSERT_VC10(offsetof(CSharedDoc, m_bRefreshCamera) == 0x10d80);
			Doc()->m_bRefreshCamera = true;
			SEND_REALMYROOM(0xce /*so do 983*/, 1, 0, 0, 0, 0);
			return 1;
		}

		case 4:
			strcpy(buf, K2L_Compatibility(fail));
			strcat(buf, K2L_Compatibility(
				"\n\xbc\xf6\xb7\xae\xc0\xcc \xc3\xe6\xba\xd0\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9."));
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(0, M983(0x23), (int)buf, 0, 0, 0, 0));
			return 1;

		case 8:
			strcpy(buf, K2L_Compatibility(fail));
			strcat(buf, K2L_Compatibility(
				"\n\xbe\xc6\xc0\xcc\xc5\xdb \xc1\xbe\xb7\xf9 \xbf\xa1\xb7\xaf\xc0\xd4\xb4\xcf\xb4\xd9."));
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(0, M983(0x23), (int)buf, 0, 0, 0, 0));
			return 1;

		case 0x200b3b:
		case 0x200b3c:
		case 0x200b3d:
		{
			IActor* gateway = AfxGetTask()->GetActor(s_gatewayActor[1]);
			gateway << MsgObject(0, M983(0x276), 1, type, 0, 0, 0);
		}
			return 1;
		}

		strcpy(buf, K2L_Compatibility(fail));
		strcat(buf, K2L_Compatibility("\n"));
		strcat(buf, MakeStr("Error : %d", type));
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(0, M983(0x23), (int)buf, 0, 0, 0, 0));
		break;
	}

	case 0x170:
	{
		if (IsLocalContent((localContentType_t)0x70 /*983: era 0x71; medido*/) != true)
			return 1;

		unsigned long result = packet.Decode4();
		unsigned long flags = packet.Decode4();
		if (result == 0)
		{
			// PKT (frente C, lote 12): 983 005a9cdb - os bytes 0x50f4/0x50f5 do 645 sao m_bItemStoragePin/m_bItemStorageLock
			// (983 +0x110c8/+0x110c9); o ramo 0x10 do 645 (pacote 0xcc e trava desligada) saiu: 8 e 0x10 fazem o mesmo
			C_ASSERT_VC10(offsetof(CSharedDoc, m_bItemStoragePin) == 0x110c8);
			C_ASSERT_VC10(offsetof(CSharedDoc, m_bItemStorageLock) == 0x110c9);
			if ((flags & 2) != 0)
			{
				SEND_TASK_REALMYROOM(M983(0x236), 0, 0, 0, 0, 0);
				Doc()->m_bItemStoragePin = true;
			}
			else if ((flags & 4) != 0 && (flags & 0x18) != 0)
			{
				if (Doc()->m_bItemStoragePin == false)
					Doc()->m_bItemStoragePin = true;
				else
					SEND_TASK_REALMYROOM(M983(0x235), 0, 0, 0, 0, 0);
				Doc()->m_bItemStorageLock = true;
			}
		}
		else
		{
			// 983 005a9e0d: o 1o parametro e o id do proprio pacote (0x170); o fonte tinha o do 645 (0x175)
			SEND_TASK_REALMYROOM(M983(0x23f), 0x170, result, 0, 0, 0);
		}
		if (CItemStorage::IsInstantiated())
			CItemStorage::Instance()->UnLock();
		break;
	}

	case 0x176:
	{
		if (IsLocalContent((localContentType_t)0x70 /*983: era 0x71; medido*/) != true)
			return 1;

		unsigned long result = packet.Decode4();
		SEND_TASK_REALMYROOM(M983(0x23f), 0x176 /*983; era 0x17b do 645*/, result, 0, 0, 0);
		if (CItemStorage::IsInstantiated())
			CItemStorage::Instance()->UnLock();
		break;
	}

	case 0x16c: // JP 983 (s63): 0x171 no KR 645; caso 005a9ed0 (D4 + envio 0xcd)
	{
		if (IsLocalContent((localContentType_t)0x70 /*983: era 0x71; medido*/) != true)
			return 1;

		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			if (result == 0x66)
				SEND_TASK_REALMYROOM(M983(0x234), 0, 0, 0, 0, 0);
			SEND_TASK_REALMYROOM(M983(0x23f), 0x16c /*983; era 0x171 do 645*/, result, 0, 0, 0);
		}
		else
		{
			WSendPacket first((enumClientPacket)0xd5);
			first.Send(TO_GAME);
			WSendPacket second((enumClientPacket)0xcd); // JP 983 (s63): 0xcf no KR 645
			second.Encode4(99);
			second.Encode2(1);
			second.Send(TO_GAME);
		}
		if (CItemStorage::IsInstantiated())
			CItemStorage::Instance()->UnLock();
		break;
	}

	case 0x16d:
	{
		if (IsLocalContent((localContentType_t)0x70 /*983: era 0x71; medido*/) != true)
			return 1;
		CItemStorage* storage = CItemStorage::Instance();
		if (storage == 0)
			return 1;

		storage->ClearStoredItemInfo();
		unsigned long totalPage = packet.Decode2();
		unsigned long curPage = packet.Decode2();
		unsigned long count = packet.Decode1();
		for (unsigned long i = 0; i < count; ++i)
		{
			sStoredItemInfo info;
			memset(&info, 0, sizeof(info));
			packet.DecodeBuffer(&info, sizeof(info));
			storage->AddStoredItemInfo(info);
		}
		storage->SetTotalPage(totalPage);
		storage->SetCurPage(curPage);
		SEND_TASK_REALMYROOM(M983(0x23a), 0, 0, 0, 0, 0);
		storage->UnLock();
		break;
	}

	case 0x16f:
	{
		if (IsLocalContent((localContentType_t)0x70 /*983: era 0x71; medido*/) != true)
			return 1;

		unsigned long result = packet.Decode4();
		if (result == 0)
			SEND_TASK_REALMYROOM(M983(0x23d), 0, result, 0, 0, 0);
		else
			SEND_TASK_REALMYROOM(M983(0x23f), 0x16f /*983; era 0x174 do 645*/, result, 0, 0, 0);
		if (CItemStorage::IsInstantiated())
			CItemStorage::Instance()->UnLock();
		break;
	}

	case 0x16e:
	{
		if (IsLocalContent((localContentType_t)0x70 /*983: era 0x71; medido*/) != true)
			return 1;

		unsigned long result = packet.Decode4();
		if (result == 0)
			SEND_TASK_REALMYROOM(M983(0x23e), 0, result, 0, 0, 0);
		else
			SEND_TASK_REALMYROOM(M983(0x23f), 0x16e /*983; era 0x173 do 645*/, result, 0, 0, 0);
		if (CItemStorage::IsInstantiated())
			CItemStorage::Instance()->UnLock();
		break;
	}

	case 0x171:
	{
		unsigned long result = packet.Decode4();
		if (result != 0)
			SEND_TASK_REALMYROOM(M983(0x23f), 0x171 /*983; era 0x176 do 645*/, result, 0, 0, 0);
		if (CItemStorage::IsInstantiated())
			CItemStorage::Instance()->UnLock();
		break;
	}

	case 0x172:
	{
		CItemStorage* storage = CItemStorage::Instance();
		if (storage == 0)
			return 1;
		storage->SetStoredPang(packet.Decode8());
		storage->UnLock();
		break;
	}

	case 0x173:
	{
		FrForm* form = NOTIFY_FORM();
		std::string message;

		switch (packet.Decode4())
		{
		case 0:
			message =
				"\xb5\xb9\xc7\xc7\xb4\xcf\xc0\xc7 \xba\xf1\xb9\xd0\xb1\xdd\xb0\xed \xbc\xb3\xc1\xa4\xc0\xcc \xba\xaf\xb0\xe6 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9";
			{
				bool locked = (packet.Decode1() == 1);
				Doc()->m_bItemStorageLock = locked; // 983 005aa336: +0x110c9 (o fonte escrevia 0x50f5 do 645)
				break;
			}

		case 0x65:
			message =
				"\xbc\xb3\xc1\xa4\xc0\xcc \xba\xaf\xb0\xe6\xb5\xc7\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9";
			break;

		case 100:
			message =
				"DataBase\xc1\xf6\xbf\xac \xc0\xe1\xbd\xc3 \xc8\xc4 \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5 \xc7\xd8 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0";
			break;

		case 0x75:
			message =
				"\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb0\xa1 \xc0\xdf\xb8\xf8\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9 \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5 \xc7\xd8 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0";
			break;
		}

		// PKT (frente C, lote 12): 983 005aa36e - texto pelo K2L_Compatibility e aviso aberto com 0x803
		const char* text = K2L_Compatibility(message.c_str());
		form->SetMessage(text, false);
		form->Open(NULL, 0x803);
		break;
	}

	case 0x174:
	{
		unsigned long result = packet.Decode4();
		FrForm* form = NOTIFY_FORM();

		switch (result)
		{
		case 0:
			form->SetMessage(K2L_Compatibility(
				"\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb0\xa1 \xba\xaf\xb0\xe6 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			break;

		case 0x65:
			form->SetMessage(K2L_Compatibility(
				"\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb0\xa1 \xba\xaf\xb0\xe6\xb5\xc7\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			break;

		case 100:
			form->SetMessage(K2L_Compatibility(
				"\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb0\xa1 \xba\xaf\xb0\xe6\xb5\xc7\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			break;

		case 0x68:
			form->SetMessage(K2L_Compatibility(
				"\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xc0\xc7 \xb1\xe6\xc0\xcc\xb0\xa1 \xc0\xdf\xb8\xf8 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.."),
				false);
			break;

		case 0x72:
			form->SetMessage(K2L_Compatibility(
				"'\xb5\xb9\xc7\xc7\xb4\xcf\xc0\xc7 \xba\xf1\xb9\xd0\xb1\xdd\xb0\xed'\xc0\xc7 \xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb0\xa1 \xbc\xb3\xc1\xa4 \xb5\xc7\xbe\xee \xc0\xd6\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			break;

		case 0x75:
			form->SetMessage(K2L_Compatibility(
				"\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb0\xa1 \xc0\xdf\xb8\xf8\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			break;
		}

		form->Open(NULL, 0x803); // 983 005aa462: 0x803, nao 3
		break;
	}

	case 0x17f:
	{
		if (IsLocalContent((localContentType_t)0x75 /*983: era 0x76; interpolado*/))
		{
			FrForm* form = NOTIFY_FORM();
			switch (packet.Decode1())
			{
			case 1:
				form->SetMessage(K2L_Compatibility(
					"\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xc1\xf6 \xbe\xca\xc0\xba \xb0\xa1\xb1\xb8\xc0\xd4\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 2:
				form->SetMessage(K2L_Compatibility(
					"\xbf\xc0\xb4\xc3\xc0\xba \xc0\xcc \xb0\xa1\xb1\xb8\xb8\xa6 \xb4\xf5 \xc0\xcc\xbb\xf3 \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 5:
				form->SetMessage(K2L_Compatibility(
					"\xbf\xc0\xb4\xc3\xc0\xba \xc0\xcc \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb4\xf5 \xc0\xcc\xbb\xf3 \xbe\xf2\xc0\xbb \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 0:
				form->SetMessage(K2L_Compatibility(
					"\xc0\xcc \xb0\xa1\xb1\xb8\xc0\xc7 \xb1\xe2\xb4\xc9\xc0\xbb \xbb\xe7\xbf\xeb\xc7\xcf\xb4\xc2\xb5\xa5 \xbd\xc3\xbd\xba\xc5\xdb \xbf\xc0\xb7\xf9\xb0\xa1 \xb9\xdf\xbb\xfd\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9. GM\xbf\xa1\xb0\xd4 \xb9\xae\xc0\xc7 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 3:
				form->SetMessage(K2L_Compatibility(
					"\xb9\xe8\xc4\xa1\xb5\xc7\xc1\xf6 \xbe\xca\xc0\xba \xb0\xa1\xb1\xb8\xc0\xd4\xb4\xcf\xb4\xd9. \xb9\xe8\xc4\xa1\xc7\xd1 \xb4\xd9\xc0\xbd \xc0\xfa\xc0\xe5\xb9\xf6\xc6\xb0\xc0\xbb \xb4\xad\xb7\xaf \xc0\xfa\xc0\xe5\xc7\xcf\xbd\xc5 \xc8\xc4\xbf\xa1 \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 4:
				form->SetMessage(K2L_Compatibility(
					"\xb0\xa1\xb1\xb8\xc0\xc7 \xb1\xe2\xb4\xc9\xc0\xbb \xbb\xe7\xbf\xeb\xc7\xcf\xbf\xa9 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbe\xf2\xb4\xc2 \xb5\xa5\xbf\xa1 \xb9\xae\xc1\xa6\xb0\xa1 \xb9\xdf\xbb\xfd\xc7\xcf\xbf\xa9 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbe\xf2\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9. GM\xbf\xa1\xb0\xd4 \xb9\xae\xc0\xc7 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 6:
				form->SetMessage(K2L_Compatibility(
					"\xc7\xf6\xc0\xe7 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xb0\xa1\xb1\xb8\xc0\xd4\xb4\xcf\xb4\xd9."),
					false);
				break;
			}
			form->Open(NULL, 3);
			return 1;
		}
		FrForm* form = NOTIFY_FORM();
		form->SetMessage(K2L_Compatibility(
			"\xb3\xad\xb7\xce \xb1\xe2\xb4\xc9\xc0\xba \xc7\xf6\xc0\xe7 \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc7\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. "),
			false);
		form->Open(NULL, 3);
		break;
	}

	case 0x17e:
	{
		if (IsLocalContent((localContentType_t)0x75 /*983: era 0x76; interpolado*/))
		{
			unsigned long p1 = packet.Decode4();
			unsigned long p2 = packet.Decode4();
			SEND_TASK_REALMYROOM(M983(0x245), p1, p2, 0, 0, 0);
			return 1;
		}
		FrForm* form = NOTIFY_FORM();
		form->SetMessage(K2L_Compatibility(
			"\xb3\xad\xb7\xce \xb1\xe2\xb4\xc9\xc0\xba \xc7\xf6\xc0\xe7 \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc7\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. "),
			false);
		form->Open(NULL, 3);
		break;
	}

	case 0x18f:
	{
		AfxGetTask()->GetMainActor() << MsgObject(0, M983(0x2), 0, 0, 0, 0, 0);
		FrForm* form = NOTIFY_FORM();

		int result = packet.Decode1();
		if (result == 0)
		{
			unsigned long tid = packet.Decode4();
			unsigned long guid = packet.Decode4();
			std::list<sItemInfo>& items =
				((tid >> 26) == 2) ? Doc()->m_partsList : Doc()->m_myItemList;

			std::list<sItemInfo>::iterator it =
				std::find_if(items.begin(), items.end(), _FindByGuid(guid));
			if (it != items.end())
			{
				it->Expired = 0;
				it->HourRemain = 0xa8;
				form->SetMessage(K2L_Compatibility(
					"\xb4\xeb\xbf\xa9 \xb1\xe2\xb0\xa3\xc0\xcc \xbf\xac\xc0\xe5 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
			}
			else
			{
				form->SetMessage(K2L_Compatibility(
					"\xc7\xd8\xb4\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xbf\xa1 \xb4\xeb\xc7\xd1 \xc1\xa4\xba\xb8\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
			}
		}
		else
		{
			switch (result)
			{
			case 1:
				form->SetMessage(K2L_Compatibility(
					"\xc7\xd8\xb4\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba \xb4\xeb\xbf\xa9 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xbe\xc6\xb4\xd5\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 2:
				form->SetMessage(K2L_Compatibility(
					"\xc7\xd8\xb4\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xbf\xa1 \xb4\xeb\xc7\xd1 \xc1\xa4\xba\xb8\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 3:
				form->SetMessage(K2L_Compatibility(
					"\xb1\xe2\xb0\xa3\xc0\xcc \xb8\xb8\xb7\xe1\xb5\xc8 \xc0\xc7\xbb\xf3\xb8\xb8 \xb1\xe2\xb0\xa3\xc0\xbb \xbf\xac\xc0\xe5 \xc7\xd2 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 5:
				form->SetMessage(K2L_Compatibility(
					"\xb1\xe2\xb0\xa3 \xbf\xac\xc0\xe5 \xbf\xe4\xc3\xbb\xc0\xcc \xb0\xc5\xba\xce \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				break;
			default:
				form->SetMessage(
					MakeStr(K2L_Compatibility(
						"\xc3\xb3\xb8\xae\xbf\xa1 \xbd\xc7\xc6\xd0 \xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.(%d)"),
						result),
					false);
				break;
			}
		}

		form->Open(NULL, 3);
		break;
	}

	case 0x190:
	{
		AfxGetTask()->GetMainActor() << MsgObject(0, M983(0x2), 0, 0, 0, 0, 0);
		FrForm* form = NOTIFY_FORM();

		int result = packet.Decode1();
		if (result == 0)
		{
			unsigned long tid = packet.Decode4();
			unsigned long guid = packet.Decode4();
			std::list<sItemInfo>& items =
				((tid >> 26) == 2) ? Doc()->m_partsList : Doc()->m_myItemList;

			std::list<sItemInfo>::iterator it =
				std::find_if(items.begin(), items.end(), _FindByGuid(guid));
			if (it != items.end())
			{
				items.erase(it);
				form->SetMessage(K2L_Compatibility(
					"\xb4\xeb\xbf\xa9 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xbb\xe8\xc1\xa6 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				AfxGetTask()->GetMainActor() << MsgObject(0, M983(0xe1), 0, 0, 0, 0, 0);
			}
			else
			{
				form->SetMessage(K2L_Compatibility(
					"\xc7\xd8\xb4\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xbf\xa1 \xb4\xeb\xc7\xd1 \xc1\xa4\xba\xb8\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
			}
		}
		else
		{
			switch (result)
			{
			case 1:
				form->SetMessage(K2L_Compatibility(
					"\xc7\xd8\xb4\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba \xb4\xeb\xbf\xa9 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xbe\xc6\xb4\xd5\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 2:
				form->SetMessage(K2L_Compatibility(
					"\xc7\xd8\xb4\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xbf\xa1 \xb4\xeb\xc7\xd1 \xc1\xa4\xba\xb8\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 4:
				form->SetMessage(K2L_Compatibility(
					"\xc4\xab\xb5\xe5\xb0\xa1 \xc0\xd6\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba \xbb\xe8\xc1\xa6 \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 5:
				form->SetMessage(K2L_Compatibility(
					"\xbe\xc6\xc0\xcc\xc5\xdb \xbb\xe8\xc1\xa6 \xbf\xe4\xc3\xbb\xc0\xcc \xb0\xc5\xba\xce \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				break;
			default:
				form->SetMessage(
					MakeStr(K2L_Compatibility(
						"\xc3\xb3\xb8\xae\xbf\xa1 \xbd\xc7\xc6\xd0 \xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.(%d)"),
						result),
					false);
				break;
			}
		}

		form->Open(NULL, 3);
		break;
	}

	case PKT645_SEM_983(0x1b3):
	{
		AfxGetTask()->GetMainActor() << MsgObject(0, M983(0x2), 0, 0, 0, 0, 0);
		unsigned long result = packet.Decode4();
		if (result != 1)
			AfxGetTask()->GetActor(s_guildActor[1])
				<< MsgObject(0, M983(0x263), 0x1a, result, 0, 0, 0);
		else
			AfxGetTask()->GetActor(s_guildActor[1])
				<< MsgObject(0, M983(0x263), 0x1a, 0x407, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x1bd):
	{
		unsigned long result = packet.Decode4();
		if (result != 1)
		{
			AfxGetTask()->GetActor(s_guildActor[1])
				<< MsgObject(0, M983(0x263), 0x1a, result, 0, 0, 0);
			return 1;
		}
		else
		{
			GUILD_USER_INFO info;
			packet.DecodeBuffer(&info, 0x119);
			AfxGetTask()->GetActor(s_guildActor[1])
				<< MsgObject(0, M983(0x263), 5, (int)&info, 0, 0, 0);
		}
		break;
	}

	case PKT645_SEM_983(0x1b4):
	{
		AfxGetTask()->GetMainActor() << MsgObject(0, M983(0x2), 0, 0, 0, 0, 0);
		unsigned long result = packet.Decode4();
		if (result != 1)
		{
			AfxGetTask()->GetActor(s_guildActor[1])
				<< MsgObject(0, M983(0x263), 0x1a, result, 0, 0, 0);
			return 1;
		}
		else
		{
			IActor* guild = AfxGetTask()->GetActor(s_guildActor[1]);
			guild << MsgObject(0, M983(0x263), 0x1a, 0x405, 0, 0, 0);
			std::string name;
			packet.DecodeStr(name);
			guild << MsgObject(0, M983(0x263), 3, (int)&name, 0, 0, 0);
		}
		break;
	}

	case PKT645_SEM_983(0x1b5):
	{
		AfxGetTask()->GetMainActor() << MsgObject(0, M983(0x2), 0, 0, 0, 0, 0);
		unsigned long result = packet.Decode4();
		if (result != 1)
		{
			AfxGetTask()->GetActor(s_guildActor[1])
				<< MsgObject(0, M983(0x263), 0x1a, result, 0, 0, 0);
			return 1;
		}
		else
		{
			IActor* guild = AfxGetTask()->GetActor(s_guildActor[1]);
			guild << MsgObject(0, M983(0x263), 0x1a, 0x40f, 0, 0, 0);
			guild << MsgObject(0, M983(0x263), 0x19, 0, 0, 0, 0);
		}
		break;
	}

	case PKT645_SEM_983(0x1c7):
	{
		unsigned long result = packet.Decode4();
		if (result != 1)
		{
			AfxGetTask()->GetMainActor() << MsgObject(0, M983(0x2), 0, 0, 0, 0, 0);
			AfxGetTask()->GetActor(s_guildActor[1])
				<< MsgObject(0, M983(0x263), 0x1a, result, 0, 0, 0);
			return 1;
		}
		else
		{
			unsigned long param = packet.Decode4();
			std::string name;
			packet.DecodeStr(name);
			AfxGetTask()->GetActor(s_guildActor[1])
				<< MsgObject(0, M983(0x263), 0x15, param, (int)&name, 0, 0);
		}
		break;
	}

	case PKT645_SEM_983(0x1c8):
	{
		AfxGetTask()->GetMainActor() << MsgObject(0, M983(0x2), 0, 0, 0, 0, 0);
		unsigned long result = packet.Decode4();
		if (result != 1)
			AfxGetTask()->GetActor(s_guildActor[1])
				<< MsgObject(0, M983(0x263), 0x1a, result, 0, 0, 0);
		else
			AfxGetTask()->GetActor(s_guildActor[1])
				<< MsgObject(0, M983(0x263), 0x1a, 0x41d, 0, 0, 0);
		break;
	}

	case 0x1b0:
	{
		// PKT (frente C, lote 12): so do 983 (005aa98a) - resposta da loja SelfDesign; aviso pelo codigo e a mensagem 0x292
		AfxGetTask()->SendMsgToMainActor(MsgObject(0, M983(0x2), 0, 0, 0, 0, 0));
		int result = packet.Decode1();
		FrForm* form = NOTIFY_FORM();
		if (result == 0)
			form->SetMessage(K2L_Compatibility(
				"\xbd\xc5\xc3\xbb\xc0\xcc\xbf\xcf\xb7\xe1\xb5\xc8\xc0\xc7\xbb\xf3\xc0\xd4\xb4\xcf\xb4\xd9."), false);
		else if (result == 1)
			form->SetMessage(K2L_Compatibility(
				"\xbd\xc5\xc3\xbb\xc0\xcc\xbf\xcf\xb7\xe1\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."), false);
		else
		{
			char buf[0x100];
			buf[0] = 0;
			memset(buf + 1, 0, sizeof(buf) - 1);
			sprintf(buf, "[SelfDeSign_Shop] Unknown Message CODE:%d", result);
			form->SetMessage(buf, false);
		}
		form->Open(NULL, 3);
		AfxGetTask()->SendMsgToMainActor(MsgObject(0, 0x292 /*so do 983*/, 0, 0, 0, 0, 0));
		break;
	}

	case 0x1b2:
	{
		// PKT (frente C, lote 12): so do 983 (005aab01) - buracos jogados insuficientes (valor atual no pacote)
		FrForm* form = NOTIFY_FORM();
		char buf[0x80];
		buf[0] = 0;
		memset(buf + 1, 0, sizeof(buf) - 1);
		sprintf(buf, K2L_Compatibility(
			"\xc7\xc3\xb7\xb9\xc0\xcc\xc7\xd1 \xc8\xa6\xbc\xf6\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9 \xc7\xf6\xc0\xe7:%d\xc8\xa6"),
			packet.Decode4());
		if (form)
		{
			form->SetMessage(buf, false);
			form->Open(NULL, 3);
		}
		AfxGetTask()->SendMsgToMainActor(MsgObject(0, M983(0x2), 0, 0, 0, 0, 0));
		break;
	}

	case 0x175:
	case 0x177:
		return 1;

	default:
		return 0;
	}

	return 1;
}

// PKT (frente C, lote 39): a tarefa do 645 continua declarada (caminhos antigos da frente A ainda fazem ChangeTask para ela);
// os pacotes do my room agora sao da CMyRoomWidget (acima), alcancada pelo OnPacketCommon.
int CRealMyRoomTask::OnPacket(WReceivedPacket& packet)
{
	return OnPacketCommon(packet);
}
// 983: sem as conferências de deslocamento do 645 (CSharedDoc no layout do 983)
