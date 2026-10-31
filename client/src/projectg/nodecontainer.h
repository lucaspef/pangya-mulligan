#pragma once

#include <string>
#include <vector>

class CNodeContainer
{
public:
	CNodeContainer(std::string name)
		: m_name(name), m_type(TYPE_NONE)
	{
	}
	virtual ~CNodeContainer() { m_list.clear(); }

	enum eType
	{
		TYPE_NONE,
		TYPE_OUTLINE,
		TYPE_IB,
		TYPE_OB,
		TYPE_AUTO,
		TYPE_BEACH,
		TYPE_CAMERA,
		TYPE_RIVER,
		TYPE_RIVEREX,
		TYPE_WATERFALL,
		TYPE_SEA,
		TYPE_WAVE,
		TYPE_NPC,
		TYPE_LAVA,
		TYPE_FILL983, // 983: 0xe ("fill", 00940c20)
	};

	void SetType(eType type) { m_type = type; }
	eType GetType() const { return m_type; }
	std::string GetName() const { return m_name; }

	void Add(WVector pos) { m_list.push_back(pos); }

private:
	std::string m_name;
	eType m_type;
	std::vector<WVector> m_list;

public:
	const std::vector<WVector>& GetList() const { return m_list; }
};
