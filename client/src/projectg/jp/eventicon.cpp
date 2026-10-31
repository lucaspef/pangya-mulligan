// JP 983: unidade 00b14aa0–00b16bf0 (CEventIcon e os métodos do CWebIcon que não são construtor nem destrutor; classes em
// include/jp/topicon.h). O ícone de evento acha pelo atributo "name" uma função numa tabela estática (`std::map` em
// 00f03814, construída em 00cf3f?? e destruída por 00cfacd0) preenchida por seis objetos estáticos de registro
// (00f03824…00f03829, inicializadores 00cf3fe0…00cf4080), na ordem abaixo. Sessão B57. Nomes das funções de evento e da
// classe de registro meus. Ficam fora os moldes de `std::map<std::string, Handler>` / `std::transform` / `CreateForm`
// (00b14b60–00b15240, 00b15540–00b166c0, 00b16930–00b16a60) e 00b16bf0–00b16c20 (outra unidade: +0x49e/+0x48e).
#include "minatl.h"
#include "projectg.h"
#include <map>
#include <algorithm>
#include <iterator>
#include <ctype.h>
#include <string.h>
#include "fresh.h"
#include "frwndmanager.h"
#include "frwnd.h"
#include "frwndinl.h"
#include "frbutton.h"
#include "frform.h"
#include "tinyxml.h"
#include "packet.h"
#include "actor.h"
#include "taskmanager.h"
#include "inputmanager.h"
#include "clientsetting.h"
#include "christmassockeventdlg.h"
#include "topicon.h"
#include "webpageconnector.h"

extern Fresh* g_pFresh; // 00f02340

// COption (frente A): método do 983 que o fonte não tem. Empréstimo sem campos.
class COptionJp
{
public:
	// 004949a0: zera as sombras, muda a tela (vChangeScreenSize com a cor atual) e avisa o CLobbyState983a
	void SetScreenSize983(int width, int height);
};

//////////////////////////////////////////////////////////////////////////////// CEventIcon

// 00b14aa0
CEventIcon::CEventIcon()
{
	m_pHandler = NULL;
}

// 00b14ac0
CEventIcon::~CEventIcon()
{
}

// 00b14ad0
void CEventIcon::OnButtonDown()
{
	if (m_pHandler)
		m_pHandler();
}

// 00b14ae0
void CEventIcon::SetIconImage(FrButton& button)
{
	CTopIcon::SetIconImage(button);
}

// 00b14af0: "one_day_quest"
void EventIcon_OneDayQuest983()
{
	WSendPacket packet((enumClientPacket)0x151);
	packet.Send(TO_GAME);
}

// 00b15250: "CChristmas2011"
void EventIcon_Christmas2011983()
{
	FrChristmasSockEventDescDlg* pDlg =
		CreateForm<FrChristmasSockEventDescDlg>(g_pFresh->GetManager(), NULL, "christmasSockEvent", NULL);
	if (pDlg)
		pDlg->Open(NULL, 1);
}

// 00b152a0: "cardpack3rd" — a loja na primeira página
void EventIcon_CardPack3rd983()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x88, (int)"CShopMain", (int)"TOPPAGE", 0, 0, 0));
}

// 00b15300: "pointshop"
void EventIcon_PointShop983()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2e0, 0, 0, 0, 0, 0));
}

// 00b15360: "event_2013_workshop"
void EventIcon_Workshop2013983()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
	WSendPacket packet((enumClientPacket)0x172);
	packet.Send(TO_GAME);
}

// 00b15410: "event_2014_arin"
void EventIcon_Arin2014983()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
	WSendPacket packet((enumClientPacket)0x192);
	packet.Send(TO_GAME);
}

typedef std::map<std::string, CEventIcon::Handler> EventIconHandlerMap983;
static EventIconHandlerMap983 s_eventIconHandlers983; // 00f03814

// objeto estático vazio que registra uma função de evento pelo nome em minúsculas
struct sEventIconReg983
{
	sEventIconReg983(const char* name, CEventIcon::Handler handler);
};

// 00b166e0
sEventIconReg983::sEventIconReg983(const char* name, CEventIcon::Handler handler)
{
	std::string key;
	std::transform(name, name + strlen(name), std::inserter(key, key.begin()), tolower);
	s_eventIconHandlers983[key] = handler;
}

static sEventIconReg983 s_regChristmas2011983("CChristmas2011", EventIcon_Christmas2011983); // 00f03824
static sEventIconReg983 s_regCardPack3rd983("cardpack3rd", EventIcon_CardPack3rd983); // 00f03825
static sEventIconReg983 s_regPointShop983("pointshop", EventIcon_PointShop983); // 00f03826
static sEventIconReg983 s_regOneDayQuest983("one_day_quest", EventIcon_OneDayQuest983); // 00f03827
static sEventIconReg983 s_regWorkshop2013983("event_2013_workshop", EventIcon_Workshop2013983); // 00f03828
static sEventIconReg983 s_regArin2014983("event_2014_arin", EventIcon_Arin2014983); // 00f03829

// 00b167a0: o nome do elemento NÃO passa para minúsculas aqui (só no registro)
bool CEventIcon::Load(TiXmlElement* pElem)
{
	if (!CTopIcon::Load(pElem))
		return false;

	std::string name = pElem->Attribute("name").c_str();
	if (s_eventIconHandlers983.find(name) == s_eventIconHandlers983.end())
		return false;

	m_pHandler = s_eventIconHandlers983[name];
	return true;
}

//////////////////////////////////////////////////////////////////////////////// CWebIcon

// 00b16a70
void CWebIcon::SetIconImage(FrButton& button)
{
	button.SetVisible(true);
	button.SetPushSound("ui_desktop_icon_click");
	button.SetStyle(FrButton::BT_NORMAL);
	CTopIcon::SetIconImage(button);
}

// 00b16ab0
void CWebIcon::OpenUrl983()
{
	CWebPageConnector* pConnector = CWebPageConnector::Instance();
	if (pConnector)
	{
		if (COption::Instance())
			((COptionJp*)COption::Instance())->SetScreenSize983(800, 600);
		pConnector->RequestOpen983(m_url.c_str(), true, 0xb);
		g_input->SetMousePoint(400.0f, 325.0f);
	}
}

// 00b16b20
void CWebIcon::OnButtonDown()
{
	OpenUrl983();
}

// 00b16b30
bool CWebIcon::Load(TiXmlElement* pElem)
{
	if (!CTopIcon::Load(pElem))
		return false;

	m_url = pElem->Attribute("url");
	return m_url != "";
}
