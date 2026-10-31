#include "wproc.h"
#include "vc10_layout.h"
#include <mbstring.h>
#include "miles.h"
#include "clientsetting.h"
#include "fpsmonitor.h"
#include "dxdiaginfo.h"
#include "networkmonitor.h"
#include "projectg.h"
#include "gamedatadb.h"
#include "transferableitem.h"
#include "voiceitem.h"
#include "contentsdoc.h"
#include "matchhistory.h"
#include "thread.h"
#include "noticeboard.h"
#include "effectmanager.h"
#include "buddymanager.h"
#include "netresourcemanager.h"
#include "rankingdlg.h"
#include "onelineboard.h"
#include "messengerdlg.h"
#include "gmtoolkit.h"
#include "replaydlg.h"
#include "matchingsystem.h"
#include "pangfbi.h"
#include "eventthread.h"
#include "messagemanager.h"
#include "winnernotice.h"
#include "topiconmanager.h"
#include "ghostmanager.h"
#include "ghosthandler.h"
#include "shufflebonus.h"
#include "intrusion.h"
#include "powergauge.h"
#include "lobbybg.h"
#include "mathconsts.h"
#include "../shared/LuaSystem/luascriptmanager.h"
#include "md5.h"
#include "fx.h"
#include "mousecursor.h"
#include "user_info.h"
#include "logininfo.h"
#include "browser.h"
#include "cardsystem.h"
#include "headicon.h"
#include "treasure.h"
#include "integritycheck.h"
#include "chatmsg.h"
#include "golftask.h"
#include "lobbytask.h"
#include "registry.h"
#include "jp/gproomtimersteward.h"
#include "wtimer.h"
#include "jp/grandprixartifactrule.h"
#include "jp/createcharacteroption.h" // B124.1
#include "jp/motionadvisor.h"
#include "jp/bgmplaylist.h"
#include "jp/uiframesetmanager.h"
#include "jp/webpageconnector.h"
#include "jp/gamestringmanager.h"
#include "jp/refresh/rfscenemanager.h"
#include "jp/lobbyjp.h"
#include "jp/clubsetworkshopmaindlg.h" // 983: s_pInstance 00e74cd8 (Rf983ChangeTask)
#include "jp/refresh/rftexturemanager.h"
#include "jp/refresh/rftextmanager.h"
#include "initdelegate.h"
#include "jp/extraresource.h" // CExtraResource 004e0e30 (A171)
#include "jp/gamestringmanager.h" // GameStringManager 00e75184 (A172: textos do GetGameString)

#include <float.h>
extern "C" __declspec(dllimport) int __cdecl mkdir(const char*);
#include <io.h>
#include <stdarg.h>

extern HWND g_hwnd;
extern bool g_bQuit;
extern WResourceManager* g_resrcmng;
extern WMilesSoundSystem* g_miles;
extern WInputDev* g_keyboard;
extern WInputDev* g_mouse;
extern WFont* g_font;

void InitSingletonClasses();
void DeleteSingletonClasses();
void ExecuteStartMode();
void InitPakedFile();
bool InitPakedFile2();
void __cdecl ReleasePAK();
bool __stdcall GetCRCCheckList(
	std::list<std::pair<std::string, unsigned long> >& list);
void RegisterTitles(TitleManager* manager);

static char captureMessage[256] = { 0 };
char g_colTex[16][32] = { 0 };
int g_last_screen_height = 0;
int g_last_screen_width = 0;
TitleManager* g_TitleManager = 0;
WFont* g_font = 0;
Fresh* g_pFresh = 0;
unsigned long g_CurrentTime = 0;
char g_executeDirectory[MAX_PATH] = { 0 };
WInputDev* g_keyboard = 0;
WInputDev* g_mouse = 0;
WInputDev* g_ime = 0;
WView* g_view = 0;
HWND g_hwnd = 0;
WResourceManager* g_resrcmng = 0;
CSoundManager* g_audio = 0;
WMilesSoundSystem* g_miles = 0;
static int processCount = 0;
static int drawCount = 0;
int g_nDrawPoly = 0;
int g_nModel = 0;
int g_nDrawModel = 0;
int g_colNum = 0;
unsigned long* g_pZuffHistogram = 0;
bool g_bAviCapture = 0;
bool g_bQuit = 0;
static unsigned long captureMessageTime = 0;
int g_iSendPacketType = 0;
int g_iRecvPacketType = 0;
int g_iRecvDisconCode = 0;
int g_iSendFailType = 0;
WMatrix g_camera;
CFpsMonitor g_FpsMonitor;
CFpsMonitor g_VramMonitor;
bool g_bFocus = true;
bool g_bOldFocus = true;
struct UIFileChecksum
{
	const char* filename;
	unsigned char digest[16];
};
static UIFileChecksum uiChecks[] = {
	{ "[left_ht.jpg",
     { 0x5b, 0xa3, 0x60, 0x07, 0x2f, 0x73, 0x87, 0x21, 0xef, 0xbd, 0xcd,
			0xd4, 0x45, 0x23, 0xfc, 0xfa } },
	{ "[middle.jpg",
     { 0x7b, 0x95, 0xfc, 0xe6, 0x44, 0xab, 0xd1, 0x81, 0xa8, 0x53, 0xd1,
			0x56, 0xf9, 0xf4, 0xa2, 0x8f } },
	{ "[right_wind_b.jpg",
     { 0xed, 0xdd, 0x43, 0xfd, 0x5e, 0x65, 0x91, 0x86, 0x15, 0x1c, 0x80,
			0x79, 0x32, 0x41, 0x77, 0x1b } }
};
static int captureCountdown = -1;
LightSet g_lightset;

extern char PY_PUBLIC_VERSION[];

void SaveJPG(const char* filename, const Bitmap* bitmap, int quality);
void SaveBMP(const char* filename, const Bitmap* bitmap);

void __cdecl TextOut(WView* view, float x, float y, const char* format, ...)
{
	char text[2048];
	va_list args;
	va_start(args, format);
	if (_vsnprintf(text, sizeof(text) - 1, format, args) == -1)
		text[sizeof(text) - 1] = 0;
	va_end(args);
	g_font->Print(view, x, y, text, 0, 0xffffffff, NULL);
}

CTmpLogo::CTmpLogo()
	: m_pLogo(NULL)
{
}

CTmpLogo::~CTmpLogo()
{
	delete m_pLogo;
}

static CTmpLogo captureLogo;

// HACK: uccmanager in the PCH and gmtoolkit declarations change scheduling ties in SaveScreenShot.
// These declarations emit no code and restore the original load order after
// SetPixel without changing other PCH users (gmtoolkit: tested 16..22 names).
void RbProjectGSchedule0();
void RbProjectGSchedule1();
void RbProjectGSchedule2();
void RbProjectGSchedule3();
void RbProjectGSchedule4();
void RbProjectGSchedule5();
void RbProjectGSchedule6();
void RbProjectGSchedule7();
void RbProjectGSchedule8();
void RbProjectGSchedule9();
void RbProjectGSchedule10();
void RbProjectGSchedule11();
void RbProjectGSchedule12();
void RbProjectGSchedule13();
void RbProjectGSchedule14();
void RbProjectGSchedule15();
void RbProjectGSchedule16();
void RbProjectGSchedule17();
void RbProjectGSchedule18();
void RbProjectGSchedule19();

static bool SaveScreenShot(const char* filename)
{
	Bitmap* image = new Bitmap(g_last_screen_width, g_last_screen_height, 24);
	if (g_resrcmng->VideoReference()->Command(W_VDEV_CAPTURE_SCREEN, 0,
			(int)image) == 1)
	{
		if (COption::Instance()->GetOption()->gCaptureLogo == 1 &&
			Doc()->m_playingMode != 2)
		{
			if (!captureLogo.m_pLogo)
				captureLogo.m_pLogo = g_resrcmng->LoadTGA("pangya_halloween.tga"); // 983 00a6455d (0xd42ddc)
			if (captureLogo.m_pLogo)
			{
				Bitmap* logo = captureLogo.m_pLogo;
				int height = logo->bi->bmiHeader.biHeight;
				int width = logo->bi->bmiHeader.biWidth;
				int x = image->bi->bmiHeader.biWidth - width;
				int y = image->bi->bmiHeader.biHeight - height;
				if (x > 0 && y > 0)
				{
					for (int row = 0; row < height; ++row)
					{
						for (int col = 0; col < width; ++col)
						{
							unsigned char r2, g2, b2, a;
							unsigned char r, g, b;
							image->GetPixel(col + x, row + y, r, g, b);
							logo->GetPixel(col, row, r2, g2, b2, a);
							int alpha = a;
							int inverseAlpha = 255 - a;
							image->SetPixel(col + x, row + y,
								(inverseAlpha * r + alpha * r2) / 255,
								(inverseAlpha * g + alpha * g2) / 255,
								(inverseAlpha * b + alpha * b2) / 255, 255);
						}
					}
				}
			}
		}
		if (strrchr(filename, '.'))
		{
			if (!strcmpi(strrchr(filename, '.'), ".jpg"))
			{
				SaveJPG(filename, image, 100);
				delete image;
				return true;
			}
			if (!strcmpi(strrchr(filename, '.'), ".bmp"))
			{
				SaveBMP(filename, image);
				delete image;
				return true;
			}
		}
	}
	delete image;
	return false;
}

