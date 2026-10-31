#include "minatl.h"
#include "ghostmanager.h"
#include "ghostdocument.h"
#include "ghosthandler.h"
#include "type_convert.h"
#include "netresourcemanager.h"
#include "shareddoc.h"

#define SAFE_DELETE(p) \
	{ \
		if (p) \
		{ \
			delete (p); \
			(p) = NULL; \
		} \
	}

CGhostDocument* CreateDocument()
{
	return new CGhostDocument;
}

CGhostManager::CGhostManager()
	: m_pDocument(NULL), m_pLatestFileInfo(NULL)
{
}

CGhostManager::~CGhostManager()
{
	SAFE_DELETE(m_pDocument);
	SAFE_DELETE(m_pLatestFileInfo);
}

CGhostDocument* CGhostManager::GetDocument()
{
	if (m_pDocument == NULL)
		m_pDocument = CreateDocument();

	return m_pDocument;
}

__forceinline void CGhostManager::ReleaseDocument()
{
	SAFE_DELETE(m_pDocument);
}

void CGhostManager::SaveDocument(const std::string& path)
{
	if (m_pDocument)
	{
		GetTickCount();
		m_pDocument->Export(path);

		ReleaseDocument();

		sResource res;
		res.filename = path;
		res.type = RESOURCE_GHOST;
		sprintf(res.arg, "%d", MyUID());

		NetResourceManager::Instance()->Upload(res,
			"http://202.213.206.154/real/upload/ghost/upload.asp"); // 983 00828bd0 (0xd2fae8)
	}
}

void CGhostManager::LoadDocument(const std::string& name)
{
	m_latestFileName = name;
	SAFE_DELETE(m_pLatestFileInfo);
	m_pLatestFileInfo = new GHOST_FILE_INFO;
	const char* fname = name.c_str();
	GHOST_FILE_INFO* info = m_pLatestFileInfo;
	sscanf(fname, "%d_%d_%d_%d_%d_%d.GST", &info->m_date, &info->m_time,
		&info->m_course, &info->m_score, &info->m_uid, &info->m_index);
	int date = info->m_date;

	sResource res;
	res.filename = name;
	res.type = RESOURCE_GHOST;
	sprintf(res.arg, "%d", date);

	NetResourceManager::Instance()->Download(res,
		"http://202.213.206.154/real/upload/ghost/ghost/"); // 983 00828cf0 (0xd2fb1c)
}

void CGhostManager::LoadDocumentFromLocalFile(const std::string& path)
{
	m_latestFileName = path;
	SAFE_DELETE(m_pLatestFileInfo);
	m_pLatestFileInfo = new GHOST_FILE_INFO;
	const char* fname = path.c_str();
	sscanf(fname, "%d_%d_%d_%d_%d_%d.GST", &m_pLatestFileInfo->m_date,
		&m_pLatestFileInfo->m_time, &m_pLatestFileInfo->m_course,
		&m_pLatestFileInfo->m_score, &m_pLatestFileInfo->m_uid,
		&m_pLatestFileInfo->m_index);

	CGhostHandler::Instance()->LoadingComplete(
		CGhostLoader().LoadFromFile(path));
}

