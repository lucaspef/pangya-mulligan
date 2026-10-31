// JP 983: unidade 004ce760–004d1050 (CreateCharacterOption, as tabelas fixas por personagem; ver
// include/jp/createcharacteroption.h). Sessão B124.
#include "minatl.h"
#include "projectg.h"
#include "wresrcmng.h"
#include "wlocalize.h"
#include "commonutil.h"
#include "createcharacteroption.h"
#include "../../shared/globalgamedefine.h" // D788: sCharacterInfo (FindEquippedSetCommand983)
#include <stdlib.h>
#include <string.h>

// 004ce760
bool CreateCharacterOption::IsPartGroupA983(unsigned long tid)
{
	switch (tid)
	{
	case 0x08016800: case 0x08058800: case 0x08098800: case 0x080dc800: case 0x08118800: case 0x08160800:
	case 0x08190800: case 0x081e2800: case 0x08214800: case 0x08254800: case 0x082d480e: case 0x08314800:
	case 0x0839c800:
		return true;
	}
	return false;
}

// 004ce7e0
bool CreateCharacterOption::IsPartGroupB983(unsigned long tid)
{
	switch (tid)
	{
	case 0x0801680f: case 0x0805880f: case 0x0809880f: case 0x080dc80f: case 0x0811880f: case 0x0816080f:
	case 0x0819080f: case 0x081e280f: case 0x0821480f: case 0x0825480f: case 0x08294811: case 0x082d4804:
	case 0x0831480c: case 0x0839c80c:
		return true;
	}
	return false;
}

// 004ce860
bool CreateCharacterOption::IsPartGroupC983(unsigned long tid)
{
	switch (tid)
	{
	case 0x08016801: case 0x08058801: case 0x08098801: case 0x080dc801: case 0x08118801: case 0x08160801:
	case 0x08190801: case 0x081e2801: case 0x08214801: case 0x08254801: case 0x082d480d: case 0x08314801:
	case 0x0839c801:
		return true;
	}
	return false;
}

// 004ce8e0
bool CreateCharacterOption::IsPartGroupD983(unsigned long tid)
{
	switch (tid)
	{
	case 0x08016810: case 0x08058810: case 0x08098810: case 0x080dc810: case 0x08118810: case 0x08160810:
	case 0x08190810: case 0x081e2810: case 0x08214810: case 0x08254810: case 0x08294812: case 0x082d4803:
	case 0x0831480d: case 0x0839c80d:
		return true;
	}
	return false;
}

// 004ce960
void CreateCharacterOption::LoadPortraits983(Bitmap** out)
{
	out[0] = g_resrcmng->LoadBitmap("m_def.tga", 0, false);
	out[1] = g_resrcmng->LoadBitmap("f_def.tga", 0, false);
	out[2] = g_resrcmng->LoadBitmap("a_def.tga", 0, false);
	out[3] = g_resrcmng->LoadBitmap("c_def.tga", 0, false);
	out[4] = g_resrcmng->LoadBitmap("d_def.tga", 0, false);
	out[5] = g_resrcmng->LoadBitmap("e_def.tga", 0, false);
	out[6] = g_resrcmng->LoadBitmap("g_def.tga", 0, false);
	out[7] = g_resrcmng->LoadBitmap("h_def.tga", 0, false);
	out[8] = g_resrcmng->LoadBitmap("i_def.tga", 0, false);
	out[9] = g_resrcmng->LoadBitmap("j_def.tga", 0, false);
	out[10] = g_resrcmng->LoadBitmap("k_def.tga", 0, false);
	out[11] = g_resrcmng->LoadBitmap("mm_def.tga", 0, false);
	out[12] = g_resrcmng->LoadBitmap("ff_def.tga", 0, false);
	out[13] = g_resrcmng->LoadBitmap("a_def.tga", 0, false);
	out[14] = g_resrcmng->LoadBitmap("cc_def.tga", 0, false);
}

