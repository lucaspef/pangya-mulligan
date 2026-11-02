#include "minatl.h"
#include "vc10_layout.h"
#include <math.h>
#include <algorithm>
#include "realmyroom_avatar.h"
#include "realmyroommain.h"
#include "balloontext.h"
#include "bitmap.h"
#include "texturepart.h"
#include "partsinfo.h"
#include "networksystem.h"
#include "netresourcemanager.h"
#include "clientsetting.h"
#include "golfdoc.h"
#include "projectg.h"
#include "woverlay.h"
#include "fremoticon.h"
#include "packet.h"
#include "shareddoc.h"
#include "quadtree.h"
#include "commonutil.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "wpuppet.h"
#include "wbone.h"
#include "wview.h"
#include "../shared/localize.h"
#include "mathconsts.h"
#include "jp/createcharacteroption.h"

// HACK: the linker keeps the copy of "G_Bird" of golfplayer.obj (earlier in link order), which is a local static of a
// naked function there, so this unit must not emit its own copy: name that one.
#define RB_STR_G_BIRD "G_Bird"


#define BONE_HEADNUB "Bip01 HeadNub"
#define BONE_PELVIS "Bip01 Pelvis"
#define OVERLAY_NEAR_Z 0.044f

WMatrix RotMat(const WVector& axis, float angle);
const char* __fastcall GetStrConst(const char* src, char* dest);
const char* GetCharsTillSpace(const char* str, char* out);

extern Fresh* g_pFresh;
extern WMatrix g_camera;

// 983: as expressoes do rosto ficam no CreateCharacterOption (00e73e84, 15 personagens; getter 004ced70, chamado em
// 007a63bd/007a796e/007a85d0); a tabela estatica do 645 tinha so 10 -> Spika/NR/HR/CC liam alem do fim (D769)

unsigned long ModifyColor(unsigned long color, float factor);

// Leading fields of CFxSpray / CFxSequence used when attaching an effect.
struct sFxSprayAttach
{
	void* m_vtbl;
	bool m_bSequence;
	const Attacher* m_pAttacher;
	Attacher m_attacher;
};

struct sFxSequenceAttach
{
	void* m_vtbl;
	bool m_bSequence;
	WVector m_wind; // +8 (CFxSequence::m_wind da visao completa)
	const Attacher* m_pAttacher;
	Attacher m_attacher;
};

CRealMyRoom_Avatar::CRealMyRoom_Avatar(const sSlotInfo& slotInfo,
	const sCharacterInfo& charInfo, CRealMyRoomMain* pMain)
	: m_boneSet(true), m_prevBoneSet(true)
{
	// A421: a visao parcial do realmyroommainui.cpp segue estes deslocamentos (realmyroom_avatar.h)
	C_ASSERT(offsetof(CRealMyRoom_Avatar, m_charInfo) == RMRAVATAR_OFS_CHARINFO);
	C_ASSERT(offsetof(CRealMyRoom_Avatar, m_pPetFrame) == RMRAVATAR_OFS_PETFRAME);
	C_ASSERT_VC10(offsetof(CRealMyRoom_Avatar, m_bVisible) == RMRAVATAR_OFS_HIDE);
	C_ASSERT(sizeof(CRealMyRoom_AvatarBurden) == RMRAVATAR_BURDEN_SIZE);
	C_ASSERT_VC10(offsetof(CRealMyRoom_AvatarManager, m_pMyAvatar) == 0xa4);
	strcpy(m_idName, slotInfo.sNick);
	strcpy(m_nickName, slotInfo.sNick);
	m_bMove = false;
	m_followNickName[0] = '\0';

	if (IsLocalContent((localContentType_t)0x10))
	{
		if (slotInfo.nChannelingFlag == 1)
		{
			strcpy(m_channelingName, slotInfo.sDisplayID);
			m_bChanneling = true;
		}
		else
		{
			memset(m_channelingName, 0, sizeof(m_channelingName));
			m_bChanneling = false;
		}
	}

	m_guid = slotInfo.dwGuid;
	m_bAdministrator = (slotInfo.dwIdentity & 4) ? true : false;
	m_gender = slotInfo.gender;
	m_bManner = slotInfo.manner;
	m_bAngelicWings = slotInfo.angelicWings;
	m_tidChar = slotInfo.tidChar;
	m_prevState = m_state = slotInfo.state;
	m_action = slotInfo.action;
	m_pMotion = NULL;
	m_pPrevMotion = NULL;
	m_motionTime = 0.0f;
	m_blendDuration = 0.0f;
	m_lifeTime983 = 0.0f;
	m_facialTime = 0.0f;
	m_up = WVector::UNIT_POS_Y;
	m_bTargetPositionChanged = false;
	m_lastMotionSendTime = 0;
	m_startPosition = WVector::UNIT_POS_Y;
	m_pMain = pMain;
	m_bPendingTargetAngle = false;
	m_startPosition = m_pMain->m_v2c8;
	m_bCharInfoChanged = false;
	m_pPetFrame = NULL;
	m_charInfo = charInfo;
	m_headIcon = -1;
	m_pEmblem = (Bitmap*)NetResourceManager::Instance()->GetEmblemByName(
		slotInfo.szEmblemName);
	m_pBalloon = NULL;
	m_random[0] = Random(0.8f, 1.0f);
	m_random[1] = Random(0.8f, 1.0f);
	m_random[2] = Random(0.8f, 1.0f);
	m_balloonTime = 1.0f;
	m_moveDelta.x = m_moveDelta.y = m_moveDelta.z = 0.0f;
	SetAngle(slotInfo.location[2]);
	SetState(m_state);
	SetAction(m_action);
	SetTradeTitle(slotInfo.strTradeTitle);

	m_light.type = 2;
	m_light.ambient = ModifyColor(g_lightset.ambient, 1.0f);
	m_light.nearOne = g_lightset.nearOne;
	m_fxBox.Init(g_resrcmng);
	m_fxBox.SetCamera(&g_camera);
	m_fxBox.m_frameTime = 100.0f / 3.0f;
}

CRealMyRoom_Avatar::~CRealMyRoom_Avatar()
{
	std::list<sTexturePart*>::iterator it;
	for (it = m_texParts.begin(); it != m_texParts.end(); ++it)
		delete *it;
	m_texParts.clear();

	m_fxBox.ClearAll(false);
}

void CRealMyRoom_Avatar::SetHeadIcon(int headIcon)
{
	m_headIcon = headIcon;
}

bool CRealMyRoom_Avatar::IsTargetPositionChanged(bool bChanged)
{
	bool bPrev = m_bTargetPositionChanged;
	m_bTargetPositionChanged = bChanged;
	return bPrev;
}

void CRealMyRoom_Avatar::SetHide(bool bHide)
{
	m_bVisible = bHide;

	if (m_pMain->m_pFxSeq)
		CFx::Instance()->SetActive(bHide);
}

