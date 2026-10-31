// JP 983: LobbyAvatarActor INTEIRA (unidade 0058c3f0–00592fa0; sessão B39): construtor 00591820, destrutor 00590da0, criador
// 00592570, registro 005925e0, as 8 virtuais (OnPreLoadInit 0058d120, OnLoad 0058c3f0, OnInit 0058c400, OnProcess 005928e0,
// OnDisplay 0058e610, OnDestroy 00592610 → 00592030, HandleMsg 00592620, OnAnotherHand 00592f50), 17 auxiliares e as quatro
// estruturas de apoio (posição 0058c480…, passeio do mascote 0058d390…, faixa do título 0058dfc0…, botão de esconder
// 0058c5d0…). Fonte reconstruída do 983 (classe sem par no 645; par no KR 839).
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "lobbyavataractor.h"
#include "shareddoc.h"
#include "contentsdoc.h"
#include "taskmanager.h"
#include "inputmanager.h"
#include "wresrcmng.h"
#include "woverlay.h"
#include "petframe.h"
#include "parttidlist.h"
#include "packet.h"
#include "ucclibrary.h"
#include "extraresource.h"
#include "s5/utilities.h"
#include "../../shared/localize.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "../../shared/LuaSystem/luascriptmanager.h"
#include "../../shared/LuaSystem/luascript.h"
#include "../../shared/LuaSystem/luatablehelper.h"
#include <cmath>
#include "createcharacteroption.h"
#include "mousecursor.h"
#include "fremoticon.h"

extern Fresh* g_pFresh; // 00f02340
extern WView* g_pMainView983; // 00f02458
extern const _WRECT g_rectUnit983_e4296c; // 00e4296c (retângulo de origem padrão)
extern const char* g_lobbyBgLua983; // 00e0cff0 -> "lobby_bg.lua"
extern const char* g_lobbyAvatarTable983; // 00e0cff4 -> "lobby_avatar_table"
#include "engine/wlocalize.h" // K2L_Compatibility 00b4ec60

#define VIEWW983() (*(float*)((char*)g_pMainView983 + VC10_OFS_BO(0x19c)))
#define VIEWH983() (*(float*)((char*)g_pMainView983 + VC10_OFS_BO(0x1a0)))
#define MOUSEX983() (*(float*)((char*)g_input + VC10_OFS_BO(0x54)))
#define MOUSEY983() (*(float*)((char*)g_input + VC10_OFS_BO(0x58)))
// bytes "a interface atende o mouse" (Fresh+0x44 e ponteiro da interface+0x40)
// D885: era (bool*)g_pFresh + 0x44 cru (VC10 Fresh::show); no cl 14.44 o FrElementDoc encolhe (show 0x34) e o cru caia no filename
C_ASSERT_VC10(offsetof(Fresh, show) == 0x44);
#define FRESHMOUSE983() (g_pFresh->show)
#define UIPTRMOUSE983() (CUiPointer983::Instance983()->m_show40) // D870: era +0x40 cru (VC10)
#define LOBBYAVATARMSG983(to, n, a, b) AfxGetTask()->GetActor(to) << MsgObject(NULL, n, a, b, 0, 0, 0)

// leitura do lua_table_helper fora de linha (0049a000 por índice, 00470f20 float, 00470e60 int) — ver holeinonemissiondlg.cpp
namespace _gateway
{
	struct LuaTable983 : lua_tinker::table
	{
		float GetByIndex(int index);
	};
	inline LuaTable983& AsTable983(const lua_tinker::table& tbl) { return *(LuaTable983*)&tbl; }
	struct LuaHelper983 : lua_system::lua_table_helper
	{
		template <typename T>
		T Get(const char* name);
	};
	// Get<int> não tinha corpo em nenhuma unidade -> stub 0 no exe (max_avatar_num/pet_pos_fix_y = 0)
	template <typename T>
	T LuaHelper983::Get(const char* name)
	{
		return table.get<T>(name);
	}
}
#define LUA983_POS(tbl, name, index) _gateway::AsTable983((tbl).GetValueByName<lua_tinker::table>(name)).GetByIndex(index)
#define LUA983_GET(tbl, type, name) ((_gateway::LuaHelper983&)(tbl)).Get<type>(name)

// documento 0xa0 (frente A): bonecos de outros jogadores a pôr no lobby
class CLobbyAvatarDocJp : public IContentsDataContainer
{
public:
	std::list<sLobbyAvatarInfo983*>* GetList983() { return (std::list<sLobbyAvatarInfo983*>*)((char*)this + 4); } // 00907950
};

// tabela de movimentos de espera por personagem e tabela de conjuntos (as mesmas da LobbyPlayer)
class CLobbyMotionTable983
{
public:
	int Count983(const unsigned long* pTid); // 009a3c80
	const char* Get983(const unsigned long* pTid, const int* pIndex); // 009a3c00
};
#define LOBBYMOTION983() ((CLobbyMotionTable983*)&::Doc()->m_chatManager)
C_ASSERT_VC10(offsetof(CSharedDoc, m_chatManager) == 0x10c24); // A748: era cru Doc()+0x10c24
class CItemManagerLobby2Jp
{
public:
	void* Find160_983(unsigned long tid); // 00a17160
};
// texto com figuras (objeto em FrWndManager+0x90; fremoticon.cpp). 00c09cf0 = FrEmoticon::PrintText11 (alinhamento 0/1/2,
// tokens "(" ")" "()", ícone espelhado com "-"); antes era Print983 sem corpo -> stub: nomes sobre os bonecos nunca saíam
#define FREMOTICON983() (g_pFresh->GetManager()->GetEmoticon())
// 00891000 = CMouseCursor::GetCursor no singleton 00e73e7c (cursor diferente do normal = clique é de outra coisa)
static inline int Fn891000_983() { return CMouseCursor::m_pInstance->GetCursor(); }
void __fastcall Fn0b304b0_983(unsigned long uid, sCharacterInfo* pInfo); // 00b304b0 (corpo NÃO lido)

IMPLEMENT_ACTOR(LobbyAvatarActor, IActor)

