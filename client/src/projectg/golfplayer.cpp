#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "partsinfo.h"
#include "golfplayer.h"
#include "quadtree.h"
#include "scenemanager.h"
#include "taileffect.h"
#include "commonutil.h"
#include "golfball.h"
#include "wbone.h"
#include "wpuppet.h"
#include "inputmanager.h"
#include "fx.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "uccmanager.h"
// HACK: emission order. These inlines come right before CClub::TypeUpdated and one compile wave late in the
// original (header order golfdoc.h, uccmanager.h, wbone.h, club.h); the dead float2int reference (defined at the
// end of this file) makes them wait that wave.
inline int float2int(float value);
inline eTeamName CGolfDoc::GetTeamGameWinner()
{
	if (0)
	{
		float2int(0.0f);
	}
	// 983 (HandleMsg 0x7462d2/0x746573): [CGolfDoc+0xd0], o mesmo campo do SetTeamGameWinner; o +0xc0 cru do 645 lia outro
	return *(eTeamName*)&m_teamNum;
}

inline sUccRefreshItem::sUccRefreshItem()
{
	if (0)
	{
		float2int(0.0f);
	}
	typeId = 0;
	memset(uccIndex, 0, sizeof(uccIndex));
}

inline bool WBone::IsInvisible()
{
	return m_flag.GetFlag(INVISIBLE);
}

#include "club.h"
#include "texturepart.h"
WVector __fastcall GetPinPos();
#include "slicestr.h"
#include "frbutton.h"
#include "packet.h"
#include "quitwindow.h"
#include "itemwindow.h"
#include "soundmanager.h"
#include "headicon.h"
#include "networksystem.h"
#include "voiceitem.h"
#include "jp/conditionalauxpartobject.h"
#include <algorithm>
#include "../shared/localize.h"
extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // B1422 temporario
extern bool g_rastroNoPlay983; // B1504 temporario (quadtree.cpp)

#define RB_REAL_96 96.0f
#define RB_MUL_4PI(x) ((x) * 12.566371f)

extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_ReadWriteBarrier)

extern WMatrix g_camera;

// HACK: references the remaining wmath.h constant so the .rdata constant
// table has the same layout as the original
static float rb_UnusedConstants()
{
	return g_HUGE;
}

unsigned long __fastcall ModifyColor(unsigned long color, float factor);

int Ceil(float value, float tolerance);
const char* GetStrConst(const char* str, char* out);
const char* GetCharsTillSpace(const char* str, char* out);
bool IsMyTurn(bool bCheck);
inline unsigned char MyIndex();

// CSceneManager::SetPuppet: scenemanager.h is in the PCH, so the name is added through a helper class
class CSceneManagerEx : public CSceneManager
{
public:
	void SetPuppet(const char* name, WPuppet* puppet);
};
#pragma comment(linker, \
	"/alternatename:?SetPuppet@CSceneManagerEx@@QAEXPBDPAVWPuppet@@@Z=?SetPuppet@CSceneManager@@QAEXPBDPAVWPuppet@@@Z")

inline int float2int(float value);

// CGolfDoc fields that golfdoc.h (in the PCH) does not name yet
// HACK: .rdata order. The original emits these three literals in the slot of Shot_PowerShot, which is naked asm
// here, so they are local statics of that function and the later C users name those copies.
#define RB_STR_POWER_READY "%s\xc6\xc4\xbf\xf6\xc1\xd8\xba\xf1"
#define RB_STR_POWER2_READY \
	"%s\xc6\xc4\xbf\xf6" \
	"2\xc1\xd8\xba\xf1"
#define RB_STR_POWERSHOT_READY "powershot_ready"
#define RB_STR_CLUB "club"

// CGolfDoc::m_bar.grade (983 +0xf0)
#define GetBarAccuracyGrade() (GOLFDOC()->m_bar.grade) // 983: m_bar.grade (+0xf0)

// WPolySoup::FindPoint: polysoup.h is in the PCH, so the name is added through a helper class
struct sSoupPoint
{
	char m_unused0[0x24];
	WVector pos; // 0x24
};

class WPolySoupEx : public WPolySoup
{
public:
	sSoupPoint* FindPoint(const char* name);
};
#pragma comment(linker, \
	"/alternatename:?FindPoint@WPolySoupEx@@QAEPAUsSoupPoint@@PBD@Z=?FindPoint@WPolySoup@@QAEPAUsPoint@1@PBD@Z")

bool IsCurMapSnow();

// helpmsg.h does not exist yet
class CHelpMsg : public WSingleton<CHelpMsg>
{
public:
	void SetHelpMsg(int id, char* text);
};

struct sTailTexture
{
	unsigned long tid;
	const char* texture;
};

static sTailTexture s_tailTexture[] = {
	{ 0x10000009, "~swinglight02.jpg"      },
	{ 0x1000000c, "~swinglight01_pink.jpg" },
	{ 0x1000000a, "~swinglight01_blue.jpg" },
};

static const char* s_motionName[] = {
	"\xbf\xec\xb5\xe5\xbc\xa6",
	"\xbf\xec\xb5\xe5\xbc\xa6\xc6\xc4\xbf\xf6",
	"\xbf\xec\xb5\xe5\xbc\xa6\xba\xfc\xb8\xa5\xc7\xea\xbd\xba\xc0\xae",
	"\xbf\xec\xb5\xe5\xbc\xa6\xb4\xca\xc0\xba\xc7\xea\xbd\xba\xc0\xae",
	"\xbf\xec\xb5\xe5\xbc\xa6\xc6\xc4\xbf\xf6\xba\xfc\xb8\xa5\xc7\xea\xbd\xba\xc0\xae",
	"\xbf\xec\xb5\xe5\xbc\xa6\xc6\xc4\xbf\xf6\xb4\xca\xc0\xba\xc7\xea\xbd\xba\xc0\xae",
	"\xbe\xc6\xc0\xcc\xbe\xf0\xbc\xa6",
	"\xbe\xc6\xc0\xcc\xbe\xf0\xbc\xa6\xc6\xc4\xbf\xf6",
	"\xbe\xc6\xc0\xcc\xbe\xf0\xbc\xa6\xba\xfc\xb8\xa5\xc7\xea\xbd\xba\xc0\xae",
	"\xbe\xc6\xc0\xcc\xbe\xf0\xbc\xa6\xb4\xca\xc0\xba\xc7\xea\xbd\xba\xc0\xae",
	"\xbe\xc6\xc0\xcc\xbe\xf0\xbc\xa6\xc6\xc4\xbf\xf6\xba\xfc\xb8\xa5\xc7\xea\xbd\xba\xc0\xae",
	"\xbe\xc6\xc0\xcc\xbe\xf0\xbc\xa6\xc6\xc4\xbf\xf6\xb4\xca\xc0\xba\xc7\xea\xbd\xba\xc0\xae",
	"\xbb\xf7\xb5\xe5\x9f\x87\xc1\xf6\xbc\xa6",
	"\xbb\xf7\xb5\xe5\x9f\x87\xc1\xf6\xbc\xa6\xc6\xc4\xbf\xf6",
	"\xc6\xdb\xc6\xc3\xbc\xa6",
	"\xc2\xaa\xc0\xba\xc6\xdb\xc6\xc3",
	"\xb3\xa1",
};

// statically initialised, so these cannot be WMatrix objects (its constructors would make the init dynamic)
extern "C"
{
	float convMat[12] = { 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f };
	float convMatInv[12] = { 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f };
}

IMPLEMENT_ACTOR(CGolfPlayer, IActor)

inline unsigned char MyIndex()
{
	bool bGM = (Doc()->m_myInfo.info.dwIdentity >> 1) & 1;
	if (bGM)
	{
		switch (Doc()->m_golfGame.gameType)
		{
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
		case 0xb: // 983 0053a260 (MyIndex) inclui 0xb
			return Doc()->GetIndex(Doc()->m_myInfo.info.dwGalleryGuid);
		default:
			return GOLFDOC()->m_currentPlayer;
		}
	}
	return Doc()->GetIndex(Doc()->m_myInfo.info.dwGuid);
}

// HACK: emits no code. c2 picks the operand order of `[base + index]` (the m_pPetInfo[i] loops, the inline strcpy)
// from its internal symbol numbering; each dead expansion of this adds 8 symbols to the caller (HandleMsg, ParsePuppetScript).
// dllimport + __forceinline: always expanded, never emitted out of line (a plain inline leaves a COMDAT behind).
__declspec(dllimport) __forceinline int RbSymPad(int a, int b)
{
	int c = a + b;
	int d = c + a;
	int e = d + c;
	int f = e + d;
	int g = f + e;
	int h = g + f;
	return h;
}

// PROVISORIO (ver golfplayer.h): membros do 645 que o CGolfPlayer do 983 nao tem
std::vector<CGolfPlayer::sCondBBSpray> CGolfPlayer::m_condSpray[4];
std::vector<CGolfPlayer::sCondBBSequence> CGolfPlayer::m_condSeq[4];
std::vector<unsigned long> CGolfPlayer::m_condIds[4];
std::vector<CFxSpray*> CGolfPlayer::m_sprays;
std::vector<CFxSequence*> CGolfPlayer::m_sequences;
std::vector<std::pair<unsigned long, unsigned long> > CGolfPlayer::m_abilityParts;

CGolfPlayer::sPetInfo::sPetInfo()
	: m_motionTime(0),
	  m_pMotion(0),
	  m_bFlag0(false),
	  m_flags(0),
	  m_val1(0),
	  m_pTexPart(0),
	  m_pCurTexPart(0),
	  m_scale(1.0f),
	  m_scaleRate(0)
{
	m_matrix.Reset();
	m_nextMotion[0] = 0;
	memset(m_shotTimes, 0, sizeof(m_shotTimes));
}

CGolfPlayer::sPetInfo::~sPetInfo()
{
}

int CGolfPlayer::GetAbilityPartsSize()
{
	return m_abilityParts.size();
}

// HACK: emission order. Implicit constructors in the original (no line in the PDB), created right here and
// compiled one wave late.
inline CGolfPlayer::Object::Object()
{
	if (0)
	{
		float2int(0.0f);
	}
}

inline CGolfPlayer::sClub::sClub()
{
}

CGolfPlayer::CGolfPlayer()
	: m_pClubPuppet(NULL),
	  m_pPetFrame(NULL),
	  m_pPetInfo(NULL),
	  m_pTail(NULL),
	  m_bFlag15d(true),
	  m_bFlag2f0(false)
{
	ResetComboState(); // 983: chamada a 0073daa0 no lugar das duas atribuicoes
	m_powerShotKeyTime = 0;
	m_val320 = 0;
	m_bPowerShot = false;
	m_bFlag329 = false;
	m_powerShotKeyState = 0;
	m_pHeadBone = NULL;
	m_valF8 = 0;
	m_bFlag330 = false;
	m_ringFxTid983[0] = 0;
	m_ringFxTid983[1] = 0;
	m_ringFxTid983[2] = 0;
	m_ringFxTid983[3] = 0;
	m_pFxInterpreter983 = new FxInterpreter(NULL, 0); // 983: 00740dad..00740dde
	m_scriptIndex983 = 0;
	m_pScriptFrame983 = NULL;
}

void CGolfPlayer::OnDestroy()
{
	GolfClub().ClearClubSet();
	std::list<sTexturePart*>::iterator it;
	for (it = m_pendingTexParts.begin(); it != m_pendingTexParts.end(); ++it)
	{
		if (*it)
		{
			delete *it;
			*it = NULL;
		}
	}
	m_pendingTexParts.clear();
	if (m_pTail)
	{
		delete m_pTail;
		m_pTail = NULL;
	}
	if (m_pPetFrame)
	{
		delete[] m_pPetFrame;
		m_pPetFrame = NULL;
	}
	if (m_pPetInfo)
	{
		delete[] m_pPetInfo;
		m_pPetInfo = NULL;
	}
}

void CGolfPlayer::RegisterClub(int index)
{
	const char* motionName[4] = {
		"\xbf\xec\xb5\xe5\xbc\xa6\xc1\xd8\xba\xf1",
		"\xbe\xc6\xc0\xcc\xbe\xf0\xbc\xa6\xc1\xd8\xba\xf1",
		"\xbb\xf7\xb5\xe5\x9f\x87\xc1\xf6\xbc\xa6\xc1\xd8\xba\xf1",
		"\xc6\xdb\xc6\xc3\xbc\xa6\xc1\xd8\xba\xf1",
	};
	GOLFDOC()->GetPartTidList(GOLFDOC()->m_currentPlayer);
	WMatrix mat;
	mat.Reset();
	GolfClub().ClearClubSet();
	for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		if (index != -1 && i != index)
		{
			continue;
		}
		if (GOLFDOC()->GetPlayer(i)->state == 3)
		{
			continue;
		}
		std::vector<WPuppet*>& pets = m_pPetFrame[i].GetAniPetList();
		sClubSet* pClubSet =
			GolfClub().AddClubSet(Doc()->m_userInfo[i].clubInfo.tid);
		CPartTidList& tids = GOLFDOC()->GetPartTidList(i);
		CItemManager& itemManager = Doc()->m_itemManager;
		IFF_STRUCT::sChar* pChar = itemManager.FindChar(tids.m_charTid);
		std::vector<WPuppet*>::iterator it;
		for (it = pets.begin(); it != pets.end(); it++)
		{
			WPuppet* pPuppet = *it;
			sClub* pClubInfo = m_clubs[i];
			for (int j = 0; j < 4; j++, pClubInfo++)
			{
				sClub& club = *pClubInfo;
				const char* name = motionName[j];
				club.m_ballPos.Reset();
				club.m_gripPos = 0.0f;
				w_motion_data* pMotion = pPuppet->FindMotionData(name);
				if (pMotion)
				{
					int frame = (int)(pMotion->s * 30.0f + 0.5f);
					int from = -1;
					int to;
					pPuppet->CheckFrameData(frame, frame, from, to);
					if (from > -1)
					{
						WSliceStr str(pPuppet->m_frame[from].data);
						str.Get("*ball_dist", "%f", &club.m_ballPos.x);
						str.Get("*grip_pos", "%f", &club.m_gripPos);
						club.m_ballPos.y = 0.0f;
						club.m_ballPos.z = 0.4f;
						if (pClubSet && pChar && pClubSet->pHead[j])
						{
							WList<w_bound_box*>& bbList =
								pClubSet->pHead[j]->m_bbList;
							w_bound_box* pBall = bbList.Find("Ball");
							w_bound_box* pTarget = bbList.Find("Ball_Target");
							if (pBall && pTarget)
							{
								pClubSet->pHead[j]->SetMatrix(mat);
								pClubSet->pHead[j]->Transform(NULL, 1.0f, 0);
								WVector diff;
								diff = (pBall->info->spherePivot *
											   pBall->bone->m_matrix -
										   pTarget->info->spherePivot *
											   pTarget->bone->m_matrix) *
									pChar->ClubScale;
								club.m_ballPos.x += diff.x;
								club.m_ballPos.z += diff.z;
							}
						}
					}
				}
			}
		}
	}
}

void CGolfPlayer::OnPreLoadInit()
{
	m_lightSet.type = 2;
	CSceneManager::Instance()->RegisterElement(OBJ_ACTOR, NULL, NULL,
		"\xc5\xac\xb7\xb4", 0x28, NULL);
	CSceneManager::Instance()->RegisterElement(OBJ_CUSTOM, NULL, m_pTail,
		"tail", 0, NULL);
	CSceneManager::Instance()->SetVisible("tail", false);
	m_pPetInfo = new sPetInfo[4];
	for (int i = 0; i < 4; i++)
	{
		CSceneManager::Instance()->RegisterElement(OBJ_ACTOR, NULL, NULL,
			MakeStr("PLAYER_%d", i), 0x18, NULL);
		CSceneManager::Instance()->SetVisible(MakeStr("PLAYER_%d", i), false);
	}
	m_pTail = new CTailEffect("~swinglight01.jpg");
}

void CGolfPlayer::OnLoad()
{
	if (CHeadIcon::Instance())
	{
		CHeadIcon::Instance()->SetLevel();
	}
	if (m_pPetFrame)
	{
		delete[] m_pPetFrame;
		m_pPetFrame = NULL;
	}
	m_pPetFrame = new CPetFrame[GOLFDOC()->m_playerNum];
	for (int i = 0; i < 4; i++)
	{
		if (i < GOLFDOC()->m_playerNum)
		{
			if (GOLFDOC()->GetPlayer(i)->state != 3)
			{
				CPartTidList* pTids = &GOLFDOC()->GetPartTidList(i);
				// 983 007434c0: sem ShowAbilityParts (SEM_PAR); liga m_bAddonParts983 (+0x354) = pecas addon por personagem
				m_pPetFrame[i].SetAddonParts983(true);
				if (!m_pPetFrame[i].Build(pTids,
						Doc()->m_userInfo[i].charInfo.tidAuxParts, true, true,
						i))
				{
					pTids->SetDefaultTids();
					m_pPetFrame[i].Build(pTids,
						Doc()->m_userInfo[i].charInfo.tidAuxParts, true, false,
						i);
				}
				m_pPetFrame[i].CalcBVolume();
				SetMotion("\xb5\xee\xc0\xe5\xb8\xf0\xbc\xc7", 0.0f, i);
				m_pPetInfo[i].m_val1 = 0;
				m_pPetInfo[i].m_nextMotion[0] = 0;
				((CSceneManagerEx*)CSceneManager::Instance())
					->SetPuppet(MakeStr("PLAYER_%d", i),
						m_pPetFrame[i].GetBonePet());
				m_extEffect.RegistExtenedEffect(&m_pPetFrame[i]);
			}
		}
		else
		{
			m_pPetInfo[i].m_bFlag0 = false;
			((CSceneManagerEx*)CSceneManager::Instance())
				->SetPuppet(MakeStr("PLAYER_%d", i), NULL);
		}
	}
	if (Doc()->m_golfGame.gameType == 1)
	{
		for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
		{
			if (i == MyIndex() ||
				(GOLFDOC()->GetPlayer(MyIndex())->teammate != 0xff &&
					i == PLAYER(MyIndex())->teammate) ||
				!WNetworkSystem::Instance()->IsConnected(
					WNetworkSystem::NET_GAME))
			{
				MakeStr("PLAYER_%d", i);
			}
		}
	}
	RegisterClub(-1);
	CheckShotScript();
}

