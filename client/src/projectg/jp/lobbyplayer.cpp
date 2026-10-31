// JP 983: LobbyPlayer INTEIRA (unidade 00565300–005699e0; sessões B31 e B35): construtor 005690c0, destrutor 00568ee0, criador
// 00569310, registro 00569380, as 7 virtuais (OnPreLoadInit 00568c10, OnLoad 00565b40, OnProcess 00568600, OnDisplay 00566330,
// OnDestroy 00568dc0, HandleMsg 005693c0, OnAnotherHand 00565f10) e 24 auxiliares; mais `CLobbyState983::IsMain983` 00565af0,
// `SendLobbyChar983` 00566f30, `IsMouseOverPet983` 00565a90 e os auxiliares do mouse (B36): `RayBox983` 00565300 (raio ×
// caixa), `IsMouseOverPoint983` 00565620, `GetPetScreenRect983` 00565730, `CanPickPet983` 005659b0 e `IsMouseOverRect983`
// 00565ab0. Fonte reconstruída do 983 (classe sem par no 645; par no KR 839).
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "lobbyplayer.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "inputmanager.h"
#include "polysoup.h"
#include "commonutil.h"
#include "wpuppet.h"
#include "wbone.h"
#include "wresrcmng.h"
#include "fxinterpreter.h"
#include "petframe.h"
#include "soundmanager.h"
#include "parttidlist.h"
#include "packet.h"
#include "ucclibrary.h"
#include "uccmanager.h"
#include "createcharacteroption.h"
#include <utility>

// JP 983: objeto apontado por CSharedDoc+0x11024 (m_pGolfDoc; A729: membro, cl novo 0x10cc4); o WPolySoup do cenário está em +8
struct sLobbyScene983
{
	char m_00[8];
	WPolySoup* m_pSoup; // +0x08
};

extern const WVector g_vZero983_f041c4; // 00f041c4
// 00f041d0: matriz identidade; o 983 a monta em tempo de execucao (no dump e zero). Era dado-stub zerado (B1524).
extern const WMatrix g_mIdentity983_f041d0(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
extern const char* g_lobbyPlayerFx983; // 00e0c294 -> "avatar_d.seq"
extern const char* g_lobbyPlayerSnd983; // 00e0c298 -> "char_change"

extern const sLobbyRect983 g_lobbyRectZero983; // 00f14668
extern int g_bF00580_983; // 00f00580

class FrWndLobbyJp : public FrWnd
{
public:
	const std::string& GetName983(); // 00be76b0 (devolve o m_wndName, +0x64)
};
#define LPWNDNAME983(p) (((FrWndLobbyJp*)(p))->GetName983())

// CPetFrame do 983 (frente A, incompleto)
class CPetFrameLobbyJp : public CPetFrame
{
public:
	int SetParts983(CPartTidList* pList, unsigned long hairTid, unsigned long a, bool bHair); // 008b28b0
	bool SetHairClr983(CPartTidList* pList, unsigned char color, unsigned long hairTid, int a); // 008af6b0 (CPetFrame::SetHairClr)
	bool SetAuxPart983(unsigned char slot, unsigned long tid); // 008acd40 (CPetFrame::SetAuxPart)
	void SetPart983(const void* part); // 008b0960
	void CalcBVolume983(); // 008afc50 (CPetFrame::CalcBVolume)
};
#define LPFRAME983(p) ((CPetFrameLobbyJp*)(p)->m_pFrame.get())

// CItemManager (frente A): 00a17160 (registro de item por tipo; a subgolfrule usa o mesmo)
class CItemManagerLobby2Jp
{
public:
	void* Find160_983(unsigned long tid); // 00a17160
};

// tabela de movimentos de espera por personagem (CSharedDoc+0x10c24; unidade hatmanager do mapa)
class CLobbyMotionTable983
{
public:
	int Count983(const unsigned long* pTid); // 009a3c80
	const char* Get983(const unsigned long* pTid, const int* pIndex); // 009a3c00
};
#define LOBBYMOTION983() ((CLobbyMotionTable983*)&::Doc()->m_chatManager)

int __fastcall Fn500480_983(unsigned char a); // 00500480 (trecho novo; corpo NÃO lido; recebe o byte CSharedDoc+0x1094c)

#define LOBBYMSG983(n) AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, n, 0, 0, 0, 0, 0)
#define DOCCHARID983() (::Doc()->m_myInfo.userEquip.guidChar) // personagem em uso
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip.guidChar) == 0xa2a && offsetof(CSharedDoc, m_chatManager) == 0x10c24 && offsetof(CSharedDoc, m_itemManager) == 0x20c && offsetof(CSharedDoc, m_roomInfo.gameType) == 0x1094c && offsetof(CSharedDoc, m_roomInfo.roomGuid) == 0x1094d && offsetof(CSharedDoc, m_offlinePlay) == 0x10d30); // A736: crus -> membros
void __fastcall SendLobbyChar983(unsigned long id);

IMPLEMENT_ACTOR(LobbyPlayer, IActor)

LobbyPlayer::LobbyPlayer()
	: m_pLoad((IDummyPlayer*)NULL), m_pShow((IDummyPlayer*)NULL), m_loadId(0), m_loadTid(0), m_id(0), m_tid(0),
	  m_pos(g_vZero983_f041c4), m_fadeTime(0), m_bVisible(false), m_bFading(false), m_work(0), m_matrix(g_mIdentity983_f041d0),
	  m_40c(true), m_bAppear(true), m_motionParam(0), m_bRandomMotion(false), m_bLocked(false), m_bDrag(false), m_csPet(2000)
{
	for (int i = 0; i < 2; ++i)
	{
		m_bDone[i] = false;
		m_bPlace[i] = false;
	}
}

LobbyPlayer::~LobbyPlayer()
{
}

// 00565af0: o estado do lobby é 2 ou 5
int CLobbyState983::IsMain983() const
{
	if (m_state == 2 || m_state == 5)
		return 1;
	return 0;
}

