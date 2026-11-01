# Ferramentas de modelos

Criar e inspecionar modelos do PangYa (`.pet`, `.mpet`, `.bpet`, `.apet`).

| Pasta | O que é | Licença |
|---|---|---|
| `tools/caddierig` | CaddieRig e `wshared`, cópia de terceiros com correções nossas | MIT (`LICENSE` e `wshared/LICENSE`) |
| `tools/modelmaker` | Conversor glTF/OBJ para `.pet`/`.mpet`, gerador de pak e exemplos | AGPL-3.0 do projeto |
| `tools/modelmaker/harness` | `pethost`: carrega e desenha com o código real do cliente, sem servidor | AGPL-3.0 do projeto |

Nenhum dado do jogo entra no git. `*.pet`, `*.mpet` e `*.exe` são ignorados; `.obj` também, por isso os exemplos são glTF.

## Compilar

Precisa do Visual Studio 2022 com as ferramentas C++ x86.

```bat
tools\caddierig\build.bat
```

Gera `tools\caddierig\bin\crigc.exe` (linha de comando) e `CaddieRig.exe` (janela).

O harness usa o cliente já compilado (`client\COMPILAR.md`):

```bat
set JP983_D3DX_INCLUDE=<pasta do d3dx9.h>
set JP983_D3DX_LIB=<pasta da d3dx9.lib x86>
tools\modelmaker\harness\build.bat
```

Para rodar, copie `pethost.exe` para uma pasta com `wangreal.dll`, `ijl15.dll` e `d3dx9_31.dll`: a dll é procurada no diretório atual.

`modelmaker.py` usa só a biblioteca padrão do Python. `examples\gerar.py` usa Pillow e só é necessário para refazer os exemplos.

## Criar um modelo

```bat
python tools\modelmaker\modelmaker.py build bola.gltf -o ball_low.pet --bone Ball_Low --texture bola.png
python tools\modelmaker\modelmaker.py build bicho.gltf -o bicho.pet
python tools\modelmaker\modelmaker.py build chapeu.gltf -o g_ha_mulligan.mpet --skeleton g_def.bpet
```

- **Entrada:** glTF 2.0 (`.gltf`) ou OBJ. O glTF traz pesos, juntas e animação; o OBJ, só malha.
- **Unidade:** 1 unidade do arquivo = 1 unidade do jogo; ajuste com `--scale`. A bola do jogo tem raio de 0,155.
- **Textura:** nome de até 31 bytes. O prefixo do nome liga efeitos do cliente, como `[`, `+`, `~` e `!` (`tools/caddierig/docs/file-formats.md`).
- **Versão:** `--version 1.1` é o padrão: normais no espaço do modelo.
- **Ossos:** todo osso recebe trilha de animação; o pai vem antes do filho. Os nomes `Bip01`, `Bip01 Spine` e `Bip01 Head` existem nos 14 esqueletos conferidos.
- **Peça `.mpet`:** o `--skeleton` lê o `*_def.bpet` da sua cópia do jogo. A malha se liga aos ossos pelo nome, e a peça leva só os ossos usados, mais um osso-malha próprio. Modele na pose e na escala do personagem.

### Exemplos

Em `tools/modelmaker/examples`, feitos do zero por `gerar.py`:

| Arquivo | Conteúdo |
|---|---|
| `bola_mulligan.gltf` + `.png` | Objeto estático: bola com uma textura |
| `boneco_mulligan.gltf` + `.png` | Malha com pesos, ossos `Bip01`, `Bip01 Spine` e `Bip01 Head`, e o movimento `stand` |

### Rota pelo CaddieRig (ASE, PMP, Ani)

```bat
python tools\modelmaker\modelmaker.py ase boneco.gltf -o saida
tools\caddierig\bin\crigc.exe build saida\boneco.pmp boneco.pet /v 1.3
```

ASE não leva pesos: cada vértice fica no osso principal, e os triângulos que cruzam ossos se perdem. Prefira `build` para modelos com pesos. No Git Bash, `/v` vira um caminho; rode no `cmd` ou no PowerShell.

### Blender

- **Criar:** modele no Blender e use File > Export > glTF 2.0. Depois, `modelmaker build`. Testado com Blender 5.2.1 nos dois exemplos, com ida e volta pelo Blender.
- **Abrir modelos do jogo para estudo:** `io_scene_mpet` não vem neste repositório, porque não tem licença. Foi feito para o Blender 2.74 e usa `scene.objects.link`, que não existe mais. Para usá-lo, instale um Blender 2.7x à parte. Ele só importa, e só `.mpet`; não anima.
- **Inspecionar:** `tools\caddierig\bin\CaddieRig.exe modelo.pet /root pasta_das_texturas`.

