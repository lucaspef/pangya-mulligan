// JP 983: unidade 005832d0–005846c0 (FrItemSearchDlg, a busca de itens por nome; ver include/jp/itemsearchdlg.h).
// Classe nova do 983 (existe no 839, não no 645). Sessão B120.
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frbutton.h"
#include "fredit.h"
#include "frcombobox.h"
#include "frlistbox.h"
#include "frgraphicinterface.h"
#include "wlocalize.h"
#include "../../shared/itemmanager.h"
#include "actor.h"
#include "taskmanager.h"
#include "shareddoc.h"
#include "itemsearchdlg.h"
#include "createcharacteroption.h"

extern Fresh* g_pFresh; // 00f02340

extern const WRect g_wrectZero983; // 00f14668 (o mesmo g_lobbyRectZero983 da lobbyplayer.cpp)

// FrComboBox do 983 (frcombobox.cpp, frente A): escolhe a linha com o texto (SetLine pela virtual +0x60); falso se não achar
class FrComboBox983 : public FrComboBox
{
public:
	bool SelectString983(const char* string); // 00bff2b0
};

bool FrComboBox983::SelectString983(const char* string)
{
	for (std::list<FrLine*>::iterator it = m_comboList.begin(); it != m_comboList.end(); ++it)
	{
		if ((*it)->text == string)
		{
			m_selected = it;
			SetLine(1, (*it)->text.c_str(), 0, false, 0);
			return true;
		}
	}
	return false;
}

// tabela dos grupos (00e0cd60, 6 × 0x20; inicializador 00ccf150, destrutor 00cf9ca0)
struct sItemSearchGroup983
{
	int id;
	std::string name;
};

sItemSearchGroup983 g_itemSearchGroups983[6] = {
	{ 0, "\xc0\xc7\xbb\xf3" }, // 의상
	{ 1, "\xbe\xc6\xc0\xcc\xc5\xdb" }, // 아이템
	{ 2, "\xc4\xb3\xb5\xf0" }, // 캐디
	{ 3, "\xb8\xb6\xbd\xba\xc4\xda\xc6\xae" }, // 마스코트
	{ 4, "\xb2\xd9\xb9\xcc\xb1\xe2" }, // 꾸미기
	{ 5, "\xb9\xe6\xb2\xd9\xb9\xcc\xb1\xe2" }, // 방꾸미기
};

#ifndef KINDOF_CAST983
// DYNAMIC_CAST com o IsKindOf embutido (o 983 não chama IObject::DynamicCast aqui); nulo vai direto ao NULL
#define KINDOF_CAST983(type, obj) \
	((__rtti_obj = (void*)(obj)) ? (((IObject*)__rtti_obj)->IsKindOf(&type::m_RTTI) ? (type*)(obj) : NULL) : NULL)
#endif

IMPLEMENT_OBJECT(FrItemSearchDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrItemSearchDlg, FrForm)

ON_FRESH_VI("combo_SearchItemGroup", FRCMD_INIT, FrItemSearchDlg::OnGroupInit)
ON_FRESH_VV("combo_SearchItemGroup", FRCMD_LBUTTONDOWN, FrItemSearchDlg::OnGroupLBtnDown)
ON_FRESH_VI("edit_SearchWord", FRCMD_INIT, FrItemSearchDlg::OnWordInit)
ON_FRESH_VI("edit_SearchWord", FRCMD_ENTERKEY, FrItemSearchDlg::OnWordEnterKey)
ON_FRESH_VI("btn_Search", FRCMD_INIT, FrItemSearchDlg::OnSearchInit)
ON_FRESH_VV("btn_Search", FRCMD_LBUTTONUP, FrItemSearchDlg::OnSearchBtnUp)
ON_FRESH_VI("list_Result", FRCMD_INIT, FrItemSearchDlg::OnResultInit)
ON_FRESH_VV("list_Result", FRCMD_LBUTTONUP, FrItemSearchDlg::OnResultLBtnUp)
ON_FRESH_VI("list_Result", FRCMD_OWNERDRAW, FrItemSearchDlg::OnResultDraw)

END_FRESH_MSGMAP()

// aviso de busca sem resultado (00e75cf8; inicializador 00ccf410, destrutor 00cf9c70)
std::string g_itemSearchNoResult983 =
	K2L_Compatibility("\xb0\xcb\xbb\xf6\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."); // 검색된 아이템이 없습니다.