// 004cead0 (o índice 13 fica sem valor)
void CreateCharacterOption::InitBarIcons983()
{
	m_barIcons[0] = "bar_m_def";
	m_barIcons[1] = "bar_f_def";
	m_barIcons[2] = "bar_a_def";
	m_barIcons[3] = "bar_c_def";
	m_barIcons[4] = "bar_d_def";
	m_barIcons[5] = "bar_e_def";
	m_barIcons[6] = "bar_g_def";
	m_barIcons[7] = "bar_h_def";
	m_barIcons[8] = "bar_i_def";
	m_barIcons[9] = "bar_j_def";
	m_barIcons[10] = "bar_k_def";
	m_barIcons[11] = "bar_mm_def";
	m_barIcons[12] = "bar_ff_def";
	m_barIcons[14] = "bar_cc_def";
}

// 004ceb40
const char* CreateCharacterOption::GetBarIcon983(int charIndex)
{
	return m_barIcons[charIndex];
}

// 004ceb50 (o índice 13 fica sem valor)
void CreateCharacterOption::InitChatFacials983()
{
	m_chatFacials[0][0] = "m_skin_01-f03.png";
	m_chatFacials[0][1] = "m_skin_01-f04.png";
	m_chatFacials[0][2] = "m_skin_01-f12.png";
	m_chatFacials[0][3] = "m_skin_01-reset.png";
	m_chatFacials[1][0] = "F_Skin-f05.png";
	m_chatFacials[1][1] = "F_Skin-f01.png";
	m_chatFacials[1][2] = "F_Skin-f06.png";
	m_chatFacials[1][3] = "F_Skin-reset.png";
	m_chatFacials[2][0] = "A_Face-f02.png";
	m_chatFacials[2][1] = "A_Face-f03.png";
	m_chatFacials[2][2] = "A_Face-f05.png";
	m_chatFacials[2][3] = "A_Face-reset.png";
	m_chatFacials[3][0] = "C_Face-f03.png";
	m_chatFacials[3][1] = "C_Face-f02.png";
	m_chatFacials[3][2] = "C_Face-f04.png";
	m_chatFacials[3][3] = "C_Face-reset.png";
	m_chatFacials[4][0] = "D_Face-f07.png";
	m_chatFacials[4][1] = "D_Face-f04.png";
	m_chatFacials[4][2] = "D_Face-f05.png";
	m_chatFacials[4][3] = "D_Face-reset.png";
	m_chatFacials[5][0] = "E_def_f03.png";
	m_chatFacials[5][1] = "E_def_f05.png";
	m_chatFacials[5][2] = "E_def_f06.png";
	m_chatFacials[5][3] = "E_def_reset.png";
	m_chatFacials[6][0] = "G_Face-blink1.png";
	m_chatFacials[6][1] = "G_Face-f03.png";
	m_chatFacials[6][2] = "G_Face-f06.png";
	m_chatFacials[6][3] = "G_Face-reset.png";
	m_chatFacials[7][0] = "H_def_f4.png";
	m_chatFacials[7][1] = "H_def_f6.png";
	m_chatFacials[7][2] = "H_def_f7.png";
	m_chatFacials[7][3] = "H_Face-reset.png";
	m_chatFacials[8][0] = "I_def_f13.png";
	m_chatFacials[8][1] = "I_def_f23.png";
	m_chatFacials[8][2] = "I_def_f21.png";
	m_chatFacials[8][3] = "I_def_reset.png";
	m_chatFacials[9][0] = "J_def_blink13.png";
	m_chatFacials[9][1] = "J_def_blink23.png";
	m_chatFacials[9][2] = "J_def_blink21.png";
	m_chatFacials[9][3] = "J_def-reset.png";
	m_chatFacials[10][0] = "K_def_f24.png";
	m_chatFacials[10][1] = "K_def_f34.png";
	m_chatFacials[10][2] = "K_def_f36.png";
	m_chatFacials[10][3] = "K_def-reset.png";
	m_chatFacials[11][0] = "MM_def_f13.png";
	m_chatFacials[11][1] = "MM_def_f15.png";
	m_chatFacials[11][2] = "MM_def_f07.png";
	m_chatFacials[11][3] = "MM_def_reset.png";
	m_chatFacials[12][0] = "FF_def_f08.png";
	m_chatFacials[12][1] = "FF_def_f11.png";
	m_chatFacials[12][2] = "FF_def_f18.png";
	m_chatFacials[12][3] = "FF_def_reset.png";
	m_chatFacials[14][0] = "CC_def_f07.png";
	m_chatFacials[14][1] = "CC_def_f09.png";
	m_chatFacials[14][2] = "CC_def_f23.png";
	m_chatFacials[14][3] = "CC_def_reset.png";
}