// 00565300: raio (org + t * dir, t em 0…1) × caixa engordada de `r`. Devolve o t da entrada (1 = não toca) e, se pedido,
// o plano da face atingida. Eixos na ordem x, z, y.
float __fastcall RayBox983(const Waabb& box, const WVector& org, const WVector& dir, WPlane* pOut, float r)
{
	const float* b = (const float*)&box;
	const float* o = (const float*)&org;
	const float* d = (const float*)&dir;
	for (int i = 0; i < 3; i = (i == 0) ? 2 : ((i == 1) ? 3 : 1))
	{
		float a = d[i] < 0 ? -d[i] : d[i];
		if (a < 0.00001f)
			continue;
		float face;
		if (d[i] > 0)
		{
			if (o[i] > b[i + 3] + r)
				return 1.0f;
			face = b[i] - r;
			if (o[i] + d[i] < face)
				return 1.0f;
			if (o[i] > face)
				continue;
		}
		else
		{
			if (o[i] < b[i] - r)
				return 1.0f;
			face = b[i + 3] + r;
			if (o[i] + d[i] > face)
				return 1.0f;
			if (o[i] < face)
				continue;
		}
		float t = (face - o[i]) / d[i];
		int j;
		for (j = 0; j < 3; j++)
		{
			if (j == i)
				continue;
			float p = d[j] * t + o[j];
			float lo = b[j] - r;
			if (lo > p)
				break;
			float hi = b[j + 3] + r;
			if (hi < p)
				break;
		}
		if (j != 3)
			continue;
		if (pOut)
		{
			float* n = (float*)pOut;
			if (i == 0)
			{
				n[0] = d[0] < 0 ? 1.0f : -1.0f;
				n[3] = -(n[0] * (d[0] * t + o[0]));
				n[1] = 0;
				n[2] = 0;
			}
			else if (i == 1)
			{
				n[0] = 0;
				n[1] = d[1] < 0 ? 1.0f : -1.0f;
				n[3] = -(n[1] * (d[1] * t + o[1]));
				n[2] = 0;
			}
			else
			{
				n[0] = 0;
				n[1] = 0;
				n[2] = d[2] < 0 ? 1.0f : -1.0f;
				n[3] = -(n[2] * (d[2] * t + o[2]));
			}
		}
		return t;
	}
	return 1.0f;
}

// 00565620: o raio do ponto `mouse` da tela passa pela caixa de 5 × 9 × 5 posta em `pos`?
bool __fastcall IsMouseOverPoint983(const WVector& mouse, const WVector& pos)
{
	WVector org;
	WVector dir;
	g_view->GetScreen2World(mouse.x, mouse.y, org, dir);
	Waabb box(WVector(-2.5f, 0, -2.5f), WVector(2.5f, 9.0f, 2.5f));
	box.min.x = pos.x - 2.5;
	box.min.y += pos.y;
	box.min.z += pos.z;
	box.max.x = pos.x + 2.5;
	box.max.y += pos.y;
	box.max.z += pos.z;
	return RayBox983(box, org, dir, NULL, 0) < 1.0f;
}

// 00565730: retângulo do boneco de `pos` na tela — varre a linha do meio da tela e depois a coluna do meio do trecho
// achado, ponto a ponto, com o teste do raio
sLobbyRect983 __fastcall GetPetScreenRect983(const WVector& pos)
{
	sLobbyRect983 rect = g_lobbyRectZero983;
	if (!CUiPointer983::Instance983())
		return rect;
	WSize size = CUiPointer983::Instance983()->GetScreenSize983();
	float cy = size.h * 0.5;
	bool bFound = false;
	int i;
	for (i = 0; size.w > i; i++)
	{
		if (!bFound)
		{
			float x = (float)i;
			if (IsMouseOverPoint983(WVector(x, cy, 0), pos))
			{
				rect.x = x;
				bFound = true;
			}
		}
		else if (!IsMouseOverPoint983(WVector((float)i, cy, 0), pos))
		{
			rect.w = i - rect.x;
			break;
		}
	}
	if (rect.w == 0)
		return g_lobbyRectZero983;
	float cx = rect.w * 0.5 + rect.x;
	bFound = false;
	for (i = 0; size.h > i; i++)
	{
		if (!bFound)
		{
			float y = (float)i;
			if (IsMouseOverPoint983(WVector(cx, y, 0), pos))
			{
				rect.y = y;
				bFound = true;
			}
		}
		else if (!IsMouseOverPoint983(WVector(cx, (float)i, 0), pos))
		{
			rect.h = i - rect.y;
			break;
		}
	}
	if (rect.h == 0)
		return g_lobbyRectZero983;
	return rect;
}

// 005659b0: o mouse pode pegar um boneco? Não com janela modal (salvo "realmyroom" e "shopmain") nem com outra janela
// debaixo do ponteiro (mesma exceção).
bool __fastcall CanPickPet983(const WVector& mouse)
{
	if (!CUiPointer983::Instance983())
		return false;
	if (!g_bF00580_983)
		return false;
	if (g_pF02340_983 && FRESHMGR983())
	{
		FrWnd* pModal = FRESHMGR983()->GetModalForm();
		if (pModal && "realmyroom" != LPWNDNAME983(pModal) && "shopmain" != LPWNDNAME983(pModal))
			return false;
	}
	if ((CUiPointer983::Instance983() ? CUiPointer983::Instance983()->m_pRF : NULL)->IsModalExist983())
		return false;
	FrWnd* pWnd = FRESHMGR983()->FindWndAt983(mouse.x, mouse.y);
	if (pWnd && "realmyroom" != LPWNDNAME983(pWnd) && "shopmain" != LPWNDNAME983(pWnd))
		return false;
	return true;
}

// 00565ab0: o mesmo teste, com um retângulo da tela no lugar do boneco
bool __fastcall IsMouseOverRect983(const WVector& mouse, WRect& rect)
{
	if (!CanPickPet983(mouse))
		return false;
	return rect.IsInRect(mouse.x, mouse.y);
}

// 00565a90
bool __fastcall IsMouseOverPet983(const WVector& mouse, const WVector& pos)
{
	if (!CanPickPet983(mouse))
		return false;
	return IsMouseOverPoint983(mouse, pos);
}

