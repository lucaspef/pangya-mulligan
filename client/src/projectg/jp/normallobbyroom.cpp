// JP 983: sala comum do lobby (`CNormalLobbyRoom`, vtable 00d0c5d0) — a lista "roomlist" da FrMultiplayRoomDlg quando o tipo de
// `CSharedDoc+0x11630` é 0. Os corpos são os `CLobbyMain::OnRoomList_RoomList*` do 645 (src/projectg/lobbymain.cpp) com a
// janela como argumento; o que mudou no 983 vai comentado em cada função. As posições e cores do desenho da linha vêm de
// roomlist_layout_ui.lua (0059dbe0). Mesma unidade do 983 que a lobbyroom.cpp (00598080–0059e260, misturada com pedaços do
// CLobbyMain da frente A). Sessão B50; a `CGrandPrixLobbyRoom` (sessão B51) vem no fim do arquivo.
#include "minatl.h"
#include "projectg.h"
#include "shareddocjp.h"
#include "docofs983.h" // D862: crus do CSharedDoc ancorados no membro (VS 2022)
#include "grandprix983.h"
#include "gamestringmanager.h"
#include "icontentsdoc.h"
#include "taskmanager.h"
#include "netresourcemanager.h"
#include "packet.h"
#include "wfont.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frviewer.h"
#include "frbutton.h"
#include "frlistbox.h"
#include "frstatic.h"
#include "frgraphicinterface.h"
#include "inputmanager.h"
#include "bitmap.h"
#include "contentsdoc.h"
#include "intrusion.h"
// `DeleteScript` é privada no cabeçalho da frente A; o 983 chama de fora da classe (pedido B → A da B50)
#define private public
#include "../../shared/LuaSystem/luascriptmanager.h"
#undef private
#include "../../shared/LuaSystem/luascript.h"
#include "../../shared/LuaSystem/luatablehelper.h"
#include "rftooltiphelper.h"
#include "roomlistdlg.h"
#include "normallobbyroom.h"
#include "gproomtimersteward.h"
#include "grandprixdocument.h"
#include "frtext.h"
#include "titlefontex.h"

extern Fresh* g_pFresh;

#define DOC983_U8(off) (*(unsigned char*)DOCPTR983(off))
#define DOC983_U32(off) (*(unsigned long*)DOCPTR983(off))

int GetCateByGameType(int gameType); // 0059f130 (lobbymain.cpp)

// modos 0x14 e 0x15 (Grand Prix); ver roomlistdlg.cpp
inline bool IsGPMode983(int mode)
{
	switch (mode)
	{
	case 0x14:
	case 0x15:
		return true;
	}
	return false;
}
// sala que aparece na lista comum: sem o bit 0 de gpRoomFlags983 e fora do Grand Prix (realGameType)
#define IS_LISTED_ROOM983(p) (!((p)->gpRoomFlags983 & 1) && !IsGPMode983((p)->realGameType))

// leitura por índice do lua_table_helper do 983 (004d5250 tabela filha, 0049a000 float de uma tabela); as definições fora de
// linha estão em dummyplayeractor.cpp e holeinonemissiondlg.cpp
struct LuaIdx983 : lua_system::lua_table_helper
{
	void Push983(int index);
	template <typename T>
	T GetByIndex(int index);
	lua_system::lua_table_helper GetTableByIndex(int index); // 004d5250
};
__forceinline void LuaIdx983::Push983(int index)
{
	if (table.m_obj)
	{
		lua_pushnumber(state, (double)index);
		lua_gettable(state, table.m_obj->m_index);
		if (lua_type(state, -1) == LUA_TNIL)
		{
			lua_remove(state, -1);
			lua_pushnil(state);
		}
		else
		{
			lua_remove(state, -1);
			lua_rawgeti(state, table.m_obj->m_index, index);
		}
	}
	else
		lua_pushnil(state);
}
template <typename T>
__forceinline T LuaIdx983::GetByIndex(int index)
{
	Push983(index);
	return lua_tinker::pop<T>(state);
}
#define LUAIDX983(tbl) (*(LuaIdx983*)&(tbl))
struct LuaTable983 : lua_tinker::table
{
	float GetByIndex(int index); // 0049a000
};
inline LuaTable983& AsTable983(const lua_tinker::table& tbl) { return *(LuaTable983*)&tbl; }

// 00e75ea0: documento 0xa5 (filtro da lista: tipo de jogo e categoria), entregue pelo construtor do seletor
static sRoomListDoc983* s_pDoc;
// tabelas de roomlist_layout_ui.lua
static std::vector<WPoint> s_offset; // 00e75ea8: 17 posições dentro da linha
static std::vector<WRect> s_baseFrame; // 00e75eb8: os 3 pedaços do fundo da linha
static std::vector<unsigned long> s_color; // 00e75ec8: 6 cores do fundo

static char* s_fontFile[] = { "[font_wind.jpg" }; // 00e0f2d4

// 00598120 (`pDoc` em ecx)
void SetRoomListDoc983(sRoomListDoc983* pDoc)
{
	s_pDoc = pDoc;
}

// 00598200: a sala saiu — tira da lista se ela passava no filtro e completa com linhas vazias até 6
void CNormalLobbyRoom::DelRoom983(FrListBox* pList, int room)
{
	sRoomInfo* pRoom = (sRoomInfo*)room;
	if (pList && pRoom)
	{
		if (s_pDoc->m_gameType != -1)
		{
			if (pRoom->gameType == s_pDoc->m_gameType)
			{
				pList->DelItem(pRoom, sizeof(sRoomInfo));
				int count = pList->GetCurrentItemSize(true);
				for (int i = count; i < 6; i++)
					pList->AddItem(NULL);
			}
		}
		else
		{
			int cate = s_pDoc->m_cate;
			if (GetCateByGameType(pRoom->gameType) == cate || cate == 4 ||
				GetCateByGameType(pRoom->gameType) == 4)
			{
				pList->DelItem(pRoom, sizeof(sRoomInfo));
				int count = pList->GetCurrentItemSize(true);
				for (int i = count; i < 6; i++)
					pList->AddItem(NULL);
			}
		}
	}
}

// 005988a0: a sala chegou — entra na lista se passa no filtro
void CNormalLobbyRoom::AddRoom983(FrListBox* pList, int room)
{
	sRoomInfo* pRoom = (sRoomInfo*)room;
	if (pList)
	{
		if (IS_LISTED_ROOM983(pRoom))
		{
			int gameType = s_pDoc->m_gameType;
			int cate = s_pDoc->m_cate;
			if (gameType != -1 ? pRoom->gameType == gameType
							   : (GetCateByGameType(pRoom->gameType) == cate || cate == 4 ||
									 GetCateByGameType(pRoom->gameType) == 4))
				pList->AddItem(pRoom);
		}
	}
}

// 00598910 (645: OnRoomList_RoomListLDown)
void CNormalLobbyRoom::OnRoomListLDown(FrRoomListDlg* pDlg)
{
	FrListBox* pList = pDlg->m_pRoomList;
	if (pList)
	{
		FrListItem* pItem = pList->GetItemUnderCursor();
		if (pItem)
		{
			pList->SelectItem(pItem, true);
			pDlg->m_pSelRoom = (sRoomInfo*)pItem->pData;
		}
		else
		{
			pList->SelectItem(NULL, true);
			pDlg->m_pSelRoom = NULL;
		}
	}
}

// 00598e70 (645: OnRoomList_RoomListRUp). 983: o pedido de detalhes (pacote 0x2d) não olha mais o tipo do canal nem se a sala
// está aberta
void CNormalLobbyRoom::OnRoomListRUp(FrRoomListDlg* pDlg)
{
	FrListBox* pList = pDlg->m_pRoomList;
	if (pList)
	{
		FrListItem* pItem = pList->GetItemUnderCursor();
		if (pItem)
		{
			sRoomInfo* pRoom = (sRoomInfo*)pItem->pData;
			pList->SelectItem(pItem, true);

			if (pRoom)
			{
				if (!pRoom->bPublic && !(bool)(Doc()->m_myInfo.info.dwIdentity & 4))
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
						(int)K2L_Compatibility("\xba\xf1\xb0\xf8\xb0\xb3 \xb9\xe6\xc0\xba \xb7\xeb\xc1\xa4\xba\xb8\xb8\xa6 \xba\xbc\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9"),
						0, 0, 0, 0));
				}
				else
				{
					WSendPacket packet((enumClientPacket)0x2d);
					packet.Encode2(pRoom->roomGuid);
					packet.Send(TO_GAME);
				}

				pDlg->m_pSelRoom = pRoom;
			}
		}
	}
}

// 00599700
CNormalLobbyRoom::CNormalLobbyRoom()
{
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = s_fontFile;
	info.numPages = 1;
	info.fontw = 16;
	info.fonth = 16;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:,";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
	m_tipItem = -1;
}

