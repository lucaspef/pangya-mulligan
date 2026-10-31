#pragma once
#include "msg983.h"

#include <memory>
#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
#include <list>
#include <queue>
#include "msgobject.h"
#endif
#if !defined(REBANG_TASKMANAGER_PCH) && defined(_MSC_VER) && _MSC_VER > 1600
#include <list> // D868: tamanho do m_unused40 no cl novo
#include <queue>
#endif
#include "work.h"
#include "anotherhand.h"
#include "frcmdtarget.h"

class CPostEffect983;
class IActor;
#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
class CBackGround;
#else
class MsgObject;
#endif

class CTaskDoc;
class Fresh;
class FrForm;
class FrTaskForm;
class CTaskWidget;
class FrGaugeBar;
class WReceivedPacket;
struct sActor;

class CTask : public IObject, public CHandOwner, public FrCmdTarget
{
public:
	static const WRTTI m_RTTI;
#ifdef REBANG_TASKMANAGER_PCH // HACK: defined in taskmanager.cpp (emission order)
	virtual const WRTTI* GetRTTI() const;
#else
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; }
#endif

	struct TIP_ITEM
	{
		int id;
		std::string text;
	};

	CTask();
	virtual ~CTask();

	IActor* AddActor(const char* className, const char* name, int skipLayer,
		const std::string& option);
	IActor* GetActor(const char* name);
	bool IsCurrentActor983(const char* name); // S65: 983 009bd4f0
	void SetInited(bool bInited);
	IActor* GetMainActor() const { return m_pMainActor; }
	CTaskDoc* GetDocument() const { return m_pDoc; }

	virtual void ApplyScreenSize();
#ifdef REBANG_TASKMANAGER_PCH // HACK: defined in taskmanager.cpp (emission order)
	virtual void Register();
#else
	virtual void Register() { }
#endif
	virtual void Init(const char* wallPaper);
	virtual void PreLoadInit();
	virtual void Reload();
	virtual void ProcessNetSyncActor(bool bForce, float step, int ballState);
	virtual void Process(float delta);
	virtual void Display();
	virtual void Destroy();
	virtual void SendGlobalMsg(MsgObject msg);
	virtual void SendMsgToMainActor(MsgObject msg);
#ifdef REBANG_TASKMANAGER_PCH // HACK: defined in taskmanager.cpp (emission order)
	virtual int OnPacket(WReceivedPacket& packet);
#else
	virtual int OnPacket(WReceivedPacket& packet) { return 0; }
#endif
	virtual void OnInitFinished() { }

#ifdef REBANG_GOLFTASK_PCH // golftask.cpp: body in stdafx.h / at the original PCH position (emission order)
	bool IsInited();
#else
	bool IsInited() { return (m_taskFlag & 2) ? true : false; }
#endif
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: body in golfrule_pch_inl.h / at the original PCH position (emission order)
	void SetPause(bool bPause);
#else
	void SetPause(bool bPause) { m_bPause = bPause; }
#endif
	bool IsRecieveWhisper() { return m_bRecieveWhisper; }
	CPostEffect983* GetPostEffect983() const { return m_pPostEffect983; } // 983: COption::CheckPendingChanges (00497530) le os bytes 0..6 e o int +8
	void SetWhisper(bool bWhisper) { m_bRecieveWhisper = bWhisper; }
	// 983 (009b7810, s100): form do CTaskWidget guardado em +0x1fc (NULL quando lá está o boneco vazio ms_pDummyActor);
	// 21 chamadores (ex.: MakeRealMyRoom compara o nome com "realmyroom"). Nome meu.
	FrTaskForm* GetWidgetForm983() const;
	void ForgetWidgetForm983(FrTaskForm* pForm);
	// 983 (pedido C46.1, frente C): +0x1fc = widget aberta por cima da tarefa (CMyRoomWidget, CShopWidget...) ou o
	// boneco ms_pNullWidget983; nunca nulo depois do construtor.
	void SetWidget983(CTaskWidget* pWidget); // 009b7780
	void DeleteWidget983(); // 009b7790: apaga a widget e volta o boneco
	void CloseWidget983(); // 009b77d0: idem (o boneco carregado antes do teste)

