#include "taskmanager_pch.h"
#include "vc10_layout.h"
// A357: o 983 sempre tem o pos-efeito (HDR/DOF/FXAA); nao depender so do DEFS do pgbuild (o a353-1 linkou um
// taskmanager.obj compilado sem a chave e perdeu os alvos fp16 / a estrela). JP983_SEM_POSTEFFECT desliga.
#if !defined(JP983_POSTEFFECT) && !defined(JP983_SEM_POSTEFFECT)
#define JP983_POSTEFFECT
#endif
#include "minatl.h"
#include "taskmanager.h"
#include "binstr.h"
#include "actor.h"
#include "netsyncactor.h"
#include "clientsetting.h"
#include "frgaugebar.h"
#include "fresh.h"
#include "rtti.h"
#include "golftask.h"
#include "chatmsg.h"
#include "projectg.h"
#include "avatar_avatar.h"
#include "frdesktop.h"
#include "frwndmanager.h"
#include "capturedbg.h"
#include "golfball.h"
#include "shoptask.h"
#include "standinginline.h"
#include "guilddefine.h"
#include "fx.h"
#include "messengerdlg.h"
#include "netresourcemanager.h"
#include "user_info.h"
#include "matchhistory.h"
#include "tradedlg.h"
#include "cardremove.h"

// HACK: class-template instantiation order (decides the order of the template COMDATs at the
// end of the unit, ORDER_MODEL section 2): the original instantiated these lists in this order
namespace ranktype
{
	struct sListInfo;
}
struct sPrize;
namespace _gateway
{
	struct sTradeBuyBase;
}
struct sRecycleItem;
struct MSNServerInfo;
struct sTradeItem;
typedef char rb_inst_order_1[sizeof(std::list<ranktype::sListInfo>) +
	sizeof(std::list<sPrize*>) + sizeof(std::list<CWork*>::iterator)];
typedef char rb_inst_order_2[sizeof(std::vector<sTreasureGift>) +
	sizeof(std::vector<sTradeItem>) +
	sizeof(std::vector<_gateway::sTradeBuyBase>)];
typedef char rb_inst_order_3[sizeof(std::list<sRoomInfo>) +
	sizeof(std::list<sChannelInfo>) + sizeof(std::list<sGameServerInfo>) +
	sizeof(std::list<sNoteInfo>) + sizeof(std::list<unsigned short>) +
	sizeof(std::list<MSNServerInfo>) + sizeof(std::vector<sTreasureGift>) +
	sizeof(std::list<sTradeItem>) +
	sizeof(std::map<std::string, IObject*(__fastcall*)()>) +
	sizeof(std::list<sSaleItem>)];
#include "buddymanager.h"
#include "avatar_task.h"
#include "hackingmanager.h"
#include "soundmanager.h"
#include "background.h"
#include "taskmain.h"
#include "privatetrade.h"
#include "s5/supplypack.h"
#include "tutorialtask.h"
#include "noticeboard.h"
#include "onelineboard.h"
namespace S5 { void SysTimeToSec983(const SYSTEMTIME* pTime, __int64* pOut); } // 00b30830 (D648)
#ifdef JP983_POSTEFFECT
#include "jp/posteffect983.h"
#endif
#include "lobbytask.h"
#include "rankingdlg.h"
#include "intrusion.h"
#include "pointeventdlg.h"
#include "networksystem.h"
#include "contentsdoc.h"
#include "treasure.h"
#include "securitykeymanager.h"
#include "dlgquickstartsetting.h"
#include "workalivecheck.h"
#include "winnernotice.h"
#include "itembuffactor.h"
#include "../shared/globalnetworkdefine.h"
#include "../shared/sharedtables.h"
#include "s5/utilities.h"
#include "guildactor.h"
#include "gatewayactor.h"
#include "cardsystem.h"
#include "powergauge.h" // PKT C39: casos 0x76 e 0x16a do 983
#include "packets/aiplayerinfocontainer.h" // PKT C75: caso 0x256 do 983 (documento 0xa3 da IA)
#include "../shared/thirties/xtdebug.h"
extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // A803 temporario
static void RastroGear0x76B1589(const char* ramo, const sUserInfo& ui);
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip.guidChar) == 0xa2a); // A737

// HACK: data of the naked CTask::OnPacketCommon tail that the original keeps in the unit's non-COMDAT
// block (start of .rdata): the level message table of case 0x112, followed by the two wmath.h constants that
// share its .rdata block.
static const struct
{
	struct
	{
		unsigned long id;
		const char* msg;
	} msg[3];
	float huge;
	float epsilon;
} rb_levelMsg = {
	{ { 0,
		  "\267\271\272\247\300\314 \303\346\272\320\307\317\301\366 \276\312\275\300\264\317\264\331. "
		  "\267\271\272\247\276\367 \310\304\277\241 \302\370\277\353\307\317\275\312\275\303\277\300." },
     { 1,
			"\267\271\272\247\300\314 \303\346\272\320\307\317\301\366 \276\312\275\300\264\317\264\331. "
			"\267\271\272\247\276\367 \310\304\277\241 \300\314\277\353\307\317\275\312\275\303\277\300." },
     { 2,
			"\275\303\273\363\275\304\300\314 \276\306\301\367 \301\276\267\341\265\307\301\366 \276\312\276\306\274\255, "
			"\304\332\275\272\270\246 \272\257\260\346 \307\322 \274\366 \276\370\275\300\264\317\264\331. "
			"\300\341\275\303 \310\304 \272\257\260\346\307\330 \301\326\275\312\275\303\277\300." } },
	3.402823466e+38f, 0.00001f
};

// HACK: IL filler. In the original the body of CTask::OnPacketCommon sits in the main IL stream; ours is in the
// RbOnpacketcommonTail template (end of the unit), so the PCH inlines of <stdexcept> (logic_error/length_error
// ctors) would be merged too late and std::deque<sPostMsg>::_Xlen, which inlines them, would wait one wave.
// clang-format off
namespace RbTaskManagerIlFill
{
	struct Pad
	{
		int v;
		ILFILLFN(1) ILFILLFN(2) ILFILLFN(3) ILFILLFN(4)
	};
}
// clang-format on

class CRealMyRoomTask
{
public:
	static const WRTTI m_RTTI;
};

// CFx::ClearFxKeeper is not declared in fx.h; adding it there perturbs
// projectg.cpp (SaveScreenShot), so it is declared in a local helper class.
class CFxTaskEx : public CFx
{
public:
	void ClearFxKeeper();
};

#pragma comment(linker, \
	"/alternatename:?ClearFxKeeper@CFxTaskEx@@QAEXXZ=?ClearFxKeeper@CFx@@QAEXXZ")

extern Fresh* g_pFresh;
extern bool g_bQuit;
extern int g_iRecvPacketType;
extern int g_iRecvDisconCode;
extern FrPointEventDlg* g_pPointEventDlg;
bool IsAngelWing(unsigned long typeId);

void __cdecl TextOut(WView* view, float x, float y, const char* format, ...);

// HACK: members the precompiled variants of scenemanager.h / avatar_avatar.h do not declare
// (declaring them there changes the name count of the PCH)
struct RbSceneManagerTM
{
	void Process(float dt);
	void Prerender(WView* view);
	void Render(WView* view, unsigned long flag);
	void RenderEffect(WView* view);  // 983 009b11b0 = jmp Render_RenderEffect (009b1180)
};
#pragma comment(linker, \
	"/alternatename:?RenderEffect@RbSceneManagerTM@@QAEXPAVWView@@@Z=?Render_RenderEffect@CSceneManager@@IAEXPAVWView@@@Z")
#pragma comment(linker, \
	"/alternatename:?Process@RbSceneManagerTM@@QAEXM@Z=?Process@CSceneManager@@QAEXM@Z")
#pragma comment(linker, \
	"/alternatename:?Prerender@RbSceneManagerTM@@QAEXPAVWView@@@Z=?Prerender@CSceneManager@@QAEXPAVWView@@@Z")
#pragma comment(linker, \
	"/alternatename:?Render@RbSceneManagerTM@@QAEXPAVWView@@K@Z=?Render@CSceneManager@@QAEXPAVWView@@K@Z")
struct RbAvatarManagerTM
{
	void SetActive(bool bActive);
};
#pragma comment(linker, \
	"/alternatename:?SetActive@RbAvatarManagerTM@@QAEX_N@Z=?SetActive@ntAvatarManager@@QAEX_N@Z")

struct sLoadParam
{
	WList<sActor*>* pList;
	int total;
	int loaded;
	bool bLocked;
	bool bDone;
};

sLoadParam g_LoadParam;

// IObject::GetRTTI is pure in objectfactory.h (shared with Fresh); the
// original's CTaskDoc vtable points at a non-pure IObject::GetRTTI.
#pragma comment(linker, \
	"/alternatename:?GetRTTI@CTaskDoc@@UBEPBVWRTTI@@XZ=" \
	"?GetRTTI@IObject@@UBEPBVWRTTI@@XZ")

void __fastcall RecvRSS0(int)
{
}

IMPLEMENT_OBJECT(CTask, IObject)

IActor* CTask::ms_pDummyActor = ActorFactory().CreateActor("Dummy");

#include "exception.h"
#include "jp/nullwidget.h" // C46.1: boneco de +0x1fc
CTaskWidget* CTask::ms_pNullWidget983 = NULL; // 00f006f8

CTask::CTask()
	: m_bSyncLoad(false),
	  m_actorList(16, 16),
	  m_pMainActor(NULL),
	  m_pDoc(NULL),
	  m_bLobbyScreenSize(true),
	  m_loadingBackGround(""),
	  m_bProgressBar(true),
	  m_bTip(true),
	  m_pLoadingFresh(NULL),
	  m_pProgressGauge(NULL)
{
	// The original stores m_taskFlag twice (init list and body).
	*(volatile unsigned long*)&m_taskFlag = 0;
	m_bPause = false;
	m_taskFlag = 0;
	m_netSyncTime = 0;
	m_bRecieveWhisper = true;
	m_pPostEffect983 = NULL; // 983 009bf433 (nota do C53.1)
	// 983 009bf436-009bf52a (pedido C46.1): o boneco de +0x1fc e criado uma vez, pela fabrica, e dividido por todas as
	// tarefas
	if (ms_pNullWidget983 == NULL)
	{
		IObject* pObj = ObjectFactory().CreateObject(CNullWidget::m_RTTI.GetName());
		if (pObj == NULL)
			throw WAppException(std::string(MakeStr("Factory failed to create : %s Obj", CNullWidget::m_RTTI.GetName())));
		ms_pNullWidget983 = DYNAMIC_CAST(CTaskWidget, pObj);
		if (ms_pNullWidget983 == NULL)
			throw WAppException(std::string(MakeStr("Casting failed to change type")));
	}
	m_pWidget983 = ms_pNullWidget983;
}

// HACK: in-class inline bodies in the original header; defined here so that they are emitted
// after the ??_E adjustor thunk, as in the original
const WRTTI* CTask::GetRTTI() const
{
	if (0)
		((CTask*)0)->CTask::
			CTask(); // HACK: waits for the constructor (emission order)
	return &m_RTTI;
}

void CTask::Register()
{
	if (0)
		((CTask*)0)->CTask::
			CTask(); // HACK: waits for the constructor (emission order)
}

int CTask::OnPacket(WReceivedPacket& packet)
{
	if (0)
		((CTask*)0)->CTask::
			CTask(); // HACK: waits for the constructor (emission order)
	return 0;
}

CTask::~CTask()
{
	// 983 009bc1b6-009bc1d0 (pedido C46.1): fecha a widget e apaga o boneco (a proxima tarefa cria outro; uma tarefa
	// preservada fica com o ponteiro velho, como no 983)
	CloseWidget983();
	if (ms_pNullWidget983)
	{
		delete ms_pNullWidget983;
		ms_pNullWidget983 = NULL;
	}
	for (sActor* pActor = m_actorList.Start(); pActor;
		pActor = m_actorList.Next())
	{
		if (pActor->actor)
			delete pActor->actor;
		delete[] (char*)pActor;
	}
	m_actorList.Reset();
	if (m_pDoc)
	{
		delete m_pDoc;
		m_pDoc = NULL;
	}
	if (m_pLoadingFresh)
	{
		delete m_pLoadingFresh;
		m_pLoadingFresh = NULL;
	}
	if (CFx::Instance())
		((CFxTaskEx*)CFx::Instance())->ClearFxKeeper();
#ifdef JP983_POSTEFFECT
	// 983 009bc1f6: dtor 009b74d0 + operator delete do pos-efeito
	if (m_pPostEffect983)
	{
		delete m_pPostEffect983;
		m_pPostEffect983 = NULL;
	}
#endif
}

void CTask::SetMainActor(IActor* pActor)
{
	m_pMainActor = pActor;
}

bool CTask::OnTipDlgResult(int result, FrForm* pForm)
{
	m_pTipForm = NULL;
	m_pProgressGauge = NULL;
	return true;
}

FrForm* CTask::LoadTipText(int a, int b)
{
	cFile* file = g_resrcmng->GetCFile("tip.txt", 0xffff);
	if (!file)
		return NULL;

	std::list<TIP_ITEM> tipList;
	char line[8192];
	while (file->Scan("%n", line))
	{
		int len = 0;
		for (int i = 0; line[i]; ++i, ++len)
		{
			if (line[i] == '\\' && line[i + 1] == 'n')
			{
				line[len] = '\n';
				++i;
			}
			else
				line[len] = line[i];
		}
		line[len] = 0;

		int length = strlen(line);
		cTokenV token;
		token.Init(line, length);
		char* id;
		token.GetTokenWithoutFail(&id, " \t", 2);
		if (token.GetPos() + 1 > length)
		{
			CloseCFile(file);
			return NULL;
		}

		TIP_ITEM item;
		item.id = atoi(id);
		item.text = line + token.GetPos();
		tipList.push_back(item);
	}
	CloseCFile(file);

	int index = rand() % tipList.size() + 1;
	xtAlways<false>::
		Get(); // residue of an xtdebug.h macro; the original call site is unknown
	FrForm* pForm =
		CreateForm<FrForm>(m_pLoadingFresh->GetManager(), this, "TIP", NULL);
	if (index && pForm)
	{
		std::list<TIP_ITEM>::iterator it = tipList.begin();
		while (it != tipList.end() && index > 1)
		{
			++it;
			--index;
		}
		// 983 009c1c8a: centrada na base da tela; a faixa "gradation" ocupa a largura toda
		float viewW = (float)g_view->GetWidth();
		float viewH = (float)g_view->GetHeight();
		const WRect& tipRect = pForm->GetRect();
		pForm->MoveWindow(WPoint((float)(int)((viewW - tipRect.w) * 0.5),
			(float)(int)(viewH - tipRect.h)));
		FrWnd* pGradation = pForm->FindChildByName("gradation");
		if (pGradation)
			pGradation->SetRect(WRect(0.0f, viewH - 244.0f, viewW, 244.0f));
		pForm->SetMessage(it->text.c_str(), false);
		pForm->EnableDrag(false);
		FrForm::ms_bModalLock983 = true; // 983: call 00be7950
		pForm->Open((FRESH_PFN_RESULT)&CTask::OnTipDlgResult, 8);
		return pForm;
	}
	return NULL;
}

void CTask::SetProgressBar(bool bProgressBar)
{
	m_bProgressBar = bProgressBar;
}

void CTask::SetLoadingBackGround(const std::string& name)
{
	m_loadingBackGround = name;
}

IActor* CTask::AddActor(const char* className, const char* name, int skipLayer,
	const std::string& option)
{
	IActor* pActor = ActorFactory().CreateActor(className);
	if (pActor == NULL)
		return NULL;
	pActor->m_pDoc = m_pDoc;
	pActor->m_pTask = this;
	sActor* pInfo = (sActor*)new char[strlen(className) + sizeof(sActor)];
	pInfo->actor = pActor;
	pInfo->name = name;
	pInfo->actor->RemoveSkipList(skipLayer);
	pInfo->flag = false;
	m_actorList.AddItem(pInfo, pInfo->name, false);
	if (!option.empty())
		pActor << MsgObject(NULL, 0x2712, (int)option.c_str(), 0, 0, 0, 0);
	return pActor;
}

// 983 009b7810 (s100): o form do CTaskWidget de +0x1fc (CTaskWidget::GetForm, cópia fora de linha 0064f1f0 = `return +0x1a0`;
// include/jp/taskwidget.h) ou NULL quando lá está o boneco vazio. Mesma leitura crua do IsCurrentActor983 abaixo.
FrTaskForm* CTask::GetWidgetForm983() const
{
	if (m_pWidget983 == NULL || m_pWidget983 == ms_pNullWidget983)
		return NULL;
	return m_pWidget983->GetForm();
}

// A762: CTaskWidget::m_pForm (+0x1a0) e protegido; a escrita passa por esta visao
struct CTaskWidgetView983 : public CTaskWidget
{
	using CTaskWidget::m_pForm;
};

// (B370, C115.4) o form foi liberado sem passar por Close/OnOK/OnCancel: a widget da tarefa nao pode ficar com o ponteiro solto
// (HandleMsg983 0x88 chamava form->Close sobre memoria liberada ao abrir My Room depois da Loja)
void CTask::ForgetWidgetForm983(FrTaskForm* pForm)
{
	if (m_pWidget983 && m_pWidget983 != ms_pNullWidget983 && m_pWidget983->GetForm() == pForm)
		((CTaskWidgetView983*)m_pWidget983)->m_pForm = NULL;
}

// S65: 983 009bd4f0 (sem par no 645): a widget de m_pWidget983 (+0x1fc), se não for o ms_pDummyActor, tem em +0x1a0 um
// objeto cujo nome (std::string em +0x64, via 00be76b0) é comparado com `name`. Nome e tipos provisórios.
bool CTask::IsCurrentActor983(const char* name)
{
	if (m_pWidget983 == ms_pNullWidget983)
		return false;
	void* pObj = m_pWidget983->GetForm();
	if (pObj == NULL)
		return false;
	// D861: era o cru std::string em +0x64 (m_wndName no VC10; anda no cl novo) -> 983 chama 00be76b0 = GetWindowName983
	return ((FrWnd*)pObj)->GetWindowName983().compare(name) == 0;
}

// 983 009b7780 (pedido C46.1)
void CTask::SetWidget983(CTaskWidget* pWidget)
{
	m_pWidget983 = pWidget;
}

// 983 009b7790 (pedido C46.1): chamada pelo OpenWidget983 quando a fabrica nao conhece o nome
void CTask::DeleteWidget983()
{
	if (m_pWidget983 == ms_pNullWidget983)
		return;
	if (m_pWidget983)
	{
		delete m_pWidget983;
		m_pWidget983 = NULL;
	}
	m_pWidget983 = ms_pNullWidget983;
}

// 983 009b77d0 (pedido C46.1): fecha a widget aberta (CTaskMain::OnUnderBar_BackUp, FrTaskForm, OpenWidget983, ~CTask)
void CTask::CloseWidget983()
{
	CTaskWidget* pNull = ms_pNullWidget983;
	if (m_pWidget983 != pNull && m_pWidget983)
	{
		delete m_pWidget983;
		m_pWidget983 = NULL;
		pNull = ms_pNullWidget983;
	}
	m_pWidget983 = pNull;
}

IActor* CTask::GetActor(const char* name)
{
	sActor* pActor = m_actorList.Find(name);
	if (pActor == NULL)
		return NULL;
	return pActor->actor;
}

int __fastcall IsMassGame(); // 983 00502f20

// 983 009b7690: fora de linha (o OnAnotherHand chama em 009bad71)
__declspec(noinline) void __fastcall SendPacket(unsigned char type)
{
	WSendPacket packet(0x48);
	packet.Encode1(type);
	packet.Send(TO_GAME);
}

void CTask::OnAnotherHand(int key, void* pParam)
{
	if (key == 0)
	{
		sLoadParam* pLoad = (sLoadParam*)pParam;
		sActor* pActor = pLoad->pList->Start();
		while (pActor)
		{
			timeGetTime();
			// 983 009bac70: o passo 0x48 sai a cada 4 atores (645: 3) e o ultimo (10) quando este e o ultimo ator da lista
			// (645: o 23o fixo); IsMassGame() 00502f20 = os mesmos tipos 4/5/6/9/10 do switch do 645.
			if (IS_KINDOF(CGolfTask, this) && OnlinePlay() && !IsMassGame())
			{
				if (pLoad->loaded % 4 == 0)
				{
					unsigned char step;
					if (pLoad->loaded + 1 == pLoad->pList->GetCount())
						step = 10;
					else
						step = pLoad->loaded / 4 + 2;
					SendPacket(step);
				}
			}
			pActor->actor->OnLoad();
			timeGetTime();
			pActor = pLoad->pList->Next();
			pLoad->loaded++;
		}
		pLoad->bDone = true;
	}
}

static const char rb_strLoadingXml[] = "loading.xml";
static const char rb_strLoading[] = "LOADING";
static const char rb_strProgress[] = "progress";
static const float rb_fadeStepMax = 0.04f;

void CTask::Load()
{
	g_LoadParam.pList = &m_actorList;
	g_LoadParam.total = g_LoadParam.pList->GetCount();
	m_pLoadingFresh = new Fresh;
	m_pProgressGauge = NULL;
	if (m_loadingBackGround != "")
		m_pLoadingFresh->Init(rb_strLoadingXml, false,
			m_loadingBackGround.c_str());
	else
		m_pLoadingFresh->Init(rb_strLoadingXml, false, NULL);
	m_pLoadingFresh->OpenLayout(rb_strLoading, NULL, true);
	if (m_bTip)
	{
		m_pTipForm = LoadTipText(0, 0);
		if (m_pTipForm)
		{
			m_pProgressGauge = DYNAMIC_CAST(FrGaugeBar,
				m_pTipForm->FindChildByName(rb_strProgress));
			if (m_pProgressGauge)
			{
				m_pProgressGauge->SetRange(0, g_LoadParam.total * 100, 0);
				m_pProgressGauge->SetDestPos(100, false);
			}
		}
	}
	if (m_bSyncLoad)
		OnAnotherHand(0, &g_LoadParam);
	else
		m_anotherHand.Work(this, 0, &g_LoadParam, false, 0);
}

void CTask::ApplyScreenSize()
{
	if (COption::Instance() == NULL)
		return;
	if (m_bLobbyScreenSize)
		COption::Instance()->vApplyLobbyScreenSize();
	else
		COption::Instance()->vApplyGameScreenSize();
}

void CTask::SetInited(bool bInited)
{
	if (0)
		((CTask*)0)->CTask::ApplyScreenSize();
	// HACK: keeps the inline IObject::GetRTTI of objectfactory.h emitted in this unit
	if (0)
		IObject::GetRTTI();
	if (bInited)
		m_taskFlag |= 2;
	else
		m_taskFlag &= ~2;
}

void CTask::Init(const char* wallPaper)
{
	if (m_pLoadingFresh)
	{
		delete m_pLoadingFresh;
		m_pLoadingFresh = NULL;
	}
	for (sActor* pActor = m_actorList.Start(); pActor;
		pActor = m_actorList.Next())
		pActor->actor->OnInit();
	if (((unsigned long)m_taskFlag >> 1 & 1) == 0)
	{
		m_taskFlag |= 2;
		OnInitFinished();
	}
	g_pFresh->GetManager()->GetDesktop()->Init(g_view->GetWidth(),
		g_view->GetHeight());
	// 983 009badd0: sem SetWallPaper aqui (o papel de parede fica o que estava; os ghost/shop/family/tutorial_background
	// do 645 nem existem no exe 983, e SetWallPaper 00c016a0 so tem 5 chamadores: golftask Destroy, scoreboard, avatar chat, fresh)
	(void)wallPaper;
	if (!IS_EXACTKINDOF(CGolfTask, AfxGetTask()))
		WSingleton<CChatMsg>::Instance()->SetActive(true, true, true);
}

void CTask::PreLoadInit()
{
	for (sActor* pActor = m_actorList.Start(); pActor;
		pActor = m_actorList.Next())
		pActor->actor->OnPreLoadInit();
}

void CTask::Reload()
{
	for (sActor* pActor = m_actorList.Start(); pActor;
		pActor = m_actorList.Next())
		pActor->actor->OnReload();
}

void CTask::Destroy()
{
	g_pFresh->CloseLayout();
	for (sActor* pActor = m_actorList.Start(); pActor;
		pActor = m_actorList.Next())
		pActor->actor->OnDestroy();
}

void CTask::PreserveBack(const char* name)
{
	for (sActor* pActor = m_actorList.Start(); pActor;
		pActor = m_actorList.Next())
		pActor->actor->OnPreserveBack(name);
}

void CTask::RestorePreserved()
{
	for (sActor* pActor = m_actorList.Start(); pActor;
		pActor = m_actorList.Next())
		pActor->actor->OnRestorePreserved();
}

void CTask::ProcessNetSyncActor(bool bForce, float step, int ballState)
{
	for (sActor* pActor = m_actorList.Start(); pActor;
		pActor = m_actorList.Next())
	{
		if (!pActor->actor->Skipped())
		{
			NetSyncActor* pSync = (NetSyncActor*)pActor->actor->DynamicCast(
				&NetSyncActor::m_RTTI);
			if (pSync)
				pSync->NSA_Step(bForce, step, ballState);
		}
	}
}

void CTask::OnProcessWhileLoading(float delta)
{
	static int s_lastLoaded;

	if (m_pLoadingFresh)
	{
		m_pLoadingFresh->OnProcess(delta);
		if (s_lastLoaded != g_LoadParam.loaded)
		{
			s_lastLoaded = g_LoadParam.loaded;
			if (m_bProgressBar && m_pProgressGauge)
			{
				m_pProgressGauge->SetPos(g_LoadParam.loaded * 100 - 100);
				m_pProgressGauge->SetDestPos(g_LoadParam.loaded * 100, false);
			}
		}
	}
}

void CTask::Process(float delta)
{
	C_ASSERT(offsetof(CTask, m_actorList) == 0x10 && offsetof(CTask, m_pMainActor) == 0x30); // 983 009bb537/009bb558
	m_pMainActor->OnProcess(delta);
	if (m_bPause)
		return;

	for (sActor* pActor = m_actorList.Start(); pActor;
		pActor = m_actorList.Next())
	{
		if (pActor->actor != m_pMainActor && !pActor->actor->Skipped())
			pActor->actor->OnProcess(delta);
		if (HackingManager::Instance())
			HackingManager::Instance()->CheckCrypticValue();
	}

	bool bFirstFrame = false;
	if (GolfBall().m_playFrame == 0)
		bFirstFrame = true;
	if (delta > 0.0f)
	{
		m_netSyncTime += delta;
		if (bFirstFrame)
			m_netSyncTime = 0.022f;
		while (m_netSyncTime > 0.02f)
		{
			if (GolfBall().m_playFrame == -1 ||
				GolfBall().m_playFrame == GolfBall().m_renderNum)
				ProcessNetSyncActor(false, 0.02f, GolfBall().m_playFrame);
			else if (GolfBall().m_playFrame == 0)
				ProcessNetSyncActor(true, 0.02f, GolfBall().m_playFrame);
			else
				GolfBall();
			m_netSyncTime -= 0.02f;
		}
	}

	if (CSceneManager::Instance() && !CCapturedBg::Instance()->IsCaptured())
	{
		if (!IS_EXACTKINDOF(CShopTask, AfxGetTask()))
			((RbSceneManagerTM*)CSceneManager::Instance())->Process(delta);
	}
}

void CTask::OnDisplayWhileLoading()
{
	if (m_pLoadingFresh)
		m_pLoadingFresh->OnDisplay(true);
}

// 983 009b9570: a tela de carga so aparece com papel de parede (fundo); sem ele (Practice) o CTaskManager mantem a foto
// da tarefa anterior opaca por cima ate o fim da carga (lobby "congelado" no original)
bool CTask::IsLoadingWallPaperVisible983() const
{
	FrDesktop* desktop = m_pLoadingFresh ? m_pLoadingFresh->GetManager()->GetDesktop() : NULL;
	return desktop && desktop->IsWallPaperVisible();
}

// 983 (B159.2): ganchos da RF (RFSceneManager, frente B); corpo no projectg.cpp, que inclui a RF
void Rf983PreDraw();	   // 009bb6a9: RFSceneManager::PreDraw983 no fim do CTask::Display
void Rf983Draw();		   // 009b974a: RFSceneManager::Draw983 + Fresh 00be2840 no CTaskManager::DisplayTask
void Rf983Update(float delta); // 009c0de6: RFSceneManager::Update983 no fim do CTaskManager::ProcessTask
void Rf983ChangeTask();	   // 009b96ac: fecha a oficina de tacos e apaga a area de trabalho da RF no CTaskManager::ChangeTask

#if defined(JP983_POSTEFFECT)
// 983 009bb490: CTask::Display com o pos-efeito (CPostEffect983 em +0x68, criado no 1o Display). Com o fundo capturado
// nao desenha cena nem pos-efeito; Flush(0,1) em i == 2/3/4 e Clear(0,2)/(0,8) se !(m_taskFlag & 1)
void CTask::Display()
{
	const bool bCaptured = CCapturedBg::Instance() && CCapturedBg::Instance()->IsCaptured();
	if (m_pPostEffect983 == NULL)
	{
		m_pPostEffect983 = new CPostEffect983(g_view);
		(m_pPostEffect983)->Setup(g_view);
	}
	CPostEffect983* fx = m_pPostEffect983;
	bool bPending = !bCaptured;
	if (bPending)
		fx->BeginScene();
	void* pos;
	for (int i = 0; i <= 5; i++)
	{
		for (sActor* pActor = m_actorList.Start(pos); pActor;
			pActor = m_actorList.Next(pos))
		{
			// 983 009bb556: o ator principal (+0x30) desenha mesmo Skipped()
			if (pActor->actor && pActor->actor->GetDisplayPriority() == i &&
				(pActor->actor == m_pMainActor || !pActor->actor->Skipped()))
				pActor->actor->OnDisplay();
		}
		if (bCaptured)
			continue;
		if (i == 2)
		{
			if (CSceneManager::Instance())
			{
				((RbSceneManagerTM*)CSceneManager::Instance())
					->Prerender(g_view);
				((RbSceneManagerTM*)CSceneManager::Instance())
					->Render(g_view, -1);
			}
			fx->EndScene();
			bPending = false;
			// 983 009bb5ce-009bb62e: tarefa atual IsKindOf(CGolfTask) (00e78fac) e fx->IsActive() (005b0030) ->
			// CSceneManager 009b11b0(g_view) = Render_RenderEffect: efeitos (m_custom2) por cima da cena ja pos-processada
			if (AfxGetTask() && AfxGetTask()->DynamicCast(&CGolfTask::m_RTTI) && fx->IsActive() &&
				CSceneManager::Instance())
				((RbSceneManagerTM*)CSceneManager::Instance())->RenderEffect(g_view);
			g_view->Flush(0, 1);
			if (!(m_taskFlag & 1))
				g_view->Clear(0, 2);
		}
		else if (i == 3 || i == 4)
		{
			g_view->Flush(0, 1);
			if (!(m_taskFlag & 1))
				g_view->Clear(0, 8);
		}
	}
	if (bPending)
		fx->EndScene();
	Rf983PreDraw();
}
#else
void CTask::Display()
{
	void* pos;
	for (int i = 0; i <= 5; i++)
	{
		for (sActor* pActor = m_actorList.Start(pos); pActor;
			pActor = m_actorList.Next(pos))
		{
			// 983 009bb556: o ator principal (+0x30) desenha mesmo Skipped()
			if (pActor->actor && pActor->actor->GetDisplayPriority() == i &&
				(pActor->actor == m_pMainActor || !pActor->actor->Skipped()))
				pActor->actor->OnDisplay();
		}
		if (i == 2)
		{
			if (CCapturedBg::Instance() == NULL ||
				!CCapturedBg::Instance()->IsCaptured())
			{
				if (CSceneManager::Instance())
				{
					((RbSceneManagerTM*)CSceneManager::Instance())
						->Prerender(g_view);
					((RbSceneManagerTM*)CSceneManager::Instance())
						->Render(g_view, -1);
				}
				if (!(m_taskFlag & 1))
					g_view->Clear(0, 2);
			}
		}
		else if (i == 4 || i == 1)
		{
			g_view->Flush(0);
		}
	}
	Rf983PreDraw();
}
#endif

void CTask::SendGlobalMsg(MsgObject msg)
{
	m_pMainActor << msg;
	void* pos;
	for (sActor* pActor = m_actorList.Start(pos); pActor;
		pActor = m_actorList.Next(pos))
	{
		IActor* actor = pActor->actor;
		if (actor != m_pMainActor && actor)
			actor << msg;
	}
}

void CTask::SendMsgToMainActor(MsgObject msg)
{
	m_pMainActor << msg;
}

#include "mousecursor.h"
extern HWND g_hwnd;
void __fastcall SetHoleType(unsigned char holeType);
void __fastcall SetShotTimeLimit(unsigned long limit);
void __fastcall SetGameTimeLimit(unsigned long limit);
int __fastcall IsMassGame();
struct RbBongdariShopDlgClear : public FrWnd
{
	void Clear();
};
#pragma comment(linker, \
	"/alternatename:?Clear@RbBongdariShopDlgClear@@QAEXXZ=?Clear@FrBongdariShopDlg@@QAEXXZ")
struct RbGimmickContainerPacket
{
	void LoadFromPacket(WReceivedPacket& packet);
};
#pragma comment(linker, \
	"/alternatename:?LoadFromPacket@RbGimmickContainerPacket@@QAEXAAVWReceivedPacket@@@Z=?LoadFromPacket@CGimmickContainer@@QAEXAAVWReceivedPacket@@@Z")
#include "serverdlg.h"
#include "uccmanager.h"
#include "netresourcemanager.h"
#include "skinnedtheme.h"
#include "messagemanager.h"
namespace S5
{
	namespace UTIL
	{
		void __fastcall SetServerProperty(unsigned long property);
	}
}
struct RbPeriodContentsPreSet
{
	void Set_PreSettingContentsDate();
};
#pragma comment(linker, \
	"/alternatename:?Set_PreSettingContentsDate@RbPeriodContentsPreSet@@QAEXXZ=?Set_PreSettingContentsDate@CPeriodContents@@QAEXXZ")
#pragma pack(push, 1)
struct sRoomInfoLegacy
{
	char title[32];
	unsigned char bPublic : 1;
	unsigned char bAvailable : 1;
	unsigned char bIntrusion : 1;
	unsigned char reserved20 : 5;
	unsigned char nUserLimit;
	unsigned char nUserNum;
	unsigned char nHole;
	unsigned char gameType;
	unsigned short roomGuid;
	unsigned char mapType;
	unsigned long tidMatch;
	sGuildRoomInfo GuildInfo;
	unsigned char reserved76[5];
};
#pragma pack(pop)
#include "uccmanager.h"
#include "netresourcemanager.h"
struct RbCardBoxDlg : public FrWnd
{
	void RecievedPacket(WReceivedPacket* pPacket);
};
#pragma comment(linker, \
	"/alternatename:?RecievedPacket@RbCardBoxDlg@@QAEXPAVWReceivedPacket@@@Z=?RecievedPacket@FrCardBoxDlg@@QAEXPAVWReceivedPacket@@@Z")
struct RbL8380SharedDoc
{
	void SetTradeMode(unsigned char mode);
};
#pragma comment(linker, \
	"/alternatename:?SetTradeMode@RbL8380SharedDoc@@QAEXE@Z=?SetTradeMode@CSharedDoc@@QAEXE@Z")
#ifdef JP983_VS_DEBUG // D843: /Od nao emite o inline alvo do alias; o endereco obriga o COMDAT
extern void (CSharedDoc::* const g_jp983DbgSetTradeMode)(unsigned char) = &CSharedDoc::SetTradeMode;
#endif
#include "offlinetradedlg.h"
struct RbL8451ShopEditDlg
{
	void SetListTradeItem();
};
#pragma comment(linker, \
	"/alternatename:?SetListTradeItem@RbL8451ShopEditDlg@@QAEXXZ=?SetListTradeItem@COfflineShopEditDlg@@QAEXXZ")
struct RbL8451FinishEditDlg
{
	void SetListTradeItem(std::list<sTradeItem>* pList);
};
#pragma comment(linker, \
	"/alternatename:?SetListTradeItem@RbL8451FinishEditDlg@@QAEXPAV?$list@UsTradeItem@@V?$allocator@UsTradeItem@@@std@@@std@@@Z=?SetListTradeItem@FrOfflineTradeFinishEditDlg@@QAEXPAV?$list@UsTradeItem@@V?$allocator@UsTradeItem@@@std@@@std@@@Z")
struct RbL8451ShopDlg
{
	void SetListTradeItem(bool flag);
};
#pragma comment(linker, \
	"/alternatename:?SetListTradeItem@RbL8451ShopDlg@@QAEX_N@Z=?SetListTradeItem@COfflineShopDlg@@QAEX_N@Z")
struct RbL8451VisitorDlg
{
	void SetListTradeItem(bool flag);
};
#pragma comment(linker, \
	"/alternatename:?SetListTradeItem@RbL8451VisitorDlg@@QAEX_N@Z=?SetListTradeItem@COfflineShopVisitorDlg@@QAEX_N@Z")
struct RbOfflineShopDlg169 : public FrWnd
{
	void SetListTradeItem(bool flag);
};
#pragma comment(linker, \
	"/alternatename:?SetListTradeItem@RbOfflineShopDlg169@@QAEX_N@Z=?SetListTradeItem@COfflineShopDlg@@QAEX_N@Z")
struct RbOfflineTradeFinishEditDlg169 : public FrWnd
{
	void SetListTradeItem(std::list<sTradeItem>* pList);
};
#pragma comment(linker, \
	"/alternatename:?SetListTradeItem@RbOfflineTradeFinishEditDlg169@@QAEXPAV?$list@UsTradeItem@@V?$allocator@UsTradeItem@@@std@@@std@@@Z=?SetListTradeItem@FrOfflineTradeFinishEditDlg@@QAEXPAV?$list@UsTradeItem@@V?$allocator@UsTradeItem@@@std@@@std@@@Z")
FrForm* __fastcall RbCreateOfflineShopDlg169(FrWndManager* pManager,
	FrCmdTarget* pCmdDest, const char* name, FrWnd* pParent);
#pragma comment(linker, \
	"/alternatename:?RbCreateOfflineShopDlg169@@YIPAVFrForm@@PAVFrWndManager@@PAVFrCmdTarget@@PBDPAVFrWnd@@@Z=??$CreateForm@VCOfflineShopDlg@@@@YIPAVCOfflineShopDlg@@PAVFrWndManager@@PAVFrCmdTarget@@PBDPAVFrWnd@@@Z")
#include "uccmanager.h"
#include "ucclibrary.h"
sItemInfo* __fastcall GetMyItemInfo(unsigned long guid);
struct RbSharedDocTradeIncome
{
	void AddTradeIncome(__int64 income);
};
#pragma comment(linker, \
	"/alternatename:?AddTradeIncome@RbSharedDocTradeIncome@@QAEX_J@Z=?AddTradeIncome@CSharedDoc@@QAEX_J@Z")
struct RbTradeDlgSending
{
	void EnableSendingServer(bool bEnable);
};
#pragma comment(linker, \
	"/alternatename:?EnableSendingServer@RbTradeDlgSending@@QAEX_N@Z=?EnableSendingServer@FrTradeDlg@@QAEX_N@Z")
struct RbTradeDlgEB : public FrWnd
{
	void ShowStaticOutOfStock(bool show);
	void EnableHideBn(bool enable);
};
#pragma comment(linker, \
	"/alternatename:?ShowStaticOutOfStock@RbTradeDlgEB@@QAEX_N@Z=?ShowStaticOutOfStock@FrTradeDlg@@QAEX_N@Z")
#pragma comment(linker, \
	"/alternatename:?EnableHideBn@RbTradeDlgEB@@QAEX_N@Z=?EnableHideBn@FrTradeDlg@@QAEX_N@Z")
struct sPrize
{
	int type;
	int rankFrom;
	int rankTo;
	unsigned long tid;
	int count;
};
int __fastcall IsAdministrator(unsigned long identity);
#include "frimagenoticedlg.h"
#include "uccmanager.h"
#include "ucclibrary.h"
#include "ucccopydlg.h"
// entrada de 0x7b bytes do pacote 0x15b; struct sem nome conhecido, campos nomeados pelo uso (offsets do asm)
#pragma pack(push, 1)
struct RbRoomReport15b
{
	char pad00[0x20];
	unsigned char flags; // +0x20
	char pad21[2];
	unsigned char nHole; // +0x23
	char pad24;
	unsigned short roomNo; // +0x25
	unsigned char course;  // +0x27
	char pad28[0x4e];
	unsigned long serverId; // +0x76
	unsigned char mode;     // +0x7a
};
#pragma pack(pop)
#include "cardremove.h"
#include "eventthread.h"
#include "tradedlg.h"
#include "packets/messengerpackets.h"	// PKT (frente C)
#include "packets/attendancepackets.h"	// PKT (frente C)
#include "packets/guildpackets.h"	// PKT (frente C)
#include "jp/webpageconnector.h" // C33: caso 0x1ad
#include "jp/shareddocjp.h" // C34: casos 0x251, 0x254 (GetState983) e 0x26a (IsTrajectoryPreviewOn)
#include "jp/clubsetlevelupwnd.h" // C34: casos 0x23d-0x23f (oficina de tacos, frente B)
#include "jp/clubsetrankupwnd.h" // C34: casos 0x240-0x243
#include "jp/clubsetoptionalfunctionswnd.h" // C34: casos 0x245, 0x246
#include "jp/grandprixdocument.h" // D985: caso 0x4b
#include "jp/gproomtimersteward.h"
#include "jp/holeinonegamedoc.h" // C36: caso 0x1f9 (conteiner 0xaa)
#include "jp/itemupdater.h" // C59: caso 0x216 (CItemUpdater, frente B)
#include "../shared/uselzo.h" // C60: caso 0x1b1 (lista de mercadorias comprimida)
#include "jp/gamestringmanager.h" // C36: caso 0x26d (GetGameString)
#include "jp/grandprixartifactrule.h" // 983 009c2920
#include "jp/gproomtimersteward.h" // A229: CGrandPrixClock::Sync983 nos casos 0x44 e 0xba
#include "transferableitem.h" // C37: casos 0x211, 0x212, 0x215 (correio)
#include "packets/postpackets.h" // C37: sMailInfoBrief983/sMailIncludeItem983 (0xc4/0x37)
#include "jp/clubsetresetwnd.h" // C37: caso 0x247 (oficina de tacos, frente B)
#include "jp/taskwidget.h" // C46: repasse a widget de +0x1fc no inicio do OnPacketCommon
#include "jp/guildmanagedlg.h" // C64: caso 0x116 chama FrGuildManageDlg::RefreshList983 (008fafc0, frente B)
bool __fastcall IsClubSetCareWndOn983(); // 004cce50 (src/projectg/jp/clubsetworkshopmaindlg.cpp, sem cabecalho)
namespace S5
{
	namespace UTIL
	{
		int __fastcall ErasePartsCardAll(unsigned long partsId,
			std::list<unsigned long>& cardList);
	}
}

// CSharedDoc::SetUserInfoMod / RequestUccInfo live in other modules; declared
// here instead of in the PCH headers (name-count canaries).
struct RbSharedDocView
{
	void SetUserInfoMod(int mod);
};
#pragma comment(linker, \
	"/alternatename:?SetUserInfoMod@RbSharedDocView@@QAEXH@Z=?SetUserInfoMod@CSharedDoc@@QAEXH@Z")
void __fastcall RequestUccInfo(sCharacterInfo& info);
void __fastcall RequestUccInfo(sUserInfo& info);
namespace ranktype
{
	struct sRankUserInfo;
	struct sListInfo
	{
		unsigned long rank;
		unsigned long guid;
		unsigned long score;
		unsigned long extra;
		unsigned short level;
		unsigned char state;
		std::string nick;
		std::string guild;
	};
}
void __fastcall RequestUccInfo(ranktype::sRankUserInfo& info);
void __fastcall RequestUccInfo(sMailIncludeItem& item); // 007e9900 (C37: casos 0x211, 0x212, 0x215)
struct RbRankingDlg
{
	void SetBottomMsg(const char* msg);
	void UpdateUserInfo();
	void UpdateRankList();
	void ClearRankList();
	void UpdateMyInfo();
	void SelectRankList(int page);
	static void __fastcall SetRankTypes(unsigned char a, unsigned char b,
		unsigned char c, unsigned char d, int e, int f);
};
#pragma comment(linker, \
	"/alternatename:?UpdateRankList@RbRankingDlg@@QAEXXZ=?UpdateRankList@FrRankingDlg@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?ClearRankList@RbRankingDlg@@QAEXXZ=?ClearRankList@FrRankingDlg@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?UpdateMyInfo@RbRankingDlg@@QAEXXZ=?UpdateMyInfo@FrRankingDlg@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?SelectRankList@RbRankingDlg@@QAEXH@Z=?SelectRankList@FrRankingDlg@@QAEXH@Z")
#pragma comment(linker, \
	"/alternatename:?SetRankTypes@RbRankingDlg@@SIXEEEEHH@Z=?SetRankTypes@FrRankingDlg@@SIXEEEEHH@Z")
#pragma comment(linker, \
	"/alternatename:?SetBottomMsg@RbRankingDlg@@QAEXPBD@Z=?SetBottomMsg@FrRankingDlg@@QAEXPBD@Z")
#pragma comment(linker, \
	"/alternatename:?UpdateUserInfo@RbRankingDlg@@QAEXXZ=?UpdateUserInfo@FrRankingDlg@@QAEXXZ")
// D867: lista/minha linha/usuario do FrRankingDlg: offsets do VC10 (C_ASSERT_VC10 em rankingdlg.cpp); no cl novo
// std::list/std::string mudam de tamanho e a classe nao e visivel aqui -> acessores de rankingdlg.cpp.
#if _MSC_VER > 1600
void* __fastcall RankList983(FrRankingDlg* p);
void* __fastcall RankMyInfo983(FrRankingDlg* p);
void* __fastcall RankUserInfo983(FrRankingDlg* p);
#define RKLIST983(p) ((std::list<ranktype::sListInfo>*)RankList983((FrRankingDlg*)(p)))
#define RKMY983(p) ((ranktype::sListInfo*)RankMyInfo983((FrRankingDlg*)(p)))
#define RKUSER983(p) ((char*)RankUserInfo983((FrRankingDlg*)(p)))
#else
#define RKLIST983(p) ((std::list<ranktype::sListInfo>*)((char*)(p) + 0x114))
#define RKMY983(p) ((ranktype::sListInfo*)((char*)(p) + 0x120))
#define RKUSER983(p) ((char*)(p) + 0x16c)
#endif
// D867: FrMessengerDlg::m_pGuildManageDlg983 (+0x18c no VC10) e COfflineShopEditDlg::m_closing (+0x33c no VC10) sao
// protegidos; no cl novo FrForm/STL mudam o layout -> visoes com using.
#if _MSC_VER > 1600
struct FrMessengerDlgView983 : public FrMessengerDlg
{
	using FrMessengerDlg::m_pGuildManageDlg983;
};
struct COfflineShopEditDlgView983 : public COfflineShopEditDlg
{
	using COfflineShopEditDlg::m_closing;
};
#define MSNGUILDMANAGE983(p) (((FrMessengerDlgView983*)(p))->m_pGuildManageDlg983)
#define OFFSHOPCLOSING983(p) (((COfflineShopEditDlgView983*)(p))->m_closing)
#else
#define MSNGUILDMANAGE983(p) (*(FrGuildManageDlg**)((char*)(p) + 0x18c))
#define OFFSHOPCLOSING983(p) (*(int*)((char*)(p) + 0x33c))
#endif
int __fastcall GalleryMode();
struct sTradeItem;
int __fastcall IsPcBang();
unsigned char __fastcall MyIndex();
unsigned long __fastcall RbMyGuid(bool bGallery);
#pragma comment(linker, "/alternatename:?RbMyGuid@@YIK_N@Z=?MyGuid@@YIK_N@Z")
extern CCommodityStatusManager g_CommodityStatusManager;
// 983 0046f4c0 (C65): fora de linha no exe; lê o tipo de jogo da sala (CSharedDoc+0x10594). Antes só declarada: ninguém
// definia ?GetGameType@@YIEXZ (out/V6/link/falta.tsv) e a inline do shareddoc.inl é do 645 (+0x4668, só no clientsetting).
inline unsigned char __fastcall GetGameType() { return CSharedDoc::Instance()->m_golfGame.gameType; }
void __fastcall SetTradeResultCode(int code, int packetId, sTradeItem* item);
struct RbSharedDocTrade : public CSharedDoc
{
	void SetStateTrade(eStateTrade state);
};
#pragma comment(linker, \
	"/alternatename:?SetStateTrade@RbSharedDocTrade@@QAEXW4eStateTrade@@@Z=?SetStateTrade@CSharedDoc@@QAEXW4eStateTrade@@@Z")
struct RbTradeDlg : public FrWnd
{
	void UpdateCountVisitor();
	void UpdateTradeTitle();
	void SetPackageSale(eTypeSalesTrade type);
	void OpenTradeShop(unsigned long uid);
	void SetListTradeItem(bool flag);
};
#pragma comment(linker, \
	"/alternatename:?SetPackageSale@RbTradeDlg@@QAEXW4eTypeSalesTrade@@@Z=?SetPackageSale@FrTradeDlg@@QAEXW4eTypeSalesTrade@@@Z")
#pragma comment(linker, \
	"/alternatename:?OpenTradeShop@RbTradeDlg@@QAEXK@Z=?OpenTradeShop@FrTradeDlg@@QAEXK@Z")
#pragma comment(linker, \
	"/alternatename:?SetListTradeItem@RbTradeDlg@@QAEX_N@Z=?SetListTradeItem@FrTradeDlg@@QAEX_N@Z")
FrForm* __fastcall RbCreateTradeDlg(FrWndManager* pManager,
	FrCmdTarget* pCmdDest, const char* name, FrWnd* pParent);
#pragma comment(linker, \
	"/alternatename:?RbCreateTradeDlg@@YIPAVFrForm@@PAVFrWndManager@@PAVFrCmdTarget@@PBDPAVFrWnd@@@Z=??$CreateForm@VFrTradeDlg@@@@YIPAVFrTradeDlg@@PAVFrWndManager@@PAVFrCmdTarget@@PBDPAVFrWnd@@@Z")
#pragma comment(linker, \
	"/alternatename:?UpdateCountVisitor@RbTradeDlg@@QAEXXZ=?UpdateCountVisitor@FrTradeDlg@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?UpdateTradeTitle@RbTradeDlg@@QAEXXZ=?UpdateTradeTitle@FrTradeDlg@@QAEXXZ")
struct RbTradeEditDlg : public FrWnd
{
	void UpdateTradeTitle();
	void UpdateTradeTitleEdit();
	// JP 983 (C-PKT lote 10): 008ee2a0, 008ee290 e o OnOpenBnUp (008f8730, pelo thunk 008f94a0) da FrTradeEditDlg.
	bool IsOpenAfterTitle983();
	void SetOpenAfterTitle983(bool b);
	void OnOpenBnUp983();
};
#pragma comment(linker, \
	"/alternatename:?IsOpenAfterTitle983@RbTradeEditDlg@@QAE_NXZ=?IsOpenAfterTitle983@FrTradeEditDlg@@QAE_NXZ")
#pragma comment(linker, \
	"/alternatename:?SetOpenAfterTitle983@RbTradeEditDlg@@QAEX_N@Z=?SetOpenAfterTitle983@FrTradeEditDlg@@QAEX_N@Z")
#pragma comment(linker, \
	"/alternatename:?OnOpenBnUp983@RbTradeEditDlg@@QAEXXZ=?OnOpenBnUp@FrTradeEditDlg@@IAEXXZ")
#pragma comment(linker, \
	"/alternatename:?UpdateTradeTitle@RbTradeEditDlg@@QAEXXZ=?UpdateTradeTitle@FrTradeEditDlg@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?UpdateTradeTitleEdit@RbTradeEditDlg@@QAEXXZ=?UpdateTradeTitleEdit@FrTradeEditDlg@@QAEXXZ")
FrForm* __fastcall RbCreateTradeEditDlg(FrWndManager* pManager,
	FrCmdTarget* pCmdDest, const char* name, FrWnd* pParent);
#pragma comment(linker, \
	"/alternatename:?RbCreateTradeEditDlg@@YIPAVFrForm@@PAVFrWndManager@@PAVFrCmdTarget@@PBDPAVFrWnd@@@Z=??$CreateForm@VFrTradeEditDlg@@@@YIPAVFrTradeEditDlg@@PAVFrWndManager@@PAVFrCmdTarget@@PBDPAVFrWnd@@@Z")
namespace _gateway
{
	struct sTradeSellParam
	{
		sTradeSellParam() { }
		~sTradeSellParam() { }
		std::vector<sTradeItem> items;
		int type;
		int reserved;
		__int64 price;
	};
	struct sTradeBuyBase
	{
		sTradeBuyBase() { }
		sTradeItem item;
		sItemInfo info;
		unsigned char flag;
	};
	struct sTradeBuyParam
	{
		sTradeBuyParam() { }
		~sTradeBuyParam() { }
		std::vector<sTradeBuyBase> items;
		int type;
		int reserved;
		__int64 price;
	};
	struct sTradeMinusParam
	{
		sTradeMinusParam() { }
		~sTradeMinusParam() { }
		std::vector<sTradeItem> items;
		std::string title;
		int count;
		int serial;
	};
}
struct sBonusBall
{
	unsigned long data[5];
};
struct RbScratchDlg : public FrWnd
{
	void EnableScratchSerialNumber(bool enable);
	void ResponseScratchSerialNumber(int result, unsigned long number);
	void InitChangeScratchCardNumber(int mode);
	void EnableNextPageButton(bool enable);
	void SetScratchItem(int count, sBonusBall* balls);
};
#pragma comment(linker, \
	"/alternatename:?EnableNextPageButton@RbScratchDlg@@QAEX_N@Z=?EnableNextPageButton@FrScratchDlg@@QAEX_N@Z")
#pragma comment(linker, \
	"/alternatename:?SetScratchItem@RbScratchDlg@@QAEXHPAUsBonusBall@@@Z=?SetScratchItem@FrScratchDlg@@QAEXHPAUsBonusBall@@@Z")
#pragma comment(linker, \
	"/alternatename:?EnableScratchSerialNumber@RbScratchDlg@@QAEX_N@Z=?EnableScratchSerialNumber@FrScratchDlg@@QAEX_N@Z")
#pragma comment(linker, \
	"/alternatename:?ResponseScratchSerialNumber@RbScratchDlg@@QAEXHK@Z=?ResponseScratchSerialNumber@FrScratchDlg@@QAEXHK@Z")
#pragma comment(linker, \
	"/alternatename:?InitChangeScratchCardNumber@RbScratchDlg@@QAEXH@Z=?InitChangeScratchCardNumber@FrScratchDlg@@QAEXH@Z")
struct RbOfflineTradeDlg : public FrWnd
{
	void SetListItem();
	void SetPageData(int a, int b);
};
#pragma comment(linker, \
	"/alternatename:?SetListItem@RbOfflineTradeDlg@@QAEXXZ=?SetListItem@COfflineTradeDlg@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?SetPageData@RbOfflineTradeDlg@@QAEXHH@Z=?SetPageData@COfflineTradeDlg@@QAEXHH@Z")
struct sRecycleItem
{
	unsigned long dwTid;
	unsigned long dwItemid;
	unsigned short iNum;
	unsigned char byLevel;
	unsigned char byFlag983; // 983: +0x0b, byte baixo de sCadieMagicBox+0x14 (00a22d80)
	short iChar;
	short iIndex;
	unsigned char byItemType;
	unsigned short wTime;
	_SYSTEMTIME date;
	unsigned long dwRandSeq;
	char szRandName[40];
	_SYSTEMTIME dateStart983; // 983: +0x50 e +0x60 (sizeof 0x70), periodo da receita no CadieMagicBox.iff
	_SYSTEMTIME dateEnd983;

	sRecycleItem() // 983: o new[] do leitor 00a22d80 zera cada elemento
	{
		memset(this, 0, sizeof(*this));
	}
};
C_ASSERT(sizeof(sRecycleItem) == 0x70);
C_ASSERT(offsetof(sRecycleItem, szRandName) == 0x28);
struct RbRecycleDlg : public FrWnd
{
	void SetResultRecycleItem(bool ok, std::list<sRecycleItem>& items,
		short count);
	void ResetMix983(); // 983 008db940: m_mixResultTime = 3.0f, m_mixing = false, EnableTotalControls(true) (pedido C54.1)
};
#pragma comment(linker, "/alternatename:?ResetMix983@RbRecycleDlg@@QAEXXZ=?ResetMix983@FrRecycleDlg@@QAEXXZ")
// 983 009b7860 (fora de linha, packet em esi e item em edi): item do pacote 0x22f = 5 dwords
// PKT (frente C, lote 52): registro do pacote 0x216 (sItemUpdate983, include/jp/itemupdater.h da frente B)
// 983 009be6c0: construtor (zera tudo, texto "", +0x64 = -1)
sItemUpdate983::sItemUpdate983()
{
	kind = 0;
	typeId = 0;
	uid = 0;
	periodType = 0;
	prevTotal983 = 0;
	total = 0;
	count = 0;
	text = "";
	uccStatus983 = 0;
	uccSeq983 = 0;
	cardTypeId = 0;
	cardSlot = 0;
	workshop.exp = 0;
	workshop.srvInt983 = 0;
	workshop.recovery983 = 0;
	workshop.srvByte983 = -1;
	for (int i = 0; i < 5; ++i)
		workshop.slots[i] = 0;
	mastery = 0;
	caddieLevel = 0;
	caddieExp = 0;
	for (int i = 0; i < 5; ++i)
		counters[i] = 0;
}
// 983 009bfc90 (C59.1: metodo declarado pela B): kind e um byte; o bloco da oficina so vem para tacos (categoria 4) com kind
// 0xcc, a mestria para personagens (1) com 0xcd e o nivel do caddie (7) com 0xce
void sItemUpdate983::Decode983(WReceivedPacket& packet)
{
	kind = packet.Decode1();
	typeId = packet.Decode4();
	uid = packet.Decode4();
	periodType = packet.Decode4();
	prevTotal983 = packet.Decode4();
	total = packet.Decode4();
	count = packet.Decode4();
	for (int i = 0; i < 5; ++i)
		counters[i] = packet.Decode2();
	text = packet.DecodeStr();
	uccStatus983 = packet.Decode4();
	uccSeq983 = packet.Decode4();
	cardTypeId = packet.Decode4();
	cardSlot = packet.Decode1();
	if ((typeId & 0xfc000000) == 0x10000000 && kind == 0xcc)
	{
		for (int i = 0; i < 5; ++i)
			workshop.slots[i] = packet.Decode2();
		workshop.exp = packet.Decode4();
		workshop.srvByte983 = packet.Decode1();
		workshop.srvInt983 = packet.Decode4();
		workshop.recovery983 = packet.Decode4();
	}
	if ((typeId & 0xfc000000) == 0x4000000 && kind == 0xcd)
		mastery = packet.Decode4();
	if ((typeId & 0xfc000000) == 0x1c000000 && kind == 0xce)
	{
		caddieLevel = packet.Decode1();
		caddieExp = packet.Decode4();
	}
}
// 983 00633680 = CSharedDoc: return m_itemUpdater(+0x11628)->Update983(now, rec) (sem declaracao no shareddoc.h; pedido C59.2)
static int SharedDocUpdateItem983(CSharedDoc* pDoc, __time64_t now, const sItemUpdate983& rec)
{
	return ((CItemUpdater*)pDoc->m_itemUpdater983[0])->Update983(now, rec);
}
// PKT (frente C, lote 53): sd_shop::CCommodityList do 983 (classe nova; declaracao em shareddoc.h, frente A). Corpos aqui como funcoes
// livres sobre os membros publicos ate a classe ganhar os metodos (pedido C60.1).
// 983 00b3c640 (__thiscall): esvazia a lista sob a secao critica
static void CommodityListClear983(sd_shop::CCommodityList& list)
{
	EnterCriticalSection(&list.m_cs);
	list.m_listVersion = 0;
	list.m_map.clear(); // 00b3c1f0
	LeaveCriticalSection(&list.m_cs);
}
#pragma pack(push, 1)
struct sCommodityRecord983 // registro de 0x11 bytes do bloco descomprimido
{
	unsigned long typeId;	 // +0x00: peca (Part.iff)
	unsigned long commodityId; // +0x04: chave do mapa e id do sUccClothes
	char uccIndex[9];		 // +0x08
};
#pragma pack(pop)
C_ASSERT(sizeof(sCommodityRecord983) == 0x11);
C_ASSERT(sizeof(sd_shop::sCommodity) == 0x248);
C_ASSERT(sizeof(IFF_STRUCT::sPart) == 0x238);
// 983 00b3cd60 (__thiscall): descomprime, e para cada registro com peca conhecida grava (commodityId -> peca + id + indice) no mapa;
// registro novo no mapa vira roupa UCC (CUccManager::AddClothes 007ee460 com slot 0, envia = 1, forca = 0)
static bool CommodityListLoad983(sd_shop::CCommodityList& list, unsigned char* buf, int size)
{
	if (buf == NULL || size < 1)
		return false;
	CCompressBuffer cb;
	cb.Decompress(buf, size);
	int count = (int)((unsigned int)cb.GetNewSize() / sizeof(sCommodityRecord983));
	if (count < 1)
		return false;
	sCommodityRecord983* recs = new sCommodityRecord983[count];
	if (recs)
		memset(recs, 0, count * sizeof(sCommodityRecord983));
	memcpy(recs, cb.GetNewBuffer(), cb.GetNewSize());
	EnterCriticalSection(&list.m_cs);
	for (int i = 0; i < count; ++i)
	{
		const sCommodityRecord983& r = recs[i];
		IFF_STRUCT::sPart* pPart = CSharedDoc::Instance()->m_itemManager.FindPart(r.typeId);
		if (!pPart)
			continue;
		std::pair<unsigned long, sd_shop::sCommodity> kv;
		sd_shop::sCommodity* pEntry = &kv.second;
		memcpy(&pEntry->part, pPart, sizeof(pEntry->part));
		pEntry->commodityId = r.commodityId;
		strncpy(pEntry->uccIndex, r.uccIndex, sizeof(pEntry->uccIndex) - 1); // 983: strcpy do registro
		pEntry->uccIndex[sizeof(pEntry->uccIndex) - 1] = 0;
		kv.first = r.commodityId;
		if (list.m_map.insert(kv).second)
		{
			sUccClothes clothes;
			clothes.id = r.commodityId;
			clothes.typeId = r.typeId;
			strncpy(clothes.uccIndex, r.uccIndex, 9);
			UccManager()->AddClothes(0, clothes, true, false);
		}
	}
	LeaveCriticalSection(&list.m_cs);
	delete[] recs;
	return true;
}

static void DecodeRecycleItem983(WReceivedPacket& packet, sRecycleItem& item)
{
	unsigned long tid = packet.Decode4();
	unsigned long itemid = packet.Decode4();
	unsigned long num1 = packet.Decode4();
	unsigned long num2 = packet.Decode4();
	unsigned long type = packet.Decode4();
	item.dwTid = tid;
	item.dwItemid = itemid;
	item.byItemType = (unsigned char)type;
	if (tid == 0x1a000010)
		item.iNum = (unsigned short)num1;
	else
		item.iNum = (unsigned short)num2;
}
#pragma comment(linker, \
	"/alternatename:?SetResultRecycleItem@RbRecycleDlg@@QAEX_NAAV?$list@UsRecycleItem@@V?$allocator@UsRecycleItem@@@std@@@std@@F@Z=?SetResultRecycleItem@FrRecycleDlg@@QAEX_NAAV?$list@UsRecycleItem@@V?$allocator@UsRecycleItem@@@std@@@std@@F@Z")
// PKT C76 (lote 69): janelas RF de maestria do 983 (casos 0x26e-0x273 do OnPacketCommon). Sem corpo no nosso fonte ate a
// frente B escrever o item (6) da F3.7.12 (pedido C76.2); os nomes daqui sao provisorios (map/pkt_c.tsv).
struct RbCharMasteryStatusDlg983 // RFCharMasteryStatusDlg (vtable 00d0fe44, ms_pInstance 00e781c8)
{
	static RbCharMasteryStatusDlg983* ms_pInstance;					// 00e781c8
	static bool IsCreated983();										// 005bd3d0
	void Refresh983();												// 005cc980
	void ShowResult983(unsigned long value, bool bOk);				// 005c9b00
};
struct RbCharMasteryTileDlg983 // RFCharMasteryTileDlg (vtable 00d0f67c, ms_pInstance 00e78190)
{
	static RbCharMasteryTileDlg983* ms_pInstance;					// 00e78190
	static bool IsCreated983();										// 005bd3e0
	void Refresh983();												// 005c59e0
};
struct RbCharMasteryCardDlg983 // RFCharMasteryCardDlg (vtable 00d0e7f4, ms_pInstance 00e7813c)
{
	static RbCharMasteryCardDlg983* ms_pInstance;					// 00e7813c
	static bool IsCreated983();										// 005b87d0
	void ShowResult983(unsigned long value, bool bOk);				// 005bb820
	void SetCharacter983(unsigned long charId);						// 005bbdb0
};
// C98 (C76.2 atendido pela B): as visoes acima ligam nas classes da B (include/jp/refresh/rfcharmastery*.h; a lib e as visoes
// velhas nao convivem nesta unidade). O 005c59e0 do 0x26e e o RFCharMasteryTileDlg::PlaySlotAni983 da B.
#pragma comment(linker, "/alternatename:?ms_pInstance@RbCharMasteryStatusDlg983@@2PAU1@A=?ms_pInstance@RFCharMasteryStatusDlg@@2PAV1@A")
#pragma comment(linker, "/alternatename:?IsCreated983@RbCharMasteryStatusDlg983@@SI_NXZ=?IsCreated983@RFCharMasteryStatusDlg@@SI_NXZ")
#pragma comment(linker, "/alternatename:?Refresh983@RbCharMasteryStatusDlg983@@QAEXXZ=?Refresh983@RFCharMasteryStatusDlg@@QAEXXZ")
#pragma comment(linker, "/alternatename:?ShowResult983@RbCharMasteryStatusDlg983@@QAEXK_N@Z=?OnResult983@RFCharMasteryStatusDlg@@QAEXK_N@Z")
#pragma comment(linker, "/alternatename:?ms_pInstance@RbCharMasteryTileDlg983@@2PAU1@A=?ms_pInstance@RFCharMasteryTileDlg@@2PAV1@A")
#pragma comment(linker, "/alternatename:?IsCreated983@RbCharMasteryTileDlg983@@SI_NXZ=?IsCreated983@RFCharMasteryTileDlg@@SI_NXZ")
#pragma comment(linker, "/alternatename:?Refresh983@RbCharMasteryTileDlg983@@QAEXXZ=?PlaySlotAni983@RFCharMasteryTileDlg@@QAEXXZ")
#pragma comment(linker, "/alternatename:?ms_pInstance@RbCharMasteryCardDlg983@@2PAU1@A=?ms_pInstance@RFCharMasteryCardDlg@@2PAV1@A")
#pragma comment(linker, "/alternatename:?IsCreated983@RbCharMasteryCardDlg983@@SI_NXZ=?IsCreated983@RFCharMasteryCardDlg@@SI_NXZ")
#pragma comment(linker, "/alternatename:?ShowResult983@RbCharMasteryCardDlg983@@QAEXK_N@Z=?OnResult983@RFCharMasteryCardDlg@@QAEXK_N@Z")
#pragma comment(linker, "/alternatename:?SetCharacter983@RbCharMasteryCardDlg983@@QAEXK@Z=?SetChar983@RFCharMasteryCardDlg@@QAEXK@Z")
struct RbBongdariShopDlg : public FrWnd
{
	void EnableExitButton(bool enable);
	void ShowDiscountUsableTimes();
	void UpdateUsableTimes();
	void InitBongdariShop();
	void SetBonusBall(int count, sBonusBall* pBalls);
	void SetBongdariShop();
	void SetDataFee(int fee);
	// PKT C74 (lote 67): so do 983; corpos na jp/pktviews983.cpp (FrBongdariShopDlgPkt983) ate a frente A ter o layout (C74.1)
	void StopAutoPlay983();												// 008b4740
	void EnableAfterResult983();										// 008b4830
	void SetResultList983(int count, std::vector<sBonusBall>& balls); // 008bee10
	void SetStateReady983();											// 008b35e0
};
#pragma comment(linker, \
	"/alternatename:?StopAutoPlay983@RbBongdariShopDlg@@QAEXXZ=?StopAutoPlay983@FrBongdariShopDlgPkt983@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?EnableAfterResult983@RbBongdariShopDlg@@QAEXXZ=?EnableAfterResult983@FrBongdariShopDlgPkt983@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?SetResultList983@RbBongdariShopDlg@@QAEXHAAV?$vector@UsBonusBall@@V?$allocator@UsBonusBall@@@std@@@std@@@Z=?SetResultList983@FrBongdariShopDlgPkt983@@QAEXHAAV?$vector@UsBonusBall@@V?$allocator@UsBonusBall@@@std@@@std@@@Z")
#pragma comment(linker, \
	"/alternatename:?SetStateReady983@RbBongdariShopDlg@@QAEXXZ=?SetStateReady983@FrBongdariShopDlgPkt983@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?EnableExitButton@RbBongdariShopDlg@@QAEX_N@Z=?EnableExitButton@FrBongdariShopDlg@@QAEX_N@Z")
#pragma comment(linker, \
	"/alternatename:?ShowDiscountUsableTimes@RbBongdariShopDlg@@QAEXXZ=?ShowDiscountUsableTimes@FrBongdariShopDlg@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?UpdateUsableTimes@RbBongdariShopDlg@@QAEXXZ=?UpdateUsableTimes@FrBongdariShopDlg@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?InitBongdariShop@RbBongdariShopDlg@@QAEXXZ=?InitBongdariShop@FrBongdariShopDlg@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?SetBonusBall@RbBongdariShopDlg@@QAEXHPAUsBonusBall@@@Z=?SetBonusBall@FrBongdariShopDlg@@QAEXHPAUsBonusBall@@@Z")
#pragma comment(linker, \
	"/alternatename:?SetBongdariShop@RbBongdariShopDlg@@QAEXXZ=?SetBongdariShop@FrBongdariShopDlg@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?SetDataFee@RbBongdariShopDlg@@QAEXH@Z=?SetDataFee@FrBongdariShopDlg@@QAEXH@Z")

// PKT (frente C, lote 12; C65/C68): objeto de 00e78ca8 = RFLobbyDesktop da frente B (include/jp/refresh/rflobbydesktop.h,
// guardado em CLobbyState983::ms_pInstance); 005fbe40 = RFLobbyDesktop::Refresh983 (corpo da B, B132).
// C72: visao local trocada pelo cabecalho da B (C68.1 atendido na B135).
#include "jp/refresh/rflobbydesktop.h"
int CTask::OnPacketCommon(WReceivedPacket& packet)
{
	// PKT (frente C, lote 39): 983 009c20d7 - a widget aberta por cima da tarefa (+0x1fc; o my room e a CMyRoomWidget,
	// 005a8360) ve o pacote antes de tudo; se tratou, acabou. No 983 +0x1fc nunca e nulo (boneco CTaskWidget 00f006f8, gravado
	// no construtor 009bf4d4/009bf52a); aqui o boneco ainda e um IActor e o construtor nao grava +0x1fc (pedido C46.1 a A):
	// sem guarda desde o C46.1 (o boneco CNullWidget devolve 0).
	if (m_pWidget983->OnPacket(packet))
		return 1;
	packet.Reset();
	int type = (unsigned short)packet.Decode2();
	g_iRecvPacketType = type;

	switch (type)
	{
	case PKT645_SEM_983(0x1d3):
	{
		unsigned long packetType = packet.Decode4();
		unsigned long value = packet.Decode4();
		WSendPacket send(packetType);
		send.Encode4(Doc()->m_myInfo.info.dwGuildId);
		send.Encode4(value);
		send.Send(TO_MSN);
	}
	break;
	case 0x182:
	{
		IActor* pActor = AfxGetTask()->GetActor("PrivateTrade");
		if (!pActor)
			break;
		DYNAMIC_CAST(CPrivateTrade, pActor)->OnPacket(&packet);
	}
	break;
	case 0x197:
	{
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);

		S5::FrSupplyPackDlg* pDlg = NULL;
		FrWnd* pWnd = g_pFresh->GetDesktop()->FindChildForm("FrSupplyPackDlg");
		if (pWnd && IS_EXACTKINDOF(S5::FrSupplyPackDlg, pWnd))
			pDlg = DYNAMIC_CAST(S5::FrSupplyPackDlg, pWnd);

		if (packet.Decode1() == 0)
		{
			if (!pDlg)
				break;

			pDlg->Close(FrCANCEL, true);
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), NULL,
				"notify", NULL);
			pForm->SetMessage(
				K2L_Compatibility("\277\300\267\371\260\241 \271\337\273\375\307\317\277\264\275\300\264\317\264\331. "
				"\300\341\275\303 \310\304 \264\331\275\303 \275\303\265\265\307\330 \301\326\274\274\277\344."),
				false);
			pForm->Open(NULL, 3);
			break;
		}

		unsigned long packTid = packet.Decode4();
		unsigned long ballTid = packet.Decode4();
		unsigned long ballCount = packet.Decode4();

		std::list<sItemInfo>& itemList = Doc()->m_myItemList;
		std::list<sItemInfo>::iterator it =
			std::find(itemList.begin(), itemList.end(), packTid);
		if (it != itemList.end())
		{
			it->Common[0]--;
			if (it->Common[0] <= 0)
				Doc()->m_myItemList.erase(it);

			AfxGetTask()->GetMainActor()
				<< MsgObject(NULL, M983(0xdd), 0, 0, 0, 0, 0);
		}

		if ((ballTid & 0xfc000000) != 0x14000000)
			break;

		std::list<sItemInfo>& ballList = Doc()->m_ballList;
		it = std::find(ballList.begin(), ballList.end(), ballTid);
		if (it == ballList.end())
			break;

		int added = ballCount - it->Common[0];
		it->Common[0] = (short)ballCount;
		if (!pDlg)
			break;
		pDlg->UpdateData(added);
	}
	break;
	case 0x11f:
	{
		// S65: corpo do 983 (009c8bd6). O 645 testava IS_EXACTKINDOF(CTutorialTask) e postava 0x151 a "TutorialMain".
		unsigned char kind = packet.Decode1();
		unsigned char value = packet.Decode1();

		if (kind == 3)
			packet.DecodeBuffer(Doc()->m_tutorialComplete, 12);

		IContentsDataContainer* pContainer;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0xa4, pContainer); // 006ab7d0; resultado sem uso no 983

		if (AfxGetTask()->IsCurrentActor983("tutorialmain"))
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x1bc, kind, value, 0, 0, 0));
	}
	break;
	case 0x114:
	{
		static const struct
		{
			unsigned long id;
			const char* msg;
		} s_levelMsg[3] = {
			{ 0,
             "\267\271\272\247\300\314 \303\346\272\320\307\317\301\366 \276\312\275\300\264\317\264\331. "
				"\267\271\272\247\276\367 \310\304\277\241 \302\370\277\353\307\317\275\312\275\303\277\300." },
			{ 1,
             "\267\271\272\247\300\314 \303\346\272\320\307\317\301\366 \276\312\275\300\264\317\264\331. "
				"\267\271\272\247\276\367 \310\304\277\241 \300\314\277\353\307\317\275\312\275\303\277\300." },
			{ 2,
             "\275\303\273\363\275\304\300\314 \276\306\301\367 \301\276\267\341\265\307\301\366 \276\312\276\306\274\255, "
				"\304\332\275\272\270\246 \272\257\260\346 \307\322 \274\366 \276\370\275\300\264\317\264\331. "
				"\300\341\275\303 \310\304 \272\257\260\346\307\330 \301\326\275\312\275\303\277\300."        }
		};
		unsigned long index = packet.Decode4();
		if (index >= 3 || index != s_levelMsg[index].id)
			break;

		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetTimeLimit(3.0f);
		pForm->SetMessage(s_levelMsg[index].msg, false);
		pForm->Open(NULL, 3);
	}
	break;
	case 0xf2:
	{
		unsigned char value = packet.Decode1();
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x6b), value, 0, 0, 0, 0));
	}
	break;
	case 0x77:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2b), 1, 0, 0, 0, 0));
		Doc()->m_expRate = packet.Decode4();
		if (GalleryMode())
		{
			Doc()->ClearRivalVars(true);
			break;
		}
		Doc()->ClearRivalVars(false);

		CSharedDoc* pDoc = Doc();
		unsigned long guidCaddie = pDoc->m_myInfo.userEquip.guidCaddie;
		std::map<unsigned int, sCaddieInfo>::const_iterator it =
			pDoc->m_caddieMap.find(guidCaddie);
		if (it != pDoc->m_caddieMap.end())
		{
			const sCaddieInfo& info = it->second;
			IFF_STRUCT::sCaddie* pCaddie = ItemManager()->FindCaddie(info.tid);
			if (info.Rent_flag && info.Remain_Date == 0 && pCaddie &&
				pCaddie->MonthlyFee > 0)
				guidCaddie = 0;
		}

		WSendPacket send((enumClientPacket)0xc);
		unsigned long equip[4];
		equip[0] = Doc()->m_myInfo.userEquip.guidChar;
		Doc()->m_myInfo.userEquip.guidCaddie = guidCaddie;
		equip[1] = guidCaddie;
		equip[2] = Doc()->m_myInfo.userEquip.guidClubSet;
		equip[3] = Doc()->m_myInfo.userEquip.tidBall;
		send.Encode1(7);
		send.EncodeBuffer(equip, 0x10);
		send.Send(TO_GAME);

		if (Doc()->m_roomInfo.realGameType == GAME_TYPE_USEMAX)
		{
			CTaskManager::Instance()->CreateTask("CLobbyTask", "CGolfDoc",
				false);
			Doc()->m_roomInfo.gameType = 4;
		}
	}
	break;
	case 0x76:
	{
		// 983 (009c22d7): sem o ramo de sUserInfoExt (IsLocalContent 0x70) e sem a condição 0x52 nas leituras
		sUserInfo userInfo;

		unsigned char gatewayArg = packet.Decode1(); // 983: 5º argumento da mensagem 0x753b ao "Gateway" (009c2420/009c2753)
		int count = packet.Decode1();

		if (!GalleryMode() && IsMassGame(Doc()->m_roomInfo.gameType))
		{
			Doc()->m_myInfo.info.angelicWings = packet.Decode1();
			Doc()->m_myInfo.info.angelicWingsEffect = packet.Decode1();
			Doc()->m_myInfo.info.devilWings = packet.Decode1();

			{
				_SYSTEMTIME serverTime;
				packet.DecodeBuffer(&serverTime, sizeof(serverTime));
				Doc()->SetServerTime(serverTime);

				CCardManager::Instance()->m_playerSlot[0] = 0;

				std::map<unsigned int, sCharacterInfo>::const_iterator it =
					Doc()->m_charMap.find(Doc()->m_myInfo.userEquip.guidChar);
				if (it != Doc()->m_charMap.end())
				{
					CCardManager::Instance()->m_charInfo[0] = (*it).second;
				}
			}

			Doc()->SetEquipCharCaddieInfoFromMyInfo();
			RastroGear0x76B1589("massa", Doc()->m_userInfo[0]);
			CPowerGauge::Instance()->UpdateClubArea983(); // 983 009c240e
			AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 0, 0, gatewayArg, 0, 0);
		}
		else
		{
			CPartTidList partTidList;
			int cardSlot = 1;

			for (int i = 1; i < 4; i++)
				Doc()->m_cardAbilityList[i].clear();

			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&userInfo, sizeof(sUserInfo)); // 0x31c6 (medido)
				Doc()->m_userInfo[i] = userInfo;

				if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
				{
					RequestUccInfo(userInfo);
				}

				{
					CCardManager::Instance()->m_playerSlot[i] = 0xff;

					_SYSTEMTIME serverTime;
					packet.DecodeBuffer(&serverTime, sizeof(serverTime));
					Doc()->SetServerTime(serverTime);

					bool bMine = false;
					if (stricmp(userInfo.info.sNick,
							Doc()->m_myInfo.info.sNick) == 0)
					{
						bMine = true;
					}

					int cardCount = packet.Decode1();

					sSCardAvilityPeriodInfo cardInfo;
					memset(&cardInfo, 0, sizeof(cardInfo));

					for (int j = 0; j < cardCount; j++)
					{
						packet.DecodeBuffer(&cardInfo, sizeof(cardInfo));
						if (!bMine)
						{
							Doc()->m_cardAbilityList[cardSlot].push_back(
								cardInfo);
						}
					}

					if (bMine)
					{
						RastroGear0x76B1589("servidor", userInfo);
						CCardManager::Instance()->m_playerSlot[i] = 0;
						CCardManager::Instance()->m_charInfo[0] =
							userInfo.charInfo;
						CCardManager::Instance()->SetClubInfo983(0, userInfo.clubInfo); // 983 009c26b7
					}
					else
					{
						CCardManager::Instance()->m_charInfo[cardSlot] =
							userInfo.charInfo;
						CCardManager::Instance()->SetClubInfo983(cardSlot, userInfo.clubInfo); // 983 009c26e2
						CCardManager::Instance()->m_playerSlot[i] = cardSlot;
						cardSlot++;
					}
				}

				partTidList.SetTids(&Doc()->m_userInfo[i].charInfo,
					userInfo.stat.Level);

				IFF_STRUCT::sCaddie* pCaddie = ItemManager()->FindCaddie(
					Doc()->m_userInfo[i].caddieInfo.tid);
				if ((pCaddie &&
						Doc()->m_userInfo[i].stat.Level < pCaddie->c.Level) ||
					(Doc()->m_userInfo[i].caddieInfo.Rent_flag &&
						Doc()->m_userInfo[i].caddieInfo.Remain_Date == 0 &&
						pCaddie->MonthlyFee > 0))
				{
					Doc()->m_userInfo[i].caddieInfo.tid = 0;
				}

				IFF_STRUCT::sClubSet* pClubSet = ItemManager()->FindClubSet(
					Doc()->m_userInfo[i].clubInfo.tid);
				if (pClubSet &&
					((Doc()->m_userInfo[i].stat.Level < pClubSet->c.Level &&
						 !pClubSet->c.IsUnderLvl) ||
						(Doc()->m_userInfo[i].stat.Level > pClubSet->c.Level &&
							pClubSet->c.IsUnderLvl)))
				{
					Doc()->m_userInfo[i].clubInfo.tid = 0x10000000;
				}
			}
			CPowerGauge::Instance()->UpdateClubArea983(); // 983 009c273d
			for (int i = 0; i < count; i++)
				AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 0, i, gatewayArg, 0, 0);
		}

		for (std::list<sSlotInfo>::iterator it = Doc()->m_slotList.begin();
			it != Doc()->m_slotList.end(); ++it)
		{
			if (it->bMaster)
			{
				it->bReady = 1;
			}
			else
			{
				it->bReady = 0;
			}
		}

		if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
		{
			CCardManager::Instance()->CalcCardPeriodAndStatus();
			return 1;
		}
	}
	break;
	case 0x52:
	{
		SendMessageA(g_hwnd, 0xadb3, 0, 0);

		RbBongdariShopDlgClear* pDlg =
			(RbBongdariShopDlgClear*)g_pFresh->GetManager()
				->GetDesktop()
				->FindChildByName("bongdari_shop");
		if (pDlg)
			pDlg->Clear();
		else
			__rtti_obj = pDlg;

		if (CUserInfo::IsInstantiated())
			CUserInfo::Instance()->Clear();

		if (IsLocalContent((localContentType_t)0x59 /*983: era 0x58; interpolado*/))
		{
			CTHunter::Instance()->SetTreasurePoint(0);
		}

		CIconToolTip::GetInstance();
		CIconToolTip::GetInstance()->ClearToolTip();

		SetCurMap(packet.Decode1());
		SetGameType(packet.Decode1());
		SetHoleType(packet.Decode1());
		SetHoles(packet.Decode1());
		Doc()->m_roomType = packet.Decode4();
		SetShotTimeLimit(packet.Decode4());
		SetGameTimeLimit(packet.Decode4());
		if (IsGrandPrixGameType983(Doc()->m_roomInfo.realGameType)) // 983 009c2903: regra cronometrada troca o tempo da tacada e o relogio vem da sala
		{
			_grandprix::_private::CGrandPrixArtifactRule* pRule = _grandprix::_private::CGrandPrixArtifactRule::Instance();
			if (pRule->IsTimed(Doc()->m_roomInfo.artifactTid983))
				Doc()->m_golfGame.shotTimeLimit = pRule->GetSeconds(Doc()->m_roomInfo.artifactTid983) * 1000;
			Doc()->m_golfGame.gpTimeLimit983 = Doc()->m_roomInfo.gpStartValue983;
		}

		if (!IsMassGame())
		{
			// busca sem efeito no original (os dois desfechos caem no mesmo ponto)
			for (int i = 0; i < 4; i++)
			{
				if (Doc()->m_userInfo[i].info.dwGuid == MyGuid(false))
					break;
			}
		}

		for (int i = 0; i < 18; i++)
		{
			// 983 009c29b5: três bytes por buraco, sempre (o 645 calculava m_courseMap e só lia m_pinIndex no tipo 0xe)
			Doc()->m_holeRandom[i] = packet.Decode4();
			Doc()->m_courseMap[i] = packet.Decode1();
			Doc()->m_pinIndex[i] = packet.Decode1();
			Doc()->m_holeOrder[i] = packet.Decode1();
		}

		memset(Doc()->m_holePar, 0, sizeof(Doc()->m_holePar));

		SetCurMap(Doc()->m_pinIndex[0]); // 983 009c2a5a: sem a condição do tipo 0xe

		CGimmickContainer* pGimmick = NULL;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0x9b,
			pGimmick);
		((RbGimmickContainerPacket*)pGimmick)->LoadFromPacket(packet);

		// 983 009c2a96: limpezas novas depois do gimmick
		Doc()->m_appGiftDownloadState = (CSharedDoc::eAppGiftDownloadState)0; // +0x11424
		CTHunter::Instance()->SetReceiveGiftPacket(false); // [00e74e10]+0x5e
		Doc()->m_bTreasureResult = false; // +0x10777
		Doc()->m_bTreasureAward = false; // +0x10778

		// 983 009c2acb: Grand Prix (0046e6d0) espera no Gateway; quem troca de tarefa e a
		// FrGPTypeRoomDlg (msg 0x86 do CLobbyMain, A109)
		if (IsGrandPrixGameType983(Doc()->m_roomInfo.realGameType))
		{
			AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753e, 0xd, 0, 0, 0, 0);
			return 1;
		}

		if (Doc()->m_golfGame.gameType != 0xa && IsMassGame())
		{
			Doc()->m_approachStartTime = g_CurrentTime;
		}

		if (IsMassGame()) // 983 009c2afd
			CPowerGauge::Instance()->ResetRandTable983();

		Doc()->m_roomSetupStage = 0;
		Doc()->LogGstUpdateHole(0xffffffff, 0xff, -1, -1);

		if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/))
		{
			Doc()->SetInitWearHalloweenItem();
		}

		if (CMessengerInfo::Instance()->GetMyStatus() != 3 /*983: +0xb0; o 645 lia +0x8c cru*/)
		{
			CMessengerInfo::Instance()->SetMyStatus((MSN::eStatusType)0);
		}

		for (std::list<sSlotInfo>::iterator it = Doc()->m_slotList.begin();
			it != Doc()->m_slotList.end(); ++it)
		{
			it->bSleep = 0;
		}

		Doc()->m_bGameOver = false;
		Doc()->SetPangyaBySet983(false); // 983 009c2ba1: 0062bfa0 (C42.1)
		GetActor("Lobby") << MsgObject(NULL, 0x314, 0, 0, 0, 0, 0); // 983 009c2bab

		CMouseCursor::Instance()->SetMode(1);
		CMouseCursor::Instance()->SetActive(false);
		COption::Instance()->vSetMinimumScreenSize(0, 0x320, 0x258);
		CTaskManager::Instance()->ChangeTask("CGolfTask", "CGolfDoc", false);
		return 1;
	}
	break;
	case 0x40:
	{
		std::string name;
		std::string msg;
		unsigned char chatType = packet.Decode1();
		std::string text;

		bool bGM = (chatType >> 7) ? true : false;
		chatType &= 0x7f;

		unsigned long color = 0xffffffff;
		bool bName = false;

		packet.DecodeStr(name);
		packet.DecodeStr(msg);

		Doc()->m_chatManager.FilteringHack(msg.c_str(), false);

		bool bIgnore = false;
		switch (chatType)
		{
		case 0:
		{
			if (strcmp(name.c_str(), Doc()->m_myInfo.info.sNick) == 0)
			{
				color = 0xffffcc00;
			}
			else
			{
				std::list<std::string>::iterator it =
					std::find(Doc()->m_ignoreList.begin(),
						Doc()->m_ignoreList.end(), name);
				if (it != Doc()->m_ignoreList.end())
				{
					bIgnore = true;
					break;
				}
			}

			text = name + " : \010" + msg;
			bName = true;

			if (bGM)
			{
				color = 0xff6eecfb;
			}
		}
		break;
		case 1:
			text = K2L_Compatibility("\276\313\270\262 : ") + name +
				K2L_Compatibility(" \264\324\300\272 \307\366\300\347 \303\244\263\316\277\241 \276\370\275\300\264\317\264\331.");
			color = 0xffff7878;
			break;
		case 5:
			text = K2L_Compatibility("\276\313\270\262 : ") + name +
				K2L_Compatibility(" \264\324\300\272 \307\366\300\347 \274\255\271\366\277\241 \276\370\275\300\264\317\264\331.");
			color = 0xffff7878;
			break;
		case 2:
			text = K2L_Compatibility("\276\313\270\262 : ") + name +
				K2L_Compatibility(" \264\324\300\314 \305\360\300\345\307\337\275\300\264\317\264\331.");
			color = 0xffff7878; // 983 009c2f03..009c2f73: so o texto e a cor; o SetGoStopMode(true) do 645 gravava o +0x5d04 cru
			break;
		case 8:
			text = K2L_Compatibility("\276\313\270\262 : ") + name +
				K2L_Compatibility(" \264\324\300\314 \305\360\300\345\307\330\274\255 \275\272\306\256\266\363\300\314\305\251\306\367\300\316\306\256\260\241 1\306\367\300\316\306\256\276\277 \277\303\266\363\260\251\264\317\264\331.");
			color = 0xffff7878;
			break;
		case 3:
			text = K2L_Compatibility(
				"\260\255\305\360\277\241 \260\374\307\321 \305\365\307\245\260\241 \272\316\260\341\265\307\276\372\275\300\264\317\264\331.");
			color = 0xffff7878;
			break;
		case 4:
			text = K2L_Compatibility("\276\313\270\262 : ") + name +
				K2L_Compatibility(" \264\324\300\272 \307\366\300\347 \261\323\274\323\270\273\300\273 \274\366\275\305\307\322 \274\366 \276\370\275\300\264\317\264\331.");
			color = 0xffff7878;
			break;
		case 6:
			text = K2L_Compatibility("\276\313\270\262 : ") + name +
				K2L_Compatibility(" \264\324\300\272 \307\366\300\347 \301\242\274\323\301\337\300\314 \276\306\264\325\264\317\264\331.");
			color = 0xffff7878;
			break;
		case 7:
			text = K2L_Compatibility("\276\313\270\262 : ") + msg;
			color = 0xffff7878;
			break;
		case 10:
		{
			unsigned long pang = packet.Decode4();
			text = K2L_Compatibility("\276\313\270\262 : ") +
				std::string(MakeStr(
					K2L_Compatibility(
						"%s\264\324\262\262\274\255 %d \306\316 \301\326\270\323\264\317\270\246 \310\271\265\346\307\317\277\264\275\300\264\317\264\331."),
					name.c_str(), pang));
			color = 0xffff7878;
		}
		break;
		// 983 (009c2c5b, tabela 009d79bc): tipos 11 a 16 novos; o 9 cai no default
		case 11:
		{
			// contagem para o início (Grand Prix): segundos que faltam
			unsigned long sec = packet.Decode4();
			if (sec == 0)
			{
				text = K2L_Compatibility(
					"\xb0\xd4\xc0\xd3 \xbd\xc3\xc0\xdb \xc1\xdf \xc0\xd4\xb4\xcf\xb4\xd9. \xc0\xe1\xbd\xc3\xb8\xb8 \xb1\xe2\xb4\xd9\xb7\xc1 \xc1\xd6\xbc\xbc\xbf\xe4.");
				if (g_audio)
					g_audio->PlaySfx("gp_in_lobby_start_5sec");
			}
			else
			{
				std::string ss; // 983: std::stringstream (a unidade não inclui <sstream>)
				unsigned long min = sec / 60;
				if (min)
				{
					ss += MakeStr(K2L_Compatibility("%d\xba\xd0"), min);
					if (sec - min * 60)
						ss += " ";
				}
				if (sec - min * 60)
					ss += MakeStr(K2L_Compatibility("%d\xc3\xca"), sec - min * 60);

				const char* __fastcall GetGameString(int key); // 00540a10 (include/jp/gamestringmanager.h)
				if (IsGrandPrixGameType983(Doc()->m_roomInfo.realGameType)) // 0046e6d0
				{
					text = std::string(MakeStr(GetGameString(0x7552), ss.c_str()));
					if (g_audio)
					{
						if (sec == 5)
						{
							g_audio->PlaySfx("gp_in_lobby_shout_5sec");
							g_audio->PlaySfx("gp_in_lobby_count_1sec");
						}
						else if (sec - 1 <= 3)
						{
							g_audio->PlaySfx("gp_in_lobby_count_1sec");
						}
					}
				}
			}
			color = 0xffff7878;
		}
		break;
		case 12:
			text = K2L_Compatibility(
				"\xbe\xcb\xb8\xb2 : \xc3\xd6\xb4\xeb \xc0\xce\xbf\xf8\xc0\xcc \xb8\xf0\xb5\xce \xb8\xf0\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9. \xb0\xd4\xc0\xd3\xc0\xcc \xc0\xda\xb5\xbf \xbd\xc3\xc0\xdb\xb5\xcb\xb4\xcf\xb4\xd9.");
			color = 0xffff7878;
			break;
		case 13:
		case 14:
		case 15:
		{
			// prêmio (13 sorteio do jackpot, 14 jackpot dividido, 15 prêmio da sorte): item e quantidade
			std::string tag("");
			if (chatType == 13)
				tag = K2L_Compatibility("<\xc0\xe8\xc6\xcc-\xc3\xdf\xc3\xb7\xb9\xe6\xbd\xc4> ");
			else if (chatType == 14)
				tag = K2L_Compatibility("<\xc0\xe8\xc6\xcc-\xb1\xd5\xb5\xee\xba\xd0\xc7\xd2> ");
			else if (chatType == 15)
				tag = K2L_Compatibility("<\xc7\xe0\xbf\xee\xbb\xf3> ");

			unsigned long tid = packet.Decode4();
			unsigned long qty = packet.Decode4();
			if (tid == 0x1a000010)
			{
				text = K2L_Compatibility("\276\313\270\262 : ") + tag +
					std::string(MakeStr(
						K2L_Compatibility("%s\xb4\xd4 %d\xc6\xce \xc1\xd6\xb8\xd3\xb4\xcf \xc8\xb9\xb5\xe6!"),
						name.c_str(), qty));
			}
			else if (qty == 1)
			{
				text = K2L_Compatibility("\276\313\270\262 : ") + tag +
					std::string(MakeStr(
						K2L_Compatibility("%s\xb4\xd4 %s \xc8\xb9\xb5\xe6!"),
						name.c_str(), ItemManager()->GetItemName(tid)));
			}
			else
			{
				text = K2L_Compatibility("\276\313\270\262 : ") + tag +
					std::string(MakeStr(
						K2L_Compatibility("%s\xb4\xd4 %s %d\xb0\xb3 \xc8\xb9\xb5\xe6!"),
						name.c_str(), ItemManager()->GetItemName(tid), qty));
			}
			color = 0xffff7878;
		}
		break;
		case 16:
		{
			// fim da partida de um jogador: tacadas, pangs e se jogou com assistência
			unsigned long score = packet.Decode4();
			__int64 pang = packet.Decode8();
			bool bAssist = packet.Decode1() == 1;
			text = std::string(MakeStr(
				K2L_Compatibility(
					"\"%s\"\xb4\xd4\xc0\xcc \"%d\"\xbd\xba\xc4\xda\xbe\xee, \"%I64d\xc6\xce\"%s\xc0\xb8\xb7\xce \xb0\xe6\xb1\xe2\xb8\xa6 \xbf\xcf\xc1\xd6\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
				name.c_str(), score, pang, bAssist ? "(assist)" : ""));
			color = 0xffff7878;
		}
		break;
		default:
			text = name + " : " + msg;
			break;
		}

		if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) &&
			Doc()->m_playingMode == 2)
		{
			bIgnore = true;
		}

		if (Doc()->m_gameMode != 5 && !bIgnore)
		{
			CChatMsg::Instance()->SetChatFacialUser(name);
			CChatMsg::Instance()->AddChatMsg(text, color, false, bName);
		}
		return 1;
	}
	break;
	case 0x41:
	{
		std::string msg;
		packet.DecodeStr(msg);
		CChatMsg::Instance()->AddChatMsg(msg, 0xffff7878, false, false);
	}
	break;
	case 0x42:
	{
		std::string msg;
		packet.DecodeStr(msg);
		CNoticeBoard::Instance()->AddNotice(msg.c_str(), 1, false);
	}
	break;
	case 0x1d3: // 983: era 0x1d5 no 645; caso 009d28c7 (D4, laço D4 + DS, CWinnerNotice::SetWinnerData 008ac130)
	{
		unsigned long count = packet.Decode4();
		while (count-- > 0)
		{
			std::string data("");
			int type = packet.Decode4();
			packet.DecodeStr(data);
			CWinnerNotice::Instance()->SetWinnerData(type, data);
		}
	}
	break;
	case 0x43:
	{
		struct sProtocol
		{
			unsigned long level;
			unsigned char sex;
			std::string name;
		} protocol;
		protocol.level = packet.Decode4();
		protocol.sex = packet.Decode1();
		packet.DecodeStr(protocol.name);

		const char* sexName[3] = { K2L_Compatibility("\263\262"), K2L_Compatibility("\277\251"), K2L_Compatibility("\272\322\270\355") };
		char szMsg[256];
		sprintf(szMsg,
			K2L_Compatibility("%s(\267\271\272\247:%d \274\272\272\260:%s)\264\324\300\314 "
			"\267\316\261\327\300\316 \307\337\275\300\264\317\264\331."),
			protocol.name.c_str(), protocol.level, sexName[protocol.sex]);
		CChatMsg::Instance()->AddChatMsg(std::string(szMsg), 0xffff7878, false,
			false);
	}
	break;
	case 0x44:
	{
		unsigned char result = packet.Decode1();
		switch (result)
		{
		case 0xd3:
		{
			FrWnd* pForm =
				g_pFresh->GetManager()->GetDesktop()->FindChildForm("server");
			if (!pForm || !IS_KINDOF(FrServerDlg, pForm))
				break;

			if (packet.Decode1() == 0)
			{
				((FrServerDlg*)pForm)->FinishAutoLogin();
			}
			else
			{
				((FrServerDlg*)pForm)->BeginAutoLogin();
			}
			return 1;
		}
		case 0xd2:
		{
			unsigned long count = packet.Decode4();

			FrWnd* pForm =
				g_pFresh->GetManager()->GetDesktop()->FindChildForm("server");
			if (!pForm || !IS_KINDOF(FrServerDlg, pForm))
				break;

			((FrServerDlg*)pForm)->AddLoginReplyCount(count);
			return 1;
		}
		case 0:
		{
			FrWnd* pForm =
				g_pFresh->GetManager()->GetDesktop()->FindChildForm("server");
			if (pForm && IS_KINDOF(FrServerDlg, pForm))
			{
				((FrServerDlg*)pForm)->AddLoginReplyCount(0x20); // 983 009c3a8b: 0x20 fecha o "Connecting" (00af31ae); 0x15 era do 645
			}

			UccManager()->BuildDisplayClothesSet();
			Doc()->ClearInventory();

			if (AfxGetTask()->IsCurrentActor983("shopmain")) // 983 009c3ab6
			{
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0xc5), 0, 0, 0, 0, 0));
			}

			// 983 (009c3b0c): uma string só, para CSharedDoc+0x10d08 (m_chatModeName no nosso cabeçalho); o 645 lia duas
			// (m_packetNickname, m_nicknameSuffix). Sem o ramo de sUserInfoExt (IsLocalContent 0x70).
			Doc()->m_chatModeName = packet.DecodeStr();

			sUserInfo userInfo;
			packet.DecodeBuffer(&userInfo, sizeof(sUserInfo)); // 0x31c6 (medido)

			RequestUccInfo(userInfo);

			_SYSTEMTIME serverTime;
			packet.DecodeBuffer(&serverTime, sizeof(serverTime));
			Doc()->SetServerTime(serverTime);
			CGrandPrixClock::Instance()->Sync983(); // 983 009c3b99: acerta e liga o relógio do Grand Prix

			unsigned char firstLogin = packet.Decode1();
			packet.Decode1();

			Doc()->SetBsUsableTimes(packet.Decode2());
			Doc()->SetBsUsableBonusTimes(packet.Decode2());
			Doc()->SetBsRemainedBonusTimes(packet.Decode2());

			if (IsLocalContent((localContentType_t)8))
			{
				Doc()->SetFlagBlock(packet.Decode4());
			}

			Doc()->m_controlServerService = packet.Decode8(); // 983 009c3c1e: 8 bytes (o 645 lia 4)
			Doc()->m_unused5060 = packet.Decode4();

			unsigned long serverProperty = packet.Decode4();
			Doc()->m_serverProperty = serverProperty;

			// 983: o pacote não traz GUILD_USER_INFO aqui (saíram a leitura e a mensagem M983(0x263) ao ator da guilda)

			S5::UTIL::SetServerProperty(serverProperty);
			S5::THEME::SetSkinnedTheme(serverProperty);

			Doc()->m_myInfo.roomIndex = userInfo.roomIndex;
			Doc()->m_myInfo.info = userInfo.info;
			Doc()->m_myInfo.stat = userInfo.stat;
			Doc()->m_myInfo.trophy = userInfo.trophy;
			Doc()->m_myInfo.userEquip = userInfo.userEquip;
			memcpy(Doc()->m_myInfo.mapStat, userInfo.mapStat,
				sizeof(userInfo.mapStat));
			memcpy(Doc()->m_myInfo.classicMapStat, userInfo.classicMapStat,
				sizeof(userInfo.classicMapStat));

			// 983 (009c3d04): mensagem 0x80 ao ator principal e, com guilda, a pagina do clube pelo CWebPageConnector (0xfb; a
			// resposta 0x1ad pede o RSS da guilda com RequestGuildRSS_JP) no lugar do RequestGuildRSS do 645
			NetResourceManager::Instance()->ResetGuildRSS();
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x80 /*so do 983*/, 0, 0, 0, 0, 0));
			if (userInfo.info.dwGuildId && CWebPageConnector::Instance())
				CWebPageConnector::Instance()->RequestOpen983("https://www5.pangya.jp/on_client/club", true, 5);

			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 5, 0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x42), 0, 0, 0, 0, 0));
			if (firstLogin == 1)
			{
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x43), 1, 0, 0, 0, 0));
			}

			if (!COption::Instance()->gIsWhisperAllowed())
			{
				WSendPacket send((enumClientPacket)0x55);
				send.Encode1(0);
				send.Send(TO_GAME);
			}

			if (WNetworkSystem::Instance()->IsConnected(
					WNetworkSystem::NET_MSN) &&
				CMessengerInfo::Instance()->GetComplete() == CONNECT_STATUS_COMPLETE /*983: +0xa4; o 645 lia +0x80 cru*/)
			{
				sUserPosition pos;
				pos.iServerGUID = Doc()->m_gameServerUID;
				wsprintfA(pos.csChannelName, "NONE");
				pos.iRoomIdx = 0xffff;
				pos.iRoomType = -1;

				WSendPacket send(0x23);
				send.EncodeBuffer(&pos, sizeof(sUserPosition));
				send.Send(TO_MSN);
			}

			// 983 009c3f1b: pacote 0xfe (dword 0) ao jogo e o caso acaba; o SetPeriodDoc/Set_PreSettingContentsDate
			// do 645 nao e chamado aqui (lote 9 da C-PKT)
			{
				WSendPacket send((enumClientPacket)0xfe);
				send.Encode4(0);
				send.Send(TO_GAME);
			}
			return 1;
		}
		case 2:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 6, 0, 0, 0, 0));
			break;
		case 4:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 0xb, 0, 0, 0, 0));
			break;
		case 5:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 8, 0, 0, 0, 0));
			break;
		case 6:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 7, 0, 0, 0, 0));
			break;
		case 7:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 9, 0, 0, 0, 0));
			break;
		case 8:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 0xa, 0, 0, 0, 0));
			break;
		case 0xb:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 0xc, 0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\274\255\271\366 \271\366\300\374\300\314 \264\331\270\250\264\317\264\331. \274\255\271\366\277\241 \301\242\274\323 \307\322 \274\366 \276\370\275\300\264\317\264\331."),
				0, 0, 0, 0));
			break;
		case 0xe:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 0xe, 0, 0, 0, 0));
			break;
		case 0xd6:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 0xf, 0, 0, 0, 0));
			break;
		case 0xd7:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 0x10, 0, 0, 0, 0));
			break;
		case 0x12:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 0x15, 0, 0, 0, 0));
			break;
		case 0xc9:
			GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x14, 0, 0, 0, 0);
			GetActor("Lobby") << MsgObject(NULL, M983(0x25),
				(int)K2L_Compatibility("\260\324\300\323\274\255\271\366\277\241 \301\242\274\323\307\322 \274\366 \276\370\275\300\264\317\264\331."),
				0, 0, 0, 0);
			return 1;
		case 0x10:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 0x11, 0, 0, 0, 0));
			break;
		case 0x13:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 0x12, 0, 0, 0, 0));
			break;
		case 0xdb:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 0x13, 0, 0, 0, 0));
			break;
		case 3:
		{
			// PKT (frente C, lote 11): 983 009c41b1 (novo; no 645 o 3 ia junto com o 0xe0): com a janela de servidores aberta,
			// zera o servidor corrente, avisa "nao foi possivel conectar" e volta a lista (00af4600 = FailedToConnectServer)
			FrWnd* pForm =
				g_pFresh->GetManager()->GetDesktop()->FindChildForm("server");
			if (pForm && IS_KINDOF(FrServerDlg, pForm))
			{
				C_ASSERT_VC10(offsetof(CSharedDoc, m_curGameServer.id) == 0x10ad1);
				Doc()->m_curGameServer.id = 0;
				((FrServerDlg*)pForm)->OnNotify(K2L_Compatibility(
					"\xbc\xad\xb9\xf6\xbf\xa1 \xc1\xa2\xbc\xd3\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."), 2);
				((FrServerDlg*)pForm)->FailedToConnectServer();
			}
			else
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x1f), 0xd, 0, 0, 0, 0));
		}
		break;
		case 0xe2: // PKT: era 0xe0 (645)
		{
			// PKT (frente C, lote 11): 983 009c423f - o tempo de bloqueio vai na mensagem 0x27 (a do erro do servidor; numero
			// do 983), sem o texto 0x76 do CMessageManager que o 645 montava
			unsigned long blockTime = packet.Decode4();
			if (blockTime == 0)
				break;

			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 0xd, 0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, blockTime, 0, 0, 0, 0));
		}
		break;
		default:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1f), 0xd, 0, 0, 0, 0));
			break;
		}
	}
	break;
	case 0x45:
	{
		unsigned short oldLevel = Doc()->m_myInfo.stat.Level;
		unsigned long oldExp = Doc()->m_myInfo.stat.dwExp;

		packet.DecodeBuffer(&Doc()->m_myInfo.stat,
			sizeof(sPangYaUserStatistics));
		packet.DecodeBuffer(&Doc()->m_myInfo.trophy, sizeof(sTrophyStatistics));

		// 983 (009c4310): 12 registros, um por vetor de 22 mapas (os 12 seguidos a partir de mapStat); 0xff = vetor sem
		// mudanca, o laco segue (009c431b pula para o proximo vetor)
		// D742: o servidor manda so 2 registros (347 B, formato do 645); o 983 le os outros 10 bytes do buffer alem do
		// pacote e um indice lixo (272) gravava 0x2b bytes em Doc+0x384a (lista de missoes) -> queda no OnQuestComplete983.
		// Com o pacote completo do 983 (12 registros) o laco e o mesmo; so nao le alem do fim do pacote.
		for (int group = 0; group < 12 && packet.GetRemainLength() > 0; group++)
		{
			unsigned char map = packet.Decode1();
			if (map != 0xff)
				packet.DecodeBuffer(&Doc()->m_myInfo.mapStat[group * 22 + map], sizeof(sMapStatistics));
		}

		std::map<unsigned long, sUserInfoTime>::iterator it;
		if (Doc()->FindUserInfoTimeGUID(MyGuid(false), it))
		{
			it->second.info.stat = Doc()->m_myInfo.stat;
			it->second.info.trophy = Doc()->m_myInfo.trophy;
		}

		// 983 009c43bc: tipo real 0xc/0xe/0x13 (00500200) ou 0x12 nao da exp ao caddie
		const unsigned char realType = Doc()->m_roomInfo.realGameType;
		if (realType == 0xc || realType == 0xe || realType == 0x13 || realType == 0x12)
			break;

		unsigned long newTotalExp = Doc()->m_myInfo.stat.Level
			? Doc()->m_levelTable[Doc()->m_myInfo.stat.Level - 1].totalExp
			: 0;
		unsigned long oldTotalExp =
			oldLevel ? Doc()->m_levelTable[oldLevel - 1].totalExp : 0;
		int exp =
			Doc()->m_myInfo.stat.dwExp - oldExp - oldTotalExp + newTotalExp;

		if (exp > 0)
		{
			std::map<unsigned int, sCaddieInfo>::iterator itCaddie =
				Doc()->m_caddieMap.find(Doc()->m_myInfo.userEquip.guidCaddie);
			if (itCaddie != Doc()->m_caddieMap.end())
			{
				sCaddieInfo& info = itCaddie->second;

				info.Exp += exp;

				// m_bonusPangTable (+0x4ff8): exp por nivel do caddie (3 niveis)
				if (info.Level == 0 &&
					info.Exp >= Doc()->m_bonusPangTable[0].exp)
				{
					info.Level++;
					info.Exp -= Doc()->m_bonusPangTable[0].exp;
				}
				if (info.Level == 1 &&
					info.Exp >= Doc()->m_bonusPangTable[1].exp)
				{
					info.Level++;
					info.Exp -= Doc()->m_bonusPangTable[1].exp;
				}
				if (info.Level == 2 &&
					info.Exp >= Doc()->m_bonusPangTable[2].exp)
				{
					info.Level++;
					info.Exp -= Doc()->m_bonusPangTable[2].exp;
				}
				return 1;
			}
		}
	}
	break;
	case 0x4b:
	{
		// 983 (009c44eb): o pacote comeca pelo erro; diferente de 0 vira texto na mensagem 0x23 ao ator principal e o caso acaba.
		unsigned long error983 = packet.Decode4();
		if (error983 != 0)
		{
			// 983 009c4513: com 0x557365 o texto é o 0x7544 da tabela; nos outros, o texto geral com o número. Os dois passam
			// pelo K2L_Compatibility antes da mensagem.
			const char* __fastcall GetGameString(int key); // 00540a10 (include/jp/gamestringmanager.h)
			std::string text("");
			if (error983 == 0x557365)
				text = GetGameString(0x7544);
			else
				text = MakeStr(K2L_Compatibility("\275\303\275\272\305\333 \277\300\267\371 : %d"), error983);
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, 0x23, (int)MakeStr(K2L_Compatibility(text.c_str())), 0, 0, 0, 0));
			break;
		}
		switch (packet.Decode1())
		{
		case 4:
		{
			unsigned long guid = packet.Decode4();
			sCharacterInfo charInfo;
			packet.DecodeBuffer(&charInfo, sizeof(sCharacterInfo));
			if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
			{
				RequestUccInfo(charInfo);
			}
			// 983 009c45cb: sempre a tarefa atual (a preservada so nos subs 1 e 5)
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x49), guid, (int)&charInfo, 0, 0, 0));
		}
		break;
		case 1:
		{
			unsigned long guid = packet.Decode4();
			sCaddieInfo caddieInfo;
			packet.DecodeBuffer(&caddieInfo, sizeof(sCaddieInfo));
			if (IS_EXACTKINDOF(ntAvatarChatTask,
					CTaskManager::Instance()->GetPreservedTask()) &&
				guid != MyGuid(false))
			{
				AfxGetPreservedTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x4a), guid, (int)&caddieInfo, 0, 0, 0));
			}
			else
			{
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x4a), guid, (int)&caddieInfo, 0, 0, 0));
			}
		}
		break;
		case 3:
		{
			unsigned long guid = packet.Decode4();
			sClubInfo clubInfo;
			packet.DecodeBuffer(&clubInfo, sizeof(sClubInfo));
			// 983 009c46c5: sempre a tarefa atual (a preservada so nos subs 1 e 5)
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x4b), guid, (int)&clubInfo, 0, 0, 0));
		}
		break;
		case 2:
		{
			unsigned long guid = packet.Decode4();
			unsigned long value = packet.Decode4();
			// 983 009c4700: sempre a tarefa atual (a preservada so nos subs 1 e 5)
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x4c), guid, value, 0, 0, 0));
		}
		break;
		case 5:
		{
			unsigned long guid = packet.Decode4();
			sMascotInfo mascotInfo;
			packet.DecodeBuffer(&mascotInfo, sizeof(sMascotInfo));
			if (IS_EXACTKINDOF(ntAvatarChatTask,
					CTaskManager::Instance()->GetPreservedTask()) &&
				guid != MyGuid(false))
			{
				AfxGetPreservedTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x4e), guid, (int)&mascotInfo, 0, 0, 0));
			}
			else
			{
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x4e), guid, (int)&mascotInfo, 0, 0, 0));
			}
		}
		break;
		case 6:
		{
			unsigned long guid = packet.Decode4();
			unsigned long value = packet.Decode4();
			float rate = 0;
			packet.DecodeBuffer(&rate, sizeof(rate));
			// 983 009c47c0: sempre a tarefa atual (a preservada so nos subs 1 e 5)
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x24e), guid, value, (int)rate, 0, 0));
		}
		break;
		}
		// 983 009c482c: na lista de Grand Prix, todo sub reconfere as linhas (o vetor e descartado)
		if (DOCJP()->GetRoomListKind983() == 1)
		{
			_gateway::_grandprix::CGrandPrixDocument* pGP = NULL;
			CContentsDoc::Instance()->GetContainer((localContentType_t)0xaf, pGP);
			__int64 now = CGrandPrixClock::Instance()->GetNow983();
			std::vector<sGPRoomItem983*> items;
			pGP->GetRoomItems983(&items, &now);
		}
	}
	break;
	case 0x4c:
	case 0x7e:
	{
		unsigned short roomIdx = packet.Decode2();
		if (roomIdx != 0xffff && roomIdx != 0x800) // 983 009c48c9: 0x800 tambem volta ao lobby
		{
			AfxPostMsg(NULL, "AvatarChat", M983(0x8), 0, 0, 0, 0);
			AfxPostMsg(NULL, "AvatarChat", M983(0x2), 0, 0, 0, 0);
			return 1;
		}

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned short noRoom = 0xffff;
		Doc()->m_myInfo.roomIndex = noRoom;
		Doc()->m_myInfo.info.dwGalleryGuid = -1;
		CProjectG::Instance()->FromGolfToLobbySettings();
		if (!IS_EXACTKINDOF(CLobbyTask, AfxGetTask()))
		{
			CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc",
				false);
		}
		// 983 009c49b7..009c4a9b (C392.1): sem aviso de volta e fora dos estados 1/2 da sala = TOPPAGE; senao, pelo tipo da sala
		// (+0x109b8): Grand Prix = TOPPAGE + 0x8b sub 4 (p2=1); tipo 0x17 ou 0xc/0xe/0x13 (00500200) = nada; outros = TOPPAGE + 0x8b sub 4.
		// Termina com a mensagem 0x17e ao ator principal.
		AfxPostMsg(NULL, "Lobby", M983(0x2), 0, 0, 0, 0);
		if (!Doc()->m_bReturnLobbyNotice && DOCJP()->GetState983() != 2 && DOCJP()->GetState983() != 1)
			AfxPostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
		else
		{
			unsigned char rt = Doc()->m_roomInfo.realGameType;
			if (IsGrandPrixGameType983(rt))
			{
				AfxPostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
				AfxPostMsg(NULL, "Lobby", 0x8b, 4, 1, 0, 0);
			}
			else if (rt != 0x17 && rt != 0xc && rt != 0xe && rt != 0x13)
			{
				AfxPostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
				AfxPostMsg(NULL, "Lobby", 0x8b, 4, 0, 0, 0);
			}
		}
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x17e, 0, 0, 0, 0, 0));
		if (CSceneManager::IsInstantiated())
			CSceneManager::Instance()->SetWeather(0, false);
		if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_MSN))
		{
			int msnState = CMessengerInfo::Instance()->GetComplete(); // 983: +0xa4 (o 645 lia +0x80 cru)
			if (msnState != 4)
			{
				CIntrusion::Instance()->FinishGame();
				return 1;
			}
			Doc()->m_roomInfo.roomGuid = noRoom;
			Doc()->m_roomInfo.gameType = 0xff;

			sUserPosition pos;
			pos.iServerGUID = Doc()->m_gameServerUID;
			pos.iChannelUid = Doc()->m_curChannel.Uid;
			wsprintfA(pos.csChannelName, "%s", Doc()->m_curChannel.Name);
			pos.iRoomIdx = noRoom;
			pos.iRoomType = -1;

			WSendPacket send(0x23);
			send.EncodeBuffer(&pos, sizeof(sUserPosition));
			send.Send(TO_MSN);
			if (CMessengerInfo::Instance()->GetMyStatus() != 3 /*983: +0xb0; o 645 lia +0x8c cru*/)
				CMessengerInfo::Instance()->SetMyStatus((MSN::eStatusType)4);
		}
		CIntrusion::Instance()->FinishGame();
		return 1;
	}
	case 0x47:
	{
		unsigned char count = packet.Decode1();
		unsigned char type = packet.Decode1();
		unsigned short lobby = packet.Decode2();
		sRoomInfo room;
		if (lobby != 0xffff)
			break;

		std::list<sRoomInfo>& roomList = Doc()->m_roomList;
		switch (type)
		{
		case 0:
		{
			roomList.clear();
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x6), 0, 0, 0, 0, 0));
			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&room, sizeof(sRoomInfo)); // 983: 0xdd, sem o ramo antigo (0x5e)
				Doc()->m_chatManager.FilteringHack(room.title, false);
				if (IsLocalContent((localContentType_t)0x2f /*983: era 0x2e; interpolado*/))
					CIntrusion::Instance()->OnAccessRoom(room);
				roomList.push_back(room);
				std::list<sRoomInfo>::iterator last = --roomList.end();
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x4), (int)&*last, 0, 0, 0, 0));
				if (room.roomGuid == Doc()->m_roomInfo.roomGuid)
				{
					Doc()->m_roomInfo = room;
					AfxGetTask()->SendMsgToMainActor(
						MsgObject(NULL, M983(0x10), 0, 0, 0, 0, 0));
				}
			}
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0xd), 0, 0, 0, 0, 0));
		}
		break;
		case 1:
		{
			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&room, sizeof(sRoomInfo)); // 983: 0xdd, sem o ramo antigo (0x5e)
				Doc()->m_chatManager.FilteringHack(room.title, false);
				if (IsLocalContent((localContentType_t)0x2f /*983: era 0x2e; interpolado*/))
					CIntrusion::Instance()->OnAccessRoom(room);
				std::list<sRoomInfo>::iterator it;
				it = std::find(roomList.begin(), roomList.end(), room.roomGuid);
				if (it == roomList.end())
				{
					roomList.push_back(room);
					std::list<sRoomInfo>::iterator last = --roomList.end();
					AfxGetTask()->SendMsgToMainActor(
						MsgObject(NULL, M983(0x4), (int)&*last, 0, 0, 0, 0));
				}
				else
				{
					*it = room;
				}
				if (room.roomGuid == Doc()->m_roomInfo.roomGuid)
				{
					Doc()->m_roomInfo = room;
					AfxGetTask()->SendMsgToMainActor(
						MsgObject(NULL, M983(0x10), 0, 0, 0, 0, 0));
				}
			}
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0xd), 0, 0, 0, 0, 0));
		}
		break;
		case 2:
		{
			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&room, sizeof(sRoomInfo));
				Doc()->m_chatManager.FilteringHack(room.title, false);
				if (IsLocalContent((localContentType_t)0x2f /*983: era 0x2e; interpolado*/))
					CIntrusion::Instance()->OnAccessRoom(room);
				std::list<sRoomInfo>::iterator it;
				it = std::find(roomList.begin(), roomList.end(), room.roomGuid);
				if (it != roomList.end())
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x5),
						(int)&*it, sizeof(sRoomInfo), 0, 0, 0));
					roomList.erase(it);
				}
			}
		}
		break;
		case 3:
		{
			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&room, sizeof(sRoomInfo));
				Doc()->m_chatManager.FilteringHack(room.title, false);
				if (IsLocalContent((localContentType_t)0x2f /*983: era 0x2e; interpolado*/))
					CIntrusion::Instance()->OnAccessRoom(room);
				std::list<sRoomInfo>::iterator it;
				it = std::find(roomList.begin(), roomList.end(), room.roomGuid);
				if (it != roomList.end())
					*it = room;
				if (room.roomGuid == Doc()->m_roomInfo.roomGuid)
				{
					Doc()->m_roomInfo = room;
					AfxGetTask()->SendMsgToMainActor(
						MsgObject(NULL, M983(0x10), 0, 0, 0, 0, 0));
				}
			}
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0xd), 0, 0, 0, 0, 0));
		}
		break;
		}
	}
	break;
	case 0x46:
	{
		unsigned char type = packet.Decode1();
		unsigned char count = packet.Decode1();
		sBriefUserInfo info;
		switch (type)
		{
		case 4:
			Doc()->m_briefUserInfoMap.clear();
			GetActor("Lobby") << MsgObject(NULL, M983(0xb), 0, 0, 0, 0, 0);
		case 5:
		{
			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&info, sizeof(sBriefUserInfo));
				if (Doc()->m_briefUserInfoMap.find(info.dwGuid) ==
					Doc()->m_briefUserInfoMap.end())
				{
					Doc()->m_briefUserInfoMap.insert(
						std::map<unsigned long, sBriefUserInfo>::value_type(
							info.dwGuid, info));
					std::map<unsigned long, sBriefUserInfo>::iterator it =
						Doc()->m_briefUserInfoMap.find(info.dwGuid);
					GetActor("Lobby")
						<< MsgObject(NULL, M983(0xa), (int)&it->second, 0, 0, 0, 0);
				}
			}
		}
		break;
		case 1:
		{
			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&info, sizeof(sBriefUserInfo));
				std::map<unsigned long, sBriefUserInfo>::iterator it =
					Doc()->m_briefUserInfoMap.find(info.dwGuid);
				if (it == Doc()->m_briefUserInfoMap.end())
				{
					Doc()->m_briefUserInfoMap.insert(
						std::map<unsigned long, sBriefUserInfo>::value_type(
							info.dwGuid, info));
					it = Doc()->m_briefUserInfoMap.find(info.dwGuid);
					GetActor("Lobby")
						<< MsgObject(NULL, M983(0xa), (int)&it->second, 0, 0, 0, 0);
				}
				else
				{
					it->second = info;
					if (info.dwIdentity & 4)
					{
						if (info.state & 1)
						{
							if (info.state & 0x400)
								GetActor("Lobby") << MsgObject(NULL, M983(0xa),
									(int)&it->second, 0, 0, 0, 0);
						}
					}
				}
			}
		}
		break;
		case 2:
		{
			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&info, sizeof(sBriefUserInfo));
				std::map<unsigned long, sBriefUserInfo>::iterator it =
					Doc()->m_briefUserInfoMap.find(info.dwGuid);
				if (it != Doc()->m_briefUserInfoMap.end())
				{
					if (info.dwIdentity & 4)
						it->second = info;
					GetActor("Lobby") << MsgObject(NULL, M983(0xc), (int)&it->second,
						sizeof(sBriefUserInfo), 0, 0, 0);
					if (info.dwIdentity & 4)
					{
						if (!(info.state & 0x400))
							Doc()->m_briefUserInfoMap.erase(it);
					}
					else
					{
						Doc()->m_briefUserInfoMap.erase(it);
					}
				}
			}
		}
		break;
		case 3:
		{
			packet.DecodeBuffer(&info, sizeof(sBriefUserInfo));
			std::map<unsigned long, sBriefUserInfo>::iterator it =
				Doc()->m_briefUserInfoMap.find(info.dwGuid);
			if (it != Doc()->m_briefUserInfoMap.end())
			{
				it->second = info;
				if (info.roomIndex == Doc()->m_myInfo.roomIndex &&
					info.roomIndex != 0xffff)
				{
					std::list<sSlotInfo>::iterator sit =
						Doc()->m_slotList.begin();
					for (; sit != Doc()->m_slotList.end(); sit++)
					{
						if (sit->dwGuid == info.dwGuid)
						{
							sit->dwIdentity = info.dwIdentity;
							sit->dwTitle = info.dwTitle;
							break;
						}
					}
				}
			}
		}
		break;
		}
	}
	break;
	case 0x4d:
	{
		// B1582: 983 009c565d nao manda o pacote 3 aqui; so FrServerDlg 00af4edf o envia
		int count = packet.Decode1();
		Doc()->m_channelList.clear();
		for (; count > 0; count--)
		{
			sChannelInfo channel;
			memset(&channel, 0, sizeof(channel));
			packet.DecodeBuffer(&channel, sizeof(channel));
			Doc()->m_channelList.push_back(channel);
		}
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1b), 0, 0, 0, 0, 0));
	}
	break;
	case 0x4e:
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		switch (packet.Decode1())
		{
		case 1:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1c), 0, 0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x20), 0, 0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x3),
				(int)MakeStr(
					K2L_Compatibility("[%s]\277\241 \300\324\300\345\307\337\275\300\264\317\264\331."),
					Doc()->m_curChannel.Name),
				0xffff7878, 0, 0, 0));
			if (Doc()->m_curChannel.Type & 0x4000)
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x3),
					(int)K2L_Compatibility("[EVENT]\306\316\271\350\306\262 5\260\346\261\342\270\246 \307\317\275\307 \266\247\270\266\264\331 1000\306\316\300\273 \265\345\270\263\264\317\264\331."),
					0xffff7878, 0, 0, 0));
			if (IsLocalContent((localContentType_t)0x29 /*983: era 0x28; interpolado*/))
				CMatchHistory::Instance()->LoginRequest(MyId());
			return 1;
		case 2:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x22),
				(int)K2L_Compatibility("\303\244\263\316\300\307 \301\244\277\370\274\366\260\241 \303\312\260\372\265\307\276\372\275\300\264\317\264\331."),
				0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x39), 0, 0, 0, 0, 0));
			break;
		case 3:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x22),
				(int)K2L_Compatibility("\303\244\263\316\300\273 \303\243\300\273\274\366 \276\370\275\300\264\317\264\331."),
				0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x39), 0, 0, 0, 0, 0));
			break;
		case 4:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x22),
				(int)K2L_Compatibility("\303\244\263\316 \301\244\272\270\260\241 \300\337\270\370 \300\374\264\336\265\307\276\372\275\300\264\317\264\331."),
				0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x39), 0, 0, 0, 0, 0));
			break;
		case 6:
		{
			// 983 009c59ac (novo): número de erro do servidor; se vier, vai ao ator principal pela mensagem 0x27 (a mesma do login) e a 0x1d fecha
			unsigned long serverError = packet.Decode4();
			if (serverError != 0)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, serverError, 0, 0, 0, 0));
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x1d, 0, 0, 0, 0, 0));
			}
		}
		break;
		}
		break;
	case 0x4f:
	{
		Doc()->ClearCurrentChannel(false);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xec), 0, 0, 0, 0, 0));
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x3a), 0, 0, 0, 0, 0));
		Doc()->m_roomList.clear();
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x6), 0, 0, 0, 0, 0));
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xeb), 0, 0, 0, 0, 0));
		COneLineBoard::Instance()->Clear();
	}
	break;
	case 0xf5:
	{
		Doc()->m_bReturnLobbyNotice = true;
		Doc()->m_bRebuildUnderBar = false;
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xeb), 0, 0, 0, 0, 0));
		if (!IS_KINDOF(CLobbyTask, AfxGetTask()))
			CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc",
				false);
		// 983 009c8df5: a lista de salas abre como janela sobre o lobby (msg 0x8b, 4); o layout "ROOMLIST" do 645 não existe
		CTaskManager::Instance()->PostMsgToMainActor983(NULL, 0, (int)"TOPPAGE", 0, 0);
		CTaskManager::Instance()->PostMsgToMainActor983(NULL, 0x8b, 4, 0, 0);
		AfxGetTask()->m_bRecieveWhisper = true;
	}
	break;
	case 0xf6:
	{
		Doc()->m_bReturnLobbyNotice = false;
		Doc()->m_bRebuildUnderBar = false;
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xeb), 0, 0, 0, 0, 0));
		if (!IS_KINDOF(CLobbyTask, AfxGetTask()))
			CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc",
				false);
		AfxPostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
		if (Doc()->m_roomListRequest983 == 1) // 983 009c8f9f
			AfxPostMsg(NULL, "Lobby", 0x84, 0, 0, 0, 0);
		AfxGetTask()->m_bRecieveWhisper = false;
		AfxPostMsg(NULL, "Lobby", 0x77, 0, 0, 0, 0);
	}
	break;
	case 0xf7:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xeb), 0, 0, 0, 0, 0));
	}
	break;
	case 0x108:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned char result = packet.Decode1();
		switch (result)
		{
		case 0:
		{
			std::string nick;
			packet.DecodeStr(nick);
			if (nick.length() >= 4 && nick.length() <= 16)
				strcpy(Doc()->m_myInfo.info.sNick, nick.c_str());
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0xe7), (int)nick.c_str(), 0, 0, 0, 0));
			if (WNetworkSystem::Instance()->IsConnected(
					WNetworkSystem::NET_MSN) &&
				CMessengerInfo::Instance()->GetComplete() == CONNECT_STATUS_COMPLETE /*983: +0xa4; o 645 lia +0x80 cru*/)
			{
				WSendPacket send(0x22);
				send.EncodeStr(nick);
				send.Send(TO_MSN);
			}
		}
		break;
		case 2:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\\c0xffff0000\\c\300\314\271\314 \273\347\277\353\265\307\260\355 \300\326\264\302 \264\353\310\255\270\355\300\324\264\317\264\331."),
				0, 0, 0, 0));
			break;
		case 3:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\264\353\310\255\270\355\300\272 \\c0xffff0000\\c\277\265\271\256 4 - 16 \261\333\300\332\\c0xff000000\\c \266\307\264\302 \\c0xffff0000\\c\307\321\261\333 2 - 8\261\333\300\332\\c0xff000000\\c\300\307 \261\346\300\314\270\246 \260\241\301\256\276\337 \307\317\270\347, \\c0xffff0000\\c\260\370\271\351\271\256\300\332\\c0xff000000\\c\270\246 \306\367\307\324\307\322 \274\366 \276\370\275\300\264\317\264\331."),
				0, 0, 0, 0));
			break;
		case 4:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\\c0xffff0000\\c\304\355\305\260\\c0xff000000\\c\260\241 \272\316\301\267\307\325\264\317\264\331.\012\273\363\301\241\277\241\274\255 \304\355\305\260\270\246 \303\346\300\374\307\330 \301\326\274\274\277\344."),
				0, 0, 0, 0));
			break;
		case 5:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\\c0xffff0000\\c\273\347\277\353\300\314 \261\335\301\366\265\310 \264\353\310\255\270\355\300\324\264\317\264\331."),
				0, 0, 0, 0));
			break;
		case 1:
		default:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\\c0xffff0000\\c\300\314\271\314 \273\347\277\353\265\307\260\355 \300\326\264\302 \264\353\310\255\270\355\300\324\264\317\264\331."),
				0, 0, 0, 0));
			break;
		}
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x40), result, 0, 0, 0, 0));
	}
	break;
	case 0x50:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		switch (result)
		{
		case 0:
		{
			std::string nick;
			packet.DecodeStr(nick);
			__int64 cookie = packet.Decode8();
			if (nick.length() >= 4 && nick.length() <= 16)
				strcpy(Doc()->m_myInfo.info.sNick, nick.c_str());

			// PKT (frente C, lote 12): 983 009c5c20 - com o conteudo 0x7f ligado o ticket 0x1a000003 e gasto por
			// CSharedDoc 006407d0 (DecreaseMyItem); sem ele, como no 645, e depois do cookie a tela do lobby (005fbe40)
			if (IsLocalContent((localContentType_t)0x7f /*983: era 0x82; interpolado*/))
			{
				int tid = 0x1a000003;
				std::list<sItemInfo>::iterator it =
					std::find(Doc()->m_myItemList.begin(),
						Doc()->m_myItemList.end(), tid);
				if (it != Doc()->m_myItemList.end())
					Doc()->DecreaseMyItem(it);
			}
			else
			{
				if (!IsPcBang())
				{
					int tid = 0x1a000003;
					std::list<sItemInfo>::iterator it =
						std::find(Doc()->m_myItemList.begin(),
							Doc()->m_myItemList.end(), tid);
					if (it == Doc()->m_myItemList.end())
					{
						Doc()->m_bonusCash = cookie;
						Doc()->RefreshAllCookie();
						if (RFLobbyDesktop::Instance983()) // 0047f920
							RFLobbyDesktop::Instance983()->Refresh983();
					}
					else
					{
						it->Common[0]--;
						if (it->Common[0] <= 0)
							Doc()->m_myItemList.erase(it);
					}
				}
			}

			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\264\353\310\255\270\355\300\314 \272\257\260\346 \265\307\276\372\275\300\264\317\264\331. \301\366\261\335\272\316\305\315 1\301\326\300\317 \310\304 \300\347 \272\257\260\346\300\314 \260\241\264\311\307\325\264\317\264\331."),
				0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0xe7), (int)nick.c_str(), 0, 0, 0, 0));

			if (WNetworkSystem::Instance()->IsConnected(
					WNetworkSystem::NET_MSN) &&
				CMessengerInfo::Instance()->GetComplete() == CONNECT_STATUS_COMPLETE /*983: +0xa4; o 645 lia +0x80 cru*/)
			{
				WSendPacket send(0x22);
				send.EncodeStr(nick);
				send.Send(TO_MSN);
			}
		}
		break;
		case 2:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\134c0xffff0000\134c\300\314\271\314 \273\347\277\353\265\307\260\355 \300\326\264\302 \264\353\310\255\270\355\300\324\264\317\264\331."),
				0, 0, 0, 0));
			break;
		case 3:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\264\353\310\255\270\355\300\272 \134c0xffff0000\134c\277\265\271\256 4 - 16 \261\333\300\332\134c0xff000000\134c \266\307\264\302 \134c0xffff0000\134c\307\321\261\333 2 - 8\261\333\300\332\134c0xff000000\134c\300\307 \261\346\300\314\270\246 \260\241\301\256\276\337 \307\317\270\347, \134c0xffff0000\134c\260\370\271\351\271\256\300\332\134c0xff000000\134c\270\246 \306\367\307\324\307\322 \274\366 \276\370\275\300\264\317\264\331."),
				0, 0, 0, 0));
			break;
		case 4:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\134c0xffff0000\134c\304\355\305\260\134c0xff000000\134c\260\241 \272\316\301\267\307\325\264\317\264\331.\012\273\363\301\241\277\241\274\255 \304\355\305\260\270\246 \303\346\300\374\307\330 \301\326\274\274\277\344."),
				0, 0, 0, 0));
			break;
		case 5:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\134c0xffff0000\134c\273\347\277\353\300\314 \261\335\301\366\265\310 \264\353\310\255\270\355\300\324\264\317\264\331."),
				0, 0, 0, 0));
			break;
		case 1:
		case 6:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\134c0xffff0000\134c\272\257\260\346 \277\344\303\273\300\314 \275\307\306\320\307\317\277\264\275\300\264\317\264\331."),
				0, 0, 0, 0));
			break;
		case 7:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\134c0xffff0000\134c\264\353\310\255\270\355 \272\257\260\346\300\314 \300\317\275\303\300\373\300\270\267\316 \301\337\264\334\265\307\276\372\275\300\264\317\264\331."),
				0, 0, 0, 0));
			break;
		case 9:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\134c0xffff0000\134c\300\314\300\374 \264\353\310\255\270\355 \272\257\260\346 \310\304 1\301\326\300\317\300\314 \301\366\263\252\301\366 \276\312\276\322\275\300\264\317\264\331."),
				0, 0, 0, 0));
			break;
		// PKT (frente C, lote 11): o 983 nao tem o 11 do 645 (tabela 009d7b8c vai ate 9; o resto cai no default) nem o texto dele
		default:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\134c0xffff0000\134c\300\314\271\314 \273\347\277\353\265\307\260\355 \300\326\264\302 \264\353\310\255\270\355\300\324\264\317\264\331."),
				0, 0, 0, 0));
			break;
		}
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x40), result, 0, 0, 0, 0));
	}
	break;
	case 0x51:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (packet.Decode1() != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\307\320\261\263 \272\257\260\346\277\241 \275\307\306\320\307\317\277\264\275\300\264\317\264\331."),
				0, 0, 0, 0));
		}
		else
		{
			Doc()->m_myInfo.info.school = packet.Decode4();
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\307\320\261\263\260\241 \272\257\260\346\265\307\276\372\275\300\264\317\264\331."),
				0, 0, 0, 0));
		}
	}
	break;
	case 0x70:
	{
		sCharacterInfo info;
		CPartTidList partTidList;
		int total = packet.Decode2();
		int count = packet.Decode2();
		for (int i = 0; i < count; i++)
		{
			packet.DecodeBuffer(&info, sizeof(info));
			partTidList.SetTids(&info, Doc()->m_myInfo.stat.Level);
			Doc()->AddChar(&info);
		}
		if (total == count)
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x2c), 1, 0, 0, 0, 0));
	}
	break;
	case 0x71:
	{
		sCaddieInfo info;
		int total = (unsigned short)packet.Decode2();
		int count = (unsigned short)packet.Decode2();
		for (int i = 0; i < count; i++)
		{
			packet.DecodeBuffer(&info, sizeof(info));
			Doc()->AddCaddie(&info);
			if (info.PCBangCaddie)
			{
				CSharedDoc* pDoc = Doc();
				unsigned long guidCaddie = pDoc->m_myInfo.userEquip.guidCaddie;
				if (guidCaddie != 0)
				{
					std::map<unsigned int, sCaddieInfo>::const_iterator it =
						pDoc->m_caddieMap.find(guidCaddie);
					if (it == pDoc->m_caddieMap.end() &&
						pDoc->m_caddieMap.size() > 0)
						pDoc->m_myInfo.userEquip.guidCaddie =
							pDoc->m_caddieMap.begin()->first;
				}
				// D751: 983 009c617d manda 0x75 1 para todo caddie PCBang (+0x18), trocando ou nao o equipado
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x68), 1, 0, 0, 0, 0));
			}
		}
		if (total == count)
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x2c), 2, 0, 0, 0, 0));
	}
	break;
	case 0xe1:
	{
		unsigned char count = packet.Decode1();
		while (count)
		{
			sMascotInfo info;
			packet.DecodeBuffer(&info, sizeof(sMascotInfo));
			CSharedDoc::Instance()->AddMascot(&info);
			// D748: 983 009c92e8 testa o byte +0x3d (PCBangMascot) e so DENTRO desse bloco confere o mascote equipado: zera
			// guidMascot quando ele NAO esta no mapa (009c93dd jne) e manda 0x75 2 nos dois casos. O nosso conferia fora do bloco,
			// para todo mascote e com o teste invertido: cada 0xe1 com o mascote equipado na lista o desequipava (guidMascot = 0).
			if (info.PCBangMascot)
			{
				const char* src =
					COption::Instance()->GetOption()->mPCBangMascotMsg;
				char* dst = info.szMsg;
				while ((*dst++ = *src++) != 0)
				{
				}
				CSharedDoc::Instance()->AddMascot(&info);

				WSendPacket send((enumClientPacket)0x9a);
				send.Encode1(2);
				send.Encode4(info.guid);
				send.EncodeStr(std::string(info.szMsg));
				send.Send(TO_GAME);
				CSharedDoc* pShared = CSharedDoc::Instance();
				unsigned long guid = pShared->m_myInfo.userEquip.guidMascot;
				if (guid != 0)
				{
					std::map<unsigned int, sMascotInfo>::const_iterator it =
						pShared->m_mascotMap.find(guid);
					if (it == pShared->m_mascotMap.end())
						pShared->m_myInfo.userEquip.guidMascot = 0;
				}
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x68), 2, 0, 0, 0, 0));
			}
			count--;
		}
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x2c), 0x10, 0, 0, 0, 0));
	}
	break;
	case 0x72:
	{
		sUserEquip equip;
		packet.DecodeBuffer(&equip, sizeof(equip));
		Doc()->SetCurEquipInfo(&equip);
	}
	break;
	case 0x73:
	{
		sItemInfo item;
		int total = (unsigned short)packet.Decode2();
		int count = (unsigned short)packet.Decode2();
		for (int i = 0; i < count; i++)
		{
			packet.DecodeBuffer(&item, sizeof(sItemInfo));

			bool bNew = true;
			switch (item.tid >> 26)
			{
			case 2:
			{
				if (IsLocalContent(S4_UCC) && IsUccClothes(item.tid))
				{
					std::list<sItemInfo>::iterator it =
						Doc()->m_uccItemList.begin();
					for (; it != Doc()->m_uccItemList.end(); it++)
					{
						if (it->tid == item.tid &&
							strcmp(it->UccIndex, item.UccIndex) == 0 &&
							it->Seq == item.Seq)
						{
							bNew = false;
							break;
						}
					}
				}

				std::list<sItemInfo>::iterator it = Doc()->m_partsList.begin();
				for (; it != Doc()->m_partsList.end(); it++)
				{
					if (it->guid == item.guid)
					{
						bNew = false;
						break;
					}
				}
			}
			break;
			case 4:
			{
				std::map<unsigned int, sItemInfo>::iterator it =
					Doc()->m_clubSetMap.begin();
				for (; it != Doc()->m_clubSetMap.end(); it++)
				{
					if (it->second.guid == item.guid)
					{
						bNew = false;
						break;
					}
				}
			}
			break;
			case 5:
			{
				std::list<sItemInfo>::iterator it = Doc()->m_ballList.begin();
				for (; it != Doc()->m_ballList.end(); it++)
				{
					if (it->guid == item.guid)
					{
						bNew = false;
						break;
					}
				}
				if (item.Common[0] > 19000)
				{
					if (item.Common[0] > 20000)
						Doc()->m_specialShotMark = 2;
					else if (Doc()->m_specialShotMark != 2)
						Doc()->m_specialShotMark = 1;
				}
			}
			break;
			case 6:
			{
				std::list<sItemInfo>::iterator it = Doc()->m_myItemList.begin();
				for (; it != Doc()->m_myItemList.end(); it++)
				{
					if (it->guid == item.guid)
					{
						bNew = false;
						break;
					}
				}
				if (item.Common[0] > 19000)
				{
					if (item.Common[0] > 20000)
						Doc()->m_specialShotMark = 2;
					else if (Doc()->m_specialShotMark != 2)
						Doc()->m_specialShotMark = 1;
				}
			}
			break;
			case 8:
			{
				std::list<sItemInfo>::iterator it =
					Doc()->m_cadItemList.begin();
				for (; it != Doc()->m_cadItemList.end(); it++)
				{
					if (it->guid == item.guid)
					{
						bNew = false;
						break;
					}
				}
			}
			break;
			case 9:
			{
				std::list<sItemInfo>::iterator it =
					Doc()->m_setItemList.begin();
				for (; it != Doc()->m_setItemList.end(); it++)
				{
					if (it->guid == item.guid)
					{
						bNew = false;
						break;
					}
				}
			}
			break;
			case 0xe:
			{
				std::list<sItemInfo>::iterator it = Doc()->m_cutinList.begin();
				for (; it != Doc()->m_cutinList.end(); it++)
				{
					if (it->guid == item.guid)
					{
						bNew = false;
						break;
					}
				}
			}
			break;
			case 0x1c:
			{
				std::list<sItemInfo>::iterator it =
					Doc()->m_auxPartList.begin();
				for (; it != Doc()->m_auxPartList.end(); it++)
				{
					if (it->guid == item.guid)
					{
						bNew = false;
						break;
					}
				}
			}
			break;
			case 0x12:
			{
				if (IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
				{
					std::list<sFurniture_List>::iterator it =
						Doc()->m_furnitureList.begin();
					for (; it != Doc()->m_furnitureList.end(); it++)
					{
						if (it->id == item.guid)
						{
							bNew = false;
							break;
						}
					}
				}
			}
			break;
			}

			if (!bNew)
				continue;

			switch (item.tid >> 26)
			{
			case 2:
				if (IsLocalContent(S4_UCC) && IsUccClothes(item.tid))
				{
					if (item.status & 1)
					{
						sUccClothes clothes;
						clothes.id = item.guid;
						clothes.typeId = item.tid;
						strncpy(clothes.uccIndex, item.UccIndex, 9);
						strncpy(clothes.uccName, item.ItemName, 0x29);
						clothes.date = item.ItemDate;
						clothes.downloadCount = 0;
						UccManager()->AddClothes(1, clothes, true, false);
					}
					else if (item.status & 2)
					{
						NetResourceManager::Instance()->DownloadUccTempClothes(
							item.tid, item.UccIndex, item.Seq, false);
					}
					Doc()->m_uccItemList.push_back(item);
				}
				else
				{
					Doc()->AddMyPartsList(item);
				}
				break;
			case 4:
				Doc()->m_clubSetMap.insert(
					std::map<unsigned int, sItemInfo>::value_type(item.guid,
						item));
				break;
			case 5:
				Doc()->m_ballList.push_back(item);
				break;
			case 6:
				Doc()->m_myItemList.push_back(item);
				break;
			case 8:
				Doc()->m_cadItemList.push_back(item);
				break;
			case 9:
				Doc()->m_setItemList.push_back(item);
				break;
			case 0xe:
				Doc()->m_cutinList.push_back(item);
				break;
			case 0x1c:
				Doc()->m_auxPartList.push_back(item);
				break;
			// D751: o 983 (009c6472, tabelas 009d7bdc/009d7c1c) so trata tipos 2..0x1c; item 0x1d do 0x73 nao entra em lista
			}
		}

		if (total == count)
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x2c), 4, 0, 0, 0, 0));

		if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/))
			Doc()->SetInitHaveHalloweenItem();

		// PKT (frente C, lote 13): 983 009c67cc - direto ao ator principal, sem testar o ator "RealMyRoom"
		if (IsLocalContent(S4_UCC))
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xe1), 0, 0, 0, 0, 0));
	}
	break;
	case 0x178:
	{
		Doc()->ClearCardStack();
		int count = packet.Decode2();
		for (; count > 0; count--)
		{
			sCardStack card;
			packet.DecodeBuffer(&card, sizeof(card));
			Doc()->AddCardStack(card, false);
		}
	}
	break;
	case 0x135:
		Doc()->m_cardList.clear();
		break;
	case 0x138:
	{
		unsigned long result = packet.Decode4();
		if (result == 0)
		{
			int count = packet.Decode2();
			for (; count > 0; count--)
			{
				sCards card;
				packet.DecodeBuffer(&card, sizeof(card));
				if ((card.tid & 0xfc000000) == 0x7c000000)
				{
					std::list<sCards>& cardList = Doc()->m_cardList;
					bool bFound = false;
					std::list<sCards>::iterator it = cardList.begin();
					for (; it != cardList.end(); it++)
					{
						if (it->uid == card.uid)
						{
							bFound = true;
							break;
						}
					}
					if (!bFound)
					{
						sCards newCard = card;
						cardList.push_back(newCard);
					}
				}
				else
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x25),
						(int)K2L_Compatibility("\300\337\270\370\265\310 \304\253\265\345 \265\245\300\314\305\315 \306\320\305\266\300\324\264\317\264\331"),
						0, 0, 0, 0));
				}
			}
			AfxGetTask()->SendMsgToMainActor( // PKT (frente C, lote 13): 983 009c94e8 - ator principal
				MsgObject(NULL, M983(0x21a), 0, 0, 0, 0, 0));
		}
		else if (result == 1)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x25),
				(int)K2L_Compatibility("\304\253\265\345 \301\244\272\270\270\246 \271\336\264\302\265\245 \275\307\306\320 \307\317\277\264\275\300\264\317\264\331"),
				0, 0, 0, 0));
		}
	}
	break;
	case 0x136:
		for (int i = 0; i < 4; i++)
			Doc()->m_cardAbilityList[i].clear();
		break;
	case 0x137:
	{
		int count = packet.Decode2();
		for (; count > 0; count--)
		{
			sSCardAvilityPeriodInfo info;
			packet.DecodeBuffer(&info, sizeof(info));
			if ((info.tid & 0xfc000000) == 0x7c000000)
			{
				// 983 009c969b: sem o card no Card.iff fica a habilidade do pacote; tipo e insercao (00641350) sempre
				sSCardAvilityPeriodInfo item = info;
				IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(item.tid);
				if (pCard)
				{
					item.Avility = pCard->Avility;
					item.AvilityValue = pCard->AvilityValue;
				}
				item.cardType = (item.tid >> 22) & 0xf;
				Doc()->m_cardAbilityList[0].push_back(item);
			}
			else
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x25),
					(int)K2L_Compatibility("\300\337\270\370\265\310 \304\253\265\345 \265\245\300\314\305\315 \306\320\305\266\300\324\264\317\264\331"),
					0, 0, 0, 0));
			}
		}
		SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0)); // PKT (frente C, lote 13): 983 009c965d
	}
	break;
	case 0x160:
	{
		unsigned long result = packet.Decode4();
		if (result == 0)
		{
			sCards card;
			packet.DecodeBuffer(&card, sizeof(sCards));

			std::list<sCards>::iterator it = Doc()->m_cardList.begin();
			for (; it != Doc()->m_cardList.end(); it++)
			{
				if (it->tid == card.tid)
				{
					it->cardCount--;
					if (it->cardCount < 1)
						Doc()->m_cardList.erase(it);
					break;
				}
			}

			// sCards: +0x08 partsTid, +0x0c partsUid, +0x10 slotNum, +0x18 useStartTime, +0x28 useEndTime
			sSCardAvilityPeriodInfo info;
			info.uid = card.uid;
			info.tid = card.tid;
			info.useStartTime = card.useStartTime;
			info.useEndTime = card.useEndTime;
			info.cardType = (card.tid >> 22) & 0xf;
			info.partsTid = card.partTid;
			info.partsUid = card.partUid;
			info.slotNum = card.selectedCardSlotNum;

			IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(card.tid);
			if (pCard)
			{
				info.Avility = pCard->Avility;
				info.AvilityValue = pCard->AvilityValue;
			}

			if (info.cardType == 2)
			{
				if (pCard->Avility == 1)
					Doc()->m_myInfo.stat.dwExp +=
						pCard->AvilityValue; // D838: 983 009c999d add [Doc+0x919] = stat.dwExp (o 645 era m_myInfo+0x155)

				if (pCard->Avility != 1 && pCard->Avility != 4 &&
					pCard->Avility != 0x11)
				{
					if (IsLocalContent((localContentType_t)0x6f))
					{
						std::list<sSCardAvilityPeriodInfo>::iterator it2 =
							Doc()->m_cardAbilityList[0].begin();
						for (; it2 != Doc()->m_cardAbilityList[0].end(); it2++)
						{
							sSCardAvilityPeriodInfo& cur = *it2;
							if ((cur.tid & 0x3c00000) == 0x800000)
							{
								if (info.Avility == cur.Avility)
								{
									Doc()->m_cardAbilityList[0].erase(it2);
									break;
								}
							}
						}
						Doc()->m_cardAbilityList[0].push_back(info);
					}
					else
					{
						Doc()->m_cardAbilityList[0].push_back(info);
					}
					CCardManager::Instance()->SetSPCardAlarm(true);
					// PKT (frente C, lote 13): 983 009c9a4e - mensagem 0xca ao "Lobby" (so do 983; sem par no 645)
					AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0xca, 0, 0, 0, 0, 0);
				}
			}
			else if (info.cardType == 0 || info.cardType == 1 || info.cardType == 5) // 983: tipo 5 novo (009c9a87)
			{
				if (info.cardType == 1)
					info.slotNum -= 4;
				if (info.cardType == 5)
					info.slotNum -= 8;

				std::list<sSCardAvilityPeriodInfo>::iterator it2 =
					Doc()->m_cardAbilityList[0].begin();
				for (; it2 != Doc()->m_cardAbilityList[0].end(); it2++)
				{
					if (it2->partsUid == info.partsUid)
					{
						if (it2->cardType == info.cardType &&
							it2->slotNum == info.slotNum)
						{
							Doc()->m_cardAbilityList[0].erase(it2);
							break;
						}
					}
				}
				Doc()->m_cardAbilityList[0].push_back(info);
			}

			CCardManager::Instance()->UpdateCardBuffDlg();
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0xdd), 1, 0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x219), card.tid, 0, 0, 0, 0));
		}
		else if (result == 1)
		{
			if (packet.Decode1() == 1)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), // 983 009c97e4: ator principal
					(int)K2L_Compatibility("\300\314 \304\253\265\345\300\307 \310\277\260\372\264\302 \300\314\271\314 \300\373\277\353\265\307\276\356 \300\326\275\300\264\317\264\331."),
					0, 0, 0, 0));
			}
			else
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
					(int)K2L_Compatibility("\304\253\265\345\270\246 \273\347\277\353\307\317\264\302\265\245 \275\307\306\320 \307\317\277\264\275\300\264\317\264\331"),
					0, 0, 0, 0));
			}
		}
		else if (result == 0x35)
		{
			// 983 (009c97a9): resultado 0x35 traz o numero do erro; texto "erro : %d" (00d405e8) na mensagem 0x23.
			unsigned long error = packet.Decode4();
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)MakeStr(K2L_Compatibility("\277\241\267\257 : %d"), error), 0, 0, 0, 0));
		}
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
	}
	break;
	case 0x139:
	{
		int count = packet.Decode2();
		for (; count > 0; count--)
		{
			sSCardAvilityPeriodInfo info;
			packet.DecodeBuffer(&info, sizeof(info));
			std::list<sSCardAvilityPeriodInfo>& list =
				Doc()->m_cardAbilityList[0];
			std::list<sSCardAvilityPeriodInfo>::iterator it = list.begin();
			for (; it != list.end(); it++)
			{
				if (it->uid == info.uid)
				{
					list.erase(it);
					break;
				}
			}
		}
	}
	break;
	case 0x162:
	{
		int count = packet.Decode2();
		for (; count > 0; count--)
		{
			sSCardAvilityPeriodInfo info;
			packet.DecodeBuffer(&info, sizeof(info));
			Doc()->m_cardAbilityList[0].push_back(info);
		}
	}
	break;
	case 0x161:
	{
		int count = packet.Decode2();
		for (; count > 0; count--)
		{
			sSCardAvilityPeriodInfo info;
			packet.DecodeBuffer(&info, sizeof(info));
			std::list<sSCardAvilityPeriodInfo>& list =
				Doc()->m_cardAbilityList[0];
			std::list<sSCardAvilityPeriodInfo>::iterator it = list.begin();
			for (; it != list.end(); it++)
			{
				if (it->uid == info.uid)
				{
					it->partsTid = info.partsTid;
					it->partsUid = info.partsUid;
					break;
				}
			}
		}
	}
	break;
	case 0x17a:
	{
		unsigned long tid = packet.Decode4();
		unsigned long uid = packet.Decode4();
		unsigned long newTid = packet.Decode4();
		unsigned long newUid = packet.Decode4();
		std::list<sSCardAvilityPeriodInfo>& list = Doc()->m_cardAbilityList[0];
		std::list<sSCardAvilityPeriodInfo>::iterator it = list.begin();
		for (; it != Doc()->m_cardAbilityList[0].end(); it++)
		{
			if (it->partsTid == tid && it->partsUid == uid)
			{
				it->partsTid = newTid;
				it->partsUid = newUid;
			}
		}
	}
	break;
	case 0x142:
	{
		sSCardAvilityPeriodInfo info;
		memset(&info, 0, sizeof(info));
		unsigned long result = packet.Decode4();
		if (result == 0)
		{
			packet.DecodeBuffer(&info, sizeof(info));
			std::list<sSCardAvilityPeriodInfo>& list =
				Doc()->m_cardAbilityList[0];
			std::list<sSCardAvilityPeriodInfo>::iterator it = list.begin();
			for (; it != list.end(); it++)
			{
				if (it->uid == info.uid)
				{
					if (it->Avility == 0xb)
					{
						int slot =
							CCardManager::Instance()->GetCardPeriodSlot();
						if (Doc()->GetNumAddItemSlotByMascot() == 1 || slot > 1)
						{
							Doc()->m_myInfo.userEquip.tidItemSlot[9] = 0; // D838: 983 009c9dda [Doc+0xa5a]
						}
						else if (Doc()->GetNumAddItemSlotByMascot() == 0 &&
							slot == 0)
						{
							Doc()->m_myInfo.userEquip.tidItemSlot[8] = 0; // D838: 983 009c9dc1 [Doc+0xa56] (o 645 era m_myInfo+0x278)
							Doc()->m_myInfo.userEquip.tidItemSlot[9] = 0;
						}
					}
					list.erase(it);
					CCardManager::Instance()->UpdateCardBuffDlg();
					break;
				}
			}
		}
		else if (result == 1)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\304\253\265\345\300\307 \261\342\260\243\301\246 \306\257\274\366\264\311\267\302 \273\350\301\246\277\241 \275\307\306\320 \307\317\277\264\275\300\264\317\264\331"),
				0, 0, 0, 0));
		}
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);
	}
	break;
	case 0x154:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		FrWnd* pWnd = g_pFresh->GetDesktop()->FindChildForm("FrCardBoxDlg");
		if (pWnd)
		{
			((RbCardBoxDlg*)pWnd)->RecievedPacket(&packet);
			break;
		}

		unsigned long result = packet.Decode4();
		int cardTid[4];
		cardTid[3] = 0;
		cardTid[0] = 0;
		cardTid[1] = 0;
		cardTid[2] = 0;
		if (result == 0)
		{
			sCards card;
			packet.DecodeBuffer(&card, sizeof(sCards));
			cardTid[0] = card.tid;

			std::list<sCards>::iterator it = Doc()->m_cardList.begin();
			for (; it != Doc()->m_cardList.end(); it++)
			{
				if (it->tid == card.tid)
				{
					it->cardCount--;
					if (it->cardCount < 1)
						Doc()->m_cardList.erase(it);
					break;
				}
			}

			int count = packet.Decode1();
			int index = 1;
			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&card, sizeof(sCards));
				int num = packet.Decode4();
				for (int j = 0; j < num; j++)
				{
					cardTid[index] = card.tid;
					if (index < 3)
						index++;
				}

				bool bFound = false;
				for (std::list<sCards>::iterator it2 =
						 Doc()->m_cardList.begin();
					it2 != Doc()->m_cardList.end(); it2++)
				{
					if (it2->tid == card.tid)
					{
						it2->cardCount += num;
						bFound = true;
						break;
					}
				}
				if (!bFound)
					Doc()->m_cardList.push_back(card);
			}

			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0xdd), 1, 0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x229), cardTid[0],
				cardTid[1], cardTid[2], cardTid[3], 0));
		}
		else if (result == 1)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\304\253\265\345\306\321 \277\300\307\302\277\241 \275\307\306\320 \307\317\277\264\275\300\264\317\264\331"),
				0, 0, 0, 0));
		}
	}
	break;
	case 0x75:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x25),
			(int)K2L_Compatibility("\300\337\270\370\265\310 \265\245\300\314\305\315 \306\320\305\266\300\273 \272\270\263\302\275\300\264\317\264\331"),
			0, 0, 0, 0));
	}
	break;
	// PKT (lote 54): 0x145 e 0x7a nao tem caso no 983 (009d78a7, return 1): ver a lista antes do default
	case 0x87:
		if (packet.Decode1() != 0)
		{
			sCaddieInfo caddie;
			packet.DecodeBuffer(&caddie, sizeof(sCaddieInfo));
			Doc()->AddCaddie(&caddie);
			Doc()->m_myInfo.userEquip.guidCaddie = caddie.guid;
			Doc()->m_userInfo[0].caddieInfo = caddie;
		}
		break;
	case 0x9f:
	{
		Doc()->m_gameServerList.clear();
		int count = packet.Decode1();
		sGameServerInfo server;
		for (int i = 0; i < count; i++)
		{
			memset(&server, 0, sizeof(sGameServerInfo));
			packet.DecodeBuffer(&server, sizeof(sGameServerInfo));
			if (server.property & 0x1000)
				continue;
			Doc()->m_gameServerList.push_back(server);
		}
		Doc()->m_channelList.clear();
		int channelCount = packet.Decode1();
		for (int i = 0; i < channelCount; i++)
		{
			sChannelInfo channel;
			packet.DecodeBuffer(&channel, sizeof(sChannelInfo));
			Doc()->m_channelList.push_back(channel);
		}
		if (IsLocalContent(S4_GOWITH_FRIEND /*983: era 0x53 (645); medido 009c20a0*/) ||
			IsLocalContent((localContentType_t)0x55 /*983: era 0x54; medido*/) ||
			IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
		{
			int mode = Doc()->m_goWithMode;
			if (mode == 1)
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x216), 0, 0, 0, 0, 0));
			else if (mode == 2)
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x216), 0, 0, 0, 0, 0));
			else if (mode == 3)
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x216), 0, 0, 0, 0, 0));
			else
			{
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x19), 0, 0, 0, 0, 0));
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x1b), 0, 0, 0, 0, 0));
			}
		}
		else
		{
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x19), 0, 0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x1b), 0, 0, 0, 0, 0));
		}
	}
	break;
	case 0xfc:
	{
#define RB_MSN_SERVERLIST \
	(CMessengerInfo::Instance()->ServerList983() /*983 009ca33d +0x88: m_serverList; o 645 lia +0x64*/)
		RB_MSN_SERVERLIST.clear();
		MSNServerInfo info;
		int count = packet.Decode1();
		for (; count > 0; count--)
		{
			memset(&info, 0, sizeof(sGameServerInfo));
			packet.DecodeBuffer(&info, sizeof(sGameServerInfo));
			RB_MSN_SERVERLIST.push_back(info);
		}
#undef RB_MSN_SERVERLIST
		CMessengerInfo::Instance()->SetServerListWaitingState(false);
	}
	break;
	case 0xa1:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (packet.Decode1() == 0)
		{
			unsigned long uid = packet.Decode4();
			unsigned char buf[0x12d];	// 983 009ca3f6: bloco de 0x12d (era 0x10d do 645)
			packet.DecodeBuffer(buf, sizeof(buf));
			// PKT (frente C, lote 13): 983 009ca3fb - sem o teste do CRealMyRoomTask (os dois ramos eram iguais)
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x50), uid, (int)buf, 0, 0, 0));
		}
		else
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\300\257\300\372\270\246 \303\243\300\273 \274\366 \276\370\275\300\264\317\264\331."),
				0, 0, 0, 0));
		}
	}
	break;
	case 0xa2:
		Doc()->m_rankServerAddr = packet.DecodeStr();
		Doc()->m_rankServerPort = packet.Decode4();
		WNetworkSystem::Instance()->Init(WNetworkSystem::NET_RANK);
		WNetworkSystem::Instance()->SendMessage(WNetworkSystem::NET_RANK,
			(NetworkUnit::eEvent)0);
		break;
	case 0xad:
	{
		char msg[512];
		std::string nick;
		packet.DecodeStr(nick);
		unsigned char channel = packet.Decode1();
		std::list<sChannelInfo>::iterator it = std::find(
			Doc()->m_channelList.begin(), Doc()->m_channelList.end(), channel);
		if (it == Doc()->m_channelList.end())
			sprintf(msg,
				K2L_Compatibility("%s\264\324\300\272 [%s]\277\241\274\255 \303\243\300\273 \274\366 \276\370\275\300\264\317\264\331."),
				nick.c_str(), (char*)&Doc()->m_curGameServer);
		else
			sprintf(msg,
				K2L_Compatibility("%s\264\324\300\272 [%s]\277\241 \301\242\274\323\307\330 \300\326\275\300\264\317\264\331."),
				nick.c_str(), it->Name);
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x3), (int)msg, 0xffff7878, 0, 0, 0));
	}
	break;
	case 0xc6:
		if (Doc()->m_myInfo.stat.Level > 0)
		{
			if (Doc()->m_myInfo.stat.dwNoMannerGameCount > 0)
				Doc()->m_myInfo.stat.dwNoMannerGameCount--;
			if (Doc()->m_myInfo.stat.dwGameCount > 0)
			{
				Doc()->m_myInfo.stat.dwGameCount--;
				if (Doc()->m_myInfo.stat.dwDisconnectCount > 0)
					Doc()->m_myInfo.stat.dwDisconnectCount--;
			}
		}
		break;
	case 0xb9:
		Doc()->m_newRecordState = packet.Decode1();
		if (IS_KINDOF(CLobbyTask, AfxGetTask()))
			AfxGetTask()->GetActor("Lobby")
				<< MsgObject(NULL, M983(0x53), 0, 0, 0, 0, 0);
		break;
	case 0x84:
	{
		unsigned char flag = packet.Decode1();
		unsigned char bNotice = flag >> 7;
		flag &= 0x7f;

		bool bSend = (flag == 0);

		unsigned long color = 0xffd2a2ff;
		if (bNotice)
		{
			color = 0xff6eecfb;
		}

		std::string nick;
		std::string text;
		std::string msg;

		packet.DecodeStr(nick);
		packet.DecodeStr(text);

		if (bSend)
		{
			msg = MyNick();
			msg += ">";
			msg += nick;
		}
		else
		{
			if (!COption::Instance()->gIsWhisperAllowed())
				return 1;

			std::list<std::string>::iterator it = std::find(
				Doc()->m_ignoreList.begin(), Doc()->m_ignoreList.end(), nick);
			if (it != Doc()->m_ignoreList.end())
				return 1;

			msg = nick + ">" + MyNick();
		}

		Doc()->m_chatManager.FilteringHack(text.c_str(), false);

		msg += " : \010'";
		msg += text;
		msg += "'";

		if (AfxGetTask()->IsRecieveWhisper())
		{
			CChatMsg::Instance()->AddChatMsg(msg, color, false, true);
		}
		else
		{
			WSendPacket send((enumClientPacket)0xde);
			send.EncodeStr(nick);
			send.Send(TO_GAME);
		}
	}
	break;
	case 0xbb:
		Doc()->m_roomSetupStage |= packet.Decode4();
		if (Doc()->m_roomSetupStage & 1)
		{
			std::string msg(
				K2L_Compatibility("\276\313\270\262 : \270\360\265\347 \307\303\267\271\300\314\276\356\260\241 \300\314\272\245\306\256 \300\307\273\363\300\273 \300\324\260\355 \300\326\300\270\271\307\267\316, 2\271\350\300\307 \305\254\270\256\276\356 \272\270\263\312\275\272\260\241 \300\373\277\353\265\313\264\317\264\331."));
			CChatMsg::Instance()->AddChatMsg(msg, 0xffff7878, false, false);
		}
		else if (Doc()->m_roomSetupStage & 2) // 983 (009ca749)
		{
			std::string msg(
				K2L_Compatibility("\xbe\xcb\xb8\xb2 : \xb8\xf0\xb5\xe7 \xc7\xc3\xb7\xb9\xc0\xcc\xbe\xee\xb0\xa1 \xc0\xcc\xba\xa5\xc6\xae \xc0\xc7\xbb\xf3\xc0\xbb \xc0\xd4\xb0\xed \xc0\xd6\xc0\xb8\xb9\xc7\xb7\xce, \xb5\xe5\xb6\xf8\xc0\xb2\xc0\xcc 2\xb9\xe8\xb7\xce \xc0\xfb\xbf\xeb\xb5\xcb\xb4\xcf\xb4\xd9."));
			CChatMsg::Instance()->AddChatMsg(msg, 0xffff7878, false, false);
		}
		break;
	case 0x89:
	{
		unsigned long result = packet.Decode4();
		const char* msg;
		if (result == 0)
			msg =
				K2L_Compatibility("\300\257\300\372\301\244\272\270 \301\266\310\270\277\241 \275\307\306\320\307\337\275\300\264\317\264\331.");
		else if (result == 2)
			msg =
				"\277\344\303\273\307\321 \300\257\300\372\300\307 \301\244\272\270\260\241 \300\337\270\370\265\307\276\372\275\300\264\317\264\331. \264\331\275\303 \275\303\265\265\307\330 \301\326\274\274\277\344";
		else if (result == 3)
		{
			if (CUserInfo::Instance()->IsVisible())
				CUserInfo::Instance()->Close();
			return 1;
		}
		else
		{
			unsigned char season = packet.Decode1();
			unsigned long uid = packet.Decode4();
			std::map<unsigned long, sUserInfoTime>::iterator it;
			if (!Doc()->FindUserInfoTimeUID(uid, it))
				break;
			it->second.dwUpdate = g_CurrentTime;
			if (!CUserInfo::Instance()->IsVisible())
				break;
			if (!CUserInfo::Instance()->IsRecvedData(season))
				break;
			if (MyUID() != uid)
				CUserInfo::Instance()->SetInfo(uid, 0, true, false, false,
					false, std::string(it->second.info.info.sNick));
			else
				CUserInfo::Instance()->SetInfo(MyUID(), RbMyGuid(false), true,
					false, false, false,
					std::string(it->second.info.info.sNick));
			CUserInfo::Instance()->Refresh();
			return 1;
		}
		if (CUserInfo::Instance()->IsVisible())
			CUserInfo::Instance()->Close();
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x23), (int)msg, 0, 0, 0, 0));
	}
	break;
	case 0x8d:
	{
		unsigned long elapsed = packet.Decode4();
		Doc()->m_approachStartTime = g_CurrentTime - elapsed;
	}
	break;
	case 0x93:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned char result = packet.Decode1();
		switch (result)
		{
		case 4:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\\c0xffff0000\\c\261\335\276\327\300\314 \272\316\301\267\307\325\264\317\264\331."),
				0, 0, 0, 0));
			break;
		case 1:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\\c0xffff0000\\c\260\355\277\353\261\342\260\243\300\314 \276\306\301\367 \263\262\276\306 \300\326\275\300\264\317\264\331."),
				0, 0, 0, 0));
			break;
		case 2:
		{
			unsigned long guid = packet.Decode4();
			__int64 value = packet.Decode8();
			CSharedDoc* pDoc = Doc();
			std::map<unsigned int, sCaddieInfo>::iterator it =
				pDoc->m_caddieMap.find(guid);
			if (it != pDoc->m_caddieMap.end())
				it->second.Remain_Date = 0x1e;
			if (Doc()->m_myInfo.stat.i64Pang > value)
				Doc()->m_myInfo.stat.i64Pang = value;
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\304\263\265\360\270\246 \\c0xffff0000\\c\307\321\264\336\260\243 \\c0xff000000\\c\300\347\260\355\277\353 \307\337\275\300\264\317\264\331."),
				0, 0, 0, 0));
			if (RFLobbyDesktop::Instance983()) // 983 009c8ad8: 0047f920
				RFLobbyDesktop::Instance983()->Refresh983();
		}
		break;
		case 0:
		case 3:
		case 5:
		case 6:
		default:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\\c0xffff0000\\c\304\263\265\360 \300\347\260\355\277\353\277\241 \275\307\306\320\307\337\275\300\264\317\264\331."),
				0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x5c), result, 0, 0, 0, 0));
			break;
		}
	}
	break;
	case 0x9d:
		if (!GalleryMode())
			break;
		packet.DecodeBuffer(&Doc()->m_userInfo[0], sizeof(sUserInfo));
		Doc()->m_myInfo.info.dwGalleryGuid = Doc()->m_userInfo[0].info.dwGuid;
		if (!IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
			break;
		RequestUccInfo(Doc()->m_userInfo[0]);
		break;
	case 0xa7:
	{
		for (int n = packet.Decode1(); n > 0; n--)
		{
			unsigned long tid = packet.Decode4();
			unsigned long guid = packet.Decode4();
			unsigned short count = packet.Decode2();

			switch (tid >> 26)
			{
			case 6:
			{
				// 983 009ca853: a lista dos que ficam com 0 (0x1a000020..27 no 645) so tem 0, entao tudo sai ao zerar.
				// 0x1a000080..83 nao casam pelo guid; 0x1a000104 casa pelo tid (009ca8c0..009ca8eb)
				for (std::list<sItemInfo>::iterator it =
						 Doc()->m_myItemList.begin();
					it != Doc()->m_myItemList.end(); ++it)
				{
					bool bKeepGuid = it->tid - 0x1a000080 <= 3;
					if ((it->guid == guid && !bKeepGuid) ||
						(it->tid == tid && tid == 0x1a000104))
					{
						it->Common[0] = count;
						if (count < 1)
						{
							Doc()->m_myItemList.erase(it);
						}
						break;
					}
				}
			}
			break;
			case 5:
			{
				for (std::list<sItemInfo>::iterator it =
						 Doc()->m_ballList.begin();
					it != Doc()->m_ballList.end(); ++it)
				{
					if (it->guid == guid)
					{
						it->Common[0] = count;
						if (count < 1)
						{
							Doc()->m_ballList.erase(it);
							// 983 009caa12: com o bit 14 de +0x812 a bola padrao e a 0x140000d8
							Doc()->m_myInfo.userEquip.tidBall =
								((Doc()->m_myInfo.info.dwIdentity >> 0xe) & 1) ? 0x140000d8 : 0x14000000;
						}
						break;
					}
				}
			}
			break;
			case 28:
			{
				for (std::list<sItemInfo>::iterator it =
						 Doc()->m_auxPartList.begin();
					it != Doc()->m_auxPartList.end(); ++it)
				{
					if (it->guid == guid)
					{
						it->Common[0] = count;

						if (count < 1)
						{
							for (std::map<unsigned int,
									 sCharacterInfo>::iterator itChar =
									 Doc()->m_charMap.begin();
								itChar != Doc()->m_charMap.end(); ++itChar)
							{
								unsigned long slot = (tid >> 21) & 0x1f;
								if (slot < 5 &&
									itChar->second.tidAuxParts[slot] == tid)
								{
									itChar->second.tidAuxParts[slot] = 0;
								}
							}
							Doc()->m_auxPartList.erase(it);
						}
						break;
					}
				}
			}
			break;
			case 2:
			{
				for (std::list<sItemInfo>::iterator it =
						 Doc()->m_partsList.begin();
					it != Doc()->m_partsList.end(); ++it)
				{
					if (it->guid == guid)
					{
						Doc()->m_partsList.erase(it);
						break;
					}
				}
			}
			break;
			case 7:
			{
				std::map<unsigned int, sCaddieInfo>::iterator it =
					Doc()->m_caddieMap.find(guid);
				if (it != Doc()->m_caddieMap.end())
				{
					Doc()->m_caddieMap.erase(it);
				}
			}
			break;
			case 16:
			{
				// 983 009caa35: mascote (mapa +0x3724) sai pelo guid; faltava desde o 645
				std::map<unsigned int, sMascotInfo>::iterator it =
					Doc()->m_mascotMap.find(guid);
				if (it != Doc()->m_mascotMap.end())
				{
					Doc()->m_mascotMap.erase(it);
				}
			}
			break;
			case 4:
			{
				for (std::map<unsigned int, sItemInfo>::iterator it =
						 Doc()->m_clubSetMap.begin();
					it != Doc()->m_clubSetMap.end(); ++it)
				{
					if (it->second.guid == guid && it->second.tid == tid)
					{
						Doc()->m_clubSetMap.erase(it);
						break;
					}
				}
			}
			break;
			case 31:
			{
				for (std::list<sCards>::iterator it = Doc()->m_cardList.begin();
					it != Doc()->m_cardList.end(); ++it)
				{
					if (it->uid == guid && it->tid == tid)
					{
						if (count < 1)
						{
							Doc()->m_cardList.erase(it);
						}
						else
						{
							it->cardCount = count;
						}
						break;
					}
				}
			}
			break;
			}
		}

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xdd), 0, 0, 0, 0, 0));
	}
	break;
	case 0xa9:
	{
		unsigned long type = packet.Decode4();
		unsigned long value = packet.Decode4();
		char msg[1024];
		switch (type)
		{
		case 0:
			sprintf(msg,
				K2L_Compatibility("< SYSTEM INFO > \307\366\300\347 \272\361\260\241 \277\303 \310\256\267\374\300\272 1/%u\300\324\264\317\264\331."),
				value);
			break;
		case 1:
			sprintf(msg,
				K2L_Compatibility("< SYSTEM INFO > \307\366\300\347 \306\316\272\270\263\312\275\272\264\302 %u%%\300\324\264\317\264\331."),
				value);
			break;
		case 2:
			sprintf(msg,
				K2L_Compatibility("< SYSTEM INFO > \307\366\300\347 \265\345\266\370\310\256\267\374\300\272 1/%u\300\324\264\317\264\331."),
				value);
			break;
		case 3:
			sprintf(msg,
				K2L_Compatibility("< SYSTEM INFO > \307\366\300\347 \276\306\300\314\305\333\265\345\267\323 \310\256\267\374\300\272 %u%%\300\324\264\317\264\331."),
				value);
			break;
		}
		if (IS_KINDOF(CLobbyTask, AfxGetTask()))
			AfxGetTask()->GetActor("Lobby")
				<< MsgObject(NULL, M983(0x3), (int)msg, 0xffff7878, 0, 0, 0);
		else
		{
			std::string str(msg);
			CChatMsg::Instance()->AddChatMsg(str, 0xffff7878, false, false);
		}
	}
	break;
	case 0xc8:
	{
		// 983 009caf51: só o total de pang (o 645 lia total + diferença e denunciava a divergência pelo pacote 0x33);
		// depois redesenha a barra do lobby (faltava: comprar status na maestria nao baixava o pang, Revisor 0x188)
		Doc()->m_myInfo.stat.i64Pang = packet.Decode8();
		if (RFLobbyDesktop::Instance983()) // 0047f920
			RFLobbyDesktop::Instance983()->Refresh983();
	}
	break;
	case 0xaa:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned short count = packet.Decode2();

		int msg; // nao inicializado no original: o default do switch do RealMyRoom reaproveita o valor anterior
		for (int i = 0; (unsigned short)i < count; i++)
		{
			sBuyItemResult result;
			packet.DecodeBuffer(&result, sizeof(sBuyItemResult));

			// 983 (009cb03b): o teste é contra o RTTI 00f030b0 = CLobbyTask (o 645 testava CShopTask e CRealMyRoomTask)
			if (IS_KINDOF(CLobbyTask, AfxGetTask()))
			{
				if (IsLocalContent((localContentType_t)0x14) &&
					result.Typeid == 0x1a000042)
				{
					sItemInfo item;
					memset(&item, 0, sizeof(sItemInfo));
					item.guid = result.guid;
					item.tid = result.Typeid;
					item.HourRemain = result.Time;
					item.ItemType = result.ItemType;
					item.Common[0] = result.Count < 1 ? 1 : result.Count;

					std::list<sItemInfo>::iterator it;
					for (it = Doc()->m_myItemList.begin();
						it != Doc()->m_myItemList.end(); ++it)
					{
						if (it->guid == result.guid)
						{
							it->Common[0] = item.Common[0];
							break;
						}
					}

					if (it == Doc()->m_myItemList.end())
					{
						item.Common[1] = packet.Decode2();
						item.Common[2] = packet.Decode2();

						// D648 (983 009cb246): a data do pacote vira segundos (00b30830) em item+0x18, nao o SYSTEMTIME do 645
						_SYSTEMTIME date;
						packet.DecodeBuffer(&date, sizeof(_SYSTEMTIME));
						S5::SysTimeToSec983(&date, (__int64*)&item.ItemDate);
						item.ItemType = 2;

						Doc()->m_myItemList.push_back(item);
					}
				}
				// 983 (009cb273): não há ramo do CRealMyRoomTask; o my room e a loja são atores ("realmyroom",
				// "shopmain") do CLobbyTask e a mensagem vai ao ator principal (virtual +0x30 = SendMsgToMainActor)
				else if (AfxGetTask()->IsCurrentActor983("realmyroom"))
				{
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
					case 8:
						msg = M983(0xcc);
						break;
					case 4:
						msg = M983(0xcf);
						break;
					case 14:
						msg = M983(0xd0);
						break;
					case 5:
					case 6:
					case 28:
						msg = M983(0xce);
						break;
					case 15:
						msg = M983(0xc9);
						break;
					case 16:
						msg = M983(0xcd);
						break;
					case 31:
						msg = M983(0xe0);
						break;
					default:
						// 983: tipo fora da tabela 009d810c (inclusive o 29) sai pelo default do despachante
						return 0;
					}

					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, msg, result.guid, result.Typeid,
						result.Time, result.ItemType, (int)&result));
					if (msg != M983(0xc9))
						AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xe1), 0, 0, 0, 0, 0));
				}
				else if (AfxGetTask()->IsCurrentActor983("shopmain"))
				{
					int shopMsg;
					switch (result.Typeid >> 26)
					{
					case 1:
						shopMsg = M983(0xb5);
						break;
					case 2:
						shopMsg = M983(0xb6);
						break;
					case 7:
						shopMsg = M983(0xb7);
						break;
					case 8:
						shopMsg = M983(0xb8);
						break;
					case 6:
						shopMsg = M983(0xb9);
						break;
					case 4:
						shopMsg = M983(0xba);
						break;
					case 5:
						shopMsg = M983(0xbb);
						break;
					case 14:
						shopMsg = M983(0xbc);
						break;
					case 15:
						shopMsg = M983(0xbd);
						break;
					case 28:
						shopMsg = M983(0xbe);
						break;
					case 16:
						shopMsg = M983(0xbf);
						break;
					case 31:
						shopMsg = M983(0x218);
						break;
					case 18:
						shopMsg = M983(0x217);
						break;
					default:
						continue;
					}

					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, shopMsg, result.guid, result.Typeid,
						result.Time, result.ItemType, (int)&result));
				}
			}
			else
			{
				switch (result.Typeid >> 26)
				{
				case 1:
				{
					sCharacterInfo info;
					memset(&info, 0, sizeof(sCharacterInfo));
					info.guid = result.guid;
					info.tid = result.Typeid;
					ItemManager()->GetDefCombo(result.Typeid, info.tidParts);

					Doc()->m_charMap.insert(
						std::map<unsigned int, sCharacterInfo>::value_type(
							info.guid, info));
				}
				break;
				case 2:
				{
					sItemInfo item;
					memset(&item, 0, sizeof(sItemInfo));
					item.guid = result.guid;
					item.tid = result.Typeid;

					if (result.ItemType == 2)
					{
						item.HourRemain = result.Time * 24;
						item.ItemType = 2;
					}

					Doc()->AddMyPartsList(item);
				}
				break;
				case 7:
				{
					sCaddieInfo info;
					memset(&info, 0, sizeof(sCaddieInfo));
					info.guid = result.guid;
					info.tid = result.Typeid;
					info.Rent_flag = 1;
					info.Remain_Date = 30;

					if (result.Typeid >= 0x1c00000c &&
						result.Typeid <= 0x1c00000e)
					{
						info.Level = result.ItemType;
						info.Exp = result.Time;
					}

					Doc()->m_caddieMap.insert(
						std::map<unsigned int, sCaddieInfo>::value_type(
							info.guid, info));
				}
				break;
				case 6:
				{
					sItemInfo item;
					memset(&item, 0, sizeof(sItemInfo));
					item.guid = result.guid;
					item.tid = result.Typeid;
					item.ItemType = result.ItemType;
					item.Common[0] = result.Count < 1 ? 1 : result.Count;

					if (result.ItemType != 0)
					{
						if (result.ItemType == 3)
							item.HourRemain = result.Time;
						else if (result.ItemType == 4)
							item.HourRemain = result.Time * 24;
						else if (result.ItemType == 5)
							item.HourRemain = result.Time * 720;
					}

					std::list<sItemInfo>::iterator it;
					for (it = Doc()->m_myItemList.begin();
						it != Doc()->m_myItemList.end(); ++it)
					{
						if (it->guid == result.guid)
						{
							it->Common[0] = item.Common[0];
							break;
						}
					}

					if (it == Doc()->m_myItemList.end())
					{
						if (IsLocalContent((localContentType_t)0x14))
						{
							if (item.tid == 0x1a000042)
							{
								item.Common[1] = packet.Decode2();
								item.Common[2] = packet.Decode2();

								item.HourRemain = result.Time;

								// D648 (983 009cb870): idem 009cb246, segundos (00b30830) em item+0x18
								_SYSTEMTIME date;
								packet.DecodeBuffer(&date, sizeof(_SYSTEMTIME));
								S5::SysTimeToSec983(&date, (__int64*)&item.ItemDate);
								item.ItemType = 2;
							}
						}

						Doc()->m_myItemList.push_back(item);
					}
				}
				break;
				case 5:
				{
					std::list<sItemInfo>::iterator it;
					for (it = Doc()->m_ballList.begin();
						it != Doc()->m_ballList.end(); ++it)
					{
						if (it->guid == result.guid)
						{
							it->Common[0] = result.Count < 1 ? 1 : result.Count;
							break;
						}
					}

					if (it == Doc()->m_ballList.end())
					{
						sItemInfo item;
						memset(&item, 0, sizeof(sItemInfo));
						item.guid = result.guid;
						item.tid = result.Typeid;
						item.ItemType = result.ItemType;
						item.Common[0] = result.Count < 1 ? 1 : result.Count;

						Doc()->m_ballList.push_back(item);
					}
				}
				break;
				case 4:
				{
					sItemInfo item;
					memset(&item, 0, sizeof(sItemInfo));
					item.guid = result.guid;
					item.tid = result.Typeid;

					Doc()->m_clubSetMap.insert(
						std::map<unsigned int, sItemInfo>::value_type(item.guid,
							item));
				}
				break;
				case 16:
				{
					sMascotInfo info;
					memset(&info, 0, sizeof(sMascotInfo));
					info.guid = result.guid;
					info.tid = result.Typeid;
					info.endDate = result.endDate;
					info.Remain_Date = result.Time;
					strcpy(info.szMsg, "PANGYA!");

					Doc()->AddMascot(&info);
				}
				break;
				case 28:
				{
					sItemInfo item;
					memset(&item, 0, sizeof(sItemInfo));
					item.guid = result.guid;
					item.tid = result.Typeid;
					item.ItemType = result.ItemType;
					item.Common[0] = result.Count < 1 ? 1 : result.Count;

					if (result.ItemType != 0)
					{
						if (result.ItemType == 3)
							item.HourRemain = result.Time;
						else if (result.ItemType == 4)
							item.HourRemain = result.Time * 24;
						else if (result.ItemType == 5)
							item.HourRemain = result.Time * 720;
					}

					std::list<sItemInfo>::iterator it;
					for (it = Doc()->m_auxPartList.begin();
						it != Doc()->m_auxPartList.end(); ++it)
					{
						if (it->guid == result.guid)
						{
							it->Common[0] = item.Common[0];
							break;
						}
					}

					if (it == Doc()->m_auxPartList.end())
					{
						Doc()->m_auxPartList.push_back(item);
					}
				}
				break;
				case 31:
				{
					unsigned long guid = result.guid;
					unsigned long tid = result.Typeid;
					int cardCount;

					if (result.Count > 0)
					{
						cardCount = result.Count;
					}
					else
					{
						cardCount = 1;
					}

					IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(tid);
					if (pCard)
					{
						sCards card;
						memset(&card, 0, sizeof(sCards));
						card.uid = guid;
						card.tid = tid;
						card.cardCount = cardCount;

						bool bFound = false;
						for (std::list<sCards>::iterator it =
								 Doc()->m_cardList.begin();
							it != Doc()->m_cardList.end(); ++it)
						{
							if (it->tid == tid)
							{
								it->cardCount += cardCount;
								bFound = true;
								break;
							}
						}

						if (!bFound)
						{
							Doc()->m_cardList.push_back(card);
						}
					}
				}
				break;
				case 14:
				{
					sItemInfo item;
					memset(&item, 0, sizeof(sItemInfo));
					item.guid = result.guid;
					item.tid = result.Typeid;
					item.ItemType = result.ItemType;
					item.HourRemain = result.Time * 24;

					Doc()->m_cutinList.push_back(item);
				}
				break;
				case 15:
					break;
				default:
					continue;
				}
			}

			if (IsAngelWing(result.Typeid))
			{
				Doc()->CheckAngelWing();
			}

			if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/))
			{
				if ((result.Typeid & 0xfc000000) == 0x8000000 &&
					(result.Typeid & 0x1800) == 0x1800)
				{
					Doc()->SetInitHaveHalloweenItem();
				}
			}
		}
	}
	break;
	case 0x95:
	{
		if (packet.Decode2() == 0x111)
		{
			switch (packet.Decode4())
			{
			case 0:
			{
				__int64 pang = packet.Decode8();
				Doc()->m_myInfo.stat.i64Pang = pang;
				if (RFLobbyDesktop::Instance983()) // 0047f920
					RFLobbyDesktop::Instance983()->Refresh983();
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\302\312\301\366\270\246 \263\262\260\345\275\300\264\317\264\331. \273\363\264\353\271\346\300\314 \264\331\300\275 \267\316\261\327\300\316\307\322 \266\247 \300\374\264\336\265\313\264\317\264\331."));
			}
			break;
			case 3:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\273\363\264\353\271\346\300\307 \302\312\301\366\307\324\300\314 \260\241\265\346\302\367\274\255 \302\312\301\366\270\246 \272\270\263\276 \274\366 \276\370\275\300\264\317\264\331."));
				break;
			case 7:
			case 10:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\300\332\275\305\277\241\260\324 \302\312\301\366\270\246 \272\270\263\276\274\366 \276\370\275\300\264\317\264\331."));
				break;
			case 12:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\272\270\300\257\306\316\300\314 \272\316\301\267\307\317\277\251 \302\312\301\366\270\246 \272\270\263\276\274\366 \276\370\275\300\264\317\264\331."));
				break;
			default:
			{
				// 983 009cbf72: janela "notify" com o texto do apelido (nao e do mensageiro, mas e o que o 983 mostra)
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
				pForm->SetMessage(K2L_Compatibility("\272\260\270\355 \272\257\260\346\277\241 \275\307\306\320\307\337\275\300\264\317\264\331."), false);
				pForm->Open(NULL, 3);
			}
			break;
			}
		}
		WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_MSN);
	}
	break;
	case 0xd5:
	{
		unsigned long value = packet.Decode4();
		if (value != 0)
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0xad), value, 0, 0, 0, 0));
	}
	break;
	case PKT645_SEM_983(0xd4):
	{
		int result = packet.Decode4();
		RbBongdariShopDlg* pDlg = (RbBongdariShopDlg*)g_pFresh->GetManager()
									  ->GetDesktop()
									  ->FindChildByName("bongdari_shop");
		if (pDlg)
			pDlg->EnableExitButton(true);
		else
			__rtti_obj = pDlg;
		switch (result)
		{
		case 3:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\300\337\270\370\265\310 \276\306\300\314\305\333 \304\332\265\345\300\324\264\317\264\331"),
				0, 0, 0, 0));
			break;
		case 2:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\261\335\276\327\300\314 \272\316\301\267\307\325\264\317\264\331",
				0, 0, 0, 0));
			break;
		case 0:
		{
			int count = packet.Decode4();
			if (count < 1 || count > 5)
				break;
			sBonusBall balls[5] = { 0 };
			for (int i = 0; i < count; i++)
				packet.DecodeBuffer(&balls[i], sizeof(sBonusBall));
			Doc()->m_myInfo.stat.i64Pang = packet.Decode8();
			Doc()->m_bonusCash = packet.Decode8();
			Doc()->RefreshAllCookie();
			if (pDlg)
			{
				pDlg->InitBongdariShop();
				pDlg->SetBonusBall(count, balls);
				pDlg->SetBongdariShop();
				pDlg->SetDataFee(0);
			}
			AfxGetTask()->GetActor("RealMyRoom")
				<< MsgObject(NULL, M983(0xe1), 0, 0, 0, 0, 0);
		}
		break;
		}
	}
	break;
	case 0xfb:
	{
		Doc()->SetBsUsableTimes(packet.Decode4());
		Doc()->SetBsUsableBonusTimes(packet.Decode4());
		RbBongdariShopDlg* pDlg = (RbBongdariShopDlg*)g_pFresh->GetManager()
									  ->GetDesktop()
									  ->FindChildByName("bongdari_shop");
		if (pDlg)
		{
			pDlg->EnableExitButton(true);
			pDlg->ShowDiscountUsableTimes();
			pDlg->UpdateUsableTimes();
		}
	}
	break;
	case 0x2f:
		switch (packet.Decode1())
		{
		case 0:
		{
			unsigned long uid = packet.Decode4();
			if (uid != MyUID())
				break;

			CMessengerInfo::Instance()->SetComplete((eConnectStatus)2);

			sUserPosition pos;
			pos.iServerGUID = Doc()->m_gameServerUID;
			unsigned char channelUid = Doc()->m_curChannel.Uid;
			if (channelUid != 0xff)
				pos.iChannelUid = channelUid;
			wsprintfA(pos.csChannelName, "%s", Doc()->m_curChannel.Name);
			unsigned short roomIdx = Doc()->m_roomInfo.roomGuid;
			if (roomIdx != 0xffff)
				pos.iRoomIdx = roomIdx;
			int roomType = Doc()->m_roomInfo.gameType;
			if (roomType != -1)
				pos.iRoomType = roomType;

			WSendPacket send(0x23);
			send.EncodeBuffer(&pos, sizeof(sUserPosition));
			send.Send(TO_MSN);

			WSendPacket send2(0x14);
			send2.Send(TO_MSN);
			CMessengerInfo::Instance()->SetComplete((eConnectStatus)3);
		}
		break;
		case 2:
		{
			WNetworkSystem* net = WNetworkSystem::Instance();
			CMessengerInfo::Instance()->Notify1(
				K2L_Compatibility("\270\336\275\305\300\372\274\255\271\366\277\241 \301\242\274\323\307\317\261\342 \300\247\307\321 \267\316\261\327\300\316\301\244\272\270\260\241 \300\337\270\370\265\307\276\356 \267\316\261\327\300\316\307\317\301\366 \270\370\307\317\277\264\275\300\264\317\264\331."));
			if (net->IsConnected(WNetworkSystem::NET_MSN))
				net->ForceShutDown(WNetworkSystem::NET_MSN);
		}
		break;
		}
		break;
	case 0x33:
		if (IsLocalContent((localContentType_t)0x61 /*983: era 0x60; medido*/))
		{
			Doc()->m_mailList.clear();
			WSendPacket send((enumClientPacket)0xe4);
			send.Send(TO_GAME);
		}
		else
		{
			Doc()->m_giftList.clear();
			WSendPacket send((enumClientPacket)0x96); // JP 983 (s63): 0x93 no KR 645
			send.Send(TO_GAME);
		}
		break;
	case 0x38:
	{
		WSendPacket send((enumClientPacket)0xe4);
		send.Send(TO_GAME);
	}
	break;
	case PKT645_SEM_983(0x38):
	{
		int count = packet.Decode4();
		for (; count > 0; count--)
		{
			GUILD_USER_MSN_LIST msn;
			packet.DecodeBuffer(&msn, sizeof(GUILD_USER_MSN_LIST));
			sFriend frnd;
			frnd.dwGuildId = msn.guildUID;
			frnd.Uid = msn.userUID;
			frnd.GuildFriend = 1;
			frnd.Gender = msn.sex;
			strcpy(frnd.NickName, msn.nickname);
			frnd.IsLogOn = msn.online;
			BuddyMGR->AddBuddy(&frnd);
		}
	}
	break;
	case PKT645_SEM_983(0x39):
	{
		GUILD_USER_MSN_LIST msn;
		packet.DecodeBuffer(&msn, sizeof(GUILD_USER_MSN_LIST));
		sFriend frnd;
		frnd.dwGuildId = msn.guildUID;
		frnd.Uid = msn.userUID;
		frnd.GuildFriend = 1;
		frnd.Gender = msn.sex;
		strcpy(frnd.NickName, msn.nickname);
		frnd.IsLogOn = msn.online;
		BuddyMGR->AddBuddy(&frnd);
		IActor* actor = GetActor(s_guildActor[1]);
		if (actor)
		{
			std::map<unsigned long, sBriefUserInfo>::iterator it;
			if (CSharedDoc::Instance()->FindUserInfoUID(msn.userUID, it) == 1)
			{
				sBriefUserInfo& info = it->second;
				char* pMark = NULL;
				actor << MsgObject(NULL, M983(0x263), 0x21, (int)&pMark, 0, 0, 0);
				strncpy(info.szEmblemName, pMark + 0x25, 11);
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
					(int)MakeStr(
						"%s \264\324\300\314 \261\346\265\345\277\241 \260\241\300\324\307\317\277\264\275\300\264\317\264\331.",
						msn.nickname),
					0, 0, 0, 0));
			}
		}
	}
	break;
	case PKT645_SEM_983(0x3a):
	{
		unsigned long uid = packet.Decode4();
		BuddyMGR->DeleteBuddy(uid);
		std::map<unsigned long, sBriefUserInfo>::iterator it;
		if (CSharedDoc::Instance()->FindUserInfoUID(uid, it) != 1)
			break;
		sBriefUserInfo& info = it->second;
		strcpy(info.szEmblemName, "guildmark");
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)MakeStr(
				"%s \264\324\300\314 \261\346\265\345\270\246 \305\273\305\360\307\317\277\264\275\300\264\317\264\331.",
				info.sNick),
			0, 0, 0, 0));
	}
	break;
	case PKT645_SEM_983(0x3b):
	{
		GUILD_INFO info;
		packet.DecodeBuffer(&info, sizeof(GUILD_INFO));
		IActor* actor = GetActor(s_guildActor[1]);
		if (actor)
			actor << MsgObject(NULL, M983(0x263), 0x16, (int)&info, 0, 0, 0);
	}
	break;
	case PKT645_SEM_983(0x3c):
	{
		GUILD_INFO info;
		packet.DecodeBuffer(&info, sizeof(GUILD_INFO));
		IActor* actor = GetActor(s_guildActor[1]);
		if (actor)
			actor << MsgObject(NULL, M983(0x263), 0x17, (int)&info, 0, 0, 0);
	}
	break;
	case 0x34:
	{
		std::string nick;
		unsigned long uid = packet.Decode4();
		nick = packet.DecodeStr();
		sFriend* pFriend = BuddyMGR->GetBuddy(uid);
		if (pFriend)
			strcpy(pFriend->NickName, nick.c_str());
	}
	break;
	case 0x39:
	{
		std::string chat = "";
		std::string msgType = packet.DecodeStr();
		std::string nick = packet.DecodeStr();
		std::string text = packet.DecodeStr();
		std::list<std::string>::iterator it =
			std::find(CSharedDoc::Instance()->m_ignoreList.begin(),
				CSharedDoc::Instance()->m_ignoreList.end(), nick);
		if (!(it != CSharedDoc::Instance()->m_ignoreList.end()))
		{
			int dummy;
			GetActor("Lobby") << MsgObject(NULL, M983(0x3f), (int)&dummy, 0, 0, 0, 0);
			chat = K2L_Compatibility("\261\346\265\345:") + nick + "> ";
			CSharedDoc::Instance()->m_chatManager.FilteringHack(text.c_str(),
				false);
			CChatMsg::Instance()->AddChatMsg(chat, 0xff5bffb0, false, true);
		}
	}
		return 1;
	case 0x30:
	{
		switch (packet.Decode2())
		{
		case 0x105:
		case 0x110:
			packet.Decode1();
			break;
		case 0x117:
		{
			int result = packet.Decode4();
			if (result == 0)
			{
				std::string nick;
				packet.DecodeStr(nick);
				unsigned long uid = packet.Decode4();
				if (IS_KINDOF(CLobbyTask, AfxGetTask()) ||
					IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
					AfxGetTask()->SendMsgToMainActor(
						MsgObject(NULL, M983(0x51), (int)nick.c_str(), uid, 0, 0, 0));
				CMessengerInfo::Instance()->SetOfflineUserNick(nick.c_str(),
					uid);
			}
			else if (result == 0xd)
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\310\270\277\370\305\273\305\360\307\321 \276\306\300\314\265\360\300\324\264\317\264\331."));
			else
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\264\353\310\255\270\355\300\314 \301\270\300\347\307\317\301\366 \276\312\275\300\264\317\264\331."));
		}
		break;
		case 0x104:
			switch (packet.Decode4())
			{
			case 0:
			{
				sFriend frnd;
				packet.DecodeBuffer(&frnd, sizeof(sFriend));
				sFriend* pFriend = BuddyMGR->GetBuddy(frnd.Uid);
				if (pFriend && pFriend->GuildFriend)
				{
					pFriend->PangyaFriend = frnd.PangyaFriend;
					pFriend->IsAccept = frnd.IsAccept;
					pFriend->IsAgree = frnd.IsAgree;
				}
				else
					BuddyMGR->AddBuddy(&frnd);
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\304\243\261\270\265\356\267\317\300\273 \277\344\303\273\307\317\277\264\275\300\264\317\264\331."));
				CMessengerInfo::Instance()->Refresh();
			}
			break;
			case 11:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\300\314\271\314 \304\243\261\270\267\316 \265\356\267\317\265\307\276\356 \300\326\275\300\264\317\264\331."));
				break;
			case 2:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\264\365 \300\314\273\363 \304\243\261\270\265\356\267\317\300\273 \307\322 \274\366 \276\370\275\300\264\317\264\331."));
				break;
			case 3:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\273\363\264\353\271\346 \304\243\261\270\275\275\267\324\300\314 \270\360\265\316 \302\367 \300\326\275\300\264\317\264\331."));
				break;
			case 6:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\300\332\261\342 \300\332\275\305\300\273 \304\243\261\270\267\316 \265\356\267\317\307\322 \274\366 \276\370\275\300\264\317\264\331."));
				break;
			case 7:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\264\353\310\255\270\355\300\314 \301\270\300\347\307\317\301\366 \276\312\275\300\264\317\264\331."));
				break;
			case 8:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\264\353\310\255\270\355\300\314 \301\270\300\347\307\317\301\366 \276\312\275\300\264\317\264\331.."));
				break;
			case 4:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\304\243\261\270 \307\343\260\241 \264\353\261\342 \273\363\305\302\300\324\264\317\264\331."));
				break;
			case 5:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\304\243\261\270\303\337\260\241 \310\304 \275\302\300\316 \260\305\300\375 \265\307\276\372\275\300\264\317\264\331."));
				break;
			case 1:
			case 9:
			case 10:
			default:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\304\243\261\270 \265\356\267\317\300\273 \275\307\306\320\307\317\277\264\275\300\264\317\264\331."));
				break;
			}
			break;
		case 0x106:
		{
			sFriend frnd;
			packet.DecodeBuffer(&frnd, sizeof(sFriend));
			sFriend* pFriend = BuddyMGR->GetBuddy(frnd.Uid);
			if (pFriend && pFriend->GuildFriend)
			{
				pFriend->PangyaFriend = frnd.PangyaFriend;
				pFriend->IsAccept = frnd.IsAccept;
				pFriend->IsAgree = frnd.IsAgree;
			}
			else
			{
				// 983 009c7531: com a opção de +0x180 do COption (004924f0) ligada, o amigo novo já entra aceito e a
				// resposta (pacote 0x19 com o uid) vai ao mensageiro; sem o aviso de "pediu amizade"
				BuddyMGR->AddBuddy(&frnd);
				if (COption::Instance()->gIsFriendConfirmNeeded())
				{
					pFriend = BuddyMGR->GetBuddy(frnd.Uid);
					pFriend->IsAccept = 1;
					pFriend->IsAgree = 1;
					strcpy(pFriend->szAlias, "Friend");
					CMessengerInfo::Instance()->Notify2(MakeStr(
						K2L_Compatibility("%s\264\324\300\314 \304\243\261\270\260\241 \265\307\276\372\275\300\264\317\264\331"),
						pFriend->NickName));
					CMessengerInfo::Instance()->Refresh();
					WSendPacket send(0x19);
					send.Encode4(pFriend->Uid);
					send.Send(TO_MSN);
					break;
				}
			}
			CMessengerInfo::Instance()->Refresh();
			std::string msg(K2L_Compatibility("\276\313\270\262 : "));
			msg += frnd.NickName;
			msg +=
				K2L_Compatibility(" \264\324\300\314 \304\243\261\270\277\344\303\273\300\273 \307\337\275\300\264\317\264\331.");
			CMessengerInfo::Instance()->Notify2(
				K2L_Compatibility(msg.c_str()));
		}
		break;
		case 0x113:
		{
			std::string text;
			std::string nick;
			unsigned long uid = packet.Decode4();
			nick = packet.DecodeStr();
			text = packet.DecodeStr();
			if (IsLocalContent((localContentType_t)0x7a /*983: medido em 009c7679, 009c7ac8, 009c7c36*/) &&
				packet.Decode1() == 1)
			{
				std::list<std::string>::iterator it =
					std::find(Doc()->m_ignoreList.begin(),
						Doc()->m_ignoreList.end(), nick);
				if (it != Doc()->m_ignoreList.end())
					break;
				text = K2L_Compatibility("\261\346\265\345:") + nick + "> " + "\010" + text;
				Doc()->m_chatManager.FilteringHack(text.c_str(), false);
				CChatMsg::Instance()->AddChatMsg(text, 0xff5bffb0, false,
					false);
				break;
			}
			sFriend* pFriend = BuddyMGR->GetBuddy(uid);
			if (pFriend)
			{
				if (!pFriend->IsLogOn)
				{
					pFriend->IsLogOn = 1;
					CMessengerInfo::Instance()->Refresh();
				}
				CMessengerInfo::Instance()->RecvChat(pFriend, text);
			}
		}
		break;
		case 0x114:
			if (packet.Decode1() == 3)
			{
				sFriend* pFriend = BuddyMGR->GetBuddy(packet.Decode4());
				if (pFriend)
				{
					CMessengerInfo::Instance()->RecvChat(pFriend,
						K2L_Compatibility("\270\336\275\303\301\366\270\246 \300\374\264\336\307\322 \274\366 \276\370\275\300\264\317\264\331."));
					pFriend->IsLogOn = 0;
					CMessengerInfo::Instance()->Refresh();
				}
			}
			break;
		case 0x10b:
		{
			if (packet.Decode4() != 0)
			{
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\304\243\261\270\273\350\301\246\277\241 \275\307\306\320\307\317\277\264\275\300\264\317\264\331."));
				break;
			}
			unsigned long uid = packet.Decode4();
			sFriend* pFriend = BuddyMGR->GetBuddy(uid);
			if (pFriend)
			{
				if (pFriend->GuildFriend)
					pFriend->PangyaFriend = 0;
				else
					BuddyMGR->DeleteBuddy(uid);
			}
			CMessengerInfo::Instance()->Refresh();
		}
		break;
		case 0x10c:
		{
			if (packet.Decode4() != 0)
			{
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\304\243\261\270\272\355\267\260 \274\263\301\244\300\273 \275\307\306\320\307\317\277\264\275\300\264\317\264\331."));
				break;
			}
			sFriend* pFriend = BuddyMGR->GetBuddy(packet.Decode4());
			if (pFriend)
				pFriend->IsBlock = 1;
			CMessengerInfo::Instance()->Refresh();
		}
		break;
		case 0x10d:
		{
			if (packet.Decode4() != 0)
			{
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\272\355\267\260 \307\330\301\366\270\246 \275\307\306\320\307\317\277\264\275\300\264\317\264\331."));
				break;
			}
			sFriend* pFriend = BuddyMGR->GetBuddy(packet.Decode4());
			if (pFriend)
			{
				pFriend->IsBlock = 0;
				pFriend->IsBlocked = 0;
			}
			CMessengerInfo::Instance()->Refresh();
		}
		break;
		case 0x119:
			switch (packet.Decode4())
			{
			case 9:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\304\243\261\270 \265\356\267\317\265\307\276\356\300\326\301\366 \276\312\275\300\264\317\264\331."));
				break;
			case 1:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\272\260\270\355\272\257\260\346\277\241 \275\307\306\320\307\337\275\300\264\317\264\331."));
				break;
			case 0:
			{
				unsigned long uid = packet.Decode4();
				std::string alias = packet.DecodeStr();
				Doc()->m_chatManager.FilteringHack(alias.c_str(), false);
				sFriend* pFriend = BuddyMGR->GetBuddy(uid);
				if (pFriend)
					strncpy(pFriend->szAlias, alias.c_str(), 10);
			}
			break;
			}
			break;
		case 0x10e:
			if (!IsLocalContent((localContentType_t)0x7a /*983: medido em 009c7679, 009c7ac8, 009c7c36*/))
			{
				sFriend frnd;
				packet.DecodeBuffer(&frnd, sizeof(sFriend));
				sFriend* pFriend = BuddyMGR->GetBuddy(frnd.Uid);
				if (!pFriend)
					break;
				if (frnd.GuildFriend)
				{
					if (frnd.PangyaFriend)
					{
						if (pFriend->IsBlocked)
							break;
						pFriend->PangyaFriend = 1;
						pFriend->GuildFriend = 1;
						pFriend->IsLogOn = 1;
					}
					else
						BuddyMGR->AddBuddy(&frnd);
					CMessengerInfo::Instance()->Notify2(MakeStr(
						K2L_Compatibility("%s\264\324\300\314 \267\316\261\327\300\316\307\317\274\314\275\300\264\317\264\331"),
						frnd.NickName));
				}
				else
				{
					if (pFriend->IsBlocked || !pFriend->IsAccept)
						break;
					pFriend->IsLogOn = 1;
					CMessengerInfo::Instance()->Notify2(MakeStr(
						K2L_Compatibility("%s\264\324\300\314 \267\316\261\327\300\316\307\317\274\314\275\300\264\317\264\331"),
						pFriend->NickName));
				}
				CMessengerInfo::Instance()->Refresh();
			}
			else
			{
				sFriend* pFriend = BuddyMGR->GetBuddy(packet.Decode4());
				if (!pFriend)
					break;
				if (pFriend->GuildFriend)
				{
					if (pFriend->PangyaFriend)
					{
						if (pFriend->IsBlocked)
							break;
						pFriend->IsLogOn = 1;
					}
				}
				else
				{
					pFriend->IsLogOn = 1;
					if (pFriend->IsBlocked || !pFriend->IsAccept)
						break;
				}
				CMessengerInfo::Instance()->Notify2(MakeStr(
					K2L_Compatibility("%s\264\324\300\314 \267\316\261\327\300\316\307\317\274\314\275\300\264\317\264\331"),
					pFriend->NickName));
				CMessengerInfo::Instance()->Refresh();
			}
			break;
		case 0x10f:
		{
			unsigned long uid = packet.Decode4();
			sFriend* pFriend = BuddyMGR->GetBuddy(uid);
			if (pFriend)
			{
				if (!IsLocalContent((localContentType_t)0x7a /*983: medido em 009c7679, 009c7ac8, 009c7c36*/) &&
					pFriend->GuildFriend && !pFriend->PangyaFriend)
				{
					BuddyMGR->DeleteBuddy(uid);
					CMessengerInfo::Instance()->Refresh();
					break;
				}
				pFriend->IsLogOn = 0;
			}
			CMessengerInfo::Instance()->Refresh();
		}
		break;
		case 0x102:
		{
			// 983 009c7c7a: cada amigo que chega ainda não aceito é aceito na hora se a opção de +0x180 do COption (004924f0)
			// estiver ligada; no fim, o meu estado vai ao mensageiro (pacote 0x1d)
			if (packet.Decode1() == 1)
				BuddyMGR->ClearBuddy();
			packet.Decode2();
			unsigned short count = packet.Decode2();
			sFriend frnd;
			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&frnd, sizeof(sFriend));
				if (frnd.Uid != Doc()->m_myInfo.info.dwUID)
				{
					BuddyMGR->AddBuddy(&frnd);
					sFriend* pFriend = BuddyMGR->GetBuddy(frnd.Uid);
					if (COption::Instance()->gIsFriendConfirmNeeded() &&
						!pFriend->IsAccept)
					{
						pFriend->IsAccept = 1;
						pFriend->IsAgree = 1;
						strcpy(pFriend->szAlias, "Friend");
					}
				}
			}
			WSendPacket send(0x1d);
			send.Encode1((unsigned char)CMessengerInfo::Instance()->GetMyStatus());
			send.Send(TO_MSN);
			CMessengerInfo::Instance()->SetComplete((eConnectStatus)4);
			CMessengerInfo::Instance()->Refresh();
		}
		break;
		case 0x109:
		{
			int result = packet.Decode4();
			if (result == 9)
			{
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\304\243\261\270 \265\356\267\317\265\307\276\356\300\326\301\366 \276\312\275\300\264\317\264\331."));
				break;
			}
			if (result != 0)
			{
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\304\243\261\270 \307\343\277\353\277\241 \275\307\306\320\307\317\277\264\275\300\264\317\264\331."));
				break;
			}
			sFriend* pFriend = BuddyMGR->GetBuddy(packet.Decode4());
			if (!pFriend)
				break;
			pFriend->IsAccept = 1;
			pFriend->IsAgree = 1;
			strcpy(pFriend->szAlias, "Friend");
			CMessengerInfo::Instance()->Notify2(MakeStr(
				K2L_Compatibility("%s\264\324\300\314 \304\243\261\270\260\241 \265\307\276\372\275\300\264\317\264\331"),
				pFriend->NickName));
			CMessengerInfo::Instance()->Refresh();
		}
		break;
		case 0x10a:
		{
			std::string unused;
			if (packet.Decode4() == 0)
			{
				sFriend* pFriend = BuddyMGR->GetBuddy(packet.Decode4());
				if (pFriend)
				{
					pFriend->IsLogOn = 1;
					pFriend->IsAccept = 1;
					pFriend->Server = 0;
					strcpy(pFriend->szAlias, "Friend");
					CMessengerInfo::Instance()->Notify2(MakeStr(
						K2L_Compatibility("%s\264\324\300\314 \304\243\261\270\260\241 \265\307\276\372\275\300\264\317\264\331"),
						pFriend->NickName));
				}
				CMessengerInfo::Instance()->Refresh();
			}
		}
		break;
		case 0x103:
		{
			Doc()->m_noteList.clear();
			unsigned char count = packet.Decode1();
			if (count > 0)
			{
				int n = count;
				do
				{
					sNoteInfo note;
					packet.DecodeBuffer(&note, sizeof(sNoteInfo));
					Doc()->m_chatManager.FilteringHack(note.sNote, false);
					Doc()->m_noteList.push_back(note);
				} while (--n);
			}
			CMessengerInfo::Instance()->Notify1(
				K2L_Compatibility("\302\312\301\366\260\241 \265\265\302\370\307\337\275\300\264\317\264\331"));
		}
		break;
		case 0x112:
			switch (packet.Decode1())
			{
			case 2:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\273\363\264\353\271\346\300\307 \302\312\301\366\307\324\300\314 \260\241\265\346\302\367\274\255 \302\312\301\366\270\246 \272\270\263\276 \274\366 \276\370\275\300\264\317\264\331."));
				break;
			case 1:
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\302\312\301\366 \272\270\263\273\261\342\277\241 \275\307\306\320\307\337\275\300\264\317\264\331."));
				break;
			case 0:
				packet.Decode1();
				Doc()->m_myInfo.stat.i64Pang -= 10;
				if (RFLobbyDesktop::Instance983()) // 0047f920
					RFLobbyDesktop::Instance983()->Refresh983();
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\302\312\301\366\270\246 \263\262\260\345\275\300\264\317\264\331. \273\363\264\353\271\346\300\314 \264\331\300\275 \267\316\261\327\300\316\307\322 \266\247 \300\374\264\336\265\313\264\317\264\331."));
				break;
			}
			break;
		case 0x118:
		{
			if (WNetworkSystem::Instance()->IsConnected(
					WNetworkSystem::NET_MSN))
				WNetworkSystem::Instance()->ForceShutDown(
					WNetworkSystem::NET_MSN);
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetMessage(
				K2L_Compatibility("\264\331\270\245 \273\347\277\353\300\332\260\241 \307\366\300\347 \260\350\301\244\300\270\267\316 \301\242\274\323\307\317\277\264\275\300\264\317\264\331."),
				false);
			pForm->Open((FRESH_PFN_RESULT)&CTask::OnOverlappedDlg, 3);
		}
		break;
		case 0x123:
		{
			packet.Decode4();
			unsigned long uid = packet.Decode4();
			sUserPosition pos;
			packet.DecodeBuffer(&pos, sizeof(sUserPosition));
			sFriend* pFriend = BuddyMGR->GetBuddy(uid);
			if (pFriend)
				pFriend->userPosition = pos;
		}
		break;
		case 0x115:
		{
			// 983 009c80f9: um bloco de 0x54 (uid, estado, 1 byte, sUserPosition) no lugar dos dois Decode4 do 645;
			// o servidor em que o amigo está vem junto
			sFriendStateNotify983 st;
			packet.DecodeBuffer(&st, sizeof(sFriendStateNotify983));
			sFriend* pFriend = BuddyMGR->GetBuddy(st.uid);
			if (pFriend)
			{
				pFriend->Guid = st.uid;
				pFriend->userPosition.iServerGUID = st.pos.iServerGUID;
				switch (st.state)
				{
				case 0:
					pFriend->IsLogOn = 1;
					pFriend->IsPlay = 1;
					break;
				case 1:
					pFriend->IsLogOn = 1;
					pFriend->IsDive = 1;
					break;
				case 3:
					pFriend->IsLogOn = 1;
					pFriend->IsBusy = 1;
					break;
				case 4:
					pFriend->IsPlay = 0;
					pFriend->IsDive = 0;
					pFriend->IsBusy = 0;
					if (!pFriend->IsLogOn)
						CMessengerInfo::Instance()->Notify2(MakeStr(
							K2L_Compatibility("%s\264\324\300\314 \267\316\261\327\300\316\307\317\274\314\275\300\264\317\264\331"),
							pFriend->NickName));
					pFriend->IsLogOn = 1;
					break;
				case 5:
					pFriend->IsPlay = 0;
					pFriend->IsDive = 0;
					break;
				}
			}
			CMessengerInfo::Instance()->Refresh();
		}
		break;
		case 0x122:
		{
			WSendPacket send(0x16);
			send.Send(TO_MSN);
			CMessengerInfo::Instance()->Close(true);
			if (WNetworkSystem::Instance()->IsConnected(
					WNetworkSystem::NET_MSN))
				WNetworkSystem::Instance()->ForceShutDown(
					WNetworkSystem::NET_MSN);
			CMessengerInfo::Instance()->Notify2(
				K2L_Compatibility("\270\336\275\305\300\372\274\255\271\366\277\241 \300\337\270\370\265\310 \300\257\300\372 \301\244\272\270\260\241 \300\374\264\336\265\307\276\372\275\300\264\317\264\331. \300\347 \301\242\274\323\307\330\301\326\275\303\261\342 \271\331\266\370\264\317\264\331"));
		}
		break;
		case 0x126:
		{
			// 983 009c826c (novo): resposta ao convite de guilda que eu mandei
			int result = packet.Decode4();
			if (result == 1)
			{
				CMessengerInfo::Instance()->Notify1(
					K2L_Compatibility("\261\346\265\345\303\312\264\353\270\246 \274\272\260\370\300\373\300\270\267\316 \301\370\307\340\307\317\277\264\275\300\264\317\264\331."));
				// 983: CMessengerInfo 009b71c0 devolve m_pMessengerDlg->[+0x18c] (janela sem nome no nosso cabeçalho) e o
				// método 008fa3f0 dela só manda o pacote 0xa5 (vazio) ao servidor de jogo
				FrMessengerDlg* pDlg = CMessengerInfo::Instance()->GetMessengerDlg();
				if (pDlg && MSNGUILDMANAGE983(pDlg))
				{
					WSendPacket send((enumClientPacket)0xa5);
					send.Send(TO_GAME);
				}
			}
			else
			{
				// 983: função à parte 009b9470(result) - texto do erro pelo número (3 a 11; 4 e o resto = "falhou")
				const char* text = "\275\307\306\320\307\317\277\264\275\300\264\317\264\331.";
				switch (result)
				{
				case 3:
					text = "\307\330\264\347 \300\257\300\372\260\241 \301\270\300\347\307\317\301\366 \276\312\275\300\264\317\264\331.";
					break;
				case 5:
					text = "\307\330\264\347 \300\257\300\372\264\302 \300\314\271\314 \264\331\270\245 \261\346\265\345\277\241 \260\241\300\324\265\307\276\356 \300\326\275\300\264\317\264\331.";
					break;
				case 6:
					text = "\300\314\271\314 \303\312\264\353\265\310 \300\257\300\372\300\324\264\317\264\331.";
					break;
				case 7:
					text = "\261\346\265\345 \260\374\270\256\300\332\270\270 \261\346\265\345\303\312\264\353\270\246 \307\322\274\366 \300\326\275\300\264\317\264\331.";
					break;
				case 8:
					text = "\303\326\264\353 \261\346\265\345\303\312\264\353 \300\316\277\370\274\366\264\302 10\270\355 \300\324\264\317\264\331.";
					break;
				case 9:
					text = "\303\326\264\353 5\260\263\300\307 \261\346\265\345\270\270 \303\312\264\353\307\322\274\366 \300\326\275\300\264\317\264\331.";
					break;
				case 10:
					text = "\307\330\264\347 \261\346\265\345\264\302 \301\270\300\347\307\317\301\366 \276\312\275\300\264\317\264\331.";
					break;
				case 11:
					text = "\261\346\265\345 \260\241\300\324 \277\344\303\273\300\273 \307\317\301\366 \276\312\276\322\275\300\264\317\264\331.";
					break;
				}
				CMessengerInfo::Instance()->Notify1(K2L_Compatibility(text));
			}
		}
		break;
		case 0x127:
		{
			// 983 009c82c4 (novo): convite de guilda recebido - entra no vetor de CSharedDoc+0x10ff8 (elemento de 0x2c) e avisa
			std::string name;
			unsigned long first = packet.Decode4();
			unsigned long second = packet.Decode4();
			packet.DecodeStr(name);
			sGuildInvite983 invite;
			invite.dw4 = first;
			invite.dw0 = second;
			strcpy(invite.szName, name.c_str());
			// o cabeçalho (frente A) ainda declara o vetor como std::vector<int> (tipo provisório; pedido C -> A)
			C_ASSERT_VC10(offsetof(CSharedDoc, m_guildInvites983) == 0x10ff8);
			Doc()->m_guildInvites983.push_back(invite);
			CMessengerInfo::Instance()->Notify2(MakeStr(
				K2L_Compatibility("%s\261\346\265\345\277\241 \303\312\264\353 \271\336\300\270\274\314\275\300\264\317\264\331."),
				invite.szName));
		}
		break;
		case 0x128:
		{
			// 983 009c838e (novo): se é a minha guilda e os 2 bits baixos de info+0x90 valem 1 ou 2, pede de novo a lista
			// (pacote 0xa5, vazio, ao servidor de jogo).
			unsigned long guildId = packet.Decode4();
			C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.info.dwGuildId) == 0x82a);
			if (Doc()->m_myInfo.info.dwGuildId == guildId)
			{
				unsigned char grade =
					Doc()->m_myInfo.info.guildGrade;
				if (grade == 1 || grade == 2)
				{
					WSendPacket send((enumClientPacket)0xa5);
					send.Send(TO_GAME);
				}
			}
		}
		break;
		}
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x48), 0, 0, 0, 0, 0));
	}
	break;
	case 0x9a:
	{
		int identity = packet.Decode4();
		Doc()->SetIdentity(identity);
	}
		if (IsPcBang())
			Doc()->AccumulateIdentity(0x80);
		if (IS_KINDOF(CGolfTask, AfxGetTask()))
		{
			unsigned long myIdentity = Doc()->m_myInfo.info.dwIdentity;
			Doc()->m_userInfo[MyIndex()].info.dwIdentity = myIdentity;
		}
		break;
	case 0xb1:
	{
		unsigned char count = packet.Decode1();
		std::list<sItemInfo> items;
		for (unsigned char i = 0; i < count; i++)
		{
			sItemInfo info;
			packet.DecodeBuffer(&info, sizeof(sItemInfo));
			items.push_back(info);
		}
		for (std::list<sItemInfo>::iterator it = items.begin();
			it != items.end(); ++it)
		{
			bool bFound = false;
			for (std::list<sItemInfo>::iterator found =
					 Doc()->m_myItemList.begin();
				found != Doc()->m_myItemList.end(); ++found)
			{
				if (found->tid == it->tid)
				{
					found->Common[0] = it->Common[0];
					bFound = true;
					break;
				}
			}
			if (!bFound)
				Doc()->m_myItemList.push_back(*it);
		}
		if (CMessengerInfo::Instance()->GetMyStatus() != 3 /*983: +0xb0; o 645 lia +0x8c cru*/)
			CMessengerInfo::Instance()->SetMyStatus((MSN::eStatusType)4);
	}
	break;
	case 0xb4:
	{
		unsigned char type = packet.Decode1();
		packet.Decode1();
		unsigned char count = packet.Decode1();
		std::list<sSpecialTrophy>* list;
		if (type != 0)
		{
			if (type <= 3 || type > 5)
				break;
			list = &Doc()->m_specialTrophyList;
		}
		else
			list = &Doc()->m_oldSpecialTrophyList;
		if (list == NULL)
			break;
		list->clear(); // 983 009cc2a3: esvazia a lista antes de ler os registros (0x612550)
		for (; count > 0; count--)
		{
			sSpecialTrophy trophy;
			packet.DecodeBuffer(&trophy, sizeof(sSpecialTrophy));
			list->push_back(trophy);
		}
	}
	break;
	case 0x169: // JP 983 (s63): 0x16f no KR 645; caso 009cc2e6 (D1 + DB 0x4e)
	{
		unsigned char type = packet.Decode1();
		sTrophyStatistics* dest;
		if (type != 0)
		{
			if (type <= 3 || type > 5)
				break;
			dest = &Doc()->m_myInfo.trophy;
		}
		else
			dest = &Doc()->m_oldSeasonTrophy;
		if (dest == NULL)
			break;
		packet.DecodeBuffer(dest, sizeof(sTrophyStatistics));
	}
	break;
	case 0x16a: // PKT C39: so do 983 (009d1ea5): tabela de 100 rand() do CPowerGauge com a semente do servidor
	{
		std::vector<int> unused983; // o 983 monta e reserva um vector<int> que nao usa
		unused983.reserve(100);
		int saved = rand();
		srand(packet.Decode4());
		CPowerGauge::Instance()->ResetRandTable983();
		srand(saved);
	}
	break;
	case 0xd8:
	{
		packet.Decode1();
		unsigned char count = packet.Decode1();
		for (; count > 0; count--)
		{
			sGuildTrophy trophy;
			packet.DecodeBuffer(&trophy, sizeof(trophy));
			Doc()->m_guildTrophyList.push_back(trophy);
		}
	}
	break;
	case 0xba:
	{
		_SYSTEMTIME time;
		packet.DecodeBuffer(&time, sizeof(time));
		Doc()->SetServerTime(time);
		CGrandPrixClock::Instance()->Sync983(); // 983 009cc399
	}
	break;
	case 0xc9:
	{
		std::string nick = packet.DecodeStr();
		std::string msg = packet.DecodeStr();
		Doc()->m_chatManager.FilteringHack(msg.c_str(), false);
		COneLineBoard::Instance()->AddOnelineMsg(nick, msg);
	}
	break;
	case 0xcb:
	{
		if (packet.Decode4() != 0)
		{
		case 0x94:
			Doc()->m_bonusCash = packet.Decode8();
			Doc()->RefreshAllCookie();
		}
		else
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\272\270\300\257\307\317\260\355 \300\326\264\302 \304\355\305\260\260\241 \272\316\301\267\307\325\264\317\264\331."),
				0, 0, 0, 0));
		}
	}
	break;
	case 0xca:
	{
		int a = packet.Decode2();
		int b = packet.Decode4();
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x5d), a, b, 0, 0, 0));
	}
	break;
	case 0xce:
		if (packet.Decode1() != 0)
		{
			float rate = Doc()->m_pangRate;
			Doc()->m_pangRate = rate + rate;
			unsigned short num = packet.Decode2();
			Doc()->m_acquireItemList[0].clear();
			int count = num;
			for (; count > 0; count--)
			{
				unsigned long itemId = packet.Decode4();
				Doc()->m_acquireItemList[0].push_back(itemId);
			}
		}
		break;
	case 0xfa:
	{
		for (int i = 0; i < 4; i++)
			Doc()->m_acquireItemList[i].clear();

		int count = packet.Decode2();
		for (; count > 0; count--)
		{
			unsigned long guid = packet.Decode4();
			unsigned char bMine = packet.Decode1();
			unsigned short num = packet.Decode2();
			if (guid == RbMyGuid(false) && bMine != 0)
			{
				float rate = Doc()->m_pangRate;
				Doc()->m_pangRate = rate + rate;
			}

			int idx = -1;
			for (int i = 0; i < 4; i++)
			{
				if (guid == Doc()->m_userInfo[i].info.dwGuid)
				{
					idx = i;
					break;
				}
			}

			int itemCount = num;
			for (; itemCount > 0; itemCount--)
			{
				unsigned long itemId = packet.Decode4();
				if (idx != -1 && itemId != 0)
					Doc()->m_acquireItemList[idx].push_back(itemId);
			}
		}
	}
	break;
	case 0x1a9: // JP 983 (s63): 0x1ae no KR 645; caso 009d29e6 (D1 D4; faixa -5)
		if (packet.Decode1())
		{
			unsigned long uid = packet.Decode4();
			CTaskManager::Instance()->SetWork(new CWorkAliveCheck(uid));
		}
		else
			CTaskManager::Instance()->ReleaseWork();
		break;
	case 0xd7:
		break;
	case 0xd1:
		Doc()->m_bNewMapEvent = 1;
		Doc()->m_newMapEventMask = packet.Decode1();
		if (IS_KINDOF(CLobbyTask, AfxGetTask()))
			AfxGetTask()->GetActor("Lobby")
				<< MsgObject(NULL, M983(0xb2), 0, 0, 0, 0, 0);
		break;
	case 0xd2:
		Doc()->m_bBingoEvent = 1;
		if (IS_KINDOF(CLobbyTask, AfxGetTask()))
			AfxGetTask()->GetActor("Lobby")
				<< MsgObject(NULL, M983(0xb3), 0, 0, 0, 0, 0);
		break;
	case 0xda:
	{
		// 983 009cc6a9: só os 8 bytes de m_controlServerService (+0x10d28). O 645 lia 4 bytes (+0x10d24) e
		// avisava o RealMyRoom (mensagem 0x22f do 645); nenhum dos dois está neste caso do 983 (pedido C -> A)
		Doc()->m_controlServerService = packet.Decode8();
	}
	break;
	case 0xdd:
	case 0x1fd: // C37: o 983 trata 0x1fd no mesmo corpo do 0xdd (009d2a49)
	{
		// PKT (frente C, lote 13): forma do 983 (009d2a49) - mensagem 2 ao ator principal antes de tudo; erro = mensagem
		// 0x27 com o codigo (os dois textos do 645 nao existem neste caso do 983); janela procurada depois das bolas;
		// no fim, aviso 0xe1 ao "Lobby" se a tarefa for CLobbyTask (RTTI 00f030b0; o 645 avisava o "RealMyRoom")
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		int result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
#if 0 // 645
		if (result != 0)
		{
			if (result != 0x1b)
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
					(int)"\275\272\305\251\267\241\304\241 \300\314\277\353 \277\300\267\371\260\241 \300\326\275\300\264\317\264\331.",
					0, 0, 0, 0));
			else
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
					(int)"\275\272\305\251\267\241\304\241\270\246 \300\314\277\353\307\317\275\303\264\302\265\245 \272\322\306\355\300\273 \265\345\267\301 \301\313\274\333\307\325\264\317\264\331. \300\347\267\316\261\327\300\316\307\317\275\303\270\351 \301\244\273\363\300\373\300\270\267\316 \300\314\277\353\307\317\275\307 \274\366 \300\326\275\300\264\317\264\331.",
					0, 0, 0, 0));
			break;
		}
#endif
		int count = packet.Decode4();
		sBonusBall balls[11];
		unsigned long* raw = (unsigned long*)balls;
		raw[0] = 0;
		memset(raw + 1, 0, 0xd8);
		for (int i = 0; i < count; i++)
			packet.DecodeBuffer(&balls[i], 0x14);
		RbScratchDlg* pDlg = (RbScratchDlg*)g_pFresh->GetManager()
								 ->GetDesktop()
								 ->FindChildByName("scratchdlg");
		if (pDlg == NULL)
		{
			__rtti_obj = pDlg;
			break;
		}
		pDlg->EnableNextPageButton(true);
		pDlg->SetScratchItem(count, balls);
		pDlg->InitChangeScratchCardNumber(0);
		if (IS_KINDOF(CLobbyTask, AfxGetTask()))
			AfxPostMsg(NULL, "Lobby", M983(0xe1), 0, 0, 0, 0);
	}
		return 1;
	case 0xde:
	{
		int result = packet.Decode4();
		RbScratchDlg* pDlg = (RbScratchDlg*)g_pFresh->GetManager()
								 ->GetDesktop()
								 ->FindChildByName("scratchdlg");
		if (pDlg)
			pDlg->EnableScratchSerialNumber(true);
		switch (result)
		{
		case 0:
		{
			int code = packet.Decode4();
			unsigned long number = packet.Decode4();
			if (pDlg)
			{
				pDlg->ResponseScratchSerialNumber(code, number);
				if (code == 0)
					pDlg->InitChangeScratchCardNumber(1);
			}
		}
		break;
		case 1:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\300\314\271\314 \273\347\277\353\265\310 \275\303\270\256\276\363 \271\370\310\243\300\324\264\317\264\331."),
				0, 0, 0, 0));
			break;
		case 2:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\301\270\300\347\307\317\301\366 \276\312\264\302 \275\303\270\256\276\363 \271\370\310\243 \300\324\264\317\264\331."),
				0, 0, 0, 0));
			break;
		case 3:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\300\257\310\277\261\342\260\243\300\314 \301\366\263\255 \275\303\270\256\276\363 \271\370\310\243\300\324\264\317\264\331."),
				0, 0, 0, 0));
			break;
		case 4:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\303\326\264\353 \301\366\261\336 \310\275\274\366\270\246 \303\312\260\372\307\317\277\264\275\300\264\317\264\331."),
				0, 0, 0, 0));
			break;
		default:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\277\303\271\331\270\245 \304\355\306\371 \271\370\310\243\260\241 \276\306\264\325\264\317\264\331."),
				0, 0, 0, 0));
			break;
		}
	}
	break;
	case 0xdf:
	{
		sItemInfo info;
		packet.DecodeBuffer(&info, sizeof(sItemInfo));
		if (info.guid == 0 || info.tid == 0 || info.Common[0] <= 0)
			break;

		std::list<sItemInfo>& itemList = Doc()->m_myItemList;
		std::list<sItemInfo>::iterator it;
		for (it = itemList.begin(); it != itemList.end(); ++it)
		{
			if (it->guid == info.guid && it->tid == info.tid)
			{
				it->Common[0] = info.Common[0];
				break;
			}
		}
		if (it == itemList.end())
			itemList.push_back(info);
	}
	break;
	case 0xe0:
		Doc()->m_packetNickname = packet.DecodeStr();
		break;
	case PKT645_SEM_983(0x16d):
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)"\306\316\276\337\270\266\304\317 \271\327 \301\370\277\255\264\353 \261\342\264\311\260\374\267\303 \272\316\272\320 \301\241\260\313\301\337\300\324\264\317\264\331.",
			0, 0, 0, 0));
		break;
	case PKT645_SEM_983(0x164):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (IsLocalContent((localContentType_t)0x67 /*983: era 0x66; interpolado*/))
		{
			int type = (unsigned short)packet.Decode2();
			unsigned char result = packet.Decode1();
			unsigned char isSet = packet.Decode1();
			if (type == 1)
			{
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x22f), 0, 0, 0, 0, 0));
				if (isSet)
				{
					if (result == 1)
						AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
							(int)"\301\370\277\255\264\353 \271\350\304\241 \274\272\260\370",
							0, 0, 0, 0));
					else
						AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
							(int)"\307\366\300\347 \301\370\277\255\264\353\260\241 \301\246\260\305\265\310 \273\363\305\302\267\316 \300\372\300\345\265\307\276\372\275\300\264\317\264\331. \301\370\277\255\264\353\277\241 \265\356\267\317\307\321 \276\306\300\314\305\333\300\314 \306\316\276\337 \270\266\304\317\277\241 \265\356\267\317\265\307\301\366 \276\312\275\300\264\317\264\331.",
							0, 0, 0, 0));
				}
				else
				{
					if (result == 1)
						AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
							(int)"\307\366\300\347 \301\370\277\255\264\353\260\241 \271\350\304\241\265\310 \273\363\305\302\267\316 \300\372\300\345\265\307\276\372\275\300\264\317\264\331. \301\370\277\255\264\353\277\241 \265\356\267\317\307\321 \276\306\300\314\305\333\300\314 \306\316\276\337 \270\266\304\317\277\241 \265\356\267\317 \265\313\264\317\264\331.",
							0, 0, 0, 0));
					else
						AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
							(int)"\307\366\300\347 \301\370\277\255\264\353\260\241 \301\246\260\305\265\310 \273\363\305\302\267\316 \300\372\300\345\265\307\276\372\275\300\264\317\264\331. \301\370\277\255\264\353\277\241 \265\356\267\317\307\321 \276\306\300\314\305\333\300\314 \306\316\276\337 \270\266\304\317\277\241 \265\356\267\317\265\307\301\366 \276\312\275\300\264\317\264\331.",
							0, 0, 0, 0));
				}
			}
			else
			{
				if (result == 1)
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
						(int)"\301\370\277\255\264\353 \271\350\304\241 \275\307\306\320",
						0, 0, 0, 0));
				else
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
						(int)"\301\370\277\255\264\353 \301\246\260\305 \275\307\306\320",
						0, 0, 0, 0));
			}
		}
	}
	break;
	case PKT645_SEM_983(0x16c):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (!IsLocalContent((localContentType_t)0x67 /*983: era 0x66; interpolado*/))
			break;

		unsigned long uid = packet.Decode4();
		unsigned long stallKey = packet.Decode4();
		unsigned char result = packet.Decode1();

		if (result != 1)
			break;

		if (MyUID() == uid)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\300\332\275\305\300\307 \301\370\277\255\264\353\264\302 \261\270\260\346\307\322 \274\366 \276\370\275\300\264\317\264\331.",
				0, 0, 0, 0));
			break;
		}

		std::list<sStall>& stallList = Doc()->m_listStallItem;

		sStall stall;
		for (std::list<sStall>::iterator it = stallList.begin();
			it != stallList.end(); it++)
		{
			if (it->stallKey == stallKey)
			{
				Doc()->SetStallItem(*it);
				break;
			}
		}

		((RbL8380SharedDoc*)Doc())->SetTradeMode(1);

		if ((&Doc()->m_stallItem)->stallKey != 0)
		{
			FrWnd* pWnd = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
				"offlinetradedlg");
			if (pWnd)
				break;
			__rtti_obj = pWnd;

			COfflineShopVisitorDlg* pDlg = CreateForm<COfflineShopVisitorDlg>(
				g_pFresh->GetManager(), this, "offlinetradedlg", NULL);
			if (!pDlg)
				break;

			pDlg->Open(NULL, 0xc0);
			pDlg->OpenTradeShop(
				Doc()->m_replayTargetUID); // Doc+0x47d4 (uid do dono da loja)
			pDlg->SetListTradeItem(false);
			pDlg->SetFadeout(false);
		}
		else
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353\260\241 \276\370\275\300\264\317\264\331.",
				0, 0, 0, 0));
		}
	}
	break;
	case PKT645_SEM_983(0x167):
	{
		unsigned long uid = packet.Decode4();
		unsigned long stallKey = packet.Decode4();
		int slot = packet.Decode1();

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		std::list<sStall>* pStallList = &Doc()->m_listStallItem;
		sStall* pStall = &Doc()->m_stallItem;

		if (Doc()->m_replayTargetUID != uid) // Doc+0x47d4 (uid do dono da loja)
			break;

		if (pStall->stallKey == stallKey)
		{
			if (pStall->slotList[slot - 1].itemData.iNum == 0)
				memset(&pStall->slotList[slot - 1].itemData, 0,
					sizeof(sTradeItem));

			Doc()->SetStallItem(*pStall);
			Doc()->ChageListStallitem(pStall);
		}
		else
		{
			if (!pStallList)
				break;

			// no original so' o primeiro elemento da lista e o primeiro slot sao testados
			std::list<sStall>::iterator it = pStallList->begin();
			if (!(it != pStallList->end()))
				break;
			if (it->stallKey != stallKey)
				break;
			if (!(0 < it->maxSlotNum))
				break;
			if (it->slotList[0].Idx != slot)
				break;

			memset(&it->slotList[0].itemData, 0, sizeof(sTradeItem));
		}

		if (MyUID() == Doc()->m_replayTargetUID)
		{
			RbL8451ShopEditDlg* pEdit =
				(RbL8451ShopEditDlg*)g_pFresh->GetManager()
					->GetDesktop()
					->FindChildByName("tradeeditdlg");
			if (pEdit)
				pEdit->SetListTradeItem();
			else
				__rtti_obj = pEdit;

			RbL8451FinishEditDlg* pBuy =
				(RbL8451FinishEditDlg*)g_pFresh->GetManager()
					->GetDesktop()
					->FindChildByName("tradebuydlg");
			if (pBuy)
				pBuy->SetListTradeItem(NULL);
			else
				__rtti_obj = pBuy;

			FrWnd* pDlg = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
				"offlinetradedlg");
			if (!pDlg)
			{
				__rtti_obj = pDlg;
				return 1;
			}
			((RbTradeDlg*)pDlg)->OpenTradeShop(Doc()->m_replayTargetUID);
			((RbL8451ShopDlg*)pDlg)->SetListTradeItem(false);
			return 1;
		}
		else
		{
			FrWnd* pBuy = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
				"tradebuydlg");
			if (pBuy)
				pBuy->Close(true);
			else
				__rtti_obj = pBuy;

			FrWnd* pDlg = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
				"offlinetradedlg");
			if (!pDlg)
			{
				__rtti_obj = pDlg;
				return 1;
			}
			((RbTradeDlg*)pDlg)->OpenTradeShop(Doc()->m_replayTargetUID);
			((RbL8451VisitorDlg*)pDlg)->SetListTradeItem(false);
			return 1;
		}
	}
	break;
	case PKT645_SEM_983(0x161):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (!IsLocalContent((localContentType_t)0x67 /*983: era 0x66; interpolado*/))
			break;

		int result = packet.Decode4();
		unsigned long code = packet.Decode4();

		if (result == 1)
		{
			__int64 pang = packet.Decode8();
			Doc()->m_myInfo.stat.i64Pang = pang;

			if (MyUID() ==
				Doc()->m_replayTargetUID) // Doc+0x47d4 (uid do dono da loja)
				break;

			WSendPacket send(0x21);

			send.EncodeStr(std::string(MyNick()));
			send.Encode4(MyUID());

			send.Send(TO_GAME);
			return result;
		}

		switch (code)
		{
		case 0:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\307\366\300\347 \301\370\277\255\264\353 \300\314\277\353\277\241 \301\246\300\347\270\246 \271\336\260\355 \300\326\275\300\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 1:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353 \260\374\267\303 \274\272\260\370",
				0, 0, 0, 0));
			break;
		case 2:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353 \260\374\267\303 \275\307\306\320",
				0, 0, 0, 0));
			break;
		case 3:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353 \265\356\267\317\277\241 \275\307\306\320\307\317\277\264\275\300\264\317\264\331.(Error DB)",
				0, 0, 0, 0));
			break;
		case 4:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353\277\241 \276\306\300\314\305\333 \265\356\267\317 \274\272\260\370\307\317\277\264\275\300\264\317\264\331. \260\341\260\372\260\252\300\273 \271\336\301\366 \270\370\307\317\277\264\275\300\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 5:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353\270\246 \274\322\300\257\307\317\301\366 \276\312\300\270\274\314\275\300\264\317\264\331. \261\270\270\305 \310\304 \300\314\277\353\307\330 \301\326\274\274\277\344.",
				0, 0, 0, 0));
			break;
		case 6:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353\300\307 \300\337\270\370\265\310 \275\275\267\324 \265\245\300\314\305\270\270\246 \260\241\301\366\260\355 \300\326\275\300\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 7:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\306\316\300\314 \272\316\301\267\307\325\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 8:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\300\314\271\314 \277\265\261\270 \275\275\267\324\300\270\267\316 \277\254\300\345\271\327 \261\270\270\305\307\322\274\366 \276\370\275\300\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 9:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\304\355\305\260\260\241 \272\316\301\267\307\325\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 10:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\265\356\267\317\307\317\267\301\264\302 \276\306\300\314\305\333\300\307 \260\263\274\366\260\241 \274\322\300\257\307\321 \274\366\267\256\272\270\264\331 \270\271\276\306\274\255 \265\356\267\317\307\322 \274\366 \276\370\275\300\264\317\264\331. ",
				0, 0, 0, 0));
			break;
		case 12:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\265\356\267\317\307\317\267\301\264\302 \276\306\300\314\305\333\300\272 \300\337\270\370\265\310 \276\306\300\314\305\333\300\324\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 13:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\275\275\267\324\277\241 \300\314\271\314 \276\306\300\314\305\333\300\314 \265\356\267\317 \265\307\276\356\300\326\275\300\264\317\264\331. \265\356\267\317 \307\330\301\366 \300\314\310\304 \264\331\275\303 \265\356\267\317\307\330\301\326\274\274\277\344",
				0, 0, 0, 0));
			break;
		case 14:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353\277\241 \265\356\267\317\307\322 \274\366 \276\370\264\302 \276\306\300\314\305\333\300\324\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 15:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\300\314\271\314 \301\370\277\255\264\353\277\241 \265\356\267\317\265\310 \276\306\300\314\305\333\300\324\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 16:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\275\275\267\324 \261\342\260\243\300\314 \301\276\267\341\265\307\276\372\275\300\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 17:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\276\306\300\314\305\333\300\307 \272\270\300\257\267\256\300\273 \303\312\260\372\307\317\260\324 \265\313\264\317\264\331. \307\366\300\347 \276\306\300\314\305\333\300\307 \260\271\274\366\270\246 \264\331\275\303 \310\256\300\316\307\317\275\303\261\342 \271\331\266\370\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 18:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\306\307\270\305\300\332\300\307 \301\370\277\255\264\353\260\241 \306\355\301\375 \270\360\265\345\267\316 \272\257\260\346\265\307\276\372\275\300\264\317\264\331. \261\270\270\305\260\241 \272\322\260\241\264\311\307\325\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 11:
		case 19:
		{
			if (MyUID() == Doc()->m_replayTargetUID)
			{
				FrWnd* pEdit =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"tradeeditdlg");
				if (pEdit)
					pEdit->Close(true);
				else
					__rtti_obj = pEdit;

				FrWnd* pBuy =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"tradebuydlg");
				if (pBuy)
					pBuy->Close(true);
				else
					__rtti_obj = pBuy;

				FrWnd* pDlg =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"offlinetradedlg");
				if (pDlg)
					pDlg->Close(true);
				else
					__rtti_obj = pDlg;
			}
			else
			{
				FrWnd* pBuy =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"tradebuydlg");
				if (pBuy)
					pBuy->Close(true);
				else
					__rtti_obj = pBuy;

				FrWnd* pDlg =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"offlinetradedlg");
				if (pDlg)
					pDlg->Close(true);
				else
					__rtti_obj = pDlg;
			}

			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353\300\307 \301\244\272\270\260\241 \272\257\260\346\265\307\276\372\275\300\264\317\264\331. \301\370\277\255\264\353\270\246 \264\331\275\303 \277\255\276\356 \301\326\274\274\277\344",
				0, 0, 0, 0));
		}
		break;
		}
	}
	break;
	case PKT645_SEM_983(0x165):
	case PKT645_SEM_983(0x166):
	{
		int page = (unsigned short)packet.Decode2();
		int count = (unsigned short)packet.Decode2();
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (count)
		{
			((CTaskMain*)AfxGetTask()->GetMainActor())->CreateMaketDlg();
			RbOfflineTradeDlg* pDlg =
				(RbOfflineTradeDlg*)g_pFresh->GetManager()
					->GetDesktop()
					->FindChildByName("offlinetraderanking");
			CSharedDoc::Instance()->ClearMaketList();
			for (int i = count; i > 0; --i)
			{
				sSaleItem item;
				packet.DecodeBuffer(&item, sizeof(sSaleItem));
				CSharedDoc::Instance()->AddMaketListItem(item);
			}
			if (pDlg)
			{
				pDlg->SetListItem();
				pDlg->SetPageData(-1, page);
			}
			return 1;
		}
		RbOfflineTradeDlg* pDlg = (RbOfflineTradeDlg*)g_pFresh->GetManager()
									  ->GetDesktop()
									  ->FindChildByName("offlinetraderanking");
		CSharedDoc::Instance()->ClearMaketList();
		if (pDlg)
		{
			pDlg->SetListItem();
			pDlg->SetPageData(1, 1);
		}
		else
		{
			__rtti_obj = pDlg;
		}
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)"\270\266\304\317\277\241\274\255 \260\313\273\366\265\310 \273\363\307\260\300\314 \276\370\275\300\264\317\264\331.",
			0, 0, 0, 0));
	}
	break;
	// S65: casos do 983 sem par no OnPacketCommon do 645, escritos pelo desmonte do despachante comum (009c20a0).
	// Correio e móveis do my room: no 645 ficam em CRealMyRoomTask::OnPacket (ids 0x125, 0x13f, 0x141, 0x143, 0x145, 0x181);
	// no 983 vão ao ator principal da tarefa. Mensagens do 983: 0x29d (era 0x228), 0x2bf (era 0x244), 0x2a6, 0x2a4, 0x107.
	case 0x12d: // 009d1b97
	{
		Doc()->m_furnitureList.clear();
		unsigned long result = packet.Decode4();
		if (result == 1)
		{
			Doc()->m_furnitureList.clear();
			int count = packet.Decode2();
			for (; count > 0; --count)
			{
				sFurniture_List furniture;
				memset(&furniture, 0, 0x1b);
				packet.DecodeBuffer(&furniture, 0x1b);
				Doc()->AddFurniture(furniture);
			}
		}
		else
		{
			AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x2a6, 0x12d, result, 0, 0, 0);
		}
		IActor* pMain = AfxGetTask()->GetMainActor();
		if (pMain)
		{
			pMain << MsgObject(NULL, 0x107, 0, 0, 0, 0, 0);
			pMain << MsgObject(NULL, 2, 0, 0, 0, 0, 0);
		}
	}
	break;
	case 0x147: // 009d1fbb
	{
		unsigned char result = packet.Decode1();
		AfxGetTask()->GetMainActor()
			<< MsgObject(NULL, 0x29d, 0x147, result, 0, 0, 0)
			<< MsgObject(NULL, 0x2bf, 1, 0, 0, 0, 0);
	}
	break;
	case 0x149: // 009d20fb
	case 0x14b: // 009d2184 (mesmo corpo; o 983 também manda 0x147 como subcódigo)
	{
		int result = packet.Decode1();
		AfxGetTask()->GetMainActor()
			<< MsgObject(NULL, 2, 0, 0, 0, 0, 0)
			<< MsgObject(NULL, 0x29d, 0x147, result, 0, 0, 0)
			<< MsgObject(NULL, 0x2bf, 1, 0, 0, 0, 0);
	}
	break;
	case 0x14d: // 009d22da
	{
		int result = packet.Decode1();
		AfxGetTask()->GetMainActor()
			<< MsgObject(NULL, 0x29d, 0x14d, result, 0, 0, 0)
			<< MsgObject(NULL, 2, 0, 0, 0, 0, 0)
			<< MsgObject(NULL, 0x2bf, 1, 0, 0, 0, 0);
	}
	break;
	case 0x164: // 009d21dd (sem leitura)
	{
		AfxGetTask()->GetMainActor()
			<< MsgObject(NULL, 2, 0, 0, 0, 0, 0)
			<< MsgObject(NULL, 0x2a4, 0, 0, 0, 0, 0)
			<< MsgObject(NULL, 0x2bf, 1, 0, 0, 0, 0);
	}
	break;
	case 0x165: // 009d225a
	{
		int value = packet.Decode4();
		AfxGetTask()->GetMainActor()
			<< MsgObject(NULL, 0x29d, 0x165, value, 0, 0, 0)
			<< MsgObject(NULL, 2, 0, 0, 0, 0, 0)
			<< MsgObject(NULL, 0x2bf, 1, 0, 0, 0, 0);
	}
	break;
	case 0x17c: // 009d2022: envio 0x29 ao MSN (no 645, 0x27) e limpa m_giftName/m_giftIndex (983: Doc+0x10d60/+0x10d7c)
	{
		if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_MSN))
			break;

		WSendPacket send(0x29);
		send.EncodeStr(MyId());
		send.EncodeStr(MyId());
		send.Send(TO_MSN);
		Doc()->m_giftName = "";
		Doc()->m_giftIndex = -1;
	}
	break;
	// S65: família do Gateway do 983 (mensagens 0x7534/0x7535/0x753a/0x753e/0x753f ao ator "Gateway"; erro != 0 vira a
	// mensagem 0x27 ao ator principal). O tratador dessas mensagens no ator Gateway do 983 não está escrito.
	case 0x1ef: // 009d3553
	{
		int values[4];
		values[0] = packet.Decode4();
		values[1] = packet.Decode4();
		values[2] = packet.Decode4();
		values[3] = packet.Decode4();
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x7535, 0xd, (int)values, 0, 0, 0);
	}
	break;
	case 0x202: // 009d36dc (o valor lido não é usado)
	{
		packet.Decode4();
		GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x7534, 2, 0, 0, 0, 0);
	}
	break;
	case 0x25a: // 009d2d03
	{
		int error = packet.Decode4();
		if (error)
		{
			SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753e, 1, (int)&packet, 0, 0, 0);
	}
	break;
	case 0x21e: // PKT C42: so do 983 (009d4bb8): lista de missoes (CSharedDoc+0x384c), pode vir em mais de um pacote; no fim, msg 0x2d (0x40)
	{
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		int total = packet.Decode4();
		int count = packet.Decode4();
		for (int i = count; i; i--)
		{
			_quest::CQuestItem item;
			item.Decode983(&packet);
			Doc()->AddQuest983(item); // 983: CSharedDoc 00642940 (C42.2)
		}
		if (total == count)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2d, 0x40, 0, 0, 0, 0));
	}
	break;
	case 0x21f: // PKT C42: so do 983 (009d4c6f): missoes que mudaram (mesma forma do 0x223/0x21f); para na primeira que nao estiver na lista
	{
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		unsigned int count = packet.Decode4();
		for (unsigned int i = 0; i < count; i++)
		{
			_quest::CQuestItem item;
			item.Decode983(&packet);
			unsigned long uid = item.GetUid983();
			std::list<_quest::CQuestItem>& quests = Doc()->m_quests983;
			std::list<_quest::CQuestItem>::iterator it = std::find_if(quests.begin(), quests.end(),
				[uid](const _quest::CQuestItem& q) { return q.GetUid983() == uid; }); // 009b78d0/009bcf30
			if (it == quests.end())
				break;
			it->Update983(item);
		}
	}
	break;
	case 0x221: // PKT C42: so do 983 (009d4dd9): missoes aceitas (Accept983); a que ficar inativa sai da lista
	{
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		unsigned int count = packet.Decode4();
		for (unsigned int i = 0; i < count; i++)
		{
			unsigned long uid = packet.Decode4();
			std::list<_quest::CQuestItem>& quests = Doc()->m_quests983;
			std::list<_quest::CQuestItem>::iterator it = std::find_if(quests.begin(), quests.end(),
				[uid](const _quest::CQuestItem& q) { return q.GetUid983() == uid; }); // 009b7900/009bcf80
			if (it == quests.end())
				break;
			it->Accept983();
			if (!it->IsActive983())
				quests.erase(it); // 00589920
		}
	}
	break;
	case 0x222: // PKT C42: so do 983 (009d4ed1): missoes canceladas (Deactivate983); a que ficar inativa sai da lista
	{
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		unsigned int count = packet.Decode4();
		for (unsigned int i = 0; i < count; i++)
		{
			unsigned long uid = packet.Decode4();
			std::list<_quest::CQuestItem>& quests = Doc()->m_quests983;
			std::list<_quest::CQuestItem>::iterator it = std::find_if(quests.begin(), quests.end(),
				[uid](const _quest::CQuestItem& q) { return q.GetUid983() == uid; }); // 009b7930/009bcfd0
			if (it == quests.end())
				break;
			it->Deactivate983();
			if (!it->IsActive983())
				quests.erase(it); // 00589920
		}
	}
	break;
	case 0x223: // PKT C42: so do 983 (009d4fcf): missoes que mudaram (mesma forma do 0x223/0x21f); para na primeira que nao estiver na lista
	{
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		unsigned int count = packet.Decode4();
		for (unsigned int i = 0; i < count; i++)
		{
			_quest::CQuestItem item;
			item.Decode983(&packet);
			unsigned long uid = item.GetUid983();
			std::list<_quest::CQuestItem>& quests = Doc()->m_quests983;
			std::list<_quest::CQuestItem>::iterator it = std::find_if(quests.begin(), quests.end(),
				[uid](const _quest::CQuestItem& q) { return q.GetUid983() == uid; }); // 009b7960/009bd020
			if (it == quests.end())
				break;
			it->Update983(item);
		}
	}
	break;
	case 0x220: // 009d4d88
	{
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753a, 4, (int)&packet, 0, 0, 0);
	}
	break;
	case 0x225: // 009d5116
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753a, 2, (int)&packet, 0, 0, 0);
	}
	break;
	case 0x226: // 009d51a8
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753a, 3, (int)&packet, 0, 0, 0);
	}
	break;
	case 0x227: // 009d523a
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753a, 5, (int)&packet, 0, 0, 0);
	}
	break;
	case 0x228: // 009d52cc
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753a, 6, (int)&packet, 0, 0, 0);
	}
	break;
	case 0x22c: // 009d542f
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753a, 9, (int)&packet, 0, 0, 0);
	}
	break;
	case 0x22d: // 009d54c1
	{
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753a, 8, (int)&packet, 0, 0, 0);
	}
	break;
	case 0x22e: // 009d5512 (sem leitura)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753a, 0xa, (int)&packet, 0, 0, 0);
	}
	break;
	case 0x278: // 009d6ffc
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753f, 3, 0, 0, 0, 0);
	}
	break;
	case 0x279: // 009d708f
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753f, 2, 0, 0, 0, 0);
	}
	break;
	case 0x27a: // 009d7122
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753f, 1, 0, 0, 0, 0);
	}
	break;
	case 0x27b: // 009d71b4
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753f, 4, 0, 0, 0, 0);
	}
	break;
	case 0x27c: // 009d7246
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753f, 5, 0, 0, 0, 0);
	}
	break;
	// S65b: casos pequenos do 983 que só avisam o ator principal (009d535e, 009d55e1, 009d557a, 009d5598).
	case 0x229:
	{
		int value = packet.Decode4();
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x325, (int)&value, 0, 0, 0, 0));
	}
	break;
	case 0x22b:
	{
		if (packet.Decode4() != 0)
			break;
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x32e, 0, 0, 0, 0, 0));
	}
	break;
	case 0x230:
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
		break;
	case 0x231:
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		break;
	// S66: mais casos do 983 sem par no OnPacketCommon do 645 (desmonte em out/F3.5/s66/dx/).
	case 0x1d4: // 009d3421
	{
		if (packet.Decode4() != 0)
			break;
		Doc()->m_loginToken983 = packet.DecodeStr();
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x1b2, 0, 0, 0, 0, 0));
	}
	break;
	case 0x1d9: // 009d2670: nível (só o byte baixo) e experiência do próprio jogador
	{
		unsigned long level = packet.Decode4();
		unsigned long exp = packet.Decode4();
		Doc()->m_myInfo.stat.Level = (unsigned char)level;
		Doc()->m_myInfo.stat.dwExp = exp;
	}
	break;
	case 0x21c: // 009d4aee
	case 0x224: // 009d50e8
	{
		int error = packet.Decode4();
		if (error)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
	}
	break;
	case 0x22a: // 009d5391
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		int value = packet.Decode4();
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x326, (int)&value, 0, 0, 0, 0));
	}
	break;
	case 0x235: // 009d55b6: os 10 itens equipados (sMyInfo +0x296)
	{
		for (int i = 0; i < 10; i++)
			Doc()->m_myInfo.userEquip.tidItemSlot[i] = packet.Decode4();
	}
	break;
	case 0x24d: // 009d5fea (os três ids no mesmo corpo; o id escolhe 0, 1 ou 2)
	case 0x24e:
	case 0x24f:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753c, type == 0x24d ? 0 : (type == 0x24e ? 1 : 2), (int)&packet, 0, 0, 0);
	}
	break;
	case 0x253: // 009d63b3 (0062bf80 = grava CSharedDoc+0x11638)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		Doc()->m_lobbyState983 = 6;
	}
	break;
	case 0x25b: // 009d2cc6
	{
		int error = packet.Decode4();
		if (error)
		{
			SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
			break;
		}
		GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753e, 2, (int)&packet, 0, 0, 0);
	}
	break;
	case 0x276: // 009d4332: o valor vai a WNetworkSystem 00a4d6d0, que no 983 é vazia (`ret 4`)
		packet.Decode4();
		break;
	case 0x277: // 009d6f8d
	case 0x27d: // 009d2b9c
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, error, 0, 0, 0, 0));
	}
	break;
	// 983: 0x1dd-0x1e0 = família nova (casos 009d23bb, 009d2481, 009d251e, 009d25bb): resposta com erro na frente,
	// repassada ao ator Gateway como mensagem 0x30a (0 = pacote inteiro, 1, 2, 3; 4 = erro). Não existe no 645.
	case 0x1dd:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			GetActor(s_gatewayActor[1])
				<< MsgObject(NULL, 0x30a, 4, error, 0, 0, 0);
			break;
		}
		GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x30a, 0, (int)&packet, 0, 0, 0);
	}
	break;
	case 0x1de:
	case 0x1df:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			GetActor(s_gatewayActor[1])
				<< MsgObject(NULL, 0x30a, 4, error, 0, 0, 0);
			break;
		}
		int value = packet.Decode4();
		GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x30a, (unsigned short)type == 0x1de ? 1 : 2, value, 0, 0, 0);
	}
	break;
	case 0x1e0:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int error = packet.Decode4();
		if (error)
		{
			GetActor(s_gatewayActor[1])
				<< MsgObject(NULL, 0x30a, 4, error, 0, 0, 0);
			break;
		}
		int v1 = packet.Decode4();
		int v2 = packet.Decode4();
		int v3 = packet.Decode4();
		GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x30a, 3, v1, v2, v3, 0);
	}
	break;
	// 645 0x162/0x163 (registro/retirada na troca offline): faixa 0x160-0x16d sem lugar no 983. A sessão 61 pôs este corpo
	// em 0x1de/0x1df por um voto de assinatura (D4 D4); o desmonte mostra que 0x1de/0x1df são os casos acima.
	case PKT645_SEM_983(0x162):
	case PKT645_SEM_983(0x163):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (!IsLocalContent((localContentType_t)0x67 /*983: era 0x66; interpolado*/))
			break;
		int result = packet.Decode4();
		int code = packet.Decode4();
		if (result == 1)
		{
			if ((unsigned short)type == 0x162)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
					(int)"\276\306\300\314\305\333\300\273 \265\356\267\317 \307\317\277\264\275\300\264\317\264\331.",
					0, 0, 0, 0));
				break;
			}
			else if ((unsigned short)type == 0x163)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
					(int)"\276\306\300\314\305\333\300\273 \301\246\260\305 \307\317\277\264\275\300\264\317\264\331.",
					0, 0, 0, 0));
				break;
			}
		}
		switch (code)
		{
		case 0:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\307\366\300\347 \301\370\277\255\264\353 \300\314\277\353\277\241 \301\246\300\347\270\246 \271\336\260\355 \300\326\275\300\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 1:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353 \260\374\267\303 \274\272\260\370",
				0, 0, 0, 0));
			break;
		case 2:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353 \260\374\267\303 \275\307\306\320",
				0, 0, 0, 0));
			break;
		case 3:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353 \265\356\267\317\277\241 \275\307\306\320\307\317\277\264\275\300\264\317\264\331.(Error DB)",
				0, 0, 0, 0));
			break;
		case 4:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353\277\241 \276\306\300\314\305\333 \265\356\267\317 \274\272\260\370\307\317\277\264\275\300\264\317\264\331. \260\341\260\372\260\252\300\273 \271\336\301\366 \270\370\307\317\277\264\275\300\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 5:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353\270\246 \274\322\300\257\307\317\301\366 \276\312\300\270\274\314\275\300\264\317\264\331. \261\270\270\305 \310\304 \300\314\277\353\307\330 \301\326\274\274\277\344.",
				0, 0, 0, 0));
			break;
		case 6:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353\300\307 \300\337\270\370\265\310 \275\275\267\324 \265\245\300\314\305\270\270\246 \260\241\301\366\260\355 \300\326\275\300\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 7:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\306\316\300\314 \272\316\301\267\307\325\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 8:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\300\314\271\314 \277\265\261\270 \275\275\267\324\300\270\267\316 \277\254\300\345\271\327 \261\270\270\305\307\322\274\366 \276\370\275\300\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 9:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\304\355\305\260\260\241 \272\316\301\267\307\325\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 10:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\265\356\267\317\307\317\267\301\264\302 \276\306\300\314\305\333\300\307 \260\263\274\366\260\241 \274\322\300\257\307\321 \274\366\267\256\272\270\264\331 \270\271\276\306\274\255 \265\356\267\317\307\322 \274\366 \276\370\275\300\264\317\264\331. ",
				0, 0, 0, 0));
			break;
		case 12:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\265\356\267\317\307\317\267\301\264\302 \276\306\300\314\305\333\300\272 \300\337\270\370\265\310 \276\306\300\314\305\333\300\324\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 13:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\275\275\267\324\277\241 \300\314\271\314 \276\306\300\314\305\333\300\314 \265\356\267\317 \265\307\276\356\300\326\275\300\264\317\264\331. \265\356\267\317 \307\330\301\366 \300\314\310\304 \264\331\275\303 \265\356\267\317\307\330\301\326\274\274\277\344",
				0, 0, 0, 0));
			break;
		case 14:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353\277\241 \265\356\267\317\307\322 \274\366 \276\370\264\302 \276\306\300\314\305\333\300\324\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 15:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\300\314\271\314 \301\370\277\255\264\353\277\241 \265\356\267\317\265\310 \276\306\300\314\305\333\300\324\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 16:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\275\275\267\324 \261\342\260\243\300\314 \301\276\267\341\265\307\276\372\275\300\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 17:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\276\306\300\314\305\333\300\307 \272\270\300\257\267\256\300\273 \303\312\260\372\307\317\260\324 \265\313\264\317\264\331. \307\366\300\347 \276\306\300\314\305\333\300\307 \260\271\274\366\270\246 \264\331\275\303 \310\256\300\316\307\317\275\303\261\342 \271\331\266\370\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 18:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\306\307\270\305\300\332\300\307 \301\370\277\255\264\353\260\241 \306\355\301\375 \270\360\265\345\267\316 \272\257\260\346\265\307\276\372\275\300\264\317\264\331. \261\270\270\305\260\241 \272\322\260\241\264\311\307\325\264\317\264\331.",
				0, 0, 0, 0));
			break;
		case 11:
		{
			if (MyUID() == Doc()->m_replayTargetUID)
			{
				FrWnd* pEdit =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"tradeeditdlg");
				if (pEdit)
					pEdit->Close(true);
				else
					__rtti_obj = pEdit;
				FrWnd* pBuy =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"tradebuydlg");
				if (pBuy)
					pBuy->Close(true);
				else
					__rtti_obj = pBuy;
				FrWnd* pShop =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"offlinetradedlg");
				if (pShop)
					pShop->Close(true);
				else
					__rtti_obj = pShop;
			}
			else
			{
				FrWnd* pBuy =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"tradebuydlg");
				if (pBuy)
					pBuy->Close(true);
				else
					__rtti_obj = pBuy;
				FrWnd* pShop =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"offlinetradedlg");
				if (pShop)
					pShop->Close(true);
				else
					__rtti_obj = pShop;
			}
			WSendPacketSem983 send((enumClientPacket)0xc4); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
			send.Encode4(Doc()->m_replayTargetUID);
			send.Send(TO_GAME);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\301\370\277\255\264\353\300\307 \301\244\272\270\260\241 \272\257\260\346\265\307\276\372\275\300\264\317\264\331. \301\370\277\255\264\353\270\246 \264\331\275\303 \277\255\276\356 \301\326\274\274\277\344",
				0, 0, 0, 0));
			return 1;
		}
		}
	}
	break;
	case PKT645_SEM_983(0x169):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (!IsLocalContent((localContentType_t)0x67 /*983: era 0x66; interpolado*/))
			break;
		packet.Decode4();
		unsigned char slot = packet.Decode1();
		static sTradeItem item;
		packet.DecodeBuffer(&item, sizeof(item));
		sStall* pStall = &Doc()->m_stallItem;
		if (item.dwTid == 0 && pStall->slotList[slot - 1].IsValid)
		{
			FILETIME ftSlot;
			FILETIME ftServer;
			SystemTimeToFileTime(&pStall->slotList[slot - 1].time, &ftSlot);
			SystemTimeToFileTime(&Doc()->GetServerTime(), &ftServer);
			__int64 diff =
				((__int64)ftSlot.dwHighDateTime - ftServer.dwHighDateTime) *
					0x100000000i64 -
				ftServer.dwLowDateTime + ftSlot.dwLowDateTime;
			if (diff / 60 / 60 / 10000000 <= 0)
				pStall->slotList[slot - 1].IsValid = 0;
		}
		pStall->slotList[slot - 1].itemData = item;
		Doc()->SetStallItem(*pStall);
		Doc()->ChageListStallitem(pStall);
		FrWnd* pEdit = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
			"tradeeditdlg");
		if (pEdit)
		{
			OFFSHOPCLOSING983(pEdit) = 1; // COfflineShopEditDlg::m_closing
			pEdit->Close(true);
			RbOfflineShopDlg169* pShop =
				(RbOfflineShopDlg169*)RbCreateOfflineShopDlg169(
					g_pFresh->GetManager(), this, "offlinetradedlg", NULL);
			if (pShop)
			{
				((FrForm*)pShop)->Open(NULL, 0xc0);
				((RbTradeDlg*)pShop)->OpenTradeShop(Doc()->m_replayTargetUID);
				pShop->SetListTradeItem(false);
				pShop->SetFadeout(false);
			}
		}
		else
			__rtti_obj = pEdit;
		RbOfflineTradeFinishEditDlg169* pBuy =
			(RbOfflineTradeFinishEditDlg169*)g_pFresh->GetManager()
				->GetDesktop()
				->FindChildByName("tradebuydlg");
		if (pBuy)
			pBuy->SetListTradeItem(NULL);
		else
			__rtti_obj = pBuy;
		RbOfflineShopDlg169* pDlg = (RbOfflineShopDlg169*)g_pFresh->GetManager()
										->GetDesktop()
										->FindChildByName("offlinetradedlg");
		if (pDlg)
		{
			((RbTradeDlg*)pDlg)->OpenTradeShop(Doc()->m_replayTargetUID);
			pDlg->SetListTradeItem(false);
			return 1;
		}
		__rtti_obj = pDlg;
	}
	break;
	case 0xe3:
	{
		int result = packet.Decode4();
		if (result == 1)
		{
			std::string nick;
			packet.DecodeStr(nick);
			GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0xe8), (int)nick.c_str(), result, 0, 0, 0);
			if (strcmp(MyNick(), nick.c_str()) == 0)
				((RbSharedDocTrade*)Doc())->SetStateTrade((eStateTrade)1);
		}
		SetTradeResultCode(result, 0xe3, NULL);
	}
	break;
	case 0xe4:
	{
		int result = packet.Decode4();
		if (result == 1)
		{
			std::string nick;
			packet.DecodeStr(nick);
			unsigned long uid = packet.Decode4();
			GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0xe8), (int)nick.c_str(), 0, 0, 0, 0);
			if (uid == Doc()->m_tradeUID || uid == MyUID())
			{
				Doc()->InitTradeData();
				FrWnd* pDlg =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"tradedlg");
				if (pDlg)
					pDlg->Close(true);
				if (strcmp(MyNick(), nick.c_str()) == 0)
				{
					FrWnd* pEdit =
						g_pFresh->GetManager()->GetDesktop()->FindChildByName(
							"tradeeditdlg");
					if (pEdit)
						pEdit->Close(true);
				}
				else
				{
					result = 3;
					Doc()->InitTradeData();
					FrWnd* pBuy =
						g_pFresh->GetManager()->GetDesktop()->FindChildByName(
							"tradebuydlg");
					if (pBuy)
					{
						FrWnd* pNotify =
							g_pFresh->GetManager()
								->GetDesktop()
								->FindChildByName("notify_okcancel");
						if (pNotify)
							pNotify->Close(true);
						pBuy->Close(true);
					}
				}
			}
		}
		SetTradeResultCode(result, 0xe4, NULL);
	}
	break;
	case 0xe5:
	{
		int result = packet.Decode4();
		if (result == 1)
		{
			std::string nick;
			packet.DecodeStr(nick);
			unsigned long uid = packet.Decode4();
			GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0xe8), (int)nick.c_str(), 2, 0, 0, 0);
			if (strcmp(nick.c_str(), MyNick()) == 0)
			{
				((RbSharedDocTrade*)Doc())->SetStateTrade((eStateTrade)2);
				FrWnd* pDlg =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"tradedlg");
				if (pDlg)
					pDlg->Close(true);
				else
					__rtti_obj = pDlg;
				FrWnd* pEdit =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"tradeeditdlg");
				if (!pEdit)
				{
					__rtti_obj = pEdit;
					FrForm* pForm = RbCreateTradeEditDlg(g_pFresh->GetManager(),
						this, "tradeeditdlg", NULL);
					if (pForm)
					{
						// 983 009ccdbc: sobe 25 pontos e abre com 0xc1, como FrTradeDlg::OnEditBnUp
						WRect rect = pForm->GetRect();
						rect.y -= 25.0;
						pForm->SetRect(rect);
						pForm->Open(NULL, 0xc1);
						pForm->SetFadeout(false);
					}
				}
			}
			else if (uid == Doc()->m_tradeUID)
			{
				result = 4;
				Doc()->InitTradeData();
				FrWnd* pDlg =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"tradedlg");
				if (pDlg)
					pDlg->Close(true);
				else
					__rtti_obj = pDlg;
				FrWnd* pBuy =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"tradebuydlg");
				if (pBuy)
					pBuy->Close(true);
				else
					__rtti_obj = pBuy;
			}
		}
		SetTradeResultCode(result, 0xe5, NULL);
	}
	break;
	case 0xe6:
	{
		int tradeType = 0;
		int result = packet.Decode4();
		if (result == 1)
		{
			Doc()->InitTradeData();
			((RbSharedDocTrade*)Doc())->SetStateTrade((eStateTrade)4);
			// 983 (009ccf1e): o Decode4 do tipo de venda saiu do pacote; tradeType fica 0
			std::string title("");
			char info[0x16] = { 0 };
			packet.DecodeBuffer(info, 0x16);
			packet.DecodeStr(title);
			Doc()->SetTradeUID(packet.Decode4());
			Doc()->SetTradeTitle(title);
			sTradeItem item;
			int count = packet.Decode4();
			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&item, sizeof(item));
				if (!Doc()->AddListTradeItem(item))
				{
					result = 0;
					break;
				}
			}
		}
		SetTradeResultCode(result, 0xe6, NULL);
		RbTradeDlg* pDlg =
			(RbTradeDlg*)g_pFresh->GetManager()->GetDesktop()->FindChildByName(
				"tradedlg");
		if (!pDlg)
		{
			__rtti_obj = pDlg;
			pDlg = (RbTradeDlg*)RbCreateTradeDlg(g_pFresh->GetManager(), this,
				"tradedlg", NULL);
			if (!pDlg)
				return 1;
			((FrForm*)pDlg)->Open(NULL, 0xc1);
			pDlg->SetFadeout(false);
		}
		if (!pDlg)
			return 1;
		if (result == 1)
		{
			pDlg->OpenTradeShop(Doc()->m_tradeUID);
			pDlg->SetListTradeItem(false);
			return 1;
		}
		pDlg->Close(true);
		return 1;
	}
	case 0xe7:
	{
		int result = packet.Decode4();
		if (result == 1)
			Doc()->InitTradeData();
		SetTradeResultCode(result, 0xe7, NULL);
	}
	break;
	case 0xe8:
	{
		int result = packet.Decode4();
		std::string title;
		if (result == 1)
		{
			packet.DecodeStr(title);
			unsigned long uid = packet.Decode4();
			std::string nick;
			packet.DecodeStr(nick);
			CTask* pTask = IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask())
				? AfxGetTask()
				: AfxGetPreservedTask();
			if (pTask)
				pTask->GetActor("AvatarChat") << MsgObject(NULL, M983(0xe9),
					(int)nick.c_str(), (int)title.c_str(), 0, 0, 0);
			if (Doc()->m_stateTrade)
			{
				if (uid == Doc()->m_tradeUID || uid == MyUID())
				{
					Doc()->SetTradeTitle(title);
					RbTradeDlg* pDlg = (RbTradeDlg*)g_pFresh->GetManager()
										   ->GetDesktop()
										   ->FindChildByName("tradedlg");
					if (pDlg)
						pDlg->UpdateTradeTitle();
					RbTradeEditDlg* pEdit =
						(RbTradeEditDlg*)g_pFresh->GetManager()
							->GetDesktop()
							->FindChildByName("tradeeditdlg");
					if (pEdit)
					{
						pEdit->UpdateTradeTitle();
						pEdit->UpdateTradeTitleEdit();
						// JP 983 (009cd32d, C-PKT lote 10; pedido da A, sessao 84): titulo confirmado -> abre a loja.
						if (pEdit->IsOpenAfterTitle983())
						{
							pEdit->OnOpenBnUp983();
							pEdit->SetOpenAfterTitle983(false);
						}
					}
				}
			}
		}
	}
	break;
	case 0xe9:
	{
		int result = packet.Decode4();
		int count = packet.Decode4();
		if (result == 1)
			Doc()->SetCountVisitor(count);
		SetTradeResultCode(result, 0xe9, NULL);
		RbTradeDlg* pDlg =
			(RbTradeDlg*)g_pFresh->GetManager()->GetDesktop()->FindChildByName(
				"tradedlg");
		if (pDlg)
			pDlg->UpdateCountVisitor();
	}
	break;
	case 0xea:
	{
		int result = packet.Decode4();
		__int64 income = packet.Decode8();
		if (result == 1)
			Doc()->SetTradeIncome(income);
		SetTradeResultCode(result, 0xea, NULL);
	}
	break;
	case 0xeb:
	{
		int tradeType = 0;
		int result = packet.Decode4();
		if (result == 1)
		{
			((RbSharedDocTrade*)Doc())->SetStateTrade((eStateTrade)result);
			Doc()->ClearListTradeItem();
			// 983 (009cd446): o Decode4 do tipo de venda saiu do pacote; tradeType fica 0
			char info[0x16] = { 0 };
			packet.DecodeBuffer(info, 0x16);
			Doc()->SetTradeUID(packet.Decode4());
			GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0xe8), (int)info, 1, 0, 0, 0);
			sTradeItem item;
			int count = packet.Decode4();
			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&item, sizeof(item));
				if (!Doc()->AddListTradeItem(item))
				{
					result = 0;
					break;
				}
			}
		}
		SetTradeResultCode(result, 0xeb, NULL);
		if (result == 1)
		{
			FrWnd* pEdit =
				g_pFresh->GetManager()->GetDesktop()->FindChildByName(
					"tradeeditdlg");
			if (pEdit)
				pEdit->Close(true);
			else
				__rtti_obj = pEdit;
			RbTradeDlg* pDlg = (RbTradeDlg*)g_pFresh->GetManager()
								   ->GetDesktop()
								   ->FindChildByName("tradedlg");
			if (!pDlg)
			{
				__rtti_obj = pDlg;
				pDlg = (RbTradeDlg*)RbCreateTradeDlg(g_pFresh->GetManager(),
					this, "tradedlg", NULL);
				if (pDlg)
				{
					((FrForm*)pDlg)->Open(NULL, 0xc1);
					pDlg->SetFadeout(false);
					pDlg->OpenTradeShop(MyUID());
					return 1;
				}
			}
		}
	}
	break;
	case PKT645_SEM_983(0x1d7):
	{
		Cm::eErrorCode error = (Cm::eErrorCode)packet.Decode4();
		if (!IS_SUCCESSFUL(error))
		{
			GetActor(s_gatewayActor[1])
				<< MsgObject(NULL, M983(0x273), 0, error, 0, 0, 0);
			break;
		}
		_gateway::sTradeSellParam param;
		param.type = 1;
		packet.Decode8();
		param.price = packet.Decode8();
		unsigned char count = packet.Decode1();
		param.items.reserve(count);
		for (unsigned char i = 0; i < count; i++)
		{
			sTradeItem item;
			packet.DecodeBuffer(&item, sizeof(sTradeItem));
			param.items.push_back(item);
		}
		GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, M983(0x273), 2, (int)&param, 0, 0, 0);
	}
	break;
	case PKT645_SEM_983(0x1d6):
	{
		Cm::eErrorCode error = (Cm::eErrorCode)packet.Decode4();
		if (!IS_SUCCESSFUL(error))
		{
			GetActor(s_gatewayActor[1])
				<< MsgObject(NULL, M983(0x273), 0, error, 0, 0, 0);
			break;
		}
		_gateway::sTradeBuyParam param;
		param.type = 1;
		param.price = packet.Decode8();
		unsigned char count = packet.Decode1();
		param.items.reserve(count);
		for (unsigned char i = 0; i < count; i++)
		{
			_gateway::sTradeBuyBase base;
			packet.DecodeBuffer(&base.item, sizeof(sTradeItem));
			base.flag = packet.Decode1();
			packet.DecodeBuffer(&base.info, sizeof(sItemInfo));
			param.items.push_back(base);
		}
		GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, M983(0x273), 1, (int)&param, 0, 0, 0);
	}
	break;
	case PKT645_SEM_983(0x1d8):
	{
		Cm::eErrorCode error = (Cm::eErrorCode)packet.Decode4();
		if (!IS_SUCCESSFUL(error))
		{
			GetActor(s_gatewayActor[1])
				<< MsgObject(NULL, M983(0x273), 0, error, 0, 0, 0);
			break;
		}
		_gateway::sTradeMinusParam param;
		packet.DecodeStr(param.title);
		param.serial = packet.Decode4();
		unsigned char count = packet.Decode1();
		param.items.reserve(count);
		for (unsigned char i = 0; i < count; i++)
		{
			sTradeItem item;
			packet.DecodeBuffer(&item, sizeof(sTradeItem));
			param.items.push_back(item);
		}
		param.count = packet.Decode4();
		GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, M983(0x273), 3, (int)&param, 0, 0, 0);
	}
	break;
	case 0xec:
	{
		int code = packet.Decode4();
		sTradeItem item;
		unsigned char result = 0;
		if (code == 1)
		{
			result = packet.Decode1();
			__int64 pang = packet.Decode8();
			packet.DecodeBuffer(&item, sizeof(sTradeItem));
			if (result == 1 || result == 3) // 983 009cd6b7: o 3 da a mesma baixa do 1; aviso no chat so com 1
			{
				switch (item.dwTid >> 26)
				{
				case 6:
				{
					std::list<sItemInfo>& itemList = Doc()->m_myItemList;
					std::list<sItemInfo>::iterator it;
					for (it = itemList.begin(); it != itemList.end(); it++)
					{
						if (it->guid == item.dwGuid)
						{
							if (it->tid == item.dwTid)
							{
								if (it->Common[0] > item.iNum)
									it->Common[0] -= item.iNum;
								else if (it->Common[0] == item.iNum)
									itemList.erase(it);
							}
							break;
						}
					}
				}
				break;
				case 5:
				{
					std::list<sItemInfo>& itemList = Doc()->m_ballList;
					std::list<sItemInfo>::iterator it;
					for (it = itemList.begin(); it != itemList.end(); it++)
					{
						if (it->guid == item.dwGuid)
						{
							if (it->tid == item.dwTid)
							{
								if (it->Common[0] > item.iNum)
									it->Common[0] -= item.iNum;
								else if (it->Common[0] == item.iNum)
									itemList.erase(it);
							}
							break;
						}
					}
				}
				break;
				case 2:
					if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) &&
						IsUccClothes(item.dwTid))
					{
						std::list<sItemInfo>::iterator it;
						for (it = Doc()->m_uccItemList.begin();
							it != Doc()->m_uccItemList.end(); it++)
						{
							if (it->guid == item.dwGuid &&
								it->tid == item.dwTid &&
								strcmpi(it->UccIndex, item.UccIndex) == 0 &&
								it->Seq == item.Seq)
							{
								Doc()->m_uccItemList.erase(it);
								if (IsLocalContent((localContentType_t)0x6f))
									Doc()->DeleteCardToPartsAttach(item.dwGuid);
								break;
							}
						}
					}
					else
					{
						std::list<sItemInfo>& itemList = Doc()->m_partsList;
						std::list<sItemInfo>::iterator it;
						for (it = itemList.begin(); it != itemList.end(); it++)
						{
							if (it->guid == item.dwGuid)
							{
								if (it->tid == item.dwTid)
								{
									itemList.erase(it);
									if (IsLocalContent(
											(localContentType_t)0x6f))
										Doc()->DeleteCardToPartsAttach(
											item.dwGuid);
								}
								break;
							}
						}
					}
					break;
				case 4:
				{
					std::map<unsigned int, sItemInfo>& clubSetMap =
						Doc()->m_clubSetMap;
					std::map<unsigned int, sItemInfo>::iterator it;
					for (it = clubSetMap.begin(); it != clubSetMap.end(); it++)
					{
						if (it->second.guid == item.dwGuid)
						{
							if (it->second.tid == item.dwTid)
								clubSetMap.erase(it);
							break;
						}
					}
				}
				break;
				case 28:
				{
					std::list<sItemInfo>& itemList = Doc()->m_auxPartList;
					std::list<sItemInfo>::iterator it;
					for (it = itemList.begin(); it != itemList.end(); it++)
					{
						if (it->guid == item.dwGuid)
						{
							if (it->tid == item.dwTid)
							{
								if (it->Common[0] > item.iNum)
									it->Common[0] -= item.iNum;
								else if (it->Common[0] == item.iNum)
									itemList.erase(it);
							}
							break;
						}
					}
				}
				break;
				case 7:
				{
					std::map<unsigned int, sCaddieInfo>& caddieMap =
						Doc()->m_caddieMap;
					std::map<unsigned int, sCaddieInfo>::iterator it =
						caddieMap.find(item.dwGuid);
					if (it != caddieMap.end())
						caddieMap.erase(it);
				}
				break;
				case 31:
				{
					std::list<sCards>& cardList = Doc()->m_cardList;
					std::list<sCards>::iterator it;
					for (it = cardList.begin(); it != cardList.end(); it++)
					{
						if (it->tid == item.dwTid)
						{
							it->cardCount -= item.iNum;
							if (it->cardCount < 1)
								Doc()->m_cardList.erase(it);
							break;
						}
					}
				}
				break;
				}
				Doc()->m_myInfo.stat.i64Pang += pang;
				((RbSharedDocTradeIncome*)Doc())->AddTradeIncome(pang);
				if (RFLobbyDesktop::Instance983()) // 0047f920
					RFLobbyDesktop::Instance983()->Refresh983();
			}
			else if (result == 0)
			{
				unsigned char type = packet.Decode1();
				sItemInfo info;
				sCaddieInfo caddie = { 0 };
				sCards card = { 0 };
				switch (type)
				{
				case 1:
					packet.DecodeBuffer(&info, sizeof(sItemInfo));
					switch (info.tid >> 26)
					{
					case 28:
					{
						std::list<sItemInfo>& itemList = Doc()->m_auxPartList;
						std::list<sItemInfo>::iterator it;
						for (it = itemList.begin(); it != itemList.end(); it++)
						{
							if (it->guid == info.guid)
								break;
						}
						if (it != itemList.end())
							it->Common[0] += info.Common[0];
						else
							itemList.push_back(info);
					}
					break;
					case 6:
					{
						std::list<sItemInfo>& itemList = Doc()->m_myItemList;
						std::list<sItemInfo>::iterator it;
						for (it = itemList.begin(); it != itemList.end(); it++)
						{
							if (it->guid == info.guid)
								break;
						}
						if (it != itemList.end())
							it->Common[0] += info.Common[0];
						else
							itemList.push_back(info);
					}
					break;
					case 5:
					{
						std::list<sItemInfo>& itemList = Doc()->m_ballList;
						std::list<sItemInfo>::iterator it;
						for (it = itemList.begin(); it != itemList.end(); it++)
						{
							if (it->guid == info.guid)
								break;
						}
						if (it != itemList.end())
							it->Common[0] += info.Common[0];
						else
							itemList.push_back(info);
					}
					break;
					}
					break;
				case 2:
				case 3:
					packet.DecodeBuffer(&info, sizeof(sItemInfo));
					switch (info.tid >> 26)
					{
					case 2:
						if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) &&
							IsUccClothes(info.tid))
						{
							Doc()->m_uccItemList.push_back(info);
							UccManager()->AddClothes(1, info.guid, info.tid,
								info.UccIndex, false, false);
						}
						else
						{
							Doc()->AddMyPartsList(info);
						}
						break;
					case 4:
						Doc()->m_clubSetMap.insert(
							std::map<unsigned int, sItemInfo>::value_type(
								info.guid, info));
						break;
					case 5:
						Doc()->m_ballList.push_back(info);
						break;
					case 6:
						Doc()->m_myItemList.push_back(info);
						break;
					case 28:
						Doc()->m_auxPartList.push_back(info);
						break;
					}
					break;
				case 4:
					packet.DecodeBuffer(&caddie, sizeof(sCaddieInfo));
					Doc()->AddCaddie(&caddie);
					break;
				case 5:
					if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
					{
						packet.DecodeBuffer(&card, sizeof(sCards));
						std::list<sCards>& cardList = Doc()->m_cardList;
						bool bFound = false;
						std::list<sCards>::iterator it;
						for (it = cardList.begin(); it != cardList.end(); it++)
						{
							if (it->tid ==
								item.dwTid) // o original compara com o tid do sTradeItem
							{
								it->cardCount += card.cardCount;
								bFound = true;
								break;
							}
						}
						if (!bFound)
							cardList.push_back(card);
					}
					break;
				}
				Doc()->m_myInfo.stat.i64Pang = pang;
			}
		}
		RbTradeDlgSending* pDlg = (RbTradeDlgSending*)g_pFresh->GetManager()
									  ->GetDesktop()
									  ->FindChildByName("tradedlg");
		if (pDlg)
			pDlg->EnableSendingServer(false);
		else
			__rtti_obj = pDlg;
		if (result == 1)
		{
			IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(item.dwTid);
			if (pItem)
			{
				std::string msg;
				if (item.iNum > 1)
				{
					msg = MakeStr(
						K2L_Compatibility("%s\300\314(\260\241) %d\260\263 \306\307\270\305\265\307\276\372\275\300\264\317\264\331."),
						pItem->Name, item.iNum);
				}
				else
				{
					sItemInfo* pInfo = GetMyItemInfo(item.dwGuid);
					sUccClothes* pClothes =
						UccManager()->FindClothes(item.dwGuid);
					if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && pInfo &&
						(pInfo->status & 1) && pClothes &&
						IsUccClothes(item.dwTid))
						msg = MakeStr(
							K2L_Compatibility("%s\300\314(\260\241) \306\307\270\305\265\307\276\372\275\300\264\317\264\331."),
							pClothes->uccName);
					else
						msg = MakeStr(
							K2L_Compatibility("%s\300\314(\260\241) \306\307\270\305\265\307\276\372\275\300\264\317\264\331."),
							pItem->Name);
				}
				CChatMsg::Instance()->AddChatMsg(msg, 0xffff, true, false);
			}
			SetTradeResultCode(code, 0xe3, NULL);
		}
		else
		{
			SetTradeResultCode(code, 0xec, &item);
		}
	}
	break;
	case 0xed:
	{
		std::string nick;
		sTradeItem item;
		packet.DecodeStr(nick);
		unsigned long uid = packet.Decode4();
		packet.DecodeBuffer(&item, sizeof(item));
		if (uid == Doc()->m_tradeUID)
		{
			int state = packet.Decode4();
			if (state == 3)
				((RbSharedDocTrade*)Doc())->SetStateTrade((eStateTrade)state);
			FrWnd* pBuy = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
				"tradebuydlg");
			if (!pBuy)
				__rtti_obj = pBuy;
			else if (!*(bool*)((char*)pBuy +
						 0x1ec)) // FrTradeBuyDlg::m_bClosing
			{
				sTradeItem* pBuyItem = ((FrTradeBuyDlg*)pBuy)->GetBuyItem();
				if (pBuyItem && pBuyItem->iIndex == item.iIndex &&
					pBuyItem->dwGuid == item.dwGuid)
				{
					if (Doc()->GetSizeTradeItem() > 1)
					{
						if (pBuyItem->iNum == item.iNum)
							CChatMsg::Instance()->AddChatMsg(
								std::string(
									K2L_Compatibility("\274\261\305\303\307\317\275\305 \273\363\307\260\300\272 \306\307\270\305\265\307\276\372\275\300\264\317\264\331.")),
								0xffff, true, false);
						else
							CChatMsg::Instance()->AddChatMsg(
								std::string(
									K2L_Compatibility("\274\261\305\303\307\317\275\305 \273\363\307\260\300\307 \274\366\267\256\300\314 \272\257\260\346\265\307\276\372\275\300\264\317\264\331.")),
								0xffff, true, false);
					}
					pBuy->Close(true);
					FrWnd* pNotify =
						g_pFresh->GetManager()->GetDesktop()->FindChildByName(
							"notify_okcancel");
					if (pNotify)
						pNotify->Close(true);
					else
						__rtti_obj = pNotify;
				}
			}
			RbTradeDlg* pDlg = (RbTradeDlg*)g_pFresh->GetManager()
								   ->GetDesktop()
								   ->FindChildByName("tradedlg");
			if (pDlg)
				pDlg->SetListTradeItem(true);
			else
				__rtti_obj = pDlg;
			Doc()->MinusListTradeItem(item);
			if (pDlg)
				pDlg->SetListTradeItem(false);
			if (Doc()->m_stateTrade == 3)
			{
				if (strcmp(nick.c_str(), MyNick()) != 0)
				{
					if (pDlg)
						pDlg->Close(true);
				}
				else if (pDlg)
				{
					((RbTradeDlgEB*)pDlg)->ShowStaticOutOfStock(true);
					((RbTradeDlgEB*)pDlg)->EnableHideBn(false);
				}
				else
				{
					RbTradeDlg* pNew = (RbTradeDlg*)RbCreateTradeDlg(
						g_pFresh->GetManager(), this, "tradedlg", NULL);
					if (pNew)
					{
						((FrForm*)pNew)->Open(NULL, 0xc1);
						pNew->OpenTradeShop(Doc()->m_tradeUID);
						pNew->SetFadeout(false);
					}
				}
				CChatMsg::Instance()->AddChatMsg(
					std::string(
						K2L_Compatibility("\270\360\265\347 \273\363\307\260\300\314 \306\307\270\305\265\307\276\372\275\300\264\317\264\331.")),
					0xffff, true, false);
			}
		}
		if (Doc()->m_stateTrade == 3)
			GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0xe8), (int)nick.c_str(), 3, 0, 0, 0);
		return 1;
	}
	break;
	case 0xee:
	{
		unsigned char result = packet.Decode1();
		SetTradeResultCode(0x10, result, NULL);
		if (result)
		{
			std::string msg(
				K2L_Compatibility("\260\263\300\316\273\363\301\241 \300\314\277\353 \301\246\300\347 \301\337\300\324\264\317\264\331. \260\263\300\316\273\363\301\241\300\273 \300\314\277\353\307\317\275\307 \274\366 \276\370\275\300\264\317\264\331."));
			CChatMsg::Instance()->AddChatMsg(msg, 0xff37, true, false);
		}
		else
		{
			std::string msg(
				K2L_Compatibility("\300\314\277\353 \301\246\300\347\301\337\300\316 \260\263\300\316\273\363\301\241\300\324\264\317\264\331. \260\263\300\316\273\363\301\241\300\273 \300\314\277\353\307\317\275\307 \274\366 \276\370\275\300\264\317\264\331."));
			CChatMsg::Instance()->AddChatMsg(msg, 0xff37, true, false);
		}
	}
	break;
	case PKT645_SEM_983(0xed):
	{
		std::string msg(
			"\301\266\260\307\300\314 \277\303\271\331\270\243\301\366 \276\312\275\300\264\317\264\331.");
		sRecycleItem item;
		memset(&item, 0, sizeof(sRecycleItem));
		std::list<sRecycleItem> itemList;
		itemList.clear();
		short count = 0;
		int result = packet.Decode4();
		switch (result)
		{
		case 0:
		{
			count = packet.Decode2();
			unsigned char num = packet.Decode1();
			while (num)
			{
				packet.DecodeBuffer(&item, 0x50);
				itemList.push_back(item);
				num--;
			}
		}
		break;
		case 2:
			msg =
				"\301\270\300\347\307\317\301\366 \276\312\300\272 \276\306\300\314\305\333\300\324\264\317\264\331.";
			break;
		case 3:
			msg =
				"\276\306\300\314\305\333 \272\270\300\257 \260\263\274\366\260\241 20,000\260\263\300\314\271\307\267\316 \301\266\307\325\300\273 \307\322\274\366\260\241 \276\370\275\300\264\317\264\331.";
			break;
		case 4:
			msg =
				"\300\314\271\314 \274\322\300\257\307\321 \276\306\300\314\305\333\300\324\264\317\264\331.";
			break;
		default:
			msg =
				"\301\266\260\307\300\314 \277\303\271\331\270\243\301\366 \276\312\275\300\264\317\264\331.";
			break;
		}
		RbRecycleDlg* pDlg = (RbRecycleDlg*)g_pFresh->GetManager()
								 ->GetDesktop()
								 ->FindChildByName("recycledlg");
		if (pDlg == NULL)
		{
			__rtti_obj = pDlg;
		}
		else if (result == 0)
		{
			pDlg->SetResultRecycleItem(true, itemList, count);
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0xe1), 0, 0, 0, 0, 0));
		}
		else
		{
			pDlg->SetResultRecycleItem(false, itemList, count);
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x23), (int)msg.c_str(), 0, 0, 0, 0));
		}
	}
		return 1;
	case 0x6b:
	{
		switch (packet.Decode1())
		{
		case 4:
			switch (packet.Decode1())
			{
			case 0:
			{
				sCharacterInfo info;
				packet.DecodeBuffer(&info, sizeof(info));
				Doc()->m_charMap[info.guid] = info;
				Doc()->BuildMyPartTidList(info);
				jp983_rastro("A803 srv 6b/4/0 guid=%08x tid=%08x guidChar=%08x", info.guid, info.tid,
					Doc()->m_myInfo.userEquip.guidChar);
				CTask* pTask = AfxGetTask();
				__rtti_obj = pTask;
				if (pTask && IS_KINDOF(CLobbyTask, pTask)) // 983 009c84c5: RTTI 00f030b0 = CLobbyTask (645: CRealMyRoomTask)
					pTask->SendMsgToMainActor(
						MsgObject(NULL, 0x2eb /* 983 009c84e1; M983(0x26c) está em aberto na tabela */, 0, 0, 0, 0, 0));
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x49),
					Doc()->m_myInfo.info.dwGuid, (int)&info, 1 /* 983 009c8512 */, 0, 0));
			}
			break;
			case 1:
			case 3:
				break;
			case 2:
			{
				unsigned long slots[10] = { 0 };
				packet.DecodeBuffer(slots, sizeof(slots));
				memcpy(Doc()->m_myInfo.userEquip.tidItemSlot, slots,
					sizeof(slots));
			}
			break;
			case 5:
			{
				Doc()->m_myInfo.userEquip.guidChar = packet.Decode4();
				std::map<unsigned int, sCharacterInfo>::iterator it =
					Doc()->m_charMap.find(Doc()->m_myInfo.userEquip.guidChar);
				jp983_rastro("A803 srv 6b/4/5 guidChar=%08x tid=%08x", Doc()->m_myInfo.userEquip.guidChar,
					it != Doc()->m_charMap.end() ? it->second.tid : 0);
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x49),
					Doc()->m_myInfo.info.dwGuid, (int)&it->second, 0, 0, 0));
			}
			break;
			case 4:
			{
				packet.DecodeBuffer(Doc()->m_myInfo.userEquip.tidSkin,
					sizeof(Doc()->m_myInfo.userEquip.tidSkin));
				std::map<unsigned long, sUserInfoTime>::iterator it;
				if (Doc()->FindUserInfoTimeGUID(MyGuid(false), it))
					memcpy(it->second.info.userEquip.tidSkin,
						Doc()->m_myInfo.userEquip.tidSkin,
						sizeof(it->second.info.userEquip.tidSkin));
				// 983 009c8696 (novo): avisa o ator "Lobby" (mensagem 0x276), achando ou não o registro
				AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x276, 0, 0, 0, 0, 0);
			}
			break;
			case 8:
			{
				sMascotInfo mascot;
				packet.DecodeBuffer(&mascot, sizeof(mascot));
				if (Doc()->m_mascotMap.find(mascot.guid) !=
					Doc()->m_mascotMap.end())
					Doc()->m_mascotMap[mascot.guid] = mascot;
			}
			break;
			case 9:
			{
				// 983 009c8761 (novo): guid do personagem + 4 valores -> sCharacterInfo::cutIn (+0x1b8)
				unsigned long guid = packet.Decode4();
				unsigned long cutIn[4] = { 0 };
				packet.DecodeBuffer(cutIn, sizeof(cutIn));
				sCharacterInfo* pChar = Doc()->FindChar983(guid);
				if (pChar)
					memcpy(pChar->cutIn, cutIn, sizeof(cutIn));
			}
			break;
			case 10:
			{
				// 983 009c87e1 (novo): 8 bytes -> sUserEquip::poster (doc +0xa92)
				unsigned long poster[2] = { 0 };
				packet.DecodeBuffer(poster, sizeof(poster));
				Doc()->m_myInfo.userEquip.poster[0] = poster[0];
				Doc()->m_myInfo.userEquip.poster[1] = poster[1];
			}
			break;
			default:
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
					(int)K2L_Compatibility("\276\313 \274\366 \276\370\264\302 \270\355\267\311 \304\332\265\345"),
					0, 0, 0, 0));
				break;
			}
			break;
		case 1:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("DB \277\241\267\257\267\316 \275\307\306\320\307\337\275\300\264\317\264\331"),
				0, 0, 0, 0));
			break;
		case 5:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\300\345\302\370\300\273 \275\307\306\320\307\337\275\300\264\317\264\331"),
				0, 0, 0, 0));
			break;
		case 6:
		{
			// 983 009c88b3 (novo): código de erro ao equipar; 0 = nada a mostrar
			unsigned long errCode = packet.Decode4();
			if (errCode)
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
					(int)MakeStr(K2L_Compatibility(
						"\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbd\xbd\xb7\xd4\xbf\xa1 \xc0\xe5\xc2\xf8 \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9.\n\nerrCode(%d)"),
						errCode),
					0, 0, 0, 0));
		}
		break;
		case 0:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\300\337\270\370\265\310 \276\306\300\314\305\333 \304\332\265\345\300\324\264\317\264\331"),
				0, 0, 0, 0));
			break;
		default:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\276\313 \274\366 \276\370\264\302 \260\341\260\372 \304\332\265\345"),
				0, 0, 0, 0));
			break;
		}
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
	}
	break;
	case 0x167:
	{
		int days = packet.Decode4();
		char msg[1024] = { 0 };
		if (days > 0)
		{
			wsprintfA(msg,
				"%d\260\263\300\307 \306\355\301\366\260\241 \272\270\260\374 \261\342\260\243\300\273 \303\312\260\372\307\317\277\264\275\300\264\317\264\331.\n\267\316\261\327\276\306\277\364 \310\244\300\272 \274\255\271\366 \300\314\265\277\300\273 \307\317\261\342 \300\374\277\241 \261\342\260\243\300\314 \301\366\263\255 \270\336\300\317\277\241 \306\367\307\324\265\310 \276\306\300\314\305\333\300\273\n\277\305\260\334\301\326\275\303\261\342 \271\331\266\370\264\317\264\331.\n",
				days);
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x23), (int)msg, 0, 0, 0, 0));
		}
	}
	break;
	case 0xf1:
	{
		CProjectG::Instance()->m_bMsnOffline = true;
		Doc()->m_guildMemberList.clear();
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x68), 0, 0, 0, 0, 0));
		if (AfxGetTask()->IsCurrentActor983("shopmain")) // 983 009ce837
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0xc5), 0, 0, 0, 0, 0));
		wchar_t wszUnused[0x200] = { 0 };
		// 983 009ce8a4: só o teste do ir com amigo (0x54) e sempre o pacote 0xb4 com canal/sala; o caminho do 645
		// (0x5a/0x5b: entrar no canal pelo pacote 4 e avisar o messenger) não existe no 983.
		if (!IsLocalContent(S4_GOWITH_FRIEND /*983: era 0x53 (645); medido 009c20a0*/))
			break;
		unsigned char channel = 0xff;
		unsigned short room = 0xffff;
		unsigned long roomKey = -1;
		if (Doc()->GetDirectMoveRoomInfo(&channel, &room, &roomKey) == true)
		{
			WSendPacket send((enumClientPacket)0xb4);
			send.Encode1(channel);
			send.Encode2(room);
			send.Send(TO_GAME);
		}
		Doc()->ClearDirectMoveRoomInfo();
		Doc()->SetGoWithModeEnd();
		return 1;
	}
	break;
	case 0xf8:
		switch (packet.Decode1())
		{
		case 0:
		{
			short min = packet.Decode2();
			CNoticeBoard::Instance()->AddNotice(
				MakeStr(
					K2L_Compatibility("[\260\370\301\366] \300\314 \274\255\271\366\264\302 %d\272\320\310\304\277\241 \300\332\265\277 \301\276\267\341\265\311 \277\271\301\244\300\324\264\317\264\331."),
					min),
				1, false);
		}
		break;
		case 1:
		{
			std::string msg(MakeStr(
				K2L_Compatibility("\276\313\270\262 : \274\255\271\366\260\241 %d\303\312\310\304\277\241 \300\332\265\277 \301\276\267\341\265\313\264\317\264\331."),
				(short)packet.Decode2()));
			CChatMsg::Instance()->AddChatMsg(msg, 0xffff7878, false, false);
		}
		break;
		}
		break;
	case 0x1389: // 983: era 0x1f5 no 645; caso 009d72d9 (ranking saiu da faixa: 0x1389-0x138c)
	{
		if (RANKINGDLG() == NULL)
			break;

		unsigned char result = packet.Decode1();
		unsigned char type0 = packet.Decode1();
		unsigned char type1 = packet.Decode1();
		unsigned char type2 = packet.Decode1();
		unsigned char type3 = packet.Decode1();
		int value0 = packet.Decode4();
		int value1 = packet.Decode4();
		RbRankingDlg::SetRankTypes(type0, type1, type2, type3, value0, value1);
		if (result != 0)
		{
			((RbRankingDlg*)RANKINGDLG())
				->SetBottomMsg(
					K2L_Compatibility("\264\365 \300\314\273\363 \267\251\305\267 \261\342\267\317\300\314 \276\370\275\300\264\317\264\331."));
			if (result == 2)
				break;
			RKLIST983(RANKINGDLG())
				->clear();
			*(int*)RKMY983(RANKINGDLG()) = -1;
			((RbRankingDlg*)RANKINGDLG())->ClearRankList();
			((RbRankingDlg*)RANKINGDLG())->UpdateMyInfo();
			return 1;
		}
		RKLIST983(RANKINGDLG())
			->clear();
		unsigned short count = packet.Decode2();
		if (count > 0u)
		{
			int n = count;
			do
			{
				ranktype::sListInfo info;
				info.rank = packet.Decode4();
				info.guid = packet.Decode4();
				info.score = packet.Decode4();
				info.extra = packet.Decode4();
				info.level = packet.Decode2();
				info.state = packet.Decode1();
				info.nick = packet.DecodeStr();
				info.guild = packet.DecodeStr();
				RKLIST983(RANKINGDLG())
					->push_back(info);
			} while (--n);
		}
		ranktype::sListInfo* pMy =
			RKMY983(RANKINGDLG());
		unsigned char myResult = packet.Decode1();
		if (myResult == 0)
		{
			pMy->rank = packet.Decode4();
			pMy->guid = packet.Decode4();
			pMy->score = packet.Decode4();
			pMy->extra = packet.Decode4();
			pMy->level = packet.Decode2();
			pMy->state = packet.Decode1();
			pMy->nick = packet.DecodeStr();
			pMy->guild = packet.DecodeStr();
		}
		else if (myResult == 1)
			pMy->rank = -1;
		((RbRankingDlg*)RANKINGDLG())->UpdateRankList();
		((RbRankingDlg*)RANKINGDLG())->UpdateMyInfo();
		return 1;
	}
	case 0x138a: // 983: era 0x1f6 no 645; caso 009d7805
	{
		if (RANKINGDLG() == NULL)
			break;

		unsigned char result = packet.Decode1();
		if (result == 2)
		{
			CRankingInfo::Instance()->OnNotify(
				"\300\257\300\372 \301\244\272\270\270\246 \260\241\301\256\277\303 \274\366 \276\370\275\300\264\317\264\331.",
				2);
			break;
		}
		if (result == 1)
		{
			((RbRankingDlg*)RANKINGDLG())
				->SetBottomMsg(
					K2L_Compatibility("\274\370\300\247\263\273\277\241 \301\244\272\270\260\241 \276\370\275\300\264\317\264\331."));
			break;
		}

		packet.DecodeBuffer(RKUSER983(RANKINGDLG()), 0x27c); // 983: sizeof(ranktype::sRankUserInfo) = 0x27c (push em 009d7865); era 0x22c
		if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
			RequestUccInfo(
				*(ranktype::sRankUserInfo*)RKUSER983(RANKINGDLG()));
		((RbRankingDlg*)RANKINGDLG())->UpdateUserInfo();
	}
	break;
	case 0x138b: // 983: era 0x1f7 no 645; caso 009d77e6
		CRankingInfo::Instance()->OnNotify(
			K2L_Compatibility("\274\255\271\366\277\241\274\255 \267\251\305\267 \301\244\272\270\270\246 \264\331\275\303 \301\244\267\304\307\317\260\355 \300\326\275\300\264\317\264\331."),
			2);
		break;
	case 0x138c: // 983: era 0x1f8 no 645; caso 009d75f4
	{
		RbRankingDlg* pDlg = (RbRankingDlg*)RANKINGDLG();
		if (pDlg == NULL)
			break;

		if (packet.Decode1())
		{
			*(int*)RKMY983(pDlg) = -1;
			CRankingInfo::Instance()->OnNotify(
				K2L_Compatibility("\274\370\300\247\263\273\277\241 \301\244\272\270\260\241 \276\370\275\300\264\317\264\331."),
				2);
			break;
		}
		unsigned char type0 = packet.Decode1();
		unsigned char type1 = packet.Decode1();
		unsigned char type2 = packet.Decode1();
		unsigned char type3 = packet.Decode1();
		int value0 = packet.Decode4();
		int value1 = packet.Decode4();
		RbRankingDlg::SetRankTypes(type0, type1, type2, type3, value0, value1);
		RKLIST983(RANKINGDLG())
			->clear();
		unsigned short count = packet.Decode2();
		if (count > 0u)
		{
			int n = count;
			do
			{
				ranktype::sListInfo info;
				info.rank = packet.Decode4();
				info.guid = packet.Decode4();
				info.score = packet.Decode4();
				info.extra = packet.Decode4();
				info.level = packet.Decode2();
				info.state = packet.Decode1();
				info.nick = packet.DecodeStr();
				info.guild = packet.DecodeStr();
				RKLIST983(RANKINGDLG())
					->push_back(info);
			} while (--n);
		}
		unsigned short page = packet.Decode2();
		((RbRankingDlg*)RANKINGDLG())->UpdateRankList();
		((RbRankingDlg*)RANKINGDLG())->SelectRankList(page);
	}
	break;
	case 0xfe:
	{
		unsigned long result = packet.Decode4();
		if (g_pPointEventDlg)
			g_pPointEventDlg->ResultMessage(result);
	}
	break;
	case 0xfd:
	{
		unsigned long point = packet.Decode4();
		Doc()->m_myInfo.info.dwPointPointEvent = point;
	}
	break;
	case 0xff:
	{
		unsigned long count = packet.Decode4();
		unsigned long totals[10] = { 0 };
		packet.DecodeBuffer(totals, count * 4);
		for (unsigned long i = 0; i < count; i++)
			g_CommodityStatusManager.UpdateTotalCount((unsigned short)i,
				totals[i]);
	}
	break;
	case 0x100:
	{
		unsigned long count = packet.Decode4();
		CCommodityStatusManager::sCommodityStatus
			items[CCommodityStatusManager::MAX_COMMODITY_COUNT] = { 0 };
		packet.DecodeBuffer(items,
			count * sizeof(CCommodityStatusManager::sCommodityStatus));
		for (unsigned long i = 0; i < count; i++)
		{
			g_CommodityStatusManager.UpdateStatus(i, items[i].status);
			g_CommodityStatusManager.UpdateMyCount(i, items[i].myCount);
		}
	}
	break;
	case 0x10d:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned char result = packet.Decode1();
		CGiftBoxEvent* pEvent = NULL;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0x1e,
			pEvent);
		if (result == 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\274\261\271\260\273\363\300\332 \277\255\261\342\277\241 \275\307\306\320\307\337\275\300\264\317\264\331."),
				0, 0, 0, 0));
			break;
		}
		if (pEvent->GetOpenedBoxTID())
		{
			_RemoveWareHouseItem(pEvent->GetOpenedBoxTID(),
				AfxGetTask()->GetMainActor(), 1); // PKT (frente C, lote 13): 983 009ceb9b - CTask+0x30 (era "RealMyRoom")
			struct
			{
				int type;
				int value;
				int unused8; // 983 009cec3b: so zerado antes do DecodeBuffer, sem leitura
				int pang;
				int unused10; // 983 009cec47: idem
			} reward = { 0 };
			packet.DecodeBuffer(&reward, 0x14);
			std::string msg;
			// PKT (frente C, lote 13): 983 009cec50 - so os tipos 3 e 4 e o resto; os tipos 5/6 do 645 (refresco do
			// presente do my room) nao existem no 983 e caem no texto do resto
			switch (reward.type)
			{
#if 0 // 645
			case 5:
				_ReFreshMyRoomGift(false, "@HanBit");
				if (pEvent->GetOpenedBoxTID() == 0x1a00003e)
					msg =
						"\306\316\276\337 3\301\326\263\342 \261\342\263\344 \276\306\301\356\305\330 10\260\263\270\246 \274\261\271\260\307\324\277\241\274\255 \310\256\300\316\307\317\274\274\277\344.";
				break;
			case 6:
				_ReFreshMyRoomGift(false, "@HanBit");
				msg =
					"\270\266\275\272\304\332\306\256(1\300\317)\264\302 \274\261\271\260\307\324\277\241\274\255 \310\256\300\316\307\317\274\274\277\344.";
				break;
#endif
			case 4:
				msg =
					K2L_Compatibility("\304\355\306\371\300\273 \271\336\300\270\274\314\275\300\264\317\264\331. \304\355\306\371\307\324\277\241\274\255 \310\256\300\316\307\317\274\274\277\344.");
				break;
			case 3:
			{
				char buf[0x40] = { 0 };
				sprintf(buf,
					"%d \306\316\300\273 \271\336\300\270\274\314\275\300\264\317\264\331. ",
					reward.value);
				msg = buf;
				Doc()->m_myInfo.stat.i64Pang = (unsigned int)reward.pang;
				if (RFLobbyDesktop::Instance983()) // 983 009ced0b: 0047f920
					RFLobbyDesktop::Instance983()->Refresh983();
			}
			break;
			default:
				msg =
					K2L_Compatibility("\276\306\300\314\305\333\300\273 \271\336\300\270\274\314\275\300\264\317\264\331.");
				break;
			}
			pEvent->SetOpenedBoxTID(0);
			AfxGetTask()->SendMsgToMainActor( // 983 009ced3b: o texto vai por K2L_Compatibility
				MsgObject(NULL, M983(0x26), (int)K2L_Compatibility(msg.c_str()), reward.value, 0, 0, 0));
			return 1;
		}
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\274\261\271\260\273\363\300\332 \277\255\261\342\277\241 \275\307\306\320\307\337\275\300\264\317\264\331."),
			0, 0, 0, 0));
	}
	break;
	case 0x109:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (packet.Decode1() == 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\274\274\271\350\265\267\272\300\305\365 \277\255\261\342\277\241 \275\307\306\320\307\337\275\300\264\317\264\331."),
				0, 0, 0, 0));
			break;
		}
		_RemoveWareHouseItem(0x1a00003b, AfxGetTask()->GetMainActor(), // PKT (frente C, lote 13): 983 009cee25
			1);
		struct
		{
			int unused0;
			unsigned long tid;
			int unused8;
			int value;
			int unused10;
		} info = { 0 };
		packet.DecodeBuffer(&info, sizeof(info));
		if (ItemManager()->FindCommonItem(info.tid))
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x26),
				(int)K2L_Compatibility("\276\306\300\314\305\333\300\273 \271\336\300\270\274\314\275\300\264\317\264\331."),
				info.tid, 0, 0, 0));
		}
		else
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x26),
				(int)K2L_Compatibility("\306\316\300\273 \271\336\300\270\274\314\275\300\264\317\264\331."),
				info.tid, 0, 0, 0));
			// 983 009ceeb7: o pang so muda no ramo sem item comum (o 645 mudava nos dois)
			if (info.tid != 0)
				Doc()->m_myInfo.stat.i64Pang = info.value;
		}
		return 1;
	}
	case 0xf9:
	{
		sGameServerInfo info;
		packet.DecodeBuffer(&info, sizeof(info));
		Doc()->m_curGameServer = info;
	}
	break;
	case 0xd0:
	{
		unsigned char mode = packet.Decode1();
		int total = (unsigned short)packet.Decode2();
		int count = (unsigned short)packet.Decode2();
		for (int i = 0; i < count; i++)
		{
			unsigned long guid = packet.Decode4();
			Doc()->m_refreshGuidList.push_back(guid);
			if (mode == 2 && Doc())
			{
				unsigned short num = packet.Decode2();
				Doc()->m_refreshCountList.push_back(num);
			}
		}
		if (total == count)
		{
			Doc()->RefreshItemListFromGuidList(mode);
			// PKT (frente C, lote 13): 983 009cf00b - CLobbyTask (RTTI 00f030b0) e ator "Lobby" (645: CRealMyRoomTask/"RealMyRoom")
			if (mode == 2 && IS_KINDOF(CLobbyTask, AfxGetTask()))
				AfxGetTask()->GetActor("Lobby")
					<< MsgObject(NULL, M983(0x67), 0, 0, 0, 0, 0);
		}
	}
	break;
	case 0xc5:
	{
		unsigned char result = packet.Decode1();
		unsigned long tid = packet.Decode4();
		int count = packet.Decode4();
		IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(tid);
		if (result == 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)MakeStr(
					K2L_Compatibility("%s \276\306\300\314\305\333\300\273 \273\350\301\246\307\317\301\366 \270\370\307\317\277\264\275\300\264\317\264\331.."),
					pItem ? pItem->Name : NULL),
				0, 0, 0, 0));
			break;
		}
		switch (tid >> 26)
		{
		case 6:
		{
			packet.Decode4();
			std::list<sItemInfo>::iterator it;
			for (it = Doc()->m_myItemList.begin();
				it != Doc()->m_myItemList.end(); it++)
			{
				if (it->tid == tid)
				{
					it->Common[0] -= (short)count;
					if (it->Common[0] < 1)
						Doc()->m_myItemList.erase(it);
					break;
				}
			}
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)MakeStr(
					// 983 009cf255: texto de 00d3fff4 ("... foram apagados") por K2L_Compatibility
					K2L_Compatibility("%d\260\263\300\307 %s \276\306\300\314\305\333\300\314 \273\350\301\246\265\307\276\372\275\300\264\317\264\331."),
					count, pItem ? pItem->Name : NULL),
				0, 0, 0, 0));
		}
		break;
		case 0xe:
		{
			std::list<sItemInfo>::iterator it;
			for (it = Doc()->m_cutinList.begin();
				it != Doc()->m_cutinList.end(); it++)
			{
				if (it->tid == tid)
				{
					Doc()->m_cutinList.erase(it);
					break;
				}
			}
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)MakeStr(
					K2L_Compatibility("%s \276\306\300\314\305\333\300\314 \273\350\301\246\265\307\276\372\275\300\264\317\264\331."),
					pItem ? pItem->Name : NULL),
				0, 0, 0, 0));
		}
		break;
		}
		// PKT (frente C, lote 13): 983 009cf2a6 - CLobbyTask (RTTI 00f030b0) e ator "Lobby" (645: CRealMyRoomTask/"RealMyRoom")
		if (IS_KINDOF(CLobbyTask, AfxGetTask()))
			GetActor("Lobby")
				<< MsgObject(NULL, M983(0xdd), tid, count, 0, 0, 0);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
	}
	break;
	case PKT645_SEM_983(0x15e):
	{
		Doc()->m_mailList.clear();
		static sMailInfoBrief mail;
		unsigned long count = packet.Decode4();
		for (unsigned long i = 0; i < count; i++)
		{
			packet.DecodeBuffer(&mail, sizeof(mail));
			Doc()->m_mailList.push_back(mail);
		}
		if (!Doc()->m_mailList.empty())
		{
			Doc()->m_bRefreshCamera = true;
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0xaa), 1, 0, 0, 0, 0));
		}
	}
	break;
	// PKT (lote 54): 0x10a nao tem caso no 983 (009d78a7, return 1): ver a lista antes do default
	case 0x10b:
	{
		short usableBonus = packet.Decode4();
		short remainedBonus = packet.Decode4();
		packet.Decode4();
		Doc()->SetBsUsableBonusTimes(usableBonus);
		Doc()->SetBsRemainedBonusTimes(remainedBonus);
		switch (Doc()->m_pendingMenu)
		{
		case 1:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x200), 0, 0, 0, 0, 0));
			Doc()->m_pendingMenu = 0;
			break;
		case 2:
			if (IS_KINDOF(CLobbyTask, AfxGetTask()))
			{
				AfxGetTask()->GetActor("Lobby")
					<< MsgObject(NULL, M983(0x201), 0, 0, 0, 0, 0);
			}
			Doc()->m_pendingMenu = 0; // 983 009cf3d6: zera mesmo fora do lobby
			break;
		}
	}
	break;
	case 0x110:
		switch (packet.Decode4())
		{
		case 0x1a: // 983 (009cf44f): le e descarta um Decode4
			packet.Decode4();
			break;
		case 0:
		{
			if (IsAdministrator(0xffffffff) != 0)
				break;
			WSendPacket send(0x21);
			send.EncodeStr(MyNick());
			send.Encode4(MyUID());
			send.Send(TO_MSN);
		}
		break;
		case 2:
		{
			if (IsAdministrator(0xffffffff) == 1)
				break;
			int count = packet.Decode4();
			std::list<sPrize*> prizeList;
			for (; count > 0; count--)
			{
				sPrize* pPrize = new sPrize;
				memset(pPrize, 0, sizeof(sPrize));
				packet.DecodeBuffer(pPrize, sizeof(sPrize));
				prizeList.push_back(pPrize);
			}
			char* text = new char[0x200];
			memset(text, 0, 0x200);
			int index = 0;
			std::list<sPrize*>::iterator it;
			for (it = prizeList.begin(); it != prizeList.end(); ++it)
			{
				sPrize* pPrize = *it;
				if (pPrize)
				{
					char name[0x40] = { 0 };
					char line[0x100] = { 0 };
					index++;
					IFF_ITEM_COMMON* pItem =
						ItemManager()->FindCommonItem(pPrize->tid);
					strcpy(name, pItem->Name);
					if (pPrize->type == 0)
					{
						if (pPrize->rankFrom == pPrize->rankTo)
							sprintf(line, K2L_Compatibility("%2d. %d\265\356 %s %d\260\263\012"),
								index, pPrize->rankFrom, name, pPrize->count);
						else
							sprintf(line,
								K2L_Compatibility("%2d. %d\265\356\272\316\305\315 %d\265\356 %s %d\260\263\012"),
								index, pPrize->rankFrom, pPrize->rankTo, name,
								pPrize->count);
					}
					else
					{
						char kind[0x20] = { 0 };
						switch (pPrize->type)
						{
						case 1:
							strcpy(kind,
								K2L_Compatibility("\272\243\275\272\306\256 \275\272\307\307\264\365"));
							break;
						case 2:
							strcpy(kind,
								K2L_Compatibility("\272\243\275\272\306\256 \267\257\263\312"));
							break;
						case 5:
							strcpy(kind,
								K2L_Compatibility("\272\243\275\272\306\256 \270\256\304\277\271\366\270\256"));
							break;
						case 6:
							strcpy(kind, K2L_Compatibility("\307\340\277\356\273\363"));
							break;
						case 4:
							strcpy(kind,
								K2L_Compatibility("\272\243\275\272\306\256 \267\325\306\333\305\315"));
							break;
						case 3:
							strcpy(kind,
								K2L_Compatibility("\272\243\275\272\306\256 \304\250\300\316"));
							break;
						}
						sprintf(line, K2L_Compatibility("%2d. %s %s %d\260\263\012"), index, kind,
							name, pPrize->count);
					}
					strcat(text, line);
				}
			}
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetTimeLimit(30.0f);
			pForm->SetMessage(text, false);
			pForm->Open(NULL, 0);
			for (it = prizeList.begin(); it != prizeList.end(); ++it)
			{
				if (*it)
				{
					delete *it;
					*it = NULL;
				}
			}
			prizeList.clear();
		}
		break;
		}
		break;
	case 0x10e:
	{
		sUserMatchHistory history[5];
		packet.DecodeBuffer(history, sizeof(history));
		CMatchHistory::Instance()->SetMatchHistory(history);
	}
	break;
	case 0x111:
	{
		unsigned long flagBlock = packet.Decode4();
		Doc()->SetFlagBlock(flagBlock);
		if (Doc()->m_myInfo.info.dwFlagBlock & 0x10)
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetTimeLimit(12.0f);
			pForm->SetMessage(
				K2L_Compatibility("\307\366\300\347 \300\374\261\244\306\307 \300\314\277\353 \301\246\300\347\301\337\300\324\264\317\264\331. "
				"\300\374\261\244\306\307\300\273 \273\347\277\353\307\317\275\307 \274\366 \276\370\275\300\264\317\264\331."),
				false);
			pForm->Open(NULL, 3);
		}
	}
	break;
	case 0x10f:
	{
		// 983 009cf98a: resultado (4 bytes) na frente; só com 0 vêm os 2 bytes (leitor 009b9020) e a mensagem vai ao ator principal
		if (packet.Decode4() == 0)
		{
			sLevelUpDone done;
			done.bDone = packet.Decode1();
			done.bLevel = packet.Decode1();
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x12d), (int)&done, 0, 0, 0, 0));
		}
	}
	break;
	case 0x113:
		if (IsLocalContent((localContentType_t)0x2f /*983: era 0x2e; interpolado*/))
		{
			IActor* pLobby = AfxGetTask()->GetActor("Lobby");
			if (pLobby)
				pLobby << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);
			CIntrusion::Instance()->PacketAnalysis(packet);
		}
		break;
	case 0x10c:
		g_iRecvDisconCode = packet.Decode4();
		break;
	case 0x112:
		if (packet.Decode1() != 0)
		{
			bool bAd = IsLocalContent((localContentType_t)0x16 /*983: era 0x15; medido*/) != 0;
			Doc()->m_bToppageAd = bAd;
		}
		else
			Doc()->m_bToppageAd = false;
		break;
	case 0x116:
	{
		// 983 009cfb3d (novo): lista de pedidos de entrada na guilda - vetor de CSharedDoc+0x10fe8 (elemento de 0x37)
		// o cabeçalho (frente A) ainda declara os dois vetores como std::vector<int> (tipo provisório; pedido C -> A)
		C_ASSERT_VC10(offsetof(CSharedDoc, m_guildJoinList983) == 0x10fe8);
		std::vector<sGuildJoinEntry983>& joinList = Doc()->m_guildJoinList983;
		joinList.clear();
		sGuildJoinEntry983 entry;
		for (int count = packet.Decode2(); count > 0; count--)
		{
			packet.DecodeBuffer(&entry, sizeof(entry));
			joinList.push_back(entry);
		}
		// 009b71f0: refaz a lista da janela de FrMessengerDlg+0x18c (FrGuildManageDlg, 008fafc0)
		// (m_pGuildManageDlg983 é protegido; offsetof medido pelo compilador = 0x18c, tools/pkt/mede.py)
		FrMessengerDlg* pMsnDlg = CMessengerInfo::Instance()->GetMessengerDlg();
		FrGuildManageDlg* pManageDlg = pMsnDlg ? MSNGUILDMANAGE983(pMsnDlg) : NULL;
		if (pManageDlg)
			pManageDlg->RefreshList983();
	}
	break;
	case 0x117:
	{
		// 983 009cfba2 (novo): resposta do pedido 0x16b; com 1 devolve o dword ao mensageiro, senão avisa (009b9470)
		unsigned long result = packet.Decode4();
		if (result == 1)
		{
			unsigned long value = packet.Decode4();
			WSendPacket send((enumClientPacket)0x16b);
			send.Encode4(value);
			send.Send(TO_MSN);
		}
		else
			GuildInviteNotify983(result);
	}
	break;
	case 0x118:
	{
		// 983 009cfc0a (novo): lista de convites de guilda recebidos - vetor de CSharedDoc+0x10ff8 (elemento de 0x2c)
		C_ASSERT_VC10(offsetof(CSharedDoc, m_guildInvites983) == 0x10ff8);
		std::vector<sGuildInvite983>& inviteList = Doc()->m_guildInvites983;
		inviteList.clear();
		for (int count = packet.Decode2(); count > 0; count--)
		{
			sGuildInvite983 invite;
			packet.DecodeBuffer(&invite, sizeof(invite));
			inviteList.push_back(invite);
		}
		// 009b71e0: refaz a lista de guilda do mensageiro (FrMessengerDlg 00900ba0)
		FrMessengerDlg* pMsnDlg = CMessengerInfo::Instance()->GetMessengerDlg();
		if (pMsnDlg)
			((RbMessengerDlgGuild983*)pMsnDlg)->RefreshGuildInvite983();
	}
	break;
	case 0x119:
	{
		// 983 009cfc76 (novo): resposta ao convite de guilda. Aceito (byte != 0): grava a guilda, avisa o mensageiro (0x27 com
		// o meu id e a guilda). Nos dois casos esvazia a lista de convites e manda o 0x16b ao mensageiro.
		unsigned long result = packet.Decode4();
		if (result != 1)
		{
			GuildInviteNotify983(result);
			break;
		}
		unsigned char accepted = packet.Decode1();
		unsigned long guildId = packet.Decode4();
		if (accepted)
		{
			C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.info.dwGuildId) == 0x82a);
			Doc()->m_myInfo.info.dwGuildId = guildId;
			WSendPacket send((enumClientPacket)0x27);
			send.EncodeStr(std::string(MyId()));
			send.Encode4(guildId);
			send.Send(TO_MSN);
		}
		Doc()->m_guildInvites983.clear();
		FrMessengerDlg* pMsnDlg = CMessengerInfo::Instance()->GetMessengerDlg();
		if (pMsnDlg)
			((RbMessengerDlgGuild983*)pMsnDlg)->RefreshGuildInvite983();
		WSendPacket send((enumClientPacket)0x16b);
		send.Encode4(guildId);
		send.Send(TO_MSN);
	}
	break;
	case 0x11c:
		if (IsLocalContent((localContentType_t)0x14))
		{
			switch (packet.Decode1())
			{
			case 0:
				Doc()->m_toppageFeature = 0;
				break;
			case 1:
				Doc()->m_toppageFeature = 1;
				GetActor("Lobby") << MsgObject(NULL, 0xcf, 1, 0, 0, 0, 0); // 983 009cfae7: acende o botao do mensageiro
				break;
			case 2:
				Doc()->m_toppageFeature = 2;
				break;
			default:
				Doc()->m_toppageFeature = 0;
				break;
			}
		}
		else
			Doc()->m_toppageFeature = 0;
		break;
	case 0x11d:
	{
		CWorldTourEvent* pEvent = NULL;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0x1f,
			pEvent);
		if (pEvent != NULL)
		{
			if (IsLocalContent((localContentType_t)0x1f /*983: era 0x1e; medido*/))
			{
				pEvent->SetMapFlag(packet.Decode4());
				if (packet.Decode1() != 0)
					pEvent->SetGotGift(true);
				else
					pEvent->SetGotGift(false);
				pEvent->SetCanGift(false);
				if (pEvent->GetMapFlag() == 0xffff && !pEvent->GetGotGift())
					pEvent->SetCanGift(true);
			}
			else
			{
				pEvent->SetMapFlag(0);
				pEvent->SetGotGift(false);
				pEvent->SetCanGift(false);
			}
		}
	}
	break;
	case 0x11e:
	{
		CWorldTourEvent* pEvent = NULL;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0x1f,
			pEvent);
		if (pEvent != NULL)
		{
			if (packet.Decode1() == 0)
			{
				pEvent->SetMapFlag(0xffff);
				pEvent->SetGotGift(false);
				pEvent->SetCanGift(true);
			}
			else
			{
				WSendPacket send(0x29); // B1582: 983 009cfe9a (0x27 era o id do 645)
				send.Encode4(MyUID());
				send.Encode4(CSharedDoc::Instance()->m_giftIndex);
				send.Send(TO_MSN);
			}
		}
	}
	break;
	case 0x127:
	{
		CHalloweenEvent* pEvent = NULL;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0x22,
			pEvent);
		if (pEvent != NULL)
		{
			pEvent->SetMapFlag(packet.Decode4());
			pEvent->SetGiftCnt(packet.Decode4());
		}
	}
	break;
	case 0x128:
		if (IsLocalContent((localContentType_t)0x22 /*983: era 0x21; interpolado*/))
		{
			CHalloweenEvent* pEvent = NULL;
			CContentsDoc::Instance()->GetContainer((localContentType_t)0x22,
				pEvent);
			if (pEvent != NULL)
			{
				pEvent->SetGiftCnt(packet.Decode4());
				pEvent->SetShowGiftDlg(true);
			}
			WSendPacket send(0x21);
			send.EncodeStr(MyNick());
			send.Encode4(MyUID());
			send.Send(TO_MSN);
		}
		break;
	case 0x12a:
		switch (packet.Decode4())
		{
		case 15:
		{
			// 983 009d002c (novo; tabela 009d82ec): resultado -> aviso "notify" de 5 s e a mensagem 2 ao ator principal
			unsigned long result = packet.Decode4();
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetTimeLimit(5.0f);
			const char* text;
			switch (result)
			{
			case 1:
				text = K2L_Compatibility("\xba\xb8\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xc4\xed\xc5\xb0\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9.");
				break;
			case 2:
				text = K2L_Compatibility("\xc6\xc7\xb8\xc5\xb1\xe2\xb0\xa3\xc0\xbb \xc8\xae\xc0\xce\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4.");
				break;
			case 3:
				text = K2L_Compatibility("\xbf\xc3\xb9\xd9\xb8\xa5 \xc4\xed\xc6\xf9 \xb9\xf8\xc8\xa3\xb0\xa1 \xbe\xc6\xb4\xd5\xb4\xcf\xb4\xd9.");
				break;
			case 4:
				text = K2L_Compatibility("\xc3\xdf\xc3\xb7\xb1\xc7 \xc1\xf6\xb1\xde \xc1\xdf \xb9\xae\xc1\xa6\xb0\xa1 \xc0\xd6\xbe\xee \xc1\xa4\xbb\xf3 \xc3\xb3\xb8\xae \xb5\xc7\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9. \xc8\xa8\xc6\xe4\xc0\xcc\xc1\xf6\xb8\xa6 \xb9\xe6\xb9\xae\xc7\xcf\xbf\xa9 \xb9\xae\xc0\xc7\xc7\xcf\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9.");
				break;
			default: // 0 e os de fora da tabela
				text = K2L_Compatibility("\xc0\xdf\xb8\xf8\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb \xc4\xda\xb5\xe5\xc0\xd4\xb4\xcf\xb4\xd9");
				break;
			}
			pForm->SetMessage(K2L_Compatibility(text), false);
			pForm->Open(NULL, 0);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		}
		break;
		case 0:
			CTHunter::Instance()->SetUsedTikiReport(true);
			if (IsLocalContent((localContentType_t)0x59 /*983: era 0x58; interpolado*/))
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x6c), 1, 0, 0, 0, 0));
			break;
		case 1:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x21b), 0, 0, 0, 0, 0));
			break;
		case 2:
		{
			FrImageNoticeDlg* pDlg = CreateForm<FrImageNoticeDlg>(
				g_pFresh->GetManager(), this, "imagenoticedlg", NULL);
			pDlg->SetCaption(
				K2L_Compatibility("\275\305\261\324\260\241\300\324 \300\314\272\245\306\256"));
			pDlg->SetDlgSize(WRect(-1.0f, -1.0f, 301.0f, 309.0f));
			pDlg->SetBackGroundImage(0, 0, WRect(0, 0, 256.0f, 220.0f),
				"event_new_02_a", true);
			pDlg->SetBackGroundImage(2, 0, WRect(256.0f, 0, 314.0f, 247.0f),
				"event_new_02_b", true);
			pDlg->SetOkButton(WRect(114.0f, 247.0f, 0, 0), true);
			pDlg->SetDesc(
				K2L_Compatibility("8\277\371 31\300\317 \306\316\276\337 \310\250\306\344\300\314\301\366\277\241\274\255 \310\256\300\316\307\317\274\274\277\344."));
			pDlg->Open(NULL, 0);
		}
		break;
		case 3:
		{
			FrImageNoticeDlg* pDlg = CreateForm<FrImageNoticeDlg>(
				g_pFresh->GetManager(), this, "imagenoticedlg", NULL);
			pDlg->SetCaption(
				K2L_Compatibility("\275\305\261\324\260\241\300\324 \300\314\272\245\306\256"));
			pDlg->SetDlgSize(WRect(-1.0f, -1.0f, 342.0f, 334.0f));
			pDlg->SetBackGroundImage(0, 0, WRect(0, 0, 256.0f, 256.0f),
				"event_new_03_a", true);
			pDlg->SetBackGroundImage(2, 0, WRect(256.0f, 0, 333.0f, 256.0f),
				"event_new_03_b", true);
			pDlg->SetOkButton(WRect(127.0f, 273.0f, 0, 0), true);
			pDlg->SetDesc(
				K2L_Compatibility("8\277\371 31\300\317 \306\316\276\337 \310\250\306\344\300\314\301\366\277\241\274\255 \310\256\300\316\307\317\274\274\277\344."));
			pDlg->Open(NULL, 0);
			// o original monta o pacote e nao chama Send
			WSendPacket send(0x21);
			send.EncodeStr(MyNick());
			send.Encode4(MyUID());
		}
		break;
		case 4:
		{
			unsigned long guid = packet.Decode4();
			if (Doc()->m_myInfo.info.dwGuid == guid)
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", NULL);
				pForm->SetTimeLimit(2.0f);
				pForm->SetMessage(
					K2L_Compatibility("\307\366\300\347 \300\345\302\370\265\310 \276\306\301\356\305\330\300\273 \270\360\265\316 \274\322\270\360\307\317\277\264\300\270\271\307\267\316, "
					"\300\345\302\370 \276\306\301\356\305\330\300\273 \261\342\272\273 \276\306\301\356\305\330\300\270\267\316 \261\263\303\274 \307\325\264\317\264\331."),
					false);
				pForm->Open(NULL, 0);
				Doc()->m_myInfo.userEquip.tidBall = 0x14000000;
			}
			for (int i = 0; i < 4; i++)
			{
				if (guid == Doc()->m_userInfo[i].info.dwGuid)
				{
					Doc()->m_userInfo[i].userEquip.tidBall = 0x14000000;
					AfxGetTask()->GetActor("ball")
						<< MsgObject(NULL, M983(0x4d), 0x14000000, i, 0, 0, 0);
				}
			}
		}
		break;
		case 5:
		{
			std::string nick = packet.DecodeStr();
			char szMsg[0x80] = { 0 };
			sprintf(szMsg,
				"\307\366\300\347 \307\303\267\271\300\314\301\337\300\316 \260\324\300\323\277\241\274\255 '%s'\264\324\277\241 \300\307\307\330 \306\370\301\327\300\314 \305\315\301\263\275\300\264\317\264\331.",
				nick.c_str());
			// PKT (frente C, lote 12): 983 009d05fa - K2L_Compatibility sobre o texto ja formatado
			CChatMsg::Instance()->AddChatMsg(std::string(K2L_Compatibility(szMsg)), 0xffff0000,
				true, false);
		}
		break;
		case 11:
		{
			std::string msg = packet.DecodeStr();
			// PKT (frente C, lote 12): 983 009d0685 - K2L_Compatibility sobre o MakeStr
			CMessengerInfo::Instance()->Notify2(K2L_Compatibility(MakeStr("%s", msg.c_str())));
		}
		break;
		// PKT (frente C, lote 12): 983 009d06a2/009d06c6 - pela tarefa ao ator principal (nao "RealMyRoom") e com o id do
		// proprio pacote no 983 (0x12a; o fonte tinha o do 645, 0x122)
		case 8:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x228), 0x12a, 1, 0, 0, 0));
			break;
		case 9:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x228), 0x12a, 0, 0, 0, 0));
			break;
		}
		break;
	case 0x131:
	{
		if (!IsLocalContent((localContentType_t)0x59 /*983: era 0x58; interpolado*/))
			return 0;
		unsigned char type = packet.Decode1();
		sTreasureItem item;
		switch (type)
		{
		case 0:
			packet.DecodeBuffer(&item, sizeof(sTreasureItem));
			CTHunter::Instance()->SetTHunterGauge(item.byMapIndex,
				item.dwGauge);
			break;
		case 1:
		{
			unsigned char count = packet.Decode1();
			if (count > 0)
			{
				int n = count;
				do
				{
					packet.DecodeBuffer(&item, sizeof(sTreasureItem));
					CTHunter::Instance()->SetTHunterGauge(item.byMapIndex,
						item.dwGauge);
					n--;
				} while (n != 0);
			}
			break;
		}
		}
	}
	break;
	case 0x132:
		if (!IsLocalContent((localContentType_t)0x59 /*983: era 0x58; interpolado*/))
			return 0;
		switch (GetGameType())
		{
		case 0:
		case 1:
		case 4:
		case 5:
		{
			unsigned long point = packet.Decode4();
			CTHunter::Instance()->SetTreasurePoint(point);
			AfxGetTask()->GetActor("Score")
				<< MsgObject(NULL, M983(0x12f), 4, 0, 0, 0, 0);
		}
		break;
		}
		break;
	case 0x133:
	{
		std::string str;
		std::vector<sTreasureHunt> repository;
		sTreasureHunt hunt;
		unsigned char count = packet.Decode1();
		repository.reserve(count);
		if (count > 0)
		{
			int n = count;
			do
			{
				packet.DecodeBuffer(&hunt, sizeof(sTreasureHunt));
				repository.push_back(hunt);
				n--;
			} while (n != 0);
		}
		CTHunter::Instance()->ReceiveRepository(repository);
		CTHunter::Instance()->SetReceiveGiftPacket(true);
		CSharedDoc::Instance()->SetAppGiftDownloadState(
			(CSharedDoc::eAppGiftDownloadState)2);
	}
	break;
	case 0x134:
		if (IsLocalContent((localContentType_t)0x67))
		{
			std::vector<sTreasureGift> gifts;
			sTreasureGift gift;
			unsigned char count = packet.Decode1();
			if (count > 0)
			{
				int n = count;
				do
				{
					packet.DecodeBuffer(&gift, sizeof(sTreasureGift));
					gifts.push_back(gift);
					n--;
				} while (n != 0);
			}
			CTHunter::Instance()->ReceiveAllGift(gifts);
			CTHunter::Instance()->UpdateItemList("");
		}
		break;
	case 0x140:
	{
		if (IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
		{
			if (packet.Decode1() == 0)
			{
				AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x23),
					(int)K2L_Compatibility("\260\374\275\311\273\347 \276\367\265\245\300\314\306\256\270\246 \275\307\306\320\307\317\277\264\275\300\264\317\264\331."),
					0, 0, 0, 0);
			}
		}
	}
	break;
	case 0x12e:
		switch (packet.Decode1())
		{
		case 0:
		{
			unsigned char result = packet.Decode1();
			unsigned long guid = packet.Decode4();
			unsigned long tid = packet.Decode4();
			char uccIndex[9];
			strncpy(uccIndex, packet.DecodeStr().c_str(), 9);
			char uccName[41];
			strncpy(uccName, packet.DecodeStr().c_str(), 41);
			// PKT (frente C, lote 13): 983 009d09cf - as quatro mensagens do caso vao ao ator principal (era "RealMyRoom")
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x214), result, (int)&guid,
				(int)&tid, (int)uccIndex, (int)uccName));
		}
		break;
		case 1:
		{
			sItemInfo item;
			packet.Decode4();
			char uccIndex[9];
			strncpy(uccIndex, packet.DecodeStr().c_str(), 9);
			unsigned char slot = packet.Decode1();
			packet.DecodeBuffer(&item, sizeof(sItemInfo));
			if (slot == 1)
			{
				std::list<sItemInfo>& uccList = Doc()->m_uccItemList;
				bool bFound = false;
				std::list<sItemInfo>::iterator it;
				for (it = uccList.begin(); it != uccList.end(); it++)
				{
					if (it->tid == item.tid &&
						strcmp(it->UccIndex, item.UccIndex) == 0 &&
						it->Seq == item.Seq)
					{
						bFound = true;
						break;
					}
				}
				if (!bFound)
				{
					uccList.push_back(item);
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xe1), 0, 0, 0, 0, 0));
				}
			}
			if (item.status & 1)
			{
				sUccClothes* pClothes = UccManager()->FindClothes(item.guid);
				if (pClothes)
				{
					pClothes->date = item.ItemDate;
					if (strlen(item.ItemName) != 0)
						strcpy(pClothes->uccName, item.ItemName);
				}
				else
				{
					sUccClothes clothes;
					clothes.id = item.guid;
					clothes.date = item.ItemDate;
					clothes.downloadCount = 0;
					clothes.icon = NULL;
					if (strlen(item.ItemName) != 0)
						strcpy(clothes.uccName, item.ItemName);
					clothes.typeId = item.tid;
					strncpy(clothes.uccIndex, item.UccIndex, 9);
					UccManager()->AddClothes(slot, clothes, false, false);
				}
			}
		}
		break;
		case 2:
		{
			unsigned long tid = packet.Decode4();
			char uccIndex[9];
			strncpy(uccIndex, packet.DecodeStr().c_str(), 9);
			int seq = packet.Decode2();
			unsigned long oldGuid = packet.Decode4();
			unsigned long newGuid = packet.Decode4();
			unsigned long newTid = packet.Decode4();
			char newUccIndex[9];
			strncpy(newUccIndex, packet.DecodeStr().c_str(), 9);
			int newSeq = packet.Decode2();
			if (packet.Decode1() == 1)
			{
				UccManager()->FindClothes(tid, uccIndex);
				sUccClothes clothes;
				clothes.typeId = newTid;
				clothes.id = newGuid;
				strncpy(clothes.uccIndex, newUccIndex, 9);
				UccManager()->AddClothes(1, clothes, false, false);

				FrUccCopyCompletedDlg* pDlg = CreateForm<FrUccCopyCompletedDlg>(
					g_pFresh->GetManager(), this, "ucc_copy_completed", NULL);
				pDlg->Init(tid, uccIndex);
				pDlg->Open(NULL, WPoint(453.0f, 189.0f), 1);

				sItemInfo* pInfo =
					GetMyItemInfo(tid, uccIndex, (unsigned short)seq);
				std::list<sItemInfo>& uccList = Doc()->m_uccItemList;
				std::list<sItemInfo>::iterator it;
				for (it = uccList.begin(); it != uccList.end(); it++)
				{
					if (it->guid == oldGuid)
					{
						it->guid = newGuid;
						it->tid = newTid;
						strncpy(it->UccIndex, uccIndex,
							9); // o original copia o indice antigo
						it->Seq = (unsigned short)newSeq;
						it->status &= ~2;
						it->status |= 1;
						strncpy(it->CopierNick, Doc()->m_myInfo.info.sNick, 22);
						strncpy(it->ItemName,
							pInfo
								? pInfo->ItemName
								: K2L_Compatibility("\301\244\272\270\270\246 \271\336\276\306\277\300\264\302 \301\337..."),
							41);
						break;
					}
				}
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xe1), 0, 0, 0, 0, 0));
			}
			else
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
					(int)K2L_Compatibility("\274\277\307\301\265\360\300\332\300\316 \300\307\273\363 \272\271\301\246\277\241 \275\307\306\320\307\337\275\300\264\317\264\331."),
					0, 0, 0, 0));
			}
		}
		break;
		case 3:
		{
			unsigned long tid = packet.Decode4();
			char uccIndex[9] = { 0 };
			strncpy(uccIndex, packet.DecodeStr().c_str(), 9);
			unsigned char result = packet.Decode1();
			if (result == 1)
			{
				std::list<sItemInfo>& uccList = Doc()->m_uccItemList;
				std::list<sItemInfo>::iterator it;
				for (it = uccList.begin(); it != uccList.end(); it++)
				{
					if (it->tid == tid && strcmpi(it->UccIndex, uccIndex) == 0)
					{
						it->status |= 2;
						break;
					}
				}
			}
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x215), result, (int)&tid,
				(int)uccIndex, 0, 0));
		}
		break;
		}
		break;
	case 0x144:
	{
		CMatchingSystem* pMS = CMatchingSystem::Instance();
		if (pMS != NULL)
		{
			unsigned char count = packet.Decode1();
			if (count > 0)
			{
				for (int i = count; i > 0; i--)
				{
					sIntrest intrest;
					packet.DecodeBuffer(&intrest, sizeof(sIntrest));
					pMS->SetDisplayIntrest(intrest);
				}
			}
		}
	}
	break;
	case 0x157:
	{
		packet.Decode1();
		unsigned long uid = packet.Decode4();
		std::map<unsigned long, sUserInfoTime>::iterator it;
		if (CSharedDoc::Instance()->FindUserInfoTimeUID(uid, it) == 0)
		{
			sUserInfoTime newInfo; // 983: construtor 00644f10 (zera tudo; sem memset)
			if (CSharedDoc::Instance()->InsertUserInfoTime(uid, newInfo) == 0)
				break;
			if (CSharedDoc::Instance()->FindUserInfoTimeUID(uid, it) == 0)
				break;
		}
		short roomIndex = packet.Decode2();
		sUserInfoTime* pInfo = &it->second;
		packet.DecodeBuffer(&it->second.info.info, sizeof(sPangYaUserInfo));
		pInfo->info.roomIndex = roomIndex;
		unsigned long guildPoint = packet.Decode4();
		it->second.dwGuildPoint = guildPoint;
	}
	break;
	case 0x158:
	{
		unsigned char type = packet.Decode1();
		unsigned long uid = packet.Decode4();
		std::map<unsigned long, sUserInfoTime>::iterator it;
		int found = CSharedDoc::Instance()->FindUserInfoTimeUID(uid, it);
		if (found == 0)
		{
			sUserInfoTime newInfo; // 983: construtor 00644f10 (zera tudo; sem memset)
			if (CSharedDoc::Instance()->InsertUserInfoTime(uid, newInfo) != 0)
				found = CSharedDoc::Instance()->FindUserInfoTimeUID(uid, it);
		}
		sPangYaUserStatistics stat;
		packet.DecodeBuffer(&stat, sizeof(sPangYaUserStatistics));
		if (Doc()->m_myInfo.info.dwUID == uid)
		{
			if (type != 0)
				Doc()->m_myInfo.stat = stat;
			else
				Doc()->m_oldSeasonStatistics = stat;
		}
		if (found == 0)
			break;
		if (type == 5)
			it->second.info.stat = stat;
		else if (type == 0)
			it->second.oldSeason.userStat = stat;
	}
	break;
	case 0x156:
	{
		packet.Decode1();
		unsigned long uid = packet.Decode4();
		std::map<unsigned long, sUserInfoTime>::iterator it;
		if (CSharedDoc::Instance()->FindUserInfoTimeUID(uid, it) != 0)
			packet.DecodeBuffer(&it->second.info.userEquip, sizeof(sUserEquip));
	}
	break;
	case 0x15d:
	{
		unsigned long uid = packet.Decode4();
		std::map<unsigned long, sUserInfoTime>::iterator it;
		if (CSharedDoc::Instance()->FindUserInfoTimeUID(uid, it) != 0)
		{
			// 983 009d1413: bloco de 0x4d (o 645 lia um GUILD_INFO de 0x129); a versao do emblema e nova (lote 9 da C-PKT)
			sGuildBrief983 guild;
			packet.DecodeBuffer(&guild, sizeof(sGuildBrief983));
			it->second.dwGuildPang = guild.guildPang;
			if (uid == MyUID())
				it->second.dwGuildPoint = guild.guildPoint;
			it->second.info.info.dwEmblemVer = guild.emblemVer;
			strcpy(it->second.info.info.szEmblemName, guild.guildMark);
			strcpy(it->second.info.info.sGuild, guild.guildName);
		}
	}
	break;
	case 0x15b:
	{
		unsigned char type = packet.Decode1();
		unsigned long uid = packet.Decode4();
		unsigned short count = packet.Decode2();
		std::map<unsigned long, sUserInfoTime>::iterator it;
		if (CSharedDoc::Instance()->FindUserInfoTimeUID(uid, it) == 0)
			break;
		// 983 (009d148f): as listas são std::vector (sUserInfoTime+0x3b1c / oldSeason +0x8cb)
		std::vector<sGuildTrophy>* pList;
		if (type == 5)
			pList = &it->second.guildTrophies;
		else
		{
			if (type != 0)
				break;
			pList = &it->second.oldSeason.guildTrophies;
		}
		if (!pList->empty())
			pList->clear();
		for (int i = count; i > 0; i--)
		{
			sGuildTrophy item;
			packet.DecodeBuffer(&item, sizeof(sGuildTrophy));
			pList->push_back(item);
		}
	}
	break;
	case 0x159:
	{
		unsigned char type = packet.Decode1();
		unsigned long uid = packet.Decode4();
		std::map<unsigned long, sUserInfoTime>::iterator it;
		if (CSharedDoc::Instance()->FindUserInfoTimeUID(uid, it) != 0)
		{
			sTrophyStatistics trophy;
			packet.DecodeBuffer(&trophy, sizeof(sTrophyStatistics));
			if (type == 5)
				it->second.info.trophy = trophy;
			else if (type == 0)
				it->second.oldSeason.trophyStat = trophy;
		}
	}
	break;
	case 0x15a:
	{
		unsigned char type = packet.Decode1();
		unsigned long uid = packet.Decode4();
		unsigned short count = packet.Decode2();
		std::map<unsigned long, sUserInfoTime>::iterator it;
		if (CSharedDoc::Instance()->FindUserInfoTimeUID(uid, it) == 0)
			break;
		// 983 (009d15e8): as listas são std::vector (sUserInfoTime+0x3b0c / oldSeason +0x8bb)
		std::vector<sSpecialTrophy>* pList;
		if (type == 5)
			pList = &it->second.trophies;
		else
		{
			if (type != 0)
				break;
			pList = &it->second.oldSeason.trophies;
		}
		if (!pList->empty())
			pList->clear();
		for (int i = count; i > 0; i--)
		{
			sSpecialTrophy item;
			packet.DecodeBuffer(&item, sizeof(sSpecialTrophy));
			pList->push_back(item);
		}
	}
	break;
	case 0x15c:
	{
		// 983 (009d16a6): tipo, uid e DUAS listas (contagem de 32 bits + registros de 0x2b); cada lista vai para um dos 12
		// vetores de 22 mapas que comecam em sUserInfoTime+0x2fe (indice por 00731d00: tipo x 1a/2a lista).
		struct MapStat983
		{
			static int Index(unsigned char type, bool second)
			{
				switch (type)
				{
				case 0: return second ? 5 : 1;
				case 5: return second ? 4 : 0;
				case 0xa: return second ? 7 : 3;
				case 0xb: return second ? 11 : 9;
				case 0x33: return second ? 6 : 2;
				case 0x34: return second ? 10 : 8;
				}
				return -1;
			}
		};
		unsigned char type = packet.Decode1();
		unsigned long uid = packet.Decode4();
		std::map<unsigned long, sUserInfoTime>::iterator it;
		if (CSharedDoc::Instance()->FindUserInfoTimeUID(uid, it) == 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, 0x10c8e1, 0, 0, 0, 0));
			break;
		}
		sMapStatistics* base = it->second.info.mapStat; // 983: 12 x 22 seguidos (mapStat, classicMapStat, mapStat983)
		bool failed = false;
		for (int list = 0; list < 2; list++)
		{
			sMapStatistics stats[22];
			for (int i = 0; i < 22; i++)
			{
				stats[i].bMap = (unsigned char)i;
				stats[i].cBestScore = 0x7f;
			}
			for (int count = packet.Decode4(); count > 0; count--)
			{
				sMapStatistics one;
				one.bMap = 0xff;
				packet.DecodeBuffer(&one, sizeof(sMapStatistics));
				stats[one.bMap] = one; // 983: sem conferir o indice
			}
			int index = MapStat983::Index(type, list == 1);
			if (index == -1)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, 0x10c8e2 + list, 0, 0, 0, 0));
				failed = true;
				break;
			}
			memcpy(base + index * 22, stats, sizeof(stats));
		}
		if (failed)
			break;
	}
	break;
	case 0x15e:
	{
		unsigned long uid = packet.Decode4();
		std::map<unsigned long, sUserInfoTime>::iterator it;
		if (CSharedDoc::Instance()->FindUserInfoTimeUID(uid, it) != 0)
		{
			sCharacterInfo charInfo;
			packet.DecodeBuffer(&charInfo, sizeof(sCharacterInfo));
			it->second.info.charInfo = charInfo;
			((RbSharedDocView*)CSharedDoc::Instance())->SetUserInfoMod(0);
			if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
				RequestUccInfo(charInfo);
		}
	}
	break;
	case 0x153:
		switch (packet.Decode1())
		{
		case 0:
		{
			unsigned char content = packet.Decode1();
			unsigned long index = packet.Decode4();
			std::string key = packet.DecodeStr();
			unsigned char result = packet.Decode1();
			SecurityKeyManager()->ResCreateKey((eSKContent)content, index, key,
				result);
		}
		break;
		case 1:
		{
			unsigned char content = packet.Decode1();
			unsigned long index = packet.Decode4();
			std::string key = packet.DecodeStr();
			unsigned char result = packet.Decode1();
			SecurityKeyManager()->ResCheckKey((eSKContent)content, index, key,
				result);
		}
		break;
		}
		break;
	case PKT645_SEM_983(0x15b):
	{
		int count = packet.Decode1();
		int type = (unsigned short)packet.Decode2();
		char szMsg[0x100] = { 0 };
		char szServer[40] = { 0 };
		char szMode[40] = { 0 };
		char szCourse[40] = { 0 };
		wsprintfA(szMsg,
			"\303\321 \260\263\274\263\265\310 \271\346 \260\263\274\366 %d",
			count);
		CChatMsg::Instance()->AddChatMsg(std::string(szMsg), 0xffff7878, false,
			false);
		for (int i = count; i > 0; i--)
		{
			RbRoomReport15b room;
			packet.DecodeBuffer(&room, sizeof(RbRoomReport15b));
			if ((room.flags & 3) || (unsigned short)type != 4 ||
				(room.flags & 4))
			{
				wsprintfA(szServer, "%d", room.serverId);
				std::list<sGameServerInfo>& serverList =
					Doc()->m_gameServerList;
				for (std::list<sGameServerInfo>::iterator it =
						 serverList.begin();
					it != serverList.end(); ++it)
				{
					if (it->id == room.serverId)
					{
						wsprintfA(szServer, "%s", it->name);
						break;
					}
				}
				switch (room.mode)
				{
				case 0:
					wsprintfA(szMode, "%s", "\267\347\305\260");
					break;
				case 1:
					wsprintfA(szMode, "%s", "\303\312\272\270");
					break;
				case 2:
					wsprintfA(szMode, "%s", "\301\337\261\336");
					break;
				case 3:
					wsprintfA(szMode, "%s", "\300\332\300\2571");
					break;
				case 4:
					wsprintfA(szMode, "%s", "\300\332\300\2572");
					break;
				default:
					wsprintfA(szMode, "%d", room.mode);
					break;
				}
				switch (room.course)
				{
				case 0:
					wsprintfA(szCourse, "%s",
						"\272\355\267\347\266\363\261\272");
					break;
				case 1:
					wsprintfA(szCourse, "%s",
						"\272\355\267\347\277\366\305\315");
					break;
				case 2:
					wsprintfA(szCourse, "%s",
						"\274\274\307\307\276\306\300\251\265\345");
					break;
				case 3:
					wsprintfA(szCourse, "%s", "\300\251\265\345\310\372");
					break;
				case 4:
					wsprintfA(szCourse, "%s",
						"\300\247\301\356\300\247\301\356");
					break;
				case 5:
					wsprintfA(szCourse, "%s",
						"\277\376\275\272\306\256\300\247\301\356");
					break;
				case 6:
					wsprintfA(szCourse, "%s", "\272\355\267\347\271\256");
					break;
				case 7:
					wsprintfA(szCourse, "%s",
						"\275\307\272\361\276\306\304\263\263\355");
					break;
				case 8:
					wsprintfA(szCourse, "%s",
						"\276\306\300\314\275\272\304\263\263\355");
					break;
				case 9:
					wsprintfA(szCourse, "%s",
						"\310\255\300\314\306\256\300\247\301\356");
					break;
				case 10:
					wsprintfA(szCourse, "%s",
						"\273\376\300\314\264\327\273\367\265\345");
					break;
				case 11:
					wsprintfA(szCourse, "%s",
						"\307\316\305\251\300\251\265\345");
					break;
				case 13:
					wsprintfA(szCourse, "%s",
						"\265\366\300\316\306\344\270\243\263\353");
					break;
				case 14:
					wsprintfA(szCourse, "%s",
						"\276\306\300\314\275\272\275\272\306\304");
					break;
				case 15:
					wsprintfA(szCourse, "%s",
						"\267\316\275\272\306\256\275\303\277\376\300\314");
					break;
				case 16:
					wsprintfA(szCourse, "%s",
						"\300\314\275\272\305\317\271\353\270\256");
					break;
				case 17:
					wsprintfA(szCourse, "%s",
						"\275\272\306\363\274\310\274\305\307\303");
					break;
				case 19:
					wsprintfA(szCourse, "%s",
						"\300\247\301\356\275\303\306\274");
					break;
				case 127:
					wsprintfA(szCourse, "%s", "\267\243\264\375");
					break;
				default:
					wsprintfA(szCourse, "%d", room.course);
					break;
				}
				wsprintfA(szMsg, "%s %s %d\271\370\271\346 %s %d\310\246",
					szServer, szMode, room.roomNo, szCourse, room.nHole);
				CChatMsg::Instance()->AddChatMsg(std::string(szMsg), 0xffff7878,
					false, false);
			}
		}
	}
	break;
	case 0x12b:
	{
		if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
			break;
		if (packet.Decode4() != 0)
		{
			sRealMyRoomAuthority auth;
			packet.DecodeBuffer(&auth, sizeof(sRealMyRoomAuthority));
			Doc()->m_rmrAuthority.ownerUID = auth.ownerUID;
			Doc()->m_rmrAuthority.bPrivate = auth.bPrivate;
			Doc()->m_rmrAuthority.serverValue983 = auth.serverValue983;
			strcpy(Doc()->m_rmrAuthority.password, auth.password);
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x21f), 0, 0, 0, 0, 0));
		}
		else if (IS_EXACTKINDOF(CLobbyTask, AfxGetTask())) // PKT (frente C, lote 13): 983 009d1b23 - RTTI 00f030b0, ator "Lobby"
			AfxPostMsg(NULL, "Lobby", M983(0x228), 0x12b /*983 009d1a7e; era 0x123 do 645*/, 0, 0, 0);
		else
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x228), 0x12b /*983; era 0x123 do 645*/, 0, 0, 0, 0));
	}
	break;
	case 0x181:
		if (packet.Decode4() == 2 && packet.Decode4() == 1)
		{
			unsigned long tid = packet.Decode4();
			_RemoveWareHouseItem(tid, AfxGetTask()->GetMainActor(), 1); // PKT (frente C, lote 13): 983 009d1ce3 - CTask+0x30
		}
		// PKT (frente C, lote 55): 983 009d1cf9 - o buff de item le o pacote de novo em todos os sub-codigos
		packet.Reset();
		CItemBuff::ProcessPacket(packet);
		break;
	case PKT645_SEM_983(0x189):
		if (IsLocalContent((localContentType_t)0x78) == 1)
		{
			unsigned long tid = packet.Decode4();
			_RemoveWareHouseItem(tid, AfxGetTask()->GetActor("Lobby"), 100);
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetTimeLimit(3.0f);
			pForm->SetMessage(
				"\300\314\272\245\306\256 \276\306\300\314\305\333\300\273 \271\336\276\322\275\300\264\317\264\331. "
				"\270\266\300\314\267\353\277\241\274\255 \310\256\300\316\307\317\274\274\277\344.",
				false);
		}
		break;
	case 0x188:
	{
		unsigned char result = packet.Decode1();
		FrCardRemoveResult* pForm = CreateForm<FrCardRemoveResult>(
			g_pFresh->GetManager(), this, "card_remove_result", NULL);
		if (!pForm)
			break;
		FrCardRemoveResult* pResult = pForm;
		if (result == 1)
		{
			unsigned long partsId = packet.Decode4();
			unsigned long tid = packet.Decode4();
			if (FrCardRemove::IsCardRemoveItem(tid) == true)
			{
				pResult->SetRemover(tid);
				_RemoveWareHouseItem(tid, AfxGetTask()->GetMainActor(), // PKT (frente C, lote 13): 983 009d1d61 - CTask+0x30
					1);
				if (partsId)
				{
					std::list<unsigned long> cardList;
					if (S5::UTIL::ErasePartsCardAll(partsId, cardList))
					{
						std::list<unsigned long>::iterator it;
						for (it = cardList.begin(); it != cardList.end(); it++)
							pResult->PushList(*it);
					}
					pResult->SetResult((FrCardRemoveResult::eRESULT)1);
				}
				else
				{
					pResult->SetResult((FrCardRemoveResult::eRESULT)2);
				}
			}
			else
			{
				pResult->SetResult((FrCardRemoveResult::eRESULT)0);
			}
		}
		else if (result == 0)
		{
			if (packet.Decode1() == 9)
				pResult->SetResult((FrCardRemoveResult::eRESULT)3);
			else
				pResult->SetResult((FrCardRemoveResult::eRESULT)0);
		}
		pForm->SetCaption(K2L_Compatibility("\273\347\277\353\260\341\260\372"));
		pForm->Open(NULL, 0);
	}
	break;
	case PKT645_SEM_983(0x18b):
		IsLocalContent((localContentType_t)0x79);
		break;
	case 0x193:
		SetMyOutsideCash(packet.Decode8());
		Doc()->RefreshAllCookie();
		if (RFLobbyDesktop::Instance983()) // 0047f920
			RFLobbyDesktop::Instance983()->Refresh983();
		break;
	case 0x195:
	{
		unsigned char season = packet.Decode1();
		unsigned long count = packet.Decode4();
		std::map<unsigned char, sMapStatistics>& stats =
			Doc()->GetMyPastMapStat(season);
		for (; count > 0; count--)
		{
			unsigned char mapId = packet.Decode1();
			sMapStatistics stat;
			packet.DecodeBuffer(&stat, sizeof(sMapStatistics));
			stats.insert(
				std::pair<const unsigned char, sMapStatistics>(mapId, stat));
		}
	}
	break;
	case PKT645_SEM_983(0x1a1):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (!IsLocalContent((localContentType_t)0x8e))
			break;
		unsigned long count = packet.Decode4();
		for (; count > 0; count--)
		{
			unsigned long value = packet.Decode4();
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x252), (int)&value, 0, 0, 0, 0));
		}
	}
	break;
	case PKT645_SEM_983(0x1cf):
	{
		int a = packet.Decode4();
		int b = packet.Decode4();
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, M983(0x26d), 0xf, a, b, 0, 0);
	}
	break;
	case PKT645_SEM_983(0x1d0):
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, M983(0x26d), 0x11, 0, 0, 0, 0);
		break;
	// ---- PKT (frente C, sessao C14, lote 7): casos que so existem no 983 ----
	case 0x96:
	{
		// 983 009cc432: saldo de +0x3870 (8 bytes) e redesenho; depois a cauda 009caf6f, como no caso 0xc8
		Doc()->m_bonusCash = packet.Decode8();
		Doc()->RefreshAllCookie();
		if (RFLobbyDesktop::Instance983()) // 0047f920
			RFLobbyDesktop::Instance983()->Refresh983();
	}
	break;
	case 0xdb:
	{
		// 983 009caf8c
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (packet.Decode4() != 4)
			break;
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, 0x23, (int)K2L_Compatibility("\300\314\271\314 \260\241\301\366\260\355 \300\326\264\302 \276\306\300\314\305\333\300\324\264\317\264\331"), 0, 0, 0, 0));
	}
	break;
	case 0x102:
	{
		// 983 009d1f07: bilhetes (+0x1ec, +0x1f0), pang e cookie de uma vez; avisa o lobby (mensagem 0x27b)
		Doc()->m_gachaTicket[0] = packet.Decode4();
		Doc()->m_gachaTicket[1] = packet.Decode4();
		Doc()->m_myInfo.stat.i64Pang = packet.Decode8();
		Doc()->m_cookie = packet.Decode8();
		Doc()->UpdateGachaTickets();
		if (IS_KINDOF(CLobbyTask, AfxGetTask()))
			AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x27b, 0, 0, 0, 0, 0);
	}
	break;
	case 0x1ad:
	{
		// 983 009d32c0: resposta ao 0xfb do CWebPageConnector (resultado, texto do servidor); com 1 abre a pagina pedida
		std::string text("");
		int result = packet.Decode4();
		text = packet.DecodeStr();
		if (result == 1 && CWebPageConnector::Instance())
			CWebPageConnector::Instance()->OnServerReply983(Doc()->m_myInfo.info.sID, text.c_str());
	}
	break;
	case 0x1e7:
	{
		// 983 009d2de4: abertura da loja de pontos
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (packet.Decode4() != 0)
		{
			std::string text(K2L_Compatibility("\301\366\261\335\300\272 \306\367\300\316\306\256 \230\336\300\273 \300\314\277\353\307\322 \274\366 \276\370\275\300\264\317\264\331."));
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)text.c_str(), 0, 0, 0, 0));
		}
		else
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2e0, 0, 0, 0, 0, 0));
	}
	break;
	case 0x1e8:
	{
		// 983 009d2ecd: meus pontos (dword em CSharedDoc+0x3860)
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (packet.Decode4() != 0)
		{
			std::string text(K2L_Compatibility("\263\273 \306\367\300\316\306\256\270\246 \260\241\301\256\277\303 \274\366 \276\370\275\300\264\317\264\331."));
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2e1, (int)text.c_str(), 0, 0, 0, 0));
			break;
		}
		C_ASSERT_VC10(offsetof(CSharedDoc, m_pointShopTP983) == 0x3860);
		*(unsigned long*)&Doc()->m_pointShopTP983 = packet.Decode4();
	}
	break;
	case 0x1e9:
	{
		// 983 009d2fb0: troca de pontos
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (packet.Decode4() != 0)
		{
			std::string text(K2L_Compatibility("\306\367\300\316\306\256 \261\263\310\257\277\241 \275\307\306\320\307\317\277\264\275\300\264\317\264\331."));
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2e1, (int)text.c_str(), 0, 0, 0, 0));
			break;
		}
		*(unsigned long*)&Doc()->m_pointShopTP983 = packet.Decode4();
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, 0x2e1, (int)K2L_Compatibility("\306\367\300\316\306\256 \261\263\310\257\277\241 \274\272\260\370\307\317\277\264\275\300\264\317\264\331."), 1, 0, 0, 0));
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2e2, 0, 0, 0, 0, 0));
	}
	break;
	case 0x1ea:
	{
		// 983 009d30f2: compra na loja de pontos (erros 0x1fc99-0x1fc9e, tabela 009d859c)
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long error = packet.Decode4();
		if (error != 0)
		{
			std::string text;
			const char* msg;
			switch (error)
			{
			case 0x1fc9a:
				msg = K2L_Compatibility("\261\270\270\305\277\241 \307\312\277\344\307\321 \306\367\300\316\306\256\260\241 \272\316\301\267\307\325\264\317\264\331.");
				break;
			case 0x1fc9e:
				msg = K2L_Compatibility("\276\306\300\314\305\333 \274\366\267\256\300\314 \300\337\270\370\265\307\276\372\275\300\264\317\264\331.");
				break;
			case 0x1fc99:
				msg = K2L_Compatibility("\277\344\303\273\307\321 \273\363\307\260\301\244\272\270\260\241 \300\337\270\370\265\307\276\372\275\300\264\317\264\331.");
				break;
			case 0x1fc9d:
				msg = K2L_Compatibility("\306\367\300\316\306\256\274\245\277\241 \265\356\267\317\265\310 \276\306\300\314\305\333\300\314 \276\306\264\325\264\317\264\331.");
				break;
			case 0x1fc9b:
				msg = K2L_Compatibility("\276\306\300\314\305\333 \260\241\260\335\300\314 \300\337\270\370\265\307\276\372\275\300\264\317\264\331.");
				break;
			default:
				msg = K2L_Compatibility("\276\306\300\314\305\333 \261\270\270\305\270\246 \275\307\306\320\307\317\277\264\275\300\264\317\264\331.");
				break;
			}
			text = K2L_Compatibility(msg);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2e1, (int)text.c_str(), 0, 0, 0, 0));
			break;
		}
		*(unsigned long*)&Doc()->m_pointShopTP983 = packet.Decode4();
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, 0x2e1, (int)K2L_Compatibility("\276\306\300\314\305\333 \261\270\270\305\270\246 \274\272\260\370\307\317\277\264\275\300\264\317\264\331."), 1, 0, 0, 0));
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2e2, 0, 0, 0, 0, 0));
	}
	break;
	case 0x1ed:
	{
		// 983 009d365e: saida forcada da partida (tabela de bytes 009d85c0)
		unsigned long code = packet.Decode4();
		switch (code)
		{
		case 0x39fbf3:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
				(int)K2L_Compatibility("\300\314\271\314 \260\324\300\323\300\273 \275\303\300\333\307\321 \264\331\270\245 \307\303\267\271\300\314\276\356\277\315 \275\303\260\243 \260\243\260\335\300\314 \305\251\260\324 \271\372\276\356\301\256, \272\316\265\346\300\314\307\317\260\324 \267\316\272\361\267\316 \260\255\301\246 \300\314\265\277\265\313\264\317\264\331."), 0, 0, 0, 0));
			break;
		case 0x39fc05:
		case 0x39fc07:
		case 0x39fc08:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
				(int)MakeStr(K2L_Compatibility("\300\337\270\370\265\310 \260\324\300\323\301\244\272\270\267\316 \260\324\300\323\301\276\267\341 \265\307\276\372\275\300\264\317\264\331.\n\nerrCode(%d)"), code), 0, 0, 0, 0));
			break;
		}
	}
	break;
	case 0x210:
	{
		// 983 009d38b2: lista de cartas nao lidas em CSharedDoc::m_mailList (registros de 0xc4 do 983; 645: pacote 0x15e
		// com 0xc2). Erro = mensagem 0x27; com carta, camera refeita e mensagem 0xce (1) - 0xce e o 0xaa do 645.
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		Doc()->m_mailList.clear();
		for (unsigned long n = packet.Decode4(); n != 0; --n)
		{
			sMailInfoBrief983 mail;
			packet.DecodeBuffer(&mail, sizeof(mail));
			Doc()->m_mailList.push_back(MailInfoBriefFrom983(mail));
		}
		if (Doc()->m_mailList.size() != 0)
		{
			Doc()->m_bRefreshCamera = true;
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0xce, 1, 0, 0, 0, 0));
		}
	}
	break;
	case 0x211:
	case 0x215:
	{
		// 983 009d3c19 (0x211) e 009d4152 (0x215): uma pagina da caixa de correio (645: pacote 0x140 do CRealMyRoomTask).
		// Os tres dwords vao, na ordem, para +0xc, +0x10 e +0x14 da pagina (o terceiro e a contagem); o 645 lia o
		// primeiro como pagina atual (+0x10). Depois: 0x2a1 (0) e 0x2bf (1) ao ator principal; o 0x215 manda tambem 0x2c0 (1).
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		sMailBoxListForPage page;
		page.iTotalPage = packet.Decode4();
		page.iCurPage = packet.Decode4();
		page.iTotalCount = packet.Decode4();
		for (int i = 0; i < page.iTotalCount; ++i)
		{
			sMailInfoBrief983 mail;
			packet.DecodeBuffer(&mail, sizeof(mail));
			page.m_lMailList.push_back(MailInfoBriefFrom983(mail));
		}
		if (IsLocalContent(S4_UCC))
		{
			for (std::list<sMailInfoBrief>::iterator it = page.m_lMailList.begin(); it != page.m_lMailList.end(); ++it)
				RequestUccInfo(it->item);
		}
		CTransferableItem::Instance()->SetMailInfoPage(page);
		if (type == 0x215)
			AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x2a1, 0, 0, 0, 0, 0)
										 << MsgObject(NULL, 0x2bf, 1, 0, 0, 0, 0)
										 << MsgObject(NULL, 0x2c0, 1, 0, 0, 0, 0);
		else
			AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x2a1, 0, 0, 0, 0, 0)
										 << MsgObject(NULL, 0x2bf, 1, 0, 0, 0, 0);
	}
	break;
	case 0x212:
	{
		// 983 009d3dcc: uma carta aberta (645: pacote 0x142 do CRealMyRoomTask); itens de 0x37 no 983 (645: 0x35).
		// Depois: 0x2a2 (0), 0x2bf (1) e 0x2c0 (1) ao ator principal.
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		sMailInfo mailInfo;
		mailInfo.dwIndex = packet.Decode4();
		strncpy(mailInfo.szFromNick, packet.DecodeStr().c_str(), 0x16);
		strncpy(mailInfo.szDate, packet.DecodeStr().c_str(), 0x20);
		strncpy(mailInfo.szContent, packet.DecodeStr().c_str(), 800);
		mailInfo.bHaveItem = packet.Decode1() == 1;
		if (mailInfo.bHaveItem)
		{
			for (int n = packet.Decode4(); n > 0; --n)
			{
				sMailIncludeItem983 item;
				packet.DecodeBuffer(&item, sizeof(item));
				mailInfo.m_lMailItemList.push_back(MailIncludeItemFrom983(item));
			}
		}
		if (IsLocalContent(S4_UCC))
		{
			for (std::list<sMailIncludeItem>::iterator it = mailInfo.m_lMailItemList.begin();
				 it != mailInfo.m_lMailItemList.end(); ++it)
				RequestUccInfo(*it);
		}
		CTransferableItem::Instance()->SetMailInfo(mailInfo);
		AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x2a2, 0, 0, 0, 0, 0)
									 << MsgObject(NULL, 0x2bf, 1, 0, 0, 0, 0)
									 << MsgObject(NULL, 0x2c0, 1, 0, 0, 0, 0);
	}
	break;
	case 0x247:
	{
		// 983 009d5a2e: resultado do reset da oficina de tacos; erro = mensagem 0x27 e a janela recebe NULL, senao o pacote
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			if (FrClubSetResetWnd::s_pInstance != NULL)
				FrClubSetResetWnd::s_pInstance->OnResetResult983(NULL);
			break;
		}
		if (FrClubSetResetWnd::s_pInstance != NULL)
			FrClubSetResetWnd::s_pInstance->OnResetResult983(&packet);
	}
	break;
	case 0x213:
	{
		// 983 009d4037: presente enviado - avisa o mensageiro (pacote 0x29) e limpa o presente pendente
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_MSN) == 1)
		{
			WSendPacket send(0x29);
			send.Encode4(MyUID());
			send.Encode4(Doc()->m_giftIndex);
			send.Send(TO_MSN);
			Doc()->m_giftIndex = -1;
			Doc()->m_giftName = "";
		}
		AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x2a3, 0x146, 0, 0, 0, 0);
	}
	break;
	case 0x214:
	{
		// 983 009d39b3: item da carta levado para o my room; a tela responde em `moved` pela mensagem 0x321
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		int moved = 0;
		AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x321, (int)&moved, 0, 0, 0, 0);
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x2bf, 1, 0, 0, 0, 0)
										 << MsgObject(NULL, 0x2c0, 1, 0, 0, 0, 0);
			if (moved != 0)
				AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x322, 0, 0, 0, 0, 0);
			break;
		}
		if (moved != 0)
			AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x322, 1, 0, 0, 0, 0);
		else
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, 0x23, (int)K2L_Compatibility("\306\355\301\366\277\241 \306\367\307\324\265\310 \276\306\300\314\305\333\300\273 \270\266\300\314\267\353\300\270\267\316 \300\314\265\277\275\303\304\327\275\300\264\317\264\331."), 0, 0, 0, 0));
		AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x2bf, 1, 0, 0, 0, 0)
									 << MsgObject(NULL, 0x2c0, 1, 0, 0, 0, 0)
									 << MsgObject(NULL, 0x2a4, 0, 0, 0, 0, 0)
									 << MsgObject(NULL, 0x106, 0, 0, 0, 0, 0);
	}
	break;
	case 0x248:
	{
		// 983 009d5aae: premio de presenca; sem premio pendente pede o estado (pacote 0x16f, vazio), senao
		// entrega o bloco a tela (mensagem 0x327: param1 = &bool, param2 = &bloco)
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		sAttendanceReward983 reward;
		reward.Decode(packet);
		if (!reward.bGift)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x1, 0, 0, 0, 0, 0));
			WSendPacket send((enumClientPacket)0x16f);
			send.Send(TO_GAME);
			break;
		}
		bool bReceived = false;
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, 0x327, (int)&bReceived, (int)&reward, 0, 0, 0));
	}
	break;
	case 0x249:
	{
		// 983 009d5be1: premio de presenca recebido. O 983 monta aqui um texto de depuracao num std::stringstream
		// ("isGift(...) today(nome X n [TID:...]) next(...) round(...)") que ninguem le; nao escrito.
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		sAttendanceReward983 reward;
		reward.Decode(packet);
		bool bReceived = true;
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, 0x327, (int)&bReceived, (int)&reward, 0, 0, 0));
	}
	break;
	case 0x24a:
	{
		// 983 009d5dfc: le o mesmo bloco, monta o mesmo texto de depuracao (nao escrito) e nao faz mais nada
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		sAttendanceReward983 reward;
		reward.Decode(packet);
	}
	break;
	case 0x250:
	{
		// 983 009d6093: o pacote vai ao ator "Gateway" (mensagem 0x753e) e a tarefa volta ao lobby
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753e, 0, (int)&packet, 0, 0, 0);
		// 983 009d6093: +0x10b28 = 1 e +0x10b29 = 0 (C39: o lote 27 tinha os nomes do layout sem o byte novo +0x10b27)
		C_ASSERT_VC10(offsetof(CSharedDoc, m_bReturnLobbyNotice) == 0x10b28);
		C_ASSERT_VC10(offsetof(CSharedDoc, m_bRebuildUnderBar) == 0x10b29);
		Doc()->m_bReturnLobbyNotice = true;
		Doc()->m_bRebuildUnderBar = false;
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x113, 0, 0, 0, 0, 0));
		if (!IS_KINDOF(CLobbyTask, AfxGetTask()))
			CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
		AfxPostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
		AfxPostMsg(NULL, "Lobby", 0x8b, 4, 1, 0, 0);
		AfxGetTask()->m_bRecieveWhisper = true; // 983: byte da tarefa em +0x1f8
	}
	break;
	// C34: respostas da oficina de tacos (983 009d5611-009d5a2e). Erro (dword != 0): mensagem 0x27 ao ator principal e a
	// janela recebe NULL; sucesso: a janela le o resto do pacote. Os testes 0049e140/0049e150/004cce50 sao "a janela existe".
	case 0x23d:
	case 0x23e:
	case 0x23f:
	{
		unsigned long result = packet.Decode4();
		WReceivedPacket* pPacket = &packet;
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			pPacket = NULL;
		}
		if (FrClubSetLevelUpWnd::s_pInstance != NULL)
		{
			if (type == 0x23d)
				FrClubSetLevelUpWnd::s_pInstance->OnLevelUpStart983(pPacket);
			else if (type == 0x23e)
				FrClubSetLevelUpWnd::s_pInstance->OnLevelUpResult983(pPacket);
			else
				FrClubSetLevelUpWnd::s_pInstance->OnLevelUpCancel983(pPacket);
		}
	}
	break;
	case 0x241:
		// 983 009d5811: sem campos
		if (FrClubSetRankUpWnd::s_pInstance != NULL)
			FrClubSetRankUpWnd::s_pInstance->OnRankUpStart983();
		break;
	case 0x240:
	case 0x242:
	case 0x243:
	{
		unsigned long result = packet.Decode4();
		WReceivedPacket* pPacket = &packet;
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			pPacket = NULL;
		}
		if (FrClubSetRankUpWnd::s_pInstance != NULL)
		{
			if (type == 0x240)
				FrClubSetRankUpWnd::s_pInstance->OnRankUpPacketB983(pPacket);
			else if (type == 0x242)
				FrClubSetRankUpWnd::s_pInstance->OnRankUpResult983(pPacket);
			else
				FrClubSetRankUpWnd::s_pInstance->OnRankUpPacketA983(pPacket);
		}
	}
	break;
	case 0x245:
	case 0x246:
	{
		unsigned long result = packet.Decode4();
		WReceivedPacket* pPacket = &packet;
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			pPacket = NULL;
		}
		if (IsClubSetCareWndOn983())
		{
			if (type == 0x245)
				FrClubSetOptionalFunctionsWnd::s_pInstance->OnExpMoveResult983(pPacket);
			else
				FrClubSetOptionalFunctionsWnd::s_pInstance->OnRecoveryResult983(pPacket);
		}
	}
	break;
	case 0x251:
	{
		// 983 009d6213: volta ao lobby (pagina inicial); erro = mensagem 0x27
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		Doc()->m_bReturnLobbyNotice = false; // 983 009d6213: +0x10b28 = 0 e +0x10b29 = 0
		Doc()->m_bRebuildUnderBar = false;
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x113, 0, 0, 0, 0, 0));
		if (!IS_KINDOF(CLobbyTask, AfxGetTask()))
			CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
		AfxPostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
		if (Doc()->m_roomListRequest983 == 0) // 0062bf70: tipo da lista de salas (ver o caso 0x84 do CLobbyMain::HandleMsg)
			AfxPostMsg(NULL, "Lobby", 0x84, 0, 0, 0, 0);
		AfxGetTask()->m_bRecieveWhisper = false; // 983: byte da tarefa em +0x1f8
		AfxPostMsg(NULL, "Lobby", 0x77, 0, 0, 0, 0);
	}
	break;
	case 0x1f8:
	{
		// 983 009d348a (C-PKT lote 28): marca do tutorial - uid do jogador e um ponto; no jogo de tipo 1 so a do proprio time.
		// Vai ao ator "TutorGuide" (CTutorGuide da B, jp/tutorguide.cpp: mensagem 0x310 = marca N no ponto).
		WVector pos(0.0f, 0.0f, 0.0f);
		unsigned long uid = packet.Decode4();
		packet.DecodeBuffer(&pos.x, 4);
		packet.DecodeBuffer(&pos.y, 4);
		packet.DecodeBuffer(&pos.z, 4);
		unsigned char idx = Doc()->GetIndex(uid);
		if (Doc()->m_golfGame.gameType == 1 &&
			PLAYER(MyIndex())->team != PLAYER(idx)->team)
			break;
		AfxGetTask()->GetActor("TutorGuide")
			<< MsgObject(NULL, 0x310, (int)&pos, idx, 0, 0, 0);
	}
	break;
	case 0x254:
	{
		// 983 009d6430: saida da sala de volta ao lobby (parecido com o 0x4c; o fim do Decode2 != 0xffff e o mesmo, 009c48dd)
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			if (result == 0x557334)
			{
				// "processando. aguarde um momento."
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
					(int)K2L_Compatibility("\xC3\xB3\xB8\xAE\xC1\xDF\xC0\xD4\xB4\xCF\xB4\xD9. \xC0\xE1\xBD\xC3\xB8\xB8 \xB1\xE2\xB4\xD9\xB8\xAE\xBC\xBC\xBF\xE4."),
					0, 0, 0, 0));
			}
			else
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		if (packet.Decode2() != 0xffff)
		{
			AfxPostMsg(NULL, "AvatarChat", M983(0x8), 0, 0, 0, 0);
			AfxPostMsg(NULL, "AvatarChat", M983(0x2), 0, 0, 0, 0);
			break;
		}
		unsigned short noRoom = 0xffff;
		Doc()->m_roomInfo.roomGuid = noRoom;
		Doc()->m_roomInfo.gameType = 0xff;
		Doc()->m_myInfo.roomIndex = noRoom;
		Doc()->m_myInfo.info.dwGalleryGuid = -1;
		CProjectG::Instance()->FromGolfToLobbySettings();
		if (!IS_EXACTKINDOF(CLobbyTask, AfxGetTask()))
			CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
		AfxPostMsg(NULL, "Lobby", M983(0x2), 0, 0, 0, 0);
		AfxPostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
		if (Doc()->m_bRebuildUnderBar || DOCJP()->GetState983() == 2)
			AfxPostMsg(NULL, "Lobby", 0x8b, 4, 1, 0, 0);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x17e, 0, 0, 0, 0, 0));
		if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_MSN) &&
			CMessengerInfo::Instance()->GetComplete() == 4)
		{
			sUserPosition pos;
			pos.iServerGUID = Doc()->m_gameServerUID;
			pos.iChannelUid = Doc()->m_curChannel.Uid;
			memset(pos.csChannelName, 0, sizeof(pos.csChannelName));
			wsprintfA(pos.csChannelName, "%s", Doc()->m_curChannel.Name);
			pos.iRoomIdx = noRoom;
			pos.iRoomType = -1;
			pos.csChannelName[63] = 0;

			WSendPacket send(0x23);
			send.EncodeBuffer(&pos, sizeof(sUserPosition));
			send.Send(TO_MSN);
			if (CMessengerInfo::Instance()->GetMyStatus() != 3)
				CMessengerInfo::Instance()->SetMyStatus((MSN::eStatusType)4);
		}
		CIntrusion::Instance()->FinishGame();
	}
	break;
	case 0x1ac:
	{
		// 983 009d3276 (C-PKT lote 29): tabela de registros de 0xe bytes em CSharedDoc+0x1146c (chave = dword +4 do registro, valor =
		// o registro alocado). 0063f760 apaga os registros e esvazia o mapa; 006414c0 insere ou, se a chave ja existe, copia por cima
		// (o 983 perde o registro novo nesse caso; aqui ele e apagado).
		std::map<int, int>& table = Doc()->m_mapEvent983;
		for (std::map<int, int>::iterator it = table.begin(); it != table.end(); ++it)
			operator delete((void*)it->second);
		table.clear();
		int count = packet.Decode4();
		for (int i = 0; i < count; ++i)
		{
			unsigned char* pRec = (unsigned char*)operator new(0xe);
			if (pRec == NULL)
				continue;
			packet.DecodeBuffer(pRec, 0xe);
			int key = *(int*)(pRec + 4);
			std::pair<std::map<int, int>::iterator, bool> res = table.insert(std::make_pair(key, (int)pRec));
			if (!res.second)
			{
				memcpy((void*)res.first->second, pRec, 0xe);
				operator delete(pRec);
			}
		}
	}
	break;
	case 0x1f9:
	{
		// 983 009d35cb: hole-in-one (conteiner 0xaa = CHoleInOneGameDoc da frente B): tentativas (1-9; nao muda nos modos de sala
		// 0xc/0xe) e dois valores; CSharedDoc+0x114ac = tentativas * 3.0 (escala lida pelo Scale983 do aiplayer.cpp no mapa 0xb).
		int tries = packet.Decode4();
		int value80 = packet.Decode4();
		int value84 = packet.Decode4();
		_gateway::_golfgame::CHoleInOneGameDoc* pHio = NULL;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0xaa, pHio);
		if (pHio == NULL) // o 983 nao testa
			break;
		unsigned char mode = Doc()->m_roomInfo.realGameType; // byte +0x109b8
		if (mode != 0xc && mode != 0xe)
			pHio->m_7c = Between(1, tries, 9);
		pHio->m_80 = value80;
		pHio->m_84 = value84;
		Doc()->m_courseScale983[0] = (float)(pHio->m_7c * 3.0);
	}
	break;
	case 0x1fa:
	{
		// 983 009d371d: trofeu especial a mais - procura o tid na m_specialTrophyList (+0x37a4); com o mesmo guid soma 1, senao
		// entra um registro novo com contagem 1
		sSpecialTrophy trophy;
		trophy.tid = packet.Decode4();
		trophy.guid = packet.Decode4();
		trophy.count = 1;
		std::list<sSpecialTrophy>& trophies = Doc()->m_specialTrophyList;
		std::list<sSpecialTrophy>::iterator it = trophies.begin();
		while (it != trophies.end() && (*it).tid != trophy.tid)
			++it;
		if (it != trophies.end() && (*it).guid == trophy.guid)
			++(*it).count;
		else
			trophies.push_back(trophy);
	}
	break;
	case 0x20e:
	{
		// 983 009d3801: mapa de CSharedDoc+0x113e0 (chave -> {chave, valor}) refeito; mensagem 0xe8 ao ator principal no fim
		Doc()->m_shopSaleState983.clear();
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (packet.Decode4() != 0)
			break;
		int count = packet.Decode4();
		for (int i = 0; i < count; ++i)
		{
			unsigned long rec[2];
			rec[0] = packet.Decode4();
			rec[1] = packet.Decode4();
			if (rec[0] == 0) // 00642990
				continue;
			sShopSaleState983 value;
			memcpy(&value, rec, 8);
			Doc()->m_shopSaleState983.insert(std::make_pair(rec[0], value));
		}
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0xe8, 0, 0, 0, 0, 0));
	}
	break;
	case 0x21d:
	{
		// 983 009d4b1c: lista de registros de 0x10 (1 byte + 3 dwords) em CSharedDoc+0x3840, que pode vir em mais de um pacote;
		// erro = mensagem 0x27; quando o total chega, mensagem 0x2d (0x20) ao ator principal
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		int total = packet.Decode4();
		int count = packet.Decode4();
		for (int i = 0; i < count; ++i)
		{
			// 983: construtor 00585520 (zera), Decode 009b90e0 (cópia do 005e6010) e CSharedDoc::AddQuestCount983 00641470 (A110)
			_quest::sQuestCount983 rec;
			memset(&rec, 0, sizeof(rec));
			rec.Decode(&packet);
			Doc()->AddQuestCount983(rec);
		}
		if (total == count)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2d, 0x20, 0, 0, 0, 0));
	}
	break;
	case 0x216:
	{
		// 983 009d434a: mudancas de item (sucessor do AddNewItemToDoc do 645). Hora do servidor (32 bits, vai como __time64_t),
		// contagem; por registro: construir, ler (009bfc90) e aplicar pelo CItemUpdater (CSharedDoc 00633680); != 0 = erro
		// (mensagem 0x27 com o codigo, para o laco). Sempre termina com a mensagem 0x102 ao ator principal.
		unsigned long now = packet.Decode4();
		unsigned long count = packet.Decode4();
		for (unsigned long i = 0; i < count; ++i)
		{
			sItemUpdate983 rec;
			rec.Decode983(packet);
			int result = SharedDocUpdateItem983(Doc(), now, rec);
			if (result != 0)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
				break;
			}
		}
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x102, 0, 0, 0, 0, 0));
	}
	break;
	case 0x22f:
	{
		// 983 009d4436: resultado da reciclagem (o 0xed do 645). Msg 2; codigo != 0 -> msg 0x27 com o codigo e o dialogo so
		// destrava (008db940); 0 -> indice (16 bits de um dword), lista de itens de 5 dwords, SetResultRecycleItem e msg 0x106.
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		int result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			RbRecycleDlg* pDlg = (RbRecycleDlg*)g_pFresh->GetManager()->GetDesktop()->FindChildByName("recycledlg");
			if (pDlg == NULL)
				__rtti_obj = pDlg;
			else
				pDlg->ResetMix983();
			break;
		}
		int index = (unsigned short)packet.Decode4();
		std::list<sRecycleItem> itemList;
		for (int n = packet.Decode4(); n != 0; n--)
		{
			sRecycleItem item;
			DecodeRecycleItem983(packet, item);
			itemList.push_back(item);
		}
		RbRecycleDlg* pDlg = (RbRecycleDlg*)g_pFresh->GetManager()->GetDesktop()->FindChildByName("recycledlg");
		if (pDlg == NULL)
		{
			__rtti_obj = pDlg;
			break;
		}
		pDlg->SetResultRecycleItem(true, itemList, (short)index);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x106, 0, 0, 0, 0, 0));
	}
	break;
	case 0x256:
	{
		// PKT C75 (lote 68): so do 983 (009d66f8), resultado dos jogadores de IA do modo solo: erro -> msg 0x27 ao ator
		// principal; senao o documento 0xa3 (CAIPlayerInfoContainer, include/packets/aiplayerinfocontainer.h) le o resto
		// (00910b80) e o "Lobby" recebe a msg 0x87 (so do 983, sem par no 645).
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		CAIPlayerInfoContainer983* pAIDoc = NULL;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0xa3, pAIDoc);
		pAIDoc->ReadPacket983(packet);
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x87, 0, 0, 0, 0, 0);
	}
	break;
	case 0x257:
	{
		// 983 009d2c0b: igual ao 0x15a, mas na 3a lista de trofeus (nova no 983): tipo 5 -> sUserInfoTime::trophies983
		// (+0x3b2c), tipo 0 -> oldSeason.trophies983 (+0x31d2+0x8db); registros de 0xc
		unsigned char type = packet.Decode1();
		unsigned long uid = packet.Decode4();
		unsigned short count = packet.Decode2();
		std::map<unsigned long, sUserInfoTime>::iterator it;
		if (CSharedDoc::Instance()->FindUserInfoTimeUID(uid, it) == 0)
			break;
		std::vector<sSpecialTrophy>* pList;
		if (type == 5)
			pList = &it->second.trophies983;
		else
		{
			if (type != 0)
				break;
			pList = &it->second.oldSeason.trophies983;
		}
		if (!pList->empty())
			pList->clear();
		for (int i = count; i > 0; i--)
		{
			sSpecialTrophy item;
			packet.DecodeBuffer(&item, sizeof(sSpecialTrophy));
			pList->push_back(item);
		}
	}
	break;
	case 0x25d:
	{
		// 983 009d294b: registros de 0xc - tipo 0 na lista de CSharedDoc+0x108e8, tipos 4 e 5 na de +0x3828 (1-3 e acima de 5:
		// nada); a lista e esvaziada antes. O primeiro dword depois do tipo e lido e descartado.
		unsigned char kind = packet.Decode1();
		packet.Decode4();
		int count = packet.Decode4();
		if (kind == 0)
		{
			Doc()->m_specialTrophyList983.clear();
			for (int i = 0; i < count; ++i)
			{
				sSpecialTrophy rec;
				packet.DecodeBuffer(&rec, 0xc);
				Doc()->m_specialTrophyList983.push_back(rec);
			}
		}
		else if (kind == 4 || kind == 5)
		{
			Doc()->m_itemCounts983.clear();
			for (int i = 0; i < count; ++i)
			{
				sItemCount983 rec;
				packet.DecodeBuffer(&rec, 0xc);
				Doc()->m_itemCounts983.push_back(rec);
			}
		}
	}
	break;
	case 0x1b1:
	{
		// PKT C60 (lote 53): so do 983 (009d3354): lista de mercadorias do Self Design (sd_shop::CCommodityList em CSharedDoc+0x1147c).
		// Decode4 = marca (vai para +0x1c), Decode4 = tamanho do bloco comprimido; marca e bloco vazios limpam a lista; sempre msg 0x75.
		unsigned long mark = packet.Decode4();
		int size = (int)packet.Decode4();
		sd_shop::CCommodityList* pList = &Doc()->m_sdCommodityList983;
		if (size > 0)
		{
			if (mark != 0)
			{
				unsigned char* buf = new unsigned char[size];
				memset(buf, 0, size);
				packet.DecodeBuffer(buf, size);
				if (pList)
				{
					CommodityListClear983(*pList);
					pList->m_listVersion = mark;
					CommodityListLoad983(*pList, buf, size);
				}
				if (buf)
					delete[] buf;
			}
		}
		else if (size == 0 && mark == 0 && pList)
		{
			CommodityListClear983(*pList);
			pList->m_listVersion = 0;
		}
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x75, 0, 0, 0, 0, 0));
	}
	break;
	case 0x264:
	{
		// PKT C60 (lote 53): so do 983 (009d6775): resposta da loja bongdari com N registros de 3 valores -> msg 0x328 ao ator
		// principal com os tres vetores e N (CTaskMain::HandleMsgCommon 0065f56c -> FrBongdariShopDlg 008bed00, frente A);
		// erro: msg 0x27 e fecha a janela "bongdari_shop".
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			FrWnd* pDlg = g_pFresh->GetManager()->GetDesktop()->FindChildByName("bongdari_shop");
			if (pDlg)
				pDlg->Close(true);
			else
				__rtti_obj = pDlg;
			break;
		}
		unsigned long count = packet.Decode4();
		std::vector<unsigned int> values1;	// 983: vector de 4 bytes proprio (009bb710/009be380/0065be00)
		std::vector<unsigned long> values2; // 00479490/004c1240/00479770
		std::vector<int> values3;			// 0049e650/004c1d50/0049e810
		for (unsigned long i = count; i != 0; --i)
		{
			values1.push_back(packet.Decode4());
			values2.push_back(packet.Decode4());
			values3.push_back((int)packet.Decode4());
		}
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x328, (int)&values1, (int)&values2, (int)&values3, (int)count, 0));
	}
	break;
	case 0x21b:
	{
		// PKT C74 (lote 67): so do 983 (009d45f3), compra da loja bongdari no lobby (o 0xd4 do 645 com outro fim): erro
		// 0x28737e so atualiza as vezes de desconto; senao msg 0x27; bandeira != 0 = pago com cupom (SetDataFee(1), msg ao ator
		// principal); bolas em 1..5, pang (+0x91e) e cookie (+0x3870), tela do lobby; a janela e avisada e a msg vai ao ator principal.
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		RbBongdariShopDlg* pDlg = (RbBongdariShopDlg*)g_pFresh->GetManager()
									  ->GetDesktop()
									  ->FindChildByName("bongdari_shop");
		if (pDlg)
			pDlg->EnableExitButton(true);
		else
			__rtti_obj = pDlg;
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			if (result == 0x28737e && pDlg)
			{
				pDlg->ShowDiscountUsableTimes();
				pDlg->UpdateUsableTimes();
			}
			else
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			if (pDlg)
				pDlg->StopAutoPlay983();
			break;
		}
		if (packet.Decode4() != 0)
		{
			pDlg->SetDataFee(1); // 983: sem teste de nulo
			if (IActor* pMain = AfxGetTask()->GetMainActor())
				pMain << MsgObject(NULL, M983(0xe1), 0, 0, 0, 0, 0);
		}
		int count = packet.Decode4();
		if ((unsigned int)(count - 1) > 4)
			break;
		sBonusBall balls[5] = { 0 };
		for (int i = 0; i < count; i++)
			packet.DecodeBuffer(&balls[i], sizeof(sBonusBall));
		Doc()->m_myInfo.stat.i64Pang = packet.Decode8();
		Doc()->m_bonusCash = packet.Decode8();
		Doc()->RefreshAllCookie();
		if (RFLobbyDesktop::Instance983()) // 0047f920
			RFLobbyDesktop::Instance983()->Refresh983();
		if (pDlg)
		{
			pDlg->InitBongdariShop();
			pDlg->SetBonusBall(count, balls);
			pDlg->SetBongdariShop();
			pDlg->SetDataFee(0);
		}
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xe1), 0, 0, 0, 0, 0));
	}
	break;
	case 0x26c:
	{
		// PKT C74 (lote 67): so do 983 (009d4867), compra da loja bongdari pelo my room: mesmo erro do 0x21b (008b4830 no
		// fim); bandeira de cupom avisa o "RealMyRoom"; N bolas num vetor; os dois valores de 8 bytes sao lidos e descartados;
		// a janela recebe a lista (008bee10), estado 1 e SetDataFee(2); msg ao "RealMyRoom".
		std::vector<sBonusBall> balls; // 008e82f0 / 008ead20 / 008ea0b0
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		RbBongdariShopDlg* pDlg = (RbBongdariShopDlg*)g_pFresh->GetManager()
									  ->GetDesktop()
									  ->FindChildByName("bongdari_shop");
		if (pDlg)
			pDlg->EnableExitButton(true);
		else
			__rtti_obj = pDlg;
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			if (result == 0x28737e && pDlg)
			{
				pDlg->ShowDiscountUsableTimes();
				pDlg->UpdateUsableTimes();
			}
			else
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			if (pDlg)
				pDlg->EnableAfterResult983();
			break;
		}
		if (packet.Decode4() != 0)
		{
			pDlg->SetDataFee(1); // 983: sem teste de nulo
			if (IActor* pRoom = AfxGetTask()->GetActor("RealMyRoom"))
				pRoom << MsgObject(NULL, M983(0xe1), 0, 0, 0, 0, 0);
		}
		int count = packet.Decode4();
		for (int i = count; i > 0; i--)
		{
			sBonusBall ball = { 0 };
			packet.DecodeBuffer(&ball, sizeof(sBonusBall));
			balls.push_back(ball);
		}
		packet.Decode8();
		packet.Decode8();
		if (pDlg)
		{
			pDlg->InitBongdariShop();
			pDlg->SetResultList983(count, balls);
			pDlg->SetStateReady983();
			pDlg->SetDataFee(2);
		}
		AfxGetTask()->GetActor("RealMyRoom") << MsgObject(NULL, M983(0xe1), 0, 0, 0, 0, 0);
		if (balls.size() != 0) // 008b4f80 / 008ea0e0
			balls.clear();
	}
	break;
	case 0x26d:
	{
		// 983 009d69ff -> CSharedDoc 00642be0 (hora, 0): hora do servidor (time_t de 32 bits) em hora local, texto 0x13882 com
		// ano, mes, dia, hora e minuto em CSharedDoc+0x1160c; byte +0x1160a = 1. Hora 0 nao muda nada.
		long t = (long)packet.Decode4();
		if (t == 0)
			break;
		__int64 v = ((__int64)t + 11644473600LL) * 10000000; // 00b30780
		FILETIME ft, localFt;
		SYSTEMTIME st;
		ft.dwLowDateTime = (DWORD)v;
		ft.dwHighDateTime = (DWORD)(v >> 32);
		FileTimeToLocalFileTime(&ft, &localFt); // 00b307f0
		FileTimeToSystemTime(&localFt, &st);
		Doc()->m_premiumEndNoticeText983 = MakeStr(GetGameString(0x13882), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
		Doc()->m_bPremiumEndNotice983 = 1;
	}
	break;
	case 0x26e:
	{
		// PKT C76 (lote 69): 0x26e-0x273 so do 983 (maestria do personagem, janelas RF). Todos: msg 2 ao ator principal; erro ->
		// msg 0x27. 009d6a1a: redesenha a janela de status e a de quadros
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		if (RbCharMasteryStatusDlg983::IsCreated983())
			RbCharMasteryStatusDlg983::ms_pInstance->Refresh983();
		if (RbCharMasteryTileDlg983::IsCreated983())
			RbCharMasteryTileDlg983::ms_pInstance->Refresh983();
	}
	break;
	case 0x26f:
	{
		// 009d6ab6: resultado na janela de status (1) e redesenha
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		unsigned long value = packet.Decode4();
		if (RbCharMasteryStatusDlg983::IsCreated983())
		{
			RbCharMasteryStatusDlg983::ms_pInstance->ShowResult983(value, true);
			RbCharMasteryStatusDlg983::ms_pInstance->Refresh983();
		}
	}
	break;
	case 0x270:
	{
		// 009d6b55: como o 0x26f com 0
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		unsigned long value = packet.Decode4();
		if (RbCharMasteryStatusDlg983::IsCreated983())
		{
			RbCharMasteryStatusDlg983::ms_pInstance->ShowResult983(value, false);
			RbCharMasteryStatusDlg983::ms_pInstance->Refresh983();
		}
	}
	break;
	case 0x271:
	{
		// 009d6bdd: resultado na janela de cartas (1), personagem em uso (CSharedDoc+0xa2a) e redesenha a janela de status
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		unsigned long value = packet.Decode4();
		if (RbCharMasteryCardDlg983::IsCreated983())
		{
			RbCharMasteryCardDlg983::ms_pInstance->ShowResult983(value, true);
			RbCharMasteryCardDlg983::ms_pInstance->SetCharacter983(CSharedDoc::Instance()->m_myInfo.userEquip.guidChar);
		}
		if (RbCharMasteryStatusDlg983::IsCreated983())
			RbCharMasteryStatusDlg983::ms_pInstance->Refresh983();
	}
	break;
	case 0x272:
	{
		// 009d6c9d: igual ao 0x271
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		unsigned long value = packet.Decode4();
		if (RbCharMasteryCardDlg983::IsCreated983())
		{
			RbCharMasteryCardDlg983::ms_pInstance->ShowResult983(value, true);
			RbCharMasteryCardDlg983::ms_pInstance->SetCharacter983(CSharedDoc::Instance()->m_myInfo.userEquip.guidChar);
		}
		if (RbCharMasteryStatusDlg983::IsCreated983())
			RbCharMasteryStatusDlg983::ms_pInstance->Refresh983();
	}
	break;
	case 0x273:
	{
		// 009d6d5d: como o 0x271 com 0
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
			break;
		}
		unsigned long value = packet.Decode4();
		if (RbCharMasteryCardDlg983::IsCreated983())
		{
			RbCharMasteryCardDlg983::ms_pInstance->ShowResult983(value, false);
			RbCharMasteryCardDlg983::ms_pInstance->SetCharacter983(CSharedDoc::Instance()->m_myInfo.userEquip.guidChar);
		}
		if (RbCharMasteryStatusDlg983::IsCreated983())
			RbCharMasteryStatusDlg983::ms_pInstance->Refresh983();
	}
	break;
	case 0x26a:
	{
		// 983 009d693f: erro = mensagem 0x27; sempre avisa a tela (0x355) se a previa da trajetoria esta ligada
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result != 0)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
		bool bOn = DOCJP()->IsTrajectoryPreviewOn();
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x355, bOn, 0, 0, 0, 0));
	}
	break;
	case 0x274:
	{
		// 983 009d6de5: 0 = dois valores para a tela (mensagens 0x2e5, 0x2e6 e 0x2e4 com os dois); 0x5b8d8b = texto
		// 0x18711 da tabela; outro numero = erro do servidor
		const char* __fastcall GetGameString(int key); // 00540a10 (include/jp/gamestringmanager.h)
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		unsigned long result = packet.Decode4();
		if (result == 0)
		{
			unsigned long first = packet.Decode4();
			unsigned long second = packet.Decode4();
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2e5, 0, 0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2e6, 0, 0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2e4, first, second, 0, 0, 0));
		}
		else if (result == 0x5b8d8b)
		{
			std::string text(GetGameString(0x18711));
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)text.c_str(), 0, 0, 0, 0));
		}
		else
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27, result, 0, 0, 0, 0));
	}
	break;
	case 0x32:
	case 0x62:
	case 0x101:
	case 0x126:
	case 0x143:
		break;
	// D752: 983 009d26a7 - Doc+0x11640 = 1 (o 0x52 zera, 009c2ba1); o nosso caia no break vazio
	case 0x199:
		Doc()->SetPangyaBySet983(true);
		break;
	// PKT (lote 54): no 983 tambem vao ao 009d78a7 (o 645 lia aqui a lista de presentes sGiftInfo)
	case 0x7a:
	case 0x10a:
	case 0x145:
		break;
	default:
		return 0;
	}

	return 1;
}

bool CTask::OnOverlappedDlg(int result, FrForm* pForm)
{
	g_bQuit = true;
	return true;
}

int CTask::_RemoveWareHouseItem(unsigned long tid, IActor* pActor, int count)
{
	unsigned long guid = 0;
	std::list<sItemInfo>::iterator it;
	if (tid == 0x1a000042 && IsLocalContent(S3_CADDIE_REPORT))
	{
		// 983 (009bdeb0): 0x281 ao ator principal da tarefa (o 645 mandava 0x204 ao "RealMyRoom")
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x204), (int)&guid, 0, 0, 0, 0));
		for (it = CSharedDoc::Instance()->m_myItemList.begin();
			it != CSharedDoc::Instance()->m_myItemList.end(); ++it)
		{
			if ((*it).guid == guid)
				break;
		}
	}
	else
	{
		for (it = CSharedDoc::Instance()->m_myItemList.begin();
			it != CSharedDoc::Instance()->m_myItemList.end(); ++it)
		{
			if ((*it).tid == tid)
				break;
		}
	}
	if (it == CSharedDoc::Instance()->m_myItemList.end())
		return 0;

	int i;
	for (i = 0; i < count; i++)
	{
		if (--(*it).Common[0] <= 0)
		{
			CSharedDoc::Instance()->m_myItemList.erase(it);
			break;
		}
	}
	pActor << MsgObject(NULL, M983(0xdd), 0, 0, 0, 0, 0);
	return i;
}

int CTask::_RemoveWareHouseItem(unsigned long tid, const char* actorName,
	int count)
{
	return _RemoveWareHouseItem(tid, AfxGetTask()->GetActor(actorName), count);
}

void CTask::_ReFreshMyRoomGift(bool bRefresh, std::string name)
{
	if (bRefresh)
	{
		WSendPacket packet(0x21);
		packet.EncodeStr(name);
		unsigned long uid = CSharedDoc::Instance()->m_myInfo.info.dwUID;
		packet.Encode4(uid);
		packet.Send(TO_MSN);
	}
	else
	{
		CSharedDoc::Instance()->m_giftList.clear();
		WSendPacket packet((enumClientPacket)0x96); // JP 983 (s63): 0x93 no KR 645
		packet.Send(TO_GAME);
	}
}

CTaskManager::CTaskManager()
	: m_bChangeTask(false),
	  m_bPreserveTask(false),
	  m_nextTaskName(NULL),
	  m_nextDocName(NULL),
	  m_pCurrentTask(NULL),
	  m_pPreservedTask(NULL),
	  m_pFade(NULL),
	  m_pWork(NULL)
{
	m_historyPos = m_history.begin();
	m_naviMode = 0;
	m_pad983_3c = NULL; // 983: último store do construtor (009c150b)
	// D868: o m_unused40 das outras unidades (taskmanager.h) tem que cobrir m_pFade..m_naviMode nos dois compiladores
#if defined(_MSC_VER) && _MSC_VER > 1600
	C_ASSERT(offsetof(CTaskManager, m_pWork) - offsetof(CTaskManager, m_pFade) == REBANG_TASKMANAGER_UNUSED40);
#else // VC10: sem sizeof(std::queue<int>) (instanciaria deque<int> e mudaria o obj)
	C_ASSERT(offsetof(CTaskManager, m_pWork) - offsetof(CTaskManager, m_pFade) == 0x4c);
#endif
}

CTaskManager::~CTaskManager()
{
	ClearTask();
	if (m_pPreservedTask)
	{
		m_pPreservedTask->Destroy();
		delete m_pPreservedTask;
		m_pPreservedTask = NULL;
	}
	if (m_pFade)
	{
		delete m_pFade;
		m_pFade = NULL;
	}
}

// B1589: troca de gear no Praticar: o sUserInfo do 0x76 traz o equipamento novo? (status = SetPlayerLevel sobre ele; PEDIDOS 14:07)
static void RastroGear0x76B1589(const char* ramo, const sUserInfo& ui)
{
	const sUserEquip& eq = Doc()->m_myInfo.userEquip;
	std::map<unsigned int, sItemInfo>::const_iterator it = Doc()->m_clubSetMap.find(eq.guidClubSet);
	const bool bAchou = it != Doc()->m_clubSetMap.end();
	jp983_rastro("B1589 0x76 %s clube guid=%08x tid=%08x PCL=%d/%d/%d/%d/%d | myInfo guid=%08x mapa tid=%08x PCL=%d/%d/%d/%d/%d | char %08x x %08x caddie %08x x %08x",
		ramo, ui.clubInfo.guid, ui.clubInfo.tid, ui.clubInfo.PCL[0], ui.clubInfo.PCL[1], ui.clubInfo.PCL[2], ui.clubInfo.PCL[3], ui.clubInfo.PCL[4],
		eq.guidClubSet, bAchou ? it->second.tid : 0, bAchou ? it->second.Common[0] : -1, bAchou ? it->second.Common[1] : -1,
		bAchou ? it->second.Common[2] : -1, bAchou ? it->second.Common[3] : -1, bAchou ? it->second.Common[4] : -1,
		ui.charInfo.guid, eq.guidChar, ui.caddieInfo.guid, eq.guidCaddie);
}

void CTaskManager::CreateTask(const char* task, const char* doc, bool preserve)
{
	if (preserve)
	{
		if (m_pPreservedTask == NULL)
		{
			m_pPreservedTask = m_pCurrentTask;
			m_pPreservedTask->PreserveBack(task);
			m_pCurrentTask = NULL;
		}
		ClearTask();
	}
	else
	{
		ClearTask();
		if (m_pPreservedTask)
		{
			if (stricmp(m_pPreservedTask->GetRTTI()->GetName(), task) == 0)
			{
				m_pCurrentTask = m_pPreservedTask;
				m_pCurrentTask->RestorePreserved();
				m_pPreservedTask = NULL;
				m_bChangeTask = false;
				m_nextTaskName = NULL;
				m_nextDocName = NULL;
				if (m_pFade)
					m_pFade->ResetScreenSize();
				return;
			}
			m_pPreservedTask->Destroy();
			delete m_pPreservedTask;
			m_pPreservedTask = NULL;
		}
	}

	m_pCurrentTask = (CTask*)ObjectFactory().CreateObject(task);
	m_pCurrentTask->ApplyScreenSize();
	if (m_pFade)
		m_pFade->ResetScreenSize();

	if (doc == NULL || *doc == '\0')
	{
		m_pCurrentTask->m_pDoc = NULL;
	}
	else
	{
		IObject* pDoc = ObjectFactory().CreateObject(doc);
		__rtti_obj = pDoc;
		m_pCurrentTask->m_pDoc =
			pDoc ? (CTaskDoc*)pDoc->DynamicCast(&CTaskDoc::m_RTTI) : NULL;
		if (m_pCurrentTask->m_pDoc)
			m_pCurrentTask->m_pDoc->m_pTask = m_pCurrentTask;
	}

	m_pCurrentTask->Register();
	m_pCurrentTask->PreLoadInit();
	g_LoadParam.loaded = 0;
	g_LoadParam.bDone = false;
	g_LoadParam.bLocked = false;
	m_bPreserveTask = preserve;
	m_bChangeTask = false;
	m_nextTaskName = NULL;
	m_nextDocName = NULL;
	m_pCurrentTask->Load();
}

// 983 009bf6e0 (pedido C46.1): widget pelo nome por cima da tarefa corrente
void CTaskManager::OpenWidget983(const char* name, const char* loadingMsg)
{
	IObject* pObj = ObjectFactory().CreateObject(name);
	if (pObj == NULL)
	{
		m_pCurrentTask->DeleteWidget983();
		return;
	}
	CTaskWidget* pWidget = DYNAMIC_CAST(CTaskWidget, pObj);
	if (pWidget)
	{
		m_pCurrentTask->CloseWidget983();
		m_pCurrentTask->SetWidget983(pWidget);
		pWidget->SetLoadingMsg983(loadingMsg);
		pWidget->Start983();
	}
	else
		delete pObj;
}

void CTaskManager::ChangeTask(const char* taskName, const char* docName,
	bool bPreserve)
{
	CMessengerInfo::Instance()->CloseWindow();
	m_nextTaskName = taskName;
	m_nextDocName = docName;
	m_bChangeTask = true;
	m_bPreserveTask = bPreserve;
	g_audio->SetFadeOutTime(1.0f);
	if (m_pFade)
	{
		delete m_pFade;
		m_pFade = NULL;
	}
	m_pFade = new CBackGround(NULL, true);
	m_fadeAlpha = 1.0f;
	Rf983ChangeTask(); // 009b96ac..009b96cf
}

void CTaskManager::PreProcess()
{
	if (m_bChangeTask)
		CreateTask(m_nextTaskName, m_nextDocName, m_bPreserveTask);
}

void CTaskManager::ProcessTask(float delta)
{
	if (m_bChangeTask)
		return;

	// 983 009c0bde: sem papel de parede na carga a foto nao esmaece antes do fim
	if (m_pFade && (m_pCurrentTask->IsLoadingWallPaperVisible983() || g_LoadParam.bLocked))
	{
		float rate = m_pCurrentTask->m_bSyncLoad ? 0.8f : 1.0f;
		float step = rate * delta;
		if (step > rb_fadeStepMax)
			step = rb_fadeStepMax;
		m_fadeAlpha -= step;
		if (m_fadeAlpha < 0.0f)
		{
			m_fadeAlpha = 0.0f;
			delete m_pFade;
			m_pFade = NULL;
		}
	}

	if (!g_LoadParam.bDone)
	{
		m_pCurrentTask->OnProcessWhileLoading(delta);
		Sleep(30);
		return;
	}

	if (!g_LoadParam.bLocked)
	{
		m_pCurrentTask->Init(NULL);
		g_LoadParam.bLocked = true;
	}
	// 983 (009c0bc0): primeiro a fila nova, direto ao ator principal (vt+0x2c = HandleMsg)
	while (!m_msgQueue983.empty())
	{
		sPostMsg& postMsg = m_msgQueue983.front();
		if (m_pCurrentTask->m_pMainActor)
			m_pCurrentTask->m_pMainActor << postMsg.msg;
		m_msgQueue983.pop();
	}
	while (!m_msgQueue.empty())
	{
		sPostMsg& postMsg = m_msgQueue.front();
		sActor* pActor =
			m_pCurrentTask->m_actorList.Find(postMsg.target.c_str());
		if (pActor)
			pActor->actor << postMsg.msg;
		m_msgQueue.pop();
	}
	m_pCurrentTask->Process(delta);
	Rf983Update(delta);
}

void CTaskManager::DisplayTask() const
{
	if (0)
		((CTaskManager*)0)
			->CTaskManager::ChangeTask(*(const char**)0, *(const char**)0,
				*(bool*)0);
	if (!g_LoadParam.bLocked)
	{
		m_pCurrentTask->OnDisplayWhileLoading();
		// 983 009b9719: sem papel de parede, a foto da tarefa anterior cobre a tela de carga
		if (m_pFade && !m_pCurrentTask->IsLoadingWallPaperVisible983())
			m_pFade->Draw(-1);
	}
	else if (!m_bChangeTask)
	{
		m_pCurrentTask->Display();
		Rf983Draw();
	}

	if (m_pFade)
		m_pFade->Draw(((int)(m_fadeAlpha * 255.0f) << 24) | 0xffffff);
}

void CTaskManager::ClearTask()
{
	if (CCapturedBg::Instance())
		CCapturedBg::Instance()->Reset();
	if (m_pCurrentTask)
	{
		m_pCurrentTask->Destroy();
		delete m_pCurrentTask;
		m_pCurrentTask = NULL;
	}
	if (HackingManager::Instance())
		HackingManager::Instance()->ClearCrypticList();
}

CTask* CTaskManager::GetCurrentTask() const
{
	return m_pCurrentTask;
}

int CTaskManager::PostMsg(const IActor* sender, const char* target, int message,
	int param1, int param2, int param3, unsigned long time)
{
	sPostMsg postMsg;
	postMsg.msg.sender = sender;
	postMsg.msg.message = message;
	postMsg.msg.param1 = param1;
	postMsg.msg.param2 = param2;
	postMsg.msg.param3 = param3;
	postMsg.msg.time = time;
	postMsg.target = target;
	m_msgQueue.push(postMsg);
	return 0;
}

// 983 009c16a0 (s100): como o PostMsg, sem alvo (string vazia) e sem time, na fila de +0x48
void CTaskManager::PostMsgToMainActor983(const IActor* sender, int message,
	int param1, int param2, int param3)
{
	sPostMsg postMsg;
	postMsg.msg.sender = sender;
	postMsg.msg.message = message;
	postMsg.msg.param1 = param1;
	postMsg.msg.param2 = param2;
	postMsg.msg.param3 = param3;
	m_msgQueue983.push(postMsg);
}

CTaskDoc::CTaskDoc()
{
}

CTaskDoc::~CTaskDoc()
{
}

CTask* CTaskDoc::GetTask() const
{
	return m_pTask;
}

void CTaskManager::NaviOpenLayout(const char* task, const char* doc,
	const char* actor, const char* layout)
{
	switch (m_naviMode)
	{
	case 0:
		if (IsNaviForwardEnable())
		{
			if (IsNaviForwardSameHistory(layout))
			{
				++m_historyPos;
				break;
			}
			m_history.erase(++m_historyPos, m_history.end());
			NaviAddHistory(task, doc, actor, layout);
		}
		else
		{
			NaviAddHistory(task, doc, actor, layout);
		}
		break;
	case 1:
		--m_historyPos;
		break;
	}
	((CTaskMain*)AfxGetTask()->GetMainActor())->CheckNaviButtons();
	m_naviMode = 0;
}

void CTaskManager::NaviAddHistory(const char* task, const char* doc,
	const char* actor, const char* layout)
{
	history_t history;
	history.task = task;
	history.doc = doc;
	history.layout = layout;
	history.actor = actor;
	m_historyPos = m_history.insert(m_history.end(), history);
}

void CTaskManager::NaviClearHistory()
{
	m_history.clear();
}

void CTaskManager::NaviGoForward()
{
	std::list<history_t>::iterator it = m_historyPos;
	if (it == m_history.end() || ++it == m_history.end())
		return;

	history_t history = *it;
	sActor* pActor = AfxGetTask()->m_actorList.Find(history.actor);
	if (pActor == NULL || pActor->actor == NULL)
		CTaskManager::Instance()->ChangeTask(history.task, history.doc, false);
	AfxPostMsg(NULL, history.actor, 0, (int)history.layout, 1, 0, 0);
	m_naviMode = 0;
}

void CTaskManager::NaviGoBackward()
{
	if (m_historyPos == m_history.begin())
		return;

	std::list<history_t>::iterator it = m_historyPos;
	history_t history = *--it;
	sActor* pActor = AfxGetTask()->m_actorList.Find(history.actor);
	if (pActor == NULL || pActor->actor == NULL)
		CTaskManager::Instance()->ChangeTask(history.task, history.doc, false);
	AfxPostMsg(NULL, history.actor, 0, (int)history.layout, 1, 0, 0);
	m_naviMode = 1;
}

bool CTaskManager::IsNaviForwardEnable()
{
	std::list<history_t>::iterator it = m_historyPos;
	if (++it == m_history.end())
		return false;
	return true;
}

bool CTaskManager::IsNaviBackwardEnable()
{
	if (m_historyPos == m_history.begin())
		return false;
	return true;
}

bool CTaskManager::IsNaviForwardSameHistory(const char* layout)
{
	std::list<history_t>::iterator it = m_historyPos;
	++it;
	if (_stricmp((*it).layout, layout) == 0)
		return true;
	return false;
}

void CTaskManager::NaviDisplayDebugInfo()
{
	int i = 0;
	float y = 200.0f;
	for (std::list<history_t>::iterator it = m_history.begin();
		it != m_history.end(); ++it, ++i)
	{
		history_t history = *it;
		TextOut(g_view, 5.0f, y, "%d: (%s)%s%s", i, history.actor,
			history.layout, it == m_historyPos ? "*" : "");
		y += 15.0f;
	}
}

void CTaskManager::AddNewItemToDoc(unsigned int type, WReceivedPacket& packet)
{
	switch (type)
	{
	case 1:
	{
		sCharacterInfo info;
		packet.DecodeBuffer(&info, sizeof(sCharacterInfo));
		CSharedDoc::Instance()->m_itemManager.GetDefCombo(info.tid,
			info.tidParts);
		CSharedDoc::Instance()->m_charMap.insert(
			std::map<unsigned int, sCharacterInfo>::value_type(info.guid,
				info));
	}
	break;
	case 2:
	{
		sItemInfo info;
		packet.DecodeBuffer(&info, sizeof(sItemInfo));
		if (IsLocalContent(S4_UCC) && IsUccClothes(info.tid))
			CSharedDoc::Instance()->m_uccItemList.push_back(info);
		else
			CSharedDoc::Instance()->AddMyPartsList(info);
		if (IsAngelWing(info.tid))
			CSharedDoc::Instance()->CheckAngelWing();
		if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/) &&
			(info.tid & 0x1800) == 0x1800)
			CSharedDoc::Instance()->SetInitHaveHalloweenItem();
	}
	break;
	case 4:
	{
		sItemInfo info;
		packet.DecodeBuffer(&info, sizeof(sItemInfo));
		CSharedDoc::Instance()->m_clubSetMap.insert(
			std::map<unsigned int, sItemInfo>::value_type(info.guid, info));
	}
	break;
	case 5:
	{
		sItemInfo info;
		packet.DecodeBuffer(&info, sizeof(sItemInfo));
		std::list<sItemInfo>::iterator it;
		for (it = CSharedDoc::Instance()->m_ballList.begin();
			it != CSharedDoc::Instance()->m_ballList.end(); ++it)
		{
			if ((*it).guid == info.guid)
			{
				(*it).Common[0] = info.Common[0] < 1 ? 1 : info.Common[0];
				break;
			}
		}
		if (it == CSharedDoc::Instance()->m_ballList.end())
			CSharedDoc::Instance()->m_ballList.push_back(info);
	}
	break;
	case 6:
	{
		sItemInfo info;
		packet.DecodeBuffer(&info, sizeof(sItemInfo));
		CSharedDoc* pDoc = CSharedDoc::Instance();
		if (info.ItemType != 0)
		{
			if (info.ItemType == 3)
				info.HourRemain = info.Common[0];
			else if (info.ItemType == 4)
				info.HourRemain = info.Common[0] * 24;
			else if (info.ItemType == 5)
				info.HourRemain = info.Common[0] * 720;
		}
		else
		{
			for (std::list<sItemInfo>::iterator it = pDoc->m_myItemList.begin();
				it != pDoc->m_myItemList.end(); ++it)
			{
				if ((*it).guid == info.guid)
				{
					(*it).Common[0] = info.Common[0] < 1 ? 1 : info.Common[0];
					(*it).HourRemain += info.HourRemain;
					return;
				}
			}
		}
		pDoc->m_myItemList.push_back(info);
	}
	break;
	case 7:
	{
		sCaddieInfo info;
		packet.DecodeBuffer(&info, sizeof(sCaddieInfo));
		CSharedDoc::Instance()->m_caddieMap.insert(
			std::map<unsigned int, sCaddieInfo>::value_type(info.guid, info));
	}
	break;
	case 8:
	{
		unsigned long guid = 0;
		unsigned long tidPart = 0;
		int partDate = 0;
		packet.DecodeBuffer(&guid, 4);
		packet.DecodeBuffer(&tidPart, 4);
		packet.DecodeBuffer(&partDate, 4);
		CSharedDoc* pDoc = CSharedDoc::Instance();
		std::map<unsigned int, sCaddieInfo>::iterator it =
			pDoc->m_caddieMap.find(guid);
		if (it != pDoc->m_caddieMap.end())
		{
			it->second.tidPart = tidPart;
			it->second.Remain_Partdate = partDate;
			if (it->second.Remain_Date < partDate)
				it->second.Remain_Date = partDate;
		}
	}
	break;
	case 0xe:
	{
		sItemInfo info;
		packet.DecodeBuffer(&info, sizeof(sItemInfo));
		info.HourRemain = info.Common[0] * 24;
		for (std::list<sItemInfo>::iterator it =
				 CSharedDoc::Instance()->m_cutinList.begin();
			it != CSharedDoc::Instance()->m_cutinList.end(); ++it)
		{
			if ((*it).tid == info.tid || (*it).guid == info.guid)
				return;
		}
		CSharedDoc::Instance()->m_cutinList.push_back(info);
	}
	break;
	case 0xf:
	{
		unsigned long hairTid = 0;
		packet.DecodeBuffer(&hairTid, 4);
		sCharacterInfo info;
		packet.DecodeBuffer(&info, sizeof(sCharacterInfo));
		CSharedDoc::Instance()->m_itemManager.GetDefCombo(info.tid,
			info.tidParts);
		CSharedDoc::Instance()
			->m_charMap[CSharedDoc::Instance()->m_myInfo.userEquip.guidChar];
		IFF_STRUCT::sHairStyle* pHair = ItemManager()->FindHairStyle(hairTid);
		if (pHair)
		{
			std::map<unsigned int, sCharacterInfo>& charMap =
				CSharedDoc::Instance()->m_charMap;
			for (std::map<unsigned int, sCharacterInfo>::iterator it =
					 charMap.begin();
				it != charMap.end(); ++it)
			{
				if (it->second.tid == (pHair->cCharID | 0x4000000))
				{
					it->second.hairClr = pHair->cHairID;
					return;
				}
			}
		}
	}
	break;
	case 0x10:
	{
		sMascotInfo info;
		packet.DecodeBuffer(&info, sizeof(sMascotInfo));
		strcpy(info.szMsg, "PANGYA!");
		CSharedDoc::Instance()->AddMascot(&info);
	}
	break;
	case 0x1c:
	{
		sItemInfo info;
		packet.DecodeBuffer(&info, sizeof(sItemInfo));
		std::list<sItemInfo>::iterator it;
		for (it = CSharedDoc::Instance()->m_auxPartList.begin();
			it != CSharedDoc::Instance()->m_auxPartList.end(); ++it)
		{
			if ((*it).guid == info.guid)
			{
				(*it).Common[0] = info.Common[0] < 1 ? 1 : info.Common[0];
				break;
			}
		}
		if (it == CSharedDoc::Instance()->m_auxPartList.end())
			CSharedDoc::Instance()->m_auxPartList.push_back(info);
	}
	break;
	// 983 009bff50: sem o caso 0x1d do 645 (m_questDropList); cai no default (nada)
	case 0x1f:
	{
		sCards card;
		packet.DecodeBuffer(&card, sizeof(sCards));
		if (ItemManager()->FindCard(card.tid))
		{
			for (std::list<sCards>::iterator it =
					 CSharedDoc::Instance()->m_cardList.begin();
				it != CSharedDoc::Instance()->m_cardList.end(); ++it)
			{
				if ((*it).tid == card.tid)
				{
					(*it).cardCount += card.cardCount < 1 ? 1 : card.cardCount;
					return;
				}
			}
			CSharedDoc::Instance()->m_cardList.push_back(card);
		}
	}
	break;
	}
}

// JP 983 (s158): nomes que este arquivo cita por uma visao (ou com a assinatura um pouco diferente: ponteiro x referencia,
// const, int x unsigned long, void x bool; ABI igual no x86) e que nao existiam com esse nome -> viravam stub no exe.
// Cada um aponta para a funcao/dado real (lista e motivo em out/F3.5/s158/apelidos.tsv).
#pragma comment(linker, "/alternatename:?IsCreated983@RbCharMasteryCardDlg983@@SI_NXZ=?IsCreated983@RFCharMasteryCardDlg@@SI_NXZ") // mesma VA 005b87d0
#pragma comment(linker, "/alternatename:?ShowResult983@RbCharMasteryCardDlg983@@QAEXK_N@Z=?OnResult983@RFCharMasteryCardDlg@@QAEXK_N@Z") // mesma VA 005bb820
#pragma comment(linker, "/alternatename:?SetCharacter983@RbCharMasteryCardDlg983@@QAEXK@Z=?SetChar983@RFCharMasteryCardDlg@@QAEXK@Z") // mesma VA 005bbdb0
#pragma comment(linker, "/alternatename:?IsCreated983@RbCharMasteryStatusDlg983@@SI_NXZ=?IsCreated983@RFCharMasteryStatusDlg@@SI_NXZ") // mesma VA 005bd3d0
#pragma comment(linker, "/alternatename:?IsCreated983@RbCharMasteryTileDlg983@@SI_NXZ=?IsCreated983@RFCharMasteryTileDlg@@SI_NXZ") // mesma VA 005bd3e0
#pragma comment(linker, "/alternatename:?Refresh983@RbCharMasteryTileDlg983@@QAEXXZ=?PlaySlotAni983@RFCharMasteryTileDlg@@QAEXXZ") // mesma VA 005c59e0
#pragma comment(linker, "/alternatename:?ShowResult983@RbCharMasteryStatusDlg983@@QAEXK_N@Z=?OnResult983@RFCharMasteryStatusDlg@@QAEXK_N@Z") // mesma VA 005c9b00
#pragma comment(linker, "/alternatename:?Refresh983@RbCharMasteryStatusDlg983@@QAEXXZ=?Refresh983@RFCharMasteryStatusDlg@@QAEXXZ") // mesma VA 005cc980
#pragma comment(linker, "/alternatename:?ms_pInstance@RbCharMasteryCardDlg983@@2PAU1@A=?ms_pInstance@RFCharMasteryCardDlg@@2PAV1@A") // dado, mesma VA 00e7813c
#pragma comment(linker, "/alternatename:?ms_pInstance@RbCharMasteryTileDlg983@@2PAU1@A=?ms_pInstance@RFCharMasteryTileDlg@@2PAV1@A") // dado, mesma VA 00e78190
#pragma comment(linker, "/alternatename:?ms_pInstance@RbCharMasteryStatusDlg983@@2PAU1@A=?ms_pInstance@RFCharMasteryStatusDlg@@2PAV1@A") // dado, mesma VA 00e781c8
