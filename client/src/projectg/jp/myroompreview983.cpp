// A261 (ponto 4 do dono, A260.1): previa 3D do My Room do 983 (CRealMyRoomMain, form "realmyroom", 0xae0) como funcoes livres
// sobre o form cru, para o CRealMyRoomForm983 (jp/myroomwidget.cpp, arquivo da B) chamar sem a base CHandOwner (+0x120).
// Campos do form (offsets do 983, o pad do nosso form ja cobre): +0x150 janela da previa (00604ed0), +0x184/+0x188/+0x18c
// middle_ui.skin_pet/skin_pet2/pet, +0x23c, +0x530 sub-aba, +0x584 previa (CExhibition), +0x58c ocupado.
// Diferenca de proposito: o 983 roda o ShowPet na thread do CAnotherHand (00605640 -> Work(form+0x120) -> OnAnotherHand 00611f50)
// sob a CRITICAL_SECTION +0x5b8; aqui roda na hora, na thread do jogo (sem lock: nada mais toca a +0x584).
#include "stdafx.h"
#include "exhibition.h"
#include "shareddoc.h"
#include "parttidlist.h"
#include "frwnd.h"
#include "rankingdlg.h"
#include "user_info.h"
#include "taskform.h"

extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
extern int g_lobbyModal983; // 00e78cac (jp/rflobbydesktop.cpp): 1 = sala 3D, 0 = TOPPAGE

#define MRF(type, off) (*(type*)(form + (off)))
// campo ja membro do CRealMyRoomForm983 (A862): offset do 983 relativo ao fim do FrTaskForm (0x120 no VC10; no cl novo cai junto)
#define MRF_OFS983(off) (sizeof(FrTaskForm) + ((off) - 0x120))

// 00605530(param): apaga a previa (+0x584, vtbl[0](1)); o 983 antes espera o CAnotherHand (WaitUntilWorkDone 9999)
void MyRoomPetClear983(char* form)
{
	CExhibition*& pv = MRF(CExhibition*, MRF_OFS983(0x584));
	if (pv)
	{
		delete pv;
		pv = NULL;
	}
}

// 00604ed0: janela da previa pela sub-aba (+0x530): 0xe-0x10 skin_pet, 0x11/0x23 skin_pet2, resto pet
void MyRoomPetSelectWnd983(char* form)
{
	int sub = MRF(int, MRF_OFS983(0x530));
	if (sub >= 0xe && sub <= 0x10)
		MRF(FrWnd*, MRF_OFS983(0x150)) = MRF(FrWnd*, MRF_OFS983(0x184));
	else if (sub == 0x11 || sub == 0x23)
		MRF(FrWnd*, MRF_OFS983(0x150)) = MRF(FrWnd*, MRF_OFS983(0x188));
	else
		MRF(FrWnd*, MRF_OFS983(0x150)) = MRF(FrWnd*, MRF_OFS983(0x18c));
}

