#include "minatl.h"
#include "vc10_layout.h"
#include "golfdoc.h"
#include "shareddoc.h"
#include "golfball.h"
#include "club.h"
#include "sky.h"
#include "scenemanager.h"
#include <string.h>
#include "exception.h"
#include "projectg.h"
#include "cardsystem.h"
#include "commonutil.h"
#include "quadtree.h"
#include "fx.h"
#include "capturedbg.h"
#include "pangfbi.h"
#include "autotarget.h"
#include "polysoup.h"
#include <stdio.h>
#include <stack>

// 983: MyGuid e chamada fora de linha (0047dd30)
__declspec(noinline) unsigned long MyGuid(bool bGallery);


bool IsCurMapSnow();
extern WMatrix g_camera;
WMatrix __cdecl MakeCameraMatrix(const WVector& eye, const WVector& at);

template <class T>
std::stack<T*, std::list<T*> > CStackableSingleton<T>::m_InstanceStack;

IObject* CGolfDocMakeInstance()
{
	return new CGolfDoc;
}
struct __sCGolfDoc
{
	__sCGolfDoc()
	{
		ObjectFactory().AddObjectFunctor(CGolfDocMakeInstance, "CGolfDoc");
	}
};
const WRTTI CGolfDoc::m_RTTI("CGolfDoc", &CTaskDoc::m_RTTI);
static __sCGolfDoc __implCGolfDoc;

std::stack<CGolfDoc*, std::list<CGolfDoc*> > CGolfDoc::m_GolfDocInstanceStack;

CGolfDoc::CGolfDoc()
	: m_pPolySoup(NULL),
	  m_pSky(NULL),
	  m_pAutoTarget(NULL),
	  m_modelNameList(8, 8),
	  m_pCameraTargetPuppet(NULL),
	  m_bTutorialPin983(0),
	  m_pPlayer(NULL),
	  m_pTeam(NULL),
	  m_pHoleData(NULL),
	  m_gameMode(0),
	  m_bNetGameEnd(0),
	  m_bTeeShotReady(0)
{
	C_ASSERT_VC10(offsetof(CGolfDoc, m_uiAlpha) == 0xdc && offsetof(CGolfDoc, m_bar) == 0xe0 && offsetof(CGolfDoc, m_cameraData) == 0x104); // 983 (s93)
	// mapa de layout do 983 (docs/LAYOUTS.md, A193): campos com acesso igual nos dois lados; m_pTeam = ~CGolfDoc (A190.1)
	C_ASSERT_VC10(sizeof(CGolfDoc) == 0x8c0 && offsetof(CGolfDoc, m_pSky) == 0xc && offsetof(CGolfDoc, m_pTeam) == 0xcc);
	C_ASSERT_VC10(offsetof(CGolfDoc, m_playerNum) == 0xd5 && offsetof(CGolfDoc, m_pHoleData) == 0xd8 && offsetof(CGolfDoc, m_currentPlayer) == 0x189);
	C_ASSERT_VC10(offsetof(CGolfDoc, m_holeResult983) == 0x340 && sizeof(sHoleResult983) == 0x21 && offsetof(CGolfDoc, m_shotSync983) == 0x45c && offsetof(CGolfDoc, m_remoteGauge983) == 0x48e);
	C_ASSERT_VC10(offsetof(CGolfDoc, m_tutorialMode) == 0x88 && offsetof(CGolfDoc, m_pCameraTargetPuppet) == 0x98 && offsetof(CGolfDoc, m_tutorialPin983) == 0x9c && offsetof(CGolfDoc, m_bTutorialPin983) == 0xa8 && offsetof(CGolfDoc, m_tutorialMinGauge983) == 0x16c); // CTutorial 983 (A724)
	C_ASSERT_VC10(offsetof(CGolfDoc, m_partTidList) == 0x494 && offsetof(CGolfDoc, m_approachGameResult) == 0x850);
	Doc()->m_pGolfDoc = this;
	m_GolfDocInstanceStack.push(this);
	m_saveChatNum = -1;
	m_tutorialMode = 0xf;
	m_currentPlayerNum = 0;
	m_playerNum = 0;
	m_shotPhase = 0;
	m_currentPlayer = 0;
	m_uiAlpha = 0xff;
	m_cameraData.clearance = 0.5f;
	m_cameraData.angleOffset = 0.34906584f;
	m_bright = 1.0f;
	m_bPause = false;
	m_skinsHoleWinner = -1;
	m_skinsPenaltyUid = -1;
	m_bGodMode = 0;
	m_debugBarValue983 = 0;
	m_bReport = false;
	m_bStopRequest = false;
	m_bDropRequest = false;
	m_maxGrade = 0;
	m_bStopDlg = false;
	m_bReplaying = 0;
	m_bNoStat = false;
	m_b195_983 = false;
	m_bShowChat = true;
	m_bDropEnable = 1;
	m_bTutorialGaugeMute = 0;
	m_impactScale = 1.0f;
	m_controlScale = 15.0f;
	m_bDay = true;
	m_skinsNextTurnWord983 = 0;
	m_skinsHolePot = 0;
	m_skinsTotalPang = 0;
	m_skinsPotBase = 0;
	m_skinsWinnerBonus983 = 0;
	m_skinsPotMult = 0;
	m_skinsCarryCount = 0;
	MakeHoleData();
	CStackableSingleton<CSceneManager>::PushInstance();
	CStackableSingleton<CCapturedBg>::PushInstance();
	new CSceneManager;
	new CCapturedBg;
	if (CFx::IsInstantiated())
	{
		CSceneManager::Instance()->RegisterElement(OBJ_CUSTOM, NULL,
			CFx::Instance(), NULL, 0, NULL);
	}
	m_pPolySoup = new WPolySoup(this);
	m_pPolySoup->SetResourceManager(g_resrcmng);
	memset(m_holeResult983, 0, sizeof(m_holeResult983));
	ClearShotResultMsg();
	memset(m_shotSync983, 0, sizeof(m_shotSync983));
	memset(&m_remoteGauge983, 0, sizeof(m_remoteGauge983));
	m_bApproach794 = true;
	m_bApproach795 = false;
	m_approachMissionType = (eApproachMissionType)0;
	m_approachPrizeType = (eApproachPrizeType)0;
	m_approachWinnerType = (eApproachWinnerType)0;
	m_approachCurHoleResult.clear();
	m_approachGameResult.clear();
	m_approachTotalPrize = 0;
	// HACK: dead references: inline COMDATs the original also emitted (owner function not identified yet)
	if (0)
	{
		CCapturedBg::IsInstantiated();
		if (m_pPlayer->stroke[0] == (unsigned char)0)
			std::list<sChatLine>::iterator();
	}
}

