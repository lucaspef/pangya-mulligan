// JP 983: unidade 0064f900–00650540 (CSubGolfRuleBase + a carga da fita de replay). Classe nova do 983; nomes de método meus.
// O `ShotDataLoad` é o CGolfRule::ShotDataLoad do 645 (golfrule.cpp da frente A) com o que o 983 acrescentou: conferência das
// peças do personagem contra a tabela de itens, os três avisos 0x753b ao "Gateway" (0, 0x0d e 0x0b com o arquivo) e o bloco
// "EXTENSION". Deslocamentos do CSharedDoc/CGolfDoc/sPlayerData medidos no exe 983.
#include "subgolfrule.h"
#include "vc10_layout.h"
#include "golfdoc.h"
#include "parttidlist.h"
#include "taskmanager.h"
#include "gatewayactor.h"
#include "lobbymain.h"
#include "../shared/localize.h"
#include <list>

// registro de 0x41 bytes da lista de Doc+0x381c (o sSCardAvilityPeriodInfo do 645; campos não levantados)
#pragma pack(push, 1)
struct sCardPeriod983
{
	unsigned long m_00, m_04, m_08, m_0c, m_10, m_14, m_18, m_1c, m_20, m_24, m_28, m_2c, m_30, m_34, m_38, m_3c;
	unsigned char m_40;
};
#pragma pack(pop)

// A740: campos do CSharedDoc/CGolfDoc pelos membros; dentro do m_userInfo[0] (POD) fica o offset do 983 relativo ao membro
#define USERINFO983(off) ((char*)&Doc()->m_userInfo[0] + ((off) - 0x38b0))
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) == 0x38b0 && sizeof(sUserInfo) == 0x31c6 && offsetof(CSharedDoc, m_itemManager) == 0x20c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_shotDataList) == 0x110bc && offsetof(CSharedDoc, m_replayPlayType) == 0x11228);
C_ASSERT_VC10(offsetof(CSharedDoc, m_replayPosition) == 0x11270 && offsetof(CSharedDoc, m_replayShotIndex) == 0x1127c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_holeOrder) == 0x1051e && offsetof(CSharedDoc, m_courseMap) == 0x104fa);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame) + offsetof(sGolfGame, gameTypeName) == 0x10595 && offsetof(CSharedDoc, m_gameTypeInfo) == 0xc8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_curChannel) + offsetof(sChannelInfo, Type) == 0x10a99 && offsetof(CGolfDoc, m_currentHole) == 0x17c);

CSubGolfRuleBase::CSubGolfRuleBase()
{
}

CSubGolfRuleBase::~CSubGolfRuleBase()
{
}

void CSubGolfRuleBase::Init()
{
}

void CSubGolfRuleBase::Process(float dt)
{
}

void CSubGolfRuleBase::On3()
{
}

void CSubGolfRuleBase::OnShotEnd()
{
}

