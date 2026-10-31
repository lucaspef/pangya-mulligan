# SPEC — Relatório do Tiki (JP 983)

Legenda: [C] lido no cliente 983 (dump e porte em C++, `rebang/local/TIKI_REPORT_A888.md`, `jp983gfx/src/projectg`),
[GB] servidor de referência (`<ref>/Server/GB`, Tourney.cs `requestUseTicketReport`,
`requestSaveTicketReport`, `requestSendTicketReport`; Modern/JP `Handle_PLAYER_OPEN_TICKET_REPORT_SCROLL`),
[I] inferido, [S] decisão deste servidor.

## 1. O que é

Quem terminou o torneio e não quer esperar os outros usa o **pergaminho** (Tiki Report Scroll, `0x1A000041`, item da loja,
quantidade em `Common[0]`): sai da sala na hora levando o **pang** da partida e recebe o **relatório** (Tiki Report,
`0x1A000042`) no My Room > Minha caixa > Relatórios. O **EXP** entra quando ele abre o relatório, que tem prazo
(7 dias; o cliente mostra "período para abrir: %dd %dh" a partir do `HourRemain`) [C].

## 2. Quando o cliente oferece (FrGameRoomDlg::CheckTikiReport, 983 `00506f30`) [C]

Sala em estado 8 (fim do jogo, esperando os outros; o jogador já está no lobby da sala), ESC, e todos valem: tem
`0x1A000041`; sem os bits 2/4/8 de `dwIdentity`; `gameType != 6`; byte `doc+0x10937 <= 30`; sem a tela de resultado
aberta; canal sem a flag `0x800`; nível `> 5`; menos de 10 relatórios (`0x1A000042`).

O servidor confere de novo (`TikiReport.Refusal`) [S]: partida `TourneyGame` não acabada, modo 4 ou 5 (não o 6/GuildMatch),
sala não escondida (prática, Grand Prix, Chip-in) e sem Grand Zodiac; o jogador terminou e não saiu; nível >= 6;
`IdentityFlags & 0xE == 0`; tem o pergaminho; menos de 10 relatórios. O byte `0x10937` e a flag do canal não existem no
servidor (sem conferência).

## 3. Pacotes

| sentido | id JP | formato |
|---|---|---|
| C->S | 0xAA | sPangYaUserStatistics (0x109) do buraco 0 (`m_holeStatistics[0]`): u32 EXP em +0x4A, i64 pang em +0x4F. O cliente já descontou 1 pergaminho, mostra a espera e fecha a sala com resultado 1 [C 00afa810] |
| C->S | 0x0F | recusou ("sair mesmo assim?"): u8 0, u16 0xFFFF, i64 pang, i64 bônus (só nos tipos 4/5) — a saída normal [C 00afa600] |
| S->C | 0x4C | u16 0xFFFF: saiu da sala, tira a espera (KR 0x4A) [GB `pacote04C(-1)`; I: é a mesma resposta da saída normal] |
| S->C | 0xAA | u16 n; n × sBuyItemResult (u32 typeid, u32 id, u16 horas, u8 tipo, u16 qtd, SYSTEMTIME fim, char[9]); para o `0x1A000042` com id novo, logo depois: u16 Common[1], u16 Common[2], SYSTEMTIME. Só o `CLobbyTask` trata (o My Room e a loja são atores dele); o cliente grava tipo 2, `HourRemain` = horas, `Common[0]` = max(qtd, 1) e a data em segundos [C taskmanager 0xaa, 009cb0ce]. A referência põe a data de fim nos dois SYSTEMTIME [GB `pacote0AA`] |
| C->S | 0xAB | u32 id do relatório, u32 `Common[1] * 0x8000 + Common[2]` [C 0060f5c0] |
| S->C | 0x11A | i32 n; SYSTEMTIME; n × sCaddieReportData (0x83). n = 0 "falha ao abrir", −1 "data inválida", −2 "dados inválidos" (só limpam a lista); n > 0: o cliente tira o item (pelo id escolhido) e abre a tela de resultado (`uniteresult`, msg 0x282) [C realmyroomtask 0x11a] |
| S->C | 0x11B | u32 uid: "saiu com o relatório" para os outros no lobby (estado do rival = 2) [C lobbytask 0x11b, GB]. Não mandamos: o 0x6C(estado 2) dele já saiu quando terminou e o servidor só não manda o estado 3 |

sCaddieReportData (0x83) [C globalgamedefine.h:1818]: +00 u32 uid; +04 i64 pang; +0C i64 bônus; +14 u32 troféu da sala
(`m_roomType`, o tid do 0x79); +18 u32 EXP; +1C u32 mascote; +20 u8 premium; +21 u8 item de pang (1 maestria, 2 nitro);
+22 u16 nível; +26 i8 placar; +27 u8 medalhas (bits 0..5 = `m_awardItem[0..5]`); +28 u8 troféu ganho (1..3);
+3F char[22] nick; +59 u32 guilda; +5D u32 versão do emblema; +61 char[12] emblema; +6D u8 modo; +71 u8 equipe
(bit 4 = saiu, estado 3); +72 u8 equipe vencedora (2 = nenhuma). O registro com o meu nick dá EXP, troféu da sala,
modo, troféu e o último byte da tela; o cliente fixa 30 vagas. O cliente não aplica o EXP: só mostra [C].

## 4. Servidor

**Usar** (`JpSystems.TikiReport` -> `GameHandler.LeaveWithTikiReport`, sob o lock das salas):
1. Recusado: só a saída normal (0x4C 0xFFFF; sem prêmio, como quem sai antes do fim). O pergaminho não é gasto (o
   cliente já descontou um na tela; volta no próximo login).
