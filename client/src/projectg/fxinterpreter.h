#pragma once

// 983: o FxInterpreter do exe e a unidade inteira da frente B (src/projectg/jp/fxinterpreter.cpp, 0087f2b0..008907d0;
// layout do 983, sizeof 0x1e0). O cabecalho do 645 com o layout provisorio e a fxinterpreter.cpp da A sairam do build do
// exe (FORA_983 no tools/f32/pgbuild.py; pedido C64.2: eram duas classes FxInterpreter no link). Nomes antigos da A ->
// nomes da B: SetUnk983_1cc -> SetMode, DetachA983/B983/C983 -> DetachGroups/DetachOrdered/DetachNamed, DetachSlot983 ->
// DetachSlot, Init983 -> InitTokens, Unk983_880d40 -> RegisterKeeper, Unk983_87f2d0 -> SetFxBox. Sem GetFxInterpreter no 983.
#include "jp/fxinterpreter983.h"
