#pragma once

#include "contentsdoc.h"
#if defined(CONTENTSDOC_DEFER_GIMMICK_CONTAINER) || defined(REBANG_INTRUSION)
class WReceivedPacket;
#endif

class CGimmickContainer : public IContentsDataContainer
{
public:
	virtual bool Initialize();
	virtual bool Release();

	bool LoadFromScript(const char* filename);
#if defined(CONTENTSDOC_DEFER_GIMMICK_CONTAINER) || defined(REBANG_INTRUSION)
	void LoadFromPacket(WReceivedPacket& packet);
#endif
	std::vector<GimmickDispositionInformation>& GetGimmickPosition(
		unsigned char hole);
	void ResetGimmikFlag(unsigned char hole,
		GimmickDispositionInformation::eFlag flag);
	GimmickDispositionInformation* FindDisposeInfo(unsigned char hole,
		GIMMICK_ITEM_TYPE type, GimmickDispositionInformation::eFlag flag);
	int GetNumDisposeInfo(unsigned char hole, GIMMICK_ITEM_TYPE type,
		GimmickDispositionInformation::eFlag flag);
	bool NeedToMakeTriPtArray(const char* texName,
		FieldItem::eDisposeTextureType* pType) const;
	void AddItemAcquired(const FieldItem::AcquiredItemInfo& info);

	int GetRandomSeed() { return m_randomSeed; }
	float GetMinDistBtwItems() const { return m_minDistBtwItems; }
	std::vector<FieldItem::AcquiredItemInfo>& GetAcquiredItemInfoArray()
	{
		return m_acquiredItemInfoArray;
	}

protected:
#ifdef CONTENTSDOC_DEFER_GIMMICK_CONTAINER
	void AddGimmickPosition(unsigned char hole,
		const std::vector<GimmickDispositionInformation>& positions);
#endif

	std::map<unsigned char, std::vector<GimmickDispositionInformation> >
		m_gimmickPositionMap;
	int m_randomSeed;
	float m_minDistBtwItems;
	std::vector<FieldItem::AcquiredItemInfo> m_acquiredItemInfoArray;

public:
	std::string m_carpetSeqName;
	std::string m_crashSeqName[3];
	// JP 983 (sizeof 0xa0): byte em +0x9c que o construtor 00915880 nao inicia; lido so em 00919f48
	// (CGolfItemHandler::OnCheckSomeMoreGetSpecialBox: com ele ligado a caixa especial vale em qualquer mapa).
	// Quem grava: LoadFromPacket 009103e0 (registro com type 1 = caixa); nome provisorio.
	bool m_bSpecialBoxAnyMap983; // +0x9c
};
