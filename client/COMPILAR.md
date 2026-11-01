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

## Rodar: do clone limpo até a tela de login

O executável compilado não substitui os dados do jogo. Você precisa da sua própria cópia do cliente JP 983.

| Vem da sua cópia do jogo, nunca do repositório | Para quê |
|---|---|
| `ProjectG*.pak`, incluindo o pak base `ProjectG800j+.pak` | Dados do jogo |
| `mss32.dll` e a pasta `mss\` | Áudio (Miles Sound System) |
| `ijl15.dll`, a original da sua cópia | JPEG (Intel JPEG Library). Uma versão com remendos para o exe original não é necessária |
| `pangya.iff`, extraído dos paks pelo `setup-dev.sh` | Itens e preços no servidor |
| `Translation/Read.aspx`, **opcional** | Textos enviados pelo servidor original |

| Gerado na sua máquina, fora do git | Como |
|---|---|
| `ProjectG.exe`, `wangreal.dll`, `updlist.exe` | Build deste repositório |
| `server/data/gameweb/patch/updatelist` | `updlist --generate`, com tamanho e CRC dos `*.pak`, `ProjectG.exe`, `wangreal.dll` e `ijl15.dll` da sua pasta |
| `server/config/pangya.json`, banco e `server/data/*.iff` | `tools/setup-dev.sh` |

1. **Clone** o repositório: `git clone https://github.com/lucaspef/pangya-mulligan.git`.
2. **Compile o cliente** em Release|Win32, conforme a seção "Compilar". São necessários `ProjectG.exe`, `wangreal.dll` e `updlist.exe`.
3. **Instale os runtimes:** [DirectX End-User Runtimes (junho de 2010)](https://www.microsoft.com/download/details.aspx?id=8109), que fornece `d3dx9_43.dll` de 32 bits em `SysWOW64`, e o Visual C++ Redistributable x86.
4. **Prepare o servidor** no WSL: siga `docs/server/COMECANDO.md` até o passo 5 (.NET 10, PostgreSQL e `setup-dev.sh`). O script copia `config/pangya.example.json`, cujo `Run` já traz `game-web`. Se o seu `config/pangya.json` é anterior a isso, acrescente `"game-web"` ao `Run`.
5. **Rode, no PowerShell, a partir da raiz do clone:**

   ```powershell
   powershell -ExecutionPolicy Bypass -File client\tools\run-local.ps1 -GameDir "D:\Jogos\PangYa"
   ```

   O script:
   1. confere o pak base, `mss32.dll`, `ijl15.dll` e `d3dx9_43.dll`;
   2. copia `ProjectG.exe` e `wangreal.dll` para a pasta do jogo. O que estava lá fica em `<arquivo>.antes-mulligan`, uma vez só;
   3. gera o `updatelist` em `server/data/gameweb/patch/`;
   4. sobe o servidor com `tools/start.sh`. Na primeira vez, a compilação demora;
   5. espera `http://localhost:30091` responder;
   6. abre o jogo por um processo elevado. O UAC pode pedir confirmação.

6. **Chegue à tela de login.** A carga leva alguns segundos. Crie a conta pelo cadastro em `http://localhost:30080/register` antes de entrar.

Depois de **cada compilação**, rode o script de novo: o CRC do exe muda, e sem `updatelist` novo o jogo acusa `ProjectG.exe` como danificado.

Parâmetros: `-NoCopy` não mexe no exe da pasta, `-NoServer` usa um servidor que já está no ar e `-NoGame` faz tudo, menos abrir o jogo. Veja também `-Login`, `-Web` e `-Www`, abaixo.

`ProjectG.exe` e `wangreal.dll` precisam ser do mesmo build: copie sempre os dois. Com o `wangreal.dll` original e o exe compilado, o jogo fecha logo na abertura com heap corrompido (`c0000374` no `jp983_stubs.log`).

### Web do jogo

O nome `game-web` no `Run` do servidor, em `Pangya.Web/GameWeb.cs`, atende em `Web.GamePort` (30091). Sozinho, ele não precisa de banco. Serve dois caminhos, sempre com `Content-Length` e sem `chunked`, lidos de `Web.GameDir` (`server/data/gameweb`) a cada pedido:

- `GET /patch/updatelist`: lista cifrada. Sem o arquivo, responde 404 e o jogo não abre;
- `GET /Translation/Read.aspx`: arquivo `Translation/Read.aspx`, se existir. Sem ele, responde uma tabela vazia: o jogo abre, mas as mensagens que dependem dessa tabela ficam em branco. O formato é base64 de `<TEXT><KEY>n</KEY><DEV></DEV><SERVICE>texto</SERVICE></TEXT>...`.

Para gerar só o `updatelist`: `updlist --generate <pasta do jogo> <arquivo>`. Para atualizar um arquivo em uma lista existente: `updlist <updatelist> ProjectG.exe wangreal.dll`. O pós-build faz o mesmo com `/p:JP983UpdateList=<arquivo>`.

### Registro

O jogo lê o valor `IntegratedPak` em `HKLM\SOFTWARE\WOW6432Node\Ntreev\Pangya_Jp`, com o nome do pak base. Sem ele, mostra um erro de arquivo e não abre. O instalador do jogo grava esse valor. Numa cópia sem instalador, o `run-local.ps1` o cria, só quando falta, e nunca troca um valor existente. Para fazer à mão, num prompt elevado:

```bat
reg add "HKLM\SOFTWARE\Ntreev\Pangya_Jp" /reg:32 /v IntegratedPak /t REG_SZ /d "ProjectG800j+.pak"
```

### Endereços

`ProjectG.exe` pede administrador (`requireAdministrator` no manifesto). O exe sai com `JP983_SERVER_CONFIG` ligado e aponta para o servidor local. Para outro endereço, use os parâmetros do script, que definem as variáveis já no processo elevado:

| Parâmetro | Variável | Padrão | Serviço |
|---|---|---|---|
| `-Login` | `JP983_LOGIN` | `127.0.0.1:30101` | Login, `host:porta` |
| `-Web` | `JP983_WEB` | `http://localhost:30091` | Web do jogo |
| `-Www` | `JP983_WWW` | `http://localhost:30080` | Páginas e emblemas |

Sem o script, defina as variáveis **no mesmo prompt elevado** de onde o jogo será aberto. A elevação do UAC descarta as variáveis de um prompt comum, e o jogo volta ao padrão sem aviso. Use `localhost`, não `127.0.0.1`, em `JP983_WEB`.

Se outro programa já ocupa `127.0.0.1:30091`, use `"GameBindIp": "::1"` na seção `Web` da configuração: `localhost` também resolve para `[::1]`.

Opcional: `/p:JP983Jogo=C:\Caminho\Do\Jogo\` (com a barra no fim) faz o pós-build copiar os dois arquivos, e o F5 do VS usa essa pasta como diretório de trabalho. `/p:JP983ServerConfig=false` tira a configuração embutida.

## Diferenças em relação ao jogo original

- O D3DX do build é o 9.43; o jogo original usava `d3dx9_31.dll`.
- O título da janela, "PangYa Mulligan!", está no código, em `SetWindowTitle` (`src/projectg/winmain.cpp`). O original vinha do `japan.dat`.
- O LZO1X de `src/shared/lzo1x.cpp` é próprio. Teste: `tools/lzo_test/lzo_roundtrip.cpp`.
