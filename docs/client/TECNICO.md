# Cliente: visão técnica

O cliente é o código C++ de `ProjectG.exe` e `wangreal.dll`, baseado no PangYa JP.R7.983.00. Compilar: `client/COMPILAR.md`.

## Estrutura

| Pasta | Conteúdo |
|---|---|
| `client/src/wangreal/` | `wangreal.dll`: Direct3D 9, entrada e IME |
| `client/src/engine/` | Biblioteca de engine, ligada dentro do exe; inclui FreeType |
| `client/src/fresh/` | Biblioteca de interface Fresh; inclui zlib e libpng |
| `client/src/projectg/` | Código do jogo; `jp/` guarda o que é específico do JP |
| `client/src/shared/` | Código comum; inclui Lua e o LZO1X próprio |
| `client/include/` | Cabeçalhos comuns a mais de um módulo |
| `client/proj/vs2022/` | Solução, projetos e recursos |
| `client/tools/` | `compat_msvc` (compatibilidade com o MSVC atual), `updlist.c` e `lzo_test` |

## Convenções

- Comentários `983: 00a6a750` citam o endereço correspondente no executável estudado.
- Os projetos usam `__fastcall` como convenção padrão (`/Gr`) e FPU x87 (`/arch:IA32`).
- `JP983_SERVER_CONFIG` embute a configuração do servidor no exe. `JP983_POSTEFFECT` e `REBANG_SEMANTIC_ONLY` são parte do build normal.
- O exe mantém a base fixa `0x400000`, sem ASLR.

## Mudanças de projeto

Os `.vcxproj` são a fonte da configuração de build: edite-os diretamente. Há 3 mudanças importantes em relação ao ambiente de reconstrução: bibliotecas do Windows vêm do Windows SDK atual, D3DX e WTL são dependências externas, e FreeType é compilada do fonte no projeto `terc`.
