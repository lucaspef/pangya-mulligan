#pragma once
#include "vc10_layout.h"
// JP 983: ClubSetWorkShopForm (unidade 004ee570–004f3110; vtables 00d05694 / 00d05684 / 00d0567c; RTTI 00e74dd4; mapa de
// mensagens 00e07948, 7 entradas). Base dos diálogos da oficina de tacos (FrClubSetWorkshopMainDlg, FrClubSetWorkshopAboutDlg,
// FrClubSetResetDlgBase…). Irmã da FrFormBase (include/jp/formbase.h), sem o form filho e sem o tamanho da tela: guarda os
// controles criados pelo nome (`m_ctrls`) e a mesma fila de figuras sobre o controle "image". sizeof 0x140 pelo `operator new`
// de 004f2dd0. Três virtuais novos depois dos da FrForm (+0x98…+0xa0). Existe no 839 (par 004f6040…), não no 645.
// Nomes dos métodos e dos campos são meus (sessão B76).

#include <map>
#include <vector>
#include <string>
#include <memory>
#include "frform.h"

class Bitmap;
namespace lua_system { class lua_table_helper; }

// DYNAMIC_CAST com o IsKindOf embutido (o 983 não chama IObject::DynamicCast aqui); nulo vai direto ao NULL
#define KINDOF_CAST983(type, obj) \
	((__rtti_obj = (void*)(obj)) ? (((IObject*)__rtti_obj)->IsKindOf(&type::m_RTTI) ? (type*)(obj) : NULL) : NULL)

class ClubSetWorkShopForm : public FrForm
{
	DECLARE_OBJECT(ClubSetWorkShopForm) // GetRTTI 004f1440; MakeInstance 004f2dd0; registro 004f2e40

public:
	// figura da fila (0x10; `new` em 004f1470 / 004f1770)
	struct sBitmapInfo
	{
		sBitmapInfo() : pBmp(NULL), bOverlay(false) { }
		Bitmap* pBmp; // +0
		float x; // +4: deslocamento dentro da fila
		float y; // +8
		bool bOverlay; // +0xc: posta por cima de outra (004f1770)
	};
	typedef std::tr1::shared_ptr<sBitmapInfo> BitmapInfoPtr;

	ClubSetWorkShopForm(); // 004f1a60
	virtual ~ClubSetWorkShopForm(); // 004f1360 (com `delete`: 004f1b90)

	virtual bool Close(bool bFade); // 0049aae0 (dobrada com a FrClubSetResetDlgBase): só repassa
	virtual bool Close(eFormRet result, bool bFade); // 0049aaf0: só repassa
	virtual bool OnInit(); // 004f0110

protected:
	virtual void OnDraw(); // 004ee5b0: só repassa
	virtual void OnProc(const float deltaTime); // 004ee590: só repassa
	virtual void OnOK(); // 004ee5c0: só repassa
	virtual void OnCancel(); // 004ee5d0: só repassa

public:
	// 004ee570 (+0x98): handler das derivadas no mapa de mensagens (FRCMD_INIT); a FrClubSetWorkshopMainDlg troca
	virtual FrWnd* OnCtrlCreated983(IObject* pObj);
	// 004f2e60 (+0x9c): guarda o controle recém-criado pelo nome, conforme o tipo
	virtual FrWnd* AddCtrlByType983(IObject* pObj);
	virtual void DrawBitmaps983(); // 004efdc0 (+0xa0)

	// 0049b9f0 (FrWnd; instância dobrada, está na unidade da FrClubSetResetDlgBase): controle pelo nome, sem conferir o tipo
	template <class T>
	T* GetCtrl983(std::string name)
	{
		CtrlMap::iterator it = m_ctrls.find(name.c_str());
		if (it != m_ctrls.end())
			return (T*)it->second;
		return NULL;
	}
	// 004f1bc0 … 004f2cc0 (17 tipos): guarda (troca, se já existe) quando o objeto é do tipo
	template <class T>
	T* AddCtrl983(const std::string& name, IObject* pObj)
	{
		T* p = KINDOF_CAST983(T, pObj);
		if (!p)
			return NULL;
		m_ctrls[name.c_str()] = p;
		return p;
	}

	// 004efa40 / 004efb00: recebem o form por argumento e não usam `this`
	bool ShowCtrl983(ClubSetWorkShopForm* pForm, std::string name, bool bShow);
	bool EnableCtrl983(ClubSetWorkShopForm* pForm, std::string name, bool bEnable);
	bool SetCtrlToolTip983(std::string name, std::string text); // 004efbc0
	bool OffsetCtrl983(const std::string& name, float x, float y); // 004efd20: desloca (não é a MoveCtrl983 da FrFormBase)
	// 004f0120: x, y = [1], [2] da tabela filha `key`
	bool OffsetCtrlLua983(std::string name, lua_system::lua_table_helper* pTbl, std::string key);
	// 004f0230: x, y pelos nomes
	bool OffsetCtrlLua983(std::string name, lua_system::lua_table_helper* pTbl, std::string keyX, std::string keyY);

	BitmapInfoPtr AddBitmap983(Bitmap* pBmp); // 004f1470: no fim da fila
	BitmapInfoPtr AddOverlayBitmap983(Bitmap* pBase, Bitmap* pBmp, const WPoint& offset); // 004f1770: por cima de uma da fila

	void SetSize983(float w, float h, bool bFlag); // 004eea80: centra na área cliente da janela (pública: B79, a LevelUp chama)

protected:
	void Layout983(); // 004ef7f0

	typedef std::map<std::string, FrWnd*> CtrlMap;
	bool m_b110; // +0x110: 3º argumento da SetSize983
	CtrlMap m_ctrls; // +0x114
	std::vector<BitmapInfoPtr> m_bitmaps; // +0x124
	float m_bmpTotalW; // +0x134: soma das larguras + 10 por figura
	float m_bmpMaxH; // +0x138
	float m_bmpNextX; // +0x13c

	DECLARE_FRESH_MSGMAP() // GetMessageMap 004ee580; mapa 00e07948 (7 entradas: INIT dos controles, X, OWNERDRAW do "image")
};
C_ASSERT_VC10(sizeof(ClubSetWorkShopForm) == 0x140);
