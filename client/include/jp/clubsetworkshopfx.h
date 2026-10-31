#pragma once
// JP 983: objeto sem RTTI da oficina de tacos (unidade 0049c120–0049da20, "semclasse" no mapa da F3.7.1; strings
// fx_*_filename/sound_*: os efeitos e os sons). Sem tabela virtual; s_pInstance 00e74284; sizeof 0x4c pelo `operator new` de
// 0049d160. A FrClubSetWorkshopMainDlg cria (+0x150), inicia, chama a cada quadro e apaga. B87: corpo inteiro em
// src/projectg/jp/clubsetworkshopfx.cpp; nome da classe e dos métodos são meus.

#include <string>
#include <vector>
#include <array>
struct WRect;

class Bitmap;
class ClubSetWorkshopHelper;
class CPvsFxBox;
class CFxSpray;
class CFxSequence;

// Animação por troca de figuras do Helper (B82: seta da OptionalFunctions em +0x114; métodos na unidade do Fx). Sem construtor
// fora de linha (o da OptionalFunctions zera os campos). 0x1c.
struct sOverlayAnim983
{
	sOverlayAnim983() : interval(0), time(0), cur(0) { }
	void Proc983(float deltaTime, ClubSetWorkshopHelper* pHelper); // 0049e030: liga só a figura da vez
	void SetUse983(bool bUse, ClubSetWorkshopHelper* pHelper); // 0049e0d0 (B85): todas as figuras
	float interval; // +0
	float time; // +4
	std::vector<int> frames; // +8: índices no Helper
	int cur; // +0x18
};

// Um efeito da oficina (B87): caixa de efeitos própria desenhada numa área da tela com câmera fixa. 0x6c (new em 0049d7e0).
struct sWorkshopFx983
{
	sWorkshopFx983(const char* filename); // 0049d5e0: .seq ou .spr pela extensão
	~sWorkshopFx983(); // 0049d400
	void Process983(const float& deltaTime); // 0049cc10 (segundos → milissegundos)
	void Stop983(); // 0049cc60
	bool IsEnd983(); // 0049cc80
	void SetRect983(const WRect& rc); // 0049cc90
	void SetRectPtr983(const WRect* pRect); // 0049cf50 (só o salto)
	void Draw983(); // 0049cf60
	void Play983(); // 0049d210: carrega e põe no centro da área

	CPvsFxBox* m_pFxBox; // +0 (new 0x440)
	WMatrix m_camera; // +4
	CFxSpray* m_pSpray; // +0x34
	CFxSequence* m_pSeq; // +0x38
	WRect m_rc; // +0x3c
	std::string m_name; // +0x4c
	bool m_bSpr; // +0x68
};

class ClubSetWorkshopFx983
{
public:
	static ClubSetWorkshopFx983* s_pInstance; // 00e74284
	static ClubSetWorkshopFx983* Create983(); // 0049d160: new ClubSetWorkshopFx983 (0x4c)

	ClubSetWorkshopFx983(); // 0049cd90
	~ClubSetWorkshopFx983(); // 0049d580 (não virtual)
	void Init983(); // 0049da00 (repassa a 0049d7e0)
	void Load983(); // 0049d7e0: sons e efeitos de clubset_workshop_ui_main
	void Proc983(const float& deltaTime); // 0049ce40
	void ResetSlots983(); // 0049cbb0: limpa os 5 trabalhos; efeito da vez = nenhum
	void DeleteSlots983(); // 0049cf10
	bool SetSlotClubM983(int slot, unsigned long guid); // 0049cbe0
	void PlaySound983(unsigned int index); // 0049ce70 (o "StartFx983" da B78 toca o som da vez)
	void StopFxM983(unsigned int index); // 0049cec0
	bool IsFxEndM983(unsigned int index); // 0049cee0
	void SetFxRectM983(unsigned int index, const WRect* pRect); // 0049d1d0
	void DrawFxM983(unsigned int index); // 0049d1f0
	void PlayFxM983(unsigned int index); // 0049d3c0 (o "HoldFx983" da B83)

