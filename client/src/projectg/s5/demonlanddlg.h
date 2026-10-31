#pragma once

#include "frform.h"
#include "treasuredlg.h"

class FrArea;
class FrGaugeBarImage;
class FrListBox;
class FrStatic;
class WOverlay;
class Bitmap;

enum eDemonDlg
{
	DEMONDLG_NEXTHOLE,
	DEMONDLG_ATTACKGAUGE,
	DEMONDLG_PROCESSRANK,
	DEMONDLG_FINALSCORE
};

class FrDemonNextHole : public FrForm
{
	DECLARE_OBJECT(FrDemonNextHole)

	FrDemonNextHole();
	virtual ~FrDemonNextHole();

	virtual bool OnInit();

	void SetState(unsigned char state);
	void SetNextHole(int hole);

protected:
	void OnAreaPattern_Init(int param);
	void OnAreaPattern_OwnerDraw(int param);
	void OnAreaNextHole_Init(int param);
	void OnAreaNextHole_OwnerDraw(int param);

	FrArea* m_pAreaPattern;
	FrArea* m_pAreaNextHole;
	WOverlay* m_pOverlay[2];
	_WRECT m_uvRect;
	const Bitmap* m_pPatternBmp;
	int m_texWidth;
	int m_texHeight;
	unsigned long m_state;
	int m_overlayIndex;
	int m_unused144;

	DECLARE_FRESH_MSGMAP()
};

class FrDemonAttackGauge : public FrForm
{
	DECLARE_OBJECT(FrDemonAttackGauge)

	FrDemonAttackGauge();
	virtual ~FrDemonAttackGauge();

	virtual bool OnInit();

	void SetState(unsigned char state);

protected:
	void OnImageBarAttackPoint_Init(int param);
	void OnAreaAttackMarker_Init(int param);
	void OnAreaAttackMarker_OwnerDraw(int param);
	void OnAreaBlingBar_Init(int param);
	void OnAreaBlingBar_OwnerDraw(int param);

	FrGaugeBarImage* m_pImageBar;
	FrArea* m_pAreaAttackMarker;
	FrArea* m_pAreaBlingBar;
	const Bitmap* m_pAttackPointBmp;
	const Bitmap* m_pGaugeLightBmp;
	unsigned long m_state;

	DECLARE_FRESH_MSGMAP()
};

class FrDemonProcessRank : public FrForm
{
	DECLARE_OBJECT(FrDemonProcessRank)

	FrDemonProcessRank();
	virtual ~FrDemonProcessRank();

	virtual bool OnInit();

protected:
	void OnAreaLeaderPet_Init(int param);
	void OnAreaLeaderPet_OwnerDraw(int param);
	void OnListBoxUserInfoList_Init(int param);
	void OnListBoxUserInfoList_OwnerDraw(int param);

	FrArea* m_pAreaLeaderPet;
	FrListBox* m_pUserInfoList;
	int m_unused118;
	int m_unused11c;

	DECLARE_FRESH_MSGMAP()
};

class FrDemonFinalScore : public FrForm
{
	DECLARE_OBJECT(FrDemonFinalScore)

	FrDemonFinalScore();
	virtual ~FrDemonFinalScore();

protected:
	virtual bool OnInit();
	virtual void OnProc(const float dt);

	void OnImageBarAttackPoint_Init(int param);
	void OnInit_Gift_List(int param);
	void OnOwnerDraw_Gift_List(int param);

	FrGaugeBarImage* m_pImageBar;
	CBoxOpener m_boxOpener;

	DECLARE_FRESH_MSGMAP()
};

class FrDemonLandTicket : public FrForm
{
	DECLARE_OBJECT(FrDemonLandTicket)

	FrDemonLandTicket();
	virtual ~FrDemonLandTicket();

	void SetTicketNumber(int number);

protected:
	void OnInit_TicketMsg_Static(int param);

	FrStatic* m_pTicketMsg;
	int m_ticketNumber;

	DECLARE_FRESH_MSGMAP()
};

class CDemonLandForm
{
public:
	CDemonLandForm();
	~CDemonLandForm();

	bool CreateDemonLandForm(eDemonDlg dlg);
	void InitializeForm();
	void AdjustWindowPos();

	FrDemonNextHole* GetDemonNextHole();
	FrDemonAttackGauge* GetDemonAttackGauge();
	FrDemonProcessRank* GetDemonProcessRank();
	FrDemonFinalScore* GetDemonFinalScore();

private:
	void ReleaseForm();
	void AdjustNextHolePos(WRect& rect);
	void AdjustAttackGaugePos(WRect& rect);

	FrDemonNextHole* m_pNextHole;
	FrDemonAttackGauge* m_pAttackGauge;
	FrDemonProcessRank* m_pProcessRank;
	FrDemonFinalScore* m_pFinalScore;
};
