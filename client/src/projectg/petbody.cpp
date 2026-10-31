#include "minatl.h"
#define REBANG_PETBODY_CPP
#include "petbody.h"
#include "texturepart.h"
#include "slicestr.h"
#include "quadtree.h"
#include "shadow.h"
#include "fx.h"
#include "wbone.h"
#include "golftask.h"
#include "rtti.h"
#include "vc10_layout.h"

extern WMatrix g_camera;

const char* GetStrConst(const char* str, char* out);
int Ceil(float value, float epsilon);
int WisZero(const float& value, float epsilon);
WMatrix RotMat(const WVector& axis, float angle);
unsigned long ModifyColor(unsigned long color, float intensity);

static WMatrix s_headMat(0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

CPetBody::CPetBody(const char* script, bool bOption)
{
	Init();
	InitScript(script, bOption);
}

void CPetBody::Init()
{
	m_pMotion = NULL;
	m_pPet = NULL;
	m_pLookAt = NULL;
	m_time = 0.0f;
	m_scale = 1.0f;
	m_moveScale = 1.0f;
	m_aniSpeed = 1.0f;
	m_flightState = 2;
	m_bFly = false;
	m_nextMotion[0] = '\0';
	m_customGround = "";
	m_rot.Reset();
	m_pos.Reset();
	m_vel.Reset();
	m_mat.Reset();
	m_leanMat.Reset();
	m_lookAngle.x = m_lookAngle.y = 0.0f;
	m_lookDest.x = m_lookDest.y = 0.0f;
	m_light.type = 2;
	m_bLocalLight = m_bGlobalLight = true;
	m_bTwin983 = false; // 983 0070c7a0: lixo aqui caçava "Bip02 Ho02 Head" em caddie comum (FindBone NULL)
	m_pelvisRot983.Reset();
	C_ASSERT_VC10(offsetof(CPetBody, m_bGlobalLight) == 0x109); // D863: fxspray.cpp grava pelo 983
}

CPetBody::~CPetBody()
{
	for (ScriptHandle* pHandle = m_scriptHandle.Start(); pHandle;
		pHandle = m_scriptHandle.Next())
		delete pHandle;
	m_scriptHandle.Reset();

	std::list<sTexturePart*>::iterator it;
	for (it = m_texPart.begin(); it != m_texPart.end(); it++)
		delete *it;
	m_texPart.clear();

	if (m_pPet)
	{
		g_resrcmng->Release(m_pPet, false);
		m_pPet = NULL;
	}
	m_localLight.clear();
}

void CPetBody::InitScript(const char* script, bool bOption)
{
	WSliceStr slice(script);
	int shadow = 0;
	char name[256];

	if (slice.Get("*pet", "%s", name))
		LoadPuppet(name, false, bOption);
	if (slice.Get("*shadow", "%d", &shadow))
		m_shadowType = shadow;
}

void CPetBody::Reset()
{
	m_flag.Reset();
	m_pLookAt = NULL;
	m_lookDest.x = m_lookDest.y = 0.0f;
	m_lookAngle.x = m_lookAngle.y = 0.0f;
	m_aniSpeed = m_moveScale = 1.0f;
	m_nextMotion[0] = '\0';
	if (m_bTwin983) // 983: 00709ecc
		m_pelvisRot983.x = m_pelvisRot983.y = m_pelvisRot983.z = 1.0f;
}

void CPetBody::SetCustomGround(char* name)
{
	m_customGround = name;
}

// 983 00709890
bool CPetBody::IsBasicMotion() const
{
	return m_pMotion && _stricmp(m_pMotion->name, "\xb1\xe2\xba\xbb") == 0;
}

// 983 0070b1c0: caddie duplo vira as 2 cabecas so na animacao basica, e as pelves quando o alvo passa de 45 graus
void CPetBody::UpdateLookBones()
{
	if (!m_flag.GetFlag(0x34))
		return;
	if (m_pPet->FindBoneByTail("Head"))
	{
		if (!m_bTwin983)
		{
			m_pPet->FindBoneByTail("Head")->SetMatrix(
				~s_headMat * GetMatrix(BODY_HEAD), NULL);
		}
		else
		{
			if (!IsBasicMotion())
				return;
			m_pPet->FindBone("Bip01 Head")->SetMatrix(
				~s_headMat * GetMatrix(BODY_HEAD1), NULL);
			m_pPet->FindBone("Bip02 Ho02 Head")->SetMatrix(
				~s_headMat * GetMatrix(BODY_HEAD2), NULL);
		}
	}
	if (m_pLookAt && m_bTwin983 && IsBasicMotion())
	{
		WVector fwd(m_mat.za.x * -1.0f, m_mat.za.y * -1.0f, m_mat.za.z * -1.0f);
		WVector to(m_pLookAt->x - m_mat.pivot.x, m_pLookAt->y - m_mat.pivot.y,
			m_pLookAt->z - m_mat.pivot.z);
		m_lookDest.y = CalcDeltaAngle(to, fwd);
		if (m_bLookTurn && Abs(m_lookDest.y) > 0.785398185f)
		{
			m_pPet->FindBone("Bip01 Pelvis")->SetMatrix(
				~s_headMat * GetMatrix(BODY_PELVIS1), NULL);
			m_pPet->FindBone("Bip02 Ho02 Pelvis")->SetMatrix(
				~s_headMat * GetMatrix(BODY_PELVIS2), NULL);
			m_pPet->FindBone("Bip01 Head")->SetMatrix(
				~s_headMat * GetMatrix(BODY_HEAD1), NULL);
			m_pPet->FindBone("Bip02 Ho02 Head")->SetMatrix(
				~s_headMat * GetMatrix(BODY_HEAD2), NULL);
		}
	}
}

void CPetBody::Display()
{
	if (ShouldDisplay())
	{
		if (m_flag.GetFlag(0x14))
			Update();

		WPuppet* pPet = m_pPet;
		if (g_view->InFrustum(pPet->GetBoundSphere()))
		{
			UpdateLookBones();

			if (m_flag.GetFlag(0x14))
			{
				if (m_bGlobalLight)
					m_pPet->UpdateLightSource(&m_light, false, NULL);
				if (m_bLocalLight)
				{
					std::vector<sLightSet>::iterator it;
					for (it = m_localLight.begin(); it != m_localLight.end();
						it++)
					{
						it->light.nearOne = RotVec(it->pos, m_leanMat);
						m_pPet->UpdateLightSource(&it->light, true, NULL);
					}
				}
			}

			m_pPet->Render(g_view, true, m_scale,
				g_view->InFrustumSafe(pPet->GetBoundSphere()), false, false);
		}
		m_flag.Disable(0x30);
	}
}

void CPetBody::DisplayShadow(char* recvName)
{
	if (ShouldDisplay())
	{
		if (m_flag.GetFlag(0x14))
			Update();

		UpdateLookBones();

		WSingleton<CShadowManager>::Instance()->AddCastPet_New(recvName, false,
			m_pPet);
	}
}

void CPetBody::SetLight()
{
	m_pPet->UpdateBound(false, false);
	m_bLocalLight = true;
	if (IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		WVector pos(m_pos.x, m_pPet->GetBoundBox().max.y, m_pos.z);
		m_light.diffuse =
			ModifyColor(g_lightset.diffuse, GetPVS().GetShadowIntensity(pos));
		m_light.ambient = g_lightset.ambient;
		m_light.nearOne = g_lightset.nearOne;
	}
	else
	{
		m_light.diffuse = 0xffffff;
		m_light.ambient = 0xffffff;
		m_light.type = 2;
		m_light.nearOne = WVector(0.0f, -1.0f, 1.0f);
		m_light.nearOne.Normalize();
	}
}

void CPetBody::Process(float dt)
{
	if (m_flag.GetFlag(4))
	{
		Animation(dt * m_aniSpeed);
		if (m_flag.GetFlag(2))
		{
			if (m_flag.GetFlag(1))
			{
				m_flag.Disable(1);
				m_vel = m_moveScale * RotVec(m_delta, m_mat);
				m_pos += m_vel;
			}
			FlightProcess(dt);
			m_mat.pivot = m_pos;
			m_leanMat.pivot = m_pos;
		}
	}
	if (m_pPet->FindBoneByTail("Head"))
		LookUpProcess(dt);
}

void CPetBody::Animation(float dt)
{
	while (m_pMotion && m_pMotion->s != m_pMotion->e)
	{
		float step = Move(dt);
		if (!(step > 0.0f))
			return;
		ScriptProcess(m_pMotion, m_time, m_time + step);
		m_time = step + m_time;
		if (!(m_time >= m_pMotion->e))
			return;
		float rest = dt - step;
		if (m_nextMotion[0])
		{
			SetMotion(m_nextMotion, m_bNextMotionMove, rest, false);
			m_nextMotion[0] = '\0';
		}
		else
			SetMotion(m_pMotion->nextmotion, m_flag.GetFlag(2), rest, true);
		dt = rest;
	}
	m_flag.Disable(6);
}

float CPetBody::Move(float dt)
{
	float time = dt + m_time;
	dt = Min(m_pMotion->end, time);

	if (m_flag.GetFlag(2))
	{
		const w_motion_data* pMotion = m_pMotion;
		if (!m_flag.GetFlag(1))
		{
			m_delta = m_pPet->GetDeltaVec(pMotion->name, m_time - pMotion->s,
				dt - pMotion->s);
			m_flag.Enable(1);
			return dt - m_time;
		}
		m_delta += m_pPet->GetDeltaVec(pMotion->name, m_time - pMotion->s,
			dt - pMotion->s);
	}
	return dt - m_time;
}

void CPetBody::SetPos(const WVector& pos)
{
	m_pos = pos;
	m_mat.pivot = m_pos;
	m_leanMat.pivot = m_pos;
	m_flag.Enable(0x10);
	m_flag.Disable(2);
}

void CPetBody::SetRotate(const WVector& rot)
{
	if (0)
		((CPetBody*)0)->CPetBody::SetLight();
	m_rot = rot;
	m_mat = RotMat(m_rot.x, 0) * RotMat(m_rot.y, 1) * RotMat(m_rot.z, 2);
	m_mat.pivot = m_pos;
	RestoreLeanMat();
	m_flag.Enable(0x10);
}

void CPetBody::SetRotate_Lean(const WVector& rot)
{
	if (0)
		((CPetBody*)0)->CPetBody::SetLight();
	m_rot = rot;
	m_leanMat = RotMat(m_rot.x, 0) * RotMat(m_rot.y, 1) * RotMat(m_rot.z, 2);
	m_leanMat.pivot = m_pos;
	m_flag.Enable(0x10);
}

void CPetBody::SetRotate(float angle)
{
	if (0)
		((CPetBody*)0)->CPetBody::SetLight();
	WVector pivot = m_mat.pivot;
	m_rot = WVector(0.0f, angle, 0.0f);
	m_mat = RotMat(angle, 1);
	m_mat.pivot = pivot;
	RestoreLeanMat();
	m_flag.Enable(0x10);
}

void CPetBody::SetRotateSlope(float angle, WVector normal)
{
	if (0)
		((CPetBody*)0)->CPetBody::SetLight();
	WVector pivot = m_mat.pivot;
	WVector n = normal;
	n.y = 0.0f;
	normal = WCrossProduct(normal, n);
	normal.y = normal.y * -1.0f;
	WMatrix saved;
	saved = m_mat;
	m_mat = RotMat(normal, angle * 0.00343774678f);
	m_mat = saved * m_mat;
	m_mat.pivot = pivot;
	RestoreLeanMat();
	m_flag.Enable(0x10);
}

void CPetBody::MultRotate(WMatrix* mat)
{
	if (mat)
	{
		WVector pivot = m_leanMat.pivot;
		WMatrix rot;
		m_leanMat.GetRotMatrix(&rot);
		m_leanMat = rot * *mat;
		m_leanMat.pivot = pivot;
		m_flag.Enable(0x10);
	}
}

void CPetBody::Rotate(float angle)
{
	m_rot.y = angle + m_rot.y;
	m_mat = m_mat * RotMat(angle, 1);
	m_mat.pivot = m_pos;
	RestoreLeanMat();
	m_flag.Enable(0x10);
}

void CPetBody::AddScriptHandle(const char* name, IActor* actor, int param)
{
	ScriptHandle* pHandle = new ScriptHandle;
	pHandle->name = name;
	pHandle->actor = actor;
	pHandle->param = param;
	m_scriptHandle += pHandle;
}

void CPetBody::AddLocalLight(const LightSet& light, WVector pos)
{
	sLightSet set;
	set.light = light;
	set.pos = pos;
	m_localLight.push_back(set);
}

void CPetBody::TakeOff(WVector target)
{
	m_flightState = 0;
	m_flightStart = m_pos;
	m_flightTarget = target;
	m_leanAngle = -0.785398185f;
	m_flightHeight = 0.0f;
	m_flightSpeed = 50.0f;
}

void CPetBody::LoadPuppet(const char* name, bool bOptimizeBone, bool bOption)
{
	m_pPet = g_resrcmng->GetPuppet(name, false, true, bOption);
	if (bOptimizeBone && m_pPet)
		m_pPet->OptimizeBone();
	SetScale(1.0f);
	SetPivot(NULL, WVector(0.0f, 0.0f, 0.0f));
}

void CPetBody::ScriptProcess(const w_motion_data* motion, float from, float to)
{
	if (from <= motion->e && from != to)
	{
		from *= 30.0f;
		to = Min(motion->e, to) * 30.0f;
		int first = Ceil(from, 0.03f);
		int last;
		if (to == floor(to))
			last = (int)to - 1;
		else
			last = (int)floor(to);
		if (first <= last &&
			m_pPet->CheckFrameData(first, last, (int&)from, (int&)to) &&
			(int&)from <= (int&)to)
		{
			for (int i = (int&)from; i <= (int&)to; ++i)
				ParsePuppetScript(m_pPet->m_frame[i].data);
		}
	}
}

void CPetBody::ParsePuppetScript(const char* script)
{
	static char* cmdList[9] = { "*ptex(", "*snd(", "*fly", "*hide", "*show",
		"*fx(", "*lookup", "*dontlookup", "*petbillboard" };
	static char texName[64];
	static char cmd[1024];

	char name[64];
	char fxCmd[64];
	char bone[64];
	char fx[64];
	char tex[64];
	const char* all = script;

	if (script)
	{
		const char* p;
		do
		{
			p = strchr(script, '*');
			if (!p)
				break;
			int count = 0;
			if (!*p)
				break;
			char c;
			do
			{
				c = p[++count];
				if (c == ' ')
					break;
				if (c == '(')
				{
					count++;
					break;
				}
			} while (c != '\0');
			if (count <= 1)
				break;
			strncpy(cmd, p, count);
			cmd[count] = 0;
			script = p + count;
			for (unsigned int n = 0; n < 9; ++n)
			{
				if (strcmpi(cmd, cmdList[n]) != 0)
					continue;
				switch (n)
				{
				case 0:
					name[0] = '\0';
					script = GetStrConst(script, name);
					if (strlen(name) != 0 && strlen(name) < 0x40)
					{
						if (name[0] == '@')
						{
							if (m_pMotion &&
								strcmpi(name + 1, m_pMotion->name) == 0)
							{
								script = GetStrConst(script, texName);
								if (strlen(texName) != 0 &&
									strlen(texName) < 0x40)
									goto texPart;
							}
						}
						else
						{
							strcpy(texName, name);
texPart:
							script = GetStrConst(script, tex);
							if (strlen(tex) != 0 && strlen(tex) < 0x40)
								SetPendingTexPart(texName, tex);
						}
					}
					break;
				case 1:
					script = GetStrConst(script, texName);
					if (strlen(texName) != 0 && strlen(texName) < 0x40)
						PlaySfx(texName);
					break;
				case 2:
					m_bFly = true;
					break;
				case 3:
					m_flag.Enable(0x2000);
					break;
				case 4:
					m_flag.Disable(0x2000);
					break;
				case 5:
					fxCmd[0] = '\0';
					script = GetStrConst(script, fxCmd);
					if (strlen(fxCmd) != 0 && strlen(fxCmd) < 0x40)
					{
						if (fxCmd[0] == '@')
						{
							if (m_pMotion &&
								strcmpi(fxCmd + 1, m_pMotion->name) == 0)
							{
								script = GetStrConst(script, fx);
								if (strlen(fx) != 0 && strlen(fx) < 0x40)
									goto attach;
							}
						}
						else
						{
							strcpy(fx, fxCmd);
attach:
							Attacher attacher;
							unsigned int j = 0;
							do
							{
								script = GetStrConst(script, bone);
								if (strlen(bone) == 0)
									break;
								if (strlen(bone) >= 0x40)
									break;
								WBone* pBone = m_pPet->FindBone(bone);
								if (pBone)
									attacher.mat[j] = &pBone->m_matrix;
								else
								{
									w_bound_box* pBox =
										m_pPet->m_bbList.Find(bone);
									if (pBox)
									{
										if (pBox->bone)
											attacher.mat[j] =
												&pBox->bone->m_matrix;
										attacher.pos[j] =
											&pBox->info->spherePivot;
									}
								}
								++j;
							} while (j < 4);
							if (0 < (int)j)
								WSingleton<CFx>::Instance()->Attach(attacher,
									fx, true, NULL);
						}
					}
					break;
				default:
					break;
				}
				break;
			}
		} while (script);

		ScriptHandle* pHandle = m_scriptHandle.Start();
		while (pHandle)
		{
			const char* pFound = strstr(all, pHandle->name);
			if (pFound)
			{
				MsgObject msg;
				msg.message = pHandle->param;
				msg.sender = NULL;
				msg.time = 0;
				msg.param1 = (int)pFound;
				pHandle->actor << msg;
			}
			pHandle = m_scriptHandle.Next();
		}
	}
}

sTexturePart* CPetBody::FindTexturePart(char* primary, char* secondary)
{
	std::list<sTexturePart*>::iterator it;
	it = std::find_if(m_texPart.begin(), m_texPart.end(),
		sFindTexPart(secondary));
	if (it != m_texPart.end())
		return *it;

	sTexturePart* pPart = new sTexturePart;
	strcpy(pPart->name, primary);
	strcpy(pPart->texture, secondary);
	m_texPart.push_back(pPart);
	return pPart;
}

bool CPetBody::ShouldDisplay()
{
	bool bShow = !m_flag.GetFlag(0x2000);
	return bShow;
}

void CPetBody::SetPendingTexPart(char* primary, char* secondary)
{
	sTexturePart* pPart = FindTexturePart(primary, secondary);
	if (!pPart->pBitmap)
		m_pPet->ChangeTexturePart(pPart->name, pPart->texture, &pPart->pBitmap,
			&pPart->rect);
	else
		m_pPet->ChangeTexturePart(pPart->name, pPart->pBitmap, pPart->rect);
}

void CPetBody::PlaySfx(char* name)
{
	if (0)
		((CPetBody*)0)
			->CPetBody::AddScriptHandle(*(const char**)0, *(IActor**)0,
				*(int*)0);
	if (name)
	{
		WVector pos;
		pos = m_pos - g_camera.pivot;
		char* pDash = strchr(name, '-');
		if (pDash)
		{
			char* pStr = pDash + 1;
			int count = pStr - name;
			if (strncmp(pStr, "\xc2\xf9\xbd\xba", 4) == 0)
			{
				int diff;
				if (Doc()->m_golfGame.gameType == 1)
				{
					sTeamData* pTeam =
						TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team);
					diff = (unsigned char)
							   pTeam->stroke[GOLFDOC()->m_currentHole - 1] -
						GOLFDOC()->GetHoleData().par;
				}
				else
				{
					sPlayerData* pPlayer = PLAYER(GOLFDOC()->m_currentPlayer);
					diff = (unsigned char)
							   pPlayer->stroke[GOLFDOC()->m_currentHole - 1] -
						GOLFDOC()->GetHoleData().par;
				}
				if (GolfBall().m_groundType == 2 && (diff == -2 || diff == -1))
				{
					char buf[64];
					strncpy(buf, name, count);
					switch (diff)
					{
					case -1:
						strcpy(buf + count, "\xb9\xf6\xb5\xf0");
						break;
					case -2:
						strcpy(buf + count, "\xc0\xcc\xb1\xdb");
						break;
					}
					strcat(buf, name + count);
					strcpy(name, buf);
				}
			}
		}

		if (IS_KINDOF(CGolfTask, AfxGetTask()))
			g_audio->PlaySfx(name, &pos, 5);
		else
			g_audio->PlaySfx(name, NULL, 0);
	}
}

