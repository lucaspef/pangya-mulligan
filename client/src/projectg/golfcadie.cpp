#include "minatl.h"
#define REBANG_GOLFCADIE_CPP
#include "golfcadie.h"
#include "effect.h"
#include "petbody.h"
// HACK: emission order. Creates the implicit Waabb::operator= before CQuadTree::SetCaddieBox, so that
// SetCaddieBox waits one wave only (as in the original).
inline void RbAssignBox(Waabb& dst, const Waabb& src)
{
	dst = src;
}
#include "quadtree.h"
#include "projectg.h"
#include "shareddoc.h"
#include "golfball.h"
#include "scenemanager.h"
#include "networksystem.h"
#include "shadow.h"
#include "fx.h"
#include "chatmsg.h"
#include "fxinterpreter.h"
#include "randomhouse.h"
#include "commonutil.h"
#include "slicestr.h"
#include "tinyxml.h"
#include "autotarget.h"
#include "club.h"
#include "wind.h"
#include "inputmanager.h"
#include "../../include/engine/wresrcmng.h"
#include "../../include/engine/westpak.h"
#include "../../include/engine/cfile.h"
#include "../../include/engine/wbone.h"


extern WMatrix g_camera;

bool IsCurMapSnow();
extern unsigned char MyIndex();

// HACK: local look-alikes (aliased at link time) until camerawork / polysoup.h declare the real ones
// (DEPENDE: camerawork; WPolySoup::FindPoint is missing from the PCH header).
class CCameraWork : public WSingleton<CCameraWork>
{
public:
	// B1465: o m_camCandidates (vector<int>) fica antes: no cl 14.44 (0xc x 0x10) o m_b1de real e 0x206, nao 0x20a (medido out/B1465)
	unsigned char m_pad[0x1e6 - (0x10 - sizeof(std::vector<int>))];
	bool m_bCaddieRun; // 983 +0x20e (645 +0x1de; 0073c0b5/0073c243/0073c259)
};
C_ASSERT_VC10(offsetof(CCameraWork, m_bCaddieRun) == 0x20e);

struct WPolySoupPoint
{
	unsigned char m_pad[0x24];
	WVector pos; // +0x24
};

class WPolySoupFind
{
public:
	WPolySoupPoint* FindPoint(const char* name);
};
#pragma comment(linker, \
	"/alternatename:?FindPoint@WPolySoupFind@@QAEPAUWPolySoupPoint@@PBD@Z=?FindPoint@WPolySoup@@QAEPAUsPoint@1@PBD@Z")

class CWindTurbulence
{
public:
	bool IsInTurbulence(const WVector& pos);
};

// Helpers used only by functions that are naked asm in the matching build.
// This compiler emits every referenced inline function out of line, and the
// original has none of these in this unit, so the dead C copies inside the
// naked functions get macros instead.
static inline int WisEqualRef(const float& left, const float& right,
	const float& epsilon = g_EPSILON)
{
	return Wabs(left - right) < epsilon;
}

// WPuppet::GetFrameData is inline in the original header; kept local so the
// PCH header does not change.
static inline char* PuppetFrameData(WPuppet* pet, int index)
{
	return pet->m_frame[index].data;
}

static inline unsigned char HoleIndex(unsigned char hole)
{
	for (unsigned char i = 0; i < 18; i++)
	{
		if (Doc()->m_holeOrder[i] == hole)
			return i + 1;
	}
	return 19;
}

static inline unsigned char CourseDifficulty()
{
	return Doc()->m_golfGame.pCourse ? Doc()->m_golfGame.pCourse->Difficulty
									 : 1;
}

// HACK: stands in for an unknown erased inline use of __rtti_obj
static __declspec(thread) int __rtti_tls_pad;

IMPLEMENT_ACTOR(CGolfCaddie, IActor)

sTalk talkTable[] = {
	{ "",			   false },
	{ "click1",         false },
	{ "click2",         false },
	{ "turnleft",       false },
	{ "turnright",      false },
	{ "great1",         false },
	{ "great2",         false },
	{ "club1",          false },
	{ "club2",          false },
	{ "shottime",       true  },
	{ "danger",         false },
	{ "ob",			 false },
	{ "wind",           true  },
	{ "putting",        false },
	{ "bunker",         true  },
	{ "transparent",    true  },
	{ "rival",          true  },
	{ "powershot",      true  },
	{ "ob_caution",     true  },
	{ "leader",         true  },
	{ "approach",       true  },
	{ "bestscore",      true  },
	{ "slopeup",        true  },
	{ "turbulence",     true  },
	{ "noisy",          true  },
	{ "trouble",        true  },
	{ "noitem",         false },
	{ "reitem",         true  },
	{ "bestdist",       true  },
	{ "ob_sequence",    true  },
	{ "ob_water",       true  },
	{ "dormie_if_win",  true  },
	{ "dormie_if_draw", true  },
	{ "dormie_shot",    true  },
	{ "eagle",          true  },
	{ "itemdrop",       true  },
};

bool XmlCaddieTalk::LoadSituation(const char* name, const parameters& params,
	std::vector<attributes>& out)
{
	TiXmlDocument doc;

	if (!doc.LoadFileEx("caddie_talk.xml"))
		return false;

	TiXmlNode* node = doc.FirstChild("situation");
	for (; node; node = node->NextSibling())
	{
		if (node->ToElement()->Attribute("name") == name)
			break;
	}

	if (!node)
		return false;

	TiXmlNode* param = node->FirstChild("param");
	while (param)
	{
		TiXmlElement* element = param->ToElement();
		std::vector<std::string>::const_iterator it = params.m_list.begin();
		attributes attr;
		while (it != params.m_list.end())
		{
			std::string value = element->Attribute((*it).c_str());
			attr.m_map[*it] = value;
			++it;
		}

		if (!attr.m_map.empty())
			out.push_back(attr);

		param = param->NextSibling();
	}

	return true;
}

void cCaddieTalkTeeShot::Load()
{
	parameters params;
	params.add("course");
	params.add("hole");
	params.add("caddie");
	params.add("talk");

	std::vector<attributes> attrs;
	if (!XmlCaddieTalk::LoadSituation("tee_shot", params, attrs))
		return;

	std::vector<attributes>::iterator it = attrs.begin();
	while (it != attrs.end())
	{
		sTalkData data;
		data.m_b = atoi((*it).get("course"));
		data.m_c = atoi((*it).get("hole"));
		data.m_a = atoi((*it).get("caddie"));
		data.m_msg = (*it).get("talk");
		m_talk.push_back(data);
		++it;
	}
}

bool cCaddieTalkTeeShot::FindMsg()
{
	if (GolfBall().m_groundType != 0)
		return false;

	std::map<unsigned int, sCaddieInfo>::iterator itCaddie =
		Doc()->m_caddieMap.find(Doc()->m_myInfo.userEquip.guidCaddie);
	if (itCaddie == Doc()->m_caddieMap.end())
		return false;

	sTalkData key;
	key.m_a = itCaddie->second.tid & 0x3ffffff;
	key.m_c = GOLFDOC()->m_currentHole;
	key.m_b = GetCurMap();

	for (std::vector<sTalkData>::iterator it = m_talk.begin();
		it != m_talk.end(); ++it)
	{
		if (*it == key)
		{
			m_msg = (*it).m_msg;
			m_talk.erase(it);
			return true;
		}
	}
	return false;
}

CCaddieTalk::~CCaddieTalk()
{
	std::for_each(m_talks.begin(), m_talks.end(), Delete_Object());
}

void CCaddieTalk::Release()
{
	std::for_each(m_talks.begin(), m_talks.end(), Delete_Object());
}

void CCaddieTalk::Init()
{
	m_busy = 0;

	ICaddieTalk* talk = new cCaddieTalkTeeShot;
	m_talks.push_back(talk);

	std::list<ICaddieTalk*>::iterator it = m_talks.begin();
	while (it != m_talks.end())
	{
		(*it)->Load();
		++it;
	}
}

const ICaddieTalk* CCaddieTalk::FindTalk()
{
	if (m_busy != 0)
		return 0;
	for (std::list<ICaddieTalk*>::iterator it = m_talks.begin();
		it != m_talks.end(); ++it)
	{
		if ((*it)->FindMsg())
		{
			m_busy = 1;
			return *it;
		}
	}
	return 0;
}

