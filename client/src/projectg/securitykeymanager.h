#pragma once

#include <list>
#include <string>

enum eSKContent
{
	SK_NONE,
	SK_UCC,
};

struct sSKItem
{
	eSKContent content;
	unsigned long index;
	std::string key;
};

class CSecurityKeyManager
{
public:
	CSecurityKeyManager();
	~CSecurityKeyManager();

	void SetKey(eSKContent content, unsigned long index, std::string key);
	std::string GetKey(eSKContent content, unsigned long index);

	void ReqCreateKey(eSKContent content, unsigned long index);
	void ReqCheckKey(eSKContent content, unsigned long index, std::string key);
	void ResCreateKey(eSKContent content, unsigned long index, std::string key,
		unsigned char result);
	void ResCheckKey(eSKContent content, unsigned long index, std::string key,
		unsigned char result);

private:
	sSKItem* GetItem(eSKContent content, unsigned long index);

	std::list<sSKItem> m_items;
};

CSecurityKeyManager* SecurityKeyManager();