void CPetBody::SetPivot(const char* name, WVector offset)
{
	if (m_pPet)
	{
		WVector pivot = m_pPet->GetPivot(name);
		m_pos -= (pivot + offset) * m_scale;
	}
}

void CPetBody::SetShadowType(int type)
{
	m_shadowType = type;
}

void CPetBody::SetMotion(const char* name, bool bMove, float time,
	bool bApplyDelta)
{
	const w_motion_data* pPrev = m_pMotion;
	m_pMotion = m_pPet->FindMotionData(name);
	if (m_pMotion)
	{
		m_bFly = false;
		m_time = Min(time + m_pMotion->s, m_pMotion->e);
		if (!(0.0f < m_time))
			m_time = 0.0f;
		if (m_pMotion->s < m_time)
			ScriptProcess(m_pMotion, m_pMotion->s, m_time);
		m_flag.Enable(4);
		if (bMove)
		{
			m_flag.Enable(2);
			return;
		}
		if (pPrev && bApplyDelta)
		{
			float len = m_pMotion->e - m_pMotion->s;
			if (!WisZero(len, g_EPSILON))
			{
				WVector delta;
				delta = m_pPet->GetDeltaVec(pPrev->name, pPrev->e - pPrev->s);
				delta = m_moveScale * RotVec(delta, m_mat);
				SetPos(delta + m_pos);
			}
		}
		m_flag.Disable(2);
	}
}

