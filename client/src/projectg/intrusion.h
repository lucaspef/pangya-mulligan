#pragma once

#include "frcmdtarget.h"
#include "singleton.h"

struct sRoomInfo;
struct sRivalData;
class WReceivedPacket;
class FrForm;

enum eIntrusionPermitCode
{
	INTRUSION_PERMIT_ENTER,
	INTRUSION_PERMIT_PLAYERS,
	INTRUSION_PERMIT_UNKNOWN2,
	INTRUSION_PERMIT_RIVALS,
	INTRUSION_PERMIT_START
};

enum eIntrusionDenyCode
{
	INTRUSION_DENY_NONE,
	INTRUSION_DENY_FULL,
	INTRUSION_DENY_TIMEOUT,
	INTRUSION_DENY_PRIVATE,
	INTRUSION_DENY_ALREADY_ENTERED,
	INTRUSION_DENY_NOT_ALLOWED,
	INTRUSION_DENY_ALREADY_PLAYED,
	INTRUSION_DENY_KICKED
};

class CIntrusion : public FrCmdTarget, public WSingleton<CIntrusion>
{
public:
	struct sInfoTime
	{
		int elapsedMin;
		int elapsedSec;
		int remainMin;
		int remainSec;

		sInfoTime()
		{
			elapsedMin = 0;
			elapsedSec = 0;
			remainMin = 0;
			remainSec = 0;
		}
	};

	CIntrusion();
	virtual ~CIntrusion();

	unsigned char GetCalcExpMemberSize();
	void FinishGame();
	unsigned long GetActiveColor();
	bool OnAccessRoom(sRoomInfo& room);
	void PacketAnalysis(WReceivedPacket& packet);
	bool IsIntrusionGame(unsigned short roomGuid);
	void DoJoinRoom(unsigned short roomGuid);

private:
	void DoIntrusionAccess(WReceivedPacket& packet, eIntrusionPermitCode code);
	void RivalDataUpdate(WReceivedPacket& packet);
	void PlayersUpdate(int bonus);
	void GetInfoTime(sInfoTime* pInfo, unsigned long elapsed,
		unsigned long limit);
	bool OnAlarmDlgResult(int result, FrForm* pForm);
	bool OnDelayDlgResult(int result, FrForm* pForm);
	bool OnDenyDlgResult(int result, FrForm* pForm);
	void DenyAnlaysis(eIntrusionDenyCode code);

	FrForm* m_pDlg;
	unsigned short m_roomGuid;
	unsigned char m_calcExpMemberSize;
};
