#include "minatl.h"
#include "projectg.h"
#include "golfdoc.h"
#include "golfball.h"
#include "club.h"
#include "quadtree.h"
#include "capturedbg.h"
#include "scenemanager.h"
#include "../shared/localize.h"
#include "grid.h"

extern WMatrix g_camera;
unsigned char GetCurMap();

// Bit test on the actor flags. The original tests them with shr + test against
// a register holding 1 (no WFlags::operator unsigned long COMDAT); a plain
// (m_flag >> bit) & 1 folds into test imm.
#define FLAG(bit) ((bool)((*(const unsigned long*)&m_flag >> (bit)) & 1))
// The grid is extended for the power putter ("1PT").
// HACK: the original hoists the GolfClub() call before the "1PT" push, as an
// inlined accessor returning CClub& would; the dummy conditional reproduces that
// temporary (and the register allocation that follows from it).
#define IS_1PT() strstr((true ? GolfClub() : GolfClub()).m_pClub->name, "1PT")

IMPLEMENT_ACTOR(CGrid, IActor)

// golfbg.cpp defines this string as a local static (the original emits it
// only there), so reference that instead of emitting our own copy.
#define rb_str_Grid "\xb1\xd7\xb8\xae\xb5\xe5"

CGrid::CGrid()
	: m_texSlope(0), m_texBlue(0)
{
	m_dispPriority = DISP_PRIORITY_1;
}

void CGrid::OnDestroy()
{
	if (g_resrcmng && m_texSlope)
	{
		g_resrcmng->Release(m_texSlope);
		m_texSlope = 0;
	}
	if (g_resrcmng && m_texBlue)
	{
		g_resrcmng->Release(m_texBlue);
		m_texBlue = 0;
	}
}

void CGrid::OnPreLoadInit()
{
	for (int i = 0; i < 4; i++)
	{
		m_vtx[i].tu = (i & 2) ? 1.0f : 0.0f;
		m_vtx[i].tv = (i & 1) ? 0.0f : 1.0f;
	}

	m_pVtx[0] = &m_vtx[0];
	m_pVtx[1] = &m_vtx[1];
	m_pVtx[2] = &m_vtx[3];
	m_pVtx[3] = &m_vtx[2];
	m_pVtx[4] = NULL;

	m_angle = 0.0f;
	m_lineOffset = -1.0f;
	m_bBlink = false;
	Reset();

	CSceneManager::Instance()->RegisterElement(OBJ_FUNC, NULL, this,
		rb_str_Grid, 0, NULL);
}

void CGrid::OnLoad()
{
	m_texSlope = g_resrcmng->LoadTexture("[slope.jpg", 0, 0, NULL);
	m_texBlue = g_resrcmng->LoadTexture("middle_blue.jpg", 0, 0, NULL);
}

void CGrid::OnReload()
{
	m_bBlink = false;
	Reset();
}

void CGrid::Reset()
{
	m_blinkTime = 0.0f;
	m_blinkColor = 0xffff0000;

	for (int i = 0; i < MAX_GRID; i++)
	{
		for (int j = 0; j < MAX_GRID; j++)
			m_grid[i][j].bPass = false;
	}

	m_bBallGrid = false;
	m_bShow = false;

	m_flag.Disable(4);

	switch (GetCurMap())
	{
	case 6:
		m_color = 0xff509e75;
		break;

	case 9:
		m_color = 0xff677943;
		break;

	case 11:
		m_color = 0xff3e7d30;
		break;

	case 13:
		m_color = 0xfffbb817;
		break;

	case 18:
		m_color = 0xff00ffff;
		break;

	case 21:
		m_color = 0xffdcdcdc;
		break;

	default:
		m_color = 0xff00562f;
		break;
	}
}