// ---- posição de um boneco ----
sLobbyAvatarMove983::sLobbyAvatarMove983(CExhibition983* p)
{
	speed = 1000.0f;
	pExh = NULL;
	state = 0;
	if (p)
	{
		pExh = p;
		sLobbyPos983 v = { 0.0f, 0.0f };
		pos = v;
		target = v;
	}
}

sLobbyAvatarMove983::sLobbyAvatarMove983(CExhibition983* p, sLobbyPos983 v, float speed_)
{
	speed = speed_;
	pExh = NULL;
	state = 0;
	if (p)
	{
		pExh = p;
		pos = v;
		target = v;
	}
}

void sLobbyAvatarMove983::SetPos(sLobbyPos983 v)
{
	pos = v;
	if (pExh)
		pExh->SetCenter(v.x, v.y);
}

void sLobbyAvatarMove983::SetTarget(sLobbyPos983 v)
{
	target = v;
	state = 0;
}

void sLobbyAvatarMove983::Reset()
{
	target = pos;
	state = 0;
}

void sLobbyAvatarMove983::Step(float dt)
{
	if (target.x == pos.x && pExh)
		return;
	bool bLeft = target.x < pos.x;
	if (state == 0)
	{
		state = bLeft ? 1 : 2;
		return;
	}
	if ((state == 2 && bLeft) || (state == 1 && !bLeft))
		pos = target;
	else if (bLeft)
		pos.x += -speed * dt;
	else
		pos.x += speed * dt;
	pExh->SetCenter(pos.x, pos.y);
}

// ---- passeio do mascote ----
sLobbyMascotWalk983::sLobbyMascotWalk983(CExhibition983* p, WVector2D pos, float speed_)
{
	targetAngle = 0.0f;
	angle = 0.0f;
	speed = speed_;
	prevDir = 2;
	if (p)
	{
		pExh = p;
		pMove = new sLobbyAvatarMove983(p, MakePos983(pos.x, pos.y), speed);
		pMove->Reset();
		Pick();
	}
}

sLobbyMascotWalk983::~sLobbyMascotWalk983()
{
	if (pMove)
	{
		delete pMove;
		pMove = NULL;
	}
}

void sLobbyMascotWalk983::SetTarget(const WVector2D* p)
{
	if (!pMove || !pExh)
		return;
	if (p->x == pExh->GetCenter().x)
	{
		wait = 0.0f;
		prevDir = dir;
		dir = 2;
	}
	else
	{
		prevDir = dir;
		dir = pExh->GetCenter().x <= p->x;
	}
	switch (dir)
	{
	case 0:
		targetAngle = 70.0f;
		break;
	case 1:
		targetAngle = -70.0f;
		break;
	case 2:
		targetAngle = 0.0f;
		break;
	}
	tx = p->x;
	ty = p->y;
	pMove->SetTarget(MakePos983(p->x, p->y));
}

void sLobbyMascotWalk983::Pick()
{
	WVector2D v;
	if (rand() % 2)
	{
		v.x = (float)(rand() % 600 + 100);
		v.y = pExh->GetCenter().y;
		SetTarget(&v);
	}
	else
	{
		v.x = pExh->GetCenter().x;
		v.y = pExh->GetCenter().y;
		SetTarget(&v);
	}
}

void sLobbyMascotWalk983::Process(float dt)
{
	if (!pMove || !pExh)
		return;
	int d = dir;
	if (d == 2)
		d = prevDir != 1;
	if (d == 0)
	{
		angle += dt * 300.0;
		if (targetAngle < angle)
			angle = targetAngle;
	}
	else if (d == 1)
	{
		angle -= dt * 300.0;
		if (angle < targetAngle)
			angle = targetAngle;
	}
	pExh->SetDirection(angle * 0.01745329238474369);
	if (dir != 2)
	{
		if (tx == pExh->GetCenter().x)
			Pick();
	}
	if (dir == 2)
	{
		wait += dt;
		if (wait >= 3.0f)
			Pick();
	}
	pMove->Step(dt);
}

// ---- faixa do título ----
sLobbyAvatarTitle983::sLobbyAvatarTitle983(const char* s, const WVector2D* pos)
{
	width = 0;
	bShow = true;
	img[0] = NULL;
	img[1] = NULL;
	img[2] = NULL;
	if (s)
		text.assign(s);
	if (text.size() == 0)
	{
		bShow = false;
		return;
	}
	x = pos->x;
	y = pos->y;
	img[0] = g_resrcmng->GetOverlay("lobby_avatar_title_frame_01.tga", 0);
	img[1] = g_resrcmng->GetOverlay("lobby_avatar_title_frame_02.tga", 0);
	img[2] = g_resrcmng->GetOverlay("lobby_avatar_title_frame_03.tga", 0);
	for (int i = 0; i < 3; ++i)
	{
		if (!img[i])
			return;
	}
	SetText(text.c_str());
}

sLobbyAvatarTitle983::~sLobbyAvatarTitle983()
{
	for (int i = 0; i < 3; ++i)
	{
		if (g_resrcmng && img[i])
		{
			g_resrcmng->Release(img[i]);
			img[i] = NULL;
		}
	}
}

void sLobbyAvatarTitle983::SetText(const char* s)
{
	if (!s)
		return;
	text.assign(s);
	if (text.size() == 0)
	{
		bShow = false;
		return;
	}
	width = (int)g_pFresh->GetManager()->GetGDI()->GetTextExtendOld983(text.c_str());
	int v = 0;
	for (int n = 1; v < width; ++n)
		v = (int)std::pow(2.0f, n);
	width = v;
}

