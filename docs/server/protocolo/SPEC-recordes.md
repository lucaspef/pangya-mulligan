# KR 645: avisos de recorde (início da partida, durante e fim) e o que o servidor manda

Marcas: [C] lido no cliente 645 (`gh/` = <ghidra>, `asm` = <pg645.asm>, `opc.c` = decompilação completa de
`CTask::OnPacketCommon`, a mesma citada em SPEC-caixas-reciclagem.md; `case N` de lá é índice da jump table, aqui sempre
cito o **id do pacote**), [R] rebang (<rebang>/source/client/ProjectG), [GB] servidor de referência
(<ref>/Server/GB), [I] inferido, [P] política nossa.

Ids S->C: GB (JP) e KR 645 estão deslocados em **-2** nesta faixa: JP 0xB4 SP_TROPHY_LIST = KR 0xB2 (`u8 u8 u8 + 0xC×n`),
JP 0xBA SERVER_TIME = KR 0xB8 (`SYSTEMTIME` → `SetServerTime`), logo **JP 0xB9 SERVER_NEW_RECORD = KR 0xB7**. [C][GB]

## 1. Quais telas existem

| Quando | O que aparece | Onde no cliente | Fonte dos dados |
|---|---|---|---|
| **Início** (intro do 1º buraco) | painel `[complete.png` + número; painel `[score_event.png` + melhor placar do curso (`"-"` se 127) | `CHoleInfo::Display` @0049bb60 (gh/fontinfo.c L15340-15405); sprites carregados em `CHoleInfo::CHoleInfo` (L12350-12352) | `m_userInfo[me].stat.nMannerCombo` (0x74) e `m_myInfo.mapStat[curMap].cBestScore` (0x42/0x43) |
| **Fim** (ao voltar para a sala) | janela **"new_record"** (`FrNewRecordDlg`) | `CLobbyMain::OpenNewRecordForm` @007e55b0 (gh/lobbymain.c L13016) | flag `Doc[0x4d7e]` posta pelo **S->C 0xB7** |
| Durante | dica ao passar o mouse no mini placar (`[best_record_l/r.png`: pang, drive, chip-in, putt mais longos **da partida**) e o "maior drive" (`CDrivingDistInfo`) | `CMiniScore::Display` (gh/gui_control.c L6778-6830), `CDrivingDistInfo` (L7700) | `Doc.m_holeStatistics` (Doc+0x4118), **calculado pelo cliente**; o servidor não manda nada |

Não há string "신기록" no .exe [C]: o texto/arte das telas está no pak (formulário `new_record` e os .png). O conteúdo
visual exato é [I].

### 1.1 Início: painel do 1º buraco [C]
`CHoleInfo::Display` (fontinfo.c L15340-15405), dentro do fade do hole-info (`this+0x8c`, posto em 255 por `Flicker`):

```
if (golfdoc[0x180] == 0 && golfdoc[0x168] == Doc[0x45f2]) {      // 0x180 = 0 só antes do 1º "próximo buraco"
                                                                  // (DoApproachNextHole põe 1, golfrule.c L12846);
                                                                  // Doc[0x45f2] = 1º buraco da ordem do 0x50
    desenha "[complete.png"  em (x, 202)
    print "%d", Doc[0x1490 + idx*0xB92]       // idx = MyIndex(), ou 0 em mass game
    if (Doc[0x476d] == 18                     // buracos (0x50)
        && Doc[0x4a49] != 0x0E                // tipo de sala 0xE (curso por buraco) não mostra
        && (Doc[0x4668] == 0 || IsMassGame())) {   // stroke ou torneio
        desenha "[score_event.png" em (x, 242)
        c = Doc[0x694 + 0x1D + GetCurMap()*0x2B]   // m_myInfo.mapStat[curso].cBestScore (clássico: 0x9F0)
        print c == 127 ? "-" : "%d", c
    }
}
```

