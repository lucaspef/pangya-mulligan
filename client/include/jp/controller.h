#pragma once
#include "objectfactory.h"

class CControlledObject;

// JP 983: base dos controladores de item/efeito (RTTI .?AVIController@@, vtable 00d0230c, 14 virtuais, 0x10 bytes).
// Os virtuais ficam fora de linha em controller.cpp (as derivadas chamam IController::OnNN por call).
// Os nomes dos virtuais são provisórios (o 983 não traz nomes de método): OnNN = slot NN da vtable.
class IController : public IObject
{
	DECLARE_OBJECT(IController)
public:
	IController(); // 004b4f50
	virtual ~IController(); // 004b4f80
	virtual bool On02(); // 004b4f90
	virtual bool On03(); // 004b4fa0
	virtual bool On04(); // 004b4fb0
	virtual bool On05(); // 004b4fc0
	virtual bool On06(float); // 004b4fd0
	virtual bool On07(int); // 004b4fe0
	virtual bool On08(); // 004b4ff0
	virtual bool On09(); // 004b5000
	virtual bool On10(int); // 004b5010
	virtual bool On11(); // 004b5020
	virtual bool On12(); // 004b5030
	virtual bool On13(); // 004b5040

	CControlledObject* GetControlled() const; // 004b5110
	// o objeto já com o tipo da derivada; fora da classe e sem `inline`: o 983 chama a instância por call (00580e80, 00580e90)
	template<class T> T* GetControlledAs();
	void SetEnable(bool bEnable); // 004b5120
	void SetEnableInline(bool bEnable) { m_bEnable = bEnable; } // embutido no lambda de 004b6f70 / 004b52a0 (controllerdoc)
	// embutidos; o 983 guarda uma cópia fora de linha de cada um no começo do controlledobject (ninguém chama por call)
	void SetControlled(CControlledObject* pObject) { m_pObject = pObject; } // 004b0df0
	void Set0c(int value) { m_0c = value; } // 004b0e00
	static bool TestMask(unsigned int kind, unsigned int flags); // 004b5130

protected:
	CControlledObject* m_pObject; // +0x04 (os C*Controller chamam métodos do C*Object por ele)
	bool m_bEnable; // +0x08
	int m_0c; // +0x0c
};

template<class T> T* IController::GetControlledAs()
{
	return (T*)m_pObject;
}