bool CRealMyRoom_Avatar::IsClickable(float x, float y) const
{
	if (!g_pFresh->show)
		return false;

	if (m_pPetFrame == NULL || m_pPetFrame->GetCurAniPet() == NULL)
		return false;

	if (m_time < 0.1f)
		return false;

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (m_state & 0x80)
	{
		WVector head;
		head = m_pPetFrame->GetBonePet()
				   ->FindBone(BONE_HEADNUB)
				   ->GetMatrix()
				   .pivot;

		WVector screen = g_view->Projection(head);

		if (!(screen.z <= 0.0f) && !(1.0f <= screen.z))
		{
			int screenX = (int)floorf(screen.x);
			int screenY = (int)floorf(screen.y) - 48;

			if (m_state & 0x80)
			{
				if (CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar != NULL)
				{
					if ((CRealMyRoom_AvatarManager::Instance()
								->m_pMyAvatar->m_position -
							m_position)
							.Magnitude() > 19.2f)
						return false;
				}

				int width = (int)pGDI->GetTextExtendOld983(m_tradeTitle) + 10;
				if (width < 100)
					width = 100;

				int left = screenX - width / 2;
				int top = screenY - 8;
				if (left < x && x < left + width && top < y && y < top + 29)
					return true;
			}
		}
	}

	return false;
}

CRealMyRoom_Avatar* CRealMyRoom_Avatar::GetFollowingAvatar() const
{
	if (m_followNickName[0] == '\0')
		return NULL;

	return CRealMyRoom_AvatarManager::Instance()->FindAvatarByNickName(
		m_followNickName);
}

bool CRealMyRoom_Avatar::GetBoneScreenCoord(const char* boneName, int* pX,
	int* pY) const
{
	if (m_pPetFrame == NULL || m_pPetFrame->GetCurAniPet() == NULL)
		return false;

	WBone* pBone = m_pPetFrame->GetBonePet()->FindBone(boneName);
	if (pBone == NULL)
		return false;

	WVector screen = g_view->Projection(pBone->GetMatrix().pivot);
	*pX = (int)floor(screen.x);
	*pY = (int)floor(screen.y);

	return true;
}

void CRealMyRoom_Avatar::ChangeCharacterInfo()
{
	m_fxBox.ClearAll(false);

	ParseBoundingBoxScript();
}

void CRealMyRoom_Avatar::SetCharactorInfo(const sCharacterInfo& charInfo)
{
	Stop();

	m_pPetFrame = NULL;
	m_charInfo = charInfo;
	m_tidChar = charInfo.tid;

	m_fxBox.ClearAll(false);
}

void CRealMyRoom_Avatar::SetPosition(const WVector& position)
{
	m_position = position;
	SetTargetPosition(position);
}

void CRealMyRoom_Avatar::SetStartPosition(const WVector& position)
{
	m_startPosition = position;
}

void CRealMyRoom_Avatar::SetTargetPosition(const WVector& position)
{
	m_targetPosition = position;
	m_bTargetPositionChanged = true;
}

void CRealMyRoom_Avatar::SetAngle(float angle)
{
	angle = RadianNormalize2PI(angle);
	m_angle = angle;
	m_targetAngle = angle;
}

void CRealMyRoom_Avatar::SetTargetAngle(float angle)
{
	m_targetAngle = RadianNormalize2PI(angle);
}

void CRealMyRoom_Avatar::SetPendingTargetAngle(float angle)
{
	m_pendingTargetAngle = RadianNormalize2PI(angle);
	m_bPendingTargetAngle = true;
}

void CRealMyRoom_Avatar::SetState(unsigned long state)
{
	if (m_state == state)
		return;

	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME) &&
		this == CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar)
	{
		WSendPacket packet((enumClientPacket)0x63);
		packet.Encode1(8);
		packet.Encode4(state);
		packet.Send(TO_GAME);
	}

	m_state = state;
	if (state & 0x7e)
		Stop();
}

void CRealMyRoom_Avatar::SetAction(int action)
{
	if (m_action == action)
		return;

	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME) &&
		this == CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar)
	{
		WSendPacket packet((enumClientPacket)0x63);
		packet.Encode1(5);
		packet.Encode4(action);
		packet.Send(TO_GAME);
	}

	m_action = action;
}

void CRealMyRoom_Avatar::SendMove(const WVector& delta, bool bForce)
{
	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME) &&
		this == CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar)
	{
		m_moveDelta += delta;

		if (m_moveDelta.Magnitude() >= 16.5f || bForce)
			m_moveDelta.Reset();
	}
}

void CRealMyRoom_Avatar::Stop()
{
	if (this == CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar)
	{
		if (!WisEqual(m_targetPosition, m_position, 0.00001f))
		{
			SendMove(WVector(0.0f, 0.0f, 0.0f), true);

			m_targetPosition = m_position;
			m_bTargetPositionChanged = true;
		}

		m_bMove = false;
		m_followNickName[0] = '\0';
	}
}

void CRealMyRoom_Avatar::Follow(const char* nickName)
{
	if (nickName)
		strcpy(m_followNickName, nickName);
	else
		m_followNickName[0] = '\0';
}

void CRealMyRoom_Avatar::ChangeMotion(int motion, const char* name,
	float blendTime)
{
	const char* motionName;

	m_motion = motion;
	if (m_pPetFrame == NULL)
		return;

	switch (motion)
	{
	case 0:
		motionName = "\xb1\xe2\xba\xbb\xc0\xda\xbc\xbc";
		break;
	case 2:
		motionName = "chat_\xbe\xc9\xb1\xe2";
		break;
	case 3:
		motionName = "chat_\xb4\xaf\xb1\xe2";
		break;
	case 1:
		motionName = "chat_\xb4\xde\xb8\xae\xb1\xe2";
		break;
	case 4:
		motionName = name;
		if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME) &&
			this == CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar)
		{
			unsigned long now = timeGetTime();

			if (now - m_lastMotionSendTime <= 1000)
				return;

			m_lastMotionSendTime = now;
			WSendPacket packet((enumClientPacket)0x63);
			packet.Encode1(7);
			packet.EncodeStr(std::string(name));
			packet.Send(TO_GAME);
		}
		break;
	default:
		motionName = "\xb1\xe2\xba\xbb\xc0\xda\xbc\xbc";
		break;
	}

	w_motion_data* pMotion =
		m_pPetFrame->FindMotionData(motionName, true, NULL);
	if (pMotion == NULL)
		return;

	if (blendTime > 0.0f && m_pMotion != NULL)
	{
		m_prevBoneSet.CopyFrom(&m_boneSet);
		m_pPrevMotion = m_pMotion;
		m_prevMotion = m_motion;
		m_pMotion = pMotion;
		m_motion = motion;
		m_motionTime = 0.0f;
		m_blendDuration = blendTime;
	}
	else
	{
		m_pPrevMotion = NULL;
		m_motionTime = 0.0f;
		m_pMotion = pMotion;
		m_pPetFrame->Animate983(&m_boneSet, NULL, pMotion->s);
	}

	if (m_motionTime == 0.0f)
	{
		bool bInvisible = motion != 4;
		WBone* pBone;

		switch (m_tidChar & 0x3ffffff)
		{
		case 4:
			pBone = m_pPetFrame->GetBonePet()->FindBone("D_Ball");
			if (pBone)
				pBone->SetInvisible(false, bInvisible);
			pBone = m_pPetFrame->GetBonePet()->FindBone("D_Racket");
			if (pBone)
				pBone->SetInvisible(false, bInvisible);
			break;
		case 5:
			pBone = m_pPetFrame->GetBonePet()->FindBone("E_Cannon");
			if (pBone)
				pBone->SetInvisible(false, bInvisible);
			pBone = m_pPetFrame->GetBonePet()->FindBone("E_Telescope");
			if (pBone)
				pBone->SetInvisible(false, bInvisible);
			break;
		case 6:
			pBone = m_pPetFrame->GetBonePet()->FindBone(RB_STR_G_BIRD);
			if (pBone)
				pBone->SetInvisible(false, bInvisible);
			break;
		case 7:
			pBone = m_pPetFrame->GetBonePet()->FindBone("H_Sword01");
			if (pBone)
				pBone->SetInvisible(false, bInvisible);
			pBone = m_pPetFrame->GetBonePet()->FindBone("H_Sword02");
			if (pBone)
				pBone->SetInvisible(false, bInvisible);
			pBone = m_pPetFrame->GetBonePet()->FindBone("H_karen");
			if (pBone)
				pBone->SetInvisible(false, bInvisible);
			break;
		}

		SetPendingTexPart("face", CreateCharacterOption::Instance()->GetChatFacial983(m_tidChar & 0x3ffffff, 3));
	}

	ProcessMotion(0.0f);
}

