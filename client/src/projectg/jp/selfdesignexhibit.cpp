// JP 983: unidade 00b3b3a0–00b3d360 (FrSelfDesignAccept e FrSelfDesignExhibit, as janelas do pedido de venda do Self Design, e o
// corpo da sd_shop::CCommodityList, a lista de mercadorias do Self Design; ver include/jp/selfdesignexhibit.h). Classes novas do 983
// (existem no 839, não no 645). Sessão B128. Os moldes do std::map<unsigned long, 0x248> (nó 0x25c) e o CreateForm<FrSelfDesignExhibit>
// 00b3ca00 o compilador gera sozinho.
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frbutton.h"
#include "frviewer.h"
#include "frgraphicinterface.h"
#include "commonutil.h"
#include "packet.h"
#include "actor.h"
#include "taskmanager.h"
#include "shareddoc.h"
#include "../../shared/itemmanager.h"
#include "uccmanager.h"
#include "../../shared/uselzo.h"
#include "selfdesignexhibit.h"

extern Fresh* g_pFresh; // 00f02340

#ifndef KINDOF_CAST983
// DYNAMIC_CAST com o IsKindOf embutido (o 983 não chama IObject::DynamicCast aqui); nulo vai direto ao NULL
#define KINDOF_CAST983(type, obj) \
	((__rtti_obj = (void*)(obj)) ? (((IObject*)__rtti_obj)->IsKindOf(&type::m_RTTI) ? (type*)(obj) : NULL) : NULL)
#endif

// 00b3b3a0 (GetMessageMap); mapa 00e419b0 (5 entradas)
BEGIN_FRESH_MSGMAP(FrSelfDesignAccept, FrForm)
ON_FRESH_VI("view", FRCMD_INIT, FrSelfDesignAccept::OnViewInit)
ON_FRESH_VI("btn_confirm", FRCMD_INIT, FrSelfDesignAccept::OnConfirmInit)
ON_FRESH_VV("btn_confirm", FRCMD_LBUTTONDOWN, FrSelfDesignAccept::OnConfirmClick)
ON_FRESH_VI("btn_cancel", FRCMD_INIT, FrSelfDesignAccept::OnCancelInit)
ON_FRESH_VV("btn_cancel", FRCMD_LBUTTONDOWN, FrSelfDesignAccept::OnCancelClick)
END_FRESH_MSGMAP()

// 00b3b3c0 (GetMessageMap); mapa 00e419b8 (5 entradas)
BEGIN_FRESH_MSGMAP(FrSelfDesignExhibit, FrForm)
ON_FRESH_VI("draw_area", FRCMD_OWNERDRAW, FrSelfDesignExhibit::OnDrawAreaDraw)
ON_FRESH_VI("btn_confirm", FRCMD_INIT, FrSelfDesignExhibit::OnConfirmInit)
ON_FRESH_VV("btn_confirm", FRCMD_LBUTTONDOWN, FrSelfDesignExhibit::OnConfirmClick)
ON_FRESH_VI("btn_cancel", FRCMD_INIT, FrSelfDesignExhibit::OnCancelInit)
ON_FRESH_VV("btn_cancel", FRCMD_LBUTTONDOWN, FrSelfDesignExhibit::OnCancelClick)
END_FRESH_MSGMAP()

IMPLEMENT_OBJECT(FrSelfDesignAccept, FrForm)
IMPLEMENT_OBJECT(FrSelfDesignExhibit, FrForm)

// 00b3b630
void FrSelfDesignAccept::OnViewInit(int param)
{
	m_pView = KINDOF_CAST983(FrViewer, param);
	if (m_pView)
		m_pView->Open("selfdesign.jpg");
}

// 00b3b6b0
void FrSelfDesignAccept::OnConfirmInit(int param)
{
	m_pConfirmBtn = KINDOF_CAST983(FrButton, param);
	if (m_pConfirmBtn)
		m_pConfirmBtn->SetPushDelay(0.1f);
}

// 00b3b740
void FrSelfDesignAccept::OnCancelInit(int param)
{
	m_pCancelBtn = KINDOF_CAST983(FrButton, param);
	if (m_pCancelBtn)
		m_pCancelBtn->SetPushDelay(0.1f);
}

// 00b3b7d0
void FrSelfDesignExhibit::OnDrawAreaDraw(int param)
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device)
	{
		WRect rect = GetRect();
		device->Print(WPoint(rect.x + 150.0, rect.y + 70.0), 1,
			K2L_Compatibility("\xc0\xc7\xbb\xf3\xc6\xc7\xb8\xc5\xbd\xc5\xc3\xbb\xc0\xbb\xc7\xcf\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"));
	}
}

// 00b3b830
void FrSelfDesignExhibit::OnConfirmInit(int param)
{
	m_pConfirmBtn = KINDOF_CAST983(FrButton, param);
	if (m_pConfirmBtn)
		m_pConfirmBtn->SetPushDelay(0.1f);
}

// 00b3b8c0
void FrSelfDesignExhibit::OnCancelInit(int param)
{
	m_pCancelBtn = KINDOF_CAST983(FrButton, param);
	if (m_pCancelBtn)
		m_pCancelBtn->SetPushDelay(0.1f);
}

// 00b3bd70
FrSelfDesignAccept::~FrSelfDesignAccept()
{
}

// 00b3bdf0
FrSelfDesignExhibit::~FrSelfDesignExhibit()
{
}

