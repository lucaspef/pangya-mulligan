#pragma once
#include "wboneset.h"
#include "partsinfo.h" // CPetFrame (HierarchyChangeObserver 00cfeb64, AddObserver983 008b14b0, ossos +0x48 e observadores +0x58)

// JP 983: WBoneSet do 983 tem 0x30 (lista 0x20 + vector 0x10, sem o m_index do 645; 0047f0a9/0047f0be põem dois a 0x30 um do
// outro). O da frente A (include/engine/wboneset.h) ainda guarda o m_index e dá 0x34 no build do exe. PROVISÓRIO: pedido em
// out/PEDIDOS.md (sessão B28); quando o m_index sair de lá, trocar por WBoneSet (o Resize983 vira método dela ou função livre).
class WBoneSetJp
{
public:
	WBoneSetJp(); // 00b4f520 (WBoneSet::WBoneSet)
	void Clear() { m_keyList.Reset(); } // 0047e830
	int GetCount983() const { return (int)m_data.size(); } // 0047e810
	void Resize983(int count) // 0047f110: esvazia a lista e redimensiona os dados
	{
		Clear();
		m_data.resize(count);
	}

	WList<WBoneKey*> m_keyList;
	std::vector<WBoneKey> m_data; // +0x20
};

// JP 983: base da ntAvatar (vtable 00cfeb6c, sizeof 0x68; construída em 007a732d pelo construtor da ntAvatar, que põe os
// campos dela a partir de +0x68). Guarda dois conjuntos de ossos do tamanho da hierarquia do CPetFrame e é avisada quando a
// hierarquia muda. Nomes dos campos e dos métodos são meus.
class AvatarBase : public CPetFrame::HierarchyChangeObserver
{
public:
	AvatarBase(); // 0047f080
	virtual void OnHierarchyChange(CPetFrame* frame); // 0047f130
	virtual ~AvatarBase(); // 0047ef00

	// 0047e4d0 (chamada em 007a47a3 e 007a7a02): liga-se ao CPetFrame atual e acerta os conjuntos
	void Attach983()
	{
		if (m_pPetFrame)
			m_pPetFrame->AddObserver983(this);
		OnHierarchyChange(m_pPetFrame);
	}
	// 0047e4f0 (chamada em 007a947f, ntAvatar::RefreshUccClothes)
	void SetPart983(const void* part)
	{
		if (m_pPetFrame)
			m_pPetFrame->SetPart983((const sUccItem*)part);
	}

protected:
	CPetFrame* m_pPetFrame; // +0x04
	WBoneSetJp m_boneSet; // +0x08
	WBoneSetJp m_blendSet; // +0x38
};