void CRealMyRoom_Avatar::ProcessMotion(float elapsed)
{
	if (m_pPetFrame == NULL || m_pPetFrame->GetCurAniPet() == NULL)
		return;

	if (m_motion != 0 && m_motion != 2 && m_motion != 3 && m_motion != 1)
	{
		if (m_pMotion->e - m_pMotion->s < m_motionTime)
			ChangeMotion(0, NULL, 0.2f);
	}

	if (m_pPrevMotion != NULL)
	{
		float ratio = m_motionTime / m_blendDuration;

		if (ratio > 1.0f)
		{
			m_pPrevMotion = NULL;
		}
		else
		{
			float time = m_motionTime;
			float length = m_pMotion->e - m_pMotion->s;
			if (length < m_motionTime)
				time = length;

			WBoneSet blendSet;
			m_pPetFrame->Animate983(&blendSet, m_pMotion->name, time);
			m_boneSet.Blend(&m_prevBoneSet, &blendSet, ratio);
		}
	}
	else
	{
		float length = m_pMotion->e - m_pMotion->s;

		if (m_motionTime > length)
		{
			if (length > 0.01f)
			{
				float loops = floorf(m_motionTime / length);
				m_motionTime -= loops * length;
			}
			else
			{
				m_motionTime = 0.0f;
			}
		}

		m_pPetFrame->Animate983(&m_boneSet, NULL, m_pMotion->s + m_motionTime);
	}

	if (m_motionTime == 0.0f)
	{
		ScriptProcess(m_pMotion, m_pMotion->s, m_pMotion->s, true);
	}
	else
	{
		ScriptProcess(m_pMotion, m_pMotion->s + m_motionTime,
			m_pMotion->s + m_motionTime + elapsed, false);
	}
}

void* CRealMyRoom_Avatar::AttachFx(const Attacher& attacher, const char* name,
	bool bSequence, defines_t* defines)
{
	int len = strlen(name);
	if (len < 5)
		return NULL;

	if (strcmpi(name + len - 3, "spr") == 0)
	{
		CFxSpray* pSpray = m_fxBox.LoadSpray(name, defines);
		if (pSpray == NULL)
			return NULL;

		((sFxSprayAttach*)pSpray)->m_bSequence = bSequence;
		((sFxSprayAttach*)pSpray)->m_pAttacher = &attacher;
		((sFxSprayAttach*)pSpray)->m_attacher.x = attacher.x;
		((sFxSprayAttach*)pSpray)->m_attacher.y = attacher.y;
		m_fxBox.AddSpray(pSpray);
		return pSpray;
	}
	else
	{
		CFxSequence* pSequence = m_fxBox.LoadSequence(name, true, defines);
		if (pSequence == NULL)
			return NULL;

		((sFxSequenceAttach*)pSequence)->m_bSequence = bSequence;
		((sFxSequenceAttach*)pSequence)->m_pAttacher = &attacher;
		((sFxSequenceAttach*)pSequence)->m_attacher.x = attacher.x;
		((sFxSequenceAttach*)pSequence)->m_attacher.y = attacher.y;
		m_fxBox.AddSequence(pSequence);
		return pSequence;
	}
}

// Moves a box from `pos` along `delta`, sliding along the planes it hits.
// Returns true when the move was blocked.
bool SlideMove(const WVector& delta, const WVector& pos, const Waabb& box,
	WVector& out)
{
	float lengthSq = delta * delta;
	if (Wabs(lengthSq) < g_EPSILON)
	{
		out = pos;
		return false;
	}

	WPlane planes[4];
	WVector start = pos;
	WVector move = delta;
	int numPlanes = 0;
	int bump;
	for (bump = 0; bump < 1; bump++)
	{
		trace_s trace;
		WVector end = move + start;
		GetPVS().TraceOBB_New(start, end, box, &trace);
		if (trace.m_flag)
		{
			out = start;
			return true;
		}
		if (trace.m_fraction > 0.0f)
		{
			out = trace.m_endPos;
			numPlanes = 0;
		}
		if (trace.m_fraction == 1.0f)
			break;
		if (numPlanes >= 3)
			return true;

		int i;
		for (i = 0; i < numPlanes; i++)
		{
			if (trace.m_normal.x * planes[i].x +
					trace.m_normal.y * planes[i].y +
					trace.m_normal.z * planes[i].z >
				0.99f)
			{
				move.x += trace.m_normal.x;
				move.y += trace.m_normal.y;
				move.z += trace.m_normal.z;
				break;
			}
		}
		if (i < numPlanes)
			continue;
		planes[numPlanes] = *(const WPlane*)&trace.m_normal;
		numPlanes++;
	}
	return bump > 0;
}

// Builds the matrix that tilts the world up axis onto `up`.
void MakeUpMatrix(WMatrix& mat, const WVector& up)
{
	if (WisEqual(up, WVector::UNIT_POS_Y, 0.00001f))
	{
		mat.Reset();
		return;
	}
	WVector axis = WCrossProduct(up, WVector::UNIT_POS_Y);
	axis.Normalize();
	mat = RotMat(axis, (float)acos((double)(WVector::UNIT_POS_Y * up)));
}

void CRealMyRoom_Avatar::ProcessChangeState()
{
	m_prevState = m_state;
}

