#define REBANG_FXSEQUENCE_CPP
#define REBANG_FX_CPP
#include "minatl.h"
#include "projectg.h"
#include "fx.h"
#include "golfball.h"
#include "quadtree.h"
#include "shareddoc.h"
#include "club.h"
#include "../shared/itemmanager.h"
#include "taskmanager.h"
#include "actor.h"
#include "contentsdoc.h"
#include "gimmickcontainer.h"
#include "mathconsts.h"
#include "wpuppet.h"


extern HWND g_hwnd;

bool IsEqual(float a, float b);
WMatrix RotMat(const WVector& axis, float angle);

// course name ("Data" field of the course IFF entry) of the current game
#define GetCourseName() \
	(Doc()->m_golfGame.pCourse ? Doc()->m_golfGame.pCourse->Data : NULL)

unsigned long CFxControl::ms_serial;

Attacher::Attacher()
{
	x = g_HUGE;
	y = g_HUGE;
	for (int i = 0; i < 4; i++)
	{
		mat[i] = NULL;
		pos[i] = NULL;
	}
}

bool Attacher::Get(WVector& out)
{
	out.Reset();

	int i;
	for (i = 0; i < sizeof(mat) / sizeof(mat[0]); i++)
	{
		if (pos[i])
		{
			out += mat[i] ? *pos[i] * *mat[i] : *pos[i];
		}
		else if (mat[i])
		{
			out += mat[i]->pivot;
		}
		else
		{
			break;
		}
	}

	if (i > 0)
	{
		out /= (float)i;
		return true;
	}
	return false;
}

void CPvsFxBox::ProcessAttacher(float elapsed)
{
	WVector pos;

	std::map<CFxSequence*, CFxSequence*>::iterator it = m_sequence.begin();
	for (; it != m_sequence.end(); ++it)
	{
		CFxSequence* sequence = (*it).second;
		std::map<CFxSequence*, Attacher>::iterator found =
			m_seqAttacher.find(sequence);
		if (found != m_seqAttacher.end() && (*found).second.Get(pos))
			sequence->m_pos = pos;
	}

	std::map<CFxSpray*, CFxSpray*>::iterator itSpray = m_spray.begin();
	for (; itSpray != m_spray.end(); ++itSpray)
	{
		CFxSpray* spray = (*itSpray).second;
		std::map<CFxSpray*, Attacher>::iterator found =
			m_sprayAttacher.find(spray);
		if (found != m_sprayAttacher.end())
		{
			if ((*found).second.Get(pos))
				spray->m_pos = pos;
		}
	}
}

bool CPvsFxBox::CheckGroundCollision(const WVector& from, const WVector& to,
	float& height, float radius)
{
	if (height > m_groundHeightMax || height < m_groundHeightMin)
		return false;

	float fromGround = GetPVS().GetGroundPoint(from, true, NULL, true).y;
	float toGround = GetPVS().GetGroundPoint(to, true, NULL, true).y;
	height = toGround;

	float diff = to.y - toGround;
	if (diff < 0.00001f && diff > -0.00001f)
		return true;

	if (to.y < toGround)
	{
		if (IsEqual(from.y, fromGround) || from.y < fromGround)
		{
			// 983 00878e80: soma em double; IsEqual recebe o float, a
			// comparacao usa o double.
			double top = to.y + radius;
			return IsEqual((float)top, toGround) || top > toGround;
		}
	}
	else
	{
		if (IsEqual(from.y, fromGround) || from.y > fromGround)
		{
			double bottom = to.y - radius;
			return IsEqual((float)bottom, toGround) || bottom < toGround;
		}
	}
	return true;
}

void CPvsFxBox::OnSequenceCreated(CFxSequence* sequence)
{
	const Attacher* attacher = sequence->m_pAttacher;
	if (attacher)
	{
		m_seqAttacher.insert(
			std::map<CFxSequence*, Attacher>::value_type(sequence, *attacher));
		sequence->m_pAttacher = &m_seqAttacher[sequence];
		memcpy(sequence->m_attachMat, m_seqAttacher[sequence].mat,
			sizeof(sequence->m_attachMat));
		memcpy(sequence->m_attachPos, m_seqAttacher[sequence].pos,
			sizeof(sequence->m_attachPos));
	}
}

void CPvsFxBox::OnSequenceDeleted(CFxSequence* sequence)
{
	std::map<CFxSequence*, Attacher>::iterator it =
		m_seqAttacher.find(sequence);
	if (it != m_seqAttacher.end())
		m_seqAttacher.erase(it);
}

