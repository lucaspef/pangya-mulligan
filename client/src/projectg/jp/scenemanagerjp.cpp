// JP 983: CSceneManager::DeleteElement (009b4130). Diferente do 645 (CSceneManager::DeleteElement da A): o 983 nao tem o guarda
// IsLocalContent(S4_REAL_MYROOM) e apaga so o 1o elemento da lista com esse nome (std::find com SceneElement::operator==:
// nome vazio nunca casa, strcmpi). Layout 983: lista de SceneElement com a sentinela em this+0xbc; no no: nome em +0x14, texView em +0x9c.
#include "lobbyjp.h"
#include "vc10_layout.h"
#include <string.h>

namespace
{
	struct SceneNode983
	{
		SceneNode983* next;
		SceneNode983* prev;
	};

	// destrutor virtual (slot 0, flag 1 = delete)
	struct VDel983
	{
		virtual ~VDel983() { }
	};
}

void CSceneManagerJp::DeleteElement(const char* name)
{
	SceneNode983* head = *(SceneNode983**)((char*)this + VC10_OFS_BO(0xbc)); // VS 2022: m_elements 0xb8 (C_ASSERT em scenemanager.cpp)
	SceneNode983* node;
	for (node = head->next; node != head; node = node->next)
	{
		const char* nodeName = (const char*)node + 0x14;
		if (nodeName[0] != '\0' && name != NULL && strcmpi(nodeName, name) == 0)
			break;
	}
	if (node == head)
		return;

	VDel983** pTexView = (VDel983**)((char*)node + 0x9c);
	if (*pTexView)
	{
		delete *pTexView;
		*pTexView = NULL;
	}
	// erase da lista: desliga o no, libera (operator delete 00c3c190) e --size (this+0xc0)
	node->prev->next = node->next;
	node->next->prev = node->prev;
	operator delete(node);
	--*(int*)((char*)this + VC10_OFS_BO(0xc0));
}
