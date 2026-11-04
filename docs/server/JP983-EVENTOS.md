# JP 983: eventos do cliente e o estado de cada um (atualizado em 04/11/2026)

Levantado no `ProjectG.exe` do JP 983: classes (RTTI), nomes de janela, a tabela de botões da página inicial
(`0xE0D930`) e as travas `IsLocalContent` (bitset em `0xF02E08`, montado só por `InitLocalizeSystem` em `0xA7A9B0`; a lista
dos ligados está em `docs/server/protocolo/SPEC-jp983-world-tour.md`). A numeração das travas é a do JP, não a do código KR.

Estados:
- **feito / testado**: o servidor atende e foi conferido dentro do jogo.
- **feito**: o servidor atende e os testes automáticos passam, mas ainda não foi visto no jogo.
- **só a janela**: abre, mas o servidor não manda os dados (contadores vazios, nada a ganhar).
- **falta**: o cliente mostra, mas o servidor não atende ou o formato é desconhecido.
- **indeterminado**: a janela existe, mas não achei quem a abre (criada pelo nome, pela fábrica de janelas).

## Modo de teste dos eventos (`JP983_EVENTOS=1`)
O 983 deixa vários eventos desligados no próprio cliente; o servidor sozinho não consegue ligá-los. Com a variável de
ambiente `JP983_EVENTOS=1` (cliente compilado com `JP983_SERVER_CONFIG`), `EnableTestEvents983` (`shared/localize.cpp`) liga
as travas 0x16, 0x19, 0x1F, 0x21, 0x22, 0x4D, 0x77, 0x78, 0x80, 0x82, 0x89 e 0x8B. Sem a variável, o cliente se comporta
como o 983 original.

No modo de teste:
- `icontentsdoc.cpp` cria os documentos que o 983 não cria (World Tour, Halloween, Natal, Ticket Exchange, Mission). Sem
  eles os botões não abrem nada e as respostas do servidor caem no vazio.
- O botão **Combo** do Papel Shop aparece (o 983 o esconde sempre em `0x8B3E40`).

**Arte que o 983 não tem.** Os botões desses eventos no lobby estão comentados no `lobby.xml` do 983, e as imagens das
janelas do Bingo, do Mission e do Natal 2007 (incluindo a neve `snow1..3`) não existem nos paks do 983; só no cliente KR.
Essa arte é de terceiros e **não está neste repositório**: para ver as janelas é preciso montar localmente um pak com o
`lobby.xml` reativado e as imagens tiradas de um cliente KR que você tenha. Sem a arte, o cliente não trava (os pontos que
liam imagem nula foram protegidos), mas as janelas aparecem incompletas.

## Eventos do lobby
| evento | trava | pacotes (cliente → servidor → cliente) | servidor | estado |
|---|---|---|---|---|
| Bingo | 0x82 | `0xE3` (u32 0 abre, 1 sorteia) → `0x18B` info/sorteio, `0x18C` prêmios | `JpSystems.Bingo`, `BingoService` | **feito / testado** (sorteio e marcação) |
| Mission Event | 0x80 | `0xE1` → `0x189`; `0xE2` (1 dia, 2 período) → `0x18A` | `JpSystems.Mission`, `MissionEventService` | **feito** (janela testada; números corrigidos) |
| Combo do Papel Shop (Big Papel) | — | `0x186` → `0x26C` | `BigPapelAsync` | **feito** |
| Grand Zodiac / HIO Event | — | — | `GrandZodiac`, `ZodiacSchedule` | **feito / testado** (abre no horário) |
| Natal 2007 | 0x4D | — | — | **só a janela** (corrigido o travamento da neve) |
| Halloween 2007 | 0x22 | — | — | **só a janela** (caixas de doce e corridas vazias) |
| Aniversário da Arin 2014 | — | `0x190..0x194` → `0x278..0x27C` | `JpSystems.Arin` | **só a janela**: anda pelas etapas; missões e prêmios oficiais desconhecidos |
| Evento da oficina 2013 (Workshop) | — | `0x24E`/`0x24F` | `JpSystems.Workshop` | **feito** (barra e EXP); contagem real pendente: o consumidor da mensagem `0x753C` (conteúdo 0xAE) não foi achado |
| World Tour | 0x1F | ver SPEC-jp983-world-tour | pronto | botão aparece só no modo de teste |
| Ticket Exchange | 0x77 | — | — | **falta** |
| Point Event | 0x16 | `0x98` | — | **falta**: o `0x98` é o mesmo pacote do Papel Shop; o formato próprio não é conhecido |
| Aniversário do Kooh | 0x8B | `0xED` → `0x278` com erro | `EventUnavailable` | **falta**: responde "indisponível" para não deixar o cliente esperando |
| Find Ebort | 0x21 | — | — | **falta** |
| Treasure Island | 0x89 | — | — | **falta** |
| Holes Event (botão "event1", `ntHolesEventDlg`) | 0x19 / 0x49 | — | — | **falta** (regras desconhecidas) |

