#include "minatl.h"
#include "vc10_layout.h"
#define REBANG_AVATAR_AVATAR_CPP
#include "avatar_avatar.h"
#include "balloontext.h"
#include "projectg.h"
#include "fresh.h"
#include "packet.h"
#include "networksystem.h"
#include "shareddoc.h"
#include "partsinfo.h"
#include "commonutil.h"
#include "fxbox.h"
#include "quadtree.h"
#include "texturepart.h"
#include "wpuppet.h"
#include "wbone.h"
#include "wview.h"
#include "fxspray.h"
#include "fxsequence.h"
#include "fxinterpreter.h"
#include "jp/createcharacteroption.h"
#include "netresourcemanager.h"
#include "lobbymain.h"
#include "avatar_main.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "woverlay.h"
#include "clientsetting.h"
#include "soundmanager.h"
#include "gmtoolkit.h"
#include "jp/motionadvisor.h"
#include <mmsystem.h>
#include <algorithm>
#include "mathconsts.h"
#include "../shared/localize.h"
extern "C" void __cdecl jp983_rastro(const char* fmt, ...);

// HACK: the linker keeps the copy of "G_Bird" of golfplayer.obj (earlier in link order), which is a local static of a
// naked function there, so this unit must not emit its own copy: name that one.
#define RB_STR_G_BIRD "G_Bird"


const char* __fastcall GetStrConst(const char* src, char* dest);
unsigned long ModifyColor(unsigned long color, float factor);

static unsigned long BrightenColor983(unsigned long color, float f)
{
	unsigned long out = 0xff000000;
	for (int sh = 16; sh >= 0; sh -= 8)
	{
		float c = (float)((color >> sh) & 0xff) * f;
		if (c > 255.0f)
			c = 255.0f;
		unsigned long v = ((unsigned long)(int)c & 0xff) + 0x14;
		if (v > 0xff)
			v = 0xff;
		out |= v << sh;
	}
	return out;
}
int Ceil(float value, float tolerance);
WMatrix RotMat(const WVector& axis, float angle);

inline int IsAdministrator(unsigned long identity);

// the only definition of the GM toolkit's hole count lives in this unit; in its
// own section (the original has it after the facial texture names: data is
// emitted in reverse definition order)
__declspec(selectany) const int CGMToolkit::MAX_HOLE = 18;

// 983: as expressoes do rosto ficam no CreateCharacterOption (00e73e84, 15 personagens; getter 004ced70, chamado em
// 007a63bd/007a796e/007a85d0); a tabela estatica do 645 tinha so 10 -> Spika/NR/HR/CC liam alem do fim

ntAvatar::ntAvatar(CQuadTree& pvs, const sSlotInfo& slot,
	const sCharacterInfo& info, ntAvatarChatMain* pChatMain)
	: m_boneSet1(512), m_boneSet2(512) // D475: (true) virava count=1 e AddBoneKey escrevia alem do vetor (645: preAlloc de 256 chaves)
{
	// A224/C696: a visao cheia confere com as constantes que as visoes parciais do avatar_avatar.h usam (= 983)
	C_ASSERT(offsetof(ntAvatar, m_state) == NTAVATAR_OFS_BASE + 0xf0 && offsetof(ntAvatar, m_burden) == NTAVATAR_OFS_BASE + 0x144 && sizeof(ntAvatarBurden) == 0x2dc);
	C_ASSERT_VC10(offsetof(ntAvatar, m_state) == 0x158 && offsetof(ntAvatar, m_charInfo) == 0x489 && sizeof(ntAvatar) == 0xc88);
	C_ASSERT(offsetof(ntAvatar, m_b3d8) == NTAVATAR_OFS_BASE + 0x420 && offsetof(ntAvatar, m_charInfo) == NTAVATAR_OFS_CHARINFO);
	C_ASSERT(offsetof(ntAvatar, m_partTids) == NTAVATAR_OFS_PARTTIDS && offsetof(ntAvatar, m_pPetFrame) == NTAVATAR_OFS_PETFRAME);
	C_ASSERT_VC10(offsetof(ntAvatar, m_motionType) == NTAVATAR_OFS_MOTIONTYPE);
	m_pPVS = &pvs;
	strcpy(m_idName, slot.sNick);
	strcpy(m_nickName, slot.sNick);
	m_moveTarget[0] = 0;
	m_followName[0] = 0;

	if (IsLocalContent((localContentType_t)0x10))
	{
		if (slot.nChannelingFlag == 1)
		{
			strcpy(m_guildName, slot.sDisplayID);
			m_bHasGuild = true;
		}
		else
		{
			memset(m_guildName, 0, sizeof(m_guildName));
			m_bHasGuild = false;
		}
	}

	m_e0 = slot.dwUserUID;
	m_guid = slot.dwGuid;
	m_e8 = (unsigned char)((slot.dwIdentity >> 2) & 1);
	m_e9 = slot.gender;
	m_ea = slot.manner;
	m_eb = slot.angelicWings;
	m_ec = slot.tidChar;
	m_state = slot.state;
	m_prevState = m_state;
	m_action = slot.action;
	m_pMotion = NULL;
	m_pPrevMotion = NULL;
	m_blendElapsed = 0.0f;
	m_blendTime = 0.0f;
	m_6f8 = 0.0f;
	m_facialTime = 0.0f;
	m_up = WVector::UNIT_POS_Y;
	m_bTargetChanged = false;
	m_motionSendTime = 0;
	m_bPendingAngle = false;
	m_pChatMain = pChatMain;
	m_b3d8 = false;
	m_pPetFrame = NULL;
	m_charInfo = info;
	m_headIcon = -1;

	m_pEmblem = NULL;
	if (slot.GuildId != 0)
		m_pEmblem = (Bitmap*)NetResourceManager::Instance()->GetEmblem983(
			slot.GuildId, slot.dwEmblemVer983);

	m_pBalloon = NULL;
	m_balloonRand[0] = Random(0.8f, 1.0f);
	m_balloonRand[1] = Random(0.8f, 1.0f);
	m_balloonRand[2] = Random(0.8f, 1.0f);
	m_714 = 1.0f;

	m_moveAccum.Reset();
	SetAngle(slot.location[2]);
	SetState(m_state);
	SetAction(m_action);
	if (slot.strTradeTitle)
		strcpy(m_tradeTitle, slot.strTradeTitle);
	else
		m_tradeTitle[0] = 0;

	m_light.type = 2;
	m_light.ambient = ModifyColor(g_lightset.ambient, 1.0f);
	m_light.nearOne = g_lightset.nearOne;

	m_fxBox.Init(g_resrcmng);
	m_fxBox.SetCamera(&g_camera);
	m_fxBox.m_frameTime = 1000.0f / 30.0f;
	m_spFx983 = std::tr1::shared_ptr<FxInterpreter>(new FxInterpreter(&m_fxBox, 0)); // 983: 007a76e2..007a7731
}

ntAvatar::~ntAvatar()
{
	if (m_pBalloon)
		ntBalloonTextManager::Instance()->DeleteBalloonText(m_pBalloon);
	for (std::list<sTexturePart*>::iterator it = m_texParts.begin();
		it != m_texParts.end(); ++it)
	{
		delete *it;
	}
	m_texParts.clear();
	m_spFx983.reset(); // os fx do intérprete vivem no m_fxBox: soltar antes do ClearAll
	m_fxBox.ClearAll(false);
}