void CGolfPlayer::OnReload()
{
	OnLoad();
}

bool CGolfPlayer::ShowAbilityParts(CPartTidList* pTids)
{
	if (m_abilityParts.empty())
	{
		AddAbilityParts(0x80a2800, 0x80a4000);
		AddAbilityParts(0x80a2801, 0x80a4001);
		AddAbilityParts(0x80a2802, 0x80a4002);
		AddAbilityParts(0x8122800, 0x8124000);
		AddAbilityParts(0x8122801, 0x8124001);
		AddAbilityParts(0x8122802, 0x8124002);
		AddAbilityParts(0x8026800, 0x8024800);
		AddAbilityParts(0x8026801, 0x8024801);
		AddAbilityParts(0x8026802, 0x8024802);
		AddAbilityParts(0x8064800, 0x8066800);
		AddAbilityParts(0x8064801, 0x8066801);
		AddAbilityParts(0x8064802, 0x8066802);
		AddAbilityParts(0x8064803, 0x8066803);
		AddAbilityParts(0x81ea800, 0x81ec800);
		AddAbilityParts(0x80e4800, 0x80e6800);
		AddAbilityParts(0x80e4801, 0x80e6801);
		AddAbilityParts(0x80e4802, 0x80e6802);
		AddAbilityParts(0x81a4800, 0x81a6800);
		AddAbilityParts(0x81a4800, 0x81a8800);
		AddAbilityParts(0x81a4801, 0x81a6801);
		AddAbilityParts(0x81a4801, 0x81a8801);
		AddAbilityParts(0x816e801, 0x8152801);
		AddAbilityParts(0x816e802, 0x8152802);
		AddAbilityParts(0x816e803, 0x8152803);
		AddAbilityParts(0x8228800, 0x8226800);
		AddAbilityParts(0x8228801, 0x8226801);
		AddAbilityParts(0x8228802, 0x8226802);
		AddAbilityParts(0x8228803, 0x8226803);
	}
	bool bChanged = false;
	int size = GetAbilityPartsSize();
	for (int i = 0; i < size; i++)
	{
		unsigned long to = m_abilityParts[i].second;
		unsigned long from = m_abilityParts[i].first;
		if ((pTids->m_charTid & 0x3ffffff) == ((from >> 18) & 0xff) &&
			pTids->IsEquipped(from, 0) == true)
		{
			pTids->SetTid(to, 0, NULL, 0);
			bChanged = true;
		}
	}
	return bChanged;
}

void CGolfPlayer::CheckShotScript()
{
	for (int i = 0; i < GOLFDOC()->GetPlayerNum(); i++)
	{
		if (GOLFDOC()->GetPlayer(i)->state == 3)
		{
			continue;
		}
		for (int j = 0; j < 16; j++)
		{
			w_motion_data* pMotion =
				m_pPetFrame[i].FindMotionData(s_motionName[j], true, NULL);
			if (pMotion == NULL)
			{
				continue;
			}
			int start = (int)ceil(pMotion->s * 30.0f);
			float endFrame = pMotion->e * 30.0f;
			int end = Wabs(floor(endFrame) - endFrame) < g_EPSILON
				? -1 - (int)(pMotion->e * -30.0f)
				: (int)floor(pMotion->e * 30.0f);
			for (int frame = start; frame < end; frame++)
			{
				int index;
				if (m_pPetFrame[i].GetCurAniPet()->CheckFrameData(frame, frame,
						index, index))
				{
					char* data =
						m_pPetFrame[i].GetCurAniPet()->GetFrameData(index);
					if (strstr(data, "*shot"))
					{
						m_pPetInfo[i].m_shotTimes[j] =
							(float)(frame - start) / 30.0f; // 983 0073f210: divide por 30 (nao *1/30)
						break;
					}
				}
			}
		}
	}
}

void CGolfPlayer::OnInit()
{
	m_pFootBones[0] = NULL;
	m_pFootBones[1] = NULL;
	m_motionName[0] = 0;
	m_swingTypeName[0] = 0;
	m_bFlag0 = false;
	m_angle = 3.1415927f;
	_ReadWriteBarrier();
	m_matrix.zz = 1.0f;
	m_matrix.yy = 1.0f;
	m_matrix.xx = 1.0f;
	m_matrix.zm = 0;
	m_matrix.ym = 0;
	m_matrix.xm = 0;
	m_matrix.yz = 0;
	m_matrix.xz = 0;
	m_matrix.zy = 0;
	m_matrix.xy = 0;
	m_matrix.zx = 0;
	m_matrix.yx = 0;
	m_lightSet.ambient = ModifyColor(g_lightset.ambient, 1.0f);
	m_lightSet.nearOne = g_lightset.nearOne;
}

void CGolfPlayer::InputProcess(float delta)
{
	if (g_input->Get(0x45))
	{
		if (m_headAngle < 1.0471975430846214) // 983: compara com double (00d1a498 = 60 * (double)0.017453292f)
		{
			m_headAngle += RB_MUL_4PI(delta);
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags |= 0x200;
		}
	}
	if (g_input->Get(0x51))
	{
		if (m_headAngle > -1.0471975430846214) // 983: double (00d1ca90)
		{
			m_headAngle -= RB_MUL_4PI(delta);
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags |= 0x200;
		}
	}
}

static int AuxPartEffectIndex983(int player);
static unsigned long AuxPartEffectTid983(int index);

void CGolfPlayer::OnProcess(float delta)
{
	switch (m_pPetInfo[GOLFDOC()->m_currentPlayer].m_val1)
	{
	case 0:
	case 4:
		if (GOLFDOC()->m_gameMode & 0xa4000)
		{
			m_bSwingPending = false;
		}
		else
		{
			// 983 00743920: nao troca para o "pronto" enquanto o 0x10e pendente (m_bFlag304, +0x248) nao saiu
			if (m_bSwingPending && !m_bFlag190 && !m_bFlag304 && m_swingTypeName[0])
			{
				const char* name = ""; // 983 00743aee: edi = 0xd65136 ("") antes do switch; tipo > 3 -> SetMotion("")
				switch (GolfClub().PowerShotType())
				{
				case 0:
					name = MakeStr("%s\xc1\xd8\xba\xf1", m_swingTypeName);
					break;
				case 1:
					name = MakeStr("%s\xc6\xc4\xbf\xf6\xc1\xd8\xba\xf1\xb3\xa1",
						m_swingTypeName);
					break;
				case 2:
				case 3:
					name = MakeStr("%s\xc6\xc4\xbf\xf6"
								   "2\xc1\xd8\xba\xf1\xb3\xa1",
						m_swingTypeName);
					break;
				}
				SetMotion(name, 0.0f, -1);
			}
			m_bSwingPending = m_bFlag190;
		}
		m_bFlag190 = false;
		break;
	case 3:
	{
		float rate;
		if (m_pPetInfo[GOLFDOC()->m_currentPlayer].m_pMotion)
		{
			rate =
				(delta + m_pPetInfo[GOLFDOC()->m_currentPlayer].m_motionTime -
					m_pPetInfo[GOLFDOC()->m_currentPlayer].m_pMotion->s) /
				(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_pMotion->e -
					m_pPetInfo[GOLFDOC()->m_currentPlayer].m_pMotion->s);
			if (rate > 1.0f)
			{
				rate = 1.0f;
				m_pPetInfo[GOLFDOC()->m_currentPlayer].m_val1 = 4;
				SendMsg(this, "Spot", M983(0xf5), 0, 0, 0, 0);
				if (OnlinePlay())
				{
					int powerShot = GolfClub().PowerShotType();
					WSendPacket packet((enumClientPacket)0x15);
					packet.Encode1(powerShot);
					packet.Send((eSendTo)0);
				}
			}
			SendMsg(this, "Screen", M983(0x1aa), (int)&rate, 0, 0, 0);
		}
		break;
	}
	}

	for (int i = 0; i < GOLFDOC()->GetPlayerNum(); i++)
	{
		if (!m_pPetInfo[i].m_bFlag0 || GOLFDOC()->GetPlayer(i)->state == 3)
		{
			continue;
		}
		m_pPetInfo[i].m_motionTime += delta;
		if (Abs(m_pPetInfo[i].m_scaleRate) > 0.0f)
		{
			sPetInfo* pScale = &m_pPetInfo[i];
			float scale = delta * pScale->m_scaleRate + pScale->m_scale;
			if (scale < 0.0f)
			{
				scale = 0.0f;
			}
			else if (scale > 1.0f)
			{
				scale = 1.0f;
			}
			pScale->m_scale = scale;
		}
		sPetInfo* pInfo = &m_pPetInfo[i];
		ScriptProcess(pInfo->m_pMotion, pInfo->m_motionTime - delta,
			pInfo->m_motionTime, i, false, false);

		if (m_pPetInfo[i].m_pMotion != NULL &&
			m_pPetInfo[i].m_motionTime > m_pPetInfo[i].m_pMotion->e)
		{
			if (m_pPetInfo[i].m_pMotion->s != m_pPetInfo[i].m_pMotion->e)
			{
				if (m_pPetInfo[i].m_val1 == 2)
				{
					m_bFlag0 = false;
					CSceneManager::Instance()->SetVisible("tail", false);
				}
				if (m_pPetInfo[i].m_nextMotion[0] != 0)
				{
					if (m_bFlag304)
					{
						m_bFlag304 = false;
						HandleMsg(MsgObject(NULL, M983(0x10e), 0, 0, 0, 0, 0));
					}
					else
					{
						SetMotion(m_pPetInfo[i].m_nextMotion,
							m_pPetInfo[i].m_motionTime -
								m_pPetInfo[i].m_pMotion->e,
							i);
						m_pPetInfo[i].m_nextMotion[0] = 0;
					}
				}
				else
				{
					char ready[64];
					sprintf(ready, "%s\xc1\xd8\xba\xf1", m_swingTypeName);
					w_motion_data* pMotion = m_pPetInfo[i].m_pMotion;
					const char* name = pMotion->name;
					if (strcmp(name, "\xb5\xee\xc0\xe5\xb8\xf0\xbc\xc7") == 0 &&
						m_swingTypeName[0])
					{
						if (strcmp(ready, pMotion->nextmotion) != 0)
						{
							SetMotion(ready,
								m_pPetInfo[i].m_motionTime - pMotion->e, i);
						}
						else
						{
							SetMotion(m_pPetInfo[i].m_pMotion->nextmotion,
								m_pPetInfo[i].m_motionTime -
									m_pPetInfo[i].m_pMotion->e,
								i);
						}
					}
					else if (i == GOLFDOC()->m_currentPlayer &&
						strcmp(name, ready) == 0 &&
						(m_bFlag190 || GolfClub().PowerShotType() != 0 ||
							GOLFDOC()->m_gameMode != 8 || m_val2f4 < 9.6f))
					{
						SetMotion(m_pPetInfo[i].m_pMotion->name,
							m_pPetInfo[i].m_motionTime -
								m_pPetInfo[i].m_pMotion->e,
							i);
					}
					else
					{
						SetMotion(m_pPetInfo[i].m_pMotion->nextmotion,
							m_pPetInfo[i].m_motionTime -
								m_pPetInfo[i].m_pMotion->e,
							i);
					}
				}
			}
			else
			{
				m_pPetInfo[i].m_motionTime = m_pPetInfo[i].m_pMotion->s;
				m_pPetInfo[i].m_flags &= ~2;
			}
		}
	}

	if (GolfClub().TypeUpdated())
	{
		ParseBoundingBoxScript(GOLFDOC()->m_currentPlayer, true, false, true);
		SetSwingType();
		SearchGround();
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags |= 0x21;
		// 983 00743920 (apos 00633480/00633500): anel com efeito do jogador da vez -> "Player" 0x335 (tid do item)
		int ring = AuxPartEffectIndex983(GOLFDOC()->m_currentPlayer);
		if (ring != -1)
		{
			SendMsg(NULL, "Player", 0x335, (int)AuxPartEffectTid983(ring), 0, 0, 0);
		}
	}

	if (m_bFlag0)
	{
		m_pTail->Process(delta);
		if (m_pTailSpots[0] && m_pTailSpots[1])
		{
			WVector nearPos, farPos;
			nearPos = m_pTailSpots[0]->info->spherePivot *
				m_pTailSpots[0]->bone->GetMatrix();
			farPos = m_pTailSpots[1]->info->spherePivot *
				m_pTailSpots[1]->bone->GetMatrix();
			m_pTail->PutSpot(nearPos, farPos);
		}
	}

	if (GolfClub().GetType() == 3)
	{
		if (GolfBall().m_groundType == 2 && GOLFDOC()->m_tutorialMode >= 4 &&
			GolfBall().m_playFrame < 0 &&
			// 983 00743920: so com o bit 8 do modo
			(GOLFDOC()->m_gameMode & 8) &&
			(g_input->GetDown("LALT", true) || g_input->GetDown("RALT", true)))
		{
			m_bFlag2a0 = !m_bFlag2a0;
		}
	}
	else
	{
		PowerShotInput();
	}

	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
	{
		RefreshAllUccClothes();
	}
}

// 983 0073e3a0: nomes de movimento (cp949) das poses de vitoria do albatroz por personagem
static bool IsAlbatrossWinPose983(unsigned long tid, const w_motion_data* pMotion)
{
	if (!pMotion || !pMotion->name)
	{
		return false;
	}
	const char* name = pMotion->name;
	switch (tid & 0x3ffffff)
	{
	case 3:
		return !strcmp(name, "\xc7\xef\xb1\xe2\xbe\xcb\xb9\xd9\xc8\xa6\xc0\xce\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee_item");
	case 5:
		return !strcmp(name, "\xc7\xd8\xc0\xfb\xbc\xb1\xbe\xcb\xb9\xd9\xc8\xa6\xc0\xce\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee_item");
	case 10:
		return !strcmp(name, "\xbe\xcb\xb9\xd9\xc8\xa6\xc0\xce\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee") ||
			!strcmp(name, "\xbf\xec\xc1\xd6\xbc\xb1\xb5\xee\xc0\xe5\xb8\xf0\xbc\xc7_item");
	}
	return false;
}

void CGolfPlayer::OnDisplay()
{
	if ((GOLFDOC()->m_gameMode & 0x20000) ||
		// 983 00744330: so os tipos 1 e 7 (o 645 incluia o 3)
		((Doc()->m_golfGame.gameType == 1 || Doc()->m_golfGame.gameType == 7) &&
			(GOLFDOC()->m_gameMode & 0x3000)))
	{
		ScoreEvent();
		return;
	}
	if (!m_bFlag15d)
	{
		return;
	}
	UpdatePetMatrix983();
	// 983 0073e4e0/0073e3a0: nao acerta a altura quando o personagem 3, 5 ou 10 esta na pose de vitoria do Hole-in-One de albatroz
	if ((GOLFDOC()->m_gameMode & 0x1002) &&
		!IsAlbatrossWinPose983(
			GOLFDOC()->GetPartTidList(GOLFDOC()->m_currentPlayer).m_charTid,
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_pMotion))
	{
		WVector pos = GetHeadMatrix().pivot;
		pos.y = 0.0f;
		pos = GetPVS().GetGroundPoint(pos, false, NULL, false);
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_matrix.pivot.y = pos.y;
	}
	if (!(GOLFDOC()->m_gameMode & 0x4000) &&
		(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags & 0xb))
	{
		for (int i = 0; i < 2; i++)
		{
			if (m_pFootBones[i])
			{
				m_pFootBones[i]->SetMatrix(
					*(WMatrix*)convMatInv * GetFootMatrix(i), NULL);
			}
		}
	}
	if ((m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags & 0x203) && m_pHeadBone)
	{
		m_pHeadBone->SetMatrix(*(WMatrix*)convMatInv * GetHeadMatrix(), NULL);
	}
	if (CHeadIcon::IsInstantiated())
	{
		WVector pos = m_pPetFrame[GOLFDOC()->m_currentPlayer]
						  .GetBonePet()
						  ->FindBone("Bip01 Head")
						  ->GetMatrix()
						  .pivot;
		CHeadIcon::Instance()->SetPlayerPos(GOLFDOC()->m_currentPlayer, pos);
	}
	if (m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags & 3)
	{
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags |= 0x30;
	}
	WPuppet* pBonePet = m_pPetFrame[GOLFDOC()->m_currentPlayer].GetBonePet();
	sClubSet* pClubSet = GolfClub().FindClubSet(
		Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].clubInfo.tid);
	WPuppet* pClub = pClubSet->pHead[GolfClub().GetType()];
	if (g_view->InFrustum(pBonePet->GetBoundSphere()))
	{
		if ((unsigned char)(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags >>
				4) &
			1)
		{
			if ((unsigned char)(m_pPetInfo[GOLFDOC()->m_currentPlayer]
									.m_flags >>
					6) &
				1)
			{
				SetLight();
			}
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags &= ~0x10;
			m_pPetFrame[GOLFDOC()->m_currentPlayer].UpdateLightSource(
				&m_lightSet, false, NULL);
		}
		if (!m_bFlag15e && !m_bFlag2a0)
		{
			m_pPetFrame[GOLFDOC()->m_currentPlayer].SetAlpha(
				(unsigned char)(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_scale *
					255.0f));
			pClub->SetAlpha(
				(unsigned char)(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_scale *
					255.0f));
		}
		else
		{
			m_pPetFrame[GOLFDOC()->m_currentPlayer].SetAlpha(
				(unsigned char)(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_scale *
					64.0f));
			pClub->SetAlpha(
				(unsigned char)(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_scale *
					64.0f));
		}
		if (!(Wabs(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_scale) < g_EPSILON))
		{
			pClub->GetAlpha(NULL);
		}
	}
	if (!((unsigned char)(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags >> 7) &
			1))
	{
		CPartTidList& tids =
			GOLFDOC()->GetPartTidList(GOLFDOC()->m_currentPlayer);
		CItemManager& itemManager = Doc()->m_itemManager;
		IFF_STRUCT::sChar* pChar = itemManager.FindChar(tids.m_charTid);
		float scale = pChar ? pChar->ClubScale : 1.0f;
		WMatrix mat;
		if ((unsigned char)(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags >>
				8) &
			1)
		{
			mat.xa = scale * m_clubMatrix.xa;
			mat.ya = scale * m_clubMatrix.ya;
			mat.za = scale * m_clubMatrix.za;
			mat.pivot = m_clubMatrix.pivot;
		}
		else
		{
			WPuppet* pPet =
				m_pPetFrame[GOLFDOC()->m_currentPlayer].GetBonePet();
			w_bound_box* pBox = pPet->m_bbList.Find(RB_STR_CLUB);
			if (pBox)
			{
				mat = pBox->bone->GetMatrix();
				mat.pivot = pBox->info->spherePivot * pBox->bone->GetMatrix();
				mat.pivot += mat.xa *
					m_clubs[GOLFDOC()->m_currentPlayer][GolfClub().GetType()]
						.m_gripPos;
				mat.xa *= scale;
				mat.ya *= scale;
				mat.za *= scale;
			}
			else
			{
				mat.Reset();
			}
		}
		pClub->SetMatrix(mat);
		pClub->Transform(NULL, 1.0f, 0);
		pClub->UpdateBSphere();
		if (GolfClub().TypeUpdated())
		{
			m_pClubPuppet = pClub;
		}
		if ((unsigned char)(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags >>
				5) &
			1)
		{
			if ((unsigned char)(m_pPetInfo[GOLFDOC()->m_currentPlayer]
									.m_flags >>
					6) &
				1)
			{
				SetLight();
			}
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags &= ~0x20;
		}
		pClub->UpdateLightSource(&m_lightSet, false, NULL);
		((CSceneManagerEx*)CSceneManager::Instance())
			->SetPuppet("\xc5\xac\xb7\xb4", pClub);
	}
	if (m_bFlag0)
	{
		m_pTail->Display();
	}
	m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags &= ~0x209;
	m_extEffect.Update(&m_pPetFrame[GOLFDOC()->m_currentPlayer]);
}