void CPetBody::SetScale(float scale)
{
	m_scale = scale;
	{
		WVector v(scale, scale, scale);
		m_leanMat.AxisScale(v);
	}
	if (m_pPet)
	{
		WVector v(scale, scale, scale);
		m_pPet->m_mat.AxisScale(v);
	}
}

void CPetBody::ChangeTexture(char* name, int index)
{
	m_pPet->ChangeTexture(name, index);
}

WMatrix CPetBody::GetMatrix(eBody body)
{
	if (0)
		((CPetBody*)0)
			->CPetBody::LoadPuppet(*(const char**)0, *(bool*)0, *(bool*)0);
	WMatrix mat;
	switch (body)
	{
	case BODY_HEAD:
	{
		WBone* pHead = m_pPet->FindBoneByTail("Head");
		mat = RotMat(m_lookAngle.x, 0) * RotMat(m_lookAngle.y, 1) * s_headMat *
			pHead->m_matrix;
		break;
	}
	case BODY_ROOT:
		mat = m_pPet->m_mat;
		break;
	case BODY_ROOT_POS:
		mat = m_pPet->m_mat;
		mat.pivot = m_pos;
		break;
	case BODY_HEAD1:
	case BODY_HEAD2:
	{
		WBone* pHead = m_pPet->FindBone(body == BODY_HEAD1 ? "Bip01 Head" : "Bip02 Ho02 Head");
		mat = RotMat(m_lookAngle.x, 0) * RotMat(m_lookAngle.y, 1) * s_headMat *
			pHead->m_matrix;
		break;
	}
	case BODY_PELVIS1:
	case BODY_PELVIS2:
	{
		WBone* pPelvis = m_pPet->FindBone(body == BODY_PELVIS1 ? "Bip01 Pelvis" : "Bip02 Ho02 Pelvis");
		mat = RotMat(m_pelvisRot983.y, 1) * s_headMat * pPelvis->m_matrix;
		break;
	}
	}
	return mat;
}

