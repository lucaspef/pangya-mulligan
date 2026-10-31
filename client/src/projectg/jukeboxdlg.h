#pragma once

#include "frform.h"
#include "soundmanager.h"
#include <vector>

class FrButton;
class FrListBox;
class FrWnd;
struct FrListItem;
class WOverlay;
class Bitmap;

// 983: faixa de BGM da BGMlist.lua (par {arquivo, titulo} de 8 bytes; vetor em 00e7b150)
struct sBGMInfo983
{
	const char* file;
	const char* title;
};

class FrJukeBoxDlg : public FrForm
{
	DECLARE_OBJECT(FrJukeBoxDlg)

public:
	FrJukeBoxDlg(); // 007ba760
	virtual ~FrJukeBoxDlg(); // 007ba400

	static void LoadBGMList(); // 007bbab0: BGMlist.lua (max_count_bgm_list.max_count, bgm_list); so se o vetor estiver vazio
	static void __fastcall PlayWithMap(unsigned char map); // 007bbce0
	static void SelectBGMByName(const char* name); // 007ba810
	void OnChangePlayPauseButton(); // 007bac60 (chamada de fora)

	// 645 (avatar_main/avatar_mainui): o 983 nao tem estas tres; ficam inline para nao pesar na unidade
	static void Init()
	{
		LoadBGMList();
		m_curBGM = rand() % m_maxCount;
	}
	static void Preserve()
	{
		m_preserve_isPlaying = g_audio->IsPlaying(m_bgmList[m_curBGM].file);
		g_audio->StopBGM(true);
	}
	static void Restore()
	{
		g_audio->StopBGM(true);
		if (m_preserve_isPlaying)
			g_audio->PlayBGM(m_bgmList[m_curBGM].file, false, true);
	}

	static bool m_preserve_isPlaying;

protected:
	virtual bool OnInit(); // 007bc1e0
	virtual void OnProc(const float delta); // 007bb4f0

	void OnBtnInit(int param); // 007ba880
	void OnBtnUp(); // 007ba4c0
	void OnPlayInit(int param); // 007ba930
	void OnPlayBtnUp(); // 007bb1b0
	void OnCloseBtnUp(); // 007ba4b0
	void OnPrevInit(int param); // 007baa50
	void OnPrevBtnUp(); // 007baae0
	void OnNextInit(int param); // 007ba9c0
	void OnNextBtnUp(); // 007bab90
	void OnListInit(int param); // 007bacf0
	void OnListOwnerDraw(int param); // 007bb290
	void OnListBtnUp(); // 007bb6a0
	void OnFrameInit(int param); // 007bafd0
	void OnFrameOwnerDraw(int param); // 007ba530

	void LoadBitmaps(); // 007ba4d0
	void InitLightRect(); // 007bad60
	void DrawPlayingBar(FrListItem* item); // 007badd0
	void DrawHoverBar(FrListItem* item); // 007baed0
	static void ShufflePlay(); // 007bb400

	static int m_maxCount; // 00e7b13c
	static int m_curBGM; // 00e7b140
	static std::vector<sBGMInfo983> m_bgmList; // 00e7b150

	FrListBox* m_pList; // +0x110
	float m_fTime; // +0x114: gira a luz enquanto toca
	char m_reserved118[0x10]; // +0x118: sem acesso no 983 (007ba3a0..007bc600, ctor 007ba760 nao zera)
	WRect m_lightRect; // +0x128
	const Bitmap* m_pBar[4]; // +0x138: Push_1, Push_2, Hover_1, Hover_2
	char m_reserved148[8]; // +0x148: sem acesso no 983
	bool m_bAnim; // +0x150
	WOverlay* m_pLight; // +0x154
	FrWnd* m_pFrame; // +0x158
	FrButton* m_pBtn; // +0x15c
	FrButton* m_pPlay; // +0x160
	FrButton* m_pNext; // +0x164
	FrButton* m_pPrev; // +0x168
	char m_reserved16c[4]; // +0x16c: sem acesso no 983 (fim do objeto, sizeof 0x170)

	DECLARE_FRESH_MSGMAP()
};
