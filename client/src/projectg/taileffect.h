#pragma once

#include <string>
#include <vector>
#include <map>

#include "scenemanager.h"

class CTailEffect : public CRenderFuncPtr
{
public:
	CTailEffect(const std::string& texture);
	~CTailEffect();
	virtual void Display();
	void PutSpot(const WVector& nearPos, const WVector& farPos);
	void Reset();
	void Process(float elapsed);
	void ChangeTexture(const std::string& texture);

protected:
	void Init(const std::string& texture);
	int GetLength() const
	{
		if (m_head < m_tail)
			return m_maxEdge - m_tail + m_head;
		return m_head - m_tail;
	}

	struct TAILEDGE
	{
		bool bActive;
		float life;
		WVector nearPos;
		WVector farPos;
	};

	std::vector<TAILEDGE> m_edge;
	std::vector<TAILEDGE> m_spot;
	WVector m_lastNear;
	WVector m_lastFar;
	float m_life;
	int m_reserved;
	int m_maxEdge;
	int m_head;
	int m_tail;
	bool m_bReserved;
	int m_texHandle;
	std::map<std::string, int> m_texMap;
};
