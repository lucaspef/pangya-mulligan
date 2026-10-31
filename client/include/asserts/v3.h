// V3 (frente C): macros dos static_assert de layout. Incluído só pelo alvo `make layoutcheck` (tools/v3/layoutcheck.py),
// depois da unidade inteira; os valores vêm do exe 983 (out/V3/RELATORIO.md). Não incluir nos fontes do jogo.
// Assert que falha = layout nosso diferente do 983: pedido para a frente dona em out/PEDIDOS.md (não mexer aqui).
#pragma once
#include <stddef.h>
#define V3_SIZE(T, N) static_assert(sizeof(T) == (N), "V3 sizeof|" #T "|" #N);
#define V3_OFF(T, m, N) static_assert(offsetof(T, m) == (N), "V3 offsetof|" #T "::" #m "|" #N);
// V3_ANOT: deslocamento anotado no comentário do cabeçalho SEM a marca 983 (pode ser do 645): rede de regressão, não fato do 983.
// Se falhar, o layoutcheck diz `anot_velha` e não quebra o alvo (tools/v3/poda.py tira a anotação velha da lista).
#define V3_ANOT(T, m, N) static_assert(offsetof(T, m) == (N), "V3 anot|" #T "::" #m "|" #N);