sLobbyRect983 LobbyPlayer::GetScreenRect983()
{
	return GetPetScreenRect983(m_pos);
}

// corpo comum das duas linhas de carga: roda o trabalho pendente de cada uma
void LobbyPlayer::OnAnotherHand(int key, void* param)
{
	if (m_hands[0].IsWorking(0))
	{
		m_loads[0].m_loader();
		m_loads[0].m_bLoaded = true;
	}
	if (m_hands[1].IsWorking(0))
	{
		m_loads[1].m_loader();
		m_loads[1].m_bLoaded = true;
	}
}

void LobbyPlayer::Request983(int work, unsigned long tid, unsigned long id)
{
	if (work == 2)
		return;
	if (m_hands[work].IsWorking(0))
		return;
	m_loads[work].m_id = id;
	m_loads[work].m_tid = tid;
	m_loads[work].m_bLoaded = false;
	m_loads[work].m_bReady = true;
}

void LobbyPlayer::OnLoad()
{
}

void LobbyPlayer::OnDisplay()
{
	IDummyPlayer* pPlayer = m_pShow.get();
	if (pPlayer && pPlayer->m_pFrame.get())
		pPlayer->UpdateTexture983();
}

void LobbyPlayer::SetPoint983(const char* name)
{
	sLobbyScene983* pScene = (sLobbyScene983*)::Doc()->m_pGolfDoc; // A729: membro (cl novo 0x10cc4)
	WPolySoup::sPoint* pPoint = pScene->m_pSoup->FindPoint(name);
	if (pPoint)
		m_pos = pPoint->pos;
}

void LobbyPlayer::Rotate983(float angle)
{
	IDummyPlayer* pPlayer = m_pShow.get();
	if (pPlayer)
	{
		WMatrix rot;
		RotMat(angle, 1, rot);
		m_pShow->m_matrix = rot * pPlayer->m_matrix;
	}
}

// há boneco em carga? (lido sob a seção crítica)
bool LobbyPlayer::IsLoading983()
{
	m_csPet.lock();
	IDummyPlayer* pPlayer = m_pLoad.get();
	m_csPet.unlock();
	return pPlayer != NULL;
}


// põe o boneco recém-carregado na cena: movimento de espera, efeito e som de aparecer, posição e elemento da cena
bool LobbyPlayer::Place983()
{
	IDummyPlayer* pPlayer = m_pShow.get();
	if (!pPlayer || !pPlayer->m_pFrame.get() || !pPlayer->SetMotion("\xb1\xe2\xba\xbb\xc0\xda\xbc\xbc", false))
		return false;
	if (m_work == 0 || m_id != 0)
	{
		if (m_bAppear)
		{
			m_pShow->m_pFx->PlayFx983(g_lobbyPlayerFx983, &m_pos);
			g_audio->PlaySfx(g_lobbyPlayerSnd983, NULL, 0, NULL, NULL, 0.5f, 200.0f);
		}
		m_fadeTime = 0;
	}
	else
	{
		m_bAppear = false;
	}
	m_pShow->SetPosition(m_pos);
	m_pShow->m_bPlaying = true;
	m_pShow->m_bShow = true;
	m_pShow->m_pFrame->SetAlpha(m_bAppear ? 0 : 0xff);
	m_bVisible = !m_bAppear;
	m_bFading = m_bAppear;
	::Doc()->m_bLobbyBgCameraSway983 = false;
	if (CLobbyState983::Instance983() && CLobbyState983::Instance983()->m_state == 1)
		AfxGetTask()->GetActor("LobbyBg") << MsgObject(NULL, 0x17f, (int)"DefaultCamera", 0, 0, 0, 0);
	CSceneManagerJp::Instance983()->AddElement983(4, 0, 0, m_pShow->m_name.c_str(), 0x18, 0);
	CSceneManagerJp::Instance983()->SetPuppet(m_pShow->m_name.c_str(), m_pShow->m_pFrame->GetBonePet());
	return true;
}

void LobbyPlayer::SetVisible983(bool bVisible, bool bFade)
{
	if (m_bVisible == bVisible)
		return;
	m_bFading = bFade;
	if (!bFade)
	{
		if (m_pShow.get() && m_pShow->m_pFrame.get())
		{
			m_pShow->m_pFrame->SetAlpha(bVisible ? 0xff : 0);
			m_bVisible = bVisible;
			m_bFading = false;
		}
	}
	else
	{
		m_fadeTime = 0;
	}
}

bool LobbyPlayer::Fade983(float dt)
{
	if (!m_pShow.get() || !m_pShow->m_pFrame.get())
		return false;
	float from = 0;
	float to = 255.0f;
	if (m_bVisible)
	{
		from = 255.0f;
		to = 0;
	}
	float alpha = 0;
	if (m_work == 0 || m_id != 0)
	{
		alpha = ::Fade983(from, to, m_fadeTime, 0.7f);
		m_fadeTime += dt;
	}
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
	m_pShow->m_pFrame->SetAlpha((unsigned char)(int)alpha);
	return m_bVisible;
}

extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
// D820 (item 11 dono10, lag ao provar pacote na loja): ms de cada fase do 0x18a; loja/handler = 983 (00a8ca90/005666d0)
static double D820Ms(LARGE_INTEGER& t)
{
	LARGE_INTEGER f, n;
	QueryPerformanceFrequency(&f);
	QueryPerformanceCounter(&n);
	double ms = (double)(n.QuadPart - t.QuadPart) * 1000.0 / (double)f.QuadPart;
	t = n;
	return ms;
}