	// trabalho em andamento de cada aba (0 = subir nível, …; B79, campos pelo uso na LevelUp)
	struct sSlot983
	{
		unsigned long guid; // +0: guid do taco (!= 0 = em uso; SetClub983)
		int tid; // +4: tid do taco (sItemInfo::tid)
		std::string name; // +8: nome do taco (B82: dica dos botões da OptionalFunctions)
		Bitmap* pBmp; // +0x24: figura do taco (posta no aviso de confirmação)
		short stat[5]; // +0x28: atributos (GetSlotStat983; B84)
		short statMax[5]; // +0x32: teto do grau atual (GetStatMax983 b=0; GetClubRankBonus983 de rank)
		short nextStatMax[5]; // +0x3c: teto do grau seguinte (b=1; rank+1)
		int clubType; // +0x48 (B84: −1 = nenhum; nome pelo ClubSetTypeName983 004a5310)
		int level; // +0x4c
		int rank; // +0x50
		int typeId; // +0x54 (GetSlotTypeId983)
		int workshopParam; // +0x58: sClubSet::nWorkshopParam983 (Recalc983; -1 = nenhum)
		int exp; // +0x5c (GetSlotExp983)
		int recoveryLeft; // +0x60: vezes que o resultado ainda pode ser cancelado (0 = não pode; recoveryMax - sItemInfo::recoveryUsed983)
		int recoveryMax; // +0x64: sClubSet::nRecoveryMax983 (B82: a "recuperar" devolve recoveryLeft a ele)
		sSlot983* pPreview; // +0x68: prévia criada pela GetPreview983 (new 0x6c)
		sSlot983(); // 004986c0 (B87; unidade anterior, ainda sem corpo)
		~sSlot983(); // 00498460 (B87; idem)
		bool SetClub983(unsigned long guid); // 00498610 (B87; idem; 0 limpa)
		sSlot983* GetPreview983(int st, bool b); // 00498720
		void Clear983(); // 00498550 (zera tudo e apaga a previa pPreview)
		void Recalc983(sItemInfo* pItem, IFF_STRUCT::sClubSet* pClub); // 00498100 (slots, grau e tetos)
		int GetStatMax983(int stat, bool b); // 00497e70 (B84: chamada direta pela FrClubSetInfoWnd)
		int GetStat983(int i) { return i != -1 ? stat[i] : 0; } // embutida (B84: o `cmp edi, -1` da 004bded0)
	};
	// livres __fastcall depois dos envios da oficina (004ce4c0…, unidade da realmyroommain; nomes meus)
	static bool __fastcall SetSlotClub983(int slot, unsigned long guid); // 004ce4c0: repassa a 0049cbe0
	static sSlot983* __fastcall GetSlot983(int slot); // 004ce4e0
	static int __fastcall GetSlotLevel983(int slot); // 004ce520 (-1 sem trabalho)
	static int __fastcall GetSlotRank983(int slot); // 004ce540 (-1 sem trabalho)
	// B81 (usados pela RankUp)
	static int __fastcall GetSlotStat983(int slot, int stat); // 004ce580: short em +0x28 + 2·stat
	static int __fastcall GetSlotStatMax983(int slot, int stat, bool b); // 004ce5a0 (repassa a 00497e70)
	static int __fastcall GetSlotExp983(int slot); // 004ce5d0: +0x5c (0 sem trabalho)
	static int __fastcall GetSlotTypeId983(int slot); // 004ce5e0: +0x54 (-1 sem trabalho)
	static bool __fastcall ResetSlot983(int slot); // 004ce670 (00498550 + 00498610(0))
	// B82 (usados pela OptionalFunctions)
	static int __fastcall GetSlotCancelCount983(int slot); // 004ce560: +0x60 (0 sem trabalho)
	static int __fastcall GetSlotCancelMax983(int slot); // 004ce570: +0x64 (0 sem trabalho)
	static bool __fastcall ClearSlot983(int slot); // 004ce650 (00498610(0))
	static bool __fastcall StartFx983(int index); // 004ce6a0 (repassa a 0049ce70)
	static bool __fastcall SetFxRect983(int index, const WRect* pRect); // 004ce6c0 (repassa a 0049d1d0; B83: área do efeito)
	static bool __fastcall HoldFx983(int index); // 004ce6e0 (repassa a 0049d3c0; a cada quadro enquanto o aviso espera)
	static bool __fastcall DrawFx983(int index); // 004ce700 (repassa a 0049d1f0)
	static bool __fastcall StopFx983(int index); // 004ce720 (repassa a 0049cec0)
	static bool __fastcall IsFxEnd983(int index); // 004ce740 (B85; repassa a 0049cee0 e devolve sempre false)
	// B84 (usados pela FrClubSetInfoWnd)
	static int __fastcall GetSlotField58_983(int slot); // 004ce600: +0x58 (−1 sem trabalho; índice da seta do rank máximo)
	static sSlot983* __fastcall GetSlotPreview983(int slot, int stat, bool b); // 004ce620: prévia com o atributo escolhido (repassa a 00498720 do slot)

	sSlot983* m_slot[5]; // +0
	std::tr1::array<const char*, 8> m_sound; // +0x14: nomes dos sons (Load983)
	std::tr1::array<sWorkshopFx983*, 5> m_fx; // +0x34: subir nível, subir rank, trocar taco, mover experiência, recuperar
	int m_cur; // +0x48: efeito da vez (5 = nenhum)
};