// 005997d0
CNormalLobbyRoom::~CNormalLobbyRoom()
{
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

// 0059a250 (645: OnRoomList_RoomListInit). 983: a lista já vem criada pela janela (aqui só é esvaziada e refeita), ficam de
// fora as salas escondidas e as do Grand Prix, e a dica aberta é esquecida
void CNormalLobbyRoom::Init(FrRoomListDlg* pDlg)
{
	FrListBox* pList = pDlg->m_pRoomList;
	if (pList)
	{
		pList->ClearItem();

		if (s_pDoc->m_gameType != -1)
		{
			int gameType = s_pDoc->m_gameType;
			std::list<sRoomInfo>::iterator it;
			for (it = Doc()->m_roomList.begin(); it != Doc()->m_roomList.end(); it++)
			{
				sRoomInfo* pRoom = &(*it);
				if (IS_LISTED_ROOM983(pRoom) && pRoom->gameType == gameType)
					pList->AddItem(pRoom);
			}
		}
		else
		{
			int cate = s_pDoc->m_cate;
			std::list<sRoomInfo>::iterator it;
			for (it = Doc()->m_roomList.begin(); it != Doc()->m_roomList.end(); it++)
			{
				sRoomInfo* pRoom = &(*it);
				if (IS_LISTED_ROOM983(pRoom) &&
					(GetCateByGameType(pRoom->gameType) == cate || cate == 4))
					pList->AddItem(pRoom);
			}
		}

		m_tipItem = -1;
	}
}

// 00502ed0 embutida: o `GetGameTypeName` do 983 tem 0x16 tipos e a tabela em CSharedDoc+0xc8
inline const char* GetGameTypeName983(unsigned char gameType)
{
	if (gameType >= 0x16)
		return NULL;

	return *(const char**)((char*)&Doc()->m_gameTypeInfo + gameType * 8);
}

// 0059f190 (no 645 RoomListGetGameCategory do lobbymain.cpp recebia só o tipo de jogo): o 983 olha o roomFlags983
// (+0xc9, bit 1 = shortgame) nos tipos de torneio e ganhou o hole in one (11); tabela de saltos 0059f1f0/0059f208.
const char* RoomListGetGameCategory983(sRoomInfo* pRoom)
{
	C_ASSERT(offsetof(sRoomInfo, gameType) == 0x58);
	switch (pRoom->gameType)
	{
	case GAME_TYPE_STROKE:
	case GAME_TYPE_TEAM:
		return K2L_Compatibility("\xb4\xeb\xc0\xfc"); // 00d0cd9c

	case GAME_TYPE_AVATARCHAT:
		return K2L_Compatibility("\xb4\xeb\xc8\xad"); // 00d07f84

	case GAME_TYPE_30S:
	case GAME_TYPE_30S_TEAM:
	case GAME_TYPE_GUILD_MATCH:
		if (pRoom->roomFlags983 & 2)
			return K2L_Compatibility("\xbc\xf4\xb0\xd4\xc0\xd3"); // 00d06804
		return K2L_Compatibility("\xb4\xeb\xc8\xb8"); // 00d0680c

	case GAME_TYPE_SKINS:
	case GAME_TYPE_APPROACH:
	case GAME_TYPE_NEW_APPROACH:
		return K2L_Compatibility("\xb9\xe8\xc6\xb2"); // 00d0cd94

	case GAME_TYPE_HOLEINONE983:
		return K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8"); // 00d0ac34

	default:
		return NULL;
	}
}

// evento de pang do mapa: registro devolvido por 0063f830 (CSharedDoc; empréstimo, nome meu) — vale quando +8 é 0 e a taxa
// de +0xc é maior que 0
struct sMapEvent983
{
	char m_00[8];
	unsigned long m_08;
	unsigned short rate; // +0x0c
};
class CSharedDoc983c
{
public:
	sMapEvent983* Fn63f830_983(unsigned long course); // 0063f830
};

// registros das tabelas .iff: só os campos comuns usados aqui (número em +4, nome em +8, ícone em +0x49)
#define IFF983_TYPEID(p) (((IFF_ITEM_COMMON*)(p))->TypeId)
#define IFF983_NAME(p) (((IFF_ITEM_COMMON*)(p))->Name)
#define IFF983_ICON(p) (((IFF_ITEM_COMMON*)(p))->Icon)
// sRoomInfo do 983: artefato da sala (mapa personalizado) em +0xc5, bit 0 de +0xc9 = sala clássica, byte +0xbf = anfitrião
// do Grand Prix (ícone "gp_host")
#define ROOM983_ARTIFACT(p) ((p)->artifactTid983)
#define ROOM983_CLASSIC(p) ((p)->roomFlags983 & 1)
#define ROOM983_GPHOST(p) ((p)->gpHost983)

// soma embutida de dois retângulos (o 983 soma também o 0 da largura e da altura)
inline WRect AddRect983(const WRect& a, const WRect& b)
{
	return WRect(a.x + b.x, a.y + b.y, a.w + b.w, a.h + b.h);
}

// 0059a600 (645: OnRoomList_RoomListOwnerDraw). 983: posições (s_offset), pedaços do fundo (s_baseFrame) e cores (s_color)
// vêm do roomlist_layout_ui.lua; a cor do fundo é pela categoria da sala; entram a luz de estado (verde/amarela/vermelha), o
// cadeado, o mapa personalizado com dica, o ícone do anfitrião do Grand Prix, o hole in one (tipo 0xb) e a sala clássica; a
// contagem de jogadores não depende mais do tipo do canal
void CNormalLobbyRoom::OnRoomListOwnerDraw(int param, FrRoomListDlg* pDlg)
{
	FrListItem* pItem = (FrListItem*)param;

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (!pGDI)
		return;

	sRoomInfo* pRoom = (sRoomInfo*)pItem->pData;

	int state = 0;
	int color = 1;

	if (pRoom)
	{
		color = 0;
		if (pItem->selected)
			state = 2;
		else if (pItem->underCursor)
			state = 1;

		if (GetCateByGameType(pRoom->gameType) == 1)
			color = pRoom->realGameType != GAME_TYPE_USEMAX ? 3 : 2;
		else if (GetCateByGameType(pRoom->gameType) == 4)
			color = 4;
	}

	float x = pItem->pos.x;
	float y = pItem->pos.y;

	pGDI->DrawTexture(pDlg->m_pRoomListBoxBmp[0][state], AddRect983(WRect(x, y, 0.0f, 0.0f), s_baseFrame[0]),
		s_color[color], 0);
	pGDI->DrawTexture(pDlg->m_pRoomListBoxBmp[1][state], AddRect983(WRect(x, y, 0.0f, 0.0f), s_baseFrame[1]),
		s_color[color], 0);
	pGDI->DrawTexture(pDlg->m_pRoomListBoxBmp[2][state], AddRect983(WRect(x, y, 0.0f, 0.0f), s_baseFrame[2]),
		s_color[color], 0);

	if (!pRoom)
		return;

	if (pItem->selected)
	{
		pGDI->DrawTexture(pDlg->m_pRoomListBoxBmp[0][3], AddRect983(WRect(x, y, 0.0f, 0.0f), s_baseFrame[0]),
			0xffffffff, 0);
		pGDI->DrawTexture(pDlg->m_pRoomListBoxBmp[1][3], AddRect983(WRect(x, y, 0.0f, 0.0f), s_baseFrame[1]),
			0xffffffff, 0);
		pGDI->DrawTexture(pDlg->m_pRoomListBoxBmp[2][3], AddRect983(WRect(x, y, 0.0f, 0.0f), s_baseFrame[2]),
			0xffffffff, 0);
	}

	// luz de estado: vermelha = em jogo ou cheia, amarela = aceita entrada no meio, verde = aberta
	const Bitmap* pLight;
	if (pRoom->bAvailable && pRoom->nUserNum < pRoom->nUserLimit)
	{
		if (pRoom->bIntrusion)
			pLight = g_pFresh->GetBitmap("room_light_yellow");
		else
			pLight = g_pFresh->GetBitmap("room_light_green");
	}
	else
		pLight = g_pFresh->GetBitmap("room_light_red");

	FrListBox* pList = pDlg->m_pRoomList;

	if (pRoom->bAvailable && pLight)
	{
		unsigned w = pLight->Width();
		unsigned h = pLight->Height();
		pGDI->DrawTexture(pLight,
			WRect(s_offset[12].x + pItem->pos.x,
				(float)(int)(float)((pList->GetItemHeight() - h) * 0.5) + (s_offset[12].y + pItem->pos.y), (float)w,
				(float)h),
			0xffffffff, 0);
	}

	if (!pRoom->bPublic)
	{
		const Bitmap* pLock = g_pFresh->GetBitmap("room_lock");
		if (pLock)
		{
			unsigned w = pLock->Width();
			unsigned h = pLock->Height();
			pGDI->DrawTexture(pLock,
				WRect(s_offset[7].x + pItem->pos.x - (float)w,
					(float)(int)(float)((pList->GetItemHeight() - h) * 0.5) + (s_offset[7].y + pItem->pos.y), (float)w,
					(float)h),
				0xffffffff, 0);
		}
	}

	// mapa personalizado: etiqueta com dica (nome e descrição do artefato) e o ícone do artefato
	if (ROOM983_ARTIFACT(pRoom))
	{
		void* pArtifact = ItemManager()->FindItem(ROOM983_ARTIFACT(pRoom));
		if (pArtifact && ((*(unsigned long*)((char*)pArtifact + 0xc0) >> 1) & 1))
		{
			const Bitmap* pTag = g_pFresh->GetBitmap("roomlist_custommap_tag");
			if (pTag)
			{
				WRect rect(s_offset[0].x + pItem->pos.x, s_offset[0].y + pItem->pos.y, (float)pTag->Width(),
					(float)pTag->Height());
				bool bOver = false;
				if (pItem->underCursor)
				{
					rect.x = (float)(rect.x - 1.0);
					// posição do cursor: FrWndManager::m_istate.mousePos (+0x18 no VC10)
					if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
					{
						const char* desc = ItemManager()->FindArtifactDesc983(ROOM983_ARTIFACT(pRoom));
						if (!desc)
							desc = "";
						m_tip = MakeStr("<font color=\"#ffaa00\">%s</font>\n%s", IFF983_NAME(pArtifact), desc);
						if (RFTooltipHelper::Instance983())
						{
							RFTooltipHelper::Instance983()->Show983(0, std::string(m_tip.c_str()), 1, 0xb);
							m_tipItem = pItem->idx;
						}
						bOver = true;
					}
				}
				if (!bOver && pItem->idx == m_tipItem && m_tip.size() != 0)
				{
					m_tip.clear();
					if (RFTooltipHelper::Instance983())
					{
						RFTooltipHelper::Instance983()->Hide983();
						m_tipItem = -1;
					}
				}
				pGDI->DrawTexture(pTag, rect, 0xffffffff, 0);
			}

			const Bitmap* pIcon = g_pFresh->GetBitmap(MakeStr("s_%s", IFF983_ICON(pArtifact)));
			if (pIcon)
			{
				WRect rect(s_offset[1].x + pItem->pos.x, pItem->pos.y + s_offset[1].y, (float)pIcon->Width(),
					(float)pIcon->Height());
				if (pItem->underCursor)
					rect.x = (float)(rect.x - 1.0);
				pGDI->DrawTexture(pIcon, rect, 0xffffffff, 0);
			}
		}
	}

	if (pRoom->gameType != GAME_TYPE_HOLEINONE983 && pRoom->bAdminMaster && pRoom->nUserLimit >= 100)
	{
		const Bitmap* pEvent = g_pFresh->GetBitmap("room_event");
		if (pEvent)
			pGDI->DrawTexture(pEvent,
				WRect(s_offset[2].x + pItem->pos.x, pItem->pos.y + s_offset[2].y, (float)pEvent->Width(),
					(float)pEvent->Height()),
				0xffffffff, 0);

		pGDI->SetTextStyle(1);
	}
	else
	{
		const char* text = MakeStr("%03d", pRoom->roomGuid);
		pGDI->PrintTitleFont983(m_pFont, s_offset[3].x + pItem->pos.x, pItem->pos.y + s_offset[3].y,
			text, 0.0f, 0, 1.0f, 0xffffb258);
	}

	pGDI->SetTextColor(0xff000000, 0xffffffff);

	pGDI->PrintText(WPoint(s_offset[4].x + pItem->pos.x, s_offset[4].y + pItem->pos.y), 0,
		RoomListGetGameCategory983(pRoom), NULL);

	switch (pRoom->gameType)
	{
	case GAME_TYPE_GUILD_MATCH:
		if (pRoom->GuildInfo.nGuildID[0])
		{
			const Bitmap* pEmblem =
				NetResourceManager::Instance()->GetEmblemByName(pRoom->GuildInfo.szEmblemName[0]);
			if (pEmblem)
				pGDI->DrawTexture(pEmblem,
					WRect(s_offset[9].x + pItem->pos.x, s_offset[9].y + pItem->pos.y, (float)pEmblem->Width(),
						(float)pEmblem->Height()),
					0xffffffff, 0);
		}

		if (pRoom->GuildInfo.nGuildID[1])
		{
			const Bitmap* pEmblem =
				NetResourceManager::Instance()->GetEmblemByName(pRoom->GuildInfo.szEmblemName[1]);
			if (pEmblem)
				pGDI->DrawTexture(pEmblem,
					WRect(s_offset[10].x + pItem->pos.x, s_offset[10].y + pItem->pos.y, (float)pEmblem->Width(),
						(float)pEmblem->Height()),
					0xffffffff, 0);
		}

		pGDI->Print(WPoint(s_offset[6].x + pItem->pos.x, s_offset[6].y + pItem->pos.y), 0, "%s",
			GetGameTypeName983(pRoom->gameType));
		break;

	case GAME_TYPE_APPROACH:
		pGDI->Print(WPoint(s_offset[6].x + pItem->pos.x, s_offset[6].y + pItem->pos.y), 0, "%s",
			GetGameTypeName983(pRoom->gameType));
		break;

	case GAME_TYPE_HOLEINONE983:
		// hole in one: "상급" (tipo real 0xd) ou "중급"
		pGDI->Print(WPoint(s_offset[6].x + pItem->pos.x, s_offset[6].y + pItem->pos.y), 0, "%s",
			K2L_Compatibility(pRoom->realGameType == 0xd ? "\xbb\xf3\xb1\xde" : "\xc1\xdf\xb1\xde"));
		break;

	default:
	{
		void* pMatch = ItemManager()->FindMatch(pRoom->tidMatch);
		if (pMatch)
		{
			char name[80] = { 0 };

			if (pRoom->realGameType != GAME_TYPE_USEMAX)
				strcpy(name, IFF983_NAME(pMatch));
			else
				strcpy(name, K2L_Compatibility("\xbc\xc5\xc7\xc3\xc4\xda\xbd\xba"));

			pGDI->Print(WPoint(s_offset[6].x + pItem->pos.x, s_offset[6].y + pItem->pos.y), 0, "%s %s", name,
				pRoom->gameType == GAME_TYPE_30S_TEAM ? K2L_Compatibility("\xc6\xc0\xc0\xfc") : "");
		}
		else
		{
			pGDI->Print(WPoint(s_offset[6].x + pItem->pos.x, s_offset[6].y + pItem->pos.y), 0, "%s %s",
				GetGameTypeName(pRoom->gameType),
				pRoom->gameType == GAME_TYPE_30S_TEAM ? K2L_Compatibility("\xc6\xc0\xc0\xfc") : "");
		}
	}
	break;
	}

	if (ROOM983_GPHOST(pRoom))
	{
		const Bitmap* pHost = g_pFresh->GetManager()->GetBitmap("LEVELS", "gp_host");
		if (pHost)
			pGDI->DrawTexture(pHost,
				WRect(s_offset[5].x + pItem->pos.x, s_offset[5].y + pItem->pos.y, (float)pHost->Width(),
					(float)pHost->Height()),
				0xffffffff, 0);
	}

	pGDI->SetTextStyle(0);

	// título: " VS " na guilda, o nome do evento no hole in one (sem texto nos outros tipos reais)
	const char* title = NULL;
	if (pRoom->gameType == GAME_TYPE_GUILD_MATCH)
		title = K2L_Compatibility("            VS             ");
	else if (pRoom->gameType == GAME_TYPE_HOLEINONE983)
	{
		switch (pRoom->realGameType)
		{
		case 0xb:
			title = K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8 \xc0\xcc\xba\xa5\xc6\xae(\xc1\xdf\xb1\xde)");
			break;
		case 0xd:
			title = K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8 \xc0\xcc\xba\xa5\xc6\xae(\xbb\xf3\xb1\xde)");
			break;
		}
	}
	else
		title = pRoom->title;

	g_pFresh->GetManager()->PrintText(WPoint(s_offset[8].x + pItem->pos.x, s_offset[8].y + pItem->pos.y), 0, title, -1.0f,
		0xffffffff);

	// mapa: cor 0 com a sala aberta, 5 em jogo
	int mapColor = pRoom->bAvailable ? 0 : 5;

	if (pRoom->realGameType != GAME_TYPE_USEMAX)
	{
		void* pCourse = ItemManager()->FindCourse(0x28000000 | (pRoom->mapType >= 0x7f ? 0x7f : pRoom->mapType));
		if (pCourse)
		{
			const Bitmap* pMap = g_pFresh->GetBitmap(MakeStr("room_%s", IFF983_ICON(pCourse)));
			if (pMap)
			{
				float mx = s_offset[14].x + pItem->pos.x;
				unsigned h = pMap->Height();
				unsigned w = pMap->Width();
				pGDI->DrawTexture(pMap,
					WRect(mx, (float)(int)(float)((pList->GetItemHeight() - h) * 0.5) + pItem->pos.y, (float)w, (float)h),
					s_color[mapColor], 0);

				sMapEvent983* pEvent = ((CSharedDoc983c*)Doc())->Fn63f830_983(IFF983_TYPEID(pCourse) & 0x3ffffff);
				if (pEvent && pEvent->m_08 == 0 && pEvent->rate > 0)
				{
					const Bitmap* pMark = g_pFresh->GetBitmap("lobby_mapevent_mark");
					if (pMark)
						pGDI->DrawTexture(pMark,
							WRect((float)(pMap->Width() - pMark->Width()) + mx, pItem->pos.y, (float)pMark->Width(),
								(float)pMark->Height()),
							0xffffffff, 0);
				}
			}
		}
	}
	else
	{
		float mx = s_offset[14].x + pItem->pos.x;
		void* pCourse = ItemManager()->FindCourse(0x2800007f);
		if (pCourse)
		{
			const Bitmap* pMap = g_pFresh->GetBitmap("room_map_17_01");
			if (pMap)
			{
				unsigned h = pMap->Height();
				unsigned w = pMap->Width();
				pGDI->DrawTexture(pMap,
					WRect(mx, (float)(int)(float)((pList->GetItemHeight() - h) * 0.5) + pItem->pos.y, (float)w, (float)h),
					s_color[mapColor], 0);
			}
		}
	}

	pGDI->Print(WPoint(s_offset[11].x + pItem->pos.x, s_offset[11].y + pItem->pos.y), 2,
		K2L_Compatibility("%d\xc8\xa6"), pRoom->nHole);

	pGDI->Print(WPoint(s_offset[13].x + pItem->pos.x, s_offset[13].y + pItem->pos.y), 2, "%d/%d", pRoom->nUserNum,
		pRoom->nUserLimit);

	if (pRoom->gameType != GAME_TYPE_OFFLINE_GHOST)
	{
		if (Doc()->m_curGameServer.eventFlags & 0x20)
		{
			if (Doc()->m_curGameServer.eventValue &
				(1 << ((pRoom->mapType > 0x7f && pRoom->mapType != 0xfd) ? pRoom->mapType - 0x80 : pRoom->mapType)))
			{
				const Bitmap* pIcon = g_pFresh->GetBitmap("icon_pangevent");
				pGDI->DrawTexture(pIcon,
					WRect(s_offset[15].x + pItem->pos.x, s_offset[15].y + pItem->pos.y, (float)pIcon->Width(),
						(float)pIcon->Height()),
					0xffffffff, 0);
			}
		}
	}

	if (ROOM983_CLASSIC(pRoom))
	{
		const Bitmap* pIcon = g_pFresh->GetBitmap("icon_classic_room");
		pGDI->DrawTexture(pIcon,
			WRect(s_offset[16].x + pItem->pos.x, s_offset[16].y + pItem->pos.y, (float)(pIcon->Width() * 0.8f),
				(float)(pIcon->Height() * 0.8f)),
			0xffffffff, 0);
	}
}

// 0059b980 (645: OnRoomList_RoomListDClick). 983: sem o teste do tipo do canal; o nível 0 também não entra no torneio de 30
// (tipos 4 e 5); os textos passam pelo K2L_Compatibility; os avisos e a janela da senha (mensagem 0x8b, 3) vão para o ator
// principal e as mensagens 0x16 e 1 para o ator "Lobby"
void CNormalLobbyRoom::OnRoomListDClick(FrRoomListDlg* pDlg)
{
	if (!pDlg->m_pSelRoom)
		return;

	if (pDlg->m_pRoomList && !pDlg->m_pRoomList->IsEnabled())
		return;

	CIntrusion::Instance()->FinishGame();

	sRoomInfo* pRoom = pDlg->m_pSelRoom;
	if (pRoom->bAvailable)
	{
		if (!(bool)(Doc()->m_myInfo.info.dwIdentity & 4))
		{
			if (pRoom->nUserNum == pRoom->nUserLimit)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
					(int)K2L_Compatibility("\xc1\xa4\xbf\xf8\xc0\xcc \xc3\xca\xb0\xfa \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9"),
					0, 0, 0, 0));
				return;
			}

			switch (pRoom->gameType)
			{
			case GAME_TYPE_NEW_APPROACH:
				if (Doc()->m_myInfo.stat.Level == 0)
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
						(int)K2L_Compatibility("\xb7\xe7\xc5\xb0"
							"F \xb7\xb9\xba\xa7\xc0\xba \xbe\xee\xc7\xc1\xb7\xce\xc4\xa1 \xb0\xd4\xc0\xd3\xc0\xbb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
						0, 0, 0, 0));
					return;
				}
				break;

			case GAME_TYPE_30S:
				if (Doc()->m_myInfo.stat.Level == 0)
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
						(int)K2L_Compatibility("\xb7\xe7\xc5\xb0"
							"F \xb7\xb9\xba\xa7\xc0\xba 30\xc0\xce\xb4\xeb\xc0\xfc \xb9\xd7 30\xc0\xce\xc6\xc0\xc0\xfc \xb0\xd4\xc0\xd3\xc0\xbb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
						0, 0, 0, 0));
					return;
				}
				break;

			case GAME_TYPE_30S_TEAM:
			{
				std::map<unsigned long, sBriefUserInfo>::iterator it =
					Doc()->m_briefUserInfoMap.find(MyGuid(false));
				if (it != Doc()->m_briefUserInfoMap.end() && (*it).second.gender > 1)
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
						(int)K2L_Compatibility("\xb0\xad\xc1\xa6 \xc1\xbe\xb7\xe1\xc0\xb2\xc0\xcc \xb3\xf4\xc0\xba \xc0\xaf\xc0\xfa\xb4\xc2 30\xc0\xce \xc6\xc0\xc0\xfc \xb0\xd4\xc0\xd3\xc0\xbb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
						0, 0, 0, 0));
					return;
				}
				if (Doc()->m_myInfo.stat.Level == 0)
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
						(int)K2L_Compatibility("\xb7\xe7\xc5\xb0"
							"F \xb7\xb9\xba\xa7\xc0\xba 30\xc0\xce\xb4\xeb\xc0\xfc \xb9\xd7 30\xc0\xce\xc6\xc0\xc0\xfc \xb0\xd4\xc0\xd3\xc0\xbb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
						0, 0, 0, 0));
					return;
				}
			}
			break;

			case GAME_TYPE_GUILD_MATCH:
			{
				std::string msg;
				bool bShow = false;
				unsigned long guildId = Doc()->m_myInfo.info.dwGuildId;

				if (guildId == 0)
				{
					msg = K2L_Compatibility("\xb1\xe6\xb5\xe5\xbf\xa1 \xb0\xa1\xc0\xd4\xc7\xcf\xbc\xc5\xbe\xdf \xb1\xe6\xb5\xe5\xb4\xeb\xc0\xfc\xc0\xcc \xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9.");
					bShow = true;
				}

				if (pRoom->GuildInfo.nGuildID[0] != guildId && pRoom->GuildInfo.nGuildID[0] != 0 &&
					pRoom->GuildInfo.nGuildID[1] != guildId && pRoom->GuildInfo.nGuildID[1] != 0)
				{
					msg = K2L_Compatibility("\xc5\xb8\xb1\xe6\xb5\xe5\xc0\xc7 \xb4\xeb\xc0\xfc\xc0\xd4\xb4\xcf\xb4\xd9.");
					bShow = true;
				}

				if (bShow)
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)msg.c_str(), 0, 0, 0, 0));
					return;
				}
			}
			break;
			}
		}

		pDlg->EnableRoomEnterControls(false);

		BOOL bAdmin = (Doc()->m_myInfo.info.dwIdentity & 0xe) == 0xe;
		if (bAdmin)
		{
			pDlg->EnableRoomEnterControls(false);

			WSendPacket packet((enumClientPacket)0x3e);
			packet.Encode2(pRoom->roomGuid);
			packet.EncodeStr(std::string(""));
			packet.Send(TO_GAME);

			AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x16, 0, 0, 0, 0, 0) << MsgObject(NULL, 1, 0, 0, 0, 0, 0);
			return;
		}

		bool bGM = (Doc()->m_myInfo.info.dwIdentity & 0x14) ? true : false;
		bool bInvite = false;
		if (IsLocalContent((localContentType_t)0x55))
		{
			if (DOC983_U32(0x11380))
				bInvite = true;
		}

		if (pRoom->bPublic || bGM || bInvite)
		{
			if (IsLocalContent((localContentType_t)0x2f) && pRoom->bIntrusion == true && !bGM)
			{
				CIntrusion::Instance()->DoJoinRoom(pRoom->roomGuid);
			}
			else
			{
				WSendPacket packet((enumClientPacket)9);
				packet.Encode2(pRoom->roomGuid);
				packet.EncodeStr(std::string(""));
				packet.Send(TO_GAME);

				AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x16, 0, 0, 0, 0, 0) << MsgObject(NULL, 1, 0, 0, 0, 0, 0);
			}
		}
		else
		{
			Doc()->m_roomInfo = *pRoom;
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x8b, 3, 0, 0, 0, 0));
		}
	}
	else
	{
		if (pRoom->bIntrusion)
		{
			Doc()->m_roomInfo = *pRoom;
			CIntrusion::Instance()->DoJoinRoom(Doc()->m_roomInfo.roomGuid);
		}
		else
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
				(int)K2L_Compatibility("\xb0\xd4\xc0\xd3 \xc1\xf8\xc7\xe0\xc1\xdf\xc0\xce \xb9\xe6\xc0\xd4\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0));
		}
	}
}