// 005833a0
void FrItemSearchDlg::OnGroupInit(int param)
{
	m_pGroup = KINDOF_CAST983(FrComboBox, param);
}

// 00583410
void FrItemSearchDlg::OnWordInit(int param)
{
	m_pWord = KINDOF_CAST983(FrEdit, param);
}

// 00583480
void FrItemSearchDlg::OnSearchInit(int param)
{
	m_pSearch = KINDOF_CAST983(FrButton, param);
}

// 005834f0
void FrItemSearchDlg::OnResultInit(int param)
{
	m_pResult = KINDOF_CAST983(FrListBox, param);
}

// 00583630: combo com os 6 grupos, o 1º escolhido
bool FrItemSearchDlg::OnInit()
{
	m_pSelBmp = g_pFresh->RegisterBitmap("30_noplayer_base");
	if (m_pGroup)
		m_pGroup->ClearList();
	for (unsigned int i = 0; i < sizeof(g_itemSearchGroups983) / sizeof(g_itemSearchGroups983[0]); i++)
		m_pGroup->AddString(K2L_Compatibility(g_itemSearchGroups983[i].name.c_str()));
	((FrComboBox983*)m_pGroup)->SelectString983(K2L_Compatibility(g_itemSearchGroups983[0].name.c_str()));
	return FrForm::OnInit();
}

// 005836c0
void FrItemSearchDlg::SendPick983(const sItemSearchResult983* p)
{
	if (p)
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2d6, p->typeId, p->uid, p->linkedTypeId, 0, 0));
}

// 00583730
void FrItemSearchDlg::SendPick2_983(unsigned long typeId, unsigned long uid)
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2d7, typeId, uid, 0, 0, 0));
}

// 00583810
void FrItemSearchDlg::OnResultLBtnUp()
{
	FrListItem* pItem = m_pResult->GetItemUnderCursor();
	if (pItem)
	{
		sItemSearchResult983* p = (sItemSearchResult983*)pItem->pData;
		SendPick983(p);
		SendPick2_983(p->typeId, p->uid);
	}
}

// 00583a00
void FrItemSearchDlg::AddResults983(std::list<sItemSearchResult983>& results)
{
	if (m_pResult)
	{
		for (std::list<sItemSearchResult983>::iterator it = results.begin(); it != results.end(); ++it)
			m_pResult->AddItem(&*it);
	}
}

// 00583a40: rosto do personagem (peça, personagem, caddie, cabelo; os outros tipos usam o índice 0xf) e o ícone do item, na altura
// da linha (figura / 1,4, no máximo 1:1); o ícone de item (tipo 6) fica 1,5 vez maior
void FrItemSearchDlg::DrawItemIcon983(unsigned long typeId, const WPoint& pos)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (!typeId)
		return;
	IFF_ITEM_COMMON* pIff = ItemManager()->FindCommonItem(typeId);
	if (!pIff)
		return;
	std::string face("");
	std::string icon(pIff->Icon);
	int charIndex = 0xf;
	switch (typeId >> 26)
	{
	case 1:
		charIndex = typeId & 0x3ffffff;
		break;
	case 2:
		charIndex = (typeId >> 18) & 0xff;
		break;
	case 9:
		charIndex = (typeId >> 13) & 0xff;
		break;
	case 0xf:
		charIndex = ItemManager()->FindHairStyle(typeId)->cCharID;
		break;
	}
	face = CreateCharacterOption::Instance()->GetBarIcon983(charIndex);
	if (!face.empty())
	{
		const Bitmap* pBmp = g_pFresh->GetBitmap(face.c_str());
		if (pBmp)
		{
			m_faceRc.x = pos.x;
			m_faceRc.y = pos.y + 2.0;
			float scale = m_pResult->GetItemHeight() / (pBmp->Height() * 1.4f);
			scale = scale < 1.0f ? scale : 1.0f;
			m_faceRc.w = pBmp->Width() * scale;
			m_faceRc.h = pBmp->Height() * scale;
			pGI->DrawTexture(pBmp, m_faceRc, 0xffffffff, 0);
		}
	}
	if (!icon.empty())
	{
		const Bitmap* pBmp = g_pFresh->GetBitmap(icon.c_str());
		if (pBmp)
		{
			m_iconRc.x = pos.x;
			m_iconRc.y = pos.y + 2.0;
			float scale = m_pResult->GetItemHeight() / (pBmp->Height() * 1.4f);
			scale += (typeId >> 26) == 6 ? scale * 0.5 : 0.0;
			scale = scale < 1.0f ? scale : 1.0f;
			m_iconRc.x += m_faceRc.w + scale * 2.0;
			m_iconRc.w = pBmp->Width() * scale;
			m_iconRc.h = pBmp->Height() * scale;
			pGI->DrawTexture(pBmp, m_iconRc, 0xffffffff, 0);
		}
	}
}