// 983 007410c0: matriz do pet do jogador da vez (ramos 0x3000/0x4000/normal, igrass), SetMatrix/Animate/UpdateBound
void CGolfPlayer::UpdatePetMatrix983()
{
	sPetInfo& info = m_pPetInfo[GOLFDOC()->m_currentPlayer];
	if (info.m_flags & 1)
	{
		if (GOLFDOC()->m_gameMode & 0x3000)
		{
			WMatrix& mat = info.m_matrix;
			mat.za = m_matrix.za;
			mat.ya = WVector(0.0f, 1.0f, 0.0f);
			mat.xa = WCrossProduct(mat.ya, mat.za);
			// 983 007410c0: ponto do solo direto de m_pos (this+0x70), sem multiplicar pela matriz
			mat.pivot =
				GetPVS().GetGroundPoint(m_pos, false, NULL, false);
		}
		else if (GOLFDOC()->m_gameMode & 0x4000)
		{
			info.m_matrix.za = m_matrix.za;
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_matrix.ya =
				WVector::UNIT_POS_Y;
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_matrix.xa = WCrossProduct(
				m_pPetInfo[GOLFDOC()->m_currentPlayer].m_matrix.ya,
				m_pPetInfo[GOLFDOC()->m_currentPlayer].m_matrix.za);
			m_pPetInfo[GOLFDOC()->m_currentPlayer]
				.m_matrix.pivot = GetPVS().GetGroundPoint(m_pos +
					m_clubs[GOLFDOC()->m_currentPlayer][GolfClub().GetType()]
							.m_ballPos *
						m_matrix,
				false, NULL, false);
		}
		else
		{
			info.m_matrix = m_matrix;
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_matrix.pivot = m_pos +
				m_clubs[GOLFDOC()->m_currentPlayer][GolfClub().GetType()]
						.m_ballPos *
					m_matrix;
		}
		const char* texName = GetPVS().GetTextureName(
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_matrix.pivot);
		if (texName &&
			(!stricmp(texName, "igrass05.dds") ||
				!stricmp(texName, "igrass04_01.dds")))
		{
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_matrix.pivot.y -=
				0.4f * 0.8f;
		}
	}
	if (m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags & 3)
	{
		m_pPetFrame[GOLFDOC()->m_currentPlayer].GetBonePet()->SetMatrix(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_matrix);
		m_pPetFrame[GOLFDOC()->m_currentPlayer].Animate983(NULL, NULL, m_pPetInfo[GOLFDOC()->m_currentPlayer].m_motionTime);
		m_pPetFrame[GOLFDOC()->m_currentPlayer].GetBonePet()->UpdateBound(true,
			true);
	}
}

void CGolfPlayer::ScoreEvent()
{
	for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		if (!m_pPetInfo[i].m_bFlag0 || GOLFDOC()->GetPlayer(i)->state == 3)
		{
			continue;
		}
		m_pPetFrame[i].Animate983(NULL, NULL, m_pPetInfo[i].m_motionTime);
		m_pPetFrame[i].GetBonePet()->UpdateBound(true, false);
		if (g_view->InFrustum(m_pPetFrame[i].GetBonePet()->GetBoundSphere()))
		{
			m_pPetFrame[i].UpdateLightSource(&m_lightSet, false, NULL);
		}
		if (CHeadIcon::IsInstantiated())
		{
			WVector pos = m_pPetFrame[i]
							  .GetBonePet()
							  ->FindBone("Bip01 Head")
							  ->GetMatrix()
							  .pivot;
			CHeadIcon::Instance()->SetPlayerPos(i, pos);
		}
		sTexturePart* pPart = m_pPetInfo[i].m_pTexPart;
		sPetInfo& info = m_pPetInfo[i];
		if (pPart && pPart != info.m_pCurTexPart)
		{
			info.m_pCurTexPart = pPart;
			if (pPart->pBitmap == NULL)
			{
				m_pPetFrame[i].GetBonePet()->ChangeTexturePart(
					m_pPetFrame[i].GetFaceTex(), pPart->texture,
					&pPart->pBitmap, &pPart->rect);
			}
			else
			{
				m_pPetFrame[i].GetBonePet()->ChangeTexturePart(
					m_pPetFrame[i].GetFaceTex(), pPart->pBitmap, pPart->rect);
			}
			m_pPetFrame[i].SetMakeup();
		}
		if (m_pClubPuppet == NULL)
		{
			return;
		}
		if (GOLFDOC()->m_gameMode & 0x3000)
		{
			if (Doc()->m_golfGame.gameType == 1)
			{
				if (i != TEAM(PLAYER(MyIndex())->team)->shooter ||
					((unsigned char)(m_pPetInfo[i].m_flags >> 7) & 1))
				{
					continue;
				}
			}
			if ((unsigned char)(m_pPetInfo[i].m_flags >> 8) & 1)
			{
				m_pClubPuppet->SetMatrix(m_clubMatrix);
			}
			else
			{
				WPuppet* pPet = m_pPetFrame[i].GetBonePet();
				w_bound_box* pBox = pPet->m_bbList.Find("club");
				WMatrix mat;
				mat = pBox->bone->GetMatrix();
				mat.pivot = pBox->info->spherePivot * pBox->bone->GetMatrix();
				mat.pivot +=
					m_clubs[i][GolfClub().GetType()].m_gripPos * mat.xa;
				m_pClubPuppet->SetMatrix(mat);
			}
			m_pClubPuppet->Transform(NULL, 1.0f, 0);
			m_pClubPuppet->UpdateBSphere();
			m_pClubPuppet->UpdateLightSource(&m_lightSet, false, NULL);
		}
	}
}

WMatrix CGolfPlayer::GetHeadMatrix()
{
	WMatrix mat;
	WBone* pBone = m_pHeadBone;
	if (pBone == NULL)
	{
		mat.Reset();
		return mat;
	}
	if (Wabs(m_headAngle) < g_EPSILON)
	{
		mat = *(WMatrix*)convMat * pBone->GetMatrix();
	}
	else
	{
		mat = RotMat(m_headAngle, 1);
		mat = mat * *(WMatrix*)convMat * pBone->GetMatrix();
	}
	return mat;
}

WMatrix CGolfPlayer::GetFootMatrix(int foot)
{
	WMatrix mat;
	WBone* pBone = m_pFootBones[foot];
	if (pBone == NULL)
	{
		mat.Reset();
		return mat;
	}
	if (Wabs(m_footAngle) < g_EPSILON)
	{
		mat = *(WMatrix*)convMat * pBone->GetMatrix();
	}
	else
	{
		mat = RotMat(m_footAngle, 0);
		mat = mat * *(WMatrix*)convMat * pBone->GetMatrix();
	}
	return mat;
}

// 983 0073e590 (out/F5/partida/b618/d.txt): tudo pelo FxInterpreter do jogador (+0x20); o 645 lia o *fx( das caixas aqui.
// Argumentos 983 (ret 0x10): index, bClear, b1 (= bShowAll de ParseAvatarScript_fx), b2 (ignorado)
void CGolfPlayer::ParseBoundingBoxScript(int index, bool bClear, bool b1, bool b2)
{
	if (bClear)
	{
		m_pFxInterpreter983->DetachFxAll(-1);
	}
	sClubSet* pClubSet = GolfClub().FindClubSet(Doc()->m_userInfo[index].clubInfo.tid);
	m_pFxInterpreter983->ParseAvatarScript_fx(&m_pPetFrame[index], pClubSet, index, b1, false);
	m_pFxInterpreter983->ParseClubScript_fx(pClubSet, index);
	m_pFxInterpreter983->AddModeRule("scythe_girl.seq", 0x200, false);
	m_pFxInterpreter983->ShowNamed("\264\353\303\265\273\347", true, 0); // d04b7c
	m_pFxInterpreter983->ShowNamed("\277\344\301\244\263\257\260\263", true, 0); // d04b70
}

// 983 00633480 (CSharedDoc): indice no vetor de efeitos de item auxiliar (+0x114b8, elementos de 0x14) do jogador cujo
// +0xc e o byte +0x12b do sUserInfo e +4 != -1; -1 se nao ha
C_ASSERT_VC10(offsetof(CSharedDoc, m_auxPartValues983) == 0x114b8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, info.dwUID) == 0x39db && sizeof(sUserInfo) == 0x31c6);
static int AuxPartEffectIndex983(int player)
{
	// B1469: membros (cl 14.44 desloca o CSharedDoc; doc+0x114b8 / doc+player*0x31c6+0x39db crus liam lixo)
	// B1567: vector<sAuxpartValue>* (= &CConditionalAuxpartObject::m_applied, D959); antes el+0xc/el+4 crus
	std::vector<sAuxpartValue>* pValues = (std::vector<sAuxpartValue>*)Doc()->m_auxPartValues983[player];
	if (!pValues)
	{
		return -1;
	}
	int n = (int)pValues->size();
	unsigned long key = Doc()->m_userInfo[player].info.dwUID; // 983: doc + player*0x31c6 + 0x39db
	for (int i = 0; i < n; i++)
	{
		const sAuxpartValue& value = (*pValues)[i];
		if ((unsigned long)value.m_uid == key && value.m_kind != -1)
		{
			return i;
		}
	}
	return -1;
}

// 983 00633500: tid do item do elemento (do jogador da vez)
C_ASSERT_VC10(sizeof(sAuxpartValue) == 0x14 && offsetof(sAuxpartValue, m_kind) == 4 && offsetof(sAuxpartValue, m_uid) == 0xc);
C_ASSERT_VC10(offsetof(IFF_STRUCT::sAuxPart, c) + offsetof(IFF_ITEM_COMMON, TypeId) == 4);
static unsigned long AuxPartEffectTid983(int index)
{
	std::vector<sAuxpartValue>* pValues = (std::vector<sAuxpartValue>*)Doc()->m_auxPartValues983[GOLFDOC()->m_currentPlayer];
	return (*pValues)[index].m_pItem->c.TypeId; // B1567: *(el+0)+4 cru = sAuxPart::c.TypeId
}