// 983 (A170): globais que as unidades da B (RF e outras) citam por nome proprio e que sao os nossos (mesma VA no 983). Sem isto
// o link as fazia stubs NULOS (build/f4e/a169-2/out/stubs.tsv) e a RF ligada iria ler janela/vista nulas.
// clang-format off
#pragma comment(linker, "/alternatename:?g_hWnd983@@3PAUHWND__@@A=?g_hwnd@@3PAUHWND__@@A") // 00f0245c
#pragma comment(linker, "/alternatename:?g_pMainView983@@3PAVWView@@A=?g_view@@3PAVWView@@A") // 00f02458
#pragma comment(linker, "/alternatename:?g_pMouseCursor983@@3PAVCMouseCursorJp983@@A=?m_pInstance@?$WSingleton@VCMouseCursor@@@@1PAVCMouseCursor@@A") // 00e73e7c
#pragma comment(linker, "/alternatename:?g_pOneLineBoard983@@3PAVCOneLineBoard@@A=?m_pInstance@?$WSingleton@VCOneLineBoard@@@@1PAVCOneLineBoard@@A") // 00e78cc8
#pragma comment(linker, "/alternatename:?g_pScreenCfgOwner983@@3PAVCScreenCfgOwner983@@A=?m_pInstance@?$WSingleton@VCOption@@@@1PAVCOption@@A") // 00e73f00
#pragma comment(linker, "/alternatename:?ms_pInstance@CSceneManagerJp@@2PAV1@A=?m_pInstance@?$WSingleton@VCSceneManager@@@@1PAVCSceneManager@@A") // 00e74230
// 983 009b2900 = CSceneManager::RegisterElement(type, puppet, func, name, flag, aabb) (o RegisterElement(SceneElemRegister&) 009b2b00 chama-o)
#pragma comment(linker, "/alternatename:?AddElement983@CSceneManagerJp@@QAEXHHHPBDHH@Z=?RegisterElement@CSceneManager@@QAEXW4eObjType@@PAVWPuppet@@PAVCRenderFuncPtr@@PBDKPAVWaabb@@@Z")
#pragma comment(linker, "/alternatename:?g_pCameraWork983@@3PAVCCameraWorkJp@@A=?m_pInstance@?$WSingleton@VCCameraWork@@@@1PAVCCameraWork@@A") // 00e74d30
#pragma comment(linker, "/alternatename:?g_pCameraWork983_e74d30@@3PAVCCameraWorkAwardJp@@A=?m_pInstance@?$WSingleton@VCCameraWork@@@@1PAVCCameraWork@@A") // 00e74d30
#pragma comment(linker, "/alternatename:?GetCursor983@CMouseCursorJp983@@QAEHXZ=?GetCursor@CMouseCursor@@QAEHXZ") // 00891000
#pragma comment(linker, "/alternatename:?SetCursor983@CMouseCursorJp983@@QAEXHH@Z=?SetCursor983@CMouseCursor@@QAEXHH@Z") // 00891020
#pragma comment(linker, "/alternatename:?InArea@CMouseCursorJp983@@QAE_NABVWRect@@@Z=?InArea@CMouseCursor@@QAE_NABVWRect@@@Z") // 00891480
// clang-format on
C_ASSERT_VC10(offsetof(CMouseCursor, m_bArea) == 0xd9 && offsetof(CMouseCursor, m_bLocked983) == 0x138 &&
	offsetof(CMouseCursor, m_cursorOwner983) == 0x13c && sizeof(CMouseCursor) == 0x140);

// 983 004927b0 (`lea eax, [ecx+0xcc]`): a configuracao de tela do COption (vWindowMode..), vista pela B (formbase.cpp:
// FrFormBase::UpdateScreenSize983) como CScreenCfgOwner983::Get983; sem corpo era stub fatal
struct sScreenCfg983;
class CScreenCfgOwner983
{
public:
	const sScreenCfg983& Get983();
};
const sScreenCfg983& CScreenCfgOwner983::Get983()
{
	// A760: era (char*)this + 0xcc cru (VC10); no cl 14.44 o m_opt anda -4 (std::string do BaseObject) -> Opt983()
	return *(const sScreenCfg983*)&((COption*)this)->Opt983();
}
// (COption::m_opt = +0xcc, m_opt.vScreenWidth = +0xd8: C_ASSERT no clientsetting.cpp, COptionLayout983)

// 983 (B159.2): ganchos da RF chamados pelo taskmanager.cpp (CTask::Display / CTaskManager::DisplayTask / ProcessTask)
void Rf983PreDraw()
{
	if (RFSceneManager::Instance())
		RFSceneManager::Instance()->PreDraw983(); // 009bb6a9
}

void Rf983Draw()
{
	if (RFSceneManager::Instance())
		RFSceneManager::Instance()->Draw983(); // 009b974a
	g_pFresh->GetManager()->DisplayFlag4000_983(); // 009b9757 (00be2840)
}

void Rf983Update(float delta)
{
	if (RFSceneManager::Instance())
		RFSceneManager::Instance()->Update983(delta); // 009c0de6
}

// 983 009b96ac..009b96cf (fim do CTaskManager::ChangeTask): ao trocar de task fecha a oficina de tacos e apaga a area de
// trabalho da RF (sem isso a barra do RFLobbyDesktop continuava desenhada por cima da partida, robo a175-11)
void Rf983ChangeTask()
{
	if (FrClubSetWorkshopMainDlg::s_pInstance)
		FrClubSetWorkshopMainDlg::s_pInstance->FrForm::Close(FrCANCEL, true); // 00be6080 direto, (2, 1)
	if (RFSceneManager::GetManager983())
		RFSceneManager::GetManager983()->DeleteDesktop(); // 004183b0
}

void InitSingletonClasses()
{
	// 983 (00a65735/00a65757; pedido B159.2): a RF antes do CPowerGauge -- cache de texturas (0x38) e gerente de cena (0x48)
	new RFTextureManager;
	new RFSceneManager;
	new CPowerGauge;
	new CMatchHistory;
	new CGhostManager;
	new CGhostHandler;
	new CSoundItemManager;
	new CShuffleBonus;
	new CIntrusion;
	new CMotionAdvisor; // 983 00a65895 (B110.1; o 983 não o apaga no DeleteSingletonClasses)
	// 983 (00a65710; pedido B75, s100): entre o `new CTimerManager` (0x214) e o `new CTransferableItem` (0x3d0) o 983 cria o
	// relógio do Grand Prix (0x80) e as regras dos artefatos (0x28). Os outros construtores novos da lista (RFTextureManager,
	// RFSceneManager, 0085ca20) esperam as classes.
	new CTimerManager; // A910: 983 00a658ba; sem ele AddTimer do GP (sala, premio) caia com this NULL
	new CGrandPrixClock;
	new _grandprix::_private::CGrandPrixArtifactRule;
	new CTransferableItem; // 983 00a65926 (new 0x3d0): sem ele o Mail do lobby (0x0211) derruba o exe (C114.2)
	new CreateCharacterOption; // 983 00a6592b (B124.1; new 0xb60, o construtor 004d0e00 grava o m_pInstance)
}

void DeleteSingletonClasses()
{
	// 983 (00a639e2..00a63a18): texto, cena e texturas da RF primeiro
	if (RFTextManager::IsInstantiated())
		delete RFTextManager::Instance();
	if (RFSceneManager::IsInstantiated())
		delete RFSceneManager::Instance();
	if (RFTextureManager::IsInstantiated())
		delete RFTextureManager::Instance();
	// A910: 983 00a63a18..00a63a4c
	if (_grandprix::_private::CGrandPrixArtifactRule::IsInstantiated())
		delete _grandprix::_private::CGrandPrixArtifactRule::Instance();
	if (CGrandPrixClock::IsInstantiated())
		delete CGrandPrixClock::Instance();
	if (CTimerManager::IsInstantiated())
		delete CTimerManager::Instance();
	if (CTransferableItem::IsInstantiated())
		delete CTransferableItem::Instance();
	if (CIntrusion::IsInstantiated())
		delete CIntrusion::Instance();
	if (CShuffleBonus::IsInstantiated())
		delete CShuffleBonus::Instance();
	if (CSoundItemManager::IsInstantiated())
		delete CSoundItemManager::Instance();
	if (CGhostHandler::IsInstantiated())
		delete CGhostHandler::Instance();
	if (CGhostManager::IsInstantiated())
		delete CGhostManager::Instance();
	if (CMatchHistory::IsInstantiated())
		delete CMatchHistory::Instance();
	if (CPowerGauge::IsInstantiated())
		delete CPowerGauge::Instance();
}

// A175: 983 00e47e20 = WSingleton<CProjectG>::ms_pInstance (ctor 00a62e40 grava o CProjectG, dtor 00a66d02 zera); a frente B
// le +0xc (m_gameSpeed) por este nome (include/jp/golfruleholeinone.h). Sem definicao o link o criava nulo (AV no robo a175-10
// em CTutorGuide::OnProcess). C683: era CUnk983_e47e20* (visao de CMainFrame).
CMainFrame* g_pMainFrame983 = NULL;

CProjectG::CProjectG(HWND hwnd)
	: m_inputList(16, 16)
{
	g_pMainFrame983 = static_cast<CMainFrame*>(this);
	g_CurrentTime = timeGetTime();
	srand(g_CurrentTime);
	m_threadId = GetCurrentThreadId();
	m_pDeviceManager = NULL;
	m_pProcManager = NULL;
	m_bMoveLoginServer = false;
	m_bLostFocus = false;
	// 983 00a66dab: os textos do jogo (GetGameString) vem do servidor; sem o gerente o GetGameString devolve NULL (AV no lobby)
	new GameStringManager;
	if (GameStringManager::Instance())
		GameStringManager::Instance()->Load("");
	m_pDeviceManager = new WDeviceManager(NULL, 9);
	m_pProcManager = new WProcManager;
	g_resrcmng = new WResourceManager;
	WFile::SetResourceManager(g_resrcmng);
	m_baseTime = g_CurrentTime;
	m_reserved = 4;
	new _systemmsg::CMessageManager;
	new CSharedDoc;
	new COption(hwnd, m_pDeviceManager, g_resrcmng);
	new HackingManager;
	ResetTimer();
	m_refreshRate = 60.0f;
	new CTaskManager;
	new BuddyManager;
	new CGMToolkit;
	if (IsLocalContent((localContentType_t)80 /*983: era 79; medido*/))
		new CReplayControlManager;
	if (IsLocalContent((localContentType_t)82 /*983: era 81; medido*/))
		new CCardManager;
	new CWebPageConnector; // JP 983 (00a67087, sizeof 0x230; pedido C33): sem ele o fluxo 0xfb/0x1ad da pagina web nao roda
	new CMouseCursorHelper; // 983 00a670a7 (0x38): sem ele o CMouseCursor::Display caia com this NULL (A182)
	new CPangFBI;
	m_bHidePrivacy = false;
	m_bHideGUI = false;
	m_bHidePI = false;
	m_pDxDiagInfo = NULL;
	m_pDxDiagInfo = new CDxDiagInfo;
	g_TitleManager = new TitleManager;
	RegisterTitles(g_TitleManager);
	new CEventThread;
	m_prevCamera.Reset();
	InitSingletonClasses();
	// 983 (00a67200): roda as funcoes de init registradas (Register*HandlerActor dos documentos do JP, que enchem a fabrica
	// nomeada); no 645 so rodavam pela mensagem 0x270 do CTaskMain, que o 983 nao manda (F4: "Need Gateway Handler" no lobby)
	CInitDelegate::ExecuteInitFunctions();
}

