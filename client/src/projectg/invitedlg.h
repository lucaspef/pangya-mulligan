#pragma once

#include "frform.h"

class FrInviteDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrInviteDlg)

	FrInviteDlg() { Init(); }
	virtual ~FrInviteDlg() { }

	void SetRoomInfo(unsigned short roomIdx);
	void SetInvaiteInfo(unsigned long svrGUID, unsigned char channelIdx,
		unsigned short roomIdx, unsigned long fromUID, unsigned long toUID);

	unsigned long GetSvrGUID() { return m_svrGUID; }
	unsigned char GetChannelIdx() { return m_channelIdx; }
	unsigned short GetRoomIdx() { return m_roomIdx; }
	unsigned long GetFromUID() { return m_fromUID; }

private:
	void Init();

	unsigned long m_svrGUID;
	unsigned char m_channelIdx;
	unsigned short m_roomIdx;
	unsigned long m_fromUID;
	unsigned long m_toUID;
};
