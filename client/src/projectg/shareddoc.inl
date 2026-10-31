inline CSharedDoc* Doc()
{
	return CSharedDoc::Instance();
}

inline CItemManager* ItemManager();

inline int GalleryMode();

inline CGolfDoc* GOLFDOC();

inline unsigned char PrevMap(unsigned char map)
{
	int cur = 20; // 983: 21 entradas (00502dc0)
	for (int i = 0; i < 21; i++)
	{
		if (s_courseOrder[i] == map)
		{
			cur = i;
			break;
		}
	}

	for (int j = cur - 1; j >= 0; j--)
	{
		if (IsLocalContent(S3_ROOKIE_CHANNEL))
		{
			if ((Doc()->m_curChannel.Type & 0x800) &&
				!Doc()->CanUseRookieChannelMap(s_courseOrder[j]))
				continue;
		}
		return s_courseOrder[j];
	}

	return map;
}

inline unsigned char NextMap(unsigned char map, bool bLoop)
{
	int cur = 0;
	for (int i = 0; i < 21; i++)
	{
		if (s_courseOrder[i] == map)
		{
			cur = i;
			break;
		}
	}

	for (int j = cur + 1; j < 21; j++)
	{
		if (IsLocalContent(S3_ROOKIE_CHANNEL))
		{
			if ((Doc()->m_curChannel.Type & 0x800) &&
				!Doc()->CanUseRookieChannelMap(s_courseOrder[j]))
				continue;
		}
		return s_courseOrder[j];
	}

	return map;
}

inline unsigned char FirstMap()
{
	if (!(Doc()->m_curChannel.Type & 0x800))
		return 20; // 983: 00505140

	if (IsLocalContent(S3_ROOKIE_CHANNEL) && (Doc()->m_curChannel.Type & 0x800))
		return 11;

	int i;
	for (i = 0; i < 21; i++)
	{
		IFF_STRUCT::sCourse* pCourse =
			ItemManager()->FindCourse(s_courseOrder[i] | 0x28000000);
		if (pCourse->Difficulty < 3)
			break;
	}
	return s_courseOrder[i];
}

inline unsigned char LastMap(bool bAll)
{
	if (!(Doc()->m_curChannel.Type & 0x800))
	{
		if (bAll)
			return 9;
		return 127;
	}

	int i;
	for (i = 20; i >= 0; i--)
	{
		if (bAll && s_courseOrder[i] == 127)
			continue;
		IFF_STRUCT::sCourse* pCourse =
			ItemManager()->FindCourse(s_courseOrder[i] | 0x28000000);
		if (pCourse->Difficulty < 3)
			break;
	}
	return s_courseOrder[i];
}

#ifndef REBANG_PCH_POOL // HACK: pool.cpp defines it itself (emission order), other units keep their IL
inline const char* GetCurMapName()
{
	if (Doc()->m_golfGame.pCourse)
		return Doc()->m_golfGame.pCourse->Data;

	return NULL;
}
#endif

inline const char* GetGameTypeName(unsigned char gameType);

inline void SetGameType(unsigned char gameType)
{
	Doc()->m_golfGame.gameType = gameType;
	if (gameType < GAME_TYPE_MAX)
		strcpy(Doc()->m_golfGame.gameTypeName, GetGameTypeName(gameType));
}

inline const sGameTypeInfo* GetGameTypeInfo(unsigned char gameType)
{
	if (gameType >= GAME_TYPE_MAX)
	{
		return NULL;
	}

	return &Doc()->m_gameTypeInfo[gameType];
}

inline const char* GetGameTypeName(unsigned char gameType)
{
	if (gameType >= GAME_TYPE_MAX)
	{
		return NULL;
	}

	return Doc()->m_gameTypeInfo[gameType].name;
}

#ifdef REBANG_GOLFRULE_PCH
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: inline bodies the original PCH has here (emission order)
inline bool IsOthersTurn(bool check)
{
	return !IsMyTurn(check);
}

inline IFF_STRUCT::sCourse* GetCurMapProp()
{
	return Doc()->m_golfGame.pCourse;
}
#endif

#endif
inline unsigned char GetHoles()
{
	return Doc()->m_golfGame.holes;
}

inline void SetHoles(unsigned char holes)
{
	Doc()->m_golfGame.holes = holes;
}

inline void SetWeather(unsigned char weather)
{
	Doc()->m_golfGame.weather = weather;
}

