#pragma once

class CIndexControl
{
public:
	CIndexControl();
	virtual ~CIndexControl();

	bool Init(unsigned long totalIndex, unsigned long indexPerPage);
	bool BuildPage(unsigned long page);
	void Draw(WPoint& pos);
	bool Click();
	bool NextPage();
	bool PrevPage();
	bool SelectIndex(unsigned long index);
	void SetSelectedNumber(unsigned long number);
	unsigned long GetSelectedNumber() const;
	unsigned long GetSelectedIndex() const;
	unsigned long GetCurIndexCount() const;
	unsigned long GetTotalPage() const;

protected:
	void ClearVariables();

	WRect m_indexRect;
	bool m_bInit;
	unsigned char m_align;
	unsigned long m_selColor;
	unsigned long m_overColor;
	unsigned long m_sepColor;
	unsigned long m_totalIndex;
	unsigned long m_totalPage;
	unsigned long m_curIndexCount;
	unsigned long m_indexPerPage;
	unsigned long m_curPage;
	unsigned long m_selIndex;
	unsigned long m_overIndex;
	WRect m_cellRect;
	WRect* m_pIndexRect;
	WPoint m_pos;
};
