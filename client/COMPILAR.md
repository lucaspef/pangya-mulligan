# Compilar o cliente

Gera `ProjectG.exe` e `wangreal.dll` no Visual Studio 2022, plataforma **Win32**. Nenhum SDK ou arquivo do jogo vem neste repositório; as dependências abaixo são instaladas por você.

## Pré-requisitos

| Item | Como obter | Para quê |
|---|---|---|
| Visual Studio 2022 | Cargas "Desenvolvimento para desktop com C++", com MSVC v143 x86/x64, Windows SDK 10 e "ATL do C++ para v143" | Compilador, Windows SDK (`d3d9`, `dinput8`, `dxguid`, libs do Windows) e ATL |
| D3DX 9 (`d3dx9.lib` e cabeçalhos) | **DirectX SDK de junho de 2010** da Microsoft, ou o pacote NuGet `Microsoft.DXSDK.D3DX` (testado: 9.29.952.8) | Não vem no Windows SDK |
| WTL 9.0 (só cabeçalhos) | Pacote NuGet `wtl` versão **9.0.4140**. As versões 9.1 e 10 não servem: não têm `WTL::CString` | Interface do cliente |

Um pacote NuGet é um `.zip`: baixe em `https://www.nuget.org/packages/<nome>/<versão>` ("Download package"), renomeie para `.zip` e extraia.

## Variáveis de ambiente

| Variável | Valor |
|---|---|
| `JP983_WTL` | Pasta do `atlapp.h`. No pacote: `lib\native\include` |
| `JP983_D3DX_INCLUDE` | Pasta do `d3dx9.h`. No pacote: `build\native\include` |
| `JP983_D3DX_LIB` | Pasta da `d3dx9.lib` x86. No pacote: `build\native\release\lib\x86` |

Com o DirectX SDK instalado, `JP983_D3DX_*` são opcionais: o instalador define `DXSDK_DIR`, usado como padrão. Também é possível passar `/p:JP983Wtl=...`, `/p:JP983DxInclude=...` e `/p:JP983DxLib=...` ao MSBuild. Se faltar algo, o build para com uma mensagem indicando a variável.

## Compilar

Abra `client\proj\vs2022\PangYaMulligan.sln`, escolha **Release** ou **Debug**, **Win32**, e faça Build. Se o VS oferecer "Redirecionar projetos", recuse. Pela linha de comando:

```bat
msbuild client\proj\vs2022\PangYaMulligan.sln /p:Configuration=Release /p:Platform=Win32 /m
```

A saída fica em `client\build\vs2022\<Configuração>\`. A solução compila `terc` (zlib, libpng, Lua e FreeType, do fonte), `updlist`, `wangreal` e `ProjectG`.

As import libs de `mss32.dll` e `ijl15.dll` são geradas no build, em `build\vs2022\<Configuração>\ProjectG\implib`, a partir de `proj\vs2022\implib\*.def` e de stubs que não vão para a saída. Nenhum binário do Miles ou da Intel é usado para compilar.

## Rodar

Você precisa da sua própria cópia do cliente JP. O executável compilado não substitui os dados do jogo.

1. Compile uma vez e copie `ProjectG.exe` e `wangreal.dll` para a pasta do jogo. Para automatizar, passe `/p:JP983Jogo=C:\Caminho\Do\Jogo\` (com a barra no fim): o pós-build copia os dois arquivos, e o F5 do VS usa essa pasta como diretório de trabalho.
2. Mantenha na pasta do jogo as DLLs que vieram com a sua cópia:
   - `mss32.dll`, do Miles Sound System;
   - `ijl15.dll`, do Intel JPEG Library. Use a original da sua cópia. Uma `ijl15.dll` com remendos para o exe original não é necessária.
3. Instale o [DirectX End-User Runtimes (junho de 2010)](https://www.microsoft.com/download/details.aspx?id=8109), que fornece `d3dx9_43.dll`, e o Visual C++ Redistributable x86.
4. Suba o servidor (`docs/server/COMECANDO.md`). O exe sai com `JP983_SERVER_CONFIG` ligado e aponta para o servidor local. Para outro endereço, defina antes de abrir o jogo:

| Variável | Padrão | Serviço |
|---|---|---|
| `JP983_LOGIN` | `127.0.0.1:30101` | Login, `host:porta` |
| `JP983_WEB` | `http://localhost:30091` | Web do jogo |
| `JP983_WWW` | `http://localhost:30080` | Páginas e emblemas |

Opcional: `/p:JP983UpdateList=<arquivo>` faz o pós-build refazer tamanho e CRC dos dois arquivos no `updatelist`. `/p:JP983ServerConfig=false` tira a configuração embutida.

## Diferenças em relação ao jogo original

- O D3DX do build é o 9.43; o jogo original usava `d3dx9_31.dll`.
- O título da janela, "PangYa Mulligan!", está no código, em `SetWindowTitle` (`src/projectg/winmain.cpp`). O original vinha do `japan.dat`.
- O LZO1X de `src/shared/lzo1x.cpp` é próprio. Teste: `tools/lzo_test/lzo_roundtrip.cpp`.