#ifdef REBANG_GOLFRULE_PCH
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: inline bodies the original PCH has here (emission order)
inline unsigned char GetStartHole()
{
	return Doc()->m_holeOrder[0];
}

inline unsigned char GetEndHole()
{
	return Doc()->m_holeOrder[Doc()->m_golfGame.holes - 1];
}
#endif

#endif
inline unsigned char GetHoleIndex(unsigned char hole)
{
	for (unsigned char i = 0; i < 18; i++)
	{
		if (Doc()->m_holeOrder[i] == hole)
			return i + 1;
	}
	return 19;
}

#ifdef REBANG_GOLFRULE_PCH
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: inline bodies the original PCH has here (emission order)
inline unsigned char GetWeather()
{
	return Doc()->m_golfGame.weather;
}
#endif

#endif
inline unsigned long GetShotTimeLimit()
{
	return Doc()->m_golfGame.shotTimeLimit;
}
#ifdef REBANG_GOLFRULE_PCH

#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: inline bodies the original PCH has here (emission order)
inline unsigned char& UIAlpha()
{
	return Doc()->m_pGolfDoc->m_uiAlpha; // 983 +0xdc (B1464: era cru; no cl 14.44 o CGolfDoc anda -0x2c, B1455)
}

inline sBar& Bar()
{
	return Doc()->m_pGolfDoc->m_bar; // 983 +0xe0 (B1464: era cru)
}

inline void SetGameOver(bool b)
{
	Doc()->m_bGameOver = b;
}

inline bool IsGameOver()
{
	return Doc()->m_bGameOver;
}

inline unsigned long GetGameTimeLimit()
{
	return Doc()->m_golfGame.gameTimeLimit;
}

inline int IsObserver(unsigned long id)
{
	if (id == 0xffffffff)
		id = Doc()->m_myInfo.info.dwIdentity;
	return (id & 0xe) == 0xe;
}
#endif
#endif

inline CItemManager* ItemManager()
{
	return &Doc()->m_itemManager;
}

inline unsigned char GetPlayerNum()
{
	return GOLFDOC()->GetPlayerNum();
}

inline sPlayerData* PLAYER(unsigned char index)
{
	return GOLFDOC()->GetPlayer(index);
}

inline sTeamData* TEAM(unsigned char index)
{
	return GOLFDOC()->GetTeam(index);
}

inline sRivalData* GUILDMATCHUP(unsigned char index)
{
	return &Doc()->m_rivalList[Doc()->GetIndex(Doc()->m_teamPlayerGuid[index])];
}

#ifdef REBANG_CLIENTSETTING_PCH
inline unsigned char GetGameType()
{
	return Doc()->m_golfGame.gameType;
}

inline void SetHoleType(unsigned char type)
{
	Doc()->m_holeType = type;
}

inline void SetShotTimeLimit(unsigned long limit)
{
	Doc()->m_golfGame.shotTimeLimit = limit;
}
#endif

inline CGolfDoc* GOLFDOC()
{
	return Doc()->m_pGolfDoc;
}

#ifdef REBANG_CLIENTSETTING_PCH
inline void SetGameTimeLimit(unsigned long limit)
{
	Doc()->m_golfGame.gameTimeLimit = limit;
}
#endif

inline const WVector& GetHoleCupPos() // 983: 005298c0 devolve referencia
{
	return GOLFDOC()->GetHoleData().pin;
}

inline unsigned char CSharedDoc::GetIndex(unsigned long uid)
{
	if (uid == 0xffffffff)
	{
		if (IS_KINDOF(CGolfTask, AfxGetTask()))
			return GOLFDOC()->m_currentPlayer;
	}
	else if (OnlinePlay())
	{
		if (m_indexMap.find(uid) == m_indexMap.end())
			return 0xff;

		return m_indexMap[uid];
	}
	else
	{
		if (IS_KINDOF(CGolfTask, AfxGetTask()))
			return GOLFDOC()->m_currentPlayer;
	}

	return 0xff;
}

// HACK: stands in for an unknown PCH inline that deletes a std::string
inline void DeleteString(std::string* p)
{
	std::auto_ptr<std::string> holder(p);
}

inline unsigned long MyGuid(bool bGallery)
{
#ifdef REBANG_TASKMAIN_PCH // taskmain.obj tests GalleryMode() first; other units keep their PCH
	if (GalleryMode() && bGallery)
#else
	if (bGallery && GalleryMode())
#endif
		return Doc()->m_myInfo.info.dwGalleryGuid;
	return Doc()->m_myInfo.info.dwGuid;
}