// 983 007a4d90 (chamada só na troca do burden, 007a7a09): fx "*fx(" das peças vestidas no boneco do lobby (brilho de anel,
// brincos etc.) e as chaves das asas que começam visíveis
void ntAvatar::AttachItemFx983()
{
	if (!m_pPetFrame)
		return;
	m_spFx983->DetachFxAll(-1);
	m_spFx983->DetachNamed(-1);
	m_spFx983->ParseAvatarScript_fx(m_pPetFrame, NULL, 0, false, false);
	m_spFx983->ShowNamed("\264\353\303\265\273\347", true, 0); // 00d04b7c "대천사"
	m_spFx983->ShowNamed("\277\344\301\244\263\257\260\263", true, 0); // 00d04b70 "요정날개"
}

bool ntAvatar::IsTargetPositionChanged(bool bReset)
{
	bool old = m_bTargetChanged;
	m_bTargetChanged = bReset;
	return old;
}

bool ntAvatar::IsClickable(float x, float y) const
{
	if (!g_pFresh->IsVisible())
		return false;

	if (!m_pPetFrame || !m_pPetFrame->GetCurAniPet())
		return false;

	// 983 007a37d0: m_alpha >= DAT_00cff040 (= -1.0842022e-19f, i.e. any alpha >= 0 passes)
	if (m_alpha < -1.0842022e-19f)
		return false;

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (m_state & 0x80)
	{
		WVector head;
		head = m_pPetFrame->GetBonePet()
				   ->FindBone("Bip01 HeadNub")
				   ->GetMatrix()
				   .pivot;

		WVector screen = g_view->Projection(head);
		if (!(screen.z <= 0.0f) && !(1.0f <= screen.z))
		{
			int screenX = (int)floorf(screen.x);
			int screenY = (int)floorf(screen.y) - 48;

			if (m_state & 0x80)
			{
				if ((ntAvatarManager::Instance()->m_pMyAvatar->m_pos - m_pos)
						.Magnitude() > 19.2f)
					return false;

				int width = (int)pGDI->GetTextExtendOld983(m_tradeTitle) + 10;
				if (width < 100)
					width = 100;

				int left = screenX - width / 2;
				int top = screenY - 8;
				if (left < x && x < left + width && top < y && y < top + 29)
					return true;

				// the state icon drawn above the head
				WOverlay* pIcon = m_pChatMain->m_stateIcons[7];
				int iconWidth = pIcon->GetWidth();
				int iconHeight = pIcon->GetHeight();
				left = screenX - iconWidth / 2;
				top = screenY - iconHeight / 2 - 36;
				if (left < x && x < left + iconWidth && top < y &&
					y < top + iconHeight)
					return true;
			}
		}
	}
	return false;
}

ntAvatar* ntAvatar::GetFollowingAvatar() const
{
	if (m_followName[0] != 0)
		return NULL;
	return ntAvatarManager::Instance()->FindAvatarByNickName(m_followName);
}

bool ntAvatar::GetBoneScreenCoord(const char* name, int* x, int* y) const
{
	if (!m_pPetFrame || !m_pPetFrame->GetCurAniPet())
		return false;
	WBone* pBone = m_pPetFrame->GetBonePet()->FindBone(name);
	if (!pBone)
		return false;
	WVector screen = g_view->Projection(pBone->GetMatrix().pivot);
	*x = (int)floor(screen.x);
	*y = (int)floor(screen.y);
	return true;
}

void ntAvatar::SetCharactorInfo(const sCharacterInfo& info)
{
	CPartTidList tids;
	Stop();
	m_pPetFrame = NULL;
	m_charInfo = info;
	m_ec = info.tid;
	m_fxBox.ClearAll(false);
}

void ntAvatar::SetPosition(const WVector& pos)
{
	if (0)
		((ntAvatar*)0)->ntAvatar::SetCharactorInfo(*(const sCharacterInfo*)0);
	m_pos = pos;
	m_targetPos = pos;
	m_bTargetChanged = true;
}

void ntAvatar::SetTargetPosition(const WVector& pos)
{
	m_targetPos = pos;
	m_bTargetChanged = true;
}

void ntAvatar::SetAngle(float angle)
{
	float normalized = RadianNormalize2PI(angle);
	m_angle = normalized;
	m_targetAngle = normalized;
}

void ntAvatar::SetTargetAngle(float angle)
{
	m_targetAngle = RadianNormalize2PI(angle);
}

void ntAvatar::SetPendingTargetAngle(float angle)
{
	m_pendingAngle = RadianNormalize2PI(angle);
	m_bPendingAngle = true;
}

void ntAvatar::SetState(unsigned long state)
{
	if (m_state == state)
		return;
	if (WNetworkSystem::Instance()->IsConnected((WNetworkSystem::eNetUnit)0) &&
		this == ntAvatarManager::Instance()->m_pMyAvatar)
	{
		WSendPacket packet((enumClientPacket)99);
		packet.Encode1(8);
		packet.Encode4(state);
		packet.Send(TO_GAME);
	}
	m_state = state;
	if (state & 0x7e)
		Stop();
}

void ntAvatar::SetAction(int action)
{
	if (m_action == action)
		return;
	if (WNetworkSystem::Instance()->IsConnected((WNetworkSystem::eNetUnit)0) &&
		this == ntAvatarManager::Instance()->m_pMyAvatar)
	{
		WSendPacket packet((enumClientPacket)99);
		packet.Encode1(5);
		packet.Encode4(action);
		packet.Send(TO_GAME);
	}
	m_action = action;
}

void ntAvatar::SendMove(const WVector& delta, bool bForce)
{
	if (WNetworkSystem::Instance()->IsConnected((WNetworkSystem::eNetUnit)0) &&
		this == ntAvatarManager::Instance()->m_pMyAvatar)
	{
		m_moveAccum += delta;
		float speed = m_spItems[2].getvalue();
		if (m_moveAccum.Magnitude() >= speed * 16.5f || bForce)
		{
			WSendPacket packet((enumClientPacket)99);
			packet.Encode1(6);
			packet.EncodeBuffer(&m_moveAccum.x, 4);
			packet.EncodeBuffer(&m_moveAccum.z, 4);
			packet.EncodeBuffer(&m_angle, 4);
			packet.Send(TO_GAME);
			m_moveAccum.Reset();
		}
	}
}

void ntAvatar::Stop()
{
	if (this == ntAvatarManager::Instance()->m_pMyAvatar)
	{
		if (m_targetPos != m_pos)
		{
			WVector zero;
			zero.x = 0.0f;
			zero.y = 0.0f;
			zero.z = 0.0f;
			SendMove(zero, true);
			m_targetPos = m_pos;
			m_bTargetChanged = true;
		}
		m_moveTarget[0] = 0;
		m_followName[0] = 0;
	}
}

void ntAvatar::Follow(const char* name)
{
	if (name)
		strcpy(m_followName, name);
	else
		m_followName[0] = 0;
}

// PKT (frente C, lote 51) - 983 007a5000: o envio do 0x63 que o 645 fazia dentro do caso 4 do ChangeMotion. Devolve false
// quando o pedido anterior foi ha 1 s ou menos (o ChangeMotion entao nao troca a animacao).
bool ntAvatar::SendMotion983(const char* name, int type)
{
	if (this == ntAvatarManager::Instance()->m_pMyAvatar)
	{
		unsigned long now = timeGetTime();
		if (now - m_motionSendTime <= 1000)
			return false;
		m_motionSendTime = now;
		WSendPacket packet((enumClientPacket)0x63);
		packet.Encode1((unsigned char)type);
		packet.EncodeStr(std::string(name));
		packet.Send(TO_GAME);
	}
	return true;
}

// D641: o 983 (007aeb9d, msg 0x198 do Lounge) le o CPetFrame do boneco direto; os outros .cpp nao veem o campo
CPetFrame* ntAvatar::GetPetFrame983() const
{
	return m_pPetFrame;
}