CGolfDoc::~CGolfDoc()
{
	ClearApproachCurHoleResult();
	ClearApproachGameResult();
	SendReport();
	m_GolfDocInstanceStack.pop();
	if (m_GolfDocInstanceStack.empty())
	{
		Doc()->m_pGolfDoc = NULL;
	}
	else
	{
		Doc()->m_pGolfDoc = m_GolfDocInstanceStack.top();
	}
	if (m_pPolySoup != NULL)
	{
		delete m_pPolySoup;
		m_pPolySoup = NULL;
	}
	if (m_pSky != NULL)
	{
		delete m_pSky;
		m_pSky = NULL;
	}
	if (m_pAutoTarget != NULL)
	{
		delete m_pAutoTarget;
		m_pAutoTarget = NULL;
	}
	if (m_pPlayer != NULL)
	{
		delete[] m_pPlayer;
		m_pPlayer = NULL;
	}
	if (m_pTeam != NULL)
	{
		delete[] m_pTeam;
		m_pTeam = NULL;
	}
	if (m_pHoleData != NULL)
	{
		delete[] m_pHoleData;
		m_pHoleData = NULL;
	}
	for (char* str = m_modelNameList.Start(); str != NULL;
		str = m_modelNameList.Next())
	{
		if (str != NULL)
		{
			delete[] str;
		}
	}
	m_modelNameList.Reset();
	delete CCapturedBg::Instance();
	delete CSceneManager::Instance();
	CStackableSingleton<CCapturedBg>::PopInstance();
	CStackableSingleton<CSceneManager>::PopInstance();
	if (CTaskManager::Instance()->GetPreservedTask() == NULL)
	{
		g_resrcmng->ReleaseAllPets();
	}
}

unsigned long __fastcall ModifyColor(unsigned long color, float bright);

void CGolfDoc::UpdateBright(float bright)
{
	if (!WisEqual(m_bright, bright, g_EPSILON))
	{
		m_bright = bright;
		g_lightset.diffuse =
			ModifyColor(g_lightset.diffuse, Doc()->m_pGolfDoc->m_bright);
		g_lightset.ambient =
			ModifyColor(g_lightset.ambient, Doc()->m_pGolfDoc->m_bright);
		g_lightset.ambient2 =
			ModifyColor(g_lightset.ambient2, Doc()->m_pGolfDoc->m_bright);
		if (CSceneManager::Instance() != NULL)
		{
			CSceneManager::Instance()->UpdateBright();
		}
		if (m_pPolySoup != NULL)
		{
			m_pPolySoup->UpdateBright();
		}
		if (m_pSky != NULL)
		{
			m_pSky->UpdateBright();
		}
	}
}

WMatrix CGolfDoc::GetDefaultCamera()
{
	WMatrix& cam = m_cameraData.mat;
	WVector eye = cam.pivot;
	float x = cam.pivot.x + cam.za.x;
	float y = cam.pivot.y + cam.za.y;
	float z = cam.pivot.z + cam.za.z;
	WVector target(x, y, z);
	target.y = tanf(m_cameraData.pitch) + y;
	WMatrix result = MakeCameraMatrix(eye, target);
	return result;
}

sPlayerData* CGolfDoc::GetPlayer(unsigned char index)
{
	if (m_pPlayer == NULL)
	{
		return NULL;
	}
	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 0xb: // 983 00663800
		return m_pPlayer;
	}
	if (index >= m_playerNum)
	{
		return NULL;
	}
	return &m_pPlayer[index];
}

// JP 983 (00661eb0): tee e pin (x, z) do buraco tirados do mapa pelo alcance; chamada no modo 0xb
// (CGolfRuleBase::NextHole983 e CGolfRuleHoleInOne::SetPlayer)
void CGolfDoc::SetHoleRange983(unsigned char hole, float range)
{
	WVector2D pin;
	WVector2D tee;
	m_pPolySoup->GetTeePinByRange983(hole, &pin, range, &tee);
	m_pHoleData[hole - 1].tee.x = tee.x;
	m_pHoleData[hole - 1].tee.z = tee.y;
	m_pHoleData[hole - 1].pin.x = pin.x;
	m_pHoleData[hole - 1].pin.z = pin.y;
}