inline unsigned long MyUID()
{
	return Doc()->m_myInfo.info.dwUID;
}

inline int IsMassGame()
{
	switch (Doc()->m_golfGame.gameType)
	{
	case GAME_TYPE_30S:
	case GAME_TYPE_30S_TEAM:
	case GAME_TYPE_GUILD_MATCH:
	case GAME_TYPE_APPROACH:
	case GAME_TYPE_NEW_APPROACH:
	case GAME_TYPE_HOLEINONE983: // 983: 00502f20 (tabela de 00502f54)
		return TRUE;
	}
	return FALSE;
}

inline int IsMassGame(unsigned char gameType)
{
	switch (gameType)
	{
	case GAME_TYPE_30S:
	case GAME_TYPE_30S_TEAM:
	case GAME_TYPE_GUILD_MATCH:
	case GAME_TYPE_APPROACH:
	case GAME_TYPE_NEW_APPROACH:
	case GAME_TYPE_HOLEINONE983: // 983: 00500480 (4, 5, 6, 9, 10, 11 e 18)
	case GAME_TYPE_USEMAX:
		return TRUE;
	}
	return FALSE;
}

inline int IsGuildGame()
{
	return Doc()->m_golfGame.gameType == GAME_TYPE_GUILD_MATCH;
}

inline char* MyId()
{
	return Doc()->m_myInfo.info.sID;
}

inline char* MyNick()
{
	return Doc()->m_myInfo.info.sNick;
}

inline __int64 MyPang()
{
	return Doc()->m_myInfo.stat.i64Pang;
}

inline __int64 MyCookie()
{
	return Doc()->m_cookie;
}

#ifdef REBANG_TASKMAIN_PCH // HACK: taskmain_pch.h only (taskmain.obj has its own copies; other units keep their IL)
inline int OfflinePlay()
{
	return Doc()->m_offlinePlay;
}

inline __int64 MyOutsideCash()
{
	return Doc()->m_cash;
}
#endif

inline int GalleryMode()
{
	bool bGM = (Doc()->m_myInfo.info.dwIdentity >> 1) & 1;
	if (bGM)
		return TRUE;
	return FALSE;
}

inline bool IsAngelWing(unsigned long typeId)
{
	switch (typeId)
	{
	case 0x08016800:
	case 0x08058800:
	case 0x08098800:
	case 0x080dc800:
	case 0x08118800:
	case 0x08160800:
	case 0x08190800:
	case 0x081e2800:
	case 0x08214800:
	case 0x08254800:
	case 0x082d480e: // 983 004ce760: mais 3 personagens que o 645 nao tinha
	case 0x08314800:
	case 0x0839c800:
		return true;
	}

	return false;
}

inline unsigned char GetCurRoomMapType()
{
	return Doc()->m_roomInfo.mapType;
}

inline unsigned char GetDiffFlag()
{
	IFF_STRUCT::sCourse* pCourse = Doc()->m_golfGame.pCourse;
	if (pCourse == NULL)
		return 0;
	return pCourse->DiffFlag;
}

inline WVector GetTeeShotPos()
{
	return Doc()->m_pGolfDoc->GetHoleData().tee;
}

inline sCameraData& GameCamera()
{
	return Doc()->m_pGolfDoc->m_cameraData;
}

#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
inline void CSharedDoc::SetBsUsableTimes(short times)
{
	m_myInfo.info.iBongdariShopUsableTimes = times;
}

inline void CSharedDoc::SetBsUsableBonusTimes(short times)
{
	m_myInfo.info.iBongdariShopBonusTimes = times;
}

inline void CSharedDoc::SetBsRemainedBonusTimes(short times)
{
	m_myInfo.info.iBongdariShopRemainedBonus = times;
}

inline void CSharedDoc::SetTradeTitle(std::string title)
{
	m_tradeTitle = title;
}

inline void CSharedDoc::SetCountVisitor(int count)
{
	m_countVisitor = count;
}

inline void CSharedDoc::SetStallItem(sStall stall)
{
	m_stallItem = stall;
}

inline void CSharedDoc::SetTradeIncome(__int64 income)
{
	m_tradeIncome = income;
}

inline void CSharedDoc::SetFlagBlock(unsigned long flagBlock)
{
	m_myInfo.info.dwFlagBlock = flagBlock;
}

inline void __fastcall SetMyOutsideCash(__int64 cash)
{
	CSharedDoc::Instance()->m_cash = cash;
}

#endif
