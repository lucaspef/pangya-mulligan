#pragma once

#include <string>
#include "frform.h"

class FrButton;
class FrGaugeBar;
class FrComboBox;
class FrArea;
class FrEdit;
class FrListBox;
class FrStatic;
class FrViewer;
class WTitleFont;

struct sChatSlot;
class FrEmoticonDlg;

class FrMessengerChatDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrMessengerChatDlg)
	FrMessengerChatDlg();
	virtual ~FrMessengerChatDlg();

	virtual bool Close(bool bResult);

	void Vibration(float time);
	void AddChatLine(std::string name, std::string msg);
	void SetUID(unsigned long uid);
	void SaveTempChat(sChatSlot& slot);
	void RestoreChat(sChatSlot& slot);
	bool GetEmoticonClientRect(WRect& rect) const;

protected:
	virtual void OnProc(const float dt);
	virtual bool OnLButtonDown(const WPoint& pt);

	void OnMessenger_CloseLBtnDown();
	void OnMessenger_ChatInputInit(int param);
	void OnMessenger_ChatInputLBtnDown();
	bool OnChatInputEnterKey(int param);
	void OnMessenger_ChatViewInit(int param);
	void OnMessenger_EmoticonInit(int param);
	void OnMessenger_EmoticonLBtnDown();
	bool OnEmoticonResult(int result, FrForm* pForm);
	void OnMessenger_CaptionInit(int param);

	FrEmoticonDlg* m_pEmoticonDlg;
	unsigned long m_unused114983; // 983: +0x114 so zerado no ctor (008fc640), sem leitor no 983 (D852)
	FrButton* m_pEmoticonBtn;
	FrEdit* m_pChatInput;
	FrEdit* m_pChatView;
	FrStatic* m_pCaption;
	unsigned long m_uid;
	std::string m_lastName;
	float m_vibrateTime;
	float m_vibrateElapsed;
	bool m_bVibrate;
	float m_vibrateDX;
	float m_vibrateDY;

private:
	DECLARE_FRESH_MSGMAP()
};