void sLobbyAvatarTitle983::Display()
{
	if (!bShow)
		return;
	for (int i = 0; i < 3; ++i)
	{
		if (!img[i])
			return;
	}
	if (text.size() == 0)
		return;
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	_WRECT src;
	_WRECT dst;
	int h = img[0]->GetHeight();
	int w = img[0]->GetWidth();
	float left = x - (img[2]->GetWidth() + w + width) * 0.5;
	float top = y - h * 0.5;
	src.x = 0.0f;
	src.y = 0.0f;
	src.w = 1.0f;
	src.h = 1.0f;
	dst.x = left;
	dst.y = top;
	dst.w = (float)w;
	dst.h = (float)h;
	img[0]->Render(g_pMainView983, src, dst, 0, 0xffffffff, 0.0f, 0);

	left = x - 0.5 * width;
	dst.x = left;
	dst.y = top;
	dst.w = (float)width;
	dst.h = (float)img[1]->GetHeight();
	src.x = 0.0f;
	src.y = 0.0f;
	src.w = 1.0f;
	src.h = 1.0f;
	img[1]->Render(g_pMainView983, src, dst, 0, 0xffffffff, 0.0f, 0);

	left = width * 0.5 + x;
	dst.x = left;
	dst.y = top;
	dst.w = (float)img[0]->GetWidth();
	dst.h = (float)img[2]->GetHeight();
	src.x = 0.0f;
	src.y = 0.0f;
	src.w = 1.0f;
	src.h = 1.0f;
	img[2]->Render(g_pMainView983, src, dst, 0, 0xffffffff, 0.0f, 0);

	pGDI->SetTextColor(0xffffffff, 0xff000000);
	pGDI->SetTextStyle(2);
	WPoint pt;
	pt.x = x;
	pt.y = y - pGDI->GetFontHeight() * 0.5;
	pGDI->PrintText(pt, 1, text.c_str(), NULL);
}

// ---- botão de esconder a interface ----
sLobbyHideButton983::sLobbyHideButton983(const WVector2D* pos, const char* a, const char* b, const char* c)
{
	state = 0;
	m_04 = 1;
	img[0] = NULL;
	img[1] = NULL;
	img[2] = NULL;
	if (a && b && c)
	{
		img[0] = g_resrcmng->GetOverlay(a, 0);
		img[1] = g_resrcmng->GetOverlay(b, 0);
		img[2] = g_resrcmng->GetOverlay(c, 0);
		for (int i = 0; i < 3; ++i)
		{
			if (!img[i])
				return;
		}
		rect.x = pos->x;
		rect.y = pos->y;
		rect.w = (float)img[0]->GetWidth();
		rect.h = (float)img[0]->GetHeight();
	}
}

sLobbyHideButton983::~sLobbyHideButton983()
{
	for (int i = 0; i < 3; ++i)
	{
		if (g_resrcmng && img[i])
		{
			g_resrcmng->Release(img[i]);
			img[i] = NULL;
		}
	}
	FRESHMOUSE983() = true;
	if (CUiPointer983::Instance983())
		UIPTRMOUSE983() = true;
}

// com o ponteiro em cima do botão a interface deixa de atender o mouse
void sLobbyHideButton983::Process()
{
	float my = MOUSEY983();
	float mx = MOUSEX983();
	if (rect.x <= mx && mx <= rect.w + rect.x && rect.y <= my && my <= rect.h + rect.y)
	{
		FRESHMOUSE983() = false;
		if (CUiPointer983::Instance983())
			UIPTRMOUSE983() = false;
		state = 1;
		return;
	}
	FRESHMOUSE983() = true;
	if (CUiPointer983::Instance983())
		UIPTRMOUSE983() = true;
	state = 0;
}

void sLobbyHideButton983::Display()
{
	if (state != 1 && img[state])
		img[state]->Render(g_pMainView983, g_rectUnit983_e4296c, rect, 0, 0xffffffff, 0.0f, 0);
}

// ---- pedido ----
sLobbyAvatarInfo983::sLobbyAvatarInfo983()
	: uid(0), name(""), title(""), titleType(0), bOwn(false)
{
}

// ---- o ator ----
LobbyAvatarActor::LobbyAvatarActor()
	: m_bgFade(0), m_pBg(NULL), m_pBgNext(NULL), m_pShadow(NULL), m_pMark(NULL), m_pScript(NULL), m_7c(0), m_uccWait(0),
	  m_pSelected(NULL), m_requestCount(0), m_maxAvatarNum(0), m_pCaddie(NULL), m_pMascot(NULL), m_pMascotWalk(NULL),
	  m_pHideBtn(NULL), m_cs(2000)
{
	m_pBg = NULL;
	m_pBgNext = NULL;
	Init983();
	m_dispPriority = (eDispPriority)4;
}

LobbyAvatarActor::~LobbyAvatarActor()
{
	FRESHMOUSE983() = true;
	if (CUiPointer983::Instance983())
		UIPTRMOUSE983() = true;
}

void LobbyAvatarActor::OnPreLoadInit()
{
	m_bShow = true;
	SetBg983(CExtraResource::Instance()->GetLobby(0));
}

void LobbyAvatarActor::OnLoad()
{
}

void LobbyAvatarActor::OnInit()
{
}

void LobbyAvatarActor::OnDestroy()
{
	Destroy983();
}

// troca o fundo: o primeiro entra direto, os seguintes entram por cima com transparência (OnProcess)
void LobbyAvatarActor::SetBg983(const char* name)
{
	if (m_pBg)
	{
		if (g_resrcmng && m_pBgNext)
		{
			g_resrcmng->Release(m_pBgNext);
			m_pBgNext = NULL;
		}
		m_pBgNext = g_resrcmng->GetOverlay(name, 0);
		m_bgFade = 0.0f;
	}
	else
		m_pBg = g_resrcmng->GetOverlay(name, 0);
}

bool LobbyAvatarActor::IsBusy983()
{
	if (m_avatars.begin() != m_avatars.end())
		return true;
	if (m_requests.size() != 0)
		return true;
	if (m_loaded.size() != 0)
		return true;
	return m_hand.IsWorking(0) != false;
}