void CPetBody::LookUpProcess(float dt)
{
	if (m_pLookAt)
	{
		WBone* pHead = m_pPet->FindBoneByTail("Head");
		const WVector& head = pHead->m_matrix.pivot;
		WVector d(m_pLookAt->x - head.x, m_pLookAt->y - head.y,
			m_pLookAt->z - head.z);
		float pitch = (float)atan((double)(d.y / sqrt(d.x * d.x + d.z * d.z)));
		m_lookDest.x = pitch < -0.34906584f
			? -0.34906584f
			: (pitch > 0.785398185f ? 0.785398185f : pitch);

		WVector fwd(m_mat.za.x * -1.0f, m_mat.za.y * -1.0f, m_mat.za.z * -1.0f);
		WVector to(m_pLookAt->x - m_mat.pivot.x, m_pLookAt->y - m_mat.pivot.y,
			m_pLookAt->z - m_mat.pivot.z);
		float yaw = CalcDeltaAngle(to, fwd);
		m_lookDest.y = yaw;
		if (m_bLookTurn)
		{
			if (Abs(yaw) > 0.785398185f)
			{
				float over = Abs(yaw) - 0.785398185f;
				if (yaw <= 0.0f)
					over = -over;
				if (m_bTwin983) // 983 0070b480: o duplo gira a pelve, nao o corpo
				{
					m_pelvisRot983.x = 0.0f;
					m_pelvisRot983.y = over;
					m_pelvisRot983.z = 0.0f;
				}
				else
					SetRotate(WVector(m_rot.x, over + m_rot.y, m_rot.z));
			}
		}
		m_lookDest.y = m_lookDest.y < -0.785398185f
			? -0.785398185f
			: (m_lookDest.y > 0.785398185f ? 0.785398185f : m_lookDest.y);
	}
	else
	{
		m_lookDest.y = 0.0f;
		m_lookDest.x = 0.0f;
	}

	WVector2D diff = m_lookDest - m_lookAngle;
	if (Abs(diff.x) > g_DEGTORAD)
	{
		m_flag.Enable(0x20);
		float t = dt / diff.Magnitude() * 1.57079637f;
		if (t > 1.0f)
			t = 1.0f;
		m_lookAngle.x = t * diff.x + m_lookAngle.x;
	}
	if (Abs(diff.y) > g_DEGTORAD)
	{
		m_flag.Enable(0x20);
		float t = dt / sqrt(diff.SquareMagnitude()) * 1.57079637f;
		if (t > 1.0f)
			t = 1.0f;
		m_lookAngle.y = t * diff.y + m_lookAngle.y;
	}
}

