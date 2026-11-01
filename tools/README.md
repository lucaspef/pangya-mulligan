# Ferramentas

*English summary at the end.*

Ferramentas para ler, verificar e criar dados de campo. Nenhum dado do jogo fica aqui: cada pessoa usa a própria cópia, e o que é extraído dela nunca entra no git.

| Pasta | O que faz |
|---|---|
| `fairwayed/` | FairwayED: editor de buracos (`.wep`) e compilador para `.gbin`. Terceiro, MIT, código sem alterações |
| `fairwayed/wshared/` | Biblioteca do FairwayED. Terceiro, MIT, código sem alterações |
| `fwcompile/` | Nosso. Chama o compilador do FairwayED pela linha de comando |
| `gbincheck/` | Nosso. Lê um `.gbin` com o parser real do cliente e compara dois arquivos |
| `mapmaker/` | Nosso. Gera um buraco novo a partir de um JSON ([instruções](mapmaker/README.md)) |

## Compilar

Precisa de Visual Studio 2022 com C++ x86 e de Python 3. A partir de um clone limpo:

```bat
tools\build-tools.bat
```

Saída em `tools\bin\` (`fwcompile.exe`, `gbincheck.exe`) e `tools\fairwayed\bin\` (`FairwayED.exe`, `fairwayc.exe`). O `build-fairwayed.bat` existe porque o `build.bat` do autor quebra ao procurar o Visual Studio pelo `vswhere`; ele define `VCVARS` e chama o original.

## Verificar contra os dados do jogo

Extraia os `.gbin`, `.sbin`, `.pet` e `_property.xml` da sua cópia para uma pasta fora do repositório.

```bat
tools\fairwayed\bin\fairwayc.exe roundtrip <dados>
python tools\gbincheck\run_batch.py <dados> <saida> --tol 0.02
python tools\gbincheck\run_batch.py <dados> <saida> --wep --tol 0.02
```

O `gbincheck` não reimplementa o formato: `make_parser.py` recorta os leitores de `client/src/projectg/polysoup.cpp` a cada build e os compila com `stub.h`. Resultado da última rodada em [docs/ferramentas.md](../docs/ferramentas.md).

---

## English summary

Tools to read, verify and create course data. No game data lives here.

| Folder | Purpose |
|---|---|
| `fairwayed/`, `fairwayed/wshared/` | FairwayED hole editor and `.gbin` compiler, third party, MIT, code unmodified |
| `fwcompile/` | Ours. Command-line front end for the FairwayED compiler |
| `gbincheck/` | Ours. Loads a `.gbin` with the client's real parser and compares two files |
| `mapmaker/` | Ours. Builds a new hole from a JSON description |

Build from a clean clone with `tools\build-tools.bat` (Visual Studio 2022 x86 C++ and Python 3). Results of the checks are in `docs/ferramentas.md`.