// troca as peças do boneco em cena pela lista dada (mensagem 0x18a); devolve o que o CPetFrame devolver
int LobbyPlayer::ChangeParts983(CPartTidList* pList, unsigned long hairTid, unsigned long a, bool bHair)
{
	int ret = 0;
	if (m_pShow.get() && m_pShow->m_pFrame.get())
	{
		if (!((CItemManagerLobby2Jp*)&::Doc()->m_itemManager)->Find160_983(hairTid))
			hairTid = 0;
		LARGE_INTEGER t0;
		QueryPerformanceCounter(&t0);
		double ms[6] = { 0, 0, 0, 0, 0, 0 };
		LOBBYMSG983(0x53);
		ret = LPFRAME983(m_pShow)->SetParts983(pList, hairTid, a, bHair);
		ms[0] = D820Ms(t0);
		if (ret)
		{
			m_pShow->m_pFrame->GetBonePet()->xUpdateTnLBuffers(true);
			ms[1] = D820Ms(t0);
			LPFRAME983(m_pShow)->CalcBVolume983();
			m_pShow->RefreshFx983(true, false, 0, 1);
			ms[2] = D820Ms(t0);
			m_pShow->m_pFrame->Refresh(pList);
			ms[3] = D820Ms(t0);
			if (bHair)
				LPFRAME983(m_pShow)->SetHairClr983(pList, pList->m_hairColor, hairTid, 1);
			else
				LPFRAME983(m_pShow)->SetHairClr983(pList, pList->m_hairColor, 0, 1);
			ms[4] = D820Ms(t0);
			WBone* pRoot = m_pShow->m_pFrame->GetBonePet()->m_rootbone;
			if (pRoot)
			{
				WBone* pBone = pRoot->FindBone(CreateCharacterOption::Instance()->GetSubBone983(m_pShow->m_charTid), 0);
				if (pBone)
					pBone->SetInvisible(false, true);
			}
		}
		LOBBYMSG983(0x54);
		ms[5] = D820Ms(t0);
		if (ms[0] + ms[1] + ms[2] + ms[3] + ms[4] + ms[5] >= 2.0)
			jp983_rastro("D820 18a tid=%08x ret=%x setpart=%.1f tnl=%.1f fx=%.1f refresh=%.1f hair=%.1f msg54=%.1f ms", hairTid, ret,
				ms[0], ms[1], ms[2], ms[3], ms[4], ms[5]);
		if (CLobbyState983::Instance983())
			CLobbyState983::Instance983()->m_bChanged = true;
	}
	return ret;
}

// A679: 0061f8b0/0061dfb0 com tid 0x70 no My Room (DAT_00e78cac != 0): 008acd40 (CPetFrame::SetAuxPart) direto na previa
void MyRoomPetFrameSetHairClr983(CPetFrame* pFrame, CPartTidList* pList, unsigned char color)
{
	((CPetFrameLobbyJp*)pFrame)->SetHairClr983(pList, color, 0, 1);
}

bool MyRoomPetFrameSetAuxPart983(CPetFrame* pFrame, unsigned char slot, unsigned long tid)
{
	return ((CPetFrameLobbyJp*)pFrame)->SetAuxPart983(slot, tid);
}

// 0061f8b0 (equipar, DAT_00e78cac != 0): em vez da mensagem 0x18a o My Room troca as pecas direto no CPetFrame da previa (+0x584 -> +0xa8):
// 008b28b0(lista, tid, uid, 1) e desfaz o "invisivel" do sub-bone do personagem (4cef20/b96960/b97910); devolve o ret
int MyRoomPetFrameSetParts983(CPetFrame* pFrame, CPartTidList* pList, unsigned long tid, unsigned long uid)
{
	CPetFrameLobbyJp* f = (CPetFrameLobbyJp*)pFrame;
	int ret = f->SetParts983(pList, tid, uid, true);
	WBone* pRoot = f->GetBonePet()->m_rootbone;
	if (pRoot)
	{
		WBone* pBone = pRoot->FindBone(CreateCharacterOption::Instance()->GetSubBone983(*(unsigned long*)f & 0x3ffffff), 0);
		if (pBone)
			pBone->SetInvisible(false, true);
	}
	return ret;
}

bool LobbyPlayer::SetAuxPart983(unsigned char slot, unsigned long tid)
{
	bool ret = false;
	if (m_pShow.get() && m_pShow->m_pFrame.get())
	{
		if (tid && !::Doc()->m_itemManager.FindAuxPart(tid))
			return false;
		LOBBYMSG983(0x53);
		ret = LPFRAME983(m_pShow)->SetAuxPart983(slot, tid);
		if (ret)
		{
			m_pShow->m_pFrame->GetBonePet()->xUpdateTnLBuffers(true);
			LPFRAME983(m_pShow)->CalcBVolume983();
			m_pShow->RefreshFx983(false, false, 0, 1);
		}
		LOBBYMSG983(0x54);
		if (CLobbyState983::Instance983())
			CLobbyState983::Instance983()->m_bChanged = true;
	}
	return ret;
}

bool LobbyPlayer::SetHair983(CPartTidList* pList, unsigned char color, unsigned long hairTid)
{
	bool ret = false;
	if (m_pShow.get() && m_pShow->m_pFrame.get())
	{
		if (hairTid && !::Doc()->m_itemManager.FindHairStyle(hairTid))
			return false;
		LOBBYMSG983(0x53);
		ret = LPFRAME983(m_pShow)->SetHairClr983(pList, color, 0, 1);
		if (ret)
		{
			m_pShow->RefreshFx983(false, false, 0, 1);
			m_pShow->m_pFrame->Refresh(pList);
		}
		LOBBYMSG983(0x54);
		if (CLobbyState983::Instance983())
			CLobbyState983::Instance983()->m_bChanged = true;
	}
	return ret;
}

// veste a roupa de desenho próprio `uccId` (o 1º argumento não é lido)
bool LobbyPlayer::SetUcc983(int a, unsigned long uccId)
{
	bool ret = false;
	if (m_pShow.get() && m_pShow->m_pFrame.get())
	{
		LOBBYMSG983(0x53);
		sUccClothes* pClothes = UccManager()->FindClothes(uccId);
		if (pClothes)
		{
			LPFRAME983(m_pShow)->SetPart983(pClothes);
			ret = true;
		}
		LOBBYMSG983(0x54);
		if (CLobbyState983::Instance983())
			CLobbyState983::Instance983()->m_bChanged = true;
		return ret;
	}
	return false;
}

