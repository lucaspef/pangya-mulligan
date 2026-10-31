// JP 983: tabela nível → nome do jogador (`CLevelNameTable983`, include/jp/levelnametable.h). Unidade 00832050–008345e0 (vão sem
// classe dentro da faixa do titlemanager.cpp do mapa; só o `std::map<int, sLevelName983>` e o que ele gera). Sessão B145, pedido
// C81.1: o construtor roda pelo .CRT$XCU antes do WinMain (s_levelNames das unidades grandprixlimitdlg/rfcharmasterytiledlg).
#include "stdafx.h"
#include "levelnametable.h"
#include <string.h>

// 008331a0: cada entrada pelo operator[] (00832bc0) com um temporário (nome, valor).
CLevelNameTable983::CLevelNameTable983()
{
	m_names[0] = sLevelName983("\xb7\xe7\xc5\xb0" "F", 30); // 루키F
	m_names[1] = sLevelName983("\xb7\xe7\xc5\xb0" "E", 40); // 루키E
	m_names[2] = sLevelName983("\xb7\xe7\xc5\xb0" "D", 50); // 루키D
	m_names[3] = sLevelName983("\xb7\xe7\xc5\xb0" "C", 60); // 루키C
	m_names[4] = sLevelName983("\xb7\xe7\xc5\xb0" "B", 70); // 루키B
	m_names[5] = sLevelName983("\xb7\xe7\xc5\xb0" "A", 140); // 루키A
	m_names[6] = sLevelName983("\xba\xf1\xb1\xe2\xb3\xca" "E", 105); // 비기너E
	m_names[7] = sLevelName983("\xba\xf1\xb1\xe2\xb3\xca" "D", 125); // 비기너D
	m_names[8] = sLevelName983("\xba\xf1\xb1\xe2\xb3\xca" "C", 145); // 비기너C
	m_names[9] = sLevelName983("\xba\xf1\xb1\xe2\xb3\xca" "B", 165); // 비기너B
	m_names[10] = sLevelName983("\xba\xf1\xb1\xe2\xb3\xca" "A", 330); // 비기너A
	m_names[11] = sLevelName983("\xc1\xd6\xb4\xcf\xbe\xee" "E", 248); // 주니어E
	m_names[12] = sLevelName983("\xc1\xd6\xb4\xcf\xbe\xee" "D", 278); // 주니어D
	m_names[13] = sLevelName983("\xc1\xd6\xb4\xcf\xbe\xee" "C", 308); // 주니어C
	m_names[14] = sLevelName983("\xc1\xd6\xb4\xcf\xbe\xee" "B", 338); // 주니어B
	m_names[15] = sLevelName983("\xc1\xd6\xb4\xcf\xbe\xee" "A", 675); // 주니어A
	m_names[16] = sLevelName983("\xbd\xc3\xb4\xcf\xbe\xee" "E", 506); // 시니어E
	m_names[17] = sLevelName983("\xbd\xc3\xb4\xcf\xbe\xee" "D", 546); // 시니어D
	m_names[18] = sLevelName983("\xbd\xc3\xb4\xcf\xbe\xee" "C", 586); // 시니어C
	m_names[19] = sLevelName983("\xbd\xc3\xb4\xcf\xbe\xee" "B", 626); // 시니어B
	m_names[20] = sLevelName983("\xbd\xc3\xb4\xcf\xbe\xee" "A", 1253); // 시니어A
	m_names[21] = sLevelName983("\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee" "E", 1002); // 아마추어E
	m_names[22] = sLevelName983("\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee" "D", 1052); // 아마추어D
	m_names[23] = sLevelName983("\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee" "C", 1102); // 아마추어C
	m_names[24] = sLevelName983("\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee" "B", 1152); // 아마추어B
	m_names[25] = sLevelName983("\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee" "A", 2304); // 아마추어A
	m_names[26] = sLevelName983("\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce" "E", 1843); // 세미프로E
	m_names[27] = sLevelName983("\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce" "D", 1903); // 세미프로D
	m_names[28] = sLevelName983("\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce" "C", 1963); // 세미프로C
	m_names[29] = sLevelName983("\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce" "B", 2023); // 세미프로B
	m_names[30] = sLevelName983("\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce" "A", 4046); // 세미프로A
	m_names[31] = sLevelName983("\xc7\xc1\xb7\xce" "E", 3237); // 프로E
	m_names[32] = sLevelName983("\xc7\xc1\xb7\xce" "D", 3307); // 프로D
	m_names[33] = sLevelName983("\xc7\xc1\xb7\xce" "C", 3377); // 프로C
	m_names[34] = sLevelName983("\xc7\xc1\xb7\xce" "B", 3447); // 프로B
	m_names[35] = sLevelName983("\xc7\xc1\xb7\xce" "A", 6894); // 프로A
	m_names[36] = sLevelName983("\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce" "E", 5515); // 내셔날프로E
	m_names[37] = sLevelName983("\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce" "D", 5595); // 내셔날프로D
	m_names[38] = sLevelName983("\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce" "C", 5675); // 내셔날프로C
	m_names[39] = sLevelName983("\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce" "B", 5755); // 내셔날프로B
	m_names[40] = sLevelName983("\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce" "A", 11511); // 내셔날프로A
	m_names[41] = sLevelName983("\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce" "E", 8058); // 월드프로E
	m_names[42] = sLevelName983("\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce" "D", 8148); // 월드프로D
	m_names[43] = sLevelName983("\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce" "C", 8238); // 월드프로C
	m_names[44] = sLevelName983("\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce" "B", 8328); // 월드프로B
	m_names[45] = sLevelName983("\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce" "A", 16655); // 월드프로A
	m_names[46] = sLevelName983("\xb8\xb6\xbd\xba\xc5\xcd" "E", 8328); // 마스터E
	m_names[47] = sLevelName983("\xb8\xb6\xbd\xba\xc5\xcd" "D", 8428); // 마스터D
	m_names[48] = sLevelName983("\xb8\xb6\xbd\xba\xc5\xcd" "C", 8528); // 마스터C
	m_names[49] = sLevelName983("\xb8\xb6\xbd\xba\xc5\xcd" "B", 8628); // 마스터B
	m_names[50] = sLevelName983("\xb8\xb6\xbd\xba\xc5\xcd" "A", 17255); // 마스터A
	m_names[51] = sLevelName983("\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd" "E", 9490); // 탑마스터E
	m_names[52] = sLevelName983("\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd" "D", 9690); // 탑마스터D
	m_names[53] = sLevelName983("\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd" "C", 9890); // 탑마스터C
	m_names[54] = sLevelName983("\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd" "B", 10090); // 탑마스터B
	m_names[55] = sLevelName983("\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd" "A", 20181); // 탑마스터A
	m_names[56] = sLevelName983("\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd" "E", 20181); // 월드마스터E
	m_names[57] = sLevelName983("\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd" "D", 20481); // 월드마스터D
	m_names[58] = sLevelName983("\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd" "C", 20781); // 월드마스터C
	m_names[59] = sLevelName983("\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd" "B", 21081); // 월드마스터B
	m_names[60] = sLevelName983("\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd" "A", 42161); // 월드마스터A
	m_names[61] = sLevelName983("\xb7\xb9\xc0\xfc\xb5\xe5" "E", 37945); // 레전드E
	m_names[62] = sLevelName983("\xb7\xb9\xc0\xfc\xb5\xe5" "D", 68301); // 레전드D
	m_names[63] = sLevelName983("\xb7\xb9\xc0\xfc\xb5\xe5" "C", 122942); // 레전드C
	m_names[64] = sLevelName983("\xb7\xb9\xc0\xfc\xb5\xe5" "B", 221296); // 레전드B
	m_names[65] = sLevelName983("\xb7\xb9\xc0\xfc\xb5\xe5" "A", 442592); // 레전드A
	m_names[66] = sLevelName983("\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5" "E", 663887); // 인피니티레전드E
	m_names[67] = sLevelName983("\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5" "D", 995831); // 인피니티레전드D
	m_names[68] = sLevelName983("\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5" "C", 1493747); // 인피니티레전드C
	m_names[69] = sLevelName983("\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5" "B", 2240620); // 인피니티레전드B
	m_names[70] = sLevelName983("\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5" "A", -1); // 인피니티레전드A
}

// 00832de0
CLevelNameTable983::~CLevelNameTable983()
{
}

// 008329d0
sLevelName983 CLevelNameTable983::Get(int level)
{
	std::map<int, sLevelName983>::iterator it = m_names.find(level);
	if (it == m_names.end())
	{
		return sLevelName983("dummy", 0);
	}
	return it->second;
}