// 983 0073b530/00739830: esvazia o set de +0xac e o refaz com o nome (entrada+4) de cada linha ativa da CaddieVoiceTable
// (0x86 bytes: +0 ativa, +2 id do caddie, +4 nome[0x40], +0x44 nivel minimo) que serve ao caddie atual
void CGolfCaddie::RebuildCaddieVoiceNames983()
{
	if (!m_pCaddie)
		return;
	m_caddieVoiceNames983.clear();

	const std::vector<IFF983::sCaddieVoice>& voices =
		Doc()->m_itemManager.m_CaddieVoice983;
	for (size_t i = 0; i < voices.size(); i++)
	{
		const unsigned char* raw = voices[i].raw;
		if (raw[0] == 0)
			continue;
		if ((int)*(const short*)((const char*)m_pCaddie->m_pItem + 0xf6) !=
			(int)*(const unsigned short*)(raw + 2))
			continue;
		if (!m_pCaddie->m_pInfo || raw[0x44] > m_pCaddie->m_pInfo->Level)
			continue;
		m_caddieVoiceNames983[std::string((const char*)raw + 4)] = (const char*)raw + 0x45;
	}
}

// 983 00730e90: caddie com voz propria (id +0xf6 do item > 0)
bool CGolfCaddie::IsVoiceCaddie983() const
{
	return m_pCaddie && *(const short*)((const char*)m_pCaddie->m_pItem + 0xf6) > 0;
}

// 983 00735030: toca a fala do caddie com voz (CaddieVoiceTable) em vez do balao do XML
void CGolfCaddie::PlayCaddieVoice983(unsigned char type, unsigned char variant)
{
	if (g_audio->IsPlaying(m_curVoiceSfx983.c_str()) || m_bTalked[type])
		return;
	m_bTalked[type] = true;
	if (m_caddieVoiceNames983.empty())
		return;

	unsigned int n = variant;
	if (variant == 0)
		n = CRandomHouse::Instance()->GetRandom() % 5 + 1;
	std::map<std::string, const char*>::iterator it =
		m_caddieVoiceNames983.find(MakeStr("%s%d", talkTable[type].m_name, n));
	if (it == m_caddieVoiceNames983.end())
		return;

	m_curVoiceSfx983 = (*it).second;
	g_audio->PlaySfx(m_curVoiceSfx983.c_str(), NULL, 0, m_curVoiceSfx983.c_str(), NULL,
		0.5f, 200.0f);
}

void CGolfCaddie::OnDestroy()
{
	m_pFxInterpreter983->DetachFxAll();

	m_fxScript = -1;
	for (std::list<sCaddie*>::iterator it = m_caddies.begin();
		it != m_caddies.end(); it++)
	{
		if ((*it)->m_pBody)
		{
			delete (*it)->m_pBody;
			(*it)->m_pBody = 0;
		}
		if (*it)
		{
			delete *it;
			*it = 0;
		}
	}
	m_caddies.clear();

	if (m_pRing)
	{
		delete m_pRing;
		m_pRing = 0;
	}
	if (m_pCaddieTalk)
	{
		delete m_pCaddieTalk;
		m_pCaddieTalk = 0;
	}
}

void CGolfCaddie::OnPreLoadInit()
{
	CSceneManager::Instance()->RegisterElement(OBJ_ACTOR, 0, this,
		"\xc4\xb3\xb5\xf0", 0x18, 0);
}

void CGolfCaddie::OnLoad()
{
	OnDestroy();

	if (GOLFDOC()->m_tutorialMode < 15)
	{
		Doc()->m_userInfo[0].caddieInfo.guid = 0xffffffff;
		Doc()->m_userInfo[0].caddieInfo.tid = 0x1c000001;
	}

	for (unsigned char i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		if (GOLFDOC()->GetPlayer(i)->state != 3)
		{
			unsigned long tid = Doc()->m_userInfo[i].caddieInfo.tid;
			if (tid)
			{
				if (GOLFDOC()->m_tutorialMode < 15)
					AddCaddie(tid, 0, true);
				else
					AddCaddie(tid, Doc()->m_userInfo[i].caddieInfo.tidPart,
						false);
			}
		}
	}

	if (!m_pCaddieTalk)
	{
		m_pCaddieTalk = new CCaddieTalk;
		m_pCaddieTalk->Init();
	}
}

void CGolfCaddie::OnReload()
{
	OnLoad();
}

std::string CaddiePetFile983(const char* bodyTex, unsigned long skinTypeId, const char* caddieData)
{
	static const char* const fixed[][2] = {{"caddie_quma_ex_01.jpg", "quma_ex_02.pet"},
		{"caddie_dol_ex_01.jpg", "dol_ex_02.pet"}, {"pipin3_bd_01-01.jpg", "pipin_ex_02.pet"},
		{"[2#caddie_cadie_004.jpg", "cadie_ex_02.pet"}, {"[2#ho_body_001_02.jpg", "ho_mesh_ex02.pet"},
		{"[2#ho_body_002_04.jpg", "ho_mesh_ex03.pet"}};
	// maid: [corpo 1..3][caddie 0x1c00001f][skin nº >= 3]
	static const char* const maidTex[] = {"caddie_maid_body_001.jpg", "caddie_maid_body_002.jpg", "caddie_maid_body_003.jpg"};
	static const char* const maid[3][2][2] = {{{"maid.pet", "maid_12.pet"}, {"maid_1.pet", "maid_13.pet"}},
		{{"maid2.pet", "maid2_2.pet"}, {"maid2_1.pet", "maid2_3.pet"}},
		{{"maid3.pet", "maid3_2.pet"}, {"maid3_1.pet", "maid3_3.pet"}}};
	// Cien: [textura][caddie 0x1c000021 Armor, 22 Secret, 23 Rare]; NULL = fica a própria textura (como no 983)
	static const char* const cienTex[] = {"[2#cien_body-01.jpg", "[2#cien_body-02.jpg", "[2#cien_body-03.jpg",
		"[2#cien_body-04.jpg", "[2#cien_elemental_01.jpg"};
	static const char* const cien[5][3] = {{"Cien_Armor.pet", "Cien_Secret.pet", "Cien_Rare.pet"},
		{"Cien_Armor_W.pet", "Cien_Secret_W.pet", "Cien_Rare_W.pet"}, {NULL, NULL, "Cien_Rare_S.pet"},
		{"Cien_Armor_WS.pet", "Cien_Secret_WS.pet", NULL}, {"Cien_Armor_e.pet", "Cien_Secret_e.pet", "Cien_Rare_e.pet"}};
	// Karen: caddie 0x1c000025..2c -> normal/raro/ex; [pele p][tipo]
	static const int karenKind[8] = {0, 1, 2, 1, 0, 1, 2, 1};
	static const char* const karen[2][3] = {{"karen.pet", "karen_r_n.pet", "karen_ex_n.pet"},
		{"karen_p.pet", "karen_r_p.pet", "karen_ex_p.pet"}};

	std::string lower(bodyTex);
	std::transform(lower.begin(), lower.end(), lower.begin(), tolower);
	unsigned long caddie = ((skinTypeId >> 0x15) & 0x1f) + ((skinTypeId >> 0x10) & 0x1f) | 0x1c000000;
	const char* file = bodyTex;
	for (int i = 0; i < 6; ++i)
		if (lower == fixed[i][0])
			return fixed[i][1];
	for (int i = 0; i < 3; ++i)
		if (lower == maidTex[i])
			return maid[i][caddie == 0x1c00001f][(skinTypeId & 0x1fff) >= 3];
	for (int i = 0; i < 5; ++i)
		if (lower == cienTex[i])
		{
			if (caddie - 0x1c000021 < 3 && cien[i][caddie - 0x1c000021])
				file = cien[i][caddie - 0x1c000021];
			return file;
		}
	bool karenP = lower == "2#karen_p_001.jpg" || lower == "2#karen_r_p_001.jpg" || lower == "2#karen_ex_p_001.jpg";
	if (karenP || lower == "[2#karen_003.jpg")
	{
		if (caddie - 0x1c000025 < 8)
			file = karen[karenP][karenKind[caddie - 0x1c000025]];
		return file;
	}
	size_t pos = lower.find("_ex_");
	if (pos == std::string::npos)
		return std::string(caddieData) + "_ex.pet";
	// "..._ex_NN.jpg": o .pet é o trecho da 1ª letra minúscula até o "_ex"
	size_t i = 0;
	while (i < lower.size() && (lower[i] < 'a' || lower[i] > 'z'))
		++i;
	return lower.substr(i, pos + 3 - i) + ".pet";
}