void CGolfPlayer::HandleMsg(const MsgObject& msg)
{
	// HACK: 10 dead expansions (80 symbols), see RbSymPad
	RbSymPad(1, 2);
	RbSymPad(1, 2);
	RbSymPad(1, 2);
	RbSymPad(1, 2);
	RbSymPad(1, 2);
	RbSymPad(1, 2);
	RbSymPad(1, 2);
	RbSymPad(1, 2);
	RbSymPad(1, 2);
	RbSymPad(1, 2);
	if (msg.time > g_CurrentTime)
	{
		StoreDelayedMsg(msg);
		return;
	}
	switch (msg.message)
	{
	case M983(0x114):
		m_bFlag2a0 = msg.param1 == 1;
		break;
	case M983(0x110):
		*(float*)msg.param1 = m_angle - g_PI;
		break;
	case M983(0x10e):
	{
		if (m_bFlag304)
		{
			break;
		}
		if (m_pPetInfo[GOLFDOC()->m_currentPlayer].m_val1 == 2)
		{
			break;
		}
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_val1 = 2;
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_nextMotion[0] = 0;
		// 983 00744b80 case 0x13c: sem nome de movimento manda 0x92 (nosso 0x77) a si mesmo antes do SetMotion
		if (m_motionName[0] == 0)
		{
			SendMsg(this, "Player", M983(0x77), 0, 0, 0, 0);
		}
		SetMotion(m_motionName, 0.0f, -1);
		if ((GolfClub().GetType() == 0 || GolfClub().GetType() == 1) &&
			GetBarAccuracyGrade() < 2)
		{
			break;
		}
		WVector delta;
		delta = GolfBall().PreData(GolfBall().m_renderNum)->pos - GetPinPos();
		delta.y = 0.0f;
		float lastDist = delta.Magnitude();
		delta = GolfBall().PreData(0)->pos - GetPinPos();
		delta.y = 0.0f;
		float firstDist = delta.Magnitude();
		switch (GolfClub().GetType())
		{
		case 3:
			if ((Doc()->m_golfGame.gameType == 7 &&
					(short)GOLFDOC()->m_skinsNextTurnWord983 == 2) ||
				(GOLFDOC()->m_skinsCarryCount &&
					(short)GOLFDOC()->m_skinsNextTurnWord983 == 3))
			{
				if (!(firstDist > 32.0f))
				{
					return;
				}
				if (!(lastDist < 3.2f))
				{
					return;
				}
			}
			else
			{
				if (!(firstDist > 32.0f))
				{
					return;
				}
				int chance;
				if (firstDist > RB_REAL_96)
				{
					chance = rand() % 4;
				}
				else if (firstDist > 64.0f)
				{
					chance = rand() % 3;
				}
				else
				{
					chance = rand() % 2;
				}
				if (!(lastDist < 3.2f))
				{
					return;
				}
				if (chance < 1)
				{
					return;
				}
			}
			break;
		case 0:
			if (!GolfBall().m_bOB && lastDist > 416.0f)
			{
				SendMsg(this, "Player", M983(0x123), 0, 0, 0, 0);
			}
			return;
		default:
			return;
		}
		SendMsg(this, "GolfRule", M983(0x9a), 0, 0, 0, 0);
		strcpy(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_nextMotion,
			"\xc6\xdb\xc6\xc3\xc8\xc4\xb4\xeb\xb1\xe2");
		break;
	}
	case M983(0x10f):
		SetSwingType();
		break;
	// 983 00744b80: 0x14b/0x14c/0x14d vem do CItemWindow::ProcessItem (itens 4/16/34/47, 39, 46) e 0x14e do console;
	// todos chamam 00743390. O nosso so tratava 0x14b (0x14c era o provisorio 0x4000011e; 0x14d/0x14e sem caso).
	case M983(0x11d):
		SetPowerShotReady983(MakeStr(RB_STR_POWER_READY, m_swingTypeName), 1, true);
		break;
	case 0x14c:
		if (!IsLocalContent((localContentType_t)0x32 /*983: era 0x31; medido*/))
		{
			break;
		}
		SetPowerShotReady983(MakeStr(RB_STR_POWER2_READY, m_swingTypeName), 3, true);
		break;
	case 0x14d:
		SetPowerShotReady983(MakeStr(RB_STR_POWER2_READY, m_swingTypeName), 2, true);
		break;
	case 0x14e:
		if (GolfClub().GetType() != 3)
		{
			int level = msg.param1;
			const char* ready;
			if (level == 1)
				ready = "\xc6\xc4\xbf\xf6\xc1\xd8\xba\xf1";
			else if (level == 2 || level == 3)
				ready = "\xc6\xc4\xbf\xf6"
						"2\xc1\xd8\xba\xf1";
			else
				break;
			SetPowerShotReady983(MakeStr("%s%s", m_swingTypeName, ready), level, false);
		}
		break;
	case M983(0xe2):
	{
		for (int i = 0; i < GOLFDOC()->GetPlayerNum(); i++)
		{
			const char* name = MakeStr("PLAYER_%d", i);
			if (PLAYER(i)->state == 3)
			{
				if (m_pPetInfo[i].m_bFlag0)
				{
					m_pPetInfo[i].m_bFlag0 = false;
					if (m_pPetFrame[i].GetBonePet())
						m_pPetFrame[i].GetBonePet()->HideBone(NULL);
					CSceneManager::Instance()->SetVisible(name, false);
				}
			}
			else
			{
				m_pPetInfo[i].m_bFlag0 = i == GOLFDOC()->m_currentPlayer;
				if (m_pPetInfo[i].m_bFlag0)
				{
					if (m_pPetFrame[i].GetBonePet())
						m_pPetFrame[i].GetBonePet()->ShowBone(NULL);
				}
				else
				{
					if (m_pPetFrame[i].GetBonePet())
						m_pPetFrame[i].GetBonePet()->HideBone(NULL);
				}
				CSceneManager::Instance()->SetVisible(name,
					m_pPetInfo[i].m_bFlag0);
				if (IsCurMapSnow())
				{
					Attacher attacher;
					WPuppet* pPet = m_pPetFrame[i].GetBonePet();
					if (!pPet)
						continue;
					w_bound_box* pBox = pPet->m_bbList.Find("Breathe");
					if (pBox)
					{
						attacher.mat[0] = &pBox->bone->GetMatrix();
						attacher.pos[0] = &pBox->info->spherePivot;
						CFx::Instance()->Attach(attacher, "breath.seq", true,
							NULL);
					}
				}
			}
			// 983 00744b80 (0x10a): sem os aneis "High-Rank.seq" dos dedos do 645; as asas "angeliceffect" vem no fim do caso
		}
		m_bFlag2f0 = false;
		m_bFlag2a0 = false;
		m_bFlag0 = false;
		m_bFlag304 = false;
		CSceneManager::Instance()->SetVisible("tail", false);
		SetSwingType();
		float radius = GolfBall().m_radius;
		m_pos = GolfBall().m_pos - WVector(0.0f, radius, 0.0f);
		{
			Waabb area;
			area.min = GolfBall().m_pos - WVector(16.0f, 0.0f, 16.0f);
			area.max = GolfBall().m_pos + WVector(16.0f, 0.0f, 16.0f);
			GetPVS().GetObjList(area, 1);
		}
		m_bFlag190 = false;
		m_bSwingPending = false;
		m_angle = GOLFDOC()->m_cameraData.angle + g_PI;
		m_matrix = RotMat(m_angle, 1);
		SearchGround();
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags |= 0x40;
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags &= ~0x180;
		WPuppet* pPet = m_pPetFrame[GOLFDOC()->m_currentPlayer].GetBonePet();
		m_pTailSpots[0] = pPet->m_bbList.Find("club_near");
		pPet = m_pPetFrame[GOLFDOC()->m_currentPlayer].GetBonePet();
		m_pTailSpots[1] = pPet->m_bbList.Find("club_far");
		m_pTail->Reset();
		m_pFootBones[0] = m_pPetFrame[GOLFDOC()->m_currentPlayer]
							  .GetBonePet()
							  ->FindBoneByTail("L Foot");
		m_pFootBones[1] = m_pPetFrame[GOLFDOC()->m_currentPlayer]
							  .GetBonePet()
							  ->FindBoneByTail("R Foot");
		m_headAngle = 0.0f;
		m_pHeadBone =
			m_pPetFrame[GOLFDOC()->m_currentPlayer].GetBonePet()->FindBone(
				"Bip01 Head");
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_nextMotion[0] = 0;
		WVector dist = GetPinPos() - PLAYER(GOLFDOC()->m_currentPlayer)->pos;
		dist.y = 0.0f;
		m_val2f4 = dist.Magnitude();
		sClubSet* pClubSet = GolfClub().FindClubSet(
			Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].clubInfo.tid);
		m_pClubPuppet = pClubSet->pHead[GolfClub().GetType()];
		CSceneManager::Instance()->SetVisible("\xc5\xac\xb7\xb4", true);
		ParseBoundingBoxScript(GOLFDOC()->m_currentPlayer, true, false, true);
		m_bPowerShot = false;
		m_bFlag329 = false;
		{
			// 983 0074636a..: anel com efeito (CSharedDoc 00633480/00633500, vetor +0x114b8) -> 0x335 para si mesmo
			int ring = AuxPartEffectIndex983(GOLFDOC()->m_currentPlayer);
			if (ring != -1)
			{
				HandleMsg(MsgObject(NULL, 0x335, AuxPartEffectTid983(ring), 0, 0, 0, 0));
			}
		}
		// 983 007465e0..: asas "angeliceffect" de cada jogador com boneco (sPetInfo +0x38) e flags de sUserInfo +0x92
		// (CSharedDoc +0x3942 + i*0x31c6) com o bit 0x40 e sem o 0x20 (os bits do 645 nesse byte nao batem, ver intrusion.cpp)
		C_ASSERT(offsetof(sUserInfo, info.dwEmblemVer) + 4 == 0x92);
		for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
		{
			unsigned char flags = *(reinterpret_cast<unsigned char*>(&Doc()->m_userInfo[i].info.dwEmblemVer) + 4);
			if (m_pPetInfo[i].m_bFlag0 && (flags & 0x20) == 0 && (flags & 0x40) != 0)
			{
				m_pFxInterpreter983->ShowNamed("angeliceffect", true, i);
			}
		}
		break;
	}
	// 983 00744b80: efeitos nomeados do FxInterpreter (+0x20) do jogador da vez; 0x335 = anel da mao esquerda
	case 0x335:
		switch ((unsigned long)msg.param1)
		{
		case 0x70210000:
		case 0x70210001:
		case 0x70210005:
		case 0x70210006:
			m_pFxInterpreter983->ShowNamed(
				"\277\336\274\325\271\335\301\366\310\277\260\372", true, // 00d20694 "왼손반지효과"
				GOLFDOC()->m_currentPlayer);
		}
		break;
	case 0x336:
		if (msg.param1)
		{
			m_pFxInterpreter983->ShowNamed((const char*)msg.param1, true, GOLFDOC()->m_currentPlayer);
		}
		break;
	case 0x337:
		m_pFxInterpreter983->ShowAllNamed(false, GOLFDOC()->m_currentPlayer);
		break;
	case 0x15a: // 983 00744b80 (sem par no 645)
		m_ringFxTid983[GOLFDOC()->m_currentPlayer] = 0;
		break;
	case 0x17c: // 983 0088c420(-1) no FxInterpreter
		m_pFxInterpreter983->DetachFxAll(-1);
		break;
	case 0xc7: // 983 00747826: FxInterpreter::SetMode (0088c460), como CGolfCaddie/gui_control
		m_pFxInterpreter983->SetMode(msg.param1);
		break;
	case 0x31b: // 983 006e0930 + 006de790: reabre o efeito estendido param2 (0 = todos) do jogador param1
		m_extEffect.RegistExtenedEffect(&m_pPetFrame[GOLFDOC()->m_currentPlayer], true);
		m_extEffect.ReopenEffect983(1, (unsigned long)msg.param1, (unsigned long)msg.param2);
		break;
	case M983(0x111):
		*(WBone**)msg.param1 =
			m_pPetFrame[GOLFDOC()->m_currentPlayer].GetBonePet()->FindBone(
				(const char*)msg.param2);
		break;
	case M983(0x113):
		*(float*)msg.param1 =
			Between(-20.0f * g_DEGTORAD, m_footAngle, 20.0f * g_DEGTORAD);
		break;
	case M983(0x11f):
		if (msg.param1)
		{
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_val1 = 4;
			float alpha = 1.0f;
			GolfClub().SetPowerShot(msg.param1);
			SendMsg(this, "Spot", M983(0xf5), 0, 0, 0, 0);
			SendMsg(this, "Screen", M983(0x1aa), (int)&alpha, 0, 0, 0);
			SetMotion(GolfClub().PowerShotType() == 1
					? MakeStr("%s\xc6\xc4\xbf\xf6\xc1\xd8\xba\xf1\xb3\xa1",
						  m_swingTypeName)
					: MakeStr("%s\xc6\xc4\xbf\xf6"
							  "2\xc1\xd8\xba\xf1\xb3\xa1",
						  m_swingTypeName),
				0.0f, -1);
		}
		else
		{
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_val1 = 0;
			float alpha = 0.0f;
			GolfClub().SetPowerShot(0);
			SendMsg(this, "Spot", M983(0xf5), 0, 0, 0, 0);
			SendMsg(this, "Screen", M983(0x1aa), (int)&alpha, 0, 0, 0);
			SetMotion(MakeStr("%s\xc1\xd8\xba\xf1", m_swingTypeName), 0.0f, -1);
		}
		break;
	case M983(0x115):
		GolfBall().m_standGround = HasStandGround(*(const WVector*)msg.param1);
		break;
	case M983(0x77):
	{
		if ((GolfClub().GetType() == 0 || GolfClub().GetType() == 1) &&
			GetBarAccuracyGrade() < 2)
		{
			if (GOLFDOC()->m_bar.impact > GOLFDOC()->m_bar.center)
			{
				strcpy(m_motionName,
					MakeStr("%s%s\xba\xfc\xb8\xa5\xc7\xea\xbd\xba\xc0\xae",
						m_swingTypeName,
						GolfClub().PowerShotType() != 0 ? "\xc6\xc4\xbf\xf6"
														: ""));
			}
			else
			{
				strcpy(m_motionName,
					MakeStr("%s%s\xb4\xca\xc0\xba\xc7\xea\xbd\xba\xc0\xae",
						m_swingTypeName,
						GolfClub().PowerShotType() != 0 ? "\xc6\xc4\xbf\xf6"
														: ""));
			}
		}
		else if (GolfClub().PowerShotType() != 0)
		{
			if (GolfClub().PowerShotType() != 1 && GolfClub().GetType() == 0)
			{
				strcpy(m_motionName,
					MakeStr("%s\xc6\xc4\xbf\xf6"
							"2",
						m_swingTypeName));
			}
			else
			{
				strcpy(m_motionName,
					MakeStr("%s\xc6\xc4\xbf\xf6", m_swingTypeName));
			}
		}
		else
		{
			if (GolfClub().GetType() == 3 && m_val2f4 < 3.2f &&
				Abs(CalcDeltaAngle(GolfBall().m_pos - GetPinPos(),
					m_pPetInfo[GOLFDOC()->m_currentPlayer].m_matrix.za)) <
					0.5235988f)
			{
				strcpy(m_motionName, "\xc2\xaa\xc0\xba\xc6\xdb\xc6\xc3");
			}
			else
			{
				strcpy(m_motionName, m_swingTypeName);
			}
		}
		const char* name;
		if (GolfClub().PowerShotType() != 0)
		{
			if (GolfClub().PowerShotType() == 1)
			{
				name = MakeStr("%s\xc6\xc4\xbf\xf6\xc1\xd8\xba\xf1\xb3\xa1",
					m_swingTypeName);
			}
			else
			{
				name = MakeStr("%s\xc6\xc4\xbf\xf6"
							   "2\xc1\xd8\xba\xf1\xb3\xa1",
					m_swingTypeName);
			}
		}
		else
		{
			name = MakeStr("%s\xc1\xd8\xba\xf1", m_swingTypeName);
		}
		w_motion_data* pMotion =
			m_pPetFrame[GOLFDOC()->m_currentPlayer].FindMotionData(name, true,
				NULL);
		if (pMotion->e - pMotion->s > 1.0f)
		{
			m_bFlag304 = true;
			SetMotion(pMotion->name, pMotion->e - pMotion->s - 1.0f, -1);
			strcpy(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_nextMotion,
				m_motionName);
		}
		else
		{
			m_bFlag304 = false;
		}
		break;
	}
	case M983(0x117):
	{
		unsigned long tid = msg.param1;
		if ((tid & 0xfc000000) == 0x70000000)
		{
			unsigned long* pAux =
				&Doc()
					 ->m_userInfo[GOLFDOC()->m_currentPlayer]
					 .charInfo.tidAuxParts[(tid >> 21) & 0x1f];
			*pAux = *pAux != tid ? tid : 0;
			GOLFDOC()->SetPlayerLevel(GOLFDOC()->m_currentPlayer);
			SendMsg(this, "Spot", M983(0xf5), 0, 0, 0, 0);
			ParseBoundingBoxScript(GOLFDOC()->m_currentPlayer, true, false,
				true);
			break;
		}
		// 983 00744b80 case 0x145: tipo 2 = peca (SetPart(.., 1), sem o Build do ShowAbilityParts do 645);
		// tipo 0xf = estilo de cabelo (cor do sHairStyle no CPartTidList e SetHairClr 008af6b0); outros tipos: nada
		CPartTidList* pTids =
			&GOLFDOC()->GetPartTidList(GOLFDOC()->m_currentPlayer);
		CItemManager& itemManager = Doc()->m_itemManager;
		if ((tid >> 26) == 0xf)
		{
			IFF_STRUCT::sHairStyle* pHair = itemManager.FindHairStyle(tid);
			if (!pHair)
				break;
			unsigned char color = pHair->cHairID;
			pTids->m_hairColor = color;
			m_pPetFrame[GOLFDOC()->m_currentPlayer].SetHairClr983(pTids, color, 0, true);
			break;
		}
		if ((tid >> 26) != 2 || !itemManager.FindPart(tid))
		{
			break;
		}
		unsigned long changed =
			m_pPetFrame[GOLFDOC()->m_currentPlayer].SetPart(pTids, tid, 0, true);
		m_pPetFrame[GOLFDOC()->m_currentPlayer].GetBonePet()->xUpdateTnLBuffers(
			true);
		if (changed)
		{
			m_pPetFrame[GOLFDOC()->m_currentPlayer].CalcBVolume();
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags |= 0x13;
		}
		ParseBoundingBoxScript(GOLFDOC()->m_currentPlayer, true, false, true);
		break;
	}
	case M983(0x118):
		for (int i = 0; i < GOLFDOC()->GetPlayerNum(); i++)
		{
			if (PLAYER(i)->state != 3)
			{
				m_pPetFrame[GOLFDOC()->m_currentPlayer]
					.GetBonePet()
					->SetRenderMode(msg.param1);
			}
		}
		break;
	case M983(0x119):
	{
		std::string motion;
		bool first = true;
		for (int i = 0; i < GOLFDOC()->GetPlayerNum(); i++)
		{
			if (Doc()->m_golfGame.gameType != 1 && PLAYER(i)->state == 3)
			{
				continue;
			}
			unsigned char index = i;
			const char* pointName;
			if (Doc()->m_golfGame.gameType == 1)
			{
				if (PLAYER(index)->teammate != 0xff)
				{
					if (GOLFDOC()->GetTeamGameWinner() == 2)
					{
						if (PLAYER(index)->team == 0)
						{
							pointName = i < PLAYER(index)->teammate
								? "\xc4\xbf\xc7\xc3_1\xb5\xee"
								: "\xc4\xbf\xc7\xc3_2\xb5\xee";
						}
						else
						{
							pointName = i < PLAYER(index)->teammate
								? "\xc4\xbf\xc7\xc3_3\xb5\xee"
								: "\xc4\xbf\xc7\xc3_4\xb5\xee";
						}
					}
					else if (PLAYER(index)->team ==
						(unsigned char)GOLFDOC()->GetTeamGameWinner())
					{
						pointName = i < PLAYER(index)->teammate
							? "\xc4\xbf\xc7\xc3_1\xb5\xee"
							: "\xc4\xbf\xc7\xc3_2\xb5\xee";
					}
					else
					{
						pointName = i < PLAYER(index)->teammate
							? "\xc4\xbf\xc7\xc3_3\xb5\xee"
							: "\xc4\xbf\xc7\xc3_4\xb5\xee";
					}
				}
				else if (GOLFDOC()->GetTeamGameWinner() == 2)
				{
					pointName = PLAYER(index)->team == 0
						? "\xc4\xbf\xc7\xc3_1\xb5\xee"
						: "\xc4\xbf\xc7\xc3_3\xb5\xee";
				}
				else
				{
					pointName = PLAYER(index)->team ==
							(unsigned char)GOLFDOC()->GetTeamGameWinner()
						? "\xc4\xbf\xc7\xc3_1\xb5\xee"
						: "\xc4\xbf\xc7\xc3_3\xb5\xee";
				}
			}
			else
			{
				pointName = MakeStr("%d\xc0\xce_%d\xb5\xee",
					GOLFDOC()->GetCurrentPlayerNum(), PLAYER(index)->order);
			}
			sSoupPoint* pPoint =
				((WPolySoupEx*)GOLFDOC()->m_pPolySoup)->FindPoint(pointName);
			m_pPetFrame[i].GetBonePet()->ShowBone(NULL);
			m_pPetFrame[i].GetBonePet()->HideBone("Bone01");
			m_pPetFrame[i].SetAlpha(0xff);
			CSceneManager::Instance()->SetVisible(MakeStr("PLAYER_%d", i),
				true);
			m_pPetInfo[i].m_matrix.Reset();
			m_pPetInfo[i].m_matrix.pivot =
				GetPVS().GetGroundPoint(pPoint->pos, false, NULL, false);
			m_pPetInfo[i].m_val1 = 0;
			m_pPetInfo[i].m_nextMotion[0] = 0;
			if (Doc()->m_golfGame.gameType != 1)
			{
				if (PLAYER(index)->rank == 1)
				{
					motion = "1\xb5\xee\xb8\xf0\xbc\xc7";
				}
				else
				{
					unsigned char playerNum = GOLFDOC()->m_playerNum;
					if (PLAYER(index)->rank == playerNum)
					{
						motion = "3\xb5\xee\xb8\xf0\xbc\xc7";
					}
					else if (PLAYER(index)->rank == 2)
					{
						motion = "2\xb5\xee\xb8\xf0\xbc\xc7";
					}
					else
					{
						motion = "3\xb5\xee\xb8\xf0\xbc\xc7";
					}
				}
			}
			else if (PLAYER(index)->team ==
					(unsigned char)GOLFDOC()->GetTeamGameWinner() ||
				GOLFDOC()->GetTeamGameWinner() == 2)
			{
				motion = "1\xb5\xee\xb8\xf0\xbc\xc7";
			}
			else
			{
				motion = "3\xb5\xee\xb8\xf0\xbc\xc7";
			}
			m_pPetInfo[i].m_pMotion =
				m_pPetFrame[i].FindMotionData(motion.c_str(), true, NULL);
			m_pPetInfo[i].m_motionTime = m_pPetInfo[i].m_pMotion->s;
			m_pPetInfo[i].m_bFlag0 = true;
			m_pPetFrame[i].GetBonePet()->SetMatrix(m_pPetInfo[i].m_matrix);
			m_pPetFrame[i].Animate983(NULL, NULL, m_pPetInfo[i].m_motionTime);
			m_lightSet.nearOne = g_lightset.nearOne;
			m_lightSet.ambient = ModifyColor(g_lightset.ambient, 1.0f);
			SetLight();
			char charName[16];
			sprintf(charName, "char%d", i);
			m_pPetFrame[i].SetInvisible(false);
			ScriptProcess(m_pPetInfo[i].m_pMotion, m_pPetInfo[i].m_pMotion->s,
				m_pPetInfo[i].m_pMotion->s, i, true, false);
			// 983 00744b80 case 0x147: depois do ScriptProcess (0073dee0) chama 0073e590(i, 1, 0, primeiro) para cada jogador;
			// primeiro = 1o jogador desenhado ([ebp-0x65]), nao i == 0: com o jogador 0 fora o placar final nao tinha primeiro
			ParseBoundingBoxScript(i, true, false, first);
			first = false;
		}
		((CSceneManagerEx*)CSceneManager::Instance())
			->SetPuppet("\xc5\xac\xb7\xb4", NULL);
		break;
	}
	case M983(0x11c):
	{
		unsigned long tid = msg.param1;
		sClubSet* pClubSet = GolfClub().AddClubSet(tid);
		if (!pClubSet)
		{
			break;
		}
		Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].clubInfo.tid = tid;
		m_pClubPuppet = pClubSet->pHead[GolfClub().GetType()];
		if (msg.param2)
		{
			RegisterClub(0);
		}
		((CSceneManagerEx*)CSceneManager::Instance())
			->SetPuppet("\xc5\xac\xb7\xb4", m_pClubPuppet);
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags |= 0x269;
		m_ballDist = m_clubs[GOLFDOC()->m_currentPlayer][GolfClub().GetType()]
						 .m_ballPos.x;
		break;
	}
	case M983(0x49):
	{
		CPartTidList* pTids = &GOLFDOC()->GetPartTidList(0);
		// 983 00744c02: nivel do jogador 0 = Doc+0x3a2d (m_userInfo[0].stat.Level; 645 Doc+0x1429)
		C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, stat) +
			offsetof(sPangYaUserStatistics, Level) == 0x3a2d);
		pTids->SetTids(&Doc()->m_userInfo[0].charInfo,
			Doc()->m_userInfo[0].stat.Level);
		if (!m_pPetFrame[0].Build(pTids,
				Doc()->m_userInfo[0].charInfo.tidAuxParts, true, true, -1))
		{
			pTids->SetDefaultTids();
			m_pPetFrame[0].Build(pTids,
				Doc()->m_userInfo[0].charInfo.tidAuxParts, true, false, -1);
		}
		m_pPetFrame[0].CalcBVolume();
		((CSceneManagerEx*)CSceneManager::Instance())
			->SetPuppet("PLAYER_0", m_pPetFrame[0].GetBonePet());
		RegisterClub(0);
		break;
	}
	case M983(0x121):
	{
		m_pPetFrame[MyIndex()].GetBonePet()->ShowBone(NULL);
		m_pPetFrame[MyIndex()].GetBonePet()->HideBone("Bone01");
		m_pPetFrame[MyIndex()].SetAlpha(0xff);
		m_pPetInfo[MyIndex()].m_matrix.Reset();
		m_pPetInfo[MyIndex()].m_matrix.pivot = GetPVS().GetGroundPoint(
			*(const WVector*)msg.param1, false, NULL, false);
		m_pPetInfo[MyIndex()].m_val1 = 0;
		m_pPetInfo[MyIndex()].m_nextMotion[0] = 0;
		m_pPetInfo[MyIndex()].m_pMotion = m_pPetFrame[MyIndex()].FindMotionData(
			(const char*)msg.param2, true, NULL);
		if (m_pPetInfo[MyIndex()].m_pMotion)
		{
			m_pPetInfo[MyIndex()].m_motionTime =
				m_pPetInfo[MyIndex()].m_pMotion->s;
		}
		else
		{
			m_pPetInfo[MyIndex()].m_motionTime = 0.0f;
		}
		m_pPetInfo[MyIndex()].m_bFlag0 = true;
		m_pPetFrame[MyIndex()].GetBonePet()->SetMatrix(m_pPetInfo[MyIndex()].m_matrix);
		m_pPetFrame[MyIndex()].Animate983(NULL, NULL, m_pPetInfo[MyIndex()].m_motionTime);
		m_pPetInfo[MyIndex()].m_flags |= 0x48;
		CSceneManager::Instance()->SetVisible(MakeStr("PLAYER_%d", MyIndex()),
			true);
		sClubSet* pClubSet =
			GolfClub().FindClubSet(Doc()->m_userInfo[MyIndex()].clubInfo.tid);
		WPuppet* pClub = pClubSet->pHead[GolfClub().GetType()];
		CSceneManager::Instance()->SetVisible("\xc5\xac\xb7\xb4", true);
		((CSceneManagerEx*)CSceneManager::Instance())
			->SetPuppet("\xc5\xac\xb7\xb4", pClub);
		m_pClubPuppet = pClub;
		m_pPetFrame[MyIndex()].SetInvisible(false);
		ScriptProcess(m_pPetInfo[MyIndex()].m_pMotion,
			m_pPetInfo[MyIndex()].m_pMotion->s,
			m_pPetInfo[MyIndex()].m_pMotion->s, MyIndex(), true, false);
		m_pPetInfo[1 - MyIndex()].m_bFlag0 = false;
		CSceneManager::Instance()->SetVisible(
			MakeStr("PLAYER_%d", 1 - MyIndex()), false);
		m_lightSet.nearOne = g_lightset.nearOne;
		m_lightSet.ambient = ModifyColor(g_lightset.ambient, 1.0f);
		SetLight(MyIndex());
		ParseBoundingBoxScript(MyIndex(), true, false, true);
		break;
	}
	case M983(0x120):
	{
		const char* motion[2];
		motion[0] = (const char*)msg.param1;
		motion[1] = (const char*)msg.param2;
		WVector pos;
		pos = *(const WVector*)msg.param3;
		// HACK: two unused locals shift the compiler's temporary numbering (x87 operand order of `pos + WVector`)
		WVector rbUnused0;
		WVector rbUnused1;
		for (int i = 0; i < 2; i++)
		{
			int index;
			if (PLAYER(MyIndex())->teammate != 0xff)
			{
				index = i == 0 ? MyIndex() : PLAYER(MyIndex())->teammate;
			}
			else
			{
				if (i == 1)
				{
					break;
				}
				index = MyIndex();
			}
			CPetFrame* pFrame = &m_pPetFrame[index];
			sPetInfo* pInfo = &m_pPetInfo[index];
			pFrame->GetBonePet()->ShowBone(NULL);
			pFrame->GetBonePet()->HideBone("Bone01");
			pFrame->SetAlpha(0xff);
			pInfo->m_matrix.Reset();
			pInfo->m_matrix.pivot = GetPVS().GetGroundPoint(pos +
					WVector(i == 0 ? -3.0f : 3.0f, 0.0f, 0.0f),
				false, NULL, false);
			pInfo->m_val1 = 0;
			pInfo->m_nextMotion[0] = 0;
			pInfo->m_pMotion = pFrame->FindMotionData(motion[i], true, NULL);
			if (pInfo->m_pMotion)
			{
				pInfo->m_motionTime = pInfo->m_pMotion->s;
			}
			else
			{
				pInfo->m_motionTime = 0.0f;
			}
			pInfo->m_bFlag0 = true;
			pFrame->GetBonePet()->SetMatrix(pInfo->m_matrix);
			pFrame->Animate983(NULL, NULL, pInfo->m_motionTime);
			m_pPetInfo[index].m_flags |= 0x48;
			const char* name = MakeStr("PLAYER_%d", index);
			CSceneManager::Instance()->SetVisible(name, true);
			if (index == TEAM(PLAYER(MyIndex())->team)->shooter)
			{
				sClubSet* pClubSet = GolfClub().FindClubSet(
					Doc()->m_userInfo[index].clubInfo.tid);
				if (!pClubSet)
				{
					continue;
				}
				WPuppet* pClub;
				if (i == GOLFDOC()->m_currentPlayer)
				{
					pClub = pClubSet->pHead[GolfClub().GetType()];
				}
				else
				{
					pClub =
						pClubSet->pHead[TEAM(PLAYER(MyIndex())->team)->club];
				}
				if (!pClub)
				{
					continue;
				}
				CSceneManager::Instance()->SetVisible("\xc5\xac\xb7\xb4", true);
				((CSceneManagerEx*)CSceneManager::Instance())
					->SetPuppet("\xc5\xac\xb7\xb4", pClub);
				m_pClubPuppet = pClub;
			}
			m_pPetFrame[index].SetInvisible(false);
			ScriptProcess(m_pPetInfo[index].m_pMotion,
				m_pPetInfo[index].m_pMotion->s, m_pPetInfo[index].m_pMotion->s,
				index, true, false);
		}
		for (int j = 0; j < GOLFDOC()->GetPlayerNum(); j++)
		{
			if (j == MyIndex())
			{
				continue;
			}
			if ((PLAYER(MyIndex())->teammate == 0xff ||
					j == PLAYER(MyIndex())->teammate) &&
				PLAYER(MyIndex())->teammate != 0xff)
			{
				continue;
			}
			m_pPetInfo[j].m_bFlag0 = false;
			CSceneManager::Instance()->SetVisible(MakeStr("PLAYER_%d", j),
				false);
		}
		m_lightSet.nearOne = g_lightset.nearOne;
		m_lightSet.ambient = ModifyColor(g_lightset.ambient, 1.0f);
		SetLight(MyIndex());
		ParseBoundingBoxScript(MyIndex(), true, false, true);
		if (PLAYER(MyIndex())->teammate != 0xff)
		{
			ParseBoundingBoxScript(PLAYER(MyIndex())->teammate, false, false,
				false);
		}
		break;
	}
	case M983(0x122):
	{
		MyIndex();
		bool bShowClub = msg.param1 != 0;
		m_pPetFrame[MyIndex()].GetBonePet()->ShowBone(NULL);
		m_pPetFrame[MyIndex()].GetBonePet()->HideBone("Bone01");
		m_pPetFrame[MyIndex()].SetAlpha(0xff);
		m_pPetInfo[MyIndex()].m_matrix.Reset();
		m_pPetInfo[MyIndex()].m_matrix.pivot = GetPVS().GetGroundPoint(
			*(const WVector*)msg.param1, false, NULL, false);
		m_pPetInfo[MyIndex()].m_val1 = 0;
		m_pPetInfo[MyIndex()].m_nextMotion[0] = 0;
		m_pPetInfo[MyIndex()].m_pMotion = m_pPetFrame[MyIndex()].FindMotionData(
			(const char*)msg.param2, true, NULL);
		if (m_pPetInfo[MyIndex()].m_pMotion)
		{
			m_pPetInfo[MyIndex()].m_motionTime =
				m_pPetInfo[MyIndex()].m_pMotion->s;
		}
		else
		{
			m_pPetInfo[MyIndex()].m_motionTime = 0.0f;
		}
		m_pPetInfo[MyIndex()].m_bFlag0 = true;
		m_pPetFrame[MyIndex()].GetBonePet()->SetMatrix(m_pPetInfo[MyIndex()].m_matrix);
		m_pPetFrame[MyIndex()].Animate983(NULL, NULL, m_pPetInfo[MyIndex()].m_motionTime);
		m_pPetInfo[MyIndex()].m_flags |= 0x48;
		for (int i = 0; i < GOLFDOC()->GetPlayerNum(); i++)
		{
			m_pPetInfo[i].m_bFlag0 = i == MyIndex();
			CSceneManager::Instance()->SetVisible(MakeStr("PLAYER_%d", i),
				i == MyIndex());
		}
		CSceneManager::Instance()->SetVisible("\xc5\xac\xb7\xb4", bShowClub);
		sClubSet* pClubSet;
		WPuppet* pClub;
		if (bShowClub &&
			(pClubSet = GolfClub().FindClubSet(
				 Doc()->m_userInfo[MyIndex()].clubInfo.tid)) != NULL &&
			(pClub = pClubSet->pHead[GolfClub().GetType()]) != NULL)
		{
			((CSceneManagerEx*)CSceneManager::Instance())
				->SetPuppet("\xc5\xac\xb7\xb4", pClub);
			m_pClubPuppet = pClub;
		}
		else
		{
			m_pClubPuppet = NULL;
		}
		m_pPetFrame[MyIndex()].SetInvisible(false);
		ScriptProcess(m_pPetInfo[MyIndex()].m_pMotion,
			m_pPetInfo[MyIndex()].m_pMotion->s,
			m_pPetInfo[MyIndex()].m_pMotion->s, MyIndex(), true, false);
		m_lightSet.nearOne = g_lightset.nearOne;
		m_lightSet.ambient = ModifyColor(g_lightset.ambient, 1.0f);
		SetLight(MyIndex());
		ParseBoundingBoxScript(MyIndex(), true, false, true);
		break;
	}
	case M983(0x127):
		if (msg.param2 == 1)
		{
			*(WVector*)msg.param1 =
				m_pPetInfo[GOLFDOC()->m_currentPlayer].m_matrix.pivot;
		}
		else
		{
			*(WVector*)msg.param1 = m_pos;
		}
		break;
	case M983(0x124):
		*(WMatrix*)msg.param2 = m_pPetFrame[msg.param1]
									.GetBonePet()
									->FindBone("Bip01 Head")
									->GetMatrix();
		break;
	case M983(0x126):
	{
		if ((GolfClub().GetType() == 0 || GolfClub().GetType() == 1) &&
			GetBarAccuracyGrade() < 2)
		{
			if (GOLFDOC()->m_bar.impact > GOLFDOC()->m_bar.center)
			{
				strcpy(m_motionName,
					MakeStr("%s%s\xba\xfc\xb8\xa5\xc7\xea\xbd\xba\xc0\xae",
						m_swingTypeName,
						GolfClub().PowerShotType() != 0 ? "\xc6\xc4\xbf\xf6"
														: ""));
			}
			else
			{
				strcpy(m_motionName,
					MakeStr("%s%s\xb4\xca\xc0\xba\xc7\xea\xbd\xba\xc0\xae",
						m_swingTypeName,
						GolfClub().PowerShotType() != 0 ? "\xc6\xc4\xbf\xf6"
														: ""));
			}
		}
		else if (GolfClub().PowerShotType() != 0)
		{
			strcpy(m_motionName,
				MakeStr("%s\xc6\xc4\xbf\xf6", m_swingTypeName));
		}
		else
		{
			if (GolfClub().GetType() == 3 && m_val2f4 < 3.2f &&
				Abs(CalcDeltaAngle(GolfBall().m_pos - GetPinPos(),
					m_pPetInfo[GOLFDOC()->m_currentPlayer].m_matrix.za)) <
					0.5235988f)
			{
				strcpy(m_motionName, "\xc2\xaa\xc0\xba\xc6\xdb\xc6\xc3");
			}
			else
			{
				strcpy(m_motionName, m_swingTypeName);
			}
		}
		for (unsigned int i = 0; i < 16; i++)
		{
			if (strcmp(s_motionName[i], m_motionName) == 0)
			{
				*(float*)msg.param1 =
					m_pPetInfo[GOLFDOC()->m_currentPlayer].m_shotTimes[i];
				break;
			}
		}
		break;
	}
	case M983(0x123):
	{
		CPartTidList* pTids =
			&GOLFDOC()->GetPartTidList(GOLFDOC()->m_currentPlayer);
		if (GolfClub().PowerShotType() != 0)
		{
			if ((pTids->m_charTid & 0x3ffffff) == 4)
			{
				if (GolfClub().PowerShotType() == 1)
				{
					strcpy(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_nextMotion,
						"\xbf\xec\xb5\xe5\xbc\xa6\xc6\xc4\xbf\xf6\xc8\xc4\xb8\xf0\xbc\xc7");
				}
				else
				{
					strcpy(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_nextMotion,
						"\xbf\xec\xb5\xe5\xbc\xa6\xc6\xc4\xbf\xf6"
						"2\xc8\xc4\xb8\xf0\xbc\xc7");
				}
			}
			else if (GolfClub().PowerShotType() == 1)
			{
				strcpy(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_nextMotion,
					"\xbf\xec\xb5\xe5\xbc\xa6\xc8\xc4\xb8\xf0\xbc\xc7");
			}
			else
			{
				strcpy(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_nextMotion,
					"\xbf\xec\xb5\xe5\xbc\xa6\xc6\xc4\xbf\xf6"
					"2\xc8\xc4\xb8\xf0\xbc\xc7");
			}
		}
		else
		{
			strcpy(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_nextMotion,
				"\xbf\xec\xb5\xe5\xbc\xa6\xc8\xc4\xb8\xf0\xbc\xc7");
		}
		break;
	}
	case M983(0x125):
		if (!msg.param1)
		{
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags |= 0x80;
			CSceneManager::Instance()->SetVisible("\xc5\xac\xb7\xb4", false);
		}
		else
		{
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags &= ~0x80;
			CSceneManager::Instance()->SetVisible("\xc5\xac\xb7\xb4", true);
		}
		break;
	case M983(0x128):
		if (m_pPetInfo && m_pPetInfo[GOLFDOC()->m_currentPlayer].m_pMotion &&
			(strstr(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_pMotion->name,
				 "\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee") ||
				strstr(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_pMotion->name,
					"\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee") ||
				strstr(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_pMotion->name,
					"\xb5\xee\xc0\xe5\xb8\xf0\xbc\xc7") ||
				strstr(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_pMotion->name,
					"\xb5\xee\xb8\xf0\xbc\xc7")))
		{
			*(WVector*)msg.param1 = WVector::ZERO;
		}
		else
		{
			*(WVector*)msg.param1 =
				m_clubs[GOLFDOC()->m_currentPlayer][GolfClub().GetType()]
					.m_ballPos;
		}
		break;
	case M983(0x129):
		*(int*)msg.param1 = m_bCombo;
		*(float*)msg.param2 = m_comboTime;
		break;
	case M983(0x157):
		if (msg.param1 != -1)
		{
			PLAYER(0)->power = msg.param1;
		}
		if (msg.param2 != -1)
		{
			PLAYER(0)->control = msg.param2;
		}
		if (msg.param3 != -1)
		{
			PLAYER(0)->curve = msg.param3;
		}
		break;
	case M983(0x1a1):
	{
		if (!m_bSwingPending)
		{
			if (GolfClub().PowerShotType() != 0 && GolfClub().GetType() != 3)
			{
				if (GolfClub().PowerShotType() != 1 &&
					GolfClub().GetType() == 0)
				{
					SetMotion(
						MakeStr("%s\xb0\xd4\xb0\xc9\xc0\xbd\xc6\xc4\xbf\xf6"
								"2",
							m_swingTypeName),
						0.0f, -1);
				}
				else
				{
					SetMotion(
						MakeStr("%s\xb0\xd4\xb0\xc9\xc0\xbd\xc6\xc4\xbf\xf6",
							m_swingTypeName),
						0.0f, -1);
				}
			}
			else
			{
				float alpha = 0.0f;
				SendMsg(this, "Screen", M983(0x1aa), (int)&alpha, 0, 0, 0);
				m_pPetInfo[GOLFDOC()->m_currentPlayer].m_val1 = 0;
				SetMotion(
					MakeStr("%s\xb0\xd4\xb0\xc9\xc0\xbd", m_swingTypeName),
					0.0f, -1);
			}
		}
		// 983 00744b80 case 0x213: sem o reposicionamento da seq do item 0x18000028 no chao (645);
		// o safetee_Avatar.seq do 0x280 ja vai preso ao osso "Bip01 Footsteps" (B1405)
		m_bFlag190 = true;
		m_angle = *(float*)msg.param1 + m_angle;
		m_matrix = RotMat(m_angle, 1);
		SearchGround();
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags |= 0x40;
		if (!IsMyTurn(false))
		{
			break;
		}
		if (m_bFlag15e)
		{
			float time = 0.0f;
			SendMsg(this, "Letter", M983(0x18d), (int)"NoPlay", (int)&time, 1, 0);
			SendMsg(this, "Caddie", M983(0x1e1), 1, 0, 0, 0);
		}
		else
		{
			SendMsg(this, "Letter", M983(0x18c), (int)"NoPlay", 0, 0, 0);
			SendMsg(this, "Caddie", M983(0x1e1), 0, 0, 0, 0);
		}
		break;
	}
	case M983(0x163):
		m_bFlag15d = true;
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_bFlag0 = true;
		m_pPetFrame[GOLFDOC()->m_currentPlayer].GetBonePet()->ShowBone(NULL);
		CSceneManager::Instance()->SetVisible(
			MakeStr("PLAYER_%d", GOLFDOC()->m_currentPlayer), true);
		CSceneManager::Instance()->SetVisible("\xc5\xac\xb7\xb4", true);
		break;
	case M983(0x164):
	{
		m_bFlag15d = false;
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_bFlag0 = false;
		m_pPetFrame[GOLFDOC()->m_currentPlayer].GetBonePet()->HideBone(NULL);
		CSceneManager::Instance()->SetVisible(
			MakeStr("PLAYER_%d", GOLFDOC()->m_currentPlayer), false);
		CSceneManager::Instance()->SetVisible("\xc5\xac\xb7\xb4", false);
		// 983 00747b9c: solta os fx do jogador no interpretador (m_sprays/m_sequences do 645 nunca eram preenchidos)
		m_pFxInterpreter983->DetachFxAll(GOLFDOC()->m_currentPlayer);
		break;
	}
	case M983(0x199):
		*(int*)msg.param1 = m_pPetInfo[GOLFDOC()->m_currentPlayer].m_val1;
		break;
	case M983(0x1f8):
	{
		int index = msg.param1;
		if (index >= GOLFDOC()->m_playerNum)
		{
			break;
		}
		CFxSequence* pSequence =
			CFx::Instance()->OpenSequence("Talk_Room.seq", true);
		if (!pSequence)
		{
			break;
		}
		pSequence->m_pos =
			m_pPetInfo[index].m_matrix.pivot + WVector(0.0f, 5.0f, 0.0f);
		break;
	}
	case M983(0x203):
	{
		// 983 (Player 0x280): evento 6 do jogador da vez (00663870), o 645 testava IsLocalContent + item 0x18000028;
		// liga: m_flags |= 1, matriz do pet (007410c0) e safetee_Avatar.seq preso ao osso "Bip01 Footsteps" (Attach 00879110)
		jp983_rastro("B1422 Player 0x280 p1=%d ev6=%d cur=%d f8=%08x", msg.param1, (int)GOLFDOC()->IsCurrentPlayerEvent983(6),
			(int)GOLFDOC()->m_currentPlayer, m_valF8); // temporario
		if (GOLFDOC()->IsCurrentPlayerEvent983(6))
		{
			if (msg.param1 == 0)
			{
				CFx::Instance()->CloseSequence((CFxSequence*)m_valF8, false);
			}
			else
			{
				m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags |= 1;
				UpdatePetMatrix983();
				Attacher attacher;
				WBone* pBone = m_pPetFrame[GOLFDOC()->m_currentPlayer].GetBonePet()->FindBone("Bip01 Footsteps");
				if (pBone)
				{
					attacher.mat[0] = &pBone->GetMatrix(); // bone+0x140
					m_valF8 = (int)CFx::Instance()->Attach(attacher, "safetee_Avatar.seq", true, NULL);
				}
				jp983_rastro("B1422 Player 0x280 bone=%p seq=%08x", pBone, m_valF8);
			}
		}
		else
		{
			CFx::Instance()->CloseSequence((CFxSequence*)m_valF8, false);
		}
		break;
	}
	case 0x2715:
		*(const char**)msg.param1 =
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_pMotion->name;
		break;
	case 0x2716:
		m_bFlag330 = msg.param3 == 1;
		SetMotion((const char*)msg.param1, (float)msg.param2, -1);
		m_bFlag330 = false;
		break;
	case 0x2718:
		m_bFlag190 = false;
		m_bSwingPending = false;
		m_angle = *(float*)msg.param1 + g_PI;
		m_matrix = RotMat(m_angle, 1);
		SearchGround();
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags |= 0x248;
		break;
	case 0x2719:
		m_pos = *(const WVector*)msg.param1;
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags |= 0x249;
		break;
	case 0x2717:
		if (msg.param1)
		{
			strcpy(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_nextMotion,
				(const char*)msg.param1);
		}
		else
		{
			strcpy(m_pPetInfo[GOLFDOC()->m_currentPlayer].m_nextMotion,
				MakeStr("%s\xc1\xd8\xba\xf1", m_swingTypeName));
		}
		break;
	}
}