// espalha os bonecos pela largura da tela; o último entra a partir do lugar do penúltimo
void LobbyAvatarActor::Layout983()
{
	if (m_avatars.begin() == m_avatars.end())
		return;
	unsigned int n = m_avatars.size();
	unsigned int step = (unsigned long)(float)(VIEWW983() - 120.0) / n;
	if (n == m_moves.size() && n != 0)
	{
		int x = 0x3c - (int)(step * -0.5);
		for (unsigned int i = 0; i < m_avatars.size(); ++i)
		{
			int y = m_petPosFixY < 0 ? 0 : m_petPosFixY;
			if (i != 0 && i == m_moves.size() - 1)
				m_moves[i].SetPos(m_moves[i - 1].pos);
			m_moves[i].SetTarget(MakePos983((float)x, (float)y));
			x += step;
		}
	}
}

const char* LobbyAvatarActor::TitleText983(int type, std::string* s)
{
	switch (type)
	{
	case 1:
		return s->c_str();
	case 2:
		return K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8 " "1\xb5\xee"); // 983 0058de01: texto fixo, nao s
	}
	return "";
}

void LobbyAvatarActor::UpdateFocus983()
{
	bool bFocus = g_pFresh->IsDesktopFocused();
	for (std::vector<sLobbyAvatar983>::iterator it = m_avatars.begin(); it != m_avatars.end(); ++it)
	{
		if (it->pExh)
			it->pExh->m_bFocused = bFocus;
	}
}

// manda o próximo pedido à outra linha (espera até 3 s pelas roupas UCC pedidas ao servidor)
void LobbyAvatarActor::Kick983()
{
	if (m_hand.IsWorking(0))
		return;
	if (m_requests.size() != 0 && (m_uccWait >= 3.0f || m_uccPending.size() == 0))
	{
		sLobbyAvatarInfo983* p = m_requests.front();
		m_requests.pop_front();
		if (p)
			m_hand.Work(this, 0, p, true, -1);
	}
}

// mascote do jogador (ou, sem ele, um sorteado entre os da tabela de itens e os dois de 0x40000001/2)
void LobbyAvatarActor::LoadMascot983()
{
	lua_system::lua_table_helper tbl = m_pScript->GetTable(g_lobbyAvatarTable983);
	float rate = LUA983_GET(tbl, float, "mascot_pet_size_rate");
	float x = m_mascotPos.x;
	float y = m_mascotPos.y;
	float w = m_areaW;
	float h = m_areaH;
	m_pMascot = new CExhibitionByHardwareTnL983(x + w * 0.5, 0.5 * h + y + 15.0, 0.35f, (unsigned long)-1, true);
	m_pMascot->SetDistance(rate * 1.25, true, false);
	m_pMascot->SetArea(w, h);
	unsigned long tid;
	sMascotInfo* pMascot = ::Doc()->GetMyEquippedMascotInfo();
	if (pMascot)
		tid = *((unsigned long*)pMascot + 1);
	else
	{
		std::vector<unsigned long> tids;
		// D826: o 983 (0058fb01) le [Doc+0x340] = _Myhead do m_MascotMap (Doc+0x33c) e testa nó+0x90 = c.InStock == 1.
		// Antes: Doc+0x340 lido como o proprio mapa -> _Myhead vinha do _Mysize (AV em begin() sem mascote equipado).
		std::map<unsigned int, IFF_STRUCT::sMascot>& items = ::Doc()->m_itemManager.m_MascotMap;
		for (std::map<unsigned int, IFF_STRUCT::sMascot>::iterator it = items.begin(); it != items.end(); ++it)
		{
			if (it->second.c.InStock == 1)
				tids.push_back(it->second.c.TypeId);
		}
		tids.push_back(0x40000001);
		tids.push_back(0x40000002);
		tid = tids[(unsigned int)rand() % tids.size()];
	}
	m_pMascot->SetModel(tid);
	m_pMascot->SetCenter(m_mascotPos.x, m_mascotPos.y);
	m_pMascot->m_b640 = false;
	m_pMascot->m_bFocused = false;
	m_pMascotWalk = new sLobbyMascotWalk983(m_pMascot, m_mascotPos, 70.0f);
}

// aceita um pedido: tira as peças que a tabela de itens não conhece, pede ao servidor as roupas UCC e põe na fila
void LobbyAvatarActor::Request983(sLobbyAvatarInfo983* p)
{
	if (p && m_requestCount < m_maxAvatarNum && p->info.tid != 0)
	{
		for (int i = 0; i < 24; ++i)
		{
			if (p->info.tidParts[i] && !((CItemManagerLobby2Jp*)&::Doc()->m_itemManager)->Find160_983(p->info.tidParts[i]))
				p->info.tidParts[i] = 0;
			if (IsLocalContent((localContentType_t)0x58) && p->info.tidParts[i] && p->info.ItemIdList[i])
			{
				if (IsUccClothes(p->info.tidParts[i]))
				{
					if (strlen(p->info.UccIndexList[i]) != 0)
					{
						m_uccWait = 0.0f;
						m_uccPending.push_back(std::string(p->info.UccIndexList[i]));
						WSendPacket packet((enumClientPacket)0xb9);
						packet.Encode1(1);
						packet.Encode4(p->info.ItemIdList[i]);
						packet.Encode1(0);
						packet.Send((eSendTo)0);
					}
				}
			}
		}
		Fn0b304b0_983(p->uid, &p->info);
		m_requests.push_back(p);
		++m_requestCount;
	}
}

