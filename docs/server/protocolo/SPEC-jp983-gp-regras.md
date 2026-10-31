# Grand Prix JP 983: regras (10 s / 15 s, tacada especial), penalidade, buracos especiais, 0x199 e pacotes que faltam

Marcas: **[EXE ✓]** conferido desmontando `<pasta do jogo>\ProjectG.exe` (scripts `tools/disa.py`, `xref.py`,
`findimm.py`); **[REF]** servidor de referência (só comportamento); **[IFF]** lido de `data/pangya_jp.iff`; **[?]** incerto.
`shareddoc` = o objeto global em `[0xE47E2C]`. `RoomInfo` = a cópia do RoomInfo JP (221 B) que o cliente guarda; os campos
`type_extend` (off. 196) e `artefato` (off. 197) aparecem como `shareddoc+0x109B8` e `+0x109B9`, e o bloco GP (off. 205)
como `+0x109C1` (o `TimeHole×1000` do bloco, off. 213, é `+0x109C9`).

## Resumo (o que muda em relação à SPEC-jp983-grandprix.md)

1. **O cliente aplica as duas regras de penalidade sozinho** [EXE ✓]: ao montar o resultado da tacada (0x1B) numa sala de
   GP (`type_extend` 0x14 ou 0x15) ele testa a regra (`artefato`), soma **+1 no próprio placar do buraco**, mostra a
   mensagem (string 0x754F, cor 0xFFFF7878) e põe **1 no 38º byte** do 0x1B. O servidor não precisa de relógio próprio:
   basta ler esse byte (e, para a regra da tacada especial, conferir os bits) e somar +1 nas tacadas do buraco, senão o
   placar do servidor (0x6D, posição final, prêmios) fica 1 abaixo do que o jogador vê.
2. A regra 0x1A000267 ("Power Shot applies Penalty" no Item.iff) **não** olha o power shot: o cliente penaliza quando o
   u32 de estado da tacada (offset 0x1F do 0x1B) tem algum dos bits **0x0E** = tomahawk (2), spike (4), cobra (8) [EXE ✓].
3. Nas regras de tempo o cliente **troca o tempo da tacada** por 10 000 / 15 000 ms ao receber o 0x52 [EXE ✓]; é isso que
   faz o relógio (CClock) andar e o cliente mandar o **0x22** a cada vez. Com o `u32 tacada` = 0 que mandamos no 0x52
   do GP, sem essa troca não haveria 0x22 (o CClock só manda com tempo ≠ 0).
4. 38º byte no S->C 0x64: nenhum leitor no cliente (golfrule) [EXE ✓, busca por `[reg+0x25]`]; no GP (torneio) o 0x64
   nem é mandado. Pode continuar 0.
5. Buracos especiais: 9 grupos (`TypeID_Link`), não só os Rookie [IFF]. O servidor monta a sequência (mapa, buraco) da
   tabela e o 0x52 do JP leva o mapa por buraco.
6. 0x199 (vazio) só liga uma marca no cliente (`shareddoc+0x11640 = 1`, zerada pelo 0x52); não achei quem lê [EXE ✓/?].
   Barato e a referência manda em todo torneio/GP: mandar ao terminar o último buraco, antes do 0x6D.
7. 0x230/0x231/0x77 no início: a referência manda em **todo** início de jogo (`Room.requestStartGame`), não só no GP; o
   stroke do JP já funciona sem eles. O 0x77 faz o cliente mandar um **0x0C sub 7** (equipamento) — não mandar.

---

## 1. Regra de 10 s / 15 s (0x1A000268 / 0x1A00029E)

### 1.1 O C->S 0x22

- Mesmo id no KR e no JP (`ids_cliente.tsv`: 0x22 = 0x22, unidade `clock`). Sem corpo.
- JP [EXE ✓] `CClock::HandleMsg` (0x717CC0): na mensagem 0x261 (início do relógio) põe `+0x25 = 1` e manda 0x22 se:
  conectado (`0x633000`), `shareddoc+0x1069C` (tempo da tacada em ms) ≠ 0, bit 1 de `shareddoc+0x812` = 0
  (não é espectador [?]) e `0x530C00(1)` (é a minha vez [?]). Na 0x262 só zera `+0x25`.