// B191 (A173.3): leitura defensiva. O 983 indexa as tabelas sem checar o tipo; um valor que não seja tabela dava erro Lua fora de
// pcall (luaD_throw -> exit(1), o "fecha com código 1" do robô a173-3/a173-7). Aqui cada nível é conferido com lua_type e
// os índices são limitados ao tamanho dos vetores; com a tabela certa o resultado é o mesmo do 983.
// A194: a tabela fica viva no chamador; o ~table_obj tira ela da pilha (lua_remove) e o indice deixaria de valer.
static bool LayoutTable983(const lua_system::lua_table_helper& tbl, lua_State** pL, int* pIdx)
{
	if (!tbl.table.m_obj)
		return false;
	*pL = tbl.table.m_obj->m_L;
	*pIdx = tbl.table.m_obj->m_index;
	return lua_type(*pL, *pIdx) == LUA_TTABLE;
}

static float LayoutNum983(lua_State* L, int idx, int i)
{
	float v = 0.0f;
	lua_rawgeti(L, idx, i);
	if (lua_type(L, -1) == LUA_TNUMBER)
		v = (float)lua_tonumber(L, -1);
	lua_pop(L, 1);
	return v;
}

// 0059dbe0: lê as três tabelas de roomlist_layout_ui.lua (posições, pedaços do fundo, cores em texto hexadecimal)
void LoadRoomListLayout983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
	{
		if (!script->RunFile("roomlist_layout_ui.lua")) // B215: arquivo ausente/erro: sem tabelas (GetTable em script vazio dava exit(1)?)
		{
			lua_system::lua_script_manger::GetInstance()->DeleteScript(script);
			s_offset.resize(17);
			s_baseFrame.resize(3);
			s_color.resize(6);
			return;
		}

		s_offset.resize(17);
		s_baseFrame.resize(3);
		s_color.resize(6);

		lua_State* L;
		int idx;
		{
			lua_system::lua_table_helper tbl = script->GetTable("roomlist_layout_ui_offset");
			if (LayoutTable983(tbl, &L, &idx))
			{
				int count = (int)lua_objlen(L, idx);
				for (int i = 1; i <= count && i <= 17; i++)
				{
					lua_rawgeti(L, idx, i);
					if (lua_type(L, -1) == LUA_TTABLE)
					{
						int t = lua_gettop(L);
						s_offset[i - 1].x = LayoutNum983(L, t, 1);
						s_offset[i - 1].y = LayoutNum983(L, t, 2);
					}
					lua_pop(L, 1);
				}
			}
		}

		{
			lua_system::lua_table_helper tbl = script->GetTable("roomlist_layout_ui_base_frame");
			if (LayoutTable983(tbl, &L, &idx))
			{
				int count = (int)lua_objlen(L, idx);
				for (int i = 1; i <= count && i <= 3; i++)
				{
					lua_rawgeti(L, idx, i);
					if (lua_type(L, -1) == LUA_TTABLE)
					{
						int t = lua_gettop(L);
						s_baseFrame[i - 1].x = LayoutNum983(L, t, 1);
						s_baseFrame[i - 1].y = LayoutNum983(L, t, 2);
						s_baseFrame[i - 1].w = LayoutNum983(L, t, 3);
						s_baseFrame[i - 1].h = LayoutNum983(L, t, 4);
					}
					lua_pop(L, 1);
				}
			}
		}

		{
			lua_system::lua_table_helper tbl = script->GetTable("roomlist_layout_ui_color");
			if (LayoutTable983(tbl, &L, &idx))
			{
				int count = (int)lua_objlen(L, idx);
				for (int i = 1; i <= count && i <= 6; i++)
				{
					lua_rawgeti(L, idx, i);
					if (lua_type(L, -1) == LUA_TSTRING)
						sscanf(lua_tostring(L, -1), "%x", &s_color[i - 1]);
					lua_pop(L, 1);
				}
			}
		}

		lua_system::lua_script_manger::GetInstance()->DeleteScript(script);
	}
}

