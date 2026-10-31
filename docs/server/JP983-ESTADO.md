# JP 983: o que funciona e o que falta (12/10/2026, noite)

Visão geral para deixar o cliente JP R7.983 jogável por inteiro. Como funciona: o game server JP usa a lógica do KR 645 e
o `JpTranslator` converte ids e structs (docs/server/protocolo/SPEC-jp983-game.md); o que só o JP tem fica em `JpSystems*.cs`
(com o Domain). Legenda: ✅ confirmado no cliente real · 🟡 feito e testado no código (xUnit + cliente JP falso), sem teste
no cliente · ⬜ falta.

## Confirmado no cliente
- ✅ Login, lista de servidores, canais, lobby, My Room, criar sala, stroke com bot, placar.
- ✅ Prática (campo e repetir buraco), tutorial, Papel Shop, Big Papel, Memorial Shop, perfil, nível 70 com EXP.
- ✅ Correio (lista, ler, pegar, apagar, mandar), presença diária com prêmio por carta, mensageiro e ranking do JP.
- ✅ Conquistas (lista e janela). Grand Prix: entrar, começar no horário, bots, próximo buraco (0x255), prêmios.
- ✅ Chip-in Practice do Grand Zodiac (pontos e bônus contados, tela de resultado).

## Feito, falta testar no cliente (🟡)
| sistema | spec | o que testar |
|---|---|---|
| Bônus "sem acertar o Pangya" (+3) no Grand Zodiac | SPEC-jp983-grand-zodiac §3.1 | acertar sem Pangya: +3 no placar do buraco |
| Oficina de tacos (nível, rank, transformar em taco especial, recuperar, transferir maestria, reset) e maestria ganha nas partidas | SPEC-jp983-oficina | abrir a oficina, subir nível/cancelar, rank-up, reset; a maestria sobe ao jogar |
| Cards no personagem (12 slots, patcher, removedor), maestria do personagem, stat ±1, Lolo, card por tempo somando o prazo | SPEC-jp983-cards | encaixar/remover card, relogar e ver o card; maestria com TP; Lolo |
| Missão diária (3 do dia, aceitar, progresso, prêmio, desistir) | SPEC-jp983-missoes §1 | abrir as missões, aceitar, jogar, pegar o prêmio |
| Point Shop (itens -> pontos -> vitrine) e Tiki Shop (milhagem -> TP) | SPEC-jp983-missoes §2 | a Point Shop abre; trocar itens; Tiki Shop dá TP |
| Relatório do Tiki (pergaminho no fim do torneio: sai com o pang; abrir o relatório dá o EXP) | SPEC-jp983-tiki-report | terminar um torneio com outro jogando, ESC, usar o pergaminho; abrir em My Room > Relatórios |
| Guilda pelo site do club (criar, procurar, pedir, aceitar, cargos, emblema) | SPEC-jp983-guilda §1 | botão club abre o site (precisa da regra do rugburn, abaixo) |
| Pedidos e convites de guilda no cliente (0xA5/0xA7, MSN 0x26, 0x118/0xA8) | SPEC-jp983-guilda §2 | mestre vê os pedidos; convidar pelo login no mensageiro |
| Grand Prix: vento pela regra, relógio do buraco (0x259) | SPEC-jp983-grandprix §5 | GP com tempo por buraco |
| Grand Zodiac de evento (HIO Event 11/13): `!zodiac int|adv`, horário na config | SPEC-jp983-grand-zodiac §1.2 | GM abre a sala, entra, joga; beam, jackpot, pontos GZ, copo |
| Efeitos de item na partida (asa, patas, anéis, brinco, luva), marcador no campo, Assist, Auto Command | SPEC-jp983-efeitos §1–§4 | equipar asa/anel e ver o efeito; botão Assist na sala |
| Buff por tempo (Sweet Yam, balas) e recarga de cometa | SPEC-jp983-efeitos §5, §8 | usar no My Room; a recarga soma 30..50 bolas |
| Trocar de servidor pela lista, letreiro (1 cookie) | SPEC-jp983-efeitos §6, §7 | mensagem no letreiro aparece para todos |
| Grand Prix: regra de 10 s/15 s, penalidade do power shot, buracos especiais, 0x199 no último buraco, bônus de clear | SPEC-jp983-gp-regras | GP com regra de tempo; terminar o último buraco |
| VIP (Premium Ticket): cometa VIP, mascote e set VIP, +5/10 % de EXP/pang, caixa de presente | SPEC-jp983-intrusao-vip-modos §2 | `/giveitem 437256195` (VIP) ou `437256194` (normal), relogar |
| Intrusão: entrar num torneio já começado (até 5 min) | SPEC-jp983-intrusao-vip-modos §1 | 2 clientes: um começa torneio, o outro dá duplo clique na sala |
| Match, dupla, pang battle, approach (com missões por buraco), Treasure Hunter | SPEC-jp983-intrusao-vip-modos §3 | criar sala de cada modo e jogar |
| Drop por buraco (medalhas, Mana, Special Shuffle, ticket do GP) | SPEC-jp983-eventos-drops §1 | jogar torneio: itens caem ao acertar o buraco |
| Prêmio de login por evento (padrão: 7 dias = Memorial Coin Premium) | SPEC-jp983-eventos-drops §2 | após 7 dias de login chega pelo correio |
| Golden Time (`!goldentime`) e Bot GM Event (`!botgm`) | SPEC-jp983-eventos-drops §3, §5 | GM digita no chat; janela de sorteio/prêmio |
| Gacha (site do jogo, botão no lobby) | SPEC-jp983-eventos-drops §6 | precisa da regra do rugburn (abaixo); jogar com Gacha Ticket |
| World Tour (16 campos com 18 buracos = prêmio por carta) | SPEC-jp983-world-tour | **o cliente JP 983 esconde o botão** (conteúdo local 0x1F desligado no exe) |