protected:
	virtual void OnProcessWhileLoading(float delta);
	virtual void OnDisplayWhileLoading();
	bool IsLoadingWallPaperVisible983() const; // 009b9570
	virtual void Load();
	virtual void PreserveBack(const char* name);
	virtual void RestorePreserved();
	virtual void OnAnotherHand(int key, void* pParam);
	void SetMainActor(IActor* pActor);
	void SetProgressBar(bool bProgressBar);
	void SetLoadingBackGround(const std::string& name);
	FrForm* LoadTipText(int a, int b);
	bool OnTipDlgResult(int result, FrForm* pForm);
	bool OnOverlappedDlg(int result, FrForm* pForm);
	int OnPacketCommon(WReceivedPacket& packet);
	void SetTip(bool bTip) { m_bTip = bTip; }

#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	friend class CTaskManager;

private:
	int _RemoveWareHouseItem(unsigned long tid, IActor* pActor, int count);
	int _RemoveWareHouseItem(unsigned long tid, const char* actorName,
		int count);
	void _ReFreshMyRoomGift(bool bRefresh, std::string name);

#endif
protected:
	static IActor* ms_pDummyActor;
	static CTaskWidget* ms_pNullWidget983; // 983 00f006f8: boneco CNullWidget (include/jp/nullwidget.h)
	bool m_bSyncLoad;
	WList<sActor*> m_actorList;
	IActor* m_pMainActor;
	CTaskDoc* m_pDoc;
	bool m_bLobbyScreenSize;
	std::string m_loadingBackGround;
	bool m_bProgressBar;
	bool m_bTip;
	Fresh* m_pLoadingFresh;
	FrForm* m_pTipForm;
	FrGaugeBar* m_pProgressGauge;
	CPostEffect983* m_pPostEffect983; // 983: +0x68 (zerado em 009bf433; o destrutor chama 009b74d0 e operator delete: objeto sem vtable)
	CAnotherHand m_anotherHand; // 983: 0x6c
	unsigned long m_taskFlag; // 983: 0x1f4
	bool m_bRecieveWhisper; // 983: 0x1f8 (= 1 em 009bf420)
	CTaskWidget* m_pWidget983; // 983: 0x1fc (009bf52a); widget aberta ou o boneco ms_pNullWidget983
	bool m_bPause; // 983: 0x200 (SetPause fora de linha em 0066f810)
	float m_netSyncTime; // 983: 0x204
#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbLoadtiptextFunclets(); // HACK: EH funclets of the naked LoadTipText (nakedify)
#endif
#endif
};

// D868: bytes de m_pFade..m_naviMode (0x40..0x8c no VC10) para as unidades sem taskmanager_pch.h
#define REBANG_TASKMANAGER_UNUSED40 (sizeof(void*) + sizeof(float) + 2 * sizeof(std::queue<int>) + sizeof(std::list<int>) + \
	sizeof(std::list<int>::iterator) + sizeof(int))

class CTaskManager : public WSingleton<CTaskManager>
{
public:
#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	struct sPostMsg
	{
		std::string target;
		MsgObject msg;
	};

	struct history_t
	{
		const char* task;
		const char* doc;
		const char* actor;
		const char* layout;
	};
	typedef std::list<history_t> HistoryList;
	typedef HistoryList::iterator HistoryIter;

#endif
	CTaskManager();
	virtual ~CTaskManager();

	int PostMsg(const IActor* sender, const char* target, int message,
		int param1, int param2, int param3, unsigned long time);
	// 983 (009c16a0, s100): mensagem sem alvo na fila nova (+0x48), entregue ao ator principal da tarefa corrente no
	// ProcessTask, antes da fila do PostMsg (ex.: MakeRealMyRoom: (0, 0x88, "CMyRoomWidget", "AVATARCHAT_MAIN", 0)). Nome meu.
	void PostMsgToMainActor983(const IActor* sender, int message, int param1,
		int param2, int param3);