CGolfCaddie::sCaddie* CGolfCaddie::AddCaddie(unsigned long id,
	unsigned long cadItem, bool unique)
{
	sCaddie* c = NULL; // 983 0073b4cd: sem o item do caddie devolve 0 (o 645 devolvia lixo)
	if (unique)
	{
		c = FindCaddie(id, cadItem);
		if (c)
			return c;
	}

	IFF_STRUCT::sCaddie* item = ItemManager()->FindCaddie(id);
	if (item)
	{
		c = new sCaddie;
		c->m_bRing = false;
		c->m_bCaddieView = true;
		c->m_pItem = item;
		if (cadItem)
		{
			IFF_STRUCT::sCadItem* part = ItemManager()->FindCadItem(cadItem);
			c->m_pBody = new CPetBody(MakeStr("*pet %s *shadow 1", CaddiePetFile983(part ? part->BodyTex : "",
				part ? part->c.TypeId : 0, item->Data).c_str()), false);
			if (part)
			{
				if (strlen(part->FaceTex))
					c->m_pBody->SetPendingTexPart("face", part->FaceTex);
				if (strlen(part->BodyTex))
					c->m_pBody->SetPendingTexPart("body", part->BodyTex);
			}
		}
		else
		{
			c->m_pBody = new CPetBody(
				MakeStr("*pet %s.pet *shadow 1", item->Data), false);
		}
		c->m_pBody->SetPos(WVector::ZERO);
		c->m_pBody->SetCustomGround("center");
		c->m_pBody->SetLight();

		LoadTalkText(c);
		LoadMotionLevel(c);
		ReadAttribute(c);
		m_caddies.push_back(c);
	}
	return c;
}

CGolfCaddie::sCaddie* CGolfCaddie::FindCaddie(unsigned long id,
	unsigned long cadItem)
{
	std::list<sCaddie*>::iterator it;
	it = std::find_if(m_caddies.begin(), m_caddies.end(),
		sFindCaddie(id, cadItem));
	if (it != m_caddies.end())
		return *it;
	return 0;
}

void CGolfCaddie::OnInit()
{
	m_pCaddie =
		FindCaddie(Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].caddieInfo.tid,
			Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].caddieInfo.tidPart);

	// 983 00734360: o corpo do caddie ganha o byte 0x188 (escala) so para os caddies 0x1c000018 e 0x1c00001d
	if (m_pCaddie)
	{
		unsigned long caddieTid = Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].caddieInfo.tid;
		m_pCaddie->m_pBody->SetTwin983(caddieTid == 0x1c000018 || caddieTid == 0x1c00001d);
	}

	m_state = 0;
	m_delay = 0;
	m_angle = 0;
	m_shotDir = 0;
	m_bDodging = false;
	m_bVisible = true;
	m_bNoItem = 0;
	m_clubDelta = 0;
	m_curVoiceSfx983.clear(); // 983 00734406: assign("", 0)
}

void CGolfCaddie::LoadTalkText(sCaddie* c)
{
	c->m_talks.clear();

	char fileName[32];
	fileName[0] = 0;
	sprintf(fileName, "%s.talk", c->m_pItem->Data);

	cFile* file = g_resrcmng->GetCFile(fileName, -1);
	if (!file)
		return;

	int length = file->Length();
	char* buffer;
	buffer = new char[length + 1];
	file->Read(buffer, length);
	buffer[length] = 0;
	CloseCFile(file);

	char* p = buffer;
	while (p && *p && strstr(p, ":"))
	{
		char name[256] = "";
		char level[256] = "";
		char motion[256] = "";

		char* next = GetBehaviorToken(name, p);
		if (!next || !*next || !name[0])
			break;
		next = GetBehaviorToken(level, next);
		if (!next || !*next || !level[0])
			break;
		p = GetBehaviorEndToken(motion, next);

		sBehavior behavior;
		behavior.m_level = atoi(level);
		behavior.m_motion = motion;
		c->m_talks[name] = behavior;
	}

	delete[] buffer;
}

void CGolfCaddie::LoadMotionLevel(sCaddie* c)
{
	c->m_motions.clear();

	char fileName[32];
	fileName[0] = 0;
	sprintf(fileName, "%s.mot", c->m_pItem->Data);

	cFile* file = g_resrcmng->GetCFile(fileName, -1);
	if (!file)
		return;

	int length = file->Length();
	char* buffer;
	buffer = new char[length + 1];
	file->Read(buffer, length);
	buffer[length] = 0;
	CloseCFile(file);

	char* p = buffer;
	while (p && *p && strstr(p, ":"))
	{
		char level[256] = "";
		char name[256] = "";

		char* end = GetBehaviorToken(level, p);
		if (!end || !*end || !level[0])
			break;
		p = GetBehaviorEndToken(name, end);

		sBehavior& behavior = c->m_motions[name];
		behavior.m_level = atoi(level);
	}

	delete[] buffer;
}

bool CGolfCaddie::IsGoodMotion(sCaddie* c, const char* motion)
{
	std::map<std::string, sBehavior>::iterator it = c->m_motions.find(motion);
	if (it == c->m_motions.end())
		return true;

	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		if (m_pCaddie->m_pInfo)
		{
			if (m_pCaddie->m_pInfo->Level < (*it).second.m_level)
				return false;
		}
		else
		{
			if ((*it).second.m_level > 0)
				return false;
		}
	}
	return true;
}

bool CGolfCaddie::ReadAttribute(sCaddie* c)
{
	WPuppet* pet = c->m_pBody->GetPet();
	if (pet)
	{
		w_motion_data* motion = pet->FindMotionData("\xb4\xde\xb8\xae\xb1\xe2");
		if (!motion)
			return false;

		int frame = (int)(motion->s * 30.0f + 0.5f);
		int count;
		int index = -1;
		pet->CheckFrameData(frame, frame, index, count);
		if (index > -1)
		{
			WSliceStr slice(PuppetFrameData(pet, index));

			char ringTexture[16];
			ringTexture[0] = '\0';
			c->m_bRing = slice.Get("*ring", "%s", ringTexture);
			if (c->m_bRing)
			{
				if (!m_pRing)
					m_pRing = new CRing(ringTexture);
				m_pRing->SetCenter(m_pos + WVector(0.0f, 0.3f, 0.0f));
				m_pRing->SetActive(true);
			}

			slice.Get("*stop_dist", "%f", &c->m_stopDist);
			c->m_stopDist *= 3.2f;
			slice.Get("*start_delay", "%f", &c->m_startDelay);
			slice.Get("*pet_speed", "%f", &c->m_petSpeed);
			bool bNoCaddieView = slice.Get("*no_caddie_view", "");
			c->m_petSpeed *= 3.2f;
			c->m_bCaddieView = !bNoCaddieView;
			return true;
		}
	}
	return false;
}

char* CGolfCaddie::GetBehaviorToken(char* dst, char* src)
{
	if (0) // HACK: OnProcess (naked) was the only user of this inline COMDAT
		((CPetBody*)0)->GetMotionName();
	if (0) // HACK: emission order (one wave later, as in the original)
		GetBehaviorEndToken(0, 0);
	while (*src == ' ' || *src == '\n')
		src++;
	while (*src)
	{
		if (*src == ' ')
		{
			src++;
			continue;
		}
		unsigned char c = (unsigned char)*src;
		if ((c > 0x7f && c < 0xa0) || (unsigned char)(c + 0x20) < 0x1d) // lead byte SJIS (983)
			*dst++ = *src++;
		*dst++ = *src++;
		if (*src == ':')
		{
			*dst = '\0';
			return src + 1;
		}
	}
	return 0;
}

char* CGolfCaddie::GetBehaviorEndToken(char* dst, char* src)
{
	while (*src == ' ')
		src++;
	*dst++ = *src++;
	while (*src != '\r' && *src != '\0')
		*dst++ = *src++;
	*dst = '\0';
	if (*src != '\0')
		src++;
	return src;
}

