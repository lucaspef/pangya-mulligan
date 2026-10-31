// JP 983 (C682, limpeza F5): definições reais dos dados que eram stub do link (tools/v6/stubs.py -> stubs983.obj). Valor =
// bytes do 983 no endereço indicado (out/F4/stubs.tsv). Unidade própria: as unidades que citam só têm o extern e não mudam.
// No 983 os quatro retângulos zero são um só (00f14668); aqui cada nome tem o seu, todos zero e const.
#include <string>
#include "engine/wtypes.h"
#include "engine/wmath.h"
#include "refresh/rftypes.h"
#include "lobbyplayer.h"
#include "uiframesetmanager.h"
#include "refreshrenderer.h"

// tamanho da tela da RF (nomes trocados no fonte, ver B129: o "H" é a largura)
extern const float g_fRFScreenW983 = 600.0f; // 00d05628
extern const float g_fRFScreenH983 = 800.0f; // 00d0562c

// modo de criação das janelas RF: 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
int g_rfCreateMode983 = 0; // 00e73e48
int g_rfModalCreateMode983 = 1; // 00e03260

// nomes de recurso do lobby
extern const char* g_lobbyAppearFx983 = "Talk_Room.seq"; // 00e0c02c -> 00d0a394
extern const char* g_lobbyBgLua983 = "lobby_bg.lua"; // 00e0cff0 -> 00d0b450
extern const char* g_lobbyAvatarTable983 = "lobby_avatar_table"; // 00e0cff4 -> 00d0b43c
extern const char* g_lobbyPlayerFx983 = "avatar_d.seq"; // 00e0c294 -> 00d0a5c8
extern const char* g_lobbyPlayerSnd983 = "char_change"; // 00e0c298 -> 00d0a5bc

// retângulos e vetores constantes (construtor vazio + armazenamento estático = zero)
extern const sLobbyRect983 g_lobbyRectZero983;
const sLobbyRect983 g_lobbyRectZero983; // 00f14668
extern const WRect g_rcZero983;
const WRect g_rcZero983; // 00f14668
extern const WRect g_wrectZero983;
const WRect g_wrectZero983; // 00f14668
const WRect RefreshRenderer::g_rectZero983; // 00f14668
extern const RFRect g_rcRFZero983;
const RFRect g_rcRFZero983; // 00f1de34
extern const RFVector2 g_vRFZero983;
const RFVector2 g_vRFZero983; // 00f1de44 (o construtor zera)
extern const WVector g_vZero983_f041c4(0.0f, 0.0f, 0.0f); // 00f041c4 (WVector() é "= default" no cl novo: const sem valor não compila)
extern const WVector g_vOne983_e42520(1.0f, 1.0f, 1.0f); // 00e42520
extern const WVector g_vUp983_e42538(0.0f, 1.0f, 0.0f); // 00e42538

// UV inteira (0, 0, 1, 1)
extern const _WRECT g_rectUnit983_e4296c = { 0.0f, 0.0f, 1.0f, 1.0f }; // 00e4296c
_WRECT g_fullUV983 = { 0.0f, 0.0f, 1.0f, 1.0f }; // (sem VA no fonte; igual ao g_rectUnit983_e4296c)

// escala 1 × 1 do CUiFrameSetManager
extern const CUiFrameSetManager::Size g_sclOne983(1.0f, 1.0f); // 00e424f8

// texto vazio da RF (GetUserString sem a chave)
extern const std::wstring g_rfEmptyString983;
const std::wstring g_rfEmptyString983; // 00e46f04