// 0060e8b0 CRealMyRoomMain::ShowPet(tid): cria a previa do item na janela +0x150 (constantes do 983: double 0.5 = 00d66ca0,
// 15.0 = 00d04f88, 3.0 = 00d66cf8; floats 0.35/16/20/18/8/12/10/7/6/5/4/9/13/11/15). Personagem (tid>>26 == 1) fora das
// sub-abas de skin so com g_lobbyModal983 (fora dele o 983 so mostra o desenho UCC, +0x228 -> +0x110: NAO portado).
void MyRoomShowPet983(char* form, unsigned long tid)
{
	if (tid == 0)
		return;
	MRF(char, MRF_OFS983(0x58c)) = 1;
	FrWnd* wnd = MRF(FrWnd*, MRF_OFS983(0x150));
	if (wnd)
	{
		MyRoomPetClear983(form);
		WRect rc;
		wnd->GetClientRect(rc);
		wnd->ClientToScreen(rc);
		const float x = rc.x, y = rc.y, w = rc.w, h = rc.h;
		const float cx = (float)(0.5 * w + x), cy = (float)(y + h * 0.5);
		CSharedDoc* doc = CSharedDoc::Instance();
		CExhibition* p = NULL;
		bool ok = false;
		int sub = MRF(int, MRF_OFS983(0x530));
		bool skin = sub == 0xe || sub == 0xf || sub == 0x10;
		switch (tid >> 26)
		{
		case 1:
			if (skin)
			{
				p = new CExhibitionBySoftwareTnL(cx, cy, 0.35f, true, 1); // 00604750: uid = 1
				p->SetArea(w, h);
				p->SetDistanceRange(16.0f, 20.0f);
				p->SetDistance(18.0f, true, false);
			}
			else if (g_lobbyModal983)
			{
				p = new CExhibitionBySoftwareTnL(cx, (float)(y + h * 0.5 + 15.0), 0.35f, true, 1);
				p->SetArea(w, h);
				p->SetDistanceRange(8.0f, 12.0f);
				p->SetDistance(10.0f, true, false);
			}
			else
				break; // UCC (00605e70): nao portado
			ok = p->SetModel(doc->GetMyPartTidList(), doc->GetMyAuxPartTidList(), NULL, 3.0f, 0);
			break;
		case 4:
			p = new CExhibitionByHardwareTnL(cx, (float)(h / 3.0 + y), 0.0f, true);
			p->SetArea(w, h);
			p->SetDistance(7.0f, true, false);
			ok = p->SetModel(tid);
			break;
		case 5:
			p = new CExhibitionByHardwareTnL(cx, cy, 0.0f, true);
			p->SetArea(w, h);
			p->SetDistance(1.0f, true, false);
			ok = p->SetModel(tid);
			break;
		case 6:
			p = new CExhibitionByHardwareTnL(cx, cy, 0.0f, true);
			p->SetArea(w, h);
			p->SetDistance(6.0f, true, false);
			p->SetDistanceRange(5.0f, 7.0f);
			ok = p->SetModel(tid);
			break;
		case 7:
		case 8:
			if (!g_lobbyModal983)
				break; // TOPPAGE: o caddie vai pelo "LobbyCaddie" (0061c960: 00643f40 + msg 0x1a9)
			p = new CExhibitionByHardwareTnL(cx, (float)(y + h * 0.5 + 15.0), 0.35f, true);
			p->SetArea(w, h);
			p->SetDistanceRange(9.0f, 13.0f);
			p->SetDistance(11.0f, true, false);
			ok = p->SetModel(tid);
			break;
		case 0xb:
			p = new CExhibitionByHardwareTnL(cx, cy, 0.0f, true);
			p->SetArea(w, h);
			p->SetDistance(18.0f, true, false);
			ok = p->SetModel(tid);
			break;
		case 0x10:
			p = new CExhibitionByHardwareTnL(cx, (float)(y + h * 0.5 + 15.0), 0.35f, true);
			p->SetArea(w, h);
			p->SetDistanceRange(4.0f, 8.0f);
			p->SetDistance(6.0f, true, false);
			ok = p->SetModel(tid);
			break;
		case 0x12:
			if (!IsLocalContent(S4_REAL_MYROOM))
				break;
			p = new CExhibitionByHardwareTnL(cx, cy, 0.0f, true);
			p->SetArea((float)(0.5 * w), (float)(h * 0.5));
			p->SetDistance(15.0f, true, false);
			ok = p->SetModel(tid);
			break;
		case 0x1f:
			p = new CExhibitionByHardwareTnL(cx, cy, 0.0f, true);
			p->SetArea(w, h);
			p->SetDistanceRange(5.0f, 7.0f);
			p->SetDistance((tid & 0x3c00000) == 0xc00000 ? 5.0f : 6.0f, true, false);
			ok = p->SetModel(tid);
			break;
		}
		jp983_rastro("D315 ShowPet tid=%08x sub=%d wnd=%p p=%p ok=%d rc=%.0f,%.0f %.0fx%.0f modal=%d", tid, sub, wnd, p, (int)ok, x, y, w, h, g_lobbyModal983);
		if (p && !ok)
			delete p;
		else if (p)
		{
			p->SetControlFlag(0x1f); // +0x564 = 0x1f
			MRF(CExhibition*, MRF_OFS983(0x584)) = p;
		}
	}
	MRF(char, MRF_OFS983(0x58c)) = 0;
}

// 00605640(form, &tid, force): refaz a previa se nao ha nenhuma ou se e de outro tid (CExhibition::m_tid, +0x55c)
void MyRoomPreview983(char* form, unsigned long tid, bool force)
{
	CExhibition* pv = MRF(CExhibition*, MRF_OFS983(0x584));
	if (force || !pv || pv->m_tid != tid)
		MyRoomShowPet983(form, tid);
}

// 0060d9b0 (fim do OWNERDRAW 0061f6d0 de "middle_ui.pet", fora da aba principal 4 e sem desenho UCC): Display(0, 0) da previa.
// B1236: portoes 00e73f04 (CRankingInfo) / 00e74e14 (CUserInfo) +0x2c: com o Ranking ou o Info do jogador abertos
// o 983 nao desenha a previa (personagem por cima da janela). NAO portada a fila +0x598/+0x59c (ShowPet aqui e sincrono).
void MyRoomPetDraw983(char* form)
{
	static int s_trace; // C729: previa some so no VS 2022
	if (s_trace < 3 && ++s_trace)
		jp983_rastro("C729 pet shop=%d rank=%p info=%p work=%p pv=%p wnd=%p", (int)CSharedDoc::Instance()->m_bBongdariShopOpen, CRankingInfo::Instance()->GetDlg(),
			CUserInfo::Instance() ? (void*)CUserInfo::Instance()->GetDlg() : NULL, MRF(void*, MRF_OFS983(0x23c)), MRF(void*, MRF_OFS983(0x584)), MRF(void*, MRF_OFS983(0x150)));
	if (CSharedDoc::Instance()->m_bBongdariShopOpen || CRankingInfo::Instance()->GetDlg() != NULL ||
		(CUserInfo::Instance() != NULL && CUserInfo::Instance()->GetDlg() != NULL) || MRF(int, MRF_OFS983(0x23c)))
		return;
	MRF(char, MRF_OFS983(0x58c)) = 1;
	if (CExhibition* pv = MRF(CExhibition*, MRF_OFS983(0x584)))
		pv->Display(0.0f, 0.0f);
	MRF(char, MRF_OFS983(0x58c)) = 0;
}