void LobbyAvatarActor::Init983()
{
	m_pScript = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	m_pScript->RunFile(g_lobbyBgLua983);
	lua_system::lua_table_helper tbl = m_pScript->GetTable(g_lobbyAvatarTable983);
	m_areaW = LUA983_POS(tbl, "area_size", 1);
	m_areaH = LUA983_POS(tbl, "area_size", 2);
	m_pShadow = g_resrcmng->GetOverlay("avatar_shadow.tga", 0);
	m_pMark = g_resrcmng->GetOverlay("lobby_avatar_mark.tga", 0);
	m_petSizeRate = LUA983_GET(tbl, float, "pet_size_rate");
	m_petPosFixY = LUA983_GET(tbl, int, "pet_pos_fix_y");
	m_maxAvatarNum = LUA983_GET(tbl, int, "max_avatar_num");
	m_mascotPos.x = LUA983_POS(tbl, "mascot_pos", 1);
	m_mascotPos.y = LUA983_POS(tbl, "mascot_pos", 2);
	m_caddiePos.x = LUA983_POS(tbl, "caddie_pos", 1);
	m_caddiePos.y = LUA983_POS(tbl, "caddie_pos", 2);
	m_avatars.reserve(m_maxAvatarNum);
	m_moves.reserve(m_maxAvatarNum);
	WOverlay* pImg = g_resrcmng->GetOverlay("main_btn_ui_hide_Normal.tga", 0);
	if (pImg)
	{
		WVector2D pos;
		pos.x = VIEWW983() - (float)pImg->GetWidth();
		pos.y = VIEWH983() - (float)pImg->GetHeight();
		m_pHideBtn = new sLobbyHideButton983(&pos, "main_btn_ui_hide_Normal.tga", "main_btn_ui_hide_Hover.tga",
			"main_btn_ui_hide_Hover.tga");
		if (g_resrcmng)
			g_resrcmng->Release(pImg);
	}
}

// pede até `n` personagens do jogador, sorteados entre os de tipo diferente do que está em uso
void LobbyAvatarActor::AddRandomChars983(int n)
{
	if (n <= 0)
		return;
	sCharacterInfo* pMine = ::Doc()->FindChar983((unsigned long)-1);
	int count = ::Doc()->m_charMap.size() - 1;
	unsigned int* order = new unsigned int[count];
	unsigned int* p = order;
	std::map<unsigned int, sCharacterInfo>::iterator it = ::Doc()->m_charMap.begin();
	for (unsigned int i = 0; i < ::Doc()->m_charMap.size(); ++i)
	{
		if (it->second.tid != pMine->tid)
			*p++ = i;
		++it;
	}
	for (int i = 0; i < count; ++i)
	{
		int r = rand() % count;
		unsigned int t = order[i];
		order[i] = order[r];
		order[r] = t;
	}
	if (n > count)
		n = count;
	for (int i = 0; i < n; ++i)
	{
		std::map<unsigned int, sCharacterInfo>::iterator pick = ::Doc()->m_charMap.begin();
		std::advance(pick, (int)order[i]);
		if (pick == ::Doc()->m_charMap.end())
			break;
		sLobbyAvatarInfo983* pInfo = new sLobbyAvatarInfo983;
		pInfo->info = pick->second;
		pInfo->bOwn = true;
		pInfo->uid = DOCLOBBYJP()->GetUid983();
		Request983(pInfo);
	}
	delete[] order;
}

// bonecos de teste com peças sorteadas (ninguém chama no 983)
void LobbyAvatarActor::AddDummies983()
{
	sLobbyAvatarInfo983* p = new sLobbyAvatarInfo983;
	int r = rand();
	p->bOwn = true;
	p->info.tid = r % 15 | 0x4000000;
	p->uid = DOCLOBBYJP()->GetUid983();
	_util::GenerateRandomTidParts(p->info.tid, p->info.tidParts, p->info.tidAuxParts);
	Request983(p);
	p = new sLobbyAvatarInfo983;
	for (int i = 0; i < m_maxAvatarNum; ++i)
	{
		r = rand();
		p->bOwn = true;
		p->info.tid = r % 15 | 0x4000000;
		p->uid = DOCLOBBYJP()->GetUid983();
		_util::GenerateRandomTidParts(p->info.tid, p->info.tidParts, p->info.tidAuxParts);
		Request983(p);
	}
}

// põe em cena os bonecos que a outra linha acabou de carregar
void LobbyAvatarActor::Collect983()
{
	if (m_hand.IsWorking(0))
		return;
	m_hand.Enter();
	if (m_loaded.size() != 0)
	{
		for (std::list<sLobbyAvatar983>::iterator it = m_loaded.begin(); it != m_loaded.end();)
		{
			CExhibition983* pExh = it->pExh;
			pExh->m_b640 = false;
			pExh->m_flags = 0x12;
			m_avatars.push_back(*it);
			sLobbyAvatarMove983 move(pExh);
			float w = VIEWW983();
			move.SetPos(MakePos983(w * 0.5, (float)m_petPosFixY));
			m_moves.push_back(move);
			WVector2D zero(0.0f, 0.0f);
			sLobbyAvatarTitle983* pTitle = new sLobbyAvatarTitle983(it->title.c_str(), &zero);
			m_titles.push_back(pTitle);
			it = m_loaded.erase(it);
		}
		m_loaded.clear();
		Layout983();
	}
	m_hand.Leave();
}

void LobbyAvatarActor::Unload983()
{
	if (m_hand.WaitUntilWorkDone(9999) == true)
		m_hand.BreakWorkingThread();
	g_resrcmng->UnlockAllByThread(m_hand.GetThreadID());
	CLobbyAvatarDocJp* pDoc = NULL;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xa0, pDoc);
	if (m_requests.size() == 0)
		m_requests.clear();
	else
	{
		for (std::list<sLobbyAvatarInfo983*>::iterator it = m_requests.begin(); it != m_requests.end(); ++it)
		{
			sLobbyAvatarInfo983* p = *it;
			if (p && p->bOwn)
				delete p;
		}
	}
	for (unsigned int i = 0; i < m_avatars.size(); ++i)
	{
		if (m_avatars[i].pExh)
		{
			delete m_avatars[i].pExh;
			m_avatars[i].pExh = NULL;
		}
	}
	for (std::list<sLobbyAvatar983>::iterator it = m_loaded.begin(); it != m_loaded.end(); ++it)
	{
		if (it->pExh)
		{
			delete it->pExh;
			it->pExh = NULL;
		}
	}
	for (unsigned int i = 0; i < m_titles.size(); ++i)
	{
		if (m_titles[i])
		{
			delete m_titles[i];
			m_titles[i] = NULL;
		}
	}
	m_avatars.clear();
	m_requests.clear();
	m_loaded.clear();
	m_moves.clear();
	m_uccPending.clear();
	m_titles.clear();
	m_uccWait = 0.0f;
	m_7c = 0;
	m_requestCount = 0;
	m_pSelected = NULL;
	if (m_pMascot)
	{
		delete m_pMascot;
		m_pMascot = NULL;
	}
	if (m_pMascotWalk)
	{
		delete m_pMascotWalk;
		m_pMascotWalk = NULL;
	}
}