void CGrid::SetGridType()
{
	unsigned char cur = GOLFDOC()->m_currentPlayer;
	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(cur);
	WVector pin = GOLFDOC()->GetHoleData().pin;
	WVector d = pin - pPlayer->pos;
	d.y = 0.0f;

	if (GolfBall().m_groundType != 2 && !(d.Magnitude() < 128.0f))
	{
		m_flag.Enable(2);
		m_cols = 10;
		m_rows = 11;
		m_blockSize = 12.8f;
		m_extraCols = 0;
		m_extraRows = 0;
		m_extraSize = 0.0f;
		MakeStaticGrid();
		Reset();
		return;
	}

	m_flag.Disable(2);
	// 983: a rodada de evento (bit 0x40) usa 21 linhas; a condicao do putter e testada nos dois ramos
	if (GOLFDOC()->IsCurrentPlayerEvent983(0x40))
	{
		if (GolfBall().m_groundType == 2 && GolfClub().GetType() == 3 &&
			!GolfClub().IsPowerPutter())
		{
			m_cols = 10;
			m_rows = 21;
			m_blockSize = 3.2f;
			m_extraCols = -4;
			m_extraRows = -5;
			m_extraSize = 3.2f;
		}
		else
		{
			m_cols = 8;
			m_rows = 21;
			m_blockSize = 6.4f;
			m_extraCols = -2;
			m_extraRows = 5;
			m_extraSize = 0.0f;
		}
	}
	else if (GolfBall().m_groundType == 2 && GolfClub().GetType() == 3 &&
		!GolfClub().IsPowerPutter())
	{
		m_cols = 10;
		m_rows = 11;
		m_blockSize = 3.2f;
		m_extraCols = -4;
		m_extraRows = 0;
		m_extraSize = 3.2f;
	}
	else
	{
		m_cols = 8;
		m_rows = 16;
		m_blockSize = 6.4f;
		m_extraCols = -2;
		m_extraRows = 5;
		m_extraSize = 0.0f;
	}
	Reset();
}

void CGrid::OnProcess(float delta)
{
	if (GOLFDOC()->m_tutorialMode < 4 || CCapturedBg::Instance()->IsCaptured())
		return;

	if (GolfClub().Updated())
		m_flag.Enable(0x10);

	if (FLAG(2))
		return;

	WVector d = g_camera.pivot - m_pos;
	d.y = 0.0f;
	if (d.Magnitude() > 320.0f)
	{
		m_flag.Enable(1);
		return;
	}

	m_flag.Disable(1);

	if (!FLAG(1))
	{
		if (FLAG(4))
		{
			m_dirZ = RotMat(m_angle, 1).za;
			m_dirX = WCrossProduct(WVector::UNIT_POS_Y, m_dirZ);

			MakeDynamicGrid();
			SetHoleGrid();
			MakeSlopeIndex();
			m_flag.Disable(8);
		}
		else
			m_flag.Enable(8);
	}

	m_flag.Disable(0x10);

	if (GolfBall().m_state != 0)
		CheckGridPass();

	if (m_bBlink)
		m_blinkTime += delta;
}

void CGrid::CheckGridPass()
{
	if (!m_bShow)
		return;

	WVector d = GolfBall().m_pos - m_grid[0][0].pos;
	d.y = 0.0f;

	d /= m_blockSize + (IS_1PT() ? m_extraSize : 0.0f);

	int row = (int)(d * m_dirZ);
	int col = (int)(d * m_dirX);

	if (row < m_rows + (IS_1PT() ? m_extraRows : 0) - 1 &&
		col < m_cols + (IS_1PT() ? m_extraCols : 0) - 1)
		m_grid[row][col].bPass = true;
}

