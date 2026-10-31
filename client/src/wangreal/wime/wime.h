#pragma once
#include <winput.h>
#include <wproc.h>
#include <imm.h>

// WIme do JP 983 (sizeof 0x1064; no 645 era 0x550). Layout tirado do construtor 0x10047360 e de ClearData
// 0x10046530. Campos marcados com "?" ainda não têm nome confirmado (F1).

class WIme : public WInputDev, public WProc
{
public:
	WIme();
	WIme(HWND hwnd);
	virtual ~WIme();
	virtual int GetState(int code, int n);
	virtual void Update(unsigned long timeStamp);
	virtual void Reset();
	virtual WInputDev* MakeClone(char* modeName, HWND hwnd);
	virtual char* GetDeviceName() { return "Ime"; }
	virtual WProc* ExternProc() { return this; }
	virtual int WinProc(unsigned int msg, unsigned long wparam,
		unsigned long lparam);
	char* GetCompStr();
	char* GetCompAttr();
	DWORD* GetCompClause();
	virtual void SetActive(bool stat)
	{
		m_bActive = stat;
		pos = 0;
		current = 0;
	}
	virtual void SetState(int type, int value);
	virtual void SetAlphaNumericMode();
	virtual void SetOpenStatus(bool open);
	virtual bool IsAlphaNumericMode();

protected:
	void OnStartComposition(unsigned int dwCommand, long dwData);
	int OnSetContext(unsigned int dwCommand, long dwData);
	void OnEndComposition(unsigned int dwCommand, long dwData);
	void OnCompositionFull(unsigned int dwCommand, long dwData); // 0x10046630 (vazia)
	int OnNotify(unsigned int dwCommand, long dwData); // 0x10046640
	void OnControl(unsigned int dwCommand, long dwData); // 0x10046870 (vazia; no 983 quem a chama é WM_IME_CHAR)
	int GetResultString();
	int GetCompString(long flag);
	virtual void ProcessResultString(char* str);
	void OnInputLangChange(unsigned int dwCommand, long dwData);
	void OnComposition(unsigned int dwCommand, long dwData);
	void OnChar(unsigned int nChar);

private:
	int IME_Enter();
	void Leave();
	int Check();
	void ReleaseKeyMap();          // 0x100463d0 (novo no 983)
	void SetKeyMap(DWORD compStat); // 0x10046470 (novo no 983)
	void InitKeyMap();             // 0x10046e70 (novo no 983)
	void ClearData();
	void PutString(char* str);
	void InitIme(HWND hwnd_);

	DWORD m_property;              // 0x14
	HKL m_hKeyLayout;              // 0x18
	DWORD m_nState;                // 0x1c  bits: 1 compondo, 2 candidatos, 4 ?, 8 IME aberto
	int m_charWidth;               // 0x20 ?
	int m_charHeight;              // 0x24 ?
	bool m_bActive;                // 0x28
	HIMC m_hIMC;                   // 0x2c
	HWND m_hWnd;                   // 0x30
	char m_compStr[0x400];         // 0x34
	char m_compAttr[0x400];        // 0x434
	DWORD m_compClause[0x100];     // 0x834
	int m_compStrLen;              // 0xc34
	int m_compAttrLen;             // 0xc38
	int m_compClauseLen;           // 0xc3c
	DWORD m_dwConvMode;            // 0xc40
	unsigned char* buff;           // 0xc44 ?
	int current;                   // 0xc48
	int old_current;               // 0xc4c
	unsigned char ringBuf[1024];   // 0xc50
	int pos;                       // 0x1050
	CANDIDATELIST* m_candList;     // 0x1054  new CANDIDATELIST no construtor; cresce em OnNotify
	DWORD m_candSize;              // 0x1058
	LRESULT m_imeVersion;          // 0x105c  resposta de MSIMEService
	UINT m_msgKeyMap;              // 0x1060  RegisterWindowMessage("MSIMEKeyMap")
};
