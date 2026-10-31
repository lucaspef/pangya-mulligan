#pragma once

#include <atlmisc.h>
#include <atlgdi.h>

class CRegistry
{
public:
	CRegistry();
	~CRegistry();

	BOOL SetRootKey(HKEY rootKey);
	BOOL ClearKey();
	BOOL CreateKey(WTL::CString key);
	BOOL DeleteKey(WTL::CString key);
	BOOL SetKey(WTL::CString key, BOOL create);
	BOOL DeleteValue(WTL::CString name);
	int GetDataSize(WTL::CString name);
	DWORD GetDataType(WTL::CString name);
	int GetSubKeyCount();
	int GetValueCount();
	BOOL KeyExists(WTL::CString key, HKEY rootKey = NULL);
	BOOL ValueExists(WTL::CString name);
	void RenameValue(WTL::CString oldName, WTL::CString newName);

	double ReadFloat(WTL::CString name, double defaultValue);
	DWORD ReadDword(WTL::CString name, DWORD defaultValue);
	int ReadInt(WTL::CString name, int defaultValue);
	BOOL ReadBool(WTL::CString name, BOOL defaultValue);
	COLORREF ReadColor(WTL::CString name, COLORREF defaultValue);
	WTL::CString ReadString(WTL::CString name, WTL::CString defaultValue);
	BOOL ReadFont(WTL::CString name, WTL::CFont* value);
	BOOL ReadPoint(WTL::CString name, WTL::CPoint* value);
	BOOL ReadSize(WTL::CString name, WTL::CSize* value);
	BOOL ReadRect(WTL::CString name, WTL::CRect* value);

	BOOL WriteBool(WTL::CString name, BOOL value);
	BOOL WriteString(WTL::CString name, WTL::CString value);
	BOOL WriteFloat(WTL::CString name, double value);
	BOOL WriteInt(WTL::CString name, int value);
	BOOL WriteDword(WTL::CString name, DWORD value);
	BOOL WriteColor(WTL::CString name, COLORREF value);
	BOOL WriteFont(WTL::CString name, WTL::CFont* value);
	BOOL WritePoint(WTL::CString name, WTL::CPoint* value);
	BOOL WriteSize(WTL::CString name, WTL::CSize* value);
	BOOL WriteRect(WTL::CString name, WTL::CRect* value);

private:
	LONG m_lastError;
	HKEY m_rootKey;
	BOOL m_lazyWrite;
	WTL::CString m_key;
};
