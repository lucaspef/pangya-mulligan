// JP 983: AvatarBase (construtor 0047f080, destrutor 0047ef00, OnHierarchyChange 0047f130; cópias fora de linha 0047e4d0,
// 0047e4f0 e WBoneSet 0047f110). Base da ntAvatar; a unidade do 983 começa nas cópias de 0047e4b0 e acaba em 0047f2a0.
// Fonte reconstruída do 983 (classe nova, sem par no 645). Sessão B28.
#include "minatl.h"
#include "projectg.h"
#include "avatarbase.h"
#include "partsinfo.h"

AvatarBase::AvatarBase()
	: m_pPetFrame(NULL)
{
}

AvatarBase::~AvatarBase()
{
}

void AvatarBase::OnHierarchyChange(CPetFrame* frame)
{
	m_boneSet.Clear();
	m_blendSet.Clear();
	if (frame)
	{
		// EndBuild983 avisa os observadores; GetBoneCount983 = tamanho dos ossos do CPetFrame (+0x48 no 983/VC10, A823)
		if (m_boneSet.GetCount983() != frame->GetBoneCount983())
			m_boneSet.Resize983(frame->GetBoneCount983());
		if (m_blendSet.GetCount983() != frame->GetBoneCount983())
			m_blendSet.Resize983(frame->GetBoneCount983());
	}
	else
	{
		m_boneSet.Resize983(0);
		m_blendSet.Resize983(0);
	}
}

// cópias fora de linha que o 983 guarda nesta unidade (0047e4d0, 0047e4f0, 0047f110)
void (AvatarBase::*const g_avatarBaseAttach983)() = &AvatarBase::Attach983;
void (AvatarBase::*const g_avatarBaseSetPart983)(const void*) = &AvatarBase::SetPart983;
void (WBoneSetJp::*const g_boneSetResize983)(int) = &WBoneSetJp::Resize983;
