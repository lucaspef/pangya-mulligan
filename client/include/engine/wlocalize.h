#pragma once

// No 645 era inline e devolvia o próprio texto; no JP 983 é uma função da unidade lang (src/wangreal/langtable.cpp)
// que procura o texto coreano em korea.dat e devolve o par de japan.dat.
const char* K2L_Compatibility(const char* str);
