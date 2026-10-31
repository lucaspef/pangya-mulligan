#include "minatl.h"
#include <stack>

#include "tikimagicboxtable.h"

void CTikiMagicBoxDoc::InsertOutput(const IFF_STRUCT::sTikiOutputTable& table)
{
	IFF_STRUCT::sTikiOutputTable* pRecord = new IFF_STRUCT::sTikiOutputTable;
	if (pRecord == NULL)
		return;
	*pRecord = table;

	std::map<std::string,
		std::map<unsigned int, IFF_STRUCT::sTikiOutputTable*>*>::iterator itr =
		m_OPTCategory.find(pRecord->strCategory);

	if (itr == m_OPTCategory.end())
	{
		std::map<unsigned int, IFF_STRUCT::sTikiOutputTable*>* pMap =
			new std::map<unsigned int, IFF_STRUCT::sTikiOutputTable*>;
		if (pMap)
		{
			m_OPTCategory.insert(std::make_pair(pRecord->strCategory, pMap));
		}

		itr = m_OPTCategory.find(pRecord->strCategory);
	}

	if (itr != m_OPTCategory.end())
	{
		std::map<unsigned int, IFF_STRUCT::sTikiOutputTable*>* pMap =
			(*itr).second;

		if (pMap)
		{
			pMap->insert(std::make_pair(pRecord->uiIndex, pRecord));
		}
	}
}

void CTikiMagicBoxDoc::InsertPointTable(
	const IFF_STRUCT::sTikiPointTable& table)
{
	IFF_STRUCT::sTikiPointTable* pRecord = new IFF_STRUCT::sTikiPointTable;
	if (pRecord == NULL)
		return;
	*pRecord = table;

	m_PointTable.insert(std::make_pair(pRecord->uiIndex, pRecord));
}

void CTikiMagicBoxDoc::InsertSpecialRecipe(
	const IFF_STRUCT::sTikiSpecialRecipe& table)
{
	IFF_STRUCT::sTikiSpecialRecipe* pRecord =
		new IFF_STRUCT::sTikiSpecialRecipe;
	if (pRecord == NULL)
		return;
	*pRecord = table;

	m_SpecialTable.insert(std::make_pair(pRecord->uiIndex, pRecord));
}

void CTikiMagicBoxDoc::CleanUp()
{
	std::map<std::string,
		std::map<unsigned int, IFF_STRUCT::sTikiOutputTable*>*>::iterator
		opcBegin = m_OPTCategory.begin(),
		opcEnd = m_OPTCategory.end();

	for (; opcBegin != opcEnd; ++opcBegin)
	{
		std::map<unsigned int, IFF_STRUCT::sTikiOutputTable*>* pMap =
			(*opcBegin).second;
		if (pMap)
		{
			std::map<unsigned int, IFF_STRUCT::sTikiOutputTable*>::iterator
				opBegin = pMap->begin(),
				opEnd = pMap->end();

			for (; opBegin != opEnd; ++opBegin)
			{
				IFF_STRUCT::sTikiOutputTable* pTable = (*opBegin).second;
				if (pTable)
				{
					delete pTable;
					(*opBegin).second = NULL;
				}
			}
			pMap->clear();

			delete pMap;
			(*opcBegin).second = NULL;
		}
	}
	m_OPTCategory.clear();

	std::map<unsigned int, IFF_STRUCT::sTikiPointTable*>::iterator
		ptBegin = m_PointTable.begin(),
		ptEnd = m_PointTable.end();

	for (; ptBegin != ptEnd; ++ptBegin)
	{
		IFF_STRUCT::sTikiPointTable* pTable = (*ptBegin).second;
		if (pTable)
		{
			delete pTable;
			(*ptBegin).second = NULL;
		}
	}
	m_PointTable.clear();

	std::map<unsigned int, IFF_STRUCT::sTikiSpecialRecipe*>::iterator
		spBegin = m_SpecialTable.begin(),
		spEnd = m_SpecialTable.end();

	for (; spBegin != spEnd; ++spBegin)
	{
		IFF_STRUCT::sTikiSpecialRecipe* pRecord = (*spBegin).second;
		if (pRecord)
		{
			delete pRecord;
			(*spBegin).second = NULL;
		}
	}
	m_SpecialTable.clear();
}
