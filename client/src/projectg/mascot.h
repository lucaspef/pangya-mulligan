#pragma once

#include "actor.h"
#include "scenemanager.h"
#include "npc.h"
#include "fxinterpreter.h"
#include "../shared/classdefine.h"

class WPuppet;
class CCameraBird;
struct sMascotInfo;

class CMascot : public IActor, public CRenderFuncPtr
{
public:
	virtual ~CMascot() { }

	DECLARE_OBJECT(CMascot)

	virtual void OnPreLoadInit();
	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnReload();
	virtual void OnProcess(float delta);
	virtual void OnDestroy();
	virtual void HandleMsg(const MsgObject& msg);

	virtual void Display();

	void ShowLuckyMsg983(unsigned long tid); // 983: 00762c40 (nome meu; HandleMsg 0x307, 5% pela tabela do CPowerGauge)

	static int ChangeMascotMsgInTex(const char* msg,
		const IFF_STRUCT::sMascot* pMascot, WPuppet* pPuppet,
		const char* texName);

	struct sMascot
	{
		sMascot()
			: pBird(NULL), pIff(NULL), pInfo(NULL)
		{
		}

		CCameraBird* pBird;
		WVector pos;
		IFF_STRUCT::sMascot* pIff;
		sMascotInfo* pInfo;
	};

protected:
	CMascot() { m_pFxInterpreter983 = new FxInterpreter(NULL, 0); } // 983: 00761d47..00761d63

	sMascot m_mascot[4];
	FxInterpreter* m_pFxInterpreter983; // 983: 0x7c
	int m_birdFxId983; // 983: 0x80 (00761b7e): id do *fx( das caixas do pássaro (ParseBoundingBoxScript_fx no 0x308, 00763162); sizeof 0x84
};

class CCameraBird : public CCreature
{
public:
	CCameraBird(const char* name, unsigned char index);

	virtual void Display();
	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

	void ChangePet(const char* name);

protected:
	unsigned char m_index;
	unsigned long m_playerInfo[6];
	WVector m_playerDir;
	WVector m_playerPos;
};
