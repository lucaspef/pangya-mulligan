// JP 983: unidade 005aaf50–005ab060 (CNullWidget, o boneco de +0x1fc do CTask); ver include/jp/nullwidget.h. Frente C,
// sessão C53 (pedido C46.1).
#include "minatl.h"
#include "projectg.h"
#include "actor.h"
#include "taskmanager.h"
#include "nullwidget.h"

// 00cd1710 (RTTI "CNullWidget" com base 00e78d88 = CTaskWidget) e 00cd1730 (registro do criador 005aafb0)
IMPLEMENT_OBJECT(CNullWidget, CTaskWidget)

// 005aaf50
CNullWidget::CNullWidget()
{
}

// 005aaf80
CNullWidget::~CNullWidget()
{
}
