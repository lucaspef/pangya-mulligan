# mapmaker

Gera um buraco novo a partir de um JSON: terreno, superfícies, água, vento e pontos. Tudo é criado aqui, sem nenhum arquivo do jogo. O resultado são os arquivos do curso e um pak que vence os paks originais.

## Uso

```bat
tools\build-tools.bat
python tools\mapmaker\mapmaker.py tools\mapmaker\examples\mulligan_01\hole.json -o saida --base-property <blue_property.xml extraído da sua cópia>
```

Sai em `saida/`:

| Arquivo | O que é |
|---|---|
| `loose/data/.../map/<curso>_<nn>.pet` | Modelo base (terreno), um osso, formato dos cenários do jogo |
| `loose/data/.../map/<curso>_<nn>.wep` | Projeto; abre no FairwayED |
| `loose/data/.../map/<curso>_<nn>.gbin` | Buraco compilado pelo `fwcompile`, versão 0x71, cores de vértice calculadas |
| `loose/data/.../map/<curso>_<nn>.sbin` | Sombra vazia. Precisa existir: a do buraco original não serve para esta malha |
| `loose/data/.../map/mm_*.dds` | Texturas DXT1 geradas pelo script |
| `loose/data/.../text/<curso>_property.xml` | Superfícies. Com `--base-property`, as do curso original mais as nossas |
| `ProjectG999.pak` | Os arquivos acima, menos o `.wep` |
| `preview.png` | Vista de cima |

Sem `--base-property` o XML traz só as nossas superfícies. Serve para testar o buraco, mas os outros buracos do curso ficam com superfícies erradas enquanto o pak estiver instalado.

## Descrição do buraco

Coordenadas em unidades do mundo (1 jarda = 3,2 unidades), `x` e `z` no plano, `y` para cima. Exemplo completo em `examples/mulligan_01/hole.json`.

| Campo | Significado |
|---|---|
| `course`, `course_dir`, `hole` | Nome interno do curso, pasta dele e número do buraco. Definem qual buraco é substituído |
| `par` | Par |
| `terrain.size`, `cell`, `tile` | Largura e comprimento, tamanho da célula, repetição da textura |
| `terrain.base_height`, `hills` | Altura base e morros (`x`, `z`, `radius`, `height`) |
| `tee`, `tee2` | Saída normal e difícil (`tee2` é opcional) |
| `pins` | Até 3 bandeiras, sobre o green |
| `zones` | Em ordem; a última que cobre o ponto vale |
| `flyover` | Pontos do sobrevoo da câmera |
| `wind` | Áreas de vento: `script` no formato `[*+-=]força direção inclinação` e `box` |
| `areas` | Outras áreas: `class`, `script` e `box` (por exemplo `*extra`) |
| `out_of_bounds` | Polígonos de fora de limite (nós `ib`) |
| `sun`, `light`, `camera` | Opcionais: posição do sol, cores (`"r g b"`), câmera |

Uma zona tem `type` (`tee`, `fairway`, `rough`, `green`, `bunker`, `water`) e uma forma: elipse (`center`, `rx`, `rz`, `rot`), faixa (`points`, `width`) ou retângulo (`rect`). `height` e `falloff` levantam ou afundam o terreno; bunker e água já vêm afundados. Green aceita `tilt` (inclinação em `x` e `z`). Água aceita `level` e `script` (padrão `*type 0`).

## Instalar para testar

O buraco novo entra substituindo um buraco que existe: o pak usa o mesmo caminho de arquivo, e o cliente procura primeiro no último pak enganchado. `ProjectG999.pak` fica por último porque a ordem é numérica.

Um pak só entra se estiver no updatelist. Com `client/tools/run-local.ps1`, copie o pak para a pasta do jogo e rode o script, que refaz a lista. Se a porta 30091 pertence a outra pasta, use `instalar.ps1`, que sobe um web próprio na 30092.

Para desfazer, apague o pak e refaça o updatelist. Use sempre uma pasta de teste.

No exemplo: **Praticar > Blue Lagoon > repetir buraco > buraco 1**. O curso é `blue`, buraco 1.

## Servidor

Substituindo um buraco, o servidor não muda: ele recebe par, tee e bandeira do cliente no pacote 0x1A e não confere o `.gbin`. O checksum desse pacote muda com o arquivo; hoje nada o compara.

Um curso com id próprio ainda não foi feito. Exigiria um registro novo em `pangya.iff` (campos `Data`, `TexProp` e `DiffFlag`, id `0x28000000 | mapa`), a lista de cursos do servidor e os textos da interface.

## Limites

- Só o terreno: ainda não há objetos (árvores, casas), `.sgbin` nem `.aibin`. Modos que usam esses dois arquivos continuam lendo os do buraco original.
- Sem sombras projetadas: o FairwayED não gera `.sbin`, e a luz é só por vértice.
- O céu, a névoa e as águas seguem as do curso substituído.
