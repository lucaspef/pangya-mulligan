// C_ASSERT de layout que depende do STL do VC10 (std::vector/list/map/string com os tamanhos do VC10: o offset do 983 só
// fecha com ele). Toolset novo (projeto do VS, D734 P2): o layout das classes com STL muda e o assert não se aplica;
// os asserts de pacote (POD) continuam no C_ASSERT normal.
#pragma once
#if defined(_MSC_VER) && _MSC_VER == 1600
#define C_ASSERT_VC10(e) C_ASSERT(e)
#else
#define C_ASSERT_VC10(e) typedef char __C_ASSERT_VC10__
#endif

// D860: offset cru (do 983/VC10) de membro que fica logo depois de BaseObject (std::string m_leakHint 0x1c no VC10, 0x18 no
// cl novo): sem outro STL no meio a classe inteira anda 4 para tras (medido: WView SCREEN_XS 0x19c -> 0x198, CInputManager
// m_mousePoint 0x54 -> 0x50; C_ASSERT em wview.cpp/inputmanager.cpp).
#if defined(_MSC_VER) && _MSC_VER > 1600
#define VC10_OFS_BO(o) ((o) - 4)
#else
#define VC10_OFS_BO(o) (o)
#endif

// D872: CFxSequence/CFxSpray (fxsequence.h/fxspray.h) tem std::map/std::list antes destes campos: no cl 14.44 andam
// -8 (sequencia) e -4 (spray). Visoes cruas do 983 (+0x1108/+0x110c/+0x1138/+0x1160) usam estes; C_ASSERT em fxsequence.cpp.
#if defined(_MSC_VER) && _MSC_VER > 1600
#define VC10_FXSEQ_ACTIVE 0x1100
#define VC10_FXSEQ_POS 0x1104
#define VC10_FXSPRAY_ACTIVE 0x1108
#define VC10_FXSPRAY_POS 0x1134
#define VC10_FXSPRAY_BGROT 0x115c
#else
#define VC10_FXSEQ_ACTIVE 0x1108
#define VC10_FXSEQ_POS 0x110c
#define VC10_FXSPRAY_ACTIVE 0x110c
#define VC10_FXSPRAY_POS 0x1138
#define VC10_FXSPRAY_BGROT 0x1160
#endif

// D873: crus do bongdarishopdlg (Memorial). CCameraWork tem std::vector antes de m_matSelect983/m_i2ac (cl 14.44 0x118/0x2d0);
// CFxBox m_spray/m_sequence: _Mysize do std::map em +8 no VC10, +4 no cl 14.44 (e os mapas antes encolhem). C_ASSERT em
// camerawork.cpp e fxbox.cpp.
#if defined(_MSC_VER) && _MSC_VER > 1600
#define VC10_CAMWORK_MATSELECT 0x118
#define VC10_CAMWORK_I2AC 0x2d0
#define VC10_MAP_MYSIZE 4
#define VC10_FXBOX_SPRAYN 0x148
#define VC10_FXBOX_SEQN 0x150
#else
#define VC10_CAMWORK_MATSELECT 0x120
#define VC10_CAMWORK_I2AC 0x2e0
#define VC10_MAP_MYSIZE 8
#define VC10_FXBOX_SPRAYN 0x164
#define VC10_FXBOX_SEQN 0x174
#endif

// B1471: FrWnd (frwnd.h) tem std::list m_childList e 2 std::string antes destes campos: no cl 14.44 m_rect 0x34->0x30,
// m_dwStyle 0x44->0x40, m_wndAlpha 0x80->0x74, m_accHoverTime 0xc4->0xb8, sizeof 0xd0->0xc4 (medido, out/B1471/probe.txt).
// Membros protegidos: crus do 983 sobre FrWnd* usam estes; derivadas de FrWnd andam VC10_FRWND_DELTA (se nada mais de STL).
#if defined(_MSC_VER) && _MSC_VER > 1600
#define VC10_FRWND_STYLE 0x40
#define VC10_FRWND_ALPHA 0x74
#define VC10_FRWND_ACCHOVER 0xb8
#define VC10_FRWND_DELTA 0xc
#else
#define VC10_FRWND_STYLE 0x44
#define VC10_FRWND_ALPHA 0x80
#define VC10_FRWND_ACCHOVER 0xc4
#define VC10_FRWND_DELTA 0
#endif
