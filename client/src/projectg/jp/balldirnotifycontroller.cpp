// JP 983: unidade 00484010–00484330 (CBallDirNotifyController). Classe nova do 983.
// Entre o MakeInstance e o Apply o 983 guarda 14 COMDATs de iterador de std::list<sItemInfo> e do find_if (00484060…00484210).
#include "balldirnotifycontroller.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "actor.h"
#include <algorithm>

CBallDirNotifyController::CBallDirNotifyController()
{
}

IMPLEMENT_OBJECT(CBallDirNotifyController, IController)

CBallDirNotifyObject* CBallDirNotifyController::GetControlled()
{
	return (CBallDirNotifyObject*)m_pObject;
}

struct FindItemByTypeId
{
	FindItemByTypeId(unsigned long typeId) : m_typeId(typeId) { }
	bool operator()(const sItemInfo& item) const { return item.tid == m_typeId; }
	unsigned long m_typeId;
};

// 00484240: gasta uma unidade do item (Common[0]) na lista do jogador e avisa a "ShotPreview" se ainda sobra
bool CBallDirNotifyController::Apply()
{
	CBallDirNotifyObject* pObject = GetControlled();
	if (pObject == NULL)
		return false;
	std::list<sItemInfo>& items = Doc()->m_myItemList;
	std::list<sItemInfo>::iterator it = std::find_if(items.begin(), items.end(), FindItemByTypeId(pObject->GetTypeId()));
	if (it != items.end())
	{
		if (it->Common[0] > 0)
			--it->Common[0];
		AfxGetTask()->GetActor("ShotPreview") << MsgObject(NULL, 0x2fd, (unsigned long)"TOGGLE_BALL_DIR_NOTIFY", it->Common[0] > 0, 0, 0, 0);
	}
	else
		AfxGetTask()->GetActor("ShotPreview") << MsgObject(NULL, 0x2fd, (unsigned long)"TOGGLE_BALL_DIR_NOTIFY", 0, 0, 0, 0);
	return true;
}

bool CBallDirNotifyController::On05()
{
	return Apply();
}
