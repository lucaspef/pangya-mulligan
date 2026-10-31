#pragma once
#include <vector>
struct sIntrest;
class CMatchingSystem : public WSingleton<CMatchingSystem>
{
public:
	CMatchingSystem();
	virtual ~CMatchingSystem();

	void Clear();
	int SetDisplayIntrest(const sIntrest& intrest);
	int GetDisplayIntrests(const std::string& group,
		std::vector<sIntrest>* out);
	const char* GetGroup(unsigned char code);
	const sIntrest* GetIntrestInfo(unsigned char code);
	unsigned char GetDisplayIntrestCode(const std::string& group,
		const std::string& name);
	std::string GetNameChecked(const std::string& group);

	int SetMyIntrest(unsigned char code);
	int CheckIntrestCode(unsigned char code);
	void ClearMyIntrests() { m_myIntrests.clear(); }

protected:
	std::vector<sIntrest> m_intrests;
	std::vector<unsigned char> m_myIntrests;
};