Offsets: `m_userInfo[4]` (sUserInfo 0xB92, vem do **0x74**) começa em Doc+0x12D0; `info` 0x10D a partir de +2, logo
`stat` = Doc+0x13DF (confere: `Level` = +0x4A = Doc+0x1429, `i64Pang` = +0x4B = Doc+0x142A, ambos usados em
golfrule/golftask). **Doc+0x1490 = stat+0xB1 = `nMannerCombo`** (sPangYaUserStatistics, `shared/globalgamedefine.h:213`
[R]). O número do "complete" é o combo de partidas completadas seguidas.

`m_myInfo.mapStat[]` vem do 0x42 (login) e é sobrescrito pelo 0x43 ao fim de cada partida (`Doc+0x694 + i*0x2B`,
opc.c L3043 case 0x11). Ou seja, a próxima partida no mesmo curso já mostra o recorde novo.

### 1.2 Fim: janela "new_record" [C]
- **S->C 0xB7** (opc.c L7617, índice 0x37): `u8 v` → `Doc[0x4d7e] = v` (`m_newRecordState`, asm 0x739469). Se a task
  corrente for `CLobbyTask`, posta a mensagem 0x53 → `OpenNewRecordForm`; senão (na partida, `CGolfTask`) só guarda.
- `OpenNewRecordForm` (lobbymain.c L13028-13035; rb lobbymain.cpp:11963): se `Doc[0x4d7e] != 0xFF` e a janela não
  está aberta: `CreateForm<FrNewRecordDlg>("new_record")`, abre, e põe `Doc[0x4d7e] = 0xFF` (mostra uma vez).
- É chamado em `OnGameRoom_Finish` (L21322, entrar/voltar à sala normal), `OnGameRoomExtRes_Finish` (L13818, sala de
  torneio/mass) e na mensagem 0x53. Então: **mande o 0xB7 a qualquer momento no fim da partida; a janela abre quando o
  jogador volta para a sala**.
- O valor só é comparado com 0xFF (único escritor fora isso: init `Doc[0x4d7e]=0xFF`, asm 0x41911D). No 645 o
  `FrNewRecordDlg` não tem handlers (só ctor/RTTI/dtor em gh/lobbymain.c L3989-4146): é o layout estático do pak. O
  rebang (newrecorddlg.cpp) tem uma lista de cursos escurecendo os com `cBestScore >= 0 || == 127`, de outra versão [R].
- O resto do `OpenNewRecordForm` (holes_event com `IsLocalContent(0x18)`, Halloween 0x21) não está ligado no KR [R].

## 2. Layout byte a byte

| Pacote | Layout | Efeito |
|---|---|---|
| S->C **0xB7** | `u16 0xB7, u8 curso` (qualquer valor ≠ 0xFF; use `curso & 0x7F`) | janela "new_record" ao voltar à sala |
| S->C 0x43 (já existe) | `sPangYaUserStatistics 0xEB, sTrophyStatistics 0x4E, u8 iNormal, [sMapStatistics 0x2B], u8 iClassic, [0x2B]` | atualiza `m_myInfo.stat` e `mapStat[iNormal]` (HUD do próximo início) |
| S->C 0x74 (já existe) | `u8 0, u8 n, n × {sUserInfo 0xB92, SYSTEMTIME, u8 cards, cards × 0x41}` | `stat.nMannerCombo` (sUserInfo+0x1C0) → número do "complete" |
| S->C 0x42 (já existe) | `... sUserInfo 0xB92 ...` | `m_myInfo.mapStat[]` → melhor placar do início |

`sMapStatistics` (0x2B) e as regras de `bMap`/`cBestScore = 127` estão em SPEC-perfil-mapas.md §1/§4.

## 3. O que o GB faz [GB]
`GameBase.requestSaveRecordCourse` (GameBase.cs L2251-2373), chamado no fim do jogador em Versus.cs:549,
PangBattle.cs:2073, GrandPrix.cs:2491, Tourney.cs:1560/1649, com `option = 1` só se a sala é de 18 buracos e ele chegou
ao 18º (`findHoleSeq(hole) == 18`; GP/torneio também com `END_GAME`):
- mapa = `m_ri.course & 0x7F`, tabelas separadas normal/natural/GP e com/sem assist;
- **recorde** se `best_score == 127 || score < best_score || pang > best_pang`: atualiza best_score/best_pang/personagem;
- soma os contadores (tacada, putt, hole, fairway, hole_in, putt_in, total_score), grava no banco;
- se recorde: **+1000 pang** e **S->C JP 0xB9 `u8 curso & 0x7F`** (= KR 0xB7).
- `combo` (= nMannerCombo): +1 por partida completa (GameBase.cs L1627/L1715), `-DECREASE_COMBO_VALUE` ao sair/cair
  (L1652; 3 no GB, "no JP é 10"), nunca abaixo de 0; `all_combo` (= nMaxMannerCombo) acompanha o máximo
  (pangya_game_st.cs L1204-1222). Vai no sUserInfo de todos os pacotes.