void CGolfCaddie::OnProcess(float dt)
{
	sCaddie* caddie = m_pCaddie;
	if (!caddie || !m_bVisible)
		return;

	if ((GOLFDOC()->m_gameMode & 0x80) && m_delay > 0.0f)
	{
		m_delay -= dt;
		if (WisZero(m_delay))
			m_delay = -0.00001f;
	}

	if (m_delay < 0.0f)
	{
		m_angle = CalcDeltaAngle(caddie->m_pBody->GetDirection(),
			WVector::UNIT_NEG_Z);
		m_angle += CalcDeltaAngle(m_target - m_pos,
			m_pCaddie->m_pBody->GetDirection());
		m_pCaddie->m_pBody->SetRotate(m_angle);
		m_delay = 0;
		if (m_state != 4)
		{
			m_state = 4;
			if (GOLFDOC()->m_tutorialMode == 15)
				m_pCaddie->m_pBody->SetMotion("\xb4\xde\xb8\xae\xb1\xe2", true,
					0.0f, false);
			else
				m_pCaddie->m_pBody->SetMotion(
					"\xc6\xa9\xc5\xcd\xb8\xae\xbe\xf3\xb4\xde\xb8\xae\xb1\xe2",
					true, 0.0f, false);
			m_runDust = CFx::Instance()->MakeRunDust(m_pos,
				m_pCaddie->m_pBody->GetVel());
			if (m_pRing && m_pCaddie->m_bRing)
				m_pRing->SetActive(false);
		}
		m_pCaddie->m_pBody->TakeOff(m_target);
	}

	if (m_state == 4)
	{
		Run();
	}
	else if (GolfBall().m_playFrame < 0)
	{
		switch (Doc()->m_golfGame.shotTimeLimit)
		{
		case 0:
			m_idleTime -= dt;
			if (m_idleTime < 0.0f)
			{
				m_idleTime = 20.0f;
				if (IsGoodMotion(m_pCaddie, "\xb1\xe2\xb4\xd9\xb8\xae\xb1\xe2"))
				{
					m_state = 1;
					m_pCaddie->m_pBody->SetMotion(
						"\xb1\xe2\xb4\xd9\xb8\xae\xb1\xe2", false, 0.0f, false);
				}
			}
			if (m_state == 1)
			{
				if (memcmp(m_pCaddie->m_pBody->GetMotionName(),
						"\xb1\xe2\xba\xbb", 5) == 0)
					m_state = 0;
			}
			break;
		case 40000:
			if (GOLFDOC()->m_remainTime < 10000 && m_state == 1)
			{
				if (IsGoodMotion(m_pCaddie, "\xc0\xe7\xc3\xcb\xc7\xcf\xb1\xe2"))
				{
					m_state = 3;
					m_pCaddie->m_pBody->SetMotion(
						"\xc0\xe7\xc3\xcb\xc7\xcf\xb1\xe2", false, 0.0f, false);
				}
			}
			else if (GOLFDOC()->m_remainTime < 20000 && m_state == 0)
			{
				if (IsGoodMotion(m_pCaddie, "\xb1\xe2\xb4\xd9\xb8\xae\xb1\xe2"))
				{
					m_state = 1;
					m_pCaddie->m_pBody->SetMotion(
						"\xb1\xe2\xb4\xd9\xb8\xae\xb1\xe2", false, 0.0f, false);
				}
			}
			else if (GOLFDOC()->m_remainTime > 20000)
			{
				m_state = 0;
			}
			break;
		case 60000:
			if (GOLFDOC()->m_remainTime < 10000 && m_state == 2)
			{
				if (IsGoodMotion(m_pCaddie, "\xc0\xe7\xc3\xcb\xc7\xcf\xb1\xe2"))
				{
					m_state = 3;
					m_pCaddie->m_pBody->SetMotion(
						"\xc0\xe7\xc3\xcb\xc7\xcf\xb1\xe2", false, 0.0f, false);
				}
			}
			else if (GOLFDOC()->m_remainTime < 20000 && m_state == 0)
			{
				if (IsGoodMotion(m_pCaddie, "\xb1\xe2\xb4\xd9\xb8\xae\xb1\xe2"))
				{
					m_state = 2;
					m_pCaddie->m_pBody->SetMotion(
						"\xb1\xe2\xb4\xd9\xb8\xae\xb1\xe2", false, 0.0f, false);
				}
			}
			else if (GOLFDOC()->m_remainTime < 40000 && m_state == 0)
			{
				if (IsGoodMotion(m_pCaddie, "\xb1\xe2\xb4\xd9\xb8\xae\xb1\xe2"))
				{
					m_state = 1;
					m_pCaddie->m_pBody->SetMotion(
						"\xb1\xe2\xb4\xd9\xb8\xae\xb1\xe2", false, 0.0f, false);
				}
			}
			else if (GOLFDOC()->m_remainTime > 40000)
			{
				m_state = 0;
			}
			break;
		}
	}

	m_pCaddie->m_pBody->Process(dt);
	if (m_pRing && m_pCaddie->m_bRing)
		m_pRing->Process(dt);

	if ((GOLFDOC()->m_gameMode & ~3) && GOLFDOC()->m_tutorialMode >= 2 &&
		!GOLFDOC()->m_shotPhase && GolfBall().m_playFrame < 0)
		TalkProcess(dt);
}


