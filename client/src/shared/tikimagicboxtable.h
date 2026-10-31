#pragma once

#include <string>
#include <map>
#include "classdefine.h"

struct sTikiMagicBoxMtr
{
	unsigned long dwTypeID;
	unsigned long dwGuid;
	unsigned long dwCount;
};

typedef std::map<unsigned int, IFF_STRUCT::sTikiOutputTable*>::iterator
	TikiOutputTableIterator;

class CTikiMagicBoxDoc : public WSingleton<CTikiMagicBoxDoc>
{
public:
	CTikiMagicBoxDoc() { CleanUp(); }
	virtual ~CTikiMagicBoxDoc() { CleanUp(); }

protected:
	std::map<std::string,
		std::map<unsigned int, IFF_STRUCT::sTikiOutputTable*>*>
		m_OPTCategory;
	std::map<unsigned int, IFF_STRUCT::sTikiPointTable*> m_PointTable;
	std::map<unsigned int, IFF_STRUCT::sTikiSpecialRecipe*> m_SpecialTable;

public:
	void InsertOutput(const IFF_STRUCT::sTikiOutputTable& table);
	void InsertPointTable(const IFF_STRUCT::sTikiPointTable& table);
	void InsertSpecialRecipe(const IFF_STRUCT::sTikiSpecialRecipe& table);
	void CleanUp();
};