// 004ced70
const char* CreateCharacterOption::GetChatFacial983(int charIndex, int kind)
{
	return m_chatFacials[charIndex][kind];
}

// 004ced90: nomes coreanos (Nuri, Hana, Azer, Cecilia, Max, Kooh, Arin, Kaz, Lucia, Nell, Spika, Nuri R, Hana R, Azer R,
// Cecilia R, "item", "outros")
void CreateCharacterOption::InitNames983()
{
	m_names[0] = "\xb4\xa9\xb8\xae";
	m_names[1] = "\xc7\xcf\xb3\xaa";
	m_names[2] = "\xbe\xc6\xc0\xfa";
	m_names[3] = "\xbc\xbc\xbd\xc7\xb8\xae\xbe\xc6";
	m_names[4] = "\xb8\xc6\xbd\xba";
	m_names[5] = "\xc4\xed";
	m_names[6] = "\xbe\xc6\xb8\xb0";
	m_names[7] = "\xc4\xab\xc1\xee";
	m_names[8] = "\xb7\xe7\xbd\xc3\xbe\xc6";
	m_names[9] = "\xb3\xda";
	m_names[10] = "\xbd\xba\xc7\xc7\xc4\xab";
	m_names[11] = "\xb4\xa9\xb8\xaeR";
	m_names[12] = "\xc7\xcf\xb3\xaaR";
	m_names[13] = "\xbe\xc6\xc0\xfaR";
	m_names[14] = "\xbc\xbc\xbd\xc7\xb8\xae\xbe\xc6R";
	m_names[15] = "\xbe\xc6\xc0\xcc\xc5\xdb";
	m_names[16] = "\xb1\xe2\xc5\xb8";
	m_nameCount = 17;
}

// 004cee50
const char* CreateCharacterOption::GetName983(int index)
{
	if (index >= 13)
		return m_names[index + 1];
	return m_names[index];
}

// 004cee80
void CreateCharacterOption::InitSubBones983()
{
	const char* subA = "A_WI_11_SUB"; // um literal só para Azer e Azer R
	m_subBones[0] = "M_WI_11_SUB";
	m_subBones[1] = "F_WI_11_SUB";
	m_subBones[2] = subA;
	m_subBones[3] = "C_WI_11_SUB";
	m_subBones[4] = "D_WI_11_SUB";
	m_subBones[5] = "E_WI_11_SUB";
	m_subBones[6] = "G_WI_11_SUB";
	m_subBones[7] = "H_WI_11_SUB";
	m_subBones[8] = "I_WI_11_SUB";
	m_subBones[9] = "J_WI_11_SUB";
	m_subBones[10] = "K_WI_11_SUB";
	m_subBones[11] = "MM_WI_11_SUB";
	m_subBones[12] = "FF_WI_11_SUB";
	m_subBones[13] = subA;
	m_subBones[14] = "CC_WI_11_SUB";
}

// 004cef20
const char* CreateCharacterOption::GetSubBone983(int charIndex)
{
	return m_subBones[charIndex];
}