void conv_ghostdata_to_userinfo(const GHOST_USER_DATA* src, sUserInfo* dst)
{
	dst->charInfo.tid = src->m_charTypeId;
	dst->charInfo.hairClr = src->m_hairColor;
	memcpy(dst->charInfo.tidParts, src->m_parts,
		sizeof(dst->charInfo.tidParts));
	memcpy(dst->charInfo.tidAuxParts, src->m_auxParts,
		sizeof(dst->charInfo.tidAuxParts));
	memcpy(dst->charInfo.PCL, src->m_charPcl, sizeof(dst->charInfo.PCL));

	dst->clubInfo.tid = src->m_clubTypeId;
	memcpy(dst->clubInfo.PCL, src->m_clubPcl, sizeof(dst->clubInfo.PCL));

	dst->userEquip.tidBall = src->m_ballTypeId;

	dst->caddieInfo.tid = src->m_caddieTypeId;
	dst->caddieInfo.tidPart = src->m_caddieParts;
	dst->caddieInfo.Level = src->m_caddieLevel;

	dst->mascotInfo.tid = src->m_mascotTypeId;
	py_strcpy(dst->mascotInfo.szMsg, src->m_mascotMsg);

	memcpy(dst->userEquip.tidItemSlot, src->m_itemSlot,
		sizeof(dst->userEquip.tidItemSlot));
	memcpy(dst->charInfo.UccIndexList, src->m_uccIndexList,
		sizeof(dst->charInfo.UccIndexList));

	py_strcpy(dst->info.sNick, src->m_nick);
	py_strcpy(dst->info.sID, src->m_id);
	py_strcpy(dst->info.sGuild, src->m_guild);
	py_strcpy(dst->info.szEmblemName, src->m_emblem);

	dst->info.dwIdentity = src->m_identity;
	dst->info.dwEmblemVer = src->m_emblemVer;

	dst->info.gender = src->m_gender;
	dst->info.devilWings = src->m_devilWing;
	dst->stat.Level = src->m_level;
	dst->info.angelicWings = src->m_angelWing;
	dst->info.angelicWingsEffect = src->m_angelEffect;
	dst->info.dwUID = src->m_uid;
}

void conv_userinfo_to_ghostdata(const sUserInfo* src, GHOST_USER_DATA* dst)
{
	dst->m_charTypeId = src->charInfo.tid;
	dst->m_hairColor = src->charInfo.hairClr;
	memcpy(dst->m_parts, src->charInfo.tidParts, sizeof(dst->m_parts));
	memcpy(dst->m_auxParts, src->charInfo.tidAuxParts, sizeof(dst->m_auxParts));
	memcpy(dst->m_charPcl, src->charInfo.PCL, sizeof(dst->m_charPcl));

	dst->m_clubTypeId = src->clubInfo.tid;
	memcpy(dst->m_clubPcl, src->clubInfo.PCL, sizeof(dst->m_clubPcl));

	dst->m_ballTypeId = src->userEquip.tidBall;

	dst->m_caddieTypeId = src->caddieInfo.tid;
	dst->m_caddieParts = src->caddieInfo.tidPart;
	dst->m_caddieLevel = src->caddieInfo.Level;

	dst->m_mascotTypeId = src->mascotInfo.tid;
	py_strcpy(dst->m_mascotMsg, src->mascotInfo.szMsg);

	memcpy(dst->m_itemSlot, src->userEquip.tidItemSlot,
		sizeof(dst->m_itemSlot));
	memcpy(dst->m_uccIndexList, src->charInfo.UccIndexList,
		sizeof(dst->m_uccIndexList));

	py_strcpy(dst->m_nick, src->info.sNick);
	py_strcpy(dst->m_id, src->info.sID);
	py_strcpy(dst->m_guild, src->info.sGuild);
	py_strcpy(dst->m_emblem, src->info.szEmblemName);

	dst->m_identity = src->info.dwIdentity;
	dst->m_emblemVer = src->info.dwEmblemVer;

	dst->m_gender = src->info.gender;
	dst->m_angelWing = src->info.angelicWings;
	dst->m_angelEffect = src->info.angelicWingsEffect;
	dst->m_devilWing = src->info.devilWings;
	dst->m_level = src->stat.Level;
	dst->m_uid = src->info.dwUID;
}

void conv_ghostdata_to_slotinfo(const GHOST_USER_DATA* src, sSlotInfo* dst)
{
	py_strcpy(dst->sNick, src->m_nick);
	py_strcpy(dst->sGuild, src->m_guild);
	py_strcpy(dst->szEmblemName, src->m_emblem);

	dst->level = src->m_level;
	dst->dwIdentity = src->m_identity;
	dst->dwEmblemVer983 = src->m_emblemVer;

	dst->gender = src->m_gender;
	dst->angelicWings = src->m_angelWing;
	dst->angelicWingsEffect = src->m_angelEffect;
	dst->bReady = 1;
	dst->connectionRank = 2;
	dst->dwGuid = -2;
	dst->tidChar = src->m_charTypeId;
	dst->tidMascot = src->m_mascotTypeId;
}

