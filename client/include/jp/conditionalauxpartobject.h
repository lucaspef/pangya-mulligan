#pragma once
#include <map>
#include "controlledobject.h"
#include "../../src/shared/classdefine.h"

// JP 983: o sAuxPart do 983 tem 6 bytes a mais que o do fonte (lidos em 004aa670): `short` em +0xde e typeid em +0xe0.
// PROVISÓRIO (classdefine.h é da frente A — pedido em out/PEDIDOS.md): leitura por deslocamento. Nomes meus.
inline short AuxPartPairBonus983(const IFF_STRUCT::sAuxPart* pItem) { return *(const short*)((const char*)pItem + 0xde); }
inline unsigned long AuxPartPairTypeId983(const IFF_STRUCT::sAuxPart* pItem) { return *(const unsigned long*)((const char*)pItem + 0xe0); }

// JP 983: elemento (0x14 bytes) dos dois vetores do CConditionalAuxpartObject; é o argumento dos tratadores por typeid
// (RTTI do std::function: `void (const sAuxpartValue&)`). Nomes dos campos meus.
struct sAuxpartValue
{
	sAuxpartValue() : m_pItem(NULL), m_kind(-1), m_value(0.0f), m_uid(-1), m_bSilent(0) { }
	// 004a8ae0: desfaz o efeito aplicado (não mexe no item nem no dono)
	void Reset() { if (m_uid != -1) { m_kind = -1; m_value = 0.0f; } }

	IFF_STRUCT::sAuxPart* m_pItem; // +0x00
	int m_kind; // +0x04 (-1 = sem efeito; 0…4 = qual campo do item vale: DriveUp, ItemDropUp, ComboUp, PangUp, typeid; campo gravado)
	float m_value; // +0x08 (valor aplicado; campo gravado)
	int m_uid; // +0x0c (uid do dono, sUserInfo::info.dwUID; -1 no elemento vazio)
	int m_bSilent; // +0x10 (1 quando o valor do item na origem não é 0: o efeito é aplicado sem aviso)
};

// JP 983: argumento do slot 8 / do slot 7 (quatro inteiros guardados em +0x74). Nomes meus.
struct sAuxpartApply
{
	int m_playerIndex; // +0x00
	unsigned long m_typeId; // +0x04 (0 = nada a aplicar)
	int m_uid; // +0x08 (-1 = nada a aplicar)
	int m_kind; // +0x0c
};

// JP 983: RTTI .?AVCConditionalAuxpartObject@@, vtable 00d021d0, 0x94 bytes (operator new em 004acc80), habilidade 0x0b.
// Peças auxiliares (AuxPart.iff, typeid 0x7021000x) cujo efeito depende de uma condição testada pelo controlador a cada tacada.
class CConditionalAuxpartObject : public CControlledObject
{
public:
	typedef std::tr1::function<void(const sAuxpartValue&)> Handler;

	CConditionalAuxpartObject(eSpecialAbility ability); // 004aca30
	virtual ~CConditionalAuxpartObject(); // 004ac8f0 (004acc50 com delete)
	virtual void On01(const sControlledParam* pParam); // 004a8a90 (guarda o jogador e repassa ao slot 3)
	virtual void On03(const sControlledParam* pParam); // 004ac3f0
	virtual void On07(int arg); // 004ab9a0 (arg = const sAuxpartApply*)
	virtual void On08(int arg); // 004a8ab0 (arg = const sAuxpartApply*: guarda em m_apply)
	virtual void On10(int index); // 004ac680

	int GetPlayerIndex() const; // 004a8b00 (fora de linha: o controlador chama a cópia da unidade do objeto)
	void ResetApply(); // 004a8b10
	bool HasApply() const; // 004a8b30
	const sAuxpartApply* GetApply() const; // 004a8b50
	int GetCount() const; // 004a9670
	sAuxpartValue* GetAt(int index); // 004a9690
	sAuxpartValue* Find(unsigned long typeId); // 004aa980
	void SyncDoc(); // 004aa030
	void Send(int index, int kind); // 004aa7d0
	void Notify(int kind, const sAuxpartValue* pValue, int playerIndex); // 004aaf70
	void Apply(const sAuxpartApply* pApply); // 004ab200
	void ResetAll(); // 004ac4c0

private:
	std::vector<sAuxpartValue> m_values; // +0x50 (reserve(5) no construtor; um elemento por peça registrada)
	std::vector<sAuxpartValue> m_applied; // +0x60 (5 elementos: efeito aplicado por posição de sCharacterInfo::tidAuxParts)
	int m_playerIndex; // +0x70
	sAuxpartApply m_apply; // +0x74
	std::map<unsigned long, Handler> m_handlers; // +0x84 (typeid → aviso)
};

// JP 983: inteiro de CGolfDoc+0xf0 (4 = tacada em andamento? significado não levantado; no fonte cai dentro de m_unusedcd).
// PROVISÓRIO (golfdoc.h é da frente A — pedido em out/PEDIDOS.md): leitura por deslocamento.
#include "golfdoc.h"
#include "golfdocofs983.h" // B1459
inline int GolfDocField_f0_983() { return *(const int*)((const char*)GDOCPTR983(GOLFDOC(), 0xf0)); }
