# JP 983: pacotes do cliente que o servidor ainda não trata (19/10/2026)

Levantamento: todos os `WSendPacket` do `ProjectG.exe` do JP (construtor `0xA4E800`, envio `0xA4DAB0`) dão **251 ids**
C->S. Cada um foi mandado ao game server de teste (cliente JP falso, no lobby, 400 bytes zerados) e a reação anotada:
215 tratados, 6 reconhecidos e ignorados de propósito, 28 com problema. Só o lado C->S; o que o cliente sabe receber e o
servidor nunca manda não entrou.

Corrigido no mesmo dia: **0x7C** (publicar a loja pessoal) e a entrada na loja, quando recusados, quebravam a conversão
para o JP e o cliente ficava sem resposta.

## Funções reais que faltam
| JP | KR | o que é | observação |
|---|---|---|---|
| 0x192 | — | ícone do aniversário da Arin 2014 | ver JP983-EVENTOS.md |
| 0x15B | — | evento de aniversário (`FrBirthdayEventDlg`) | idem |

## Código morto no cliente (nada chama: sem servidor)
| JP | KR | o que é | prova |
|---|---|---|---|
| 0x1E | 0x1E | vender item para a loja (janela "repay": u8 tipo, u8 1, u32 typeid, u32 id) | `CShopMain::Repay` (JP `0xA87610`) sem chamada, salto nem ponteiro; no KR também ninguém chama |
| 0xEA | 0xEC | melhorar caddie OLD -> novo (1->0x10 … 7->0x16; itens "upgrade (Pippin)" etc. no CaddieItem.iff) | `UpgradeCaddie` (JP `0xA90BF0`) sem chamada, salto nem ponteiro; no KR vinha da compra do item de upgrade |

## Eventos desligados no próprio cliente (não aparecem)
| JP | KR | o que é |
|---|---|---|
| 0x6E, 0x6F | 0x6E, 0x6F | combinações do evento de Valentine (`NtValentineEventDlg`) |
| 0x8D, 0x8E | 0x8A, 0x8B | Point Event (trava 0x16) |
| 0xE1, 0xE2 | 0xE3, 0xE4 | Mission Event (trava 0x80) |
| 0xE3 | 0xE5 | Bingo (trava 0x82) |

## Sem efeito (podem ser ignorados em silêncio)
| JP | KR | o que é |
|---|---|---|
| 0x01 | 0x01 | `WNetworkSystem::SendTTL` (manter a conexão) |
| 0x88 | — | resposta do anti-cheat (CSAuth2) |
| 0x8C | 0x89 | navegador interno fechado (`CBrowser::Close`) |
| 0x91 | 0x8E | nick do portal Paran (só KR) |
| 0x9A | 0x97 | mensagem do mascote de PC bang |

## Só do JP, ainda não identificados
0x68 (`0xAA4DD0`), 0x83 (`0xAF5E00`, a função do 0x04 de entrar no canal), 0x89 (`0x721420`, função de depuração),
0xA4, 0xDF (`0xB06460`, lobby), 0xED (`0x594F90`), 0xF4 (`0x9BE990`), 0xFD (`0xB3C7D0`).

## Servidor -> cliente (o que o servidor mandou nas sessões reais)
Fonte: `logs/pacotes` (Logging.PacketHex), 965 sessões no game server JP de 14 a 19/10: **110 ids**. Cada um passou pelo
despacho dos 10 tratadores de pacote do exe (simulando a árvore de comparações e as tabelas de salto a partir do id):
**108 caem num tratador**. Os outros dois:
- **0x3F**: a saudação da conexão, tratada antes do despacho (certo).
- **0x7A** (KR 0x78, caixa de presentes, mandado em todo login): o JP não tem tratador; o servidor não manda mais ao JP.

Formato: cruzados com `jp983-protocolo/rx_cmp.tsv` (leituras do 983 × do 645). Os que mudaram e o servidor manda já
têm conversão (0x15C amigos, 0x20E, 0x21B Papel) ou são do JP (0x216, 0x21D, 0x21E, 0x257, 0x264); 0x131 (medidor do
Treasure Hunter) e 0x68 (compra) batem com o tamanho mandado. Isto confere id e formato pela tabela, não o tamanho exato
lido em cada caminho do tratador.

