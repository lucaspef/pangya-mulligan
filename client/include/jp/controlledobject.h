#pragma once
#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

class IController;
namespace _gateway { namespace _controller { class CControllerDoc; } }

// JP 983: registro de habilidade do item devolvido por 00a177a0 (tabela do CItemManager) e apontado pelo argumento do slot 1
// (lido em 005ac340, 004b6a00 e 004b7e90, que monta um na pilha: 0x50 bytes). Nomes provisórios.
struct sControlledSource
{
	unsigned long m_typeId; // +0x00 (typeid do item; 0056b070 guarda e 0056b190 procura o item comum por ele)
	int m_04; // +0x04 (3º argumento de 004b7e90)
	int m_08;
	int m_0c;
	unsigned long m_abilities[3]; // +0x10 (eSpecialAbility; 0 e 0x24 = nenhuma)
	float m_values[3]; // +0x1c (índice = sControlledParam::m_index)
	SYSTEMTIME m_from; // +0x28
	SYSTEMTIME m_to; // +0x38
	unsigned int m_mask; // +0x48 (IController::TestMask)
	unsigned char m_4c; // +0x4c
	unsigned char m_bPeriod; // +0x4d: 1 = só vale entre m_from e m_to
	char m_4e[2];
};

struct sControlledParam
{
	union
	{
		int m_00; // +0x00
		unsigned char m_playerIndex; // +0x00 (índice do jogador: 0056b070 copia 1 byte e o controlador compara com o da tacada)
	};
	int m_index; // +0x04
	sControlledSource* m_pSource; // +0x08
};

// JP 983: o valor passa pelo registro (ecx) até o construtor de cada C*Object; valores vistos nos registros (mov ecx, N antes de 004b4220).
enum eSpecialAbility
{
	eSpecialAbility_ImpactZone = 0x01,
	eSpecialAbility_ImpactZoneElfEar = 0x02,
	eSpecialAbility_ImpactZoneElfEarImitation = 0x03,
	eSpecialAbility_ImpactZoneHighElfEar = 0x04,
	eSpecialAbility_ImpactZoneOwnerItem = 0x05,
	eSpecialAbility_ImpactZoneAuxPartWeakWind = 0x06,
	eSpecialAbility_ComboGauge = 0x07, // CComboGaugeObject (004a88d0/004a8940)
	eSpecialAbility_ComboGaugeLimitIncrease = 0x08,
	eSpecialAbility_ComboGaugeGame = 0x09, // CComboGaugeGameObject (004a8990/004a8a00)
	eSpecialAbility_PangyaCorrection = 0x0a,
	eSpecialAbility_ConditionalAuxpart = 0x0b,
	eSpecialAbility_WindDirection = 0x0c,
	eSpecialAbility_SetEffect = 0x0d,
	eSpecialAbility_GroundInclination = 0x0e,
	eSpecialAbility_ImpactZoneProbability5 = 0x0f,
	eSpecialAbility_WindVolume = 0x10,
	eSpecialAbility_ImpactZoneUndine = 0x11,
	eSpecialAbility_LosslessHit = 0x12,
	eSpecialAbility_AutoMiracleSign = 0x13,
	eSpecialAbility_BallDirNotify = 0x14,
	eSpecialAbility_TrajectoryPreview = 0x15,
	eSpecialAbility_PangyaAddCorrection = 0x16,
	eSpecialAbility_PowerShotItem = 0x17,
	eSpecialAbility_SafetyItem = 0x18,
	eSpecialAbility_PlusYardItem = 0x19,
	eSpecialAbility_ComboGaugeEveryshot = 0x1a,
	eSpecialAbility_ImpactZoneMascot = 0x1b,
	eSpecialAbility_WeatherImpactZone = 0x1c,
	eSpecialAbility_LosslessHitWeather = 0x1d,
	eSpecialAbility_Putter10Yard = 0x1e,
	eSpecialAbility_ClubsetExpMultiply = 0x1f,
	eSpecialAbility_MiracleSignSkill = 0x20,
	eSpecialAbility_StatusSlotPlus = 0x21,
	eSpecialAbility_WindMinus = 0x22,
	eSpecialAbility_PangyaCorrectionSet = 0x23,
	eSpecialAbility_Max = 0x24, // tamanho das listas de objetos do CControllerDoc (004b9e90)
};