void CGolfCaddie::TalkProcess(float dt)
{
	bool bTalkTime = g_CurrentTime - m_talkTime > 1500;
	WVector dir = RotMat(m_shotDir, 1).za;

	if (GOLFDOC()->m_tutorialMode <= 6)
	{
		// tutorial stages
		if (Abs(m_clubDelta) > 1)
		{
			if (m_clubDelta > 0)
				SetBubble(1, false, 0);
			else
				SetBubble(2, false, 0);
		}

		WPolySoupPoint* point =
			((WPolySoupFind*)GOLFDOC()->m_pPolySoup)
				->FindPoint(MakeStr("\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1%d",
					GOLFDOC()->m_tutorialMode + 1));
		if (!point)
			return;

		float angle = CalcDeltaAngle(GOLFDOC()->m_cameraData.mat.za,
			point->pos - GolfBall().m_pos);
		float dist = (point->pos - GolfBall().m_pos).Magnitude();
		float remain = dist - GolfClub().GetRange() * 3.2f;

		if (!bTalkTime)
			return;
		if (m_bTalkFlag)
			return;

		if (angle < -0.5235988f)
		{
			m_bTalkFlag = true;
			SetBubble(3, false, 0);
		}
		else if (angle > 0.5235988f)
		{
			m_bTalkFlag = true;
			SetBubble(4, false, 0);
		}

		if (!((double)Abs(angle) < 0.1745329238474369))
			return;

		if (m_bTalked[3] || m_bTalked[4])
		{
			float limit = GOLFDOC()->m_tutorialMode == 2 ? 89.6f : 44.8f;
			if (Abs(remain) > limit)
				SetBubble(5, false, 0);
			else
				SetBubble(6, false, 0);
			m_bTalked[3] = false;
			m_bTalked[4] = false;
		}
		else if (GolfClub().Updated() && Abs(remain) < 48.0f)
		{
			SetBubble(6, false, 0);
		}

		float limit = GOLFDOC()->m_tutorialMode == 2 ? 89.6f : 44.8f;
		if (!(Abs(remain) > limit))
			return;
		if (remain > 0.0f)
			SetBubble(7, false, 0);
		else
			SetBubble(8, false, 0);
		return;
	}

	if (!m_bubbleTalkType && m_pCaddie)
	{
		const ICaddieTalk* talk = m_pCaddieTalk->FindTalk();
		if (talk)
		{
			WBone* bone =
				m_pCaddie->m_pBody->GetPet()->FindBone("Bip01 HeadNub");
			if (bone)
			{
				WVector pos = bone->GetMatrix().pivot;
				const char* msg = talk->GetTalkMsg();
				if (msg)
					SendMsg(this, "Bubble", M983(0x13), (int)msg, (int)&pos, 0, 0);
			}
		}
	}

	if (Doc()->m_golfGame.shotTimeLimit && GOLFDOC()->m_remainTime < 10000)
		SetBubble(9, false, 0);

	if (bTalkTime && !m_bTalkFlag)
	{
		if (GolfClub().GetType() != 3)
		{
			if (Abs(CalcDeltaAngle(dir, m_pos - GolfBall().m_pos)) <
				0.08726646f)
			{
				m_bTalkFlag = true;
				SetBubble(10, false, 0);
			}
		}

		int mesh = GetPVS().GetMeshIndex(
			GolfBall().m_pos + dir * (GolfClub().GetRange() * 3.2f), true);
		if (GolfClub().GetType() != 3 && mesh < 0)
		{
			m_bTalkFlag = true;
			SetBubble(11, false, 0);
		}
		if (m_bNoItem)
		{
			m_bTalkFlag = true;
			SetBubble(25, false, 0);
		}
	}

	if (GolfBall().m_groundType == 2)
	{
		if (bTalkTime && !m_bTalkFlag)
		{
			if (Abs(CalcDeltaAngle(dir, GetHoleCupPos() - GolfBall().m_pos)) >
				1.0471976f)
			{
				m_bTalkFlag = true;
				SetBubble(13, false, 0);
			}
		}

		if (Doc()->m_golfGame.shotTimeLimit &&
			GOLFDOC()->m_remainTime < 30000 &&
			m_pCaddie->m_pInfo->tid == 0x1c000000)
			SetBubble(15, false, 0);

		WVector pin = GOLFDOC()->GetHoleData().pin;
		float radius = GolfBall().m_radius;
		if (GolfBall().m_pos.y - radius + 1.6f < pin.y)
		{
			if (PLAYER(GOLFDOC()->m_currentPlayer)
						->putt[GOLFDOC()->m_currentHole - 1] ==
					(unsigned char)0 &&
				Doc()->m_golfGame.shotTimeLimit &&
				GOLFDOC()->m_remainTime <
					Doc()->m_golfGame.shotTimeLimit - 5000)
				SetBubble(22, false, 0);
		}
	}
	else
	{
		if (Doc()->m_golfGame.shotTimeLimit && GOLFDOC()->m_remainTime < 35000)
		{
			if (Wind().GetPower983() > 4.0f)
				SetBubble(12, false, 0);
		}
		if (GolfBall().m_groundType == 3)
			SetBubble(14, false, 0);
	}

	// 983 (00736f80): bubble 16 = quem tem menos tacadas no total empata com a gente (so strokeplay, a partir do buraco 3)
	if (Doc()->m_golfGame.gameType == 0 && GOLFDOC()->m_currentHole > 2 &&
		PLAYER(GOLFDOC()->m_currentPlayer)
				->stroke[GOLFDOC()->m_currentHole - 1] == (unsigned char)1)
	{
		int best = GOLFDOC()->m_currentPlayer;
		for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
		{
			if (PLAYER(i)->state != 3 && i != GOLFDOC()->m_currentPlayer &&
				PLAYER(i)->totalStroke <= PLAYER(best)->totalStroke)
				best = i;
		}
		if (best != GOLFDOC()->m_currentPlayer &&
			PLAYER(best)->totalStroke ==
				PLAYER(GOLFDOC()->m_currentPlayer)->totalStroke)
			SetBubble(16, false, 0);
	}

	// 983: par 5, tacada 1, gauge ja >= 33 -> 17 (sem vento/PVS como no 645)
	if (GOLFDOC()->GetHoleData().par == 5)
	{
		if (PLAYER(GOLFDOC()->m_currentPlayer)
				->stroke[GOLFDOC()->m_currentHole - 1] == (unsigned char)1)
		{
			if (PLAYER(GOLFDOC()->m_currentPlayer)->gauge >= 33.0f)
				SetBubble(17, false, 0);
		}
	}

	if (Doc()->m_golfGame.shotTimeLimit &&
		GOLFDOC()->m_remainTime < Doc()->m_golfGame.shotTimeLimit - 5000 &&
		GolfBall().m_groundType != 2 && GolfClub().GetApproachMode() < 4)
	{
		if (PLAYER(GOLFDOC()->m_currentPlayer)
				->stroke[GOLFDOC()->m_currentHole - 1] <=
			GOLFDOC()->GetHoleData().par)
			SetBubble(20, false, 0);
	}

	if (GolfClub().GetType() != 3 &&
		((CWindTurbulence&)Wind()).IsInTurbulence(g_camera.pivot))
	{
		m_turbulenceTime += dt;
		if (m_turbulenceTime > 2.0f)
		{
			WVector windVel = Wind().GetWind(g_camera.pivot);
			float wind = (float)sqrt((double)(windVel * windVel));
			if (wind > 11.0f)
				SetBubble(23, false, 5);
			else if (wind > 9.0f)
				SetBubble(23, false, 4);
			else if (wind > 7.0f)
				SetBubble(23, false, 3);
			else if (wind > 5.0f)
				SetBubble(23, false, 2);
			else if (wind > 3.0f)
				SetBubble(23, false, 1);
		}
	}
	else
	{
		m_turbulenceTime = 0;
	}

	if (IsMyTurn(false) && g_input->GetDown("ITEMSLOT", true) &&
		!(Doc()->m_curChannel.Type & 8))
	{
		if (!PLAYER(GOLFDOC()->m_currentPlayer)->itemNum)
			SetBubble(26, false, 0);
		else if (PLAYER(GOLFDOC()->m_currentPlayer)->item)
			SetBubble(27, false, 0);
	}

	if (Doc()->m_golfGame.gameType == 1)
	{
		if (HoleIndex(GOLFDOC()->m_currentHole) < Doc()->m_golfGame.holes)
		{
			if (TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)->score >=
				TEAM(1 - PLAYER(GOLFDOC()->m_currentPlayer)->team)->score -
					(HoleIndex(GOLFDOC()->m_currentHole) -
						Doc()->m_golfGame.holes))
			{
				SetBubble(
					TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)->score ==
							TEAM(1 - PLAYER(GOLFDOC()->m_currentPlayer)->team)
									->score -
								(HoleIndex(GOLFDOC()->m_currentHole) -
									Doc()->m_golfGame.holes)
						? 31
						: 32,
					false, 0);
				if (GolfBall().m_groundType == 2)
				{
					if (TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)
							->stroke[GOLFDOC()->m_currentHole - 1] <
						TEAM(1 - PLAYER(GOLFDOC()->m_currentPlayer)->team)
							->stroke[GOLFDOC()->m_currentHole - 1])
						SetBubble(33, false, 0);
				}
			}
		}

		if (TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)
				->stroke[GOLFDOC()->m_currentHole - 1] == (unsigned char)1)
		{
			for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
			{
				if (PLAYER(i)->team !=
						PLAYER(GOLFDOC()->m_currentPlayer)->team &&
					PLAYER(i)->flag == 1)
				{
					SetBubble(18, false, 0);
					break;
				}
			}
		}
	}
	else
	{
		if (Doc()->m_golfGame.gameType == 0)
		{
			int cur = GOLFDOC()->m_currentPlayer;
			if (PLAYER(cur)->stroke[GOLFDOC()->m_currentHole - 1] ==
				(unsigned char)1)
			{
				for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
				{
					if (PLAYER(i)->flag == 1 && i != MyIndex())
					{
						SetBubble(18, false, 0);
						break;
					}
				}
			}

			if (PLAYER(cur)->stroke[GOLFDOC()->m_currentHole - 1] ==
					(unsigned char)1 &&
				GOLFDOC()->m_playerNum > 1)
			{
				int i;
				for (i = 0; i < GOLFDOC()->m_playerNum; i++)
				{
					if (PLAYER(i)->state != 3 && i != cur &&
						PLAYER(i)->score < PLAYER(cur)->score + 2)
						break;
				}
				if (i == GOLFDOC()->m_playerNum &&
					(GOLFDOC()->m_currentHole & 3) == 0)
					SetBubble(19, false, 0);
			}

			// 983: flag 2 e ninguem com mais tacadas no total que a gente
			if (PLAYER(cur)->flag == 2 && GOLFDOC()->m_currentHole > 1)
			{
				unsigned char most = PLAYER(cur)->totalStroke;
				for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
				{
					if (PLAYER(i)->state != 3 && i != cur &&
						most < PLAYER(i)->totalStroke)
						most = PLAYER(i)->totalStroke;
				}
				if (most <= PLAYER(cur)->totalStroke)
					SetBubble(29, false, 0);
			}
		}

		if (PLAYER(GOLFDOC()->m_currentPlayer)->flag == 2)
		{
			int bEagle = 0;
			SendMsg(this, "NPC", M983(0x188), 4, (int)&bEagle, 0, 0);
			if (bEagle)
				SetBubble(30, false, 0);
		}
	}
}

// HACK: "Bip01 HeadNub" is owned by camerawork (a local static of its naked SetCamera, reached through
// /alternatename). A genuine literal COMDAT here would be kept as well, so the live C uses the alias.
#define RB_HEAD_NUB "Bip01 HeadNub"