void CRealMyRoom_Avatar::Process(float elapsed)
{
	if (m_bCharInfoChanged)
	{
		m_partTidList = m_burden.m_partTidList;
		m_bCharInfoChanged = false;
		m_pPetFrame = m_burden.m_pPetFrame;
		m_burden.m_pPetFrame->CalcBVolume();
		m_pPetFrame->UpdateLightSource(&g_lightset, false, NULL);
		ParseBoundingBoxScript();

		// The standing motion of each action (stand, sit, lie).
		int motion;
		switch (m_action)
		{
		case 0:
			motion = 0;
			break;
		case 1:
			motion = 2;
			break;
		case 2:
			motion = 3;
			break;
		default:
			motion = 0;
			break;
		}
		ChangeMotion(motion, NULL, 0.0f);
	}

	if (CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar == this &&
		(CSharedDoc::Instance()->m_myInfo.info.dwFlagBlock & 8))
		m_headIcon = 3;
	else
		m_headIcon = -1;

	m_prevState = m_state;
	ProcessMotion(elapsed);

	if (m_followNickName[0])
	{
		CRealMyRoom_Avatar* pTarget =
			CRealMyRoom_AvatarManager::Instance()->FindAvatarByNickName(
				m_followNickName);
		if (pTarget != NULL)
		{
			WVector delta;
			delta = pTarget->m_position - m_position;
			float distance = WVectorLen(delta);
			if (distance > 64.0f)
			{
				m_followNickName[0] = 0;
				Stop();
			}
			else if (distance > 12.8f && distance > 0.0f)
			{
				delta.Normalize();
				delta *= distance - 12.8f;
				SetTargetPosition(delta + m_position);
				SetAction(0);
			}
		}
		else
		{
			Stop();
			m_bMove = false;
			m_followNickName[0] = 0;
		}
	}

	WVector moved;
	moved.x = moved.y = moved.z = 0.0f;
	WVector dir;
	dir = m_targetPosition - m_position;
	if (!(Wabs(dir.x) < g_EPSILON) || !(Wabs(dir.z) < g_EPSILON))
	{
		dir.y = 0.0f;
		dir.Normalize();
		WVector delta;
		delta = dir * 16.5f * elapsed;
		WVector remain;
		remain = WVector(m_targetPosition.x - m_position.x, 0.0f,
			m_targetPosition.z - m_position.z);
		if (WVectorLen(remain) < delta.Magnitude())
			delta = remain;

		WVector newPos;
		if (this == CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar &&
			m_bVisible)
		{
			Waabb box(WVector(-1.0f, -40.0f, -1.0f), WVector(1.0f, 8.0f, 1.0f));
			SlideMove(delta, m_position, box, newPos);
		}
		else
		{
			newPos = delta + m_position;
		}

		moved = newPos - m_position;
		moved.y = 0.0f;
		if (WVectorLen(moved) > 0.01f)
		{
			int ground;
			GetPVS().GetGroundPoint(newPos, true, &ground, true);
			if (ground >= 0)
				m_position = newPos;
			else if (this == CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar)
				moved.x = moved.y = moved.z = 0.0f;
		}
		else
		{
			moved.x = moved.y = moved.z = 0.0f;
		}
	}

	SendMove(moved, false);

	float speed = moved.x * moved.x + moved.z * moved.z;
	if ((float)sqrt((double)speed) > 0.01f)
	{
		if (m_motion != 1)
			ChangeMotion(1, NULL, 0.2f);
	}
	else
	{
		if (this == CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar)
		{
			if (m_moveDelta.x != 0.0f || m_moveDelta.z != 0.0f)
				SendMove(WVector::ZERO, true);
			m_targetPosition = m_position;
		}
		if (m_motion != 4)
		{
			switch (m_action)
			{
			case 0:
				if (m_motion != 0)
					ChangeMotion(0, NULL, 0.2f);
				break;
			case 1:
				if (m_motion != 2)
					ChangeMotion(2, NULL, 0.2f);
				break;
			case 2:
				if (m_motion != 3)
					ChangeMotion(3, NULL, 0.3f);
				break;
			}
		}
	}

	if (speed > 0.001f)
	{
		m_targetAngle = (float)atan2((double)dir.z, (double)dir.x) + 1.5707964f;
	}
	else if (this != CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar &&
		m_bPendingTargetAngle)
	{
		float angle = m_pendingTargetAngle;
		m_targetAngle = RadianNormalize2PI(angle);
	}

	float diff = m_targetAngle - m_angle;
	diff = RadianNormalize2PI(diff);
	if (diff > g_PI)
		diff -= g_2_PI;
	if (diff > 0.0f && m_targetAngle < m_angle)
		m_angle -= g_2_PI;
	else if (diff < 0.0f && m_targetAngle > m_angle)
		m_angle += g_2_PI;
	if (fabs(diff) > 0.001f)
	{
		float angle = m_angle + elapsed * diff * 10.0f;
		m_angle = angle;
		if (diff > 0.0f && angle > m_targetAngle)
			m_angle = m_targetAngle;
		else if (diff < 0.0f && angle < m_targetAngle)
			m_angle = m_targetAngle;
	}

	m_position = GetPVS().GetGroundPoint(m_position, false, NULL, false);

	WVector toCamera;
	toCamera = g_view->camera.pivot - m_position;
	float cameraDistance = (float)sqrt(
		(double)(toCamera.z * toCamera.z + toCamera.x * toCamera.x));
	if (cameraDistance > 192.0f)
	{
		m_time = 0.0f;
	}
	else if (cameraDistance > 160.0f)
	{
		m_time = 1.0f - (cameraDistance - 160.0f) * 0.03125f;
	}
	else
	{
		bool bFaded = false;
		if (IsLocalContent((localContentType_t)10))
		{
			std::map<unsigned long, sBriefUserInfo>::iterator it =
				CSharedDoc::Instance()->m_briefUserInfoMap.find(m_guid);
			if (((*it).second.dwIdentity & 4) && !((*it).second.state & 1))
				bFaded = true;
		}
		if (bFaded)
			m_time = 0.1f;
		else
			m_time = 1.0f;
	}

	WVector normal;
	if (m_motion == 3)
		normal = GetPVS().GetMeshNormal(m_position);
	else
		normal = WVector::UNIT_POS_Y;

	WVector axis;
	axis = WCrossProduct(m_up, normal);
	axis.Normalize();
	float axisLengthSq = axis * axis;
	if (!(Wabs(axisLengthSq) < g_EPSILON))
	{
		if ((float)acos((double)(m_up * normal)) < elapsed * g_2_PI)
			m_up = normal;
		else
			m_up = RotVec(m_up, RotMat(axis, elapsed * -6.2831855f));
	}

	WMatrix mat;
	MakeUpMatrix(mat, m_up);
	mat = RotMat(m_angle, 1) * mat;
	mat.pivot = m_position;

	if (m_pPetFrame)
	{
		m_pPetFrame->GetBonePet()->SetMatrix(mat);
		m_pPetFrame->ApplyBoneSet983(&m_boneSet);
		m_pPetFrame->Transform(g_view, 1.0f, 0);
		m_pPetFrame->UpdateBound(true);
		m_pPetFrame->SetAlpha((unsigned char)(m_time * 255.0f));
	}

	if (m_facialTime > 0.0f)
	{
		m_facialTime -= elapsed;
		if (m_facialTime < 0.0f)
		{
			m_facialTime = 0.0f;
			SetPendingTexPart("face", CreateCharacterOption::Instance()->GetChatFacial983(m_tidChar & 0x3ffffff, 3));
		}
	}

	float elapsedMs = elapsed * 1000.0f;
	m_fxBox.ProcessAttacher(elapsedMs);
	m_fxBox.CFxBox::Process(elapsedMs);

	m_motionTime += elapsed;
	m_lifeTime983 += elapsed;
}