// 983 (00748850): the second pass is a checkerboard fan fill (alpha 0x88339944 / 0), not DrawLine; the ball cell uses fan flag 0x200.
void CGrid::Display()
{
	if (Skipped() || GOLFDOC()->m_tutorialMode < 4 ||
		CCapturedBg::Instance()->IsCaptured() || m_flag.GetFlag(1) || FLAG(2))
		return;

	int i, j, k;

	for (i = 0; i < m_rows + (IS_1PT() ? m_extraRows : 0) - 1; i++)
	{
		for (j = 0; j < m_cols + (IS_1PT() ? m_extraCols : 0) - 1; j++)
		{
			if (m_grid[i][j].bPass)
			{
				for (k = 0; k < 4; k++)
				{
					WVector pos =
						m_grid[i + (k & 1)][j + (((unsigned)k >> 1) & 1)].pos;
					m_vtx[k].SetPosition(pos);
					m_vtx[k].diffuse = 0x80ffffff;
				}

				g_view->DrawPolygonFan(m_pVtx, 0x20000000, 0, false);
			}
		}
	}

	int c = 0;
	for (i = 0; i < m_rows + (IS_1PT() ? m_extraRows : 0) - 1; i++, c++)
	{
		int p = c + i;
		for (j = 0; j < m_cols + (IS_1PT() ? m_extraCols : 0) - 1; j++)
		{
			c++;
			p++;
			unsigned color = (p % 2) ? 0x00000000 : 0x88339944;
			for (k = 0; k < 4; k++)
			{
				WVector pos =
					m_grid[i + (k & 1)][j + (((unsigned)k >> 1) & 1)].pos;
				m_vtx[k].SetPosition(pos);
				m_vtx[k].diffuse = color;
			}

			g_view->DrawPolygonFan(m_pVtx, 0x20000000, 0, false);
		}
	}


	for (i = 0; i < m_rows + (IS_1PT() ? m_extraRows : 0) - 1; i++)
	{
		for (j = 0; j < m_cols + (IS_1PT() ? m_extraCols : 0) - 1; j++)
		{
			for (k = 0; k < 4; k++)
			{
				WVector pos = m_grid[i][j].slopePos;
				pos.x += ((k & 2) ? 1.0f : -1.0f) * 0.2f;
				pos.z += ((k & 1) ? 1.0f : -1.0f) * 0.2f;
				m_vtx[k].SetPosition(pos);

				if (m_bBlink)
				{
					if (m_blinkTime > 0.5f)
					{
						m_blinkTime = 0.0f;
						m_blinkColor =
							(m_blinkColor == 0xffff0000) ? m_color : 0xffff0000;
					}
					m_vtx[k].diffuse = m_blinkColor;
				}
				else
				{
					m_vtx[k].diffuse =
						m_grid[i][j].bPass ? 0xffff0000 : m_color;
				}
			}

			g_view->DrawPolygonFan(m_pVtx, (m_texSlope & 0x7ff) | 0x20000000, 0,
				false);
		}
	}

	if (FLAG(1) || !FLAG(3) || m_ballRow < 0 || m_ballCol < 0)
		return;

	for (k = 0; k < 4; k++)
	{
		m_vtx[k].SetPosition(
			m_grid[m_ballRow + (k & 1)][m_ballCol + (((unsigned)k >> 1) & 1)]
				.pos);
		m_vtx[k].diffuse = 0x5000ff00;
	}

	g_view->DrawPolygonFan(m_pVtx, 0, 0x200, false);
}


void CGrid::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case 0x2718:
		m_angle = *(float*)msg.param1;
		SetGridType();
		m_flag.Enable(0x10);
		break;

	case M983(0x1a1):
		m_angle += *(float*)msg.param1;
		m_flag.Enable(0x10);
		break;

	case M983(0x74):
		if (msg.param1)
			m_pos = *(WVector*)msg.param1;
		SetGridType();
		break;

	case M983(0x1d2):
		*(float*)msg.param1 = m_blockSize + (IS_1PT() ? m_extraSize : 0.0f);
		*(int*)msg.param2 = m_rows + (IS_1PT() ? m_extraRows : 0);
		break;

	case M983(0x163):
		m_flag.Disable(4);
		break;

	case M983(0x164):
		if (!m_bShow)
			m_flag.Enable(4);
		break;

	case M983(0x87):
		m_bShow = msg.param1 != 0;
		break;

	case M983(0x1d3):
		m_lineOffset = (float)(msg.param1 * (double)3.2f);
		break;

	case M983(0x1d4):
		SetHoleGrid();
		break;

	case M983(0x207):
		if (IsLocalContent(S3_TUTORIAL_RENEWAL))
		{
			if (msg.param1)
			{
				m_bBlink = true;
				m_blinkTime = 0.0f;
				m_blinkColor = 0xffff0000;
			}
			else
				m_bBlink = false;
		}
		break;
	}
}

void CGrid::MakeStaticGrid()
{
	for (int i = 0; i < m_rows; i++)
	{
		for (int j = 0; j < m_cols; j++)
		{
			WVector pos = m_pos +
				WVector((j - m_cols / 2) * m_blockSize, 0.0f,
					(i - m_rows / 2) * m_blockSize);
			m_grid[i][j].pos = GetPVS().GetGroundPoint(pos, true, NULL, true) +
				WVector(0.0f, GolfBall().GetRadius(), 0.0f);
		}
	}

	m_dirZ = WVector::UNIT_POS_Z;
	m_dirX = WVector::UNIT_POS_X;
	MakeSlopeIndex();
}

void CGrid::MakeDynamicGrid()
{
	for (int i = 0; i < m_rows + (IS_1PT() ? m_extraRows : 0); i++)
	{
		for (int j = 0; j < m_cols + (IS_1PT() ? m_extraCols : 0); j++)
		{
			WVector pos = GolfBall().m_pos +
				(m_blockSize + (IS_1PT() ? m_extraSize : 0.0f)) *
					(m_dirZ * (float)i +
						m_dirX *
							((j + 0.5f) -
								(m_cols + (IS_1PT() ? m_extraCols : 0)) / 2));
			m_grid[i][j].pos = GetPVS().GetGroundPoint(pos, true, NULL, true) +
				WVector(0.0f, GolfBall().GetRadius(), 0.0f);
		}
	}
}

