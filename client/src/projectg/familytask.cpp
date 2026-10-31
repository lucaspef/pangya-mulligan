#include "minatl.h"
#include "familytask.h"
#include "actor.h"
#include "fresh.h"
#include "../shared/sharedtables.h"
#include "jp/levelnametable.h"

extern Fresh* g_pFresh;
extern int g_iRecvPacketType;

static CLevelNameTable983 s_levelNames; // 983 00f038ec: objeto estatico da unidade

IMPLEMENT_OBJECT(CFamilyTask, CTask)

CFamilyTask::CFamilyTask()
{
	IActor::m_defaultSkipLayer = 0;
	IActor::m_pLayer = NULL;
}

CFamilyTask::~CFamilyTask()
{
}

void CFamilyTask::Init(const char* wallPaper)
{
	CTask::Init("family_background.jpg");

	SetWhisper(false);
}

void CFamilyTask::Register()
{
	SetMainActor(AddActor("CFamilyMain", "Family", 0, ""));
}

void CFamilyTask::Load()
{
	SetProgressBar(false);
	SetTip(false);

	CTask::Load();
}

void CFamilyTask::Process(float delta)
{
	CTask::Process(delta);
	g_pFresh->OnProcess(delta);
}

void CFamilyTask::Display()
{
	CTask::Display();
	g_pFresh->OnDisplay(true);
}

int CFamilyTask::OnPacket(WReceivedPacket& packet)
{
	unsigned short type = packet.Decode2();

	g_iRecvPacketType = type;

	switch (type)
	{
	case 0xaf:
	{
		unsigned char level = packet.Decode1();
		unsigned long exp = packet.Decode4();

		char buffer[256];

		Doc()->m_myInfo.stat.dwExp += exp;

		if (level > Doc()->m_myInfo.stat.Level)
		{
			Doc()->m_myInfo.stat.dwExp -=
				s_levelNames.Get(Doc()->m_myInfo.stat.Level).value;
			sprintf(buffer,
				K2L_Compatibility("%d\xc0\xc7 \xb0\xe6\xc7\xe8\xc4\xa1\xb8\xa6 \xbe\xf2\xbe\xee\xbc\xad, %s\xb7\xce \xb7\xb9\xba\xa7\xbe\xf7 \xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."),
				exp, K2L_Compatibility(s_levelNames.Get(level).name));
		}
		else
		{
			sprintf(buffer,
				K2L_Compatibility("%d\xc0\xc7 \xb0\xe6\xc7\xe8\xc4\xa1\xb8\xa6 \xbe\xf2\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
				exp);
		}

		Doc()->m_myInfo.stat.Level = level;

		GetActor("Family") << MsgObject(NULL, M983(0x23), (int)buffer, 0, 0, 0, 0);
		break;
	}

	default:
		return OnPacketCommon(packet);
	}

	return 1;
}
