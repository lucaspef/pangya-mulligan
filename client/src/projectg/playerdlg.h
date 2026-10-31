#pragma once

#include <stdlib.h>
#include <list>
#include <vector>
#include <map>

#include "../shared/classdefine.h"
#include "../shared/globalgamedefine.h"
#include "frform.h"

class WTitleFont;
class Bitmap;
class CExhibition;
class CPartTidList;
class FrPlayerEmptyDlg;
class FrPlayerDlg;
enum eLvlType;
enum PANGYA_ITEM_GROUP;
extern Fresh* g_pFresh;
class CEquipHelper
{
public:
	enum eCategory
	{
		CATEGORY_CHARACTER = 1,
		CATEGORY_CLUB = 4,
		CATEGORY_BALL = 5,
		CATEGORY_CADDIE = 7,
		CATEGORY_MASCOT = 0x10
	};

private:
	CEquipHelper();
	~CEquipHelper();

public:
	static CEquipHelper* Instance()
	{
		static CEquipHelper _Instance;
		return &_Instance;
	}

	IFF_ITEM_COMMON* GetItemCommon(eCategory category, const sUserEquip& equip);
	unsigned long GetEquipItemGuid(eCategory category, bool bRandom);
	unsigned long GetEquipItemTid(eCategory category, bool bRandom);
	unsigned long GetGuidFromTid(eCategory category, unsigned long tid);

private:
	template <class T>
	unsigned long RandSelectGuid(T& m)

	{
		if (m.size() == 0)
			return 0;

		typename T::iterator it = m.begin();

		int n = rand() % (int)m.size();
		for (int i = 0; i < n; ++i)
			++it;

		return it->first;
	}

	template <class T>
	unsigned long MapFindEquipTid(T& m, unsigned long guid)
	{
		if (guid == 0)
		{
			int size = (int)m.size();
			if (size == 0)
				return 0;
			typename T::iterator it = m.begin();

			int n = rand() % size;
			for (int i = 0; i < n; ++i)
				++it;

			return it->second.tid;
		}
		else
		{
			typename T::iterator it = m.find(guid);
			if (it != m.end())
				return it->second.tid;
		}
		return 0;
	}
};

class FrSubEquipBar : public FrForm
{
	DECLARE_OBJECT(FrSubEquipBar)

	enum eCategory
	{
		CATEGORY_CHARACTER = 1,
		CATEGORY_CLUB = 4,
		CATEGORY_BALL = 5,
		CATEGORY_CADDIE = 7,
		CATEGORY_MASCOT = 0x10
	};
	enum eSubType
	{
		SUBTYPE_0,
		SUBTYPE_1,
		SUBTYPE_2,
		SUBTYPE_3
	};

	struct _sortitem
	{
		_sortitem(IFF_ITEM_COMMON* item, unsigned long id)
			: pItem(item), guid(id)
		{
		}

		bool operator<(const _sortitem& rhs) const
		{
			if (pItem->TypeId == rhs.pItem->TypeId)
				return guid < rhs.guid;

			return pItem->TypeId < rhs.pItem->TypeId;
		}

		IFF_ITEM_COMMON* pItem;
		unsigned long guid;
	};

	FrSubEquipBar();
	virtual ~FrSubEquipBar();

	void Open(bool (FrCmdTarget::*pFnResult)(int, FrForm*), eCategory category,
		const WPoint& pos, eSubType subType);
	bool SetClientEquipInfo(PANGYA_ITEM_GROUP group, unsigned long guid);
	IFF_ITEM_COMMON* GetResult() { return m_pResult; }
	unsigned long GetLastSelectedGuid() { return m_lastSelectedGuid; }
	eCategory GetCategory() { return m_category; }

protected:
	virtual void OnDraw();
	virtual void OnProc(const float delta);
	virtual void OnOK();
	virtual void OnCancel();

	void OnInitSubEquipList(int param);
	void OnDownSubEquipList(int param);
	void OnDrawSubEquipList(int param);

private:
	void Initialize();

	std::list<_sortitem> m_sortList;
	IFF_ITEM_COMMON* m_pResult;
	FrListBox* m_pSubEquipList;
	eCategory m_category;
	eSubType m_subType;
	unsigned long m_lastSelectedGuid;

	DECLARE_FRESH_MSGMAP()
public:
	// 983 00b24120: guid do taco -> (carta de personagem, carta de caddie) desenhadas sobre o icone (vazia = card_club_*_empty)
	std::map<unsigned long, std::pair<const Bitmap*, const Bitmap*> > m_clubCardMap983; // 983 +0x130 (0x10: cabeca em +0x134)
	WTitleFont* m_pCountFont983; // 983 +0x140: numero de bolas (subtipo 1)
	char m_pad983_144[0xc]; // 983: sem leitura/escrita (ctor 00b23c70, dtor 00b23760, vtable d51e14, msgmap, SetClientEquipInfo, Open, 00b24120: so ate +0x140); sizeof 0x150 (push em 00b240b3)
};

class CStatBar
{
public:
	struct sBar
	{
		sBar(const char* name)
			: fCur(0.0f)
		{
			fLevel = 0.0f;
			fMax = 0.0f;
			pBitmap = g_pFresh->GetBitmap(name);
		}

