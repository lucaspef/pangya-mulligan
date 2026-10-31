#pragma once

#include <vector>
#include "encrypttypes.h"
#include "../../include/engine/wscene.h"

class CFxSequence;
class WPuppet;
class WView;

// 983: vetor cifrado (0x3c): 3 WCrypticValue<float> + cache em claro; 006c7e00 = Get(). Nome meu.
struct WCrypticVector983
{
	WCrypticValue<float> x;
	WCrypticValue<float> y;
	WCrypticValue<float> z;
	WVector plain;

	const WVector& Get(); // 006c7e00 (fora de linha no wind.cpp: o 983 chama)
	WCrypticVector983& operator=(const WVector& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}
	operator WVector() { return Get(); }
};

class CWind
{
public:
	struct sWind
	{
		int index;
		Waabb area;
		WVector wind;
		unsigned char type;
		float power;
		float pitch;
		float yaw;
		int link;
	};

	CWind();
	CWind(const CWind& src); // 983 006cb9f0
	virtual ~CWind() { }

	void Init(unsigned char a, unsigned char b, unsigned char c,
		unsigned char d);
	void Process(float dt);
	void Display(WPuppet* puppet, float cx, float cy);
	WVector GetWind(const WVector& pos);
	WVector GetGlobalWind();
	float GetGlobalDirection();
	void SetIntensity(float intensity);
	void SetDirection(float direction);
	// JP 983 (pedido B6): 006c9290 e 006c92e0, mais dois acessos usados por jp/classicwind.cpp (nomes da frente B)
	void SetClassicDirection(float radian);
	void ApplyClassicDirection();
	bool IsLocked983() { return m_b54_983; }
	unsigned char IsSpecial983() { return *(unsigned char*)&m_bSpecial; }
	// JP 983: métodos que o 645 não tem (nomes da frente B em include/jp/classicwind.h; corpos no wind.cpp)
	float GetDirectionSum983(); // 006c8020: fmod(m_direction + m_cryptic34_983 + m_cryptic44_983, 2pi)
	void ChangeDirection983(float direction, bool bRestore); // 006c9300
	float GetPower983(); // 006ca4b0: comprimento do GetGlobalWind
	// 983: o GetGlobalWind/GetWind do 645 em três passos (nomes meus)
	void GetBaseWind983(WVector& wind); // 006c87c0: vento especial ou global
	WVector GetWind983(); // 006c8c30: GetBaseWind983 por valor (nome da frente B)
	void ApplyCardPowerDown983(WVector& wind); // 006c86f0: carta que enfraquece o vento
	void ApplyPlayerItem983(WVector& wind); // 006c9550: itens do jogador (vento 1 m, invertido, evento 0x80)
	void SetWind(const WVector& wind);
	void PlusWind(const WVector& wind);
	void SetSpecialWind(bool bSpecial, bool bSilent); // 983 (006c7e40): 2º argumento novo
	void GenerateFx(bool bOn);
	void CheckSpecialWind();
	bool IsGlobalWind_Compare();
	void MakeWindList();
	bool IsInTurbulence(const WVector& pos);
	bool IsInTurbulence(const WVector& pos, const WVector& size);
	void DrawWindArea(WView* view);
	bool HasSpecialWind() { return m_specialIntensity > 0.0f; }
	unsigned long GetSpWindStartFrame() { return m_spWindStartFrame; }
#ifdef REBANG_QUADTREE_CPP // HACK: other includers keep their IL (emission order)
	void SetSpWindStartFrame(unsigned long frame)
	{
		m_spWindStartFrame = frame;
	}
	bool IsActiveSpecialWind() { return m_bSpecial; }
#endif
#ifdef REBANG_GOLFRULE_PCH
	bool IsActiveSpecialWind() { return m_bSpecial; }
	void SetActive(bool active) { m_bEnable = active; }
#endif
#ifdef REBANG_GOLFTASK_CPP // HACK: other includers keep their IL (emission order)
	void SetActive(bool active) { m_bEnable = active; }
#endif

protected:
	bool IsInclude(const Waabb& area, const WVector& pos);
	void AddWindArea(const Waabb& area, char* desc);

	unsigned long m_spWindStartFrame;
	WVector m_wind;
	WCrypticValue<float> m_intensity;
	WCrypticValue<float> m_direction;
	WCrypticValue<float> m_cryptic34_983; // 983: 0x34 (zerado em 006c9300); nome meu
	WCrypticValue<float> m_cryptic44_983; // 983: 0x44 (setter 006c9290); nome meu
	bool m_b54_983; // 983: 0x54 (006c9300 grava !arg2); nome meu
	float m_roll;
	WMatrix m_mat;
	LightSet m_light;
	float m_pitch; // 983: 0xa8 (Process 006caaf0, Display 006c83b0)
	unsigned long m_pitchSnapFrames983; // 983: 0xac (inteiro; zerado no Init 006c9e20, contado no Process); nome meu
	float m_minIntensity;
	float m_maxIntensity;
	WCrypticValue<bool> m_bEnable;
	std::vector<sWind> m_areas;
	static WVector m_netWind; // 983: não existe (PROVISÓRIO: static para o .cpp do 645 compilar)
	WCrypticVector983 m_globalWind; // 983: 0xd8
	WCrypticVector983 m_specialWind; // 983: 0x114
	WCrypticValue<float> m_specialIntensity;
	WCrypticValue<float> m_specialDirection;
	bool m_bSpecial;
	WMatrix m_fxMat;
	WVector m_fxPos;
	CFxSequence* m_pFxSeq;
};

CWind& Wind();
void PushWind983(); // 983 006cc5d0: ctor do CGolfTask
void PopWind983(); // 983 006cb3c0: dtor do CGolfTask
