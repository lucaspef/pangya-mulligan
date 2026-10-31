#pragma once

#include <map>

#include "frform.h"

class CWindFont;
class CExhibition;
class CParticle2D;
class FrButton;
class WReceivedPacket;

class FrCardBoxDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrCardBoxDlg)

	FrCardBoxDlg();
	virtual ~FrCardBoxDlg();

	virtual void Init(unsigned long typeId, unsigned long uid, int count);
	void RecievedPacket(WReceivedPacket* pPacket);

protected:
	DECLARE_FRESH_MSGMAP()

	virtual bool OnInit();
	virtual void OnProc(const float dt);
	virtual void OnOK();
	virtual void OnCancel();

	void OnResultBtnInit(int param);
	void OnResultBtnUp();
	void OnCloseBtnInit(int param);
	void OnCloseBtnUp();
	void OnAreaBgInit(int param);
	void OnAreaBgOwnerDraw();
	void OnPackOpenBtnInit(int param);
	void OnPackOpenBtnUp();
	void OnOpenNextBtnInit(int param);
	void OnOpenNextBtnUp();

private:
	enum eANIMATION
	{
		ANI_BOX_OPEN,
		ANI_PACK_DOWN,
		ANI_PACK_TURN,
		ANI_PACK_WAIT,
		ANI_PACK_OPEN,
		ANI_ZOOM_IN,
		ANI_RESULT,
		ANI_CARD_COVER
	};

	void SetAnimation(unsigned long animation);
	void Display2DBg();
	void LoadCardBoxPet(unsigned long typeId, int count);
	void LoadCardPackPet(unsigned long typeId);
	void SetPackVisibleInBox();

	CWindFont* m_pFont;
	CExhibition* m_pExhibition;
	WRect m_bgRect;
	int m_animation;
	FrButton* m_pPackOpenBtn;
	FrButton* m_pOpenNextBtn;
	FrButton* m_pCloseBtn;
	FrButton* m_pResultBtn;
	unsigned long m_typeId;
	unsigned long m_uid;
	WRect m_packOpenRect;
	float m_packOpenShake;
	std::map<unsigned long, unsigned int> m_resultMap;
	CParticle2D* m_pParticle;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnresultbtnupFunclets(); // HACK: EH funclets of the naked OnResultBtnUp (nakedify)
#endif
};
