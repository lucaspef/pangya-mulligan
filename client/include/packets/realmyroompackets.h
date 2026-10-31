// realmyroompackets.h - frente C (pacotes). JP 983: dois tipos novos do pacote 0x20 (equipar) do my room que o KR 645 nao
// tem. No 983 sao metodos do CRealMyRoomMain (thiscall, 0060e6c0 e 0060e810): desde a A s98 (pedido C27) declarados em
// src/projectg/realmyroommain.h como Net_EquipCutIn983() e Net_EquipPoster983() (corpo em realmyroommainui.cpp); as notas
// abaixo ficam como documentacao.
#pragma once

// 0060e810, tipo 9: as 4 cenas de corte (sCharacterInfo::cutIn, +0x1b8) do personagem atual (CSharedDoc::FindChar983(-1)).
// O 983 chama depois do Net_EquipMascot (HandleMsg 00624374/006243b1, virtual 006165d0) ou do SendEquipItemSlot
// (SelectMenu 00622815, 00615dd0).
// void CRealMyRoomMain::Net_EquipCutIn983();
// 0060e6c0, tipo 10: os 2 posteres (sUserEquip::poster) se a copia do my room (CRealMyRoomMain::m_userEquip.poster,
// 983 this+0x78c) difere da do CSharedDoc; depois avisa o ator "LobbyBg" (mensagem 0x182). So na virtual 006165d0.
// void CRealMyRoomMain::Net_EquipPoster983();