// 0059e250
void LoadRoomListLayoutThunk983()
{
	LoadRoomListLayout983();
}

// ------------------------------------------------------------------------------------------------------------------------
// CGrandPrixLobbyRoom (vtable 00d0c588, 0x194 bytes) — a lista "roomlist" quando o tipo de `CSharedDoc+0x11630` é 1. Sessão
// B51. Cada linha é um `sGPRoomItem983` que o documento 0xaf do Grand Prix entrega (00558ef0); a sala escolhe a linha, pede os
// detalhes e manda os avisos 0x753e ao ator "Gateway". O desenho (0059c710) está no fim do arquivo. O relógio do Grand Prix (00e74e8c,
// 0055ce30–0055ee70) e o documento 0xaf são da F3.7.8: entram aqui emprestados pelo endereço.

// sGPInfo983 / sGPRoomItem983 / sGPEnter983: include/jp/grandprix983.h (B65)

// relógio e auxiliares das linhas: include/jp/gproomtimersteward.h (B67; o 00e74e8c é o WSingleton do CGrandPrixClock)
// documento 0xaf: include/jp/grandprixdocument.h (B68)

static const char* s_gateway = "Gateway"; // 00e0f284

// 00598960 (no 983 o primeiro argumento vem em eax: unidade com LTCG)
static bool TestFlag983(unsigned long flags, unsigned long mask)
{
	static unsigned long s_flags; // 00e75eec
	s_flags = flags;
	return (flags & mask) != 0;
}