void CPetBody::FlightProcess(float dt)
{
	CGolfBall& ball = GolfBall();
	WVector v = m_pos;
	int state = m_flightState;
	float flightLen = (m_flightTarget - m_flightStart).Magnitude();
	float dist = WVector(m_flightTarget.x - v.x, m_flightTarget.y,
		m_flightTarget.z - v.z)
					 .Magnitude();
	float groundDiff;

	switch (state)
	{
	case 0:
		if (m_bFly)
		{
			if (!(Wabs(m_leanAngle) < g_EPSILON))
			{
				float lean = dt * 0.400000006f + m_leanAngle;
				if (lean < 0.0f)
					m_leanAngle = lean;
				else
					m_leanAngle = 0.0f;
				v.y = CalcDeltaAngle(GetDirection(), WVector::UNIT_NEG_Z);
				v.x = m_leanAngle;
				v.z = 0.0f;
				SetRotate_Lean(v);
			}
			if (ball.m_pos.y > m_flightHeight)
				m_flightHeight = ball.m_pos.y;
			if (m_pos.y < m_flightHeight)
			{
				m_pos.y = dt * m_flightSpeed + m_pos.y;
				if (m_flightSpeed > 0.0f)
					m_flightSpeed = m_flightSpeed - dt * 25.0f;
			}
			if (m_flightHeight + 10.0f < m_pos.y)
				m_pos.y = m_pos.y - dt * 10.0f;
			if (dist * 1.5f <= flightLen)
			{
				m_leanAngle = 0.0f;
				m_flightState = 1;
				RestoreLeanMat();
			}
		}
		else
		{
			m_leanAngle = 0.0f;
			m_flightState = 1;
			RestoreLeanMat();
		}
		break;

	case 1:
		if (!m_bFly || dist * 3.0f <= flightLen)
			m_flightState = 2;
		break;

	case 2:
		groundDiff = 0.0f;
		if (!m_customGround.empty())
		{
			w_bound_box* pBox = m_pPet->m_bbList.Find(m_customGround.c_str());
			if (pBox)
			{
				w_bound_box_info* pInfo = pBox->info;
				WVector mn = pInfo->aabb.min;
				v = pInfo->aabb.max;
				mn = mn * pBox->bone->m_matrix;
				v = v * pBox->bone->m_matrix;
				mn.x = (mn.x + v.x) * 0.5f;
				mn.z = (mn.z + v.z) * 0.5f;
				groundDiff =
					mn.y - GetPVS().GetGroundPoint(mn, true, NULL, true).y;
			}
		}
		if (m_bFly)
		{
			if (!(Wabs(m_leanAngle - -0.392699093f) < g_EPSILON))
			{
				float lean = m_leanAngle - dt * 0.300000012f;
				if (lean > -0.392699093f)
					m_leanAngle = lean;
				else
					m_leanAngle = -0.392699093f;
				v.y = CalcDeltaAngle(GetDirection(), WVector::UNIT_NEG_Z);
				v.x = m_leanAngle;
				v.z = 0.0f;
				SetRotate_Lean(v);
			}
		}
		else
		{
			m_leanAngle = 0.0f;
			RestoreLeanMat();
		}
		{
			float step = dt * 36.0f;
			if (step <= groundDiff)
				groundDiff = step;
			m_pos.y = m_pos.y - groundDiff;
		}
		break;
	}

	if (m_bFly)
	{
		if (!(Wabs(ball.m_initCurve) < g_EPSILON))
		{
			v.x = WVector::UNIT_POS_Y.y * m_vel.z -
				WVector::UNIT_POS_Y.z * m_vel.y;
			v.y = WVector::UNIT_POS_Y.z * m_vel.x -
				WVector::UNIT_POS_Y.x * m_vel.z;
			v.z = WVector::UNIT_POS_Y.x * m_vel.y -
				WVector::UNIT_POS_Y.y * m_vel.x;
			v.Normalize();
			float d = (ball.m_pos - m_pos) * v;
			m_pos += v * d * 1.20000005f * dt;
		}
	}
}

