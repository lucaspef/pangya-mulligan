#include "minatl.h"
#include "projectg.h"
#include "wind.h"
#include "fx.h"
#include "quadtree.h"
#include "cardsystem.h"
#include "../shared/localize.h"
#include "s5/utilities.h"
#include "s5/classicserver/classicserver.h"
#include "slicestr.h"
#include "weather.h"
#include "jp/uiframesetmanager.h" // GetA4983 (escala da seta do vento, 006c83b0)
#include "scenemanager.h"
#include "soundmanager.h"
#include "mathconsts.h"
#include "gatewayactor.h" // s_gatewayActor (aviso 0x753b do SetSpecialWind do 983)
#include "jp/grandprixartifactrule.h" // CGrandPrixArtifactRule::Modify (00546aa0), chamada pelo Init do 983

extern WMatrix g_camera;

// These inlines lived in headers in the original, but inline bodies in PCH
// headers shift the IL offsets other units' emission order depends on.
inline void CQuadTree::SetRandSeed(unsigned long seed)
{
	m_randSeed = seed;
}

// 983: WView::GetFOV é a função 00b4fa30 (wview.cpp), sem cópia inline aqui

// HACK: CGolfDoc::GetCurrentPlayerNum is a real function of golfdoc.cpp (it returns m_currentPlayerNum);
// the original inlines the current player read here without emitting a body, and a dllimport
// __forceinline accessor is never emitted (same as tutorial.cpp).
__declspec(dllimport) __forceinline unsigned char RbCurrentPlayer(CGolfDoc* doc)
{
	return doc->m_currentPlayer;
}

// 983: a mesma conta da IsGrandPrixGameType983 (0046e6d0), em linha (o Init 006c9e20 faz movzx/add -0x14/cmp 1)
static __forceinline bool IsGrandPrixGameTypeInl983(int realGameType)
{
	return realGameType >= 0x14 && realGameType <= 0x15;
}

// 983 00e79064: pilha de CWind; o topo e o vento atual. A partida roda numa copia e o CWind base nao ve o que ela mudou
static std::vector<CWind*>& WindStack983()
{
	static std::vector<CWind*> stack;
	return stack;
}

// 983 006cc4b0
CWind& Wind()
{
	std::vector<CWind*>& stack = WindStack983();
	if (stack.empty())
		stack.push_back(new CWind);
	return *stack.back();
}

// 983 006cc5d0
void PushWind983()
{
	CWind& top = Wind();
	WindStack983().push_back(new CWind(top));
}

// 983 006cb2f0(1)
void PopWind983()
{
	std::vector<CWind*>& stack = WindStack983();
	while (stack.size() > 1)
	{
		delete stack.back();
		stack.pop_back();
	}
}

// HACK: literal placement only. "normal" must be the genuine ??_C@ COMDAT
// (frbutton/frcombobox/frgaugebar/lbaselib define it too) and sit right before
// the literals of AddWindArea, but the asm of AddWindArea cannot name a string
// literal. This inline is compiled right after Wind() and emits the literal
// there; its code is discarded at link time, because golfrulebase.obj (linked
// earlier) already has the real instantiation under the same name (wind itself
// never uses WCrypticValue<unsigned char>).
template <>
inline WCrypticValue<unsigned char>::operator unsigned char()
{
	if (0)
		Wind();
	*(const char**)this = "normal";
	return 0;
}

WVector CWind::m_netWind; // 983: não existe (PROVISÓRIO)