// 00598990
CGrandPrixLobbyRoom::CGrandPrixLobbyRoom()
	: m_188(0), m_18c(0)
{
	memset(m_offset, 0, sizeof(m_offset));
	m_normalItem = sGPPoint983();
	m_eventItem = sGPPoint983();
}

// 005989f0 (com `delete`: 00598a00)
CGrandPrixLobbyRoom::~CGrandPrixLobbyRoom()
{
}

// 00598a30
void CGrandPrixLobbyRoom::OnRoomListLDown(FrRoomListDlg* pDlg)
{
	FrListBox* pList = pDlg->m_pRoomList2;
	FrListItem* pItem = pList->GetItemUnderCursor();
	if (pItem)
		pList->SelectItem(pItem, true);
	else
		pList->SelectItem(NULL, true);
}

// 00598f90: a sala chegou — quem refaz a lista é o "Gateway"
void CGrandPrixLobbyRoom::AddRoom983(FrListBox* pList, int room)
{
	MsgObject msg(NULL, 0x753e, 0xb, room, 0, 0, 0);
	AfxGetTask()->GetActor(s_gateway) << msg;
}

// 00599000: a sala saiu
void CGrandPrixLobbyRoom::DelRoom983(FrListBox* pList, int room)
{
	MsgObject msg(NULL, 0x753e, 0xc, room, 0, 0, 0);
	AfxGetTask()->GetActor(s_gateway) << msg;
}

// 00599070: pedido de entrada (aviso 0x753e / 7)
static void SendGPEnter983(sGPRoomItem983* pItem, int a, int b)
{
	sGPEnter983 arg;
	arg.state = pItem->state;
	arg.pInfo = pItem->pInfo;
	arg.m_08 = a;
	arg.m_0c = b;
	MsgObject msg(NULL, 0x753e, 7, (int)&arg, 0, 0, 0);
	AfxGetTask()->GetActor(s_gateway) << msg;
}

// 005990f0: duplo clique — cada motivo de recusa vira o aviso 0x753e / 8 com a linha (o "Gateway" mostra a mensagem); sem
// recusa e com o Grand Prix aberto, o pedido de entrada
void CGrandPrixLobbyRoom::OnRoomListDClick(FrRoomListDlg* pDlg)
{
	FrListItem* pItem = pDlg->m_pRoomList2->GetItemUnderCursor();
	if (!pItem)
		return;
	sGPRoomItem983* p = (sGPRoomItem983*)pItem->pData;
	if (!p)
		return;

	p->flags &= ~0x180;
	bool bTimed = true;
	int kind = (p->pInfo->ticket >> 0x13) & 0x7f;
	if (kind != 0)
	{
		if (kind == 0x63)
		{
			if (p->pInfo->open[0] == 0 || p->pInfo->start[0] == 0 || p->pInfo->end[0] == 0)
				bTimed = false;
		}
	}
	else
	{
		bTimed = false;
		if (TestFlag983(p->opts, 4))
		{
			p->flags |= 0x100;
			MsgObject msg(NULL, 0x753e, 8, (int)p, 0, 0, 0);
			AfxGetTask()->GetActor(s_gateway) << msg;
			return;
		}
	}

	if (!p->bEnabled)
		return;

	if (bTimed == true)
	{
		__int64 now = CGrandPrixClock::Instance()->GetNow983();
		GPItemUpdateState983(p, &now, 0);
		if (p->state != 2)
		{
			SendGPEnter983(p, pDlg->m_15c, pDlg->m_158);
			return;
		}
	}

	if (TestFlag983(p->flags, 3))
	{
		MsgObject msg(NULL, 0x753e, 8, (int)p, 0, 0, 0);
		AfxGetTask()->GetActor(s_gateway) << msg;
	}
	else if (TestFlag983(p->flags, 0xc))
	{
		MsgObject msg(NULL, 0x753e, 8, (int)p, 0, 0, 0);
		AfxGetTask()->GetActor(s_gateway) << msg;
	}
	else if (TestFlag983(p->flags, 0x10) == true && !GPHasTicket983(p->pInfo->ticket))
	{
		MsgObject msg(NULL, 0x753e, 8, (int)p, 0, 0, 0);
		AfxGetTask()->GetActor(s_gateway) << msg;
	}
	else if (!GPItemCheckCondition983(p))
	{
		MsgObject msg(NULL, 0x753e, 8, (int)p, 0, 0, 0);
		AfxGetTask()->GetActor(s_gateway) << msg;
	}
	else
	{
		switch (p->state)
		{
		case 2:
			SendGPEnter983(p, pDlg->m_15c, pDlg->m_158);
			break;
		}
	}
}