// 004cef40 (chamador 006a6330)
void CreateCharacterOption::SetDefaultParts983(unsigned long* parts, int charIndex)
{
	switch (charIndex)
	{
	case 0:
		parts[3] = 0x08006001;
		parts[5] = 0x0800a000;
		parts[6] = 0;
		parts[7] = 0;
		parts[8] = 0x08010002;
		parts[19] = 0x08026800;
		break;
	case 1:
		parts[0] = 0x08040802;
		parts[2] = 0x08044000;
		parts[4] = 0;
		parts[5] = 0;
		parts[7] = 0x0804e001;
		parts[18] = 0x08064800;
		break;
	case 2:
		parts[0] = 0x08080801;
		parts[1] = 0x08083001;
		parts[2] = 0x08084004;
		parts[3] = 0;
		parts[4] = 0x08088801;
		parts[5] = 0x0808a004;
		parts[6] = 0;
		parts[7] = 0x0808e003;
		parts[8] = 0x08090800;
		parts[17] = 0x080a2800;
		break;
	case 3:
		parts[2] = 0x080c4003;
		parts[3] = 0;
		parts[5] = 0x080ca003;
		parts[7] = 0x080ce003;
		parts[8] = 0x080d0801;
		parts[10] = 0x080d4800;
		parts[18] = 0x080e4800;
		break;
	case 4:
		parts[2] = 0x08104002;
		parts[3] = 0x08106001;
		parts[4] = 0x08108002;
		parts[5] = 0x0810a002;
		parts[9] = 0x08112000;
		parts[10] = 0x08114000;
		parts[17] = 0x08122800;
		break;
	case 5:
		parts[23] = 0x0816e801;
		break;
	case 6:
		parts[18] = 0x081a4800;
		break;
	case 7:
		parts[21] = 0x081ea800;
		break;
	case 8:
		parts[20] = 0x08228800;
		break;
	case 9:
		parts[2] = 0x0824400b;
		parts[5] = 0;
		parts[20] = 0x08268800;
		break;
	case 10:
		parts[19] = 0x082a6800;
		break;
	case 11:
		parts[2] = 0x082c4001;
		parts[4] = 0x082c8000;
		parts[5] = 0x082ca001;
		parts[7] = 0x082ce000;
		break;
	case 12:
		parts[2] = 0x08304001;
		parts[3] = 0x08306400;
		parts[4] = 0x08308400;
		parts[5] = 0x0830a400;
		break;
	case 14:
		parts[2] = 0x08384400;
		parts[3] = 0x08386400;
		parts[4] = 0x08388400;
		parts[5] = 0x0838a400;
		break;
	}
}

// 004cf170 (chamador 00b2e950): peças sorteadas para o personagem
void CreateCharacterOption::RandomParts983(unsigned long* parts, unsigned long charTid)
{
	switch (charTid & 0x3ffffff)
	{
	case 0:
		parts[0] = rand() % 11 | 0x08000800;
		parts[3] = rand() % 20 | 0x08006000;
		parts[4] = rand() % 3 | 0x08008800;
		parts[5] = rand() % 17 | 0x0800a000;
		parts[8] = rand() % 11 | 0x08010000;
		break;
	case 1:
		parts[0] = rand() % 20 | 0x08040800;
		parts[2] = rand() % 20 | 0x08044000;
		parts[4] = rand() % 11 | 0x08048000;
		parts[7] = rand() % 10 | 0x0804e000;
		break;
	case 2:
		parts[0] = rand() % 9 | 0x08080800;
		parts[2] = (rand() % 18 + 1) | 0x08084000;
		parts[4] = rand() % 4 | 0x08088800;
		parts[5] = (rand() % 14 + 1) | 0x0808a000;
		parts[7] = rand() % 11 | 0x0808e000;
		parts[8] = rand() % 3 | 0x08090800;
		break;
	case 3:
		parts[0] = rand() % 7 | 0x080c0000;
		parts[2] = rand() % 22 | 0x080c4000;
		parts[4] = rand() % 5 | 0x080c8000;
		parts[5] = rand() % 15 | 0x080ca000;
		parts[7] = rand() % 12 | 0x080ce000;
		parts[10] = rand() % 4 | 0x080d4800;
		break;
	case 4:
		parts[0] = rand() % 6 | 0x08100000;
		parts[2] = rand() % 17 | 0x08104000;
		parts[3] = rand() % 8 | 0x08106000;
		parts[4] = rand() % 12 | 0x08108000;
		parts[5] = rand() % 10 | 0x0810a000;
		parts[9] = rand() % 4 | 0x08112000;
		parts[10] = rand() % 2 | 0x08114000;
		parts[11] = rand() % 2 | 0x08116000;
		break;
	case 5:
		parts[0] = rand() % 6 | 0x08140000;
		parts[2] = rand() % 12 | 0x08144000;
		parts[5] = rand() % 13 | 0x0814a000;
		parts[7] = rand() % 15 | 0x0814e000;
		break;
	case 6:
		parts[0] = rand() % 10 | 0x08180000;
		parts[2] = rand() % 15 | 0x08184000;
		parts[4] = rand() % 8 | 0x08188000;
		parts[5] = rand() % 9 | 0x0818a000;
		break;
	case 8:
		parts[0] = 0x08200000;
		parts[2] = 0x08204018;
		parts[4] = 0x08208000;
		parts[5] = 0x0820a000;
		parts[7] = 0x0820e000;
		break;
	case 9:
		parts[0] = 0x0824001e;
		parts[2] = 0x08244000;
		parts[4] = 0x08248003;
		parts[5] = 0x0824a000;
		parts[7] = 0x0824e007;
		break;
	case 10:
		parts[0] = 0x08280400;
		parts[2] = 0x08284400;
		parts[4] = 0x08288400;
		parts[5] = 0x0828a400;
		parts[7] = 0x0828e400;
		break;
	case 11:
		parts[0] = 0x082c0400;
		parts[2] = 0x082c4400;
		parts[4] = 0x082c8400;
		parts[5] = 0x082ca400;
		parts[7] = 0x082ce400;
		break;
	case 12:
		parts[0] = 0x08300400;
		parts[2] = 0x08304400;
		parts[4] = 0x08308400;
		parts[5] = 0x0830a400;
		parts[6] = 0x0830c400;
		break;
	case 14:
		parts[0] = 0x08380400;
		parts[2] = 0x08384400;
		parts[4] = 0x08388400;
		parts[5] = 0x0838a400;
		parts[6] = 0x0838c400;
		break;
	}
}