// 983 006cb9f0: copia tudo menos o Fx (+0x1b0). No corpo: WCrypticValue precisa do operator=, a soma depende do endereco
CWind::CWind(const CWind& src)
{
	m_spWindStartFrame = src.m_spWindStartFrame;
	m_wind = src.m_wind;
	m_intensity = src.m_intensity;
	m_direction = src.m_direction;
	m_cryptic34_983 = src.m_cryptic34_983;
	m_cryptic44_983 = src.m_cryptic44_983;
	m_b54_983 = src.m_b54_983;
	m_roll = src.m_roll;
	m_mat = src.m_mat;
	m_light = src.m_light;
	m_pitch = src.m_pitch;
	m_pitchSnapFrames983 = src.m_pitchSnapFrames983;
	m_minIntensity = src.m_minIntensity;
	m_maxIntensity = src.m_maxIntensity;
	m_bEnable = src.m_bEnable;
	m_areas = src.m_areas;
	m_globalWind = src.m_globalWind;
	m_specialWind = src.m_specialWind;
	m_specialIntensity = src.m_specialIntensity;
	m_specialDirection = src.m_specialDirection;
	m_bSpecial = src.m_bSpecial;
	m_fxMat = src.m_fxMat;
	m_fxPos = src.m_fxPos;
	m_pFxSeq = NULL;
}

CWind::CWind()
{
	// 983 (006cb590): cripticos zerados um a um, SetClassicDirection(0) e SetIntensity(0) no fim; pivot = largura / 640 * -170
	m_bEnable = true;
	m_minIntensity = 1.0f;
	m_b54_983 = false;
	m_maxIntensity = 9.0f;
	m_intensity = 0.0f;
	m_direction = 0.0f;
	m_cryptic34_983 = 0.0f;
	SetClassicDirection(0.0f);
	m_specialIntensity = -1.0f;
	m_specialDirection = 0.0f;
	m_bSpecial = false;
	m_mat = RotMat(-0.5235988f, 0);
	m_mat.pivot = (float)(g_view->GetWidth() / 640.0 * -170.0) * RotMat(-0.5235988f, 0).za;
	m_pitchSnapFrames983 = 0;
	m_light.type = 2;
	m_light.diffuse = m_light.ambient = 0xffffff;
	m_light.nearOne = WVector(0.0f, -1.0f, 1.0f).Normalize();
	m_pFxSeq = NULL;
	SetIntensity(0.0f);
}

void CWind::SetSpecialWind(bool bSpecial, bool bSilent)
{
	// 983 (006c7e40): bSilent (o bOneFrame do CQuadTree::PreProcess) só troca o estado; a cor do vento da carta e o aviso
	// 0x753b/0x17 ao Gateway vêm depois do efeito
	if (m_bSpecial == bSpecial)
		return;

	m_bSpecial = bSpecial;
	if (bSilent)
		return;

	if (bSpecial)
	{
		CFx::Instance()->UpdateExtWind(m_specialWind.Get());
		g_audio->PlaySfx("\xb7\xb9\xc0\xcc\xc1\xae\xb9\xdf\xbb\xe7", NULL, 0,
			"\xb7\xb9\xc0\xcc\xc1\xae\xb9\xdf\xbb\xe7");
		CCardManager::Instance()->SetWindColor(
			CCardManager::Instance()->GetWindPowerDown(m_specialIntensity) > 0.0f);
	}
	else
	{
		CFx::Instance()->UpdateExtWind(m_globalWind.Get());
		CCardManager::Instance()->SetWindColor(
			CCardManager::Instance()->GetWindPowerDown(m_intensity) > 0.0f);
	}
	AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 0x17, 0, 0, 0, 0);
}

void CWind::GenerateFx(bool bOn)
{
	if (!bOn)
	{
		if (m_pFxSeq)
		{
			CFx::Instance()->CloseSequence(m_pFxSeq, true); // 983 006c7720
			m_pFxSeq = NULL;
		}
	}
	else if (!m_pFxSeq)
	{
		Attacher attacher;
		attacher.mat[0] = &m_fxMat;
		attacher.pos[0] = &m_fxPos;
		m_pFxSeq = (CFxSequence*)CFx::Instance()->Attach(attacher,
			"SilviaCannon.seq", false, NULL);
		if (m_pFxSeq)
			m_pFxSeq->SetLight(g_lightset);
		g_audio->PlaySfx("\xbf\xa1\xb3\xca\xc1\xf6\xc1\xfd\xbe\xe0", NULL, 0,
			"\xbf\xa1\xb3\xca\xc1\xf6\xc1\xfd\xbe\xe0");
	}
}

namespace
{
	float UnusedZero()
	{
		return 0.0f;
	}
}