void CPvsFxBox::OnSprayCreated(CFxSpray* spray)
{
	const Attacher* attacher = spray->m_pAttacher;
	if (attacher)
	{
		m_sprayAttacher.insert(
			std::map<CFxSpray*, Attacher>::value_type(spray, *attacher));
		spray->m_pAttacher = &m_sprayAttacher[spray];
		memcpy(spray->m_attachMat, m_sprayAttacher[spray].mat,
			sizeof(spray->m_attachMat));
		memcpy(spray->m_attachPos, m_sprayAttacher[spray].pos,
			sizeof(spray->m_attachPos));
	}
}

void CPvsFxBox::OnSprayDeleted(CFxSpray* spray)
{
	std::map<CFxSpray*, Attacher>::iterator it = m_sprayAttacher.find(spray);
	if (it != m_sprayAttacher.end())
		m_sprayAttacher.erase(it);
}

void CPvsFxBox::UpdateLightSrc(const LightSet& light)
{
	for (std::map<CFxSequence*, CFxSequence*>::iterator it = m_sequence.begin();
		it != m_sequence.end(); ++it)
	{
		CFxSequence* sequence = (*it).second;

		std::map<CFxSpray*, CFxSpray*>& sprayList = sequence->GetSprayList();
		std::map<CFxSpray*, CFxSpray*>::iterator itSpray;
		for (itSpray = sprayList.begin(); itSpray != sprayList.end(); ++itSpray)
		{
			(*itSpray).second->SetLight(light);
		}
	}

	std::map<CFxSpray*, CFxSpray*>::iterator itSpray;
	for (itSpray = m_spray.begin(); itSpray != m_spray.end(); ++itSpray)
		(*itSpray).second->SetLight(light);
}

CFx::CFx()
{
	m_pvsFxBox.Init(g_resrcmng);
	m_pvsFxBox.SetCamera(&g_camera);
	m_pvsFxBox.m_frameTime = 1000.0f / 30.0f;
	m_bActive = true;
}

CFx::~CFx()
{
	ClearControl();
}

void CFx::Process(float elapsed)
{
	if (!m_bActive)
		return;

	m_pvsFxBox.ProcessAttacher(elapsed);

	std::map<int, CFxControl*>::iterator it = m_control.begin();
	while (it != m_control.end())
	{
		CFxControl* control = (*it).second;

		if (!control->Process(elapsed))
		{
			m_control.erase(it++);
			delete control;
		}
		else
		{
			++it;
		}
	}

	m_pvsFxBox.Process(elapsed * 1000.0f);
}

void CFx::Display()
{
	if (m_bActive)
		m_pvsFxBox.Display(g_view);
}

CFxSequence* CFx::OpenSequence(const char* filename, bool bDelete)
{
	CFxSequence* sequence = m_pvsFxBox.LoadSequence(filename, bDelete, NULL);
	if (sequence)
		m_pvsFxBox.AddSequence(sequence);

	return sequence;
}

void CFx::CloseSequence(CFxSequence* sequence, bool bDeleteChild)
{
	m_pvsFxBox.DeleteSequence(sequence, bDeleteChild);
}

CFxSpray* CFx::OpenSpray(const char* filename)
{
	CFxSpray* spray = m_pvsFxBox.LoadSpray(filename, NULL);
	if (spray)
		m_pvsFxBox.AddSpray(spray);

	return spray;
}

void CFx::CloseSpray(CFxSpray* spray)
{
	m_pvsFxBox.DeleteSpray(spray);
}

void CFx::ClearAll(bool bMarkedOnly)
{
	ClearControl();
	m_pvsFxBox.ClearAll(bMarkedOnly);
}

void CFx::AddFxKeeper(sFxKeeper& keeper)
{
	m_pvsFxBox.AddFxKeeper(keeper);
}

void CFx::EraseFxKeeper(CFxKeeper* keeper)
{
	m_pvsFxBox.EraseFxKeeper(keeper);
}

void CFx::ClearFxKeeper()
{
	m_pvsFxBox.ClearFxKeeper();
}

CFxSpray* CFx::GetSpray(int index)
{
	return m_pvsFxBox.GetSpray(index);
}

bool CFx::FindSequence(CFxSequence* sequence)
{
	return m_pvsFxBox.FindSequence(sequence);
}

bool CFx::FindSpray(CFxSpray* spray)
{
	return m_pvsFxBox.FindSpray(spray);
}