	void CreateTask(const char* task, const char* doc, bool preserve);
	// 983 (009bf6e0; pedido C46.1): abre a widget `name` (fábrica + DYNAMIC_CAST CTaskWidget) por cima da tarefa
	// corrente; `loadingMsg` vai ao ator principal enquanto ela carrega. Nome meu.
	void OpenWidget983(const char* name, const char* loadingMsg);
	void PreProcess();
	void ProcessTask(float delta);
	void DisplayTask() const;
#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	void ClearTask();
#endif
	bool IsTaskChanging() { return m_bChangeTask; }
#ifdef REBANG_TASKMAIN_PCH // other units keep the PCH name count
	bool IsNaviBackwardEnable();
	bool IsNaviForwardEnable();
#endif
	void ChangeTask(const char* taskName, const char* docName, bool bPreserve);
	CTask* GetCurrentTask() const;
	CTask* GetPreservedTask() const { return m_pPreservedTask; }

#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	void SetWork(CWork* pWork);
	void ReleaseWork();

	void NaviOpenLayout(const char* task, const char* doc, const char* actor,
		const char* layout);
	void NaviGoForward();
	void NaviGoBackward();
#ifndef REBANG_TASKMAIN_PCH
	bool IsNaviForwardEnable();
	bool IsNaviBackwardEnable();
#endif
	void NaviDisplayDebugInfo();

	void AddNewItemToDoc(unsigned int type, WReceivedPacket& packet);

protected:
	void NaviAddHistory(const char* task, const char* doc, const char* actor,
		const char* layout);
	void NaviClearHistory();
	bool IsNaviForwardSameHistory(const char* layout);
#else
	void SetWork(CWork* pWork) { m_pWork.reset(pWork); }
	void ReleaseWork() { delete m_pWork.release(); }
#endif

protected:
	bool m_bChangeTask;
	bool m_bPreserveTask;
	const char* m_nextTaskName;
	const char* m_nextDocName;
	CTask* m_pCurrentTask;
	CTask* m_pPreservedTask;
	// 983 (construtor 009c13e0, destrutor 009c1110, ProcessTask 009c0bc0; s100): 4 bytes novos em +0x3c (zerados no fim do
	// construtor; gravados só pelo 009b79e0, sem chamador direto) e a fila nova em +0x48; sizeof 0x74 -> 0x90.
	void* m_pad983_3c;
#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	CBackGround* m_pFade; // 983: 0x40
	float m_fadeAlpha; // 0x44
	std::queue<sPostMsg> m_msgQueue983; // 0x48: PostMsgToMainActor983
	std::queue<sPostMsg> m_msgQueue; // 0x60
	HistoryList m_history; // 0x78
	HistoryIter m_historyPos; // 0x84
	int m_naviMode; // 0x88
#elif defined(_MSC_VER) && _MSC_VER > 1600
	// D868: cl novo = mesmos membros com o STL dele (deque/list encolhem): com o 0x4c do VC10 o gameunit lia m_pWork em
	// +0x8c, fora do objeto (sizeof 0x7c), e o ReleaseWork do OnConnect chamava lixo (robo vs2022-6). C_ASSERT em taskmanager.cpp.
	unsigned char m_unused40[REBANG_TASKMANAGER_UNUSED40];
#else
	unsigned char m_unused40[0x4c];
#endif
	std::auto_ptr<CWork> m_pWork;
#ifndef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
							   // fora do pch: membros 0x40..0x8c ficam em m_unused40 (visao; so o pch usa os nomes)
#endif
};

inline int AfxPostMsg(const IActor* sender, const char* target, int message,
	int param1, int param2, int param3, unsigned long time)
{
	return CTaskManager::Instance()->PostMsg(sender, target, message, param1,
		param2, param3, time);
}

inline CTask* AfxGetTask()
{
	if (CTaskManager::Instance() == NULL)
		return NULL;
	return CTaskManager::Instance()->GetCurrentTask();
}

inline CTask* AfxGetPreservedTask()
{
	if (CTaskManager::Instance() == NULL)
		return NULL;
	return CTaskManager::Instance()->GetPreservedTask();
}

#ifdef REBANG_CLIENTSETTING_PCH
// HACK: the original PCH instantiated this member along with Instance.
template bool WSingleton<CTaskManager>::IsInstantiated();
#endif