// C429: definida antes do 1o uso (declaracao antecipada static + noinline dava LNK1314 no wind.obj)
// 983 006c7fd0 (not inlined there): holeRandom of the current hole / 0x29
// 983 (006c7fd0): desvio do canhao no buraco = (holeRandom % 41 - 20) meio-graus, em radianos
__declspec(noinline) static float SpecialHoleOffset983()
{
	return (float)(((int)(Doc()->m_holeRandom[GOLFDOC()->m_currentHole - 1] % 0x29) - 20.0) *
		0.5 * g_DEGTORAD);
}

void CWind::CheckSpecialWind()
{
	WPolySoup::sArea* area = GOLFDOC()->m_pPolySoup->FindArea("silviacannon");
	if (area)
	{
		WSliceStr slice(area->name);
		char name[64];
		WVector vec;
		slice.Get("*fx", "%s %f %f %f", name, &vec.x, &vec.y, &vec.z);
		// 983 006c9ae0: o desvio do buraco (006c7fd0) entra aqui tambem, como no SetIntensity
		m_specialDirection = SpecialHoleOffset983() + vec.y;
		m_specialIntensity =
			(float)((Doc()->m_holeRandom[GOLFDOC()->m_currentHole - 1] & 3) +
				6);
		m_fxMat.Reset();
		m_fxMat.Rotate(vec);
		m_fxMat.pivot = WVector((area->aabb.max.x + area->aabb.min.x) * 0.5f,
			(area->aabb.max.y + area->aabb.min.y) * 0.5f,
			(area->aabb.max.z + area->aabb.min.z) * 0.5f);
		m_fxPos.x = m_fxPos.y = m_fxPos.z = 0.0f;
		m_specialWind = m_specialIntensity * RotMat(m_specialDirection, 1).xa;
	}
	else
	{
		m_specialIntensity = -1.0f;
	}

	m_bSpecial = false;
	if (m_pFxSeq)
	{
		if (CFx::Instance()->FindSequence(m_pFxSeq) &&
			m_pFxSeq->m_filename == "SilviaCannon.seq")
			CFx::Instance()->CloseSequence(m_pFxSeq, false);
		m_pFxSeq = NULL;
	}
}

void CWind::Init(unsigned char a, unsigned char b, unsigned char c,
	unsigned char d)
{
	// 983 (006c9e20): o ajuste de direção vai para m_cryptic34_983; m_netWind e o ramo clássico saíram; no Grand Prix o artefato
	// da sala muda a força (CGrandPrixArtifactRule::Modify)
	int seed;
	if (Doc()->m_gameMode != 5)
	{
		if (a == 0xff)
			a = rand() % 9;
		if (b == 0xff)
			b = rand() % 0xff;
		if (GOLFDOC()->m_tutorialMode < 15 && GOLFDOC()->m_tutorialMode != 8)
			a = 0;
		if (d)
		{
			m_b54_983 = false;
			seed = b;
			GetPVS().SetRandSeed(seed);
			m_intensity = a + 1.0;
			m_intensity = Between<float>(1.0f, m_intensity, 9.0f);
			m_direction = seed * g_2_PI / 255.0;
			m_cryptic34_983 = 0.0f;
		}
		else
		{
			m_intensity += (a >> 1) - 2;
			m_intensity = Between<float>(1.0f, m_intensity, 9.0f);
			m_cryptic34_983 += (b - 0x80) * (20.0 * g_DEGTORAD) / 128.0;
			if (GOLFDOC()->m_pPolySoup)
				GOLFDOC()->m_pPolySoup->UpdateBgFxWind();
			goto finish;
		}
	}
	else
	{
		if (a == 0xff || b == 0xff)
			goto finish;
		seed = b;
		GetPVS().SetRandSeed(seed);
		m_intensity = a + 1.0;
		m_intensity = Between<float>(1.0f, m_intensity, 9.0f);
		m_direction = seed * g_2_PI / 255.0;
		m_cryptic34_983 = 0.0f;
		SetClassicDirection(0.0f);
	}
	m_roll = 0.0f;
	m_pitchSnapFrames983 = 0;
	m_pitch = 0.0f;

finish:
	if (IsGrandPrixGameTypeInl983(Doc()->m_roomInfo.realGameType))
		m_intensity = _grandprix::_private::CGrandPrixArtifactRule::Instance()->Modify(
			Doc()->m_roomInfo.artifactTid983, m_intensity, a, c);
	WMatrix mat;
	RotMat(GetDirectionSum983(), 1, mat);
	float intensity = m_intensity;
	m_globalWind = mat.za * intensity;
}