- KR: `docs/server/protocolo/SPEC-ingame.md:174` — "CClock::HandleMsg @0x4cc3e6 quando o relógio da tacada do jogador local
  começa". Hoje o KR trata como `CTurnClock` sem resposta (`GameHandler.Play.cs`, `case CTurnClock or CTeamHoleIn or
  CMatchHoleIn: p.Skip`), inclusive no torneio (o `HandleMass` nunca o vê porque o `case` acima retorna antes).

### 1.2 De onde vem o tempo [EXE ✓]

Handler do 0x52 (despachante 0x9C20A0, caso 0x9C27F1): `u8 mapa, u8 tipo_show (+0x10490), u8 ordem (+0x10699 [?]),
u8 buracos, u32 troféu (+0x10494), u32 tacada (+0x1069C), u32 jogo (+0x106A0)`. Depois:

```
if (0x46E6D0(type_extend))            // 0x14 <= type_extend <= 0x15  (GP)
{
    if (0x546900(artefato))           // artefato == 0x1A000268 || == 0x1A00029E
        shareddoc+0x1069C = 0x546920(artefato) * 1000;   // 10 -> 10000 ms, 15 -> 15000 ms
    shareddoc+0x106A4 = RoomInfo.gp.TimeHole_ms;          // relógio do buraco (vem do RoomInfo, não do 0x52)
}
```

Ou seja: o relógio de 10/15 s e o 0x22 dependem só de o RoomInfo da sala (0x49/0x4A/0x86) ter `type_extend = 0x14` e
`artefato = regra` — o tradutor já faz isso (`JpTranslator.Room.cs` ~l.325, `b[196] = Jp983.GrandPrixType`).
O relógio do buraco (`TimeHole`) também é lido do bloco GP do RoomInfo.

### 1.3 Como a penalidade é calculada no cliente [EXE ✓]

Montagem do 0x1B (`0x6847A0`, chamada por `0x684BA0`, que manda `EB(0x26)`):

```
if (type_extend - 0x14 <= 1) {                         // GP
    n = 0x546A60(artefato, &resultado, inicioDaVez);   // 0 ou 1
    if (n) {
        resultado[0x25] = 1;                           // 38º byte
        placar_local[buraco] += n;                     // valor cifrado do placar do próprio jogador
        mensagem 0x754F (vermelha);
    }
}
0x546A60(regra, r, t0):
    0x1A000267             -> (r[0x1F] & 0x0E) != 0                 // tomahawk / spike / cobra
    0x1A000268, 0x1A00029E -> limite(regra) <= (agora_ms - t0) / 1000 // limite 10 ou 15
    outra                  -> 0