bool LobbyPlayer::Input983(float dt)
{
	if (!g_bFocus)
		return false;
	unsigned int buttons = (CUiPointer983::Instance983() ? CUiPointer983::Instance983()->GetButtons983() : NULL)->Get983();
	unsigned int released = buttons & 2;
	if (released)
		m_bDrag = false;
	if (m_bLocked || !m_pShow.get())
		return false;
	if (!CUiPointer983::Instance983() || !CUiPointer983::Instance983()->GetButtons983() || !g_pF02340_983 || !FRESHMGR983())
		return false;
	CLobbyState983* pState = CLobbyState983::Instance983();
	if (pState && (pState->IsGroupA983() || pState->IsGroupB983()) && !pState->IsMain983())
		return false;

	WVector mouse = *CUiPointer983::Instance983()->GetPos983();
	WVector delta = CInputManager::Instance()->GetMouseDelta();
	if (CUiPointer983::Instance983()->IsOverWnd983(mouse.x, mouse.y))
	{
		if (!CLobbyState983::Instance983()->m_bOverPet)
			CUiPointer983::Instance983()->SetCursor983(0, 0);
		return false;
	}
	if (!m_bDrag && !IsMouseOverPet983(mouse, m_pos))
	{
		if (!CLobbyState983::Instance983()->m_bOverPet)
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
	// clique: volta ao movimento de espera, no lugar
	if ((buttons & 4) && m_pShow->SetMotion("\xb1\xe2\xba\xbb\xc0\xda\xbc\xbc", true))
	{
		m_pShow->SetPosition(m_pos);
		m_pShow->m_bShow = true;
		m_pShow->m_bPlaying = true;
	}
	CUiPointer983::Instance983()->SetCursor983(1, 1);
	return true;
}

// 00566f30: avisa o servidor do personagem em uso (0 = o do documento); chamado também de 005d2e15 e 005f71ca
void __fastcall SendLobbyChar983(unsigned long id)
{
	if (id == 0)
		id = DOCCHARID983();
	CLobbyState983* pState = CLobbyState983::Instance983();
	if (!pState)
		return;
	if (pState->m_state == 2 || pState->m_state == 3 || pState->m_state == 4 || pState->m_state == 5 || pState->m_state == 6
		|| pState->m_state == 7)
	{
		WSendPacket packet((enumClientPacket)0x20);
		packet.Encode1(5);
		packet.Encode4(id);
		packet.Send((eSendTo)0);
	}
	else
	{
		int type = 0xb;
		if ((short)::Doc()->m_roomInfo.roomGuid != -1 && !Fn500480_983(::Doc()->m_roomInfo.gameType))
			type = 0xc;
		WSendPacket packet(type);
		packet.Encode1(4);
		packet.Encode4(id);
		packet.Send((eSendTo)0);
	}
}

// avisa a tela "Lobby" se o movimento do boneco acabou (param1 = 1) ou se é um movimento que não termina em "\xb3\xa1"
bool LobbyPlayer::MotionEnd983()
{
	IDummyPlayer* pPlayer = m_pShow.get();
	if (!pPlayer || !m_tid)
		return false;
	int bEnd = 1;
	if (pPlayer->m_bPlaying)
	{
		const char* name = pPlayer->m_pMotion->name;
		if (_stricmp("\xb3\xa1", name + strlen(name) - 2) != 0)
			bEnd = 0;
	}
	AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x195, bEnd, 0, 0, 0, 0);
	return true;
}

// no fim de um movimento ("…\xb3\xa1"), sorteia o próximo da tabela do personagem (se for de conjunto, só com a peça vestida)
bool LobbyPlayer::RandomMotion983()
{
	if (!m_bRandomMotion || !m_pShow.get() || !m_pShow->m_pFrame.get() || !m_tid)
		return false;
	const char* name = m_pShow->m_pMotion->name;
	if (_stricmp("\xb3\xa1", name + strlen(name) - 2) == 0)
	{
		unsigned long tid = m_tid & 0x3ffffff;
		int count = LOBBYMOTION983()->Count983(&tid);
		int index = rand() % count;
		unsigned long tid2 = m_tid & 0x3ffffff;
		const char* motion = LOBBYMOTION983()->Get983(&tid2, &index);
		int type = 5;
		if (CreateCharacterOption::Instance()->IsSetMotion983(motion, &type))
		{
			if (!CreateCharacterOption::Instance()->FindEquipped983(m_tid, ::Doc()->GetMyPartTidList().m_tid, type))
				return true;
		}
		if (m_pShow->m_pFrame->FindMotionData(motion, true, NULL))
			m_pShow->SetMotionData983(motion);
	}
	return true;
}