CProjectG::~CProjectG()
{
	g_pMainFrame983 = NULL;
	while (!m_recvPacketQueue.empty())
	{
		delete m_recvPacketQueue.front();
		m_recvPacketQueue.pop();
	}
	CInitDelegate::ExecuteReleaseFunctions(); // 983 00a667ef
	if (g_view)
	{
		g_resrcmng->Release(g_view);
		g_view = NULL;
	}
	if (g_font)
	{
		g_resrcmng->Release(g_font);
		g_font = NULL;
	}
	DeleteSingletonClasses();
	if (CPangFBI::IsInstantiated())
		delete CPangFBI::Instance();
	if (IsLocalContent((localContentType_t)89 /*983: era 88; interpolado*/))
		if (CTHunter::IsInstantiated())
			delete CTHunter::Instance();
	if (CMatchingSystem::IsInstantiated())
		delete CMatchingSystem::Instance();
	if (CGMToolkit::IsInstantiated())
		delete CGMToolkit::Instance();
	if (BuddyManager::IsInstantiated())
		delete BuddyManager::Instance();
	if (CHeadIcon::IsInstantiated())
		delete CHeadIcon::Instance();
	if (CTaskManager::IsInstantiated())
		delete CTaskManager::Instance();
	if (CChatMsg::IsInstantiated())
		delete CChatMsg::Instance();
	if (CNetworkMonitor::IsInstantiated())
		delete CNetworkMonitor::Instance();
	if (WNetworkSystem::IsInstantiated())
		delete WNetworkSystem::Instance();
	if (CMouseCursor::IsInstantiated())
		delete CMouseCursor::Instance();
	if (CUserInfo::IsInstantiated())
		delete CUserInfo::Instance();
	if (CMessengerInfo::IsInstantiated())
		delete CMessengerInfo::Instance();
	if (CSharedDoc::IsInstantiated())
		delete Doc();
	if (CContentsDoc::IsInstantiated())
		delete CContentsDoc::Instance();
	if (COption::IsInstantiated())
		delete COption::Instance();
	if (HackingManager::IsInstantiated())
		delete HackingManager::Instance();
	if (CNoticeBoard::IsInstantiated())
		delete CNoticeBoard::Instance();
	if (CWinnerNotice::IsInstantiated())
		delete CWinnerNotice::Instance();
	if (CFx::IsInstantiated())
		delete CFx::Instance();
	if (CAztecHoleinEffect::IsInstantiated())
		delete CAztecHoleinEffect::Instance();
	if (NetResourceManager::IsInstantiated())
		delete NetResourceManager::Instance();
	if (COneLineBoard::IsInstantiated())
		delete COneLineBoard::Instance();
	if (CRankingInfo::IsInstantiated())
		delete CRankingInfo::Instance();
	if (IsLocalContent((localContentType_t)80 /*983: era 79; medido*/))
		if (CReplayControlManager::IsInstantiated())
			delete CReplayControlManager::Instance();
	if (IsLocalContent((localContentType_t)82 /*983: era 81; medido*/))
		if (CCardManager::IsInstantiated())
			delete CCardManager::Instance();
	if (CGameDataDB::IsInstantiated())
		delete CGameDataDB::Instance();
	// 983: o CIconManager e criado e destruido pelo CExtraResource (004e0e30), nao aqui
	if (g_pFresh)
	{
		g_pFresh->GetManager()->DestroyToolTip();
		if (g_pFresh)
		{
			delete g_pFresh;
			g_pFresh = NULL;
		}
	}
	if (g_input)
	{
		delete g_input;
		g_input = NULL;
	}
	if (g_audio)
	{
		delete g_audio;
		g_audio = NULL;
	}
	if (g_miles)
	{
		delete g_miles;
		g_miles = NULL;
	}
	if (g_resrcmng)
	{
		delete g_resrcmng;
		g_resrcmng = NULL;
	}
	if (m_pDeviceManager)
	{
		delete m_pDeviceManager;
		m_pDeviceManager = NULL;
	}
	if (m_pProcManager)
	{
		delete m_pProcManager;
		m_pProcManager = NULL;
	}
	if (m_pDxDiagInfo)
	{
		delete m_pDxDiagInfo;
		m_pDxDiagInfo = NULL;
	}
	if (g_TitleManager)
	{
		delete g_TitleManager;
		g_TitleManager = NULL;
	}
	if (CEventThread::IsInstantiated())
		delete CEventThread::Instance();
	if (_systemmsg::CMessageManager::IsInstantiated())
		delete _systemmsg::CMessageManager::Instance();
	if (CExtraResource::IsInstantiated()) // 983 00a66bc0
		delete CExtraResource::Instance();
	if (GameStringManager::IsInstantiated()) // 983 00a66bd7
		delete GameStringManager::Instance();
	if (CVoiceItem::Instance())
	{
		CVoiceItem::Instance()->Destroy();
		if (CVoiceItem::IsInstantiated())
			delete CVoiceItem::Instance();
	}
	if (m_avi.IsOpened())
		m_avi.Close();
	ReleasePAK();
}

void CALLBACK MyTimerProc(HWND hwnd, UINT message, UINT timer, DWORD time)
{
}

void CProjectG::FPU_Check()
{
	m_fpuControl = _controlfp(0, 0);
	if (m_fpuControl != 0xa001f)
		m_fpuControl = _controlfp(_PC_24, _MCW_PC);
}

void CProjectG::SetMoveLoginServer(const char* id, const char* password)
{
	m_bMoveLoginServer = true;
	m_moveLoginId = id;
	m_moveLoginPassword = password;
}

bool CProjectG::Ready()
{
	FPU_Check();
	// 983 00a6943c: textos nao carregados = fim (m_bLoaded, +0x44 no VC10; pelo membro: no cl 14.x o STL muda o layout)
	if (GameStringManager::Instance() && !GameStringManager::Instance()->IsLoaded())
	{
		MessageBox(g_hwnd, "string load failed.", "Error", 0);
		return false;
	}
	// 983 00a6946a: em janela (gBindMouseCursor 0) esconde pelo contador do ShowCursor; preso ou tela cheia = SetCursor(NULL)
	if (COption::Instance()->GetOption()->gBindMouseCursor != 1 && COption::Instance()->vIsWindowed())
	{
		if (COption::Instance()->GetOption()->gBindMouseCursor == 0 && g_pMouseCursorHelper983)
			g_pMouseCursorHelper983->SetVisible(false);
	}
	else
		SetCursor(NULL);
	if (IsLocalContent(S3_ENCRYPTION_LV1))
	{
		if (!InitPakedFile2())
			return false;
	}
	else
		InitPakedFile();
	// 983 00a694f4 (A171): o CExtraResource (B, extraresource.cpp) cria o CIconManager (icones do topo do lobby; sem ele o
	// OnToppage_TopIcon1Init caia com this NULL) e carrega os temas/avisos/papeis de parede
	new CExtraResource;
	CExtraResource::Instance()->Load();
	CExtraResource::Instance()->GetLobby(2);
	Doc()->LoadItemDb();
	Doc()->InitGolfDoc();
	Doc()->LoadChatFilter();
	Doc()->LoadVisGroup("visgroup.def");
	new CContentsDoc;
	if (!CContentsDoc::Instance()->Initialize())
		return false;
	char commandLine[MAX_PATH];
	strcpy(commandLine, GetCommandLine());
	strlwr(commandLine);
	if (strstr(commandLine, "-integrity"))
	{
		CIntegrityCheck check;
		if (!check.Report("ResrcIntegrity.txt"))
			g_bQuit = true;
	}
	GetCurrentDirectory(MAX_PATH, g_executeDirectory);
	g_view = g_resrcmng->GetView(0);
	g_input = new CInputManager;
	COption::Instance()->CheckDebugRegValid();
	COption::Instance()->SetVideoDevice(g_resrcmng->VideoReference());
	COption::Instance()->SetGlobalView(g_view);
	COption::Instance()->SetAudioManager(g_audio);
	COption::Instance()->SetInputManager(g_input);
	UploadShaderSource();
	g_font = g_resrcmng->GetTexFont("data/Misc/font.bmp");
	LoadKeyLayout();
	new CChatMsg;
	new WNetworkSystem;
	new CNetworkMonitor;
	new CMouseCursor;
	new CHeadIcon;
	new CUserInfo;
	new CMessengerInfo;
	new CNoticeBoard;
	new CWinnerNotice;
	new CFx;
	new CAztecHoleinEffect;
	new NetResourceManager;
	new COneLineBoard;
	new CRankingInfo;
	new CUiFrameSetManager; // 983 00a69916 (new 0xa8, 006c6a10: lê uiframeset.xml); A174: faltava -> AV no SetA4983 ao entrar na partida
	if (IsLocalContent(S4_TREASURE_HUNTER))
	{
		new CTHunter;
		CTHunter::Instance()->Initialize(0x16); // 983 00a6996f: 22 mapas (Abbot Mine 20, Mystic Ruins 21); C111.1
	}
	if (IsLocalContent(S4_MATCHING_SYSTEM))
		new CMatchingSystem;
	new CGameDataDB;
	// 983: sem `new CIconManager` (quem cria e o CExtraResource, 004e0e30, chamado no comeco do Ready)
	new CVoiceItem;
	CVoiceItem::Instance()->Initialize();
	// 983 00a69a04: Lua (console, callbacks de arquivo, root.lua) antes do Fresh; sem ele o CBGMPlayList983 caia (C109.1)
	lua_system::InitLuaSystem983();
	g_pFresh = new Fresh;
	g_pFresh->Init("all.xml", true, false);
	g_pFresh->GetManager()->CreateToolTip();
	// 983 (00a69a54..00a69a80; B159.2): liga a RF (RFManager, interface, desenhista) e cria o texto da RF (0x30)
	if (RFSceneManager::Instance())
		RFSceneManager::Instance()->Init983();
	new RFTextManager;
	COption::Instance()->dApplyDebugSetting();
	CNetworkMonitor::Instance()->Init();
	g_FpsMonitor.Init(200, 200, WRect(10, 300, 200, 100));
	g_VramMonitor.Init(200, 600, WRect(10, 300, 200, 100));
	g_VramMonitor.SetColor(0, 0xff0000ff);
	int course = COption::Instance()->dGetLobbyCourse();
	if (course >= 0)
	{
		LOBBYBG_MAP = course;
		int map = course;
		if (course > 127 && course != 253)
			map = course - 128;
		CItemManager* items = &Doc()->m_itemManager;
		LOBBYBG_MAPNAME = items->FindCourse(map | 0x28000000)->Data;
	}
	ExecuteStartMode();
	SYSTEM_INFO systemInfo;
	memset(&systemInfo, 0, sizeof(systemInfo));
	GetSystemInfo(&systemInfo);
	if (systemInfo.dwNumberOfProcessors > 1)
	{
		std::list<std::pair<std::string, unsigned long> > checks;
		if (!GetCRCCheckList(checks))
			return false;
		for (std::list<std::pair<std::string, unsigned long> >::iterator it =
				 checks.begin();
			it != checks.end(); ++it)
			m_crc.AddWork((*it).first.c_str(), (*it).second);
		m_crc.SetSleepTime(1);
		m_crc.Begin(g_hwnd);
	}
	return true;
}

