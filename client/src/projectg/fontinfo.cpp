#include "minatl.h"
#include "vc10_layout.h"
#include "fontinfo.h"

#include "shareddoc.h"
#include "golfdocofs983.h" // B1459
#include "jp/docofs983.h" // B1473: crus do CSharedDoc (m_myInfo/m_userInfo) ancorados no membro (VS 2022)
#include "wresrcmng.h"
#include "soundmanager.h"
#include "golfball.h"
#include "club.h"
#include "clientsetting.h"
#include "inputmanager.h"
#include "wind.h"
#include "../shared/localize.h"
#include "binstr.h"
#include "simpleoverlay.h"
#include "shufflebonus.h"
#include "logininfo.h"
#include "commonutil.h"
#include "s5/classicserver/classicserver.h"
#include "../shared/s5/sharedutilities.h"

// HACK: s5/utilities.h would add its static Base64 tables (0x440 bytes of .data) that the
// original object does not have; only this declaration is needed here.
namespace S5
{
	namespace UTIL
	{
		unsigned long GetServerProperty();
	}
}

inline WRect WRect::operator+(WPoint& point) const
{
	return WRect(x + point.x, y + point.y, w, h);
}

#include "quadtree.h"
#include "mousecursor.h"
#include "cardsystem.h"
#include "jp/uiframesetmanager.h"
#include "jp/shareddocjp.h" // 983: GetHoleValue983 (00633560)
#include "contentsdoc.h"
#include "jp/holeinonegamedoc.h" // B1484: crus do conteiner 0xaa (m_24/m_pMine/m_7c, depois de std::map) pelo membro
#define TIKIDOC983(p) ((_gateway::_golfgame::CHoleInOneGameDoc*)(p))
#include "grounditemman.h" // D881: PlayerStatsOfs983 (GroundItemTable)



// HACK: stands in for an unknown erased inline use of __rtti_obj
static __declspec(thread) int __rtti_tls_pad;

struct PangOverlayInfo
{
	unsigned int id;
	unsigned int y;
	unsigned int x;
};

__int64 g_pangValueTable[256] = {
	50,
	50,
	30,
	12,
	3,
	0,
	100,
	100,
	60,
	30,
	11,
	0,
	110,
	110,
	65,
	32,
	12,
	0,
	180,
	180,
	98,
	59,
	20,
	0,
	150,
	150,
	85,
	43,
	16,
	0,
	55,
	55,
	36,
	17,
	5,
	0,
	100,
	100,
	60,
	30,
	11,
	0,
	150,
	150,
	85,
	43,
	16,
	0,
	80,
	80,
	52,
	26,
	9,
	0,
	110,
	110,
	65,
	32,
	12,
	0,
	80,
	80,
	52,
	26,
	9,
	0,
	50,
	50,
	30,
	12,
	3,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	180,
	180,
	98,
	59,
	20,
	0,
	50,
	50,
	30,
	12,
	3,
	0,
	50,
	50,
	30,
	12,
	3,
	0,
	80,
	80,
	52,
	26,
	9,
	0,
	800,
	800,
	520,
	260,
	90,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	80,
	80,
	52,
	26,
	9,
	0,
	5208208757389214273,
	5786930140093827657,
	6365651522798441041,
	7378413942531512921,
	7957135325236127847,
	8535856707940741231,
	3689065129052829815,
	3398873257388422452,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	270582939647,
	-1,
	274877906943,
	227633266740,
	236223201334,
	244813135928,
	253403070522,
	261993005116,
	-1,
	-1,
	-1,
	4294967295,
	8589934593,
	17179869187,
	25769803781,
	34359738375,
	42949672969,
	51539607563,
	60129542157,
	68719476751,
	77309411345,
	85899345939,
	94489280533,
	103079215127,
	-4294967271i64,
	-1,
	-1,
	115964116991,
	120259084315,
	128849018909,
	137438953503,
	146028888097,
	154618822691,
	163208757285,
	171798691879,
	180388626473,
	188978561067,
	197568495661,
	206158430255,
	214748364849,
	-4294967245i64,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
};

const char* g_pangOverlayFilenames[2] = { "[pang_bonus.jpg",
	"[pang_bonus2.jpg" };

PangOverlayInfo g_pangOverlayInfo[25] = {
	{ 0u, 0,  0 },
	{ 0u, 1,  0 },
	{ 0u, 2,  0 },
	{ 0u, 3,  0 },
	{ 0u, 12, 1 }, // 983 e0af34: case 4 novo
	{ 0u, 4,  0 },
	{ 0u, 5,  0 },
	{ 0u, 6,  0 },
	{ 0u, 7,  0 },
	{ 0u, 8,  0 },
	{ 0u, 0,  1 },
	{ 0u, 1,  1 },
	{ 0u, 2,  1 },
	{ 0u, 3,  1 },
	{ 0u, 4,  1 },
	{ 0u, 5,  1 },
	{ 0u, 6,  1 },
	{ 0u, 7,  1 },
	{ 0u, 8,  1 },
	{ 0u, 9,  1 },
	{ 0u, 10, 1 },
	{ 0u, 11, 1 },
	{ 0u, 12, 1 },
	{ 1u, 3,  1 },
	{ 1u, 3,  0 },
};

PangOverlayInfo g_specialShotPangOverlayInfo[3][4] = {
	{
     { 0u, 0, 0 },
     { 1u, 1, 0 },
     { 1u, 1, 1 },
     { 1u, 1, 1 },
	 },
	{
     { 0u, 0, 0 },
     { 1u, 2, 0 },
     { 1u, 2, 1 },
     { 1u, 2, 1 },
	 },
	{
     { 0u, 0, 0 },
     { 1u, 0, 0 },
     { 1u, 0, 1 },
     { 1u, 0, 1 },
	 },
};

PangOverlayInfo g_holeInPangOverlayInfo[6] = {
	{ 0u, 9,  0 },
	{ 0u, 10, 0 },
	{ 0u, 11, 0 },
	{ 0u, 12, 0 },
	{ 0u, 13, 0 },
	{ 0u, 14, 0 },
};

PangOverlayInfo g_totalPangOverlayInfo[4] = {
	{ 0u, 15, 1 },
	{ 0u, 14, 1 },
	{ 1u, 4,  1 },
	{ 1u, 4,  0 },
};

static const int s_clearBonusTable[20] = {
	20,
	50,
	55,
	80,
	65,
	24,
	50,
	70,
	40,
	55,
	40,
	20,
	0,
	80,
	20,
	20,
	40,
	40,
	0,
	40,
};
// HACK: the original calls WVector::Normalize out of line and never compiles its body here
// (the body reads g_EPSILON, which would get storage at the end of this unit's constants).
#define NormalizeVector(v) ((v).Normalize())

static const float s_minDistance = 1.0f;
static const float s_fifteen = 15.0f;
static const float s_half = 0.5f;
static const float s_accuracyScale = 0.35f;
static const float s_degToRad = 0.017453292f;
static const float s_epsilon = 0.00001f;
static const float s_barEnd = 500.0f;
static const float s_barTextY = 8.0f;
static const float s_barStart = 140.0f;

// The original reads these from the unit's .rdata (not from __real@ literals);
// referencing them from an inline gives them storage, as in wmath.h.
namespace
{
	inline void InstantiateFontInfoConstants()
	{
		(void)s_clearBonusTable;
		(void)s_minDistance;
		(void)s_fifteen;
		(void)s_half;
		(void)s_accuracyScale;
		(void)s_degToRad;
		(void)s_epsilon;
		(void)s_barEnd;
		(void)s_barTextY;
		(void)s_barStart;
	}
}

// HACK: the original has the delay before the bar width in .bss (definition order) but
// initializes the bar width first, m_fontList next and the delay last. Each initializer
// therefore fills the other variable (the self-assignment generates no code): the
// initializers run in the original order and the storage keeps the original layout.
namespace
{
	extern float s_barWidth;
}
static float s_floatDelay = (s_barWidth = s_barEnd - s_barStart, s_floatDelay);
WList<CFontInfo::sFontInfo*> CFontInfo::m_fontList(4, 4);
namespace
{
	float s_barWidth = (s_floatDelay = s_half / s_fifteen, s_barWidth);
}
bool CFontInfo::m_loaded = false;

bool IsMyTurn(bool bCheck);
unsigned char MyIndex();
int __fastcall WisZero(const float& value, float epsilon);
int __fastcall float2int(float value);
// HACK: GetHoleCupPos is an inline of shareddoc.inl that the original calls out of line here: the calls go
// through an alias (the COMDAT is golfdoc's)
WVector GetPinPos();
#pragma comment(linker, \
	"/alternatename:?GetPinPos@@YI?AVWVector@@XZ=?GetHoleCupPos@@YI?AVWVector@@XZ")
const WVector& __fastcall GetHoleCupPos(WVector& out);
#pragma comment(linker, \
	"/alternatename:?GetHoleCupPos@@YIABVWVector@@AAV1@@Z=?GetHoleCupPos@@YI?AVWVector@@XZ")
// JP 983: o GetHoleCupPos por valor do 645 (alvo dos dois apelidos acima) nao existe mais: o do 983 devolve referencia
// (005298c0, shareddoc.inl). Sem corpo, os dois nomes viravam stub FATAL no exe (fontinfo e golfplayer, partida).
WVector GetPinPos()
{
	return GetHoleCupPos();
}
const WVector& __fastcall GetHoleCupPos(WVector& out)
{
	out = GetHoleCupPos();
	return out;
}
extern WMatrix g_camera;

// HACK: helpers local to this file. The original has no out-of-line copy of them, and
// dllimport + __forceinline is the only inline form the compiler never emits as a COMDAT.
#define LOCAL_INLINE __declspec(dllimport) __forceinline

LOCAL_INLINE unsigned char DocByte(int offset);
namespace
{
	LOCAL_INLINE unsigned char GameType();
}

namespace
{
	LOCAL_INLINE char* GroundItemTable()
	{
		CTask* task = AfxGetTask();
		IActor* actor = task->GetActor("GroundItemMan");
		return actor != NULL ? *(char**)((char*)actor + GroundItemMan::PlayerStatsOfs983()) : NULL; // 983 0052d0dd: m_pPlayerStats (+0xa34 VC10; D881: cl 14.44 0xa2c)
	}
}

LOCAL_INLINE WVector& MousePoint()
{
	return *(WVector*)((char*)g_input + VC10_OFS_BO(0x54));
}

namespace
{
	LOCAL_INLINE unsigned long PlayerCard(unsigned char index)
	{
		return CSharedDoc::Instance()->m_userInfo[index].userEquip.tidBall; // 983: +0x3b42
	}
}

LOCAL_INLINE float FAbs(float value)
{
	return value > 0.0f ? value : -value;
}

inline bool __fastcall IsGameType(unsigned char type)
{
	return Doc()->m_golfGame.gameType == type ? true : false;
}

CFontInfo::CFontInfo()
{
	Buffer() = NULL;
	if (m_loaded == false)
	{
		m_loaded = true;
		AddFont("[font_hole.jpg", 16, 16, 1, 128, 32, "1234567890ym%-.,+>");
		AddFont("[font_bar.jpg", 16, 16, 1, 128, 32, "1234567890ym%-. ");
		AddFont("[font_wind.jpg", 16, 16, 1, 128, 64, "1234567890ym%-.?/:");
		AddFont("[font_club.jpg", 32, 32, 1, 128, 128, "0123456789abcdef");
		AddFont("[bigfont.jpg", 32, 32, 1, 256, 128, "1234567890ym%-./x?() ");
		// 983 (006e27c0): fonte nova da potencia maxima do terreno (CGroundMaxPowerInfo)
		AddFont("[font_condition.jpg", 16, 16, 1, 128, 32, "1234567890%");
		if (IsGameType(7))
		{
			AddFont("[font_hole_skins_red.jpg", 16, 16, 1, 128, 32,
				"1234567890-,");
			AddFont("[font_hole_skins_blue.jpg", 16, 16, 1, 128, 32,
				"1234567890+,");
			AddFont("[font_hole_skins_grey.jpg", 16, 16, 1, 128, 32,
				"1234567890>,");
		}
		if (IsGameType(6))
		{
			AddFont("[font_hole_skins_red.jpg", 16, 16, 1, 128, 32,
				"1234567890-,");
			AddFont("[font_hole_skins_blue.jpg", 16, 16, 1, 128, 32,
				"1234567890+,");
		}
		// 983 (006e27c0): tipo de jogo 0xb (hole in one)
		if (IsGameType(0xb))
		{
			AddFont("font_holein_font.png", 0x2e, 0x36, 1, 0x1fa, 0x36,
				"0123456789+");
		}
	}
}

CFontInfo::~CFontInfo()
{
	Clear();
}

void CFontInfo::AddFont(const char* name, int a, int b, int c, int d, int e,
	char* chars)
{
	// 983 (00e16334): 10 nomes; os 2 ultimos sao novos (A175: sem eles a fonte de font_condition nao entrava e o
	// CGroundMaxPowerInfo derrubava a carga da partida no robo a175-5)
	static const char* s_fontNames[10] = {
		"[font_hole.jpg",
		"[font_bar.jpg",
		"[font_wind.jpg",
		"[font_club.jpg",
		"[bigfont.jpg",
		"[font_hole_skins_red.jpg",
		"[font_hole_skins_blue.jpg",
		"[font_hole_skins_grey.jpg",
		"font_holein_font.png",
		"[font_condition.jpg",
	};
	sFontInfo* info = (sFontInfo*)operator new(sizeof(sFontInfo));
	info->filename = NULL;
	info->numPages = 0;
	info->fontw = 0;
	info->fonth = 0;
	info->texw = 0;
	info->texh = 0;
	info->flag = 0;
	info->pCharSet = NULL;
	for (int i = 0; i < 10; ++i)
	{
		if (strcmp(s_fontNames[i], name) == 0)
		{
			info->filename = (char**)&s_fontNames[i];
			break;
		}
	}
	if (info->filename == NULL)
	{
		delete info;
		return;
	}
	info->fontw = a;
	info->fonth = b;
	info->numPages = c;
	info->texw = d;
	info->texh = e;
	info->pCharSet = chars;
	info->id = -1;
	strcpy(info->keycode, name);
	m_fontList.AddItem(info, info->keycode, false);
}

void CFontInfo::Init(char* name, sFont* font)
{
	if (font == NULL)
	{
		font = &m_font;
	}
	font->pInfo = m_fontList.Find(name);
	font->pFont = g_resrcmng->GetTitleFont();
	font->pFont->Create(font->pInfo);
	m_color = 0xffffffff;
	m_align = 0;
	m_valign = 0;
}

void CFontInfo::Clear()
{
	// 983 (006e24b0): devolve antes as fontes "_xN" e esvazia o mapa
	for (std::map<float, sScaledFont>::iterator it = m_font.scaled.begin();
		it != m_font.scaled.end(); ++it)
	{
		if (g_resrcmng != NULL && it->second.pFont != NULL)
		{
			g_resrcmng->Release(it->second.pFont);
			it->second.pFont = NULL;
		}
	}
	m_font.scaled.clear();
	if (g_resrcmng != NULL && m_font.pFont != NULL)
	{
		g_resrcmng->Release(m_font.pFont);
		m_font.pFont = NULL;
	}
	if (Buffer() != NULL)
	{
		delete[] Buffer();
		Buffer() = NULL;
	}
	for (sFontInfo* info = m_fontList.Start(); info != NULL;
		info = m_fontList.Next())
	{
		delete info;
	}
	m_fontList.Reset();
	m_loaded = false;
}

float CFontInfo::GetTextWidth(const char* text, float spacing, float scale)
{
	// HACK: the first read of a const float makes the compiler use its storage for
	// every equal literal of the unit (the original has no __real@3f800000).
	if (s_minDistance < 0.0f)
	{
		return 0.0f;
	}
	int len = strlen(text);
	float w = len * spacing;
	return w + w + (float)(m_font.pInfo->fontw * len) * scale * 0.5f;
}

float CFontInfo::GetExactTextWidth(const char* text, float spacing, float scale)
{
	if (text != NULL)
	{
		if (strlen(text) != 0)
		{
			float oldScale = m_font.pFont->GetScale();
			m_font.pFont->SetScale(scale);
			const char* chars = m_font.pInfo->pCharSet;
			const char* p = text;
			char c = *p;
			float width = 0.0f;
			if (c != '\0')
			{
				do
				{
					++p;
					width = m_font.pFont->GetCharWidth(g_view,
								strchr(chars, c) - chars) +
						spacing * scale + width;
					c = *p;
				} while (c != '\0');
			}
			m_font.pFont->SetScale(oldScale);
			return width - spacing;
		}
	}
	return 0.0f;
}

float CFontInfo::Print(float x, float y, const char* text, float spacing,
	int type, float scale, sFont* font)
{
	if (text == NULL || strlen(text) == 0)
	{
		return 0.0f;
	}
	if (font == NULL)
	{
		font = &m_font;
	}
	const char* chars = font->pInfo->pCharSet;
	float oldScale = font->pFont->GetScale();
	font->pFont->SetScale(scale);
	float offset = 0.0f;
	if (m_align == 1)
	{
		for (const char* p = text; *p != '\0'; ++p)
		{
			offset +=
				font->pFont->GetCharWidth(g_view, strchr(chars, *p) - chars) +
				spacing * scale;
		}
		offset = (offset - spacing) * 0.5f;
	}
	else if (m_align == 2)
	{
		for (const char* p = text; *p != '\0'; ++p)
		{
			offset +=
				font->pFont->GetCharWidth(g_view, strchr(chars, *p) - chars) +
				spacing * scale;
		}
		offset = offset - spacing;
	}
	float cur = x - offset;
	for (const char* p = text; *p != '\0'; ++p)
	{
		cur = font->pFont->PutChar(g_view, cur, y, strchr(chars, *p) - chars,
				  type, m_color) +
			spacing + cur;
	}
	font->pFont->SetScale(oldScale);
	return offset;
}

// 983 006e2600: registra a imagem da fonte para a escala `scale` (so a 1a vez; 1.0 e a propria fonte)
void CFontInfo::sFont::AddScaled(float scale, const char* name)
{
	if (scale >= 0.0f && scale != 1.0f && scaled.find(scale) == scaled.end())
	{
		sScaledFont info;
		info.scale = scale;
		info.name = name;
		info.pFont = NULL;
		scaled.insert(std::make_pair(scale, info));
	}
}

// 983 006e1830: fonte para a escala da tela: a 1a variante com escala >= `scale` (ou a maior); cria a WTitleFont
// com as medidas do sFontInfo multiplicadas pela escala da variante
void CFontInfo::sFont::FindScaled(float scale, WTitleFont** font, float* fontScale)
{
	if (fabs(scale - 1.0f) < 1e-5f || scaled.size() == 0)
	{
		*fontScale = 1.0f;
		*font = pFont;
		return;
	}
	std::map<float, sScaledFont>::iterator it = scaled.begin();
	for (; it != scaled.end(); ++it)
	{
		if (fabs(it->first - scale) < 1e-5f || scale < it->first)
		{
			break;
		}
	}
	if (it == scaled.end())
	{
		--it;
	}
	sScaledFont& v = it->second;
	if (v.pFont == NULL)
	{
		char* filename = (char*)v.name.c_str();
		WTITLEFONT desc;
		desc.filename = &filename;
		desc.numPages = 1;
		desc.fontw = (int)(pInfo->fontw * v.scale);
		desc.fonth = (int)(pInfo->fonth * v.scale);
		desc.texw = (int)(pInfo->texw * v.scale);
		desc.texh = (int)(pInfo->texh * v.scale);
		desc.flag = 0;
		desc.pCharSet = pInfo->pCharSet;
		v.pFont = g_resrcmng->GetTitleFont();
		if (!v.pFont->Create(&desc))
		{
			g_resrcmng->Release(v.pFont);
			v.pFont = NULL;
		}
	}
	*font = v.pFont;
	*fontScale = v.scale;
}

// escala da tela pelo Dim do CUiFrameSetManager (tela / 640x480), como no 983
static float ScreenScale983(int sclType)
{
	CUiFrameSetManager::Dim dim;
	dim.Set(1.0f, sclType);
	return dim.Calc(CUiFrameSetManager::Size((float)(g_view->GetWidth() / 640.0),
		(float)(g_view->GetHeight() / 480.0)));
}

// posicao (arredondada para baixo) de um ponto com nome do CUiFrameSetManager na tela atual; embutido em cada Display do 983
static bool UiPoint983(const char* name, float* x, float* y, bool roundDown = true)
{
	CUiFrameSetManager::NamedPoint* pt = CUiFrameSetManager::Instance()->FindNamedPoint(name);
	if (pt == NULL)
	{
		return false;
	}
	WPoint pos;
	pt->Calc(CUiFrameSetManager::Size((float)(g_view->GetWidth() / 640.0),
		(float)(g_view->GetHeight() / 480.0)), pos);
	*x = roundDown ? (float)floor(pos.x) : pos.x;
	*y = roundDown ? (float)floor(pos.y) : pos.y;
	return true;
}

// 983 006e1de0: largura do texto na escala da tela; charX/charW (opcionais) recebem a posicao e a largura de cada letra
float CFontInfo::GetScaledTextWidth(int sclType, const char* text, float spacing, float scale,
	float* charX, float* charW, int type)
{
	if (text == NULL || strlen(text) == 0)
	{
		return 0.0f;
	}
	float screen = ScreenScale983(sclType);
	WTitleFont* font = NULL;
	float fontScale = 1.0f;
	m_font.FindScaled(screen, &font, &fontScale);
	if (font == NULL)
	{
		for (int i = 0; text[i] != '\0'; ++i)
		{
			if (charX != NULL)
				charX[i] = 0.0f;
			if (charW != NULL)
				charW[i] = 0.0f;
		}
		return 0.0f;
	}
	float oldScale = font->GetScale();
	font->SetScale(scale * screen / fontScale);
	const char* chars = m_font.pInfo->pCharSet;
	float step = scale * spacing * screen;
	float width = 0.0f;
	for (int i = 0; text[i] != '\0'; ++i)
	{
		if (charX != NULL)
			charX[i] = width;
		float w = font->GetCharWidth(g_view, strchr(chars, text[i]) - chars, type != 0);
		if (charW != NULL)
			charW[i] = w;
		width = w + width + step;
	}
	font->SetScale(oldScale);
	return width - step;
}

// 983 006e2190: Print na escala da tela (fonte "_xN" escolhida pelo FindScaled), alinhado por m_align e m_valign
float CFontInfo::PrintScaled(int sclType, float x, float y, const char* text, float spacing,
	float scale, int type)
{
	if (text == NULL || strlen(text) == 0)
	{
		return 0.0f;
	}
	float width = GetScaledTextWidth(sclType, text, spacing, scale, NULL, NULL, 0);
	if (width == 0.0f)
	{
		return 0.0f;
	}
	if (m_align == 1)
	{
		x -= width * 0.5f;
	}
	else if (m_align == 2)
	{
		x -= width;
	}
	float screen = ScreenScale983(sclType);
	WTitleFont* font = NULL;
	float fontScale = 1.0f;
	m_font.FindScaled(screen, &font, &fontScale);
	if (font == NULL)
	{
		return 0.0f;
	}
	float fsc = scale * screen / fontScale;
	if (m_valign == 1)
	{
		y -= font->GetFontInfo()->fonth * fsc * 0.5f;
	}
	else if (m_valign == 2)
	{
		y -= font->GetFontInfo()->fonth * fsc;
	}
	float oldScale = font->GetScale();
	font->SetScale(fsc);
	const char* chars = m_font.pInfo->pCharSet;
	float step = scale * spacing * screen;
	for (const char* p = text; *p != '\0'; ++p)
	{
		x = font->PutChar(g_view, x, y, strchr(chars, *p) - chars, type, m_color) + step + x;
	}
	font->SetScale(oldScale);
	return width;
}

CDistToHoleInfo::CDistToHoleInfo()
{
	m_x = g_view->GetWidth() * 0.5f + 20.0f;
	m_y = g_view->GetHeight() * 0.25f + 25.0f;
	m_scale = 0.77f;
	m_targetY = 63.0f;
	m_targetScale = 0.5f;
	m_state = 5;
	m_timer2 = 0.0f;
	Init("[bigfont.jpg");
	Buffer() = new char[32];
}

void CDistToHoleInfo::Process(float dt)
{
	if (0)
		((CHoleCupInfo*)0)->CHoleCupInfo::Reset();
	if (GOLFDOC()->m_gameMode & 2)
	{
		return;
	}
	if ((GOLFDOC()->m_gameMode & 0x8210) || !IsMyTurn(true) ||
		(GOLFDOC()->m_tutorialMode < 15 && GOLFDOC()->m_tutorialMode != 8) ||
		(GOLFDOC()->m_tutorialMode == 8 &&
			PLAYER(GOLFDOC()->m_currentPlayer)->stroke[0] == (unsigned char)1))
	{
		m_state = 0;
	}
	if (Doc()->m_golfGame.gameType == 10)
	{
		m_state = 0;
	}
	switch (m_state)
	{
	case 1:
		m_timer1 = dt + m_timer1;
		if (m_timer1 > 0.5f)
		{
			m_timer1 = 0.5f;
			m_state = 0;
		}
		m_timer2 = m_timer1 + m_timer1;
		if (g_input->GetList() && GOLFDOC()->m_gameMode != 4)
		{
			m_state = 0;
			return;
		}
		break;
	case 2:
		m_alpha = dt * 510.0f + m_alpha;
		if (m_alpha > 255.0f)
		{
			m_timer1 = 0.0f;
			m_state = 4;
			m_alpha = 255.0f;
			return;
		}
		break;
	case 4:
		m_timer1 = dt + m_timer1;
		if (m_timer1 > 2.0f)
		{
			m_timer1 = 0.0f;
			m_state = 1;
		}
		if (g_input->GetList() && GOLFDOC()->m_gameMode != 4)
		{
			// 983 (00530dfb): tecla apaga tambem o texto do "Letter"; nao ha teste de pino atras da camera (645)
			m_state = 3;
			AfxGetTask()->GetActor("Letter") << MsgObject(NULL, M983(0x18f), 0, 0, 0, 0, 0);
			return;
		}
		break;
	case 3:
		m_alpha = m_alpha - dt * 510.0f;
		if (!(m_alpha < 0.0f))
		{
			return;
		}
		m_state = 0;
	case 0:
		m_timer2 = 1.0f;
		m_alpha = 255.0f;
		break;
	}
}

void CDistToHoleInfo::Display()
{
	if (m_state != 5 &&
		(!(Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].info.dwIdentity & 1) ||
			m_state == 0 || GOLFDOC()->m_gameMode == 4))
	{
		WVector pin = GOLFDOC()->GetHoleData().pin;
		WVector screen = g_view->Projection(pin);
		if (m_timer2 <= 0.0f || (!(screen.z <= 0.0f) && !(s_minDistance <= screen.z)))
		{
			float inv = s_minDistance - m_timer2;
			float x = (screen.x + m_offset) * m_timer2 + inv * m_x;
			m_curX = x;
			float y = inv * m_y + m_targetY * m_timer2;
			m_curY = y;
			float scale = inv * m_scale + m_targetScale * m_timer2;
			m_curScale = scale;
			SetColor((int)m_alpha << 24 | 0xff0000);
			float py;
			if (GOLFDOC()->m_gameMode == 0x200)
			{
				py = screen.y;
			}
			else
			{
				py = y;
			}
			Print(x, py, Buffer(), scale * 6.0f, 0, scale, NULL);
		}
	}
}

void CDistToHoleInfo::Fix()
{
	m_curX = m_targetX;
	m_alpha = 255.0f;
	m_state = 0;
	m_curY = m_targetY;
	m_curScale = m_targetScale;
}

void CDistToHoleInfo::Reset()
{
	sPlayerData* player = PLAYER(GOLFDOC()->m_currentPlayer);
	WVector pin = GOLFDOC()->GetHoleData().pin;
	WVector dist(pin.x - player->pos.x, 0.0f, pin.z - player->pos.z);
	if (IsLocalContent(S4_REPLAY_SYSTEM) && Doc()->m_playingMode == 2)
	{
		WVector pin2 = GOLFDOC()->GetHoleData().pin;
		dist = pin2 - GolfBall().m_pos;
		dist.y = 0.0f;
	}
	m_distance = dist.Magnitude() * 0.3125f;
	if (m_distance < 100.0f)
	{
		sprintf(Buffer(), "%.1fy", m_distance);
		if (m_distance > 10.0f)
		{
			m_offset = -20.0f;
		}
		else
		{
			m_offset = -14.0f;
		}
	}
	else
	{
		sprintf(Buffer(), "%.0fy", m_distance);
		m_offset = -20.0f;
	}
	m_timer1 = 0.0f;
	m_timer2 = 0.0f;
	if (IsMyTurn(true) &&
		!(IsLocalContent(S4_REPLAY_SYSTEM) && Doc()->m_playingMode == 2))
	{
		m_alpha = 0.0f;
		m_state = 2;
		m_curX = m_x;
		m_curY = m_y;
		m_curScale = m_scale;
		return;
	}
	m_state = 0;
	m_alpha = 255.0f;
}

void CDistToHoleInfo::ResetUiPosition()
{
	m_x = g_view->GetWidth() * 0.5f + 20.0f;
	m_y = g_view->GetHeight() * 0.25f + 25.0f;
}

CGuildInfo::CGuildInfo()
{
	Init("[font_hole_skins_red.jpg", &m_fontRed);
	Init("[font_hole_skins_blue.jpg", &m_fontBlue);
	Buffer() = new char[32];
	Buffer2() = new char[32];
	m_alpha = 0.0f;
	m_scale = 1.5f;
	m_sfx = "pang_down";
	memset(&m_score, 0, sizeof(m_score));
	memset(&m_prevScore, 0, sizeof(m_prevScore));
}

CGuildInfo::~CGuildInfo()
{
	if (Buffer2() != NULL)
	{
		delete Buffer2();
		Buffer2() = NULL;
	}
}

void CGuildInfo::Process(float dt)
{
	if (0)
		((CFontInfo*)0)
			->CFontInfo::Print(*(float*)0, *(float*)0, *(const char**)0,
				*(float*)0, *(int*)0, *(float*)0, *(sFont**)0);
	m_alpha += dt;
	if (m_score.a > m_prevScore.a)
	{
		m_prevScore.a++;
	}
	if (m_score.b > m_prevScore.b)
	{
		m_prevScore.b++;
	}
	float scale = 2.5f - m_alpha * 1.5f;
	if (scale < 1.5f)
	{
		m_scale = 1.5f;
		return;
	}
	if (scale > 2.5f)
	{
		m_scale = 2.5f;
		return;
	}
	m_scale = scale;
}

void CGuildInfo::SetScore(int a, int b)
{
	m_score.a = a;
	m_score.b = b;
	m_alpha = 0.0f;
	if (m_sfx != NULL)
	{
		if (!g_audio->IsPlaying(m_sfx))
		{
			g_audio->PlaySfx(m_sfx, NULL, 0, m_sfx, NULL, 0.5f, 200.0f);
		}
	}
}

void CGuildInfo::Display()
{
	m_align = 0;
	sprintf(Buffer(), "%d", m_prevScore.a);
	sprintf(Buffer2(), "%d", m_prevScore.b);
	Print(g_view->GetWidth() - 80.0f, 250.0f, Buffer(), 4.0f, 0, m_scale,
		&m_fontRed);
	Print(g_view->GetWidth() - 30.0f, 250.0f, Buffer2(), 4.0f, 0, m_scale,
		&m_fontBlue);
}

CHoleCupInfo::CHoleCupInfo()
{
	m_distInfo = NULL;
	Init("[font_bar.jpg");
	m_state = 1;
	m_overlay[0] = g_resrcmng->GetOverlay("[top_hole_l.jpg", 0);
	m_overlay[1] = g_resrcmng->GetOverlay("[top_hole_m.jpg", 0);
	m_overlay[2] = g_resrcmng->GetOverlay("[top_hole_h.jpg", 0);
	m_distInfo = new CDistToHoleInfo;
}

CHoleCupInfo::~CHoleCupInfo()
{
	for (int i = 0; i < 3; ++i)
	{
		if (m_overlay[i] != NULL)
		{
			g_resrcmng->Release(m_overlay[i]);
			m_overlay[i] = NULL;
		}
	}
	if (m_distInfo != NULL)
	{
		delete m_distInfo;
		m_distInfo = NULL;
	}
}

void CHoleCupInfo::Reset()
{
	if (0)
		((CHoleCupInfo*)0)->CHoleCupInfo::~CHoleCupInfo();
	WVector pin = GOLFDOC()->GetHoleData().pin;
	sPlayerData* player = PLAYER(GOLFDOC()->m_currentPlayer);
	CGolfBall& ball = GolfBall();
	float height = (pin.y - (player->pos.y - ball.GetRadius())) * 0.3125f;
	m_height = height * 0.9144f;
	if (m_distInfo != NULL)
	{
		m_distInfo->Reset();
	}
}

void CHoleCupInfo::Fix()
{
	if (m_distInfo != NULL)
	{
		m_distInfo->Fix();
	}
}

void CHoleCupInfo::GetScreenPos(float* x, float* y)
{
	*x = m_x;
	*y = m_y;
}

void CHoleCupInfo::Process(float dt)
{
	if (m_distInfo != NULL)
	{
		m_distInfo->Process(dt);
	}
}

void CHoleCupInfo::Display()
{
	char text[32];
	if (m_distInfo != NULL)
	{
		m_distInfo->Display();
	}
	if (FAbs(m_height) > 10.0f)
	{
		sprintf(text, "%.1fm", m_height);
	}
	else
	{
		sprintf(text, "%.2fm", m_height);
	}
	WVector screen = g_view->Projection(WVector(GOLFDOC()->GetHoleData().pin));
	if (!(screen.z <= 0.0f) && !(s_minDistance <= screen.z))
	{
		m_x = screen.x;
		m_y = GOLFDOC()->m_gameMode == 0x200 ? screen.y : 62.0f;
		CMouseCursor::Instance()->SetHoleCup(m_x, m_y);
		if (FAbs(m_height) < 0.1f)
		{
			m_state = 1;
			Print(m_x + 16.0f, m_y - 26.0f, text, 3.0f, 0, 1.0f, NULL);
		}
		else if (m_height > 0.0f)
		{
			m_state = 2;
			Print(m_x + 16.0f, m_y - 17.0f, text, 3.0f, 0, 1.0f, NULL);
		}
		else
		{
			m_state = 0;
			Print(m_x + 16.0f, m_y - 37.0f, text, 3.0f, 0, 1.0f, NULL);
		}
		if (m_overlay[m_state] != NULL)
		{
			m_overlay[m_state]->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
				WRect(m_x - 32.0f, m_y - 47.0f, 64.0f, 64.0f), 0, 0xffffffff, 0,
				0);
		}
	}
}


void CHoleCupInfo::ResetUiPosition()
{
	if (m_distInfo != NULL)
	{
		m_distInfo->ResetUiPosition();
	}
}