void ntAvatar::ChangeMotion(int motion, const char* name, float blendTime)
{
	const char* motionName;
	m_motionType = motion;
	if (!m_pPetFrame)
		return;
	m_spFx983->SetCond(name ? name : "", 0); // 983 007a77ea: fx "*fx(@pose)" das peças acendem pela pose
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
		// PKT (frente C, lote 51) - 983 (007a7828): o 0x63 saiu para o SendMotion983 (007a5000), sem o IsConnected
		motionName = name;
		if (!SendMotion983(name, 7))
			return;
		break;
	case 5: // 983 (007a783c): caso novo, tipo 0xa no pacote 0x63
		motionName = name;
		if (!SendMotion983(name, 0xa))
			return;
		break;
	default:
		motionName = "\xb1\xe2\xba\xbb\xc0\xda\xbc\xbc";
		break;
	}

	// 983 007a785f: nome da animacao pela tabela por personagem (CMotionAdvisor::GetMotionName 005a6540; chave = burden+4, parte = ec & 0x3ffffff)
	if (CMotionAdvisor::Instance())
	{
		const char* advised = CMotionAdvisor::Instance()->GetMotionName(m_burden.m_face, m_ec & 0x3ffffff, motionName);
		if (advised)
			motionName = advised;
	}

	w_motion_data* pMotion =
		m_pPetFrame->FindMotionData(motionName, true, NULL);
	if (!pMotion)
		return;

	if (blendTime > 0.0f && m_pMotion)
	{
		m_boneSet2.CopyFrom(&m_boneSet1);
		m_pPrevMotion = m_pMotion;
		m_prevMotionType = m_motionType;
		m_pMotion = pMotion;
		m_motionType = motion;
		m_blendElapsed = 0.0f;
		m_blendTime = blendTime;
	}
	else
	{
		m_pPrevMotion = NULL;
		m_blendElapsed = 0.0f;
		m_pMotion = pMotion;
		m_pPetFrame->Animate983(&m_boneSet1, NULL, pMotion->s);
	}

	if (m_blendElapsed == 0.0f)
	{
		bool bInvisible = motion != 4 && motion != 5; // 983 007a77d0: FUN_008b0570(0) nos tipos 4 e 5, senao 1
		switch ((m_ec & 0x3ffffff) - 4)
		{
		case 0:
		{
			WBone* pBone = m_pPetFrame->GetBonePet()->FindBone("D_Ball");
			if (pBone)
				pBone->SetInvisible(false, bInvisible);
			pBone = m_pPetFrame->GetBonePet()->FindBone("D_Racket");
			if (pBone)
				pBone->SetInvisible(false, bInvisible);
			break;
		}
		case 1:
		{
			WBone* pBone = m_pPetFrame->GetBonePet()->FindBone("E_Cannon");
			if (pBone)
				pBone->SetInvisible(false, bInvisible);
			pBone = m_pPetFrame->GetBonePet()->FindBone("E_Telescope");
			if (pBone)
				pBone->SetInvisible(false, bInvisible);
			break;
		}
		case 2:
		{
			WBone* pBone = m_pPetFrame->GetBonePet()->FindBone(RB_STR_G_BIRD);
			if (pBone)
				pBone->SetInvisible(false, bInvisible);
			break;
		}
		case 3:
		{
			WBone* pBone = m_pPetFrame->GetBonePet()->FindBone("H_Sword01");
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
		}
		SetPendingTexPart("face", CreateCharacterOption::Instance()->GetChatFacial983(m_ec & 0x3ffffff, 3));
	}

	ProcessMotion(0.0f);
}

void ntAvatar::ProcessMotion(float elapsed)
{
	if (!m_pPetFrame || !m_pPetFrame->GetCurAniPet())
		return;

	if (m_motionType != 0 && m_motionType != 2 && m_motionType != 3 &&
		m_motionType != 1 && m_motionType != 5) // 983 007a7120: o tipo 5 tambem nao volta sozinho ao 0
	{
		if (m_pMotion->e - m_pMotion->s < m_blendElapsed)
			ChangeMotion(0, NULL, 0.2f);
	}

	if (m_pPrevMotion)
	{
		float t = m_blendElapsed / m_blendTime;
		if (t > 1.0f)
		{
			m_pPrevMotion = NULL;
		}
		else
		{
			float time = m_blendElapsed;
			float length = m_pMotion->e - m_pMotion->s;
			if (length < m_blendElapsed)
				time = length;
			WBoneSet blendSet(512); // D475: sem count o m_data ficava vazio (AV em AddBoneKey)
			m_pPetFrame->Animate983(&blendSet, m_pMotion->name, time);
			m_boneSet1.Blend(&m_boneSet2, &blendSet, t);
		}
	}
	else
	{
		float length = m_pMotion->e - m_pMotion->s;
		if (m_blendElapsed > length)
		{
			if (length > 0.01f)
			{
				float loops = floorf(m_blendElapsed / length);
				m_blendElapsed -= loops * length;
			}
			else
			{
				m_blendElapsed = 0.0f;
			}
		}
		m_pPetFrame->Animate983(&m_boneSet1, NULL, m_pMotion->s + m_blendElapsed);
	}

	if (m_blendElapsed == 0.0f)
		ScriptProcess(m_pMotion, m_pMotion->s, m_pMotion->s, true);
	else
		ScriptProcess(m_pMotion, m_pMotion->s + m_blendElapsed,
			m_pMotion->s + m_blendElapsed + elapsed, false);
}

void* ntAvatar::AttachFx(const Attacher& attacher, const char* name, bool bFlag,
	defines_t* defines)
{
	// fxspray.h/fxsequence.h do not expose these fields yet
	struct sSprayAttach
	{
		unsigned long m_0;
		bool m_bFlag;
		const Attacher* m_pAttacher;
		float m_x;
		float m_y;
	};
	struct sSequenceAttach
	{
		unsigned long m_0;
		bool m_bFlag;
		unsigned char m_pad[0xc];
		const Attacher* m_pAttacher;
		float m_x;
		float m_y;
	};

	int len = strlen(name);
	if (len < 5)
		return NULL;
	if (strcmpi(name + len - 3, "spr") == 0)
	{
		CFxSpray* pSpray = m_fxBox.LoadSpray(name, defines);
		if (!pSpray)
			return NULL;
		sSprayAttach* p = (sSprayAttach*)pSpray;
		p->m_bFlag = bFlag;
		p->m_pAttacher = &attacher;
		p->m_x = attacher.x;
		p->m_y = attacher.y;
		m_fxBox.AddSpray(pSpray);
		return pSpray;
	}
	else
	{
		CFxSequence* pSequence = m_fxBox.LoadSequence(name, true, defines);
		if (!pSequence)
			return NULL;
		sSequenceAttach* p = (sSequenceAttach*)pSequence;
		p->m_bFlag = bFlag;
		p->m_pAttacher = &attacher;
		p->m_x = attacher.x;
		p->m_y = attacher.y;
		m_fxBox.AddSequence(pSequence);
		return pSequence;
	}
}

CQuadTree& ntAvatar::GetPVS()
{
	return *m_pPVS;
}

void ntAvatar::SetLocalPVS(CQuadTree& pvs)
{
	m_pPVS = &pvs;
}

static void ClipVelocity(const WVector& in, const WVector& normal, WVector& out,
	float overbounce)
{
	float backoff =
		(in.x * normal.x + in.y * normal.y + in.z * normal.z) * overbounce;
	out = in - normal * backoff;
}

