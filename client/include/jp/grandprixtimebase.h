#pragma once
// JP 983: CGrandPrixTimeBase (RTTI .?AVCGrandPrixTimeBase@@, vtable 00d09f44: [0] puro, [1] 0055aa10 vazio). Base das peças do relógio
// do Grand Prix (CGrandPrixHour 00d09f50, Minute 00d0a16c, Second 00d0a178, Clock: unidade gproomtimersteward). Os quatro métodos
// ficam no fim da unidade do CGrandPrixHandler (0055a9d0–0055aa10). Sessão B65; B66: [0] = avanço de uma unidade (devolve o "vai
// um"), as três peças derivadas. Nomes meus.
class CGrandPrixTimeBase
{
public:
	// B67: construtor, Set e Get embutidos em todo o 983 (0055a9d0/0055a9e0/0055aa00 são cópias fora de linha sem chamador, que o
	// nosso build não gera; o 983 chama o [1] pela vtable no Set)
	CGrandPrixTimeBase() : m_value(0) { } // 0055a9d0
	virtual bool Inc() = 0; // [0]: soma 1; true = deu a volta (Minute 0055ce90, Second 0055ceb0, Hour 0055d240 → 0055cdb0)
	virtual void OnChange(); // [1] 0055aa10 (CGrandPrixHour 0055ce30)
	void Set(unsigned short value) { m_value = value; OnChange(); } // 0055a9e0
	unsigned short Get() { return m_value; } // 0055aa00

protected:
	unsigned short m_value; // +0x04 (B67: sem sinal — o 983 compara com jb)
};

// hora: m_value é a hora de 12 (1–12; 0x63 = inválida), m_hour a de 24
class CGrandPrixHour : public CGrandPrixTimeBase
{
public:
	CGrandPrixHour()
		: m_hour(0), m_bPM(0), m_10(0)
	{
	}
	virtual bool Inc(); // 0055cdb0 (a vtable aponta para o salto 0055d240)
	virtual void OnChange(); // 0055ce30: m_value (de 24) → m_hour, m_bPM e m_value de 12
	bool Tick() { return Inc(); } // 0055cd30
	// hora de 24 → m_value de 12 (1–12; 0 vira 12; 24 ou mais, 0x63 e falso); comum ao Inc e ao OnChange (B67)
	bool SetHour12(unsigned short h)
	{
		bool ok = true;
		if (h > 0 && h <= 12)
			m_value = h;
		else if (h > 12 && h < 24)
			m_value = h - 12;
		else if (h == 0)
			m_value = 12;
		else
		{
			m_value = 0x63;
			ok = false;
		}
		return ok;
	}
	unsigned short GetHour() { return m_hour; } // 0055cd40
	void Set10(int v) { m_10 = v; } // 0055ce80 (uso não visto)

	unsigned short m_hour; // +0x08
	int m_bPM; // +0x0c
	int m_10; // +0x10
};
C_ASSERT(sizeof(CGrandPrixHour) == 0x14);

class CGrandPrixMinute : public CGrandPrixTimeBase
{
public:
	CGrandPrixMinute() { } // 0055ced0
	virtual bool Inc(); // 0055ce90
};

class CGrandPrixSecond : public CGrandPrixTimeBase
{
public:
	CGrandPrixSecond() { } // 0055cee0
	virtual bool Inc(); // 0055ceb0
};

// 0055cd50: dois dígitos (com zero à esquerda) em out[0..1], sem terminador; o 983 passa o valor em cx e o destino em edx
void __fastcall GPFormat2_983(unsigned short value, char* out);