CClubRangeInfo::CClubRangeInfo()
{
	Init("[font_wind.jpg");
	for (int i = 2; i >= 0; i--)
	{
		(&m_pos.x)[i] = 0.0f;
	}
	m_align = 1;
	Buffer() = new char[32];
}

void CClubRangeInfo::Display()
{
	WVector screen = g_view->Projection(m_pos);
	if (!(screen.z <= 0.0f) && !(s_minDistance <= screen.z))
	{
		CGolfBall& ball = GolfBall();
		float dx = m_pos.x - ball.m_pos.x;
		sprintf(Buffer(), "%.0fy",
			sqrtf(
				dx * dx + (m_pos.z - ball.m_pos.z) * (m_pos.z - ball.m_pos.z)) *
				0.3125f);
		Print(screen.x, screen.y - 25.0f, Buffer(), 3.0f, 0, 1.0f, NULL);
	}
}


CFlyInfo::CFlyInfo()
{
	m_groundUI = NULL;
	m_textUI = NULL;
	m_overlay = NULL;
	Init("[bigfont.jpg");
	SetColor(0xffffffff);
	m_align = 0;
	Buffer() = new char[32];
	m_align = 2;
	m_groundUI = new CSimpleUI("[ground.jpg");
	m_groundUI->Add("Fairway", WRect(0, 0, 128, 25), false);
	m_groundUI->Add("Rough", WRect(0, 25, 128, 25), false);
	m_groundUI->Add("Green", WRect(0, 50, 128, 25), false);
	m_groundUI->Add("Bunker", WRect(0, 75, 128, 25), false);
	m_groundUI->Add("Tee", WRect(0, 100, 128, 25), false);
	m_groundUI->Add("Ice", WRect(0, 125, 128, 25), false);
	m_groundUI->Add("Sand", WRect(0, 150, 128, 25), false);
	m_groundUI->Add("Water", WRect(0, 175, 128, 25), false);
	m_groundUI->Add("Vector", WRect(0, 200, 128, 25), false);
	m_textUI = new CSimpleUI("[ground_text.jpg");
	m_textUI->Add("\xba\xf1\xb0\xc5\xb8\xae", WRect(0, 0, 64, 27), false);
	m_textUI->Add("\xb3\xb2\xc0\xba\xb0\xc5\xb8\xae", WRect(0, 27, 64, 27),
		false);
	if (IsGameType(10))
	{
		m_overlay = g_resrcmng->GetOverlay("approach_out.png", 0);
	}
	m_time = 0.0f;
}

CFlyInfo::~CFlyInfo()
{
	if (m_groundUI != NULL)
	{
		delete m_groundUI;
		m_groundUI = NULL;
	}
	if (m_textUI != NULL)
	{
		delete m_textUI;
		m_textUI = NULL;
	}
	if (g_resrcmng != NULL && m_overlay != NULL)
	{
		g_resrcmng->Release(m_overlay);
		m_overlay = NULL;
	}
}

void CFlyInfo::Process(float dt)
{
	if ((GOLFDOC()->m_gameMode & 0x80) == 0)
	{
		return;
	}
	CGolfBall& ball = GolfBall();
	if (ball.m_state != 0)
	{
		m_time = 0.0f;
		m_alphaGround = 0.0f;
		m_alphaTextA = 0.0f;
		m_alphaTextB = 0.0f;
		CalcDistance();
		return;
	}
	if (ball.m_bOB)
	{
		return;
	}
	if (ball.m_water != 0)
	{
		return;
	}
	if (ball.IsHoleIn())
	{
		return;
	}
	if (WisZero(m_time, 0.00001f))
	{
		CalcDistance();
		switch (GetPVS().GetGroundType(ball.m_pos, true, NULL))
		{
		case 1:
			strcpy(m_ground, "Fairway");
			break;
		case 2:
			strcpy(m_ground, "Green");
			break;
		case 4:
			strcpy(m_ground, "Rough");
			break;
		case 3:
			strcpy(m_ground, "Bunker");
			break;
		case 0:
			strcpy(m_ground, "Tee");
			break;
		case 7:
			strcpy(m_ground, "Ice");
			break;
		case 8:
			strcpy(m_ground, "Vector");
			break;
		case 11:
			strcpy(m_ground, "Sand");
			break;
		case 14:
			strcpy(m_ground, "Water");
			break;
		default:
			strcpy(m_ground, "Rough");
			break;
		}
	}
	dt += m_time;
	m_time = dt;
	m_alphaGround = Between(0.0f, dt * 510.0f + m_alphaGround, 255.0f);
	m_alphaTextA = Between(0.0f, (dt - 1.0f) * 510.0f + m_alphaTextA, 255.0f);
	m_alphaTextB = Between(0.0f, (dt - 1.5f) * 510.0f + m_alphaTextB, 255.0f);
}

LOCAL_INLINE float LengthXZ(WVector v)
{
	return sqrtf(v.x * v.x + v.z * v.z);
}

void CFlyInfo::CalcDistance()
{
	const CGolfBall::sExpected* start = GolfBall().PreData(0);
	m_carry = LengthXZ(GolfBall().m_pos - start->pos) * 0.3125f;
	WVector pin = GOLFDOC()->GetHoleData().pin;
	m_total = LengthXZ(GolfBall().m_pos - pin) * 0.3125f;
}

// A733: crus 0x1xxxx do CSharedDoc -> membros (o cl novo desloca o bloco; VC10 = 983)
C_ASSERT_VC10(offsetof(CSharedDoc, m_rivalList) == 0x1042c && offsetof(CSharedDoc, m_gameMode) == 0x1048c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_holeOrder) == 0x1051e && offsetof(CSharedDoc, m_approachStartTime) == 0x1057c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.pCourse) == 0x10590 && offsetof(CSharedDoc, m_golfGame.gameType) == 0x10594);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.holes) == 0x10699 && offsetof(CSharedDoc, m_golfGame.weather) == 0x1069a);
C_ASSERT_VC10(offsetof(CSharedDoc, m_teamResult) == 0x106c0 && offsetof(CSharedDoc, m_roomSetupStage) == 0x10d38);
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo.realGameType) == 0x109b8);

void CFlyInfo::Display()
{
	CGolfBall& ball = GolfBall();
	if (IsGameType(10))
	{
		if (Doc()->m_approachStartTime == 0)
		{
			return;
		}
		SetColor(0xffff3434);
		m_align = 0;
		unsigned long mode = GOLFDOC()->m_gameMode;
		if ((mode & 0x40) != 0)
		{
			sprintf(Buffer(), "-");
			Print(100.0f, 202.0f, Buffer(), 2.0f, 0, 0.64f, NULL);
		}
		else
		{
			unsigned int whole;
			unsigned int frac;
			if ((char)mode < 0)
			{
				if (GolfBall().m_state == 0)
				{
					goto L_best;
				}
				whole = float2int(m_total);
				frac = float2int((m_total - (float)(int)whole) * 10.0f);
			}
			else
			{
				if ((mode & 0x800) == 0)
				{
					goto L_second;
				}
L_best:
				unsigned int best =
					*(unsigned int*)((char*)*(int*)&Doc()->m_rivalList +
						MyIndex() * 0x1b8 + 0x190);
				if (best == 0xffffffff)
				{
					if (m_overlay != NULL)
					{
						WRect src(0, 0, 1.0f, 1.0f);
						WRect dst(72.0f, 193.0f, 64.0f, 32.0f);
						m_overlay->Render(g_view, src, dst, 0, 0xffff3434, 0.0f,
							0);
					}
					goto L_second;
				}
				whole = best / 10;
				frac = best % 10;
			}
			sprintf(Buffer(), "%d.%d", whole, frac);
			Print(80.0f, 202.0f, Buffer(), 2.0f, 0, 0.64f, NULL);
		}
L_second:
		if ((GOLFDOC()->m_gameMode & 0x8c0) == 0)
		{
			return;
		}
		int second = *(int*)((char*)*(int*)&Doc()->m_rivalList +
			MyIndex() * 0x1b8 + 0x194);
		if (second == -1)
		{
			sprintf(Buffer(), "-");
			Print(100.0f, 226.0f, Buffer(), 2.0f, 0, 0.64f, NULL);
			return;
		}
		sprintf(Buffer(), "%d.%03d", second / 1000, second % 1000);
		Print(80.0f, 226.0f, Buffer(), 2.0f, 0, 0.64f, NULL);
		return;
	}
	if ((GOLFDOC()->m_gameMode & 0x80) == 0)
	{
		return;
	}
	SetColor(0xffffffff);
	int whole = (int)m_carry;
	int frac = (int)((m_carry - (float)whole) * 100.0f);
	float barY = COption::Instance()->gGetBar_Y();
	float left = g_view->GetWidth() * 0.5f - 20.0f;
	float top = barY - 33.0f;
	float right = g_view->GetWidth() * 0.5f + 35.0f;
	float fade = (m_time - 0.5f) + (m_time - 0.5f);
	float fx = 0.0f;
	if (0.0f <= fade)
	{
		fx = fade;
		if (1.0f < fade)
		{
			fx = 1.0f;
		}
	}
	fx = (right - left) * fx;
	float fy = 0.0f;
	if (0.0f <= fade)
	{
		fy = fade;
		if (1.0f < fade)
		{
			fy = 1.0f;
		}
	}
	fy = ((g_view->GetHeight() * 0.5f - 37.0f) - top) * fy;
	m_align = 2;
	sprintf(Buffer(), "%d", whole);
	float y = top + fy;
	Print((left - 5.0f) + fx, y, Buffer(), 4.0f, 0, 0.64f, NULL);
	float dotX = left + fx;
	m_align = 1;
	Print(dotX, y, ".", 4.0f, 0, 0.64f, NULL);
	m_align = 0;
	if (*GDOCPTR983(GOLFDOC(), 0x194) == 0)
	{
		sprintf(Buffer(), "%02dy", frac);
	}
	else
	{
		sprintf(Buffer(), "%dy", frac / 10);
	}
	Print(dotX + 5.0f, y, Buffer(), 4.0f, 0, 0.64f, NULL);

	SetColor(0xffff3434);
	whole = (int)m_total;
	frac = (int)((m_total - (float)whole) * 100.0f);
	left = g_view->GetWidth() * 0.5f - 20.0f;
	top = barY - 12.0f;
	right = g_view->GetWidth() * 0.5f + 35.0f;
	fade = (m_time - 1.0f) + (m_time - 1.0f);
	fx = 0.0f;
	if (0.0f <= fade)
	{
		fx = fade;
		if (1.0f < fade)
		{
			fx = 1.0f;
		}
	}
	fx = (right - left) * fx;
	fy = 0.0f;
	if (0.0f <= fade)
	{
		fy = fade;
		if (1.0f < fade)
		{
			fy = 1.0f;
		}
	}
	fy = ((g_view->GetHeight() * 0.5f - 7.0f) - top) * fy;
	m_align = 2;
	sprintf(Buffer(), "%d", whole);
	y = top + fy;
	Print((left - 5.0f) + fx, y, Buffer(), 4.0f, 0, 0.64f, NULL);
	dotX = left + fx;
	m_align = 1;
	Print(dotX, y, ".", 4.0f, 0, 0.64f, NULL);
	m_align = 0;
	if (*GDOCPTR983(GOLFDOC(), 0x194) == 0)
	{
		if (ball.m_state == 0 && ball.IsHoleIn())
		{
			frac = 0;
		}
		sprintf(Buffer(), "%02dy", frac);
	}
	else if (ball.m_state == 0 && ball.IsHoleIn())
	{
		sprintf(Buffer(), "%dy", 0);
	}
	else
	{
		sprintf(Buffer(), "%dy", frac / 10);
	}
	Print(dotX + 5.0f, y, Buffer(), 4.0f, 0, 0.64f, NULL);

	if (ball.m_state != 0)
	{
		return;
	}
	if (ball.m_bOB)
	{
		return;
	}
	if (ball.IsHoleIn())
	{
		return;
	}
	if (ball.m_water != 0)
	{
		return;
	}
	if (m_groundUI != NULL)
	{
		m_groundUI->SetColor((int)m_alphaGround << 24 | 0xffffff);
		m_groundUI->Render(m_ground, g_view->GetWidth() * 0.5f - 64.0f,
			g_view->GetHeight() * 0.5f - 72.0f, 1.0f, 0);
	}
	if (m_textUI == NULL)
	{
		return;
	}
	if (IsLocalContent(S4_REPLAY_SYSTEM) && Doc()->m_playingMode == 2)
	{
		m_alphaTextA = 0.0f;
		m_alphaTextB = 0.0f;
	}
	m_textUI->SetColor((int)m_alphaTextA << 24 | 0xffffff);
	m_textUI->Render("\xba\xf1\xb0\xc5\xb8\xae",
		g_view->GetWidth() * 0.5f - 80.0f, g_view->GetHeight() * 0.5f - 37.0f,
		1.0f, 0);
	m_textUI->SetColor((int)m_alphaTextB << 24 | 0xffffff);
	m_textUI->Render("\xb3\xb2\xc0\xba\xb0\xc5\xb8\xae",
		g_view->GetWidth() * 0.5f - 80.0f, g_view->GetHeight() * 0.5f - 7.0f,
		1.0f, 0);
	return;
}


CWindInfo::CWindInfo()
{
	Init("[font_wind.jpg");
	m_align = 1;
	Buffer() = new char[32];
	// 983 (00525d60): centro vertical e imagens da fonte para as telas de 2x e 3x
	m_valign = 1;
	m_font.AddScaled(2.0f, "[font_wind_x2.jpg");
	m_font.AddScaled(3.0f, "[font_wind_x3.jpg");
}

void CWindInfo::Display()
{
	if (0)
		new CWindInfo;
	// 983 (005316f0): mais tres itens de vento de 1 m (0x1800002c/2d/2f) e sem o caso do tipo de jogo 3
	unsigned long item = PLAYER(GOLFDOC()->m_currentPlayer)->item;
	if (item == 0x18000006 || item == 0x18000020 || item == 0x1800002c ||
		item == 0x1800002d || item == 0x1800002f)
	{
		SetColor(0xff00ff00);
		sprintf(Buffer(), "1m");
	}
	else if (PLAYER(GOLFDOC()->m_currentPlayer)->item == 0x18000016)
	{
		SetColor(0xff0000ff);
		sprintf(Buffer(), "%.0fm", Wind().GetPower983());
	}
	else if ((GolfBall().m_playFrame >= 0 &&
			Wind().IsInTurbulence(GolfBall().m_pos)) ||
		(GolfBall().m_playFrame < 0 && Wind().IsInTurbulence(g_camera.pivot)))
	{
		SetColor(0xffffffff);
		sprintf(Buffer(), "?");
	}
	else
	{
		SetColor(0xffffffff);
		switch (Doc()->m_golfGame.gameType)
		{
		case 0:
		case 1:
		case 7:
			CCardManager::Instance()->SetPlayerIndex(
				GOLFDOC()->m_currentPlayer);
			break;
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
			CCardManager::Instance()->SetPlayerIndex(0xff);
			break;
		}
		if (CCardManager::Instance()->GetWindColor() == 1)
		{
			SetColor(0xff59d3ff);
		}
		// 983 (005317ad/005318e0): forca = GetPower983 (006ca4b0: vetor com card de vento e itens)
		sprintf(Buffer(), "%.0fm", Wind().GetPower983());
	}
	// 983: posicao pelo ponto "p_wind_speed_text_center" do CUiFrameSetManager e Print na escala da tela
	float x, y;
	if (UiPoint983("p_wind_speed_text_center", &x, &y))
	{
		PrintScaled(4, x, y, Buffer(), 1.0f, 1.0f, 0);
	}
}

CBonusInfo::CBonusInfo()
	: m_pangIcon(NULL), m_pcBangIcon(NULL)
{
	Init("[bigfont.jpg");
	SetColor(0xffffffff);
	m_align = 2;
	Buffer() = new char[32];
	m_time = 0.0f;
	m_pang = 0;
	m_bonusPang = 0;
	m_expPang = 0;
	m_skinsPang = 0;
	m_total = 0;
	if (Doc()->m_golfGame.gameType == 7)
		m_pangIcon = g_resrcmng->GetOverlay("[add_pang_skins.png", 0);
	else
		m_pangIcon = g_resrcmng->GetOverlay("[add_pang.png", 0);
	if (CLoginInfo::Instance()->IsPcBang() != 0)
		m_pcBangIcon = g_resrcmng->GetOverlay("[pc_eventmark.tga", 0);
	if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/))
		m_localIcon =
			g_resrcmng->GetOverlay(MakeStr("%s.tga", "z_Halloween_Male"), 0);
}

CBonusInfo::~CBonusInfo()
{
	// dead copy of the C body of CBonusInfo::Display (its naked version has an indirect jmp, the switch dispatch, which keeps
	// any C code in that function alive): keeps the original call graph (emission order, inline COMDATs)
	if (0)
		do
		{
			if (m_pangIcon != NULL)
			{
				if (Doc()->m_golfGame.gameType == 7)
				{
					WRect dst(g_view->GetWidth() * 0.5f - 256.0f,
						(((g_view->GetHeight() - 256.0f) - 128.0f) + 60.0f) *
							0.5f,
						256.0f, 256.0f);
					WRect src(0.0f, 0.0f, 1.0f, 1.0f);
					m_pangIcon->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
				}
				else
				{
					WRect dst(g_view->GetWidth() * 0.5f - 256.0f,
						(g_view->GetHeight() - 128.0f) * 0.5f, 256.0f, 128.0f);
					WRect src(0.0f, 0.0f, 1.0f, 1.0f);
					m_pangIcon->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
				}
			}
			if (CLoginInfo::Instance()->IsPcBang() != 0 && m_pcBangIcon != NULL)
			{
				WRect dst(g_view->GetWidth() * 0.5f + 150.0f,
					(g_view->GetHeight() + 25.0f) * 0.5f, 70.0f, 50.0f);
				WRect src(0.0f, 0.0f, 1.0f, 1.0f);
				m_pcBangIcon->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
			}
			if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/) &&
				Doc()->IsWearHalloweenItem())
			{
				WOverlay* icon = m_localIcon;
				WRect dst(g_view->GetWidth() * 0.5f + 170.0f,
					g_view->GetHeight() * 0.5f - 52.0f,
					(float)(int)icon->GetWidth(),
					(float)(int)icon->GetHeight());
				WRect src(0.0f, 0.0f, 1.0f, 1.0f);
				icon->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
				m_align = 2;
				sprintf(Buffer(), "%%%d", 100);
				Print(g_view->GetWidth() * 0.5f + 220.0f,
					g_view->GetHeight() * 0.5f - 18.0f, Buffer(), 3.0f, 0, 1.0f,
					NULL);
			}
			int y = 0;
			if (Doc()->m_golfGame.gameType == 7)
			{
				m_align = 2;
				sprintf(Buffer(), "%I64d", m_skinsPang);
				Print(g_view->GetWidth() * 0.5f + 105.0f,
					g_view->GetHeight() * 0.5f - 88.0f, Buffer(), 3.0f, 0, 1.0f,
					NULL);
				m_align = 2;
				sprintf(Buffer(), "%I64d", m_pang);
				Print(g_view->GetWidth() * 0.5f + 100.0f,
					g_view->GetHeight() * 0.5f - 53.0f, Buffer(), 3.0f, 0, 1.0f,
					NULL);
				sprintf(Buffer(), "%I64d", m_bonusPang);
				Print(g_view->GetWidth() * 0.5f + 100.0f,
					g_view->GetHeight() * 0.5f - 18.0f, Buffer(), 3.0f, 0, 1.0f,
					NULL);
				m_align = 0;
				sprintf(Buffer(), "%%%I64d", m_expPang);
				Print(g_view->GetWidth() * 0.5f + 105.0f,
					g_view->GetHeight() * 0.5f - 18.0f, Buffer(), 3.0f, 0, 1.0f,
					NULL);
				m_align = 2;
				sprintf(Buffer(), "%I64d", m_total);
				Print(g_view->GetWidth() * 0.5f + 100.0f,
					g_view->GetHeight() * 0.5f + 25.0f, Buffer(), 3.0f, 0, 1.0f,
					NULL);
				y = 28;
			}
			else
			{
				m_align = 2;
				sprintf(Buffer(), "%I64d", m_pang);
				Print(g_view->GetWidth() * 0.5f + 40.0f,
					g_view->GetHeight() * 0.5f - 53.0f, Buffer(), 3.0f, 0, 1.0f,
					NULL);
				sprintf(Buffer(), "%I64d", m_bonusPang);
				Print(g_view->GetWidth() * 0.5f + 40.0f,
					g_view->GetHeight() * 0.5f - 18.0f, Buffer(), 3.0f, 0, 1.0f,
					NULL);
				m_align = 0;
				sprintf(Buffer(), "%%%I64d", m_expPang);
				Print(g_view->GetWidth() * 0.5f + 45.0f,
					g_view->GetHeight() * 0.5f - 18.0f, Buffer(), 3.0f, 0, 1.0f,
					NULL);
			}
			WOverlay::DrawBox(g_view,
				WRect((g_view->GetWidth() - 400.0f) * 0.5f,
					g_view->GetHeight() * 0.5f + s_fifteen, 400.0f, 3.0f),
				0, 0xff000000, 0.001f);
			WOverlay::DrawBox(g_view,
				WRect((g_view->GetWidth() - 400.0f) * 0.5f + 1.0f,
					g_view->GetHeight() * 0.5f + 16.0f, 398.0f, 1.0f),
				0, 0xffffffff, 0.001f);
			std::vector<unsigned long>& items = Doc()->m_acquireItemList[0];
			int x = 0;
			int row = y;
			if (items.size() == 0)
			{
				Print(g_view->GetWidth() * 0.5f + 30.0f,
					g_view->GetHeight() * 0.5f + (float)y + 28.0f, "-", 3.0f, 0,
					1.0f, NULL);
				break;
			}
			{
				for (std::vector<unsigned long>::iterator it = items.begin();
					it != items.end(); ++it)
				{
					unsigned long typeId = *it;
					WOverlay* icon;
					std::map<unsigned long, WOverlay*>::iterator found =
						m_itemIcons.find(typeId);
					if (found != m_itemIcons.end())
						icon = (*found).second;
					else
					{
						IFF_ITEM_COMMON* item =
							Doc()->m_itemManager.FindCommonItem(typeId);
						if (item == NULL)
							continue;
						switch (typeId >> 26)
						{
						case 2:
						case 5:
						case 6:
						case 0x1d:
							icon = g_resrcmng->GetOverlay(
								MakeStr("%s.tga", item->Icon), 0);
							if (icon == NULL)
								continue;
							m_itemIcons.insert(
								std::map<unsigned long, WOverlay*>::value_type(
									typeId, icon));
							break;
						default:
							continue;
						}
					}
					if (icon != NULL)
					{
						WRect dst(g_view->GetWidth() * 0.5f + (float)x +
								s_fifteen,
							g_view->GetHeight() * 0.5f + (float)row + 25.0f,
							(float)(int)icon->GetWidth(),
							(float)(int)icon->GetHeight());
						if (typeId >> 26 != 2 && typeId >> 26 != 0x1d)
						{
							WRect src(0.0f, 0.0f, 1.0f, 1.0f);
							icon->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
						}
						else
						{
							WRect big(dst.x - 11.0f, dst.y - 11.0f, dst.w,
								dst.h);
							WRect src(0.0f, 0.0f, 1.0f, 1.0f);
							icon->Render(g_view, src, big, 0, 0xffffffff, 0, 0);
						}
						int width = icon->GetWidth();
						if (width <= 40)
							width = 40;
						x += width;
						if (x > 132)
						{
							row += 40;
							x = 0;
						}
					}
				}
			}
		} while (0);
	if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/))
	{
		if (g_resrcmng != NULL && m_localIcon != NULL)
		{
			g_resrcmng->Release(m_localIcon);
			m_localIcon = NULL;
		}
	}
	if (g_resrcmng != NULL)
	{
		if (m_pangIcon != NULL)
		{
			g_resrcmng->Release(m_pangIcon);
			m_pangIcon = NULL;
		}
		if (g_resrcmng != NULL && m_pcBangIcon != NULL)
		{
			g_resrcmng->Release(m_pcBangIcon);
			m_pcBangIcon = NULL;
		}
	}
	for (std::map<unsigned long, WOverlay*>::iterator it = m_itemIcons.begin();
		it != m_itemIcons.end(); ++it)
	{
		if (g_resrcmng != NULL && (*it).second != NULL)
		{
			g_resrcmng->Release((*it).second);
			(*it).second = NULL;
		}
	}
	m_itemIcons.clear();
}

void CBonusInfo::Process(float dt)
{
	m_time += dt;
	m_pang = PLAYER(MyIndex())->totalPang;
	m_bonusPang = PLAYER(MyIndex())->bonusPang;
	// 983 (0053ad60): auxprop e o ultimo fator; a divisao por 100 vem antes dele
	double auxProp = Doc()->GetAuxPartProperty(MyIndex(), 3);
	__int64 expPang = (__int64)(int)((double)(unsigned long)Doc()->m_expRate *
		(double)(m_pang + m_bonusPang) * (double)Doc()->m_pangRate / 100.0 *
		auxProp);
	m_expPang = expPang - m_pang - m_bonusPang;
	m_skinsPang = PLAYER(MyIndex())->skinsTotalPang;
	m_total = m_skinsPang + m_bonusPang + m_pang + m_expPang;
}

void CBonusInfo::Display()
{
	if (m_pangIcon != NULL)
	{
		if (Doc()->m_golfGame.gameType == 7)
		{
			WRect dst(g_view->GetWidth() * 0.5f - 256.0f,
				(((g_view->GetHeight() - 256.0f) - 128.0f) + 60.0f) * 0.5f,
				256.0f, 256.0f);
			WRect src(0.0f, 0.0f, 1.0f, 1.0f);
			m_pangIcon->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
		}
		else
		{
			WRect dst(g_view->GetWidth() * 0.5f - 256.0f,
				(g_view->GetHeight() - 128.0f) * 0.5f, 256.0f, 128.0f);
			WRect src(0.0f, 0.0f, 1.0f, 1.0f);
			m_pangIcon->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
		}
	}
	if (CLoginInfo::Instance()->IsPcBang() != 0 && m_pcBangIcon != NULL)
	{
		WRect dst(g_view->GetWidth() * 0.5f + 150.0f,
			(g_view->GetHeight() + 25.0f) * 0.5f, 70.0f, 50.0f);
		WRect src(0.0f, 0.0f, 1.0f, 1.0f);
		m_pcBangIcon->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
	}
	if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/) &&
		Doc()->IsWearHalloweenItem())
	{
		WOverlay* icon = m_localIcon;
		WRect dst(g_view->GetWidth() * 0.5f + 170.0f,
			g_view->GetHeight() * 0.5f - 52.0f, (float)(int)icon->GetWidth(),
			(float)(int)icon->GetHeight());
		WRect src(0.0f, 0.0f, 1.0f, 1.0f);
		icon->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
		m_align = 2;
		sprintf(Buffer(), "%%%d", 100);
		Print(g_view->GetWidth() * 0.5f + 220.0f,
			g_view->GetHeight() * 0.5f - 18.0f, Buffer(), 3.0f, 0, 1.0f, NULL);
	}
	int y = 0;
	if (Doc()->m_golfGame.gameType == 7)
	{
		m_align = 2;
		sprintf(Buffer(), "%I64d", m_skinsPang);
		Print(g_view->GetWidth() * 0.5f + 105.0f,
			g_view->GetHeight() * 0.5f - 88.0f, Buffer(), 3.0f, 0, 1.0f, NULL);
		m_align = 2;
		sprintf(Buffer(), "%I64d", m_pang);
		Print(g_view->GetWidth() * 0.5f + 100.0f,
			g_view->GetHeight() * 0.5f - 53.0f, Buffer(), 3.0f, 0, 1.0f, NULL);
		sprintf(Buffer(), "%I64d", m_bonusPang);
		Print(g_view->GetWidth() * 0.5f + 100.0f,
			g_view->GetHeight() * 0.5f - 18.0f, Buffer(), 3.0f, 0, 1.0f, NULL);
		m_align = 0;
		sprintf(Buffer(), "%%%I64d", m_expPang);
		Print(g_view->GetWidth() * 0.5f + 105.0f,
			g_view->GetHeight() * 0.5f - 18.0f, Buffer(), 3.0f, 0, 1.0f, NULL);
		m_align = 2;
		sprintf(Buffer(), "%I64d", m_total);
		Print(g_view->GetWidth() * 0.5f + 100.0f,
			g_view->GetHeight() * 0.5f + 25.0f, Buffer(), 3.0f, 0, 1.0f, NULL);
		y = 28;
	}
	else
	{
		m_align = 2;
		sprintf(Buffer(), "%I64d", m_pang);
		Print(g_view->GetWidth() * 0.5f + 40.0f,
			g_view->GetHeight() * 0.5f - 53.0f, Buffer(), 3.0f, 0, 1.0f, NULL);
		sprintf(Buffer(), "%I64d", m_bonusPang);
		Print(g_view->GetWidth() * 0.5f + 40.0f,
			g_view->GetHeight() * 0.5f - 18.0f, Buffer(), 3.0f, 0, 1.0f, NULL);
		m_align = 0;
		sprintf(Buffer(), "%%%I64d", m_expPang);
		Print(g_view->GetWidth() * 0.5f + 45.0f,
			g_view->GetHeight() * 0.5f - 18.0f, Buffer(), 3.0f, 0, 1.0f, NULL);
	}
	WOverlay::DrawBox(g_view,
		WRect((g_view->GetWidth() - 400.0f) * 0.5f,
			g_view->GetHeight() * 0.5f + s_fifteen, 400.0f, 3.0f),
		0, 0xff000000, 0.001f);
	WOverlay::DrawBox(g_view,
		WRect((g_view->GetWidth() - 400.0f) * 0.5f + 1.0f,
			g_view->GetHeight() * 0.5f + 16.0f, 398.0f, 1.0f),
		0, 0xffffffff, 0.001f);
	std::vector<unsigned long>& items = Doc()->m_acquireItemList[0];
	int x = 0;
	int row = y;
	if (items.size() == 0)
	{
		Print(g_view->GetWidth() * 0.5f + 30.0f,
			g_view->GetHeight() * 0.5f + (float)y + 28.0f, "-", 3.0f, 0, 1.0f,
			NULL);
		return;
	}
	{
		for (std::vector<unsigned long>::iterator it = items.begin();
			it != items.end(); ++it)
		{
			unsigned long typeId = *it;
			WOverlay* icon;
			std::map<unsigned long, WOverlay*>::iterator found =
				m_itemIcons.find(typeId);
			if (found != m_itemIcons.end())
				icon = (*found).second;
			else
			{
				IFF_ITEM_COMMON* item =
					Doc()->m_itemManager.FindCommonItem(typeId);
				if (item == NULL)
					continue;
				switch (typeId >> 26)
				{
				case 2:
				case 5:
				case 6:
					icon = g_resrcmng->GetOverlay(MakeStr("%s.tga", item->Icon),
						0);
					if (icon == NULL)
						continue;
					m_itemIcons.insert(
						std::map<unsigned long, WOverlay*>::value_type(typeId,
							icon));
					break;
				default:
					continue;
				}
			}
			if (icon != NULL)
			{
				WRect dst(g_view->GetWidth() * 0.5f + (float)x + s_fifteen,
					g_view->GetHeight() * 0.5f + (float)row + 25.0f,
					(float)(int)icon->GetWidth(),
					(float)(int)icon->GetHeight());
				WRect src(0.0f, 0.0f, 1.0f, 1.0f);
				icon->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
				int width = icon->GetWidth();
				if (width <= 40)
					width = 40;
				x += width;
				if (x > 132)
				{
					row += 40;
					x = 0;
				}
			}
		}
	}
}


CRemainTimeInfo::CRemainTimeInfo()
{
	Init("[font_bar.jpg");
	m_align = 1;
	SetColor(0xff147eff);
	Buffer() = new char[32];
	// 983 (00525e30)
	m_valign = 1;
	m_font.AddScaled(2.0f, "[font_bar_x2.jpg");
	m_font.AddScaled(3.0f, "[font_bar_x3.jpg");
}

void CRemainTimeInfo::Display()
{
	// 983 (00531a60): mesmo texto; posicao pelo ponto "p_digital_timer_text_center" e Print na escala da tela
	if (GetShotTimeLimit() == 0)
	{
		sprintf(Buffer(), "-");
	}
	else
	{
		unsigned long remain = GOLFDOC()->m_remainTime / 1000;
		IsLocalCountry(30);
		if (GOLFDOC()->m_remainTime > remain * 1000)
		{
			remain++;
		}
		sprintf(Buffer(), "%d",
			remain > GetShotTimeLimit() / 1000 ? GetShotTimeLimit() / 1000
											   : remain);
	}
	float x, y;
	if (UiPoint983("p_digital_timer_text_center", &x, &y))
	{
		PrintScaled(4, x, y, Buffer(), 1.0f, 1.0f, 0);
	}
}

CTimeOutInfo::CTimeOutInfo()
{
	Init("[bigfont.jpg");
	Buffer() = new char[32];
	m_align = 2;
	m_state = 3;
	m_alpha = 0.0f;
}

void CTimeOutInfo::Reset()
{
	m_state = 0;
	m_alpha = 0.0f;
}

void CTimeOutInfo::Process(float dt)
{
	switch (m_state)
	{
	case 2:
		m_timer = dt + m_timer;
		if (m_timer > 1.5f)
		{
			m_state = 1;
		}
		break;
	case 0:
		m_alpha = dt * 510.0f + m_alpha;
		if (m_alpha > 255.0f)
		{
			m_alpha = 255.0f;
			m_state = 2;
			m_timer = 0.0f;
		}
		break;
	case 1:
		m_alpha = m_alpha - dt * 510.0f;
		if (!(m_alpha < 0.0f))
		{
			return;
		}
		m_state = 3;
	case 3:
		m_alpha = 0.0f;
		break;
	}
}

void CTimeOutInfo::Display()
{
	if (0)
		new CTimeOutInfo;
	if (m_state != 3)
	{
		sprintf(Buffer(), "%d",
			3 - PLAYER(GOLFDOC()->m_currentPlayer)->timeout);
		SetColor((int)m_alpha << 24 | 0xff0000);
		Print(g_view->GetWidth() * 0.5f - 60.0f,
			g_view->GetHeight() * 0.25f + 3.0f, Buffer(), 1.0f, 0, 1.0f, NULL);
	}
}

CTotalDistInfo::CTotalDistInfo()
{
	Init("[font_hole.jpg");
	Buffer() = new char[32];
	m_align = 1;
}