		const Bitmap* pBitmap;
		float fCur;
		float fLevel;
		float fCapacity;
		float fMax;
		int penalty;
	};

	CStatBar(float gap, float height);
	~CStatBar();

	void SetRange(eLvlType type, int level, int capacity, int penalty);
	void SetPos(eLvlType type, int level, int capacity, int penalty);
	void SetDestTime(float time);
	void Process(float delta);
	void Render(const WPoint& pos);

private:
	std::vector<sBar> m_bars;
	float m_gap;
	float m_destTime;
	float m_height;
};

class FrPlayerDlg : public FrForm
{
	DECLARE_OBJECT(FrPlayerDlg)

	enum eButtonDir
	{
		BD_PREV,
		BD_NEXT,
		BD_CURRENT,
		BD_RANDOM
	};
	FrPlayerDlg()
		: m_pExhibition(NULL),
		  m_pPartTidList(NULL),
		  m_team(2),
		  m_index(-1),
		  m_pEmptyDlg(NULL)
	{
	}
	virtual ~FrPlayerDlg();

	void Init(int index);
	void SetTeam(unsigned char team);
	void Select();
	void UnSelect();
	void SetName(const char* name);
	const char* GetName();
	void SetState();
	void SetRandomState();
	void ShowPet(bool bShow);
	void HidePet();
	void OnCustomProc(float delta);

protected:
	void OnBaseInit(int param);
	void OnSelectInit(int param);
	void OnPetInit(int param);
	void OnCharPrevInit(int param);
	void OnCharNextInit(int param);
	void OnCaddieInit(int param);
	void OnCaddieLimitInit(int param);
	void OnCaddiePrevInit(int param);
	void OnCaddieNextInit(int param);
	void OnClubInit(int param);
	void OnClubLimitInit(int param);
	void OnClubPrevInit(int param);
	void OnClubNextInit(int param);
	void OnBallInit(int param);
	void OnBallPrevInit(int param);
	void OnBallNextInit(int param);
	void OnRemoveInit(int param);
	void OnNameInit(int param);
	void OnIndexInit(int param);
	void OnPowerBarInit(int param);
	void OnControlBarInit(int param);
	void OnImpactBarInit(int param);
	void OnSpinBarInit(int param);
	void OnCurveBarInit(int param);
	void OnPowerEditInit(int param);
	void OnControlEditInit(int param);
	void OnImpactEditInit(int param);
	void OnSpinEditInit(int param);
	void OnCurveEditInit(int param);
	void OnPetOwnerDraw();
	void OnBaseBtnUp();
	void OnRemoveBtnUp();
	void OnCharPrevBtnUp();
	void OnCharNextBtnUp();
	void OnCaddiePrevBtnUp();
	void OnCaddieNextBtnUp();
	void OnClubPrevBtnUp();
	void OnClubNextBtnUp();
	void OnBallPrevBtnUp();
	void OnBallNextBtnUp();

	void SetCharacter();
	void SetCaddie(unsigned long guid);
	void SetClub(unsigned long guid);
	void SetBall(unsigned long tid);
	void SetCharControl(eButtonDir dir);
	void SetCaddieControl(eButtonDir dir);
	void SetClubControl(eButtonDir dir);
	void SetBallControl(eButtonDir dir);
	void SetLevelBar();
	void SetLevelNum(unsigned char type, unsigned char level,
		unsigned char max);

	FrArea* m_pPet;
	FrButton* m_pCharPrev;
	FrButton* m_pCharNext;
	FrArea* m_pCaddie;
	FrArea* m_pCaddieLimit;
	FrButton* m_pCaddiePrev;
	FrButton* m_pCaddieNext;
	FrArea* m_pClub;
	FrArea* m_pClubLimit;
	FrButton* m_pClubPrev;
	FrButton* m_pClubNext;
	FrArea* m_pBall;
	FrButton* m_pBallPrev;
	FrButton* m_pBallNext;
	FrButton* m_pRemove;
	FrEdit* m_pName;
	FrArea* m_pIndex;
	FrButton* m_pBase;
	FrArea* m_pSelect;
	FrGaugeBar* m_pLevelBar[5];
	FrEdit* m_pLevelNum[5];
	CExhibition* m_pExhibition;
	CPartTidList* m_pPartTidList;
	unsigned char m_team;
	int m_index;
	FrPlayerEmptyDlg* m_pEmptyDlg;

	DECLARE_FRESH_MSGMAP()
};

class FrPlayerEmptyDlg : public FrForm
{
	DECLARE_OBJECT(FrPlayerEmptyDlg)

	FrPlayerEmptyDlg()
		: m_pPlayerDlg(NULL), m_index(-1)
	{
	}

	void Init(int index);

protected:
	void OnBaseInit(int param);
	void OnCreateInit(int param);
	void OnIndexInit(int param);
	void OnCreateBtnUp();

	FrButton* m_pCreate;
	FrArea* m_pIndex;
	FrArea* m_pBase;
	FrPlayerDlg* m_pPlayerDlg;
	int m_index;

	DECLARE_FRESH_MSGMAP()
};