// 004cf610 (BuildDesc 00854620): nome do personagem traduzido
void CreateCharacterOption::GetCharName983(char* out, int charIndex)
{
	if (!out)
		return;
	switch (charIndex)
	{
	case 0: strcpy(out, K2L_Compatibility("\xb4\xa9\xb8\xae")); break;
	case 1: strcpy(out, K2L_Compatibility("\xc7\xcf\xb3\xaa")); break;
	case 2: strcpy(out, K2L_Compatibility("\xbe\xc6\xc0\xfa")); break;
	case 3: strcpy(out, K2L_Compatibility("\xbc\xbc\xbd\xc7")); break;
	case 4: strcpy(out, K2L_Compatibility("\xb8\xc6\xbd\xba")); break;
	case 5: strcpy(out, K2L_Compatibility("\xc4\xed")); break;
	case 6: strcpy(out, K2L_Compatibility("\xbe\xc6\xb8\xb0")); break;
	case 7: strcpy(out, K2L_Compatibility("\xc4\xab\xc1\xee")); break;
	case 8: strcpy(out, K2L_Compatibility("\xb7\xe7\xbd\xc3\xbe\xc6")); break;
	case 9: strcpy(out, K2L_Compatibility("\xb3\xda")); break;
	case 10: strcpy(out, K2L_Compatibility("\xbd\xba\xc7\xc7\xc4\xab")); break;
	case 11: strcpy(out, K2L_Compatibility("\xb4\xa9\xb8\xae")); break;
	case 12: strcpy(out, K2L_Compatibility("\xc7\xcf\xb3\xaa")); break;
	case 14: strcpy(out, K2L_Compatibility("\xbc\xbc\xbd\xc7")); break;
	}
}

#define SET_PART983(c, i, t, k) m_setParts[c][i].Set(t, k, m_setMotions[k])

