// JP 983: unidade 00542c70–00544ce0 (CGDSkinItem). As funções de texto trocam o nome do arquivo de textura por
// "<nome><value da linha><ext>" quando uma peça vestida tem linha na tabela e o tipo de textura não é o `name` da linha.
#include "minatl.h"
#include <stdlib.h>
#include "gdskinitem.h"
#include "subgolfrule.h"
#include "wresrcmng.h"

CGDSkinItem* CGDSkinItem::s_pInstance;

bool CGDSkinItem::FileExists(const char* filename)
{
	cFile* pFile = g_resrcmng->GetCFile(filename, 0xffff);
	if (!pFile)
		return false;
	CloseCFile(pFile);
	return true;
}

void CGDSkinItem::ResetCur()
{
	m_cur = 0;
	m_bCur = false;
}

void CGDSkinItem::SetCur(int cur)
{
	m_cur = cur;
	m_bCur = true;
}

int CGDSkinItem::GetCur()
{
	if (!m_bCur)
		return 0;
	return m_cur;
}

void CGDSkinItem::ResetParts()
{
	for (int i = 0; i < 0x18; i++)
		m_parts[i] = 0;
	m_bParts = false;
}

void CGDSkinItem::SetParts(const sSkinParts983* pParts)
{
	if (pParts)
	{
		for (int i = 0; i < 0x18; i++)
			m_parts[i] = pParts->tidParts[i];
		m_bParts = true;
	}
}

void CGDSkinItem::SetParts(const unsigned long* pParts)
{
	if (pParts)
	{
		for (int i = 0; i < 0x18; i++)
			m_parts[i] = pParts[i];
		m_bParts = true;
	}
}

bool CGDSkinItem::HasRow(const unsigned long* pParts)
{
	if (pParts == NULL)
		return false;

	sRow row;
	for (int i = 0; i < 0x18; i++)
	{
		if (pParts[i])
		{
			for (std::list<sRow>::iterator it = m_rows.begin(); it != m_rows.end(); )
			{
				row = *it++;
				if (row.typeId == pParts[i])
					return true;
			}
		}
	}
	return false;
}

bool CGDSkinItem::IsRow(unsigned long typeId)
{
	if (typeId == 0)
		return false;

	sRow row;
	for (std::list<sRow>::iterator it = m_rows.begin(); it != m_rows.end(); )
	{
		row = *it++;
		if (row.typeId == typeId)
			return true;
	}
	return false;
}

#define SKIN983_ITEM() ((CItemManagerJp2*)&Doc()->m_itemManager)
#define SKIN983_MASK(tid) (*(unsigned long*)((char*)SKIN983_ITEM()->Find983(tid) + 0xec))

bool CGDSkinItem::Overlaps(const unsigned long* pParts, unsigned long typeId)
{
	if (typeId == 0)
		return false;
	if (pParts == NULL)
		return true;

	unsigned long mask = SKIN983_MASK(typeId);
	unsigned long found = 0;
	for (int i = 0; i < 0x18; i++)
	{
		if (pParts[i])
			found |= SKIN983_MASK(pParts[i]) & mask;
	}
	return found != 0;
}

bool CGDSkinItem::OverlapsKept(const unsigned long* pParts, unsigned long typeId)
{
	if (typeId == 0)
		return false;
	if (pParts == NULL)
		return true;

	unsigned long mask = SKIN983_MASK(typeId);
	unsigned long found = 0;
	for (int i = 0; i < 0x18; i++, pParts++)
	{
		unsigned long part = *pParts;
		if (part)
		{
			unsigned long before = found;
			found |= SKIN983_MASK(part) & mask;
			if (!IsRow(part))
				found = before;
		}
	}
	return found != 0;
}

bool CGDSkinItem::HasPartRow()
{
	const unsigned long* pParts = m_parts;
	if (pParts == NULL)
		return false;

	for (int i = 0; i < 0x18; i++, pParts++)
	{
		unsigned long part = *pParts;
		if (part)
		{
			sRow row;
			for (std::list<sRow>::iterator it = m_rows.begin(); it != m_rows.end(); )
			{
				row = *it++;
				if (row.typeId == part)
					return true;
			}
		}
	}
	return false;
}