void ExecuteStartMode()
{
	CTaskManager::Instance()->CreateTask("CLobbyTask", "CGolfDoc", false);
	CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)"LOGIN", 0, 0, 0);
}

void CProjectG::LoadKeyLayout()
{
	struct KeyBinding
	{
		const char* name;
		int key;
		unsigned long flags;
		int type;
	};
	KeyBinding bindings[] = {
		{ "\273\363", 6 },
		{ "\307\317", 7 },
		{ "\301\302", 4 },
		{ "\277\354", 5 },
		{ "\273\363", 87 },
		{ "\307\317", 83 },
		{ "\301\302", 65 },
		{ "\277\354", 68 },
		{ "LSHIFT", 129, 1 },
		{ "RSHIFT", 130, 1 },
		{ "\275\272\306\344\300\314\275\272\271\331", 32 },
		{ "PAGE_DOWN", 3, 1 },
		{ "PAD_PLUS", 140 },
		{ "PAGE_UP", 2, 1 },
		{ "PAD_MINUS", 136 },
		{ "\265\360\271\366\261\327\305\260", 8 },
		{ "HOME", 12 },
		{ "END", 26 },
		{ "PAD_0", 144 },
		{ "NUM_0", 48 },
		{ "PAD_1", 141 },
		{ "NUM_1", 49 },
		{ "PAD_2", 142 },
		{ "NUM_2", 50 },
		{ "PAD_3", 143 },
		{ "NUM_3", 51 },
		{ "PAD_4", 137 },
		{ "NUM_4", 52 },
		{ "PAD_5", 138 },
		{ "NUM_5", 53 },
		{ "PAD_6", 139 },
		{ "NUM_6", 54 },
		{ "PAD_7", 133 },
		{ "NUM_7", 55 },
		{ "PAD_8", 134 },
		{ "NUM_8", 56 },
		{ "PAD_9", 135 },
		{ "NUM_9", 57 },
		{ "ITEMSLOT", 73 },
		{ "\301\244\301\366", 80 },
		{ "LALT", 28, 1 },
		{ "RALT", 29, 1 },
		{ "LCONTROL", 30 },
		{ "RCONTROL", 31 },
		{ "TAB", 9, 1 },
		{ "FASTFORWARD", 32 },
		{ "\305\327\275\272\306\256", 84 },
		{ "PARAM_UPDATE", 85 },
		{ "CONSOLE", 10, 1 },
		{ "F15", 102, 1 },
		{ "FUNC_1", 14, 1 },
		{ "FUNC_2", 15, 1 },
		{ "FUNC_3", 16, 1 },
		{ "FUNC_4", 17, 1 },
		{ "FUNC_5", 18, 1 },
		{ "FUNC_6", 19, 1 },
		{ "FUNC_7", 20, 1 },
		{ "FUNC_8", 21, 1 },
		{ "FUNC_9", 22, 1 },
		{ "FUNC_10", 23, 1 },
		{ "FUNC_11", 24, 1 },
		{ "FUNC_12", 25, 1 },
		{ "REVIEW_PREV", 91, 1 },
		{ "REVIEW_NEXT", 93, 1 },
		{ "FUNC_15", 102, 1 },
		{ "INSERT", 127, 1 },
		{ "ESCAPE", 27, 1 },
		{ "ENTER", 13, 1 },
		{ "PADENTER", 145, 1 },
		{ "HIDE", 72 },
		{ "JUKEBOX", 77 },
		{ "REPLAY", 82 },
		{ "SKIP", 83 },
		{ "CANCEL", 67 },
		{ "FPS_SHOT", 70 },
		{ "BALL_TRACE", 71 },
		{ "PARTICLE_1", 46 },
		{ "PARTICLE_2", 44 },
		{ "\270\305\305\251\267\316\303\242", 96 },
		{ "\301\244\301\3662", 86 },
		{ "EMOTICON", 69 },
		{ "BANISH", 1 },
		{ "RANK", 81 },
		{ "ROTATE_CW", 81 },
		{ "MESSENGER", 127, 1 },
		{ "ROTATE_CCW", 69 },
		{ "DEBUG_INFO", 146, 1 },
		{ "PLAYER_INFO", 147 },
		{ "CALIPERS_RIGHT", 88 },
		{ "CALIPERS_LEFT", 90 },
		{ "\260\355\274\323", 78 },
		{ "\300\372\274\323", 66 },
		{ "UNDO", 90 },
		{ "REDO", 89 },
		{ "PAINT", 71 },
		{ "GRID", 76 },
		{ "DRAG", 72 },
		{ "BRUSH", 66 },
		{ "ZOOMIN", 93 },
		{ "ZOOMOUT", 91 },
		{ "PEN", 80 },
		{ "ERASER", 69 },
		{ "PICKCOLOR", 73 },
		{ "AUTO_TOMAHAWK", 84 },
		{ "AUTO_COBRA", 67 },
		{ "AUTO_SPIKE", 83 },
		{ "SEARCH", 16, 1 },
		{ NULL },
	};
	for (int i = 0; bindings[i].name; ++i)
		g_input->Register(bindings[i].name, bindings[i].key, bindings[i].flags,
			bindings[i].type == 1 ? 3 : 2);
	g_input->ExcludeKey("BALL_TRACE");
	g_input->ExcludeKey("PARAM_UPDATE");
	g_input->ExcludeKey("FPS_SHOT");
	g_input->ExcludeKey("\305\327\275\272\306\256");
	g_input->ExcludeKey("\265\360\271\366\261\327\305\260");
	g_input->ExcludeKey("PARTICLE_1");
	g_input->ExcludeKey("PARTICLE_2");
	g_input->ExcludeKey("\301\244\301\366");
	g_input->ExcludeKey("\301\244\301\3662");
	g_input->ExcludeKey("\260\355\274\323");
	g_input->ExcludeKey("\300\372\274\323");
}

// HACK: the original is inline asm (TEB -> PEB -> BeingDebugged); the ebp frame with the dead
// "= 1" store and the reload are the __asm fingerprint. __readfsdword(0x18) with a volatile local
// matches except for the registers (VC7.1 /O2 rotates eax, ecx, edx; the asm reuses eax).
int MainLoop()
{
	int beingDebugged = 1;
	__asm
	{
		mov eax, fs:[18h]
		mov eax, [eax + 30h]
		movzx eax, byte ptr [eax + 2]
		mov beingDebugged, eax
	}
	return beingDebugged;
}

int CProjectG::MainLoop(int flags)
{
	FPU_Check();
	g_CurrentTime = timeGetTime();
	WNetworkSystem::Instance()->Process();
	ProcessRecvPacketQueue();
	WFlags captureFlags = m_flag;
	int mainFlags = m_mainFlags;
	if (!bool(captureFlags & 2))
		return Main(mainFlags | 0x80);
	return Main(mainFlags);
}