// Moves a box along vel, sliding along whatever it hits (Quake's
// PM_SlideMove). Returns true when the move was blocked or clipped.
bool __fastcall SlideMove(const WVector& vel, const WVector& pos,
	CQuadTree* pPVS, const Waabb& box, WVector& out)
{
	if (Wabs(vel.SquareMagnitude()) < g_EPSILON)
	{
		out = pos;
		return false;
	}

	WPlane planes[5];
	WVector origin = pos;
	WVector velocity = vel;
	int numPlanes = 0;
	int bump;
	for (bump = 0; bump < 4; bump++)
	{
		WVector end = origin + velocity;
		trace_s trace;
		pPVS->TraceAABB(origin, end, box, &trace);
		if (trace.m_flag)
		{
			out = origin;
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
			if (trace.m_normal * planes[i].normal > 0.99f)
			{
				velocity += trace.m_normal;
				break;
			}
		}
		if (i < numPlanes)
			continue;

		planes[numPlanes] =
			*(const WPlane*)&trace
				 .m_normal; // m_normal + m_planeDist are a WPlane
		numPlanes++;

		for (i = 0; i < numPlanes; i++)
		{
			if (velocity * planes[i].normal >= 0.0f)
				continue;

			WVector clipVelocity;
			ClipVelocity(velocity, planes[i].normal, clipVelocity, 1.001f);

			for (int j = 0; j < numPlanes; j++)
			{
				if (j == i)
					continue;
				if (clipVelocity * planes[j].normal >= 0.0f)
					continue;

				ClipVelocity(clipVelocity, planes[j].normal, clipVelocity,
					1.001f);
				if (clipVelocity * planes[i].normal >= 0.0f)
					continue;

				WVector dir;
				dir = WCrossProduct(planes[i].normal, planes[j].normal);
				dir.Normalize();
				clipVelocity = dir * (dir * velocity);

				for (int k = 0; k < numPlanes; k++)
				{
					if (k == i || k == j)
						continue;
					if (clipVelocity * planes[k].normal >= 0.0f)
						continue;
					return true;
				}
			}

			velocity = clipVelocity;
			break;
		}
	}
	return bump > 0;
}

// The rotation that turns the world's up axis onto a ground normal.
void MakeUpRotation(WMatrix& mat, const WVector& up)
{
	if (WisEqual(up, WVector::UNIT_POS_Y, 0.00001f))
	{
		mat.Reset();
	}
	else
	{
		WVector axis;
		axis = WCrossProduct(up, WVector::UNIT_POS_Y);
		axis.Normalize();
		mat = RotMat(axis, (float)acos(up * WVector::UNIT_POS_Y));
	}
}

void ntAvatar::ProcessChangeState()
{
	m_prevState = m_state;
}

void ntAvatar::Process(float elapsed)
{
	if (m_b3d8)
	{
		m_partTids = m_burden.m_tids;
		m_b3d8 = false;
		m_pPetFrame = m_burden.m_pPetFrame;
		m_pPetFrame->CalcBVolume();
		m_pPetFrame->UpdateLightSource(&g_lightset, false, NULL);
		AttachItemFx983(); // 983: 007a7a09

		switch (m_action)
		{
		case 0:
			ChangeMotion(0, NULL, 0.0f);
			break;
		case 1:
			ChangeMotion(2, NULL, 0.0f);
			break;
		case 2:
			ChangeMotion(3, NULL, 0.0f);
			break;
		default:
			ChangeMotion(0, NULL, 0.0f);
			break;
		}
	}

	if (ntAvatarManager::Instance()->m_pMyAvatar == this &&
		(CSharedDoc::Instance()->m_myInfo.info.dwFlagBlock & 8))
		m_headIcon = 3;
	else
		m_headIcon = -1;

	ProcessChangeState();
	ProcessMotion(elapsed);

	if (m_followName[0] != '\0')
	{
		ntAvatar* pTarget =
			ntAvatarManager::Instance()->FindAvatarByNickName(m_followName);
		if (pTarget)
		{
			WVector delta;
			delta = pTarget->m_pos - m_pos;
			float dist = delta.Magnitude();
			if (dist > 64.0f)
			{
				m_followName[0] = '\0';
				Stop();
			}
			else if (dist > 12.8f && dist > 0.0f)
			{
				delta.Normalize();
				delta *= dist - 12.8f;
				SetTargetPosition(m_pos + delta);
				SetAction(0);
			}
		}
		else
		{
			Stop();
			m_moveTarget[0] = '\0';
			m_followName[0] = '\0';
		}
	}

	WVector move;
	move.Reset();
	WVector dir;
	dir = m_targetPos - m_pos;
	if (!(Wabs(dir.x) < g_EPSILON) || !(Wabs(dir.z) < g_EPSILON))
	{
		dir.y = 0.0f;
		dir.Normalize();

		float speed = m_spItems[2].getvalue();
		WVector velocity;
		velocity = dir * 16.5f * speed * elapsed;

		WVector toTarget;
		toTarget =
			WVector(m_targetPos.x - m_pos.x, 0.0f, m_targetPos.z - m_pos.z);
		if (velocity.Magnitude() > toTarget.Magnitude())
			velocity = toTarget;

		WVector newPos;
		if (this == ntAvatarManager::Instance()->m_pMyAvatar)
		{
			Waabb box(WVector(-1.0f, -40.0f, -1.0f), WVector(1.0f, 8.0f, 1.0f));
			SlideMove(velocity, m_pos, m_pPVS, box, newPos);
		}
		else
		{
			newPos = m_pos + velocity;
		}

		move = newPos - m_pos;
		move.y = 0.0f;

		if (move.Magnitude() > 0.01f)
		{
			int hit;
			m_pPVS->GetGroundPoint(newPos, true, &hit, true);
			// 983 007a79b0: so o MEU avatar zera o passo sem chao (hit < 0); os outros andam mesmo assim
			if (hit >= 0 || this != ntAvatarManager::Instance()->m_pMyAvatar)
				m_pos = newPos;
			else
				move.Reset();
		}
		else
		{
			move.Reset();
		}
	}

	SendMove(move, false);

	float moveSq = move.x * move.x + move.z * move.z;
	if (sqrtf(moveSq) > 0.01f)
	{
		if (m_motionType != 1)
			ChangeMotion(1, NULL, 0.2f);
	}
	else
	{
		if (this == ntAvatarManager::Instance()->m_pMyAvatar)
		{
			if (m_moveAccum.x != 0.0f || m_moveAccum.z != 0.0f)
				SendMove(WVector::ZERO, true);
			m_targetPos = m_pos;
		}

		if (m_motionType != 4)
		{
			switch (m_action)
			{
			case 0:
				if (m_motionType != 0)
					ChangeMotion(0, NULL, 0.2f);
				break;
			case 1:
				if (m_motionType != 2)
					ChangeMotion(2, NULL, 0.2f);
				break;
			case 2:
				if (m_motionType != 3)
					ChangeMotion(3, NULL, 0.3f);
				break;
			}
		}
	}

	if (moveSq > 0.001f)
	{
		m_targetAngle = (float)atan2(dir.z, dir.x) + 1.5707964f;
	}
	else if (this != ntAvatarManager::Instance()->m_pMyAvatar)
	{
		if (m_bPendingAngle)
		{
			float angle = m_pendingAngle;
			m_targetAngle = RadianNormalize2PI(angle);
		}
	}

	float turn = RadianDelta(m_targetAngle, m_angle);
	if (turn > 0.0f && m_targetAngle < m_angle)
		m_angle -= g_2_PI;
	else if (turn < 0.0f && m_targetAngle > m_angle)
		m_angle += g_2_PI;

	if (fabs(turn) > 0.001f)
	{
		m_angle += elapsed * turn * 10.0f;
		if ((turn > 0.0f && m_angle > m_targetAngle) ||
			(turn < 0.0f && m_angle < m_targetAngle))
			m_angle = m_targetAngle;
	}

	m_pos = m_pPVS->GetGroundPoint(m_pos, false, NULL, false);

	WVector toCamera;
	toCamera = g_view->camera.pivot - m_pos;
	float cameraDist = sqrtf(toCamera.x * toCamera.x + toCamera.z * toCamera.z);
	if (cameraDist > 192.0f)
	{
		m_alpha = 0.0f;
	}
	else if (cameraDist > 160.0f)
	{
		m_alpha = 1.0f - (cameraDist - 160.0f) * 0.03125f;
	}
	else
	{
		m_alpha = 1.0f;
	}
	// 983 007a79b0: vale tambem na faixa de fade (multiplica); sem registro no mapa nao muda
	if (IsLocalContent((localContentType_t)10))
	{
		CSharedDoc* pDoc = CSharedDoc::Instance();
		int guid = m_guid;
		std::map<unsigned long, sBriefUserInfo>::iterator it =
			pDoc->m_briefUserInfoMap.find(guid);
		if (it != pDoc->m_briefUserInfoMap.end() &&
			(it->second.dwIdentity & 4) && !(it->second.state & 1))
			m_alpha *= 0.1f;
	}

	WVector up;
	if (m_motionType == 3)
		up = m_pPVS->GetMeshNormal(m_pos);
	else
		up = WVector::UNIT_POS_Y;

	WVector axis = WCrossProduct(m_up, up);
	axis.Normalize();

	if (Wabs(axis.SquareMagnitude()) >= g_EPSILON)
	{
		if (acos(up * m_up) < elapsed * g_2_PI)
		{
			m_up = up;
		}
		else
		{
			WMatrix rot = RotMat(axis, elapsed * -6.2831855f);
			m_up = RotVec(m_up, rot);
		}
	}

	WMatrix mat;
	MakeUpRotation(mat, m_up);
	mat = RotMat(m_angle, 1) * mat;

	float scale = m_spItems[0].getvalue();
	mat.pivot = m_pos;
	WVector axisScale(scale, scale, scale);
	mat.AxisScale(axisScale);

	if (m_pPetFrame)
	{
		m_pPetFrame->GetBonePet()->SetMatrix(mat);
		m_pPetFrame->ApplyBoneSet983(&m_boneSet1);
		m_pPetFrame->Transform(g_view, 1.0f, 0);
		m_pPetFrame->UpdateBound(true);
		m_pPetFrame->SetAlpha((unsigned char)(m_alpha * 255.0f));
		m_pPetFrame->GetBonePet()->ScaleBone("Bip01 Head",
			m_spItems[1].getvalue());
	}

	if (m_facialTime > 0.0f)
	{
		m_facialTime -= elapsed;
		if (m_facialTime < 0.0f)
		{
			m_facialTime = 0.0f;
			SetPendingTexPart("face", CreateCharacterOption::Instance()->GetChatFacial983(m_ec & 0x3ffffff, 3));
		}
	}

	m_718 += elapsed;
	if (m_718 > 5.0f)
	{
		ntBalloonTextManager::Instance()->DeleteBalloonText(m_pBalloon);
		m_pBalloon = NULL;
	}

	m_fxBox.ProcessAttacher(elapsed * 1000.0f);
	m_fxBox.Process(elapsed * 1000.0f);

	m_blendElapsed += elapsed;
	m_6f8 += elapsed;
}

