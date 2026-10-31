#pragma once
// JP 983: unidade 004ca630–004cccd0 = ClubSetWorkshopChildWndBase (vtable 00d039b0, RTTI .?AVClubSetWorkshopChildWndBase@@) +
// ClubSetWorkshopHelper (vtable 00d039d4). O mapa da F3.7.1 cortava duas unidades; os métodos do Helper estão no meio da faixa
// da ChildWndBase, e o 1º método do Helper (004ca630) fica antes dela. Existem no 839 (pares 004ce740…004d0c00), não no 645.
// Layouts medidos nos construtores (004cb690, 004cc630) e destrutores. Nomes dos métodos e dos campos são meus (sessão B77).
#include <string>
#include <vector>
#include "clubsetworkshopform.h"

class FrWnd;
class IObject;
class WOverlay;
class CWindFont;
class CClubSetWorkShopCardNumFont;
struct sItemInfo;
struct sCards;

// Página (aba) de um diálogo da oficina: lista de controles do form que ela mostra/esconde junto. sizeof 0x28.
class ClubSetWorkshopChildWndBase
{
public:
	ClubSetWorkshopChildWndBase(); // 004cb690
	virtual ~ClubSetWorkshopChildWndBase(); // 004cb6f0 (com `delete`: 004cbb00)
	virtual void SetShow983(bool bShow); // 004ca660 (+4)
	virtual void OnBeginWork983(); // 004ca6c0 (+8): desliga as abas da FrClubSetWorkshopMainDlg
	virtual void OnEndWork983(); // 004ca6d0 (+0xc): liga as abas
	// +0x10…+0x1c: vazios, dobrados com outras classes (0049eb90 ret 4, 0049eba0 ret, 0049ebb0 ret 4, 0049ebc0 ret).
	// +0x14 e +0x1c: desenho do "area_npc_img*" e do "area_logo_*" das páginas (mapa da MainDlg; nomes da B78)
	virtual void Fn10_983(int) { }
	virtual void OnDrawNpcImg983() { }
	virtual void Fn18_983(int) { }
	virtual void OnDrawLogo983() { }

	bool IsShow983() const { return m_bShow; } // B79: o `mov al, [+8]; test al, al` do desenho das figuras
	FrWnd* AddCtrl983(IObject* pObj); // 004cbb30: cria pelo form (+0x9c) e guarda
	void AddCtrlByName983(std::string name); // 004cc110: controle já criado no form, pelo nome

protected:
	void Reset983()
	{
		m_pForm = NULL;
		m_bShow = false;
		m_npcImgName = NULL;
		m_logoName = NULL;
		m_wnds.clear();
		m_npcSpeechOverlay = 0;
	}
	FrWnd* FindCtrl983(std::string name) { return m_pForm->GetCtrl983<FrWnd>(name); }
	// embutidas que recebem o nome por valor e repassam ao form (B79: a cópia do nome antes da chamada; 0049e8f0 = uma
	// cópia fora de linha da EnableCtrl983, chamada pela LevelUp 004eb10a e pela RankUp 004c722c)
	template <class T>
	T* GetCtrl983(std::string name) { return m_pForm->GetCtrl983<T>(name); }
	bool ShowCtrl983(std::string name, bool bShow) { return m_pForm->ShowCtrl983(m_pForm, name, bShow); }
	bool EnableCtrl983(std::string name, bool bEnable) { return m_pForm->EnableCtrl983(m_pForm, name, bEnable); }
	// B80: mais três embutidas, todas fora de linha na LoadLayout983 004ec7b0 da LevelUp (0049e980, 004c12a0, 004c1470)
	bool OffsetCtrlLua983(std::string name, lua_system::lua_table_helper* pTbl, std::string key)
	{
		return m_pForm->OffsetCtrlLua983(name, pTbl, key);
	}
	bool OffsetCtrlLua983(std::string name, lua_system::lua_table_helper* pTbl, std::string keyX, std::string keyY)
	{
		return m_pForm->OffsetCtrlLua983(name, pTbl, keyX, keyY);
	}
	bool SetCtrlToolTip983(std::string name, std::string text) { return m_pForm->SetCtrlToolTip983(name, text); }
	// B83: fora de linha na unidade da OptionalFunctions (004c13b0): desloca pelo form (cópia do nome)
	bool OffsetCtrl983(std::string name, float x, float y) { return m_pForm->OffsetCtrl983(std::string(name), x, y); }