// 004cf830: comandos de conjunto do bate-papo (nalgi = voar, rideum = ritmo, SP1, SP2) e as peças que os liberam
void CreateCharacterOption::InitSetParts983()
{
	m_setMotions[0].command = K2L_Compatibility("\xb3\xaf\xb1\xe2");
	m_setMotions[0].motion = K2L_Compatibility("chat_\xb3\xaf\xb1\xe2");
	m_setMotions[1].command = K2L_Compatibility("\xb8\xae\xb5\xeb");
	m_setMotions[1].motion = K2L_Compatibility("chat_\xbe\xc6\xc0\xcc\xc5\xdb\xb4\xed\xbd\xba");
	m_setMotions[2].command = K2L_Compatibility("\xb3\xaf\xb1\xe2");
	m_setMotions[2].motion = K2L_Compatibility("chat_\xb3\xaf\xb1\xe2" "2");
	m_setMotions[3].command = K2L_Compatibility("SP1");
	m_setMotions[3].motion = K2L_Compatibility("chat_sp1");
	m_setMotions[4].command = K2L_Compatibility("SP2");
	m_setMotions[4].motion = K2L_Compatibility("chat_sp2");

	SET_PART983(0, 0, 0x0801680a, 0);
	SET_PART983(0, 1, 0x0801680b, 0);
	SET_PART983(0, 2, 0x0801680c, 0);
	SET_PART983(0, 3, 0x0801680e, 0);
	SET_PART983(0, 4, 0x08022817, 1);
	SET_PART983(0, 5, 0x0801680f, 2);
	SET_PART983(0, 6, 0x08016810, 2);
	SET_PART983(0, 7, 0x08016815, 2);
	SET_PART983(0, 8, 0x08016816, 2);
	SET_PART983(1, 0, 0x0805880a, 0);
	SET_PART983(1, 1, 0x0805880b, 0);
	SET_PART983(1, 2, 0x0805880c, 0);
	SET_PART983(1, 3, 0x0805880e, 0);
	SET_PART983(1, 4, 0x08062817, 1);
	SET_PART983(1, 5, 0x0805880f, 2);
	SET_PART983(1, 6, 0x08058810, 2);
	SET_PART983(1, 7, 0x08058815, 2);
	SET_PART983(1, 8, 0x08058816, 2);
	SET_PART983(2, 0, 0x0809880a, 0);
	SET_PART983(2, 1, 0x0809880b, 0);
	SET_PART983(2, 2, 0x0809880c, 0);
	SET_PART983(2, 3, 0x0809880e, 0);
	SET_PART983(2, 4, 0x080a6817, 1);
	SET_PART983(2, 5, 0x0809880f, 2);
	SET_PART983(2, 6, 0x08098810, 2);
	SET_PART983(2, 7, 0x08098815, 2);
	SET_PART983(2, 8, 0x08098816, 2);
	SET_PART983(3, 0, 0x080dc80a, 0);
	SET_PART983(3, 1, 0x080dc80b, 0);
	SET_PART983(3, 2, 0x080dc80c, 0);
	SET_PART983(3, 3, 0x080dc80e, 0);
	SET_PART983(3, 4, 0x080e2817, 1);
	SET_PART983(3, 5, 0x080dc80f, 2);
	SET_PART983(3, 6, 0x080dc810, 2);
	SET_PART983(3, 7, 0x080dc815, 2);
	SET_PART983(3, 8, 0x080dc816, 2);
	SET_PART983(4, 0, 0x0811880a, 0);
	SET_PART983(4, 1, 0x0811880b, 0);
	SET_PART983(4, 2, 0x0811880c, 0);
	SET_PART983(4, 3, 0x0811880e, 0);
	SET_PART983(4, 4, 0x08128817, 1);
	SET_PART983(4, 5, 0x0811880f, 2);
	SET_PART983(4, 6, 0x08118810, 2);
	SET_PART983(4, 7, 0x08118815, 2);
	SET_PART983(4, 8, 0x08118816, 2);
	SET_PART983(5, 0, 0x0816080a, 0);
	SET_PART983(5, 1, 0x0816080b, 0);
	SET_PART983(5, 2, 0x0816080c, 0);
	SET_PART983(5, 3, 0x0816080e, 0);
	SET_PART983(5, 4, 0x0816a817, 1);
	SET_PART983(5, 5, 0x0816080f, 2);
	SET_PART983(5, 6, 0x08160810, 2);
	SET_PART983(5, 7, 0x08160816, 2);
	SET_PART983(5, 8, 0x08160817, 2);
	SET_PART983(6, 0, 0x0819080a, 0);
	SET_PART983(6, 1, 0x0819080b, 0);
	SET_PART983(6, 2, 0x0819080c, 0);
	SET_PART983(6, 3, 0x0819080e, 0);
	SET_PART983(6, 4, 0x081a0817, 1);
	SET_PART983(6, 5, 0x0819080f, 2);
	SET_PART983(6, 6, 0x08190810, 2);
	SET_PART983(6, 7, 0x08190815, 2);
	SET_PART983(6, 8, 0x08190816, 2);
	SET_PART983(7, 0, 0x081e280a, 0);
	SET_PART983(7, 1, 0x081e280b, 0);
	SET_PART983(7, 2, 0x081e280c, 0);
	SET_PART983(7, 3, 0x081e280e, 0);
	SET_PART983(7, 4, 0x081e6817, 1);
	SET_PART983(7, 5, 0x081e280f, 2);
	SET_PART983(7, 6, 0x081e2810, 2);
	SET_PART983(7, 7, 0x081e2815, 2);
	SET_PART983(7, 8, 0x081e2816, 2);
	SET_PART983(8, 0, 0x0821480a, 0);
	SET_PART983(8, 1, 0x0821480b, 0);
	SET_PART983(8, 2, 0x0821480c, 0);
	SET_PART983(8, 3, 0x0821480e, 0);
	SET_PART983(8, 4, 0x08222817, 1);
	SET_PART983(8, 5, 0x0821480f, 2);
	SET_PART983(8, 6, 0x08214810, 2);
	SET_PART983(8, 7, 0x08214816, 2);
	SET_PART983(8, 8, 0x08214817, 2);
	SET_PART983(8, 9, 0x0821481c, 2);
	SET_PART983(9, 0, 0x0825480a, 0);
	SET_PART983(9, 1, 0x0825480b, 0);
	SET_PART983(9, 2, 0x0825480c, 0);
	SET_PART983(9, 3, 0x0825480e, 0);
	SET_PART983(9, 4, 0x08262817, 1);
	SET_PART983(9, 5, 0x0825480f, 2);
	SET_PART983(9, 6, 0x08254810, 2);
	SET_PART983(9, 7, 0x08254815, 2);
	SET_PART983(9, 8, 0x08254816, 2);
	SET_PART983(10, 0, 0x0829480a, 0);
	SET_PART983(10, 1, 0x0829480b, 0);
	SET_PART983(10, 2, 0x0829480c, 0);
	SET_PART983(10, 3, 0x0829480e, 0);
	SET_PART983(10, 4, 0x082a2817, 1);
	SET_PART983(10, 5, 0x08294811, 2);
	SET_PART983(10, 6, 0x08294812, 2);
	SET_PART983(10, 7, 0x08294817, 2);
	SET_PART983(10, 8, 0x08294818, 2);
	SET_PART983(11, 0, 0x082e0800, 1);
	SET_PART983(11, 1, 0x082d4804, 2);
	SET_PART983(11, 2, 0x082d4803, 2);
	SET_PART983(11, 3, 0x082d480f, 2);
	SET_PART983(11, 4, 0x082d4810, 2);
	SET_PART983(12, 0, 0x0831e80c, 1);
	SET_PART983(12, 1, 0x0831480c, 2);
	SET_PART983(12, 2, 0x0831480d, 2);
	SET_PART983(12, 3, 0x08314811, 2);
	SET_PART983(12, 4, 0x08314812, 2);
	SET_PART983(12, 5, 0x08304021, 3);
	SET_PART983(12, 6, 0x08304021, 4);
	SET_PART983(14, 0, 0x0839c80c, 2);
	SET_PART983(14, 1, 0x0839c80d, 2);
	SET_PART983(14, 2, 0x0839c811, 2);
	SET_PART983(14, 3, 0x0839c812, 2);
}