void ntAvatar::Render()
{
	if (IsLocalContent((localContentType_t)10))
	{
		if (this != ntAvatarManager::Instance()->m_pMyAvatar && m_e8)
		{
			CSharedDoc* pDoc = CSharedDoc::Instance();
			int guid = m_guid;
			const sBriefUserInfo& info =
				(*pDoc->m_briefUserInfoMap.find(guid)).second;
			if (!(sBriefUserInfo(info).state & 1))
			{
				if (guid != CSharedDoc::Instance()->m_myInfo.info.dwGuid)
					return;
			}
		}
	}

	WVector pos;
	if (!m_pPetFrame || !m_pPetFrame->GetCurAniPet())
	{
		pos = m_pPVS->GetGroundPoint(m_pos, false, NULL, false);
	}
	else
	{
		pos = m_pPetFrame->GetBonePet()
				  ->FindBone("Bip01 Pelvis")
				  ->GetMatrix()
				  .pivot;
		pos = m_pPVS->GetGroundPoint(pos, false, NULL, false);
	}

	if (COption::Instance()->vIsShadowEnabled())
	{
		WVector xAxis = WVector(2.9f, 0.0f, 0.0f) * m_spItems[0].getvalue();
		WVector zAxis = WVector(0.0f, 0.0f, 2.9f) * m_spItems[0].getvalue();
		m_pPVS->DrawDecal(m_pChatMain->m_shadowTexture, pos, xAxis, zAxis, 1.0f,
			m_alpha, W_POLY_BLEND_NORMAL, false);
	}

	if (m_pPetFrame && m_pPetFrame->GetCurAniPet())
	{
		if (g_view->InFrustum(m_pPetFrame->GetBonePet()->GetBoundBox()))
		{
			if ((ntAvatar*)ntAvatarManager::Instance()->m_a8 == this)
			{
				// 983 007a5110: o avatar destacado ganha +20 por canal (alfa ff)
				m_light.nearOne = g_lightset.nearOne;
				m_light.ambient = BrightenColor983(g_lightset.ambient, 1.0f);
				m_light.diffuse = BrightenColor983(g_lightset.diffuse,
					m_pPVS->GetShadowIntensity(pos));
			}
			else
			{
				m_light.nearOne = g_lightset.nearOne;
				m_light.ambient = ModifyColor(g_lightset.ambient, 1.0f);
				m_light.diffuse = ModifyColor(g_lightset.diffuse,
					m_pPVS->GetShadowIntensity(pos));
			}
			m_pPetFrame->UpdateLightSource(&m_light, false, NULL);
			m_pPetFrame->Render(g_view, true, 1.0f, false, false, false);
		}
	}

	// 983 007a5110 nao chama Flush antes do Display da caixa de efeitos
	m_fxBox.Display(g_view);
}