// trabalho 0: boneco do personagem `id` do jogador (-1 ou 0 = nenhum) ou, sem ele, do tipo `tid` com as peças padrão; com o
// byte CSharedDoc+0x10d30 ligado, um personagem sorteado com as peças do roteiro do pódio
bool LobbyPlayer::Load983(unsigned long id, unsigned long tid)
{
	if (IsLoading983())
		return false;
	IDummyPlayer* pPlayer = new IDummyPlayer(2);
	if (!pPlayer)
		return false;
	{
		std::tr1::shared_ptr<CPetFrame> pFrame((CPetFrame*)NULL);
		pPlayer->m_pFrame = pFrame;
	}
	do
	{
	if (::Doc()->m_offlinePlay == 0)
	{
		if (id == (unsigned long)-1)
			id = 0;
		sCharacterInfo* pInfo = ::Doc()->FindChar983(id);
		if (pInfo)
		{
			pPlayer->m_charTid = pInfo->tid & 0x3ffffff;
			pPlayer->m_name.assign(MakeStr("LOBBY_PLAYER_%d_%d", DOCLOBBYJP()->GetUid983(), pPlayer->m_charTid));
			if (!IDummyPlayer::Load983(pPlayer, pInfo->tidParts, pInfo->tidAuxParts, pInfo->ItemIdList, pInfo->hairClr,
					*((unsigned char*)pInfo + 9), true, 1))
				break;
			m_csPet.lock();
			m_loadId = pInfo->guid;
			m_loadTid = pInfo->tid;
			m_csPet.unlock();
		}
		else
		{
			if (!tid)
				break;
			pPlayer->m_charTid = tid & 0x3ffffff;
			pPlayer->m_name = MakeStr("LOBBY_PLAYER_%d_%d", DOCLOBBYJP()->GetUid983(), pPlayer->m_charTid);
			::Doc()->BuildMyPartTidListDefault(tid);
			CSharedDoc* pDoc = ::Doc();
			if (!IDummyPlayer::Build983(pPlayer, &pDoc->GetMyPartTidList(), pDoc->GetMyAuxPartTidList(), true))
				break;
			m_csPet.lock();
			m_loadId = 0;
			m_loadTid = tid;
			m_csPet.unlock();
		}
	}
	else
	{
		unsigned long tids[24];
		memset(tids, 0, sizeof(tids));
		unsigned long uids[5] = { 0, 0, 0, 0, 0 };
		unsigned long uids2[5] = { 0, 0, 0, 0, 0 };
		pPlayer->m_charTid = rand() % 15;
		IDummyPlayer::ReadDefault983(pPlayer, NULL, tids, uids);
		pPlayer->m_name.assign(MakeStr("LOBBY_PLAYER_%d_%d", DOCLOBBYJP()->GetUid983(), pPlayer->m_charTid));
		if (!IDummyPlayer::Load983(pPlayer, tids, uids, uids2, 0, 0, false, 1))
			break;
		m_csPet.lock();
		m_loadId = 0;
		m_loadTid = pPlayer->m_charTid | 0x4000000;
		m_csPet.unlock();
	}
	CCsGuard983 lock(m_csPet);
	m_matrix = pPlayer->m_matrix;
	{
		std::tr1::shared_ptr<IDummyPlayer> pNew(pPlayer);
		m_pLoad = pNew;
	}
	m_bDone[0] = true;
	return true;
	} while (0);
	delete pPlayer;
	return false;
}

// trabalho 1: boneco do tipo `tid` — com as peças do jogador, se ele tiver o personagem; senão com as padrão
bool LobbyPlayer::LoadDefault983(unsigned long tid)
{
	if (!tid)
		return false;
	if (IsLoading983())
		return false;
	IDummyPlayer* pPlayer = new IDummyPlayer(2);
	if (!pPlayer)
		return false;
	{
		std::tr1::shared_ptr<CPetFrame> pFrame((CPetFrame*)NULL);
		pPlayer->m_pFrame = pFrame;
	}
	unsigned long id = DOCLOBBYJP()->FindCharIdByTid983(tid);
	if (id == (unsigned long)-1)
		id = 0;
	do
	{
	sCharacterInfo* pInfo = ::Doc()->FindChar983(id);
	if (pInfo)
	{
		pPlayer->m_charTid = pInfo->tid & 0x3ffffff;
		pPlayer->m_name.assign(MakeStr("LOBBY_PLAYER_%d_%d", DOCLOBBYJP()->GetUid983(), pPlayer->m_charTid));
		if (!IDummyPlayer::Load983(pPlayer, pInfo->tidParts, pInfo->tidAuxParts, pInfo->ItemIdList, pInfo->hairClr,
				*((unsigned char*)pInfo + 9), true, 1))
			break;
		m_csPet.lock();
		m_loadId = pInfo->guid;
		m_loadTid = pInfo->tid;
		m_csPet.unlock();
	}
	else
	{
		pPlayer->m_charTid = tid & 0x3ffffff;
		pPlayer->m_name.assign(MakeStr("LOBBY_PLAYER_%d_%d", DOCLOBBYJP()->GetUid983(), pPlayer->m_charTid));
		::Doc()->BuildMyPartTidListDefault(tid);
		CSharedDoc* pDoc = ::Doc();
		if (!IDummyPlayer::Build983(pPlayer, &pDoc->GetMyPartTidList(), pDoc->GetMyAuxPartTidList(), true))
			break;
		m_csPet.lock();
		m_loadTid = tid;
		m_loadId = id;
		m_csPet.unlock();
	}
	CCsGuard983 lock(m_csPet);
	{
		std::tr1::shared_ptr<IDummyPlayer> pNew(pPlayer);
		m_pLoad = pNew;
	}
	m_bDone[1] = true;
	return true;
	} while (0);
	delete pPlayer;
	return false;
}

// tira o boneco da cena e espera a linha do trabalho 0
void LobbyPlayer::Unload983()
{
	if (m_pShow.get())
	{
		CSceneManagerJp::Instance983()->DeleteElement(m_pShow->m_name.c_str());
		m_pShow.reset();
		std::tr1::shared_ptr<IDummyPlayer> pNone((IDummyPlayer*)NULL);
		m_pShow = pNone;
	}
	m_id = 0;
	m_tid = 0;
	m_bPlace[0] = false;
	m_loads[0].m_bLoaded = true;
	if (m_hands[0].IsWorking(0))
		m_hands[0].WaitUntilWorkDone((unsigned long)-1);
	g_resrcmng->UnlockAllByThread(m_hands[0].GetThreadID());
}

// idem para o trabalho 1; devolve a lista de peças do documento ao personagem em uso
void LobbyPlayer::UnloadDefault983()
{
	if (m_pShow.get())
	{
		CSceneManagerJp::Instance983()->DeleteElement(m_pShow->m_name.c_str());
		m_pShow.reset();
		std::tr1::shared_ptr<IDummyPlayer> pNone((IDummyPlayer*)NULL);
		m_pShow = pNone;
	}
	if (m_tid)
	{
		::Doc()->BuildMyPartTidListDefault(m_tid);
		::Doc()->BuildMyPartTidList();
	}
	m_id = 0;
	m_tid = 0;
	m_bPlace[1] = false;
	m_loads[1].m_bLoaded = true;
	if (m_hands[1].IsWorking(0))
		m_hands[1].WaitUntilWorkDone((unsigned long)-1);
	g_resrcmng->UnlockAllByThread(m_hands[1].GetThreadID());
}