void CTotalDistInfo::Display()
{
	WVector pin = GOLFDOC()->GetHoleData().pin;
	CGolfBall& ball = GolfBall();
	m_distance =
		WVectorLen(WVector(pin.x - ball.m_pos.x, 0.0f, pin.z - ball.m_pos.z)) *
		0.3125f;
	if (m_distance < s_minDistance)
	{
		sprintf(Buffer(), "%.1fy", m_distance);
	}
	else
	{
		sprintf(Buffer(), "%.0fy", m_distance);
	}
	Print(34.0f, COption::Instance()->gGetBar_Y() + s_barTextY, Buffer(), 3.0f,
		0, 1.0f, NULL);
}


CShotCountInfo::CShotCountInfo()
{
	Init("[font_hole.jpg");
	Buffer() = new char[32];
	m_align = 1;
	m_pTikiData983 = NULL;
	if (IsGameType(0xb))
		m_pTikiData983 =
			CContentsDoc::Instance()->GetContainer((localContentType_t)0xaa);
	if (IsGameType(7))
	{
		CFontInfo::Init("[font_hole_skins_red.jpg", &m_fontRed);
		CFontInfo::Init("[font_hole_skins_grey.jpg", &m_fontGrey);
		CFontInfo::Init("[font_hole_skins_blue.jpg", &m_fontBlue);
		m_coin = g_resrcmng->GetOverlay("coin_pang.tga", 0);
	}
}

CShotCountInfo::~CShotCountInfo()
{
	if (IsGameType(7) && g_resrcmng != NULL && m_coin != NULL)
	{
		g_resrcmng->Release(m_coin);
		m_coin = NULL;
	}
}

void CShotCountInfo::StartFloating()
{
	m_timer = s_floatDelay;
	m_count = 0;
	m_alpha = 255;
}

void CShotCountInfo::Process(float dt)
{
	if (!(m_timer <= 0.0f))
	{
		float old = m_timer;
		m_timer = old - dt;
		if (old - dt < 0.0f)
		{
			m_count = m_count + 1;
			float next = s_floatDelay;
			if (m_count == 60)
			{
				m_timer = 0.0f;
				return;
			}
			m_alpha = m_alpha - 4;
			m_timer = next;
		}
	}
}

void CShotCountInfo::Display(float x, float y, unsigned char index)
{
	// 983 (0053d990): sala tiki (0xb) le a tacada do container (+0x24) e o placar de FUN_005706c0 (*(+0x20)+0xc)
	if (Doc()->m_golfGame.gameType == 0xb)
	{
		sprintf(Buffer(), "%d",
			m_pTikiData983 ? TIKIDOC983(m_pTikiData983)->m_24 : 0);
	}
	else if (Doc()->m_golfGame.gameType == 1)
	{
		sTeamData* team = GOLFDOC()->GetTeam(index);
		sprintf(Buffer(), "%d",
			(unsigned char)team->stroke[GOLFDOC()->m_currentHole - 1]);
	}
	else
	{
		sPlayerData* player = GOLFDOC()->GetPlayer(index);
		sprintf(Buffer(), "%d",
			(unsigned char)player->stroke[GOLFDOC()->m_currentHole - 1]);
	}

	if (IsLocalContent(S4_REPLAY_SYSTEM) && Doc()->m_playingMode == 2)
	{
		sPlayerData* player = GOLFDOC()->GetPlayer(index);
		player->stroke[GOLFDOC()->m_currentHole - 1] =
			Doc()->m_replayHoleStroke;
		GOLFDOC()->GetPlayer(index)->score = 0;
		player = GOLFDOC()->GetPlayer(index);
		sprintf(Buffer(), "%d",
			(unsigned char)player->stroke[GOLFDOC()->m_currentHole - 1]);
	}
	Print(x, y, Buffer(), 1.0f, 0, 1.0f, NULL);

	if (Doc()->m_golfGame.gameType == 0xb)
		sprintf(Buffer(), "%d",
			m_pTikiData983
				? TIKIDOC983(m_pTikiData983)->m_pMine->m_0c
				: 0);
	else if (Doc()->m_golfGame.gameType == 1)
		sprintf(Buffer(), "%d", GOLFDOC()->GetTeam(index)->score);
	else if (Doc()->m_golfGame.gameType == 6)
	{
		CSharedDoc* doc = Doc();
		sprintf(Buffer(), "%d",
			doc->m_rivalList[doc->GetIndex(doc->m_teamPlayerGuid[index])]
				.guildPoint);
	}
	else
		sprintf(Buffer(), "%d", GOLFDOC()->GetPlayer(index)->score);
	Print(x, y + 16.0f, Buffer(), 1.0f, 0, 1.0f, NULL);

	if (Doc()->m_golfGame.gameType == 7)
	{
		unsigned char right = index & 2;
		float offset = right ? -48.0f : 0.0f;
		if (m_coin != NULL)
		{
			m_coin->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
				WRect(x - 52.0f + offset, y + 32.0f, 32.0f, 32.0f), 0, m_color,
				0, 0);
		}

		char price[32] = "";
		__int64 pang = Doc()->m_userInfo[index].stat.i64Pang;
		MakePriceStrA(price, wAbs(pang));
		if (pang < 0)
			sprintf(Buffer(), "-%s", price);
		else
			strcpy(Buffer(), price);
		Print(x - 22.0f + offset, y + 39.0f, Buffer(), 0.0f, 0, 1.0f, NULL);

		__int64 holePang = GOLFDOC()->GetPlayer(index)->skinsHolePang;
		MakePriceStrA(price, wAbs(holePang));
		float px = (right ? -108.0f : 0.0f) + x + 20.0f;
		float py = y + 29.0f;
		if (GOLFDOC()->m_gameMode & 0x1000)
		{
			if (GOLFDOC()->GetPlayer(index)->skinsHolePang > 0)
			{
				sprintf(Buffer(), "+%s", price);
				SetColor((0xff - m_alpha) << 24 | 0xffffff);
				Print(px, (float)(60 - m_count) + py, Buffer(), 0.0f, 0, 1.0f,
					&m_fontBlue);
			}
			else if (GOLFDOC()->GetPlayer(index)->skinsHolePang < 0)
			{
				SetColor(m_alpha << 24 | 0xffffff);
				sprintf(Buffer(), "-%s", price);
				Print(px, py - (float)m_count, Buffer(), 0.0f, 0, 1.0f,
					&m_fontRed);
			}
		}
	}
}

CPowerInfo::CPowerInfo()
{
	Init("[font_bar.jpg");
	Buffer() = new char[32];
	m_align = 1;
	// 983 (005283f0)
	m_valign = 1;
	m_font.AddScaled(2.0f, "[font_bar_x2.jpg");
	m_font.AddScaled(3.0f, "[font_bar_x3.jpg");
}

void CPowerInfo::Display()
{
	SetColor(GOLFDOC()->m_uiAlpha << 24 | 0xffffff);
	CGolfBall& ball = GolfBall();
	float dx = m_pos.x - ball.m_pos.x;
	float dz = m_pos.z - ball.m_pos.z;
	float dist = sqrtf(dz * dz + dx * dx) * 0.3125f;
	// 983 (00531c20): posicoes pelas medidas "d_club_range_x", "d_half_club_range_x" e "d_club_range_y" do
	// CUiFrameSetManager e Print na escala da tela
	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	CUiFrameSetManager::NamedDim* fullX = ui->FindNamedDim("d_club_range_x");
	CUiFrameSetManager::NamedDim* halfX = ui->FindNamedDim("d_half_club_range_x");
	CUiFrameSetManager::NamedDim* rangeY = ui->FindNamedDim("d_club_range_y");
	CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0), (float)(g_view->GetHeight() / 480.0));
	sprintf(Buffer(), "%.0fy", dist);
	if (fullX != NULL && rangeY != NULL)
	{
		PrintScaled(4, (float)floor(fullX->Calc(scl)), (float)floor(rangeY->Calc(scl)), Buffer(), 1.0f, 1.0f, 0);
	}
	sprintf(Buffer(), "%.0fy", dist * 0.5f);
	if (halfX != NULL && rangeY != NULL)
	{
		PrintScaled(4, (float)floor(halfX->Calc(scl)), (float)floor(rangeY->Calc(scl)), Buffer(), 1.0f, 1.0f, 0);
	}
}


CClubTypeInfo::CClubTypeInfo()
{
	Init("[font_club.jpg");
	Buffer() = new char[32];
	// 983 (00526150): centrado nos dois eixos, imagens "_x2"/"_x3"
	m_align = 1;
	m_valign = 1;
	m_font.AddScaled(2.0f, "[font_club_x2.jpg");
	m_font.AddScaled(3.0f, "[font_club_x3.jpg");
}

void CClubTypeInfo::Display()
{
	sprintf(Buffer(), "%x", GolfClub().GetID() > 15 ? 15 : GolfClub().GetID());
	// 983 (00531f60): no ponto "p_club_type_center" (sem arredondar), espacamento 1 e Print na escala da tela
	float x, y;
	if (UiPoint983("p_club_type_center", &x, &y, false))
	{
		PrintScaled(4, x, y, Buffer(), 1.0f, 1.0f, 0);
	}
}

CAccuracyInfo::CAccuracyInfo()
{
	Buffer2() = NULL;
	Init("[font_bar.jpg");
	SetColor(0xffffffff);
	m_align = 0;
	m_active = false;
	Buffer2() = NULL;
	Buffer() = NULL;
	Buffer() = new char[32];
	Buffer()[0] = 0;
	Buffer2() = new char[32];
	Buffer2()[0] = 0;
	m_x = 0.0f;
	m_align = 1;
}

CAccuracyInfo::~CAccuracyInfo()
{
	if (Buffer() != NULL)
	{
		delete[] Buffer();
		Buffer() = NULL;
	}
	if (Buffer2() != NULL)
	{
		delete[] Buffer2();
		Buffer2() = NULL;
	}
}

LOCAL_INLINE float AccuracyBarX()
{
	return GOLFDOC()->m_bar.impact;
}

LOCAL_INLINE int AccuracyType()
{
	return *(int*)GDOCPTR983(GOLFDOC(), 0xf0);
}

LOCAL_INLINE char AccuracyFlag()
{
	return *GDOCPTR983(GOLFDOC(), 0xf4);
}

void CAccuracyInfo::Process(float dt)
{
	if (m_active)
	{
		m_alpha = m_alpha - dt * 128.0f;
		if (m_alpha < 0.0f)
		{
			m_alpha = 0.0f;
			m_active = false;
		}
	}
}

void CAccuracyInfo::SetPower()
{
	m_active = true;
	m_alpha = 255.0f;
	sprintf(Buffer(), "@Full");
	m_x = 500.0f; // 983 (00526320): 500.0 (_DAT_00d08898); o 645 tinha 436
}

void CAccuracyInfo::SetAccuracy()
{
	m_active = true;
	m_alpha = 255.0f;
	m_x = AccuracyBarX(); // 983 (0052a510): +0xe4 da barra sem o -64 do 645 (Display ja desconta 140 na medida)
	Buffer()[0] = 0;
	Buffer2()[0] = 0;
	switch (AccuracyType())
	{
	case 0:
	case 1:
		sprintf(Buffer(), "@Bad");
		break;
	case 2:
	case 3:
	case 4:
		if (IsLocalContent((localContentType_t)0x51 /*983: era 0x50; medido*/) || AccuracyType() == 4)
		{
			if ((GolfBall().m_initSpecial & 0x70) == 0)
			{
				if ((GolfBall().m_initSpecial & 3) != 0)
				{
					sprintf(Buffer(), "@PowerSpin");
					return;
				}
				if ((GolfBall().m_initSpecial & 0xc) != 0)
				{
					sprintf(Buffer(), "@PowerCurve");
					return;
				}
				if (!IsLocalContent((localContentType_t)0x51 /*983: era 0x50; medido*/))
				{
					if (GolfClub().PowerShotType() != 0 && AccuracyFlag() != 0)
					{
						sprintf(Buffer(), "@SuperPangya");
						return;
					}
					sprintf(Buffer(), "@Pangya");
					return;
				}
				if (AccuracyType() == 4)
				{
					if (GolfClub().PowerShotType() != 0 && AccuracyFlag() != 0)
					{
						sprintf(Buffer(), "@SuperPangya");
						return;
					}
					sprintf(Buffer(), "@Pangya");
					return;
				}
			}
			else
			{
				if (GolfClub().PowerShotType() == 2 ||
					GolfClub().PowerShotType() == 3)
					strcpy(Buffer(), "@Power");
				else
					strcpy(Buffer(), "@");
				if ((GolfBall().m_initSpecial & 0x10) != 0)
					strcat(Buffer(), "Tomahawk");
				else if ((GolfBall().m_initSpecial & 0x20) != 0)
					strcat(Buffer(), "Cobra");
				else if ((GolfBall().m_initSpecial & 0x40) != 0)
					strcat(Buffer(), "Spike");
				if ((GolfBall().m_initSpecial & 3) != 0)
				{
					sprintf(Buffer2(), "@PowerSpin");
					return;
				}
				if ((GolfBall().m_initSpecial & 0xc) != 0)
				{
					sprintf(Buffer2(), "@PowerCurve");
					return;
				}
			}
		}
		break;
	}
}


// 983 (00539410): o texto de precisao (@Pangya/@Bad/...) e um quadro do CUiFrameSetManager desenhado nas medidas
// "d_impact_accuracy_x_offset" (x da batida - 140) e "d_impact_accuracy_y_offset" (-22 / -34); o 645 usava a barra + CSimpleUI
void CAccuracyInfo::Display()
{
	if (0)
		((CAccuracyInfo*)0)->SetAccuracy();
	C_ASSERT_VC10(offsetof(CAccuracyInfo, m_x) == 0x34);
	if (!m_active)
		return;
	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	CUiFrameSetManager::NamedDim* offsetX = ui->FindNamedDim("d_impact_accuracy_x_offset");
	CUiFrameSetManager::NamedDim* offsetY = ui->FindNamedDim("d_impact_accuracy_y_offset");
	if (strchr(Buffer(), '@') != NULL)
	{
		unsigned long color;
		if (IsLocalContent((localContentType_t)0x51 /*983: era 0x50; medido*/))
		{
			if (stricmp(Buffer(), "@Full") == 0 || stricmp(Buffer(), "@Bad") == 0 || AccuracyType() == 4)
				color = (int)(m_alpha + 0.5f) << 24 | 0xffffff;
			else
				color = (int)(m_alpha + 0.5f) << 24 | 0x8a8a8a;
		}
		else
			color = (int)(m_alpha + 0.5f) << 24 | 0xffffff;
		if (offsetX != NULL && offsetY != NULL)
		{
			offsetX->Set((float)(m_x - 140.0), 4);
			offsetY->Set(-22.0f, 4);
			ui->DrawFrameSet(std::string(Buffer()), g_view, 0, color);
		}
	}
	if (strchr(Buffer2(), '@') != NULL && offsetX != NULL && offsetY != NULL)
	{
		offsetX->Set((float)(m_x - 140.0), 4);
		offsetY->Set(-34.0f, 4);
		ui->DrawFrameSet(std::string(Buffer2()), g_view, 0, (int)(m_alpha + 0.5f) << 24 | 0xffffff);
	}
}

CHoleInfo::CHoleInfo()
{
	m_pCupUI983 = NULL;
	m_pTikiData983 = NULL;
	m_ui[0] = NULL;
	m_ui[1] = NULL;
	m_ui[2] = NULL;
	m_ui[3] = NULL;
	m_ui[4] = NULL;
	m_overlay[0] = NULL;
	m_overlay[1] = NULL;
	m_overlay[2] = NULL;
	m_overlay[3] = NULL;
	m_overlay[4] = NULL;
	m_overlay[5] = NULL;
	m_overlay[6] = NULL;
	m_overlay[7] = NULL;
	m_overlay[8] = NULL;
	m_overlay[9] = NULL;
	m_overlay[10] = NULL;
	m_overlay[11] = NULL;
	Init("[bigfont.jpg");
	Buffer() = new char[32];
	m_time = 0.0f;
	m_alpha[0] = 0.0f;
	m_alpha[1] = 0.0f;
	m_slide[0].startX = g_view->GetWidth() * 0.5f - 116.0f;
	m_slide[0].startY = 97.0f;
	m_slide[0].endX = -14.0f;
	m_slide[0].endY = 8.0f;
	m_slide[1].startX = g_view->GetWidth() * 0.5f - 75.0f;
	m_slide[1].startY = 127.0f;
	m_slide[1].endX = 100.0f;
	m_slide[1].endY = 7.0f;
	m_slide[2].startX = g_view->GetWidth() * 0.5f - 144.0f;
	m_slide[2].startY = 159.0f;
	m_slide[2].endX = 85.0f;
	m_slide[2].endY = 30.0f;
	// 983 (005320a0): sala tiki (0xb) usa "[eventmap.jpg" com os cursos 0x40/0x41; os demais "[start_name.jpg"
	// com os cursos de indice <= 0x15
	std::map<unsigned int, IFF_STRUCT::sCourse>& courses =
		Doc()->m_itemManager.m_CourseMap;
	std::map<unsigned int, IFF_STRUCT::sCourse>::iterator it;
	if (Doc()->m_golfGame.gameType == 0xb)
	{
		m_ui[0] = new CSimpleUI("[eventmap.jpg");
		for (unsigned int id = 0x40; (int)id < 0x42; ++id)
		{
			it = courses.find(id | 0x28000000);
			if (it != courses.end())
			{
				m_ui[0]->Add((*it).second.c.Name,
					WRect(256.0f, (float)(int)(id - 0x40) * 36.0f, 256.0f, 36.0f),
					false);
			}
		}
	}
	else
	{
		m_ui[0] = new CSimpleUI("[start_name.jpg");
		for (it = courses.begin(); it != courses.end(); ++it)
		{
			unsigned int index = (*it).second.c.TypeId & 0x3ffffff;
			if (index > 0x15)
				break;
			WRect rect;
			if (index < 14)
				rect = WRect(0.0f, (float)(int)index * 36.0f, 256.0f, 36.0f);
			else
				rect = WRect(256.0f, (float)(int)(index - 14) * 36.0f, 256.0f, 36.0f);
			m_ui[0]->Add((*it).second.c.Name, rect, false);
		}
	}

	m_ui[1] = new CSimpleUI("[start_hole.png");
	m_ui[1]->Add("Hole", WRect(0.0f, 0.0f, 64.0f, 32.0f), false);
	m_ui[1]->Add("Par", WRect(0.0f, 32.0f, 64.0f, 32.0f), false);
	unsigned char type = Doc()->m_golfGame.gameType;
	// 983 (005347e0): sala tiki (0xb) tem a barra Cup/Scr propria e o container 0xaa; curso curto ganha "(short)"
	if (type == 0xb)
	{
		m_pCupUI983 = new CSimpleUI("[cuplevel.jpg");
		m_pCupUI983->Add("Cup", WRect(0.0f, 0.0f, 128.0f, 32.0f), false);
		m_pCupUI983->Add("Scr", WRect(0.0f, 32.0f, 128.0f, 32.0f), false);
		void* pContainer =
			CContentsDoc::Instance()->GetContainer((localContentType_t)0xaa);
		if (pContainer)
			m_pTikiData983 = pContainer;
	}
	if (GOLFDOC()->m_b195_983)
		m_ui[1]->Add("(short)", WRect(0.0f, 224.0f, 99.0f, 32.0f), false);
	if (type == 7)
	{
		m_ui[1]->Add("Pang", WRect(0.0f, 64.0f, 64.0f, 32.0f), false);
		m_ui[1]->Add("x2", WRect(0.0f, 96.0f, 64.0f, 32.0f), false);
		m_ui[1]->Add("x4", WRect(0.0f, 128.0f, 64.0f, 32.0f), false);
		m_ui[1]->Add("x6", WRect(0.0f, 160.0f, 64.0f, 32.0f), false);
		m_ui[1]->Add("x8", WRect(0.0f, 192.0f, 64.0f, 32.0f), false);
	}
	if (Doc()->m_golfGame.gameType == 10)
	{
		m_overlay[11] = g_resrcmng->GetOverlay("text_approach.png", 0);
	}
	else
	{
		m_ui[2] = new CSimpleUI("[start_posi.jpg");
		m_ui[3] = new CSimpleUI("[nplayer.jpg");
		if (Doc()->m_golfGame.gameType == 0)
		{
			m_ui[2]->Add("\xbc\xb1\xb5\xce", WRect(0.0f, 0.0f, 64.0f, 32.0f),
				false);
			m_ui[3]->Add("1", WRect(0.0f, 0.0f, 32.0f, 16.0f), false);
			m_ui[3]->Add("2", WRect(32.0f, 0.0f, 32.0f, 16.0f), false);
			m_ui[3]->Add("3", WRect(0.0f, 16.0f, 32.0f, 16.0f), false);
			m_ui[3]->Add("4", WRect(32.0f, 16.0f, 32.0f, 16.0f), false);
		}
		else
		{
			m_ui[2]->Add("vs", WRect(0.0f, 32.0f, 64.0f, 32.0f), false);
			m_ui[3]->Add("red", WRect(0.0f, 32.0f, 32.0f, 16.0f), false);
			m_ui[3]->Add("blue", WRect(32.0f, 32.0f, 32.0f, 16.0f), false);
		}
		m_overlay[1] = g_resrcmng->GetOverlay("[complete.png", 0);
		m_overlay[2] = g_resrcmng->GetOverlay("[score_event.png", 0);
		if (Doc()->m_golfGame.gameType == 7)
		{
			m_overlay[9] = g_resrcmng->GetOverlay("[skins_pang01.png", 0);
			m_overlay[10] = g_resrcmng->GetOverlay("[skins_pang03.png", 0);
			m_overlay[3] = g_resrcmng->GetOverlay("extrastage.png", 0);
			m_overlay[4] = g_resrcmng->GetOverlay("extrastagesmall.png", 0);
			m_overlay[5] = g_resrcmng->GetOverlay("P_coin01.png", 0);
			m_overlay[6] = g_resrcmng->GetOverlay("P_coin02.png", 0);
			m_overlay[7] = g_resrcmng->GetOverlay("P_coin03.png", 0);
			m_ui[4] = new CSimpleUI("[emoti_object.png");
			m_ui[4]->Add("0", WRect(0.0f, 128.0f, 64.0f, 64.0f), false);
			m_ui[4]->Add("1", WRect(64.0f, 128.0f, 64.0f, 64.0f), false);
			m_ui[4]->Add("2", WRect(128.0f, 128.0f, 64.0f, 64.0f), false);
			m_ui[4]->Add("3", WRect(192.0f, 128.0f, 64.0f, 64.0f), false);
			m_ui[4]->Add("4", WRect(0.0f, 192.0f, 64.0f, 64.0f), false);
			m_ui[4]->Add("5", WRect(64.0f, 192.0f, 64.0f, 64.0f), false);
			m_ui[4]->Add("6", WRect(128.0f, 192.0f, 64.0f, 64.0f), false);
			m_ui[4]->Add("7", WRect(192.0f, 192.0f, 64.0f, 64.0f), false);
			m_emote = -1;
		}
		m_overlay[0] = g_resrcmng->GetOverlay("text_shufflebonus.png", 0);
		m_overlay[8] = g_resrcmng->GetOverlay("text_mapbonus.png", 0);
	}
}


CHoleInfo::~CHoleInfo()
{
	if (m_pCupUI983 != NULL)
	{
		delete m_pCupUI983;
		m_pCupUI983 = NULL;
	}
	if (m_ui[0] != NULL)
	{
		delete m_ui[0];
		m_ui[0] = NULL;
	}
	if (m_ui[1] != NULL)
	{
		delete m_ui[1];
		m_ui[1] = NULL;
	}
	if (m_ui[2] != NULL)
	{
		delete m_ui[2];
		m_ui[2] = NULL;
	}
	if (m_ui[3] != NULL)
	{
		delete m_ui[3];
		m_ui[3] = NULL;
	}
	if (g_resrcmng != NULL && m_overlay[9] != NULL)
	{
		g_resrcmng->Release(m_overlay[9]);
		m_overlay[9] = NULL;
	}
	if (g_resrcmng != NULL && m_overlay[10] != NULL)
	{
		g_resrcmng->Release(m_overlay[10]);
		m_overlay[10] = NULL;
	}
	if (g_resrcmng != NULL && m_overlay[11] != NULL)
	{
		g_resrcmng->Release(m_overlay[11]);
		m_overlay[11] = NULL;
	}
	if (g_resrcmng != NULL && m_overlay[3] != NULL)
	{
		g_resrcmng->Release(m_overlay[3]);
		m_overlay[3] = NULL;
	}
	if (g_resrcmng != NULL && m_overlay[4] != NULL)
	{
		g_resrcmng->Release(m_overlay[4]);
		m_overlay[4] = NULL;
	}
	if (g_resrcmng != NULL && m_overlay[5] != NULL)
	{
		g_resrcmng->Release(m_overlay[5]);
		m_overlay[5] = NULL;
	}
	if (g_resrcmng != NULL && m_overlay[6] != NULL)
	{
		g_resrcmng->Release(m_overlay[6]);
		m_overlay[6] = NULL;
	}
	if (g_resrcmng != NULL && m_overlay[7] != NULL)
	{
		g_resrcmng->Release(m_overlay[7]);
		m_overlay[7] = NULL;
	}
	if (g_resrcmng != NULL && m_overlay[0] != NULL)
	{
		g_resrcmng->Release(m_overlay[0]);
		m_overlay[0] = NULL;
	}
	if (m_ui[4] != NULL)
	{
		delete m_ui[4];
		m_ui[4] = NULL;
	}
	if (g_resrcmng != NULL && m_overlay[1] != NULL)
	{
		g_resrcmng->Release(m_overlay[1]);
		m_overlay[1] = NULL;
	}
	if (g_resrcmng != NULL && m_overlay[2] != NULL)
	{
		g_resrcmng->Release(m_overlay[2]);
		m_overlay[2] = NULL;
	}
	if (g_resrcmng != NULL && m_overlay[8] != NULL)
	{
		g_resrcmng->Release(m_overlay[8]);
		m_overlay[8] = NULL;
	}
}

void CHoleInfo::Flicker()
{
	m_state = 4;
	m_flicker = 0;
}

void CHoleInfo::Reset()
{
	unsigned char index = 0;
	m_time = 0.0f;
	m_state = 0;
	m_alpha[0] = 0.0f;
	m_alpha[1] = 0.0f;
	m_slide[0].x = m_slide[0].startX;
	m_player = 0;
	m_slide[0].y = m_slide[0].startY;
	m_slide[0].scale = 1.0f;
	m_slide[1].x = m_slide[1].startX;
	m_slide[1].y = m_slide[1].startY;
	m_slide[1].scale = 1.0f;
	m_slide[2].x = m_slide[2].startX;
	m_slide[2].y = m_slide[2].startY;
	m_slide[2].scale = 1.0f;
	if (GOLFDOC()->m_playerNum != 0)
	{
		while (PLAYER(index)->state == 3 || (PLAYER(index)->order & 0xf) != 1)
		{
			++index;
			if (GOLFDOC()->m_playerNum <= index)
			{
				return;
			}
		}
		m_player = index;
	}
}

void CHoleInfo::Fix()
{
	m_state = 3;
	m_time = 0.0f;
	m_alpha[0] = 255.0f;
	m_alpha[1] = 255.0f;
	m_slide[0].x = m_slide[0].endX;
	m_slide[0].y = m_slide[0].endY;
	m_slide[0].scale = 0.5f;
	m_slide[1].x = m_slide[1].endX;
	m_slide[1].y = m_slide[1].endY;
	m_slide[1].scale = 0.6f;
	m_slide[2].x = m_slide[2].endX;
	m_slide[2].y = m_slide[2].endY;
	m_slide[2].scale = 0.6f;
}

LOCAL_INLINE int DocTeamScore(int team)
{
	return Doc()->m_teamResult[team].score;
}

LOCAL_INLINE int DocCourse()
{
	return (int)Doc()->m_golfGame.pCourse;
}

LOCAL_INLINE int DocInt(int offset)
{
	return *(int*)((unsigned char*)Doc() + offset);
}

// B1533: GolfInt/GolfByte recebem GDOCOFS983(0xNNN) (cru do 983 -> segue o membro): no cl 14.44 o std::list/std::string do
// CGolfDoc andam o resto e GolfByte(0x17c) lia outro campo como m_currentHole (tacada do buraco errado no placar de pang).
LOCAL_INLINE int GolfInt(int offset)
{
	return *(int*)((unsigned char*)GOLFDOC() + offset);
}

// D949: 983 0053aec0 soma 15.0 (0xd04f88). No VC10 o cru alem do fim da tabela (20 ints = 0x50) cai em s_fifteen
// (+0x54, ordem do .rdata da unidade); no cl novo a ordem nao e garantida -> o nome.
LOCAL_INLINE float VsOffset()
{
#if _MSC_VER > 1600
	return s_fifteen;
#else
	return *(const float*)((const char*)s_clearBonusTable + 0x54);
#endif
}

unsigned char __fastcall GetHoleIndex(unsigned char hole);
int __fastcall IsMassGame(unsigned char gameType);