## Falta (⬜)
- Valores estimados (sem fonte oficial): pesos de raridade dos cards, preço do letreiro, chance de transformar taco,
  prêmios da caixa VIP, valores do Grand Zodiac. Os de evento (gacha, Golden Time, Bot GM, drops, login) ficam na config.

Eventos de época (Point, Find Ebort, Halloween, Natal, Ticket Exchange, Mission, Bingo, Treasure Island, aniversário
do Kooh): o código existe no cliente, mas o próprio exe JP 983 os desliga (tabela em SPEC-jp983-world-tour.md). Sem
patch no cliente, não há o que o servidor ligar.

## Config dos eventos (seção `GameJp` do config/pangya.json)
`LoginRewards`, `WorldTour` (`Enabled`, `Start`, `End`, `Prizes`), `BotGmEvent` (`Enabled`, `Windows`), `GoldenTime` (`Enabled`, `Rounds`), `HoleDrops`, `ZodiacSchedule`
(`["20:00 int"]`); o gacha fica na raiz (`Gacha.Items`). Sem nada na config valem os padrões do código
(src/Pangya.Core/Config/PangyaConfig.cs); Golden Time e Bot GM só por horário se `Enabled`, ou pelo comando do GM.

## rugburn (site da guilda e emblemas)
O cliente abre `https://www5.pangya.jp/on_client/club/...` e baixa emblemas de `http://www5.pangya.jp/upload/club/mark/`.
O `rugburn.json` do cliente precisa reescrever os dois para o servidor web (porta 30080):
`https://www5.pangya.jp/on_client/club` -> `http://127.0.0.1:30080/on_client/club` e
`http://www5.pangya.jp/upload/club/mark/` -> `http://127.0.0.1:30080/upload/club/mark/`.
O gacha precisa de mais uma: `https://www5.pangya.jp/on_client/gacha` -> `http://127.0.0.1:30080/on_client/gacha`.