void CProjectG::CheckScreenShot()
{
	if (g_input->GetDown("FUNC_11", true) || Doc()->GetReplayState() == 6)
	{
		if (IsLocalContent(S4_REPLAY_SYSTEM) &&
			(Doc()->GetReplayState() == 6 || Doc()->GetReplayState() == 9))
		{
			__int64 freeSpace = CalcHddFreeSpace();
			if (!freeSpace)
			{
				CChatMsg::Instance()->AddChatMsg(
					K2L_Compatibility("\307\317\265\345\300\307 \277\353\267\256\300\273 \303\274\305\251 \307\322 \274\366 \276\370\300\270\271\307\267\316 \265\277\277\265\273\363 \306\304\300\317\300\273 \273\375\274\272 \307\322 \274\366 \276\370\275\300\264\317\264\331."),
					0xffff7878, true, false);
				return;
			}
			if (freeSpace < 0x40000000)
			{
				CChatMsg::Instance()->AddChatMsg(
					K2L_Compatibility("\307\317\265\345\300\307 \277\353\267\256\300\314 \272\316\301\267\307\317\277\251 avi\306\304\300\317\300\273 \273\375\274\272\307\322 \274\366 \276\370\275\300\264\317\264\331."),
					0xffff7878, true, false);
				return;
			}
			COption::Instance()->dSetCaptureFormat(2);
			if (Doc()->GetReplayState() == 6)
				Doc()->SetReplayState(7);
		}
		else
			COption::Instance()->dSetCaptureFormat(0);
		m_bHideGUI = COption::Instance()->gGetCaptureHideGUI() == 1;
		m_bHidePI = COption::Instance()->gGetCaptureHidePI() == 1;
		if (g_input->Get("LCONTROL", false) ||
			g_input->Get("RCONTROL", false) || m_bHidePI)
		{
			m_bHidePrivacy = true;
			g_pFresh->HidePrivacy(true);
		}
		captureCountdown = 3;
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1f9), 0, 0, 0, 0, 0));
	}
	if (captureCountdown > 0)
		--captureCountdown;
	if (captureCountdown == 0)
	{
		m_bHidePrivacy = false;
		captureCountdown = -1;
		g_pFresh->HidePrivacy(false);
		char suffix[4] = "";
		char description[64] = "";
		if (m_bHideGUI)
		{
			strcat(suffix, "G");
			strcat(description, "(GUI");
			if (!m_bHidePI)
				strcat(description, K2L_Compatibility("\301\246\260\305)"));
		}
		if (m_bHidePI)
		{
			strcat(suffix, "U");
			if (m_bHideGUI)
				strcat(description, ", ");
			else
				strcat(description, "(");
			strcat(description,
				K2L_Compatibility("\260\263\300\316\301\244\272\270\301\246\260\305)"));
		}
		const char* format = COption::Instance()->dGetCaptureFormat();
		mkdir("capture");
		char filename[64];
		do
		{
			sprintf(filename, "capture/pangya%s_%03d.%s", suffix,
				m_captureNum++, format);
		} while (_access(filename, 0) != -1);
		if (!stricmp(format, "avi"))
		{
			Doc()->SetReplayState(7);
			if (m_avi.IsOpened())
			{
				m_avi.Close();
				m_mainFlags.Disable(0x22);
				m_flag.Disable(2);
				SetFPS(m_refreshRate);
				if (IsLocalContent(S4_REPLAY_SYSTEM))
					Doc()->SetReplayState(0);
				SetGameSpeed(1.0f);
				COption::Instance()->dSetCaptureFormat(0);
			}
			else
			{
				int width = COption::Instance()->dGetAviWidth();
				int height = COption::Instance()->dGetAviHeight();
				int fps = COption::Instance()->dGetAviFps();
				if (m_avi.Open(filename, width, height, fps, g_hwnd))
				{
					g_input->SetFakeInput(
						"\275\272\306\344\300\314\275\272\271\331", false);
					SetFPS((float)fps);
					m_flag.Enable(2);
					m_mainFlags.Enable(0x22);
				}
			}
		}
		else
		{
			if (!COption::Instance()->gGetCaptureHidePI() &&
				Doc()->m_playingMode != 2)
			{
				// 983 00a64790(0, !m_bHideGUI): o carimbo de data nao sai (arg 0, chamador unico 00a68e31)
				CMouseCursor* cursor = CMouseCursor::Instance();
				CUiPointer983* scene = CUiPointer983::Instance983();
				if (m_bHideGUI)
				{
					bool show = g_pFresh->show;
					bool visible = cursor->IsVisible(); // 983 le +0x64 e devolve no +0x125
					bool show40 = scene ? scene->m_show40 : false;
					g_pFresh->show = false;
					cursor->Toggle(false);
					if (scene)
						scene->m_show40 = false;
					Draw();
					g_pFresh->show = show;
					cursor->Toggle(visible);
					if (scene)
						scene->m_show40 = show40;
				}
				else
				{
					if (scene)
						scene->m_show40 = true;
					Draw();
				}
				g_view->EndScene();
				g_resrcmng->VideoReference()->EndScene();
			}
			if (SaveScreenShot(filename))
				sprintf(captureMessage,
					K2L_Compatibility("\310\255\270\351%s\300\314 %s\277\241 \300\372\300\345\265\307\276\372\275\300\264\317\264\331."),
					description, filename);
			else
				sprintf(captureMessage,
					K2L_Compatibility("\275\272\305\251\270\260\274\246 \300\372\300\345\300\314 \275\307\306\320 \307\337\275\300\264\317\264\331."));
			captureMessageTime = timeGetTime();
			// 983 00a68e8b: so depois de salvar; o ramo avi deixa ligado durante a gravacao
			m_bHideGUI = false;
			m_bHidePI = false;
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1f9), 0, 0, 0, 0, 0));
		}
	}
}

void CheckQuickEscape()
{
}

void CheckDebugKey()
{
}

void CProjectG::OpenDisconDlg(const char* text, bool gameServer)
{
	if (g_pFresh)
	{
		FrForm* form =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		if (form)
		{
			form->SetMessage(text, false);
			// 983 (00a67d49/00a67d80): flag 0xa03 (o 645 tinha 515 = 0x203) e a caixa fixa (FWS_FIXED)
			if (gameServer == true)
				form->Open((FRESH_PFN_RESULT)&CProjectG::OnGameServerResult,
					0xa03);
			else
				form->Open((FRESH_PFN_RESULT)&CProjectG::OnLoginServerResult,
					0xa03);
			form->SetFixed(true);
		}
	}
	else
	{
		MessageBox(NULL, text, K2L_Compatibility("\276\313\270\262"), 0);
		if (gameServer == true)
			OnGameServerResult(1, NULL);
		else
			OnLoginServerResult(1, NULL);
	}
}

void CProjectG::FromGolfToLobbySettings()
{
	if (g_input)
	{
		g_input->FreeKey();
		g_input->FreeMouse();
	}
	if (g_audio)
		g_audio->StopAllSound();
	if (CFx::Instance())
		CFx::Instance()->ClearAll(true); // 983 00a63b6d
	if (CUserInfo::Instance())
		CUserInfo::Instance()->Close();
	if (CMouseCursor::Instance())
		CMouseCursor::Instance()->SetMode(CMouseCursor::SCREEN_LOBBY);
	g_resrcmng->VideoReference()->Command(W_VDEV_RELEASE_CAPTURERESOURCE, 0, 0);
}

void CProjectG::Process(float delta)
{
	g_input->Update();
	if (!WisEqual(GetGameSpeed(), 1.0f, g_EPSILON))
	{
		if (!IS_KINDOF(CGolfTask, AfxGetTask()))
			SetGameSpeed(1.0f);
	}
	CNetworkMonitor::Instance()->Process(delta);
	g_FpsMonitor.Trace(delta == 0.0f ? -1 : (int)(1.0f / delta));
	g_FpsMonitor.Process(0.0f);
	int available =
		g_resrcmng->VideoReference()->Command(W_VDEV_GETAVAIL_VRAM, 0, 0);
	static int initialVram = -1;
	if (initialVram == -1)
		initialVram = available;
	g_VramMonitor.Trace((initialVram - available) / 20000);
	g_VramMonitor.Process(0.0f);
	const float rawDelta983 = delta; // A910: 983 [ebp+8], sem GameSpeed
	delta *= GetGameSpeed();
	if (IsLocalContent(S4_REPLAY_SYSTEM) && bool(m_flag & 2))
	{
		static float captureTime;
		captureTime += delta;
		if (delta > 1.0f / 15.0f)
		{
			delta = 1.0f / 15.0f;
			SetGameSpeed(0.0f);
			captureTime = 0.0f;
			g_bAviCapture = true;
		}
		else if (captureTime > 1.0f / 15.0f)
		{
			delta -= captureTime - 1.0f / 15.0f;
			SetGameSpeed(0.0f);
			captureTime = 0.0f;
			g_bAviCapture = true;
		}
	}
	CTaskManager::Instance()->PreProcess();
	if (Doc()->m_replayState != 7 && g_input->GetDown("MESSENGER", true) &&
		NET()->GetSocket(WNetworkSystem::NET_GAME)->m_bConnected)
		CMessengerInfo::Instance()->Open(true);
	// 983 00a69138: avanca o TIME dos efeitos (.fx; mtlclrani da Neon Comet etc.)
	g_resrcmng->VideoReference()->AddElapsedTime(delta);
	CMouseCursor::Instance()->Process(delta);
	CMessengerInfo::Instance()->Process(delta);
	CTaskManager::Instance()->ProcessTask(delta);
	CUserInfo::Instance()->Process(delta);
	CRankingInfo::Instance()->Process(delta);
	CNoticeBoard::Instance()->Process(delta);
	CWinnerNotice::Instance()->Process(delta);
	CFx::Instance()->Process(delta);
	CBrowser::Instance()->Process(delta);
	COneLineBoard::Instance()->Process(delta);
	CBGMPlayList983::Instance()->Process983(delta); // 983 00a6920e: acabou a faixa, recomeca ou passa a proxima
	CTimerManager::Instance()->UpdateTimer(rawDelta983); // A910: 983 00a69220
	CGrandPrixClock::Instance()->Update983(rawDelta983); // A910: 983 00a69232
	lua_system::lua_script_manger::GetInstance()->Update(delta);
	g_audio->Process(delta, g_view);
	if (g_miles)
		g_miles->Process();
	COption::Instance()->CheckPendingChanges();
	CheckScreenShot();
	if (m_pDxDiagInfo && !m_pDxDiagInfo->IsGathered())
		m_pDxDiagInfo->GatherInfo();
	if (g_bQuit)
		Quit();
	++processCount;
	if (g_bOldFocus != g_bFocus)
	{
		g_bOldFocus = g_bFocus;
		if (!COption::Instance()->GetOption()->vWindowMode)
			g_mouse->InitDevice(g_hwnd, g_bFocus);
	}
	if (!g_bFocus)
	{
		if (NET()->GetSocket(WNetworkSystem::NET_GAME)->GetRcvCnt() < 5000)
			Sleep(50);
	}
	else if (CBrowser::Instance()->IsOpen())
		Sleep(10);
}

void CProjectG::ReportError(int code, const char* format, ...)
{
	char text[256] = "";
	va_list args;
	va_start(args, format);
	if (_vsnprintf(text, sizeof(text) - 1, format, args) != -1)
	{
		char report[512] = "";
		const char* nick = MyNick();
		_snprintf(report, sizeof(report) - 1, "%d %s %s %s %s", code,
			strlen(nick) ? nick : "unknown", PY_PUBLIC_VERSION, "645.00", text);
		CPangFBI::Instance()->ProcessReport("app_log", report);
	}
	va_end(args);
}

bool CProjectG::HidePrivacy()
{
	return m_bHidePrivacy;
}

bool CProjectG::HideGUI()
{
	return m_bHideGUI;
}

bool CProjectG::HidePI()
{
	return m_bHidePI;
}

static bool CheckFileMD5(const char* filename, const unsigned char* expected)
{
	cFile* file = g_resrcmng->GetCFile(filename, 0xffff);
	if (!file)
		return false;
	unsigned int size = file->Length();
	unsigned char* data = new unsigned char[size];
	file->Read(data, size);
	CloseCFile(file);
	MD5Context context;
	unsigned char digest[16];
	MD5Init(&context);
	MD5Update(&context, data, size);
	MD5Final(digest, &context);
	delete[] data;
	for (int i = 0; i < 16; ++i)
		if (expected[i] != digest[i])
			return false;
	return true;
}