void CWind::Process(float dt)
{
	if (HasSpecialWind())
	{
		// 983 (006caaf0): o volume pela distancia a camera sai uma vez so, antes dos dois sons
		WVector delta = m_fxMat.pivot - g_camera.pivot;
		float volume = Max(0.0f, (float)(1.0 - delta.Magnitude() / 960.0));
		if (g_audio->IsPlaying("\xb7\xb9\xc0\xcc\xc1\xae\xb9\xdf\xbb\xe7"))
			g_audio->SetSFXVolumeWhilePlaying(
				"\xb7\xb9\xc0\xcc\xc1\xae\xb9\xdf\xbb\xe7", volume);
		if (g_audio->IsPlaying("\xbf\xa1\xb3\xca\xc1\xf6\xc1\xfd\xbe\xe0"))
			g_audio->SetSFXVolumeWhilePlaying(
				"\xbf\xa1\xb3\xca\xc1\xf6\xc1\xfd\xbe\xe0", volume);
	}

	if (GolfBall().m_playFrame < 0)
		m_wind = GetWind(g_camera.pivot);
	else
		m_wind = GetWind(*(WVector*)&GolfBall());

	if (!(m_wind.Magnitude() < 0.1f) && m_bEnable)
	{
		// 983: sem o ramo classico (o classicserver do 983 nao tem WindRollSpeed/WindAdditionalPitchAngle*)
		m_roll += (m_wind.Magnitude() * 2.0f - 1.0f) * 0.47123894f * dt;
		if (m_roll > g_2_PI)
			m_roll -= g_2_PI;

		// 983 (006caaf0): o pitch segue o vento com passo proporcional ao erro (|delta| * 5.3407 * dt);
		// perto do alvo (<= 1 grau) conta quadros (+0xac) e, depois de 10, encaixa de uma vez
		// (o eixo da camera sai por ternario e a CalcDeltaAngle e chamada uma vez so, como no 983)
		float angle = CalcDeltaAngle(m_wind, (GOLFDOC()->m_gameMode & 0x200)
			? GOLFDOC()->m_cameraData.mat.za : g_camera.za);
		float delta2 = GetBaseAngle(angle - m_pitch);
		float absDelta = Abs(delta2);
		float step = absDelta * 5.34070780952419 * dt;
		if (absDelta > 0.017453292f)
		{
			m_pitch = GetBaseAngle(Between(-step, delta2, step) + m_pitch);
			m_pitchSnapFrames983 = 1;
		}
		else if (m_pitchSnapFrames983 && absDelta > 0.00001f)
		{
			if (++m_pitchSnapFrames983 > 10)
				m_pitch = GetBaseAngle(delta2 + m_pitch);
			else
				m_pitch = GetBaseAngle(Between(-step, delta2, step) + m_pitch);
		}
		else if (m_pitchSnapFrames983)
			m_pitchSnapFrames983 = 0;
	}
}

void CWind::SetWind(const WVector& wind)
{
	m_direction = CalcDeltaAngle(wind, WVector::UNIT_POS_Z);
	SetIntensity(wind.Magnitude());
	if (CSceneManager::Instance()->m_pWeather)
		CSceneManager::Instance()->m_pWeather->UpdateWind();
}

void CWind::PlusWind(const WVector& wind)
{
	SetWind(m_globalWind.Get() + wind);
}

// 983 (006c7e00)
const WVector& WCrypticVector983::Get()
{
	plain.Set(x, y, z);
	return plain;
}

