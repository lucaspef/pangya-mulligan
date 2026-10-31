#pragma once

// Layouts from the PDB/Ghidra; fields not yet seen in any
// function are kept as byte padding named by offset.

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "actor.h"
#include "../shared/classdefine.h"
#include "../shared/globalgamedefine.h"
#include "../shared/itemmanager.h"
#include "scenemanager.h"
#include "fxinterpreter.h"

struct sTalk
{
	const char* m_name;
	bool m_bFlag;
};

// 983 0073a7a0 (o mesmo do IDummyPet::Load): .pet do caddie pela textura do corpo do skin
std::string CaddiePetFile983(const char* bodyTex, unsigned long skinTypeId, const char* caddieData);

class CCaddieTalk;
class CPetBody;
class CRing;

class CGolfCaddie : public IActor, public CRenderFuncPtr
{
public:
	virtual ~CGolfCaddie() { }

	DECLARE_OBJECT(CGolfCaddie)

	virtual void OnPreLoadInit();
	virtual void OnInit();
	virtual void OnLoad();
	virtual void OnReload();
	virtual void OnDestroy();
	virtual void OnProcess(float dt);
	virtual void Display();
	virtual void DisplayShadow();
	virtual void HandleMsg(const MsgObject& msg);

public:
	struct sBehavior
	{
		unsigned char m_level;
		std::string m_motion;
	};

	struct sCaddie
	{
		sCaddie()
			: m_pItem(0), m_pInfo(0)
		{
		}

		CPetBody* m_pBody;
		unsigned char m_pad04[0xc];
		IFF_STRUCT::sCaddie* m_pItem;
		sCaddieInfo* m_pInfo;
		std::map<std::string, sBehavior> m_talks;
		std::map<std::string, sBehavior> m_motions;
		float m_stopDist;
		float m_startDelay;
		float m_petSpeed;
		bool m_bRing;
		bool m_bCaddieView;
	};

	struct sFindCaddie
	{
		sFindCaddie(unsigned long id, unsigned long cadItem)
			: m_id(id), m_cadItem(cadItem)
		{
		}
		bool operator()(sCaddie* c)
		{
			if (c->m_pInfo == 0)
				return c->m_pItem->c.TypeId == m_id;
			return c->m_pInfo->tidPart == m_cadItem &&
				c->m_pItem->c.TypeId == m_id;
		}

		unsigned long m_id;
		unsigned long m_cadItem;
	};

protected:
	CGolfCaddie()
		: m_bVisible(true), m_pRing(0), m_pCaddieTalk(0) // 983: gravados antes de construir o map de 0xac (0073a63e)
	{
		// the unit owns storage for these four (passed by reference to inlines)
		(void)&g_PI;
		(void)&g_DEGTORAD;
		(void)&g_HUGE;
		(void)&g_EPSILON;
		m_pFxInterpreter983 = new FxInterpreter(NULL, 0);
	}

	bool IsGoodMotion(sCaddie* c, const char* motion);
	bool ReadAttribute(sCaddie* c);
	bool ShouldDisplay();
	char* GetBehaviorToken(char* dst, char* src);
	char* GetBehaviorEndToken(char* dst, char* src);
	sCaddie* AddCaddie(unsigned long id, unsigned long cadItem, bool unique);
	sCaddie* FindCaddie(unsigned long id, unsigned long cadItem);
	void Dodge();
	void DontLookUp();
	void LoadMotionLevel(sCaddie* c);
	void LoadTalkText(sCaddie* c);
	void LookUp(WVector* v, bool b);
	void ResetTalk();
	void Run();
	void SearchGround();
	void SetBubble(unsigned char a, bool b, unsigned char c);
	void SetBubble(unsigned char a, const char* msg);
	sBehavior* SetTalkToBubble(unsigned char type, bool bChatOnly,
		unsigned char param);
	void SetCaddieTalk_ChatOnly(unsigned char a);
	void RebuildCaddieVoiceNames983(); // 0073b530
	bool IsVoiceCaddie983() const;     // 00730e90
	void PlayCaddieVoice983(unsigned char type, unsigned char variant); // 00735030
	void TalkProcess(float dt);
	void UseSpecialAbility();

