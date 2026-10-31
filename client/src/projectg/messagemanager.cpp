#include "minatl.h"

#include "actor.h"
#include "messagemanager.h"

using namespace _systemmsg;

CMessageManager::CMessageManager()
{
}

CMessageManager::~CMessageManager()
{
}

bool CMessageManager::InsertMessage(eMsgIdentifier id, const char* msg)
{
	std::map<eMsgIdentifier, const char*>::const_iterator it =
		m_msgMap.find(id);
	if (it != m_msgMap.end())
	{
		return false;
	}

	m_msgMap.insert(std::make_pair(id, msg));

	return true;
}

bool CMessageManager::InsertMessageGroup(sMsgInfo* info, int num)
{
	// 983 (00919260): a contagem e info[num].id, nao num; os chamadores do 983 passam num = ultima entrada e o
	// laco segue pela pilha (ids lixo). B1408: os nossos chamadores terminam com { N, NULL } para a contagem ser N
	// (lixo da nossa pilha podia registrar 300..303 antes do grupo da CGolfItemDoc -> GetMsg(0x12f) errado na asa)
	int count = info[num].id;
	for (int i = 0; i < count; ++i)
	{
		std::map<eMsgIdentifier, const char*>::const_iterator it =
			m_msgMap.find(info[i].id);
		if (it != m_msgMap.end())
		{
			return false;
		}

		m_msgMap.insert(std::make_pair(info[i].id, info[i].msg));
	}

	return true;
}

const char* CMessageManager::GetMsg(eMsgIdentifier id)
{
	std::map<eMsgIdentifier, const char*>::const_iterator it =
		m_msgMap.find(id);
	if (it == m_msgMap.end())
	{
		return NULL;
	}

	return (*it).second;
}

void CMessageManager::NotifyNormalMessage(eMsgIdentifier id)
{
	const char* msg = GetMsg(id);

	if (msg == NULL)
	{
		msg = K2L_Compatibility( // 983 00919359
			"\xc1\xf6\xc1\xa4\xb5\xc7\xc1\xf6 \xbe\xca\xc0\xba \xb8\xde\xbd\xc3\xc1\xf6 \xc4\xda\xb5\xe5\xc0\xd4\xb4\xcf\xb4\xd9.");
	}

	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), (int)msg, 0, 0, 0, 0));
}

void CMessageManager::NotifyNormalMessage(const char* msg)
{
	if (msg == NULL)
	{
		return;
	}

	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), (int)msg, 0, 0, 0, 0));
}
