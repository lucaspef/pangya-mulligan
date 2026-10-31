#pragma once

#include "actor.h"
#include "scenemanager.h"
#include <list>
#include <string>
#include <utility>
#include <vector>

class CBarFont;
class WOverlay;
class FrButton;
class SpotHistory;

class IReviewHistory
{
public:
	virtual ~IReviewHistory() { }
	virtual const char* ClassName() const = 0;
	virtual void AddItem(unsigned int param1, unsigned int param2) = 0;
	virtual void Clear() = 0;
	virtual void Display() = 0;
	virtual void PopBack() = 0;
	virtual void PopFront() = 0; // 983: 7o virtual novo (so o 983 tem)
};

class ShotReview : public IActor
{
	DECLARE_OBJECT(ShotReview)

public:
	virtual void HandleMsg(const MsgObject& msg);

protected:
	ShotReview();
	virtual ~ShotReview();

	virtual void OnDisplay(); // 983: 008482e0 (sem CRenderFuncPtr)

	virtual void OnInit();
	virtual void OnLoad();
	virtual void OnProcess(float delta);

public:
	struct ReviewHistoryFinder
	{
		ReviewHistoryFinder(const std::string& name)
			: m_name(name)
		{
		}
		bool operator()(const IReviewHistory* history) const
		{
			return m_name == history->ClassName();
		}

		std::string m_name;
	};

private:
	// 983 (ctor 0084b5b0, sizeof 0xd4): so IActor (sem CRenderFuncPtr; o 983 nao registra elemento de cena)
	void RefreshButtons();							// 00847360
	void SetButtonsVisible(bool visible);			// 00847310
	void UpdateHistoryMode();						// 00847c30 (OnProcess)
	void ReleaseReviewCamera();						// 00847ce0
	void StepReview(bool next);						// 0084a520
	void SelectLastReview();						// 0084a2d0
	void ApplyReviewCamera(int index);				// 00849f80
	void OnReviewKeys();							// 0084ae40
	int ReviewCount() const;						// 00846760
	inline void ForEachHistory(void (IReviewHistory::*fn)());
	inline SpotHistory* FindSpotHistory() const;
	IReviewHistory* FindHistory(const char* name) const;

	std::vector<IReviewHistory*> m_histories;		// 983: +0x18
	bool m_bVisible;								// 983: +0x28
	bool m_bRecordBounce;							// +0x29
	bool m_bEnabled;								// +0x2a (ENABLE)
	int m_maxHistory;								// +0x2c (SET_MAX_HIS; ctor 10)
	int m_count;									// +0x30 (contador de tacadas guardadas)
	std::vector<FrButton*> m_buttons;				// +0x34 (pre, next)
	bool m_bHistoryMode;							// +0x44
	bool m_bReviewCamera;							// +0x45 (camera de revisao ligada)
	int m_reviewIndex;								// +0x48
	unsigned char m_savedCamera[0x54];				// +0x4c (copia de sCameraData em GolfDoc+0x104)
	float m_savedMatrix[12];						// +0xa0 (copia de g_camera 00f024a0)
	WOverlay* m_pFrame;								// +0xd0 airnote_frame.tga
};

class SpotHistory : public IReviewHistory
{
public:
	SpotHistory();
	virtual ~SpotHistory();
	virtual const char* ClassName() const { return "SpotHistory"; }
	virtual void AddItem(unsigned int param1, unsigned int param2);
	virtual void Clear();
	virtual void Display();
	virtual void PopBack();
	virtual void PopFront();
	const std::pair<WVector, WVector>* At(unsigned int index) const; // 008489d0

private:
	std::list<std::pair<WVector, WVector> > m_spots;
	unsigned int m_pad983_0c; // +0x0c: construtor/destrutor 00848f10/0084900c e At nao tocam
	CBarFont* m_pFont;
	WOverlay* m_pIndicator;
	WOverlay* m_pWaterIndicator;
	int m_numberBase;
};

class BallTraceHistory : public IReviewHistory
{
public:
	BallTraceHistory();
	virtual ~BallTraceHistory();
	virtual const char* ClassName() const { return "BallTraceHistory"; }
	virtual void AddItem(unsigned int param1, unsigned int param2);
	virtual void Clear();
	virtual void Display();
	virtual void PopBack();
	virtual void PopFront();

private:
	std::list<std::vector<WVector> > m_traces;
	WOverlay* m_pIndicator; // 983 +0x10: [fall_indicator1.png
};

class FirstBoundHistory : public IReviewHistory
{
public:
	FirstBoundHistory();
	virtual ~FirstBoundHistory();
	virtual const char* ClassName() const { return "FirstBoundHistory"; }
	virtual void AddItem(unsigned int param1, unsigned int param2);
	virtual void Clear();
	virtual void Display();
	virtual void PopBack();
	virtual void PopFront();

private:
	void DrawIndicator(const std::pair<WVector, WVector>& bounce, unsigned long color); // 00847150

	std::list<std::pair<WVector, WVector> > m_points;
	WOverlay* m_pIndicator; // 983 +0x10 (sizeof 0x18: new 0x18 no ShotReview 983)
	CBarFont* m_pFont;
};