```

`t0` = `[golfrule+0x648]`, gravado com o relógio do jogo (`[0xF02344]`, ms) em "play_my_turn" (0x67F4CC) e em outros
dois pontos (0x68CB5A com um estado 4; 0x6A5312) [?: o momento exato — se a conta inclui o voo da bola — não ficou
claro; para o servidor não importa porque ele confia no byte].

### 1.4 Quando para e como chega ao placar

- **Cliente**: a penalidade entra no placar local na hora da tacada; o 38º byte vai para o servidor. Não existe pacote
  S->C de "penalidade".
- **Referência** [REF `GrandPrix.cs`]: relógio no servidor — `requestStartTurnTime` (0x22) liga `startTimeRule` (10/15 s);
  `requestInitShot` (0x12) e o fim do buraco param (`stopTimeRule`); estourou com `init_shot == 0` → `penalidade++`.
  Regra da tacada especial: `requestTranslateSyncShotData` (0x1B) soma +1 se `display.special_shot` (bit 3 do u32 em
  0x1B; o cliente usa o u32 em 0x1F, ver §2). No `finishHole`: se não houve desistência nem tempo do buraco esgotado,
  `tacada_num += penalidade`; isso vai no **0x6D** (`u8 tacadas, i32 placar`) e na posição final. A referência ignora o
  38º byte.
- **Nós hoje**: nada. O 0x22 é descartado, o 38º byte é ignorado (`DecodeResult` lê 37), o `TourneyGame` não soma
  penalidade. Resultado: o 0x6D do servidor mostra 1 tacada a menos por penalidade e a posição/prêmio sai errada.

### 1.5 O que implementar

Confiar no byte do cliente (é o que ele mostra) e, na regra da tacada especial, também nos bits (independe do byte):

- `penalidade da tacada = 1` se `rule ∈ {0x1A000267, 0x1A000268, 0x1A00029E}` e (`byte37 ≠ 0` ou
  (`rule == 0x1A000267` e `(ShotFlags & 0x0E) ≠ 0`)). No máximo 1 por tacada (o cliente soma 1).
- Somar no `FinishHole` como a referência: só se o buraco não terminou por desistência (par + 4) nem pelo relógio do
  buraco (`HoleClock`).
- Opcional (anti-trapaça): guardar a hora do 0x22 e logar quando `agora − hora_0x22 > limite + 2 s` na tacada (0x12)
  sem o byte ligado. Não penalizar pelo servidor (o cliente não veria a tacada a mais).

---

## 2. Regra da tacada especial (0x1A000267)

- Bits do 0x1B (decifrado; layout [REF `ShotSyncData`] = [EXE ✓] montagem em 0x6847A0):

| off | tam | campo |
|---|---|---|
| 0x00 | 4 | oid |
| 0x04 | 12 | x, y, z |
| 0x10 | 1 | estado (2 jogável, 3 OB/água, 4 no buraco/perto, 5 área proibida) |
| 0x11 | 2 | bunker, ? |
| 0x13 | 4 | pang |
| 0x17 | 4 | bônus |
| 0x1B | 4 | display (`+0x4E0` do jogador): bit 3 special_shot, bit 8 acerto_hole, **bit 13 clear_bonus** (0x2000), ... |
| 0x1F | 4 | estado da tacada: 0x02 tomahawk, 0x04 spike, 0x08 cobra, 0x10/0x20 spin frente/trás, 0x40/0x80 curva, 0x800 sem setas, **0x1000 power shot, 0x2000 power shot duplo**, 0x100000.. tipo do taco (madeira/ferro/PW-SW/putter), 0x4000000 ? |
| 0x23 | 2 | tempo da tacada (`[0xE47E94]`) |
| 0x25 | 1 | **penalidade do GP** (0 ou 1) |

- Penalidade: `(u32@0x1F & 0x0E) ≠ 0` [EXE ✓ 0x546950]. Power shot simples sem tomahawk/spike/cobra **não** penaliza.
- Nossos campos: `DecodeResult` já passa `res[0x1B]` como `ShotResult.Display` e `res[0x1F]` como `ShotResult.ShotFlags`.
- Uso no arquivo [IFF]: 0x1A000267 em 48 GPs, 0x1A000268 em 24 (grupo 0x180200 "クイックショット選手権", 120 s/buraco),
  0x1A000266 em 24, 0x1A000265 em 4 (Rookie 0x100..0x400), **0x1A00029E em nenhum** (nem existe no Item.iff).

---

## 3. O 38º byte do resultado (0x1B C->S / 0x64 S->C)

- **C->S 0x1B**: o JP manda exatamente 38 bytes cifrados (XOR `chave_da_sala[i % 16]`), **sem** os 16 da chave que o KR
  manda depois [REF `requestReadSyncShotData` exige 38; EXE ✓ `EB(0x26)`]. O tradutor não tem `fromClient[0x1B]`: vai
  intacto; o KR (`GameHandler.Play.cs`, `DecodeResult`) vê 38 < 37 + 16, usa `r.Key` e decifra só 37. O 38º byte está em
  `rest[37] ^ key[37 % 16]` (= `key[5]`).
- **S->C 0x64** (KR 0x62, só nos modos por vez): o tradutor completa 37 -> 38 com 0
  (`JpTranslator.Room.cs`, `toClient[0x62]`). O handler (CGolfTask 0x6A0090) guarda os 38 bytes por jogador em
  `[shareddoc+0x11024]+0x3C4 + i*0x26` e avisa o GolfRule (msg 0x94); não achei leitura de `+0x25` desse vetor
  [EXE ✓ varredura de `byte ptr [reg+0x25]` em 0x660000-0x6B2000: só escritas]. GP é torneio: não há 0x64. **Deixar 0**
  (ou repassar o valor do cliente; indiferente).
- No GP o valor que importa é o do C->S: é o que entra na conta de tacadas do servidor (§1.5).

---

## 4. Buracos especiais (GrandPrixSpecialHole.iff)

### 4.1 Layout [EXE iff_983.tsv 0x14 / REF `GrandPrixSpecialHole.cs` / IFF]

Cabeçalho `u16 n (84), u16, u32 versão (13)`; registro de 20 bytes:

| off | tipo | campo |
|---|---|---|
| 0 | u32 | Enable |
| 4 | u32 | TypeID = **TypeID_Link** do GP (GrandPrixData off. 8) |
| 8 | u32 | HolePOS (1..n: posição na partida) |
| 12 | u32 | Map (mapa do buraco) |
| 16 | u32 | Hole (número do buraco nesse mapa) |

### 4.2 Conteúdo [IFF] — `HolePOS:Map/Hole`

| Link | GPs | buracos |
|---|---|---|
| 0x100 | Rookie "Control Practice Tournament" (3H, 1 m de vento) | 1:11/2 2:16/6 3:20/11 |
| 0x200 | Rookie "Comet Landing Practice" (3H) | 1:0/2 2:2/5 3:19/16 |
| 0x300 | Rookie "Hills Practice" (3H) | 1:8/5 2:2/15 3:11/16 |
| 0x400 | Rookie "Course Practice" (6H) | 1:11/3 2:0/5 3:15/6 4:1/7 5:2/11 6:3/17 |
| 0x500 | Rookie "Beginner Grand Prix I" (6H) | 1:11/1 2:0/4 3:2/6 4:14/16 5:11/12 6:3/3 |
| 0x80B00 | 24 GPs 0x80B01.. "お花見パンヤツアー（9H）" (120 s) | 9 buracos, 1 a 9 em mapas 5/11/13/15/19/20 |
| 0x80C00 | 23 GPs 0x80C01.. "お花見パンヤツアー" (18H) | 18 buracos, buraco i no mapa da tabela |
| 0x180100 | 24 GPs 0x180101.. "パンヤオープン" (18H, 180 s) | 18 buracos mistos |
| 0x180200 | 24 GPs 0x180201.. "クイックショット選手権" (18H, 10 s) | 18 buracos mistos |

Rookie 0x600..0x800 não têm (mapa e ordem normais). A SPEC atual (§4.4: "só os GPs Rookie usam") está errada.

### 4.3 Como o servidor monta [REF `CourseManager.init_seq` / `init_hole` / `makePacketHoleInfo`]

- Se a sala é GP e há registros para o `TypeID_Link`: a sequência é a da tabela (mapa, buraco), completada até 18 com
  `(mapa da sala, nº seguinte)`. A referência ordena por **Hole** — no 0x500 isso troca a ordem (1,3,4,6,12,16 em vez de
  1,4,6,16,12,3); ordenar por **HolePOS** [?: o nome do campo indica; não há como confirmar sem servidor oficial].
- O pin de cada buraco: sorteio 0..2 como nos outros (`Random % 3`). Vento/clima normais (com a regra de vento).
- O mapa da sala (`RoomInfo.course` e 1º byte do 0x52) continua o `Course` do GrandPrixData (sem 0x80) [REF].
- Bots (0x256): a referência põe `mapa = mapa da sala & 0x7F` e `buraco = número do buraco da sequência` [REF].

### 4.4 O que vai no 0x52 (JP)

`u8 mapa (Course do GP), u8 tipo_show (4), u8 ordem, u8 buracos, u32 troféu, u32 tacada (0), u32 jogo (0)`, 18 ×
`{u32 semente/id, u8 pin, u8 **mapa do buraco**, u8 **nº do buraco**}`, `u32 semente`, 18 × `u8 0` (sem cubos/moedas).
O JP já tem o mapa por buraco em todo 0x52; o tradutor hoje só usa mapa por buraco no Special Shuffle
(`perHoleCourse = k[1] == ShuffleCourseMode`), senão `k[0] & 0x7F`.

---

## 5. 0x199 no último buraco e o bônus de "clear"

- **Cliente** [EXE ✓]: S->C 0x199 (vazio, despachante 0x9C20A0 caso 0x9D26A7) = `shareddoc.SetFlag11640(1)` (0x62BFA0).
  O 0x52 zera (0x9C2BA1) e o início do jogo também (0x64AC4E). O getter (0x62BFB0) não tem chamada direta e não há
  outra leitura de `+0x11640` [?: pode ser chamado por tabela]. Conclusão: não trava nada; é a marca "terminei o
  último buraco".
- **Referência** [REF `GrandPrix.checkEndShotOfHole` / `changeTurn`, igual em `TourneyBase` e `VersusBase`]: no fim da
  tacada (0x1C) que fecha o buraco (acertou, desistiu ou tempo do buraco), se `findHoleSeq(buraco) == qntd_hole`:
  1. **0x199** só a esse jogador;
  2. se o resultado tem `display.clear_bonus` (bit 13, 0x2000): `bonus_pang += clear_bonus(mapa da sala) × buracos / 2`;
  3. `finishHole` (soma as penalidades) → `changeHole` → 0x6D (`u8 1`), 0x40 tipo 16, 0x6C estado 2, etc.
- **clear_bonus por mapa** [REF `MapSystem`, número do mapa = enum `ROOM_INFO_COURSE`]: 0 Blue Lagoon 20, 1 Blue Water 50,
  2 Sepia Wind 55, 3 Wind Hill 80, 4 Wiz Wiz 65, 5 West Wiz 24, 6 Blue Moon 50, 7 Silvia Cannon 70, 8 Ice Cannon 40,
  9 White Wiz 55, 10 Shining Sand 40, 11 Pink Wind 20, 13 Deep Inferno 80, 14 Ice Spa 20, 15 Lost Seaway 20,
  16 Eastern Valley 40, 17 Chronicle 1 Chaos 360, 18 Ice Inferno 70, 19 Wiz City 40, 20 Abbot Mine 40, 21 Mystic Ruins 40,
  64 Grand Zodiac 0. Ex.: Blue Lagoon 9H = 20 × 9 / 2 = 90. É só pang no servidor (o cliente não recebe pacote dele;
  vai no bônus do 0x6D e no pang final).
- **Nós hoje**: não mandamos 0x199 nem somamos o bônus (nem no torneio comum).

---

## 6. Outros pacotes da referência que não mandamos

| pacote | quando [REF] | cliente [EXE ✓] | precisa? |
|---|---|---|---|
| 0x230 (vazio) | todo início de jogo (`Room.requestStartGame`), broadcast | msg 1 para a tarefa atual (`0x47D230`) | não (o stroke JP funciona sem) |
| 0x231 (vazio) | idem, logo depois | msg 2 para a tarefa atual | não |
| 0x77 `u32 taxa de pang` | idem | grava `shareddoc+0x10584`, msg 0x2C, chama `0x643830`, e **manda C->S 0x0C `u8 7` + 16 B** (4 × u32 do equipamento; zera o caddie se ...) e, se `type_extend == 0x12`, troca CLobbyTask→CGolfDoc | **não mandar** (gera um 0x0C sub 7 que o KR não trata) |
| 0x40 tipo 16 | quem termina (`GameBase.sendFinishMessage`), broadcast: `u8 16, str nick, u16 0, i32 placar, u64 pang, u8 assist` | aviso no chat "fulano terminou" | cosmético; barato (mandar no `RivalState(p, 2)` do GP) |
| 0xCE `u8 0, u16 n, n × u32` | fim, a cada um | itens que caíram | **já sai**: KR 0xCC (`GameHandler.GameEnd.cs`, `SMyItemsWon`) -> JP 0xCE pelo deslocamento +2 |
| 0x79 | fim | placar do torneio | já sai (KR 0x77 + 24 zeros) |
| 0x199 | último buraco | marca | mandar (§5) |
| 0x255 | quando **todos** os humanos fecharam o buraco | CGolfTask 0x6A4687 | hoje vai a cada um junto com o 0x65 (`toClient[0x63]`): funciona; manter |
| 0x1F7 `i32 oid, u8 buraco, 87 B` | eco do C->S 0x12F (fim da tacada) em torneio/GP, broadcast | CGolfTask 0x6A3AC3 | não (posição dos rivais já vai no 0x6C; o 0x12F é silenciado em `JpTranslator.JpOnly.cs`) |
| 0x244 `u32 0`, 0x24F `u32 0` | depois do 0x06 em todo modo | 0x24F lê D4; 0x244 sem caso no despachante lido | não (o stroke funciona sem) |

O C->S 0x199 é outra coisa (anéis com patas, `requestActiveRingPawsRingSetJP`), já tratado em `JpTranslator.Effects.cs`.

---

## 7. Plano de implementação

Regra 13 (sem LINQ em `src/`). Ordem sugerida: 7.1 → 7.2 → 7.3 → 7.4; cada passo testável sozinho.

### 7.1 Penalidade (regras 0x267/0x268/0x29E)

1. `src/Pangya.Domain/Rooms/RoomGame.cs`: `ShotResult` ganha `byte Penalty = 0` (último parâmetro opcional).
2. `src/Pangya.Protocol.KR645/Game/GameHandler.Play.cs`, `DecodeResult`: depois de decifrar os 37,
   `byte pen = rest.Length == InGameOutput.ResultLength + 1 ? (byte)(rest[ResultLength] ^ key[ResultLength % 16]) : 0;`
   e passar `pen` no `ShotResult`. (O KR nunca manda 38: o pacote dele tem 37 + 16.)
3. `src/Pangya.Domain/Rooms/GrandPrix.cs`, `GrandPrixRules`:
   `public static bool Penalized(int rule, in ShotResult r) => rule switch { PowerShotPenalty => r.Penalty != 0 || (r.ShotFlags & 0x0E) != 0, TenSeconds or FifteenSeconds => r.Penalty != 0, _ => false };`
   e `public static int ShotSeconds(int rule) => rule == TenSeconds ? 10 : rule == FifteenSeconds ? 15 : 0;`
   (renomear `PowerShotPenalty` para `SpecialShotPenalty` no comentário: são tomahawk/spike/cobra).
4. `src/Pangya.Domain/Rooms/MassGame.cs`:
   - `MassPlayer`: `public int Penalty { get; set; }` (do buraco atual) e `public bool HoleTimedOut { get; set; }`.
   - `TourneyGame.Result`: depois de aceitar o resultado, `if (Room.GrandPrix is { } gp && GrandPrixRules.Penalized(gp.Def.Rule, r)) p.Penalty++;`
   - `TourneyGame.HoleClock`: marcar `p.HoleTimedOut = true` antes do `FinishHole`.
   - `TourneyGame.FinishHole`: no começo,
     `bool gaveUp = p.Strokes[i] >= ParOf(hole) + GiveUpOverPar && !(p.HasResult && p.ResultState == ShotResult.StateHoled);`
     `if (p.Penalty > 0 && !gaveUp && !p.HoleTimedOut) p.Strokes[i] += p.Penalty;` e zerar `Penalty`/`HoleTimedOut`
     (inclusive para bots). Fica antes do `p.Total += st` para o 0x6D (`MassOutput.RivalHole`) e a posição final saírem certos.
   - Não mexer no `ShotFinished` (a desistência continua contando só as tacadas reais, como a referência).
5. 0x22: nenhuma mudança obrigatória. Opcional: em `HandlePlay`, no torneio, `case CTurnClock` gravar
   `mp.TurnClockAt = Stopwatch.GetTimestamp()` e, no `Shoot`, logar se passou do limite sem byte (diagnóstico).
6. Testes (`tests/`): `DecodeResult` com 38 bytes (penalidade 1/0); `TourneyGame` com sala GP regra 0x268: tacada com
   `Penalty = 1` no buraco par 4 com 4 tacadas → 0x6D com 5 / placar +1; regra 0x267 com `ShotFlags = 0x1004` (spike +
   power) → +1, com `0x1000` → 0; desistência não soma.

### 7.2 Buracos especiais

1. `src/Pangya.Domain/Rooms/GrandPrix.cs`: `GrandPrixDef` ganha `(byte Course, byte Hole)[] SpecialHoles` (vazio por padrão;
   colocar como propriedade `init` para não quebrar os construtores existentes).
2. `src/Pangya.Protocol.JP983/Jp983GameData.cs`, getter `GrandPrix`: ler `GrandPrixSpecialHole.iff` (`Rows(...)`, 20 B:
   `I32(r,0)` enable, `I32(r,4)` link, `I32(r,8)` pos, `I32(r,12)` mapa, `I32(r,16)` buraco) num
   `Dictionary<int, List<(int Pos, byte Course, byte Hole)>>`, ordenar cada lista por `Pos` (laço simples) e preencher
   `SpecialHoles` de cada def pelo `Link`.
3. `src/Pangya.Domain/Rooms/RoomManager.cs`, `PrepareStart`: depois de montar `order`/`seeds`, se
   `room.GrandPrix?.Def.SpecialHoles is { Length: > 0 } sh`: `HoleCourses = new byte[18]`; para `i < sh.Length`:
   `order[i] = sh[i].Hole; HoleCourses[i] = sh[i].Course`; para o resto `order[i] = i + 1; HoleCourses[i] = CoursePlayed`
   (como a referência). Não usar o `ShuffleCourses`. `Field` já é null no GP.
4. `src/Pangya.Protocol.JP983/Game/JpTranslator.Room.cs`, `GameInit`: `bool gpHoles = room?.GrandPrix != null &&
   room.HoleCourses.Length == 18;` e `byte course = perHoleCourse ? k[o + 4] : gpHoles ? room!.HoleCourses[i] : (byte)(k[0] & 0x7F);`
   (o KR do torneio continua mandando 5 bytes por buraco; o número do buraco já vem do `HoleOrder`).
5. Conferir no KR que nada do torneio depende de `HoleCourses.Length > 0` para outra coisa:
   `GameHandler.GameEnd.cs:56` usa `r0.HoleCourses.Length > 0` (registro de mapa do Special Shuffle) — trocar a condição
   para `(int)r0.Settings.Mode == RoomManager.ShuffleCourseMode` ou excluir GP.
6. Bots: `GrandPrix.MakeBots` já recebe `room.HoleOrder` (chamado no `GamePlayers`, depois do `PrepareStart`): os números
   saem certos; o mapa fica o do GP (como a referência).
7. Teste no cliente: Rookie 0x100 → 3 buracos em Pink Wind 2, Eastern Valley 6, Abbot Mine 11 [?: confirmar nomes pelo
   Course.iff]; Rookie 0x500 na ordem por HolePOS.

### 7.3 0x199 e bônus de clear

1. `IMassOutput`: `void LastHoleDone(MassPlayer p) { }`.
2. `TourneyGame.FinishHole`: se `p.HoleIndex == HoleCount - 1` (antes do `++`) e `!p.IsBot`: bônus de clear e
   `Output.LastHoleDone(p)` **antes** do `Output.RivalHole` (ordem da referência: 0x199, depois 0x6D).
   - Bônus: `if (holed && (p.LastDisplay & 0x2000) != 0) p.Bonus += (uint)(Course.ClearBonus(Room.CoursePlayed) * HoleCount / 2);`
     guardar `LastDisplay = r.Display` no `Result`. Tabela do §5 num `static` de `Pangya.Domain` (ex. `Rooms/ClearBonus.cs`,
     `byte -> int` por `switch`). Vale para todo torneio (a referência faz o mesmo no `TourneyBase`) [?: o KR 645 pode já
     somar no cliente — conferir num torneio KR se o pang final muda antes de ligar fora do GP; ligar primeiro só no GP].
3. `src/Pangya.Protocol.KR645/Game/MassOutput.cs`: `LastHoleDone` → `if (p.RoomPlayer.Session is GameHandler { GrandPrix: { } sink }) sink.LastHole();`
   e `IGrandPrixSink.LastHole()` default vazio; `JpSystems.GrandPrix.cs`: `public void LastHole() => conn.SendTranslated(new PacketWriter(0x199));`
   (0x199 está fora das faixas: já sai com o id do JP pelo `SendTranslated`, como o 0x259). Para torneio comum do JP
   também, se quiser seguir a referência: chamar o mesmo pelo `JpSystems` sem exigir GP.

### 7.4 0x40 tipo 16 (opcional, cosmético)

`IGrandPrixSink.Finished(MassPlayer)` chamado no `RivalState(p, 2)` do GP; `JpSystems` manda a todos da sala
`0x40 u8 16, Str(nick), U16(0), I32(Score), U64(Pang + Bonus [?: a ref usa só pang]), U8(assist)` com `SendTranslated`
em cada `GameHandler` da sala.

### 7.5 Não fazer

- 0x77 / 0x230 / 0x231 no início (§6).
- Relógio de 10/15 s no servidor como a referência (o cliente já penaliza e mostra; um relógio do servidor só criaria
  divergência).

## Pontos [?] para testar no cliente

1. Sala GP com regra 0x268: o relógio mostra 10 s? Chega 0x22 a cada vez? Uma tacada lenta mostra a mensagem 0x754F e
   o 0x1B decifrado tem `[37] = 1`?
2. Regra 0x267: spike com power shot → `[37] = 1`; power shot sem especial → 0.
3. Rookie 0x100: os 3 buracos carregam nos mapas da tabela com o 1º byte do 0x52 = 0 (sem 0x80)? Se o cliente carregar
   tudo no mapa 0, testar `0x80 | Course` no 1º byte (como o Special Shuffle).
4. 0x199: algum efeito visível (nenhum esperado).
5. Bônus de clear: o bit 0x2000 aparece no display do último buraco? O pang final do cliente bate com o do servidor?