void CRealMyRoom_Avatar::Render()
{
	if (!m_bVisible)
		return;

	if (IsLocalContent((localContentType_t)10))
	{
		if (this != CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar &&
			m_bAdministrator)
		{
			int guid = m_guid;
			std::map<unsigned long, sBriefUserInfo>::const_iterator it =
				CSharedDoc::Instance()->m_briefUserInfoMap.find(guid);
			if (!(sBriefUserInfo((*it).second).state & 1))
			{
				if (guid != CSharedDoc::Instance()->m_myInfo.info.dwGuid)
					return;
			}
		}
	}

	WVector pos;
	if (m_pPetFrame == NULL || m_pPetFrame->GetCurAniPet() == NULL)
	{
		pos = GetPVS().GetGroundPoint(m_position, false, NULL, false);
	}
	else
	{
		pos =
			m_pPetFrame->GetBonePet()->FindBone(BONE_PELVIS)->GetMatrix().pivot;
		pos = GetPVS().GetGroundPoint(pos, false, NULL, false);
	}

	if (COption::Instance()->vIsShadowEnabled())
	{
		WVector normal(2.9f, 0.0f, 0.0f);
		WVector tangent(0.0f, 0.0f, 2.9f);
		GetPVS().DrawDecal(m_pMain->m_resD, pos, normal, tangent, 1.0f, m_time,
			W_POLY_BLEND_NORMAL, false);
	}

	if (m_pPetFrame != NULL && m_pPetFrame->GetCurAniPet() != NULL)
	{
		if (g_view->InFrustum(m_pPetFrame->GetBonePet()->m_bound))
		{
			if (CRealMyRoom_AvatarManager::Instance()->m_pFocusAvatar == this)
			{
				m_light.nearOne = g_lightset.nearOne;
				m_light.diffuse = m_light.ambient = 0xffffffff;
			}
			else
			{
				m_light.nearOne = g_lightset.nearOne;
				m_light.ambient = 0xffd2d2d2;
				m_light.diffuse = ModifyColor(g_lightset.diffuse,
					GetPVS().GetShadowIntensity(pos));
			}

			m_pPetFrame->UpdateLightSource(&m_light, false, NULL);
			m_pPetFrame->Render(g_view, true, 1.0f, false, false, true);
		}
	}

	g_view->Flush(1);

	m_fxBox.Display(g_view);
}

void CRealMyRoom_Avatar::RenderOverlay(bool bDetail)
{
	if (m_pBalloon)
		m_pBalloon->SetTransparency(m_time);

	if (GOLFDOC()->m_gameMode != 0x200)
	{
		if (Wabs(m_time) < g_EPSILON)
			return;
	}

	float alpha = GOLFDOC()->m_gameMode == 0x200 ? 1.0f : m_time;

	if (m_pPetFrame == NULL || m_pPetFrame->GetCurAniPet() == NULL)
		return;

	if (!g_view->InFrustum(m_pPetFrame->GetBonePet()->m_bound))
		return;

	WVector head;
	head = m_pPetFrame->GetBonePet()->FindBone(BONE_HEADNUB)->GetMatrix().pivot;

	WVector screen = g_view->Projection(head);
	if (!(screen.z <= OVERLAY_NEAR_Z) && !(screen.z >= 1.0f))
	{
		screen.x = floorf(screen.x);
		screen.y = floorf(screen.y);

		if (IsLocalContent((localContentType_t)10))
		{
			if (m_guid != CSharedDoc::Instance()->m_myInfo.info.dwGuid)
			{
				std::map<unsigned long, sBriefUserInfo>::iterator it =
					CSharedDoc::Instance()->m_briefUserInfoMap.find(m_guid);
				if ((*it).second.IsIdentity(4) && !((*it).second.state & 1))
					return;
			}
		}

		if (CProjectG::Instance()->HidePrivacy())
			return;

		if (!m_bVisible)
			return;

		float y = screen.y - 4.0f;
		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
		y -= 22.0f;
		FrEmoticon* pEmoticon = g_pFresh->GetManager()->GetEmoticon();

		WOverlay* pIcon;
		if (m_bAngelicWings)
			pIcon = static_cast<WOverlay*>(m_pMain->m_resC[m_gender % 2]);
		else if (m_bManner)
			pIcon = static_cast<WOverlay*>(m_pMain->m_resB[m_gender % 2]);
		else
			pIcon = static_cast<WOverlay*>(m_pMain->m_resA[m_gender]);

		unsigned int emblemWidth = m_pEmblem ? m_pEmblem->Width() : 0;
		int iconWidth = pIcon->GetWidth();
		float x;
		unsigned long alphaColor;
		unsigned long color;
		WRect dst;
		{
			float width =
				(float)emblemWidth + pEmoticon->GetTextWidth(m_nickName);
			WRect src(0.0f, 0.0f, 1.0f, 1.0f);
			x = screen.x - (width + iconWidth) * 0.5f;
			dst = WRect(x, y - 7.0f, pIcon->GetWidth(), pIcon->GetHeight());
			x = (float)pIcon->GetWidth() + x;

			alphaColor = (unsigned char)(int)(alpha * 255.0f) << 24;
			color = alphaColor | 0xffffff;
			pIcon->Render(g_view, src, dst, 0, color, 0.0f, 0);
		}

		if (m_pEmblem)
		{
			dst = WRect(0.5f / (float)m_pEmblem->Width() + x,
				0.5f / (float)m_pEmblem->Height() + y - 5.0f,
				(float)m_pEmblem->Width(), (float)m_pEmblem->Height());
			pGDI->DrawTexture(m_pEmblem, dst, color, 0);
			x = (float)m_pEmblem->Width() + x;
		}

		pGDI->SetScale(1.0f);
		pGDI->SetTextStyle(2);
		if (CRealMyRoom_AvatarManager::Instance()->m_pFocusAvatar == this)
			pGDI->SetTextColor(alphaColor, color);
		else
			pGDI->SetTextColor(color, alphaColor);

		pEmoticon->SetAnim(false);
		pEmoticon->PrintText(WPoint(x, y), 0, m_nickName, color);
	}
}

void CRealMyRoom_Avatar::ScriptProcess(const w_motion_data* motion, float from,
	float to, bool bInit)
{
	if ((!(from <= motion->e) || from == to) && !bInit)
		return;

	int start, end;
	from *= 30.0f;
	if (bInit)
	{
		start = Ceil(from, 0.03f);
		end = start;
	}
	else
	{
		to = Min(motion->e, to) * 30.0f;
		start = Ceil(from, 0.03f);
		end =
			Wabs((float)floor(to) - to) < 1e-5f ? (int)to - 1 : (int)floor(to);

		if (start == (int)floor(motion->s * 30.0f + 0.5f))
			start++;

		if (start > end)
			return;
	}

	if (m_pPetFrame)
	{
		WPuppet* pPuppet = m_pPetFrame->GetCurAniPet();

		if (pPuppet)
		{
			int first, last;
			if (pPuppet->CheckFrameData(start, end, first, last))
			{
				for (int i = first; i <= last; i++)
					ParsePuppetScript(pPuppet->m_frame[i].data);
			}
		}
	}
}