void CGolfCaddie::SetBubble(unsigned char type, bool bChatOnly,
	unsigned char param)
{
	// 983 007358c0: voz primeiro; texto com ':' (dois falantes) poe o balao na media das duas cabecas
	if (IsVoiceCaddie983())
	{
		PlayCaddieVoice983(type, param);
		return;
	}
	sBehavior* behavior = SetTalkToBubble(type, bChatOnly, param);
	if (!behavior)
		return;

	m_bubbleTalkType = type;
	WPuppet* pet = m_pCaddie->m_pBody->GetPet();
	WBone* head = pet->FindBone(RB_HEAD_NUB);
	if (!head)
		return;
	WVector pos = head->GetMatrix().pivot;
	if (strstr(behavior->m_motion.c_str(), ":"))
	{
		WBone* head2 = pet->FindBone("Bip02 Ho02 HeadNub");
		if (head2)
		{
			const WVector& p2 = head2->GetMatrix().pivot;
			pos.x = (float)((p2.x + pos.x) * 0.5);
			pos.y = (float)((pos.y + p2.y) * 0.5);
			pos.z = (float)(0.5 * (pos.z + p2.z));
		}
	}
	SendMsg(this, "Bubble", M983(0x13), (int)behavior->m_motion.c_str(), (int)&pos, 0,
		0);
}

void CGolfCaddie::SetBubble(unsigned char type, const char* msg)
{
	if (m_pCaddie && !m_bTalked[type])
	{
		m_bTalked[type] = true;

		std::map<std::string, sBehavior>::iterator it =
			m_pCaddie->m_talks.find(talkTable[type].m_name);
		if (it == m_pCaddie->m_talks.end())
			return;

		if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
		{
			if (m_pCaddie->m_pInfo)
			{
				if (m_pCaddie->m_pInfo->Level < (*it).second.m_level)
					return;
			}
			else if ((*it).second.m_level > 0)
				return;
		}

		m_bubbleTalkType = type;
		WVector pos = m_pCaddie->m_pBody->GetPet()
						  ->FindBone(RB_HEAD_NUB)
						  ->GetMatrix()
						  .pivot;
		SendMsg(this, "Bubble", M983(0x13), (int)msg, (int)&pos, 0, 0);
	}
}

CGolfCaddie::sBehavior* CGolfCaddie::SetTalkToBubble(unsigned char type,
	bool bChatOnly, unsigned char param)
{
	if (!m_pCaddie || m_bTalked[type])
		return 0;
	if (!bChatOnly && m_bubbleTalkType)
		return 0;
	if (GOLFDOC()->m_tutorialMode != 15)
		return 0;

	m_bTalked[type] = true;
	std::map<std::string, sBehavior>::iterator it =
		m_pCaddie->m_talks.find(MakeStr("%s%d", talkTable[type].m_name,
			!param ? CRandomHouse::Instance()->GetRandom() % 5 + 1 : param));
	if (it == m_pCaddie->m_talks.end())
		return 0;

	sBehavior* behavior = &(*it).second;
	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		if (m_pCaddie->m_pInfo)
		{
			if (m_pCaddie->m_pInfo->Level < behavior->m_level)
				return 0;
		}
		else
		{
			if (behavior->m_level > 0)
				return 0;
		}
	}
	return behavior;
}

// 983 00734410: tira '/c' + 6 (cor), '/' e ':' do texto do balao (lead bytes SJIS)
static void StripTalkMarkup983(const std::string& in, std::string& out)
{
	unsigned char src[0x101] = {0};
	char dst[0x101] = {0};
	strncpy((char*)src, in.c_str(), 0x100);
	int n = 0;
	unsigned char* p = src;
	while (*p)
	{
		unsigned char c = *p;
		if (c == '/')
		{
			p += (p[1] == 'c') ? 8 : 1;
		}
		else if (c == ':')
		{
			p++;
		}
		else
		{
			bool dbcs = (c > 0x7f && c < 0xa0) || (unsigned char)(c + 0x20) < 0x1d;
			int at = n;
			if (dbcs)
			{
				dst[n++] = (char)c;
				p++;
			}
			dst[n++] = (char)*p++;
			if (!*p && dbcs)
				dst[at] = '\0';
		}
		if (p > src + 0x100)
			break;
	}
	out = dst;
}

void CGolfCaddie::SetCaddieTalk_ChatOnly(unsigned char type)
{
	if (IsVoiceCaddie983())
	{
		PlayCaddieVoice983(type, 0);
		return;
	}
	sBehavior* behavior = SetTalkToBubble(type, true, 0);
	if (!behavior)
		return;

	std::string msg;
	IFF_STRUCT::sCaddie* item = ItemManager()->FindCaddie(
		Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].caddieInfo.tid);
	if (item)
	{
		if (strstr(behavior->m_motion.c_str(), ":"))
			StripTalkMarkup983(behavior->m_motion, msg);
		else
		{
			msg = item->c.Name;
			msg += " : ";
			msg += behavior->m_motion;
		}
		CChatMsg::Instance()->AddChatMsg(msg, 0xff00ff00, true, false);
	}
}

void CGolfCaddie::Display()
{
	if (!ShouldDisplay())
		return;

	m_pCaddie->m_pBody->Display();
	if (m_pRing && m_pCaddie->m_bRing)
		m_pRing->Display();
}

void CGolfCaddie::DisplayShadow()
{
	if (!ShouldDisplay())
		return;

	CShadowManager::Instance()->RegisterCaster_New("caddie", 0x10, "sun",
		g_lightset);
	m_pCaddie->m_pBody->DisplayShadow("caddie");
}

void CGolfCaddie::ResetTalk()
{
	m_bubbleTalkType = 0;
	for (int i = 0; i < 36; i++)
		m_bTalked[i] = false;
	m_bTalkFlag = false;
	m_talkTime = g_CurrentTime;
}

