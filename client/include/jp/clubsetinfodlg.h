#pragma once
#include "vc10_layout.h"
// JP 983: FrClubSetInfoDlg (unidade 00498810–00499800; vtables 00d00394 / 00d00384 / 00d0037c; RTTI 00e74248; mapa de mensagens
// 00e04a10 com 7 entradas). Ficha de um taco no meu quarto ("information", criada pelo CRealMyRoomMain 0061a9b0 com o
// CreateForm<FrClubSetInfoDlg> 0060bea0 da frente A, que chama SetItem983): figura, nome e nível pedido, nível, prazo, os
// bônus das cartas de atributo (slots +N, distância −N), descrição do .iff e o ícone "feito na oficina". Fecha sozinha
// (OnOK) quando a inicialização do taco (FrClubSetResetWnd) chega ao resultado. sizeof 0x140 pelo `operator new` de
// 00498b70. Existe no 839, não no 645. Nomes dos métodos e dos campos são meus (sessão B91).
#include "frform.h"

class FrArea;
class FrButton;
class FrEdit;
class FrStatic;
struct IFF_ITEM_COMMON;
struct sItemInfo;

class FrClubSetInfoDlg : public FrForm
{
	DECLARE_OBJECT(FrClubSetInfoDlg) // GetRTTI 00498880; MakeInstance 00498b70; registro 00498be0

public:
	FrClubSetInfoDlg(); // 00498820
	virtual ~FrClubSetInfoDlg(); // 00498890 (com `delete`: 00498c00; ajustes 00498b40 / 00498b50)

	virtual bool OnInit(); // 004988e0: só repassa

	// 00499730 (chamada em 0061a9e4): só tacos (tid 0x10xxxxxx); preenche a ficha inteira
	void SetItem983(sItemInfo* pInfo);

protected:
	virtual void OnProc(const float deltaTime); // 00498c30: OnOK quando o reset chega ao resultado (não chama a base)

	void SetPortrait983(IFF_ITEM_COMMON* pItem); // 004988f0: ícone do .iff no "portrait"
	void SetName983(IFF_ITEM_COMMON* pItem); // 00498920: "이름: %s" e "레벨: 이하/이상" no "name"
	void SetLevel983(IFF_ITEM_COMMON* pItem); // 004989f0: "level_%03d" no "level"
	const char* GetSlotText983(int slot, int value); // 00498a30: "<slot>+N" (vazio com 0)
	const char* GetDistanceText983(int value); // 00498ab0: "비거리-N" (vazio com 0)
	void SetDesc983(IFF_ITEM_COMMON* pItem); // 00498fc0: Desc.iff na mensagem do form
	void SetWorkshopIcon983(unsigned long clubGuid); // 00499100: taco mexido na oficina → "clubmade_icon_make"
	void SetCardText983(unsigned long clubTid, unsigned long clubGuid); // 00499210: cartas de atributo do taco no "avility"
	// 00499550: tipo de prazo, dias e horas restantes e "vencido" do taco no CSharedDoc::m_clubSetMap
	void FindClubPeriod983(unsigned long clubTid, unsigned char* pType, int* pDays, int* pHours, bool* pExpired);
	void SetPeriodText983(IFF_ITEM_COMMON* pItem); // 004995e0: prazo no "name"

	void OnPortraitInit983(int param); // 00498c50
	void OnNameInit983(int param); // 00498cc0
	void OnLevelInit983(int param); // 00498d30
	void OnAvilityInit983(int param); // 00498da0
	void OnStaticAvilityInit983(int param); // 00498e10
	void OnIconInit983(int param); // 00498e80
	void OnOkInit983(int param); // 00498ef0

	FrArea* m_pPortrait; // +0x110: "portrait"
	FrEdit* m_pName; // +0x114: "name"
	FrArea* m_pLevel; // +0x118: "level"
	FrEdit* m_pAvility; // +0x11c: "avility"
	FrStatic* m_pStaticAvility; // +0x120: "static_avility" (desce 25 quando há prazo, sobe 3 sem)
	FrArea* m_pIcon; // +0x124: "icon_clubsetworkshop"
	FrButton* m_pOk; // +0x128: "ok" (o construtor não zera)
	bool m_bPeriod; // +0x12c: o taco tem prazo
	unsigned long m_clubGuid; // +0x130
	unsigned char m_unused134983[0xc]; // +0x134: sem acesso no 983 (00498810..004999ff)

	DECLARE_FRESH_MSGMAP() // GetMessageMap 00498810; mapa 00e04a10
};
C_ASSERT_VC10(sizeof(FrClubSetInfoDlg) == 0x140);