void CHoleInfo::Display()
{
	if (0.0f < m_alpha[0])
	{
		unsigned long alpha = (unsigned long)(int)m_alpha[0] << 24;
		unsigned char gameType = Doc()->m_golfGame.gameType;
		if (*GDOCPTR983(GOLFDOC(), 0x194) == 0)
		{
			const char* name = (const char*)(DocCourse() + 8);
			if (m_ui[0] != NULL)
			{
				m_ui[0]->SetColor(alpha | 0xffffff);
				m_ui[0]->Render(name, m_slide[0].x, m_slide[0].y,
					m_slide[0].scale, 0);
			}
			if (gameType == 0xb)
			{
				// 983 (0053aec0): sala tiki mostra Cup/Scr com os dois contadores do container 0xaa
				SetColor(alpha | 0xffffff);
				m_pCupUI983->SetColor(alpha | 0xffffff);
				m_pCupUI983->Render("Cup", m_slide[1].x, m_slide[1].y,
					m_slide[1].scale, 0);
				m_pCupUI983->Render("Scr",
					m_slide[1].scale * 65.0f + m_slide[1].x, m_slide[1].y,
					m_slide[1].scale, 0);
				int first = TIKIDOC983(m_pTikiData983)->m_7c;
				int second = TIKIDOC983(m_pTikiData983)->m_pMine->m_0c;
				sprintf(Buffer(), "%d", first);
				Print(m_slide[1].scale * 26.0f + m_slide[1].x,
					m_slide[1].scale * 4.0f + m_slide[1].y, Buffer(), 2.0f, 0,
					m_slide[1].scale, NULL);
				sprintf(Buffer(), "%d", second);
				Print(m_slide[1].scale * 145.0f + m_slide[1].x,
					m_slide[1].scale * 4.0f + m_slide[1].y, Buffer(), 2.0f, 0,
					m_slide[1].scale, NULL);
				goto holeinfo_tiki_done983;
			}
			if (m_ui[1] != NULL)
			{
				m_ui[1]->SetColor(alpha | 0xffffff);
				m_ui[1]->Render("Hole", m_slide[1].x, m_slide[1].y,
					m_slide[1].scale, 0);
				m_ui[1]->Render("Par", m_slide[1].scale * 105.0f + m_slide[1].x,
					m_slide[1].y, m_slide[1].scale, 0);
				if (GOLFDOC()->m_b195_983)
					m_ui[1]->Render("(short)",
						m_slide[1].x - 15.0f + m_slide[1].scale * 105.0f * 2.0f,
						m_slide[1].y + 2.0f, m_slide[1].scale, 0);
			}
			SetColor(alpha | 0xffffff);
			// 983 (0053b3a0): numero mostrado = GetHoleValue983(GetHoleIndex(buraco)) & 0xff, nao o buraco cru
			sprintf(Buffer(), "%d",
				(unsigned int)DOCJP()->GetHoleValue983(
					GetHoleIndex(GOLFDOC()->m_currentHole)) & 0xff);
			Print(m_slide[1].scale * 65.0f + m_slide[1].x,
				m_slide[1].scale * 4.0f + m_slide[1].y, Buffer(), 2.0f, 0,
				m_slide[1].scale, NULL);
			sprintf(Buffer(), "/");
			Print(m_slide[1].scale * 96.0f + m_slide[1].x,
				m_slide[1].scale * 4.0f + m_slide[1].y, Buffer(), 2.0f, 0,
				m_slide[1].scale, NULL);
			sprintf(Buffer(), "%d", (unsigned int)GOLFDOC()->GetHoleData().par);
			Print(m_slide[1].scale * 160.0f + m_slide[1].x,
				m_slide[1].scale * 4.0f + m_slide[1].y, Buffer(), 2.0f, 0,
				m_slide[1].scale, NULL);
			holeinfo_tiki_done983:;
		}
		else if (gameType == 10 || (m_state != 1 && m_state != 0))
		{
			const char* name = (const char*)(DocCourse() + 8);
			if (gameType == 10)
			{
				if (m_ui[0] != NULL)
				{
					m_ui[0]->SetColor(alpha | 0xffffff);
					m_ui[0]->Render(name, m_slide[0].endX, m_slide[0].endY,
						0.5f, 0);
				}
				if (m_ui[1] != NULL)
				{
					m_ui[1]->SetColor(alpha | 0xffffff);
					m_ui[1]->Render("Hole", m_slide[1].endX, m_slide[1].endY,
						0.6f, 0);
				}
				SetColor(alpha | 0xffffff);
				unsigned int holes = Doc()->m_golfGame.holes;
				unsigned char index = GetHoleIndex(GOLFDOC()->m_currentHole);
				sprintf(Buffer(), "%d/%d", (unsigned int)index, holes);
				Print(m_slide[1].endX + 39.0f, m_slide[1].endY + 2.4f, Buffer(),
					2.0f, 0, 0.6f, NULL);
			}
			else
			{
				if (m_ui[0] != NULL)
				{
					m_ui[0]->SetColor(alpha | 0xffffff);
					m_ui[0]->Render(name, m_slide[0].x, m_slide[0].y,
						m_slide[0].scale, 0);
				}
				if (m_overlay[4] != NULL)
				{
					_WRECT src;
					src.x = 0.0f;
					src.y = 0.0f;
					src.w = 1.0f;
					src.h = 1.0f;
					_WRECT dst;
					dst.x = m_slide[1].x;
					dst.y = m_slide[1].y;
					dst.w = 128.0f;
					dst.h = 32.0f;
					m_overlay[4]->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
				}
			}
		}
		if (gameType == 7)
		{
			sprintf(Buffer(), "%I64d", *(__int64*)GDOCPTR983(GOLFDOC(), 0x1b8));
			// 983 (0053aec0): largura do valor = GetTextWidth(buffer, 1.0, escala)
			float width = GetTextWidth(Buffer(), 1.0f, m_slide[2].scale);
			SetColor(((unsigned long)(int)m_alpha[0] << 24) | 0xffde44);
			Print(m_slide[2].scale * 118.0f + m_slide[2].x,
				m_slide[2].scale * 4.0f + m_slide[2].y, Buffer(), 2.0f, 0,
				m_slide[2].scale, NULL);
			if (1 < (unsigned int)GolfInt(GDOCOFS983(0x1c0)) && m_ui[1] != NULL)
			{
				m_ui[1]->Render(MakeStr("x%d", GolfInt(GDOCOFS983(0x1c0))),
					m_slide[2].scale * 218.0f + m_slide[2].x + width,
					m_slide[2].y, m_slide[2].scale, 0);
			}
			if (m_state == 3)
			{
				if (1 < (unsigned int)GolfInt(GDOCOFS983(0x1c0)))
				{
					if (m_ui[1] != NULL)
					{
						m_ui[1]->Render(MakeStr("x%d", GolfInt(GDOCOFS983(0x1c0))),
							m_slide[2].scale * 118.0f + m_slide[2].x + width,
							m_slide[2].y, m_slide[2].scale, 0);
					}
					width = m_slide[2].scale * 32.0f + width + 3.0f;
				}
				if (m_ui[1] != NULL)
				{
					m_ui[1]->Render("Pang",
						m_slide[2].scale * 118.0f + m_slide[2].x + width,
						m_slide[2].y, m_slide[2].scale, 0);
				}
			}
		}
	}
	if (Doc()->m_playingMode == 2)
		return;
	if (m_alpha[1] <= 0.0f)
		return;
	if (GolfInt(GDOCOFS983(0x88)) != 0xf)
		return;
	unsigned long alphaB = (unsigned long)(int)m_alpha[1] << 24;
	if (m_state != 3 && Doc()->m_gameMode == 0)
	{
		_WRECT src;
		_WRECT dst;
		src.x = 0.0f;
		src.y = 0.0f;
		src.w = 1.0f;
		src.h = 1.0f;
		if (Doc()->m_golfGame.gameType == 7)
		{
			if (*GDOCPTR983(GOLFDOC(), 0x194) != 0 && m_overlay[3] != NULL)
			{
				dst.x = g_view->GetWidth() * 0.5f - 110.0f;
				dst.y = 97.0f;
				dst.w = 256.0f;
				dst.h = 64.0f;
				m_overlay[3]->Render(g_view, src, dst, 0, alphaB | 0xffffff, 0, 0); // 983: com alfa
			}
			if (m_overlay[9] != NULL)
			{
				dst.x = g_view->GetWidth() * 0.5f - 209.0f; // 983 d06ec8 (169 era do 645)
				dst.y = 143.0f;
				dst.w = 256.0f;
				dst.h = 64.0f;
				m_overlay[9]->Render(g_view, src, dst, 0, alphaB | 0xffffff, 0,
					0);
			}
			sprintf(Buffer(), "%I64d", *(__int64*)GDOCPTR983(GOLFDOC(), 0x1b8));
			float textWidth = GetTextWidth(Buffer(), 1.0f, m_slide[2].scale);
			if (1 < (unsigned int)GolfInt(GDOCOFS983(0x1c0)))
				textWidth = textWidth + 32.0f;
			if (m_overlay[10] != NULL)
			{
				dst.x = m_slide[2].scale * 118.0f + m_slide[2].x + textWidth;
				dst.y = 143.0f;
				dst.w = 256.0f;
				dst.h = 64.0f;
				m_overlay[10]->Render(g_view, src, dst, 0, alphaB | 0xffffff, 0,
					0);
			}
			if (*GDOCPTR983(GOLFDOC(), 0x194) == 0 &&
				GOLFDOC()->m_currentHole == Doc()->m_holeOrder[0])
			{
				for (int i = 0; i < 5; ++i)
				{
					WOverlay* overlay;
					if (i < (int)((unsigned int)DocInt(DOCOFS983(0x9a0)) % 5 +
								(3 < m_emote ? 1 : 0)))
						overlay = m_overlay[5];
					else
						overlay = m_overlay[6];
					if (overlay != NULL)
					{
						dst.x = (float)i * 50.0f +
							(g_view->GetWidth() - 250.0f) * 0.5f;
						dst.y = 250.0f;
						dst.w = 64.0f;
						dst.h = 64.0f;
						overlay->Render(g_view, src, dst, 0, alphaB | 0xffffff,
							0, 0);
					}
				}
				if (m_emote < 0 || 7 < m_emote)
				{
					if (m_emote == -1 && DocInt(DOCOFS983(0x9a0)) == 5 &&
						m_overlay[7] != NULL)
					{
						dst.x = (g_view->GetWidth() - 250.0f) * 0.5f + 55.0f;
						dst.y = 265.0f;
						dst.w = 256.0f;
						dst.h = 32.0f;
						m_overlay[7]->Render(g_view, src, dst, 0, 0xffffffff, 0,
							0);
					}
				}
				else if (m_ui[4] != NULL)
				{
					unsigned int slot = (unsigned int)DocInt(DOCOFS983(0x9a0)) % 5;
					m_ui[4]->Render(MakeStr("%d", m_emote),
						((g_view->GetWidth() - 250.0f) * 0.5f +
							(float)slot * 50.0f) -
							10.0f,
						225.0f, 1.5f, 0);
				}
			}
		}
		else if (Doc()->m_golfGame.gameType == 10 &&
			GOLFDOC()->m_currentHole == Doc()->m_holeOrder[0] &&
			m_overlay[11] != NULL)
		{
			dst.x = g_view->GetWidth() * 0.5f - 128.0f;
			dst.y = 143.0f;
			dst.w = 256.0f;
			dst.h = 128.0f;
			m_overlay[11]->Render(g_view, src, dst, 0, alphaB | 0xffffff, 0, 0); // 983: com alfa
		}
		unsigned char realType = Doc()->m_roomInfo.realGameType;
		// 983 0053be54: sem jogos/recorde nos modos 0xc, 0xe e 0x13 (Practice)
		if (realType != 0xc && realType != 0xe && realType != 0x13 &&
			*GDOCPTR983(GOLFDOC(), 0x194) == 0 &&
			GOLFDOC()->m_currentHole == Doc()->m_holeOrder[0])
		{
			m_flicker = 0;
			if (m_overlay[1] != NULL)
			{
				dst.x = g_view->GetWidth() * 0.5f - 106.0f;
				dst.y = 202.0f;
				dst.w = 256.0f;
				dst.h = 32.0f;
				m_overlay[1]->Render(g_view, src, dst, 0, alphaB | 0xffffff, 0,
					0);
			}
			int playerIndex =
				IsMassGame(Doc()->m_golfGame.gameType) == 0 ? MyIndex() : 0;
			sprintf(Buffer(), "%d", DocInt(playerIndex * 0x31c6 + DOCOFS983(0x3a94)));
			Print(g_view->GetWidth() * 0.5f + 100.0f, 202.0f, Buffer(), 3.0f, 0,
				1.0f, NULL);
			if (Doc()->m_golfGame.holes == 0x12 && Doc()->m_roomInfo.realGameType != 0x12 && // 983 0053bfd8: Practice (0x12; o 645 usava 0xe) sem recorde
				(Doc()->m_golfGame.gameType == 0 ||
					IsMassGame(Doc()->m_golfGame.gameType) != 0))
			{
				if (m_overlay[2] != NULL)
				{
					dst.x = g_view->GetWidth() * 0.5f - 106.0f;
					dst.y = 242.0f;
					dst.w = 256.0f;
					dst.h = 72.0f;
					m_overlay[2]->Render(g_view, src, dst, 0, alphaB | 0xffffff,
						0, 0);
				}
				char best;
				if (S5::CLASSICSRV::IsClassicGame983() == 0)
					best = (char)DocByte(GetCurMap() * 0x2b + DOCOFS983(0xab7));
				else
					best = (char)DocByte(GetCurMap() * 0x2b + DOCOFS983(0x121b));
				const char* text =
					best == 0x7f ? "-" : MakeStr("%d", (int)best);
				Print(g_view->GetWidth() * 0.5f + 100.0f, 242.0f, text, 3.0f, 0,
					1.0f, NULL);
			}
			// 983 0053c14a: mapa com evento (registro de CSharedDoc+0x1146c, dword 0 == 1) mostra text_mapbonus
			std::map<int, int>::iterator itEvent =
				Doc()->m_mapEvent983.find(GetCurMap());
			const int* pEvent = itEvent != Doc()->m_mapEvent983.end() ?
				(const int*)itEvent->second : NULL;
			if (pEvent != NULL && *pEvent == 1 && m_overlay[8] != NULL)
			{
				dst.x = g_view->GetWidth() * 0.5f - 88.0f;
				dst.y = 160.0f;
				dst.w = 215.0f;
				dst.h = 43.0f;
				m_overlay[8]->Render(g_view, src, dst, 0, alphaB | 0xffffff, 0,
					0);
			}
		}
	}
	CShuffleBonus::Instance()->Render(m_overlay[0], m_alpha[1]);
	if (*GDOCPTR983(GOLFDOC(), 0x194) == 0 &&
		GOLFDOC()->m_currentHole == Doc()->m_holeOrder[0])
		return;
	if (Doc()->m_golfGame.gameType == 4)
		return;
	if ((DocInt(DOCOFS983(0x812)) >> 1 & 1) != 0)
		return;
	if (m_ui[2] != NULL)
		m_ui[2]->SetColor(alphaB | 0xffffff);
	float vsX = g_view->GetWidth() * 0.5f + VsOffset() - 32.0f;
	switch (Doc()->m_golfGame.gameType)
	{
	case 0:
		if (m_ui[2] != NULL)
			m_ui[2]->Render("---", g_view->GetWidth() * 0.5f - 45.0f, 160.0f,
				1.0f, 0);
		if (m_ui[3] == NULL)
			return;
		m_ui[3]->SetColor(alphaB | PLAYER(m_player)->color);
		m_ui[3]->Render(MakeStr("%d", m_player + 1),
			g_view->GetWidth() * 0.5f + 35.0f, 164.0f, 1.0f, 0);
		return;
	case 1:
		if (m_ui[2] != NULL)
		{
			m_ui[2]->Render("vs", vsX, 161.0f, 1.0f, 0);
			m_ui[3]->SetColor(alphaB | 0xffffff);
			m_ui[3]->Render("red", vsX - 50.0f, 168.0f, 1.0f, 0);
			m_ui[3]->Render("blue", g_view->GetWidth() * 0.5f + 68.0f, 168.0f,
				1.0f, 0);
		}
		sprintf(Buffer(), "%d", TEAM(0)->score);
		Print(vsX - VsOffset(), 164.0f, Buffer(), 3.0f, 0, 1.0f, NULL);
		sprintf(Buffer(), "%d", TEAM(1)->score);
		Print((g_view->GetWidth() * 0.5f + 47.0f) - 5.0f, 164.0f, Buffer(),
			3.0f, 0, 1.0f, NULL);
		return;
	// 983 (0053aec0): o switch final so tem os casos 0, 1 e 5; modo 3 nao desenha placar de versus
	case 5:
	{
		if (m_state == 3)
			return;
		SetColor(alphaB | 0xffffff);
		if (m_ui[2] != NULL)
			m_ui[2]->Render("vs", vsX, 161.0f, 1.0f, 0);
		if (m_ui[3] != NULL)
			m_ui[3]->SetColor(alphaB | 0xffffff);
		float redWidth =
			m_font.pFont->GetTextWidthOld983(g_view, MakeStr("%d", DocTeamScore(0)));
		if (m_ui[3] != NULL)
			m_ui[3]->Render("red", ((vsX - 45.0f) - redWidth), 168.0f, 1.0f, 0);
		sprintf(Buffer(), "%d", DocTeamScore(0));
		Print((vsX - 5.0f) - redWidth, 164.0f, Buffer(), 3.0f, 0, 1.0f, NULL);
		float blueWidth =
			m_font.pFont->GetTextWidthOld983(g_view, MakeStr("%d", DocTeamScore(1)));
		if (m_ui[3] != NULL)
			m_ui[3]->Render("blue",
				(float)((double)g_view->GetWidth() * 0.5 + blueWidth + 57.0),
				168.0f, 1.0f, 0);
		sprintf(Buffer(), "%d", DocTeamScore(1));
		Print((g_view->GetWidth() * 0.5f + 47.0f) - 5.0f, 164.0f, Buffer(),
			3.0f, 0, 1.0f, NULL);
	}
		return;
	default:
		return;
	}
}


void CHoleInfo::ResetUiPosition()
{
	m_slide[0].x = m_slide[0].endX = -14.0f;
	m_slide[0].y = m_slide[0].endY = 8.0f;
	m_slide[0].startY = 97.0f;
	m_slide[0].startX = g_view->GetWidth() * 0.5f - 116.0f;
	m_slide[1].x = m_slide[1].endX = 100.0f;
	m_slide[1].startY = 127.0f;
	m_slide[1].y = m_slide[1].endY = 6.0f;
	m_slide[1].startX = g_view->GetWidth() * 0.5f - 75.0f;
}


void CHoleInfo::Process(float dt)
{
	switch (m_state)
	{
	case 0:
	{
		m_time += dt;
		for (int i = 0; i < 2; i++)
		{
			m_alpha[i] += dt * 255.0f;
			if (m_alpha[i] > 255.0f)
			{
				m_alpha[i] = 255.0f;
			}
		}
		unsigned char type = Doc()->m_golfGame.gameType;
		float limit;
		switch (type)
		{
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
		case 11: // 983 0052a7e0: tiki (0xb) tambem
			limit = 1.0f;
			if (GOLFDOC()->m_currentHole != Doc()->m_holeOrder[0])
			{
				break;
			}
		default:
			limit = 4.0f;
		}
		if (Doc()->m_golfGame.gameType == 10)
		{
			limit = 2.0f;
		}
		if (Doc()->m_golfGame.gameType == 7 &&
			*GDOCPTR983(GOLFDOC(), 0x194) == 0 &&
			GOLFDOC()->m_currentHole == Doc()->m_holeOrder[0])
		{
			if (m_emote == -1)
			{
				if (m_time > 1.0f)
				{
					m_emote = 0;
					g_audio->PlaySfx("play_fail_item", NULL, 0, NULL, NULL,
						0.5f, 200.0f);
				}
			}
			else if (m_emote < 8)
			{
				m_emote = (int)((m_time - 1.0f) * s_barTextY);
			}
		}
// 983 0052a7e0: estado 0 nao anima o slide 3 (so o estado 1)
		if (m_time > limit)
		{
			m_time = 0.0f;
			m_state = 1;
			if (Doc()->m_golfGame.gameType == 10)
			{
				float x = m_slide[1].endX + 60.0f;
				m_slide[3].startX = m_slide[3].endX;
				m_slide[3].endX = x;
			}
		}
		break;
	}
	case 2:
		m_alpha[0] -= dt * 128.0f;
		if (m_alpha[0] < 178.5f)
		{
			m_alpha[0] = 178.5f;
			// 983: entrada em start_hole (golfdoc+0x195) no buraco atual = texto "Letter" 0x1fe + Screen 0x216
			if (*GDOCPTR983(GOLFDOC(), 0x195) != 0 &&
				GOLFDOC()->m_currentHole == Doc()->m_holeOrder[0])
			{
				float duration = 2.0f;
				MsgObject letter(NULL, 0x1fe, 0, 0, 0, 0, 0);
				letter.param1 = (int)"\xc2\xf7\xb7\xca\xc7\xa5\xbd\xc3";
				letter.param2 = (int)&duration;
				AfxGetTask()->GetActor("Letter") << letter;
				AfxGetTask()->GetActor("Screen") << MsgObject(NULL, M983(0x1a4), 1, 0, 0, 0, 0);
			}
			m_state = 3;
		}
		break;
	case 1:
		m_alpha[1] -= dt * 510.0f;
		if (m_alpha[1] < 0.0f)
		{
			m_alpha[1] = 0.0f;
		}
		m_time += dt * 1.5f;
		if (m_time > 1.0f)
		{
			m_state = 2;
			m_time = 1.0f;
		}
		{
			sSlide* sl = m_slide;
			sl[0].x = sl[0].endX * m_time + (1.0f - m_time) * sl[0].startX;
			sl[0].y = sl[0].endY * m_time + (1.0f - m_time) * sl[0].startY;
			sl[0].scale = m_time * 0.5f + (1.0f - m_time);
			sl[1].x = sl[1].endX * m_time + (1.0f - m_time) * sl[1].startX;
			sl[1].y = sl[1].endY * m_time + (1.0f - m_time) * sl[1].startY;
			sl[1].scale = m_time * 0.6f + (1.0f - m_time);
			sl[2].x = sl[2].endX * m_time + (1.0f - m_time) * sl[2].startX;
			sl[2].y = sl[2].endY * m_time + (1.0f - m_time) * sl[2].startY;
			sl[2].scale = m_time * 0.6f + (1.0f - m_time);
			sl[3].x = sl[3].endX * m_time + (1.0f - m_time) * sl[3].startX;
			sl[3].y = sl[3].endY * m_time + (1.0f - m_time) * sl[3].startY;
			sl[3].scale = m_time * 0.6f + (1.0f - m_time);
		}
		break;
	case 4:
		m_time += dt;
		if (m_time > 0.25f)
		{
			m_time -= 0.25f;
			m_alpha[0] = 255.0f - m_alpha[0];
			if (m_alpha[0] > 0.0f && ++m_flicker == 3)
			{
				m_state = 3;
			}
		}
		break;
	}
}

// 983 (00533d80): linha base do Pang = medida "d_panginfo_base_y" (Calc na escala W/640, H/480);
// sem a medida, gGetBar_Y() - 108
static float PangInfoBaseY983()
{
	CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0), (float)(g_view->GetHeight() / 480.0));
	CUiFrameSetManager::NamedDim* d = CUiFrameSetManager::Instance()->FindNamedDim("d_panginfo_base_y");
	if (d != NULL)
		return (float)floor((double)d->Calc(scl));
	return COption::Instance()->gGetBar_Y() - 108.0f;
}

void CPangInfo::ProcessCases(float dt)
{
	if (m_flags & 0x10)
		return;

	if (m_flags & 8)
	{
		if (m_case[1].flags & 0x1000)
			m_case[1].field1C = m_case[1].field1C - dt * 96.0f;
		if (m_case[2].flags & 0x1000)
			m_case[2].field1C = m_case[2].field1C - dt * 96.0f;
		if (m_case[3].flags & 0x1000)
			m_case[3].field1C = m_case[3].field1C - dt * 96.0f;
		if (m_case[4].flags & 0x1000)
			m_case[4].field1C = m_case[4].field1C - dt * 96.0f;
		if (m_case[5].flags & 0x1000)
			m_case[5].field1C = m_case[5].field1C - dt * 96.0f;
		if (m_case[6].flags & 0x1000)
			m_case[6].field1C = m_case[6].field1C - dt * 96.0f;
		if (m_case[7].flags & 0x1000)
			m_case[7].field1C = m_case[7].field1C - dt * 96.0f;
		if (m_case[8].flags & 0x1000)
			m_case[8].field1C = m_case[8].field1C - dt * 96.0f;
		if (m_case[9].flags & 0x1000)
			m_case[9].field1C = m_case[9].field1C - dt * 96.0f;
		if (m_case[10].flags & 0x1000)
			m_case[10].field1C = m_case[10].field1C - dt * 96.0f;
		if (m_case[18].flags & 0x1000)
			m_case[18].field1C = m_case[18].field1C - dt * 96.0f;
		if (m_case[17].flags & 0x1000)
			m_case[17].field1C = m_case[17].field1C - dt * 96.0f;
		if (m_case[15].flags & 0x1000)
			m_case[15].field1C = m_case[15].field1C - dt * 96.0f;
		if (m_case[19].flags & 0x1000)
			m_case[19].field1C = m_case[19].field1C - dt * 96.0f;
		if (m_case[20].flags & 0x1000)
			m_case[20].field1C = m_case[20].field1C - dt * 96.0f;
		if (IsLocalContent((localContentType_t)3) &&
			(m_case[21].flags & 0x1000))
			m_case[21].field1C = m_case[21].field1C - dt * 96.0f;
		if (m_case[22].flags & 0x1000)
			m_case[22].field1C = m_case[22].field1C - dt * 96.0f;
	}

	for (int i = 0; i < 25; ++i)
	{
		unsigned int state = m_case[i].flags;
		if (state & 0x1000)
		{
			float y = m_case[i].field18 - dt * 40.0f;
			m_case[i].field18 = y;
			if (y <= m_case[i].field1C)
				m_case[i].flags = state & 0xffffefff;
			continue;
		}
		switch (state & 0xfff)
		{
		case 2:
			if (m_case[i].field14 > 0.5f)
			{
				m_case[i].flags = 4;
				m_case[i].field14 = 0.0f;
			}
			break;
		case 4:
			if (m_case[i].field14 > 0.5f)
			{
				m_case[i].flags = 8;
				m_case[i].field14 = 0.0f;
			}
			break;
		case 0x10:
		{
			if (m_sound == NULL)
			{
				if (m_gain[0] > 0)
					m_sound = "pang_up";
				else
					m_sound = "pang_down";
				if (m_sound != NULL && !g_audio->IsPlaying(m_sound))
					g_audio->PlaySfx(m_sound, NULL, 1, m_sound, NULL, 0.5f,
						200.0f);
			}
			float y = dt * 96.0f + m_case[i].field18;
			float limit = PangInfoBaseY983();
			float top = limit + 20.0f;
			m_case[i].field18 = y;
			if (y > top)
			{
				if (!m_applied[i])
				{
				switch (i)
				{
				case 1:
					m_shown[0] += (*(__int64*)&m_case[i].field8 - 1) *
						*(__int64*)&m_case[0].field8;
					m_slot[0].total = m_shown[0];
					break;
				case 9:
				{
					m_shown[0] += *(__int64*)&m_case[9].field8;
					int low = (int)m_shown[0];
					m_shown[0] = low < 0 ? 0 : low;
					m_slot[0].total = m_shown[0];
				}
				break;
				case 10:
				case 11:
				case 12:
					m_shown[0] += (*(__int64*)&m_case[i].field8 - 1) *
						*(__int64*)&m_case[9].field8;
					m_slot[0].total = m_shown[0];
					break;
				case 15:
					if (m_shownTotal[0] > 0)
					{
						m_shown[0] += m_shownTotal[0];
						m_slot[0].total = m_shown[0];
					}
					if (m_shownTotal[1] > 0)
					{
						m_shown[1] += m_shownTotal[1];
						m_slot[1].total = m_shown[1];
					}
					break;
				case 20:
					m_shown[1] += *(__int64*)&m_case[i].field8;
					m_slot[1].total = m_shown[1];
					break;
				case 21:
					m_shown[1] += *(__int64*)&m_case[i].field8;
					m_slot[1].total = m_shown[1];
					break;
				case 4:
				case 13:
				case 14:
				case 17:
				case 18:
				case 22:
					m_shown[1] += *(__int64*)&m_case[i].field8;
					m_slot[1].total = m_shown[1];
					break;
				case 23:
					m_shown[2] += *(__int64*)&m_case[i].field8;
					m_slot[2].total = m_shown[2];
					break;
				case 24:
					m_shown[3] += *(__int64*)&m_case[i].field8;
					m_slot[3].total = m_shown[3];
					break;
				default:
					m_shown[0] += *(__int64*)&m_case[i].field8;
					m_slot[0].total = m_shown[0];
					break;
				}
				m_case[i].flags = 0x20;
				m_case[i].flag34 = 0;
				m_applied[i] = 1;
				}
				switch (i)
				{
				case 23:
					m_slot[2].state = 2;
					m_slot[2].field14 = 0.0f;
					break;
				case 24:
					m_slot[3].state = 2;
					m_slot[3].field14 = 0.0f;
					break;
				default:
					for (int k = 0; k < 2; ++k)
					{
						m_slot[k].state = 2;
						m_slot[k].field14 = 0.0f;
					}
					break;
				}
				break;
			}
			if (y <= limit)
				break;
			float f = (y - limit) * 0.05f;
			if (f > s_minDistance)
				f = s_minDistance;
			float a = (top - y) * 12.75f;
			if (a < 0.0f)
				a = 0.0f;
			else if (a > 255.0f)
				a = 255.0f;
			m_case[i].flag34 = (unsigned char)(int)a;
			switch (i)
			{
			case 1:
				m_slot[0].total =
					float2int(((float)(*(__int64*)&m_case[i].field8 - 1) * f) *
						(float)*(__int64*)&m_case[0].field8) +
					m_shown[0];
				break;
			case 10:
			case 11:
			case 12:
				m_slot[0].total =
					float2int(((float)(*(__int64*)&m_case[i].field8 - 1) * f) *
						(float)*(__int64*)&m_case[9].field8) +
					m_shown[0];
				break;
			case 9:
			{
				int amount = float2int(f * (float)*(__int64*)&m_case[9].field8);
				m_slot[0].total = amount + m_shown[0];
				int low = (int)m_slot[0].total;
				m_slot[0].total = low < 0 ? 0 : low;
				break;
			}
			case 15:
			{
				float t = (float)(*(__int64*)&m_case[i].field8 - 1) * f;
				m_slot[0].total =
					float2int((float)m_shownTotal[0] * t) + m_shown[0];
				m_slot[1].total =
					float2int((float)m_shownTotal[1] * t) + m_shown[1];
				break;
			}
			case 4:
			case 13:
			case 14:
			case 17:
			case 18:
			case 20:
			case 21:
			case 22:
				m_slot[1].total =
					float2int((float)*(__int64*)&m_case[i].field8 * f) +
					m_shown[1];
				break;
			case 23:
				m_slot[2].total =
					float2int((float)*(__int64*)&m_case[i].field8 * f) +
					m_shown[2];
				break;
			case 24:
				m_slot[3].total =
					float2int((float)*(__int64*)&m_case[i].field8 * f) +
					m_shown[3];
				break;
			default:
				m_slot[0].total =
					float2int((float)*(__int64*)&m_case[i].field8 * f) +
					m_shown[0];
				break;
			}
			break;
			break;
		}
		case 0x20:
			m_case[i].flags = 0x2000;
			if (m_sound != NULL)
				g_audio->StopSfx(m_sound);
			break;
		case 0x40:
			m_case[i].field18 =
				(PangInfoBaseY983() - (float)m_case[i].field10 * 20.0f) -
				m_case[i].field14 * 160.0f;
			{
				float a = (0.5f - m_case[i].field14) * 255.0f;
				if (a < 0.0f)
					a = 0.0f;
				m_case[i].flag34 = (unsigned char)(int)a;
			}
			if (m_case[i].field14 > 0.5f)
				m_case[i].flags = 0x80;
			break;
		}
	}
}


static void InitPangCase(CPangInfo::sPangCase& c, const PangOverlayInfo* e,
	float half, float sixteenth)
{
	c.field20 = e->id;
	float x = (float)e->x * 0.5f;
	float y = (float)e->y * 0.0625f;
	c.field24 = x;
	c.field28 = y;
	c.field2C = half;
	c.field30 = sixteenth;
}

CPangInfo::CPangInfo()
{
	Init("[bigfont.jpg");
	m_align = 2;
	Buffer() = new char[32];
	WOverlay** overlay = m_overlay;
	for (unsigned int i = 0; i < 2; ++i)
	{
		*overlay++ = g_resrcmng->GetOverlay(g_pangOverlayFilenames[i], 0);
	}
	float half = 0.5f;
	float sixteenth = 0.0625f;
	const PangOverlayInfo* misc = &g_pangOverlayInfo[0];
	for (int i = 0; i < 25; ++i)
		InitPangCase(m_case[i], misc + i, half, sixteenth);
	m_f78c = 0.5f;
	m_f790 = 0.8125f;
	m_f794 = 0.5f;
	m_f798 = 0.0625f;
	m_f788 = 0;
	const PangOverlayInfo* tail = &g_totalPangOverlayInfo[0];
	for (int i = 0; i < 4; ++i)
	{
		m_gain[i] = 0;
		m_pang[i] = 0;
		m_shown[i] = 0;
		m_shownTotal[i] = 0;
		m_slot[i].total = 0;
		m_slot[i].field10 = -1;
		m_slot[i].overlayIndex = tail->id;
		float x = (float)tail->x * 0.5f;
		float y = (float)tail->y * 0.0625f;
		m_slot[i].rect24.x = x;
		m_slot[i].rect24.y = y;
		m_slot[i].rect24.w = half;
		m_slot[i].rect24.h = sixteenth;
		m_slot[i].alpha34 = 0xff;
		++tail;
	}
	m_flags = 0;
	m_sound = NULL;
	m_bonusTime[0] = 0.0f;
	m_bonusTime[1] = 0.0f;
	m_bonusTime[2] = 0.0f;
	m_bonusTime[3] = 0.0f;
	m_f750 = 0.0f;
	ResetUiPosition();
	m_f758 = 0;
	m_extraPang[0] = 0;
	m_extraPang[1] = 0;
	m_extraPang[2] = 0;
	m_extraPang[3] = 0;
	if (IsLocalContent((localContentType_t)0x33 /*983: era 0x32; interpolado*/))
	{
		m_caddieBonus.SetTicket();
	}
	// 983 CPangInfo::CPangInfo 005384c0: sem a tabela de 240 ints do 645 (m_valueTable, sem leitor no 983 nem no nosso
	// desde o B1257: GetHoleInPang le o registro do campo em FindCourseBonus983); no fim, o container tiki em this+0x7fc
	C_ASSERT_VC10(offsetof(CPangInfo, m_pTikiData983) == 0x7fc && offsetof(CPangInfo, m_applied) == 0x800);
	C_ASSERT_VC10(sizeof(CPangInfo) == 0x820);
	m_pTikiData983 = NULL;
	if (IsGameType(0xb))
		m_pTikiData983 =
			CContentsDoc::Instance()->GetContainer((localContentType_t)0xaa);
}

CPangInfo::~CPangInfo()
{
	for (int i = 0; i < 2; ++i)
	{
		if (g_resrcmng != NULL && m_overlay[i] != NULL)
		{
			g_resrcmng->Release(m_overlay[i]);
			m_overlay[i] = NULL;
		}
	}
}

void CPangInfo::ResetUiPosition()
{
	float y = PangInfoBaseY983() + 23.0f;
	m_slot[0].field18 = y;
	m_slot[1].field18 = y;
	m_slot[2].field18 = y;
	m_slot[3].field18 = y;
	float bottom;
	float top;
	unsigned char type = GameType();
	if (type == 0 || type == 3 || type == 7)
	{
		top = 48.0f;
		bottom = 141.0f;
	}
	else
	{
		if (type != 1)
			return;
		top = 51.0f;
		bottom = 100.0f;
	}
	float right = g_view->GetWidth() - 186.0f;
	m_uiRect[1] = top;
	m_uiRect[0] = -25.0f;
	m_uiRect[2] = -25.0f;
	m_uiRect[3] = bottom;
	m_uiRect[4] = right;
	m_uiRect[5] = top;
	m_uiRect[6] = right;
	m_uiRect[7] = bottom;
	m_uiRect[8] = -25.0f;
	m_uiRect[9] = 68.0f;
	m_uiRect[10] = -25.0f;
	m_uiRect[11] = 161.0f;
	m_uiRect[12] = right;
	m_uiRect[13] = 68.0f;
	m_uiRect[14] = right;
	m_uiRect[15] = 161.0f;
}


void CPangInfo::ChangeTurn()
{
	if (m_sound != NULL)
	{
		g_audio->StopSfx(m_sound);
	}
	if (GameType() == 7 && *((unsigned char*)GDOCPTR983(GOLFDOC(), 0x194)) != 0)
	{
		__int64 total = (int)PLAYER(GOLFDOC()->m_currentPlayer)->totalPang;
		m_shown[0] = total;
		m_slot[0].total = total;
		m_pang[0] = total;
		__int64 bonus = (int)PLAYER(GOLFDOC()->m_currentPlayer)->bonusPang;
		bonus = bonus + m_extraPang[GOLFDOC()->m_currentPlayer];
		m_slot[1].total = bonus;
		m_shown[1] = bonus;
		m_pang[1] = bonus;
	}
	else
	{
		__int64 total = (int)PLAYER(GOLFDOC()->m_currentPlayer)->totalPang;
		m_slot[0].total = total;
		m_shown[0] = total;
		m_pang[0] = total;
		__int64 bonus = (int)PLAYER(GOLFDOC()->m_currentPlayer)->bonusPang;
		m_slot[1].total = bonus;
		m_shown[1] = bonus;
		m_pang[1] = bonus;
	}
	char* table = GroundItemTable();
	if (table != NULL)
	{
		__int64 value =
			((GroundItemMan::PlayerStats*)table)[GOLFDOC()->m_currentPlayer].numAcquiredLive[0];
		m_slot[2].total = value;
		m_shown[2] = value;
		m_pang[2] = value;
		value = ((GroundItemMan::PlayerStats*)table)[GOLFDOC()->m_currentPlayer].numAcquiredLive[1];
		m_slot[3].total = value;
		m_shown[3] = value;
		m_pang[3] = value;
		ResetVars(0, 1);
		return;
	}
	m_slot[2].total = 0;
	m_shown[2] = 0;
	m_pang[2] = 0;
	m_slot[3].total = 0;
	m_shown[3] = 0;
	m_pang[3] = 0;
	ResetVars(0, 1);
}

void CPangInfo::Process(float dt)
{
	// 983: bloco `if (0)` de ordem de emissão do 645 retirado (classes só declaradas no ramo de bytes)
	unsigned char type = GameType();
	if (type != 10)
	{
		int tutorial = GOLFDOC()->m_tutorialMode;
		if ((tutorial >= 15 || tutorial == 8) &&
			(GOLFDOC()->m_gameMode & 0x81880) != 0 && m_flags != 0 &&
			// 983 (0053df4f): com o modo 0x1000 so os tipos 1 e 7 (o 3 era do 645)
			((GOLFDOC()->m_gameMode & 0x1000) == 0 || type == 1 || type == 7))
		{
			UpdateBallState();
			ProcessTotal(dt);
			ProcessCases(dt);
			CheckReadyToMakeDecision();
			CheckReadyToFinalize(dt);
			if ((m_flags & 8) == 0)
			{
				RecoveryBonus();
				SuperPangya();
				LongDrive();
				Multiply(1, 2);
				Approach();
				LongPutt();
				LongChipIn();
				BeamImpact();
				SpecialShot();
				NiceRecovery();
				ClashCombo();
				HoleIn();
				Multiply(10, 2);
				Multiply(11, 2);
				DormieBonus();
				HappyBonus();
				AztecBonus();
				AztecBonusWater();
				AngelWingBonus();
				DevilWingBonus();
				if (IsLocalContent((localContentType_t)3))
				{
					MascotBonus();
				}
				AbilityItemBonus();
				GimmickItem((GIMMICK_ITEM_TYPE)0);
				GimmickItem((GIMMICK_ITEM_TYPE)1);
				BonusCase4();
				ClearBonus();
			}
			else
			{
				ProcessHappyBonus(dt);
				ProcessAztecBonus(dt);
				ProcessAngelBonus(dt);
				ProcessDevilBonus(dt);
			}
			for (int i = 0; i < 25; i++)
			{
				if ((m_case[i].flags & 0x46) != 0)
				{
					m_case[i].field14 = dt + m_case[i].field14;
				}
			}
			m_f6f4 = GolfBall().m_playFrame;
		}
	}
}