void ntAvatar::RenderOverlay(bool bDetail)
{
	if (m_pBalloon)
		m_pBalloon->SetTransparency(m_alpha);

	if (GOLFDOC()->m_gameMode != 0x200)
	{
		if (Wabs(m_alpha) < g_EPSILON)
			return;
	}
	float alpha = GOLFDOC()->m_gameMode == 0x200 ? 1.0f : m_alpha;

	if (!m_pPetFrame || !m_pPetFrame->GetCurAniPet())
		return;
	if (!g_view->InFrustum(m_pPetFrame->GetBonePet()->m_bound))
		return;

	WVector head;
	head =
		m_pPetFrame->GetBonePet()->FindBone("Bip01 HeadNub")->GetMatrix().pivot;
	WVector screen = g_view->Projection(head);
	if (screen.z <= 0.044f || screen.z >= 1.0f)
		return;

	screen.x = floorf(screen.x);
	screen.y = floorf(screen.y);

	if (IsLocalContent((localContentType_t)10))
	{
		int guid = m_guid;
		if (guid != CSharedDoc::Instance()->m_myInfo.info.dwGuid)
		{
			std::map<unsigned long, sBriefUserInfo>::iterator it =
				CSharedDoc::Instance()->m_briefUserInfoMap.find(guid);
			if ((bool)(((*it).second.dwIdentity >> 2) & 1) &&
				!((*it).second.state & 1))
				return;
		}
	}
	if (CProjectG::Instance()->HidePrivacy())
		return;

	FrEmoticon* pEmoticon = g_pFresh->GetManager()->GetEmoticon();
	float y = screen.y - 4.0f - 22.0f;
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	// gender icon, guild emblem and nick name in one centered row
	WOverlay* pIcon;
	int gender = m_e9;
	if (m_eb)
		pIcon = m_pChatMain->m_angelIcons[gender % 2];
	else if (m_ea)
		pIcon = m_pChatMain->m_mannerIcons[gender % 2];
	else
		pIcon = m_pChatMain->m_genderIcons[gender];

	unsigned int emblemWidth = m_pEmblem ? m_pEmblem->bi->bmiHeader.biWidth : 0;
	int iconWidth = pIcon->GetWidth();
	float x = screen.x -
		(pEmoticon->GetTextWidth(m_nickName) + iconWidth + emblemWidth) * 0.5f;

	WRect src(0.0f, 0.0f, 1.0f, 1.0f);
	WRect dst;
	dst = WRect(x, y - 7.0f, pIcon->GetWidth(), pIcon->GetHeight());
	x += pIcon->GetWidth();

	unsigned long shadow = (unsigned char)(alpha * 255.0f) << 24;
	unsigned long color = shadow | 0xffffff;
	pIcon->Render(g_view, src, dst, 0, color, 0.0f, 0);

	if (m_pEmblem)
	{
		float width = (float)(unsigned int)m_pEmblem->bi->bmiHeader.biWidth;
		float height = (float)(unsigned int)m_pEmblem->bi->bmiHeader.biHeight;
		dst = WRect(0.5f / width + x, 0.5f / height + y - 5.0f, width, height);
		pGDI->DrawTexture(m_pEmblem, dst, color, 0);
		x += (unsigned int)m_pEmblem->bi->bmiHeader.biWidth;
	}

	pGDI->SetScale(1.0f);
	pGDI->SetTextStyle(2);
	if ((ntAvatar*)ntAvatarManager::Instance()->m_a8 == this)
		pGDI->SetTextColor(shadow, color);
	else
		pGDI->SetTextColor(color, shadow);

	pEmoticon->SetAnim(false);
	pEmoticon->PrintText(WPoint(x, y), 0, m_nickName, color);

	if (m_headIcon != -1)
	{
		// 4x4 grid of head icons
		WRect iconSrc((m_headIcon % 4) * 0.25f, (m_headIcon % 16 / 4) * 0.25f,
			0.25f, 0.25f);
		WRect iconDst(screen.x - 64.0f - 15.0f, screen.y - 64.0f + 20.0f, 64.0f,
			64.0f);
		m_pChatMain->m_emotiObject->Render(g_view, iconSrc, iconDst, 0,
			((unsigned char)(m_alpha * 255.0f) << 24) | 0xffffff, 0.0f, 0);
	}

	if (!bDetail)
		return;

	unsigned long state = m_state;
	if (!(state & 0xfe))
		return;

	WOverlay* pStateIcon;
	if (state & 0x80)
	{
		// private shop: title frame above the name
		y -= 22.0f;
		int width = (int)pGDI->GetTextExtendOld983(m_tradeTitle) + 10;
		if (width < 100)
			width = 100;

		RECT rect;
		rect.left = (int)(screen.x - width / 2);
		rect.top = (int)(y - 8.0f);
		rect.right = width;
		rect.bottom = 29;
		WOverlay::DrawFrameOverlay9(g_view, m_pChatMain->m_stateBars, rect,
			(ntAvatar*)ntAvatarManager::Instance()->m_ac == this ? 0xff808080
																 : 0xffffffff);

		unsigned long oldColor = pGDI->GetTextColor();
		pGDI->SetScale(1.0f);
		pGDI->SetTextStyle(
			(ntAvatar*)ntAvatarManager::Instance()->m_ac == this ? 2 : 0);
		unsigned long titleShadow = (unsigned char)(m_alpha * 255.0f) << 24;
		pGDI->SetTextColor(titleShadow | 0xffffff, titleShadow);
		pEmoticon->SetAnim(true);
		pEmoticon->PrintText(WPoint(screen.x, y), 1, m_tradeTitle,
			((unsigned char)(m_alpha * 255.0f) << 24) | 0xffffff);
		pGDI->SetTextColor(oldColor, 0xffffffff);

		pStateIcon = m_pChatMain->m_stateIcons[7];
	}
	else
	{
		if (state & 0x8000)
			return;
		if (!(state & 0x7e))
			return;

		if (state & 2)
			pStateIcon = m_pChatMain->m_stateIcons[1];
		else if (state & 4)
			pStateIcon = m_pChatMain->m_stateIcons[2];
		else if (state & 8)
			pStateIcon = m_pChatMain->m_stateIcons[3];
		else if (state & 0x10)
			pStateIcon = m_pChatMain->m_stateIcons[4];
		else if (state & 0x20)
			pStateIcon = m_pChatMain->m_stateIcons[5];
		else if (state & 0x40)
			pStateIcon = m_pChatMain->m_stateIcons[6];
		else
			return;
	}

	if (pStateIcon)
	{
		int width = pStateIcon->GetWidth();
		int height = pStateIcon->GetHeight();
		WRect stateDst(screen.x - width / 2, y - height / 2 - 36.0f, width,
			height);
		WRect stateSrc(0.5f / pStateIcon->GetWidth(),
			0.5f / pStateIcon->GetHeight(), 1.0f, 1.0f);
		pStateIcon->Render(g_view, stateSrc, stateDst, 0,
			((unsigned char)(m_alpha * 255.0f) << 24) | 0xffffff, 0.0f, 0);
	}
}

// 983 0047e4f0: so 1x CPetFrame::SetPart 008b0960 (as texturas UCC)
void ntAvatar::RefreshUccClothes(const sUccItem* pUcc)
{
	if (!m_pPetFrame)
		return;
	m_pPetFrame->SetPart983(pUcc);
}

void ntAvatar::ScriptProcess(const w_motion_data* data, float from, float to,
	bool bAll)
{
	if ((!(from <= data->e) || from == to) && !bAll)
		return;

	int start, end;
	from *= 30.0f;
	if (bAll)
	{
		start = Ceil(from, 0.03f);
		end = start;
	}
	else
	{
		float limit = data->e;
		if (limit > to)
			limit = to;
		to = limit * 30.0f;
		start = Ceil(from, 0.03f);
		end = Wabs((float)floor(to) - to) < g_EPSILON ? (int)to - 1
													  : (int)floor(to);

		if (start == (int)floor(data->s * 30.0f + 0.5f))
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
				WPuppet& pet = *pPuppet;
				for (int i = first; i <= last; i++)
					ParsePuppetScript(pet.GetFrameData(i));
			}
		}
	}
}

