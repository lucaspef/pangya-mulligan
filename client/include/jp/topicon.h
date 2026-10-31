#pragma once
#include "vc10_layout.h"
// JP 983: ícones da área de trabalho do lobby (os "top icons" do 645 refeitos). No 983 NÃO existem `CIconInterface`,
// `CWizcityOpenEvent` nem `CWebIcons_WebEvent1/2` (sem RTTI); os ícones vêm do XML do tema baixado pelo `CExtraResource`
// (`<topicons>`, 00b149f0), um elemento por ícone, e a classe sai do atributo de tipo (fábrica 006b15c0). Classes da RTTI do
// 983: `CTopIcon` (vtable 00d19ca4, 0x94), `CWebIcon` (00d19cb8, 0xb0), `CDescImageOnlyIcon` (00d513c0 + 00d513b4, 0xc8),
// `CEventIcon` (00d513e4, 0x98) e `CIconManager` (00d513dc; instância 00e74d88). Substitui, no 983, o topiconinterface.h /
// topiconmanager.h / topicons.h / webicons.h do 645 (frente A): pedido B56 em out/PEDIDOS.md. Sessão B56; nomes de método
// e de campo meus, menos os que o 645 já tinha (InitalizeTopIcon, OnButtonDown, SetIconImage).

#include <string>
#include <vector>
#include "engine/singleton.h"
#include "frcmdtarget.h"

class FrButton;
class FrForm;
class FrNoticeDlg;
class TiXmlElement;
class WRect;

// unidade 006b12a0–006b1a50 (sem arquivo ainda; vive no meio da faixa da trajectorypreviewcontroller)
class CTopIcon
{
public:
	CTopIcon(); // 006b1390
	virtual ~CTopIcon(); // 006b13e0 (com `delete`: 006b1530)

	virtual void OnButtonDown() = 0;
	// 006b12a0: as quatro imagens que não estão vazias (estados 0–3) e o botão guardado; sem a 4ª, desliga o pisca
	virtual void SetIconImage(FrButton& button);
	// 006b1830: atributo "name" e os quatro <normal>/<over>/<selected>/<blink> (baixados pelo CExtraResource)
	virtual bool Load(TiXmlElement* pElem);

	// 006b15c0: cria o ícone pelo tipo do elemento (CEventIcon, CWebIcon ou CDescImageOnlyIcon) e chama `Load`
	static CTopIcon* Create(TiXmlElement* pElem);

	// cópia fora de linha em 00b147c0
	std::string GetName() const { return m_name; }
	// cópia fora de linha em 00b13f40
	FrButton* GetButton() const { return m_pButton; }

protected:
	void StripExt983(std::string& file); // 006b1490: tira a extensão (do último ".")

	std::string m_name; // +0x04
	std::string m_normal; // +0x20
	std::string m_over; // +0x3c
	std::string m_selected; // +0x58
	std::string m_blink; // +0x74
	FrButton* m_pButton; // +0x90
};

class CWebIcon : public CTopIcon
{
public:
	CWebIcon() {} // embutido no Create (cópia fora de linha 006b14c0)
	virtual ~CWebIcon() {} // embutido no `delete` 006b1560 (cópia fora de linha 006b14f0)

	virtual void OnButtonDown(); // 00b16b20: só um `jmp` para a OpenUrl983
	virtual void SetIconImage(FrButton& button); // 00b16a70
	virtual bool Load(TiXmlElement* pElem); // 00b16b30: + atributo "url"; falso se vier vazio

protected:
	void OpenUrl983(); // 00b16ab0: tela em 800×600 e abre o "url" pelo CWebPageConnector

	std::string m_url; // +0x94
};

// ícone que abre o "notice_popup" com uma ou mais imagens (<file>) e o título de <caption name=…>
class CDescImageOnlyIcon : public CTopIcon, public FrCmdTarget
{
public:
	CDescImageOnlyIcon(); // 00b13ce0
	virtual ~CDescImageOnlyIcon(); // 00b13d90 (com `delete`: 00b13e30; pela base FrCmdTarget: 00b13d80)

	virtual void OnButtonDown(); // 00b13b40
	virtual void SetIconImage(FrButton& button); // 00b13b00
	virtual bool Load(TiXmlElement* pElem); // 00b13e60

protected:
	bool LoadCaption(TiXmlElement* pElem); // 00b13c20
	bool OnNoticeResult(int result, FrForm* pForm); // 00b13b30: esquece o diálogo

	std::string m_caption; // +0x98
	std::vector<std::string> m_files; // +0xb4
	FrNoticeDlg* m_pNoticeDlg; // +0xc4
};

// ícone de evento: o `Load` (00b167a0) escolhe pelo atributo "name" uma função de uma tabela estática (`std::map` em
// 00f03814, preenchida por seis objetos estáticos de registro 00f03824…00f03829, nomes em minúsculas); unidade eventicon
class CEventIcon : public CTopIcon
{
public:
	typedef void (*Handler)();

	CEventIcon(); // 00b14aa0
	virtual ~CEventIcon(); // 00b14ac0 (com `delete`: 00b14ed0)

	virtual void OnButtonDown(); // 00b14ad0: chama a função, se houver
	virtual void SetIconImage(FrButton& button); // 00b14ae0: só a da base
	virtual bool Load(TiXmlElement* pElem); // 00b167a0

protected:
	Handler m_pHandler; // +0x94
};

class CIconManager : public WSingleton<CIconManager>
{
public:
	CIconManager(); // 00b147f0
	virtual ~CIconManager(); // 00b14840 (com `delete`: 00b149c0)

	void InitalizeTopIcon(unsigned long index, int wnd); // 00b144d0
	void OnButtonDown(unsigned long index); // 00b14320
	void UpdateLayout983(); // 00b14290: repõe os botões quando a tela muda de tamanho (chamada de 006035b0)
	std::string GetIconName983(unsigned long index) const; // 00b148f0
	void LoadExtra983(TiXmlElement* pElem); // 00b149f0: um ícone por filho de <topicons>

protected:
	void CalcRect983(unsigned long index, WRect* pRect); // 00b141f0: colunas de 80 a partir de (5, 185)

	std::vector<CTopIcon*> m_icons; // +0x28
};

C_ASSERT_VC10(sizeof(CTopIcon) == 0x94);
C_ASSERT_VC10(sizeof(CWebIcon) == 0xb0);
C_ASSERT_VC10(sizeof(CDescImageOnlyIcon) == 0xc8);
C_ASSERT_VC10(sizeof(CEventIcon) == 0x98);
C_ASSERT_VC10(sizeof(CIconManager) == 0x38);