// 983 (006c87c0)
void CWind::GetBaseWind983(WVector& wind)
{
	wind = (m_bSpecial ? m_specialWind : m_globalWind).Get();
}

// 983 (006c86f0): o "if (IsLocalContent(S4_CARD_SYSTEM) && Doc()->m_gameMode == 1)" do 645, agora também com o vento especial
void CWind::ApplyCardPowerDown983(WVector& wind)
{
	if (IsLocalContent(S4_CARD_SYSTEM) && (Doc()->m_gameMode == 1 || m_bSpecial == true))
	{
		float magnitude = wind.Magnitude();
		float power = magnitude - CCardManager::Instance()->GetWindPowerDown(magnitude);
		if (power < 1.0f)
			power = 1.0f;
		wind.Normalize();
		wind *= power;
	}
}

// 983 (006c9550): itens do jogador da vez
void CWind::ApplyPlayerItem983(WVector& wind)
{
	if (!GOLFDOC()->m_playerNum)
		return;
	switch (GOLFDOC()->GetPlayer(RbCurrentPlayer(GOLFDOC()))->windAbility983)
	{
	case 1:
		wind.Normalize();
		break;
	}
	if (GOLFDOC()->IsCurrentPlayerEvent983(0x80))
	{
		float power = wind.Magnitude() - 1.0;
		if (power < 1.0f)
			power = 1.0f;
		wind.Normalize();
		wind *= power;
		CCardManager::Instance()->SetWindColor(power > 0.0f);
	}
	switch (GOLFDOC()->GetPlayer(RbCurrentPlayer(GOLFDOC()))->item)
	{
	case 0x18000006:
	case 0x18000020:
	case 0x1800002c:
	case 0x1800002d:
	case 0x1800002f:
		wind.Normalize();
		return;
	case 0x18000016:
		wind.x = -wind.x;
		wind.y = -wind.y;
		wind.z = -wind.z;
		break;
	}
}

// 983 (006ca4b0)
float CWind::GetPower983()
{
	WVector wind = GetGlobalWind();
	return wind.Magnitude();
}

// 983 (006c8020)
float CWind::GetDirectionSum983()
{
	return (float)fmod(m_direction + m_cryptic34_983 + m_cryptic44_983, g_2_PI);
}

// 983 (006c9300): troca de direção pelo item (bRestore = devolve a direção antiga)
void CWind::ChangeDirection983(float direction, bool bRestore)
{
	m_b54_983 = !bRestore;
	if (m_b54_983)
	{
		m_cryptic34_983 = 0.0f;
		SetClassicDirection(0.0f);
	}
	SetDirection(direction);
}

WVector CWind::GetGlobalWind()
{
	// 983 (006ca480): sem o m_bEnable e sem o m_netWind; os passos do 645 viraram GetBaseWind983/ApplyCardPowerDown983/ApplyPlayerItem983
	WVector wind;
	GetBaseWind983(wind);
	ApplyCardPowerDown983(wind);
	ApplyPlayerItem983(wind);
	return wind;
}

float CWind::GetGlobalDirection()
{
	// 983 (006c8c50): direção somada (GetDirectionSum983) e teste do jogador nulo
	if (m_bSpecial)
		return m_specialDirection;
	if (GOLFDOC()->GetPlayer(RbCurrentPlayer(GOLFDOC())) &&
		GOLFDOC()->GetPlayer(RbCurrentPlayer(GOLFDOC()))->item == 0x18000016)
		return GetDirectionSum983() * -1.0;
	return GetDirectionSum983();
}

// 983 (006c8c30): vento sem área, carta nem item (nome da frente B: CWindJpShot::GetWind983)
WVector CWind::GetWind983()
{
	WVector wind;
	GetBaseWind983(wind);
	return wind;
}