static bool IsAngelWingItem(unsigned long id)
{
	switch (id)
	{
	case 0x801680f:
	case 0x805880f:
	case 0x809880f:
	case 0x80dc80f:
	case 0x811880f:
	case 0x816080f:
	case 0x819080f:
	case 0x81e280f:
	case 0x821480f:
	case 0x825480f:
	case 0x8294811:
	case 0x82d4804:
	case 0x831480c:
	case 0x839c80c:
		return true;
	}
	return false;
}

static bool IsDevilWingItem(unsigned long id)
{
	switch (id)
	{
	case 0x8016810:
	case 0x8058810:
	case 0x8098810:
	case 0x80dc810:
	case 0x8118810:
	case 0x8160810:
	case 0x8190810:
	case 0x81e2810:
	case 0x8214810:
	case 0x8254810:
	case 0x8294812:
	case 0x82d4803:
	case 0x831480d:
	case 0x839c80d:
		return true;
	}
	return false;
}

static bool HaveWingItem(int index, bool devil)
{
	for (int i = 0; i < 0x60; i += 4)
	{
		unsigned long id = *(unsigned long*)((unsigned char*)Doc() +
			index * 0x31c6 + DOCOFS983(0x680e) + i);
		if (devil ? IsDevilWingItem(id) : IsAngelWingItem(id))
			return true;
	}
	return false;
}

CPangInfo::sPangCase::sPangCase()
{
}

int __fastcall GetAngelBonus(sPlayerData* player)
{
	unsigned char level = player->level;
	if (level < 6)
	{
		return 7;
	}
	if (level > 5 && level < 11)
	{
		return 5;
	}
	if (level > 10 && level < 21)
	{
		return 3;
	}
	return level > 20;
}

LOCAL_INLINE unsigned char DocByte(int offset)
{
	return *((unsigned char*)Doc() + offset);
}

// forceinline como o DocByte antigo: o byte e lido antes do GetCurMap() (obj VC10 igual)
LOCAL_INLINE unsigned char HoleOrder(int i)
{
	return Doc()->m_holeOrder[i];
}

int ClearBonusRate983(unsigned char map); // golfrule.cpp, 983 mapa 00e78e60; s_clearBonusTable do 645: mapa 18 = 0, sem 20/21 (lia fora)
int __fastcall GetGameClearBonus()
{
	// 983 (0052aff0): so 0, 1 e 4/5/6 pagam; o ramo de gameType 3 e os demais tipos no ramo geral eram do 645
	unsigned char gameType = Doc()->m_golfGame.gameType;
	if (gameType == 4 || gameType == 5 || gameType == 6)
	{
		// 983 (0052afc0): metade da taxa por buraco, sem classico e sem contar jogadores
		unsigned char holeCount = Doc()->m_golfGame.holes;
		return (int)((unsigned int)(ClearBonusRate983(GetCurMap()) * holeCount) >> 1);
	}
	if (gameType != 0 && gameType != 1)
	{
		return 0;
	}
	int bonus = 0;
	if (Doc()->m_golfGame.gameType == 1)
	{
		if (S5::CLASSICSRV::IsClassicGame983())
		{
			for (int i = 0; i < Doc()->m_golfGame.holes; ++i)
			{
				if (GOLFDOC()->m_pHoleData[Doc()->m_holeOrder[i] - 1].playerNum ==
					4)
				{
					bonus += S5::CLASSICSRV::GetClearBonus(4, GetCurMap());
				}
			}
			return bonus;
		}
		CGolfDoc* golf = GOLFDOC();
		int count = 0;
		unsigned char holes = GetHoleIndex(golf->m_currentHole);
		for (int i = 0; i < holes; ++i)
		{
			if (golf->m_pHoleData[Doc()->m_holeOrder[i] - 1].playerNum <= 4)
			{
				++count;
			}
		}
		return ClearBonusRate983(GetCurMap()) * count;
	}
	if (S5::CLASSICSRV::IsClassicGame983())
	{
		for (int i = 0; i < Doc()->m_golfGame.holes; ++i)
		{
			bonus += S5::CLASSICSRV::GetClearBonus(
				GOLFDOC()->m_pHoleData[HoleOrder(i) - 1].playerNum,
				GetCurMap());
		}
		return bonus;
	}
	unsigned char two = 0;
	unsigned char three = 0;
	unsigned char four = 0;
	for (int i = 0; i < Doc()->m_golfGame.holes; ++i)
	{
		char num = GOLFDOC()->m_pHoleData[Doc()->m_holeOrder[i] - 1].playerNum;
		if (num == 2)
		{
			++two;
		}
		else if (num == 3)
		{
			++three;
		}
		else if (num == 4)
		{
			++four;
		}
	}
	return (four * 3 + three * 2 + two) * ClearBonusRate983(GetCurMap());
}

bool __fastcall IsEnabledOverDrive(unsigned char type)
{
	return type != 19;
}

LOCAL_INLINE unsigned char& PlayerByte(sPlayerData* player, int offset)
{
	return *((unsigned char*)player + offset);
}

void CPangInfo::Display()
{
	if (0)
		((CPangInfo*)0)->CPangInfo::~CPangInfo();
	if (IsGameType(10))
		return;
	int mode = GOLFDOC()->m_tutorialMode;
	if (mode < 15 && mode != 8)
		return;
	Display_TotalPang();
	Display_TotalGimmickNum();
	__int64 big = Max(m_slot[0].total, m_slot[1].total);
	float digitsWidth;
	if (big > 10000)
		digitsWidth = 105.0f;
	else if (big > 1000)
		digitsWidth = 85.0f;
	else
		digitsWidth = 65.0f;
	unsigned int gameMode = *(unsigned int*)GDOCPTR983(GOLFDOC(), 0x158);
	if ((gameMode & 0x81880) == 0 || m_flags == 0)
		return;
	if ((gameMode & 0x1000) != 0 && !IsGameType(1) && !IsGameType(7))
		return;
	_WRECT dst;
	for (int i = 24; i >= 0; --i)
	{
		if (i == 21 && // 983 slot 21 (983 tem 1 case a mais no indice 8)
			*(unsigned long*)((unsigned char*)Doc() +
				GOLFDOC()->m_currentPlayer * 0x31c6 + DOCOFS983(0x6a3c)) == 0x40000006 &&
			GolfBall().IsHoleIn())
		{
			if (GolfBall().m_holeFrame > m_f6f4)
				continue;
			m_case[21].flag34 = ((int)m_case[21].flags > 0x1f) - 1;
		}
		if ((m_case[i].flags & 0xfffffe5e) == 0)
			continue;
		float scale;
		switch (m_case[i].flags & 0xfff)
		{
		case 2:
		{
			float t = m_case[i].field14 * 0.8f + 0.6f;
			scale = t > 1.0f ? 1.0f : t;
		}
			m_case[i].flag34 =
				(unsigned char)(int)Min(m_case[i].field14 * 510.0f, 255.0f);
			break;
		case 4:
			scale = Max((0.5f - m_case[i].field14) * 0.8f + 0.6f, 0.6f);
			break;
		default:
			scale = 0.6f;
			break;
		}
		if (m_case[i].flag34 == 0)
			continue;
		dst.w = scale * 256.0f;
		dst.h = scale * 32.0f;
		dst.x = (float)floor((double)(((g_view->GetWidth() - 10.0f) - dst.w) -
			scale * digitsWidth));
		dst.y = m_case[i].field18 - dst.h * 0.7f;
		unsigned long color =
			((unsigned long)m_case[i].flag34 << 24) | 0xffffff;
		int overlayIndex;
		_WRECT src;
		if (i == 18 &&
			Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].userEquip.tidBall == 0x14000016)
		{
			overlayIndex = m_f788;
			src = *(_WRECT*)&m_f78c;
		}
		else
		{
			overlayIndex = m_case[i].field20;
			src = *(_WRECT*)&m_case[i].field24;
		}
		if (m_overlay[overlayIndex] != NULL)
			m_overlay[overlayIndex]->Render(g_view, src, dst, 0, color, 0, 0);
		SetColor(color);
		if (i != 1 && i != 10 && i != 11 && i != 12 && i != 15)
		{
			if (i == 19)
			{
				if (*(__int64*)&m_case[19].field8 >= 0)
				{
					sprintf(Buffer(), "%I64d", *(__int64*)&m_case[19].field8);
				}
				else
				{
					sprintf(Buffer(), "%c", '?');
				}
			}
			else
			{
				sprintf(Buffer(), "%I64d", *(__int64*)&m_case[i].field8);
			}
		}
		else
		{
			sprintf(Buffer(), "x%I64d", *(__int64*)&m_case[i].field8);
		}
		Print(g_view->GetWidth() - 10.0f, dst.y + 2.0f, Buffer(), 3.0f, 0,
			scale, NULL);
	}
	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
		break;
	case 5:
		break;
	case 6:
		break;
	case 9:
		break;
	case 10:
	case 0xb:
		break;
	default:
		if ((m_case[17].flags & 0x101) == 0 && (m_flags & 8) != 0)
		{
			float t = 1.0f < m_bonusTime[1] ? 1.0f : m_bonusTime[1];
			dst.w = 153.6f;
			dst.h = 19.2f;
			for (int p = 0; p < GOLFDOC()->m_playerNum; ++p)
			{
				if (GOLFDOC()->GetPlayer(p)->state != 3 &&
					p != GOLFDOC()->m_currentPlayer)
				{
					if (IsGameType(1) &&
						PLAYER(p)->team ==
							PLAYER(GOLFDOC()->m_currentPlayer)->team)
						// 983 0052e9a2: bit 7 do dwIdentity do jogador da vez dobra o 30
						sprintf(Buffer(), "%d",
							(Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].info.dwIdentity & 0x80) ? 60 : 30);
					else
						sprintf(Buffer(), "%d", 5);
					int slot;
					if (IsGameType(1))
					{
						if (PLAYER(p)->team == 0)
							slot = p < (PLAYER(p)->teammate == 0xff);
						else
							slot = p < (PLAYER(p)->teammate == 0xff) + 2;
					}
					else
					{
						slot = p;
					}
					WVector2D pos = (1.0f - t) * *(WVector2D*)&m_f6f8 +
						t * ((WVector2D*)m_uiRect)[slot];
					float x = pos.x;
					float y = pos.y;
					if (m_f754 > 1)
						y -= 10.0f;
					dst.x = x;
					dst.y = y;
					if (m_overlay[m_case[17].field20] != NULL)
						m_overlay[m_case[17].field20]->Render(g_view,
							*(_WRECT*)&m_case[17].field24, dst, 0, 0xffffffff,
							0, 0);
					Print(dst.x + 180.0f, dst.y + 2.0f, Buffer(), 3.0f, 0, 0.6f,
						NULL);
				}
			}
		}
	}
	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 0xb:
		break;
	default:
		if ((m_case[19].flags & 0x101) == 0 && (m_flags & 8) != 0)
		{
			float t = 1.0f < m_bonusTime[2] ? 1.0f : m_bonusTime[2];
			dst.w = 153.6f;
			dst.h = 19.2f;
			for (int p = 0; p < GOLFDOC()->m_playerNum; ++p)
			{
				if (GOLFDOC()->GetPlayer(p)->state != 3 &&
					p != GOLFDOC()->m_currentPlayer)
				{
					// 983 0052ec0a: texto pelas flags (+0x4e0) do jogador da vez; sem nenhuma, o Buffer fica como estava
					unsigned int curFlags =
						*(unsigned int*)((unsigned char*)GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer) + 0x4e0);
					if (curFlags & 0x40000)
						sprintf(Buffer(), "%d", GetAngelBonus(GOLFDOC()->GetPlayer(p)));
					else if (curFlags & 0x2000000)
						sprintf(Buffer(), "%d", 10);
					int slot;
					if (IsGameType(1))
					{
						if (PLAYER(p)->team == 0)
							slot = p < (PLAYER(p)->teammate == 0xff);
						else
							slot = p < (PLAYER(p)->teammate == 0xff) + 2;
					}
					else
					{
						slot = p;
					}
					float x;
					float y;
					if (m_f754 > 1)
					{
						WVector2D pos = (1.0f - t) * *(WVector2D*)&m_f6f8 +
							t * ((WVector2D*)m_uiRect)[slot + 4];
						x = pos.x;
						y = pos.y;
					}
					else
					{
						WVector2D pos = (1.0f - t) * *(WVector2D*)&m_f6f8 +
							t * ((WVector2D*)m_uiRect)[slot];
						x = pos.x;
						y = pos.y;
					}
					if (m_f754 > 1)
						y -= 10.0f;
					dst.x = x;
					dst.y = y;
					if (m_overlay[m_case[19].field20] != NULL)
						m_overlay[m_case[19].field20]->Render(g_view,
							*(_WRECT*)&m_case[19].field24, dst, 0, 0xffffffff,
							0, 0);
					Print(dst.x + 180.0f, dst.y + 2.0f, Buffer(), 3.0f, 0, 0.6f,
						NULL);
				}
			}
		}
	}
	if ((m_case[13].flags & 0x101) == 0 &&
		((m_flags & 8) != 0 ||
			(*((unsigned char*)GDOCPTR983(GOLFDOC(), 0x159)) & 0x10) != 0))
	{
		float t = 1.0f < m_bonusTime[0] ? 1.0f : m_bonusTime[0];
		sprintf(Buffer(), "%d", 20);
		dst.w = 153.6f;
		dst.h = 19.2f;
		for (int p = 0; p < GOLFDOC()->m_playerNum; ++p)
		{
			if (GOLFDOC()->GetPlayer(p)->state != 3 &&
				p != GOLFDOC()->m_currentPlayer)
			{
				int slot;
				if (IsGameType(1))
				{
					if (PLAYER(p)->team == 0)
						slot = p < (PLAYER(p)->teammate == 0xff);
					else
						slot = p < (PLAYER(p)->teammate == 0xff) + 2;
				}
				else
				{
					slot = p;
				}
				WVector2D pos = (1.0f - t) * *(WVector2D*)&m_f6f8 +
					t * ((WVector2D*)m_uiRect)[slot];
				float x = pos.x;
				float y = pos.y;
				if (m_f754 > 1)
					y += 10.0f;
				dst.x = x;
				dst.y = y;
				if (m_overlay[m_case[13].field20] != NULL)
					m_overlay[m_case[13].field20]->Render(g_view,
						*(_WRECT*)&m_case[13].field24, dst, 0, 0xffffffff, 0,
						0);
				Print(dst.x + 180.0f, dst.y + 2.0f, Buffer(), 3.0f, 0, 0.6f,
					NULL);
			}
		}
	}
}

LOCAL_INLINE unsigned char PlayerStroke(unsigned char index)
{
	return PLAYER(index)->stroke[GOLFDOC()->m_currentHole - 1];
}

LOCAL_INLINE unsigned char TeamStroke(unsigned char index)
{
	return TEAM(index)->stroke[GOLFDOC()->m_currentHole - 1];
}

LOCAL_INLINE unsigned char& GolfByte(int offset)
{
	return *((unsigned char*)GOLFDOC() + offset);
}

LOCAL_INLINE sPlayerData* GolfDirectPlayer()
{
	unsigned char index = GOLFDOC()->m_currentPlayer;
	return GOLFDOC()->GetPlayer(index);
}

LOCAL_INLINE sTeamData* GolfDirectTeam()
{
	unsigned char index = GOLFDOC()->m_currentPlayer;
	unsigned char team = ((unsigned char*)GOLFDOC()->GetPlayer(index))[0x4e9];
	return GOLFDOC()->GetTeam(team);
}

LOCAL_INLINE bool GolfRowCleared()
{
	return GolfDirectPlayer()->stroke[GOLFDOC()->m_currentHole - 1] ==
		(unsigned char)1;
}

LOCAL_INLINE bool GolfTeamCleared()
{
	return GolfDirectTeam()->stroke[GOLFDOC()->m_currentHole - 1] ==
		(unsigned char)1;
}

LOCAL_INLINE int PlayerGearDirect()
{
	unsigned char index = GOLFDOC()->m_currentPlayer;
	return *(int*)((unsigned char*)GOLFDOC()->GetPlayer(index) + 0x4bc);
}

LOCAL_INLINE int PlayerGear()
{
	return *(int*)((unsigned char*)PLAYER(GOLFDOC()->m_currentPlayer) + 0x4bc);
}

LOCAL_INLINE unsigned char DocGameType()
{
	return Doc()->m_golfGame.gameType;
}

LOCAL_INLINE unsigned char& TeamByte(unsigned char index, int offset)
{
	return *((unsigned char*)TEAM(index) + offset);
}

LOCAL_INLINE unsigned int& PlayerFlags(sPlayerData* player)
{
	return *(unsigned int*)&player->shotFlag;
}

LOCAL_INLINE unsigned int& CurPlayerFlags()
{
	unsigned char index = GOLFDOC()->m_currentPlayer;
	return PlayerFlags(GOLFDOC()->GetPlayer(index));
}

LOCAL_INLINE unsigned char ClubKind()
{
	return *(unsigned char*)(*(int*)((unsigned char*)&GolfClub() + 4) + 0x22);
}

LOCAL_INLINE int PlayerCardInt(unsigned char index, int offset)
{
	return *(int*)((unsigned char*)Doc() + index * 0x31c6 + offset);
}

LOCAL_INLINE unsigned char PlayerCardByte(unsigned char index, int offset)
{
	return *((unsigned char*)Doc() + index * 0x31c6 + offset);
}

// true when the stroke code of the current hole is within the par bound; false on unknown game types
LOCAL_INLINE bool ParBoundOk(bool allowType10 = false)
{
	switch (DocGameType())
	{
	case 10:
		if (!allowType10)
			break;
		// fallthrough
	case 0:
	case 4:
	case 5:
	case 6:
	case 7:
	case 9:
	{
		unsigned char* parRow =
			(unsigned char*)(*(int*)((unsigned char*)GDOCPTR983(GOLFDOC(), 0xd8)) +
				GolfByte(GDOCOFS983(0x17c)) * 0x38 - 0x38);
		if ((unsigned char)GolfDirectPlayer()
				->stroke[GOLFDOC()->m_currentHole - 1] <= parRow[0x18])
			return true;
		break;
	}
	case 1:
	{
		unsigned char* parRow =
			(unsigned char*)(*(int*)((unsigned char*)GDOCPTR983(GOLFDOC(), 0xd8)) +
				GolfByte(GDOCOFS983(0x17c)) * 0x38 - 0x38);
		if ((unsigned char)GolfDirectTeam()
				->stroke[GOLFDOC()->m_currentHole - 1] <= parRow[0x18])
			return true;
		break;
	}
	case 0xb:
		return true; // 983 0052d640: tiki sempre dentro do par
	}
	return false;
}

// WIP (run 21): blocks 1..4 of the original (up to the 0x2a0 case); the rest is not written yet
LOCAL_INLINE const WVector* ExpectedPos(CGolfBall& ball, int index)
{
	return index >= 0 ? (index < 0x5dc ? &ball.m_expected[index].pos
									   : &ball.m_pExtra[index - 0x5dc].pos)
					  : NULL;
}

LOCAL_INLINE const WVector* ExpectedPosInRange(CGolfBall& ball, int index)
{
	if (index < 0 || index > ball.m_renderNum)
		return NULL;
	return index < 0x5dc ? &ball.m_expected[index].pos
						 : &ball.m_pExtra[index - 0x5dc].pos;
}

// 983 (00a18890): bonus de Pang do mascote; par de shorts em +0x12c (melhor tacada, normal), -1/-1 = tabela fixa 0x2a/0x4a por mapa
static int MascotPang983(unsigned long tid, int map, bool best)
{
	IFF_STRUCT::sMascot* m = Doc()->m_itemManager.FindMascot(tid);
	if (m == NULL)
		return 0;
	short hi = (short)(m->PangBonus983 & 0xffff);
	short lo = (short)(m->PangBonus983 >> 16);
	if (hi == -1 && lo == -1)
	{
		int top;
		if ((tid & 0x3ffffff) == 0x2a)
			top = 0x1e;
		else if ((tid & 0x3ffffff) == 0x4a)
			top = 0x32;
		else
			return 0;
		if (map > 0x7f && map != 0xfd)
			map -= 0x80;
		switch (map)
		{
		case 8: case 9: case 0xe: case 0x11: case 0x12:
			return best ? top : 0;
		}
		return best ? 10 : 0;
	}
	return best ? hi : lo;
}

// 983 (0052d760): bonus de bola/bounce (cases 17/18); PreCalc e ramo tiki (00531180) chamam
void CPangInfo::BallBonus983()
{
	unsigned char curPlayer = GOLFDOC()->m_currentPlayer;
	__int64 cardBonus;
	__int64 added;
	int bounce;
	unsigned long ballTid = PlayerCard(curPlayer);
	IFF_STRUCT::sBall* ballIff = ItemManager()->FindBall(ballTid);

	// 983 (0052d760): bola comum (Price e COM[0] zerados) cai no ramo B; as demais, com modo 4 e sem putter, no A
	if ((ballIff == NULL || ballIff->c.Price != 0 || ballIff->COM[0] != 0) &&
		*(int*)&GolfByte(GDOCOFS983(0xf0)) == 4 && ClubKind() != 3)
	{
		if (ParBoundOk())
		{
			CItemSoundTable& sounds = CItemSoundTable::Instance();
			m_case[17].value = sounds.GetFixed983(ballTid, 1);
			if ((PlayerCardInt(curPlayer, DOCOFS983(0x3922)) >> 7 & 1) != 0)
				m_case[17].value *= 2;
			m_gain[1] += m_case[17].value;
			CurPlayerFlags() |= 0x4000;
			cardBonus = sounds.GetFixed983(ballTid, 3) +
				CCardManager::Instance()->GetCardBounceBonusUp();
			if (cardBonus != 0 && !GolfBall().m_bOB)
			{
				bounce = GetPVS().GetBounceCount();
				if (bounce > 0x14)
					bounce = 0x14;
				added = (__int64)bounce * cardBonus;
				m_gain[1] += added;
				m_case[18].value = added;
			}
		}
	}
	else
	{
		if (*(int*)&GolfByte(GDOCOFS983(0xf0)) == 4 && ClubKind() != 3)
		{
			CCardManager::Instance()->SetPlayerIndex(curPlayer);
			cardBonus = CCardManager::Instance()->GetCardBounceBonusUp();
			if (cardBonus > 0 && !GolfBall().m_bOB)
			{
				bounce = GetPVS().GetBounceCount();
				if (bounce > 0x14)
					bounce = 0x14;
				added = (__int64)bounce * cardBonus;
				m_gain[1] += added;
				m_case[18].value = added;
			}
		}
		else
		{
			m_case[18].flags = 0x100;
		}
		if (ParBoundOk() && (ballTid & 0x3ffffff) != 0 && ClubKind() != 3)
		{
			m_case[17].value = CItemSoundTable::Instance().GetFixed983(ballTid, 2);
			m_gain[1] += m_case[17].value;
		}
	}
	if (m_case[17].value == 0)
		m_case[17].flags = 0x100;
	if (m_case[18].value == 0)
	{
		m_case[18].flags = 0x100;
	}
	else if ((GolfBall().m_initSpecial & 0x50) != 0 &&
		GetPVS().GetBounceCount() < 2)
	{
		m_gain[1] -= m_case[18].value;
		m_case[18].flags = 0x100;
		m_case[18].value = 0;
	}
	else
	{
		CurPlayerFlags() |= 0x20000;
	}

	if (IsLocalContent((localContentType_t)3 /*983: 0052dada*/))
	{
		unsigned char* sp;
		if (DocGameType() == 1)
			sp = (unsigned char*)TEAM(((unsigned char*)PLAYER(GOLFDOC()->m_currentPlayer))[0x4e9]) +
				GolfByte(GDOCOFS983(0x17c)) * 0x10 - 0xc;
		else
			sp = (unsigned char*)PLAYER(GOLFDOC()->m_currentPlayer) + 0x1c +
				GolfByte(GDOCOFS983(0x17c)) * 0x10;
		unsigned char strokes = ~(sp[0xe] ^ sp[0]);
		if (IsLocalContent((localContentType_t)0x50) && Doc()->m_playingMode == 2)
			strokes = *(unsigned char*)&Doc()->m_replayShotIndex;
		MascotCase983(strokes);
	}
}

// 983 (0052d1b0): bonus do mascote (case 21) por tipo de mascote; strokes = tacadas do buraco
void CPangInfo::MascotCase983(unsigned char strokes)
{
	unsigned char cur = GOLFDOC()->m_currentPlayer;
	unsigned long tid = PlayerCardInt(cur, DOCOFS983(0x6a3c));
	bool has = PlayerCardInt(cur, DOCOFS983(0x3b9e)) != 0;
	int mode = *(int*)&GolfByte(GDOCOFS983(0xf0));
	bool best = strokes == 1;
	__int64 v;
	unsigned int flag = 0;
	switch (tid & 0x3ffffff)
	{
	case 4: case 8: case 0xe: case 0xf: case 0x10: case 0x13: case 0x18: case 0x19:
	case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x22: case 0x24: case 0x25:
	case 0x26: case 0x29: case 0x2a: case 0x4a:
		if (has && ClubKind() != 3 && mode > 1)
		{
			unsigned char* sp;
			if (DocGameType() == 1)
				sp = (unsigned char*)TEAM(((unsigned char*)PLAYER(cur))[0x4e9]) +
					GolfByte(GDOCOFS983(0x17c)) * 0x10 - 0xc;
			else
				sp = (unsigned char*)PLAYER(cur) + 0x1c + GolfByte(GDOCOFS983(0x17c)) * 0x10;
			unsigned char par = *(unsigned char*)(*(int*)((unsigned char*)GDOCPTR983(GOLFDOC(), 0xd8)) -
				0x20 + GolfByte(GDOCOFS983(0x17c)) * 0x38);
			if ((unsigned char)~(sp[0xe] ^ sp[0]) <= par)
				m_case[21].value = MascotPang983(tid, GetCurMap(), mode == 4);
		}
		if (m_case[21].value <= 0)
			goto done;
		m_gain[1] += m_case[21].value;
		flag = mode == 4 ? 0x100000 : 0x200000;
		break;
	case 5:
		if (best)
		{
			v = PlayerCardByte(cur, DOCOFS983(0x6a40)) + 1;
			m_gain[1] += v;
			m_case[21].value = v;
		}
		goto done;
	case 6:
	{
		if (!GolfBall().IsHoleIn())
			goto done;
		unsigned char* sp = (unsigned char*)PLAYER(cur) + 0x1c + GolfByte(GDOCOFS983(0x17c)) * 0x10;
		if (best)
			v = 100;
		else
		{
			switch ((unsigned char)~(sp[0xe] ^ sp[0]) -
				(int)*(unsigned char*)(*(int*)((unsigned char*)GDOCPTR983(GOLFDOC(), 0xd8)) - 0x20 +
					GolfByte(GDOCOFS983(0x17c)) * 0x38))
			{
			case 0: v = 0x14; break;
			case -3: v = 100; break;
			case -2: v = 0x32; break;
			case -1: v = 0x1e; break;
			default: v = 10;
			}
		}
		m_gain[1] += v;
		m_case[21].value = v;
		flag = 0x100000;
		break;
	}
	case 7: case 0x14: case 0x15: case 0x16: case 0x17: case 0x23: case 0x30:
		if (!has || !best || mode < 2)
			goto done;
		v = MascotPang983(tid, GetCurMap(), mode == 4);
		m_gain[1] += v;
		m_case[21].value = v;
		flag = mode == 4 ? 0x100000 : 0x200000;
		break;
	default:
		if (!has || !best || mode != 4)
			goto done;
		m_gain[1] += 10;
		m_case[21].value = 10;
		flag = 0x100000;
		break;
	}
	CurPlayerFlags() |= flag;
done:
	m_case[21].flags = m_case[21].value != 0 ? 1 : 0x100;
}