## 4. O que o nosso servidor manda hoje
- **0xB7: nunca.** Nenhum envio em src/ (grep `0xB7` só acha o C->S `CQuickMatch`, GameHandler.Social.cs:16). É o que
  falta para a tela de fim aparecer.
- `CourseResult.ApplyTo` (src/Pangya.Domain/Players/CourseRecord.cs:28-41) já decide recorde (`Holes >= 18 && Score <
  BestScore`), mas `Rewards.ApplyAsync` (src/Pangya.Domain/Players/Rewards.cs:51-77) não devolve essa informação
  (`Reward(Pang, Exp, LevelsUp)`, L11).
- 0x43 com o mapStat do curso: OK (GameHandler.GameEnd.cs:74-78; chamadores InGame.cs:128 só stroke,
  MassOutput.cs:151 torneio). O HUD do próximo início já usa o recorde novo.
- 0x42/0x74 com `mapStat[i]` (`bMap = i`, 127 sem recorde): OK (PlayerStructs.cs:190-200, 222).
- **`nMannerCombo`/`nMaxMannerCombo` = 0 sempre**: `PlayerStructs.Statistics` (PlayerStructs.cs:146-162) não preenche e
  `PlayerStats` (src/Pangya.Domain/Players/PlayerStats.cs) não tem o campo. O painel "complete" do início mostra 0.
- Bônus de 1000 pang por recorde: não existe.

## 5. Correção recomendada
1. [C] **Mandar `0xB7 u8 curso`** quando a partida bater o recorde do curso. Lugar: `GameHandler.FinishAsync`, logo
   depois do 0x43 (GameHandler.GameEnd.cs:78), para o mapStat novo já estar no Doc. Para saber se houve recorde,
   `Rewards.ApplyAsync` passa a devolver `NewRecord` (ex.: comparar `courses[c].BestScore < old?.BestScore ?? 127`, ou
   `CourseResult.ApplyTo` devolver o flag). Vale para stroke (InGame) e torneio (MassOutput); não para approach/match.
2. [P][GB] Critério: o nosso (`18 buracos completos && placar < melhor`) é o certo para o HUD. O GB também conta
   "pang maior que o melhor" como recorde; se quiser igualar, use os dois, mas o aviso de placar é o que o jogador espera.
   Bônus de +1000 pang como o GB é opcional (o pacote de pang `SPang` já é mandado em FinishAsync, GameHandler.GameEnd.cs:81).
3. [GB][P] **Preencher `nMannerCombo`/`nMaxMannerCombo`**: novo `PlayerStats.MannerCombo/MaxMannerCombo`, +1 em partida
   completa (em `PlayerStats.After`), −3 (mín. 0) quando sai/cai antes do fim; mandar em `Statistics()`. Isso dá o número
   do painel "complete" no início.
4. Nada a fazer para "durante": os melhores da partida (drive/chip-in/putt) são do cliente.
5. Teste: jogar stroke 18 buracos num curso sem recorde → ao voltar à sala abre "new_record"; nova partida no mesmo curso
   → intro do 1º buraco mostra o placar no painel `score_event` e o combo no `complete`.

## 6. Pontos em aberto
- Arte/texto do formulário `new_record` e dos .png (pak, não extraído) [I].
- Se o oficial mandava 0xB7 com outro valor (o cliente só testa ≠ 0xFF).
- `Doc[0x4a49] == 0x0E`: tipo de sala que muda de curso por buraco (o 0x50 lê um curso por buraco nesse caso,
  opc.c L4588-4606); não mostra recorde.
