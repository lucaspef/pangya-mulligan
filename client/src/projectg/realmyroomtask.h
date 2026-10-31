#pragma once
#include <list>
#include "taskmanager.h"
#include "singleton.h"

class CCameraWork;
class CRealMyRoom_Avatar;
class CPetFrame;

class CRealMyRoom_AvatarBurden
{
public:
	~CRealMyRoom_AvatarBurden();
	bool LoadAvatarBurden();

	// D717: tamanho do real (realmyroom_avatar.h RMRAVATAR_BURDEN_SIZE 0x2d4, pet em +0x2d0); com 0x290/+0x288 os nos das listas
	// ficavam 0x44 B curtos e o LoadAvatarBurden real (OnAnotherHand) escrevia m_partTidList/m_pPetFrame fora do no
	int m_guid;
	unsigned char m_unused4[0x2cc];
	CPetFrame* m_pPetFrame;
};
C_ASSERT(sizeof(CRealMyRoom_AvatarBurden) == 0x2d4 && offsetof(CRealMyRoom_AvatarBurden, m_pPetFrame) == 0x2d0);

class CRealMyRoom_AvatarManager : public WSingleton<CRealMyRoom_AvatarManager>
{
public:
	CRealMyRoom_AvatarManager();
	virtual ~CRealMyRoom_AvatarManager();
	CRealMyRoom_Avatar* FindAvatarByGuid(int guid) const;

	unsigned char m_unused0[4 + 0x78]; // A727: m_avatarCount + m_pAvatar[30] apos a base (0xa4 no VC10, 0xa0 no cl novo)
	CRealMyRoom_Avatar* m_pMyAvatar;
	unsigned char m_unusedA4[0xc];
};

struct sFurniture_List;
struct sStoredItemInfo;
struct sMailInfo;
struct sMailIncludeItem;
struct sMailInfoBrief;

class CRealMyRoomData : public WSingleton<CRealMyRoomData>
{
public:
	CRealMyRoomData();
	virtual ~CRealMyRoomData();

	bool AddModel(sFurniture_List model);
	bool SaveModel(bool bForce);

	unsigned char m_data[0xa0 - sizeof(WSingleton<CRealMyRoomData>)];
};

class CCameraWork : public WSingleton<CCameraWork>
{
public:
	CCameraWork();
	virtual ~CCameraWork();

	unsigned char m_data[0x2ec - sizeof(WSingleton<CCameraWork>)]; // 983: sizeof 0x2ec (push 0x2ec em 008c0c08; era 0x2b4 do 645: o new estourava)
};

class CItemStorage : public WSingleton<CItemStorage>
{
public:
	CItemStorage();
	virtual ~CItemStorage();

	void ClearStoredItemInfo();
	void AddStoredItemInfo(const sStoredItemInfo& info);
	void UnLock();

	void SetTotalPage(unsigned long page) { m_totalPage = page; }
	void SetCurPage(unsigned long page) { m_curPage = page; }
	void SetStoredPang(__int64 pang) { m_storedPang = pang; }

	// A727: conta do fim da base (FrCmdTarget 4, m_visible/m_lock 4, 3 std::list, m_pTitleFont): 0x58 no VC10, 0x48 no cl novo
	unsigned char m_pad0[8 + 3 * sizeof(std::list<void*>) + 4];
	__int64 m_storedPang;
	unsigned long m_totalPage;
	unsigned long m_curPage;
	unsigned char m_pad1[0x180 - 0x68];
};

#include "transferableitem.h"

class CRealMyRoomTask : public CTask
{
	DECLARE_OBJECT(CRealMyRoomTask)

protected:
	CRealMyRoomTask();
	virtual ~CRealMyRoomTask();

public:
	void AddAvatarBurden(const CRealMyRoom_AvatarBurden& burden);
	void RemoveAvatarBurden(const CRealMyRoom_AvatarBurden& burden);

protected:
	virtual void Register();
	virtual void Init(const char* wallPaper);
	virtual void PreLoadInit();
	virtual void Process(float delta);
	virtual void Display();
	virtual void Destroy();
	virtual int OnPacket(WReceivedPacket& packet);
	virtual void Load();
	virtual void RestorePreserved();
	virtual void OnAnotherHand(int key, void* pParam);

private:
	void DeleteBurdenPet(std::list<CRealMyRoom_AvatarBurden>& loaded,
		std::list<CRealMyRoom_AvatarBurden>& pending);

	std::list<CRealMyRoom_AvatarBurden> m_burdenList;
	std::list<CRealMyRoom_AvatarBurden> m_burdenQueue;
	std::list<CRealMyRoom_AvatarBurden> m_burdenDelete;
	CRITICAL_SECTION m_csBurden;
	bool m_bExit;
};