bool CProjectG::CheckUIHack()
{
	UIFileChecksum* check = uiChecks;
	for (unsigned int i = 0; i < sizeof(uiChecks) / sizeof(uiChecks[0]);
		++i, ++check)
		if (!CheckFileMD5(check->filename, check->digest))
			return false;
	return true;
}

void CProjectG::ResetCaptuerNum()
{
	m_captureNum = 0;
}

void CProjectG::Draw()
{
	if (CBrowser::Instance()->IsOpen())
		return;
	WMatrix camera = g_camera;
	unsigned char flash = 0;
	if (CFx::Instance())
		CFx::Instance()->GetOutput(camera.pivot, flash);
	g_view->SetCamera(camera);
	g_view->BeginScene();
	g_view->Clear(0, 6); // 983 00a64b4e: cor e Z
	g_view->SetPrevCamera(m_prevCamera);
	m_prevCamera = g_view->GetCamera();
	CTaskManager::Instance()->DisplayTask();
	CMouseCursor::Instance()->Display();
	CNoticeBoard::Instance()->Display();
	CWinnerNotice::Instance()->Display();
	if (!CSceneManager::Instance())
		CFx::Instance()->Display();
	CBrowser::Instance()->Display();
	if (IsLocalContent(S4_REPLAY_SYSTEM) && Doc()->m_playingMode == 1)
	{
		if (IS_EXACTKINDOF(CGolfTask, AfxGetTask()) &&
			(Doc()->m_replayPlayType == 1 || Doc()->m_replayPlayType == 2) &&
			Doc()->m_roomInfo.realGameType != GAME_TYPE_USEMAX)
		{
			FrGraphicInterface* graphics = g_pFresh->GetManager()->GetGDI();
			graphics->SetTextStyle(2);
			graphics->SetTextColor(0xffff0000, 0xffffffff);
			if (Doc()->m_replayPlayType == 1)
				graphics->Print(WPoint(300, g_view->GetHeight() - 40), 0,
					K2L_Compatibility("\270\256\307\303\267\271\300\314\260\241 \300\372\300\345\265\307\276\372\275\300\264\317\264\331.(\274\246)"));
			else if (Doc()->m_replayPlayType == 2)
				graphics->Print(WPoint(300, g_view->GetHeight() - 40), 0,
					K2L_Compatibility("\270\256\307\303\267\271\300\314\260\241 \300\372\300\345\265\307\276\372\275\300\264\317\264\331."));
			graphics->SetTextStyle(0);
		}
	}
	if (timeGetTime() - captureMessageTime < 1000 && CChatMsg::Instance())
	{
		WFont* font = CChatMsg::Instance()->GetMaskedFont();
		font->Print(g_view,
			g_view->GetWidth() * 0.5f -
				font->GetTextWidthOld983(g_view, captureMessage) * 0.5f,
			g_view->GetHeight() - 25.0f, captureMessage, 0, 0xffff0000, NULL);
		CChatMsg::Instance()->GetMaskedFont()->Flush(g_view);
	}
	if (flash)
	{
		WRect rect(0, 0, g_view->GetWidth(), g_view->GetHeight());
		WOverlay::DrawBox(g_view, rect, 0, (flash << 24) | 0xffffff, 0.001f);
	}
	++drawCount;
}

void CProjectG::Paint()
{
	if (!CBrowser::Instance()->IsOpen())
	{
		g_view->EndScene();
		g_view->Render();
		g_last_screen_width = (int)g_view->GetWidth();
		g_last_screen_height = (int)g_view->GetHeight();
		WVideoDev* video = g_view->GetVideoDevice();
		if (video)
		{
			int state = video->GetDeviceState();
			if (state == 1 && g_bFocus)
			{
				g_bFocus = false;
				m_bLostFocus = true;
			}
			else if (m_bLostFocus && state == 0 && !g_bFocus)
			{
				g_bFocus = true;
				m_bLostFocus = false;
			}
		}
		if (bool(m_flag & 2) && g_bAviCapture)
		{
			static Bitmap frame(g_last_screen_width, g_last_screen_height, 24);
			if (g_resrcmng->VideoReference()->Command(W_VDEV_CAPTURE_SCREEN, 0,
					(int)&frame) == 1)
			{
				m_avi.Write(&frame);
				SetGameSpeed(1.0f);
				g_bAviCapture = false;
			}
		}
	}
}

// 983 00a62d40 / 00a62cb0: Ctrl+V (WinProc) liga o pedido; o chat (CChatMsg::Process 009a03b0) consome e le o texto
// da area de transferencia do Windows (CF_TEXT, ate 0x3ff bytes)
static bool s_bClipboardPaste983; // 983 00f0248a
static bool s_bCtrlDown983;       // 983 00f02a68
static bool s_bPasteKeyDown983;   // 983 00f02a67
static char s_clipboardText983[0x400]; // 983 00f02550

bool ConsumeClipboardPaste983()
{
	if (s_bClipboardPaste983)
	{
		s_bClipboardPaste983 = false;
		return true;
	}
	return false;
}

const char* GetClipboardText983()
{
	if (IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(g_hwnd))
	{
		HANDLE hMem = GetClipboardData(CF_TEXT);
		if (hMem)
		{
			const unsigned char* src = (const unsigned char*)GlobalLock(hMem);
			if (src)
			{
				memset(s_clipboardText983, 0, sizeof(s_clipboardText983));
				_mbsnbcpy((unsigned char*)s_clipboardText983, src, 0x3ff);
				GlobalUnlock(hMem);
			}
		}
		CloseClipboard();
		return s_clipboardText983;
	}
	memset(s_clipboardText983, 0, sizeof(s_clipboardText983));
	return s_clipboardText983;
}