void CPangInfo::PreCalc(bool param)
{
	float over;
	CGolfBall& ball = GolfBall();
	ChangeTurn();
	m_flags = 1;
	m_gain[0] = 0;
	m_gain[1] = 0;
	m_gain[2] = 0;
	m_gain[3] = 0;
	// 983 (00531180): sala tiki (0xb) so tem bola/bounce, mascote e hole-in; o resto fica oculto
	if (IsGameType(0xb))
	{
		for (int i = 0; i < 25; i++)
			m_case[i].flags = 0x100;
		m_case[17].flags = 1;
		m_case[18].flags = 1;
		m_case[21].flags = 1;
		BallBonus983();
		if (GolfBall().IsHoleIn())
		{
			PlayerFlags(GOLFDOC()->GetPlayer(GolfByte(GDOCOFS983(0x189)))) |= 0x100;
			m_case[9].flags = 1;
			// 983 00531251: this+0x7fc + m_7c; o cru +0x7c lia outro campo no VS (lixo/0 = 100/0 = INT64_MIN no painel)
			int tikiCount = TIKIDOC983(m_pTikiData983)->m_7c;
			__int64 hio = (__int64)(100.0 / (float)sqrt((double)tikiCount));
			m_case[9].value += hio;
			m_gain[0] += hio;
		}
		GetHoleInOverlayInfo(1, 0, (unsigned int&)m_case[9].field20,
			m_case[9].field24, m_case[9].field28);
		Record(GolfByte(GDOCOFS983(0x189)), &m_gain[0]);
		return;
	}
	if (GolfByte(GDOCOFS983(0x194)) != 0)
	{
		for (int i = 0; i < 25; i++)
			m_case[i].flags = 0x100;
		return;
	}
	unsigned char who;
	sPlayerData* p;
	if (!IsGameType(1))
	{
		who = GOLFDOC()->m_currentPlayer;
		p = (sPlayerData*)((unsigned char*)PLAYER(GOLFDOC()->m_currentPlayer) +
			(GolfByte(GDOCOFS983(0x17c)) + 1) * 0x10 + 0xc); // 983: stroke[] em +0x2c (645 +0x20)
	}
	else
	{
		who = GOLFDOC()->m_currentPlayer;
		p = PLAYER(who);
		who = ((unsigned char*)p)[0x4e9];
		p = (sPlayerData*)((unsigned char*)TEAM(((unsigned char*)p)[0x4e9]) +
			GolfByte(GDOCOFS983(0x17c)) * 0x10 - 0xc);
	}
	unsigned char scoreCode =
		~(((unsigned char*)p)[0xe] ^ ((unsigned char*)p)[0]);
	if (IsLocalContent(S4_REPLAY_SYSTEM) &&
		Doc()->m_playingMode == 2)
		scoreCode = *(unsigned char*)&Doc()->m_replayShotIndex;
	int parCode = *(unsigned char*)(*(int*)((unsigned char*)GDOCPTR983(GOLFDOC(), 0xd8)) -
		0x20 + GolfByte(GDOCOFS983(0x17c)) * 0x38);
	int diff = scoreCode - parCode;
	m_f6d4 = diff;
	*(int*)m_pad6d8 =
		*(unsigned char*)(*(int*)((unsigned char*)&GolfClub() + 4) + 0x22);
	m_f6e0 = GolfClub().GetRange(NULL);
	// 983 (005324fc): byte da bola +0x59 em this+0x72c; o 0x714 do 645 cai no flag34 (alpha) do m_case[24]
	C_ASSERT_VC10(offsetof(CPangInfo, m_pad6d8) == 0x728 && offsetof(CPangInfo, m_case) == 0x1a0);
	*(int*)(m_pad6d8 + 4) = *((unsigned char*)&ball + 0x59);
	const WVector* start =
		ball.m_renderNum < 0 ? NULL : &ball.m_expected[0].pos;
	WVector pin = *(const WVector*)(*(int*)((unsigned char*)GDOCPTR983(GOLFDOC(), 0xd8)) -
		0x2c + GolfByte(GDOCOFS983(0x17c)) * 0x38);
	WVector d(pin.x - start->x, pin.y - start->y, pin.z - start->z);
	WVector* dir = (WVector*)&m_dirX;
	*dir = d;
	m_dirY = 0.0f;
	m_f6e4 =
		sqrt(dir->x * dir->x + dir->y * dir->y + dir->z * dir->z) * 0.3125f;
	NormalizeVector(*dir);

	const WVector* cur = ExpectedPos(ball, ball.m_renderNum);
	const WVector* first =
		ball.m_renderNum < 0 ? NULL : &ball.m_expected[0].pos;
	WVector travel;
	travel = WVector(first->x - cur->x, first->y - cur->y, first->z - cur->z);
	float travelDist =
		sqrt(travel.x * travel.x + travel.z * travel.z) * 0.3125f;
	over = 0.0f < travelDist ? travelDist : 0.0f;
	over = over - m_f6e0;
	if (scoreCode == 1 && (*(int*)m_pad6d8 == 0 || *(int*)m_pad6d8 == 1))
	{
		if (IsLocalContent(S3_CADDIEBONUS))
		{
			__int64 bonus = m_caddieBonus.GetBonusPang((eCaddie)12, 0);
			if (bonus != 0)
			{
				m_case[22].value += bonus;
				m_gain[1] += bonus;
				CurPlayerFlags() |= 0x01000000;
			}
		}
		if (over > 0.0f && GolfBall().m_bOB == 0 && GetCurMap() != 0x13)
		{
			m_gain[0] += (int)over;
			CurPlayerFlags() |= 1;
		}
		else
		{
			m_case[0].flags = 0x100;
		}
		if (GolfByte(GDOCOFS983(0xf4)) == 0 || *(int*)&GolfByte(GDOCOFS983(0xf0)) != 4 ||
			GolfClub().PowerShotType() == 0)
		{
			m_case[1].flags = 0x100;
		}
		else
		{
			m_gain[0] = m_gain[0] * 2;
			m_case[1].value = 2;
			CurPlayerFlags() |= 4;
		}
	}
	else
	{
		m_case[0].flags = 0x100;
		m_case[1].flags = 0x100;
	}
	const WVector* now = ball.m_renderNum >= 0
		? (ball.m_renderNum < 0x5dc
				  ? &ball.m_expected[ball.m_renderNum].pos
				  : &ball.m_pExtra[ball.m_renderNum - 0x5dc].pos)
		: NULL;
	WVector pinV = *(const WVector*)(*(int*)((unsigned char*)GDOCPTR983(GOLFDOC(), 0xd8)) -
		0x2c + GolfByte(GDOCOFS983(0x17c)) * 0x38);
	WVector toPin;
	toPin = WVector(pinV.x - now->x, pinV.y - now->y, pinV.z - now->z);
	float pinDist = sqrt(toPin.x * toPin.x + toPin.z * toPin.z) * 0.3125f;
	if (*(int*)&m_f6d4 < 0 && *(int*)m_pad6d8 != 3)
	{
		const WVector* ground =
			ball.m_renderNum < 0 ? NULL : &ball.m_expected[0].pos;
		if (GetPVS().GetGroundType(*ground, true, NULL) != 2 &&
			pinDist < 7.0f && !ball.IsHoleIn() && m_f6e4 > 30.0f)
		{
			int mult = 1;
			if (S5::CLASSICSRV::IsClassicGame983() != 0)
				mult = 2;
			int amount;
			if (m_f6e4 < 60.0f)
				amount = ((1 - (10 - float2int(pinDist)) * m_f6d4) * mult) / 2;
			else
				amount = -((10 - float2int(pinDist)) * m_f6d4 * mult);
			m_gain[0] += amount;
			PlayerFlags(PLAYER(GOLFDOC()->m_currentPlayer)) |= 0x200;
		}
		else
		{
			m_case[2].flags = 0x100;
		}
	}
	else
	{
		m_case[2].flags = 0x100;
	}
	if (m_f6e4 > 17.0f && *(int*)m_pad6d8 == 3 && m_f6d4 <= 0)
	{
		sPlayerData* pl = PLAYER(GOLFDOC()->m_currentPlayer);
		unsigned char* row =
			(unsigned char*)pl + (GolfByte(GDOCOFS983(0x17c)) + 0x13) * 0x10 + 0xc; // 983: +0x13c
		if ((unsigned char)~(row[0xe] ^ row[0]) != 0)
		{
			m_case[3].flags = 0x100;
		}
		else if (ball.IsHoleIn())
		{
			int amount;
			if (S5::CLASSICSRV::IsClassicGame983() != 0)
				amount = float2int(travelDist) * 2;
			else
				amount = float2int(travelDist);
			m_gain[0] += amount * 2;
			CurPlayerFlags() |= 0x80;
			// 983 (00528620): bonus do cartao NPC (efeito 4) sobre a distancia
			float rate = CCardManager::Instance()->GetNpcCard4Rate983();
			if (0.0f < rate)
			{
				int base = float2int(travelDist);
				base = S5::CLASSICSRV::IsClassicGame983() == 0 ? base * 2
															   : base * 4;
				__int64 card = (__int64)floor((double)((float)base * rate));
				m_case[4].flags = 1;
				m_case[4].value += card;
				m_gain[0] += card;
			}
		}
	}
	else
	{
		m_case[3].flags = 0x100;
	}
	if (*(int*)m_pad6d8 != 3 && ball.m_groundType != 2 && m_f6e4 > 17.0f &&
		m_f6d4 <= 0)
	{
		if (ball.IsHoleIn())
		{
			bool plain = true;
			int amount;
			// 983 (00532300): classic dobra o multiplicador (4x/8x)
			int mul =
				S5::CLASSICSRV::IsClassicGame983() != 0 ? 2 : 1;
			if ((ball.m_initSpecial & 0x70) != 0 ||
				GolfClub().GetApproachMode() != 4)
			{
				amount = float2int(travelDist) * 2 * mul;
			}
			else
			{
				m_gain[0] += float2int(travelDist) * 4 * mul;
				PlayerFlags(PLAYER(GOLFDOC()->m_currentPlayer)) |= 0x10000;
				plain = false;
			}
			if (plain)
				m_gain[0] += amount;
			if (travelDist < 200.0f)
				PlayerFlags(PLAYER(GOLFDOC()->m_currentPlayer)) |= 0x20;
			else
				PlayerFlags(PLAYER(GOLFDOC()->m_currentPlayer)) |= 0x40;
		}
	}
	else
	{
		m_case[5].flags = 0x100;
	}
	const WVector* ground0 =
		ball.m_renderNum < 0 ? NULL : &ball.m_expected[0].pos;
	if (GetPVS().GetGroundType(*ground0, true, NULL) != 2 &&
		ball.m_cupFrame > 0 && ball.m_cupFrame <= ball.m_renderNum &&
		ball.m_rollFrame < 0 &&
		(ball.m_cupFrame < ball.m_bounceFrame || ball.m_bounceFrame < 0))
	{
		WVector cupTmp;
		const WVector& cup = GetHoleCupPos(cupTmp);
		const WVector& hit = ball.PreData(ball.m_cupFrame)->pos;
		float dx = hit.x - cup.x;
		float dy = hit.y - cup.y;
		float dz = hit.z - cup.z;
		float cupDist = sqrt(dx * dx + dy * dy + dz * dz);
		if (m_f6d4 <= 0 && ball.IsHoleIn() &&
			cupDist > ball.m_radius + ball.m_radius)
		{
			m_case[6].value = 100;
			m_gain[0] += 100;
			PlayerFlags(PLAYER(GOLFDOC()->m_currentPlayer)) |= 0x10;
		}
		else
		{
			m_case[6].flags = 0x100;
		}
	}
	else
	{
		m_case[6].flags = 0x100;
	}
	if ((ball.m_special & 0x70) != 0)
	{
		if (PlayerGearDirect() == 0x1800000a ||
			PlayerGearDirect() == 0x18000007 ||
			PlayerGearDirect() == 0x18000002 ||
			PlayerGearDirect() == 0x1800000b ||
			PlayerGearDirect() == 0x18000008 || PlayerGear() == 0x18000003 ||
			PlayerGear() == 0x1800000f || PlayerGear() == 0x18000004 ||
			PlayerGear() == 0x18000010 || PlayerGear() == 0x18000012 ||
			PlayerGear() == 0x1800001f || PlayerGear() == 0x18000022 ||
			PlayerGear() == 0x18000021 || PlayerGear() == 0x1800002c ||
			PlayerGear() == 0x18000027 || PlayerGear() == 0x1800002a ||
			PlayerGear() == 0x1800002b)
			m_case[8].field8 = 5;
		else
			m_case[8].field8 = 10;
		m_case[8].fieldC = 0;
		m_gain[0] += m_case[8].value;
		CurPlayerFlags() |= 8;
		GetSpecialShotOverlayInfo(ball.m_special, GolfClub().PowerShotType(),
			(unsigned int&)m_case[8].field20, m_case[8].field24,
			m_case[8].field28);
	}
	else
	{
		m_case[8].flags = 0x100;
	}
	m_case[7].flags = 0x100;

	unsigned char teamCount = 0;
	if (!param)
	{
		const std::vector<CQuadTree::sColData>& colList = GetPVS().GetColList();
		short prevTeam = -1;
		std::vector<CQuadTree::sColData>::const_iterator it;
		for (it = colList.begin(); it != colList.end(); ++it)
		{
			short team = (*it).objIndex;
			if (team >= 0 && team != prevTeam)
			{
				teamCount++;
				prevTeam = team;
			}
		}
	}

	if (ball.IsHoleIn())
	{
		CurPlayerFlags() |= 0x100;
		if (DocGameType() == 1)
		{
			m_case[9].flags = 0x100;
		}
		else
		{
			m_case[9].value = GetHoleInPang(scoreCode, parCode);
			GetHoleInOverlayInfo(scoreCode, parCode,
				(unsigned int&)m_case[9].field20, m_case[9].field24,
				m_case[9].field28);
		}
		if (*(int*)&GolfByte(GDOCOFS983(0xf0)) == 4 && m_f6d4 <= 0 && teamCount == 0 &&
			(ball.m_initSpecial & 0xf) != 0)
		{
			if ((ball.m_initSpecial & 3) != 0)
			{
				if (DocGameType() == 1)
				{
					TeamByte(((unsigned char*)PLAYER(
								 GOLFDOC()->m_currentPlayer))[0x4e9],
						0x3b1) = 0xa;
					m_case[10].flags = 0x100;
				}
				else
				{
					S5::CLASSICSRV::IsClassicGame983();
					m_gain[0] += m_case[9].value * 2;
					PlayerFlags(PLAYER(GOLFDOC()->m_currentPlayer)) |= 0x400;
				}
				m_case[11].flags = 0x100;
				m_case[12].flags = 0x100;
			}
			else
			{
				if (DocGameType() == 1)
				{
					TeamByte(((unsigned char*)PLAYER(
								 GOLFDOC()->m_currentPlayer))[0x4e9],
						0x3b1) = 0xb;
					m_case[11].flags = 0x100;
					m_case[10].flags = 0x100;
					m_case[12].flags = 0x100;
				}
				else
				{
					m_gain[0] += m_case[9].value * 2;
					PlayerFlags(PLAYER(GOLFDOC()->m_currentPlayer)) |= 0x800;
					m_case[10].flags = 0x100;
					m_case[12].flags = 0x100;
				}
			}
		}
		else
		{
			if (DocGameType() == 1)
			{
				if (m_f6d4 <= 0)
					((unsigned char*)GolfDirectTeam())[0x3b1] = 9;
			}
			else
			{
				m_gain[0] += m_case[9].value;
			}
			m_case[10].flags = 0x100;
			m_case[11].flags = 0x100;
			if (teamCount > 0)
			{
				if (DocGameType() == 1)
				{
					if (m_f6d4 <= 0)
					{
						((unsigned char*)GolfDirectTeam())[0x3b1] = 0xc;
						((unsigned char*)GolfDirectTeam())[0x3b2] = teamCount;
					}
					m_case[12].flags = 0x100;
				}
				else
				{
					if (m_f6d4 <= 0)
						m_gain[0] +=
							(__int64)((int)pow(2.0f, (int)teamCount) - 1) *
							m_case[9].value;
					m_case[12].value = 1;
					CurPlayerFlags() |= 2;
				}
			}
			else
			{
				m_case[12].flags = 0x100;
			}
		}
	}
	else
	{
		if ((ball.m_bOB != 0) + m_f6d4 < 5 || !param)
		{
			m_case[9].flags = 0x100;
		}
		else
		{
			// 983 (00532300): valor5 do campo atual (00a07150, faixa = classico), sem x5; field28 = 0.9375
			IFF983::sCourseBonus* courseBonus = ItemManager()->FindCourseBonus983(
				GetCurMap(), S5::CLASSICSRV::IsClassicGame983() == 1);
			if (courseBonus != NULL)
				m_case[9].value = courseBonus->value5;
			m_gain[0] += m_case[9].value;
			m_case[9].field28 = 0.9375f;
		}
		m_case[10].flags = 0x100;
		m_case[11].flags = 0x100;
		if (teamCount > 0)
		{
			m_case[12].value = 1;
		}
		else
		{
			m_case[12].flags = 0x100;
		}
	}
	if (ball.IsHoleIn() &&
		(((DocGameType() == 0 || DocGameType() == 7) &&
			 GolfRowCleared()) ||
			(DocGameType() == 1 && GolfTeamCleared())))
	{
		m_case[13].value = 0x14;
		m_gain[1] += m_case[13].value;
		CurPlayerFlags() |= 0x1000;
	}
	else
	{
		m_case[13].flags = 0x100;
	}

	const WVector* hit;
	if (ball.m_afterBurnerFrame > 0 &&
		ball.m_afterBurnerFrame < ball.m_renderNum &&
		(hit = ExpectedPosInRange(ball, ball.m_afterBurnerFrame)) != NULL &&
		GetPVS().GetGroundType(*hit, true, NULL) >= 3)
	{
		if (GOLFDOC()->m_pPolySoup->IsOutOfBound(*hit))
		{
			if (!GOLFDOC()->m_pPolySoup->IsOutOfBound(
					ball.PreData(ball.m_renderNum)->pos))
			{
				m_gain[0] += 0xf;
				PlayerFlags(GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)) |=
					0x8000;
			}
		}
		else
		{
			unsigned char ground = GetPVS().GetGroundType(
				ball.PreData(ball.m_renderNum)->pos, true, NULL);
			if (ground == 1 || ground == 2)
			{
				m_gain[0] += 10;
				CurPlayerFlags() |= 0x8000;
			}
		}
	}
	else
	{
		m_case[16].flags = 0x100;
	}
	BallBonus983();

	// 983 (0052e160): bonus do cartao NPC (efeito 3) sobre o Pang do hole-in, so ate o par
	if (ball.IsHoleIn())
	{
		float rate = CCardManager::Instance()->GetNpcCard3Rate983();
		__int64 holePang = GetHoleInPang(scoreCode, parCode);
		if (0.0f < rate && scoreCode <= parCode)
		{
			__int64 card = (__int64)floor((double)(rate * (float)holePang));
			m_case[4].flags = 1;
			m_case[4].value += card;
			m_gain[1] += card;
		}
	}

	Doc()->SetInitAbilityItem();

	if ((Doc()->GetFlagAbilityItem() & 1) != 0 && *(int*)&GolfByte(GDOCOFS983(0xf0)) > 1 &&
		ClubKind() != 3 && ParBoundOk(true))
	{
		if (*(int*)&GolfByte(GDOCOFS983(0xf0)) == 4)
		{
			m_case[22].value += Doc()->GetNumBonusPangItem() * 3;
			CurPlayerFlags() |= 0x400000;
		}
		else
		{
			m_case[22].value += Doc()->GetNumBonusPangItem() * 2;
			CurPlayerFlags() |= 0x800000;
		}
		m_gain[1] += m_case[22].value;
	}

	if (Doc()->m_golfGame.weather != 2 || param)
	{
		m_case[15].flags = 0x100;
	}
	else
	{
		if (m_gain[0] > 0)
		{
			m_shownTotal[0] = m_gain[0];
			m_gain[0] = m_gain[0] * 2;
		}
		if (m_gain[1] > 0)
		{
			m_shownTotal[1] = m_gain[1];
			m_gain[1] = m_gain[1] * 2;
		}
		m_case[15].value = 2;
	}

	if ((PlayerCardByte(GOLFDOC()->m_currentPlayer, DOCOFS983(0x3942)) & 0x20) == 0 ||
		*(int*)&GolfByte(GDOCOFS983(0xf0)) != 4 || ClubKind() == 3)
	{
		m_case[19].flags = 0x100;
		if (HaveWingItem(GOLFDOC()->m_currentPlayer, false) &&
			*(int*)&GolfByte(GDOCOFS983(0xf0)) == 4 && ClubKind() != 3 &&
			(DocGameType() == 0 || DocGameType() == 1 || DocGameType() == 7 ||
				DocGameType() == 9 || DocGameType() == 10))
		{
			m_case[19].flags = 1;
			m_case[19].value = 10;
			CurPlayerFlags() |= 0x2000000;
		}
	}
	else
	{
		switch (DocGameType())
		{
		case 0:
		case 1:
		case 4:
		case 5:
		case 6:
		case 7:
		case 9:
		case 10:
			m_case[19].value = -1;
			CurPlayerFlags() |= 0x40000;
			break;
		default:
			m_case[19].flags = 0x100;
		}
	}

	if ((PlayerCardByte(GOLFDOC()->m_currentPlayer, DOCOFS983(0x3942)) & 0x80) == 0 ||
		*(int*)&GolfByte(GDOCOFS983(0xf0)) != 4 || ClubKind() == 3)
	{
		m_case[20].flags = 0x100;
		if (HaveWingItem(GOLFDOC()->m_currentPlayer, true) &&
			*(int*)&GolfByte(GDOCOFS983(0xf0)) == 4 && ClubKind() != 3)
		{
			m_gain[1] += 10;
			m_case[20].flags = 1;
			m_case[20].value = 10;
			CurPlayerFlags() |= 0x4000000;
		}
	}
	else
	{
		m_gain[1] += 1;
		m_case[20].value = 1;
		CurPlayerFlags() |= 0x80000;
	}

	if (*(int*)&GolfByte(GDOCOFS983(0x88)) == 0xf &&
		GolfByte(GDOCOFS983(0x17c)) == Doc()->m_holeOrder[Doc()->m_golfGame.holes - 1] &&
		(DocGameType() == 0 || DocGameType() == 1 || DocGameType() == 4 ||
			DocGameType() == 5 || DocGameType() == 6) &&
		(ball.IsHoleIn() || (!ball.IsHoleIn() && param)))
	{
		int clearBonus = GetGameClearBonus();
		m_case[14].value = clearBonus;
		float clearProb =
			CCardManager::Instance()->GetSpecialClearBonusProb(GetCurMap());
		int clearExtra = (int)(GetGameClearBonus() * clearProb);
		m_case[14].value += clearExtra;
		if (IsLocalContent((localContentType_t)0x33 /*983: era 0x32; interpolado*/))
		{
			__int64 caddiePang =
				m_caddieBonus.GetBonusPang((eCaddie)0xd, m_case[14].value);
			if (caddiePang != 0)
			{
				m_case[22].value += caddiePang;
				m_gain[1] += caddiePang;
				CurPlayerFlags() |= 0x1000000;
			}
		}
		if ((Doc()->m_roomSetupStage & 1) != 0)
			m_case[14].value = m_case[14].value * 2;
		if (param)
			m_gain[1] += m_case[14].value;
		CurPlayerFlags() |= 0x2000;
	}
	else
	{
		m_case[14].flags = 0x100;
	}
	if (m_case[22].value == 0)
		m_case[22].flags = 0x100;
	if (m_case[4].value == 0)
		m_case[4].flags = 0x100;
	Record(GOLFDOC()->m_currentPlayer, &m_gain[0]);
}

void CPangInfo::PreCalcMatchHoleInBonus()
{
	if (!IsGameType(3))
		return;
	if (m_sound != NULL)
		g_audio->StopSfx(m_sound);
	int total = PLAYER(MyIndex())->totalPang;
	m_slot[0].total = total;
	m_shown[0] = total;
	m_pang[0] = total;
	int bonus = PLAYER(MyIndex())->bonusPang;
	m_slot[1].total = bonus;
	m_shown[1] = bonus;
	m_pang[1] = bonus;
	ResetVars(1, 0x100);
	PlayerByte(PLAYER(GOLFDOC()->m_currentPlayer), 0x4e1) &= 0xef;
	sPlayerData* me = PLAYER(MyIndex());
	if (me->holeInPose != 0)
	{
		m_case[9].flags = 1;
		int strokes = PlayerStroke(MyIndex());
		int par = GOLFDOC()->GetHoleData().par;
		if (strokes - par <= 0)
		{
			*(__int64*)&m_case[9].field8 = GetHoleInPang(strokes, par);
			GetHoleInOverlayInfo(strokes, par, (unsigned int&)m_case[9].field20,
				m_case[9].field24, m_case[9].field28);
		}
		else
		{
			m_case[9].flags = 0x100;
		}
		// 983 (0053c900): pose 0xa/0xb do 983 (= 9/0xa daqui) sempre dobram; o teste do classic
		// (00b30580) e codigo morto
		if (me->holeInPose == 9)
		{
			m_gain[0] += *(__int64*)&m_case[9].field8 * 2;
			m_case[10].flags = 1;
		}
		else if (me->holeInPose == 0xa)
		{
			m_gain[0] += *(__int64*)&m_case[9].field8 * 2;
			m_case[11].flags = 1;
		}
		else
		{
			m_gain[0] += *(__int64*)&m_case[9].field8;
		}
		if (me->holeInCollision != 0)
		{
			int mult = (int)pow(2.0f, (int)me->holeInCollision);
			*(__int64*)&m_case[12].field8 = mult;
			if (strokes <= par)
				m_gain[0] += (__int64)(mult - 1) * *(__int64*)&m_case[9].field8;
			m_case[12].flags = 1;
		}
		if (Doc()->m_golfGame.weather == 2)
		{
			if (m_gain[0] > 0)
			{
				m_shownTotal[0] = m_gain[0];
				m_gain[0] *= 2;
			}
			if (m_gain[1] > 0)
			{
				m_shownTotal[1] = m_gain[1];
				m_gain[1] *= 2;
			}
			m_case[15].flags = 1;
		}
	}
	int diff = PLAYER(MyIndex())->score - PLAYER(1 - MyIndex())->score;
	unsigned char hole = 0;
	do
	{
		unsigned char next = hole + 1;
		if (Doc()->m_holeOrder[hole] == GOLFDOC()->m_currentHole)
		{
			hole = next;
			break;
		}
		hole = next;
	} while (hole < 0x12);
	if (hole >= 0x12)
		hole = 0x13;
	int remaining = Doc()->m_golfGame.holes - hole;
	bool lost = false;
	bool lostBonus = false;
	if (remaining > 0)
	{
		if (PlayerStroke(MyIndex()) < PlayerStroke(1 - MyIndex()))
			diff = diff + 1;
		else if (PlayerStroke(1 - MyIndex()) < PlayerStroke(MyIndex()))
			diff = diff - 1;
		if (remaining < diff)
		{
			int bonusValue = remaining * 20;
			m_gain[0] += bonusValue;
			*(__int64*)&m_case[7].field8 = bonusValue;
			m_case[7].flags = 1;
			lostBonus = true;
		}
		else
		{
			int diff2 = PLAYER(1 - MyIndex())->score - PLAYER(MyIndex())->score;
			if (PlayerStroke(1 - MyIndex()) < PlayerStroke(MyIndex()))
				diff2 = diff2 + 1;
			else if (PlayerStroke(MyIndex()) < PlayerStroke(1 - MyIndex()))
				diff2 = diff2 - 1;
			if (remaining < diff2)
				lost = true;
		}
	}
	if (GOLFDOC()->m_currentHole == Doc()->m_holeOrder[Doc()->m_golfGame.holes - 1] ||
		lostBonus || lost)
	{
		m_case[14].field8 = GetGameClearBonus();
		m_case[14].fieldC = m_case[14].field8 >> 31;
		float prob =
			CCardManager::Instance()->GetSpecialClearBonusProb(GetCurMap());
		*(__int64*)&m_case[14].field8 +=
			(int)((float)GetGameClearBonus() * prob);
		if ((Doc()->m_roomSetupStage & 1) != 0)
			*(__int64*)&m_case[14].field8 *= 2;
		m_case[14].flags = 1;
		PlayerByte(PLAYER(GOLFDOC()->m_currentPlayer), 0x4e1) |= 0x20;
		m_gain[1] += *(__int64*)&m_case[14].field8;
		if (PlayerStroke(1 - MyIndex()) < PlayerStroke(MyIndex()))
			m_gain[0] = 0;
		Record(MyIndex(), &m_gain[0]);
	}
	else
	{
		if (PlayerStroke(MyIndex()) <= PlayerStroke(1 - MyIndex()))
			Record(MyIndex(), &m_gain[0]);
		else
			m_gain[0] = 0;
	}
	__int64 other = 0;
	if (PLAYER(1 - MyIndex())->holeInPose != 0 &&
		PlayerStroke(MyIndex()) <= PlayerStroke(1 - MyIndex()))
	{
		int strokes = PlayerStroke(1 - MyIndex());
		int par = GOLFDOC()->GetHoleData().par;
		if (strokes - par <= 0)
		{
			other = GetHoleInPang(strokes, par);
			if (PLAYER(1 - MyIndex())->holeInPose == 9 ||
				PLAYER(1 - MyIndex())->holeInPose == 0xa)
				other *= 2;
			if (PLAYER(1 - MyIndex())->holeInCollision != 0)
				other *=
					(int)pow(2.0f, (int)PLAYER(1 - MyIndex())->holeInCollision);
			if (Doc()->m_golfGame.weather == 2)
				other *= 2;
		}
	}
	if (lost)
		other += remaining * 20;
	if (other > 0)
		Record(1 - MyIndex(), &other);
}

void CPangInfo::PreCalcTeamHoleInBonus()
{
	if (!IsGameType(1))
		return;
	if (m_sound != NULL)
		g_audio->StopSfx(m_sound);
	int total = TEAM(PLAYER(MyIndex())->team)->totalPang;
	m_slot[0].total = total;
	m_shown[0] = total;
	m_pang[0] = total;
	int bonus = TEAM(PLAYER(MyIndex())->team)->bonusPang;
	m_slot[1].total = bonus;
	m_shown[1] = bonus;
	m_pang[1] = bonus;
	ResetVars(1, 0x100);
	PlayerByte(PLAYER(GOLFDOC()->m_currentPlayer), 0x4e1) &= 0xef;
	unsigned char myTeam = PLAYER(MyIndex())->team;
	sTeamData* team = TEAM(myTeam);
	if (team->holeInPose != 0)
	{
		m_case[9].flags = 1;
		int strokes = TeamStroke(myTeam);
		int par = GOLFDOC()->GetHoleData().par;
		int diff = strokes - par;
		// 983 (0053c900): hole-in sempre paga, tambem acima do par (valor5 do campo)
		*(__int64*)&m_case[9].field8 = GetHoleInPang(strokes, par);
		GetHoleInOverlayInfo(strokes, par, (unsigned int&)m_case[9].field20,
			m_case[9].field24, m_case[9].field28);
		// 983 (0053c900, gameType 1): pose 0xa/0xb (PreCalc) sempre dobram; o teste do
		// classic (00b30580) e codigo morto
		if (team->holeInPose == 0xa)
		{
			m_gain[0] += *(__int64*)&m_case[9].field8 * 2;
			m_case[10].flags = 1;
		}
		else if (team->holeInPose == 0xb)
		{
			m_gain[0] += *(__int64*)&m_case[9].field8 * 2;
			m_case[11].flags = 1;
		}
		else
		{
			m_gain[0] += *(__int64*)&m_case[9].field8;
		}
		if (team->holeInCollision != 0)
		{
			int mult = (int)pow(2.0f, (int)team->holeInCollision);
			*(__int64*)&m_case[12].field8 = mult;
			if (diff <= 0)
				m_gain[0] += (__int64)(mult - 1) * *(__int64*)&m_case[9].field8;
			m_case[12].flags = 1;
		}
		if (Doc()->m_golfGame.weather == 2)
		{
			if (m_gain[0] > 0)
			{
				m_shownTotal[0] = m_gain[0];
				m_gain[0] *= 2;
			}
			if (m_gain[1] > 0)
			{
				m_shownTotal[1] = m_gain[1];
				m_gain[1] *= 2;
			}
			m_case[15].flags = 1;
		}
	}
	unsigned char myTeam2 = PLAYER(MyIndex())->team;
	unsigned char otherTeam = 1 - myTeam2;
	int scoreDiff = TEAM(myTeam2)->score - TEAM(otherTeam)->score;
	unsigned char hole = 0;
	CGolfDoc* doc = GOLFDOC();
	do
	{
		unsigned char next = hole + 1;
		if (Doc()->m_holeOrder[hole] == doc->m_currentHole)
		{
			hole = next;
			break;
		}
		hole = next;
	} while (hole < 0x12);
	if (hole >= 0x12)
		hole = 0x13;
	int remaining = Doc()->m_golfGame.holes - hole;
	bool lostBonus = false;
	bool lost = false;
	if (remaining > 0)
	{
		// 983 (0053c900): menos tacadas no buraco = ganhou o buraco
		if (TeamStroke(myTeam2) < TeamStroke(otherTeam))
			scoreDiff = scoreDiff + 1;
		else if (TeamStroke(otherTeam) < TeamStroke(myTeam2))
			scoreDiff = scoreDiff - 1;
		if (remaining < scoreDiff)
		{
			int bonusValue = remaining * 20;
			m_gain[0] += bonusValue;
			*(__int64*)&m_case[7].field8 = bonusValue;
			m_case[7].flags = 1;
			lostBonus = true;
		}
		else
		{
			int diff2 = TEAM(otherTeam)->score - TEAM(myTeam2)->score;
			if (TeamStroke(otherTeam) < TeamStroke(myTeam2))
				diff2 = diff2 + 1;
			else if (TeamStroke(myTeam2) < TeamStroke(otherTeam))
				diff2 = diff2 - 1;
			if (remaining < diff2)
				lost = true;
		}
	}
	if (GOLFDOC()->m_currentHole == Doc()->m_holeOrder[Doc()->m_golfGame.holes - 1] ||
		lostBonus || lost)
	{
		m_case[14].field8 = GetGameClearBonus();
		m_case[14].fieldC = m_case[14].field8 >> 31;
		float prob =
			CCardManager::Instance()->GetSpecialClearBonusProb(GetCurMap());
		*(__int64*)&m_case[14].field8 +=
			(int)((float)GetGameClearBonus() * prob);
		if ((Doc()->m_roomSetupStage & 1) != 0)
			*(__int64*)&m_case[14].field8 *= 2;
		m_case[14].flags = 1;
		PlayerByte(PLAYER(GOLFDOC()->m_currentPlayer), 0x4e1) |= 0x20;
		m_gain[1] += *(__int64*)&m_case[14].field8;
		if (TeamStroke(otherTeam) < TeamStroke(myTeam2))
			m_gain[0] = 0;
		Record(MyIndex(), &m_gain[0]);
	}
	else if (TeamStroke(otherTeam) < TeamStroke(myTeam2))
	{
		m_gain[0] = 0;
	}
	else
	{
		Record(MyIndex(), &m_gain[0]);
	}
	__int64 otherPang = 0;
	if (TEAM(otherTeam)->holeInPose != 0 &&
		TeamStroke(otherTeam) <= TeamStroke(myTeam2))
	{
		int strokes = TeamStroke(otherTeam);
		int par = GOLFDOC()->GetHoleData().par;
		otherPang = GetHoleInPang(strokes, par);
		if (TEAM(otherTeam)->holeInPose == 0xa ||
			TEAM(otherTeam)->holeInPose == 0xb)
			otherPang *= 2;
		if (TEAM(otherTeam)->holeInCollision != 0)
			otherPang *=
				(int)pow(2.0f, (int)TEAM(otherTeam)->holeInCollision);
		if (Doc()->m_golfGame.weather == 2)
			otherPang *= 2;
	}
	if (lost)
		otherPang += remaining * 20;
	if (otherPang > 0)
	{
		int i = 0;
		while (PLAYER(i)->team == myTeam2)
		{
			++i;
			if (i > 3)
				return;
		}
		Record(i, &otherPang);
	}
}

void CPangInfo::ShowMyPang()
{
	if (GameType() == 7)
	{
		if (m_sound != NULL)
		{
			g_audio->StopSfx(m_sound);
		}
		ResetVars(0, 0x100);
		CSharedDoc* doc = CSharedDoc::Instance();
		if (GameType() == 7 &&
			*((unsigned char*)GDOCPTR983(GOLFDOC(), 0x17c)) ==
				doc->m_holeOrder[doc->m_golfGame.holes - 1])
		{
			__int64 total = (int)PLAYER(MyIndex())->totalPang;
			m_slot[0].total = total;
			m_shown[0] = total;
			m_pang[0] = total;
			__int64 bonus = (int)PLAYER(MyIndex())->bonusPang;
			bonus = bonus + m_extraPang[MyIndex()];
			m_slot[1].total = bonus;
			m_shown[1] = bonus;
			m_pang[1] = bonus;
		}
		else
		{
			__int64 total = (int)PLAYER(MyIndex())->totalPang;
			m_slot[0].total = total;
			m_shown[0] = total;
			m_pang[0] = total;
			__int64 bonus = (int)PLAYER(MyIndex())->bonusPang;
			m_slot[1].total = bonus;
			m_shown[1] = bonus;
			m_pang[1] = bonus;
		}
		char* table = GroundItemTable();
		if (table != NULL)
		{
			__int64 value = ((GroundItemMan::PlayerStats*)table)[MyIndex()].numAcquiredLive[0];
			m_slot[2].total = value;
			m_shown[2] = value;
			m_pang[2] = value;
			value = ((GroundItemMan::PlayerStats*)table)[MyIndex()].numAcquiredLive[1];
			m_slot[3].total = value;
			m_shown[3] = value;
			m_pang[3] = value;
			return;
		}
		m_slot[2].total = 0;
		m_shown[2] = 0;
		m_pang[2] = 0;
		m_slot[3].total = 0;
		m_shown[3] = 0;
		m_pang[3] = 0;
	}
}

void CPangInfo::ResetVars(int flags, int state)
{
	m_flags = flags;
	m_sound = NULL;
	m_f6f0 = 0;
	m_f6f4 = 0;
	m_f780 = 0;
	m_f784 = -1;
	m_slot[0].state = 8;
	m_slot[1].state = 8;
	m_slot[2].state = 8;
	m_slot[3].state = 8;
	m_slot[0].field14 = 0;
	*(int*)&m_shownTotal[0] = 0;
	*((int*)&m_shownTotal[0] + 1) = 0;
	*(int*)&m_gain[0] = 0;
	*((int*)&m_gain[0] + 1) = 0;
	m_slot[1].field14 = 0;
	*(int*)&m_shownTotal[1] = 0;
	*((int*)&m_shownTotal[1] + 1) = 0;
	*(int*)&m_gain[1] = 0;
	*((int*)&m_gain[1] + 1) = 0;
	m_slot[2].field14 = 0;
	*(int*)&m_shownTotal[2] = 0;
	*((int*)&m_shownTotal[2] + 1) = 0;
	*(int*)&m_gain[2] = 0;
	*((int*)&m_gain[2] + 1) = 0;
	m_slot[3].field14 = 0;
	*(int*)&m_shownTotal[3] = 0;
	*((int*)&m_shownTotal[3] + 1) = 0;
	*(int*)&m_gain[3] = 0;
	*((int*)&m_gain[3] + 1) = 0;
	m_f750 = 0;
	for (int i = 0; i < 25; i++)
	{
		m_case[i].flags = state;
		m_case[i].field8 = 0;
		m_case[i].fieldC = 0;
		m_case[i].field10 = -1;
		m_case[i].field14 = 0;
		m_case[i].flag34 = 0;
		m_applied[i] = 0;
	}
}

__int64 CPangInfo::GetHoleInPang(int a, int b)
{
	// 983 (0052b040): registro do campo em CItemManager (00a07150, faixa = classico), nao a tabela fixa
	// do 645 em this+0x7a8 (no layout do 983 esse deslocamento nao e tabela nenhuma)
	unsigned char map = GetCurMap();
	if (GOLFDOC()->m_tutorialMode < 15)
	{
		map = 0;
	}
	IFF983::sCourseBonus* courseBonus = ItemManager()->FindCourseBonus983(
		map, S5::CLASSICSRV::IsClassicGame983() == 1);
	if (courseBonus == NULL)
	{
		return 0;
	}
	if (a == 1)
	{
		return courseBonus->value0;
	}
	switch (a - b)
	{
	case -3:
		return courseBonus->value1;
	case -2:
		return courseBonus->value2;
	case -1:
		return courseBonus->value3;
	case 0:
		return courseBonus->value4;
	default:
		return courseBonus->value5;
	}
}

void CPangInfo::GetHoleInOverlayInfo(int a, int b, unsigned int& id, float& x,
	float& y)
{
	int index;
	if (a == 1)
	{
		index = 0;
	}
	else
	{
		switch (a - b)
		{
		case -3:
			index = 1;
			break;
		case -2:
			index = 2;
			break;
		case -1:
			index = 3;
			break;
		case 0:
			index = 4;
			break;
		default:
			index = 5;
		}
	}
	id = g_holeInPangOverlayInfo[index].id;
	x = (float)g_holeInPangOverlayInfo[index].x * 0.5f;
	y = (float)g_holeInPangOverlayInfo[index].y * 0.0625f;
}