// 00583e40: linha da lista — fundo sob o cursor, figuras, nome (azul e estilo 2 sob o cursor) e o item ligado à direita
void FrItemSearchDlg::OnResultDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (!pGI)
		return;
	unsigned long color = pGI->GetTextColor();
	sItemSearchResult983* p = (sItemSearchResult983*)pItem->pData;
	if (!p->typeId)
		return;
	if (pItem->underCursor)
		pGI->DrawTexture(m_pSelBmp, WRect(WPoint(pItem->pos.x - 5.0, pItem->pos.y - 3.0), 298.0f, 31.0f), 0xffffffff, 0);
	DrawItemIcon983(p->typeId, pItem->pos);
	pGI->SetTextColor(0xff000000, 0xffffffff);
	if (pItem->underCursor == true)
	{
		pGI->SetTextColor(0xff0000ff, 0xffffffff);
		pGI->SetTextStyle(2);
	}
	int h = m_pResult->GetItemHeight();
	float textOff = m_iconRc.w + m_faceRc.w + 10.0;
	float x = pItem->pos.x + textOff;
	float y = (float)(h / 2 - pGI->GetFontHeight11() / 2) + pItem->pos.y;
	pGI->Print11(WPoint(x, y), 0, p->name.c_str());
	if (p->linkedTypeId)
	{
		IFF_ITEM_COMMON* pLinked = ItemManager()->FindCommonItem(p->linkedTypeId);
		if (pLinked)
		{
			float w = pGI->GetTextExtendOld11_983(p->name.c_str());
			WPoint pos(w + x + 10.0, pItem->pos.y);
			WPoint text(pos.x + textOff, y);
			DrawItemIcon983(pLinked->TypeId, pos);
			pGI->Print11(text, 0, pLinked->Name);
		}
	}
	pGI->SetTextColor(color, 0xffffffff);
	pGI->SetTextStyle(0);
}

// 00584190 (também o LBUTTONDOWN do combo, 00584410)
void FrItemSearchDlg::ClearResult983()
{
	if (m_results.size())
		m_results.clear();
	if (m_pResult && m_pResult->GetCurrentItemSize(false))
		m_pResult->ClearItem();
	m_faceRc = g_wrectZero983;
	m_iconRc = g_wrectZero983;
}

// 00584250
FrItemSearchDlg::FrItemSearchDlg()
	: m_pGroup(NULL), m_pWord(NULL), m_pSearch(NULL), m_pResult(NULL)
{
	m_results.clear();
	m_faceRc = g_wrectZero983;
	m_iconRc = g_wrectZero983;
	m_pSelBmp = NULL;
}

// 005843c0
FrItemSearchDlg::~FrItemSearchDlg()
{
	m_pSelBmp = NULL;
}

// 00584410
void FrItemSearchDlg::OnGroupLBtnDown()
{
	ClearResult983();
}

// 00584420: o ator principal enche m_results (0x2d3: grupo, palavra, lista); vazia = aviso 0x24
void FrItemSearchDlg::Search983(const std::string& word, int group)
{
	ClearResult983();
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2d3, group, (int)&word, (int)&m_results, 0, 0));
	if (m_results.size() == 0)
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x24, (int)g_itemSearchNoResult983.c_str(), 0, 0, 0, 0));
	else
		AddResults983(m_results);
}

// 005845d0: palavra de 2 letras ou mais; grupo = índice do texto do combo − 1
void FrItemSearchDlg::OnSearchBtnUp()
{
	ClearResult983();
	std::string word(m_pWord->GetLine(1, false));
	if (!word.size() || word.size() < 2)
		return;
	Search983(word, m_pGroup->FindString(m_pGroup->GetLine(1, false)) - 1);
}

// 005846b0
void FrItemSearchDlg::OnWordEnterKey(int param)
{
	OnSearchBtnUp();
}