bool LobbyPlayer::LoadByWork983(int work, unsigned long id, unsigned long tid)
{
	bool ret = false;
	switch (work)
	{
	case 0:
		ret = Load983(id, tid);
		break;
	case 1:
		ret = LoadDefault983(tid);
		break;
	}
	return ret;
}

// na linha principal: o boneco que a outra linha acabou de carregar passa a ser o da cena
void LobbyPlayer::Promote983()
{
	if (IsLoading983() == true)
	{
		if (m_pShow.get())
			CSceneManagerJp::Instance983()->DeleteElement(m_pShow->m_name.c_str());
		CCsGuard983 lock(m_csPet);
		m_pShow = std::move(m_pLoad);
		{
			std::tr1::shared_ptr<IDummyPlayer> pNone((IDummyPlayer*)NULL);
			m_pLoad = pNone;
		}
		m_tid = m_loadTid;
		m_loadTid = 0;
		m_id = m_loadId;
		m_loadId = 0;
		for (int i = 0; i < 2; ++i)
		{
			m_bPlace[i] = m_bDone[i];
			m_bDone[i] = false;
		}
	}
}

void LobbyPlayer::OnProcess(float delta)
{
	for (int i = 0; i < 2; ++i)
	{
		if (m_loads[i].m_bReady == true)
		{
			m_loads[i].m_bReady = false;
			if (!m_hands[i].IsWorking(0))
				m_hands[i].Work(this, 0, NULL, true, -1);
		}
	}
	Promote983();
	if (m_pShow.get() && m_pShow->m_pFrame.get())
	{
		if (m_bPlace[0])
		{
			Place983();
			m_bPlace[0] = false;
		}
		else if (m_bPlace[1])
		{
			Place983();
			m_bPlace[1] = false;
		}
	}
	if (m_pShow.get())
	{
		m_pShow->Process(delta);
		MotionEnd983();
		RandomMotion983();
		bool bOver = Input983(delta);
		if (CLobbyState983::Instance983())
			CLobbyState983::Instance983()->m_bCursorBusy = bOver;
	}
	if (m_bFading)
		Fade983(delta);
}

// corpo de um trabalho de carga, entre os avisos 0x53 e 0x54 à tela "Lobby"
void LobbyPlayer::LoadWork983(int work)
{
	LOBBYMSG983(0x53);
	unsigned long tid = m_loads[work].m_tid;
	unsigned long id = m_loads[work].m_id;
	m_work = work;
	LoadByWork983(work, id, tid);
	m_loads[work].m_id = (unsigned long)-1;
	m_loads[work].m_tid = 0;
	LOBBYMSG983(0x54);
}

bool LobbyPlayer::LoadProc0_983()
{
	if (m_loads[0].m_bLoaded == true)
		return false;
	LoadWork983(0);
	Sleep(15);
	LOBBYMSG983(0x57);
	return true;
}

bool LobbyPlayer::LoadProc1_983()
{
	if (m_loads[1].m_bLoaded == true)
		return false;
	LoadWork983(1);
	Sleep(15);
	LOBBYMSG983(0x57);
	return true;
}

void LobbyPlayer::OnPreLoadInit()
{
	_util::WMemFun983<LobbyPlayer, bool> fun;
	m_loads[0].m_loader = *_util::InitMemFun983(&fun, this, &LobbyPlayer::LoadProc0_983);
	m_loads[0].m_bLoaded = false;
	m_loads[0].m_bReady = false;
	m_loads[0].m_id = (unsigned long)-1;
	m_loads[0].m_tid = 0;
	_util::WMemFun983<LobbyPlayer, bool> fun1;
	m_loads[1].m_loader = *_util::InitMemFun983(&fun1, this, &LobbyPlayer::LoadProc1_983);
	m_loads[1].m_bLoaded = false;
	m_loads[1].m_bReady = false;
	m_loads[1].m_tid = 0;
	m_loads[1].m_id = (unsigned long)-1;
}

void LobbyPlayer::OnDestroy()
{
	Unload983();
	m_pos = g_vZero983_f041c4;
	m_cache1.clear();
	m_cache2.clear();
}

