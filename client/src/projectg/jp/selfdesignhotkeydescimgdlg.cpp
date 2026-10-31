// JP 983: unidade 006274a0–00627600 (SelfDesignHotkeyDescImgDlg; ver include/jp/selfdesignhotkeydescimgdlg.h). O corte da
// F3.7.1 (00626629–00627610) começava no meio: 00626640–006269df é a tabela de saltos da função anterior (frente A) e
// 006269e0–006274a0 é outra unidade, sem classe (bloco "extension" em XML dentro de um arquivo em pedaços de 0x13 bytes,
// chamada pelo golfrule 006884f0 e pelo FrameImageSet 00650072), ainda não escrita. Sessão B57.
#include "minatl.h"
#include "projectg.h"
#include "selfdesignhotkeydescimgdlg.h"

IMPLEMENT_OBJECT(SelfDesignHotkeyDescImgDlg, FrForm)

BEGIN_FRESH_MSGMAP(SelfDesignHotkeyDescImgDlg, FrForm)

ON_FRESH_VV("close", FRCMD_LBUTTONUP, SelfDesignHotkeyDescImgDlg::OnFreshOkay)

END_FRESH_MSGMAP()

// 006274d0
SelfDesignHotkeyDescImgDlg::~SelfDesignHotkeyDescImgDlg()
{
}
