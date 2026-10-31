#pragma once

#include "frform.h"
#include <vector>
#include <map>

#include "../shared/globalgamedefine.h"

class FrBingoDetailDlg;
class FrBingoGiftDlg;
class WTitleFont;

class CBingoPoint
{
public:
	CBingoPoint(int number);
	~CBingoPoint();

	void SetPointNumber(int number) { m_pointNumber = number; }
	int GetPointNumber() const { return m_pointNumber; }

	// HACK: emission order (the original compiles these three one wave late)
	void EnableProperty(int property)
	{
		if (0)
			operator=(0);
		int& value = m_property;
		value |= property;
	}
	void DisableProperty(int property)
	{
		if (0)
			operator=(0);
		int& value = m_property;
		value &= ~property;
	}
	bool IsProperty(int property)
	{
		if (0)
			operator=(0);
		if (m_property & property)
			return true;
		return false;
	}

	void DrawPoint(FrGraphicInterface* gdi, const Bitmap** const bitmap,
		const WRect& rect, float x, float y);

	CBingoPoint& operator=(int number);
	CBingoPoint& operator=(bool b);

private:
	int m_pointNumber;
	bool m_bEnable;
	int m_property;
};

class CBingoLine
{
public:
	CBingoLine(int size);
	~CBingoLine();

	CBingoLine& operator=(const CBingoLine& line);
	bool operator+=(CBingoPoint& point);
	int operator[](int index);
	const int operator[](int index) const;

	CBingoPoint& GetBingoPoint(int index);
	const CBingoPoint& GetBingoPoint(int index) const;
	CBingoPoint* FindBingoPoint(int number);
	int IsHavePointNumber(int number);
	bool IsCompleteLine();
	void ClearOverProperty();

private:
	std::vector<CBingoPoint> m_points;
	unsigned long m_reserved;
};

class CBingoBoard
{
public:
	CBingoBoard();
	~CBingoBoard();

	CBingoLine& operator[](int index);
	CBingoPoint* FindBingoPoint(int number);
	bool AimPoint(int number);
	void ClearOverProperty();

private:
	bool Initialize();
	void ReScanBoard();

	std::vector<CBingoLine> m_lines;
	std::vector<int> m_completeLine;
};

class FrBingoEventDlg : public FrForm
{
	DECLARE_OBJECT(FrBingoEventDlg)

	FrBingoEventDlg();
	virtual ~FrBingoEventDlg();

	virtual bool OnInit();
	virtual void OnProc(const float dt);

	void SetBingoData(GlobalEnum::sBingoInfo* info);
	void SetSelectedData(int number, int count, unsigned int newLine);
	void SetGiftData(std::vector<unsigned long>* gifts,
		unsigned int completeLine);

	bool OnDetailDlgResult(int result, FrForm* form);
	bool OnGiftDlgResult(int result, FrForm* form);

	struct sPoint
	{
		float x;
		float y;
		sPoint()
			: x(0), y(0)
		{
		}
		sPoint(float _x, float _y)
			: x(_x), y(_y)
		{
		}
	};

protected:
	void OnInitStartButton(int);
	void OnLButtonDownStartButton();
	void OnInitEventDetail(int);
	void OnLButtonDownEventDetail();
	void OnInitBoardNumberArea(int);
	void OnOwnerDrawBoardNumberArea(int);

private:
	void LoadBingoImage();
	void InitCharMatchMap();
	void InitNumberMatchMap();
	void DrawCompleteBar(FrGraphicInterface* gdi, const WRect& rect);
	void DrawBoardNumber(FrGraphicInterface* gdi, const WRect& rect);
	void DrawRandomNumber(FrGraphicInterface* gdi, const WRect& rect);
	void DrawTryCount(FrGraphicInterface* gdi, const WRect& rect);
	void DrawBingoLogo(FrGraphicInterface* gdi, const WRect& rect);

	static sFRESH_ENTRY _MsgEntries[0];

protected:
	static sFRESH_MSGMAP _MsgMap;
	virtual const sFRESH_MSGMAP* GetMessageMap() const;

private:
	FrButton* m_pStartButton;
	FrButton* m_pEventDetail;
	FrArea* m_pBoardNumberArea;
	CBingoBoard m_board;
	WPoint m_boardPos;
	WPoint m_randomNumberPos;
	WPoint m_tryCountPos;
	const Bitmap* m_pBitmap[10];
	FrBingoDetailDlg* m_pDetailDlg;
	FrBingoGiftDlg* m_pGiftDlg;
	int m_tryCount;
	unsigned long m_boardIndex;
	int m_selectedNumber;
	unsigned int m_completeLine;
	unsigned long m_aimedPoint;
	unsigned int m_newLine;
	bool m_bWaitResult;
	bool m_bReserved1;
	bool m_bReserved2;
	char m_tryCountText[3];
	std::map<char, sPoint> m_charMatchMap;
	std::map<int, sPoint> m_randomNumberMatchMap;
	std::map<int, sPoint> m_boardNumberMatchMap;
};

class FrBingoDetailDlg : public FrForm
{
	DECLARE_OBJECT(FrBingoDetailDlg)

	FrBingoDetailDlg();
	virtual ~FrBingoDetailDlg();

protected:
	void OnInitExplaneViewer(int);

private:
	static sFRESH_ENTRY _MsgEntries[0];

protected:
	static sFRESH_MSGMAP _MsgMap;
	virtual const sFRESH_MSGMAP* GetMessageMap() const;

private:
	FrViewer* m_pExplaneViewer;
};

class FrBingoGiftDlg : public FrForm
{
	DECLARE_OBJECT(FrBingoGiftDlg)

	FrBingoGiftDlg();
	virtual ~FrBingoGiftDlg();

	void SetGiftData(std::vector<unsigned long>* gifts);
	void SetGiftCount(std::vector<unsigned long>* counts);

	struct sGift
	{
		sGift(unsigned long _typeId, const char* _image)
			: typeId(_typeId), image(_image)
		{
		}

		unsigned long typeId;
		const char* image;
	};

protected:
	void OnInitGiftArea(int);
	void OnOwnerDrawGiftArea(int);

private:
	static sFRESH_ENTRY _MsgEntries[0];

protected:
	static sFRESH_MSGMAP _MsgMap;
	virtual const sFRESH_MSGMAP* GetMessageMap() const;

private:
	std::vector<unsigned long> m_giftList;
	std::vector<unsigned long> m_giftCount; // 983: +0x120 (SetGiftCount 00b00c00)
	FrArea* m_pGiftArea;
	float m_offsetX[4];
	const Bitmap* m_pGiftImage[4];
	WTitleFont* m_pCountFont; // 983: +0x154, fonte dos contadores
public:
	char m_pad983_158[8]; // 983: sem leitura/escrita (ctor 00afeea0 so ate +0x154; vtables d4ec9c/d4ec8c/d4ec84 e 00afa9f0..00b01a10 sem +0x158..+0x15f desta classe); sizeof 0x160 (push em 00aff25a)
};
