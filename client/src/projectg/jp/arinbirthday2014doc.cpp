// JP 983: _gateway::CArinBirthDay2014Doc (documento 0xb0; ver include/jp/arinbirthday2014doc.h). Classe nova do 983. No exe o código
// está dentro da unidade do gatewaydoc.cpp (frente A): construtor/destrutor/GetGatewayHandler/criador/registro/Decode em
// 00920a90–00920e60 e o Initialize em 00921360, depois do CSpecialBoxDoc::Initialize; o "arinbirthday2014handler_p2" e o
// "arinbirthday2014doc" da F3.7.1 (00920fbe–009218d0) são esse fim do gatewaydoc + o specialbox.cpp da A. Sessão B127, F3.7.12 item (2).
#include "minatl.h"
#include "projectg.h"
#include "packet.h"
#include "arinbirthday2014doc.h"
#include "arinbirthday2014handler.h"

using namespace _gateway;

// 00920a90
CArinBirthDay2014Doc::CArinBirthDay2014Doc(int type) : CGatewayControlDocJp(type)
{
}

// 00920ab0
CGatewayHandler* CArinBirthDay2014Doc::GetGatewayHandler()
{
	return (CGatewayHandler*)(CGatewayHandlerJp*)m_pHandler; // a visão CGatewayHandlerJp (controllerhandler.h) não deriva do CGatewayHandler
}

// 00920be0
IContentsDataContainer* __fastcall CArinBirthDay2014Doc::Create()
{
	return new CArinBirthDay2014Doc(0xb0);
}

// 00920c70 (chamada pelo inicializador estático 00ce6810)
static bool RegisterArinBirthDay2014Doc()
{
	CContentsDoc::CreateCreatorList983();
	CContentsDoc::RegisterCreator983(&CArinBirthDay2014Doc::Create);
	return true;
}

static bool s_bDocRegistered = RegisterArinBirthDay2014Doc();

// 00920cc0
void CArinBirthDay2014Doc::Decode983(WReceivedPacket* pPacket)
{
	m_missions.clear();
	unsigned int count = pPacket->Decode4();
	for (unsigned int i = 0; i < count; ++i)
	{
		unsigned long id = pPacket->Decode4();
		unsigned long value = pPacket->Decode4();
		unsigned long n = pPacket->Decode4();
		bool bDone = pPacket->Decode1() == 1;
		bool bRewarded = pPacket->Decode1() == 1;
		m_missions.push_back(sArinMission983(id, value, n, bDone, bRewarded));
	}
}

// 00920050
bool CArinBirthDay2014Doc::Release()
{
	if (m_pHandler)
	{
		delete m_pHandler;
		m_pHandler = NULL;
	}
	return true;
}

// 00921360
bool CArinBirthDay2014Doc::Initialize()
{
	m_pHandler = _util::NewCreator<CArinBirthDay2014Handler>::CreateObject();
	return true;
}
