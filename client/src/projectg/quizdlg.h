#pragma once

#include <string>
#include "frform.h"

struct sQuizCase
{
	bool bCorrect;
	std::string text;
};

struct sQuiz
{
	std::string question;
	sQuizCase cases[5];
};

class FrListBox;

class AnswerCard
{
public:
	enum eCheckNum
	{
		CHECK_NONE = 0,
		CHECK_1 = 1,
		CHECK_2 = 2,
		CHECK_3 = 4,
		CHECK_4 = 8
	};

	AnswerCard()
	{
		m_bMultiCheck = false;
		m_answer = 0;
	}
	~AnswerCard() { }
	bool WriteAnswer(eCheckNum num);
	bool IsChecked(eCheckNum num) const;
	int ISAllCheck() const;
	unsigned char GetAnswerCard() const;
	void DeleteAnswer(eCheckNum num);
	void SetMode(bool bMultiCheck) { m_bMultiCheck = bMultiCheck; }

private:
	bool m_bMultiCheck;
	unsigned char m_answer;
};

class FrQuizDlg : public FrForm
{
	DECLARE_OBJECT(FrQuizDlg)
	FrQuizDlg();

	virtual void OnProc(const float delta);
	void SetupQuiz(int index);
	const char* GetQuestion();
	bool IsCorrectAnswer();
	void ProcessChkBox();

protected:
	void OnBQuiz1Init(int param);
	void OnBQuiz1BtnDown();
	void OnBQuiz2Init(int param);
	void OnBQuiz2BtnDown();
	void OnBQuiz3Init(int param);
	void OnBQuiz3BtnDown();
	void OnBQuiz4Init(int param);
	void OnBQuiz4BtnDown();
	void OnCasesInit(int param);
	void OnCasesOwnerDraw(int param);
	void OnCasesBtnDown();

	FrListBox* m_pCases;
	sQuiz* m_pQuiz;
	int m_caseIndex[4];
	int m_answer;
	int m_reserved;
	bool m_bRefresh;
	FrButton* m_pBQuiz[4];
	AnswerCard m_card;

	DECLARE_FRESH_MSGMAP()
};