	ClubSetWorkShopForm* m_pForm; // +4
	bool m_bShow; // +8
	std::vector<FrWnd*> m_wnds; // +0xc
	const char* m_npcImgName; // +0x1c: figura do "area_npc_img*" (npc_img_filename do Lua; B79)
	const char* m_logoName; // +0x20: figura do "area_logo_*" (logo_img_filename)
	int m_npcSpeechOverlay; // +0x24: índice (Helper) da figura do balão "area_npc_speech_*" (img_npc_speech_filename)
};

// Figuras e fontes de uma página da oficina. sizeof 0x50.
class ClubSetWorkshopHelper
{
public:
	// figura carregada (0x24; cópia 004cb880)
	struct sOverlay983
	{
		sOverlay983() : bUse(true), alpha(0xff) { }
		std::string name; // +0
		WOverlay* pOverlay; // +0x1c
		bool bUse; // +0x20
		unsigned char alpha; // +0x21: opacidade 0…255 (SetOverlayAlpha983)
	};
	// moldura de três figuras (esquerda, meio esticado, direita); 0x20
	struct sFrame983
	{
		int left, mid, right; // índices em m_overlays
		int leftW, midW, rightW;
		int offsetX, offsetY; // +0x18/+0x1c: deslocamento somado ao (x, y) do DrawFrame983
	};

	ClubSetWorkshopHelper(); // 004cc630
	virtual ~ClubSetWorkshopHelper(); // 004cc6f0 (com `delete`: 004cc990)

	unsigned long GetTypeId983(unsigned int index); // 004ca630
	sItemInfo* FindMyItem983(unsigned int index); // 004cb280
	sCards* FindCard983(unsigned int index); // 004cb2d0
	sItemInfo* FindMyItemByGuid983(unsigned long guid); // 004cb320
	sItemInfo* FindClubSetByGuid983(unsigned long guid); // 004cb350
	WOverlay* FindOverlay983(std::string name); // 004cb170
	void DrawOverlay983(std::string name, int x, int y, int w); // 004cb5a0
	bool CreateFonts983(int count); // 004cb970
	int LoadOverlay983(std::string name, bool bUse); // 004cc9c0
	int LoadFrame983(std::string left, std::string mid, std::string right, int width, int offsetX, int offsetY); // 004ccaf0
	// fora da unidade (na da FrClubSetRankUpWnd, a 1ª que usa; B79): figura pelo índice e fonte dos números das cartas
	void DrawOverlayByIndex983(unsigned int index, int x, int y, int w); // 004ca410
	void SetOverlayUse983(unsigned int index, bool bUse); // 004ca4d0
	bool CreateCardNumFont983(); // 004c9b20: m_pCardNumFont = new CClubSetWorkShopCardNumFont (se não existe)
	void SetOverlayAlpha983(unsigned int index, float alpha); // 004ca510 (B81): 0…1 → alpha (0…255)
	void DrawFrame983(unsigned int index, int x, int y); // 004ca590 (B81): as três figuras da moldura
	CWindFont* GetFont983(unsigned int index) { return index < m_fonts.size() ? m_fonts[index] : NULL; }

protected:
	std::vector<unsigned long> m_typeIds; // +4
	std::vector<sOverlay983> m_overlays; // +0x14
	int m_lastOverlay; // +0x24
	std::vector<sFrame983> m_frames; // +0x28
	int m_lastFrame; // +0x38
	std::vector<CWindFont*> m_fonts; // +0x3c
	CClubSetWorkShopCardNumFont* m_pCardNumFont; // +0x4c (B79: era m_p4c)
};