void CGolfPlayer::SetMotion(const char* name, float time, int index)
{
	if (index < 0)
	{
		index = GOLFDOC()->m_currentPlayer;
	}
	if (IsLocalContent((localContentType_t)0x6d))
	{
		int count = m_pPetFrame[index].GetMotionCountIncludeName(name,
			s_motionName[16]);
		if (count > 1)
		{
			char motionName[0x80] = "";
			sprintf(motionName, "%s%02d", name, rand() % count + 1);
			m_pPetInfo[index].m_pMotion =
				m_pPetFrame[index].FindMotionData(motionName, true, NULL);
			if (m_pPetInfo[index].m_pMotion == NULL)
			{
				m_pPetInfo[index].m_pMotion =
					m_pPetFrame[index].FindMotionData(name, true, NULL);
			}
		}
		else
		{
			m_pPetInfo[index].m_pMotion =
				m_pPetFrame[index].FindMotionData(name, true, NULL);
		}
	}
	else
	{
		m_pPetInfo[index].m_pMotion =
			m_pPetFrame[index].FindMotionData(name, true, NULL);
	}
	if (m_pPetInfo[index].m_pMotion == NULL)
	{
		return;
	}
	m_pPetInfo[index].m_scale = 1.0f;
	m_pPetInfo[index].m_scaleRate = 0;
	if (time < 0.0f)
	{
		m_pPetInfo[index].m_motionTime = Max(m_pPetInfo[index].m_pMotion->s,
			time + m_pPetInfo[index].m_pMotion->e);
	}
	else
	{
		m_pPetInfo[index].m_motionTime = Min(m_pPetInfo[index].m_pMotion->e,
			time + m_pPetInfo[index].m_pMotion->s);
	}
	m_pPetInfo[index].m_flags |= 0x33;
	m_bFlag2f0 = ((m_pPetInfo[index].m_flags >> 8) & 1) != 0;
	m_pPetInfo[index].m_flags &= ~0x180;
	m_pPetFrame[index].SetInvisible(false);
	ScriptProcess(m_pPetInfo[index].m_pMotion, m_pPetInfo[index].m_pMotion->s,
		m_pPetInfo[index].m_pMotion->s, index, true, time < 0.0f);
	// 983 00742780: alem do G_Bird (char 6) esconde K_Ro_E (char 10) no "deungjang"; char 0xe esconde CC_glasses
	// na pose de vitoria quando tem a peca 10 equipada
	unsigned long tidChar = m_pPetFrame[index].GetTidChar() & 0x3ffffff;
	if ((tidChar == 6 || tidChar == 10) &&
		!(Wabs(time) < g_EPSILON) &&
		strcmpi(name, "\xb5\xee\xc0\xe5\xb8\xf0\xbc\xc7") == 0)
	{
		const char* boneName = tidChar == 6 ? "G_Bird" : "K_Ro_E";
		WBone* pBone = m_pPetFrame[index].GetBonePet()->FindBone(boneName);
		if (pBone != NULL)
		{
			pBone->SetInvisible(false, true);
		}
	}
	if (tidChar == 0xe &&
		strcmp(name, "\xbe\xcb\xb9\xd9\xc8\xa6\xc0\xce\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee") == 0)
	{
		if (GOLFDOC()->GetPartTidList(IsMassGame() ? 0 : MyIndex()).m_tid[10] != 0)
		{
			WBone* pBone = m_pPetFrame[index].GetBonePet()->FindBone("CC_glasses");
			if (pBone != NULL)
			{
				pBone->SetInvisible(false, true);
			}
		}
	}
	// 983 00742b3a: fecha com FxInterpreter::SetCond(pose, jogador) (008900e0), no lugar do m_condIds.clear() do 645
	if (m_pFxInterpreter983)
	{
		m_pFxInterpreter983->SetCond(name, index);
	}
	m_condIds[index].clear();
}