// JP 983 (00661f30): pin e lista de todos os tees do buraco (o primeiro vira o tee); chamada pelo modo Hole-in-One
// (0057afcc, CHoleInOneGameHandler, que fica com a lista)
void CGolfDoc::SetHoleTees983(unsigned char hole, std::vector<WVector2D>* pTees)
{
	WVector2D pin;
	m_pPolySoup->GetPinAndTees983(hole, &pin, pTees);
	m_pHoleData[hole - 1].tee.x = (*pTees)[0].x;
	m_pHoleData[hole - 1].tee.z = (*pTees)[0].y;
	m_pHoleData[hole - 1].pin.x = pin.x;
	m_pHoleData[hole - 1].pin.z = pin.y;
}

// JP 983 (00663870): o jogador da vez tem algum dos bits de evento
bool CGolfDoc::IsCurrentPlayerEvent983(unsigned long flag)
{
	sPlayerData* pPlayer = GetPlayer(m_currentPlayer);
	if (pPlayer == NULL)
		return false;
	return (*(unsigned long*)&pPlayer->event & flag) != 0;
}

sTeamData* CGolfDoc::GetTeam(unsigned char index)
{
	if (m_pTeam == NULL)
	{
		return NULL;
	}
	return &m_pTeam[index];
}

void CGolfDoc::MakeTeam()
{
	m_teamNum = 2;
	if (m_pTeam != NULL)
	{
		delete[] m_pTeam;
		m_pTeam = NULL;
	}
	m_pTeam = new sTeamData[2];
	memset(m_pTeam, 0, sizeof(sTeamData) * 2);
}

void CGolfDoc::MakePlayer(unsigned char num)
{
	m_currentPlayerNum = num;
	m_playerNum = num;
	if (m_pPlayer != NULL)
	{
		delete[] m_pPlayer;
		m_pPlayer = NULL;
	}
	m_pPlayer = new sPlayerData[m_playerNum];
	memset(m_pPlayer, 0, sizeof(sPlayerData) * m_playerNum);
}

int __cdecl CompareRank(const void* a, const void* b)
{
	sPlayerData* p1 = (sPlayerData*)a;
	sPlayerData* p2 = (sPlayerData*)b;
	int hole = Doc()->m_pGolfDoc->m_currentHole;
	if (p2->stroke[hole - 1] <= p1->stroke[hole - 1] &&
		(p2->stroke[hole - 1] != p1->stroke[hole - 1] || p1->state != 1 ||
			p2->state == 1))
	{
		return 1;
	}
	return 0;
}

void CGolfDoc::MakeHoleData()
{
	if (m_pHoleData != NULL)
	{
		delete[] m_pHoleData;
		m_pHoleData = NULL;
	}
	m_pHoleData = new sHoleData[18];
	memset(m_pHoleData, 0, sizeof(sHoleData) * 18);
}

void CGolfDoc::InitShotResultMsg()
{
	float radius = GolfBall().GetRadius();
	WVector& tee = GetHoleData().tee;
	WVector pos(tee.x, radius + tee.y, tee.z);
	for (int i = 0; i < m_playerNum; ++i)
	{
		m_shotResultMsg[i].pos[0] = pos.x;
		m_shotResultMsg[i].pos[1] = pos.y;
		m_shotResultMsg[i].pos[2] = pos.z;
		m_shotResultMsg[i].flag = 0;
	}
}

void CGolfDoc::LoadXml(const char* filename, unsigned char index, bool reload)
{
	TiXmlDocument doc;
	cFile* file = g_resrcmng->GetCFile(filename, 0xffff);
	if (file)
	{
		char* buf = new char[file->Length() + 1];
		file->Read(buf, file->Length());
		buf[file->Length()] = 0;
		CloseCFile(file);

		if (doc.LoadFileFromMemoryEx(buf))
		{
			if (reload || m_xmlFile.empty() || m_xmlFile != filename)
			{
				m_propertyInfo.Init(doc);
			}
			m_mapInfo.Init(doc, index);
			m_xmlFile = filename;
			delete[] buf;
			return;
		}
		delete[] buf;
	}

	throw WAppException(
		"\xc5\xd8\xbd\xba\xc3\xc4 \xbc\xd3\xbc\xba \xc6\xc4\xc0\xcf\xc0\xbb \xb7\xce\xb5\xf9 \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9");
}

// B870: SendReport 983 00666580 (m_rivalList +0x1042c, passo 0x1b8, nick +8; m_userInfo +0x38b0+0x18, passo 0x31c6)
C_ASSERT_VC10(offsetof(CSharedDoc, m_rivalList) == 0x1042c && sizeof(sRivalData) == 0x1b8 && offsetof(sRivalData, nickname) == 8);

void CGolfDoc::SendReport()
{
	if (!m_bReport)
	{
		return;
	}

	char dir[MAX_PATH] = { 0 };
	strcpy(dir, g_executeDirectory);
	char buf[1024] = { 0 };

	FILE* fp = fopen(MakeStr("%s\\chat_history.txt", dir), "wt");
	if (fp == NULL)
	{
		return;
	}

	SYSTEMTIME time;
	GetLocalTime(&time);
	sprintf(buf,
		K2L_Compatibility("\xbd\xc5\xb0\xed\xbd\xc3\xb0\xa3 : %d:%02d:%02d %02d:%02d:%02d\n"),
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute,
		time.wSecond);
	fwrite(buf, strlen(buf), 1, fp);
	sprintf(buf,
		"------------------------------------------------------------\n");
	fwrite(buf, strlen(buf), 1, fp);
	sprintf(buf, K2L_Compatibility("\xbe\xc6\xc0\xcc\xb5\xf0 : \xb4\xeb\xc8\xad\xb8\xed\n"));
	fwrite(buf, strlen(buf), 1, fp);

	if (IsMassGame())
	{
		for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
			it != Doc()->m_rivalList.end(); ++it)
		{
			sprintf(buf, "%s\n", it->nickname);
			fwrite(buf, strlen(buf), 1, fp);
		}
	}
	else
	{
		for (unsigned char i = 0; i < m_playerNum; i++)
		{
			sprintf(buf, "\t%s\n", Doc()->m_userInfo[i].info.sNick);
			fwrite(buf, strlen(buf), 1, fp);
		}
	}

	sprintf(buf,
		"------------------------------------------------------------\n");
	fwrite(buf, strlen(buf), 1, fp);

	for (std::list<sChatLine>::iterator it = Doc()->m_chatLineList.begin();
		it != Doc()->m_chatLineList.end(); ++it)
	{
		fwrite(it->text.c_str(), strlen(it->text.c_str()), 1, fp);
		fwrite("\n", 1, 1, fp);
	}

	fclose(fp);
	_snprintf(buf, sizeof(buf) - 1, "%s %d %s\\chat_history.txt",
		Doc()->m_myInfo.info.sNick, 4, dir);
	CPangFBI::Instance()->ProcessReport("crime_log", buf);
}

