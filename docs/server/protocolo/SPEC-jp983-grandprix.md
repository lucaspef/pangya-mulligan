# Grand Prix no cliente JP R7.983: especificação para o servidor

Fontes: **[REF]** servidor JP de referência (C#, AGPL): `GameServer/Game/Channel.cs`, `Game/GameModes/GrandPrix.cs`,
`Game/GameModes/Event/RoomGrandPrix.cs`, `Game/Base/TourneyBase.cs`, `Game/Base/GameBase.cs`, `Game/CourseManager.cs`,
`Models/grand_prix_type.cs`, `PacketFunc/packet_func_sv.cs`, `PangyaAPI.IFF.JP/Models/Data/GrandPrix*.cs`.
**[EXE]** = `docs/server/jp983-protocolo/pacotes_recebe_983.tsv`, `pacotes_envio_983.tsv`, `pk_cmp.tsv`, `rx_cmp.tsv`, `iff_983.tsv`
(o tsv de leitura só lista as leituras **diretas** do `case`; o que fica numa subfunção, como laços de lista, não aparece).
**[IFF]** = conferido lendo `data/pangya_jp.iff` (script em `<scripts locais>/gp2.py`, `gp3.py`). **[?]** = incerto.

Convenção: inteiros little-endian; `str` = u16 tamanho + bytes (sem zero); `char[n]` = campo fixo com zeros;
SYSTEMTIME = 8 × u16 (16 bytes). Ids são do **JP** salvo indicação ("KR 0xNN" = id do 645 que o tradutor converte).

---

## 0. Resumo do fluxo

```
Login: lista de servidores com propriedade 0x800 ─► 0x44 termina com u32 propriedade (0x800)
Lobby/canal ─ Start Game ─► diálogo "Grand Prix / jogo normal"
   escolhe Grand Prix ─► C->S 0x176 (vazio)
       S->C 0x46 sub4, 0x46 sub5, 0x47 salas, 0x46 sub1 (broadcast), 0x250 (entrou no lobby do GP)
   escolhe um GP na lista (montada pelo cliente com GrandPrixData.iff + 0x250) ─► C->S 0x179 u32 id do GP
       ok:  S->C 0x4A, 0x49, 0x48 sub0 (+ 0x48 sub1 se a sala já existia), 0x47 (lobby), 0x46 sub3
       erro: S->C 0x253 u32 código
   (GP com horário) S->C 0x40 tipo 11 a cada 30 s / 10 s / 1 s com os segundos para começar
   (Rookie) jogador aperta Start ─► C->S 0x0E
   Início: S->C 0x253 i32 0, 0x230, 0x231, 0x77 u32 taxa de pang  [REF; ver §5.1]
           0x76 (u8 4 = torneio, u32 1, SYSTEMTIME), 0x256 (bots), 0x52 por jogador
   Partida = torneio (cada um a sua bola) + relógio por buraco (0x259) + regra (vento/10 s/penalidade)
   Fim: 0x199 no último buraco; 0x40 tipo 16, 0x6D, 0x6C estado 2; quando todos terminam:
        0xCE, 0x79, 0x258 pódio, 0x25C+0x216 troféu, 0x216 prêmios, 0x25A GP concluído
   C->S 0x06 (estatística 0x109) ─► 0x45, [0x6B tipo 8], 0x244, 0x24F, 0xC8
   Sair da sala ─► C->S 0x17A ─► S->C 0x254 u32 0, i16 -1 (volta ao lobby do GP)
   Sair do lobby do GP ─► C->S 0x177 ─► S->C 0x251 u32 0 (+ 0x46 sub2 aos outros)
```

---

## 1. Propriedade do servidor 0x800

- Bits ([REF] `PangyaAPI.Network/Models/Pangya_St.cs`, `uProperty`): 1 special, 2 small_play, 4 ladder, 8 adult,
  16 mantle, 32 skins, 64 only_rookie, 128 natural, 256 championship, 512 azul, 1024 verde, **2048 (0x800) grand_prix**,
  4096 relay, bit 31 rookie_beginner_only.
- Onde vai:
  1. **Login, lista de servidores** (S->C do login, `JpLoginHandler.ServerList`): ServerInfo de 92 bytes, campo
     `u32 propriedade` no **offset 74** (`char[40] nome, u32 id, i32 máx, i32 atuais, char[18] endereço, i32 porta,
     u32 propriedade, ...`). Hoje sai de `config.Property` (`ServerInfo.Flags`).
  2. **0x44** (dados do jogador, game server): o **último u32** do pacote (`... u64 bloqueios, i32 contagem,
     u32 propriedade`) [EXE: `... D8 D4 D4`]. Hoje `JpTranslator.Convert(KPlayerInfo)` já escreve `property`.
  3. Canais 0x4D: cada ChannelInfo de 85 tem `u32 propriedade` próprio (hoje 16, igual à referência). Não é o que
     liga o GP [?: a referência usa a propriedade do servidor na criação do canal, mas o GP é decidido pela do servidor].
- O que o cliente faz: com 0x800, o botão Start Game abre a escolha "Grand Prix / jogo normal" (sem ele vai direto ao
  Multiplay, que manda 0x81). Escolhendo Grand Prix o cliente manda **0x176** (`sGrandPrixLobbyRequest983::Send`,
  `grandprixlimitdlg`, e `CLobbyMain::HandleMsg`) [EXE pk_cmp].
- Servidor: com o jogador no lobby do GP (`lobby = 176` na referência), a troca de servidor para um sem 0x800 é
  recusada (`GameService.requestChangeServer`, erro 0x7500002). Só ligar 0x800 junto com pelo menos a fase 1 (§6):
  sem tratamento de 0x176 o cliente fica esperando o 0x250.

---

## 2. Pacotes cliente -> servidor do Grand Prix

Todos fora das faixas do tradutor (`ClientId` devolve −1 para ≥ 0x83 fora das faixas): hoje **caem** com "sem par no
KR". Precisam ser tratados na camada JP (antes do tradutor) ou ganhar um handler no KR com id próprio.

| id | corpo (bytes) | quem manda [EXE] | handler [REF] |
|---|---|---|---|
| **0x176** | vazio (0) | `grandprixlimitdlg` / `CLobbyMain::HandleMsg` (Start Game -> Grand Prix) | `packet176` -> `Channel.requestEnterLobbyGrandPrix` -> `enterLobbyGrandPrix` |
| **0x177** | vazio (0) | `lobbymain` FUN_00ab6620 (sair do lobby do GP; par do 0x82) | `packet177` -> `requestExitLobbyGrandPrix` -> `leaveLobbyGrandPrix` |
| **0x179** | `u32 id do GP` (4) | `FrGrandPrixOpenDlg::OnOk` (`grandprixopendlg`) | `packet179` -> `requestEnterRoomGrandPrix` (cria ou entra) |
| **0x17A** | `u8 opção, i16 valor, u8[16] chave` (19) [EXE: `E1 E2 E8 E8`] | `UTIL::S5::SendExitGPRoom983` | `packet17A` -> `requestExitRoomGrandPrix` -> `leaveRoomGrandPrix(s, 1)` |

- 0x17A: opção 0 = sai fora de jogo, 1 = sai em jogo; valor sempre −1 na referência; chave = a chave da sala (16 bytes,
  a mesma do RoomInfo offset 69). A referência lê e ignora os três.
- Durante a sala/partida o GP usa os pacotes comuns (ids ≤ 0x82, iguais no KR):
  - **0x2D** `u16 nº da sala` (o `CGrandPrixLobbyRoom` também manda, [EXE] `E2`) -> detalhe da sala (KR 0x84 -> JP 0x86).
  - **0x0E** `u32` = começar (só no GP **Rookie**, que não tem horário; ver §5.1).
  - **0x34** (tee pronto / fim da intro do personagem): na referência **liga o relógio do buraco** (`TimeHole`).
  - **0x22** (relógio da vez): liga o relógio da regra de 10 s / 15 s.
  - **0x65** `f32 velocidade` (time booster) -> **0xC7** `f32 velocidade, i32 oid` (KR 0xC5; já existe).
  - 0x12 tacada, 0x1B resultado (38 bytes, o 38º = penalidade do GP), 0x1C, 0x13/0x14/0x15/0x16, 0x17 item, 0x31, 0x48,
    0x1A, 0x11: iguais ao torneio do KR.
  - **0x06** fim: `EB(0x109)` = estatística do JP de 265 bytes (o `FrGrandPrixAwardDlg` manda depois da tela de prêmio
    [EXE]). Atenção: o KR (`CGameStats`) espera a struct de 235 (`sPangYaUserStatistics`); o JP manda 265 (o mesmo vale
    para o 0x31). Converter 265 -> 235 ou o placar final grava errado [?: conferir a ordem dos campos do UserInfo JP].

---

## 3. Respostas servidor -> cliente

### 3.1 Entrar no lobby do GP (resposta ao 0x176) [REF `Channel.enterLobbyGrandPrix` + `enterLobby(s, 176)`]

Ordem:
1. **0x46** (KR 0x44) `u8 4, u8 1, PlayerLobbyInfo 200` — limpa a lista com o próprio jogador.
2. **0x46** `u8 5, u8 n, n × 200` — todos do canal que estão em algum lobby (a referência não filtra por lobby 176).
3. **0x47** (KR 0x45) `u8 n, u8 sub 0, u16 0xFFFF, n × RoomInfo 221` — todas as salas do canal (as de GP aparecem com
   `type_extend = 0x14` e o bloco GP preenchido, §3.3).
4. **0x46** `u8 1, u8 1, 200` em broadcast no canal (entrou).
5. **0x250** (só JP; [EXE] lê `D4` e o resto numa subfunção):

| off | tipo | campo |
|---|---|---|
| 0 | u32 | resultado: 0 = ok; senão código de erro e **o pacote acaba aqui** (desligado: 0x750001; genérico: 0x750000) |
| 4 | u32 | **nT** = nº de tipos de GP de evento ativos (bits de `GP_EVENT`; 0 se nenhum) |
| 8 | nT × u32 | tipo ativo (1..16 = bit+1); hoje só existe o tipo **1** (`TypeGP = 1`, "event_common") |
| 8+4nT | u32 | **nC** = nº de GPs já concluídos pelo jogador |
| +4 | nC × (u32, u32) | `TypeID_Link` do GP, melhor posição (1 = 1º; inclui os bots na contagem) |
| fim | f32 | média de placar do jogador (`UserInfo.getMediaScore`) [?: f32 confirmado só pela referência] |

Tamanho: 16 + 4·nT + 8·nC. Mínimo (nada ativo, nada concluído): `00000000 00000000 00000000 <f32>` = 16 bytes.
O cliente usa isso para montar a lista: GP com `Lock_YN = 1` só abre se `Clear_GP_TypeID` estiver na lista nC;
`condition[0..1]` = média mínima/máxima; `TypeGP > 0` só aparece se o tipo estiver em nT [?: inferido das checagens
do servidor em `requestEnterRoomGrandPrix`; o cliente provavelmente faz as mesmas para cinzar a lista].

### 3.2 Sair do lobby do GP (0x177)
- `leaveLobby`: tira da sala se estiver (sem pacote extra), lobby = 255, **0x46 sub 2** (saiu) ao canal, chat "BYE".
- **0x251** `u32 0` (4 bytes) [EXE: `D4`].

### 3.3 Entrar/criar sala de GP (0x179) [REF `Channel.requestEnterRoomGrandPrix`]

Checagens, na ordem (erro = **0x253 u32 código**, 4 bytes, [EXE: `D4`]):

| código | motivo |
|---|---|
| 0x6700003 | jogador bloqueado para qualquer sala |
| 0x6700004 | jogador bloqueado para GP |
| 0x6700001 | GP não existe na GrandPrixData ou `Active = 0` |
| 0x670000C | fora do horário: agora < `Open` (hh:mm) ou agora > `Start` (se `Open.Hour ≥ 23` e `Start.Hour ≤ 1`, Start conta +1 dia) |
| 0x6700006 | nível < `MinLevel` ou (`MaxLevel > 0` e nível > `MaxLevel`) |
| 0x6700007 | GrandPrixConditionEquip(`TypeID_Link`) exige item equipado e não está |
| 0x6700008 | média < `condition[0]` ou (`condition[1] > 0` e média > `condition[1]`) |
| 0x6700009 | não tem `ticket.qntd` de `ticket._typeid` (0x1A000264 "Grand Prix Ticket") |
| 0x670000A | `Lock_YN > 0`, `Clear_GP_TypeID ≠ 0` e o jogador não concluiu esse GP |
| 0x670000B | `TypeGP > 0` e o tipo não está ativo no servidor |
| 0x6700002 | falhou ao criar a sala |
| 0x6700005 | sala cheia (30) |
| 0x6700000 | erro genérico |

Sala: **Rookie** (aba 0 e GP "normal", §4.1) = sempre uma sala nova só do jogador (instância); os outros = uma sala por
id de GP no canal (cria se não existir, senão entra). RoomInfo da sala de GP:

| campo (offset no RoomInfo JP 221) | valor |
|---|---|
| nome (0, char[64]) | `GrandPrixData.Name` (Shift-JIS do .iff; o KR grava cp949 — copiar os bytes do .iff, até 63) |
| senha_flag (64) | 1 (sem senha) [?] |
| max_player (67) | **30** |
| qntd_hole (87) | `course_info.Qntd_hole` |
| tipo_show (88) | **4** (torneio: `setTipo` põe 4 para tipo > 14) |
| modo (91) | `course_info.Modo` (ordem dos buracos) |
| course (92) | `course_info.Course` |
| time_vs (93), time_30s (97) | 0, 0 |
| trofel (101) | troféu da sala (`updateTrofel`, igual ao torneio) [?] |
| master (192, i32) | **−2** (sala sem dono; vale também para o Rookie) |
| type_extend (196, u8) | **0x14** (20 = GRAND_PRIX) |
| artefato (197, u32) | `GrandPrixData.rule` (0 ou 0x1A000265..0x1A00029E) |
| natural/short (201, u32) | bit0 = `flag[0]` (natural), bit1 = `flag[1]` (short game) |
| GP (205, 16 B) | `u32 id do GP, u32 TypeID_Link, u32 TimeHole × 1000 (ms), u32 1 (ativo)` |

Hoje `KrToJp.Room` zera type_extend/artefato/natural/GP: precisa receber esses valores da sala de GP.

Resposta de sucesso (ordem da referência):
- Sala nova: **0x4A** (KR 0x48; `u16 nº, u8 tipo_show, u8 mapa, u8 buracos, u8 modo, u32 natural/short, u8 máx, u8
  galeria, u8 dormindo, u32 tempo vs, u32 tempo 30s, u32 troféu, u8 senha_flag [+ str senha], str nome`) ->
  **0x49** (KR 0x47) `i16 0 + RoomInfo 221` -> **0x48** sub 0 (KR 0x46) `u8 0, u16 0xFFFF, u8 n, n × 348, u8 0`
  (modo em massa: sem o personagem de 513) -> lobby: **0x47** sub 1 (sala nova) -> **0x46** sub 3 (jogador atualizado).
- Sala existente: igual, mais **0x48 sub 1** (entrou) aos outros e 0x47 sub 3 (sala atualizada) no lobby.

Contagem regressiva (GP com horário; não no Rookie) [REF `RoomGrandPrix.count_down_to_start`]: broadcast na sala de
**0x40** `u8 11, str "" (u16 0), str "" (u16 0), u32 segundos para começar` (13 bytes; [EXE] 0x40 lê `D1 DS DS D4`),
a cada 30 s enquanto faltar > 40 s, depois a cada 10 s, e no fim a cada 1 s. Em 0: se houver ≥ 1 jogador, começa (§5.1);
senão a sala é destruída.

### 3.4 Sair da sala de GP (0x17A)
- `leaveRoom` normal (0x48 sub 2 aos outros, 0x47 no lobby; no jogo: 0x61 `i32 oid` + 0x6C `u32 oid, u8 3`).
- **0x254** `u32 0, i16 −1` (6 bytes) [EXE: `D4 D2`, depois o cliente avisa o messenger (0x23)]. O jogador volta ao
  lobby do GP (lobby continua 176; a referência não reenvia 0x250).

### 3.5 Pacotes da partida de GP (só JP, além do torneio)

| id | quando | layout | tam. | [EXE] |
|---|---|---|---|---|
| **0x253** | início (sucesso) | `i32 0` | 4 | `D4` |
| **0x230** | início | vazio | 0 | vazio |
| **0x231** | início | vazio | 0 | vazio |
| **0x77** (KR 0x75?) | início | `u32 taxa de pang` | 4 | `D4` (e o cliente responde ao servidor) [?] |
| **0x76** (KR 0x74) | dados iniciais, broadcast | `u8 tipo_show (4), u32 1, SYSTEMTIME início` | 21 | `D1 D1 D1 D1 D1 DB(0x10)` |
| **0x256** | logo depois do 0x76, broadcast | ver abaixo | 6 + Σ(5 + 40·h) | `D4` (+ subfunção) |
| **0x52** (KR 0x50) | depois do 0x256, a cada jogador | igual ao torneio; GP sem cubos/moedas | — | — |
| **0x259** | tempo do buraco acabou (só para o jogador) | `u32 0` | 4 | `D4` |
| **0x255** | todos terminaram o buraco atual | vazio | 0 | vazio (CGolfTask) |
| **0x199** | o jogador terminou o último buraco | vazio | 0 | vazio |
| **0x216** | ticket gasto (início), troféu e prêmios (fim) | ver abaixo | 8 + n × 50 [?] | `D4 D4` |
| **0x258** | fim: pódio | `u32 0, u8 n, n × 222` | 5 + 222n | `D4` (+ sub) |
| **0x25C** | fim: troféu do GP ganho | `u32 0, u32 typeid do troféu` | 8 | `D4 D4` |
| **0x25A** | fim: GP concluído/melhorou a posição | `u32 0, u32 TypeID_Link, u32 posição` | 12 | `D4` (+ sub) |
| **0x244** | fim (depois do 0x06) | `u32 0` | 4 | — [?: não está no tsv lido] |
| **0x24F** | fim (depois do 0x06) | `u32 0` | 4 | `D4` |
| **0xC8** (KR 0xC6) | fim (depois do 0x06) | `u64 pang, u64 0` | 16 | `D8` |

**0x256 — bots do GP** [REF `GrandPrix.sendInitialData`, `Bot.Hole.ToArray`]:
```
u32 0                      resultado
u16 nBots                  30 − nº de jogadores (0 se a sala tiver 30)
nBots × {
  u32 id                   GrandPrixAIOptionalData.ID (o cliente tira nome/visual da tabela)
  u8  h                    nº de buracos (= buracos da sala)
  h × Hole (40 bytes) {
    u32 mapa               (course & 0x7F)
    u32 nº do buraco       (número no mapa, não a sequência)
    i32 placar             tacadas − par do buraco
    u32 0
    u64 pang               0..350 (×2 com chuva)
    u64 bônus              0..199
    u64 0
  }
}
```
Os bots não são jogadores: não entram no 0x48 nem mandam tacadas; o cliente mostra o progresso deles no placar do
torneio [?: como o cliente distribui os buracos dos bots no tempo não está claro]. Ordenados por placar (menor primeiro)
e, empatados, por pang (maior primeiro).

**0x216 — itens atualizados** [REF `GrandPrix.consomeTicket`, `sendTrofel`, `sendRewardRankAndGrandPrix`]:
```
u32 hora (unix)
u32 n
n × {
  u8  tipo                 2
  u32 typeid
  i32 id                   id do item no inventário (o addItem preenche)
  u32 flag_time            0, ou 4 (= dias) em item por tempo
  i32 qtd antes
  i32 qtd depois
  i32 qtd (ou tempo)       ticket: −qntd gasta
  u8[25] 0
}                          = 50 bytes por item
```
[?] O `pacote216` genérico da referência escreve, no lugar dos 25 zeros, `str ucc_idx, u32 status, u32 seq, u8[5]`
(15 bytes com idx vazio). As funções do GP usam 25 zeros; conferir no exe a subfunção do 0x216 antes de fixar. Com
n = 0 o pacote é `u32 hora, u32 código`.

**0x258 — pódio** [REF `RankPlayerDisplayChracter.ToArray`], só jogadores humanos entre os 3 primeiros da ordem final
(bots contam na posição mas não entram): `u32 uid, u32 posição (1..3), u8 cabelo, u8 camisa, u32[24] peças (typeid),
u32[5] anéis/aux, u32[24] ids das peças` = **222** bytes por entrada. Ordenado por posição.

**0x25D** (login): troféus do GP, `u8 temporada, u32 ignorado, u32 n, n × 12` [EXE: `D1 D4 D4 DB(0xc)`]; tipo 4/5 é a lista
que o My Room mostra, tipo 0 vai para outra que ele não lê (mandamos 5; SPEC-jp983-game.md §Troféus). O **0x25C** só dá o
tid à janela de prêmio: quem põe o troféu no My Room na hora é o 0x216. **0x257** (perfil): `u8 temporada, u32 uid, i16 n, n × 12`
[EXE: `D1 D4 D2 DB(0xc)`]. Não são necessários para jogar.

---

## 4. Tabelas .iff do Grand Prix

Arquivo: `data/pangya_jp.iff` (zip). Cabeçalho de cada tabela: `u16 n, u16, u32 versão (13)`, depois n registros fixos.
Tamanhos conferidos com o exe ([EXE] `iff_983.tsv`) e com o arquivo ([IFF]).

| tabela | registro | n no cliente |
|---|---|---|
| GrandPrixData.iff | 0x330 (816) | 741 (todos `Active = 1`) |
| GrandPrixRankReward.iff | 0x4C (76) | 781 (215 GPs × 3 posições + ...) |
| GrandPrixAIOptionalData.sff | 0x25C (604) | 990 (110 por classe 1..9) |
| GrandPrixSpecialHole.iff | 0x14 (20) | 84 |
| GrandPrixConditionEquip.iff | 0x210 (528) | 2 |

### 4.1 GrandPrixData.iff (816 bytes) [IFF: offsets conferidos com os dados]

| off | tipo | campo | uso |
|---|---|---|---|
| 0 | u32 | Active | 0 = desligado (erro 0x6700001) |
| 4 | u32 | **ID** | id do GP (o que vem no 0x179); ver "id" abaixo |
| 8 | u32 | **TypeID_Link** | grupo: chave do RankReward, SpecialHole, ConditionEquip e da lista de concluídos |
| 12 | u32 | TypeGP | 0 normal; 1 evento ("event_common", liga com o bit 1 do `GP_EVENT`) |
| 16 | u16 | **TimeHole** | segundos por buraco (0, 90, 120, 180); 0 = sem relógio |
| 18 | char[66] | Name | Shift-JIS |
| 84 | u32, u32 | ticket typeid, qntd | 0x1A000264, 1..3 |
| 92 | char[41] | Event_Icon | "" ou "event_common" |
| 133 | u8 ×3 | flag: natural, short game, buraco ×2 | vão no u32 natural da sala (bit0, bit1) |
| 136 | u32 | **rule** | 0; 0x1A000265 vento 1 m; 0x1A000266 super vento; 0x1A000267 power shot = penalidade; 0x1A000268 regra de 10 s; 0x1A00029E 15 s; 0x1A000269 imã, 0x1A00026A sem volta, 0x1A00028F vento 3–5 m, 0x1A000290 vento 7–9 m (não usados no arquivo atual) |
| 140 | u32 | Course | mapa |
| 144 | u32 | Modo | ordem dos buracos |
| 148 | u8 | Qntd_hole | 3, 6, 9, 18 |
| 149 | u8 | MinLevel | |
| 150 | u8 | MaxLevel | 0 = sem limite |
| 151 | u8 | (zero) | |
| 152 | u32[2] | condition | média mín./máx. (0 = sem limite) |
| 160 | i32 ×3 | bot: ScoreBotMax, Med, Min | placar alvo dos bots (ex.: −1, 4, 12) |
| 172 | u32 | _class | classe dos bots (1..7; casa com AIOptionalData.Class) |
| 176 | u32 | pang | igual ao qntd do Pang Pouch em reward[0] (só exibição) [?] |
| 180 | u32[5] | reward typeid | prêmio de participação (todos que terminam); 0x1A000010 = Pang Pouch (qtd = pang) |
| 200 | u32[5] | reward qntd | |
| 220 | u32[5] | reward tempo (dias) | > 0: item por tempo (qtd 1, flag_time 4) |
| 240 | SYSTEMTIME | Open | só `wHour` (+8) e `wMinute` (+10) valem; o último u16 é lixo |
| 256 | SYSTEMTIME | Start | hora em que a partida começa sozinha |
| 272 | SYSTEMTIME | End | |
| 288 | u32 | (zero) | |
| 292 | u32 | Clear_GP_TypeID | GP (TypeID_Link) que precisa ter concluído |
| 296 | u32 | Lock_YN | 1 = exige Clear_GP_TypeID |
| 300 | char[516] | Info | descrição |

**Id do GP** (`GrandPrixData.isGrandPrixNormal/Event/getGrandPrixAba`): `(ID >> 24) & 3` = 0 normal, 3 evento;
aba = `(ID & 0xFFFFFF) >> 19`: 0 **Rookie**, 1 Beginner, 2 Junior, 3 Event/Open. No arquivo: 8 Rookie (0x100..0x800,
sem horário, cada um libera o seguinte por `Lock_YN`), 287 Beginner (0x80101..), 287 Junior (0x100101..), 144 aba 3
(0x180101..), 15 de evento (0x318A701.., TypeGP 1). Os GPs com horário vêm em grupos de 24 ids (um por hora do dia,
`ID & 0xFF` = 1..24, `Open.Hour` = 0..23): ex. 0x80101..0x80118 "Shining Sand杯(9H)", abre hh:00, começa hh:10.
Os horários são do relógio do servidor (o oficial: JST) [?].

### 4.2 GrandPrixRankReward.iff (76 bytes)
| off | tipo | campo |
|---|---|---|
| 0 | u32 | Active |
| 4 | u32 | ID = TypeID_Link do GP |
| 8 | u32 | Rank (1..3) |
| 12 | u32[5] | typeid (ex.: 0x1A000010 Pang Pouch, 0x1A000273 Memorial Coin, 0x1A000011 Time Booster) |
| 32 | u32[5] | qntd (ex.: 400/300/200 pang) |
| 52 | u32[5] | tempo (dias) |
| 72 | u32 | **Trophy** (0x2F010101.. "クラス1 グランプリ大会1位トロフィー", tabela Match.iff) |

Uso: ordena por Rank; a posição final i (0-based, contando os bots) pega o registro i se existir (só top 3).
Dá o troféu (0x25C + 0x216) e os itens (0x216 junto com os de participação).

### 4.3 GrandPrixAIOptionalData.sff (604 bytes) — bots
| off | tipo | campo |
|---|---|---|
| 0 | u32 | Active |
| 4 | i32 | **ID** (vai no 0x256) |
| 8 | char[36] | nome (Shift-JIS) |
| 44 | u32 | BetterOrNo (0/1) |
| 48 | u32 | TypeID (nº do personagem 0..) |
| 52 | u32 | **Class** (1..9) |
| 56 | u32[24] | peças (typeid) |
| 152 | u32[24] | ids das peças |
| 248 | u8[216] | UccIndexList |
| 464 | u32[5] | aux/anéis |
| 484 | u8[120] | (zeros) |

Uso no servidor: sorteia `30 − jogadores` ids entre os `Active = 1` com `Class = GrandPrixData._class` (repete se faltar).
O visual é do cliente (pela tabela). Placar dos bots: ver §5.3.

### 4.4 GrandPrixSpecialHole.iff (20 bytes)
`u32 Enable, u32 TypeID (= TypeID_Link do GP), u32 HolePOS (1..), u32 Map, u32 Hole`. Uso [REF `CourseManager.init_seq`]:
se a sala de GP tem registros para o seu `TypeID_Link`, a sequência de buracos é a da tabela (mapa e buraco por
posição) e completa até 18 com o mapa da sala. Só os GPs Rookie (0x100..0x800) usam. O 0x52 do JP já leva `u8 mapa`
por buraco (como no Special Shuffle). A referência ordena por `Hole`; o certo deve ser por `HolePOS` [?].

### 4.5 GrandPrixConditionEquip.iff (528 bytes)
`u32 active, u32 typeid (= TypeID_Link), u32 item exigido, char[516] texto`. Arquivo: 0x180500 exige 0x10000000
(Air Knight Set); 0x3180200 exige personagem 0x04000000 (Nuri). Verificado ao entrar (erro 0x6700007) e, na sala, ao
trocar caddie/taco/personagem a referência força o item exigido (`RoomGrandPrix.requestChangePlayerItemRoom`).

---

## 5. Partida de GP × stroke normal

O GP é um **torneio** (`GrandPrix : TourneyBase`): cada jogador joga a sua bola, sem vez. O KR já tem isso
(`TourneyGame`/`MassGame`, `MassOutput`); os ids do torneio já batem pelo tradutor: 0x6C/0x6B/0x6A do KR -> 0x6E (posição),
0x6D (buraco terminado: `u32 oid, u8 buraco, u8 tacadas, i32 placar, u64 pang, u64 bônus, u8 1`), 0x6C (estado: `u32 oid,
u8 2 terminou / 3 saiu`); 0x8A -> 0x8C (tempo acabou); 0x8B -> 0x8D (`u32 ms`); 0x12A -> 0x132 (Treasure Hunter);
0x77 -> **0x79** (placar do torneio: `i32 exp, u32 troféu da sala, u8 troféu ganho, u8 time, ...` [EXE `D4 D4 D1 D1 D4 D4
DB(0x60) DB(0x18)`]).

### 5.1 Início
- **Rookie** (aba 0, normal): sala instância, o jogador manda **0x0E** (o `Start()` do KR exige ser o dono: a sala de GP
  tem master −2, então liberar para GP Rookie). Erro: **0x253** `u32 0x5900200`.
- **Com horário**: o servidor começa sozinho no `Start` (§3.3). Não precisa de 0x0E.
- Ao começar: gasta o ticket de todos (**0x216**, qtd negativa; se faltar ticket o jogador fica de fora [?]), cria os
  bots, e manda **0x253 i32 0**, **0x230**, **0x231**, **0x77 u32 taxa de pang** a toda a sala [REF]; depois os dados
  iniciais: **0x76** (u8 4, u32 1, SYSTEMTIME), **0x256** (bots), **0x52** por jogador. No stroke do JP que já funciona o
  servidor não manda 0x230/0x231/0x77 (o KR manda 0x74 + 0x50 direto); para o GP o mínimo seguro é 0x253 0 antes do
  0x76 e 0x256 entre 0x76 e 0x52 [?: confirmar no cliente se 0x230/0x231/0x77 são necessários].
- Mapa: `course` do GP; vento conforme a regra [REF `CourseManager`]: 1 m -> vento máx. 1; super vento -> 9..15;
  3–5 m -> 2..5; 7–9 m -> mín. 6. GP **não tem cubos nem moedas** (os 18 bytes do fim do 0x52 = 0).

### 5.2 Durante
- **Relógio do buraco** (`TimeHole` s > 0): começa no **0x34** do jogador; para no fim do buraco. Estourou e o jogador
  não acabou: tacadas = máximo do buraco (`total_shot`, desistência), time_out = 1 e **0x259 u32 0** só para ele.
- **Regra de 10 s / 15 s** (`rule` 0x1A000268 / 0x1A00029E): relógio por tacada que começa no **0x22** e para no
  0x12; se estourar antes de tacar, **+1 de penalidade**.
- **Regra do power shot** (0x1A000267): cada tacada com `display.special_shot` -> **+1 de penalidade**.
- Penalidades somam nas tacadas do buraco ao terminá-lo. O resultado da tacada (0x1B/0x64) tem 38 bytes: o 38º é a
  penalidade do GP (o tradutor hoje corta para 37 e devolve 0: no GP devolver o valor do servidor) [?].
- Desistência automática como no torneio (tacadas ≥ máximo do buraco); 3 desistências -> expulsa.
- Quando **todos** os humanos terminam o buraco corrente: **0x255** (vazio) em broadcast [REF `sendAllToNextHole`]
  [?: no torneio do KR cada um anda sozinho com o 0x63; no GP a referência sincroniza com 0x255 — conferir se o cliente
  espera o 0x255 para ir ao próximo buraco].
- Último buraco: **0x199** (vazio) ao jogador; bônus de "clear" 30 s se houver.
- Saída no meio: 0x61 `i32 oid` aos outros, 0x6C estado 3; não conta como quit se a opção for 0x800.

### 5.3 Bots e posição final
- Placar alvo (`init_bots`): média dos jogadores da sala ajusta `bot.ScoreBotMin/Med/Max`
  (`fator = ((18/buracos)·média_bot·1.7 + 72 − média_sala + 180) / 180`); cada bot sorteia um tipo (MAX 1/5, MED, MIN)
  e um alvo, e distribui por buraco com limites de par e do máximo do buraco, pesando o vento e a chuva. Pang por buraco
  0..350 (×2 com chuva), bônus 0..199. Fórmula livre: o servidor pode simplificar (o cliente só mostra).
- Ordem final (`requestCalculeRankPlace`): jogadores (menos quem saiu) + bots, por placar crescente, empate por pang.

### 5.4 Fim e prêmios (quando todos os humanos terminam) [REF `GrandPrix.finish`/`requestFinishData`]
Para cada jogador que terminou, nesta ordem:
1. **0x40** tipo 16 (broadcast, quando ele termina): `u8 16, str nick, u16 0, i32 placar, u64 pang, u8 assist`.
   **0x6D** (u8 1) e **0x6C** estado 2.
2. **0xCE** `u8 0, u16 n, n × u32 typeid` (itens que caíram) [EXE `D1 D2 D4`].
3. **0x79** placar (exp, troféu...). EXP: 3 buracos 2, 6 -> 4, 9 -> 5, 18 -> 7 × fator de estrelas do mapa
   (`(máx(estrelas,1.1) − 1.1)·0.125 + 1`) × taxas; Rookie × 0.12 e pang ÷ 3; nível 70 não ganha.
4. **0x258** pódio.
5. **0x25C** + **0x216** troféu da posição (RankReward.Trophy, top 3).
6. **0x216** prêmios: os 5 de `GrandPrixData.reward` (todos) + os 5 do RankReward da posição (top 3); item de conjunto
   vira as peças; não repete item não acumulável que já tem.
7. **0x25A** se o GP (`TypeID_Link`) é novo na lista do jogador ou a posição melhorou (gravar a lista: é o que vai no 0x250).

Depois o cliente manda **0x06** (estatística 0x109) e o servidor: grava (Rookie não grava estatística nem conta jogo;
mapa do GP vai no registro "grand prix", código 52), soma EXP (jogador, caddie, mascote), **0x45** (o JP tem 12 blocos
de mapa: normal, natural, assist ×2 e **grand prix atual / resto / assist** — o do GP é o 9º), **0x6B tipo 8** (mascote,
se tiver), **0x244 u32 0**, **0x24F u32 0**, **0xC8 u64 pang, u64 0**. O cliente então sai com **0x17A** (§3.4).

---

## 6. Fases

### Fase 1 — entrar e sair do lobby do GP sem travar
1. Ligar 0x800 (`Property`) na lista do login e no fim do 0x44 (já existe a configuração).
2. Na camada JP, tratar **0x176**: a lógica do 0x81 do KR (entrar no lobby: 0x46 sub 4/5, 0x47, 0x46 sub 1 em broadcast),
   **sem** o 0xF5, e então **0x250** `u32 0, u32 0, u32 0, f32 média` (16 bytes). Marcar o jogador como "lobby GP".
3. **0x177**: a lógica do 0x82 (sair do lobby, 0x46 sub 2) e **0x251** `u32 0`.
4. **0x179**: enquanto não houver GP, responder **0x253** `u32 0x6700001` (o cliente mostra erro e continua no lobby).
5. **0x17A**: **0x254** `u32 0, i16 −1` (por segurança).
6. 0x253/0x250/0x251/0x254 são só do JP: mandar já com o id do JP (como o `SendTranslated` do 0x44), sem passar pelo
   mapa de ids.
Teste: Start Game -> Grand Prix abre a lista; voltar ao lobby; clicar num GP mostra o erro e não trava.

### Fase 2 — jogar um GP completo
1. Leitor das 5 tabelas (§4) a partir de `pangya_jp.iff` (Domain: GrandPrixService com as checagens de §3.3).
2. Persistência: lista de GPs concluídos (`TypeID_Link`, melhor posição) por jogador; troféus de GP (0x2F01xxxx) e o
   registro de mapa "grand prix".
3. 0x179 completo: sala de GP (instância no Rookie, compartilhada por id nos outros), RoomInfo com type_extend 0x14,
   artefato, natural, bloco GP, tipo_show 4, máx 30, master −2; 0x4A/0x49/0x48/0x47/0x46.
4. Agendador por canal: contagem 0x40 tipo 11 e início no `Start`; Rookie começa com 0x0E sem ser dono.
5. Partida: `TourneyGame` do KR com: bots virtuais (0x256 e na ordem final), relógio do buraco (0x34 -> 0x259), regras
   (vento no sorteio do buraco, 10/15 s via 0x22, penalidade do power shot), penalidade no 38º byte, 0x255 ao fechar o
   buraco de todos, 0x199 no último, sem cubos/moedas, buracos especiais do Rookie.
6. Fim: 0xCE, 0x79, 0x258, 0x25C, 0x216, 0x25A; no 0x06: 0x45 (com o bloco GP), 0x244, 0x24F, 0xC8; ticket gasto no
   início (0x216). Converter a estatística do 0x06/0x31 (265 -> 235).
7. 0x17A sai da sala e volta ao lobby do GP (0x254).

### Pontos a confirmar no cliente [?]
- Layout completo do 0x250 depois do primeiro u32 (subfunção não lida) e se o f32 da média é mesmo f32.
- Cauda do item no 0x216 (25 zeros × `str + u32 + u32 + 5`).
- Se 0x230/0x231/0x77 são exigidos no início do GP e se o cliente responde ao 0x77.
- Se o cliente espera o 0x255 para mudar de buraco no GP.
- Tamanho da entrada do 0x258 (222) e do bot (5 + 40·h) — só pela referência.
- 0x244 não aparece no `pacotes_recebe_983.tsv` (talvez tratado fora dos despachantes lidos).