void* CFx::Attach(const Attacher& attacher, const char* filename, bool bMark,
	defines_t* defines)
{
	int len = strlen(filename);
	if (len < 5)
		return NULL;

	if (strcmpi(filename + len - 3, "spr") == 0)
	{
		CFxSpray* spray = m_pvsFxBox.LoadSpray(filename, defines);
		if (spray == NULL)
			return NULL;

		spray->m_bMark = bMark;
		spray->m_pAttacher = &attacher;
		spray->m_attachX = attacher.x;
		spray->m_attachY = attacher.y;
		m_pvsFxBox.AddSpray(spray);
		return spray;
	}
	else
	{
		CFxSequence* sequence =
			m_pvsFxBox.LoadSequence(filename, true, defines);
		if (sequence == NULL)
			return NULL;

		sequence->m_bMark = bMark;
		sequence->m_pAttacher = &attacher;
		sequence->m_attachX = attacher.x;
		sequence->m_attachY = attacher.y;
		m_pvsFxBox.AddSequence(sequence);
		return sequence;
	}
}

bool CFx::GetOutput(WVector& out, unsigned char& value)
{
	return m_pvsFxBox.GetOutput(out, value);
}

CFxControl* CFx::FindControl(int serial)
{
	std::map<int, CFxControl*>::iterator it = m_control.find(serial);
	if (it != m_control.end())
		return (*it).second;

	return NULL;
}

int CFx::MakeRunDust(const WVector& pos, const WVector& vel)
{
	return AddFxControl(new CFxRunDust(m_pvsFxBox, pos, vel));
}

bool CFx::RemoveRunDust(int serial)
{
	std::map<int, CFxControl*>::iterator it = m_control.find(serial);
	if (it != m_control.end())
	{
		CFxControl* control = (*it).second;
		m_control.erase(it);
		delete control;
		return true;
	}

	return false;
}

int CFx::MakeCobra(const CGolfBall& ball)
{
	return AddFxControl(new CFxCobra(m_pvsFxBox, ball));
}

int CFx::MakeTomahawk(const CGolfBall& ball, bool bNotify,
	unsigned long itemId)
{
	return AddFxControl(new CFxTomahawk(m_pvsFxBox, ball, bNotify, itemId));
}

int CFx::MakePowerSpin(CGolfBall& ball)
{
	return AddFxControl(new CFxPowerSpin(m_pvsFxBox, ball));
}

int CFx::MakeShotImpact(CGolfBall& ball)
{
	return AddFxControl(new CFxShotImpact(m_pvsFxBox, ball));
}

int CFx::MakeClubImpact(CGolfBall& ball, unsigned long clubSet)
{
	return AddFxControl(new CFxClubImpact(m_pvsFxBox, ball, clubSet));
}

int CFx::MakeSpike(const CGolfBall& ball, bool bNotify)
{
	return AddFxControl(new CFxSpike(m_pvsFxBox, ball, bNotify));
}

int CFx::MakePowerShot2(CGolfBall& ball)
{
	return AddFxControl(new CFxPowerShot2(m_pvsFxBox, ball));
}

int CFx::MakeBoosterWarp(CGolfBall& ball, unsigned char type)
{
	return AddFxControl(new CFxBoosterWarp(m_pvsFxBox, ball, type));
}

int CFx::AddFxControl(CFxControl* control)
{
	m_control.insert(
		std::map<int, CFxControl*>::value_type(control->m_serial, control));
	return control->m_serial;
}

void CFx::ClearControl()
{
	for (std::map<int, CFxControl*>::iterator it = m_control.begin();
		it != m_control.end(); ++it)
	{
		delete (*it).second;
	}

	m_control.clear();
}

void CFx::SetActive(bool bActive)
{
	m_bActive = bActive;
}

void CFx::UpdateExtWind(WVector wind)
{
	std::map<CFxSpray*, CFxSpray*>::iterator it;
	for (it = m_pvsFxBox.GetSprayList().begin();
		it != m_pvsFxBox.GetSprayList().end(); ++it)
	{
		(*it).second->SetWind(wind);
	}

	std::map<CFxSequence*, CFxSequence*>& sequenceList =
		m_pvsFxBox.GetSequenceList();
	for (std::map<CFxSequence*, CFxSequence*>::iterator itSeq =
			 sequenceList.begin();
		itSeq != sequenceList.end(); ++itSeq)
	{
		(*itSeq).second->m_wind = wind;
	}
}

CFxRunDust::CFxRunDust(CPvsFxBox& fxBox, const WVector& pos, const WVector& vel)
	: m_fxBox(fxBox), m_pos(pos), m_vel(vel)
{
	m_pGround = m_fxBox.LoadSequence("GroundRun.seq", false, NULL);
	m_pWater = m_fxBox.LoadSequence("WaterRun.seq", false, NULL);

	if (m_pGround && m_pWater)
	{
		m_fxBox.AddSequence(m_pGround);
		m_fxBox.AddSequence(m_pWater);
	}

	m_serial = ms_serial++;
}

CFxRunDust::~CFxRunDust()
{
	m_fxBox.DeleteSequence(m_pGround, false);
	m_fxBox.DeleteSequence(m_pWater, false);

	delete m_pGround;
	delete m_pWater;
}

