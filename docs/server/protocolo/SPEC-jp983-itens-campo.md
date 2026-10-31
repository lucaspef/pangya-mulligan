# JP R7.983 — itens de campo (moedas de pang e Spinning Cube)

Fontes: decomp do exe 983 (lane B1588, `out/B1588/FIELD_ITEMS.md` no container `jp983src`; **[EXE]**), servidor GB da
comunidade (`Server/GB/GameServer`: `CoinCubeLocationUpdateSystem`, `CubeCoinSystem`, `CourseManager.makePacketHoleSpinningCubeInfo`,
`TourneyBase.calcule_shot_to_coin`; **[GB]**), **[I]** inferido. KR 645: `SPEC-modes.md` "Wiz City".

## Diferença para o KR 645
O 645 manda só tipo/índice e cada cliente sorteia a posição (triângulos `ct_green_b`/`ct_booster`). O 983 **não gera posição**
[EXE]: o caminho de sorteio saiu do `GroundItemMan::OnInit` (0085e2c0); tudo vem do servidor no 0x52. Por isso `Zeros(18)`
(18 contagens zero) não mostrava nada.

## Fim do S->C 0x52 (e do 0x113 {4,4} da intrusão) — CGimmickContainer::LoadFromPacket 009103e0 [EXE]
```
u32 seed                         // + buraco atual semeia o MTRand do GroundItemMan: só decide o jackpot do ímã
18 × { u8 n; n × registro 0x24 } // o nº do grupo não é usado: vale o campo +0x11
```
| ofs | tipo | campo | o que o servidor manda |
|---|---|---|---|
| +0x00 | i32 | tipo | 0 moeda (coin.pet), 1 caixa (ct_event_05.pet) |
| +0x04 | u32 | índice | volta no 0x1C; servidor: `vez << 8 \| índice no buraco` (único na partida) |
| +0x08 | u32 | flag | 0 (o cliente usa: 0 livre, 1 pego) |
| +0x0C | u32 | ? | sem leitor no 983; mandamos o curso (como o GB) |
| +0x10 | u8 | ? | sem leitor; mandamos o buraco físico (como o GB) |
| +0x11 | u8 | vez | **índice 0-based na ordem de jogo** (lista de buracos do próprio 0x52) |
| +0x12 | u16 | 0 | |
| +0x14 | f32×3 | x, y, z | posição no mundo |
| +0x20 | u32 | textura | 0 = com ímã; != 0 gira; **4 = registro descartado**. Mandamos o `tipo_location` do GB: 0 borda do green, 1 carpete, 2 ar, 3 chão |

Ajuste do cliente sobre o y (0085df20): moeda y+0,1; caixa y−18 (Wiz City: y−3). O cliente aceita itens nos tipos de jogo
0, 1, 4, 5, 6, sem filtro de curso. Registros para outros tipos chegam e são ignorados.

## Pegar: C->S 0x1C (CGolfRule::EncodePacket 00688e80) [EXE]
`u8 é_minha_tacada; u8 n; n × {u8 tipo; u32 índice}` (5 bytes por item; o KR tem 7: + u8 quantidade, u8 textura).
O tradutor confere cada índice (existe, tem posição, é do buraco atual de quem tacou) e passa ao KR
`{tipo, índice no buraco, 1, textura}` (textura do KR: 1 borda do green, 2 o resto). O que não confere vai para o log
("item de campo suspeito"). O KR credita só a cópia de quem tacou, confere o tipo e nunca paga (jogador, buraco, índice)
duas vezes: moeda = 1..50 pang na borda do green, 1..200 no resto (S->C 0xC6 → JP), caixa = 1 Spin Cube.
Não há resposta por item no 983 [I]: o cliente mostra efeito e contador sozinho.

## De onde vêm as posições (`FieldSpots`, tabela `field_spots`, migração 024)
- **Seed do GB** (dump `bk-schema-mssql-complet2022.sql`, `pangya_coin_cube_location`): Wiz City inteiro (curso 19: 195
  moedas na borda do green, 145 no carpete, 15 caixas) e 23 pontos de chão do Shining Sand (curso 10). As demais linhas do
  dump eram cópias das coordenadas do Wiz City em todos os cursos e ficaram de fora.
- **Aprendidos** [GB calcule_shot_to_coin]: no 0x1B aceito (resultado da tacada), a posição final da bola vira ponto de
  moeda se: estado 2 (área jogável), fora do bunker (+0x11 do resultado), tacada ≤ par, a mais de 70 y (224 unidades) da
  bandeira, curso normal (fora do Wiz City, GP e Grand Zodiac). Pontos a menos de 5 y se juntam (soma acertos); no máximo
  100 por buraco. Gravados a cada 10 s (só o que mudou), carregados ao subir.
- O GB põe as caixas fora do Wiz City no ar (topo do voo pela física); aqui a caixa usa um ponto de chão erguido 20 [I].

## Quantidades por buraco
- Wiz City (a do KR, [I] GB getAllCoinCubeInHoleWizCity): buracos 3/12: 5 caixas + 55 moedas; 14: 2 + 46; 18: 3 + 30;
  os outros 20 moedas — limitadas no JP aos pontos do seed (338 itens nos 18 buracos).
- Outros cursos: só buracos com pontos; par 3 → 2, par 4 → 5, par 5 → 8 itens (limitado aos pontos); 1 caixa em 1 de cada
  3 buracos [I]. Sem pontos: sem itens.
- Grand Prix e Grand Zodiac: sem itens. Repetir buraco: os itens só na 1ª vez.
- O KR 645 continua recebendo itens só com a sala toda no Wiz City (ele mesmo gera a posição).