namespace sd_shop
{
// Vista da B com os métodos que o 983 tem e a declaração da A (shareddoc.h) ainda não (pedidos C60.1 / B128.1). Mesmo layout,
// sem membros novos; quando a A declarar Clear/Load em CCommodityList, os corpos passam para ela e esta vista sai.
class CCommodityListJp983 : public CCommodityList
{
public:
	void Clear983(); // 00b3c640
	bool Load983(unsigned char* buf, int size); // 00b3cd60
};

#pragma pack(push, 1)
struct sCommodityRecord983 // registro de 0x11 bytes do bloco descomprimido
{
	sCommodityRecord983() { memset(this, 0, sizeof(*this)); }

	unsigned long typeId; // +0x00: peça (Part.iff)
	unsigned long commodityId; // +0x04: chave do mapa e id da roupa UCC
	char uccIndex[9]; // +0x08
};
#pragma pack(pop)
C_ASSERT(sizeof(sCommodityRecord983) == 0x11);

C_ASSERT(sizeof(sCommodity) == 0x248);

// 00b3c640: esvazia a lista sob a seção crítica
void CCommodityListJp983::Clear983()
{
	EnterCriticalSection(&m_cs);
	m_listVersion = 0;
	m_map.clear(); // 00b3c1f0
	LeaveCriticalSection(&m_cs);
}
}

// 00b3c670
FrSelfDesignAccept::FrSelfDesignAccept()
	: m_pConfirmBtn(NULL), m_pCancelBtn(NULL), m_pView(NULL)
{
	m_arg1 = 0;
	m_arg2 = 0;
	memset(m_uccIndex, 0, sizeof(m_uccIndex));
	m_text = "";
}

// 00b3c720
FrSelfDesignExhibit::FrSelfDesignExhibit()
	: m_pConfirmBtn(NULL), m_pCancelBtn(NULL)
{
	m_arg1 = 0;
	m_arg2 = 0;
	memset(m_uccIndex, 0, sizeof(m_uccIndex));
	m_text = "";
}

// 00b3c7d0: pedido de venda (pacote 0xfd) e fecha
void FrSelfDesignExhibit::OnConfirmClick()
{
	WSendPacket packet((enumClientPacket)0xfd);
	packet.Encode4(m_arg1);
	packet.Encode4(m_arg2);
	packet.EncodeStr(std::string(m_uccIndex));
	packet.EncodeStr(m_text);
	packet.Send(TO_GAME);
	AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x291, 0, 0, 0, 0, 0) << MsgObject(NULL, 1, 0, 0, 0, 0, 0);
	Close(true);
}

// 00b3cbe0 (chamada por 00620e00)
void FrSelfDesignAccept::SetData983(int arg1, int arg2, const char* uccIndex, const std::string& text)
{
	m_arg1 = arg1;
	m_arg2 = arg2;
	strcpy(m_uccIndex, uccIndex);
	m_text = text;
}

// 00b3ccc0
void FrSelfDesignExhibit::SetData983(int arg1, int arg2, const char* uccIndex, const std::string& text)
{
	m_arg1 = arg1;
	m_arg2 = arg2;
	strcpy(m_uccIndex, uccIndex);
	m_text = text;
}

namespace sd_shop
{
// 00b3cd60: descomprime e, para cada registro com peça conhecida, grava (commodityId -> peça + id + índice) no mapa; registro novo
// no mapa vira roupa UCC (CUccManager::AddClothes 007ee460 no slot 0, envia, sem forçar). Chamada pelo caso 0x1b1 do OnPacketCommon.
bool CCommodityListJp983::Load983(unsigned char* buf, int size)
{
	if (buf == NULL || size < 1)
		return false;
	CCompressBuffer cb;
	cb.Decompress(buf, size);
	int count = (int)(cb.GetNewSize() / sizeof(sCommodityRecord983));
	if (count < 1)
		return false;
	sCommodityRecord983* recs = new sCommodityRecord983[count];
	memcpy(recs, cb.GetNewBuffer(), cb.GetNewSize());
	EnterCriticalSection(&m_cs);
	for (int i = 0; i < count; ++i)
	{
		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(recs[i].typeId);
		if (pPart)
		{
			sCommodity entry;
			entry.part = *pPart;
			entry.commodityId = recs[i].commodityId;
			strcpy(entry.uccIndex, recs[i].uccIndex);
			std::pair<const unsigned long, sCommodity> kv(entry.commodityId, entry);
			if (m_map.insert(kv).second == true)
			{
				sUccClothes clothes;
				clothes.id = recs[i].commodityId;
				clothes.typeId = recs[i].typeId;
				strncpy(clothes.uccIndex, recs[i].uccIndex, 9);
				UccManager()->AddClothes(0, clothes, true, false);
			}
		}
	}
	LeaveCriticalSection(&m_cs);
	if (recs)
		delete[] recs;
	return true;
}
}

// 00b3d000: abre a FrSelfDesignExhibit com os mesmos dados e fecha
void FrSelfDesignAccept::OnConfirmClick()
{
	FrSelfDesignExhibit* pExhibit = CreateForm<FrSelfDesignExhibit>(g_pFresh->GetManager(), this, "selfdesign_exhibit", NULL);
	if (pExhibit)
	{
		pExhibit->SetData983(m_arg1, m_arg2, m_uccIndex, m_text);
		pExhibit->SetCaption(K2L_Compatibility("\xbc\xbf\xc7\xc1\xb5\xf0\xc0\xda\xc0\xce" "CP\xc6\xc7\xb8\xc5\xb5\xee\xb7\xcf\xbd\xc5\xc3\xbb"));
		pExhibit->Open(NULL, 0);
	}
	Close(true);
}

namespace sd_shop
{
// 00b3d160
CCommodityList::~CCommodityList()
{
	static_cast<CCommodityListJp983*>(this)->Clear983();
	DeleteCriticalSection(&m_cs);
}

// 00b3d270
CCommodityList::CCommodityList()
{
	InitializeCriticalSection(&m_cs);
	m_listVersion = 0;
	m_map.clear();
}
}
