#pragma once
#include "vc10_layout.h"
// JP 983: FrFormBase (unidade 004fb5f0–004ff080; vtables 00d05dbc / 00d05dac / 00d05da4; RTTI 00e74e04; mapa de mensagens
// 00e08570). Form genérico montado por script: guarda os controles criados pelo nome (`m_ctrls`, alimentado por
// `OnCtrlCreated983`, que as derivadas põem no mapa de mensagens), uma fila de figuras desenhadas sobre o controle "image"
// (`m_bitmaps`) e arruma "cancel2", "caption", "image", "message", "ok" e "cancel" (`Layout983`). Existe no 839 (par 00503020…),
// não no 645. sizeof 0x150 pelo `operator new` de 004fefd3; quatro virtuais novos depois dos da FrForm (+0x98…+0xa4).
// Nomes dos métodos e dos campos são meus (sessão B54).

#include <map>
#include <vector>
#include <string>
#include <memory>
#include "frform.h"

class Bitmap;

class FrFormBase : public FrForm
{
	DECLARE_OBJECT(FrFormBase) // GetRTTI 004fdc50; FrFormBaseMakeInstance 004fefb0; registro 004ff020

public:
	// figura da fila (0x10; `new` em 004fea90 / 004fec30)
	struct sBitmapInfo
	{
		sBitmapInfo() : pBmp(NULL), bOverlay(false) { }
		Bitmap* pBmp; // +0
		float x; // +4: deslocamento dentro da fila
		float y; // +8
		bool bOverlay; // +0xc: posta por cima de outra (004fec30)
	};
	typedef std::tr1::shared_ptr<sBitmapInfo> BitmapInfoPtr;

	FrFormBase(); // 004fe910
	virtual ~FrFormBase(); // 004fdb60 (com `delete`: 004fea60)

	virtual bool Close(bool bFade); // 004fdc90: só repassa
	virtual bool Open(FRESH_PFN_RESULT pFnResult, unsigned long flag); // 004fb600
	virtual bool Close(eFormRet result, bool bFade); // 004fdca0: só repassa
	virtual void SetMessage(const char* msg, bool spaceAlign); // 004fb690: só repassa
	virtual bool OnInit(); // 004fd380

protected:
	virtual void OnDraw(); // 004fdc60: só repassa
	virtual void OnProc(const float deltaTime); // 004fb640: NÃO chama a base; só repassa ao form filho
	virtual void OnOK(); // 004fdc70: só repassa
	virtual void OnCancel(); // 004fdc80: só repassa

public:
	virtual void SetSubForm983(FrFormBase* pForm); // 004fb670 (+0x98): nulo não troca
	virtual void ClearSubForm983(); // 004fdcb0 (+0x9c)
	virtual bool PreOpen983(); // 004fdcc0 (+0xa0): chamado pela Open e, no filho, pela OnInit do pai
	virtual void DrawBitmaps983(); // 004fd050 (+0xa4)

	// 004fedd0: handler das derivadas (45 mapas de mensagens o citam) — guarda o controle recém-criado pelo nome
	void OnCtrlCreated983(IObject* pObj);

	// 004fc2b0 (FrWnd) e 004fc410 (FrEdit): controle pelo nome, sem conferir o tipo
	template <class T>
	T* GetCtrl983(std::string name)
	{
		CtrlMap::iterator it = m_ctrls.find(name.c_str());
		if (it != m_ctrls.end())
			return (T*)it->second;
		return NULL;
	}
	// 004fdd60 … 004fe860 (17 tipos): guarda se o nome ainda não existe e o objeto é do tipo
	template <class T>
	T* AddCtrl983(const std::string& name, IObject* pObj)
	{
		CtrlMap::iterator it = m_ctrls.find(name);
		if (it == m_ctrls.end())
		{
			T* p = DYNAMIC_CAST(T, pObj);
			if (p)
			{
				m_ctrls[name] = p;
				return p;
			}
		}
		return NULL;
	}

	// 004fc7b0 / 004fc870 / 004fc930: recebem o form por argumento e não usam `this`
	bool ShowCtrl983(FrFormBase* pForm, std::string name, bool bShow);
	bool EnableCtrl983(FrFormBase* pForm, std::string name, bool bEnable);
	bool FadeOutCtrl983(FrFormBase* pForm, std::string name, bool bOut);
	bool SetCtrlToolTip983(std::string name, std::string text); // 004fc9f0
	bool MoveCtrl983(const std::string& name, float x, float y); // 004fcfb0

	BitmapInfoPtr AddBitmap983(Bitmap* pBmp); // 004fea90: no fim da fila
	BitmapInfoPtr AddOverlayBitmap983(Bitmap* pBase, Bitmap* pBmp, const WPoint& offset); // 004fec30: por cima de uma da fila

protected:
	void UpdateScreenSize983(); // 004fba30
	void SetSize983(float w, float h, bool bFlag); // 004fbd50: centra na tela
	void Layout983(); // 004fcb50

	typedef std::map<std::string, FrWnd*> CtrlMap;
	CtrlMap m_ctrls; // +0x110
	std::vector<BitmapInfoPtr> m_bitmaps; // +0x120
	float m_bmpTotalW; // +0x130: soma das larguras + 10 por figura
	float m_bmpMaxH; // +0x134
	float m_bmpNextX; // +0x138
	float m_screenW; // +0x13c
	float m_screenH; // +0x140
	FrFormBase* m_pSubForm; // +0x144
	bool m_b148; // +0x148: 3º argumento da SetSize983

	DECLARE_FRESH_MSGMAP() // GetMessageMap 004fb5f0; mapa 00e08570 (8 entradas, B1181)
};
C_ASSERT_VC10(sizeof(FrFormBase) == 0x150);
