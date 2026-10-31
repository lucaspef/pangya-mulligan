#pragma once
#include "frform.h"

class FrArea;
class FrButton;
class FrEdit;

class FrFormBar : public FrForm
{
public:
	DECLARE_OBJECT(FrFormBar)

	FrFormBar();
	virtual ~FrFormBar();

	virtual bool Open(FRESH_PFN_RESULT pFnResult, unsigned long flag);
	virtual void SetDesc(const char* desc);

	void Attach(const WRect& rect, const char* image);
	void SetFixedWnd();
	void SetGaugeSize(const WSize& size);
	void MoveBar(const WPoint& pos);
	WPoint GetRelativeBarPos();
	void SetFrameImg(const char* image);
	void SetGaugeImg(const char* image);
	void SetRange(int min, int max);
	void SetPos(float pos);
	const WRect& GetGaugeRect() { return m_gaugeRect; }
	void SetGaugeFrameSize(const WSize& size);
	void SetGaugePivot(const WPoint& pivot);
	void EnableButton(bool bEnable);
	void SetAutoCloseTimer(float time);

	struct sATTACHRES
	{
		const Bitmap* pBitmap;
		WRect rect;
	};

protected:
	virtual void OnProc(const float deltaTime);

	void OnInitEdMsg(int param);
	void OnInitBtnOK(int param);
	void OnDownBtnOK();
	void OnInitBtnNO(int param);
	void OnDownBtnNO();
	void OnInitGaugeArea(int param);
	void OnDrawGaugeArea();

private:
	void Init();
	void MakeDefault();
	void ReCalcRects();

	std::list<sATTACHRES> m_attachList;
	FrArea* m_pGaugeArea;
	const Bitmap* m_pGaugeImg;
	const Bitmap* m_pFrameImg;
	WRect m_frameRect;
	WRect m_gaugeRect;
	WRect m_orgRect;
	float m_min;
	float m_max;
	float m_pos;
	WPoint m_gaugePivot;
	WPoint m_barPos;
	WSize m_frameSize;
	std::pair<FrButton*, FrButton*> m_btn;
	FrEdit* m_pEdMsg;
	float m_autoCloseTime;
	float m_autoCloseRemain;

	DECLARE_FRESH_MSGMAP()
};