	sCaddie* m_pCaddie;            // +0x1c
	std::list<sCaddie*> m_caddies; // +0x20
	unsigned char m_pad2c[4];
	int m_state;      // +0x30
	int m_runDust;    // +0x34
	WVector m_pos;    // +0x38
	float m_delay;    // +0x44
	float m_angle;    // +0x48
	WVector m_target; // +0x4c
	bool m_bDodging;  // +0x58
	bool m_bVisible;  // +0x59
	unsigned char m_pad5a[2];
	float m_idleTime; // +0x5c
	int m_clubDelta;  // +0x60
	bool m_bNoItem;   // +0x64
	unsigned char m_pad65[3];
	float m_shotDir;                // +0x68
	float m_turbulenceTime;         // +0x6c
	int m_fxScript;                 // +0x70
	FxInterpreter* m_pFxInterpreter983; // 983: 0x74 = new FxInterpreter(NULL, 0) no fim do construtor (0073a6ab..0073a6d4)
	CRing* m_pRing;                 // +0x74; 983: 0x78
	unsigned char m_bubbleTalkType; // +0x78
	bool m_bTalked[36];             // +0x79
	bool m_bTalkFlag;               // +0x9d
	unsigned long m_talkTime;       // +0xa0
	CCaddieTalk* m_pCaddieTalk;     // +0xa4; 983: 0xa8
	std::map<std::string, const char*> m_caddieVoiceNames983; // 983: +0xac, map<nome, arquivo> (no de 0x30, valor em +0x28); refeito pelo caso 0x359 (0073b530) com as falas da CaddieVoiceTable (nome=entrada+4, arquivo=entrada+0x45); lido por 00735030
	std::string m_curVoiceSfx983;  // 983: 0xbc..0xd8 nome do sfx da voz do caddie tocando (PlayCaddieVoice983 00735030); sizeof 0xd8
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbReadattributeFunclets(); // HACK: EH funclets of the naked ReadAttribute (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnloadFunclets(); // HACK: EH funclets of the naked OnLoad (nakedify)
#endif
};

struct ICaddieTalk
{
	virtual void Load() = 0;
	virtual bool FindMsg() = 0;
	virtual const char* GetTalkMsg() const { return 0; }
	virtual int GetPriority() { return 0; }
	virtual ~ICaddieTalk() { }
};

struct XmlCaddieTalk
{
	struct parameters
	{
		void add(const std::string& s) { m_list.push_back(s); }

		std::vector<std::string> m_list;
	};

	struct attributes
	{
		const char* get(const std::string& key)
		{
			std::map<std::string, std::string>::iterator it = m_map.find(key);
			if (it != m_map.end())
				return it->second.c_str();
			return 0;
		}

		std::map<std::string, std::string> m_map;
	};

	virtual bool LoadSituation(const char* name, const parameters& params,
		std::vector<attributes>& out);
};

struct cCaddieTalkTeeShot : public XmlCaddieTalk, public ICaddieTalk
{
	struct sTalkData
	{
		bool operator==(const sTalkData& other) const
		{
			if (m_a == -1)
				return m_b == other.m_b && m_c == other.m_c;
			return m_a == other.m_a && m_b == other.m_b && m_c == other.m_c;
		}

		int m_a;
		int m_b;
		int m_c;
		std::string m_msg;
	};

	cCaddieTalkTeeShot() { }
	virtual ~cCaddieTalkTeeShot() { }
	virtual void Load();
	virtual bool FindMsg();
	virtual const char* GetTalkMsg() const { return m_msg.c_str(); }

	std::vector<sTalkData> m_talk;
	std::string m_msg;
};

class CCaddieTalk
{
public:
	~CCaddieTalk();

	void Init();
	void Release();
	void OnStop() { m_busy = 0; }
	const ICaddieTalk* FindTalk();

protected:
	std::list<ICaddieTalk*> m_talks;
	int m_busy;
};
