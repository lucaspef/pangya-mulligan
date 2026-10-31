#pragma once

#include "actor.h"
#include "tweaker.h"
#include "gdvoiceitem.h"

class WOverlay;

namespace S5
{
	class CCutin : public IActor, public tweaker_cmd_target
	{
		DECLARE_OBJECT(CCutin)

	protected:
		CCutin();

	public:
		virtual ~CCutin() { }

		virtual void OnInit();
		virtual void OnProcess(float delta);
		virtual void OnDisplay();
		virtual void OnDestroy();
		virtual void HandleMsg(const MsgObject& msg);

	protected:
		void SetGameState(unsigned long state);
		bool IsGameState(unsigned long state);
		void SetCondition(unsigned long condition);
		int GetCondition();
		bool ISCondition(unsigned long condition);

		bool CheckGameState();
		bool PrepareCutIn();
		bool ProcessWorkIn(float delta);
		bool ProcessWorkInFinish(float delta);
		bool ProcessWorkOut(float delta);
		void ClearAllData(bool bAll);
		void OutDraw_Setting();
		void PlayVoice(unsigned long tid, unsigned long condition);

		void SetOverlayInfo(WOverlay* pOverlay, eCUTINANISTLYE style,
			WRect& rect);
		bool ProcessOverlay(float delta, WOverlay* pOverlay,
			eCUTINANISTLYE style, WRect& rect);
		bool ProcessOverlay_Out(float delta, WOverlay* pOverlay,
			eCUTINANISTLYE style, WRect& rect);
		void DisplayOverlay(WOverlay* pOverlay, eCUTINANISTLYE style,
			WRect& rect);
		void RectScaling(WRect& rect, float scale);
		void ScreenRate(WRect& rect, WOverlay* pOverlay);

		std::string TwAnimateTest(const tweaker_param& param);

		int m_state;
		CUTIN_ANI_STYPEINFO m_info;
		WOverlay* m_pCharOverlay;
		WOverlay* m_pBgOverlay;
		WOverlay* m_pPatternOverlay;
		WOverlay* m_pTextOverlay;
		float m_alpha;
		float m_rotate;
		float m_scale;
		WRect m_rcChar;
		WRect m_rcBg;
		WRect m_rcPattern;
		WRect m_rcText;
		float m_delay;
		unsigned long m_gameState;
		unsigned long m_unused13c;
		unsigned long m_condition;
		// 983 +0x144: vozes da tabela "VoiceCutin" por tid (OnInit 00b33d30); o 983 nao tem a fila "next" do 645
		std::multimap<unsigned long, sGD_VOICEITEM> m_voiceMap;
	};

	void CCutin::SetGameState(unsigned long state)
	{
		m_gameState |= state;
	}

	void CCutin::SetCondition(unsigned long condition)
	{
		m_condition |= condition;
	}

	int CCutin::GetCondition()
	{
		return m_condition;
	}
}
