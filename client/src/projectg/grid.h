#pragma once

#include "actor.h"
#include "scenemanager.h"

class CGrid : public IActor, public CRenderFuncPtr
{
public:
	enum
	{
		MAX_GRID = 31 // 983 (era 21): m_grid 0x303c -> 0x691c = 31 * 31 * 0x1c (voto m_angle 0x3064 -> 0x6944; cauda igual, 0x1dc)
	};

	virtual ~CGrid() { }

	DECLARE_OBJECT(CGrid)

public:
	virtual void OnPreLoadInit();
	virtual void OnLoad();
	virtual void OnReload();
	virtual void OnProcess(float delta);
	virtual void OnDestroy();
	virtual void Display();

protected:
	CGrid();

	virtual void HandleMsg(const MsgObject& msg);

	void Reset();
	void SetGridType();
	void CheckGridPass();
	void MakeStaticGrid();
	void MakeDynamicGrid();
	int GetColor(float slope);
	void SetHoleGrid();
	void MakeSlopeIndex();
	float GetSlope(int row, int col, float size, bool bRow);

private:
	void MoveLine(float delta);

	struct sGridBlock
	{
		WVector pos;
		WVector slopePos;
		bool bPass;
	};

	WVector m_pos;
	sGridBlock m_grid[MAX_GRID][MAX_GRID];
	float m_angle;
	int m_cols;
	int m_rows;
	float m_blockSize;
	int m_extraCols;
	int m_extraRows;
	float m_extraSize;
	int m_ballRow;
	int m_ballCol;
	bool m_bBallGrid;
	WVector m_dirZ;
	WVector m_dirX;
	int m_texSlope;
	int m_texBlue;
	bool m_bShow;
	float m_lineOffset;
	unsigned long m_color;
	WTVertex* m_pVtx[5];
	WTVertex m_vtx[4];
	bool m_bBlink;
	float m_blinkTime;
	unsigned long m_blinkColor;
	float m_moveLine;
	int m_reserved;
};