bool CFxRunDust::Process(float elapsed)
{
	WVector pos = m_pos;
	pos.y = GetPVS().GetGroundPoint(m_pos, true, NULL, true).y;

	float top = pos.y + 1.2f;
	if (m_vel.Magnitude() < 0.5f || m_pos.y > top)
	{
		m_pGround->SetActive(false, false);
		m_pWater->SetActive(false, false);
	}
	else
	{
		CFxSequence* sequence;
		if (GetPVS().IsOverWater(pos))
		{
			m_pGround->SetActive(false, false);
			m_pWater->SetActive(true, false);

			sequence = m_pWater;
		}
		else
		{
			m_pGround->SetActive(true, false);
			m_pWater->SetActive(false, false);

			sequence = m_pGround;
		}

		sequence->Pos() = pos;
	}

	return true;
}

CFxCobra::CFxCobra(CPvsFxBox& fxBox, const CGolfBall& ball)
	: m_fxBox(fxBox), m_ball(ball)
{
	m_bStarted = false;

	m_pSequence[0] = m_fxBox.LoadSequence("Cobra_BG.seq", false, NULL);

	m_pSequence[1] = m_fxBox.LoadSequence("Cobra_BW.seq", false, NULL);
	m_pSequence[2] = m_fxBox.LoadSequence("Cobra_BR.seq", false, NULL);
	m_pSequence[3] = m_fxBox.LoadSequence("Cobra_C.seq", false, NULL);

	if (m_pSequence[0] && m_pSequence[1] && m_pSequence[2] && m_pSequence[3])
	{
		m_fxBox.AddSequence(m_pSequence[0]);
		m_fxBox.AddSequence(m_pSequence[1]);
		m_fxBox.AddSequence(m_pSequence[2]);
		m_fxBox.AddSequence(m_pSequence[3]);
	}

	m_serial = ms_serial++;
}

CFxCobra::~CFxCobra()
{
	for (int i = 0; i < 4; i++)
	{
		m_fxBox.DeleteSequence(m_pSequence[i], false);
		delete m_pSequence[i];
		m_pSequence[i] = NULL;
	}
}

bool CFxCobra::Process(float elapsed)
{
	if (!(m_ball.m_special & 0x20) || !m_pSequence[0] || !m_pSequence[1] ||
		!m_pSequence[2] || !m_pSequence[3])
	{
		return false;
	}

	WVector pos = m_ball.m_pos;
	pos.y = GetPVS().GetGroundPoint(m_ball.m_pos, true, NULL, true).y;

	if (m_bStarted)
	{
		if (m_ball.m_vel.y < 0.0f)
			return false;

		m_pSequence[3]->Pos() = m_ball.m_pos;

		// 983 0087ce50: !(y < topo) (NaN cai no ramo de apagar)
		float top = pos.y + 22.4f;
		if (!(m_ball.m_pos.y < top) || m_ball.m_vel.Magnitude() < 0.5f)
		{
			m_pSequence[0]->SetActive(false, false);
			m_pSequence[1]->SetActive(false, false);
			m_pSequence[2]->SetActive(false, false);
		}
		else if (GetPVS().IsOverWater(pos))
		{
			m_pSequence[0]->SetActive(false, false);
			m_pSequence[1]->SetActive(true, false);
			m_pSequence[2]->SetActive(true, false);

			m_pSequence[1]->Pos() = pos;
			m_pSequence[2]->Pos() = pos;
		}
		else
		{
			m_pSequence[0]->SetActive(true, false);
			m_pSequence[1]->SetActive(false, false);
			m_pSequence[2]->SetActive(false, false);

			m_pSequence[0]->Pos() = pos;
		}
	}
	else if (m_ball.m_vel.Magnitude() > 1.0f)
	{
		m_bStarted = true;
		CFxSequence* sequence = m_fxBox.LoadSequence("ACobra_A.seq", true, NULL);
		m_fxBox.AddSequence(sequence);
		sequence->Pos() = m_ball.m_pos;
	}

	return true;
}

// 983 008794c0: itens (bolas/skins) do tomahawk com pluma
// (array na pilha; membro thiscall/ret 4: o 983 chama com ecx = this)
bool CFxTomahawk::IsPlumeItem(unsigned long id)
{
	unsigned long ids[] = {0x10000083, 0xc0001fe, 0xc0001ff,
		0xc000200, 0xc000201, 0x10000084, 0xc000202, 0xc000203, 0xc000204,
		0xc000205, 0x10000085, 0xc000206, 0xc000207, 0xc000208, 0xc000209};
	for (unsigned int i = 0; i < 15; i++)
		if (id == ids[i])
			return true;
	return false;
}