void CGolfCaddie::HandleMsg(const MsgObject& msg)
{
	if (0) // HACK: keeps the inline naked TalkProcess emitted (nakedify)
		((CGolfCaddie*)0)->TalkProcess(*(float*)0);
	switch (msg.message)
	{
	case M983(0x13):
		m_bVisible = true;
		m_state = 0;
		m_idleTime = 20.0f;
		m_delay = 0;
		m_bDodging = false;
		ResetTalk();
		m_pCaddie = FindCaddie(
			Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].caddieInfo.tid,
			Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].caddieInfo.tidPart);
		if (m_pCaddie)
		{
			m_pCaddie->m_pInfo =
				&Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].caddieInfo;
			SearchGround();
			m_pCaddie->m_pBody->Reset();
			m_pCaddie->m_pBody->SetPos(m_pos);
			m_pCaddie->m_pBody->SetRotate(m_angle);
			m_pCaddie->m_pBody->SetMotion("\xb1\xe2\xba\xbb", false, 0.0f,
				false);
			m_pCaddie->m_pBody->Update();
			m_pCaddie->m_pBody->SetLight();
			m_pCaddie->m_pBody->LookUp(0, true);
			m_pFxInterpreter983->DetachFx(m_fxScript);
			m_fxScript = m_pFxInterpreter983->ParseBoundingBoxScript_fx(
				m_pCaddie->m_pBody->GetPet());
			GetPVS().SetCaddieBox(m_pCaddie->m_pBody->GetBoundBox());
			if (m_pRing && m_pCaddie->m_bRing)
			{
				m_pRing->SetCenter(m_pos + WVector(0.0f, 0.3f, 0.0f));
				m_pRing->SetActive(true);
			}
			if (IsCurMapSnow())
			{
				Attacher attacher;
				WPuppet* pet = m_pCaddie->m_pBody->GetPet();
				w_bound_box* box = pet->m_bbList.Find("Breathe");
				if (box)
				{
					attacher.mat[0] = &box->bone->m_matrix;
					attacher.pos[0] = &box->info->spherePivot;
					CFx::Instance()->Attach(attacher, "breath.seq", true, 0);
				}
			}
			CSceneManager::Instance()->SetVisible("\xc4\xb3\xb5\xf0", true);
			CSceneManager::Instance()->SetPuppet("\xc4\xb3\xb5\xf0",
				m_pCaddie->m_pBody->GetPet());
			m_bNoItem = 0;
		}
		else
		{
			m_pFxInterpreter983->DetachFxAll();
			CSceneManager::Instance()->SetPuppet("\xc4\xb3\xb5\xf0", 0);
		}
		break;

	case M983(0x4a):
	{
		unsigned long tid = msg.param1;
		unsigned long tidPart = msg.param3;
		sCaddie* c = 0;
		// the part item must belong to this caddie
		if (tid && tidPart && (tid & 0x3ffffff) != ((tidPart >> 21) & 0x1f) + ((tidPart >> 16) & 0x1f)) // 983 0073b98a: soma os 2 campos do tid da peca
			break;

		ResetTalk();
		if (tid)
			c = AddCaddie(tid, tidPart, true);
		m_pCaddie = c;
		if (c)
		{
			c->m_pInfo = &Doc()->m_userInfo[0].caddieInfo;
			m_pCaddie->m_pInfo->tid = tid;
			int level = msg.param2;
			m_pCaddie->m_pInfo->Level = level < 0 ? 0 : level > 3 ? 3 : level;
			SearchGround();
			m_pCaddie = c;
			m_pCaddie->m_pBody->Reset();
			m_pCaddie->m_pBody->SetPos(m_pos);
			m_pCaddie->m_pBody->SetRotate(m_angle);
			m_pCaddie->m_pBody->SetMotion("\xb1\xe2\xba\xbb", false, 0.0f,
				false);
			m_pCaddie->m_pBody->Update();
			m_pCaddie->m_pBody->SetLight();
			m_pCaddie->m_pBody->LookUp(0, true);
			GetPVS().SetCaddieBox(m_pCaddie->m_pBody->GetBoundBox());
			CSceneManager::Instance()->SetPuppet("\xc4\xb3\xb5\xf0",
				m_pCaddie->m_pBody->GetPet());
			if (m_pRing && m_pCaddie->m_bRing)
			{
				m_pRing->SetCenter(m_pos + WVector(0.0f, 0.3f, 0.0f));
				m_pRing->SetActive(true);
			}
		}
		else
		{
			Doc()->m_userInfo[0].caddieInfo.tid = 0;
			CSceneManager::Instance()->SetPuppet("\xc4\xb3\xb5\xf0", 0);
		}
		break;
	}

	case M983(0xf4):
		*(WVector*)msg.param1 = m_pos;
		break;

	case M983(0x118):
		if (m_pCaddie)
			m_pCaddie->m_pBody->GetPet()->SetRenderMode(msg.param1);
		break;

	case M983(0x121):
		m_bVisible = true;
		m_state = 0;
		m_idleTime = 20.0f;
		m_delay = 0;
		m_bDodging = false;
		ResetTalk();
		m_pCaddie = FindCaddie(Doc()->m_userInfo[MyIndex()].caddieInfo.tid,
			Doc()->m_userInfo[MyIndex()].caddieInfo.tidPart);
		if (m_pCaddie)
		{
			m_pCaddie->m_pInfo = &Doc()->m_userInfo[MyIndex()].caddieInfo;
			m_pCaddie->m_pBody->Reset();
			m_pCaddie->m_pBody->SetPos(*(WVector*)msg.param1);
			m_pCaddie->m_pBody->SetRotate(5.4977875f);
			m_pCaddie->m_pBody->SetMotion((const char*)msg.param2, false, 0.0f,
				false);
			m_pCaddie->m_pBody->Update();
			m_pCaddie->m_pBody->SetLight();
			m_pCaddie->m_pBody->LookUp(0, true);
			if (m_pRing && m_pCaddie->m_bRing)
			{
				m_pRing->SetCenter(m_pos + WVector(0.0f, 0.3f, 0.0f));
				m_pRing->SetActive(true);
			}
			CSceneManager::Instance()->SetVisible("\xc4\xb3\xb5\xf0", true);
			CSceneManager::Instance()->SetPuppet("\xc4\xb3\xb5\xf0",
				m_pCaddie->m_pBody->GetPet());
			m_bNoItem = 0;
		}
		else
		{
			CSceneManager::Instance()->SetPuppet("\xc4\xb3\xb5\xf0", 0);
		}
		break;

	case M983(0x163):
		m_bVisible = true;
		break;

	case M983(0x164):
		m_bVisible = false;
		// 983 0073b910 case 0x1cf: FUN_00889df0(+0x70) solta os efeitos do caddie
		m_pFxInterpreter983->DetachFx(m_fxScript);
		break;

	case M983(0x1a1):
		m_bTalkFlag = false;
		m_talkTime = g_CurrentTime;
		m_shotDir += *(float*)msg.param1;
		break;

	case M983(0x1d6):
		Dodge();
		break;

	case M983(0x1d7):
	{
		int bRun = 1;
		SendMsg(this, "NPC", M983(0x184), (int)&bRun, 0, 0, 0);
		if (m_pCaddie && !m_bDodging && bRun)
		{
			if (msg.param1)
			{
				m_delay = -1.0f;
				WPolySoupPoint* point =
					((WPolySoupFind*)GOLFDOC()->m_pPolySoup)
						->FindPoint((const char*)msg.param1);
				m_target =
					GetPVS().GetGroundPoint(point->pos, false, 0, false) -
					GolfBall().m_pos;
				m_target.y = 0;
				m_pCaddie->m_pBody->SetMoveScale(6.0f);
				m_target += GolfBall().m_pos;
				CCameraWork::Instance()->m_bCaddieRun = true;
				break;
			}
			// HACK: the literal is owned by the naked ReadAttribute (its first user in the original)
			if (IsGoodMotion(m_pCaddie,
					"\xb4\xde\xb8\xae\xb1\xe2")) // "correr" (EUC-KR), como no ReadAttribute
			{
				m_delay = m_pCaddie->m_startDelay;
				CGolfBall& ball = GolfBall();
				m_target = GolfBall().PreData(GolfBall().m_renderNum)->pos -
					(ball.m_renderNum < 0 ? (CGolfBall::sExpected*)0
										  : ball.m_expected)
						->pos;
				m_target.y = 0;
				float extra = (float)(GolfBall().m_renderNum -
								  GolfBall().m_bounceFrame - 150) *
					0.6f;
				float time =
					(m_target.Magnitude() + extra) / m_pCaddie->m_petSpeed;
				m_pCaddie->m_pBody->SetMoveScale(
					time / (GolfBall().m_totalTime - 1.0f));
				WVector back = m_target.Normalize() * 9.6f;
				m_target =
					GolfBall().PreData(GolfBall().m_renderNum)->pos - back;
				// 983 00744b80/0073b910 case 0x24f: FUN_00730ec0 (tid 0x1c000021..23) pula o olhar para a bola
				unsigned long runTid =
					Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].caddieInfo.tid;
				if (!(runTid > 0x1c000020 && runTid < 0x1c000024))
				{
					LookUp(&GolfBall().m_pos, false);
				}
				CCameraWork::Instance()->m_bCaddieRun = true;
				break;
			}
		}
		CCameraWork::Instance()->m_bCaddieRun = false;
		break;
	}

	case M983(0x1d8):
		LookUp((WVector*)msg.param1, msg.param2 != 0);
		break;

	case M983(0x1d9):
		DontLookUp();
		break;

	case M983(0x1da):
		g_audio->PlaySfx("caddie_XXX");
		if (Doc()->m_golfGame.gameType == 1)
			TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)->gauge = 0.0f;
		else
			PLAYER(GOLFDOC()->m_currentPlayer)->gauge = 0.0f;
		break;

	case M983(0x1db):
		ResetTalk();
		break;

	case M983(0x1df):
		m_clubDelta = msg.param1;
		break;

	case M983(0x1dc):
		SetBubble((unsigned char)msg.param1, true, 0);
		break;

	case M983(0x1de):
		SetCaddieTalk_ChatOnly((unsigned char)msg.param1);
		break;

	case M983(0x1dd):
		if (!talkTable[m_bubbleTalkType].m_bFlag)
			m_bTalked[m_bubbleTalkType] = false;
		m_bubbleTalkType = 0;
		m_pCaddieTalk->OnStop();
		break;

	case M983(0x1e2):
		m_angle = *(float*)msg.param1;
		if (m_pCaddie)
			m_pCaddie->m_pBody->SetRotate(m_angle);
		break;

	case M983(0x1e0):
		*(sCaddie**)msg.param1 = m_pCaddie;
		break;

	case M983(0x1e1):
		m_bNoItem = msg.param1 == 1;
		break;

	case M983(0x1e3):
	{
		unsigned long tid = msg.param1;
		unsigned long tidPart = msg.param2;
		if (!ItemManager()->FindCaddie(tid))
			break;
		IFF_STRUCT::sCadItem* part = ItemManager()->FindCadItem(tidPart);
		if (!part)
			break;
		for (std::list<sCaddie*>::iterator it = m_caddies.begin();
			it != m_caddies.end(); it++)
		{
			sCaddie* c = *it;
			if (!c || !c->m_pInfo)
				continue;
			if (c->m_pInfo->tid != tid)
				continue;
			if (c->m_pInfo->tidPart != tidPart)
				continue;
			if (strlen(part->FaceTex))
				c->m_pBody->SetPendingTexPart("face", part->FaceTex);
			if (strlen(part->BodyTex))
				c->m_pBody->SetPendingTexPart("body", part->BodyTex);
		}
		break;
	}

	// 983 0073b910 caso 0x359: FUN_0073b530 refaz o set de +0xac
	case 0x359:
		RebuildCaddieVoiceNames983();
		break;

	// 983 0073b910 caso 0xc7: FUN_0088c460 (FxInterpreter::SetMode) no interpretador de fx em +0x74
	case 0xc7:
		m_pFxInterpreter983->SetMode(msg.param1);
		break;

	case 0x2716:
		if (m_pCaddie && IsGoodMotion(m_pCaddie, (const char*)msg.param1))
			m_pCaddie->m_pBody->SetMotion((const char*)msg.param1, false, 0.0f,
				false);
		break;

	case 0x2717:
		if (m_pCaddie && IsGoodMotion(m_pCaddie, (const char*)msg.param1))
			m_pCaddie->m_pBody->SetNextMotion((const char*)msg.param1, false);
		break;

	case 0x2718:
		m_shotDir = *(float*)msg.param1;
		break;

	case 0x2719:
		m_pos = *(WVector*)msg.param1;
		if (m_pCaddie)
		{
			m_pCaddie->m_pBody->SetPos(m_pos);
			if (m_pRing && m_pCaddie->m_bRing)
			{
				m_pRing->SetCenter(m_pos + WVector(0.0f, 0.3f, 0.0f));
			}
		}
		break;
	}
}