// 005998a0: posições da linha — "grandprixuioffset"; no canal de evento (0062bf30 == 0x63) a linha tem o outro tamanho e as
// posições de "gpeventlist" entram por cima
void CGrandPrixLobbyRoom::LoadOffset983(FrListBox* pList)
{
	lua_system::lua_script* script = GetLobbyRoomScript983();
	if (!script)
		return;

	{
		lua_system::lua_table_helper tbl = script->GetTable("grandprixuioffset");
		int count = lua_tinker::call<int, lua_tinker::table>(tbl.state, "get_array_size", tbl.table);
		sGPPoint983* pOffset = m_offset;
		for (int i = 1; i <= count; i++, pOffset++)
		{
			float x = AsTable983(LUAIDX983(tbl).GetTableByIndex(i).table).GetByIndex(1);
			float y = AsTable983(LUAIDX983(tbl).GetTableByIndex(i).table).GetByIndex(2);
			pOffset->x = x;
			pOffset->y = y;
		}
	}

	if (((CSharedDocJp*)Doc())->GetGPChannel983() != 0x63)
	{
		pList->Resize((int)m_normalItem.x, (int)m_normalItem.y);
		return;
	}
	pList->Resize((int)m_eventItem.x, (int)m_eventItem.y);

	lua_system::lua_table_helper tbl = GetLobbyRoomScript983()->GetTable("gpeventlist");
	int count = lua_tinker::call<int, lua_tinker::table>(tbl.state, "get_array_size", tbl.table);
	sGPPoint983* pOffset = m_offset;
	for (int i = 1; i <= count; i++, pOffset++)
	{
		float x = LUAIDX983(LUAIDX983(tbl).GetTableByIndex(i)).GetByIndex<float>(1);
		float y = LUAIDX983(LUAIDX983(tbl).GetTableByIndex(i)).GetByIndex<float>(2);
		pOffset->x = x;
		pOffset->y = y;
	}
}

// 00599c30 (cópia idêntica em 00599b90): `out` = os dois números da tabela `name`
static __declspec(noinline) void ReadGPPoint983(lua_system::lua_table_helper* pTbl, const char* name, sGPPoint983* out)
{
	out->x = AsTable983(pTbl->GetValueByName<lua_tinker::table>(name)).GetByIndex(1);
	out->y = AsTable983(pTbl->GetValueByName<lua_tinker::table>(name)).GetByIndex(2);
}

// 00599cd0: grandprix_lobby_ui.lua (tamanho das linhas) e os bitmaps do fundo e dos nomes
void CGrandPrixLobbyRoom::LoadScript983()
{
	LoadLobbyRoomScript983("grandprix_lobby_ui.lua");
	lua_system::lua_script* script = GetLobbyRoomScript983();
	if (script)
	{
		{
			lua_system::lua_table_helper tbl = script->GetTable("grandprixlistitem");
			ReadGPPoint983(&tbl, "gp_normal_item", &m_normalItem);
			ReadGPPoint983(&tbl, "gp_event_item", &m_eventItem);
		}

		m_pGpBmp[0][0] = g_pFresh->GetBitmap("gp_roomlistbox_n_01");
		m_pGpBmp[1][0] = g_pFresh->GetBitmap("gp_roomlistbox_n_02");
		m_pGpBmp[2][0] = g_pFresh->GetBitmap("gp_roomlistbox_n_03");
		m_pGpBmp[0][1] = g_pFresh->GetBitmap("gp_roomlistbox_d_01");
		m_pGpBmp[1][1] = g_pFresh->GetBitmap("gp_roomlistbox_d_02");
		m_pGpBmp[2][1] = g_pFresh->GetBitmap("gp_roomlistbox_d_03");
		m_pGpBmp[0][2] = g_pFresh->GetBitmap("gp_roomlistbox_e_01");
		m_pGpBmp[1][2] = g_pFresh->GetBitmap("gp_roomlistbox_e_02");
		m_pGpBmp[2][2] = g_pFresh->GetBitmap("gp_roomlistbox_e_03");
		m_pGpBmp[0][3] = g_pFresh->GetBitmap("gp_roomlistbox_o_01");
		m_pGpBmp[1][3] = g_pFresh->GetBitmap("gp_roomlistbox_o_02");
		m_pGpBmp[2][3] = g_pFresh->GetBitmap("gp_roomlistbox_o_03");
		m_pEventBmp[0][0] = g_pFresh->GetBitmap("event_roomlistbox_n_01");
		m_pEventBmp[1][0] = g_pFresh->GetBitmap("event_roomlistbox_n_02");
		m_pEventBmp[2][0] = g_pFresh->GetBitmap("event_roomlistbox_n_03");
		m_pEventBmp[0][1] = g_pFresh->GetBitmap("event_roomlistbox_d_01");
		m_pEventBmp[1][1] = g_pFresh->GetBitmap("event_roomlistbox_d_02");
		m_pEventBmp[2][1] = g_pFresh->GetBitmap("event_roomlistbox_d_03");
		m_pEventBmp[0][2] = g_pFresh->GetBitmap("event_roomlistbox_e_01");
		m_pEventBmp[1][2] = g_pFresh->GetBitmap("event_roomlistbox_e_02");
		m_pEventBmp[2][2] = g_pFresh->GetBitmap("event_roomlistbox_e_03");
		m_pEventBmp[0][3] = g_pFresh->GetBitmap("event_roomlistbox_o_01");
		m_pEventBmp[1][3] = g_pFresh->GetBitmap("event_roomlistbox_o_02");
		m_pEventBmp[2][3] = g_pFresh->GetBitmap("event_roomlistbox_o_03");

		lua_system::lua_table_helper names = GetLobbyRoomScript983()->GetTable("grandprixRoomListUINames");
		int count = lua_tinker::call<int, lua_tinker::table>(names.state, "get_array_size", names.table);
		const Bitmap** ppBmp = m_pNameBmp;
		for (int i = 1; i <= count; i++, ppBmp++)
			*ppBmp = g_pFresh->GetBitmap(LUAIDX983(names).GetByIndex<const char*>(i));
	}
}

// 0059a390: botão direito — sala já criada: detalhes pelo pacote 0x2d e mensagem 1; ainda não: mensagem 0x34 (1, typeid)
void CGrandPrixLobbyRoom::OnRoomListRUp(FrRoomListDlg* pDlg)
{
	FrListBox* pList = pDlg->m_pRoomList2;
	FrListItem* pItem = pList->GetItemUnderCursor();
	if (pItem)
	{
		sGPRoomItem983* p = (sGPRoomItem983*)pItem->pData;
		if (p)
		{
			pList->SelectItem(pItem, true);
			if (p->pRoom)
			{
				WSendPacket packet((enumClientPacket)0x2d);
				packet.Encode2(p->pRoom->roomGuid);
				packet.Send(TO_GAME);
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
			}
			else
			{
				// lista de CSharedDoc+0x10a40 (elemento não levantado; ver roomlistdlg.cpp): 0051fbc0 esvazia
				((std::list<int>*)DOCPTR983(0x10a40))->clear();
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x34, 1, p->pInfo->typeId, 0, 0, 0));
			}
		}
	}
}

// predicados do Init (embutidos no 983; o terceiro é o 00599500)
struct GPItemState983
{
	int state;
	GPItemState983(int s) : state(s) { }
	bool operator()(FrListItem* pItem) const
	{
		if (pItem && pItem->pData)
			return ((sGPRoomItem983*)pItem->pData)->state == state;
		return false;
	}
};
struct GPItemTypeId983
{
	int typeId;
	GPItemTypeId983(int t) : typeId(t) { }
	bool operator()(FrListItem* pItem) const
	{
		if (pItem && pItem->pData)
			return ((sGPRoomItem983*)pItem->pData)->pInfo->typeId == typeId;
		return false;
	}
};