long __stdcall CProjectG::WinProc(HWND hwnd, unsigned int message,
	unsigned int wParam, long lParam)
{
	switch (message)
	{
	case WM_KEYDOWN: // 983 00a67dd0 (0x100)
		if (s_bCtrlDown983 && !s_bPasteKeyDown983 && (wParam == 'V' || wParam == 'v'))
		{
			s_bPasteKeyDown983 = true;
			s_bClipboardPaste983 = true;
		}
		if (wParam == VK_CONTROL)
			s_bCtrlDown983 = true;
		break;
	case WM_KEYUP: // 983 (0x101)
		if (s_bPasteKeyDown983 && (wParam == 'V' || wParam == 'v'))
		{
			s_bClipboardPaste983 = false;
			s_bPasteKeyDown983 = false;
		}
		if (wParam == VK_CONTROL)
			s_bCtrlDown983 = false;
		break;
	case 44468:
		if (m_crc.IsWorking())
			m_crc.Resume();
		break;
	case 44467:
		if (m_crc.IsWorking())
			m_crc.Pause();
		break;
	case 44466:
		if (!wParam)
		{
			std::string filename = (const char*)lParam;
			MessageBox(NULL,
				MakeStr(
					K2L_Compatibility("%s\306\304\300\317\277\241 \274\325\273\363\300\314 \271\337\260\337\265\307\276\372\275\300\264\317\264\331. \264\331\275\303 \275\307\307\340\307\330 \301\326\274\274\277\344."),
					filename.c_str()),
				K2L_Compatibility("\306\316\276\337\277\241\267\257"), MB_ICONEXCLAMATION);
			PostQuitMessage(0);
		}
		break;
	case WM_USER + 10:
	{
		if (!NET())
			return 0;
		WClientSocket* socket = NET()->GetSocket(WNetworkSystem::NET_GAME);
		if (!socket)
			return 0;
		int reason;
		if (!WSAGETSELECTERROR(lParam))
		{
			switch (WSAGETSELECTEVENT(lParam))
			{
			case FD_READ:
				if (socket->TryRead(false))
					goto DefaultProcessing;
				socket->Close();
				reason = 1;
				break;
			case FD_WRITE:
				socket->Flush();
				goto DefaultProcessing;
			case FD_CLOSE:
				reason = 2;
				break;
			default:
				goto DefaultProcessing;
			}
		}
		else
		{
			// 983 00a68285: erro no FD_CONNECT (inclusive o que o GameUnit::Connect posta) usa o próprio código como motivo
			reason = 3;
			if (WSAGETSELECTEVENT(lParam) == FD_CONNECT)
				reason = WSAGETSELECTERROR(lParam);
		}
		if (g_iRecvDisconCode)
			g_iRecvDisconCode = 0;
		else if (AfxGetTask()->GetActor("GolfRule"))
		{
			AfxGetTask()->GetActor("GolfRule") << MsgObject(NULL, M983(0x24d), 0,
				(int)"GB_LOCAL_NETWORK_ERROR2", 0, 0, 0); // 983 00a682d1
			ReportError(4, "%d_%d_%d_%d_%d_%d(%d_%d)", reason,
				WSAGETSELECTERROR(lParam), g_iSendFailType, WSAGETSELECTEVENT(lParam) /*983; 645: WSAGetLastError()*/,
				g_iSendPacketType, g_iRecvPacketType,
				Doc()->m_roomInfo.roomGuid, MyGuid(false));
		}
		else
			ReportError(4, "%d_%d_%d_%d_%d_%d", reason,
				WSAGETSELECTERROR(lParam), g_iSendFailType, WSAGETSELECTEVENT(lParam) /*983*/,
				g_iSendPacketType, g_iRecvPacketType);
		// 983 00a68395: sem a troca automática de servidor do 645 (0x5a/0x5b); o motivo vai no texto
		// 983 00a683b7: motivo < 4 = texto 0x65 ("... (socked error: %d)") formatado so com o erro, sem " : ErrorCode"
		OpenDisconDlg(reason >= 4
			? MakeStr("%s : ErrorCode [%d]",
				K2L_Compatibility("\260\324\300\323\274\255\271\366\277\241 \301\242\274\323\307\322 \274\366 \276\370\275\300\264\317\264\331."),
				WSAGETSELECTERROR(lParam))
			: MakeStr(GetGameString(0x65), WSAGETSELECTERROR(lParam)),
			true);
		return 1;
	}
	case WM_USER + 12:
		if (WSAGETSELECTERROR(lParam))
		{
			if (WSAGETSELECTEVENT(lParam) == FD_CONNECT)
			{
				if (WSAGETSELECTERROR(lParam) != WSAEWOULDBLOCK)
					NET()->SendMessage(WNetworkSystem::NET_MSN,
						NetworkUnit::eEVENT_FAIL);
				return 0;
			}
			CMessengerInfo::Instance()->Close(true);
			NET()->ForceShutDown(WNetworkSystem::NET_MSN);
			return 0;
		}
		else
		{
			if (WSAGETSELECTEVENT(lParam) == FD_CONNECT)
			{
				NET()->GetSocket(WNetworkSystem::NET_MSN)->m_bConnected = true;
				NET()->SendMessage(WNetworkSystem::NET_MSN,
					NetworkUnit::eEVENT_NEXT);
			}
			else if (WSAGETSELECTEVENT(lParam) == FD_READ)
			{
				if (NET()->GetSocket(WNetworkSystem::NET_MSN) &&
					!NET()->GetSocket(WNetworkSystem::NET_MSN)->TryRead(false))
				{
					CMessengerInfo::Instance()->Close(true);
					NET()->ForceShutDown(WNetworkSystem::NET_MSN);
				}
			}
			else if (WSAGETSELECTEVENT(lParam) == FD_WRITE)
			{
				WClientSocket* socket =
					NET()->GetSocket(WNetworkSystem::NET_MSN);
				if (socket)
					socket->Flush();
			}
			else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
			{
				CMessengerInfo::Instance()->Close(true);
				NET()->ForceShutDown(WNetworkSystem::NET_MSN);
			}
		}
		return 0;
	case WM_USER + 13:
		if (WSAGETSELECTERROR(lParam))
		{
			NET()->GetSocket(WNetworkSystem::NET_LOGIN)->m_bConnected = false;
			OpenDisconDlg(
				K2L_Compatibility("\267\316\261\327\300\316 \274\255\271\366\277\315 \277\254\260\341\300\314 \262\367\276\356\301\263\275\300\264\317\264\331."),
				false);
			return 0;
		}
		if (WSAGETSELECTEVENT(lParam) == FD_READ)
		{
			if (NET()->GetSocket(WNetworkSystem::NET_LOGIN) &&
				!NET()->GetSocket(WNetworkSystem::NET_LOGIN)->TryRead(false))
				OpenDisconDlg(
					K2L_Compatibility("\267\316\261\327\300\316 \274\255\271\366\277\315 \277\254\260\341\300\314 \262\367\276\356\301\263\275\300\264\317\264\331."),
					false);
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_WRITE)
		{
			WClientSocket* socket = NET()->GetSocket(WNetworkSystem::NET_LOGIN);
			if (socket)
				socket->Flush();
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
			OpenDisconDlg(
				K2L_Compatibility("\267\316\261\327\300\316 \274\255\271\366\277\315 \277\254\260\341\300\314 \262\367\276\356\301\263\275\300\264\317\264\331."),
				false);
		return 0;
	case WM_USER + 14:
	{
		// C751: 983 00a68544..a685ec. Queda do ranking so avisa o "Lobby", se existir; o 645 fechava o cursor e
		// trocava para CLobbyTask (tirava o jogador da sala/partida). FD_CLOSE so fecha o socket, sem aviso.
		bool bAvisa = false;
		if (WSAGETSELECTERROR(lParam))
		{
			NET()->GetSocket(WNetworkSystem::NET_RANK)->m_bConnected = false;
			bAvisa = true;
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_READ)
		{
			if (NET()->GetSocket(WNetworkSystem::NET_RANK) &&
				!NET()->GetSocket(WNetworkSystem::NET_RANK)->TryRead(false))
				bAvisa = true;
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
			NET()->GetSocket(WNetworkSystem::NET_RANK)->Close();

		if (bAvisa && CTaskManager::Instance() && AfxGetTask()->GetActor("Lobby"))
			CTaskManager::Instance()->PostMsg(NULL, "Lobby", M983(0x1a), 0, 0, 0, 0);
		return 0;
	}

	case WM_SETFOCUS:
		if (!g_bFocus && CBrowser::Instance() && CBrowser::Instance()->IsOpen())
			CBrowser::Instance()->InvalidateButtons983();
		if (g_hwnd == GetForegroundWindow())
		{
			// 983 00a67e9f: com foco o cursor do sistema some (o jogo desenha o seu)
			if (g_pMouseCursorHelper983 && g_pMouseCursorHelper983->m_showCount >= 0)
				g_pMouseCursorHelper983->SetVisible(false);
			g_bFocus = true;
		}
		break;
	case WM_KILLFOCUS:
		if (!CBrowser::Instance() || !CBrowser::Instance()->IsOpen())
			g_bFocus = false;
		if (g_pMouseCursorHelper983)
		{
			if (BindMouseCursor983() != 0)
				g_pMouseCursorHelper983->SetVisible(true);
			else if (RFSceneManager::Instance() && RFSceneManager::Instance()->IsTyping983())
			{
				// 983 00a67fb5: digitando num campo da RF o foco segue no jogo
				g_bFocus = true;
				g_pMouseCursorHelper983->SetVisible(false);
			}
			else
				g_pMouseCursorHelper983->Restore();
		}
		break;
	case WM_SETCURSOR:
		// 983 00a680d8: no modo janela solto, cursor do sistema escondido na area cliente (HTCLIENT) e visivel na borda
		if (COption::Instance() && BindMouseCursor983() != 1 && COption::Instance()->vIsWindowed() && g_bFocus &&
			g_pMouseCursorHelper983 && g_pMouseCursorHelper983->m_state != LOWORD(lParam))
			g_pMouseCursorHelper983->SetState(LOWORD(lParam));
		break;
	case WM_CLOSE:
		Quit();
		return 0;
	case WM_NCHITTEST:
		if (!bool(m_flag & 4))
			return HTCLIENT;
		break;
	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_SIZE:
		case SC_MOVE:
		case SC_MAXIMIZE:
		case SC_KEYMENU:
		case SC_MONITORPOWER:
			if (!bool(m_flag & 4))
				return 1;
		}
		break;
	case WM_ACTIVATEAPP:
		if (!bool(m_flag & 4))
		{
			if (wParam)
			{
				SetWindowLong(g_hwnd, GWL_STYLE,
					GetWindowLong(g_hwnd, GWL_STYLE) & ~WS_SYSMENU);
				ShowWindow(g_hwnd, SW_RESTORE);
			}
			else
			{
				SetWindowLong(g_hwnd, GWL_STYLE,
					GetWindowLong(g_hwnd, GWL_STYLE) & ~WS_SYSMENU);
				ShowWindow(g_hwnd, SW_MINIMIZE);
			}
			return 1;
		}
		if (g_miles)
			g_miles->Activate(wParam != 0);
		break;
	case WM_PAINT:
		if (CBrowser::Instance() && CBrowser::Instance()->IsOpen())
		{
			CBrowser::Instance()->InvalidateButtons983();
			return DefWindowProc(g_hwnd, message, wParam, lParam);
		}
		break;

	case WM_SYSKEYDOWN:
		if (wParam == VK_F4)
		{
			if (IS_KINDOF(CGolfTask, AfxGetTask()))
			{
				if (IsLocalContent(S4_REPLAY_SYSTEM))
				{
					if (Doc()->m_playingMode == 1)
						return 1;
				}
				else
					return 1;
			}
		}
		break;
	}
DefaultProcessing:
	// 983 (00a67f2b; B159.2): a RF ve toda mensagem antes do WProcManager
	if (RFSceneManager::Instance())
		RFSceneManager::Instance()->OnWndMsg983(message, wParam, lParam);
	if (m_pProcManager)
		return m_pProcManager->WinProc(g_hwnd, message, wParam, lParam);
	return 0;
}

bool CProjectG::Init()
{
	m_pDeviceManager->SetHWND(g_hwnd);
	if (!SetVideoDevice())
		return false;
	if (!SetInputDevice())
		return false;
	SetAudioDevice();
	UpdateWindow(g_hwnd);
	ShowWindow(g_hwnd, SW_SHOW);
	InvalidateRect(g_hwnd, NULL, FALSE);
	UpdateWindow(g_hwnd);
	m_captureNum = 0;
	return true;
}

// 983 00a63c80 (C104.2): leitor dos #include dos shaders; a wangreal (Command 0x27) chama como __fastcall
static bool __fastcall ShaderIncludeProc983(const char* name, const void** ppData, unsigned int* pBytes)
{
	cFile* file = g_resrcmng->GetCFile(name, -1);
	if (file == NULL)
		return false;
	*pBytes = file->Length();
	void* data = operator new(*pBytes);
	file->Read(data, *pBytes);
	*ppData = data;
	CloseCFile(file);
	return true;
}

// 983 00a63bb0 (unico chamador: RunProjectG 00a728a8 com -1,-1): janela no tamanho da opcao, centrada na tela (sem
// prender y em 0 como o Reset do wangreal)
void CProjectG::ResizeWindow983(int width, int height)
{
	const sOption* opt = COption::Instance()->GetOption();
	if (width < 0)
		width = opt->vScreenWidth; // [0xe73f00]+0xd8
	if (height < 0)
		height = opt->vScreenHeight; // +0xdc
	RECT rect = { 0, 0, width, height };
	AdjustWindowRectEx(&rect, WS_CAPTION | WS_THICKFRAME, FALSE, 0);
	int x = (rect.left - rect.right + GetSystemMetrics(SM_CXSCREEN)) / 2;
	int y = (rect.top - rect.bottom + GetSystemMetrics(SM_CYSCREEN)) / 2;
	OffsetRect(&rect, x, y);
	MoveWindow(g_hwnd, x, y, rect.right - rect.left, rect.bottom - rect.top,
		FALSE);
}

bool CProjectG::SetVideoDevice()
{
	char caption[64] = "";
	int tnl = COption::Instance()->vGetTnLMode();
	RECT rect = { 0, 0, 800, 600 };
	AdjustWindowRectEx(&rect, WS_CAPTION | WS_THICKFRAME, FALSE, 0);
	int x = (rect.left - rect.right + GetSystemMetrics(SM_CXSCREEN)) / 2;
	int y = (rect.top - rect.bottom + GetSystemMetrics(SM_CYSCREEN)) / 2;
	OffsetRect(&rect, x, y);
	MoveWindow(g_hwnd, x, y, rect.right - rect.left, rect.bottom - rect.top,
		FALSE);
	m_flag.Enable(4);
	WVideoDev* video = m_pDeviceManager->CreateVideoDevice(NULL, "Window", tnl);
	if (!video)
	{
		// 983 00a65155: PANGYA_ARG_SUB=noWindow nao mostra a mensagem
		char arg[0x40] = "";
		GetEnvironmentVariableA("PANGYA_ARG_SUB", arg, sizeof(arg));
		if (strcmp(arg, "noWindow") != 0)
			MessageBox(g_hwnd,
				K2L_Compatibility("\264\331\300\275\260\372 \260\260\300\272 \271\256\301\246\267\316 \275\307\307\340\307\322 \274\366 \276\370\275\300\264\317\264\331.\012\012* DirectX 9.0c \300\314\273\363\300\314 \274\263\304\241 \265\307\301\366 \276\312\276\322\260\305\263\252\012* \265\360\275\272\307\303\267\271\300\314 \276\356\264\360\305\315(\272\361\265\360\277\300\304\253\265\345)\300\307 \265\345\266\363\300\314\271\366\260\241 \303\326\275\305 \271\366\300\374\300\314 \276\306\264\317\260\305\263\252\012* \265\360\275\272\307\303\267\271\300\314 \276\356\264\360\305\315(\272\361\265\360\277\300\304\253\265\345)\300\307 \274\272\264\311\277\241 \271\256\301\246\260\241 \300\326\275\300\264\317\264\331.\012\012\306\316\276\337 \275\307\307\340\275\303 \263\252\277\300\264\302 \303\271 \310\255\270\351\277\241\274\255 [\275\307\307\340\271\256\301\246\272\270\260\355]\270\246 \264\255\267\257 \304\304\307\273\305\315 \301\244\272\270\270\246 \265\356\267\317\307\317\275\305 \310\304,  \012\306\316\276\337 \310\250\306\344\300\314\301\366\300\307 [\260\355\260\264\301\366\277\370->FAQ]\270\246 \300\320\276\356\272\270\275\303\261\342 \271\331\266\370\264\317\264\331."),
				caption, MB_ICONERROR);
		return false;
	}
	video->SetMainThreadId(m_threadId);
	SetProc(video);
	g_resrcmng->SetVideoReference(video);
	video->Command(WX_VDEV_SET_SHADER_INCLUDE_PROC, (int)ShaderIncludeProc983, 0); // 983 00a63c80 (esta funcao = 983 00a65030, A330 conferida igual)
	m_refreshRate = (float)floor(video->GetMonitorSupportFps());
	SetFPS(m_refreshRate);
	return true;
}

bool CProjectG::SetAudioDevice()
{
	bool enabled = COption::Instance()->aIsMssEnabled();
	int frequency = COption::Instance()->aGetMssFrequency();
	int bits = COption::Instance()->aGetMssBits();
	int channels = COption::Instance()->aGetMssChannels();
	bool hardware = COption::Instance()->aIsMssHwSoundEnabled();
	float sfx = COption::Instance()->aGetSfxVolume();
	float bgm = COption::Instance()->aGetBgmVolume();
	g_miles = new WMilesSoundSystem(g_hwnd, "mss", enabled, frequency, bits,
		channels);
	g_audio = new CSoundManager;
	if (!g_miles || !g_miles->Init(g_hwnd, frequency, bits))
	{
		g_audio->SetMSS(NULL);
		return false;
	}
	WMilesSoundSystem::w_speaker_type speaker =
		COption::Instance()->aGetMssSpeaker();
	int balance = COption::Instance()->aGetMssBalance();
	g_miles->SpeakerType(speaker);
	g_miles->SetBalance(balance);
	g_miles->SetDistance(400.0f * g_CM_TO_WU, 3000.0f * g_CM_TO_WU);
	g_audio->SetMSS(g_miles);
	if (enabled)
	{
		g_audio->SetVolume(Between(0.0f, sfx, 1.0f));
		g_audio->SetBGMVolume(Between(0.0f, bgm, 1.0f));
	}
	else
	{
		g_audio->SetVolume(0.0f);
		g_audio->SetBGMVolume(0.0f);
	}
	return true;
}

void CProjectG::UploadShaderSource()
{
	WVideoDev* video = g_resrcmng->VideoReference();
	if (video)
	{
		cFile* file = g_resrcmng->GetCFile("pangya.fx", -1);
		if (file)
		{
			char* source = new char[file->Length() + 1];
			file->Read(source, file->Length());
			source[file->Length()] = 0;
			video->SetShaderSource(source);
			delete[] source;
			CloseCFile(file);
		}
		else
			video->SetShaderSource(NULL);
	}
}

bool CProjectG::SetInputDevice()
{
	g_keyboard = m_pDeviceManager->CreateInputDevice("DirectInput", "keyboard");
	SetProc(g_keyboard);
	AddInputDev(g_keyboard);
	g_mouse = m_pDeviceManager->CreateInputDevice("DirectInput", "mouse");
	SetProc(g_mouse);
	AddInputDev(g_mouse);
	g_ime = m_pDeviceManager->CreateInputDevice("Ime", "");
	SetProc(g_ime);
	AddInputDev(g_ime);
	return true;
}

void CProjectG::SetProc(WDevice* device)
{
	WProc* proc = device->ExternProc();
	if (proc)
		proc->SetProc(m_pProcManager, g_hwnd);
}

void CProjectG::Update(int time)
{
	for (WInputDev* input = m_inputList.Start(); input;
		input = m_inputList.Next())
		input->Update(g_CurrentTime);
}

void CProjectG::SetWindowed(bool windowed)
{
	if (windowed)
		m_flag.Enable(4);
	else
		m_flag.Disable(4);
}

void CProjectG::OnMsnPacket(WReceivedPacket& packet)
{
}

void CProjectG::ProcessRecvPacketQueue()
{
	if (CTaskManager::Instance())
	{
		CTask* task = CTaskManager::Instance()->GetCurrentTask();
		if (task && task->IsInited() &&
			!CTaskManager::Instance()->IsTaskChanging())
		{
			while (!m_recvPacketQueue.empty())
			{
				WReceivedPacket* packet = m_recvPacketQueue.front();
				if (!task->OnPacket(*packet))
				{
					packet->Reset();
					ProcessPacket(*packet);
				}
				delete packet;
				m_recvPacketQueue.pop();
			}
		}
	}
}

void CProjectG::OnPacket(WReceivedPacket& packet)
{
	if (CTaskManager::Instance())
	{
		CTask* preserved = CTaskManager::Instance()->GetPreservedTask();
		CTask* task = CTaskManager::Instance()->GetCurrentTask();
		if (task)
		{
			if (task->IsInited() && !CTaskManager::Instance()->IsTaskChanging())
			{
				while (!m_recvPacketQueue.empty())
				{
					WReceivedPacket* pending = m_recvPacketQueue.front();
					if (!task->OnPacket(*pending))
					{
						if (preserved)
						{
							pending->Reset();
							if (!preserved->OnPacket(*pending))
							{
								pending->Reset();
								ProcessPacket(*pending);
							}
						}
						else
						{
							pending->Reset();
							ProcessPacket(*pending);
						}
					}
					delete pending;
					m_recvPacketQueue.pop();
				}
				if (!task->OnPacket(packet))
				{
					if (preserved)
					{
						packet.Reset();
						if (!preserved->OnPacket(packet))
						{
							packet.Reset();
							ProcessPacket(packet);
						}
					}
					else
					{
						packet.Reset();
						ProcessPacket(packet);
					}
				}
			}
			else if (packet.Decode2() == 0x62) // 983 00a67575 (645: 96); C-PKT lote 56
			{
				packet.Reset();
				ProcessPacket(packet);
			}
			else
			{
				WReceivedPacket* pending = new WReceivedPacket;
				packet.Copy(pending);
				m_recvPacketQueue.push(pending);
			}
			return;
		}
	}
	ProcessPacket(packet);
}

void CProjectG::ProcessPacket(WReceivedPacket& packet)
{
	packet.Decode2();
}

void CProjectG::OnUDPPacket(WReceivedPacket& packet)
{
}

bool CProjectG::OnGameServerResult(int result, FrForm* form)
{
	if (CLoginInfo::Instance()->IsWebLogin())
	{
		g_bQuit = true;
		return true;
	}
	if (result)
	{
		FromGolfToLobbySettings();
		CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)"SERVERLIST",
			0, 0, 0);
		CMouseCursor::Instance()->SetActive(true);
		// 983 00a676d5: no lugar de tirar o servidor da lista (645), guarda o id no conjunto de servidores que
		// falharam (a proxima lista do pacote 0x2 os pula), zera os dados de servidor/canal e poe o ServerDlg no estado 0xd
		if (Doc()->m_curGameServer.id)
			Doc()->m_failedGameServerIds983.insert(Doc()->m_curGameServer.id);
		Doc()->ClearServerVars();
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", M983(0x1f), 0xd, 0, 0, 0);
	}
	return true;
}