void CWind::SetIntensity(float intensity)
{
	if (Doc()->m_gameMode == 5 && m_bSpecial)
	{
		WPolySoup::sArea* area =
			GOLFDOC()->m_pPolySoup->FindArea("silviacannon");
		if (area)
		{
			WSliceStr slice(area->name);
			char name[64];
			WVector vec;
			slice.Get("*fx", "%s %f %f %f", name, &vec.x, &vec.y, &vec.z);
			m_specialDirection = SpecialHoleOffset983() + vec.y;
			m_specialIntensity = intensity;
			m_fxMat.Reset();
			m_fxMat.Rotate(vec);
			m_fxMat.pivot = WVector((area->aabb.max.x + area->aabb.min.x) * 0.5f,
				(area->aabb.max.y + area->aabb.min.y) * 0.5f,
				(area->aabb.max.z + area->aabb.min.z) * 0.5f);
			m_fxPos.x = m_fxPos.y = m_fxPos.z = 0.0f;
			m_specialWind =
				m_specialIntensity * RotMat(m_specialDirection, 1).xa;
		}
	}
	else
	{
		// 983 (006c8cf0): gira pela soma das direcoes (GetDirectionSum983); sem m_netWind
		m_intensity = intensity;
		WMatrix mat;
		RotMat(GetDirectionSum983(), 1, mat);
		m_globalWind = mat.za * m_intensity;
		std::vector<sWind>::iterator it;
		for (it = m_areas.begin(); it != m_areas.end(); ++it)
		{
			(*it).wind.Normalize();
			float v;
			if ((*it).type == 1)
				v = m_intensity * (*it).power;
			else
				v = m_intensity + (*it).power;
			(*it).wind *= v;
		}
	}
}

void CWind::SetDirection(float direction)
{
	if (Doc()->m_gameMode == 5 && m_bSpecial)
	{
		WPolySoup::sArea* area =
			GOLFDOC()->m_pPolySoup->FindArea("silviacannon");
		if (area)
		{
			WSliceStr slice(area->name);
			char name[64];
			WVector vec;
			slice.Get("*fx", "%s %f %f %f", name, &vec.x, &vec.y, &vec.z);
			m_specialDirection = direction;
			m_fxMat.Reset();
			m_fxMat.Rotate(vec);
			m_fxMat.pivot = WVector((area->aabb.max.x + area->aabb.min.x) * 0.5f,
				(area->aabb.max.y + area->aabb.min.y) * 0.5f,
				(area->aabb.max.z + area->aabb.min.z) * 0.5f);
			m_fxPos.x = m_fxPos.y = m_fxPos.z = 0.0f;
			m_specialWind = m_specialIntensity * RotMat(vec.y, 1).xa;
		}
	}
	else
	{
		// 983 (006c9030): gira pela soma das direcoes (GetDirectionSum983); sem m_netWind
		m_direction = direction;
		WMatrix mat;
		RotMat(GetDirectionSum983(), 1, mat);
		float intensity = m_intensity;
		m_globalWind = mat.za * intensity;
	}
}

// JP 983 (006c9290; pedido B6): guarda a direção do vento clássico (nome da frente B)
void CWind::SetClassicDirection(float radian)
{
	m_cryptic44_983 = radian;
}

// JP 983 (006c92e0): reaplica a direção atual (lê o valor em +0x24 = m_direction, não o de +0x44)
void CWind::ApplyClassicDirection()
{
	SetDirection(m_direction);
}

// HACK: Init is only referenced through this alias so that its literal
// statics are emitted after the strings of Display (original data order)
struct RbWindInit
{
	void Init(unsigned char a, unsigned char b, unsigned char c,
		unsigned char d);
};
#pragma comment(linker, \
	"/alternatename:?Init@RbWindInit@@QAEXEEEE@Z=?Init@CWind@@QAEXEEEE@Z")

void CWind::MakeWindList()
{
	if (!OnlinePlay() && Doc()->GetPlayingMode() != 2)
		((RbWindInit*)&Wind())->Init(0xff, 0xff, 0, 1);

	m_areas.clear();
	int i = 0;
	WPolySoup* soup = GOLFDOC()->m_pPolySoup;
	if (soup->GetAreaNum() > 0)
	{
		do
		{
			WPolySoup::sArea* area = soup->GetArea(i);
			if (area->type == 4)
				AddWindArea(area->aabb, area->name);
			soup = GOLFDOC()->m_pPolySoup;
			++i;
		} while (i < soup->GetAreaNum());
	}
}

