#pragma once

class CItemManager;
struct sCharacterInfo;

class CPartTidList
{
public:
	static void SetItemManager(CItemManager* pItemManager);

	CPartTidList();

	bool SetTids(sCharacterInfo* pInfo, unsigned char level);
	bool SetTid(unsigned long tid, unsigned long uid, const char* name,
		unsigned short flag);
	void SetDefaultTids();
	bool IsComboValid() const;
	bool IsEquipped(unsigned long tid, unsigned long uid) const;
	unsigned long GetOverlapped(unsigned long tid) const;
	bool CanSetPart(unsigned long tid) const;

protected:
	bool SetTids(unsigned long charTid, unsigned char hairColor,
		unsigned char shirtsColor, unsigned long* tids, unsigned char level,
		unsigned long* uids);
	void EmptyList(unsigned long mask);

public:
	unsigned long m_charTid;
	unsigned char m_defPartNum;
	unsigned char m_partNum;
	unsigned char m_hairColor;
	unsigned char m_shirtsColor;
	unsigned long m_tid[24];
	unsigned long m_uid[24];

	static CItemManager* m_pItemManager;
};
