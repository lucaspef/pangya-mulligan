// JP 983 (A222): ltable.c do Lua com o luaconf.h do 983 (lua_number2int por fld/fistp, 983 luaH_get 00c2c8c1). A lua51_Release_MD_fastcall.lib
// do rebang usa o truque da soma (6755399441055744.0), que erra com a FPU em _PC_24 (CProjectG::FPU_Check): o lua_gettable com
// chave numerica voltava nil (E146, C150.1, a220-1). Esta unidade define todos os simbolos do membro ltable.obj da lib, que deixa de entrar.
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
extern "C" {
#include "Lua/src/ltable.c"
}