## Instalar no cliente

O cliente procura cada arquivo nesta ordem (`WResourceManager::GetCFile`):

1. Lista `extracontents.xml`, que vem do servidor web.
2. Paks, do último montado para o primeiro.
3. Arquivo solto na pasta do jogo.

Logo, **arquivo solto só vale para nome que não existe em nenhum pak**. Para trocar um modelo existente, use um pak novo.

### Trocar um modelo existente

O exemplo troca a bola padrão. O item Comet (`0x14000000`) tem `Data = ball_low`.

```bat
python modelmaker.py build examples\bola_mulligan.gltf -o ball_low.pet --bone Ball_Low --texture bola_mulligan.png
python modelmaker.py pak ProjectG999.pak data/ball/ball_low.pet=ball_low.pet data/ball/bola_mulligan.png=examples\bola_mulligan.png
```

1. Copie `ProjectG999.pak` para a pasta do jogo.
2. Refaça o updatelist com `updlist --generate <pasta do jogo> <updatelist servido>`. **Pak fora do updatelist não é montado**: a lista define o que entra, e os paks são ordenados pelo número do nome.
3. Abra o jogo normalmente. Nada muda no servidor.

### Item novo

Exige alterar dados nos dois lados. Esta rota **não foi testada**.

| Onde | O quê |
|---|---|
| Cliente | Novo registro em `Ball.iff` ou `Part.iff`, dentro de `pangya_jp.iff`, em pak próprio |
| `TypeId` | `TypeId >> 26` dá o grupo: 2 peça, 5 bola (`docs/client/IFF_ITEM_COMMON.md`) |
| Modelo | `Data` é o nome sem extensão: bola = `<Data>.pet`, peça = `<Data>.mpet` |
| Peça | `Part.iff` também define `Tex`, `OrgTex`, `PosMask` e `HideMask` |
| Servidor | O mesmo iff em `server/data/pangya.iff` e `pangya_jp.iff` (`server/tools/sync-iff.py`). Depois, entregue o item ao inventário |

Item fora do iff é descartado: o cliente remove do boneco a peça que o `pangya.iff` não conhece.

## Conferir sem abrir o jogo

```bat
rem carrega e desenha; com --pak usa os paks do jogo e o pangya.fx
pethost ball_low.pet --pak <pasta do jogo> --bmp saida.bmp
pethost boneco.pet --pak <pasta do jogo> --anim 0.25 --bmp saida.bmp --yaw 0
pethost g_def.bpet --pak <pasta do jogo> --wear g_fc_def.mpet --wear minha_peca.mpet --bmp saida.bmp
```

Sem `--pak` não há `pangya.fx`, e modelos com vários ossos saem errados. Isso é limite do harness, não do arquivo.

`pethost` compila `client/src/engine` real e usa a `wangreal.dll` real: `GetPuppet`, `LoadPET`, `WxTnLBuffer` e os shaders do jogo.

## Testar no jogo

1. Com o jogo fechado, instale o pak e refaça o updatelist, como acima.
2. Suba servidor e web com `client\tools\run-local.ps1 -GameDir <pasta>`.
3. Entre com uma conta local que tenha a bola Comet equipada.
4. Abra uma partida ou o Practice. A bola deve aparecer listrada de vermelho, com faixa azul.
5. Para desfazer, apague o pak e refaça o updatelist.

## Correções sobre o CaddieRig original

Base: `CaddieRig` commit `4fb0618`, `wshared` commit `116770a`, ambos de <https://codeberg.org/retreev>. O submódulo virou pasta comum, e `.gitmodules` saiu.

| Arquivo | Correção | Por quê |
|---|---|---|
| `build.bat`, `wshared/build.bat` | Chamada ao `vswhere` | Quebrava com `(x86)` no caminho quando só há o Visual Studio Community |
| `wshared/src/petfile.cpp` | Byte de camada padrão `0xff`, não `0` | Com `SMTL` vazio, `0` derruba o cliente em `.pet`/`.mpet` v1.2 e v1.3. Os 3.186 arquivos do jogo nesse caso trazem `0xff` |
| `src/compile.cpp` | Duas chaves de escala, não uma | Com uma, `GetKeyScale` divide por zero depois de t=0 e o boneco some em v1.1 ou mais. O jogo grava duas |

`docs/file-formats.md` ainda diz "the second triangle byte is 0": vale a tabela acima.