**KR 0xEA (JP 0xEC), compra na loja pessoal**: estava bloqueado (NotConvertedYet) e é a resposta do comprador e o aviso
do vendedor; agora convertido (item da loja 170 -> 172, item comprado 168 -> 196).

## O que o 983 sabe receber e o servidor nunca manda (19/10/2026)
456 ids tratados nos despachantes do 983 (`pacotes_recebe_983.tsv`) contra os que o servidor pode mandar (constantes S* do KR
pela ServerId + ids do JP): **196 nunca mandados**, classificados um a um (código do cliente KR, exe, referência) em
`jp983-protocolo/s2c_nunca_mandados.tsv`: 35 falta, 72 menor, 26 evento travado, 15 já mandados (falso positivo da busca),
15 mortos, 5 PC bang/só KR, 28 sem gatilho achado.

**Tabela de ids do tradutor (ServerId) com buracos**, que descartam pacotes que o servidor já manda:
- KR 0x108..0x113 é KR + 2 (só 0x109, 0x10D e 0x111 estavam nas exceções): motivo da desconexão, últimas 5 partidas,
  bloqueios, primeiro acesso, janela de aviso, tabela de voz.
- KR 0x11E..0x122 é KR + 8 (a faixa começa no 0x11E, não no 0x123): inclui o resultado de abrir caixa no My Room (0x121).

**Feito em 19/10/2026**: buracos da ServerId; login de conta nova (0x0E, 0x0F, 0x11); caixa do My Room (0xB2 -> 0x129,
conteúdo em `Boxes` da config); fim no 1º buraco sem placar (0x65/0x67); Quick Start (0x132/0x133/0x135; 0x134 e 0x136 não
são necessários); fim da tacada para a sala (0x12F -> 0x1F7); últimos jogadores do perfil (0x9C -> 0x10E, migração 023);
aviso de caddie de férias no login (0xD2/0xD4); apagar item (0xC3/0xC5), denúncia (0x92/0x94) e fita de replay (0xA2/0xA4).

**Corrigido em 27/10/2026 — ordem do início da partida (taxa antes dos jogadores)**: a taxa de pang (KR 0x75 = JP 0x77),
que entrou em 19/10 (97f39f0), ia DEPOIS do 0x74 (JP 0x76). O cliente trata a taxa com `ClearRivalVars` (983 009c21ae:
memset dos 4 sUserInfo da partida), então apagava o taco/personagem que o 0x76 acabara de gravar: o JP 983 original caía
ao entrar no Practice (AV em CGolfPlayer::HandleMsg 00745a46, FindClubSet NULL) e o cliente reconstruído ficava com o
personagem preto, sem linhas e caía na tacada. Agora `GameHandler.Start` manda 0x75 → 0x74 → 0x50 (teste
`GameStartSendsMascotSeedAndRate` confere a ordem). Conferido com o robô: original e reconstruído no Practice com
personagem, taco e caddie. **Ressalva**: a mudança está na lógica comum, então vale também para o cliente KR 645, que não
foi testado com a ordem nova (o 645 é da mesma família de código; se der problema nele, trocar a ordem só no `JpTranslator`).

**Faltas, por importância**
9. Início de jogo com a taxa (0x77 = KR 0x75) e semente do efeito de mascote (0x16A).
10. Raspadinha nova (0x1FD), Ticket Report do torneio (0x12A), uso de móvel no My Room (0x17E/0x17F), UCC de todos (0x1B1), ícone de ausente (0x8E),
    moeda do Special Shuffle (0x198), bloqueio de chat (0xAC).
11. Só GM: modo galeria/espectador (0x98, 0x99, 0x9D, 0xBC–0xBE).

O 0x194 (resposta da melhoria de caddie) entra como código morto: nada no 983 abre a janela (ver acima).