void CGolfPlayer::SetSwingType()
{
	switch (GolfClub().GetType())
	{
	case 0:
		strcpy(m_swingTypeName, "\xbf\xec\xb5\xe5\xbc\xa6");
		break;
	case 1:
		strcpy(m_swingTypeName, "\xbe\xc6\xc0\xcc\xbe\xf0\xbc\xa6");
		break;
	case 2:
		strcpy(m_swingTypeName, "\xbb\xf7\xb5\xe5\x9f\x87\xc1\xf6\xbc\xa6");
		break;
	case 3:
		strcpy(m_swingTypeName, "\xc6\xdb\xc6\xc3\xbc\xa6");
		break;
	}
	char* motion;
	if (GolfClub().PowerShotType())
	{
		if (GolfClub().PowerShotType() == 1)
		{
			motion = MakeStr("%s\xc6\xc4\xbf\xf6\xc1\xd8\xba\xf1\xb3\xa1",
				m_swingTypeName);
		}
		else
		{
			motion = MakeStr("%s\xc6\xc4\xbf\xf6"
							 "2\xc1\xd8\xba\xf1\xb3\xa1",
				m_swingTypeName);
		}
	}
	else
	{
		motion = MakeStr("%s\xc1\xd8\xba\xf1", m_swingTypeName);
	}
	SetMotion(motion, 0.0f, -1);
	m_ballDist =
		m_clubs[GOLFDOC()->m_currentPlayer][GolfClub().GetType()].m_ballPos.x;
	m_pPetInfo[GOLFDOC()->m_currentPlayer].m_val1 = 0;
}

void CGolfPlayer::ScriptProcess(w_motion_data* pMotion, float from, float to,
	int index, bool bAll, bool bSkip)
{
	if (pMotion == NULL)
	{
		return;
	}
	if ((!(from <= pMotion->e) || from == to) && !bAll)
	{
		return;
	}
	int start, end;
	from *= 30.0f;
	if (bAll)
	{
		start = Ceil(from, 0.03f);
		end = start;
	}
	else
	{
		to = Min(pMotion->e, to) * 30.0f;
		start = Ceil(from, 0.03f);
		end = Wabs((float)floor((double)to) - to) < g_EPSILON
			? (int)to - 1
			: (int)(float)floor((double)to);
		if (start ==
			float2int((float)floor((double)(pMotion->s * 30.0f + 0.5f))))
		{
			start++;
		}
		if (start > end)
		{
			return;
		}
	}
	{
		int first, last;
		if (m_pPetFrame[index].GetCurAniPet()->CheckFrameData(start, end, first,
				last))
		{
			for (int i = first; i <= last; i++)
			{
				ParsePuppetScript(
					m_pPetFrame[index].GetCurAniPet()->GetFrameData(i), index,
					bSkip);
			}
		}
	}
}

void CGolfPlayer::CloseBoundingBoxFx(int index, unsigned long id)
{
	if (id == 0 || index > 4)
	{
		return;
	}
	std::vector<sCondBBSpray>::iterator spray;
	for (spray = m_condSpray[index].begin(); spray != m_condSpray[index].end();)
	{
		if ((*spray).id == id)
		{
			CFx::Instance()->CloseSpray((*spray).pSpray);
			m_condSpray[index].erase(spray);
		}
		else
		{
			spray++;
		}
	}
	std::vector<sCondBBSequence>::iterator sequence;
	for (sequence = m_condSeq[index].begin();
		sequence != m_condSeq[index].end();)
	{
		if ((*sequence).id == id)
		{
			CFx::Instance()->CloseSequence((*sequence).pSequence, true);
			m_condSeq[index].erase(sequence);
		}
		else
		{
			sequence++;
		}
	}
}

// 983 (0073da60): o analisador saiu para IPetScriptObject::ParseScript, que chama um virtual OnScript* por palavra.
void CGolfPlayer::ParsePuppetScript(char* script, int index, bool bSkip)
{
	m_scriptIndex983 = index;
	m_pScriptFrame983 = &m_pPetFrame[index];
	m_pScriptFrame = m_pScriptFrame983;
	ParseScript(script, bSkip);
	m_bFlag2f0 = false;
}

// 983 (0073e0b0)
void CGolfPlayer::OnScriptSnd(const char*& p)
{
	static char sndName[0x40];
	p = GetStrConst(p, sndName);
	if (strlen(sndName) != 0 && strlen(sndName) < 64)
	{
		WVector pos;
		pos = m_pos - g_camera.pivot;
		g_audio->PlaySfx(sndName, &pos, 5, NULL, NULL, 0.5f, 200.0f);
	}
}

// 983 (00740c50): a mensagem ao GolfRule e 0x93 (645: 0x78)
void CGolfPlayer::OnScriptShot(const char*& p)
{
	PlayShotSfx(m_scriptIndex983);
	if (GOLFDOC()->m_gameMode & 0x40)
	{
		SendMsg(this, "GolfRule", 0x93/*983*/, 0x80, 0, 0, 0);
	}
}

// 983 (00741660, 335 B): conferido (b620/t.txt): tabela e17a80 = s_tailTexture, default ~swinglight01.jpg
void CGolfPlayer::OnScriptTail(const char*& p)
{
	int index = m_scriptIndex983;
	do
	{
		int j;
		sTailTexture* pTexture = s_tailTexture;
		for (j = 0; j < 3; j++, pTexture++)
		{
			if (Doc()
					->m_userInfo[GOLFDOC()->m_currentPlayer]
					.clubInfo.tid == pTexture->tid)
			{
				m_pTail->ChangeTexture(pTexture->texture);
				break;
			}
		}
		if (j == 3)
		{
			m_pTail->ChangeTexture("~swinglight01.jpg");
		}
		m_bFlag0 = true;
		CSceneManager::Instance()->SetVisible("tail", true);
		break;
	}
	while (0);
}

// 983 (0073e160): conferido b620
void CGolfPlayer::OnScriptShowClub(const char*& p)
{
	m_pPetInfo[m_scriptIndex983].m_flags &= ~0x80;
	CSceneManager::Instance()->SetVisible("\xc5\xac\xb7\xb4", true);
}

// 983 (0073e190): conferido b620
void CGolfPlayer::OnScriptHideClub(const char*& p)
{
	m_pPetInfo[m_scriptIndex983].m_flags |= 0x80;
	CSceneManager::Instance()->SetVisible("\xc5\xac\xb7\xb4", false);
}

// 983 (0073f420, 493 B): conferido (b620/d.txt): matriz do osso 'club' + grip, flags |= 0x100
void CGolfPlayer::OnScriptLockClub(const char*& p)
{
	int index = m_scriptIndex983;
	do
	{
		if (!m_bFlag2f0)
		{
			WPuppet* pPet = m_pPetFrame[index].GetBonePet();
			w_bound_box* pBox = pPet->m_bbList.Find("club");
			m_clubMatrix = pBox->bone->GetMatrix();
			m_clubMatrix.pivot =
				pBox->info->spherePivot * pBox->bone->GetMatrix();
			m_clubMatrix.pivot +=
				m_clubs[index][GolfClub().GetType()].m_gripPos *
				m_clubMatrix.xa;
		}
		m_pPetInfo[index].m_flags |= 0x100;
		break;
	} while (0);
}