bool CSubGolfRuleBase::ShotDataLoad(const char* filename)
{
	size_t len = strlen(filename);
	FILE* fp = fopen(len == 0 ? "D:/work/ProjectG_S4/bin/save/pangya_003.sav" : filename, "rb");
	if (fp == NULL)
	{
		g_bQuit = true;
		return false;
	}
	unsigned long tag = 0;
	unsigned char map = 0;
	CPartTidList parts;
	CGolfRule::sShotData* pShot = RULEJP(m_pRule)->GetShotData983();
	int shotCount = 0;
	unsigned char hole = 0;
	int gameId = 0;
	unsigned long gameTag = 0xffffffff;
	CGolfRule::sShotData data;
	SYSTEMTIME time;
	memset(pShot, 0, sizeof(CGolfRule::sShotData));
	Doc()->m_shotDataList.clear();
	Doc()->m_replayShotIndex = 0;
	fread(&tag, 4, 1, fp);
	fread(&gameTag, 4, 1, fp);
	fread(&map, 1, 1, fp);
	fread(&time, 0x10, 1, fp);
	fread(&gameId, 4, 1, fp);
	Doc()->m_replayPlayType = gameId;
	fread(&Doc()->m_userInfo[0], 0x31c6, 1, fp);
	fread(&parts, 200, 1, fp);
	fread(&hole, 1, 1, fp);
	fread(GOLFDOC()->GetPlayer(0), 0xdb0, 1, fp);
	fread(&Doc()->m_curChannel.Type, 4, 1, fp);
	if (gameId == 1)
	{
		fread(pShot, 0x88, 1, fp);
		Doc()->m_replayShotIndex = pShot->holeStroke;
	}
	else if (gameId == 2)
	{
		fread(&shotCount, 4, 1, fp);
		for (int i = 0; i < shotCount; i++)
		{
			fread(&data, 0x88, 1, fp);
			Doc()->m_shotDataList.push_back(data);
			memset(&data, 0, sizeof(data));
		}
	}
	// por personagem (índice = tid & 0x3ffffff): até quatro posições da lista de peças que têm de existir na tabela de itens
	int slots[15][4] = {
		{ 0x12, -1, -1, -1 }, { 0x13, -1, -1, -1 }, { 0x12, -1, -1, -1 }, { 0x13, -1, -1, -1 },
		{ 6, 7, 0x12, 0x13 }, { 8, 9, -1, -1 }, { 0xf, 0x13, 0x14, -1 }, { 9, 0xa, 0xb, 0x16 },
		{ 8, 0x13, -1, -1 }, { 8, 0x15, 0x16, -1 }, { 8, 0x14, 0x15, -1 }, { -1, -1, -1, -1 },
		{ -1, -1, -1, -1 }, { -1, -1, -1, -1 }, { -1, -1, -1, -1 },
	};
	for (int j = 0; j < 4; j++)
	{
		int slot = slots[parts.m_charTid & 0x3ffffff][j];
		if (slot == -1)
			break;
		if (parts.m_tid[slot] != 0 && ((CItemManagerJp2*)&Doc()->m_itemManager)->Find983(parts.m_tid[slot]) == NULL)
		{
			parts.m_tid[slot] = 0;
			*(unsigned long*)(USERINFO983(0x680e) + slot * 4) = 0;
		}
	}
	((CCardManagerJp*)CCardManager::Instance())->Set983(0, USERINFO983(0x6a1c));
	if (IsLocalContent((localContentType_t)0x52))
	{
		int count = 0;
		fread(&count, 4, 1, fp);
		if (count > 0)
		{
			Doc()->m_cardPeriodList.clear();
			for (int i = 0; i < count; i++)
			{
				sCardPeriod983 info;
				fread(&info, 0x41, 1, fp);
				((CSharedDocJp2*)Doc())->AddCardPeriod983(&info);
			}
		}
		memcpy(CCardManager::Instance()->m_charInfo, USERINFO983(0x6802), 0x201);
		((CCardManagerJp*)CCardManager::Instance())->CalcCardPeriodAndStatus983();
		((CCardManagerJp*)CCardManager::Instance())->SetPlayerIndex983(0);
	}
	AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 0, 0, 0, 0, 0)
		<< MsgObject(NULL, 0x753b, 0xd, 0, 0, 0, 0) << MsgObject(NULL, 0x753b, 0xb, (int)fp, 0, 0, 0);
	ReplayReadExtension983(fp, &time);
	fclose(fp);
	*pShot = Doc()->m_shotDataList.front();
	SetCurMap(map);
	Doc()->m_holeOrder[0] = pShot->hole;
	((CSharedDocJp2*)Doc())->SetHoles983(Doc()->m_holeOrder, false);
	unsigned char n = 0;
	unsigned int idx;
	do
	{
		idx = n;
		n++;
		if (Doc()->m_holeOrder[idx] == pShot->hole)
			goto found;
	} while (n < 0x12);
	n = 0x13;
found:
	Doc()->m_courseMap[n - 1] = hole;
	GOLFDOC()->m_currentHole = pShot->hole;
	GOLFDOC()->SetPartTidList(0, parts.m_charTid, parts.m_hairColor, parts.m_shirtsColor, parts.m_tid, parts.m_uid);
	Doc()->m_golfGame.gameType = 0;
	strcpy(Doc()->m_golfGame.gameTypeName, *(char**)&Doc()->m_gameTypeInfo);
	*(float*)&Doc()->m_replayPosition = *(float*)((char*)pShot + 0x20);
	Doc()->m_replayPlayType = gameId;
	if (IsLocalContent((localContentType_t)0x58))
		RequestUccInfo(Doc()->m_userInfo[0]);
	return true;
}
