#pragma once

#include <list>
#include <map>
#include <string>

class CFxSequence;
class CPetFrame;

class CExtendedEffect
{
public:
	struct sEffectData
	{
		unsigned long id;
		std::string bone;
		std::string altBone; // 983: se nao vazio, osso achado por w_bound_box (WPuppet+0x150)
		std::string file;
		bool bAutoOpen;
		bool bReopen; // 983 +0x59: reabrir a sequencia no Update
		CFxSequence* pSequence;
	};

	struct sAvatarInfo
	{
		CPetFrame* pFrame;
		std::list<sEffectData> effects;
	};

	CExtendedEffect();
	virtual ~CExtendedEffect();

	int RegistExtenedEffect(CPetFrame* pFrame, bool bReopen = false);
	void Update(CPetFrame* pFrame);
	void ReopenEffect983(int unused, unsigned long player, unsigned long id); // 006de790 (CGolfPlayer 0x31b)

private:
	int AddEffect(unsigned long id, const char* bone, const char* file,
		bool bAutoOpen, const char* altBone);
	sEffectData* FindEffect(unsigned long id);
	int RegistSequence(unsigned long id, std::list<sEffectData>* pList,
		bool bReopen);

	std::map<unsigned long, sEffectData> m_effects;
	std::list<sAvatarInfo> m_avatars;
};