bool CGDSkinItem::Check(const unsigned long* pParts, unsigned long typeId)
{
	m_bKept = false;
	if (!m_bParts)
		return false;

	if (typeId == 0)
	{
		m_bKept = false;
		return HasRow(pParts) ? true : false;
	}
	if (IsRow(typeId))
	{
		m_bKept = true;
		HasRow(pParts);
		return true;
	}
	if (HasRow(pParts))
	{
		if (OverlapsKept(pParts, typeId))
		{
			m_bKept = true;
			return false;
		}
		m_bKept = false;
		return true;
	}
	m_bKept = false;
	return false;
}

bool CGDSkinItem::ReplaceByCur(const char* kind, const char* name, std::string* pOut, unsigned long typeId)
{
	sRow row;
	for (std::list<sRow>::iterator it = m_rows.begin(); it != m_rows.end(); )
	{
		row = *it++;
		if (row.typeId == typeId)
		{
			if (row.name != kind)
			{
				char fname[0x100];
				char ext[0x100];
				_splitpath(name, NULL, NULL, fname, ext);
				*pOut = fname + row.value + ext;
				if (!FileExists(pOut->c_str()))
					*pOut = name;
				return true;
			}
		}
	}
	*pOut = name;
	return false;
}

bool CGDSkinItem::Replace(const char* kind, const char* name, std::string* pOut, const unsigned long* pParts)
{
	sRow row;
	if (pParts == NULL)
		pParts = m_parts;
	for (int i = 0; i < 0x18; i++)
	{
		if (pParts[i])
		{
			for (std::list<sRow>::iterator it = m_rows.begin(); it != m_rows.end(); )
			{
				row = *it++;
				if (row.typeId == pParts[i])
				{
					if (row.name != kind)
					{
						char fname[0x100];
						char ext[0x100];
						_splitpath(name, NULL, NULL, fname, ext);
						*pOut = fname + row.value + ext;
						if (!FileExists(pOut->c_str()))
							*pOut = name;
						return true;
					}
				}
			}
		}
	}
	*pOut = name;
	return false;
}

bool CGDSkinItem::Replace(const char* kind, const char* name, std::string* pOut, const sSkinParts983* pParts)
{
	SetParts(pParts);

	sRow row;
	const unsigned long* pList = m_parts;
	if (pParts)
		pList = pParts->tidParts;
	for (int i = 0; i < 0x18; i++)
	{
		if (pList[i])
		{
			for (std::list<sRow>::iterator it = m_rows.begin(); it != m_rows.end(); )
			{
				row = *it++;
				if (row.typeId == pList[i])
				{
					if (row.name != kind)
					{
						char fname[0x100];
						char ext[0x100];
						_splitpath(name, NULL, NULL, fname, ext);
						*pOut = fname + row.value + ext;
						if (!FileExists(pOut->c_str()))
							*pOut = name;
						return true;
					}
				}
			}
		}
	}
	*pOut = name;
	return false;
}

void CGDSkinItem::Apply(const char* kind, const char* name, std::string* pOut, const sSkinParts983* pParts, int cur)
{
	if (cur == 0)
	{
		if (pParts == NULL)
		{
			*pOut = name;
			ResetCur();
			return;
		}
		ResetCur();
	}
	else
	{
		SetCur(cur);
	}
	SetParts(pParts);

	if (Check(m_parts, cur) && m_bKept)
		ReplaceByCur(kind, name, pOut, cur);
	else
		Replace(kind, name, pOut, pParts);
}

CGDSkinItem::CGDSkinItem()
{
	m_b7a = false;
}

CGDSkinItem::~CGDSkinItem()
{
	s_pInstance = NULL;
}

void CGDSkinItem::Initialize(TiXmlNode* pNode)
{
	TiXmlNode* pChild = pNode->FirstChild("row");
	while (pChild)
	{
		sRow row;
		std::string strTypeId = pChild->ToElement()->Attribute("typeid");
		row.name = pChild->ToElement()->Attribute("name");
		row.value = pChild->ToElement()->Attribute("value");
		sscanf(strTypeId.c_str(), "%d", &row.typeId);
		m_rows.push_back(row);
		pChild = pChild->NextSibling("row");
	}
	ResetCur();
	ResetParts();
}
