#include "minatl.h"
#include "projectg.h"
#include "golfdoc.h"
#include <time.h>
#include "contentsdoc.h"
#include "ambientsound.h"

extern WMatrix g_camera;

// HACK: WPolySoup::m_soundBox (std::list<sSoundBox> at offset 0x494c) is private,
// so it is accessed by offset.
#define SOUNDBOX_LIST(pPolySoup) \
	(*(std::list<sSoundBox>*)((char*)(pPolySoup) + 0x4970)) // 983: +0x4970 (645: +0x494c; 0070ea87, 0070eb40)
#if defined(_MSC_VER) && _MSC_VER > 1600
// A728: +0x4970 e do layout do VC10 (o WPolySoup encolhe no cl novo, vs2022-2); la, o membro (friend)
#undef SOUNDBOX_LIST
#define SOUNDBOX_LIST(pPolySoup) ((pPolySoup)->m_soundBox)
#endif

// 983: contents container 0x9f = map id -> list of random ambient sounds
// (CRandomCourseSound, icontentsdoc.cpp: std::map<uchar, vector<const char*>> at +4; 0090b7a0 = find != end, 0090b7d0 = value).
struct sRandomSoundContainer983 : IContentsDataContainer
{
	std::map<unsigned char, std::vector<const char*> > m_map;
	__declspec(noinline) bool Has(unsigned char key) // 983 0090b7a0
	{
		return m_map.find(key) != m_map.end();
	}
	__declspec(noinline) std::vector<const char*>& Get(unsigned char key) // 983 0090b7d0
	{
		return m_map.find(key)->second;
	}
	virtual bool Initialize() { return true; }
	virtual bool Release() { return true; }
};

static float g_randomSoundTimer = 0.0f; // 983 00e79ef8

IMPLEMENT_ACTOR(CAmbientSound, IActor)

CAmbientSound::CAmbientSound()
{
	AddSkipList(0x85040);
	m_bRandomSound = false;
}

void CAmbientSound::OnLoad()
{
	sRandomSoundContainer983* container = NULL;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0x9f, container);
	unsigned char map = GetCurMap();
	if (container->Has(map))
	{
		std::vector<const char*>& src = container->Get(map);
		m_randomSounds.assign(src.begin(), src.end());
		srand((unsigned int)time(NULL));
	}
}

void CAmbientSound::RandomizeNextRandomSound(int base, int range)
{
	m_fNextRandomSound = (float)(rand() % range + base);
}

void CAmbientSound::UpdateRandomSound(float delta)
{
	if (!m_bRandomSound)
		return;

	g_randomSoundTimer += delta;
	if (m_fNextRandomSound <= g_randomSoundTimer)
	{
		int count = (int)m_randomSounds.size();
		g_audio->PlaySfx(m_randomSounds[rand() % count].c_str(), NULL, 0, NULL,
			NULL);
		RandomizeNextRandomSound(10, 20);
		g_randomSoundTimer = 0.0f;
	}
}

void CAmbientSound::OnInit()
{
	std::list<sSoundBox>& boxes = SOUNDBOX_LIST(GOLFDOC()->m_pPolySoup);
	for (std::list<sSoundBox>::iterator it = boxes.begin(); it != boxes.end();
		++it)
	{
		if ((*it).interval < 0.0f && !g_audio->IsPlaying((*it).name))
			g_audio->PlaySfx((*it).name, NULL, 1, (*it).name);
	}

	if (m_randomSounds.empty())
		m_bRandomSound = false;
	else
	{
		RandomizeNextRandomSound(10, 20);
		m_bRandomSound = true;
	}
}

float CAmbientSound::GetVolume(const Wobb& obb, float range)
{
	if (obb.IsInclude(g_camera.pivot) || range < 0.0f)
		return 1.0f;

	float sum = 0.0f;
	WVector dir = g_camera.pivot - obb.center;
	for (int i = 0; i < 3; i++)
	{
		float half = obb.extend[i].Magnitude() * 0.5f;
		WVector h = dir * 0.5f;
		float dist = (h.x * obb.extend[i].x + h.y * obb.extend[i].y +
						 h.z * obb.extend[i].z) /
			half;

		if (dist < half * -1.0f)
		{
			dist += half;
			sum += dist * dist;
		}
		else if (dist > half)
		{
			dist -= half;
			sum += dist * dist;
		}
	}

	float volume = 1.0f - sqrtf(sum) / (range * 3.2f);
	return Max(0.0f, volume);
}

void CAmbientSound::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case M983(0x1f4):
	{
		std::list<sSoundBox>& boxes = SOUNDBOX_LIST(GOLFDOC()->m_pPolySoup);
		for (std::list<sSoundBox>::iterator it = boxes.begin();
			it != boxes.end(); ++it)
			g_audio->StopSfx((*it).name);
	}
	break;
	}
}

void CAmbientSound::OnProcess(float delta)
{
	std::list<sSoundBox>& boxes = SOUNDBOX_LIST(GOLFDOC()->m_pPolySoup);

	std::map<std::string, float> volumes;
	volumes.clear();

	for (std::list<sSoundBox>::iterator it = boxes.begin(); it != boxes.end();
		++it)
	{
		float volume;
		if ((*it).interval > 0.0f)
		{
			(*it).time += delta;
			if ((*it).time > (*it).interval)
			{
				(*it).time = 0.0f;
				g_audio->PlaySfx((*it).name);
			}

			if (g_audio->IsPlaying((*it).name))
				volume = GetVolume((*it).obb, (*it).distance);
			else
				volume = 0.0f;
		}
		else
		{
			volume = GetVolume((*it).obb, (*it).distance);
		}

		std::map<std::string, float>::iterator found;
		found = volumes.find((*it).name);
		if (found == volumes.end())
			volumes[(*it).name] = volume;
		else
			volumes[(*it).name] = Max(volumes[(*it).name], volume);
	}

	UpdateRandomSound(delta);

	for (std::map<std::string, float>::iterator i = volumes.begin();
		i != volumes.end(); ++i)
	{
		if (g_audio->IsPlaying((*i).first.c_str()))
			g_audio->SetSFXVolumeWhilePlaying((*i).first.c_str(), (*i).second);
	}
}
