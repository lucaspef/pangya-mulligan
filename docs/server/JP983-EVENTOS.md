# JP 983: eventos do cliente e o estado de cada um (12/10/2026)

Levantado no `ProjectG.exe` do JP 983: classes (RTTI), nomes de janela, a tabela de botões da página inicial
(`0xE0D930`) e as travas `IsLocalContent` (bitset em `0xF02E08`, montado só por `InitLocalizeSystem` em `0xA7A9B0`; a lista
dos ligados está em `docs/server/protocolo/SPEC-jp983-world-tour.md`). A numeração das travas é a do JP, não a do código KR.

Estados:
- **desativado**: o próprio cliente esconde (trava desligada). O servidor não consegue ligar.
- **ativo / feito**: o cliente mostra e o servidor já atende.
- **ativo / falta**: o cliente mostra (ou não tem trava), mas o servidor não atende ou o formato é desconhecido.
- **indeterminado**: a janela existe, mas não achei quem a abre (criada pelo nome, pela fábrica de janelas).

## Desativados no cliente
| evento | trava (JP) | observação |
|---|---|---|
| Point Event | 0x16 | também esconde o botão "event1" |
| World Tour | 0x1F | servidor pronto (SPEC-jp983-world-tour), mas o botão não aparece |
| Find Ebort | 0x21 | |
| Halloween 2007 | 0x22 | |
| Natal 2007 (e a meia de Natal) | 0x4D | |
| Ticket Exchange | 0x77 | |
| Mission Event | 0x80 | também conferido no tratador do lobby (`0xABC140`) |
| Bingo | 0x82 | idem |
| Treasure Island | 0x89 | |
| Aniversário do Kooh | 0x8B | |
| Holes Event (botão "event1", `ntHolesEventDlg`) | 0x19 / 0x49 | |

## Ativos e feitos no servidor
| evento / sistema | onde |
|---|---|
| Missão diária (OneDay Quest) | JpSystems.DailyQuest |
| Presença diária | JpSystems.Attendance (✅ no cliente) |
| Evento da oficina 2013 (contagem de buracos, EXP) | JpSystems.Workshop |
| Grand Prix (lista de eventos do GP) | JpSystems.GrandPrix (✅) |
| Grand Zodiac / HIO Event e missão do HIO | GrandZodiac, JpZodiacEvent |
| Golden Time, Bot GM Event, letreiro | JpGoldenTime, JpBotGmEvent, Ticker |
| Treasure Hunter, Caixa Mágica, Point/Tiki Shop, Gacha, Premium | — |
| Prêmio de login, drops por buraco, missões do approach (sem janela própria) | — |

## Ativos ou sem trava, mas faltando no servidor
| evento | o que se sabe |
|---|---|
| Aniversário da Arin 2014 (só do JP: `CArinBirthDay2014Handler`, janelas de estágio e missões) | ícone no topo manda **0x192** vazio; a janela abre por um evento interno (`0x478720`); a resposta do servidor e as missões não são conhecidas (sem fonte, sem referência) |
| Special Box (só do JP: `CSpecialBoxHandler`, `trispecialbox`) | gatilho não encontrado |
| Caixas de item com tela: Fortune (Lucia/Ponta), presente/seleção do Kooh 2011/2013 | trava ligada (0x70/0x58); o servidor não confere |

## Indeterminados (janela existe, quem abre não achado)
Valentine (`NtValentineEventDlg`, combinações 0x6E/0x6F), New Map Event, popup do Natal 2011, aniversário
(`FrBirthdayEventDlg`, 0x15B), Mighty MAC, Return Gift, Pang Gift, App Treasure Gift, Common Event About. O Event Prize é
aberto pelo tratador de pacotes do lobby (`0xABC140`).

Ghost (gravar/baixar voltas): fora desta lista, por decisão.