2. Valeu: `MassPlayer.Reported = true`; EXP pela fórmula do torneio (`Rewards.Exp`: jogadores × buracos × estrelas ×
   taxas; **sem desconto por posição no torneio**, então o valor já é o final); o fim da partida dele roda na hora com
   EXP 0 (`BeginGameEnd(fixedExp: 0)` + `FinishGameEnd`): pang e bônus **do placar do servidor** (`MassPlayer.Pang/Bonus`,
   os do último resultado de tacada, limitados por buraco como sempre), estatística, recorde do curso, conquistas;
   0x45 e 0xC8 com o pang novo. O pang/EXP do 0xAA do cliente só vão para o log se forem diferentes.
3. Sai da sala como na saída normal (0x4C 0xFFFF, lista de salas). Para os outros ele segue "terminou": o
   `TourneyGame.PlayerLeft` não manda o estado 3 de quem tem `Reported`. Ele não entra na classificação de troféus (como
   quem sai).
4. Gasta 1 pergaminho (`PlayerActions.DeleteItemAsync`), cria o relatório e manda o 0xAA. Atributos do item:
   `c1` = 0, `c2` = 1..0x7FFF (sorteado), `date` (hora da partida), `exp`, `match` (troféu da sala), `mode`, `rows`
   (uid, nick, nível, placar, pang, bônus, mascote, premium, guilda, emblema, troféu, medalhas, saiu). `ExpiresAt` = +7 dias.
   `Common[1] = 0` faz o parâmetro do 0xAB ser igual a `Common[2]` com qualquer multiplicador (o GB usa 0x800, o 983 0x8000).
5. Fim da partida (`MassOutput.GameOver`): para quem saiu com o relatório e ainda está com a sessão, as linhas viram as
   finais (placares, troféus e medalhas de todos); se o relatório já foi aberto, nada muda.

**Abrir** (0xAB -> `GameHandler.OpenTikiReportAsync`): o item é dele e é `0x1A000042` (senão 0), o parâmetro confere e os
dados leem (senão −2), não venceu (senão −1). Numa gravação: item apagado + EXP/nível (+ o caddie equipado, como no fim
de partida, `Rewards.AddExpAsync`). Depois 0x11A com as linhas (EXP na minha), e só então os presentes de nível (0x10F) e o
0x45 com EXP/nível novos (a referência também credita depois do 0x11A, "para o visual não bugar").

**Vencidos**: −1 ao abrir; saem do inventário no login seguinte (`PurgeExpiredReportsAsync`). No inventário (0x73) o
relatório vai com `Common[1]/[2]`, `HourRemain` = horas que faltam (mínimo 1; 0 seria "permanente"), tipo 2 e o fim do
prazo em segundos unix na 1ª data do item JP (+24), como o cliente grava ao receber o 0xAA.

## 5. Diferenças da referência (GB) e o que é inferido

- GB cria o relatório **no fim do torneio** (com todos) e o entrega a quem saiu (0x11C u8 1 + 0xAA); prazo de 24 h. Aqui o
  relatório nasce **na saída** (o jogador o vê na hora) e recebe as linhas finais no fim se ele ainda estiver online; prazo
  de 7 dias (o correio do cliente fixa 0xA8 horas). [S]
- GB dá o troféu/medalhas a quem saiu com o relatório; aqui ele fica fora da classificação (como quem sai). [S]
- GB credita o EXP mesmo com o relatório vencido; aqui −1 e nada. [S]
- 0x4C 0xFFFF depois do C->S 0xAA: confirmado no GB, não capturado do servidor original. [I]
- Data do 0xAA e a do 0x11A: a referência põe o fim do prazo no 0xAA e a data da partida no 0x11A; seguimos isso. [I]

Código: `src/Pangya.Domain/Rooms/TikiReport.cs`, `src/Pangya.Protocol.KR645/Game/GameHandler.TikiReport.cs`,
`src/Pangya.Protocol.JP983/Game/JpSystems.TikiReport.cs`, `JpTranslator.JpOnly.cs` (0xAA/0xAB -> ids internos
0x7FAA/0x7FAB). Testes: `tests/Pangya.Tests/TikiReportTests.cs`, `Jp983TikiReportTests.cs`.

## 6. Relatório sem dados (bug de 30/10)

Sintoma: na aba Relatórios o card mostrava "0 ano 0 mês 0 dia / 0 h 0 min" e abrir não fazia nada. Causa: o item
`0x1A000042` da conta do dono veio do `give-all` de 07/10 (entra como "usável" ×10, como qualquer item do IFF), sem
`ExpiresAt` e sem atributos: data zerada no card e `Common[1]*0x8000 + Common[2] = 0`; com o parâmetro 0 o cliente
(0060f5c0) **não manda o 0xAB** (nenhum 0xAB nos logs de pacotes), então o servidor nunca soube do clique [C, log].
O `0x1A000042` não se compra (preço 10.000.000, oculto na loja) e não tem como ser reconstruído sem a partida.

Correção [S]: `ShopService.GrantAsync` recusa o `0x1A000042` (`NotForSale`: loja, give, give-all, caixas, gacha); o
relatório só nasce do pergaminho. No login, `TikiReport.Stale` apaga, além dos vencidos, os relatórios sem prazo, com
parâmetro 0 ou sem linhas legíveis (log "relatório(s) do Tiki vencido(s) ou sem dados apagado(s)"). O banco não foi
editado à mão: o item do dono sai no próximo login.
