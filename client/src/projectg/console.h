#pragma once

class WFont;

class CConsole : public WSingleton<CConsole>
{
public:
	CConsole();
	virtual ~CConsole();

	void Init();
	void ShutDown();
	void Display();
	void Process(float dt);
	static void __fastcall Print(const char* text)
	{
		if (IsInstantiated())
			Instance()->AddText(text);
	}

protected:
	bool PushIntoBsStack(char ch);
	char PopFromBsStack();
	void ClearBsStack();
	int BrowseCommand(const char* input, char* out);
	void AddText(const char* text);
	void BadArgument(int index);
	void Command(const char* cmd);
	int TweakerCommandProc(const char* cmd);
	void InputProcess(float dt);

	float m_fAlpha;
	int m_state;
	void* m_histPos;
	int m_histDir;
	char m_bsStack[0x80];
	bool m_bBrowsing;
	char m_browse[0x103];
	char* m_pMatch;
	WFont* m_pFont;
	WList<char*> m_cmdList;
	WList<char*> m_textList;
	bool m_bComposing;
	int m_scroll;
	char m_input[0x80];
	float m_fSpeed;
};