void CGolfDoc::RefreshPlayerNum()
{
	m_currentPlayerNum = 0;
	for (unsigned char i = 0; i < m_playerNum; ++i)
	{
		if (m_pPlayer[i].state != 3)
		{
			++m_currentPlayerNum;
		}
	}
}

void CGolfDoc::SetCurrentPlayerNum(int num)
{
	m_currentPlayerNum = num;
}

unsigned char CGolfDoc::GetCurrentPlayerNum()
{
	return m_currentPlayerNum;
}

CPartTidList& CGolfDoc::GetPartTidList(unsigned char index)
{
	return m_partTidList[index];
}

void CGolfDoc::ClearShotResultMsg()
{
	memset(m_shotResultMsg, 0, sizeof(m_shotResultMsg));
}

void CGolfDoc::LogStrokeScore()
{
}

void CWaveInfo::Load(const char* filename)
{
	cFile* file = g_resrcmng->GetCFile(filename, 0xffff);
	if (file == NULL)
	{
		return;
	}

	m_wave.clear();

	char buf[1024];
	int len = file->Length();
	file->Read(buf, len);
	buf[len] = 0;

	char* next = buf;
	char* line = next;
	while (*next)
	{
		while (*next && *next != '\n')
		{
			next++;
		}
		if (*next)
		{
			next++;
		}

		if (line[0] != '/' || line[1] != '/')
		{
			int index;
			sWave wave;
			if (sscanf(line, "%d %s %f %f", &index, wave.texture, &wave.width,
					&wave.freq) == 4)
			{
				std::map<int, sWave>::iterator it = m_wave.find(index);
				if (it == m_wave.end())
				{
					wave.freq *= 0.001f;
					m_wave[index] = wave;
				}
			}
		}
		line = next;
	}

	CloseCFile(file);
}

void CRiverInfo::Load(const char* filename)
{
	cFile* file = g_resrcmng->GetCFile(filename, 0xffff);
	if (file == NULL)
	{
		return;
	}

	m_river.clear();

	char buf[1024];
	int len = file->Length();
	file->Read(buf, len);
	buf[len] = 0;

	char* next = buf;
	char* line = next;
	while (*next)
	{
		while (*next && *next != '\n')
		{
			next++;
		}
		if (*next)
		{
			next++;
		}

		if (line[0] != '/' || line[1] != '/')
		{
			int index;
			sRiver river;
			if (sscanf(line, "%d %f %f %f %f", &index, &river.scrollVel,
					&river.tileHead, &river.tileSide, &river.depth) == 5)
			{
				std::map<int, sRiver>::iterator it = m_river.find(index);
				if (it == m_river.end())
				{
					m_river[index] = river;
				}
			}
		}
		line = next;
	}

	CloseCFile(file);
}

void CMapInfo::Init(const TiXmlDocument& doc, unsigned char index)
{
	TiXmlNode* project = doc.FirstChild("round")->FirstChild("project");
	if (project == NULL)
	{
		return;
	}

	m_item.clear();

	TiXmlNode* wep = project->FirstChild("wep");
	for (unsigned char i = 0; i < index; i++)
	{
		wep = wep->NextSibling();
	}

	std::string name = wep->ToElement()->Attribute("name");
	TiXmlNode* node = wep->FirstChild();
	while (node)
	{
		std::string key = node->ToElement()->Value();
		std::string value = node->FirstChild()->Value();
		m_item[key] = value;
		node = node->NextSibling();
	}
}

const char* CMapInfo::FindItem(const std::string& key)
{
	if (m_item.find(key) == m_item.end())
	{
		return NULL;
	}
	return m_item[key].c_str();
}

void sBar::Reset()
{
	gauge = BAR_MIDDLE;
	gaugeStep = BAR_MIDDLE;
	center = BAR_START;
	power = BAR_START;
	impact = BAR_START;
	bFullPower = false;
	dir = 0;
	calipers = BAR_START;
}

void CGolfDoc::SetApproachMissionDesc(const char* desc)
{
	strcpy(m_approachMissionDesc, desc);
}

char* CGolfDoc::GetApproachMissionDesc()
{
	return m_approachMissionDesc;
}

void CGolfDoc::SetApproachMissionType(eApproachMissionType type)
{
	m_approachMissionType = type;
}

void CGolfDoc::SetApproachPrizeType(eApproachPrizeType type)
{
	m_approachPrizeType = type;
}

void CGolfDoc::SetApproachWinnerType(eApproachWinnerType type)
{
	m_approachWinnerType = type;
}

