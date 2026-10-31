#pragma once
#include "baseobject.h"
#include "singleton.h"
#include "../../include/engine/wtypes.h"

// clang-format off
#define CHATMSG_PAD10(n) \
	void PchPad##n##0(); \
	void PchPad##n##1(); \
	void PchPad##n##2(); \
	void PchPad##n##3(); \
	void PchPad##n##4(); \
	void PchPad##n##5(); \
	void PchPad##n##6(); \
	void PchPad##n##7(); \
	void PchPad##n##8(); \
	void PchPad##n##9();
#define CHATMSG_PAD100(n) \
	CHATMSG_PAD10(n##0) \
	CHATMSG_PAD10(n##1) CHATMSG_PAD10(n##2) CHATMSG_PAD10(n##3) \
		CHATMSG_PAD10(n##4) CHATMSG_PAD10(n##5) CHATMSG_PAD10(n##6) \
			CHATMSG_PAD10(n##7) CHATMSG_PAD10(n##8) CHATMSG_PAD10(n##9)

// clang-format on

class WFont;
class WOverlay;
struct tagCANDIDATELIST;

struct sCompStr
{
	char* pCompStr;
	unsigned char* pCompAttr;
	unsigned long* pCompCls;
	unsigned long dwCompAttrLen;
	unsigned long dwCompClsLen;
};

class CChatMsg : public WSingleton<CChatMsg>
{
public:
	using WSingleton<CChatMsg>::m_pInstance;
	CChatMsg();
	virtual ~CChatMsg();
	void Reset();
	void ReplaceBackupUiAlpha();
	unsigned long ConvertToUIColor(unsigned long color);

	void Init();
	// HACK: never defined or called. The number of names declared in the PCH
	// steers MSVC internal tables: itemmanager and mascotinfodlg only keep
	// their original inlining/register allocation with ~620 extra names here.
	// clang-format off
	CHATMSG_PAD100(a) CHATMSG_PAD100(b) CHATMSG_PAD100(c) CHATMSG_PAD100(d)
	CHATMSG_PAD100(e) CHATMSG_PAD100(f) CHATMSG_PAD10(g0) CHATMSG_PAD10(h0)
		// clang-format on
		void ResetUiPosition();
	void ClearHistory();
	void SetActive(bool active, bool b2, bool b3);
	void ToggleInsertMode();
	void AddChatMsg(const std::string& msg, unsigned long color, bool b1,
		bool b2);
	void RemoveColor(const std::string& src, std::string& dst);
	void ProcessWindow(float deltaTime);
	void Process(float deltaTime);
	void GetChatText(std::string& front, std::string& comp, std::string& end);
	void GetChatText_JP(std::string& front, std::string& comp, std::string& end,
		int maxBytes);
	int GetCurCaretPos() { return m_iCurByte; }
	const char* GetCompChar() const;
	void GetCompositionString(sCompStr* pCompStr);
	const char* GetCompositionString();
	void SetChatText(const char* text, bool b);
	void SetChatFacialUser(std::string& user);
	void ResetCurCaretPos()
	{
		m_iCurByte = 0;
		m_iCurChar = 0;
	}
	void CurCaretMoveLeft();
	void CurCaretMoveRight() { OnMoveRight(); }
	int GetNumBytes() { return m_iNumBytes; }
	void Open(bool b1, bool b2);
	void Close(bool b1, bool b2);
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: body in golfrule_pch_inl.h / at the original PCH position (emission order)
	bool IsOpened();
	bool IsClosed();
#else
	bool IsOpened() { return m_iState == 3; }
	bool IsClosed() { return m_iState == 0; }
#endif
	bool IsOpenning() { return m_iState == 2 || m_iState == 1; }
	void ProcessMacro(const float deltaTime);
	void PrintInChatWindow();
	void PrintInScreen();
	void DrawChatWindow();
	void PrintOnChatRoom();
	std::string GetIncomingChatMsg() const;
	int GetConsolKeyCode() const { return m_iConsolCode; }
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: body in golfrule_pch_inl.h / at the original PCH position (emission order)
	bool IsActive() const;
	WFont* GetMaskedFont() const;
#else
	bool IsActive() const { return m_bActive; }
	WFont* GetMaskedFont() const { return m_pMaskedFont; }
#endif
	char IsExtChar(int c);
	void SetAlwaysActive();
	bool HasBeenPrinted();
	void EnableCaretMove(bool enable) { m_bCaretMove = enable; }
	void SetBufLen(int len) { m_nMaxBytes = len; }
	void SetBufWidth(float width) { m_fMaxWidth = width; }
	void Toggle(bool
			toggle); // golfrule.cpp: inline in golfrule_pch_inl.h (chatmsg.cpp has it out of line)
	const char* MakeShortID(const char* id, float width);
	const char* GetCompStr983() const { return m_szComp; } // 983: FrWndManager::ProcessHotKey (00be58e0) lê m_szComp[0] (+0x4c0); nome meu
	bool IME_IsCandWinOpened();
	bool IME_IsCompDone();
	bool IME_IsImeOpened();
	bool IME_IsCompInProgress();
	bool IME_IsAlphaNumericMode();
	void IME_SetAlphaNumericMode();
	void IME_SetOpenStatus(bool open);
	tagCANDIDATELIST* const IME_GetCandList();

protected:
	void InitFont();
	void ProcessResultString();
	void ProcessCompString();
	void ProcessEditKey(const float deltaTime);
	void ProcessResultString_JP();
	void ProcessCompString_JP();
	void OnMoveLeft();
	void OnMoveRight();
	void OnMoveEnd();
	void OnMoveHome();
	void OnBackSpace();
	void OnDelete();
	void DeleteCharacter();
	void DeleteSelected();
	void CopySelected();
	void PasteCopied();
	void InsertWCharacter(int code);
	void WriteCharacter(int code);

	bool m_bPrinted;
	bool m_bCaretMove;
	int m_nMaxBytes;
	float m_fMaxWidth;
	WFont* m_pMaskedFont;
	WOverlay* m_pChatWindow[3];
	WRect m_rcChatWinSrc[3];
	WRect m_rcChatWinDst[3];
	float m_fVel;
	bool m_bActive;
	bool m_bInsert;
	bool m_bCompInProgress;
	bool m_bShowCaret;
	float m_fCaretTime;
	int m_iState;
	int m_iCompCode;
	int m_iPrevCode;
	int m_iConsolCode;
	char m_szChat[1024];
	char m_szComp[1024];
	char m_szExt[1024];
	bool m_abWide[1024];
	int m_iNumBytes;
	int m_iCurByte;
	int m_iNumChars;
	int m_iCurChar;
	int m_iSelByteS;
	int m_iSelByteE;
	int m_iNumSelBytes;
	int m_iSelCharS;
	int m_iSelCharE;
	int m_iNumSelChars;
	char m_szCopied[1024];
	char m_szCopiedExt[1024];
	bool m_abCopied[1024];
	int m_iNumCopyBytes;
	int m_iNumCopyChars;
	unsigned long m_lastMsgTime;
	unsigned char m_bBackupUiAlpha;
	bool m_bOpen;
	bool m_bToggle;
	std::string m_chatFacialUser;
	unsigned char m_bCompAttr[1024];
	unsigned long m_dwCompCls[256];
	unsigned long m_dwCompStrLen;
	unsigned long m_dwCompAttrLen;
	unsigned long m_dwCompClsLen;
};