### Regras do servidor
- **Bingo** (`GameConfig.Bingo`): 10 cartelas 5×5 (a do jogador é `uid % 10`, iguais às tabelas do cliente). Começa com
  `StartTries` fichas (3); cada partida terminada fora do Practice com pelo menos `MinHoles` buracos dá `TriesPerGame` (1),
  até 99. Linhas: 5 horizontais, 5 verticais e 2 diagonais (12 bits). Cada linha nova manda `LinePrizes` por carta (padrão:
  1 Gacha Ticket). Tabela `bingo` (migração 026).
- **Mission** (`GameConfig.Mission`): regras tiradas da arte original. Do dia: 3 partidas de versus e 2 de torneio, zera
  à meia-noite. Do período: 30 versus, um putt de 30y, um recorde novo, 30 eagles e todos os campos em 18 buracos (máscara
  `0xEFFF`, o campo 12 não conta). O prêmio é sorteado pelo peso entre `DayPrizes`/`TermPrizes` e vai por carta. Tabela
  `mission_event` (migração 027).
- **Grand Zodiac** (`GameJp.ZodiacSchedule`): lista de `"HH:MM int|adv"` no relógio do servidor; vazia desliga.

## Outros sistemas já atendidos
| evento / sistema | onde |
|---|---|
| Missão diária (OneDay Quest) | JpSystems.DailyQuest |
| Presença diária | JpSystems.Attendance (testado) |
| Grand Prix (lista de eventos do GP) | JpSystems.GrandPrix (testado) |
| Golden Time, Bot GM Event, letreiro | JpGoldenTime, JpBotGmEvent, Ticker |
| Anéis que se gastam (por hora / por buraco) e bônus do anel da sorte | `Rings` |
| Treasure Hunter, Caixa Mágica, Point/Tiki Shop, Gacha, Premium | — |
| Prêmio de login, drops por buraco, missões do approach (sem janela própria) | — |

## Ainda sem servidor
| evento | o que se sabe |
|---|---|
| Special Box (só do JP: `CSpecialBoxHandler`, `trispecialbox`) | gatilho não encontrado |
| Caixas de item com tela: Fortune (Lucia/Ponta), presente/seleção do Kooh 2011/2013 | trava ligada (0x70/0x58); o servidor não confere |

## Indeterminados (janela existe, quem abre não achado)
Valentine (`NtValentineEventDlg`, combinações 0x6E/0x6F), New Map Event, popup do Natal 2011, aniversário
(`FrBirthdayEventDlg`, 0x15B), Mighty MAC, Return Gift, Pang Gift, App Treasure Gift, Common Event About. O Event Prize é
aberto pelo tratador de pacotes do lobby (`0xABC140`).

Ghost (gravar/baixar voltas): fora desta lista, por decisão.
