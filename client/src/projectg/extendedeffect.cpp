#include "minatl.h"
#include "fx.h"
#include "shareddoc.h"
#include "golfdoc.h"

// HACK: CFx::FindSequence and CFxSpray::SetActive are not declared in the shared
// headers. Adding them there perturbs projectg.cpp (SaveScreenShot), so they are
// declared in local helper classes instead.
class CFxSprayEx : public CFxSpray
{
public:
	void SetActive(bool bActive) { m_bActive = bActive; }
};

class CFxEx : public CFx
{
public:
	bool FindSequence(CFxSequence* pSequence);
};

#pragma comment(linker, \
	"/alternatename:?FindSequence@CFxEx@@QAE_NPAVCFxSequence@@@Z=?FindSequence@CFx@@QAE_NPAVCFxSequence@@@Z")

#include "partsinfo.h"
#include "extendedeffect.h"
#include "mathconsts.h"
#include "wbone.h"

CExtendedEffect::CExtendedEffect()
{
	// 983 (006e0270): abobora do taco e luvas neon, por tid da parte
	static const unsigned long s_pumpkin[] = { 0x8000852, 0x804086f, 0x809a820,
		0x80d0837, 0x811a81c, 0x815a84c, 0x818e047, 0x81d881c, 0x8212040,
		0x8252003 };
	static const unsigned long s_glove[] = { 0x800881e, 0x8046831, 0x8088821,
		0x80c8035, 0x8106021, 0x8148036, 0x8186030, 0x81c8025, 0x820803a,
		0x8248035, 0x8288013 };

	for (int i = 0; i < 10; i++)
		AddEffect(s_pumpkin[i], "Bip Dummy", "ball_pumpkin.seq", true, "");
	for (int i = 0; i < 11; i++)
		AddEffect(s_glove[i], "Bip01 L Hand", "s_neon_windglove.seq", false,
			"L_neon_windglove");
	for (int i = 0; i < 11; i++)
		AddEffect(s_glove[i], "Bip01 R Hand", "s_neon_windglove.seq", false,
			"R_neon_windglove");
}

CExtendedEffect::~CExtendedEffect()
{
	for (std::list<sAvatarInfo>::iterator it = m_avatars.begin();
		it != m_avatars.end(); it++)
	{
		for (std::list<sEffectData>::iterator itEffect = (*it).effects.begin();
			itEffect != (*it).effects.end(); itEffect++)
		{
			if ((*itEffect).pSequence)
				CFx::Instance()->CloseSequence((*itEffect).pSequence, false);
		}

		(*it).effects.clear();
	}

	m_avatars.clear();
	m_effects.clear();
}

int CExtendedEffect::AddEffect(unsigned long id, const char* bone,
	const char* file, bool bAutoOpen, const char* altBone)
{
	if (id == 0 || bone == NULL || file == NULL)
		return 0;

	std::map<unsigned long, sEffectData>::iterator it = m_effects.find(id);
	if (it != m_effects.end())
		return 0;

	sEffectData data;
	data.id = id;
	data.bone = bone;
	data.altBone = altBone;
	data.file = file;
	data.bAutoOpen = bAutoOpen;
	data.bReopen = false;
	data.pSequence = NULL;
	m_effects[id] = data;
	return 1;
}

CExtendedEffect::sEffectData* CExtendedEffect::FindEffect(unsigned long id)
{
	sEffectData* pEffect = NULL;
	std::map<unsigned long, sEffectData>::iterator it = m_effects.find(id);
	if (it != m_effects.end())
		pEffect = &it->second;

	return pEffect;
}

int CExtendedEffect::RegistSequence(unsigned long id,
	std::list<sEffectData>* pList, bool bReopen)
{
	if (id == 0 || pList == NULL)
		return 0;

	sEffectData* pEffect = FindEffect(id);
	if (pEffect == NULL || bReopen == pEffect->bAutoOpen)
		return 0;

	sEffectData data;
	data.id = pEffect->id;
	data.bone = pEffect->bone;
	data.altBone = pEffect->altBone;
	data.file = pEffect->file;
	data.bAutoOpen = pEffect->bAutoOpen;
	data.bReopen = bReopen;
	data.pSequence = NULL;
	data.pSequence = CFx::Instance()->OpenSequence(data.file.c_str(), true);
	if (data.pSequence == NULL)
		return 0;

	pList->push_back(data);
	return 1;
}

int CExtendedEffect::RegistExtenedEffect(CPetFrame* pFrame, bool bReopen)
{
	if (pFrame == NULL)
		return 0;

	for (std::list<sAvatarInfo>::iterator it = m_avatars.begin();
		it != m_avatars.end(); it++)
	{
		if (pFrame == (*it).pFrame)
			return 0;
	}

	sAvatarInfo info;
	info.pFrame = pFrame;
	CPartList& partList = pFrame->GetPartList();
	for (unsigned char i = 0; i < 24; i++)
	{
		const sPart* pPart = partList.GetPart(i);
		if (pPart)
		{
			unsigned long tid = pPart->tid;
			RegistSequence(tid, &info.effects, bReopen);
		}
	}

	if (!info.effects.empty())
	{
		m_avatars.push_back(info);
		info.effects.clear();
	}

	return 1;
}

// 983 006de790: marca bReopen (+0x59) no efeito id (0 = todos) de cada avatar, so se player e o jogador da vez
// (o 983 compara com o jogador da vez, nao com o avatar); o primeiro argumento (1, do HandleMsg) nao e lido
void CExtendedEffect::ReopenEffect983(int, unsigned long player, unsigned long id)
{
	for (std::list<sAvatarInfo>::iterator it = m_avatars.begin();
		it != m_avatars.end(); it++)
	{
		if (player == GOLFDOC()->m_currentPlayer)
		{
			std::list<sEffectData>::iterator itEffect = (*it).effects.begin();
			if (id == 0)
			{
				for (; itEffect != (*it).effects.end(); itEffect++)
					(*itEffect).bReopen = true;
			}
			else
			{
				for (; itEffect != (*it).effects.end(); itEffect++)
				{
					if ((*itEffect).id == id)
					{
						(*itEffect).bReopen = true;
						break;
					}
				}
			}
		}
	}
}

void CExtendedEffect::Update(CPetFrame* pFrame)
{
	if (0)
		((CFxSprayEx*)NULL)->SetActive(false);

	for (std::list<sAvatarInfo>::iterator it = m_avatars.begin();
		it != m_avatars.end(); it++)
	{
		bool bActive = true;
		if (pFrame && pFrame != (*it).pFrame)
			bActive = false;

		for (std::list<sEffectData>::iterator itEffect = (*it).effects.begin();
			itEffect != (*it).effects.end(); itEffect++)
		{
			sEffectData& effect = *itEffect;
			if (!((CFxEx*)CFx::Instance())->FindSequence(effect.pSequence))
			{
				if (effect.bAutoOpen == true || effect.bReopen == true)
				{
					effect.pSequence = CFx::Instance()->OpenSequence(
						effect.file.c_str(), true);
					effect.bReopen = false;
				}
				else
					effect.pSequence = NULL;
			}

			if (effect.pSequence)
			{
				effect.pSequence->SetActive(bActive, false);
				WPuppet* pPet = (*it).pFrame->GetBonePet();
				WBone* pBone;
				if (effect.altBone.empty())
					pBone = pPet->FindBone(effect.bone.c_str());
				else
					pBone = pPet->FindBoundBox(effect.altBone.c_str())->bone;
				effect.pSequence->Pos() = pBone->m_matrix.pivot;
			}
		}
	}
}