// 0059c0c0: refaz a lista com as linhas do documento 0xaf e escolhe uma — a primeira aberta (2), senão a primeira fora de
// hora (0), senão a do Grand Prix atual do documento
void CGrandPrixLobbyRoom::Init(FrRoomListDlg* pDlg)
{
	FrListBox* pList = pDlg->m_pRoomList2;
	pList->ClearItem();
	m_190 = -1;
	m_188 = pDlg->m_158;
	m_18c = pDlg->m_15c;
	LoadOffset983(pList);

	_gateway::_grandprix::CGrandPrixDocument* pDoc = NULL;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xaf, pDoc);

	__int64 now = CGrandPrixClock::Instance()->GetNow983();
	std::vector<sGPRoomItem983*> items;
	pDoc->GetRoomItems983(&items, &now);
	for (std::vector<sGPRoomItem983*>::iterator it = items.begin(); it != items.end(); ++it)
		pList->AddItem(*it);

	FrListBox::ITEM_LIST::iterator it =
		std::find_if(pList->m_itemList.begin(), pList->m_itemList.end(), GPItemState983(2));
	if (it != pList->m_itemList.end() && *it)
	{
		pList->SelectItem(*it, true);
	}
	else
	{
		it = std::find_if(pList->m_itemList.begin(), pList->m_itemList.end(), GPItemState983(0));
		if (it != pList->m_itemList.end() && *it)
		{
			pList->SelectItem(*it, true);
		}
		else
		{
			it = std::find_if(pList->m_itemList.begin(), pList->m_itemList.end(),
				GPItemTypeId983(pDoc->GetCurrentTypeId983()));
			if (it != pList->m_itemList.end())
				pList->SelectItem(*it, true);
		}
	}
}

// emprestados da F3.7.8 para a hora da linha (as duas recebem em registrador)
// 0055cd50: dois algarismos, sem o zero final (sprintf_s(buf, 4, "%d") e, abaixo de 10, "0" + o algarismo)
void __fastcall GPTwoDigits983(unsigned short n, char* out)
{
	char buf[4] = { 0 };
	sprintf_s(buf, 4, "%d", (int)n);
	if (n < 10)
	{
		out[0] = '0';
		out[1] = buf[0];
	}
	else
	{
		out[0] = buf[0];
		out[1] = buf[1];
	}
}
static char s_gpTime[4]; // 00e75ef4