eApproachMissionType CGolfDoc::GetApproachMissionType() const
{
	return m_approachMissionType;
}

eApproachPrizeType CGolfDoc::GetApproachPrizeType() const
{
	return m_approachPrizeType;
}

eApproachWinnerType CGolfDoc::GetApproachWinnerType() const
{
	return m_approachWinnerType;
}

std::list<sApproachResultData*>& CGolfDoc::GetApproachCurHoleResult()
{
	return m_approachCurHoleResult;
}

std::list<sApproachResultData*>& CGolfDoc::GetApproachGameResult()
{
	return m_approachGameResult;
}

void CGolfDoc::AddApproachTotalPrize(unsigned long prize)
{
	m_approachTotalPrize += prize;
}

unsigned long CGolfDoc::GetApproachTotalPrize() const
{
	return m_approachTotalPrize;
}

int __fastcall CalcPowerPenalty(int upgrade);

void CGolfDoc::SetPlayerLevel(unsigned char index)
{
	sPlayerData* player = GetPlayer(index);
	unsigned long tidChar = Doc()->m_userInfo[index].charInfo.tid;
	const unsigned long* pParts = Doc()->m_userInfo[index].charInfo.tidParts;
	const unsigned long* pAuxParts =
		Doc()->m_userInfo[index].charInfo.tidAuxParts;
	unsigned long tidClub = Doc()->m_userInfo[index].clubInfo.tid;
	const short* pClubStat = Doc()->m_userInfo[index].clubInfo.PCL;
	const char* pStat = Doc()->m_userInfo[index].charInfo.PCL;
	unsigned long tidCaddie = Doc()->m_userInfo[index].caddieInfo.tid;
	unsigned char level = Doc()->m_userInfo[index].stat.Level;

	if (Doc()->m_offlinePlay)
	{
		player->power = rand() % 20 + 10;
		player->control = rand() % 10 + 15;
		player->accuracy = rand() % 5 + 5;
		player->spin = rand() % 5 + 5;
		player->curve = rand() % 5 + 5;
	}
	else
	{
		if (IsLocalContent(S4_CARD_SYSTEM))
			CCardManager::Instance()->SetPlayerIndex(index);

		// 983 00664e51..: GetLevel 00a23620 recebe UMA sCapacityParam983 (ebp-0x60) com guid/slots do taco
		// (+0x6a1c/+0x6a2e), mascote (+0x6a3c) e maestria (+0x69cf); a chamada do 645 deixava fora
		// mascote, Club Workshop e maestria (B1264)
		const sUserInfo& ui = Doc()->m_userInfo[index];
		sCapacityParam983 param;
		param.tidChar = tidChar;
		param.m_8 = (unsigned long)pStat;
		param.tidParts = pParts;
		param.tidAuxParts = pAuxParts;
		param.tidClub = tidClub;
		param.clubAttrUp = pClubStat;
		param.clubSlotUp = ui.clubInfo.slotUp983;
		param.guidClub = ui.clubInfo.guid;
		param.tidCaddie = tidCaddie;
		param.m_28 = ui.mascotInfo.tid;
		param.level = level;
		param.m_30 = (int)ui.charInfo.mastery;
		param.type = LVL_POWER;
		player->power = Doc()->m_itemManager.GetLevel(&param);
		param.type = LVL_CONTROL;
		player->control = Doc()->m_itemManager.GetLevel(&param);
		param.type = LVL_ACCURACY;
		player->accuracy = Doc()->m_itemManager.GetLevel(&param);
		param.type = LVL_SPIN;
		player->spin = Doc()->m_itemManager.GetLevel(&param);
		param.type = LVL_CURVE;
		player->curve = Doc()->m_itemManager.GetLevel(&param);
	}

	int power = player->power;
	if (power < 0)
		power = 0;
	else if (power > 100)
		power = 100;
	player->power = power - 15;

	player->control -= Max(0, player->power - (CalcPowerPenalty(level) + 5));
	player->accuracy -= Max(0, player->power - (CalcPowerPenalty(level) + 5));

	int accuracy = player->accuracy;
	if (accuracy < 0)
		accuracy = 0;
	else if (accuracy > 30)
		accuracy = 30;
	player->accuracy = accuracy;

	int control = player->control;
	if (control < 0)
		control = 0;
	else if (control > 30)
		control = 30;
	player->control = control;

	int spin = player->spin;
	if (spin < 1)
		spin = 1;
	else if (spin > 30)
		spin = 30;
	player->spin = spin;

	int curve = player->curve;
	if (curve < 1)
		curve = 1;
	else if (curve > 30)
		curve = 30;
	player->curve = curve;
}

void CGolfDoc::ClearApproachCurHoleResult()
{
	std::list<sApproachResultData*>::iterator it;
	for (it = m_approachCurHoleResult.begin();
		it != m_approachCurHoleResult.end(); ++it)
	{
		if (*it != NULL)
		{
			delete *it;
			*it = NULL;
		}
	}
	m_approachCurHoleResult.clear();
}

void CGolfDoc::ClearApproachGameResult()
{
	std::list<sApproachResultData*>::iterator it;
	for (it = m_approachGameResult.begin(); it != m_approachGameResult.end();
		++it)
	{
		if (*it != NULL)
		{
			delete *it;
			*it = NULL;
		}
	}
	m_approachGameResult.clear();
}

sApproachResultData* CGolfDoc::GetAppMyCurHoleResult()
{
	for (std::list<sApproachResultData*>::iterator it = m_approachCurHoleResult.begin();
		it != m_approachCurHoleResult.end(); ++it)
	{
		sApproachResultData* data = *it;
		if (data != NULL && data->dwGUID == MyGuid(false))
		{
			return data;
		}
	}
	return NULL;
}