CFxTomahawk::CFxTomahawk(CPvsFxBox& fxBox, const CGolfBall& ball, bool bNotify,
	unsigned long itemId)
	: m_fxBox(fxBox), m_ball(ball)
{
	m_state = 0;
	m_pSequence = NULL;
	m_serial = ms_serial++;
	m_itemId = itemId;
	m_bNotify = bNotify;
}

CFxTomahawk::~CFxTomahawk()
{
	if (m_pSequence)
	{
		m_fxBox.DeleteSequence(m_pSequence, false);
		delete m_pSequence;
		m_pSequence = NULL;
	}
}

bool CFxTomahawk::Process(float elapsed)
{
	WVector ground = GetPVS().GetGroundPoint(m_ball.m_pos, true, NULL, true);

	if (m_ball.m_tomahawkFrame > 0 &&
		m_ball.m_playFrame >= m_ball.m_tomahawkFrame)
	{
		m_state = 3;
	}

	switch (m_state)
	{
	case 0:
		if (m_ball.m_vel.Magnitude() > 1.0f)
		{
			m_state = 1;
			CFxSequence* sequence;
			if (IsPlumeItem(m_itemId) == true)
				sequence = m_fxBox.LoadSequence("plume_tomahawk_a.seq", true, NULL);
			else
				sequence = m_fxBox.LoadSequence("ATomahawk_A.seq", true, NULL);
			if (!sequence)
				return false;
			m_fxBox.AddSequence(sequence);
			sequence->Pos() = m_ball.m_pos;

			m_pSequence = m_fxBox.LoadSequence("Tomahawk_B.seq", false, NULL);
			if (!m_pSequence)
				return false;
			m_fxBox.AddSequence(m_pSequence);
		}
		break;
	case 1:
		if (m_ball.m_vel.y < 0.6f)
		{
			m_state = 2;
			if (m_pSequence)
			{
				m_fxBox.DeleteSequence(m_pSequence, false);
				delete m_pSequence;
				m_pSequence = NULL;
			}

			m_pSequence = m_fxBox.LoadSequence("Tomahawk_C.seq", false, NULL);
			if (!m_pSequence)
				return false;
			m_fxBox.AddSequence(m_pSequence);
		}
		break;
	case 3:
		if (m_pSequence)
		{
			m_fxBox.DeleteSequence(m_pSequence, false);
			delete m_pSequence;
			m_pSequence = NULL;
		}

		CFxSequence* sequence;
		if (IsPlumeItem(m_itemId) == true)
			sequence = m_fxBox.LoadSequence("plume_tomahawk_d.seq", true, NULL);
		else
			sequence = m_fxBox.LoadSequence("Dtomahawk_d2.seq", true, NULL);
		if (sequence)
		{
			m_fxBox.AddSequence(sequence);
			sequence->Pos() = m_ball.m_pos;

			g_audio->PlaySfx("\xb0\xf8_\xc5\xe4\xb8\xb6\xc8\xa3\xc5\xa9");

			// 983 (0087a9b0): a mensagem 0x1ca ao Trajectory e incondicional
			// (o flag do construtor, +0x18, nao e lido no Process)
			AfxGetTask()->GetActor("Trajectory")
				<< MsgObject(NULL, M983(0x15f), 0, 0, 0, 0, 0);
		}
		return false;
	}

	if (m_pSequence)
		m_pSequence->Pos() = m_ball.m_pos;
	return true;
}

CFxPowerSpin::CFxPowerSpin(CPvsFxBox& fxBox, CGolfBall& ball)
	: m_fxBox(fxBox), m_ball(ball)
{
	m_bRotated = false;

	char name[32];
	switch (GetPVS().GetGroundType(ball.m_pos, true, NULL))
	{
	case 3:
	case 6:
		sprintf(name, "PowerSpin_%s_S.seq", GetCourseName());
		break;

	case 5:
		sprintf(name, "PowerSpin_%s_P.seq", GetCourseName());
		break;

	case 9:
		sprintf(name, "PowerSpin_%s_W.seq", GetCourseName());
		break;

	default:
		if (!stricmp(GetCourseName(), "pola"))
			sprintf(name, "PowerSpin_icee_G.seq");
		else
			sprintf(name, "PowerSpin_%s_G.seq", GetCourseName());
		break;
	}

	m_pSequence = m_fxBox.LoadSequence(name, true, NULL);

	if (!m_pSequence)
		m_pSequence = m_fxBox.LoadSequence("PowerSpin_Blue_G.seq", true, NULL);

	if (m_pSequence)
		m_fxBox.AddSequence(m_pSequence);

	m_serial = ms_serial++;
}