void conv_legacycard_to_ghostcard(const sSCardAvilityPeriodInfo* src,
	GHOST_CARD_DATA* dst)
{
	dst->m_typeId = src->tid;
	dst->m_itemId = src->uid;
	dst->m_attachedId = src->partsTid;
	dst->m_partsItemId = src->partsUid;
	dst->m_ability = src->Avility;
	dst->m_abilityValue = src->AvilityValue;
	dst->m_slotNum = src->slotNum;
	dst->m_cardType = src->cardType;
}

void conv_ghostcard_to_legacycard(const GHOST_CARD_DATA* src,
	sSCardAvilityPeriodInfo* dst)
{
	dst->Avility = src->m_ability;
	dst->AvilityValue = src->m_abilityValue;
	dst->cardType = src->m_cardType;
	dst->partsTid = src->m_attachedId;
	dst->partsUid = src->m_partsItemId;
	dst->slotNum = src->m_slotNum;
	dst->tid = src->m_typeId;
	dst->uid = src->m_itemId;
	dst->valid = 1;
}

void conv_ghostshot_to_shotprotocol(const sShotProtocol* src,
	GHOST_SHOT_PROTOCOL* dst)
{
	dst->m_maxClick = src->m_maxClick;
	dst->m_minClick = src->m_minClick;
	dst->m_curve = src->m_curve;
	dst->m_spin = src->m_spin;
	dst->m_accuracy = src->m_accuracy;
	dst->m_special = src->m_special;
	dst->m_frame = src->m_frame;
	dst->m_shotAngle = src->m_shotAngle;
	dst->m_shotTime = src->m_shotTime;
	dst->m_center = src->m_center;
	dst->m_club = src->m_club;
	dst->m_randomPower = src->m_randomPower;
	dst->m_randomAngle = src->m_randomAngle;
	dst->m_pangyaArea = src->m_pangyaArea;
}

void conv_shotprotocol_to_ghostshot(const GHOST_SHOT_PROTOCOL* src,
	sShotProtocol* dst)
{
	dst->m_maxClick = src->m_maxClick;
	dst->m_minClick = src->m_minClick;
	dst->m_curve = src->m_curve;
	dst->m_spin = src->m_spin;
	dst->m_accuracy = src->m_accuracy;
	dst->m_special = src->m_special;
	dst->m_frame = src->m_frame;
	dst->m_shotAngle = src->m_shotAngle;
	dst->m_shotTime = src->m_shotTime;
	dst->m_center = src->m_center;
	dst->m_club = src->m_club;
	dst->m_randomPower = src->m_randomPower;
	dst->m_randomAngle = src->m_randomAngle;
	dst->m_pangyaArea = src->m_pangyaArea;
}

#define REGISTER_CONVERTER(from, to, fn) \
	static bool s_reg_##fn = \
		type_convert::insert_converter<from, to>((type_convert::convert_fn)fn)

REGISTER_CONVERTER(sUserInfo, GHOST_USER_DATA, conv_userinfo_to_ghostdata);
REGISTER_CONVERTER(GHOST_USER_DATA, sUserInfo, conv_ghostdata_to_userinfo);
REGISTER_CONVERTER(GHOST_USER_DATA, sSlotInfo, conv_ghostdata_to_slotinfo);
REGISTER_CONVERTER(GHOST_CARD_DATA, sSCardAvilityPeriodInfo,
	conv_ghostcard_to_legacycard);
REGISTER_CONVERTER(sSCardAvilityPeriodInfo, GHOST_CARD_DATA,
	conv_legacycard_to_ghostcard);
REGISTER_CONVERTER(sShotProtocol, GHOST_SHOT_PROTOCOL,
	conv_ghostshot_to_shotprotocol);
REGISTER_CONVERTER(GHOST_SHOT_PROTOCOL, sShotProtocol,
	conv_shotprotocol_to_ghostshot);