void LobbyPlayer::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case 0x85:
		{
			bool bNone = msg.param1 == 0;
			Unload983();
			HandleMsg(MsgObject(NULL, 0x185, 0, 0, !bNone, 0, 0));
		}
		break;
	case 0x184:
		OnDestroy();
		break;
	case 0x185: // mostra o personagem param1 (id) do jogador ou o tipo param2; param3 = sem efeito de aparecer
		{
			if (WisEqual(m_pos, g_vZero983_f041c4, 0.00001f))
				SetPoint983("\xbd\xc3\xc0\xdb\xc1\xa1");
			unsigned long id = msg.param1 ? *(unsigned long*)msg.param1 : 0;
			unsigned long tid = msg.param2 ? *(unsigned long*)msg.param2 : 0;
			bool bAppear = msg.param3 == 0;
			if (id == (unsigned long)-1)
				id = 0;
			if (id == 0 && tid == 0)
				id = DOCCHARID983();
			if (m_id != id)
			{
				UnloadDefault983();
				m_bAppear = bAppear;
				Request983(0, tid, id);
			}
		}
		break;
	case 0x186:
		if (m_pShow.get())
		{
			m_pShow->m_bShow = false;
			m_pShow->m_bPlaying = false;
		}
		Unload983();
		break;
	case 0x187: // mostra o personagem padrão do tipo param1
		{
			unsigned long tid = msg.param1 ? *(unsigned long*)msg.param1 : 0;
			bool bAppear = msg.param2 == 0;
			if (m_tid != tid)
			{
				if (tid == 0)
					tid = m_tid;
				if (m_tid)
					UnloadDefault983();
				m_bAppear = bAppear;
				Request983(1, tid, 0);
			}
		}
		break;
	case 0x188:
		UnloadDefault983();
		break;
	case 0x189:
		UnloadDefault983();
		m_cache2.clear();
		break;
	case 0x192: // troca o personagem em uso por param1 e avisa o servidor
		{
			unsigned long id = msg.param1;
			HandleMsg(MsgObject(NULL, 0x186, 0, 0, 0, 0, 0));
			DOCCHARID983() = id;
			HandleMsg(MsgObject(NULL, 0x185, 0, 0, 0, 0, 0));
			SendLobbyChar983(0);
			LOBBYMSG983(0x1a0);
		}
		break;
	case 0x18a:
		{
			CPartTidList* pList = (CPartTidList*)msg.param1;
			unsigned long hairTid = msg.param2 ? *(unsigned long*)msg.param2 : 0;
			unsigned long a = msg.param3 ? *(unsigned long*)msg.param3 : 0;
			bool bHair = msg.param5 ? *(bool*)msg.param5 : true;
			int* pRet = NULL;
			if (msg.param4)
				pRet = (int*)msg.param4;
			int ret = ChangeParts983(pList, hairTid, a, bHair);
			if (pRet)
				*pRet = ret;
		}
		break;
	case 0x18b:
		{
			unsigned long* pTid = (unsigned long*)msg.param1;
			int* pRet = NULL;
			if (msg.param3)
				pRet = (int*)msg.param3;
			unsigned long tid = msg.param2 ? *pTid : 0;
			bool ret = SetAuxPart983((unsigned char)((*pTid >> 21) & 0x1f), tid);
			if (pRet)
				*pRet = ret;
		}
		break;
	case 0x18c:
		{
			unsigned long hairTid = msg.param3 ? *(unsigned long*)msg.param3 : 0;
			int* pRet = NULL;
			if (msg.param4)
				pRet = (int*)msg.param4;
			bool ret = SetHair983((CPartTidList*)msg.param1, *(unsigned char*)msg.param2, hairTid);
			if (pRet)
				*pRet = ret;
		}
		break;
	case 0x18d:
		{
			int* pRet = NULL;
			if (msg.param3)
				pRet = (int*)msg.param3;
			bool ret = SetUcc983(msg.param1, *(unsigned long*)msg.param2);
			if (pRet)
				*pRet = ret;
		}
		break;
	case 0x1ce:
	case 0x1cf:
		if (m_pShow.get() && m_pShow->m_pFrame.get())
			m_pShow->m_pFrame->SetAlpha(0xff);
		break;
	case 0x198: // devolve o CPetFrame do boneco em param1
		if (m_pShow.get() && m_pShow->m_pFrame.get())
			const_cast<MsgObject&>(msg).param1 = (int)m_pShow->m_pFrame.get();
		break;
	case 0x199: // toca o movimento de nome param1
		if (msg.param1 && m_pShow.get())
		{
			std::string name((const char*)msg.param1);
			m_pShow->SetMotionData983(name.c_str());
			m_pShow->m_bPlaying = true;
			m_motionParam = msg.param2;
			m_bRandomMotion = msg.param3 != 0;
		}
		break;
	case 0x19a: // volta ao movimento de espera
		if (m_pShow.get())
		{
			std::string name("\xb1\xe2\xba\xbb\xc0\xda\xbc\xbc");
			if (m_pShow->m_pMotion->name != name)
			{
				m_pShow->SetMotionData983(name.c_str());
				m_pShow->m_bPlaying = true;
			}
		}
		break;
	case 0x190:
		if (msg.param1)
			*(unsigned long*)msg.param1 = m_tid;
		break;
	case 0x194:
		if (msg.param1)
			*(sLobbyRect983*)msg.param1 = GetScreenRect983();
		break;
	case 0x2ae:
		m_bLocked = true;
		break;
	case 0x2af:
		m_bLocked = false;
		break;
	}
}

// layout medido no 983: `new` de 0x438 em 00569333; construtor 005690c0 (vetores de 2 × 0x28 em +0x70 e de 2 CAnotherHand em
// +0xc0, matriz em +0x3dc, seção crítica em +0x418)
struct sLobbyPlayerLayout983
{
	C_ASSERT_VC10(sizeof(sLobbyPlayerLoad983) == 0x28);
	C_ASSERT(sizeof(CCriticalSection983) == 0x1c);
	C_ASSERT_VC10(sizeof(LobbyPlayer) == 0x438);
	C_ASSERT(offsetof(LobbyPlayer, m_pShow) == 0x24);
	C_ASSERT_VC10(offsetof(LobbyPlayer, m_cache2) == 0x3c);
	C_ASSERT_VC10(offsetof(LobbyPlayer, m_pos) == 0x5c);
	C_ASSERT_VC10(offsetof(LobbyPlayer, m_loads) == 0x70);
	C_ASSERT_VC10(offsetof(LobbyPlayer, m_hands) == 0xc0);
	C_ASSERT_VC10(offsetof(LobbyPlayer, m_fadeTime) == 0x3d0);
	C_ASSERT_VC10(offsetof(LobbyPlayer, m_matrix) == 0x3dc);
	C_ASSERT_VC10(offsetof(LobbyPlayer, m_40c) == 0x40c);
	C_ASSERT_VC10(offsetof(LobbyPlayer, m_csPet) == 0x418);
	C_ASSERT_VC10(offsetof(LobbyPlayer, m_tid) == 0x58);
	C_ASSERT_VC10(offsetof(LobbyPlayer, m_bPlace) == 0x6a);
	C_ASSERT_VC10(offsetof(sLobbyPlayerLoad983, m_bLoaded) == 0x18);
	C_ASSERT_VC10(offsetof(sLobbyPlayerLoad983, m_id) == 0x20);
	C_ASSERT_VC10(offsetof(LobbyPlayer, m_work) == 0x3d8);
	C_ASSERT_VC10(offsetof(LobbyPlayer, m_bAppear) == 0x40d);
	C_ASSERT_VC10(offsetof(LobbyPlayer, m_bRandomMotion) == 0x414);
	C_ASSERT_VC10(offsetof(LobbyPlayer, m_bDrag) == 0x416);
	C_ASSERT_VC10(offsetof(CLobbyState983, m_bChanged) == 0x258); // D870: cl novo 0x240 (= RFLobbyDesktop, assert em lobbymain.cpp)
};