int CGrid::GetColor(float slope)
{
	float t = (slope > 0.0f ? slope : -slope) / 0.32f;
	if (t > 1.0f)
		t = 1.0f;

	if (slope > 0.0f)
		return ULblend(0xffa1c400, 0xffc6f2de, (unsigned char)(t * 255.0f));
	return ULblend(0xffa1c400, 0xff00562f, (unsigned char)(t * 255.0f));
}

void CGrid::SetHoleGrid()
{
	WVector pin = GOLFDOC()->GetHoleData().pin;
	WVector d = pin - GolfBall().m_pos;
	d.y = 0.0f;

	float size = m_blockSize + (IS_1PT() ? m_extraSize : 0.0f);

	m_ballRow = (int)(d * m_dirZ / size);
	m_ballCol = (int)floorf(d * m_dirX / size - 0.5f) +
		(m_cols + (IS_1PT() ? m_extraCols : 0)) / 2;

	if (m_ballRow > m_rows + (IS_1PT() ? m_extraRows : 0) - 2)
		m_ballRow = -1;
	if (m_ballCol > m_cols + (IS_1PT() ? m_extraCols : 0) - 2)
		m_ballCol = -1;

	m_bBallGrid = true;

	float unit;
	if (GolfClub().GetApproachMode() > 1)
		unit = 6.4f;
	else if (IS_1PT())
		unit = 6.4f;
	else
		unit = 3.2f;

	int count = (int)(d.Magnitude() / unit);
	int rows = m_rows + (IS_1PT() ? m_extraRows : 0);
	int half = (int)((m_cols + (IS_1PT() ? m_extraCols : 0)) * 0.5f);

	if (GolfBall().m_groundType == 2 && GolfClub().GetType() == 3 && half >= 1)
	{
		float* pData = new float[rows];
		pData[0] = (m_grid[0][half].pos.y + m_grid[0][half - 1].pos.y) * 0.5f;
		for (int i = 1; i < rows; i++)
		{
			pData[i] =
				(m_grid[i][half].pos.y + m_grid[i][half - 1].pos.y +
					m_grid[i - 1][half - 1].pos.y + m_grid[i - 1][half].pos.y) *
				0.25f;
		}
		SendMsg(this, "Screen", M983(0x1ae), count, rows - 1, (int)pData, 0);
		delete[] pData;
	}
	else
		SendMsg(this, "Screen", M983(0x1ae), count, rows - 1, 0, 0);
}

void CGrid::MoveLine(float delta)
{
	m_moveLine += delta;
}

void CGrid::MakeSlopeIndex()
{
	float size = m_blockSize + (IS_1PT() ? m_extraSize : 0.0f);

	for (int i = 0; i < m_rows + (IS_1PT() ? m_extraRows : 0) - 1; i++)
	{
		for (int j = 0; j < m_cols + (IS_1PT() ? m_extraCols : 0) - 1; j++)
		{
			float sz = GetSlope(i, j, size, true);
			if (sz < -0.45f)
				sz = -0.45f;
			else if (sz > 0.45f)
				sz = 0.45f;
			sz += 0.5f;

			float sx = GetSlope(i, j, size, false);
			if (sx < -0.45f)
				sx = -0.45f;
			else if (sx > 0.45f)
				sx = 0.45f;
			sx += 0.5f;

			m_grid[i][j].slopePos = GetPVS().GetGroundPoint(m_grid[i][j].pos +
											(m_dirZ * sz + m_dirX * sx) * size,
										true, NULL, true) +
				WVector(0.0f, GolfBall().GetRadius(), 0.0f);
		}
	}
}

float CGrid::GetSlope(int row, int col, float size, bool bRow)
{
	if (bRow)
		return atanf((GetPVS().GetGreenPos(m_grid[row][col].pos) -
						 GetPVS().GetGreenPos(m_grid[row + 1][col].pos)) /
				   size) *
			8.0f;
	return atanf((GetPVS().GetGreenPos(m_grid[row][col].pos) -
					 GetPVS().GetGreenPos(m_grid[row][col + 1].pos)) /
			   size) *
		8.0f;
}