void CRealMyRoom_Avatar::ParsePuppetScript(const char* script)
{
	static char s_command[1024];
	static char s_name[64];

	unsigned long len;

	if (script == NULL)
		return;

	const char* str = script;
	do
	{
		str = strchr(str, '*');
		if (str == NULL)
			return;

		int cmdLen = 0;
		while (str[cmdLen] != '\0')
		{
			++cmdLen;
			if (str[cmdLen] == ' ')
				break;
			if (str[cmdLen] == '(')
			{
				++cmdLen;
				break;
			}
		}

		if (cmdLen <= 1)
			return;

		strncpy(s_command, str, cmdLen);
		str += cmdLen;
		s_command[cmdLen] = '\0';

		if (strcmp(s_command, "*fx(") == 0)
		{
			char name[64];
			char fxName[64];
			char token[64];
			char boneName[64];

			name[0] = '\0';
			const char* next = GetStrConst(str, name);
			if (!(len = strlen(name)) || strlen(name) >= 64)
				return;

			if (name[0] == '@')
			{
				// "@<motion>" restricts the effect to one motion.
				if (m_pMotion == NULL)
					return;
				if (strcmp(name + 1, m_pMotion->name) != 0)
					return;

				next = GetStrConst(next, fxName);
				if (!(len = strlen(fxName)) || strlen(fxName) >= 64)
					return;
			}
			else
			{
				int i = 0;
				char c;
				do
				{
					c = name[i];
					fxName[i] = c;
					++i;
				} while (c != '\0');
			}

			str = GetStrConst(next, token);
			int tokenLen = strlen(token);
			if (tokenLen == 0 || tokenLen >= 64)
				return;

			int j = 0;
			char c;
			do
			{
				c = token[j];
				boneName[j] = c;
				++j;
			} while (c != '\0');

			if (!(len = strlen(boneName)) || strlen(boneName) >= 64)
				return;

			Attacher attacher;
			unsigned int count;
			for (count = 0; count < 4; ++count)
			{
				if (count != 0)
				{
					str = GetStrConst(str, boneName);
					if (!(len = strlen(boneName)) || strlen(boneName) >= 64)
						break;
				}

				WBone* pBone = m_pPetFrame->GetBonePet()->FindBone(boneName);
				if (pBone != NULL)
				{
					attacher.mat[count] = &pBone->GetMatrix();
				}
				else
				{
					WPuppet* pPuppet = m_pPetFrame->GetBonePet();
					w_bound_box* bb = pPuppet->m_bbList.Find(boneName);
					if (bb != NULL)
					{
						if (bb->bone != NULL)
							attacher.mat[count] = &bb->bone->GetMatrix();
						attacher.pos[count] = &bb->info->spherePivot;
					}
				}
			}

			if ((int)count > 0)
				AttachFx(attacher, fxName, true, NULL);
		}
		else if (strcmp(s_command, "*hidebone(") == 0 ||
			strcmp(s_command, "*showbone(") == 0)
		{
			char boneName[64] = "";
			int count = 0;

			do
			{
				str = GetStrConst(str, boneName);
				if (!(len = strlen(boneName)) || strlen(boneName) >= 64)
					break;

				WBone* pBone = m_pPetFrame->GetBonePet()->FindBone(boneName);
				if (pBone != NULL)
					pBone->SetInvisible(false, s_command[1] == 'h');

				if (str == NULL)
					return;
				if (*str == ')')
					break;
			} while (++count < 64);
		}
		else if (strcmp(s_command, "*snd(") == 0)
		{
			str = GetStrConst(str, s_name);
			if (!(len = strlen(s_name)) || strlen(s_name) >= 64)
				return;

			WVector delta = m_position - g_camera.pivot;
			if (delta.Magnitude() < 192.0f)
				g_audio->PlaySfx(s_name, &delta, 5, NULL, NULL, 0.5f, 200.0f);
		}
		else if (strcmp(s_command, "*stepsnd(") == 0)
		{
			if (IsLocalContent((localContentType_t)10))
			{
				// Hidden administrators walk silently.
				if (this != CRealMyRoom_AvatarManager::Instance()->m_pMyAvatar)
				{
					len = m_guid;
					std::map<unsigned long, sBriefUserInfo>::const_iterator it =
						CSharedDoc::Instance()->m_briefUserInfoMap.find(len);
					if (m_bAdministrator &&
						!(sBriefUserInfo((*it).second).state & 1))
						continue;
				}

				const char* sound = GetPVS().GetStepSound(
					GetPVS().GetGroundPoint(m_position, false, NULL, false));
				if (sound != NULL)
				{
					WVector delta = m_position - g_camera.pivot;
					if (delta.Magnitude() < 192.0f)
						g_audio->PlaySfx(sound, &delta, 5, sound, NULL, 0.5f,
							200.0f);
				}
			}
			else
			{
				const char* sound = GetPVS().GetStepSound(
					GetPVS().GetGroundPoint(m_position, false, NULL, false));
				if (sound != NULL)
				{
					WVector delta = m_position - g_camera.pivot;
					if (delta.Magnitude() < 192.0f)
						g_audio->PlaySfx(sound, &delta, 5, sound, NULL, 0.5f,
							200.0f);
				}
			}
		}
		else if (strcmp(s_command, "*ptex(") == 0)
		{
			char part[64];
			char texture[64];

			part[0] = '\0';
			const char* next = GetStrConst(str, part);
			if (!(len = strlen(part)) || strlen(part) >= 64)
				return;

			if (part[0] == '@')
			{
				if (m_pMotion == NULL)
					return;
				if (strcmpi(part + 1, m_pMotion->name) != 0)
					return;

				next = GetStrConst(next, s_name);
				if (!(len = strlen(s_name)) || strlen(s_name) >= 64)
					return;
			}
			else
			{
				int i = 0;
				char c;
				do
				{
					c = part[i];
					s_name[i] = c;
					++i;
				} while (c != '\0');
			}

			str = GetStrConst(next, texture);
			if (!(len = strlen(texture)) || strlen(texture) >= 64)
				return;

			if (m_facialTime > 0.0f)
				return;

			SetPendingTexPart(s_name, texture);
		}
		else if (stricmp(s_command, "*showvis(") == 0)
		{
			ParseScript_ShowVis(true, str);
		}
		else if (stricmp(s_command, "*hidevis(") == 0)
		{
			ParseScript_ShowVis(false, str);
		}
	} while (str != NULL);
}

void CRealMyRoom_Avatar::ParseScript_ShowVis(bool bShow, const char* groups)
{
	char token[256] = "";
	int count = 0;

	do
	{
		groups = GetStrConst(groups, token);

		if (strlen(token) == 0 || strlen(token) >= sizeof(token))
			break;

		std::list<std::string>* pGroup =
			CSharedDoc::Instance()->GetVisGroup(token);

		if (pGroup)
		{
			for (std::list<std::string>::iterator it = pGroup->begin();
				it != pGroup->end(); ++it)
			{
				WBone* pBone =
					m_pPetFrame->GetBonePet()->FindBone((*it).c_str());

				if (pBone)
					pBone->SetInvisible(false, !bShow);
			}
		}

		if (groups == NULL || *groups == ')')
			break;
	} while (++count < 64);
}