CFxPowerSpin::~CFxPowerSpin()
{
}

bool CFxPowerSpin::Process(float elapsed)
{
	if (!m_fxBox.FindSequence(m_pSequence))
		return false;

	if (!m_bRotated)
	{
		std::map<CFxSpray*, CFxSpray*>& sprayList = m_pSequence->GetSprayList();
		if (sprayList.size() != 0)
		{
			WVector dir = m_ball.PreData(m_ball.m_playFrame)->pos -
				m_ball.PreData(0)->pos;
			dir.y = 0.0f;
			dir = (m_ball.m_special & 1) ? dir.Normalize() * -1.0f
										 : dir.Normalize();

			WVector front(0.0f, 0.0f, -1.0f);
			float theta = (float)acos(
				dir.x * front.x + dir.y * front.y + dir.z * front.z);
			float angle = (dir.x < 0.0f) ? g_PI * 2.0f - theta : theta;
			WMatrix mat = RotMat(WVector(0.0f, 1.0f, 0.0f), angle);

			for (std::map<CFxSpray*, CFxSpray*>::iterator it =
					 sprayList.begin();
				it != sprayList.end(); ++it)
			{
				CFxSpray* spray = (*it).second;
				spray->InitialMaxVel() = spray->InitialMaxVel() * mat;
				spray->InitialMinVel() = spray->InitialMinVel() * mat;
			}

			m_bRotated = true;
		}
	}
	else
	{
		std::map<CFxSpray*, CFxSpray*>& sprayList = m_pSequence->GetSprayList();
		for (std::map<CFxSpray*, CFxSpray*>::iterator it = sprayList.begin();
			it != sprayList.end(); ++it)
		{
			(*it).second->m_pos = m_ball.m_pos;
		}
	}

	return true;
}

CFxShotImpact::CFxShotImpact(CPvsFxBox& fxBox, CGolfBall& ball)
	: m_fxBox(fxBox), m_ball(ball)
{
	m_bRotated = false;

	char name[32];
	switch (GetPVS().GetGroundType(ball.m_pos, false, NULL))
	{
	case 3:
		sprintf(name, "ShotImpact_%s_S.seq", GetCourseName());
		m_pSequence = m_fxBox.LoadSequence(name, true, NULL);
		break;

	case 5:
	case 7:
	case 8:
		m_pSequence = m_fxBox.LoadSequence("ShotImpact_P.seq", true, NULL);
		break;

	case 4:
		sprintf(name, "ShotImpact_%s_G.seq", GetCourseName());
		m_pSequence = m_fxBox.LoadSequence(name, true, NULL);
		break;

	case 0:
		m_pSequence = m_fxBox.LoadSequence("ShotImpact_T.seq", true, NULL);
		break;

	case 9:
		m_pSequence = m_fxBox.LoadSequence("ShotImpact_W.seq", true, NULL);
		break;

	default:
		m_pSequence = NULL;
		break;
	}

	if (m_pSequence)
	{
		m_fxBox.AddSequence(m_pSequence);

		m_pSequence->Pos() = m_ball.PreData(0)->pos;
	}

	m_serial = ms_serial++;
}

CFxShotImpact::~CFxShotImpact()
{
}

bool CFxShotImpact::Process(float elapsed)
{
	if (!m_fxBox.FindSequence(m_pSequence))
		return false;

	if (!m_bRotated)
	{
		std::map<CFxSpray*, CFxSpray*>& sprayList = m_pSequence->GetSprayList();
		if (sprayList.size() != 0)
		{
			WVector dir = m_ball.PreData(0)->vel;
			dir.y = 0.0f;
			dir.Normalize();

			WVector front(0.0f, 0.0f, -1.0f);
			float theta = (float)acos(
				dir.x * front.x + dir.y * front.y + dir.z * front.z);
			float angle = (dir.x < 0.0f) ? g_PI * 2.0f - theta : theta;
			WMatrix mat = RotMat(WVector(0.0f, 1.0f, 0.0f), angle);

			for (std::map<CFxSpray*, CFxSpray*>::iterator it =
					 sprayList.begin();
				it != sprayList.end(); ++it)
			{
				CFxSpray* spray = (*it).second;
				spray->InitialMaxVel() = spray->InitialMaxVel() * mat;
				spray->InitialMinVel() = spray->InitialMinVel() * mat;
			}

			m_bRotated = true;
		}
	}

	return true;
}

