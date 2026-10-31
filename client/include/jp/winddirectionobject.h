#pragma once
#include "controlledobject.h"

// JP 983: argumento do slot 7/8 do CWindDirectionObject (direção sorteada pelo servidor + tipo do sorteio). Nome meu.
struct sWindDirectionChange
{
	float direction; // +0x00
	unsigned char kind; // +0x04 (0, 1 = seta branca; 2, 3 = seta escura)
};

// JP 983: RTTI .?AVCWindDirectionObject@@, vtable 00d30090, 0x74 bytes (operator new em 0082dff0), habilidade 0x0c.
// Item que troca a direção do vento na tacada: guarda a direção anterior, aplica a nova (CWind 006c9300) e avisa no chat.
class CWindDirectionObject : public CControlledObject
{
public:
	CWindDirectionObject(eSpecialAbility ability); // 0082df00
	virtual ~CWindDirectionObject(); // 0082de00
	virtual void On01(const sControlledParam* pParam); // 0082e2e0
	virtual void On05(); // 0082de10
	virtual void On06(int); // 0082e060
	virtual void On07(int pChange); // 0082e0e0
	virtual void On08(int pChange); // 0082dfb0
	virtual void On10(int index); // 0082de30

	unsigned long GetTypeId() const { return m_typeId; } // cópia fora de linha em 0082d900
	unsigned char GetPlayerIndex() const { return m_playerIndex; } // 0082d910
	bool IsActive() const { return m_bActive; } // 0082d920
	int GetKind() const { return m_kind; } // 0082d930
	void ClearChange(); // 0082dec0
	void SetChange(const sWindDirectionChange* pChange); // 0082ded0
	sWindDirectionChange* GetChange(); // 0082def0

	// acessos embutidos nos métodos; o 983 guarda uma cópia fora de linha de cada um em 0082dd80…0082ddf0
	void SetActive(bool bActive) { m_bActive = bActive; } // 0082dd80
	void SetKind(int kind) { m_kind = kind; } // 0082dd90
	void SetOldDirection(float direction) { m_oldDirection = direction; } // 0082dda0
	float GetOldDirection() const { return m_oldDirection; } // 0082ddb0
	void SetSumDirection(float direction) { m_sumDirection = direction; } // 0082ddc0
	float GetSumDirection() const { return m_sumDirection; } // 0082ddd0
	void SetNewDirection(float direction) { m_newDirection = direction; } // 0082dde0
	float GetNewDirection() const { return m_newDirection; } // 0082ddf0

private:
	void Reset() // o corpo do On05, embutido no fim do On06
	{
		m_sumDirection = 0.0f;
		m_bActive = false;
		m_newDirection = 0.0f;
		m_kind = 0;
	}

	unsigned long m_typeId; // +0x50 (typeid do item)
	unsigned char m_playerIndex; // +0x54
	int m_kind; // +0x58 (1 = "right_wind_wind_white", 2 = "right_wind_wind_dark")
	float m_oldDirection; // +0x5c (direção do vento antes da troca)
	float m_sumDirection; // +0x60 (CWind 006c8020 antes da troca)
	float m_newDirection; // +0x64
	bool m_bActive; // +0x68
	sWindDirectionChange m_change; // +0x6c (troca recebida pelo slot 8, usada pelo slot 7)
};