void CRealMyRoom_Avatar::ParseBoundingBoxScript()
{
	if (m_pPetFrame == NULL || m_pPetFrame->GetBonePet() == NULL)
		return;

	static std::vector<CFxSpray*> s_sprays;
	static std::vector<CFxSequence*> s_sequences;

	char key[] = "*fx";
	WList<w_bound_box*>* pList = &m_pPetFrame->GetBonePet()->m_bbList;

	unsigned int i;
	for (i = 0; i < s_sprays.size(); ++i)
		m_fxBox.DeleteSpray(s_sprays[i]);
	for (i = 0; i < s_sequences.size(); ++i)
		m_fxBox.DeleteSequence(s_sequences[i], true);

	s_sprays.std::vector<CFxSpray*>::~vector();
	s_sequences.std::vector<CFxSequence*>::~vector();

	for (w_bound_box* bb = pList->Start(); bb != NULL; bb = pList->Next())
	{
		const char* option = bb->info->option;
		if (option == NULL || bb->bone == NULL ||
			bb->bone->m_flag.GetFlag(WBone::INVISIBLE))
			continue;

		const char* next;
		const char* ext;
		for (const char* str = strstr(option, key); str != NULL;
			str = strstr(next, key))
		{
			char name[64];
			char part[64];

			unsigned int pos = strlen(key);
			if (str[pos] == '(')
				++pos;

			if (pos > strlen(key))
				next = GetStrConst(str + pos, name);
			else
				next = GetCharsTillSpace(str + pos, name);

			int len = strlen(name);
			if (len == 0 || len >= 64)
				continue;

			ext = strrchr(name, '.');
			if (ext == NULL || strlen(ext) < 4)
				continue;

			next = GetStrConst(next, part);
			len = strlen(part);
			if (len == 0 || len >= 64)
				continue;

			// "&<tid>" entries restrict the effect to avatars wearing one
			// of the listed parts.
			bool bCondition = false;
			bool bWearing = false;
			do
			{
				if (part[0] != '&')
					break;

				bCondition = true;
				unsigned long tid = atoi(part + 1);
				if (m_pPetFrame->GetPartList().FindPart(tid) ||
					m_pPetFrame->FindAuxPart(tid))
				{
					bWearing = true;
					break;
				}

				next = GetStrConst(next, part);
				len = strlen(part);
			} while (len != 0 && len < 64);
			if (bCondition && !bWearing)
				continue;

			Attacher attacher;
			attacher.mat[0] = &bb->bone->GetMatrix();
			attacher.pos[0] = &bb->info->spherePivot;

			if (stricmp(ext + 1, "spr") == 0)
			{
				CFxSpray* pSpray = m_fxBox.LoadSpray(name, NULL);
				if (pSpray != NULL)
				{
					// The original keeps a pointer to this stack attacher
					// next to its copy.
					((sFxSprayAttach*)pSpray)->m_pAttacher = &attacher;
					((sFxSprayAttach*)pSpray)->m_attacher.x = attacher.x;
					((sFxSprayAttach*)pSpray)->m_attacher.y = attacher.y;
					memcpy(((sFxSprayAttach*)pSpray)->m_attacher.mat,
						attacher.mat, sizeof(attacher.mat));
					memcpy(((sFxSprayAttach*)pSpray)->m_attacher.pos,
						attacher.pos, sizeof(attacher.pos));
					m_fxBox.AddSpray(pSpray);
					s_sprays.push_back(pSpray);
				}
			}
			else if (stricmp(ext + 1, "seq") == 0)
			{
				CFxSequence* pSequence = m_fxBox.LoadSequence(name, true, NULL);
				if (pSequence != NULL)
				{
					((sFxSequenceAttach*)pSequence)->m_pAttacher = &attacher;
					((sFxSequenceAttach*)pSequence)->m_attacher.x = attacher.x;
					((sFxSequenceAttach*)pSequence)->m_attacher.y = attacher.y;
					m_fxBox.AddSequence(pSequence);
					s_sequences.push_back(pSequence);
				}
			}
		}
	}
}

sTexturePart* CRealMyRoom_Avatar::FindTexturePart(const char* part,
	const char* texture)
{
	sTexturePart* pTexPart;
	std::list<sTexturePart*>::iterator it;

	it = std::find_if(m_texParts.begin(), m_texParts.end(),
		sFindTexPart(texture));

	if (it != m_texParts.end())
	{
		pTexPart = *it;
	}
	else
	{
		pTexPart = new sTexturePart;

		strcpy(pTexPart->name, part);
		strcpy(pTexPart->texture, texture);
		m_texParts.push_back(pTexPart);
	}

	return pTexPart;
}

void CRealMyRoom_Avatar::SetPendingTexPart(const char* part,
	const char* texture)
{
	if (m_pPetFrame == NULL || m_pPetFrame->GetCurAniPet() == NULL)
		return;

	sTexturePart* pTexPart = FindTexturePart(part, texture);

	if (pTexPart->pBitmap == NULL)
	{
		m_pPetFrame->GetBonePet()->ChangeTexturePart(m_pPetFrame->GetFaceTex(),
			pTexPart->texture, &pTexPart->pBitmap, &pTexPart->rect);
	}
	else
	{
		m_pPetFrame->GetBonePet()->ChangeTexturePart(m_pPetFrame->GetFaceTex(),
			pTexPart->pBitmap, pTexPart->rect);
	}

	m_pPetFrame->SetMakeup();
}

void CRealMyRoom_Avatar::SetChatFacial(int facial)
{
	SetPendingTexPart("face", CreateCharacterOption::Instance()->GetChatFacial983(m_tidChar & 0x3ffffff, facial));

	m_facialTime = 5.0f;
}

void CRealMyRoom_Avatar::SetBalloonText(const char* text)
{
	m_balloonTime = m_time;
	m_balloonElapsed = 0;
}

void CRealMyRoom_Avatar::SetTradeTitle(const char* title)
{
	if (title)
		strcpy(m_tradeTitle, title);
	else
		m_tradeTitle[0] = '\0';
}

void CRealMyRoom_Avatar::RefreshUccClothes(unsigned long tid, unsigned long uid)
{
	if (m_pPetFrame == NULL)
		return;

	if (IsLocalContent(S4_GUID_BASE))
	{
		m_pPetFrame->SetPart(&m_partTidList, tid, uid);
		m_pPetFrame->SetPart(&m_partTidList, tid, uid);
	}
}

CRealMyRoom_AvatarManager::CRealMyRoom_AvatarManager()
{
	m_avatarCount = 0;
	for (int i = 0; i < 30; i++)
		m_pAvatar[i] = NULL;

	m_pMyAvatar = NULL;
	m_pFocusAvatar = NULL;
	m_pClickAvatar = NULL;
	m_bRender = true;
	m_bOverlayDetail = true;
}

CRealMyRoom_AvatarManager::~CRealMyRoom_AvatarManager()
{
	FreeAllAvatars();
}

void CRealMyRoom_AvatarManager::FreeAllAvatars()
{
	for (int i = 0; i < 30; i++)
	{
		if (m_pAvatar[i])
		{
			if (m_pAvatar[i]->m_pPetFrame)
			{
				delete m_pAvatar[i]->m_pPetFrame;
				m_pAvatar[i]->m_pPetFrame = NULL;
			}

			if (m_pAvatar[i])
			{
				delete m_pAvatar[i];
				m_pAvatar[i] = NULL;
			}
		}
	}
}

CRealMyRoom_Avatar* CRealMyRoom_AvatarManager::FindAvatarByIDName(
	const char* idName) const
{
	for (int i = 0; i < 30; i++)
	{
		if (m_pAvatar[i] != NULL)
		{
			if (strcmp(m_pAvatar[i]->m_idName, idName) == 0)
				return m_pAvatar[i];
		}
	}

	return NULL;
}