void ntAvatar::ParsePuppetScript(const char* script)
{
	static char s_name[0x40];
	static char s_command[0x400];

	while (script)
	{
		script = strchr(script, '*');
		if (!script)
			return;

		int len;
		for (len = 0; script[len];)
		{
			++len;
			if (script[len] == ' ')
				break;
			if (script[len] == '(')
			{
				++len;
				break;
			}
		}
		if (len <= 1)
			return;

		strncpy(s_command, script, len);
		script += len;
		s_command[len] = '\0';

		if (!strcmp(s_command, "*fx("))
		{
			char name[0x40];
			char fxName[0x40];
			char boneArg[0x40];
			char bone[0x40];

			name[0] = '\0';
			script = GetStrConst(script, name);
			if (strlen(name) == 0 || strlen(name) >= sizeof(name))
				return;

			if (name[0] == '@')
			{
				// "@motion": only while that motion is playing
				if (!m_pMotion)
					return;
				if (strcmp(name + 1, m_pMotion->name) != 0)
					return;
				script = GetStrConst(script, fxName);
				if (strlen(fxName) == 0 || strlen(fxName) >= sizeof(fxName))
					return;
			}
			else
			{
				strcpy(fxName, name);
			}

			script = GetStrConst(script, boneArg);
			int boneLen = (int)strlen(boneArg);
			if (boneLen == 0 || boneLen >= (int)sizeof(boneArg))
				return;
			strcpy(bone, boneArg);
			if (strlen(bone) == 0 || strlen(bone) >= sizeof(bone))
				return;

			Attacher attacher;
			int i;
			for (i = 0; i < 4; ++i)
			{
				if (i != 0)
				{
					script = GetStrConst(script, bone);
					if (strlen(bone) == 0 || strlen(bone) >= sizeof(bone))
						break;
				}

				WBone* pBone = m_pPetFrame->GetBonePet()->FindBone(bone);
				if (pBone)
				{
					attacher.mat[i] = &pBone->GetMatrix();
				}
				else
				{
					WPuppet* pPuppet = m_pPetFrame->GetBonePet();
					w_bound_box* pBox = pPuppet->m_bbList.Find(bone);
					if (pBox)
					{
						if (pBox->bone)
							attacher.mat[i] = &pBox->bone->GetMatrix();
						attacher.pos[i] = &pBox->info->spherePivot;
					}
				}
			}
			if (i > 0)
				AttachFx(attacher, fxName, true, NULL);
		}
		else if (!strcmp(s_command, "*hidebone(") ||
			!strcmp(s_command, "*showbone("))
		{
			char bone[0x40] = "";
			int count = 0;
			do
			{
				script = GetStrConst(script, bone);
				if (strlen(bone) == 0 || strlen(bone) >= sizeof(bone))
					break;
				WBone* pBone = m_pPetFrame->GetBonePet()->FindBone(bone);
				if (pBone)
					pBone->SetInvisible(false, s_command[1] == 'h');
				if (!script)
					return;
				if (*script == ')')
					break;
			} while (++count < 64);
		}
		else if (!strcmp(s_command, "*snd("))
		{
			script = GetStrConst(script, s_name);
			if (strlen(s_name) == 0 || strlen(s_name) >= sizeof(s_name))
				return;

			WVector delta = m_pos - g_camera.pivot;
			if (delta.Magnitude() < 192.0f)
				g_audio->PlaySfx(s_name, &delta, 5, NULL, NULL, 0.5f, 200.0f);
		}
		else if (!strcmp(s_command, "*stepsnd("))
		{
			if (IsLocalContent((localContentType_t)10))
			{
				if (this != ntAvatarManager::Instance()->m_pMyAvatar)
				{
					int guid = m_guid;
					const sBriefUserInfo& info =
						(*CSharedDoc::Instance()->m_briefUserInfoMap.find(guid))
							.second;
					if (m_e8)
					{
						// hidden users make no noise
						if (!(sBriefUserInfo(info).state & 1))
							continue;
					}
				}
				CQuadTree& pvs = *m_pPVS;
				const char* sound = pvs.GetStepSound(
					pvs.GetGroundPoint(m_pos, false, NULL, false));
				if (sound)
				{
					WVector delta = m_pos - g_camera.pivot;
					if (delta.Magnitude() < 192.0f)
						g_audio->PlaySfx(sound, &delta, 5, sound, NULL, 0.5f,
							200.0f);
				}
			}
			else
			{
				CQuadTree& pvs = *m_pPVS;
				const char* sound = pvs.GetStepSound(
					pvs.GetGroundPoint(m_pos, false, NULL, false));
				if (sound)
				{
					WVector delta = m_pos - g_camera.pivot;
					if (delta.Magnitude() < 192.0f)
						g_audio->PlaySfx(sound, &delta, 5, sound, NULL, 0.5f,
							200.0f);
				}
			}
		}
		else if (!strcmp(s_command, "*ptex("))
		{
			char part[0x40];
			char texture[0x40];

			part[0] = '\0';
			script = GetStrConst(script, part);
			if (strlen(part) == 0 || strlen(part) >= sizeof(part))
				return;

			if (part[0] == '@')
			{
				if (!m_pMotion)
					return;
				if (strcmpi(part + 1, m_pMotion->name) != 0)
					return;
				script = GetStrConst(script, s_name);
				if (strlen(s_name) == 0 || strlen(s_name) >= sizeof(s_name))
					return;
			}
			else
			{
				strcpy(s_name, part);
			}

			script = GetStrConst(script, texture);
			if (strlen(texture) == 0 || strlen(texture) >= sizeof(texture))
				return;

			// a chat facial is still being shown
			if (m_facialTime > 0.0f)
				return;
			SetPendingTexPart(s_name, texture);
		}
		else if (!stricmp(s_command, "*showvis("))
		{
			ParseScript_ShowVis(true, script);
		}
		else if (!stricmp(s_command, "*hidevis("))
		{
			ParseScript_ShowVis(false, script);
		}
	}
}

void ntAvatar::ParseScript_ShowVis(bool bShow, const char* script)
{
	char token[0x100] = "";
	int count = 0;
	do
	{
		script = GetStrConst(script, token);
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
		if (!script || *script == ')')
			break;
	} while (++count < 64);
}

sTexturePart* ntAvatar::FindTexturePart(const char* part, const char* texture)
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

void ntAvatar::SetPendingTexPart(const char* part, const char* texture)
{
	if (!m_pPetFrame || !m_pPetFrame->GetCurAniPet())
		return;
	sTexturePart* pTexPart = FindTexturePart(part, texture);
	if (!pTexPart->pBitmap)
		m_pPetFrame->GetBonePet()->ChangeTexturePart(m_pPetFrame->GetFaceTex(),
			pTexPart->texture, &pTexPart->pBitmap, &pTexPart->rect);
	else
		m_pPetFrame->GetBonePet()->ChangeTexturePart(m_pPetFrame->GetFaceTex(),
			pTexPart->pBitmap, pTexPart->rect);
	m_pPetFrame->SetMakeup();
}

void ntAvatar::SetChatFacial(int facial)
{
	SetPendingTexPart("face", CreateCharacterOption::Instance()->GetChatFacial983(m_ec & 0x3ffffff, facial));
	m_facialTime = 5.0f;
}

void ntAvatar::SetBalloonText(const char* text)
{
	if (!m_pBalloon)
		m_pBalloon = ntBalloonTextManager::Instance()->CreateBalloonText(m_guid,
			text, m_balloonRand, 5.0f);
	else
		m_pBalloon->SetText(text);
	m_714 = m_alpha;
	m_718 = 0.0f;
}

void ntAvatar::SetTradeTitle(const char* title)
{
	if (title)
		strcpy(m_tradeTitle, title);
	else
		m_tradeTitle[0] = 0;
}

SpecialPrizeItem& ntAvatar::GetMySpItem(eSPAVILITYTYPE type)
{
	return m_spItems[(int)type];
}

void ntAvatar::UseSPItem(eSPAVILITYTYPE type)
{
	WSendPacket packet(12);
	packet.Encode1(6);
	packet.Encode4(ntAvatarManager::Instance()->m_pMyAvatar->GetGuid());
	packet.Encode4(type);
	packet.Send(TO_GAME);
}

ntAvatarManager::ntAvatarManager()
{
	m_pPVS = NULL;
	m_pPVS = &::GetPVS();
	m_count = 0;
	for (int i = 0; i < 30; ++i)
		m_avatars[i] = NULL;
	m_pMyAvatar = NULL;
	m_a8 = 0;
	m_ac = 0;
	m_bFlag0 = true;
	m_bFlag1 = true;
}

ntAvatarManager::~ntAvatarManager()
{
	FreeAllAvatars();
}

void ntAvatarManager::FreeAllAvatars()
{
	for (int i = 0; i < 30; ++i)
	{
		if (m_avatars[i])
		{
			if (m_avatars[i]->m_pPetFrame)
			{
				delete m_avatars[i]->m_pPetFrame;
				m_avatars[i]->m_pPetFrame = NULL;
			}
			if (m_avatars[i])
			{
				delete m_avatars[i];
				m_avatars[i] = NULL;
			}
		}
	}
}