void CPangInfo::GetSpecialShotOverlayInfo(unsigned long flags,
	unsigned char type, unsigned int& id, float& x, float& y)
{
	int index;
	if ((flags & 0x10) != 0)
	{
		index = 0;
	}
	else
	{
		index = (flags & 0x20) != 0 ? 1 : 2;
	}
	id = g_specialShotPangOverlayInfo[index][type].id;
	x = (float)g_specialShotPangOverlayInfo[index][type].x * 0.5f;
	y = (float)g_specialShotPangOverlayInfo[index][type].y * 0.0625f;
}

void CPangInfo::UpdateBallState()
{
	CGolfBall& ball = GolfBall();
	if (ball.m_playFrame >= 0 && (GOLFDOC()->m_gameMode & 0x1000) == 0 &&
		(m_flags & 0x10) == 0)
	{
		CGolfBall& cur = GolfBall();
		const WVector& start = cur.PreData(0)->pos;
		int frame = cur.m_playFrame;
		WVector delta = cur.PreData(frame)->pos - start;
		WVector cup = GetPinPos() - cur.PreData(frame)->pos;
		m_f6e8 = sqrtf(delta.x * delta.x + delta.z * delta.z) * 0.3125f;
		m_f6ec = sqrtf(cup.x * cup.x + cup.y * cup.y + cup.z * cup.z) * 0.3125f;
	}
}

void CPangInfo::ProcessTotal(float dt)
{
	if ((m_flags & 0x10) == 0)
	{
		m_slot[0].field14 = dt + m_slot[0].field14;
		if (m_slot[0].state != 2 || m_slot[0].field14 <= 0.15f)
		{
			if (m_slot[0].state == 4 && 0.1f < m_slot[0].field14)
			{
				m_slot[0].state = 8;
				m_slot[0].field14 = 0;
			}
		}
		else
		{
			m_slot[0].state = 4;
			m_slot[0].field14 = 0;
		}
		m_slot[1].field14 = dt + m_slot[1].field14;
		if (m_slot[1].state != 2 || m_slot[1].field14 <= 0.15f)
		{
			if (m_slot[1].state == 4 && 0.1f < m_slot[1].field14)
			{
				m_slot[1].state = 8;
				m_slot[1].field14 = 0;
			}
		}
		else
		{
			m_slot[1].state = 4;
			m_slot[1].field14 = 0;
		}
		m_slot[2].field14 = dt + m_slot[2].field14;
		if (m_slot[2].state != 2 || m_slot[2].field14 <= 0.15f)
		{
			if (m_slot[2].state == 4 && 0.1f < m_slot[2].field14)
			{
				m_slot[2].state = 8;
				m_slot[2].field14 = 0;
			}
		}
		else
		{
			m_slot[2].state = 4;
			m_slot[2].field14 = 0;
		}
		m_slot[3].field14 = dt + m_slot[3].field14;
		if (m_slot[3].state != 2 || m_slot[3].field14 <= 0.15f)
		{
			if (m_slot[3].state == 4 && 0.1f < m_slot[3].field14)
			{
				m_slot[3].state = 8;
				m_slot[3].field14 = 0;
			}
		}
		else
		{
			m_slot[3].state = 4;
			m_slot[3].field14 = 0;
		}
	}
}

void CPangInfo::ProcessHappyBonus(float dt)
{
	if ((m_flags & 8) != 0)
	{
		m_bonusTime[0] = dt + m_bonusTime[0];
	}
}

void CPangInfo::ProcessAztecBonus(float dt)
{
	if ((m_flags & 8) != 0)
	{
		m_bonusTime[1] = dt + m_bonusTime[1];
	}
}

void CPangInfo::ProcessAngelBonus(float dt)
{
	if ((m_flags & 8) != 0)
	{
		m_bonusTime[2] = dt + m_bonusTime[2];
	}
}

void CPangInfo::ProcessDevilBonus(float dt)
{
	if ((m_flags & 8) != 0)
	{
		m_bonusTime[3] = dt + m_bonusTime[3];
	}
}

void CPangInfo::CheckReadyToMakeDecision()
{
	int i = 0;
	if (GolfBall().m_state == 0 && (m_flags & 8) == 0)
	{
		if ((m_flags & 4) == 0)
		{
			m_flags |= 4;
			sPangCase* c = m_case;
			do
			{
				if ((c->flags & 0x1007) != 0)
				{
					m_flags &= ~4;
					return;
				}
				i++;
				c++;
			} while (i < 25);
			return;
		}
		m_f754 = 0;
		m_flags |= 8;
		if (m_case[17].flags != 0x100)
		{
			m_bonusTime[1] = 0;
			m_f754 = 1;
			AfxGetTask()->GetActor("Screen")
				<< MsgObject(NULL, M983(0x1c3), 0, 0, 0, 0, 0);
		}
		if (m_case[19].flags != 0x100)
		{
			m_f754++;
			m_bonusTime[2] = 0;
			AfxGetTask()->GetActor("Screen")
				<< MsgObject(NULL, M983(0x1c3), 0, 0, 0, 0, 0);
		}
		if (m_case[20].flags != 0x100)
		{
			m_f754++;
			m_bonusTime[3] = 0;
			AfxGetTask()->GetActor("Screen")
				<< MsgObject(NULL, M983(0x1c3), 0, 0, 0, 0, 0);
		}
		if (m_case[13].flags != 0x100)
		{
			m_f754++;
			m_bonusTime[0] = 0;
			AfxGetTask()->GetActor("Screen")
				<< MsgObject(NULL, M983(0x1c3), 0, 0, 0, 0, 0);
		}
	}
}

void CPangInfo::CheckReadyToFinalize(float dt)
{
	if (GolfBall().m_state == 0 && (m_flags & 0x10) == 0 &&
		(m_case[17].flags == 0x100 || s_minDistance <= m_bonusTime[1]) &&
		(m_case[19].flags == 0x100 || s_minDistance <= m_bonusTime[2]) &&
		(m_case[20].flags == 0x100 || s_minDistance <= m_bonusTime[3]) &&
		(m_case[13].flags == 0x100 || s_minDistance <= m_bonusTime[0]))
	{
		if (m_f750 == 0.0f)
		{
			for (int i = 0; i < 25; i++)
			{
				int flags = m_case[i].flags;
				if (flags != 0x80 && flags != 0x100 && flags != 0x2000)
				{
					return;
				}
			}
		}
		float time = m_f750;
		m_f750 = dt + time;
		if (1.5f < dt + time)
		{
			m_flags |= 0x10;
		}
	}
}

void CPangInfo::Insert(int index)
{
	float baseY = PangInfoBaseY983();
	int level = -1;
	if (index == 0)
	{
		level = 0;
		m_case[0].field10 = 0;
	}
	else
	{
		if (index == 9)
		{
			if (m_case[12].flags == 0x100)
				goto bonus;
			level = m_case[12].field10;
			m_case[9].field10 = level;
		}
		else
		{
			if (index != 10 && index != 11)
			{
				if (index == 23 || index == 24)
				{
					m_case[index].field10 = m_f6f0;
					goto done;
				}
bonus:
				if (m_case[15].flags == 0x100 || index == 14 || index == 15)
				{
					if ((m_case[23].flags & 0x101) == 0)
						level = m_case[23].field10;
					if ((m_case[24].flags & 0x101) == 0)
					{
						if (level < 0)
							level = m_case[24].field10;
						else if (m_case[24].field10 <= level)
							level = m_case[24].field10;
					}
					if (level < 0)
					{
						m_case[index].field10 = m_f6f0;
						goto check;
					}
				}
				else
				{
					level = m_case[15].field10;
				}
				goto store;
			}
			int base = m_case[9].field10;
			level = base + 1;
store:
			m_case[index].field10 = level;
		}
check:
		if (level < 0)
			goto done;
	}
	for (int i = 0; i < 25; ++i)
	{
		if (i != index && (m_case[i].flags & 0x101) == 0 &&
			m_case[i].field10 >= level)
		{
			m_case[i].flags |= 0x1000;
			++m_case[i].field10;
			m_case[i].field1C =
				baseY - (float)m_case[i].field10 * 20.0f;
		}
	}
done:
	++m_f6f0;
	m_case[index].flags = 2;
	m_case[index].field18 =
		baseY - (float)m_case[index].field10 * 20.0f;
	if (index == 13 || index == 17 || index == 19)
	{
		m_f6f8 = (float)floor((double)((g_view->GetWidth() - 10.0f) - 192.6f));
		m_f6fc = m_case[index].field18 - 13.44f;
	}
}

void CPangInfo::MakeDecision(int index, bool bonus)
{
	m_case[index].flags = ((-(unsigned int)bonus & 0xffffffd0) + 0x40) |
		(m_case[index].flags & 0xfffff000);
	m_case[index].field14 = 0;
}

void CPangInfo::SuperPangya()
{
	if (m_case[1].flags != 0x100 && m_case[1].flags == 1)
	{
		Insert(1);
	}
}

namespace
{
	LOCAL_INLINE unsigned char GameType()
	{
		return CSharedDoc::Instance()->m_golfGame.gameType;
	}

}

void CPangInfo::LongDrive()
{
	if (m_case[0].flags != 0x100)
	{
		if (m_f6e0 < m_f6e8)
		{
			if (m_case[0].flags == 1)
			{
				Insert(0);
			}
			*(__int64*)&m_case[0].field8 = (int)(m_f6e8 - m_f6e0);
		}
		if ((m_flags & 4) != 0)
		{
			if (m_f6e0 < m_f6e8 && GolfBall().m_bOB == false)
			{
				MakeDecision(0, true);
				return;
			}
			MakeDecision(0, false);
		}
	}
}

void CPangInfo::Approach()
{
	if (m_case[2].flags != 0x100)
	{
		if (GolfBall().m_state == 0)
		{
			if (m_case[2].flags == 1)
			{
				Insert(2);
				int multiplier = 1;
				if (S5::CLASSICSRV::IsClassicGame983())
				{
					multiplier = 2;
				}
				bool closeRange = m_f6e4 < 60.0f;
				*(__int64*)&m_case[2].field8 =
					-((10 - float2int(m_f6ec)) * m_f6d4 * multiplier);
				if (closeRange)
				{
					*(__int64*)&m_case[2].field8 =
						(*(__int64*)&m_case[2].field8 + 1) / 2;
				}
			}
			else if ((m_flags & 4) != 0)
			{
				MakeDecision(2, true);
			}
		}
	}
}

void CPangInfo::LongPutt()
{
	if (m_case[3].flags != 0x100)
	{
		if (GolfBall().m_state != 0)
		{
			if (m_case[3].flags != 1 || GolfBall().m_playFrame < 1)
			{
				*(__int64*)&m_case[3].field8 = float2int(m_f6e8) *
					(S5::CLASSICSRV::IsClassicGame983() == 1 ? 4 : 2);
				return;
			}
			const WVector& start = GolfBall().PreData(0)->pos;
			WVector dir(GolfBall().m_pos.x - start.x, 0.0f,
				GolfBall().m_pos.z - start.z);
			NormalizeVector(dir);
			if (dir.x * m_dirX + dir.y * m_dirY + dir.z * m_dirZ <=
				cos(0.34906584f))
			{
				return;
			}
			Insert(3);
			return;
		}
		if ((m_flags & 4) != 0)
		{
			MakeDecision(3, GolfBall().IsHoleIn() != 0);
		}
	}
}


void CPangInfo::LongChipIn()
{
	if (m_case[5].flags != 0x100)
	{
		if (GolfBall().m_state == 0)
		{
			if (m_case[5].flags == 1)
			{
				m_case[5].flags = 0x100;
			}
			if ((m_flags & 4) != 0)
			{
				MakeDecision(5, GolfBall().IsHoleIn() != 0);
			}
		}
		else
		{
			if (m_case[5].flags == 1 && GolfBall().m_playFrame > 0)
			{
				const WVector& start = GolfBall().PreData(0)->pos;
				WVector dir(GolfBall().m_pos.x - start.x, 0.0f,
					GolfBall().m_pos.z - start.z);
				NormalizeVector(dir);
				if (dir.x * m_dirX + dir.y * m_dirY + dir.z * m_dirZ <=
					cos(0.34906584f))
				{
					return;
				}
				if (30.0f <= m_f6ec)
				{
					return;
				}
				Insert(5);
				return;
			}
			int base = float2int(m_f6e8);
			*(__int64*)&m_case[5].field8 =
				base * (S5::CLASSICSRV::IsClassicGame983() == 1 ? 4 : 2);
			// 983 (00536620/00536684): CClub+0x3c = m_approachMode (= PreCalc 00532b6c), nao m_pClub->type
			if ((GolfBall().m_initSpecial & 0x70) == 0 &&
				GolfClub().GetApproachMode() == 4)
			{
				*(__int64*)&m_case[5].field8 = *(__int64*)&m_case[5].field8 * 2;
			}
		}
	}
}

void CPangInfo::BeamImpact()
{
	if (m_case[6].flags != 0x100)
	{
		if (GolfBall().m_state == 0)
		{
			if (m_case[6].flags == 1)
			{
				Insert(6);
				return;
			}
			if ((m_flags & 4) != 0)
			{
				MakeDecision(6, true);
			}
		}
	}
}

void CPangInfo::SpecialShot()
{
	if (m_case[8].flags != 0x100)
	{
		if (GolfBall().m_state == 0)
		{
			if (m_case[8].flags == 1)
			{
				Insert(8);
				return;
			}
			if ((m_flags & 4) != 0)
			{
				MakeDecision(8, true);
			}
		}
	}
}

void CPangInfo::NiceRecovery()
{
	if (m_case[16].flags != 0x100)
	{
		if (m_case[16].flags == 1)
		{
			if (GolfBall().m_afterBurnerFrame < GolfBall().m_playFrame)
			{
				Insert(0x10);
				bool out = GOLFDOC()->m_pPolySoup->IsOutOfBound(
					GolfBall().PreData(GolfBall().m_afterBurnerFrame)->pos);
				m_case[16].fieldC = 0;
				if (out)
				{
					m_case[16].field8 = 0xf;
					return;
				}
				m_case[16].field8 = 10;
				return;
			}
		}
		else if ((m_flags & 4) != 0)
		{
			if (GOLFDOC()->m_pPolySoup->IsOutOfBound(
					GolfBall().PreData(GolfBall().m_afterBurnerFrame)->pos))
			{
				MakeDecision(0x10,
					!GOLFDOC()->m_pPolySoup->IsOutOfBound(
						GolfBall().PreData(GolfBall().m_renderNum)->pos));
				return;
			}
			WVector pos = GolfBall().PreData(GolfBall().m_renderNum)->pos;
			unsigned char ground = GetPVS().GetGroundType(pos, true, NULL);
			if (ground != 1 && ground != 2)
			{
				MakeDecision(0x10, false);
				return;
			}
			MakeDecision(0x10, true);
		}
	}
}

void CPangInfo::HoleIn()
{
	if (m_case[9].flags != 0x100 && GolfBall().m_state == 0)
	{
		if (m_case[9].flags == 1)
		{
			Insert(9);
			return;
		}
		if ((m_flags & 4) != 0)
		{
			unsigned char mine;
			unsigned char other;
			if (Doc()->m_golfGame.gameType == 1)
			{
				unsigned char team = PLAYER(MyIndex())->team;
				CGolfDoc* doc = GOLFDOC();
				sTeamData* otherTeam = TEAM(1 - team);
				other = otherTeam->stroke[doc->m_currentHole - 1];
				doc = GOLFDOC();
				sTeamData* myTeam = TEAM(team);
				mine = myTeam->stroke[doc->m_currentHole - 1];
			}
			else
			{
				if (Doc()->m_golfGame.gameType != 3)
				{
					m_case[9].flags = (m_case[9].flags & 0xfffff010) | 0x10;
					m_case[9].field14 = 0.0f;
					return;
				}
				CGolfDoc* doc = GOLFDOC();
				sPlayerData* otherPlayer = PLAYER(1 - MyIndex());
				other = otherPlayer->stroke[doc->m_currentHole - 1];
				doc = GOLFDOC();
				mine = PLAYER(MyIndex())->stroke[doc->m_currentHole - 1];
			}
			m_case[9].field14 = 0.0f;
			if (mine > other)
			{
				m_case[9].flags = (m_case[9].flags & 0xfffff000) | 0x40;
				return;
			}
			m_case[9].flags = (m_case[9].flags & 0xfffff010) | 0x10;
		}
	}
}

void CPangInfo::Multiply(int index, int value)
{
	if (m_case[index].flags != 0x100)
	{
		if (GolfBall().m_state == 0)
		{
			if (m_case[index].flags == 1)
			{
				Insert(index);
				*(__int64*)&m_case[index].field8 = value;
				return;
			}
			if ((m_flags & 4) != 0)
			{
				if (index == 1)
				{
					MakeDecision(1, (m_case[0].flags >> 4) & 1);
					return;
				}
				if (GameType() != 1)
				{
					if (GolfBall().IsHoleIn() && m_f6d4 < 1)
					{
						MakeDecision(index, true);
						return;
					}
					MakeDecision(index, false);
					return;
				}
				MakeDecision(index, m_case[9].flags != 0x100);
			}
		}
	}
}

void CPangInfo::ClashCombo()
{
	if (m_case[12].flags != 0x100)
	{
		if (GolfBall().m_state == 0)
		{
			if ((m_flags & 4) != 0)
			{
				if (GolfBall().IsHoleIn() && m_f6d4 < 1)
				{
					MakeDecision(0xc, true);
					return;
				}
				MakeDecision(0xc, false);
				return;
			}
		}
		else
		{
			const std::vector<CQuadTree::sColData>& colList =
				GetPVS().GetColList();
			int frame = m_f6f4;
			if (frame < GolfBall().m_playFrame)
			{
				do
				{
					std::vector<CQuadTree::sColData>::const_iterator it;
					for (it = colList.begin(); it != colList.end(); ++it)
					{
						if ((*it).objIndex != -1 && (*it).frame == frame)
						{
							__int64& combo = *(__int64*)&m_case[12].field8;
							combo = (int)combo * 2;
							if (combo > 0x10)
								combo = 0x10;
							if (combo > 1 && m_case[12].flags == 1)
							{
								Insert(0xc);
							}
							break;
						}
					}
					frame++;
				} while (frame < GolfBall().m_playFrame);
			}
		}
	}
}

void CPangInfo::DormieBonus()
{
	if (GameType() == 1 && m_case[7].flags != 0x100)
	{
		if (m_case[7].flags == 1)
		{
			Insert(7);
			return;
		}
		if ((m_flags & 4) != 0)
			MakeDecision(7, true);
	}
}

void CPangInfo::HappyBonus()
{
	switch (GameType())
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 11:
		break;
	default:
		if (GolfBall().m_state == 0 && m_case[13].flags != 0x100)
		{
			if (m_case[13].flags == 1)
			{
				Insert(0xd);
				return;
			}
			if ((m_flags & 4) != 0)
			{
				m_case[13].field8 = 0x14;
				m_case[13].fieldC = 0;
				MakeDecision(0xd, true);
			}
		}
	}
}

void CPangInfo::AztecBonusWater()
{
	if (m_case[18].flags == 0x100)
		return;
	unsigned long card = PlayerCard(GOLFDOC()->m_currentPlayer);
	__int64 bonus = 0;
	if (card == 0x14000005 || card == 0x14000080 || card == 0x14000016 ||
		card == 0x1400003c || card == 0x1400003d)
	{
		bonus = 1;
	}
	CCardManager::Instance()->SetPlayerIndex(GOLFDOC()->m_currentPlayer);
	bonus += CCardManager::Instance()->GetCardBounceBonusUp();
	if (GolfBall().m_state != 0)
	{
		for (int i = m_f6f4; i < GolfBall().m_playFrame; ++i)
		{
			if (m_f780 >= 0x14)
				break;
			if (i - m_f784 > 5)
			{
				if (GolfBall().PreData(i)->event == 4 ||
					GolfBall().PreData(i)->event == 1 ||
					GolfBall().PreData(i)->event == 2)
				{
					++m_f780;
					m_f784 = i;
				}
			}
		}
		int count = m_f780;
		if (count > 0x14)
			count = 0x14;
		__int64 gain = count * bonus;
		*(__int64*)&m_case[18].field8 = gain;
		if (gain > 0 && m_case[18].flags == 1)
			Insert(0x12);
	}
	else if ((m_flags & 4) != 0)
	{
		MakeDecision(0x12, !GolfBall().m_bOB);
	}
}

void CPangInfo::AngelWingBonus()
{
	if (m_case[19].flags != 0x100)
	{
		if (m_case[19].flags == 1)
		{
			Insert(0x13);
			return;
		}
		if ((m_flags & 4) != 0)
		{
			m_case[19].field8 = 0x37;
			m_case[19].fieldC = 0;
			MakeDecision(0x13, false);
		}
	}
}

void CPangInfo::DevilWingBonus()
{
	if (m_case[20].flags == 0x100)
	{
		return;
	}
	if (m_case[20].flags == 1)
	{
		Insert(0x14);
		return;
	}
	if ((m_flags & 4) == 0)
	{
		return;
	}
	unsigned int itemFlags = PlayerFlags(GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer));
	if (itemFlags == 0x80000)
	{
		m_case[20].field8 = 1;
		m_case[20].fieldC = 0;
	}
	else if (itemFlags == 0x4000000)
	{
		m_case[20].field8 = 10;
		m_case[20].fieldC = 0;
	}
	MakeDecision(0x14, true);
}

void CPangInfo::MascotBonus()
{
	if (m_case[21].flags != 0x100)
	{
		if (m_case[21].flags == 1)
		{
			Insert(0x15);
			return;
		}
		if ((m_flags & 4) != 0)
		{
			MakeDecision(0x15, true);
		}
	}
}

void CPangInfo::AbilityItemBonus()
{
	if (m_case[22].flags != 0x100)
	{
		if (m_case[22].flags == 1)
		{
			Insert(0x16);
			return;
		}
		if ((m_flags & 4) != 0)
		{
			MakeDecision(0x16, true);
		}
	}
}

void CPangInfo::GimmickItem(GIMMICK_ITEM_TYPE type)
{
	int index;
	if (type == 0)
	{
		index = 0x17;
	}
	else
	{
		if (type != 1)
		{
			return;
		}
		index = 0x18;
	}
	if (GolfBall().m_state == 0 && m_case[index].flags == 1)
	{
		m_case[index].flags = 0x100;
	}
	sPangCase* c = &m_case[index];
	if (c->flags != 0x100)
	{
		char* table = GroundItemTable();
		int count;
		if (table != NULL &&
			(count = *(int*)(table +
				 (type + GOLFDOC()->m_currentPlayer * 6) * 4)) > 0)
		{
			if (c->flags == 1)
			{
				Insert(index);
			}
			*(__int64*)&m_case[index].field8 = count;
		}
		if ((m_flags & 4) != 0)
		{
			MakeDecision(index, true);
		}
	}
}

void CPangInfo::AztecBonus()
{
	switch (PlayerCard(GOLFDOC()->m_currentPlayer) & 0x3ffffff)
	{
	case 0:
	case 0x2e:
	case 0x87:
	case 0xe0:
		break;
	default:
		if (m_case[17].flags == 1)
			Insert(0x11);
		else if (m_case[17].flags == 0x100)
			return;
		if ((m_flags & 4) != 0)
			MakeDecision(0x11, true);
	}
}

void CPangInfo::BonusCase4()
{
	if (m_case[4].flags != 0x100)
	{
		if (m_case[4].flags == 1)
		{
			Insert(4);
			return;
		}
		if ((m_flags & 4) != 0)
			MakeDecision(4, GolfBall().IsHoleIn() != 0);
	}
}

void CPangInfo::ClearBonus()
{
	if (m_case[14].flags != 0x100 &&
		((GOLFDOC()->m_gameMode & 0x80) == 0 || GolfBall().m_state == 0))
	{
		if (m_case[14].flags == 1)
		{
			Insert(0xe);
			return;
		}
		if ((m_flags & 4) != 0)
			MakeDecision(0xe, true);
	}
}

void CPangInfo::RecoveryBonus()
{
	if (m_case[15].flags != 0x100)
	{
		if (m_case[15].flags == 1)
		{
			Insert(0xf);
			return;
		}
		if ((m_flags & 4) != 0)
		{
			if (m_gain[0] <= 0 && m_gain[1] <= 0)
			{
				MakeDecision(0xf, false);
				return;
			}
			MakeDecision(0xf, true);
		}
	}
}

void CPangInfo::Display_TotalPang()
{
	__int64 big = m_slot[1].total;
	if (!(m_slot[0].total < big))
		big = m_slot[0].total;
	float digitsWidth;
	if (big > 10000)
		digitsWidth = 105.0f;
	else if (big > 1000)
		digitsWidth = 85.0f;
	else
		digitsWidth = 65.0f;
	int row = 0;
	for (int i = 0; i <= 1; ++i)
	{
		sPangSlot& slot = m_slot[i];
		if (i <= 0 || slot.total != 0)
		{
			float scale;
			if (slot.state == 2)
			{
				float t = slot.field14 * 0.6666666f + 0.6f;
				scale = (t > 0.7f) ? 0.7f : t;
			}
			else if (slot.state == 4)
			{
				float t = (0.15f - slot.field14) * 0.6666666f + 0.6f;
				scale = (t < 0.6f) ? 0.6f : t;
			}
			else
			{
				scale = 0.6f;
			}
			_WRECT dst;
			dst.w = scale * 256.0f;
			dst.h = scale * 32.0f;
			dst.x =
				(float)floor((double)(((g_view->GetWidth() - 10.0f) - dst.w) -
					scale * digitsWidth));
			double floorY = floor((double)(slot.field18 - dst.h * 0.7f));
			float rowY = (float)row * 20.0f;
			unsigned long color =
				((unsigned long)slot.alpha34 << 24) | 0xffffff;
			SetColor(color);
			dst.y = rowY + (float)floorY;
			if (m_overlay[slot.overlayIndex] != NULL)
				m_overlay[slot.overlayIndex]->Render(g_view, slot.rect24, dst,
					0, color, 0, 0);
			sprintf(Buffer(), "%I64d", slot.total);
			Print(g_view->GetWidth() - 10.0f, dst.y + 2.0f, Buffer(), 3.0f, 0,
				scale, NULL);
			++row;
		}
	}
}


void CPangInfo::Display_TotalGimmickNum()
{
	if (m_slot[2].total > 0 || m_slot[3].total > 0)
	{
		int base = 1;
		if (m_slot[1].total > 0)
			base = 2;
		float textX = g_view->GetWidth() - 10.0f;
		float x = textX;
		float rowY = (float)base * 20.0f;
		for (int i = 2; i < 4; ++i)
		{
			sPangSlot& slot = m_slot[i];
			float scale;
			if (slot.state == 2)
			{
				float t = slot.field14 * 0.6666666f + 0.6f;
				scale = (t > 0.7f) ? 0.7f : t;
			}
			else if (slot.state == 4)
			{
				float t = (0.15f - slot.field14) * 0.6666666f + 0.6f;
				scale = (t < 0.6f) ? 0.6f : t;
			}
			else
			{
				scale = 0.6f;
			}
			_WRECT dst;
			dst.w = scale * 256.0f;
			unsigned long color =
				((unsigned long)slot.alpha34 << 24) | 0xffffff;
			SetColor(color);
			dst.h = scale * 32.0f;
			dst.y = (float)floor((double)(slot.field18 - dst.h * 0.7f)) + rowY;
			sprintf(Buffer(), "%I64d", slot.total);
			x -= Print(x, dst.y + 2.0f, Buffer(), 3.0f, 0, scale, NULL);
			textX -= GetExactTextWidth(Buffer(), 3.0f, 0.6f);
			dst.x = (float)floor((double)((x - dst.w) - scale * 5.0f));
			if (m_overlay[slot.overlayIndex] != NULL)
				m_overlay[slot.overlayIndex]->Render(g_view, slot.rect24, dst,
					0, color, 0, 0);
			x = textX - 37.2f;
		}
	}
}

void CPangInfo::Record(unsigned char index, __int64* values)
{
	if (values[0] < 0 && -values[0] > (__int64)(int)PLAYER(index)->totalPang)
	{
		PLAYER(index)->pang[GOLFDOC()->m_currentHole - 1] =
			-(int)PLAYER(index)->totalPang;
		PLAYER(index)->totalPang = 0;
	}
	else
	{
		PLAYER(index)->pang[GOLFDOC()->m_currentHole - 1] += (int)values[0];
		PLAYER(index)->totalPang += (int)values[0];
	}
	if (Doc()->m_golfGame.gameType == 1)
	{
		TEAM(PLAYER(index)->team)->pang[GOLFDOC()->m_currentHole - 1] =
			PLAYER(index)->pang[GOLFDOC()->m_currentHole - 1];
		TEAM(PLAYER(index)->team)->totalPang = PLAYER(index)->totalPang;
		if (PLAYER(index)->teammate != 0xff)
		{
			PLAYER(PLAYER(index)->teammate)
				->pang[GOLFDOC()->m_currentHole - 1] =
				PLAYER(index)->pang[GOLFDOC()->m_currentHole - 1];
			PLAYER(PLAYER(index)->teammate)->totalPang =
				PLAYER(index)->totalPang;
		}
	}
	PLAYER(index)->bonusPang += (int)values[1];
	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 0xb: // 983 (0052f0d0): tiki tambem sem bonus dos outros
		break;
	default:
		if ((PlayerByte(PLAYER(GOLFDOC()->m_currentPlayer), 0x4e1) & 0x10) != 0)
		{
			for (int i = 0; i < GOLFDOC()->m_playerNum; ++i)
			{
				if (PLAYER(i)->state != 3 && i != GOLFDOC()->m_currentPlayer)
				{
					PLAYER(i)->bonusPang += 20;
				}
			}
			if (Doc()->m_golfGame.gameType == 1)
			{
				TEAM(1 - PLAYER(index)->team)->bonusPang += 20;
			}
		}
		if ((PlayerByte(PLAYER(GOLFDOC()->m_currentPlayer), 0x4e1) & 0x40) != 0)
		{
			for (int i = 0; i < GOLFDOC()->m_playerNum; ++i)
			{
				if (PLAYER(i)->state != 3 && i != GOLFDOC()->m_currentPlayer)
				{
					PLAYER(i)->bonusPang += 5;
				}
			}
			if (Doc()->m_golfGame.gameType == 1)
			{
				TEAM(1 - PLAYER(index)->team)->bonusPang += 5;
			}
		}
		if ((PlayerByte(PLAYER(GOLFDOC()->m_currentPlayer), 0x4e2) & 4) != 0)
		{
			for (int i = 0; i < GOLFDOC()->m_playerNum; ++i)
			{
				if (PLAYER(i)->state != 3 && i != GOLFDOC()->m_currentPlayer)
				{
					int level = PLAYER(i)->level;
					int bonus = 0;
					if (level <= 5)
					{
						bonus = 7;
					}
					else if (level >= 6 && level <= 10)
					{
						bonus = 5;
					}
					else if (level >= 11 && level <= 20)
					{
						bonus = 3;
					}
					else if (level >= 21)
					{
						bonus = 1;
					}
					PLAYER(i)->bonusPang += bonus;
				}
			}
		}
		if ((PlayerByte(PLAYER(GOLFDOC()->m_currentPlayer), 0x4e2) & 8) != 0)
		{
			for (int i = 0; i < GOLFDOC()->m_playerNum; ++i)
			{
				GOLFDOC()->GetPlayer(i);
			}
		}
		if ((PlayerByte(PLAYER(GOLFDOC()->m_currentPlayer), 0x4e3) & 2) != 0)
		{
			for (int i = 0; i < GOLFDOC()->m_playerNum; ++i)
			{
				if (PLAYER(i)->state != 3 && i != GOLFDOC()->m_currentPlayer)
				{
					PLAYER(i)->bonusPang += 10;
				}
			}
		}
		if (IsLocalContent((localContentType_t)3))
		{
			if ((PlayerByte(PLAYER(GOLFDOC()->m_currentPlayer), 0x4e2) &
					0x10) != 0)
			{
				for (int i = 0; i < GOLFDOC()->m_playerNum; ++i)
				{
					GOLFDOC()->GetPlayer(i);
				}
			}
			else if ((PlayerByte(PLAYER(GOLFDOC()->m_currentPlayer), 0x4e2) &
						 0x20) != 0)
			{
				for (int i = 0; i < GOLFDOC()->m_playerNum; ++i)
				{
					GOLFDOC()->GetPlayer(i);
				}
			}
		}
		if ((PlayerByte(PLAYER(GOLFDOC()->m_currentPlayer), 0x4e2) & 0x40) != 0)
		{
			for (int i = 0; i < GOLFDOC()->m_playerNum; ++i)
			{
				GOLFDOC()->GetPlayer(i);
			}
		}
		else if ((PlayerByte(PLAYER(GOLFDOC()->m_currentPlayer), 0x4e2) &
					 0x80) != 0)
		{
			for (int i = 0; i < GOLFDOC()->m_playerNum; ++i)
			{
				GOLFDOC()->GetPlayer(i);
			}
		}
		break;
	}
	if (Doc()->m_golfGame.gameType == 1)
	{
		TEAM(PLAYER(index)->team)->bonusPang = PLAYER(index)->bonusPang;
		if (PLAYER(index)->teammate != 0xff)
		{
			PLAYER(PLAYER(index)->teammate)->bonusPang =
				PLAYER(index)->bonusPang;
		}
	}
}

void CPangInfo::Restore()
{
	m_flags = 2;
	m_f784 = -1;
	m_f6f0 = 0;
	m_sound = NULL;
	m_f6f4 = 0;
	m_f780 = 0;
	m_slot[0].total = m_pang[0];
	m_shown[0] = m_slot[0].total;
	m_slot[1].total = m_pang[1];
	m_shown[1] = m_slot[1].total;
	m_slot[0].state = 8;
	m_slot[1].state = 8;
	m_slot[2].total = m_pang[2];
	m_slot[3].total = m_pang[3];
	m_slot[2].state = 8;
	m_slot[3].state = 8;
	m_shown[2] = m_slot[2].total;
	m_shown[3] = m_slot[3].total;
	m_slot[0].field14 = 0;
	m_slot[1].field14 = 0;
	m_slot[2].field14 = 0;
	m_slot[3].field14 = 0;
	m_f750 = 0;
	for (int i = 0; i < 25; i++)
	{
		if (m_case[i].flags != 0x100)
		{
			m_case[i].flags = 1;
		}
		m_case[i].field10 = -1;
		m_case[i].field14 = 0;
		m_case[i].flag34 = 0;
		m_applied[i] = 0;
	}
	// 983 (00526e22): this+0x448/0x44c = m_case[12].field8/fieldC; o 0x3f8/0x3fc do 645 cai no meio do m_case[10]
	C_ASSERT(sizeof(sPangCase) == 0x38);
	m_case[12].field8 = 1;
	m_case[12].fieldC = 0;
}

