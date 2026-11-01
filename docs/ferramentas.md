# Ferramentas de campo: estado

Como compilar e usar: [tools/README.md](../tools/README.md) e [tools/mapmaker/README.md](../tools/mapmaker/README.md).

Testado com os dados do JP 983.

## FairwayED, sem compilar nada

`fairwayc roundtrip` carrega e regrava sem editar:

| Arquivos | Quantidade | Idênticos |
|---|---|---|
| `.gbin` | 360 | 360 |
| `.sbin` | 359 | 359 |
| `.sgbin` | 634 | 634 |
| `.aibin` | 361 | 361 |
| `.wep` | 20 | 20 |
| `_property.xml` | 23 | 21 |

Os dois XML diferentes têm tabulações soltas; o editor mantém os bytes originais até haver edição.

## `.gbin` recompilado, lido pelo parser real do cliente

311 buracos 0x71 reconstruídos em projeto, recompilados e relidos pelos leitores de `polysoup.cpp`. Os 48 buracos 0x72 ficam fora: o FairwayED só escreve 0x71. Tolerância de 0,02, porque o projeto guarda posições com duas casas.

| Campo | Iguais |
|---|---|
| Câmeras | 697 de 697 |
| Pontos | 3.792 de 3.792 |
| Áreas | 2.245 de 2.245 |
| Nós | 1.881 de 1.881 |
| Tee e pin pelo nome | 933 de 933 |
| Par | 311 de 311 |
| Cores do modelo base | 311 de 311 |
| Modelos | 39.498 de 39.524 |
| Campos do modelo base | 1.242 de 1.244 |
| Contagens do cabeçalho | 1.816 de 1.866 |
| Conjunto de texturas | 261 de 311 |
| Ordem das texturas | 1.349 de 18.355 |
| Checksum do arquivo | 11 de 311 |
| Leitura termina no byte esperado | 311 de 311 |

As diferenças:

- **Posições:** sem tolerância quase todos os registros divergem na 3ª casa. A perda está na importação para `.wep`, não no compilador.
- **Ordem das texturas:** não afeta o cliente, que procura por nome e por handle.
- **Conjunto de texturas (50 buracos):** explica também as 50 contagens. Segundo a documentação do FairwayED, são texturas que nenhum triângulo usa ou modelos refeitos depois; não confirmei caso a caso.
- **Modelos (26):** a importação perde os bits baixos de `flag` além de 2 e 4. Exemplo: `fighter07_ani.pet` em `silv_01`, `type` 3 no original e 0 no recompilado, o que desliga a animação.
- **Modelo base (2):** a importação perde a escala. Exemplo: `c_1_chaos_rank`, 1,33 no original e 1,00 no recompilado.
- **Checksum:** soma os primeiros 87 bytes e muda com qualquer arredondamento. O servidor não o compara.

Essas perdas ocorrem ao reimportar um `.gbin`. Compilar um projeto novo não passa por esse caminho, e por isso o código do FairwayED ficou sem alterações.

## Projetos `.wep` originais

19 projetos com `.gbin` 0x71 correspondente. Tee, pin, par e nós batem em todos. O resto diverge porque os projetos são anteriores à versão publicada dos buracos (`DefaultCamera` movida, pontos e áreas acrescentados): 218 de 228 pontos e 2.799 de 3.402 modelos.

## Compilado carregado pelo cliente

| Etapa | Estado |
|---|---|
| `.gbin` novo lido pelo parser real, offline | Provado no buraco de exemplo: tee, 3 pins e par certos, leitura termina no byte esperado |
| Map Check do FairwayED | Provado: 0 avisos |
| Winding do terreno | Provado contra `wind_01.pet`: mesma regra em 2.660 de 2.660 triângulos |
| Pak novo lido por um leitor independente | Provado: 10 de 10 arquivos idênticos |
| Cliente abre com o pak no updatelist | Provado: passa da verificação de arquivos sem erro |
| Cliente carrega o buraco e se joga nele | Provado em Praticar, Blue Lagoon, buraco 1: carrega, mostra 322y e par 4, a tacada voa 293y e o jogo reconhece o Rough |
| `.pet` gerado no `WPuppet` real e física da tacada | Provado no mesmo teste |
| Cair na água, entrar no buraco e terminar a partida | **Não provado.** O teste parou na primeira tacada |

Buraco sem objetos exige o cliente com a correção de `WPolySoup::LoadModels983`. Antes dela, a lista de objetos animados do cenário anterior continuava valendo e o jogo caía ao carregar.

## Problema conhecido

Nesta máquina, as quatro janelas 3D do `FairwayED.exe` ficam brancas e `/shot` trava, também com um buraco original. O projeto abre e a árvore de elementos aparece; o `fwcompile` não depende dessa parte.