void LobbyAvatarActor::Destroy983()
{
	Unload983();
	WOverlay** bg = &m_pBg;
	for (int i = 0; i < 2; ++i)
	{
		if (g_resrcmng && bg[i])
		{
			g_resrcmng->Release(bg[i]);
			bg[i] = NULL;
		}
	}
	if (g_resrcmng && m_pShadow)
	{
		g_resrcmng->Release(m_pShadow);
		m_pShadow = NULL;
	}
	if (g_resrcmng && m_pMark)
	{
		g_resrcmng->Release(m_pMark);
		m_pMark = NULL;
	}
	if (m_pHideBtn)
	{
		delete m_pHideBtn;
		m_pHideBtn = NULL; // 983 005920c4: zera [this+0x258] (0x186 e OnDestroy passam aqui)
	}
	m_bgFade = 0.0f;
	if (m_pScript)
		lua_system::lua_script_manger::GetInstance()->ReleaseScript(m_pScript);
}

// corpo do trabalho da outra linha: monta o boneco do pedido e põe na lista dos carregados
void LobbyAvatarActor::LoadAvatar983(sLobbyAvatarInfo983* p)
{
	if (!p)
		return;
	float w = m_areaW;
	float h = m_areaH;
	CExhibition983* pExh = new CExhibitionByHardwareTnL983(0.0f, 0.0f, 0.35f, p->uid, false);
	if (!pExh)
		return;
	pExh->SetDistance(m_petSizeRate * 1.25, true, false);
	pExh->SetArea(w, h);
	CPartTidList parts;
	if (!parts.SetTids(&p->info, 0xff))
		return;
	if (!parts.IsComboValid())
		parts.SetDefaultTids();
	pExh->SetModel(parts, p->info.tidAuxParts, NULL, 3.0f, m_avatars.size());
	sLobbyAvatar983 avatar;
	avatar.pExh = pExh;
	avatar.name = p->name;
	avatar.title = TitleText983(p->titleType, &p->title);
	{
		CCsGuard983 guard(m_cs);
		m_loaded.push_back(avatar);
		if (p->bOwn)
			delete p;
	}
}

// o personagem em uso e mais alguns do próprio jogador
void LobbyAvatarActor::AddMyChars983()
{
	sCharacterInfo* pMine = ::Doc()->FindChar983((unsigned long)-1);
	if (!pMine)
		return;
	sLobbyAvatarInfo983* p = new sLobbyAvatarInfo983;
	p->info = *pMine;
	p->bOwn = true;
	p->uid = DOCLOBBYJP()->GetUid983();
	Request983(p);
	unsigned int n = m_maxAvatarNum - 1;
	if (::Doc()->m_charMap.size() - 1 < n)
		n = ::Doc()->m_charMap.size() - 1;
	AddRandomChars983(n);
}

// monta a fila de pedidos: o personagem em uso, os do jogador que couberem e os do documento 0xa0; depois o mascote
void LobbyAvatarActor::Start983()
{
	sLobbyAvatarInfo983* p = NULL;
	CLobbyAvatarDocJp* pDoc = NULL;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xa0, pDoc);
	std::list<sLobbyAvatarInfo983*>* pList = pDoc->GetList983();
	if (pList->size() == 0)
		AddMyChars983();
	else
	{
		p = new sLobbyAvatarInfo983;
		p->info = *::Doc()->FindChar983((unsigned long)-1);
		p->bOwn = true;
		p->uid = DOCLOBBYJP()->GetUid983();
		Request983(p);
		if ((int)pList->size() < m_maxAvatarNum - 1)
		{
			unsigned int n = m_maxAvatarNum - pList->size() - 1;
			if (::Doc()->m_charMap.size() - 1 < n)
				n = ::Doc()->m_charMap.size() - 1;
			AddRandomChars983(n);
		}
		for (std::list<sLobbyAvatarInfo983*>::iterator it = pList->begin(); it != pList->end(); ++it)
			Request983(*it);
	}
	LoadMascot983();
}

void LobbyAvatarActor::OnAnotherHand(int key, void* param)
{
	m_hand.Enter();
	if (param)
		LoadAvatar983((sLobbyAvatarInfo983*)param);
	m_hand.Leave();
}

void LobbyAvatarActor::HandleMsg(const MsgObject& msg)
{
	if ((unsigned long)msg.time > (unsigned long)g_CurrentTime)
	{
		StoreDelayedMsg(msg);
		return;
	}
	switch (msg.message)
	{
	case 0x85:
		Unload983();
		Start983();
		break;
	case 0x185:
		if (!IsBusy983())
		{
			Unload983();
			Start983();
		}
		break;
	case 0x186:
		Destroy983();
		break;
	case 0x18f: // chegou a roupa UCC pedida: sai da lista de espera
		{
			const char* id = (const char*)msg.param1 + 4;
			for (std::list<std::string>::iterator it = m_uccPending.begin(); it != m_uccPending.end();)
			{
				if (it->compare(id) == 0)
					it = m_uccPending.erase(it);
				else
					++it;
			}
		}
		break;
	case 0x191:
		if (m_pSelected)
			*(unsigned long*)msg.param1 = m_pSelected->m_tid & 0x3ffffff;
		else
			*(unsigned long*)msg.param1 = (unsigned long)-1;
		break;
	case 0x196:
		m_pSelected->m_bRandomMotion = msg.param1 != 0;
		break;
	case 0x197:
		*(int*)msg.param1 = m_pSelected->m_bRandomMotion;
		break;
	case 0x198:
		if (m_pSelected->m_pPetFrame)
			const_cast<MsgObject&>(msg).param1 = (int)m_pSelected->m_pPetFrame;
		break;
	case 0x199:
		if (m_pSelected)
		{
			if (_stricmp(m_pSelected->GetMotionName(), (const char*)msg.param1) == 0)
			{
				m_pSelected->m_bMotionToggle = !m_pSelected->m_bMotionToggle;
				break;
			}
			m_pSelected->m_bMotionToggle = false;
			m_pSelected->SetMotion((const char*)msg.param1, false, 0.0f);
			m_motionParam = msg.param2;
			m_pSelected->m_bRandomMotion = msg.param3 != 0;
		}
		break;
	case 0x1a1:
		m_pSelected = NULL;
		break;
	case 0x1a2:
		SetBg983(CExtraResource::Instance()->GetLobby(1));
		break;
	case 0x1a3:
		m_bShow = msg.param1 == 1;
		break;
	case 0x1a4:
		*(CExhibition983**)msg.param1 = m_pSelected;
		break;
	}
}