// 0059c710: desenho de uma linha da lista do Grand Prix. Fundo em três pedaços (o do canal de evento — 0062bf30 == 0x63 — é
// o outro jogo de bitmaps, mais alto), texto da linha, tipo do bilhete, descrição, estado, selo, número, prêmio, nome,
// horário de começo e fim (só fora do canal 0), recusa, mapa, buracos e a contagem de jogadores da sala já criada. As
// posições são as de `m_offset` (grandprix_lobby_ui.lua) e os bitmaps os de `m_pNameBmp`, pelo índice
void CGrandPrixLobbyRoom::OnRoomListOwnerDraw(int param, FrRoomListDlg* pDlg)
{
	FrListItem* pItem = (FrListItem*)param;

	FrWndManager* pManager = g_pFresh->GetManager();
	FrGraphicInterface* pGDI = pManager->GetGDI();
	if (!pGDI)
		return;

	sGPRoomItem983* p = (sGPRoomItem983*)pItem->pData;

	int state = 0;
	if (p)
	{
		if (pItem->selected)
			state = 2;
		else if (pItem->underCursor)
			state = 1;
	}

	int channel = ((CSharedDocJp*)Doc())->GetGPChannel983();

	if (channel != 0x63)
	{
		WRect left(m_offset[12].x + pItem->pos.x, pItem->pos.y, 122.0f, 97.0f);
		WRect src(0.0f, 0.0f, 4.0f, 97.0f);
		WRect mid(m_offset[13].x + pItem->pos.x, pItem->pos.y, 295.0f, 97.0f);
		WRect right(m_offset[14].x + pItem->pos.x, pItem->pos.y, 100.0f, 97.0f);

		if (state == 2)
		{
			pGDI->DrawTexture(m_pGpBmp[0][1], left, 0xffffffff, 0);
			pGDI->DrawTexture(m_pGpBmp[1][1], src, mid, 0xffffffff, 0);
			pGDI->DrawTexture(m_pGpBmp[2][1], right, 0xffffffff, 0);
		}
		pGDI->DrawTexture(m_pGpBmp[0][state], left, 0xffffffff, 0);
		pGDI->DrawTexture(m_pGpBmp[1][state], src, mid, 0xffffffff, 0);
		pGDI->DrawTexture(m_pGpBmp[2][state], right, 0xffffffff, 0);
	}
	else
	{
		WRect left(m_offset[12].x + pItem->pos.x, pItem->pos.y, 122.0f, 146.0f);
		WRect src(0.0f, 0.0f, 4.0f, 146.0f);
		WRect mid(m_offset[13].x + pItem->pos.x, pItem->pos.y, 295.0f, 146.0f);
		WRect right(m_offset[14].x + pItem->pos.x, pItem->pos.y, 100.0f, 146.0f);

		if (state == 2)
		{
			pGDI->DrawTexture(m_pEventBmp[0][1], left, 0xffffffff, 0);
			pGDI->DrawTexture(m_pEventBmp[1][1], src, mid, 0xffffffff, 0);
			pGDI->DrawTexture(m_pEventBmp[2][1], right, 0xffffffff, 0);
		}
		pGDI->DrawTexture(m_pEventBmp[0][state], left, 0xffffffff, 0);
		pGDI->DrawTexture(m_pEventBmp[1][state], src, mid, 0xffffffff, 0);
		pGDI->DrawTexture(m_pEventBmp[2][state], right, 0xffffffff, 0);
	}

	if (!p)
		return;

	pGDI->SetTextStyle(0);
	pGDI->SetTextColor(0xff000000, 0xffffffff);

	sGPInfo983* pInfo = p->pInfo;

	if (!p->title.empty())
		pManager->PrintText(WPoint(pItem->pos.x + m_offset[2].x, pItem->pos.y + m_offset[2].y), 0, p->title.c_str(), -1.0f,
			0xffffffff);

	const Bitmap* pA;
	const Bitmap* pB = NULL;

	if (channel != 0x63)
	{
		// tipo do bilhete: nome e moldura
		switch ((pInfo->ticket >> 19) & 0x7f)
		{
		case 1:
			pA = m_pNameBmp[1];
			pB = m_pNameBmp[5];
			break;
		case 2:
			pA = m_pNameBmp[2];
			pB = m_pNameBmp[6];
			break;
		case 3:
			pA = m_pNameBmp[3];
			pB = m_pNameBmp[7];
			break;
		default:
			pA = m_pNameBmp[0];
			pB = m_pNameBmp[4];
			break;
		}

		pGDI->DrawTexture(pB,
			WRect(m_offset[15].x + pItem->pos.x, m_offset[15].y + pItem->pos.y, (float)pB->Width(), (float)pB->Height()),
			0xffffffff, 0);
		pGDI->DrawTexture(pA,
			WRect(m_offset[16].x + pItem->pos.x, m_offset[16].y + pItem->pos.y, (float)pA->Width(), (float)pA->Height()),
			0xffffffff, 0);

		if (channel == 0)
		{
			float tx = m_offset[17].x + pItem->pos.x;
			float ty = (float)(m_offset[17].y + pItem->pos.y + 38.0);
			FrTEXT text(pInfo->desc);
			pGDI->PrintText(WPoint(tx, ty), 0, text);
		}
	}
	else
	{
		pA = g_pFresh->GetBitmap(pInfo->bmpName);
		if (pA)
			pGDI->DrawTexture(pA, WRect(pItem->pos.x, pItem->pos.y, (float)pA->Width(), (float)pA->Height()), 0xffffffff, 0);

		float tx = m_offset[17].x + pItem->pos.x;
		float ty = (float)(m_offset[17].y + pItem->pos.y + 38.0);
		FrTEXT text(pInfo->desc);
		pGDI->PrintText(WPoint(tx, ty), 0, text);
	}

	// estado em duas metades (só fora do canal 0)
	switch (p->state)
	{
	case 0:
	case 1:
	case 2:
		pA = m_pNameBmp[8];
		pB = m_pNameBmp[9];
		break;
	case 3:
	case 4:
		pA = m_pNameBmp[10];
		pB = m_pNameBmp[11];
		break;
	}

	if (channel != 0)
	{
		pGDI->DrawTexture(pA,
			WRect(m_offset[5].x + pItem->pos.x, m_offset[5].y + pItem->pos.y, (float)pA->Width(), (float)pA->Height()),
			0xffffffff, 0);
		pGDI->DrawTexture(pB,
			WRect(m_offset[5].x + pItem->pos.x + (float)pA->Width(), m_offset[5].y + pItem->pos.y, (float)pB->Width(),
				(float)pB->Height()),
			0xffffffff, 0);
	}

	// estado por extenso
	switch (p->state)
	{
	case 0:
		pA = m_pNameBmp[12];
		break;
	case 1:
		pA = m_pNameBmp[13];
		break;
	case 2:
		pA = m_pNameBmp[14];
		break;
	case 3:
		pA = m_pNameBmp[15];
		break;
	case 4:
		pA = m_pNameBmp[16];
		break;
	}

	if (channel != 0)
	{
		float sx = m_offset[4].x;
		float sy = m_offset[4].y;
		if (p->state == 4)
		{
			sx = (float)(sx + 6.0);
			sy = (float)(sy + 6.0);
		}
		pGDI->DrawTexture(pA, WRect(pItem->pos.x + sx, pItem->pos.y + sy, (float)pA->Width(), (float)pA->Height()),
			0xffffffff, 0);
	}

	// selo
	const Bitmap* pKind;
	switch (p->kind)
	{
	case 0:
		pKind = m_pNameBmp[25];
		break;
	case 1:
		pKind = m_pNameBmp[29];
		break;
	case 2:
	case 3:
		pKind = m_pNameBmp[26];
		break;
	default:
		pKind = m_pNameBmp[28];
		break;
	}
	pGDI->DrawTexture(pKind,
		WRect(m_offset[7].x + pItem->pos.x, m_offset[7].y + pItem->pos.y, (float)pKind->Width(), (float)pKind->Height()),
		0xffffffff, 0);

	{
		float nx = m_offset[8].x + pItem->pos.x;
		float ny = m_offset[8].y + pItem->pos.y;
		((WTitleFontEx*)m_18c)->Print(nx, ny, MakeStr("%d", pInfo->m_ac), (eTITLEFONTALINE)0, 0.0f, 1.0f);
	}

	if (TestFlag983(p->opts, 2))
	{
		float gx = m_offset[1].x + pItem->pos.x;
		const Bitmap* pGift = g_pFresh->GetBitmap("gp_gift");
		pGDI->DrawTexture(pGift, WRect(gx, m_offset[1].y + pItem->pos.y, (float)pGift->Width(), (float)pGift->Height()),
			0xffffffff, 0);
	}

	pManager->PrintText(WPoint(pItem->pos.x + m_offset[17].x, m_offset[17].y + pItem->pos.y), 0, pInfo->name, -1.0f,
		0xffffffff);

	if (TestFlag983(p->opts, 4))
	{
		const Bitmap* pBmp = m_pNameBmp[27];
		pGDI->DrawTexture(pBmp,
			WRect(m_offset[9].x + pItem->pos.x, m_offset[9].y + pItem->pos.y, (float)pBmp->Width(), (float)pBmp->Height()),
			0xffffffff, 0);
	}

	if (pInfo->m_85 == true)
	{
		const Bitmap* pBmp = m_pNameBmp[24];
		pGDI->DrawTexture(pBmp,
			WRect((float)(m_offset[17].x + pItem->pos.x - 30.0), (float)(m_offset[17].y + pItem->pos.y - 2.0),
				(float)pBmp->Width(), (float)pBmp->Height()),
			0xffffffff, 0);
	}

	// horário de começo e de fim, com o selo AM/PM
	{
		float x = m_offset[0].x + pItem->pos.x;
		float y = m_offset[0].y + pItem->pos.y;
		if (channel != 0)
		{
			bool pm = pInfo->start[4] > 11;
			GPTwoDigits983(GPHour12_983(pInfo->start[4]), s_gpTime);
			((WTitleFontEx*)m_188)->Print(x, y, s_gpTime, (eTITLEFONTALINE)0, 0.0f, 1.0f);
			GPTwoDigits983(pInfo->start[5], s_gpTime);
			((WTitleFontEx*)m_188)->Print((float)(x + 47.0), y, s_gpTime, (eTITLEFONTALINE)0, 0.0f, 1.0f);

			const Bitmap* pAmPm = g_pFresh->GetBitmap(pm == true ? "gp_time_pm" : "gp_time_am");
			float y2 = (float)(y + 6.0);
			pGDI->DrawTexture(pAmPm, WRect((float)(x + 91.0), y2, (float)pAmPm->Width(), (float)pAmPm->Height()),
				0xffffffff, 0);

			pm = pInfo->end[4] > 11;
			GPTwoDigits983(GPHour12_983(pInfo->end[4]), s_gpTime);
			double x2 = x + 172.0;
			((WTitleFontEx*)m_188)->Print((float)x2, y, s_gpTime, (eTITLEFONTALINE)0, 0.0f, 1.0f);
			GPTwoDigits983(pInfo->end[5], s_gpTime);
			((WTitleFontEx*)m_188)->Print((float)(x2 + 47.0), y, s_gpTime, (eTITLEFONTALINE)0, 0.0f, 1.0f);

			pAmPm = g_pFresh->GetBitmap(pm == true ? "gp_time_pm" : "gp_time_am");
			pGDI->DrawTexture(pAmPm, WRect((float)(x2 + 91.0), y2, (float)pAmPm->Width(), (float)pAmPm->Height()),
				0xffffffff, 0);
		}
	}

	// recusa: aviso, e os dois selos
	{
		float x = m_offset[3].x + pItem->pos.x;
		float y = m_offset[3].y + pItem->pos.y;
		if (TestFlag983(p->flags, 0x1f))
		{
			const Bitmap* pBmp = m_pNameBmp[30];
			pGDI->DrawTexture(pBmp, WRect(x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
			if (p->pInfo->m_86)
			{
				pBmp = m_pNameBmp[32];
				pGDI->DrawTexture(pBmp, WRect((float)(x + 20.0), y, (float)pBmp->Width(), (float)pBmp->Height()),
					0xffffffff, 0);
			}
			pBmp = m_pNameBmp[31];
			pGDI->DrawTexture(pBmp,
				WRect((float)(x + 50.0), (float)(y + 4.0), (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
		}
		else if (p->pInfo->m_86)
		{
			const Bitmap* pBmp = m_pNameBmp[32];
			pGDI->DrawTexture(pBmp, WRect(x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
		}
	}

	// mapa (o sorteado tem bitmap fixo)
	{
		const Bitmap* pMap = NULL;
		if (!TestFlag983(p->opts, 1))
		{
			void* pCourse = ItemManager()->FindCourse(0x28000000 | (pInfo->course >= 0x7f ? 0x7f : pInfo->course));
			if (pCourse)
				pMap = g_pFresh->GetBitmap(MakeStr("room_%s", IFF983_ICON(pCourse)));
		}
		else
			pMap = g_pFresh->GetBitmap("room_map_17_01");

		if (pMap)
			pGDI->DrawTexture(pMap,
				WRect(m_offset[6].x + pItem->pos.x, m_offset[6].y + pItem->pos.y, (float)pMap->Width(),
					(float)pMap->Height()),
				0xffffffff, 0);
	}

	// buracos ("%d홀"; no mapa sorteado, "%d홀 [스페셜]")
	pGDI->Print(WPoint(m_offset[10].x + pItem->pos.x, m_offset[10].y + pItem->pos.y), 2,
		TestFlag983(p->opts, 1)
			? MakeStr("%s [%s]", K2L_Compatibility("%d\xc8\xa6"), K2L_Compatibility("\xbd\xba\xc6\xe4\xbc\xc8"))
			: K2L_Compatibility("%d\xc8\xa6"),
		pInfo->nHole);

	// jogadores
	{
		const Bitmap* pBmp = m_pNameBmp[23];
		pGDI->DrawTexture(pBmp,
			WRect(m_offset[11].x + pItem->pos.x, m_offset[11].y + pItem->pos.y, (float)pBmp->Width(),
				(float)pBmp->Height()),
			0xffffffff, 0);
	}
	pGDI->SetTextColor(0xffffffff, 0xffffffff);

	sRoomInfo* pRoom = p->pRoom;
	if (!pRoom)
	{
		// sem sala criada: sai sem repor a cor do texto (assim no 983)
		if (channel == 0)
			pGDI->Print(WPoint((float)(m_offset[11].x + pItem->pos.x + 22.0), (float)(m_offset[11].y + pItem->pos.y + 5.0)),
				0, " -/-");
		else
			pGDI->Print(WPoint((float)(m_offset[11].x + pItem->pos.x + 22.0), (float)(m_offset[11].y + pItem->pos.y + 5.0)),
				0, "%d/%d", 0, 30);
		return;
	}

	if (channel == 0)
		pGDI->Print(WPoint((float)(m_offset[11].x + pItem->pos.x + 22.0), (float)(m_offset[11].y + pItem->pos.y + 5.0)), 0,
			" -/-");
	else
		pGDI->Print(WPoint((float)(m_offset[11].x + pItem->pos.x + 22.0), (float)(m_offset[11].y + pItem->pos.y + 5.0)), 0,
			"%d/%d", pRoom->nUserNum, pRoom->nUserLimit);

	pGDI->SetTextColor(0xff000000, 0xffffffff);
}

// 0049a000 (leitura por indice, float; nil -> 0)
float LuaTable983::GetByIndex(int index)
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