void CWind::AddWindArea(const Waabb& area, char* desc)
{
	if (0)
	{
		Init(0, 0, 0, 0);
		Process(0.0f);
	}
	sWind wind;
	wind.index = m_areas.size();
	wind.area = area;
	wind.link = -1;
	wind.pitch = 0.0f;
	wind.yaw = 0.0f;
	char difficulty[8] = "hard";
	switch (*desc)
	{
	case '*':
		sscanf(desc + 1, "%f %f %f %s", &wind.power, &wind.pitch, &wind.yaw,
			difficulty);
		wind.type = 1;
		wind.wind = (m_intensity * wind.power) *
			(RotMat(wind.yaw * g_DEGTORAD, 0) *
				RotMat(wind.pitch * g_DEGTORAD, 1))
				.za;
		break;
	case '+':
		sscanf(desc + 1, "%f %f %f %s", &wind.power, &wind.pitch, &wind.yaw,
			difficulty);
		wind.type = 0;
		wind.wind = (m_intensity + wind.power) *
			(RotMat(wind.yaw * g_DEGTORAD, 0) *
				RotMat(wind.pitch * g_DEGTORAD, 1))
				.za;
		break;
	case '-':
		sscanf(desc + 1, "%f %f %f %s", &wind.power, &wind.pitch, &wind.yaw,
			difficulty);
		wind.type = 2;
		wind.wind = Max(0.0f, m_intensity - wind.power) *
			(RotMat(wind.yaw * g_DEGTORAD, 0) *
				RotMat(wind.pitch * g_DEGTORAD, 1))
				.za;
		break;
	case '=':
		sscanf(desc + 1, "%f %f %f %s", &wind.power, &wind.pitch, &wind.yaw,
			difficulty);
		wind.type = 3;
		wind.wind = wind.power *
			(RotMat(wind.yaw * g_DEGTORAD, 0) *
				RotMat(wind.pitch * g_DEGTORAD, 1))
				.za;
		break;
	default:
		sscanf(desc, "%f %f %f %s", &wind.power, &wind.pitch, &wind.yaw,
			difficulty);
		wind.type = 0;
		wind.wind = (m_intensity + wind.power) *
			(RotMat(wind.yaw * g_DEGTORAD, 0) *
				RotMat(wind.pitch * g_DEGTORAD, 1))
				.za;
		break;
	}

	switch (GetDiffFlag())
	{
	case 0:
		if (strcmpi(difficulty, "normal") != 0)
			return;
		break;
	case 1:
		if (strcmpi(difficulty, "hard") == 0)
			return;
		break;
	case 2:
		if (strcmpi(difficulty, "easy") == 0)
			return;
		break;
	}

	std::vector<sWind>::iterator it;
	for (it = m_areas.begin(); it != m_areas.end(); ++it)
	{
		if (wind.area & (*it).area)
		{
			wind.link = (*it).index;
			(*it).link = wind.index;
		}
	}
	m_areas.push_back(wind);
}

bool CWind::IsInclude(const Waabb& area, const WVector& pos)
{
	return area.IsInclude(pos);
}

WVector CWind::GetWind(const WVector& pos)
{
	// 983 (006ca4f0): sem o m_bEnable; área por CWind::IsInclude; carta e itens nas funções do GetGlobalWind
	WVector wind;
	GetBaseWind983(wind);

	std::vector<sWind>::iterator it;
	for (it = m_areas.begin(); it != m_areas.end(); ++it)
	{
		if (IsInclude((*it).area, pos))
		{
			if ((*it).link >= 0)
			{
				if (IsInclude(m_areas[(*it).link].area, pos))
				{
					WVector c1 = ((*it).area.max + (*it).area.min) * 0.5f - pos;
					WVector c2 = (m_areas[(*it).link].area.min + m_areas[(*it).link].area.max) * 0.5f - pos;
					WVector e1 = (*it).area.max - (*it).area.min;
					WVector e2 = m_areas[(*it).link].area.max - m_areas[(*it).link].area.min;
					float f1 = 1.0f - c1.Magnitude() * 2.0f / e1.Magnitude();
					float f2 = 1.0f - c2.Magnitude() * 2.0f / e2.Magnitude();
					wind = (*it).wind * (f1 / (f1 + f2)) + m_areas[(*it).link].wind * (f2 / (f1 + f2));
					break;
				}
			}
			wind = (*it).wind;
			break;
		}
	}

	ApplyCardPowerDown983(wind);
	ApplyPlayerItem983(wind);
	return wind;
}

