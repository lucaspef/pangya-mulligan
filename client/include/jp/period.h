#pragma once
#include <windows.h>
#include <map>

// JP 983: tabela fixa de períodos de evento (datas de 2010–2012) consultada pela hora do servidor. Singleton estático local
// (instância 00e75d24, guarda 00e75d38; destrutor no atexit 00cf9cc0); vtable 00d5450c só com o destrutor. Quem pergunta:
// 00596b50 (ícone do desktop, período 0x18). Classe nova do 983 (existe no 839, não no 645). Unidade 00b3d360–00b3f030
// (jp/period.cpp; o corte do mapa começava em 00b3e730: MakeTime983 e os moldes do std::map vêm antes; 00b3d091–00b3d360 é o
// sd_shop::CCommodityList). Nomes dos métodos e dos campos são meus.
class CPeriod
{
public:
	struct sPeriod983				// 0x22 (new 0x22 em 00b3e975)
	{
		SYSTEMTIME start;			// +0x00
		SYSTEMTIME end;				// +0x10
		bool bDaily;				// +0x20: além das datas, só entre a hora de start e a de end de cada dia
	};

	CPeriod();						// 00b3e9d0
	virtual ~CPeriod();				// 00b3e730

	static CPeriod& Instance()		// embutido (cópia fora de linha em 00593350)
	{
		static CPeriod s_period;
		return s_period;
	}

	bool IsInPeriod983(int id);		// 00b3eef0
	sPeriod983* Find983(int id);	// 00b3e5a0
	bool Remove983(int id);			// 00b3e5e0 (sem chamador)

protected:
	bool Add983(int id, SYSTEMTIME start, SYSTEMTIME end, bool bDaily);	// 00b3e970
	SYSTEMTIME MakeTime983(WORD year, WORD month, WORD day, WORD hour, WORD minute); // 00b3d360

	std::map<int, sPeriod983*> m_periods; // +0x04
};