void LobbyAvatarActor::OnDisplay()
{
	_WRECT dst;
	if (m_pBg)
	{
		dst.h = VIEWH983();
		dst.w = VIEWW983();
		dst.x = 0.0f;
		dst.y = 0.0f;
		m_pBg->Render(g_pMainView983, g_rectUnit983_e4296c, dst, 0, 0xffffffff, 0.0f, 0);
	}
	if (m_pBgNext)
	{
		dst.h = VIEWH983();
		dst.w = VIEWW983();
		dst.x = 0.0f;
		dst.y = 0.0f;
		m_pBgNext->Render(g_pMainView983, g_rectUnit983_e4296c, dst, 0, ((int)(m_bgFade * 255.0) << 24) | 0xffffff, 0.0f, 0);
	}
	if (g_pFresh->IsCurrentLayout("TOPPAGE") == false || !m_bShow)
		return;
	std::vector<sLobbyAvatar983>::iterator it;
	for (it = m_avatars.begin(); it != m_avatars.end(); ++it)
	{
		CExhibition983* pExh = it->pExh;
		if (pExh && m_pShadow && m_pMark)
		{
			const WVector2D& c = pExh->GetCenter();
			int markW = m_pMark->GetWidth();
			float cx = c.x;
			dst.x = c.x - m_pShadow->GetWidth() * 0.5;
			dst.y = c.y + 135.0 - 10.0;
			dst.w = (float)m_pShadow->GetWidth();
			dst.h = (float)m_pShadow->GetHeight();
			m_pShadow->Render(g_pMainView983, g_rectUnit983_e4296c, dst, 0, 0xffffffff, 0.0f, 0);
			if (pExh == m_pSelected)
			{
				_WRECT mark;
				mark.y = c.y + 135.0;
				mark.w = (float)m_pMark->GetWidth();
				mark.h = (float)m_pMark->GetHeight();
				mark.x = cx - markW * 0.5;
				m_pMark->Render(g_pMainView983, g_rectUnit983_e4296c, mark, 0, 0xffffffff, 0.0f, 0);
			}
		}
	}
	for (it = m_avatars.begin(); it != m_avatars.end(); ++it)
	{
		if (it->pExh)
			it->pExh->Display(0.0f, 0.0f);
	}
	g_pMainView983->Clear(0, 8);
	if (m_requests.size() == 0)
	{
		if (m_pMascot)
			m_pMascot->Display(0.0f, 0.0f);
		if (m_pCaddie)
			m_pCaddie->Display(0.0f, 0.0f);
	}
	FrEmoticon* pEmo = FREMOTICON983();
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	for (unsigned int i = 0; i < m_avatars.size(); ++i)
	{
		CExhibition983* pExh = m_avatars[i].pExh;
		if (pExh)
		{
			const WVector2D& c = pExh->GetCenter();
			if (m_avatars[i].name.size() != 0 && pEmo)
			{
				pGDI->SetTextColor(0xffffffff, 0xff000000);
				pGDI->SetTextStyle(2);
				WVector2D pos;
				pos.x = c.x;
				pos.y = c.y + 170.0;
				pEmo->PrintText11(*(const WPoint*)&pos, 1, m_avatars[i].name.c_str(), 0xffffffff);
			}
			if (pExh == m_pSelected || pExh->InValidArea())
			{
				sLobbyAvatarTitle983* pTitle = m_titles[i];
				pTitle->x = c.x;
				pTitle->y = c.y + 170.0 + 25.0;
				m_titles[i]->Display();
			}
		}
	}
	if (m_pHideBtn)
		m_pHideBtn->Display();
}