bool CWind::IsInTurbulence(const WVector& pos)
{
	std::vector<sWind>::iterator it;
	for (it = m_areas.begin(); it != m_areas.end(); ++it)
	{
		if (IsInclude((*it).area, pos))
			return true;
	}
	return false;
}

bool CWind::IsInTurbulence(const WVector& pos, const WVector& size)
{
	std::vector<sWind>::iterator it;
	for (it = m_areas.begin(); it != m_areas.end(); ++it)
	{
		if (Min(pos.x, pos.x + size.x) > (*it).area.max.x)
			continue;
		if (Max(pos.x, pos.x + size.x) < (*it).area.min.x)
			continue;
		if (Min(pos.z, size.z + pos.z) > (*it).area.max.z)
			continue;
		if (Max(pos.z, size.z + pos.z) < (*it).area.min.z)
			continue;
		return true;
	}
	return false;
}

void CWind::Display(WPuppet* puppet, float cx, float cy)
{
	if (!puppet)
		return;
	if (m_wind.Magnitude() < 0.1f || !m_bEnable)
		return;

	// 983 006c83b0: FOV fixo de 90° e projeção perspectiva durante o desenho (repostos no fim)
	WView::PROJECTION_MODE oldMode = g_view->GetProjectionMode();
	float fov = g_view->GetFOV();
	g_view->SetFOV(1.5707964f);
	if (GOLFDOC()->m_gameMode & 0x200)
		m_mat = RotMat(-1.5707964f, 0);
	else
		m_mat = RotMat(-0.5235988f, 0);
	m_mat.pivot = m_mat.za * -170.0f; // 983: distancia fixa (o 645 usava largura * -0,265625)
	g_view->SetProjectionMode(WView::PERSPECTIVE);
	g_view->SetCamera(m_mat);
	g_view->SetScreenCenter(cx, cy);
	g_view->UpdateCamera();

	WMatrix puppetMat;

	if (Wabs(m_wind.y) < g_EPSILON)
	{
		puppetMat = RotMat(m_roll, 2) * RotMat(m_pitch, 1);
	}
	else
	{
		double tilt = m_wind.y > 0.0f ? (double)1.5707964f : (double)-1.5707964f; // 983: pi/2 em float alargado
		puppetMat = RotMat(m_roll, 2) * RotMat((float)tilt, 0) *
			RotMat(m_pitch, 1);
	}
	// 983 006c83b0: sem m_bA4 a matriz (3x3) sai multiplicada por floor(altura / 480) * 480 / altura (WMatrix::operator*=, 006c7630, que aqui nao e expandido)
	if (!CUiFrameSetManager::Instance()->GetA4983())
		puppetMat *= (float)((float)floor((float)(g_view->GetHeight() / 480.0)) * 480.0 / g_view->GetHeight());
	puppet->SetMatrix(puppetMat);
	puppet->Transform(NULL, 1.0f, 0);
	puppet->UpdateLightSource(&m_light, false, false);
	puppet->Render(g_view, true, 1.0f, true, 0, 0);
	g_view->SetProjectionMode(oldMode);
	g_view->SetCamera(g_camera);
	g_view->SetFOV(fov);
	g_view->ResetScreenCenter();
	g_view->UpdateCamera();
}

void CWind::DrawWindArea(WView* view)
{
	std::vector<sWind>::iterator it;
	for (it = m_areas.begin(); it != m_areas.end(); ++it)
	{
		view->DrawAABB((*it).area, 0xffff0000, false, 0);
	}
}

bool CWind::IsGlobalWind_Compare()
{
	return m_globalWind == m_netWind;
}