// 983 007a4e70 (so chamado pelo ToggleTopViewMode 007aa250): fx das pecas de todos os avatares no modo 0x200 (vista de cima) ou 8
void ntAvatarManager::SetFxMode983(bool bTopView)
{
	int mode = bTopView ? 0x200 : 8;
	for (int i = 0; i < 30; ++i)
	{
		if (m_avatars[i])
			m_avatars[i]->m_spFx983->SetMode(mode);
	}
}

ntAvatar* ntAvatarManager::FindAvatarByIDName(const char* name) const
{
	for (int i = 0; i < 30; ++i)
	{
		if (m_avatars[i] && strcmp(m_avatars[i]->m_idName, name) == 0)
			return m_avatars[i];
	}
	return NULL;
}

ntAvatar* ntAvatarManager::FindAvatarByNickName(const char* name) const
{
	for (int i = 0; i < 30; ++i)
	{
		if (m_avatars[i] && strcmp(m_avatars[i]->m_nickName, name) == 0)
			return m_avatars[i];
	}
	return NULL;
}

ntAvatar* ntAvatarManager::FindAvatarByGuid(int guid) const
{
	for (int i = 0; i < 30; ++i)
	{
		if (m_avatars[i] && m_avatars[i]->m_guid == guid)
			return m_avatars[i];
	}
	return NULL;
}

ntAvatar* ntAvatarManager::CreateAvatar(const sSlotInfo& slot,
	const sCharacterInfo& info, ntAvatarChatMain* pChatMain)
{
	int i;
	for (i = 0; i < 30; ++i)
	{
		if (!m_avatars[i])
			break;
	}
	if (i == 30)
		return NULL;
	++m_count;
	ntAvatar* pAvatar = new ntAvatar(*m_pPVS, slot, info, pChatMain);
	WVector pos(slot.location[0], 0.0f, slot.location[1]);
	pos = m_pPVS->GetGroundPoint(pos, false, NULL, false);
	pAvatar->SetPosition(pos);
	pAvatar->SetAngle(slot.location[2]);
	m_avatars[i] = pAvatar;
	return pAvatar;
}

bool ntAvatarManager::DeleteAvatar(const char* name)
{
	int i;
	for (i = 0; i < 30; ++i)
	{
		if (m_avatars[i] && strcmp(m_avatars[i]->m_idName, name) == 0)
			break;
	}
	if (i == 30)
		return false;
	if (m_avatars[i])
	{
		delete m_avatars[i];
		m_avatars[i] = NULL;
	}
	--m_count;
	return true;
}

ntAvatar* ntAvatarManager::AvatarByRayIntersect(const WVector& pos,
	const WVector& dir) const
{
	Waabb aabb;
	for (int i = 0; i < 30; i++)
	{
		if (m_avatars[i])
		{
			if (IsLocalContent((localContentType_t)10))
			{
				if (m_avatars[i]->m_guid !=
					CSharedDoc::Instance()->m_myInfo.info.dwGuid)
				{
					CSharedDoc* pDoc = CSharedDoc::Instance();
					std::map<unsigned long, sBriefUserInfo>::iterator it =
						pDoc->m_briefUserInfoMap.find(m_avatars[i]->m_guid);
					if (!IsAdministrator(0xffffffff))
					{
						if (sBriefUserInfo((*it).second).dwIdentity & 4)
						{
							if (!(sBriefUserInfo((*it).second).state & 1))
								break;
						}
					}
				}
			}

			aabb.min = WVector(-1.0f, 0.0f, -1.0f);
			aabb.max = WVector(1.0f, 8.0f, 1.0f);
			aabb.min += m_avatars[i]->m_pos;
			aabb.max += m_avatars[i]->m_pos;

			if (RayIntersectAABB(aabb, pos, dir, NULL, 0.0f) < 1.0f)
				return m_avatars[i];
		}
	}
	return NULL;
}

ntAvatar* ntAvatarManager::CheckClick(float x, float y) const
{
	for (int i = 0; i < 30; ++i)
	{
		if (m_avatars[i] && m_avatars[i]->IsClickable(x, y))
			return m_avatars[i];
	}
	return NULL;
}

void ntAvatarManager::Process(float elapsed)
{
	for (int i = 0; i < 30; ++i)
	{
		if (m_avatars[i])
			m_avatars[i]->Process(elapsed);
	}
}

void ntAvatarManager::Render()
{
	if (!m_bFlag0)
		return;
	for (int i = 0; i < 30; ++i)
	{
		if (m_avatars[i])
			m_avatars[i]->Render();
	}
}

void ntAvatarManager::ResetAvatarPositionAngle()
{
	for (int i = 0; i < 30; ++i)
	{
		if (m_avatars[i])
		{
			m_avatars[i]->m_pos = m_avatars[i]->m_targetPos;
			m_avatars[i]->m_angle = m_avatars[i]->m_targetAngle;
		}
	}
}

CQuadTree& ntAvatarManager::GetPVS()
{
	return *m_pPVS;
}

void ntAvatarManager::SetLocalPVS(CQuadTree& pvs)
{
	m_pPVS = &pvs;
}

static int __cdecl CompareCameraDistance(const void* a, const void* b)
{
	WVector delta = g_view->GetCamera().pivot;
	WVector deltaA;
	deltaA = delta - (*(ntAvatar* const*)a)->GetPosition();
	delta = delta - (*(ntAvatar* const*)b)->GetPosition();
	float distA =
		deltaA.x * deltaA.x + deltaA.y * deltaA.y + deltaA.z * deltaA.z;
	float distB = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
	if (distA > distB)
		return -1;
	if (distA < distB)
		return 1;
	return 0;
}

void ntAvatarManager::RenderOverlay()
{
	if (!m_bFlag0)
		return;
	ntAvatar* avatars[30];
	int count = 0;
	for (int i = 0; i < 30; ++i)
	{
		if (m_avatars[i])
			avatars[count++] = m_avatars[i];
	}
	qsort(avatars, count, sizeof(ntAvatar*), CompareCameraDistance);
	for (int j = 0; j < count; ++j)
	{
		if (avatars[j])
			avatars[j]->RenderOverlay(m_bFlag1);
	}
}

ntAvatarBurden::ntAvatarBurden(unsigned long guid, unsigned long face,
	const sCharacterInfo& info, CPetFrame* pPetFrame)
{
	m_guid = guid;
	m_face = face;
	m_info = info;
	m_pPetFrame = pPetFrame;
}

ntAvatarBurden::~ntAvatarBurden()
{
}

bool ntAvatarBurden::Load()
{
	if (!m_tids.SetTids(&m_info, 0xff))
		m_tids.SetDefaultTids();
	if (!m_tids.IsComboValid())
		m_tids.SetDefaultTids();
	// 983 007a3410: apaga o frame velho antes de montar o novo
	if (m_pPetFrame)
		delete m_pPetFrame;
	m_pPetFrame = new CPetFrame;
	if (!m_pPetFrame->Build(&m_tids, m_info.tidAuxParts, true, true, m_face))
	{
		delete m_pPetFrame;
		m_pPetFrame = NULL;
		return false;
	}
	m_pPetFrame->SetHairClr983(&m_tids, m_info.hairClr, 0, true); // 983 007a3512
	m_pPetFrame->SetShirtsClr(&m_tids, m_info.shirtsClr); // 983 007a3524: byte inteiro (o & 0xf era do bitfield do 645)
	return true;
}

// shareddoc.inl in the original (a late PCH inline): compiled after the unit's
// own functions
inline int IsAdministrator(unsigned long identity)
{
	if (identity == 0xffffffff)
		identity = CSharedDoc::Instance()->m_myInfo.info.dwIdentity;
	return (identity & 4) == 4;
}
