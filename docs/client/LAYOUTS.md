# LAYOUTS — mapa de layout das classes que mais quebraram

Registro técnico. As referências a `out/`, `build/`, `ref/` e `tools/cmpfunc` são do ambiente de reconstrução e não acompanham este repositório.

Objetivo: tamanho e deslocamento de cada campo no 983 × o nosso, corrigir o nosso e travar com assert de compilação no build.

## Como se mede (refazer depois de mexer num cabeçalho)
```
tools/cmpfunc build/pg-TAG/obj/src/projectg/X.obj --no-build --ref ref/ProjectG_983_dump.exe --map map/projectg.tsv --out DIR/cmp/X
python3 tools/f35/campos.py Classe cabeçalho.h > DIR/Classe_nosso.txt          # deslocamentos como o nosso cl vê
python3 tools/f35/campos983.py X.obj DIR/cmp/X/funcs.tsv DIR/Classe_nosso.txt Classe > DIR/Classe.tsv
python3 tools/f35/dispdiff.py X.obj DIR/cmp/X/funcs.tsv                      # deslocamentos só de um lado, por função
```
`campos983.py` conta, nas funções pareadas da própria classe, os acessos `[reg+disp]` dentro de cada campo: `ambos` = o 983
usa o mesmo deslocamento (confere); só `só983`/`sóobj` = suspeito (olhar com `cmpfunc --diff`). Campo sem acesso nas funções
da classe = sem prova pelo uso (acessado por outras unidades): vale o sizeof e os asserts abaixo. Cuidado: `[reg+0xb8]` depois
de `fs:[0x2c]` é TLS, não campo. Tabelas desta sessão: out/F5/layouts/*.tsv (build a192-1, antes das correções).

## Situação por classe (A193)
| Classe | sizeof 983 | campos | confere | suspeito | sem uso na classe | estado |
|---|---|---|---|---|---|---|
| CGolfDoc | 0x8c0 (new 0066dce3) | 91 | 34 | 0 | 57 | igual (diferenças só de geração de código) |
| CSharedDoc | singleton (0x11648 nosso) | 252 | 158 | 15 → 0 reais | 79 | 3 defeitos corrigidos (abaixo); o resto é código, não layout |
| FrWnd (base de toda janela) | 0xd0 (FrForm começa em +0xd0) | 24 | 24 | 0 | 0 | igual (A206: +0x98 SetIconRect 00be61e0, +0xc8 DoCreate 00be3757) |
| FrForm | 0x110 | 14 | 13 | 0 | 1 | igual (o 983 zera +0xd1/+0xd2 com um `word`) |
| CTaskWidget | 0x1f8 | 8 | 8 | 0 | 0 | igual |
| CGolfPlayer | 0x29c | 47 | 40 | 1 (m_extEffect +0x278, só HandleMsg do 983) | 6 | partida = etapa 2 |
| sRoomInfo (sala) | 0xdd (pacote 0x49) | — | — | — | — | corrigido: título de 64, sem senha |
| sMascotInfo (jogador) | 0x3e | — | — | — | — | corrigido: Remain_Date de 1 byte |
| FrMakeRoomDlg (criar sala) | 0x210 | 11 | 11 | 0 | — | igual (A216; C_ASSERT) |
| FrGameRoomDlg (janela da sala) | 0x430 (new 0050ac20) | 134 | 94 | 0 (3 falsos: par errado do cmpfunc; +0x20c é do Doc) | 46 | igual (A217; C_ASSERT) |
| ntAvatarBurden (Lounge) | 0x2dc (copia 0xb7 dwords 007b3ec0; nó 0x2e4 007b4069) | 5 | 5 | 0 | — | corrigido A219: as duas visões do cabeçalho (real 0x2d8, parcial do avatar_task 0x294 com m_pPet em +0x28c) = 0x2dc, pet +0x2d4; C_ASSERT |
| ntAvatar (Lounge) | 0xc88 (new 007a8715) | ~30 do ctor | até +0x754 com +0x68 | base AvatarBase (0x68) e std::list de 8 B faltam (cauda interna) | — | A224: pad do 645 removido (charInfo 0x421 = 983 0x489); as 4 visões parciais seguem NTAVATAR_OFS_* com C_ASSERT; m_state983 +0xf0 (era +0x158 = lixo no pacote 0x63) |
| sBriefUserInfo (jogador da lista) | 200 (entrada do 0x0046) | 9 | 9 | 0 | — | igual (A215; 983 00af98f0) |
| sUserInfo / sPlayerData | 0x31c6 / 0xdb0 | — | — | — | — | sizeof confere (V3); chatBlock +0x56c |
| sPangYaUserInfo (jogador, 0x0044) | 0x12d | 24 | 13 | 1 → corrigido | 10 (+0x4c..+0x6b sem leitura) | corrigido A223: sGuild 20 (não 21), emblema em +0x40; C_ASSERT |
| IFF_ITEM_COMMON (cabeçalho dos itens do .iff) | 0xc0 | 26 | — (medido nos dados do .iff, não pelo campos983) | 0 | — | 24/10: bloco Tiki Shop (+0x84..+0x9b) e DateFlag (+0x9c) com nome; C_ASSERT; ver docs/IFF_ITEM_COMMON.md |

## Defeitos achados e corrigidos (A193)
1. **CSharedDoc +0x381c/+0x3834**: o nosso tinha `m_questList` (sQuest, 16 B) em +0x381c, onde o 983 guarda a lista de períodos
   de carta (registro 0x41; push_back 00641410, CCardManager::CalcCardPeriodAndStatus983 0080a810), e o cardsystem lia a mesma
   lista como cartas (tipos misturados). As missões do 983 ficam em +0x3834 (My Room 0061c56b; ClearInventory 00647800).
   Agora: `m_cardPeriodList` (+0x381c) e `m_questList` (+0x3834); os acessos crus `Doc()+0x381c` saíram (cardsystem,
   pktviews983, subgolfrulebase). ClearInventory na ordem do 983 (limpa +0x3840, +0x384c, +0x3828; zera +0x3860).
2. **CSharedDoc +0x10d24/+0x10d28**: a máscara de serviços do 983 é u64 em +0x10d28 (IsControlServerService 0062ba40 recebe e
   devolve `__int64`); +0x10d24 não é usado. O ClearCommonVars zerava +0x10d24 e deixava a máscara velha. Agora
   `m_controlServerService` é `unsigned __int64` em +0x10d28 (pacotes 009c3c1e/009cc6a9 gravam direto).
3. **sMascotInfo**: no 983 Remain_Date tem 1 byte em +0x2b (GetNumAddItemSlotByMascot 0063dca0 `cmp byte [+0x2b]`), endDate
   em +0x2d e PCBangMascot em +0x3d (FrMascotInfoDlg 00a806fc). O nosso tinha `short` (endDate em +0x2e, data do prazo
   errada). mascotinfodlg passou a usar os campos em vez de deslocamentos crus.
4. **sRoomInfo**: o 983 grava o título com `memset(+0, 0x40)` / `strncpy(0x3f)` e tira o bPublic do pacote (OnPacket 0x48,
   00ac0ea5); não há senha na struct. Agora `title[64]`; o OnPasswordInit do FrChangeRoomInfoDlg começa vazio (antes mostrava
   o fim do título quando ele tinha mais de 32 caracteres).

5. **sPangYaUserInfo sGuild/szEmblemName** (A223): no 983 sGuild tem 20 bytes e o emblema fica em info+0x40 — conv_ghostdata_to_userinfo
   00828130 copia nick/guilda/emblema com py_strcpy de 22/20/12 (00827f30/00827f70/00827fb0) em ebx+0x18/+0x2e/+0x42 (sUserInfo+2), e
   FrMessengerDlg lê o emblema da guilda em Doc+0x7e2. O nosso tinha sGuild[21] (emblema em +0x41, 1 byte adiante). Agora sGuild[20],
   szEmblemName +0x40, `unk983_4c[32]` até school (+0x6c). A cauda (+0x6c..+0x129) confere com `docrefs.py 7a2 12d` (school 0x80e,
   gallery 0x816, bits 0x832, Bongdari 0x835/37/39, pointEvent 0x83b, flagBlock 0x83f, timeBlock 0x843, displayID 0x84b, UID 0x8cb).
   messengerdlg deixou o `sGuild + 20` cru. O servidor C# grava o emblema em +0x3d (layout dele, não do 983).

## Asserts no build (C_ASSERT, compilam com o exe)
- CGolfDoc::CGolfDoc (golfdoc.cpp): sizeof, m_pSky, m_pTeam, m_playerNum, m_pHoleData, m_cameraData, m_currentPlayer,
  m_partTidList, m_approachGameResult; A717: m_holeResult983 0x340 (sHoleResult983 0x21 x4), m_shotSync983 0x45c, m_remoteGauge983 0x48e.
- CSharedDoc::CSharedDoc (shareddoc.cpp): m_myInfo, m_userInfo, sUserInfo, as listas 0x381c..0x384c, m_cookie, m_cash,
  m_controlServerService, m_chatManager, m_roomInfo e sRoomInfo (sizeof, bPublic, nUserLimit, nHole, holeType,
  realGameType), sMascotInfo, m_myInfo.info.dwIdentity == 0x812 (A199; 983 005fae60 lê o bit 14 = premium).
- A201 (shareddoc.cpp, escopo de arquivo): convite/"ir junto" 0x11288..0x11380 (serverTimeTick, goWithMode/FriendUID,
  inviteRoomInfo, directMoveType/Room/UID = 0062bc40..0062bcc0, friendGameSvrUID, inviteMode) e lobby/chat 0x10b26..0x10d08
  (bBackgroundVideo, 10b27, bRebuildUnderBar, selChannel, chatLineList/History, ignoreList, lobbyExitReason, chatModeName).
- A203 (shareddoc.cpp): 29 campos 0x10d28..0x11028 com `docrefs.py 10d28 5c` / `10fd4 58` / `11008 24` = o nosso: controlServerService
  (__int64), offlinePlay, pcBang, roomSetupStage, serverProperty, startHole..newRecordCourse, tutorialComplete[3]/StartMode (=0xf),
  giftName (string; tamanho/capacidade em +0x10d70/74), giftIndex (=-1), bRefreshCamera/MessengerAlarm/Init, levelTable, time/guild
  (teamPlayerGuid, guildBonusPang/Point/Team), os 2 vetores 0x10fe8/0x10ff8, authTicket, pGolfDoc, pendingMenu.
- FrForm::FrForm (frform.cpp) e CTaskWidget::CTaskWidget (jp/taskwidget.cpp): sizeof e os campos usados por Open/Close.
- A206: FrForm::FrForm trava também o FrWnd inteiro (22 campos por offsetof(FrForm, ...) + sizeof(FrWnd) == 0xd0; m_guiType983 e
  m_pToolTipData são privados e ficam presos entre +0xc4 e 0xd0) e os 13 campos do FrForm; CTaskWidget com os 8 campos.
  FrForm +0xf8 (m_unk_f8) segue sem acesso no 983 (ctor 00be5f00 / Open 00be6520 / Close 00be6098): nome provisório.
  Tabela: out/F5/layouts/FrWnd.tsv (cmpfunc frwnd.obj pg-a197: 77 IGUAL de 83).
- A209 (lobbymain.cpp, CLobbyMain::AnchorMotionBtn983): CLobbyMain +0x4b0 m_pMotionBtn983, +0x4b4 m_pMotionDlg983, +0x750/+0x760
  WRect do recurso (motion_btn_start / portrait_jukeboxbtn, Init 00ab7430/00ab76b0), +0x770 m_pToppageJukeboxBtn983, +0x774
  m_pJukeBoxDlg983. C_ASSERT dentro do membro (campos protegidos: no escopo de arquivo dá C2248).
- A215 (jp/roomlistdlg.cpp, escopo de arquivo): sBriefUserInfo sizeof 200 + dwGuid 4, roomIndex 8, sNick 0xa, dwIdentity 0x21,
  dwTitle 0x25, m_GuildId 0x2e, unk983_32 0x32, state 0x42 (pacote 0x0046 do a214-3 + Ghidra 00af98f0); prova negativa C2118.
- A216 (makeroomdlg.cpp, FrMakeRoomDlg::FrMakeRoomDlg): sizeof 0x210 (983 new 00ab482a) + m_pMakeBtn 0x110, classictype_btn 0x15c,
  map_ctrl 0x180/0x184, artifact_ctrl 0x188..0x1a4, m_artifact983 0x1fc (cursor +0xc, tid do no +0xc), m_classicFlags 0x20c; o pacote
  0x08 (00ae2203/00ae2238) deixou de ler this+0x20c/this+0x1fc cru. Prova negativa C2118 (linha 143).
- A217 (jp/gameroomdlg.cpp, struct FrGameRoomDlgAcc983 = amigo já declarado no cabeçalho): sizeof 0x430 + os 134 campos anotados
  (+0x140..+0x408); campos983 (cmpfunc do obj jpbuild): 94 usados = 983; tabela out/F5/layouts/FrGameRoomDlg.tsv. Prova negativa C2118.
- A223 (shareddoc.cpp, CSharedDoc::CSharedDoc): sPangYaUserInfo sizeof 0x12d + sNick/sGuild/szEmblemName (deslocamento e tamanho),
  school, dwGalleryGuid, Bongdari, pointEvent, flagBlock, timeBlock, sDisplayID, dwUID; o pch velho (pg-a220) dava C2118 = prova negativa.
- Fora do build continua o `make layoutcheck` (V3, include/asserts/), que colhe as anotações `// 983: 0xNN` dos cabeçalhos.

## Em aberto
- CSharedDoc: FECHADO (A204). A204 provou 0x1102c..0x11210 (22 campos: unused5060..countVisitor; `docrefs.py 1102c 1e8`, 0x111c8 =
  word por 00633140/00633150) e a cauda 0x11384..0x11640 (30 campos/blocos; `docrefs.py 11390 2b4`); 18 C_ASSERT em shareddoc.cpp,
  prova negativa (0x11641 -> C2118). Nenhum deslocamento divergiu. A211 tipou o resto: `m_playerCutinItem983[4]` (0x11448;
  0062be40 zera / 0062be60 grava / 0062be80 lê), `float m_unk983_114ac[3]` (00643830 fstp b0/b4) e `void* m_auxPartValues983[4]`
  (0x114b8; 0062bea0), com C_ASSERT; golfrule/taskmanager/shareddoc deixaram de usar cast cru (pg-a211 compila).
- CGolfPlayer A457: C_ASSERT sizeof 0x29c, +0x260/+0x268/+0x26c/+0x270 (power shot), +0x278 m_extEffect (golfplayer.cpp PowerShotInput).
- CGolfPlayer (partida, etapa 2): HandleMsg/SetMotion/PowerShotInput usam deslocamentos diferentes (+0x189, +0x158, +0x278) =
  código do 645 ainda no lugar; medir quando a partida voltar.
- sRoomInfo +0xc3: `unk983_c3` sem leitura direta no 983 (docrefs 109b7 só acha coincidências) — continua provisório.
- sRoomInfo +0xc5..+0xdc (A197, Doc+0x108f4+desl; `tools/f35/docrefs.py 109b9 18`): agora union de `unk983_c5[6]` com nomes
  (os usos antigos continuam): +0xc5 `artifactTid983` (CGrandPrixArtifactRule::IsTimed 00546900, início do jogo 009c2913),
  +0xc9 `roomFlags983` (00b30560/00b30570, OnPacket 0x48 00ac0dd6), +0xcd `gpTypeId983` (HandleMsgCommon 0065d995,
  FrGrandPrixAwardDlg 0054d1cd), +0xd1 `gpTicket983` (HasGrandPrixSpecialHole983 em 00511da7 = nosso SetGPRoom983, e 0072f32f),
  +0xd5 `gpStartValue983` (009c2958: copiado para m_golfGame.unk983_118, Doc+0x106a4), +0xd9 `unk983_d9` (sem leitura).
  C_ASSERT de todos em CSharedDoc::CSharedDoc. Os usos nossos de c5[0..3] conferem com o 983.
- ntAvatar (A224): cauda do 983 mapeada no A421 (abaixo); o nosso difere só por dentro (pet fora da base), sem leitura de outra unidade.

## A234: varredura de `new` (983 × nosso, todas as classes)
`tools/new983.py <objdump.dis> [endereço do operator new]` acha `push N; call operator new` seguido do ctor (ecx=eax).
983: operator new 0xc3c238; nosso (E162): 0xc0c1d4. Nomes pelos ctors em map/projectg.tsv e no .map do nosso link.
Resultado: out/F5/layouts/new_tamanhos.tsv (418 classes com par; 406 com o mesmo tamanho).
- **Defeito (corrigido A234):** FrItemAttachCardDlg tinha DOIS tamanhos no nosso exe: realmyroommainui.obj fazia new 0x390
  (declaração parcial de ccardbook.h, pad 0x268) e o ctor (carddlg.obj) escreve 0x3b0 = estouro de 0x20 B no heap ao abrir o
  anexar carta do My Room. Pad 0x288 + C_ASSERT dos 4 parciais em ccardbook.h; obj novo: `push $0x3b0`. Só essa classe tinha 2 tamanhos.
- Nosso MENOR que o 983 (só importa se código do 983 com deslocamento cru passar do fim): CPost_ReciveDlg 0x210/0x220,
  CSpot 0x260/0x274, CLetter 0x84/0x53c, ShotPreview 0x80/0x238 (partida), FrSpCardUseDlg 0x130/0x120 (maior).
  Nosso MAIOR: CLobbyMain 0xc6c/0x7a4, CRealMyRoomMain 0xe20/0xae0, CShopMain 0x720/0x6c0, CSky 0x918/0x8fc,
  FrCardAttachDlg 0x180/0x170, FrDemonNextHole 0x150/0x120 (campos do 645 que sobraram; sem queda conhecida).

## A236: CRealMyRoomMain — código com deslocamentos do 645 sobre o layout do 983
- O CTaskMain do 983 (= o nosso) tem 0x218; o do 645 tinha 0x1e0. Toda classe derivada andou +0x38 (CRealMyRoomMain: +0x38 de
  0x2c4, +0x44 de 0x6d8, +0x50 de 0x9e0 em diante, pelos nomes `m_X_<desl645>`). Base CHandOwner: 645 this+0x1e4, nosso this+0x21c
  (`lea 0x21c(%esi)` no obj pg-a234); m_hand: 645 0x7e0, nosso 0x824, 983 0x588 (ShowMainModel 0061a888: m_bUserLock = 0x58c).
- Corrigido A236 (ramo C++ = REBANG_SEMANTIC_ONLY): ShowPet/ShowMainModel (`this+0x810` = m_csUser do 645 -> m_hand.Enter/Leave),
  OnSelItemPrev/NextUp (0x7f0/0x7f4/0x7f8/0x82c -> m_hand.IsWorking(0), mesmo corpo do .inl), ChangeCharacter/SelectItemList
  (0x1e4 -> static_cast<CHandOwner*>), HandleMsg 0x26c (map em this+0x968 = DENTRO do m_hand -> m_mapChar). obj pg-a234 compila;
  0x810/0x7f8/0x82c/0x968 sumiram, aparecem 0x854/0x83c/0x9ac.
- **A237: FECHADO, alarme falso.** O asm naked NÃO é compilado: tools/f32/pgbuild.py:16 põe /DREBANG_SEMANTIC_ONLY em toda unidade
  (o `make exe` usa esses objs). Prova: pg-a234/realmyroommainui.obj tem 0 acessos `0x810(` (o asm tem `[edi+810h]`); os 4 `0x7e4`/1 `0x1e4`
  restantes são membros pelo compilador (layout nosso). Casts crus que sobram: +0x48/+0x15c (dentro do CTaskMain = 983);
  +0x154 virou `m_pOptionDlg` com C_ASSERT (case 0xc7; obj pg-a237 com o mesmo código).
- Varredura de `this+N` cru em fontinfo.cpp (CPangInfo) e golfrule.cpp: só classes da partida (etapa 2). Fora da partida: nada aberto.

## A259: CPostEffect983 (pós-efeito do 983) — campos do CSharedDoc/CGolfDoc com nome
- IsActive (005b0030) e LoadShaders (005afce0) liam doc+0x10594 / [doc+0x11024]+0x158 crus: agora `m_golfGame.gameType`,
  `m_pGolfDoc->m_gameMode`, com C_ASSERT CSharedDoc 0x10594/0x11024 e CGolfDoc::m_gameMode 0x158 (posteffect983.cpp; compila,
  jpbuild 1/1; cmpfunc das 2 funções igual ao de antes).

## A260: form "realmyroom" do 983 = CRealMyRoomMain (0xae0) — base CHandOwner em +0x120 (prévia do ponto 4 do dono)
- ctor 0061f1f0: FrTaskForm (00523690) -> +0 vtable 00d1497c (RTTI CRealMyRoomMain), +0x120 vtable 00d1495c (2ª base CHandOwner,
  1 virtual: OnAnotherHand 00611f50 -> key 0 = ShowPet 0060e8b0). O nosso CRealMyRoomForm983 só deriva de FrTaskForm (pad desde +0x120).
- Campos da prévia: +0x584 objeto da prévia (00605530 apaga via vtbl[0](1)), +0x58c flag, +0x5b8 CRITICAL_SECTION, +0x150/+0x1e4
  áreas (x,y,w,h em +0x34..+0x40), +0x228 widget; +0x12c objeto com vtbl+0x18(0) e +0x528=1 (0061aab0). Evidência: out/F5/inventario/a260/.

## A262: varredura de `new` refeita no exe do dono (reg-20261016-1520, HEAD ecadba3d, md5 43cb1473)
- `tools/new983cmp.py <983_new.tsv> <nosso_new.tsv> <nosso.map> [saida]` (novo): junta as 2 tabelas do new983.py pela classe do
  construtor COM namespace (o 1º script juntava `rf::TiXmlElement` 0x50 com `TiXmlElement` 0x48 = falso "2 tamanhos").
  Nosso operator new nesse exe: 0xc11a94. Tabela: out/F5/layouts/new_tamanhos.tsv (6ª coluna = situação).
- **0 classes com 2 tamanhos** no nosso exe; 407 pares, 397 IGUAIS, 10 DIFERENTES = a mesma lista do A234 (CLobbyMain, CRealMyRoomMain,
  CShopMain, CSky, FrCardAttachDlg, FrDemonNextHole, FrSpCardUseDlg maiores; CLetter, CSpot, ShotPreview menores = partida).
  FrItemAttachCardDlg (0x3b0) e CPost_ReciveDlg (0x220, B430) passaram a IGUAL.
- Classes "983" novas das frentes (sem nome no mapa do 983): sLobbyAvatarInfo983 0x248, sGPTimeBoard983 0x7c, ClubSetWorkshopFx983 0x4c
  (+sSlot983 0x6c), RFTextRun983 0xac, CGaugeBar983 0x4c, CRoomCheer983 0x54 (983 0053f0d0 = 0x54), FrAreaEntrance983 0x30 (00beb2e0),
  CAIPlayerInfoContainer983 0x34 (00915b90), WPolySoup::sGroundBoneNode 0x7c: todas com assert de sizeof = new do 983.
- A375: WPolySoup pointNum +0x94, m_point +0xc4; sPoint 0x64 (pos +0x24, pos2_983 +0x54); sPointNode 0x78 (m_local.pivot +0x38, m_world.pivot +0x68, m_rec +0x74) — C_ASSERT em polysoup.cpp (983 007059c0/009382c0/0092b190).
- rf::TiXml* (rftinyxml.obj): o 983 aloca 0x100/0x40/0x30/0xe8/0xd0 na fábrica 00c18008..c180c5, o nosso 0x30/0x38/0x50 (outra
  representação de string). Biblioteca fechada (só os métodos dela tocam os campos) = não é defeito; só importa se código do 983 com
  deslocamento cru passar a ler esses objetos.

## A337: WxBatchState / LightSet (contrato exe → wangreal.dll do 983, prévia 3D com material de shader)
- dll 983 ApplyMtlParams 100098a0 / xApplyShaderSysParams 1003c750: semântica 5 (g_mtlClr) = WxBatchState::xdwDiffuse +0x1c;
  6/7/8 (luz) = LightSet nearOne +4 / diffuse +0x10 / ambient +0x14. Trava: static_assert em include/engine/wscene.h
  (LightSet 0x1c, WxBatchState 0x60, xmW +0x20, xfDepth +0x50); prova negativa: +0x20 -> C2338 nos 17 pch (pg-a337n).

## A412: FrClubSetOptionalFunctionsWnd (Club Workshop > Maintenance, 0x160)
- Deslocamentos lidos pelo exe 983 em 004c27a0/004c2ba0/004c2fb0/004c3af0/004c3b60 (+0x88 origem, +0xa0 moveExp, +0xa8 recuperar,
  +0x144/+0x148 itens, +0x154 aviso, +0x15e trava 0x16b) travados com C_ASSERT no construtor (campos protected); prova negativa
  0x88 -> 0x89 = C2118 na linha 287 (jpbuild). sItemInfo typeId +4 / Common +0xc: A411 (clubsetworkshopchildwndbase.cpp).

## A414: FrClubSetWorkshopSelector (lista "Select a Club Set" do Club Workshop, 0x180)
- Deslocamentos lidos pelo exe 983 em 004a4ef0 (OnInit: +0x110 itens, +0x158 uso), 004a4540 (OnItemDraw: +0x120 lista, +0x134..+0x154
  fontes/posições, +0x160 escolhido, +0x178/+0x17c imagens) e nos chamadores 004c1f20/004c2060 (+0x158, +0x164 marcados) travados com
  C_ASSERT no construtor (clubsetworkshopselector.cpp); negativa m_kind 0x158 -> 0x15c não compila (jpbuild 0 de 1).

## A416: FrClubSetOptionalFunctionsWnd (Club Maintenance) — campos do envio no OK
- 004c3af0 lê [+0x88] origem, [+0x8c] destino, [+0x90] typeId, [+0x94] qtd e trava +0x15d; 004c3b60 lê [+0xa8] taco, [+0xac] typeId e
  trava +0x15e; ambos testam result == 1 (FrOK). C_ASSERT no construtor (clubsetoptionalfunctionswnd.cpp); negativa 0x8c -> 0x88 = 0 de 1.

## A418: sItemUpdate983 (registro do 0x216) — maestria nova do taco depois do Transfer
- 009bfc90 grava +8 uid, +0x1c counters[5], +0x28 std::string (move-assign: tamanho +0x38, capacidade +0x3c), +0x50 carta (byte),
  +0x54 bloco da oficina (só taco + kind 0xcc: slots, +0x60 exp, +0x64, +0x68, +0x6c), +0x70 mestria (0xcd), +0x74/+0x78 caddie (0xce).
  005891c0 copia o bloco para o sItemInfo do Doc+0x3740 (+0xaa..+0xc0). C_ASSERT em itemupdater.cpp; negativa 0x70 -> 0x74 = 0 de 1.

## A419: FrClubSetOptionalFunctionsWnd — aviso do resultado do Transfer/Recover
- 004c0f2a: +0x154 tipo (1 = exp movida, 2 = recuperação; posto por 004c3e20/004c3dd0 quando result == 0), +0x14c tempo += dt,
  +0x150 espera (fx_clubmanage_playtime, 004c6163); passou -> 004c0c70 "notify" (textos 0xd02a68/0xd02a8c, 004cd580/004cd5a0, 0x802),
  StopFx 3/4, solta +0x15d/+0x15e. = nosso. C_ASSERT +0x14c/+0x150 e LEFT_BUTTON == 0; negativa 0x150 -> 0x154 = 0 de 1.

## A421: CRealMyRoom_Avatar — visão parcial do realmyroommainui.cpp estava no layout do 645
- Medido no build (sonda `char (*)[offsetof]`, pg-a421): charInfo 0x421, CPartTidList 0x624, pet 0x6ec, m_bVisible (oculto) 0x71c,
  guid 0xdc, posição 0xf4, alvo 0x128, estado 0xe8; sizeof(CRealMyRoom_AvatarBurden) 0x2d4; manager m_pMyAvatar 0xa4.
- A visão parcial (troca de personagem no My Room 8928/9032, saída do avatar 37195) lia charInfo em 0x3d9, pet em 0x660 (dentro da
  CPartTidList) e o oculto em 0x690, e o burden na pilha tinha 0x290 B: o construtor do burden (0x2d4) escrevia 0x44 B além dele.
- Corrigido: RMRAVATAR_OFS_* em realmyroom_avatar.h, casts crus trocados por GetCharacterInfo()/GetPetFrame(); C_ASSERT nas duas
  visões; negativa (pet 0x660) = C2118. ntAvatar::GetPartTidList lia +0x598 (645) -> NTAVATAR_OFS_PARTTIDS.
- Cauda do ntAvatar no 983 (ctor 007a7320, dtor 007a6ee0): CPetFrame* em +4 e WBoneSet em +8 (base AvatarBase; o nosso põe o pet em
  0x6ec), CPartTidList 0x68c, std::list 0x754 (8 B), LightSet 0x764 (0x77c = 0xffffff), balão 0x7a4, rand[3] 0x7a8, emblema 0x7fc,
  chat 0x800, CPvsFxBox 0x804, SpecialPrizeItem[5] 0xc44 (0xc cada), shared_ptr<FxInterpreter> 0xc80, tamanho 0xc88. Só código nosso lê.

## A422: IFF_STRUCT::sPart — My Room lia os campos no layout do 645
- sPart do 983 = 0x238 (C_ASSERT taskmanager.cpp); 645 + 0x30 antes destes campos: Attr 0x1e4, Slot 0x1ee, CharacterSlot 0x228,
  CaddieSlot 0x22a, Beginner 0x234 (jp/myroomwidget.cpp, tirado do 983, já lia 0x228/0x22a).
- realmyroommainui.cpp (#else = exe): BuildDisplayStand (6 sítios), DrawPartsSlot e OnItemListOwnerDraw liam 0x1f8/0x1fa/0x200/0x1b4/0x1be
  (645): no 983 0x1f8 é EquippableWith[] = contagem de slots de carta/Beginner/stats da roupa errados nas listas do My Room.
- Corrigido com os campos nomeados + C_ASSERT; obj pg-a422: 0x1f8/0x1fa 5+4 -> 0x228/0x22a 5+4; negativa (Beginner 0x200) não compila.

## A423: FrWnd/FrButton (wangreal.dll original) e CSharedDoc — casts crus do makeroomdlg.cpp (SALAS)
- 983 escreve nos botões da sala FrButton+0x104 = 0, +0x108 = 0.1 (= SetPushDelay(0): m_pushDelay/m_pushedTime) e FrWnd+0x44 |= 0x200
  (m_dwStyle, FWS_NODBLCLICK = UseDblClick(false)); lê FrWnd+0x34/+0x38 = m_rect.x/y (posição do map_ctrl).
- Nosso cabeçalho src/fresh/include tem esses campos nos mesmos deslocamentos: C_ASSERT por classe derivada (FrButtonLayout983, acesso
  protegido) m_rect 0x34, m_dwStyle 0x44, m_pushDelay 0x104, m_pushedTime 0x108; doc: m_myInfo.stat.Level 0x91d, m_roomInfo.holeType 0x1094f.
- makeroomdlg.cpp: 8 botões + map_ctrl + 2 bytes do doc trocados por chamadas/campos nomeados; pg-a423 compila, negativa (0x100) = C2118.
- Restam crus: doc+0x375c (CItemCursor983), doc+0x10d3c (flags; jp/roomlistdlg.cpp tem sDocFlags983 local), sItem+0xc0 (artefato).

## A424: CSharedDoc/sItem — últimos casts crus do makeroomdlg.cpp (SALAS)
- doc+0x375c = m_myItemList (lista que o CItemCursor983 copia, 983 00adead0); doc+0x10d3c = m_serverProperty (0x10d28 controlServerService
  de 8 B, offlinePlay, pcBang, roomSetupStage; bit 7 = tipo do servidor como máscara, 00520900); C_ASSERT de offsetof dos dois.
- sItem+0xc0 = dword do bitfield RandomBox (IFF_ITEM_COMMON = 0xc0); `& 6` = bits 1-2 -> novo campo sItem::Artifact : 2 (classdefine.h,
  sem mudar o tamanho). makeroomdlg.cpp sem nenhum deslocamento cru. Ainda crus (outras frentes): shopmain.cpp:2709 (E), equipdlg.cpp:91,
  jp/myroomwidget.cpp:858 = mesmo `& 6` -> trocar por ->Artifact quando mexerem.
- Prova: pg-a424 (pch refeito: classdefine.h mudou) compila; objdump -d do makeroomdlg.obj pg-a423 × pg-a424 = 18818 linhas, diff 0
  (mesmo código); negativa m_serverProperty 0x10d40 = C2118 (pg-a424n).

## A472: CFxSequence — posição do efeito em +0x110c (não +0x1108)
- Layout real e o do #else (fxsequence.h) compilados: m_bActive = 0x1108, m_pos = 0x110c (= 983 0074e6d0 case 0x27f, 008b9810).
- 8 escritas cruas em +0x1108 (Safety safetee_ball.seq x2, Talk_Room.seq x6 no My Room/lobby) punham o vetor sobre m_bActive e deslocado; corrigidas. C_ASSERT em fxsequence.cpp.