// 983 00741890 (2175 B): "*fx(" do jogador. Condicao "@movimento" -> nome do efeito; "&tid" = grupo (peca vestida ou taco);
// o efeito vai para o FxInterpreter do jogador (+0x20): ja existente so liga/desliga (cmd == 7), senao e criado.
void CGolfPlayer::OnScriptFx(const char*& p, bool bSkip, int cmd)
{
	int index = m_scriptIndex983;
	if (bSkip)
		return;
	char cond[64];
	char name[64];
	char boneName[64];
	cond[0] = 0;
	p = GetStrConst(p, cond);
	if (strlen(cond) == 0 || strlen(cond) >= 64)
		return;
	if (cond[0] == '@')
	{
		if (m_pPetInfo[index].m_pMotion == NULL || strcmp(cond + 1, m_pPetInfo[index].m_pMotion->name) != 0)
			return;
		p = GetStrConst(p, name);
		if (strlen(name) == 0 || strlen(name) >= 64)
			return;
	}
	else
	{
		strcpy(name, cond);
		cond[0] = 0;
	}
	if (stricmp(name, "*playbbfx") == 0)
	{
		ParseBoundingBoxScript(index, true, true, true);
		// 983 00741a1b: se o jogador da vez tem anel com efeito em +0x24[m_currentPlayer], manda 0x335 a si mesmo com ele
		if (m_ringFxTid983[GOLFDOC()->m_currentPlayer] != 0)
		{
			HandleMsg(MsgObject(NULL, 0x335, m_ringFxTid983[GOLFDOC()->m_currentPlayer], 0, 0, 0, 0));
		}
		return;
	}
	p = GetStrConst(p, boneName);
	int boneLen = (int)strlen(boneName);
	if (boneLen == 0 || boneLen >= 64)
		return;
	bool bCond = false;
	sClubSet* pClubSet = GolfClub().FindClubSet(Doc()->m_userInfo[index].clubInfo.tid);
	std::vector<unsigned long> groups;
	if (boneName[0] == '&')
	{
		unsigned long tid;
		do
		{
			tid = atoi(boneName + 1);
			if (!bCond &&
				(m_pPetFrame[index].GetPartList().FindPart(tid) ||
					(pClubSet && pClubSet->pInfo && pClubSet->pInfo->ClubIds[GolfClub().GetType()] == tid)))
			{
				bCond = true;
				groups.push_back(tid);
			}
			p = GetStrConst(p, boneName);
			boneLen = (int)strlen(boneName);
		} while (boneLen != 0 && boneLen < 64 && boneName[0] == '&');
		if (!bCond)
			return;
		// 983: fecha o grupo do ultimo "&tid" lido e poe esse mesmo tid na lista (ao lado do primeiro que casou)
		jp983_rastro("B1505 script fecha grupo tid=%lu motion=%s slot=%d", tid, // B1505 temporario (brinco: Arin x Nuri(R))
			m_pPetInfo[index].m_pMotion ? m_pPetInfo[index].m_pMotion->name : "", index);
		m_pFxInterpreter983->CloseGroup(index, tid);
		groups.push_back(tid);
	}
	char bone[64];
	strcpy(bone, boneName);
	if (strlen(bone) == 0 || strlen(bone) >= 64)
		return;
	Attacher attacher;
	unsigned int j;
	for (j = 0; j < 4; j++)
	{
		if (j != 0)
		{
			p = GetStrConst(p, bone);
			if (strlen(bone) == 0 || strlen(bone) >= 64)
				break;
		}
		WBone* pBone = m_pPetFrame[index].GetBonePet()->FindBone(bone);
		if (pBone)
		{
			attacher.mat[j] = &pBone->m_matrix;
			continue;
		}
		WPuppet* pPet = m_pPetFrame[index].GetBonePet();
		w_bound_box* pBox = pPet->m_bbList.Find(bone);
		if (pBox)
		{
			if (pBox->bone)
				attacher.mat[j] = &pBox->bone->m_matrix;
			attacher.pos[j] = &pBox->info->spherePivot;
		}
		else if (pClubSet && pClubSet->pHead[GolfClub().GetType()])
		{
			pBone = pClubSet->pHead[GolfClub().GetType()]->FindBone(bone);
			if (pBone)
				attacher.mat[j] = &pBone->m_matrix;
			else
			{
				pBox = pClubSet->pHead[GolfClub().GetType()]->m_bbList.Find(bone);
				if (pBox)
				{
					if (pBox->bone)
						attacher.mat[j] = &pBox->bone->m_matrix;
					attacher.pos[j] = &pBox->info->spherePivot;
				}
			}
		}
	}
	if ((int)j <= 0)
		return;
	if (strcmp(name, "i_tak.seq") == 0)
		return;
	char* ext = strrchr(name, '.');
	if (!ext || strlen(ext) < 4)
		return;
	bool bOn = cmd == 7;
	if (stricmp(ext + 1, "spr") == 0)
	{
		if (groups.empty())
		{
			CFxSpray* spray = (CFxSpray*)m_pFxInterpreter983->FindSpray(name, cond, &attacher, 0, index, 0);
			if (spray)
				spray->m_bActive = bOn;
			else
				m_pFxInterpreter983->Attach(name, cond, &attacher, FxInterpreter::eET_SPRAY, true, 0, 0, false);
		}
		else
		{
			for (unsigned int n = 0; n < groups.size(); n++)
			{
				CFxSpray* spray = (CFxSpray*)m_pFxInterpreter983->FindSpray(name, cond, &attacher, groups[n], index, 0);
				if (spray)
					spray->m_bActive = bOn;
				else
				{
					int ret = m_pFxInterpreter983->Attach(name, cond, &attacher, FxInterpreter::eET_SPRAY, true, groups[n], 0,
						false);
					jp983_rastro("B1512 spr anexa %s grupo=%lu osso=%s mat=%d pos=%d ret=%d slot=%d", name, groups[n], boneName, // B1512 temporario (asas da Arin)
						attacher.mat[0] != NULL, attacher.pos[0] != NULL, ret, index);
				}
			}
		}
	}
	else
	{
		if (groups.empty())
		{
			CFxSequence* seq = (CFxSequence*)m_pFxInterpreter983->FindSequence(name, cond, &attacher, 0, index, 0);
			if (!seq)
				m_pFxInterpreter983->Attach(name, cond, &attacher, FxInterpreter::eET_SEQUENCE, true, 0, 0, false);
			else if (*((bool*)seq + VC10_FXSEQ_ACTIVE) != bOn)
				*((bool*)seq + VC10_FXSEQ_ACTIVE) = bOn;
		}
		else
		{
			for (unsigned int n = 0; n < groups.size(); n++)
			{
				CFxSequence* seq = (CFxSequence*)m_pFxInterpreter983->FindSequence(name, cond, &attacher, groups[n], index, 0);
				if (!seq)
				{
					int ret = m_pFxInterpreter983->Attach(name, cond, &attacher, FxInterpreter::eET_SEQUENCE, true, groups[n], 0,
						false);
					jp983_rastro("B1512 seq anexa %s grupo=%lu osso=%s mat=%d pos=%d ret=%d slot=%d", name, groups[n], boneName, // B1512 temporario (asas da Arin)
						attacher.mat[0] != NULL, attacher.pos[0] != NULL, ret, index);
				}
				else if (*((bool*)seq + VC10_FXSEQ_ACTIVE) != bOn)
					*((bool*)seq + VC10_FXSEQ_ACTIVE) = bOn;
			}
		}
	}
}

// 983 (00742360): leitor 005ace20 (kind -1, bFlag 1: troca pelas 24 pecas do quadro do roteiro) e SetPendingTexPart (00742120)
class IPetScriptObjectJp : public IPetScriptObject
{
public:
	bool ParsePTex983(w_motion_data* motion, std::string* pTexture, const char*& p, int kind, bool bFlag); // 005ace20
};

void CGolfPlayer::OnScriptPTex(const char*& p)
{
	std::string texture;
	if (((IPetScriptObjectJp*)static_cast<IPetScriptObject*>(this))->ParsePTex983(
			m_pPetInfo[m_scriptIndex983].m_pMotion, &texture, p, -1, true))
	{
		SetPendingTexPart(m_scriptIndex983, (char*)texture.c_str());
	}
}

// 983 (0073d840)
void CGolfPlayer::OnScriptShowBone(const char*& p, int cmd)
{
	ScriptShowBone(m_pScriptFrame983, cmd, p);
}

// 983 (00742d00)
void CGolfPlayer::OnScriptPlayIf(const char*& p)
{
	if (!m_bFlag330)
	{
		const char* motionName = ScriptPlayIf(m_pScriptFrame983, p);
		if (motionName)
		{
			SetMotion(motionName, 0.0f, m_scriptIndex983);
			m_pPetInfo[m_scriptIndex983].m_nextMotion[0] = 0;
		}
	}
}

// 983 (0073d860, 473 B): conferido com o 983 (b619/d.txt)
void CGolfPlayer::OnScriptOpa(const char*& p)
{
	int index = m_scriptIndex983;
	do
	{
		char cond[64];
		char args[64];
		cond[0] = 0;
		args[0] = 0;
		p = GetStrConst(p, cond);
		if (strlen(cond) == 0 || strlen(cond) >= 64)
		{
			break;
		}
		if (cond[0] == '@')
		{
			if (m_pPetInfo[index].m_pMotion == NULL ||
				strcmpi(cond + 1,
					m_pPetInfo[index].m_pMotion->name) != 0)
			{
				break;
			}
			p = GetStrConst(p, args);
			if (strlen(args) == 0 || strlen(args) >= 64)
			{
				break;
			}
		}
		else
		{
			strcpy(args, cond);
		}
		if (strlen(args) != 0 && strlen(args) < 64)
		{
			float from;
			float to;
			float time = 0.0f;
			sscanf(args, "%f %f %f", &from, &to, &time);
			m_pPetInfo[index].m_scale = from;
			time = (float)(time / 30.0); // 983: fdiv por double (00cfe8e0); 645: time *= 1.0f / 30.0f
			if (time > 0.0f)
			{
				m_pPetInfo[index].m_scaleRate = (to - from) / time;
			}
			else
			{
				m_pPetInfo[index].m_scaleRate = 0.0f;
			}
		}
		break;
	}
	while (0);
}

// 983 (0073da40)
void CGolfPlayer::OnScriptShowVis(const char*& p, int cmd)
{
	ScriptShowVis(m_pScriptFrame983, cmd == 13, p);
}

void CGolfPlayer::ParseScript_ShowVis(int index, bool bShow, const char* script)
{
	char token[0x100] = "";
	int count = 0;
	do
	{
		script = GetStrConst(script, token);
		if (strlen(token) == 0 || strlen(token) >= sizeof(token))
		{
			break;
		}
		std::list<std::string>* pGroup =
			CSharedDoc::Instance()->GetVisGroup(token);
		if (pGroup)
		{
			for (std::list<std::string>::iterator it = pGroup->begin();
				it != pGroup->end(); ++it)
			{
				WBone* pBone =
					m_pPetFrame[index].GetBonePet()->FindBone((*it).c_str());
				if (pBone)
				{
					pBone->SetInvisible(false, !bShow);
				}
			}
		}
		if (script == NULL || *script == ')')
		{
			break;
		}
	} while (++count < 64);
}

// 983 0073f610: os sons de tacada do clubset vem do CVoiceItem (00859f30, mascara por estado)
// no lugar dos PlaySfx fixos do 645; so os efeitos fly_* / HG_SpecialShot_1 / chao continuam fixos
void CGolfPlayer::PlayShotSfx(int index)
{
	if (index < 0 || index >= GOLFDOC()->m_playerNum)
	{
		return;
	}
	IFF_STRUCT::sClubSet* pIffSet = ItemManager()->FindClubSet(
		Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].clubInfo.tid);
	if (pIffSet == NULL)
	{
		return;
	}
	unsigned long clubTid = pIffSet->c.TypeId;

	if (strstr(m_pPetInfo[index].m_pMotion->name, "\xc7\xea\xbd\xba\xc0\xae"))
	{
		if (GolfClub().GetType() == 0)
		{
			if (strstr(m_motionName,
					"\xba\xfc\xb8\xa5\xc7\xea\xbd\xba\xc0\xae") == NULL)
				CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x200);
			else
				CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x400);
		}
		else if (GolfClub().GetType() == 1)
		{
			CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x800);
		}
		if ((GolfClub().GetType() == 0 || GolfClub().GetType() == 1) &&
			GetBarAccuracyGrade() < 2)
		{
			SendMsg(this, "Gallery", M983(0x103), 0, 0, 0, 0);
		}
		return;
	}

	WCrypticValue<unsigned char>* pStroke;
	if (Doc()->m_golfGame.gameType == 1)
	{
		pStroke = &TEAM(GOLFDOC()->GetPlayer(index)->team)
					   ->stroke[GOLFDOC()->m_currentHole - 1];
	}
	else
	{
		pStroke =
			&GOLFDOC()->GetPlayer(index)->stroke[GOLFDOC()->m_currentHole - 1];
	}
	int stroke = *pStroke;

	if (GolfClub().PowerShotType())
	{
		CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x2000);
	}

	float power = 0.0f;
	SendMsg(this, "GolfRule", M983(0x8b), (int)&power, 0, 0, 0);
	int strength = power > 0.9f ? 1 : 2;

	bool bSpecialBall = Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].userEquip.tidBall == 0x1400000b;
	if (GolfBall().m_initSpecial & 0x10)
	{
		if (bSpecialBall)
			g_audio->PlaySfx("HG_SpecialShot_1");
		else
			CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x8000);
		g_audio->PlaySfx("fly_tomahawk");
	}
	else if (GolfBall().m_initSpecial & 0x20)
	{
		if (bSpecialBall)
			g_audio->PlaySfx("HG_SpecialShot_1");
		else
			CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x4000);
		g_audio->PlaySfx("fly_cobra");
	}
	else if (GolfBall().m_initSpecial & 0x40)
	{
		if (bSpecialBall)
			g_audio->PlaySfx("HG_SpecialShot_1");
		else
			CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x10000);
		g_audio->PlaySfx("swing_spike");
	}

	switch (GolfClub().GetType())
	{
	case 0:
		if (stroke == 1)
			CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x02);
		else if (strength == 1)
			CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x08);
		else
			CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x10);
		if (GolfBall().m_initSpecial & 0x40)
		{
			CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x4000);
			g_audio->PlaySfx("fly_cobra");
		}
		break;
	case 1:
	case 2:
		if (GolfClub().GetType() == 2)
			CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x80);
		else if (strength == 1)
			CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x20);
		else
			CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x40);
		switch (GolfBall().m_groundType)
		{
		case 1:
			g_audio->PlaySfx(MakeStr(
				"\xbe\xc6\xc0\xcc\xbe\xf0_\xc6\xe4\xbe\xee\xbf\xfe\xc0\xcc%d",
				strength));
			break;
		case 3:
			g_audio->PlaySfx(
				MakeStr("\xbe\xc6\xc0\xcc\xbe\xf0_\xb8\xf0\xb7\xa1\xb9\xe7%d",
					strength));
			g_audio->PlaySfx(
				"\xba\xa1\xc4\xbf\xb8\xf0\xb7\xa1\xb3\xaf\xb8\xb2");
			break;
		case 4:
			g_audio->PlaySfx(
				MakeStr("\xbe\xc6\xc0\xcc\xbe\xf0_\xb3\xaa\xb9\xb5\xc0\xd9%d",
					strength));
			break;
		}
		break;
	case 3:
		CVoiceItem::Instance()->PlayClubSfx983(clubTid, 0x100);
		break;
	}
}

void CGolfPlayer::SetPendingTexPart(int index, char* name)
{
	std::list<sTexturePart*>::iterator it;
	it = std::find_if(m_pendingTexParts.begin(), m_pendingTexParts.end(),
		sFindTexPart(name));
	if (it != m_pendingTexParts.end())
	{
		if (*it != m_pPetInfo[index].m_pTexPart)
		{
			m_pPetInfo[index].m_pTexPart = *it;
		}
	}
	else
	{
		sTexturePart* pTexPart = new sTexturePart;
		strcpy(pTexPart->texture, name);
		m_pendingTexParts.push_back(pTexPart);
		m_pPetInfo[index].m_pTexPart = pTexPart;
	}
	// 983 00742120: aplica no mesmo instante a textura ao boneco do jogador da vez (o OnDisplay do 983 nao aplica mais)
	sTexturePart* pPart = m_pPetInfo[GOLFDOC()->m_currentPlayer].m_pTexPart;
	if (pPart &&
		pPart != m_pPetInfo[GOLFDOC()->m_currentPlayer].m_pCurTexPart)
	{
		m_pPetInfo[GOLFDOC()->m_currentPlayer].m_pCurTexPart = pPart;
		if (pPart->pBitmap == NULL)
		{
			m_pPetFrame[GOLFDOC()->m_currentPlayer]
				.GetBonePet()
				->ChangeTexturePart(
					m_pPetFrame[GOLFDOC()->m_currentPlayer].GetFaceTex(),
					pPart->texture, &pPart->pBitmap, &pPart->rect);
		}
		else
		{
			m_pPetFrame[GOLFDOC()->m_currentPlayer]
				.GetBonePet()
				->ChangeTexturePart(
					m_pPetFrame[GOLFDOC()->m_currentPlayer].GetFaceTex(),
					pPart->pBitmap, pPart->rect);
		}
		m_pPetFrame[GOLFDOC()->m_currentPlayer].SetMakeup();
	}
}