sApproachResultData* CGolfDoc::GetAppMyGameResult()
{
	for (std::list<sApproachResultData*>::iterator it = m_approachGameResult.begin();
		it != m_approachGameResult.end(); ++it)
	{
		sApproachResultData* data = *it;
		if (data != NULL && data->dwGUID == MyGuid(false))
		{
			return data;
		}
	}
	return NULL;
}

void CGolfDoc::ApproachDataAllClear()
{
	m_bApproach795 = false;
	m_approachMissionType = (eApproachMissionType)0;
	m_approachPrizeType = (eApproachPrizeType)0;
	m_approachWinnerType = (eApproachWinnerType)0;
	m_approachCurHoleResult.clear();
	m_approachGameResult.clear();
	m_approachTotalPrize = 0;
}

CPropertyInfo::~CPropertyInfo()
{
}

void CPropertyInfo::Init(const TiXmlDocument& doc)
{
	// HACK: dead references; they emit the CGolfDoc inlines below in the original's place (emission order).
	if (0)
		((CGolfDoc*)0)->CGolfDoc::GetCameraData();
	if (0)
		new ((void*)0) CGolfDoc::sTarget;
	TiXmlNode* round = doc.FirstChild("round");
	if (round == NULL)
	{
		throw WAppException(std::string("app exception"));
	}

	TiXmlNode* declare = round->FirstChild("declare");
	if (declare == NULL)
	{
		return;
	}

	m_property.clear();
	m_textureIndex.clear();

	int count = 0;
	declare->ToElement()->Attribute("count", &count);
	if (count)
	{
		m_property.reserve(count);
	}

	m_defaultIndex = -1;
	int index = 0;

	sProperty prop;
	char type[128];

	struct sItem
	{
		const char* name;
		const char* format;
		void* value;
	};
	sItem item[] = {
		{ "bound",       "%f", &prop.bound     },
		{ "roll",        "%f", &prop.roll      },
		{ "min",         "%f", &prop.min[0]    },
		{ "max",         "%f", &prop.max[0]    },
		{ "min2",        "%f", &prop.min[1]    },
		{ "max2",        "%f", &prop.max[1]    },
		{ "min3",        "%f", &prop.min[2]    },
		{ "max3",        "%f", &prop.max[2]    },
		{ "pass",        "%d", &prop.pass      },
		{ "pet",         "%s", type            },
		{ "bound_sound", "%s", prop.boundSound },
		{ "roll_sound",  "%s", prop.rollSound  },
	};

	TiXmlNode* defaultNode = declare->FirstChild("class");
	while (defaultNode)
	{
		m_defaultIndex++;
		std::string name = defaultNode->ToElement()->Attribute("name");
		if (name == "default")
		{
			break;
		}
		defaultNode = defaultNode->NextSibling("class");
	}

	TiXmlNode* node = declare->FirstChild("class");
	do
	{
		TiXmlElement* elem = node->ToElement();
		if (stricmp(elem->Value(), "class") != 0)
		{
			return;
		}

		std::string name = elem->Attribute("name");
		if (name.empty())
		{
			return;
		}

		memset(type, 0, sizeof(type));
		for (int i = 0; i < 12; i++)
		{
			TiXmlNode* child = node->FirstChild(item[i].name);
			if (child == NULL)
			{
				child = defaultNode->FirstChild(item[i].name);
			}
			std::string text = child->ToElement()->FirstChild()->Value();
			sscanf(text.c_str(), item[i].format, item[i].value);
		}

		if (strstr(type, "green"))
		{
			prop.ground = 2;
		}
		else if (strstr(type, "fairway"))
		{
			prop.ground = 1;
		}
		else if (strstr(type, "bunker"))
		{
			prop.ground = 3;
		}
		else if (strstr(type, "rough"))
		{
			prop.ground = 4;
		}
		else if (strstr(type, "snow"))
		{
			prop.ground = 5;
		}
		else if (strstr(type, "road"))
		{
			prop.ground = 6;
		}
		else if (strstr(type, "ice"))
		{
			prop.ground = 7;
		}
		else if (strstr(type, "water"))
		{
			prop.ground = 9;
		}
		else if (strstr(type, "sand"))
		{
			prop.ground = 11;
		}
		else if (strstr(type, "booster"))
		{
			prop.ground = 13;
		}
		else if (strstr(type, "special"))
		{
			prop.ground = 12;
		}
		else if (strstr(type, "wb"))
		{
			prop.ground = 14;
		}
		else if (strstr(type, "vector"))
		{
			prop.ground = 8;
		}

		char* ext = strstr(prop.boundSound, ".wav");
		if (ext)
		{
			*ext = 0;
		}
		ext = strstr(prop.rollSound, ".wav");
		if (ext)
		{
			*ext = 0;
		}

		int ground = prop.ground;
		if (prop.ground == 0 || prop.ground == 4)
		{
			if (GetCurMap() == 10)
			{
				ground = 11;
			}
			else
			{
				ground = IsCurMapSnow() ? 5 : 2;
			}
		}

		switch (ground)
		{
		case 5:
			strcpy(prop.stepSound, "\xb9\xdf\xc0\xda\xb1\xb9_snow");
			break;
		case 7:
			strcpy(prop.stepSound, "\xb9\xdf\xc0\xda\xb1\xb9_ice");
			break;
		case 3:
		case 11:
			strcpy(prop.stepSound, "\xb9\xdf\xc0\xda\xb1\xb9_sand");
			break;
		default:
			strcpy(prop.stepSound, "\xb9\xdf\xc0\xda\xb1\xb9_green");
			break;
		case 9:
			strcpy(prop.stepSound, "\xb9\xdf\xc0\xda\xb1\xb9_water");
			break;
		}

		m_property.push_back(prop);

		TiXmlNode* textures = node->FirstChild("textures");
		if (textures)
		{
			TiXmlNode* texture = textures->FirstChild("item");
			while (texture)
			{
				std::string text = texture->ToElement()->FirstChild()->Value();
				texture = texture->NextSibling();
				m_textureIndex[text] = index;
			}
		}

		index++;
		node = node->NextSibling("class");
	} while (node);
}