void CPangInfo::Skip()
{
	if (m_sound != NULL)
	{
		g_audio->StopSfx(m_sound);
	}
	int value = (int)m_pang[0] + (int)m_gain[0];
	if (value < 1)
	{
		value = 0;
	}
	m_slot[0].total = value;
	value = (int)m_pang[1] + (int)m_gain[1];
	if (value < 1)
	{
		value = 0;
	}
	m_slot[1].total = value;
	value = (int)m_pang[2] + (int)m_gain[2];
	if (value < 1)
	{
		value = 0;
	}
	m_slot[2].total = value;
	value = (int)m_pang[3] + (int)m_gain[3];
	if (value < 1)
	{
		value = 0;
	}
	m_slot[3].total = value;
}

#include "wutil.h"

// B1478: 983 crus GOLFDOC()+0xe0/0xe4/0xe8/0x100 = m_bar.power/impact/gauge/calipers (VC10); no cl 14.44 m_bar = 0xb4
// (std::string/std::list antes) e o cru lia/gravava fora da barra
C_ASSERT_VC10(offsetof(CGolfDoc, m_bar) == 0xe0 && offsetof(CGolfDoc, m_bar.gauge) == 0xe8 && offsetof(CGolfDoc, m_bar.calipers) == 0x100);

C_ASSERT(sizeof(CHoleRuler983) == 0x18);

// 983 0052bf60: so com o modo 0x200; camera a ate 128 do pino (xz) e 12.8..192 acima dele. A escala sai da projecao de
// um ponto 16 a frente e 4.8 ao lado da camera, dividida pela altura da camera sobre o chao (guardado por x/z da camera).
void CHoleRuler983::Display(WView* view)
{
	if ((GOLFDOC()->m_gameMode & 0x200) == 0)
		return;
	const WMatrix& cam = view->camera;
	const WVector& pin = GOLFDOC()->m_pHoleData[GOLFDOC()->m_currentHole - 1].pin;
	WVector d(cam.pivot.x - pin.x, cam.pivot.y - pin.y, cam.pivot.z - pin.z);
	float dist = sqrtf(d.x * d.x + d.z * d.z);
	if (!(dist <= 128.0f) || !(d.y >= (double)12.8f) || !(d.y <= 192.0f))
		return;
	if (!m_bValid || cam.pivot.x != m_x || cam.pivot.z != m_z)
	{
		m_x = cam.pivot.x;
		m_z = cam.pivot.z;
		m_ground = GetPVS().GetGroundPoint(cam.pivot, true, NULL, true);
		m_bValid = true;
	}
	WVector h(cam.pivot.x - m_ground.x, cam.pivot.y - m_ground.y, cam.pivot.z - m_ground.z);
	WVector side = cam.xa * 4.8f;
	WVector ahead = cam.za * 16.0f;
	WVector p = cam.pivot + ahead + side;
	WTVertex v;
	view->Projection2(&v, p);
	double px = (v.sx - view->m_center_x) * 16.0;
	float scale = (float)(px / h.Magnitude());
	for (unsigned int i = 0; i < 41; i++)
	{
		float len;
		if (i % 20 == 0)
			len = 22.0f;
		else if (i % 10 == 0)
			len = 16.0f;
		else if ((i & 1) == 0)
			len = 10.0f;
		else
			len = 4.0f;
		float x = (float)floor((view->m_center_x - 1.0) - ((double)i - 20.0) * scale * 0.5);
		float y = view->m_center_y - 0.5 * len;
		WOverlay::DrawBox(view, WRect(x - 1.0f, y - 1.0f, 4.0f, len + 2.0f), 0x20300000, 0xff000000, 0.001f);
		WOverlay::DrawBox(view, WRect(x, y, 2.0f, len), 0x20300000, 0xffffffff, 0.001f);
	}
}

namespace S5
{
	bool IsUsableCalipers();
}

// JP 983: CCalipers reescrita (= 839). Ver fontinfo.h.
C_ASSERT_VC10(sizeof(CCalipers::sMode) == 0x68);
C_ASSERT_VC10(sizeof(CCalipers) == 0x568);
C_ASSERT(offsetof(sPlayerData, item) == 0x4bc);
C_ASSERT_VC10(offsetof(CGolfDoc, m_shotPhase) == 0x188);

namespace
{
	// escala da vista para o tamanho da tela de 640 Ã— 480 (CUiFrameSetManager::Dim::Calc)
	LOCAL_INLINE CUiFrameSetManager::Size ViewScale983()
	{
		return CUiFrameSetManager::Size((float)(g_view->GetWidth() / 640.0), (float)(g_view->GetHeight() / 480.0));
	}
}

CCalipers::sMode::sMode(const char* name, float step, const char* format, int minPhase, bool active)
	: m_name(name), m_format(format)
{
	m_step = step;
	m_minPhase = minPhase;
	m_active = active;
}

CCalipers::sMode& CCalipers::sMode::operator=(const sMode& other)
{
	m_name.assign(other.m_name, 0, std::string::npos);
	m_step = other.m_step;
	m_format.assign(other.m_format, 0, std::string::npos);
	m_minPhase = other.m_minPhase;
	m_active = other.m_active;
	return *this;
}

CCalipers::CCalipers()
{
	Init("[font_bar.jpg");
	Buffer() = new char[32];
	m_align = 1;
	m_valign = 1;
	m_font.AddScaled(2.0f, "[font_bar_x2.jpg");
	m_font.AddScaled(3.0f, "[font_bar_x3.jpg");
	m_overlay = g_resrcmng->GetOverlay("callipers_o.png", 0);
	m_flashTime = 0.0f;
	m_moving = 0;
	SnapValue983(m_target, 0.0f, 0.0f, 1.0f, 1.0f);
	m_mouseX = 0.0f;
	m_flashing = 0;
	m_canFlash = 1;
	m_fsm.addStateTransition(0, 1, 1);
	m_fsm.addStateTransition(0, 2, 2);
	m_fsm.addStateTransition(1, 2, 0);
	m_fsm.addStateTransition(1, 0, 0);
	m_fsm.addStateTransition(2, 1, 0);
	m_fsm.addStateTransition(2, 0, 0);
	m_fsm.setCurrentState(0);
	// 0/1: sem texto; 2/3: modo 1; 4/5: modo 2 (padrao); 6/7: item 0x18000009; 8/9: S5 / modo 4 (tipo 0xb); 10/11: modo 5
	m_modes[0] = sMode("", 36.0f, "", 4, false);
	m_modes[1] = sMode("", 36.0f, "", 4, false);
	m_modes[2] = sMode("", 1.0f, "%.0fy", 3, false);
	m_modes[3] = sMode("", 1.0f, "%.1fy", 2, false);
	m_modes[4] = sMode("", 1.0f, "%.1fy", 2, false);
	m_modes[5] = sMode("", 1.0f, "%.1fy", 2, false);
	m_modes[6] = sMode("", 1.0f, "%.1fy", 2, true);
	m_modes[7] = sMode("", 1.0f, "%.1fy", 2, true);
	m_modes[8] = sMode("", 1.0f, "%.1fy", 2, true);
	m_modes[9] = sMode("", 1.0f, "%.1fy", 2, true);
	m_modes[10] = sMode("", 6.0f, "%.0fy", 3, true);
	m_modes[11] = sMode("", 1.0f, "%.1fy", 2, true);
	m_mode = NULL;
	m_modeBase = NULL;
	ResetBar();
}

CCalipers::~CCalipers()
{
	if (g_resrcmng != NULL && m_overlay != NULL)
	{
		g_resrcmng->Release(m_overlay);
		m_overlay = NULL;
	}
}

void CCalipers::SnapValue983(float& out, float value, float lo, float hi, float step)
{
	if (step > 1.0f)
	{
		value = (float)(value - 140.0);
		int istep = (int)step;
		float base = (float)((int)value / istep * istep);
		float half = (float)ceil((float)(step * 0.5));
		if (half + base > value)
		{
			value = (float)(base + 140.0);
		}
		else
		{
			value = (float)((float)((int)base + istep) + 140.0);
		}
	}
	if (lo > value)
	{
		value = lo;
	}
	else if (hi < value)
	{
		value = hi;
	}
	out = (float)floor(value);
}

float CCalipers::GetStep()
{
	if (m_mode != NULL)
	{
		return m_mode->m_step;
	}
	return 36.0f;
}

void CCalipers::SetMode983(int mode)
{
	m_modeBase = &m_modes[mode * 2];
}

void CCalipers::SetCalipersLimit(float limit)
{
	SnapValue983(m_limit, limit, (float)(GolfClub().GetNarrowAccuracy() + 140.0), 500.0f, GetStep());
	m_target = m_limit;
	unsigned int mode = *(unsigned int*)GDOCPTR983(GOLFDOC(), 0x158);
	if (mode < 0x201)
	{
		if (mode != 0x200 && mode != 8 && mode != 0x100)
		{
			return;
		}
	}
	else if (mode != 0x8000 && mode != 0x40000)
	{
		return;
	}
	if (m_visible == 1)
	{
		GOLFDOC()->m_bar.calipers = m_target;
	}
}

void CCalipers::ResetBar()
{
	m_repeat = 0.5f;
	m_prevKeys = 0;
	m_keys = 0;
	m_state = 0;
	m_prevState = 0;
	m_speed = GetStep();
	m_visible = 0;
	m_started = 0;
	m_reset = 0;
	m_mode = NULL;
}

void CCalipers::SetEvent(int input)
{
	m_fsm.stateTransition(input);
	m_prevState = m_state;
	m_state = m_fsm.getCurrentStateID();
}

void CCalipers::MoveLeft()
{
	m_moving = 1;
	if (GolfClub().GetType() == 3)
	{
		SnapValue983(m_limit, m_limit - m_speed, 144.0f, 500.0f, GetStep());
	}
	else
	{
		SnapValue983(m_limit, m_limit - m_speed, (float)(GolfClub().GetNarrowAccuracy() + 140.0), 500.0f,
			GetStep());
	}
	if (m_speed <= 15.0)
	{
		m_speed = (float)(m_speed + 0.5);
	}
	m_target = m_limit;
}

void CCalipers::MoveRight()
{
	m_moving = 1;
	SnapValue983(m_limit, m_limit + m_speed, 140.0f, 500.0f, GetStep());
	if (m_speed <= 15.0)
	{
		m_speed = (float)(m_speed + 0.5);
	}
	m_target = m_limit;
}

void CCalipers::KeyInputEnd()
{
	m_speed = GetStep();
	SetEvent(0);
	m_moving = 0;
}

void CCalipers::ExecuteKeyAction(int action)
{
	if (action != 0)
	{
		if (action == 1)
		{
			MoveLeft();
		}
		else if (action == 2)
		{
			MoveRight();
		}
	}
}

void CCalipers::ChangeKeyState()
{
	int state = m_state;
	if (state == 0)
	{
		if ((m_keys & 1) != 0)
		{
			SetEvent(1);
			m_speed = GetStep();
			return;
		}
		if ((m_keys & 2) != 0)
		{
			SetEvent(2);
		}
		m_speed = GetStep();
	}
	else if (state == 1)
	{
		if (m_prevState != 1 && (m_keys & 1) == 0)
		{
			KeyInputEnd();
		}
		if ((m_keys & 2) != 0)
		{
			SetEvent(2);
		}
	}
	else if (state == 2)
	{
		if (m_prevState != 2 && (m_keys & 2) == 0)
		{
			KeyInputEnd();
		}
		if ((m_keys & 1) != 0)
		{
			SetEvent(1);
		}
	}
}

void CCalipers::ProcKeyState(float dt)
{
	static float s_calipersHold;
	if ((*(unsigned int*)GDOCPTR983(GOLFDOC(), 0x158) & 0xfffff4f7) != 0)
	{
		return;
	}
	if (CMouseCursor::Instance()->GetArea() == 5)
	{
		m_reset = 1;
		return;
	}
	int left = g_input->Get("CALIPERS_LEFT", true);
	int right = g_input->Get("CALIPERS_RIGHT", true);
	if (left == 1)
	{
		m_keys |= 1;
	}
	if (right == 1)
	{
		m_keys |= 2;
	}
	unsigned int keys = m_keys;
	int action = 0;
	if (keys == 0 && (m_prevKeys == 1 || m_prevKeys == 2))
	{
		s_calipersHold = 0.0f;
		m_repeat = 0.35f;
	}
	else if (s_calipersHold <= m_repeat || (keys != 1 && keys != 2))
	{
		if (((keys & 1) == 0 || (keys & 2) == 0) && keys != 0)
		{
			if (s_calipersHold < s_epsilon)
			{
				action = 1;
			}
			s_calipersHold = s_calipersHold + dt;
		}
	}
	else
	{
		s_calipersHold = s_calipersHold - m_repeat;
		action = 2;
		m_repeat = 0.05f;
	}
	ChangeKeyState();
	if (action != 0)
	{
		int act = 0;
		if (action == 1)
		{
			if (m_started != 0)
			{
				ExecuteKeyAction(m_state);
				return;
			}
			m_started = 1;
			GOLFDOC()->m_bar.calipers = m_target;
			m_visible = 1;
			return;
		}
		if (action == 2)
		{
			act = m_state;
		}
		ExecuteKeyAction(act);
	}
	m_prevKeys = m_keys;
	m_keys = 0;
}

void CCalipers::Reset()
{
	m_mouseX = 0.0f;
	m_canFlash = 1;
	m_moving = 0;
	if (!IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) ||
		CSharedDoc::Instance()->m_replayPlayType != 1)
	{
		GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)->item = 0;
	}
	m_mode = NULL;
	CUiFrameSetManager::Size scl = ViewScale983();
	CUiFrameSetManager::NamedDim* gaugeY = CUiFrameSetManager::Instance()->FindNamedDim(
		GolfClub().GetType() == 3 ? "d_putter_gauge_y" : "d_driver_gauge_y");
	if (gaugeY != NULL)
	{
		CUiFrameSetManager::Dim offset;
		offset.Set(-23.0f, 4);
		float y = (float)(gaugeY->Calc(scl) + offset.Calc(scl));
		WVector mouse = MousePoint();
		if (y < mouse.y)
		{
			g_input->SetMousePoint(mouse.x, y);
		}
	}
}

void CCalipers::Process(float dt)
{
	if (IsMyTurn(false))
	{
		bool putter = GolfClub().GetType() == 3;
		if (GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)->item == 0x18000009)
		{
			m_mode = &m_modes[6 + putter];
		}
		else if (!S5::IsUsableCalipers() || CSharedDoc::Instance()->m_gameMode == 1 ||
			CSharedDoc::Instance()->m_gameMode == 2)
		{
			if (m_modeBase != NULL)
			{
				m_mode = m_modeBase + putter;
			}
		}
		else
		{
			m_mode = &m_modes[8 + putter];
		}
	}
	if (m_mode == NULL || !m_mode->m_active)
	{
		return;
	}
	ProcKeyState(dt);
	if (GOLFDOC()->m_shotPhase == 0)
	{
		CUiFrameSetManager::NamedDim* baseX = CUiFrameSetManager::Instance()->FindNamedDim("d_default_pangya_x");
		if (baseX != NULL)
		{
			CUiFrameSetManager::Size scl = ViewScale983();
			float mouseX = MousePoint().x;
			float x = (float)(UnscaleByViewHeight983(g_view, (float)(mouseX - baseX->Calc(scl))) + 140.0);
			if (CMouseCursor::Instance()->GetArea() == 5)
			{
				float low = GolfClub().GetType() == 3 ? 140.0f : (float)(GolfClub().GetNarrowAccuracy() + 140.0);
				if (low < x)
				{
					SnapValue983(m_target, x, (float)(GolfClub().GetNarrowAccuracy() + 140.0), 500.0f, GetStep());
					m_limit = m_target;
					m_mouseX = m_target;
					GOLFDOC()->m_bar.calipers = m_target;
					CMouseCursor::Instance()->SetCursor(7);
					m_moving = 0;
				}
			}
			else
			{
				if (m_reset == 1)
				{
					GOLFDOC()->m_bar.calipers = 0.0f;
					m_started = 0;
					m_reset = 0;
					m_visible = 0;
				}
				if (m_moving != 0)
				{
					GOLFDOC()->m_bar.calipers = m_target;
				}
			}
		}
	}
	if (m_overlay == NULL)
	{
		return;
	}
	if (GOLFDOC()->m_bar.calipers > 140.0f && m_target > 140.0f && m_target - 5.0 < GOLFDOC()->m_bar.gauge &&
		GOLFDOC()->m_bar.gauge <= m_target + 5.0)
	{
		if (m_flashing == 0 && m_canFlash != 0 &&
			((GOLFDOC()->m_shotPhase == 2 && *(int*)GDOCPTR983(GOLFDOC(), 0xf8) == -1) ||
				GolfClub().GetType() == 3))
		{
			m_flashTime = 0.0f;
			m_flashing = 1;
			m_canFlash = 0;
			g_audio->PlaySfx("\xb9\xf6\xc6\xb0\xc5\xac\xb8\xaf", NULL, 0, NULL, NULL, 0.5f, 200.0f);
		}
	}
	if (m_flashing == 0)
	{
		return;
	}
	m_flashTime = dt + m_flashTime;
	if ((double)0.4f < m_flashTime)
	{
		m_flashing = 0;
		m_flashTime = 0.0f;
	}
}

void CCalipers::DrawShotPowerRange(float& x, bool limit)
{
	x = limit ? GOLFDOC()->m_bar.calipers : GOLFDOC()->m_bar.power;
	if (GOLFDOC()->m_bar.impact > x)
	{
		x = m_limit;
	}
	float dist;
	if (x <= 500.0)
	{
		dist = (float)((x - 140.0) * GolfClub().GetRange() / s_barWidth);
	}
	else
	{
		x = 500.0f;
		dist = GolfClub().GetRange();
	}
	sprintf(Buffer(), m_mode->m_format.c_str(), (double)dist);
	SetColor(0xff1afd00);
	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	CUiFrameSetManager::NamedDim* baseX = ui->FindNamedDim("d_default_pangya_x");
	CUiFrameSetManager::NamedDim* rangeY = ui->FindNamedDim(
		GolfClub().GetType() == 3 ? "d_putter_calipers_range_y" : "d_driver_calipers_range_y");
	if (baseX != NULL && rangeY != NULL)
	{
		CUiFrameSetManager::Size scl = ViewScale983();
		float offset = ScaleByViewHeight983(g_view, (float)(x - 140.0));
		PrintScaled(4, baseX->Calc(scl) + offset, rangeY->Calc(scl), Buffer(), 1.0f, 1.0f, 0);
	}
}

void CCalipers::DrawLimitLine(float& x, bool line)
{
	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	CUiFrameSetManager::Size scl = ViewScale983();
	CUiFrameSetManager::NamedDim* baseX = ui->FindNamedDim("d_default_pangya_x");
	CUiFrameSetManager::NamedDim* centerX = ui->FindNamedDim("d_calipers_center_x");
	if (baseX == NULL || centerX == NULL)
	{
		return;
	}
	float offset = ScaleByViewHeight983(g_view, (float)(x - 140.0));
	centerX->Set(baseX->Calc(scl) + offset);
	bool putter = GolfClub().GetType() == 3;
	bool hilite = (*(int*)GDOCPTR983(GOLFDOC(), 0xf8) != 0 || putter) && m_flashing != 0 && m_flashTime >= 0.1 &&
		m_flashTime < (double)0.2f;
	const char* name;
	if (putter)
	{
		if (hilite)
		{
			name = line ? "f_putter_calipers_indicator_and_line_hilite" : "f_putter_calipers_indicator_hilite";
		}
		else
		{
			name = line ? "f_putter_calipers_indicator_and_line" : "f_putter_calipers_indicator";
		}
	}
	else if (hilite)
	{
		name = line ? "f_driver_calipers_indicator_and_line_hilite" : "f_driver_calipers_indicator_hilite";
	}
	else
	{
		name = line ? "f_driver_calipers_indicator_and_line" : "f_driver_calipers_indicator";
	}
	ui->DrawFrameSet(name, g_view, 0, 0xffffffff);
}

void CCalipers::Display()
{
	if (m_mode == NULL)
	{
		return;
	}
	bool active = m_mode->m_active;
	if ((active && m_visible == 1) || (int)m_mode->m_minPhase <= (int)GOLFDOC()->m_shotPhase)
	{
		float x = 0.0f;
		DrawShotPowerRange(x, m_mode->m_active && m_visible == 1);
		DrawLimitLine(x, m_mode->m_active);
	}
	if (!m_mode->m_active)
	{
		return;
	}
	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	ui->DrawFrameSet("f_calipers_key_info", g_view, 0, 0xffffffff);
	if (m_visible != 1 || m_overlay == NULL)
	{
		return;
	}
	float mouseX = MousePoint().x;
	float low = GolfClub().GetType() == 3 ? 140.0f : (float)(GolfClub().GetNarrowAccuracy() + 140.0);
	if (mouseX <= low)
	{
		return;
	}
	if (g_input->Get("CALIPERS_LEFT", true) != 0 || m_mouseX - m_target > 0.0)
	{
		ui->DrawFrameSet("f_calipers_left_key_pressed", g_view, 0, 0xffffffff);
	}
	if (g_input->Get("CALIPERS_RIGHT", true) != 0 || (m_mouseX > 0.0f && m_mouseX - m_target < 0.0))
	{
		ui->DrawFrameSet("f_calipers_right_key_pressed", g_view, 0, 0xffffffff);
	}
}

CHolePoseSkins::CHolePoseSkins()
{
	Init("[bigfont.jpg");
	SetColor(0xffffffff);
	m_align = 2;
	Buffer() = new char[32];
	m_count = 0;
	m_pose = 0;
	m_alpha = 0.0f;
	m_state = 0;
	m_delay = 2.0f;
	m_win[0] = g_resrcmng->GetOverlay("skins_win01.jpg", 0);
	m_win[1] = g_resrcmng->GetOverlay("skins_win02.jpg", 0);
	m_win[2] = g_resrcmng->GetOverlay("skins_win03.jpg", 0);
	m_lose[0] = g_resrcmng->GetOverlay("skins_lose01.jpg", 0);
	m_lose[1] = g_resrcmng->GetOverlay("skins_lose02.jpg", 0);
	m_draw[0] = g_resrcmng->GetOverlay("skins_draw01.jpg", 0);
	m_draw[1] = g_resrcmng->GetOverlay("skins_draw02.jpg", 0);
}

CHolePoseSkins::~CHolePoseSkins()
{
	if (g_resrcmng != NULL)
	{
		if (m_win[0] != NULL)
		{
			g_resrcmng->Release(m_win[0]);
			m_win[0] = NULL;
		}
		if (g_resrcmng != NULL)
		{
			if (m_win[1] != NULL)
			{
				g_resrcmng->Release(m_win[1]);
				m_win[1] = NULL;
			}
			if (g_resrcmng != NULL)
			{
				if (m_win[2] != NULL)
				{
					g_resrcmng->Release(m_win[2]);
					m_win[2] = NULL;
				}
				if (g_resrcmng != NULL)
				{
					if (m_lose[0] != NULL)
					{
						g_resrcmng->Release(m_lose[0]);
						m_lose[0] = NULL;
					}
					if (g_resrcmng != NULL)
					{
						if (m_lose[1] != NULL)
						{
							g_resrcmng->Release(m_lose[1]);
							m_lose[1] = NULL;
						}
						if (g_resrcmng != NULL)
						{
							if (m_draw[0] != NULL)
							{
								g_resrcmng->Release(m_draw[0]);
								m_draw[0] = NULL;
							}
							if (g_resrcmng != NULL && m_draw[1] != NULL)
							{
								g_resrcmng->Release(m_draw[1]);
								m_draw[1] = NULL;
							}
						}
					}
				}
			}
		}
	}
}

void CHolePoseSkins::Process(float dt)
{
	if ((*((unsigned char*)GDOCPTR983(GOLFDOC(), 0x159)) & 0x10) == 0)
	{
		m_state = 0;
		return;
	}
	switch (m_state)
	{
	case 1:
		if (m_delay > 0.0f)
		{
			float delay = m_delay;
			m_delay = delay - dt;
			if (delay - dt < 0.0f)
				m_state = 2;
		}
		break;
	case 2:
		m_alpha = m_alpha - dt * 510.0f;
		if (m_alpha < 0.0f)
		{
			m_alpha = 0.0f;
			m_state = 0;
		}
		break;
	}
}

void CHolePoseSkins::SetPose(int pose, int count)
{
	if (m_state == 0)
	{
		m_pose = pose;
		m_count = count;
		m_alpha = 255.0f;
		m_state = 1;
		m_delay = 2.0f;
		if (pose > 0)
		{
			m_poseA = m_win[0];
			return;
		}
		if (pose == 0)
		{
			m_poseA = m_draw[0];
			m_poseB = m_draw[1];
			return;
		}
		if (pose < 0)
		{
			m_poseA = m_lose[0];
			m_poseB = m_lose[1];
		}
	}
}

void CHolePoseSkins::Display()
{
	if (m_state != 0)
	{
		WOverlay* overlay = m_poseA;
		_WRECT src;
		src.x = 0.0f;
		src.y = 0.0f;
		src.w = 1.0f;
		src.h = 1.0f;
		int alpha = float2int(m_alpha);
		unsigned long color = alpha * 0x1000000 + 0xffffff;
		_WRECT dst;
		if (overlay != NULL)
		{
			dst.w = (float)overlay->GetWidth();
			dst.h = (float)overlay->GetHeight();
			dst.x = g_view->GetWidth() * 0.5f - dst.w * 0.5f;
			dst.y = g_view->GetHeight() * 0.25f - dst.h * 0.5f;
			overlay->Render(g_view, src, dst, 0, color, 0, 0);
		}
		if (m_pose < 1)
		{
			overlay = m_poseB;
			if (overlay == NULL)
				return;
			dst.w = (float)overlay->GetWidth();
			dst.h = (float)overlay->GetHeight();
			dst.x = g_view->GetWidth() * 0.5f - dst.w * 0.5f;
			dst.y = dst.y + dst.h + 50.0f;
		}
		else
		{
			sprintf(Buffer(), "%d", m_count);
			int length = (int)strlen(Buffer());
			int digitsWidth = m_font.pInfo->fontw * length;
			float textWidth = (float)length * 6.0f + (float)digitsWidth * 0.5f;
			overlay = m_win[1];
			if (overlay != NULL)
			{
				dst.w = (float)overlay->GetWidth();
				dst.h = (float)overlay->GetHeight();
				dst.x = (g_view->GetWidth() * 0.5f - textWidth * 0.5f) - dst.w;
				dst.y = dst.y + dst.h + 50.0f;
				overlay->Render(g_view, src, dst, 0, color, 0, 0);
			}
			m_align = 1;
			float textY = dst.y + 3.0f;
			SetColor(((int)m_alpha << 24) | 0xffde44);
			Print(g_view->GetWidth() * 0.5f + 5.0f, textY, Buffer(), 3.0f, 0,
				1.0f, NULL);
			overlay = m_win[2];
			if (overlay == NULL)
				return;
			dst.w = (float)overlay->GetWidth();
			dst.h = (float)overlay->GetHeight();
			dst.x = (textWidth + g_view->GetWidth()) * 0.5f;
		}
		overlay->Render(g_view, src, dst, 0, color, 0, 0);
	}
}

__declspec(dllimport) __forceinline unsigned char GolfDocPlayerNum(
	CGolfDoc* doc)
{
	return doc->m_playerNum;
}

CExtraHoleInfo::CExtraHoleInfo()
{
	Init("[font_hole.jpg");
	SetColor(0xffffffff);
	m_align = 0;
	Buffer() = new char[32];
	m_align = 2;
	for (unsigned char i = 0; i < GolfDocPlayerNum(GOLFDOC()); ++i)
	{
		m_shotTime[i] = 0;
		m_holeTime[i] = 0;
	}
	m_ui = new CSimpleUI("[nplayer.jpg");
	m_ui->Add("1", WRect(0, 0, 32, 16), false);
	m_ui->Add("2", WRect(32, 0, 32, 16), false);
	m_ui->Add("3", WRect(0, 16, 32, 16), false);
	m_ui->Add("4", WRect(32, 16, 32, 16), false);
	m_leader = 0xff;
}

CExtraHoleInfo::~CExtraHoleInfo()
{
	if (m_ui != NULL)
	{
		delete m_ui;
		m_ui = NULL;
	}
}

void CExtraHoleInfo::SetShotTime(unsigned long time)
{
	m_shotTime[GOLFDOC()->m_currentPlayer] = time;
}

void CExtraHoleInfo::UpdateLeader()
{
	if (*GDOCPTR983(GOLFDOC(), 0x194) != 0)
	{
		WVector pin = GOLFDOC()->GetHoleData().pin;
		unsigned char player = GOLFDOC()->m_currentPlayer;
		WVector& pos = GOLFDOC()->GetPlayer(player)->pos;
		float dx = pos.x - pin.x;
		float dz = pos.z - pin.z;
		float yards = sqrtf(dx * dx + dz * dz) * 0.3125f;
		m_holeTime[GOLFDOC()->m_currentPlayer] =
			(int)floor((double)(yards * 100.0f));
		unsigned char leader = m_leader;
		if (leader == 0xff)
		{
			m_leader = GOLFDOC()->m_currentPlayer;
			return;
		}
		unsigned char current = GOLFDOC()->m_currentPlayer;
		if (m_holeTime[current] < m_holeTime[leader] ||
			(m_holeTime[current] == m_holeTime[leader] &&
				m_shotTime[current] < m_shotTime[leader]))
			m_leader = current;
	}
}

C_ASSERT_VC10(offsetof(CGolfDoc, m_bNoStat) == 0x194); // A749: ex-cru *((char*)golfDoc + 0x194)

void CExtraHoleInfo::Display()
{
	CGolfDoc* golfDoc = GOLFDOC();
	if (golfDoc->m_bNoStat)
	{
		for (unsigned char i = 0; i < golfDoc->m_playerNum; ++i)
		{
			if (GOLFDOC()->GetPlayer(i)->state != 3)
			{
				if (m_ui != NULL)
				{
					m_ui->SetColor(GOLFDOC()->GetPlayer(i)->color | 0xff000000);
					m_ui->Render(MakeStr("%d", i + 1),
						g_view->GetWidth() - 161.0f, (float)i * 23.0f + 213.0f,
						1.0f, 0);
				}
				if (GOLFDOC()->GetPlayer(i)->state == 1)
					SetColor(0xffff0a0a);
				else
					SetColor(
						(-(unsigned int)(i != m_leader) & 0x2dff) - 0x2e00);
				sPlayerData* player = GOLFDOC()->GetPlayer(i);
				if (player->stroke[GOLFDOC()->m_currentHole - 1] ==
					(unsigned char)0)
				{
					float y = (float)i * 23.0f + 213.0f;
					Print(g_view->GetWidth() - 85.0f, y, "-", 3.0f, 0, 1.0f,
						NULL);
					Print(g_view->GetWidth() - 25.0f, y, "-", 3.0f, 0, 1.0f,
						NULL);
				}
				else
				{
					unsigned int index = i;
					float y = (float)index * 23.0f + 213.0f;
					const char* text;
					float spacing;
					float x;
					if (m_holeTime[index] == 0)
					{
						spacing = 3.0f;
						x = g_view->GetWidth() - 85.0f;
						text = "-";
					}
					else
					{
						x = g_view->GetWidth();
						spacing = 0.0f;
						text = MakeStr("%d.%dy", m_holeTime[index] / 100,
							m_holeTime[index] % 100);
						x = x - 65.0f;
					}
					Print(x, y, text, spacing, 0, 1.0f, NULL);
					if (m_shotTime[index] == 0)
					{
						spacing = 3.0f;
						x = g_view->GetWidth() - 25.0f;
						text = "-";
					}
					else
					{
						x = g_view->GetWidth();
						spacing = 0.0f;
						text = MakeStr("%d.%03d", m_shotTime[index] / 1000,
							m_shotTime[index] % 1000);
						x = x - 5.0f;
					}
					Print(x, y, text, spacing, 0, 1.0f, NULL);
				}
			}
			golfDoc = GOLFDOC();
		}
	}
}


CFontOverlay::CFontOverlay()
{
	m_holeCup = NULL;
	m_fly = NULL;
	m_hole = NULL;
	m_accuracy = NULL;
	m_timeOut = NULL;
	Init();
}

CFontOverlay::~CFontOverlay()
{
	if (m_holeCup != NULL)
	{
		delete m_holeCup;
		m_holeCup = NULL;
	}
	if (m_fly != NULL)
	{
		delete m_fly;
		m_fly = NULL;
	}
	if (m_hole != NULL)
	{
		delete m_hole;
		m_hole = NULL;
	}
	if (m_accuracy != NULL)
	{
		delete m_accuracy;
		m_accuracy = NULL;
	}
	if (m_timeOut != NULL)
	{
		delete m_timeOut;
		m_timeOut = NULL;
	}
}

void CFontOverlay::Init()
{
	m_holeCup = new CHoleCupInfo;
	m_fly = new CFlyInfo;
	m_hole = new CHoleInfo;
	m_accuracy = new CAccuracyInfo;
	m_timeOut = new CTimeOutInfo;
	m_showHoleCup = false;
	m_showFly = false;
	m_showA = true;
	m_showAccuracy = true;
	m_showTimeOut = true;
}

void CFontOverlay::Process(float dt)
{
	if (m_showHoleCup && GOLFDOC()->m_tutorialMode > 3 && m_holeCup != NULL)
		m_holeCup->Process(dt);
	if (m_showFly && m_fly != NULL)
		m_fly->Process(dt);
	if ((GOLFDOC()->m_tutorialMode == 7 || GOLFDOC()->m_tutorialMode > 7) &&
		m_hole != NULL)
		m_hole->Process(dt);
	if (m_showAccuracy && m_accuracy != NULL)
		m_accuracy->Process(dt);
	if (m_showTimeOut && m_timeOut != NULL)
		m_timeOut->Process(dt);
}

void CFontOverlay::Display()
{
	if (((m_showHoleCup && GOLFDOC()->m_tutorialMode >= 4) ||
			GOLFDOC()->m_tutorialMode == 1) &&
		m_holeCup != NULL)
		m_holeCup->Display();
	if (m_showFly && m_fly != NULL)
		m_fly->Display();
	if ((GOLFDOC()->m_tutorialMode == 7 || GOLFDOC()->m_tutorialMode >= 8) &&
		m_hole != NULL)
		m_hole->Display();
	if (m_showAccuracy && m_accuracy != NULL)
		m_accuracy->Display();
	if (m_showTimeOut && m_timeOut != NULL)
		m_timeOut->Display();
}

void CFontOverlay::ResetUiPosition()
{
	if (m_holeCup != NULL && m_holeCup->m_distInfo != NULL)
		m_holeCup->m_distInfo->ResetUiPosition();
	if (m_hole != NULL)
		m_hole->ResetUiPosition();
}