void CGolfPlayer::SearchGround()
{
	WVector cur = m_ballDist * RotMat(m_angle, 1).xa;
	WVector dir = cur;
	float t;
	for (t = 0.0f; t < 1.0f; t += 0.1f)
	{
		cur = GetPVS().GetGroundPoint(m_pos + dir * t, true, NULL, false);
		WVector pos = GetPVS().GetGroundPoint(m_pos + dir * (t + 0.1f), true,
			NULL, false);
		if (((pos - m_pos).Magnitude() - m_ballDist) *
				((cur - m_pos).Magnitude() - m_ballDist) <=
			0.0f)
		{
			break;
		}
	}

	float next = t + 0.1f;
	while (t < next)
	{
		cur = GetPVS().GetGroundPoint(m_pos + dir * t, true, NULL, false);
		t += 0.01f;
		WVector pos =
			GetPVS().GetGroundPoint(m_pos + dir * t, true, NULL, false);
		if (((pos - m_pos).Magnitude() - m_ballDist) *
				((cur - m_pos).Magnitude() - m_ballDist) <=
			0.0f)
		{
			m_matrix.xa = pos - m_pos;
			m_matrix.xa.Normalize();
			m_matrix.ya = WCrossProduct(m_matrix.za, m_matrix.xa);
			if (GOLFDOC()->m_bNoStat || GOLFDOC()->m_b195_983) // 983 0073e640: +0x194 e +0x195 do doc
			{
				m_bFlag15e = false;
			}
			else if (Wabs(m_matrix.xa.y) > sin(20.0f * g_DEGTORAD))
			{
				m_bFlag15e = true;
			}
			else
			{
				g_rastroNoPlay983 = true;
				m_bFlag15e = GetPVS().OverlappedWithObject(
					m_clubs[GOLFDOC()->m_currentPlayer][GolfClub().GetType()]
								.m_ballPos *
							m_matrix +
						m_pos,
					1.6f);
				g_rastroNoPlay983 = false;
			}
			if (m_bFlag15e) // B1504 temporario: NoPlay no Blue Water 2 (rampa = xa.y > sin 20)
				jp983_rastro("B1504 NoPlay pos=(%.1f,%.1f,%.1f) xa.y=%.3f", m_pos.x, m_pos.y, m_pos.z, m_matrix.xa.y);
			SendMsg(this, "GolfRule", M983(0x7), !m_bFlag15e, 0, 0, 0);
			SendMsg(this, "GolfRule", M983(0x77), !m_bFlag15e, 0, 0, 0);
			SendMsg(this, "GolfRule", M983(0x7), !m_bFlag15e, 0, 0, 0);
			m_footAngle = (float)asin(m_matrix.xa.y) * -1.0f;
			m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags |= 0xb;
			return;
		}
	}
	m_matrix = RotMat(m_angle, 1);
	m_bFlag15e = true;
	SendMsg(this, "GolfRule", M983(0x77), 0, 0, 0, 0);
	m_footAngle = 0.0f;
	m_pPetInfo[GOLFDOC()->m_currentPlayer].m_flags |= 0xb;
}

bool CGolfPlayer::SearchGround(const WVector& rPos, float angle)
{
	WVector dir = m_ballDist * RotMat(angle, 1).xa;
	WVector cur;
	float t;
	for (t = 0.0f; t < 1.0f; t += 0.1f)
	{
		int index;
		cur = GetPVS().GetGroundPoint(rPos + dir * t, true, NULL, false);
		WVector pos = GetPVS().GetGroundPoint(rPos + dir * (t + 0.1f), true,
			&index, false);
		if (index < 0)
		{
			return false;
		}
		if (((pos - rPos).Magnitude() - m_ballDist) *
				((cur - rPos).Magnitude() - m_ballDist) <=
			0.0f)
		{
			break;
		}
	}

	float next = t + 0.1f;
	while (t < next)
	{
		cur = GetPVS().GetGroundPoint(rPos + dir * t, true, NULL, false);
		t += 0.01f;
		WVector pos =
			GetPVS().GetGroundPoint(rPos + dir * t, true, NULL, false);
		if (((pos - rPos).Magnitude() - m_ballDist) *
				((cur - rPos).Magnitude() - m_ballDist) <=
			0.0f)
		{
			WMatrix mat = RotMat(m_angle, 1);
			mat.xa = pos - rPos;
			mat.xa.Normalize();
			mat.ya = WCrossProduct(mat.za, mat.xa);

			const CGolfBall::sExpected* pMin =
				GolfBall().PreData(GolfBall().m_renderNum);
			WVector vMin(pMin->pos.x - 16.0f, pMin->pos.y, pMin->pos.z - 16.0f);
			const CGolfBall::sExpected* pMax =
				GolfBall().PreData(GolfBall().m_renderNum);
			WVector vMax(pMax->pos.x + 16.0f, pMax->pos.y, pMax->pos.z + 16.0f);
			GetPVS().GetObjList(Waabb(vMin, vMax), 1);

			if (Wabs(mat.xa.y) > sin(20.0f * g_DEGTORAD))
			{
				return false;
			}
			if (GetPVS().OverlappedWithObject(
					m_clubs[GOLFDOC()->m_currentPlayer][GolfClub().GetType()]
								.m_ballPos *
							mat +
						rPos,
					1.6f))
			{
				return false;
			}
			return true;
		}
	}
	return false;
}

unsigned char CGolfPlayer::HasStandGround(const WVector& rPos)
{
	float angle = 0.0f;
	unsigned char count = 0;
	// 983 007405e0: passo 45 * (double)0.017453292f (literal double 3fe921fb51000000)
	for (int i = 0; i < 8; i++, angle += 0.7853981573134661)
	{
		if (SearchGround(rPos - WVector(0.0f, GolfBall().GetRadius(), 0.0f),
				angle))
		{
			count++;
		}
	}
	return count;
}

void CGolfPlayer::SetLight()
{
	WBone* pBone =
		m_pPetFrame[GOLFDOC()->m_currentPlayer].GetBonePet()->FindBoneByTail(
			"Head");
	m_lightSet.diffuse = ModifyColor(g_lightset.diffuse,
		GetPVS().GetShadowIntensity(pBone->GetMatrix().pivot));
}

void CGolfPlayer::SetLight(int index)
{
	if (index >= 0 && index < 4)
	{
		WBone* pBone = m_pPetFrame[index].GetBonePet()->FindBoneByTail("Head");
		if (pBone)
		{
			m_lightSet.diffuse = ModifyColor(g_lightset.diffuse,
				GetPVS().GetShadowIntensity(pBone->GetMatrix().pivot));
		}
	}
}

void CGolfPlayer::ResetComboState()
{
	m_bCombo = false;
	m_comboTime = 0.0f;
}

void CGolfPlayer::StartComboState()
{
	m_bCombo = true;
	m_comboTime = 0.0f;
}

void CGolfPlayer::EndComboState()
{
	// 983 (0073e220): divide por double (00d09458); 645: m_comboTime * (1.0f / 33.0f)
	int count = (int)(float)(m_comboTime / 33.0);
	m_comboTime = 0.0f;
	m_comboCount = count;
	m_bCombo = false;
}

// 983 007407e0: nivel do gauge (do time no Team: gameType 1 = doc +0x10594): 0 (< 33), 1 (>= 33), 2 (>= 66), 3 (>= 99)
static int PowerGaugeLevel983()
{
	float gauge = Doc()->m_golfGame.gameType == 1
		? (float)TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)->gauge
		: (float)PLAYER(GOLFDOC()->m_currentPlayer)->gauge;
	if (gauge >= 99.0f)
	{
		return 3;
	}
	if (gauge >= 66.0f)
	{
		return 2;
	}
	return gauge < 33.0f ? 0 : 1;
}

// 983 0073edb0: +1 power shot com GolfDoc +0x890[jogador] e o bit 0x10 do event do jogador (00663870)
static bool PowerShotBonus983()
{
	CGolfDoc* gd = GOLFDOC();
	if (gd->m_pPowerShotItem983[gd->m_currentPlayer] == NULL)
	{
		return false;
	}
	return PLAYER(gd->m_currentPlayer)->event.GetFlag(0x10);
}

// 983 007406b0: item da vez que da power shot: 1 (0x18000004/10/22/27/2f), 2 (0x1800002e), 0 nenhum
static int PowerShotItemType983()
{
	unsigned long item = PLAYER(GOLFDOC()->m_currentPlayer)->item;
	if (item == 0x18000004 || item == 0x18000010 || item == 0x18000022 || item == 0x18000027 ||
		item == 0x1800002f)
	{
		return 1;
	}
	return item == 0x1800002e ? 2 : 0;
}

// 983 00740a10 (+ 007409e0): m_comboCount <= power shots disponiveis = item, ou min(nivel do gauge + bonus, 2)
bool CGolfPlayer::Decide_PowerShot()
{
	int avail = PowerGaugeLevel983() + PowerShotBonus983();
	int itemType = PowerShotItemType983();
	if (avail > 2)
	{
		avail = 2;
	}
	if (itemType == 0)
	{
		itemType = avail;
	}
	return m_comboCount <= itemType;
}

// 983 00742d50: sem a recusa do combo 2 (m_bFlag329), sem IsLocalContent, sem botoes predict_* / ShotPreview / Practice
void CGolfPlayer::Shot_PowerShot()
{
	if (!(GOLFDOC()->m_gameMode & 0x208) || Doc()->m_playingMode != 1)
	{
		return;
	}
	if (Decide_PowerShot())
	{
		if (!m_bFlag190 && m_swingTypeName[0])
		{
			if (m_pPetInfo[GOLFDOC()->m_currentPlayer].m_val1 == 3)
			{
				g_audio->StopSfx("powershot_ready");
				m_pPetInfo[GOLFDOC()->m_currentPlayer].m_val1 = 0;
				SetMotion(MakeStr("%s\xc1\xd8\xba\xf1", m_swingTypeName), 0.0f,
					-1);
				int param = 0;
				SendMsg(this, "Screen", M983(0x1aa), (int)&param, 0, 0, 0);
			}
			else
			{
				g_audio->PlaySfx("powershot_ready", NULL, NULL,
					"powershot_ready", 0, 0.5f, 200.0f);
				m_pPetInfo[GOLFDOC()->m_currentPlayer].m_val1 = 3;
				unsigned long item = PLAYER(GOLFDOC()->m_currentPlayer)->item;
				if (item == 0x18000027)
				{
					GolfClub().SetPowerShot(3);
				}
				else if (item == 0x1800002e)
				{
					GolfClub().SetPowerShot(2);
				}
				else
				{
					GolfClub().SetPowerShot(m_comboCount);
				}
				GolfClub().SetPowerShotExtra983(PowerShotBonus983(), PowerShotItemType983()); // club +0x6c/+0x70
				unsigned char type = GolfClub().PowerShotType();
				char* motion;
				if (type == 2 || type == 3)
				{
					motion = MakeStr("%s\xc6\xc4\xbf\xf6"
									 "2\xc1\xd8\xba\xf1",
						m_swingTypeName);
				}
				else
				{
					motion = MakeStr("%s\xc6\xc4\xbf\xf6\xc1\xd8\xba\xf1",
						m_swingTypeName);
				}
				SetMotion(motion, 0.0f, -1);
				SendMsg(this, "Screen", 0x313, 1, 0, 0, 0);
				int autoShot = 0;
				SendMsg(this, "GolfRule", 0x32c, (int)&autoShot, 0, 0, 0);
				SendMsg(this, "Screen", 0x32f, autoShot, 0, 0, 0);
			}
		}
	}
	else if (CHelpMsg::Instance())
	{
		g_audio->PlaySfx("play_unplayable", NULL, NULL, 0, 0, 0.5f, 200.0f);
		CHelpMsg::Instance()->SetHelpMsg(1, NULL);
	}
}

void CGolfPlayer::Cancel_PowerShot()
{
	if (GolfClub().PowerShotType())
	{
		g_audio->StopSfx(RB_STR_POWERSHOT_READY);
		g_audio->PlaySfx("powershot_cancel", NULL, NULL, 0, 0, 0.5f, 200.0f);
		float param = 0.0f; // 983: fldz/fstp
		GolfClub().SetPowerShot(0);
		SendMsg(this, "Spot", M983(0xf5), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1aa), (int)&param, 0, 0, 0);
		SetSwingType();
		if (OnlinePlay())
		{
			WSendPacket packet((enumClientPacket)0x15);
			packet.Encode1(0);
			packet.Send((eSendTo)0);
		}
		// 983 00743000: depois do pacote, Screen 0x32f (sem auto shot) e GolfRule 0x32a (esvazia a fila, solta a tela)
		SendMsg(this, "Screen", 0x32f, 0, 0, 0, 0);
		SendMsg(NULL, "GolfRule", 0x32a, 0, 0, 0, 0);
	}
}

// 983 00743390: Power Shot pronto (itens 0x14b/0x14c/0x14d com bCombo = 1; comando 0x14e do console com 0)
void CGolfPlayer::SetPowerShotReady983(const char* motion, int level, bool bCombo)
{
	SetMotion(motion, 0.0f, -1);
	m_pPetInfo[GOLFDOC()->m_currentPlayer].m_val1 = 4;
	GolfClub().SetPowerShot(level);
	SendMsg(this, "Spot", M983(0xf5), 0, 0, 0, 0);
	float alpha = 1.0f;
	m_bPowerShot = true;
	m_powerShotKeyState = 0;
	m_bFlag329 = bCombo;
	SendMsg(this, "Screen", M983(0x1aa), (int)&alpha, 0, 0, 0);
	SendMsg(this, "Screen", 0x313, 1, 0, 0, 0);
	int autoShot = 0;
	SendMsg(this, "GolfRule", 0x32c, (int)&autoShot, 0, 0, 0);
	SendMsg(this, "Screen", 0x32f, autoShot, 0, 0, 0);
}

void CGolfPlayer::PowerShotInput()
{
	// 983 00743180: modos com m_gameMode & 0x30c0 nao tem power shot (sai antes de tudo, inclusive do disparo por tempo);
	// sem o atalho do item 0x18000027 (IsLocalContent 0x32) do 645; cancelar so com m_gameMode & 0x208. O relogio do 983 e
	// (*00f03ff0)() em segundos (00b48da0 QPC / 00b48df0 timeGetTime) contra 0.5 (00d66d08) = timeGetTime contra 500 ms aqui.
	C_ASSERT(offsetof(sBar, impact) == 0x4 && offsetof(sBar, grade) == 0x10 && offsetof(sBar, center) == 0x1c);
	C_ASSERT(offsetof(sCameraData, angle) == 0x18);
	C_ASSERT_VC10(offsetof(CGolfDoc, m_bar) == 0xe0 && offsetof(CGolfDoc, m_cameraData) == 0x104);
	C_ASSERT_VC10(offsetof(CGolfDoc, m_skinsNextTurnWord983) == 0x1a8 && offsetof(CGolfDoc, m_skinsCarryCount) == 0x1c4);
	C_ASSERT_VC10(sizeof(CGolfPlayer) == 0x29c);
	C_ASSERT_VC10(offsetof(CGolfPlayer, m_comboCount) == 0x260);
	C_ASSERT_VC10(offsetof(CGolfPlayer, m_powerShotKeyTime) == 0x268);
	C_ASSERT_VC10(offsetof(CGolfPlayer, m_bPowerShot) == 0x26c);
	C_ASSERT_VC10(offsetof(CGolfPlayer, m_powerShotKeyState) == 0x270);
	C_ASSERT_VC10(offsetof(CGolfPlayer, m_extEffect) == 0x278);
	C_ASSERT_VC10(sizeof(CExtendedEffect) == 0x20 && offsetof(CGolfPlayer, m_pad983_298) == 0x298);
	if (GOLFDOC()->m_gameMode & 0x30c0)
	{
		return;
	}
	if (IsMyTurn(false) && !CQuitWindow::Instance()->IsActive() &&
		CItemWindow::IsClosed())
	{
		if (m_powerShotKeyState == 0)
		{
			if (g_input->GetUp("LALT") || g_input->GetUp("RALT"))
			{
				if (!m_bPowerShot)
				{
					m_powerShotKeyState = 1;
					m_powerShotKeyTime = timeGetTime();
				}
				else if (GOLFDOC()->m_gameMode & 0x208)
				{
					Cancel_PowerShot();
					m_bPowerShot = false;
				}
			}
		}
		else if (m_powerShotKeyState == 1)
		{
			if (g_input->GetDown("LALT", true) ||
				g_input->GetDown("RALT", true))
			{
				if (timeGetTime() < m_powerShotKeyTime + 500)
				{
					m_powerShotKeyState = 2;
				}
				else if (m_bPowerShot && (GOLFDOC()->m_gameMode & 0x208))
				{
					Cancel_PowerShot();
					m_bPowerShot = false;
					m_powerShotKeyState = 0;
				}
			}
		}
		else if (m_powerShotKeyState == 2)
		{
			if (g_input->GetUp("LALT") || g_input->GetUp("RALT"))
			{
				m_comboCount = 2;
				Shot_PowerShot();
				m_bPowerShot = true;
				m_powerShotKeyState = 0;
			}
		}
	}
	if (m_powerShotKeyState == 1 && !m_bPowerShot &&
		timeGetTime() >= m_powerShotKeyTime + 500) // 983: 0.5 <= dt
	{
		m_comboCount = 1;
		Shot_PowerShot();
		m_bPowerShot = true;
		m_powerShotKeyState = 0;
	}
}

bool CGolfPlayer::IsAnimFxRunning(int index, unsigned long id)
{
	if (id == 0 || index > 4)
	{
		return false;
	}
	std::vector<unsigned long>::iterator it;
	for (it = m_condIds[index].begin(); it != m_condIds[index].end(); it++)
	{
		if (*it == id)
		{
			return true;
		}
	}
	return false;
}

void CGolfPlayer::RefreshAllUccClothes()
{
	// HACK: emission order: waits one wave, as in the original
	if (0)
	{
		m_pPetFrame[0].GetAniPetList().begin();
	}
	sUccRefreshItem item;
	for (int i = 0; i < 10; i++)
	{
		if (!UccManager()->GetRefreshItem(item))
		{
			return;
		}
		for (int player = 0; player < GOLFDOC()->m_playerNum; player++)
		{
			CPartTidList& tids = GOLFDOC()->GetPartTidList(player);
			unsigned char part = (unsigned char)((item.typeId >> 13) & 0x1f);
			if (tids.m_tid[part] == item.typeId)
			{
				unsigned long uid = tids.m_uid[part];
				sUccClothes* pClothes = UccManager()->FindClothes(uid);
				if (pClothes && !strcmpi(pClothes->uccIndex, item.uccIndex))
				{
					// 983 00740ada: 1x CPetFrame::SetPart 008b0960 (so as texturas UCC)
					m_pPetFrame[player].SetPart983(pClothes);
				}
			}
		}
	}
}

void CGolfPlayer::AddAbilityParts(unsigned long tid, unsigned long id)
{
	m_abilityParts.push_back(std::make_pair(tid, id));
}

// The inline helpers below sit in headers of the PCH in the original, with a late IL offset: every function that
// uses them waits for the next compile wave. Defining them at the end of the file gives the same emission order.
// commonutil.h:22 in the original (inline; not in our PCH headers)
inline int float2int(float value)
{
	return (int)value;
}