int CPropertyInfo::GetPropertyIndex(const char* texture)
{
	std::map<std::string, int>::iterator it = m_textureIndex.find(texture);
	if (it == m_textureIndex.end())
	{
		return m_defaultIndex;
	}
	return m_textureIndex[texture];
}

void CGolfDoc::ResetTeamVars()
{
	if (m_pTeam == NULL)
		return;

	for (int i = 0; i < 2; ++i)
	{
		m_pTeam[i].color = (i == 0 ? 0xffff0000 : 0xff0060ff) & 0x00ffffff;
		m_pTeam[i].state = 2;
		m_pTeam[i].gauge = 0.0f;
		m_pTeam[i].totalStroke = 0;
		m_pTeam[i].score = 0;
		m_pTeam[i].totalPang = 0;
		m_pTeam[i].bonusPang = 0;
		m_pTeam[i].shooter = 0;
		m_pTeam[i].club = 0;
		m_pTeam[i].turn = 0;
		m_pTeam[i].teeShooter = 0;
		m_pTeam[i].holeInPose = 0;
		m_pTeam[i].holeInCollision = 0;
		for (int hole = 0; hole < 18; ++hole)
		{
			m_pTeam[i].stroke[hole] = 0;
			m_pTeam[i].putt[hole] = 0;
			m_pTeam[i].pang[hole] = 0;
		}

		if (OnlinePlay() && HackingManager::IsInstantiated())
		{
			for (int hole = 0; hole < 18; ++hole)
			{
				HackingManager::Instance()->AddCrypticByteValue(
					&m_pTeam[i].stroke[hole]);
				HackingManager::Instance()->AddCrypticByteValue(
					&m_pTeam[i].putt[hole]);
				HackingManager::Instance()->AddCrypticIntValue(
					&m_pTeam[i].pang[hole]);
			}
			HackingManager::Instance()->AddCrypticIntValue(
				&m_pTeam[i].totalPang);
			HackingManager::Instance()->AddCrypticIntValue(
				&m_pTeam[i].bonusPang);
			HackingManager::Instance()->AddCrypticFloatValue(&m_pTeam[i].gauge);
		}
	}
}

// 983 00666950: chama o WView::GetFOV de verdade (00b4fa30); o 645 lia SCREEN_XS/SCREEN_YS/FOV direto
WMatrix CGolfDoc::MakeTopViewCameraMatrix(bool check)
{
	WMatrix mat;
	mat.za = WVector::UNIT_NEG_Y;
	mat.ya = GOLFDOC()->m_cameraData.mat.za;
	mat.ya.Normalize();
	mat.xa = WCrossProduct(mat.ya, mat.za);

	WVector pin = GetHoleCupPos();
	sPlayerData* player = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
	const WVector& pos = player->pos;
	WVector d = pin - pos;
	d.y = 0.0f;
	float dist = d.Magnitude();

	if (g_input->Get("LCONTROL", true) || g_input->Get("RCONTROL", true))
	{
		mat.pivot = (GetHoleCupPos() + GetTeeShotPos()) * 0.5f;
		mat.pivot.y = Max(GetHoleCupPos().y, GetTeeShotPos().y) +
			(Abs(GetHoleCupPos().z - GetTeeShotPos().z) + 480.0f) * 0.5f /
				(tanf(g_view->GetFOV() * 0.5) * g_view->GetRatio()); // 983 00666fe9
	}
	else if ((g_input->Get("LSHIFT", true) || g_input->Get("RSHIFT", true) ||
				 g_input->Get(1)) &&
		IsMyTurn(true))
	{
		CGolfDoc* doc = Doc()->m_pGolfDoc;
		// 983 (00666950, apos o teste de LSHIFT): o alcance do taco*3.2 e limitado a distancia ate o pin (local_8)
		float reach = GolfClub().GetRange() * 3.2f;
		if (dist < reach)
			reach = dist;
		WVector offset = reach * doc->m_cameraData.mat.za;
		WVector pos = offset + GolfBall().m_pos;
		mat.pivot = pos;
		int index;
		mat.pivot.y =
			GetPVS().GetGroundPoint(pos, true, &index, true).y + 64.0f;
		if (index < 0)
			mat.pivot.y = GolfBall().m_pos.y + 64.0f;
	}
	else if (GolfBall().m_groundType == 2 &&
		dist < GolfClub().GetRange() * 3.2f)
	{
		if (dist < 16.0f)
		{
			mat.pivot =
				(GOLFDOC()->m_cameraData.mat.za * 0.2f + WVector::UNIT_POS_Y) *
					32.0f +
				GolfBall().m_pos;
		}
		else
		{
			mat.pivot = (GOLFDOC()->m_cameraData.mat.za * 0.4f +
							WVector(0.0f, 2.0f, 0.0f)) *
					dist +
				GolfBall().m_pos;
		}
	}
	else if (GolfClub().GetApproachMode() <= 3)
	{
		mat.pivot = (GOLFDOC()->m_cameraData.mat.za * 0.5f +
						WVector(0.0f, 1.4f, 0.0f)) *
				Max(32.0f, dist) +
			GolfBall().m_pos;
	}
	else
	{
		mat.pivot = (GOLFDOC()->m_cameraData.mat.za * 0.5f +
						WVector(0.0f, 1.4f, 0.0f)) *
				(GolfClub().GetRange() * 3.2f) +
			GolfBall().m_pos;
	}

	if (check &&
		(mat.pivot.y < g_camera.pivot.y ||
			WVectorLen(mat.pivot - g_camera.pivot) > 32.0f))
		return g_camera;
	return mat;
}