void LobbyAvatarActor::OnProcess(float delta)
{
	if (g_pFresh->IsCurrentLayout("TOPPAGE") == false)
		return;
	UpdateFocus983();
	if ((g_input->Get("LCONTROL", false) || g_input->Get("RCONTROL", false)) && g_input->GetDown("HIDE", false))
	{
		FRESHMOUSE983() = !FRESHMOUSE983();
		UIPTRMOUSE983() = !UIPTRMOUSE983();
	}
	if (m_pHideBtn)
		m_pHideBtn->Process();
	if (m_uccPending.size() != 0)
		m_uccWait += delta;
	std::vector<sLobbyAvatar983>::iterator it;
	for (it = m_avatars.begin(); it != m_avatars.end(); ++it)
	{
		CExhibition983* pExh = it->pExh;
		if (!pExh)
			continue;
		pExh->Process(delta, true);
		if (pExh->m_bRandomMotion)
		{
			// acabou um movimento que termina em "\xb3\xa1": sorteia outro movimento de espera
			const char* name = pExh->GetMotionData()->name;
			size_t len = strlen(name);
			if (_stricmp("\xb3\xa1", pExh->GetMotionData()->name + len - 2) == 0)
			{
				unsigned long tid = pExh->m_tid & 0x3ffffff;
				int index = 0;
				int count = LOBBYMOTION983()->Count983(&tid);
				index = rand() % count;
				unsigned long tid2 = pExh->m_tid & 0x3ffffff;
				const char* motion = LOBBYMOTION983()->Get983(&tid2, &index);
				int type = 5;
				if (!CreateCharacterOption::Instance()->IsSetMotion983(motion, &type) ||
					CreateCharacterOption::Instance()->FindEquipped983(pExh->m_tid, pExh->m_partTids, type))
				{
					if (pExh->m_pPetFrame->FindMotionData(motion, true, NULL))
						pExh->SetMotion(motion, false, 0.0f);
				}
			}
		}
	}
	if (g_pFresh->IsDesktopFocused())
	{
		bool bHit = false;
		for (it = m_avatars.begin(); it != m_avatars.end(); ++it)
		{
			CExhibition983* pExh = it->pExh;
			if (!pExh)
				continue;
			if (pExh->InValidArea() && g_input->GetButton(LEFT_BUTTON) == 1)
			{
				bHit = true;
				if (m_pSelected == pExh)
					continue;
				m_pSelected = pExh;
				LOBBYAVATARMSG983("Lobby", 0x192, pExh->m_tid & 0x3ffffff, 0);
				LOBBYAVATARMSG983("Lobby", 0x19d, 1, 0);
			}
			if (m_pSelected == pExh)
			{
				unsigned long tid = m_pSelected->m_tid & 0x3ffffff;
				int bEnd;
				if (!m_pSelected->m_bMotionToggle)
				{
					const char* name = pExh->GetMotionData()->name;
					size_t len = strlen(name);
					if (_stricmp("\xb3\xa1", pExh->GetMotionData()->name + len - 2) == 0)
						bEnd = 1;
					else
					{
						if (m_pSelected->m_bRandomMotion == true)
							LOBBYAVATARMSG983("LobbyAvatar", 0x196, 1, 0);
						bEnd = 0;
					}
				}
				else
					bEnd = 1;
				LOBBYAVATARMSG983("Lobby", 0x195, bEnd, 0);
				LOBBYAVATARMSG983("Lobby", 0x19c, (int)pExh->GetMotionName(), tid);
			}
		}
		if (!bHit && g_input->GetButton(LEFT_BUTTON) == 1 && !Fn891000_983())
		{
			m_pSelected = NULL;
			LOBBYAVATARMSG983("Lobby", 0x192, -1, 0);
			LOBBYAVATARMSG983("Lobby", 0x19d, 0, 0);
		}
	}
	for (unsigned int i = 0; i < m_moves.size(); ++i)
		m_moves[i].Step(delta);
	if (m_requests.size() == 0)
	{
		if (m_pMascot)
			m_pMascot->Process(delta, true);
		if (m_pCaddie)
			m_pCaddie->Process(delta, true);
		if (m_pMascotWalk)
			m_pMascotWalk->Process(delta);
	}
	if (m_pBgNext)
	{
		m_bgFade = delta + delta + m_bgFade;
		if (m_bgFade > 1.0f)
		{
			m_bgFade = 1.0f;
			if (g_resrcmng && m_pBg)
			{
				g_resrcmng->Release(m_pBg);
				m_pBg = NULL;
			}
			m_pBg = m_pBgNext;
			m_pBgNext = NULL;
		}
	}
	Collect983();
	Kick983();
}

struct sLobbyAvatarLayout983
{
	C_ASSERT(sizeof(CExhibition983) == 0x64c);
	C_ASSERT(offsetof(CExhibition983, m_pPetFrame) == 0xa8);
	C_ASSERT(offsetof(CExhibition983, m_tid) == 0x55c);
	C_ASSERT(offsetof(CExhibition983, m_partTids) == 0x580);
	C_ASSERT(offsetof(CExhibition983, m_bRandomMotion) == 0x643);
	C_ASSERT_VC10(sizeof(sLobbyAvatarInfo983) == 0x248);
	C_ASSERT(offsetof(sLobbyAvatarInfo983, name) == 0x208);
	C_ASSERT_VC10(offsetof(sLobbyAvatarInfo983, titleType) == 0x240);
	C_ASSERT_VC10(sizeof(sLobbyAvatar983) == 0x3c);
	C_ASSERT(sizeof(sLobbyAvatarMove983) == 0x1c);
	C_ASSERT(sizeof(sLobbyMascotWalk983) == 0x28);
	C_ASSERT_VC10(sizeof(sLobbyAvatarTitle983) == 0x38);
	C_ASSERT(sizeof(sLobbyHideButton983) == 0x24);
	C_ASSERT_VC10(sizeof(LobbyAvatarActor) == 0x278);
	C_ASSERT(offsetof(LobbyAvatarActor, m_loaded) == 0x34);
	C_ASSERT_VC10(offsetof(LobbyAvatarActor, m_avatars) == 0x40);
	C_ASSERT_VC10(offsetof(LobbyAvatarActor, m_requests) == 0x60);
	C_ASSERT_VC10(offsetof(LobbyAvatarActor, m_titles) == 0x6c);
	C_ASSERT_VC10(offsetof(LobbyAvatarActor, m_hand) == 0x80);
	C_ASSERT_VC10(offsetof(LobbyAvatarActor, m_pSelected) == 0x208);
	C_ASSERT_VC10(offsetof(LobbyAvatarActor, m_uccPending) == 0x228);
	C_ASSERT_VC10(offsetof(LobbyAvatarActor, m_pMascot) == 0x238);
	C_ASSERT_VC10(offsetof(LobbyAvatarActor, m_pMascotWalk) == 0x24c);
	C_ASSERT_VC10(offsetof(LobbyAvatarActor, m_bShow) == 0x254);
	C_ASSERT_VC10(offsetof(LobbyAvatarActor, m_cs) == 0x25c);
};

// 0049a000 (leitura por indice, float; nil -> 0)
float _gateway::LuaTable983::GetByIndex(int index)
{
	if (!m_obj)
		return 0.0f;
	lua_State* state = m_obj->m_L;
	{
		lua_pushnumber(state, (double)index);
		lua_gettable(state, m_obj->m_index);
		if (lua_type(state, -1) == LUA_TNIL)
			lua_remove(state, -1);
		else
		{
			lua_remove(state, -1);
			lua_rawgeti(state, m_obj->m_index, index);
			return lua_tinker::pop<float>(state);
		}
	}
	lua_pushnil(state);
	return lua_tinker::pop<float>(state);
}
