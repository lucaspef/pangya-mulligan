#pragma once
#include <string>
#include <vector>
#include "frtext.h"
#include "../../../include/engine/wtypes.h"

class FrWndManager;
class WOverlay;

class FrToolTip
{
	friend class FrWnd;
	friend class FrWndManager;

public:
	// 983: string vazia global (00e43b6c, iniciada em 00cf7fe0) usada aqui e em FrWnd::GetToolTipText; nome meu
	static const std::string m_emptyText983;

protected:
	FrToolTip(FrWndManager* pManager);
	virtual ~FrToolTip();
	virtual void Move(const WPoint& point);
	virtual void SetToolTipText(const std::string& text);
	virtual void OnProcess(float deltaTime);
	virtual void OnDisplay();
	virtual void SetFrameStyle(unsigned long style);

private:
	void Reset();
	void Prepare();

	class cToolTipFrame
	{
	public:
		enum eFrameIdx
		{
			LT,
			MT,
			RT,
			LM,
			MM,
			RM,
			LB,
			MB,
			RB,
			FRMIDX_MAX
		};

		typedef std::vector<WOverlay*> TOOLTIPFRM;

		cToolTipFrame();
		~cToolTipFrame();
		void Init();
		void SetStyle(unsigned long style);
		void Render(const WRect& rect, unsigned long color);

		std::vector<TOOLTIPFRM> m_Frames;
		unsigned long m_style;
	};

	float m_fAlpha;
	cToolTipFrame m_Frame;
	WRect m_rcWnd;
	FrWndManager* m_pWndManager;
	std::string m_orgText;
	FrTEXT m_FrText;
	unsigned long m_textColor;
};