void CPetBody::LookUp(WVector* target, bool bTurn)
{
	if (m_pPet->FindBoneByTail("Head"))
	{
		m_pLookAt = target;
		m_bLookTurn = bTurn;
	}
}

float CPetBody::GetAnimationLength(const char* name)
{
	w_motion_data* pMotion = m_pPet->FindMotionData(name);
	return pMotion->e - pMotion->s;
}

void CPetBody::SetNextMotion(const char* name, bool bMove)
{
	m_bNextMotionMove = bMove;
	strcpy(m_nextMotion, name);
}

__forceinline void CPetBody::Update()
{
	// HACK: the reference keeps the (otherwise dead) stack store of bMove
	bool bMove = m_flag.GetFlag(2);
	const bool& rMove = bMove;
	m_pPet->SetMatrix(m_leanMat);
	m_pPet->Animate983(NULL, NULL, m_time, bMove != 0, NULL, NULL);
	m_pPet->Transform(NULL, 1.0f, 0);
	m_pPet->UpdateBSphere();
}

WPuppet* CPetBody::GetPet()
{
	// HACK: only references g_PI and g_HUGE so that the const float block
	// (PI, DEGTORAD, HUGE, EPSILON) is emitted in full like the original
	if (0)
		m_scale = g_PI + g_HUGE;
	return m_pPet;
}
