#pragma once
class CMatchHistory : public WSingleton<CMatchHistory>
{
public:
	bool IsUpdated() { return m_bUpdated; }
	void SetMatchHistory(sUserMatchHistory* pHistory)
	{
		if (pHistory == NULL)
		{
			return;
		}
		memcpy(m_history, pHistory, sizeof(m_history));
		if (!m_bUpdated)
			m_bUpdated = true;
	}

	void LoginRequest(std::string id)
	{
		if (m_loginId != id)
		{
			m_loginId = id;
			RequestHistory();
		}
	}

	void RequestHistory()
	{
		WSendPacket packet((enumClientPacket)0x9c);
		packet.Send((eSendTo)0);
		m_bUpdated = false;
	}

	void GetMatchHistory(sUserMatchHistory* pHistory)
	{
		memcpy(pHistory, m_history, sizeof(m_history));
	}

	CMatchHistory()
		: m_bUpdated(false)
	{
		memset(m_history, 0, sizeof(m_history));
	}
	virtual ~CMatchHistory() { }

protected:
	sUserMatchHistory m_history[5];
	bool m_bUpdated;
	std::string m_loginId;
};