CRealMyRoom_Avatar* CRealMyRoom_AvatarManager::FindAvatarByNickName(
	const char* nickName) const
{
	for (int i = 0; i < 30; i++)
	{
		if (m_pAvatar[i] != NULL)
		{
			if (strcmp(m_pAvatar[i]->m_nickName, nickName) == 0)
				return m_pAvatar[i];
		}
	}

	return NULL;
}

CRealMyRoom_Avatar* CRealMyRoom_AvatarManager::FindAvatarByGuid(int guid) const
{
	for (int i = 0; i < 30; i++)
	{
		if (m_pAvatar[i] != NULL)
		{
			if (m_pAvatar[i]->m_guid == guid)
				return m_pAvatar[i];
		}
	}

	return NULL;
}

CRealMyRoom_Avatar* CRealMyRoom_AvatarManager::CreateAvatar(
	const sSlotInfo& slotInfo, const sCharacterInfo& charInfo,
	CRealMyRoomMain* pMain)
{
	int slot;

	for (slot = 0; slot < 30; slot++)
	{
		if (m_pAvatar[slot] == NULL)
			break;
	}

	if (slot == 30)
		return NULL;

	m_avatarCount++;

	CRealMyRoom_Avatar* pAvatar =
		new CRealMyRoom_Avatar(slotInfo, charInfo, pMain);
	pAvatar->SetPosition(GetPVS().GetGroundPoint(
		WVector(slotInfo.location[0], 0.0f, slotInfo.location[2]), false, NULL,
		false));
	pAvatar->SetAngle(slotInfo.location[1]);

	m_pAvatar[slot] = pAvatar;
	return pAvatar;
}

bool CRealMyRoom_AvatarManager::DeleteAvatar(const char* idName)
{
	int i;
	for (i = 0; i < 30; i++)
	{
		if (m_pAvatar[i])
		{
			if (strcmp(m_pAvatar[i]->m_idName, idName) == 0)
				break;
		}
	}

	if (i == 30)
		return false;

	if (m_pAvatar[i])
	{
		delete m_pAvatar[i];
		m_pAvatar[i] = NULL;
	}
	m_avatarCount--;

	return true;
}

CRealMyRoom_Avatar* CRealMyRoom_AvatarManager::AvatarByRayIntersect(
	const WVector& pos, const WVector& dir) const
{
	Waabb aabb;
	for (int i = 0; i < 30; i++)
	{
		if (m_pAvatar[i])
		{
			if (IsLocalContent((localContentType_t)10))
			{
				CSharedDoc* pDoc = CSharedDoc::Instance();
				if (m_pAvatar[i]->m_guid != pDoc->m_myInfo.info.dwGuid)
				{
					std::map<unsigned long, sBriefUserInfo>::iterator it;
					it = pDoc->m_briefUserInfoMap.find(m_pAvatar[i]->m_guid);

					if (!(pDoc->m_myInfo.info.dwIdentity & 4))
					{
						if ((sBriefUserInfo((*it).second).dwIdentity >> 2) & 1)
						{
							if (!(sBriefUserInfo((*it).second).state & 1))
								break;
						}
					}
				}
			}

			aabb.min = WVector(-1.0f, 0.0f, -1.0f);
			aabb.min += m_pAvatar[i]->m_position;
			aabb.max = WVector(1.0f, 8.0f, 1.0f);
			aabb.max += m_pAvatar[i]->m_position;

			if (RayIntersectAABB(aabb, pos, dir, NULL, 0.0f) < 1.0f)
				return m_pAvatar[i];
		}
	}

	return NULL;
}

CRealMyRoom_Avatar* CRealMyRoom_AvatarManager::CheckClick(float x,
	float y) const
{
	for (int i = 0; i < 30; i++)
	{
		if (m_pAvatar[i] && m_pAvatar[i]->IsClickable(x, y))
			return m_pAvatar[i];
	}

	return NULL;
}

void CRealMyRoom_AvatarManager::Process(float elapsed)
{
	for (int i = 0; i < 30; i++)
	{
		if (m_pAvatar[i] != NULL)
			m_pAvatar[i]->Process(elapsed);
	}
}

void CRealMyRoom_AvatarManager::Render()
{
	if (!m_bRender)
		return;

	for (int i = 0; i < 30; i++)
	{
		if (m_pAvatar[i] != NULL)
			m_pAvatar[i]->Render();
	}
}

void CRealMyRoom_AvatarManager::ResetAvatarPositionAngle()
{
	for (int i = 0; i < 30; i++)
	{
		if (m_pAvatar[i] != NULL)
		{
			m_pAvatar[i]->m_position = m_pAvatar[i]->m_targetPosition;
			m_pAvatar[i]->m_angle = m_pAvatar[i]->m_targetAngle;
		}
	}
}

static int __cdecl CompareCameraDistance(const void* a, const void* b)
{
	WVector camera = g_view->GetCamera().pivot;

	WVector deltaA;
	deltaA = camera - (*(CRealMyRoom_Avatar* const*)a)->GetPosition();
	WVector deltaB;
	deltaB = camera - (*(CRealMyRoom_Avatar* const*)b)->GetPosition();

	float distA = deltaA.SquareMagnitude();
	float distB = deltaB.SquareMagnitude();

	if (distA > distB)
		return -1;
	if (distA < distB)
		return 1;
	return 0;
}

void CRealMyRoom_AvatarManager::RenderOverlay()
{
	if (!m_bRender)
		return;

	CRealMyRoom_Avatar* avatars[30];
	int count = 0;
	for (int i = 0; i < 30; i++)
	{
		if (m_pAvatar[i])
			avatars[count++] = m_pAvatar[i];
	}

	qsort(avatars, count, sizeof(CRealMyRoom_Avatar*), CompareCameraDistance);

	for (int j = 0; j < count; j++)
	{
		if (avatars[j])
			avatars[j]->RenderOverlay(m_bOverlayDetail);
	}
}

CRealMyRoom_AvatarBurden::CRealMyRoom_AvatarBurden()
	: m_charInfo(), m_pPetFrame(NULL)
{
}

CRealMyRoom_AvatarBurden::CRealMyRoom_AvatarBurden(unsigned long guid,
	const sCharacterInfo& charInfo, CPetFrame* pPetFrame)
	: m_charInfo()
{
	m_guid = guid;

	m_charInfo = charInfo;
	m_pPetFrame = pPetFrame;
}

CRealMyRoom_AvatarBurden::~CRealMyRoom_AvatarBurden()
{
}

bool CRealMyRoom_AvatarBurden::LoadAvatarBurden()
{
	if (!m_partTidList.SetTids(&m_charInfo, 0xff))
		m_partTidList.SetDefaultTids();
	if (!m_partTidList.IsComboValid())
		m_partTidList.SetDefaultTids();

	if (m_pPetFrame)
	{
		delete m_pPetFrame;
		m_pPetFrame = NULL;
	}

	m_pPetFrame = new CPetFrame;

	if (!m_pPetFrame->Build(&m_partTidList, m_charInfo.tidAuxParts, true, false,
			m_guid))
	{
		delete m_pPetFrame;
		m_pPetFrame = NULL;
		return false;
	}

	m_pPetFrame->SetHairClr983(&m_partTidList, m_charInfo.hairClr, 0, true); // mesmo par do ntAvatarBurden::Load 983
	m_pPetFrame->SetShirtsClr(&m_partTidList, m_charInfo.shirtsClr);

	return true;
}