// Opens the impact effect of the club set for the club type in hand
// ("<icon>_<w|i|s|p>_impact.seq", else the ".spr" of that name) at the ball.
CFxClubImpact::CFxClubImpact(CPvsFxBox& fxBox, CGolfBall& ball,
	unsigned long clubSet)
	: m_fxBox(fxBox), m_ball(ball)
{
	const char* typeName[4] = { "w", "i", "s", "p" };
	char name[256];

	IFF_STRUCT::sClubSet* club = ItemManager()->FindClubSet(clubSet);

	sprintf(name, "%s_%s_impact.seq", club->c.Icon,
		typeName[GolfClub().GetType()]);

	CFxSequence* sequence = m_fxBox.LoadSequence(name, true, NULL);

	if (sequence)
	{
		m_fxBox.AddSequence(sequence);
		sequence->Pos() = m_ball.PreData(0)->pos;
	}
	else
	{
		sprintf(name, "%s_%s_impact.spr", club->c.Icon,
			typeName[GolfClub().GetType()]);

		CFxSpray* spray = m_fxBox.LoadSpray(name, NULL);

		if (spray)
		{
			m_fxBox.AddSpray(spray);
			spray->m_pos = m_ball.PreData(0)->pos;
		}
	}
}

CFxClubImpact::~CFxClubImpact()
{
}

bool CFxClubImpact::Process(float elapsed)
{
	return false;
}

CFxSpike::CFxSpike(CPvsFxBox& fxBox, const CGolfBall& ball, bool bNotify)
	: m_fxBox(fxBox), m_ball(ball)
{
	m_pSequence[0] = m_fxBox.LoadSequence("CSpike_C.seq", false, NULL);
	m_pSequence[1] = m_fxBox.LoadSequence("Spike_ball.seq", false, NULL);

	m_state = 0;
	m_serial = ms_serial++;
	m_bNotify = bNotify;
}

CFxSpike::~CFxSpike()
{
	for (int i = 0; i < 2; i++)
	{
		if (m_pSequence[i])
		{
			m_fxBox.DeleteSequence(m_pSequence[i], false);
			delete m_pSequence[i];
		}
	}
}

bool CFxSpike::Process(float elapsed)
{
	WVector ground = GetPVS().GetGroundPoint(m_ball.m_pos, true, NULL, true);

	if (m_ball.m_spikeEndFrame > 0 &&
		m_ball.m_playFrame >= m_ball.m_spikeEndFrame)
	{
		m_state = 4;
	}

	switch (m_state)
	{
	case 0:
	{
		m_fxBox.AddSequence(m_pSequence[0]);
		m_pSequence[0]->Pos() = m_ball.m_pos;

		CFxSequence* sequence = m_fxBox.LoadSequence("ASpike_A.seq", true, NULL);
		if (!sequence)
			return false;
		m_fxBox.AddSequence(sequence);
		sequence->Pos() = m_ball.m_pos;

		m_state = 1;
		break;
	}
	case 1:
		if (m_ball.m_topFrame < m_ball.m_playFrame)
		{
			CFxSequence* sequence =
				m_fxBox.LoadSequence("spike_shot-01.seq", true, NULL);
			if (!sequence)
				return false;
			m_fxBox.AddSequence(sequence);
			sequence->Pos() = m_ball.m_pos;

			g_audio->PlaySfx("spike_ball_duration");

			m_state = 2;
		}
		else
		{
			m_pSequence[0]->Pos() = m_ball.m_pos;
		}

		if (!(m_ball.m_special & 0x40))
			return false;
		break;
	case 2:
		if (m_ball.m_topFrame + 60 < m_ball.m_playFrame)
		{
			CFxSequence* sequence =
				m_fxBox.LoadSequence("SSpike_shot.seq", true, NULL);
			m_fxBox.AddSequence(sequence);
			sequence->Pos() = m_ball.m_pos;

			m_fxBox.AddSequence(m_pSequence[1]);
			m_pSequence[1]->Pos() = m_ball.m_pos;

			m_pSequence[0]->SetActive(false, false);

			g_audio->PlaySfx("spike_beam");

			m_state = 3;
		}
		break;
	case 3:
		m_pSequence[1]->Pos() = m_ball.m_pos;
		break;
	case 4:
	{
		CFxSequence* sequence =
			m_fxBox.LoadSequence("DSpike_d.seq", true, NULL);
		if (!sequence)
			return false;
		m_fxBox.AddSequence(sequence);
		sequence->Pos() = m_ball.m_pos;

		m_pSequence[1]->SetActive(false, false);

		g_audio->PlaySfx("\xb0\xf8_\xc5\xe4\xb8\xb6\xc8\xa3\xc5\xa9");

		if (m_bNotify)
		{
			AfxGetTask()->GetActor("Trajectory")
				<< MsgObject(NULL, M983(0x15f), 0, 0, 0, 0, 0);
		}
		return false;
	}
	}

	return true;
}