// JP 983: gravador de estado embutido no CControlledObject (+0x04, 0x24 bytes). Nome provisório (sem RTTI).
// 004b4930 soma os tamanhos, 004b4950/004b4d90 gravam/leem FILE*, 004b4ce0 tira uma cópia dos campos.
// Nomes dos métodos meus.
class CControlledRecord
{
public:
	CControlledRecord(); // 004b4c40
	template<class T> void Add(T* pField); // um campo de sizeof(T) bytes (instâncias: bool 00569ff0)
	void Commit(); // 004b4930: soma os tamanhos dos campos em m_recordSize
	void* Get(int index); // 004b4530 (m_records[index] ou 0)
	void Write(FILE* fp); // 004b4950: nº de registros, tamanho do registro e cada registro
	void Clear(); // 004b4c60: joga fora os registros
	void Snapshot(); // 004b4ce0: registro novo com a cópia de cada campo
	void Read(FILE* fp); // 004b4d90: o inverso de Write (não mexe em m_recordSize)
	std::vector<std::pair<size_t, void*> > m_fields; // +0x00
	std::vector<std::unique_ptr<char[]> > m_records; // +0x10
	size_t m_recordSize; // +0x20
};

// fora da classe e sem `inline`: o 983 chama a instância por call (00569ff0), não embute
template<class T> void CControlledRecord::Add(T* pField)
{
	m_fields.push_back(std::pair<size_t, void*>(sizeof(T), pField));
}

// JP 983: base dos objetos controlados (RTTI .?AVCControlledObject@@, vtable 00d022d8, 12 virtuais, 0x50 bytes).
// Nomes dos virtuais provisórios: OnNN = slot NN.
class CControlledObject
{
public:
	typedef std::tr1::function<CControlledObject*(eSpecialAbility)> Creator;

	CControlledObject(eSpecialAbility ability); // 004b3d70
	virtual ~CControlledObject(); // 004b3db0 (004b3f30 com delete)
	virtual void On01(const sControlledParam* pParam) = 0;
	virtual void On02(int) { } // 004b0e50
	virtual void On03(const sControlledParam*) { } // 004b0e60 (o argumento é o do slot 1: 0056b530 repassa)
	virtual void On04(int arg); // 004b3300: para cada nome em m_controllerNames, acha o controlador e chama 004b3190
	virtual void On05() { } // 004b0e70
	virtual void On06(int) { } // 004b0e80
	virtual void On07(int) { } // 004b0e90
	virtual void On08(int) { } // 004b0ea0
	virtual void On09(int) { } // 004b0eb0
	virtual void On10(int) { } // 004b0ec0
	virtual bool On11() { return true; } // 004b0ed0

	void AddController(const char* name); // 004b40a0: m_controllerNames.push_back(name)
	void RemoveController(const char* name); // 004b3bd0: tira o nome de m_controllerNames
	void Attach(IController* pController, int arg); // 004b3190: controlador novo em m_controllers (grava arg no +0x0c dele)
	void Snapshot(); // 004b0f20: m_record.Snapshot(), menos com o byte global ligado
	void Save(FILE* fp); // 004b1150: habilidade + m_record
	void Load(FILE* fp); // 004b1180: m_record (a habilidade quem lê é o chamador, 004bb67e)
	// um por slot do IController: grava este objeto no controlador (+0x04) e chama o slot em cada um de m_controllers
	void Fire04(); // 004b27b0
	void Fire05(); // 004b27e0 (antes, m_record.Clear())
	void Fire06(const float& value); // 004b2810
	void Fire12(); // 004b2860
	void Fire07(int arg); // 004b2890
	void Fire08(); // 004b28c0
	void Fire10(int arg); // 004b28f0
	void Fire13(); // 004b2920
	void Fire09(); // 004b2950
	void Fire11(); // 004b2980
	static _gateway::_controller::CControllerDoc* GetDoc(); // 004b0e10 (global 00e74c98)
	static void SetDoc(_gateway::_controller::CControllerDoc* pDoc); // 004b0e20
	static void SetActive983(bool bActive); // 004b0e30 (global 00e74c9c)
	bool Is28() const { return m_28; }
	void Set28(bool b) { m_28 = b; }
	static bool IsActive983(); // 004b0e40: devolve o byte global 00e74c9c (nome meu; gravado por 004b0e30 a partir de 004ba08d e 004bb6b5)
	static bool Register(eSpecialAbility ability, Creator creator); // 004b4220
	static CControlledObject* Create(eSpecialAbility ability); // 004b4140

protected:
	static std::map<eSpecialAbility, Creator>& GetCreators(); // 004b3fd0 (mapa `static` local, 00e74ca0)
	CControlledRecord m_record; // +0x04
	bool m_28; // +0x28
	std::vector<IController*> m_controllers; // +0x2c
	std::vector<std::string> m_controllerNames; // +0x3c (elemento de 0x1c)
	eSpecialAbility m_ability; // +0x4c
};

// forma que casa os registros (ecx = habilidade, edx = criador antes de montar o std::function): função embutida
inline bool RegisterControlledObject(eSpecialAbility ability, CControlledObject* (*creator)(eSpecialAbility))
{
	return CControlledObject::Register(ability, creator);
}
