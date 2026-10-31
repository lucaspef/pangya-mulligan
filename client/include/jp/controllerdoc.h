#pragma once
#include <map>
#include <string>
#include <vector>
#include "gatewaydocjp.h"
#include "controlledobject.h"

class IController;
struct sCharacterInfo;

// JP 983: dono dos controladores e dos objetos controlados (RTTI .?AVCControllerDoc@_controller@_gateway@@, vtable 00d026c8,
// 0x40 bytes pelo `new` de 004baa50, construtor 004ba7e0, base _gateway::CGatewayControlDoc). Layout pelo construtor: três
// std::map de 0x10 (nós de 0x24, 0x24 e 0x30) e o ator. Os nomes dos métodos e dos campos são meus.
namespace _gateway
{
namespace _controller
{
class CControllerDoc : public CGatewayControlDocJp
{
public:
	typedef std::vector<CControlledObject*> ObjectList;
	typedef std::map<unsigned char, ObjectList> ObjectMap; // chave = índice do jogador (1 byte)
	typedef std::map<std::string, IController*> ControllerMap;

	CControllerDoc(int type); // 004ba7e0
	virtual ~CControllerDoc(); // 004ba950 (004baa20 com delete)
	virtual bool Initialize(); // 004baac0
	virtual bool Release(); // 004b9e70
	virtual CGatewayHandler* GetGatewayHandler() { return m_pHandler; } // 004ba940

	static CGatewayControlDocJp* __fastcall Create(); // 004baa50: `new CControllerDoc(0xad)` (o criador registrado em 004bab00)

	CControlledObject* CreateObject(eSpecialAbility ability); // 004b52e0
	int GetMode983() const { return m_04; } // +0x04 da base (0x10 = os objetos de m_objects estão em uso)
	ObjectMap& GetObjects(); // 004b52f0 (fora de linha: o CControllerHandler chama por call)
	ObjectMap& GetSubObjects(); // 004b5300
	// 004b6a00: cria (se faltar) em `list` o objeto de cada habilidade do item e entrega o item a ele (On01 + On04(flags))
	void AddItem(ObjectList& list, int player, unsigned long typeId, unsigned int flags);
	int CountKept(ObjectList* pList); // 004b69c0: quantos objetos da lista têm On11() verdadeiro (os que o Save grava)
	void DeleteObjects(ObjectList& list); // 004b7360
	void DeleteControllers(); // 004b79a0
	void RefreshEnable(); // 004b79c0: m_bEnable de cada controlador = On02()
	void FireOn03(); // 004b79e0
	CControlledObject* GetObjectConst(int player, unsigned long index) const; // 004b7a00 (ninguém chama)
	// 004b7e90: habilidade avulsa (sem registro de item) para um jogador que já tem lista em m_objects
	void AddAbility(int player, unsigned long ability, int arg3, unsigned long typeId, float value);
	void FireOn09(int player); // 004b7fb0: m_objects[player].at(13)->On09(0)
	void Purge(int player); // 004b8010: apaga os objetos de m_objects[player] cujo On11() devolve falso
	CControlledObject* GetObjectAt(int player, unsigned long index); // 004b8080
	ObjectList* GetObjectList(unsigned char player); // 004b80e0
	IController* Find(const std::string& name); // 004b8390: 0 se não achar
	void RemoveSubObjects(int player); // 004b9980
	void Clear(); // 004b9cf0
	void SetSubObjects(int player, const sCharacterInfo* pInfo); // 004b9e90: objetos das peças de um personagem → m_subObjects
	void SetObjects(int player); // 004ba060: objetos do jogador (peças, mascote e, se for o meu, os itens) → m_objects
	bool RegisterControllers(); // 004ba550: os 31 nomes de controlador

private:
	ObjectMap m_objects; // +0x0c
	ObjectMap m_subObjects; // +0x1c
	ControllerMap m_controllers; // +0x2c
	CGatewayHandler* m_pHandler; // +0x3c: ator "CControllerHandler" criado em Initialize
};
}
}
