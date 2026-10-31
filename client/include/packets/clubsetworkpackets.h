// clubsetworkpackets.h - frente C (pacotes). JP 983: envios da oficina de tacos (ClubSetWork*, fila F3.7.9 da frente B).
// No 983 sao 7 funcoes livres __fastcall na unidade realmyroommain.cpp (004ce120-004ce4b0, depois dos moveis); os corpos
// estao no fim de src/projectg/realmyroommain.cpp. Quem chama sao os resultados de dialogo das classes novas da oficina
// (todas "vao_novo": sem par no KR 645). Os nomes sao nossos; ids e campos medidos no exe 983.
#pragma once

// 0x164: 004ce120, chamada por 004ec4f0 (resultado do "btn_confirm_levelup", dialogo aberto em 004eaf20) com
// (this+0xb4, this+0xbc, this+0xc0). Corpo: E4(b) E2(c) E4(a).
void __fastcall SendClubSetLevelUp983(unsigned long a, unsigned long b, unsigned short c);
// 0x165 (bSim) / 0x166 (nao), sem corpo: 004ce1b0, chamada por 004a1e20 (true) e 004a1e50 (false), resultados dos
// dialogos "clubsetwork_okcancel" de 004a1950/004a1b60 (imagem "clubmade_rank%d_normal").
void __fastcall SendClubSetWork165_983(bool bSim);
// 0x167: 004ce220, chamada por 004c7250 (resultado do "btn_confirm_rankup", dialogo de 004c7000). Mesmo corpo do 0x164.
void __fastcall SendClubSetRankUp983(unsigned long a, unsigned long b, unsigned short c);
// 0x168 (bSim) / 0x169 (nao), sem corpo: 004ce2b0, chamada por 004a1e80 (true) e pelo vizinho (false).
void __fastcall SendClubSetWork168_983(bool bSim);
// 0x16b: 004ce320, chamada por 004c3b60 (dialogo de 004c1140) com (this+0xa8, this+0xac). Corpo: E4(b) E4(a).
void __fastcall SendClubSetWork16b_983(unsigned long a, unsigned long b);
// 0x16c: 004ce3a0, chamada por 004c3af0 (dialogo de 004c0fc0) com (this+0x88, this+0x8c, this+0x90 = typeid do item,
// this+0x94). Corpo: E4(typeId) E4(a) E4(b) E4(d).
void __fastcall SendClubSetWork16c_983(unsigned long a, unsigned long b, unsigned long typeId, unsigned long d);
// 0x16d: 004ce440, chamada por 00499ac0 (resultado do dialogo de 0049c380, "newRandomBox2") com (this+0xc, this+8).
// Corpo: E4(b) E4(a).
void __fastcall SendClubSetRandomBox983(unsigned long a, unsigned long b);
