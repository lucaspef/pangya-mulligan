#pragma once

#include "frform.h"

class FrArea;

class FrNewMapEventDlg : public FrForm
{
	DECLARE_OBJECT(FrNewMapEventDlg)

	FrNewMapEventDlg();
	virtual ~FrNewMapEventDlg();

protected:
	void OnCaptionInit(int param);
	void OnInfo1Init(int param);
	void OnScore0Init(int param);
	void OnScore1Init(int param);
	void OnScore2Init(int param);
	void OnScore3Init(int param);
	void OnScore4Init(int param);
	void OnScore5Init(int param);
	void OnScore0OwnerDraw();
	void OnScore1OwnerDraw();
	void OnScore2OwnerDraw();
	void OnScore3OwnerDraw();
	void OnScore4OwnerDraw();
	void OnScore5OwnerDraw();

	FrArea* m_pScore[6];
	unsigned char m_pad983_128[8];	// +0x128: sem leitura/escrita no 983 (00b0a600..00b0b000; new 0x130)

	DECLARE_FRESH_MSGMAP()
};
