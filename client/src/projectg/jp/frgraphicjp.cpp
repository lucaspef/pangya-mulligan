// JP 983: FrWnd983 (00be7950). O antigo FrGraphicInterfaceJp::PrintFont983 (00bed170, B210) virou
// FrGraphicInterface::PrintTitleFont983 (src/fresh/source/frgraphicinterface.cpp), uma so copia como no 983.
#include <windows.h>
#include "frform.h" // FrForm::ms_bModalLock983 (00f1ab38)

// FrWnd983 (visão): 00be7950 só liga o byte global 00f1ab38 = FrForm::ms_bModalLock983 (o Open não liga o modal e o
// 1º DrawModalBack983 o desliga). A473: era um bool solto que ninguém lia -> lista de salas/sala escureciam o lobby (C378.1)

class FrWnd983
{
public:
	static void SetF1ab38_983(); // 00be7950
};

void FrWnd983::SetF1ab38_983()
{
	FrForm::ms_bModalLock983 = true;
}