void CGolfDoc::MakeDefaultCamera(bool apply)
{
	// HACK: dead reference: GameCamera (shareddoc.inl) is emitted by this unit in the original
	if (0)
	{
		GameCamera();
		// the naked RotMat (stdafx.h) has no C body: its callees are referenced here
		(void)cosf(m_cameraData.angle);
		(void)sinf(m_cameraData.angle);
		(void)WMatrix(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	}
	if (GolfBall().m_groundType == 2)
	{
		m_cameraData.distance = 15.0f;
		m_cameraData.height = 7.0f;
	}
	else
	{
		m_cameraData.distance = 20.0f;
		m_cameraData.height = 5.0f;
	}

	WVector dir;
	dir = RotMat(m_cameraData.angle, 1).za;
	dir.y = 0.0f;
	dir.Normalize();

	if (!IsMyTurn(true) &&
		(!IsLocalContent(S4_REPLAY_SYSTEM) || Doc()->m_playingMode != 2))
	{
		m_cameraData.target = dir * 15.0f + GolfBall().m_pos;
		m_cameraData.distance += 20.0f;
		if (GolfBall().m_groundType == 2)
		{
			m_cameraData.height += 3.0f;
		}
		dir = dir * RotMat(m_cameraData.angleOffset, 1);
	}
	else
	{
		m_cameraData.target = GolfBall().m_pos;
	}

	CItemManager& itemManager = Doc()->m_itemManager;
	IFF_STRUCT::sChar* chr = itemManager.FindChar(
		Doc()->m_userInfo[Doc()->m_pGolfDoc->m_currentPlayer].charInfo.tid);
	if (chr)
	{
		m_cameraData.distance = chr->ClubScale * m_cameraData.distance;
		m_cameraData.height = chr->ClubScale * m_cameraData.height;
	}

	WVector at;
	at = dir + m_cameraData.target;
	m_cameraData.mat = MakeCameraMatrix(m_cameraData.target, at);
	m_cameraData.mat.pivot -= m_cameraData.distance * m_cameraData.mat.za;

	float minHeight = m_cameraData.target.y + m_cameraData.height;
	float height =
		GetPVS().GetGroundPoint(m_cameraData.mat.pivot, false, NULL, false).y +
		m_cameraData.clearance;
	if (height < minHeight)
	{
		height = minHeight;
	}
	m_cameraData.mat.pivot.y = height;

	if (apply)
	{
		at.y = tanf(m_cameraData.pitch) + at.y;
		g_camera = MakeCameraMatrix(m_cameraData.target, at);
		g_camera.pivot = m_cameraData.mat.pivot;
	}

	g_view->SetFOV(0.9599311f);
}

const sProperty& CPropertyInfo::GetProperty(int index)
{
	if (index < 0 || (unsigned int)index >= m_property.size())
	{
		index = m_defaultIndex;
	}
	return m_property[index];
}

void CGolfDoc::SetApproachCurHoleResult(sApproachResultData& result)
{
	sApproachResultData* data = new sApproachResultData;
	if (data != NULL)
	{
		*data = result;
		m_approachCurHoleResult.push_back(data);
	}
}

void CGolfDoc::SetApproachGameResult(sApproachResultData& result)
{
	sApproachResultData* data = new sApproachResultData();
	if (data == NULL)
	{
		return;
	}
	*data = result;
	std::list<sApproachResultData*>::iterator it;
	for (it = m_approachGameResult.begin(); it != m_approachGameResult.end();
		++it)
	{
		if ((*it)->dwGUID == result.dwGUID)
		{
			break;
		}
	}
	if (it == m_approachGameResult.end())
	{
		m_approachGameResult.push_back(data);
		return;
	}
	delete data;
}

// JP 983 (00661fb0): corpo fora de linha, stdcall, lista como 1o argumento; o membro (00662070) so calcula
// &m_partTidList[index] e repassa os 5 argumentos
__declspec(noinline) void __stdcall GolfDocSetPartTidList983(CPartTidList* list,
	unsigned long charTid, unsigned char hairColor, unsigned char shirtColor,
	unsigned long* partTid, unsigned long* partId)
{
	sCharacterInfo info;
	info.tid = charTid;
	info.hairClr = hairColor;
	info.shirtsClr = shirtColor;
	memcpy(info.tidParts, partTid, sizeof(info.tidParts));
	if (partId != NULL)
	{
		memcpy(info.ItemIdList, partId, sizeof(info.ItemIdList));
	}
	if (!list->SetTids(&info, 0xff))
	{
		list->SetDefaultTids();
	}
	if (!list->IsComboValid())
	{
		list->SetDefaultTids();
	}
}

void CGolfDoc::SetPartTidList(unsigned char index, unsigned long charTid,
	unsigned char hairColor, unsigned char shirtColor, unsigned long* partTid,
	unsigned long* partId)
{
	GolfDocSetPartTidList983(&m_partTidList[index], charTid, hairColor,
		shirtColor, partTid, partId);
}