void CGolfCaddie::SearchGround()
{
	for (int i = 0; i < 4; i++)
	{
		m_angle = GOLFDOC()->m_cameraData.angle +
			(i & 1 ? 1.0f : -1.0f) * 0.6981316953897476 + (i & 2 ? g_PI : 0.0f); // 983 d178f0 (double)
		m_pos = GetPVS().GetGroundPoint(
			GolfBall().m_pos + RotMat(m_angle, 1).za * 22.4f, true, 0, false);

		if (GetPVS().GetMeshIndex(m_pos, true) < 0)
			continue;
		if (!(GetPVS().GetMeshNormal(m_pos).y < cos(0.3490658402442932)))
		{
			if (!GetPVS().OverlappedWithObject(m_pos, 1.6f))
				break;
		}
	}
}

bool CGolfCaddie::ShouldDisplay()
{
	if (!m_bVisible || !m_pCaddie)
		return false;

	float dist = (g_camera.pivot - m_pos).Magnitude();
	if (dist > 640.0f)
		return false;
	return true;
}


void CGolfCaddie::Dodge()
{
	if (!m_pCaddie)
		return;

	// 983 007351e0: pos/vel = GolfBall m_expected[0] (null se m_renderNum < 0, como em Run)
	CGolfBall::sExpected* exp0 =
		GolfBall().m_renderNum < 0 ? (CGolfBall::sExpected*)0 : GolfBall().m_expected;
	if (!exp0)
		return;
	WVector delta = m_pos - exp0->pos;
	float angle = CalcDeltaAngle(delta, exp0->vel);

	switch (GetPVS().CollideBetweenBallandCaddie())
	{
	case 2:
	case 3:
		if (angle > 0.0f)
		{
			if (IsGoodMotion(m_pCaddie,
					"\xbf\xde\xc2\xca\xc0\xb8\xb7\xce\xc7\xc7\xc7\xcf\xb1\xe2"))
			{
				m_bDodging = true;
				m_pCaddie->m_pBody->SetMotion(
					"\xbf\xde\xc2\xca\xc0\xb8\xb7\xce\xc7\xc7\xc7\xcf\xb1\xe2",
					false, 0.0f, false);
			}
		}
		else
		{
			if (IsGoodMotion(m_pCaddie,
					"\xbf\xc0\xb8\xa5\xc2\xca\xc0\xb8\xb7\xce\xc7\xc7\xc7\xcf\xb1\xe2"))
			{
				m_bDodging = true;
				m_pCaddie->m_pBody->SetMotion(
					"\xbf\xc0\xb8\xa5\xc2\xca\xc0\xb8\xb7\xce\xc7\xc7\xc7\xcf\xb1\xe2",
					false, 0.0f, false);
			}
		}
		break;
	case 1:
		if (IsGoodMotion(m_pCaddie,
				"\xb3\xf4\xc0\xba\xb0\xf8\xc7\xc7\xc7\xcf\xb1\xe2"))
		{
			m_bDodging = true;
			m_pCaddie->m_pBody->SetMotion(
				"\xb3\xf4\xc0\xba\xb0\xf8\xc7\xc7\xc7\xcf\xb1\xe2", false, 0.0f,
				false);
		}
		break;
	case 0:
		m_bDodging = false;
		{
			// 983 FUN_00730ec0: tid do caddie 0x1c000021..23 nao olha para a bola
			unsigned long dodgeTid =
				Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].caddieInfo.tid;
			if (!(dodgeTid > 0x1c000020 && dodgeTid < 0x1c000024))
				LookUp(&GolfBall().m_pos, true);
		}
		break;
	}
}

void CGolfCaddie::Run()
{
	if (!m_pCaddie)
		return;

	m_pos = m_pCaddie->m_pBody->GetPos();
	WVector delta = m_target - m_pos;
	WVector vel = m_pCaddie->m_pBody->GetVel();
	float dot = delta.x * vel.x + delta.z * vel.z;
	float stopDist = Wabs(GolfBall().m_initCurve) < g_EPSILON
		? m_pCaddie->m_stopDist
		: m_pCaddie->m_stopDist * 2;

	if ((float)sqrt((double)(delta.x * delta.x + delta.z * delta.z)) <
			stopDist ||
		(GolfBall().m_playFrame > 0 &&
			(GolfBall().m_renderNum - GolfBall().m_playFrame < 50 ||
				dot < 0.0f)))
	{
		m_pCaddie->m_pBody->SetMoveScale(1.0f);
		if (GOLFDOC()->m_tutorialMode == 15)
			m_pCaddie->m_pBody->SetMotion("\xb1\xde\xc1\xa4\xb0\xc5", true,
				0.0f, false);
		else
			m_pCaddie->m_pBody->SetMotion(
				"\xc6\xa9\xc5\xcd\xb8\xae\xbe\xf3\xb1\xde\xc1\xa4\xb0\xc5",
				true, 0.0f, false);
		m_state = 0;
		CFx::Instance()->RemoveRunDust(m_runDust);

		if (GolfBall().m_renderNum > 0)
		{
			CGolfBall& ball = GolfBall();
			unsigned char ground = GetPVS().GetGroundType(
				ball.PreData(ball.m_renderNum)->pos, true, 0);
			if (ball.m_bOB || ground >= 3)
				m_pCaddie->m_pBody->SetNextMotion("\xba\xa1\xc4\xbf", false);

			float speed = (float)sqrt((double)(vel.x * vel.x + vel.z * vel.z));
			if (Wabs(speed) < g_EPSILON)
			{
				CGolfBall& b = GolfBall();
				WVector pos;
				pos = b.PreData(b.m_renderNum)->pos -
					(b.m_renderNum < 0 ? (CGolfBall::sExpected*)0
									   : b.m_expected)
						->pos;
				pos.Normalize();
				pos = b.PreData(b.m_renderNum)->pos - pos * 5.0f;
				pos = GetPVS().GetGroundPoint(pos, false, 0, false);
				m_pCaddie->m_pBody->SetPos(pos);
				m_pos = pos;
			}
		}
	}
	m_pCaddie->m_pBody->SetLight();
}

void CGolfCaddie::LookUp(WVector* v, bool b)
{
	if (m_pCaddie != 0)
		m_pCaddie->m_pBody->LookUp(v, b);
}

void CGolfCaddie::DontLookUp()
{
	if (m_pCaddie != 0)
		m_pCaddie->m_pBody->LookUp(0, true);
}

void CGolfCaddie::UseSpecialAbility()
{
	if (0) // HACK: emission order (after Run, as in the original)
		Run();
}

// JP 983 (s158): nomes que este arquivo cita por uma visao (ou com a assinatura um pouco diferente: ponteiro x referencia,
// const, int x unsigned long, void x bool; ABI igual no x86) e que nao existiam com esse nome -> viravam stub no exe.
// Cada um aponta para a funcao/dado real (lista e motivo em out/F3.5/s158/apelidos.tsv).
#pragma comment(linker, "/alternatename:?IsInTurbulence@CWindTurbulence@@QAE_NABVWVector@@@Z=?IsInTurbulence@CWind@@QAE_NABVWVector@@@Z") // cast de Wind()