CFxPowerShot2::CFxPowerShot2(CPvsFxBox& fxBox, CGolfBall& ball)
	: m_fxBox(fxBox), m_ball(ball)
{
	m_serial = ms_serial++;

	m_bDone = false;
}

CFxPowerShot2::~CFxPowerShot2()
{
}

bool CFxPowerShot2::Process(float elapsed)
{
	if (!m_bDone)
	{
		CFxSequence* sequence =
			m_fxBox.LoadSequence("woodwave2.seq", true, NULL);

		if (sequence)
		{
			m_fxBox.AddSequence(sequence);

			WVector pos = m_ball.PreData(0)->pos;
			pos.y += 1.6f;

			sequence->Pos() = pos;
		}
		else
		{
			MessageBoxA(g_hwnd, "Couldn't load 'woodwave2.seq'.", "Error",
				MB_OK);
		}

		m_bDone = true;
	}
	return true;
}

// Booster warp of the ball; type 6 is the magic harp gimmick, which swaps the
// wave for the carpet sequence (the 983 plays the harp sounds in CMagicCarpet).
CFxBoosterWarp::CFxBoosterWarp(CPvsFxBox& fxBox, CGolfBall& ball,
	unsigned char type)
	: m_fxBox(fxBox), m_ball(ball)
{
	m_time = 0.0f;
	m_bHarp = 0;
	m_bLoaded = false;
	m_serial = ms_serial++;

	std::string twistName = "Twist.seq";
	std::string waveName = "Wave.seq";

	if (GetCurMap() == 0x14)
		waveName = "ab_booster_eff.seq";

	if (type == 6)
	{
		CGimmickContainer* gimmick = NULL;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0x9b,
			gimmick);
		waveName = gimmick->m_carpetSeqName;
		m_bHarp = 1;
	}

	m_pSequence[0] = m_fxBox.LoadSequence(twistName.c_str(), false, NULL);
	m_pSequence[1] = m_fxBox.LoadSequence(waveName.c_str(), false, NULL);

	if (m_pSequence[0] && m_pSequence[1])
	{
		m_fxBox.AddSequence(m_pSequence[0]);
		m_fxBox.AddSequence(m_pSequence[1]);

		if (type == 6)
		{
			m_pSequence[0]->SetActive(false, false);
		}
		else
		{
			m_pSequence[0]->Pos() = m_ball.m_pos;
			m_pSequence[0]->SetActive(true, false);
		}

		// Map 0x14: the wave goes to the "center" box of the object that the
		// ball's next pre-data record points to (+0x14 puppet, +0x108 position).
		// 983 0087e4e0: GetCurMap() de novo aqui (sem bool guardado) e sem
		// checar o registro do PreData.
		if (GetCurMap() == 0x14)
		{
			const unsigned char* obj = (const unsigned char*)
				m_ball.PreData(m_ball.m_playFrame + 1)->hitObj983;
			if (obj)
			{
				const w_bound_box* box = (*(WPuppet**)(obj + 0x14))
					->FindBoundBox("center");
				const Waabb& aabb = box->info->aabb;
				const WVector& pos = *(const WVector*)(obj + 0x108);
				// 983: centro do aabb guardado em float antes de somar a pos.
				WVector center((aabb.max.x + aabb.min.x) * 0.5f,
					(aabb.max.y + aabb.min.y) * 0.5f,
					(aabb.max.z + aabb.min.z) * 0.5f);
				m_pSequence[1]->Pos() = WVector(pos.x + center.x,
					pos.y + center.y, pos.z + center.z);
				m_pSequence[1]->SetActive(true, false);
			}
		}
		else
		{
			m_pSequence[1]->Pos() = m_ball.m_pos;
			m_pSequence[1]->SetActive(true, false);
		}

		m_bLoaded = true;
	}
}

CFxBoosterWarp::~CFxBoosterWarp()
{
	for (int i = 0; i < 2; i++)
	{
		if (m_pSequence[i])
		{
			m_fxBox.DeleteSequence(m_pSequence[i], false);
			delete m_pSequence[i];
			m_pSequence[i] = NULL;
		}
	}
}

bool CFxBoosterWarp::Process(float elapsed)
{
	if (m_bHarp == 1)
		m_time += elapsed;

	if (2.0f < m_time)
	{
		m_pSequence[0]->SetActive(false, false);
		m_pSequence[1]->SetActive(false, false);
		return false;
	}

	if (m_bLoaded)
	{
		if (m_ball.m_vel.Magnitude() < 0.5f)
		{
			m_pSequence[0]->SetActive(false, false);
			m_pSequence[1]->SetActive(false, false);
		}
		else
		{
			m_pSequence[0]->Pos() = m_ball.m_pos;
		}
	}

	return true;
}