bool CProjectG::OnLoginServerResult(int result, FrForm* form)
{
	g_bQuit = true;
	return true;
}

__int64 CProjectG::CalcHddFreeSpace()
{
	__int64 freeSpace = 0;
	char* drive = GetCurrentHdd();
	typedef BOOL(WINAPI * DiskSpaceProc)(LPCSTR, PULARGE_INTEGER,
		PULARGE_INTEGER, PULARGE_INTEGER);
	DiskSpaceProc getSpace = (DiskSpaceProc)GetProcAddress(
		GetModuleHandle("kernel32.dll"), "GetDiskFreeSpaceExA");
	if (getSpace)
	{
		ULARGE_INTEGER available, total, free;
		if (getSpace(drive, &available, &total, &free))
			freeSpace = free.QuadPart;
		else
			return 0;
	}
	return freeSpace;
}

char* CProjectG::GetCurrentHdd()
{
	CRegistry registry;
	registry.SetRootKey(HKEY_LOCAL_MACHINE);
	registry.SetKey("Software\\Ntreev\\Pangya_Jp", true);
	WTL::CString directory = "";
	directory = registry.ReadString("Install_Dir", "");
	static char drive[255];
	sprintf(drive, "%s", (const char*)directory);
	drive[2] = 0;
	return drive;
}