#undef SET_PART983

// 004d0b10
bool CreateCharacterOption::FindSetCommand983(const char* text, unsigned int* pKind)
{
	for (unsigned int i = 0; i < SET_MOTION_COUNT983; i++)
	{
		if (strstr(text, MakeStr("%s%s", "/", m_setMotions[i].command)))
		{
			*pKind = i;
			return true;
		}
	}
	return false;
}

// 004d0b70
bool CreateCharacterOption::FindSetMotionExact983(const char* motion, unsigned int* pKind)
{
	for (unsigned int i = 0; i < SET_MOTION_COUNT983; i++)
	{
		if (strcmp(motion, m_setMotions[i].motion) == 0)
		{
			*pKind = i;
			return true;
		}
	}
	return false;
}

// 004d0bd0
unsigned long CreateCharacterOption::FindEquipped983(unsigned long charTid, const unsigned long* parts, int kind)
{
	for (unsigned int i = 0; i < SET_PART_COUNT983; i++)
	{
		if (m_setParts[charTid & 0x3ffffff][i].kind == kind)
		{
			for (int j = 0; j < 24; j++)
			{
				if (m_setParts[charTid & 0x3ffffff][i].tid && parts[j] == m_setParts[charTid & 0x3ffffff][i].tid)
					return m_setParts[charTid & 0x3ffffff][i].tid;
			}
		}
	}
	return 0;
}

// 004d0c40
bool CreateCharacterOption::FindEquippedSetCommand983(const char* text, char* motion, unsigned int* pKind,
	const sCharacterInfo* pInfo)
{
	if (!pInfo)
		return false;
	for (unsigned int i = 0; i < SET_MOTION_COUNT983; i++)
	{
		if (strstr(text, MakeStr("%s%s", "/", m_setMotions[i].command)))
		{
			*pKind = i;
			if (FindEquipped983(pInfo->tid, pInfo->tidParts, i))
			{
				strcpy(motion, m_setMotions[i].motion);
				return true;
			}
		}
	}
	return false;
}

// 004d0ce0
unsigned int CreateCharacterOption::GetSetCount983()
{
	return SET_MOTION_COUNT983;
}

// 004d0cf0
bool CreateCharacterOption::IsSetMotion983(const char* motion, int* pKind)
{
	for (unsigned int i = 0; i < SET_MOTION_COUNT983; i++)
	{
		if (_stricmp(m_setMotions[i].motion, motion) == 0)
		{
			*pKind = i;
			return true;
		}
	}
	return false;
}

// 004d0dc0
CreateCharacterOption::~CreateCharacterOption()
{
}

// 004d0de0
void CreateCharacterOption::Load983()
{
	InitBarIcons983();
	InitChatFacials983();
	InitNames983();
	InitSubBones983();
	InitSetParts983();
}

// 004d0e00
CreateCharacterOption::CreateCharacterOption()
{
	Load983();
}

// 004d0e90 (chamador 005d5240)
void CreateCharacterOption::GetCharFileNames983(std::string* names)
{
	names[0] = "nuri_old";
	names[1] = "hana_old";
	names[2] = "azer";
	names[3] = "cecilia_old";
	names[4] = "max";
	names[5] = "kooh";
	names[6] = "arin";
	names[7] = "kaz";
	names[8] = "lucia";
	names[9] = "nell";
	names[10] = "spika";
	names[11] = "nuri";
	names[12] = "hana";
	names[13] = "azer";
	names[14] = "cecilia";
}

// 004d0fa0
void CreateCharacterOption::GetSetMotion983(int charIndex, const unsigned long* parts, int kind, std::string& motion,
	std::string& command)
{
	for (int i = 0; i < SET_PART_COUNT983; i++)
	{
		for (int j = 0; j < 24; j++)
		{
			if (m_setParts[charIndex][i].tid && kind == m_setParts[charIndex][i].kind &&
				parts[j] == m_setParts[charIndex][i].tid)
			{
				motion = m_setParts[charIndex][i].motion.motion;
				command = m_setParts[charIndex][i].motion.command;
				break;
			}
		}
	}
}
