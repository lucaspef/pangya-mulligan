#include "minatl.h"

#define REBANG_GATEWAYDOC_CPP
#include "gatewaydoc.h"
#include "messagemanager.h"
#include "specialbox.h"
#include "gatewayactor.h"
#include "jp/workshopevent2013handler.h" // 983 (B126.1): manipulador do CWorkShop2013Doc

using namespace _gateway;

// JP 983 (009204f0): só grava o tipo e constrói o vetor; os contadores não são zerados aqui
CGolfItemDoc::CGolfItemDoc(int type)
	: CGatewayControlDoc(type)
{
}

bool CGolfItemDoc::Initialize()
{
	// 983 (009206d0): zera os tres contadores antes de criar o manipulador (o construtor nao os zera)
	m_calipersCount = 0;
	m_specialBoxCount = 0;
	m_ownSpecialBoxCount = 0;

	// 983: so cria o manipulador (o Initialize dele nao e chamado aqui)
	m_pHandler = _util::NewCreator<CGolfItemHandler>::CreateObject();

	for (int i = 0; i < 2; ++i)
	{
		m_usableItemList.push_back(0);
	}

	// 983 (009206d0): textos do 983 (300 "guarda ate %d", 301/302 sem "(aviso)", 303 novo) via K2L_Compatibility;
	// sem ele o coreano saia cru na partida (dono 23/10: 3 linhas vermelhas ilegiveis)
	_systemmsg::sMsgInfo msgInfo[] = {
		{ (_systemmsg::eMsgIdentifier)300,
         K2L_Compatibility("(\xbe\xcb\xb8\xb2) %s \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba %d\xb0\xb3\xb1\xee\xc1\xf6 \xba\xb8\xb0\xfc \xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9.") },
		{ (_systemmsg::eMsgIdentifier)301,
         K2L_Compatibility("\xb4\xf5 \xc0\xcc\xbb\xf3 %s \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbe\xf2\xc0\xbb \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.") },
		{ (_systemmsg::eMsgIdentifier)302,
         K2L_Compatibility("\xc7\xf6\xc0\xe7 %d\xb0\xb3 \xba\xb8\xc0\xaf, %d\xb0\xb3 \xc8\xb9\xb5\xe6\xb0\xa1\xb4\xc9") },
		{ (_systemmsg::eMsgIdentifier)303,
         K2L_Compatibility("%s \xc0\xc7 \xbd\xc5\xba\xf1\xc7\xd1 \xb4\xc9\xb7\xc2\xc0\xcc \xb9\xdf\xb5\xbf\xb5\xc7\xbe\xee \xc1\xf6\xc7\xfc\xc0\xc7 \xb1\xe2\xbf\xef\xb1\xe2\xb0\xa1 \xbb\xe7\xb6\xf3\xc1\xfd\xb4\xcf\xb4\xd9.") },
		{ (_systemmsg::eMsgIdentifier)4, NULL }, // B1408: contagem lida em info[num].id (ver InsertMessageGroup)
	};

	_systemmsg::CMessageManager::Instance()->InsertMessageGroup(msgInfo,
		sizeof(msgInfo) / sizeof(msgInfo[0]) - 1);

	return true;
}

bool CGolfItemDoc::Release()
{
	if (m_pHandler)
	{
		delete m_pHandler;
		m_pHandler = NULL;
	}

	return true;
}

void CGolfItemDoc::ResetCalipersVariable()
{
	m_calipersCount = 0; // 983 (0091ffc0): so o contador
}

void CGolfItemDoc::DeceaseCalipersCount()
{
	if (m_calipersCount <= 0)
	{
		m_calipersCount = 0;
	}
	else

		m_calipersCount--;
}

void CGolfItemDoc::DecreaseItemCount2c983()
{
	if (m_2c <= 0)
	{
		m_2c = 0;
	}
	else

		m_2c--;
}


#ifndef REBANG_ICONTENTSDOC
// JP 983 (009201f0)
CSpecialBoxDoc::CSpecialBoxDoc(int type)
	: CGatewayControlDoc(type)
{
}
#endif

bool CSpecialBoxDoc::Initialize()
{
	// 983: so cria o manipulador (o Initialize dele nao e chamado aqui)
	m_pHandler = _util::NewCreator<CSpecialBoxHandler>::CreateObject();

	_systemmsg::sMsgInfo msgInfo[] = {
		{ (_systemmsg::eMsgIdentifier)200,
         K2L_Compatibility("\xbb\xf3\xc0\xda\xb8\xa6 \xbf\xad\xb1\xe2 \xc0\xa7\xc7\xd1 \xc0\xe7\xb7\xe1\xb0\xcb\xbb\xf6\xc0\xbb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.")                          },
		{ (_systemmsg::eMsgIdentifier)201,
         K2L_Compatibility("\xbb\xf3\xc0\xda\xb8\xa6 \xbf\xad\xb1\xe2 \xc0\xa7\xc7\xd8 \xc7\xca\xbf\xe4\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xc3\xa3\xc0\xbb \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.") },
		{ (_systemmsg::eMsgIdentifier)202,
         K2L_Compatibility("\xbb\xf3\xc0\xda\xb8\xa6 \xbf\xad\xb1\xe2 \xc0\xa7\xc7\xd8 \xc7\xca\xbf\xe4\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb \xb0\xb3\xbc\xf6\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9.")  },
		{ (_systemmsg::eMsgIdentifier)203,
         K2L_Compatibility("%s \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xb8\xb7\xce \xbb\xf3\xc0\xda\xb8\xa6 \xbf\xad\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.")                                                          },
		{ (_systemmsg::eMsgIdentifier)4, NULL }, // B1408: contagem lida em info[num].id (ver InsertMessageGroup)
	};

	_systemmsg::CMessageManager::Instance()->InsertMessageGroup(msgInfo,
		sizeof(msgInfo) / sizeof(msgInfo[0]) - 1);

	return true;
}

bool CSpecialBoxDoc::Release()
{
	if (m_pHandler)
	{
		delete m_pHandler;
		m_pHandler = NULL;
	}

	return true;
}

#ifndef REBANG_ICONTENTSDOC
// JP 983 (00920280)
CWorkShop2013Doc::CWorkShop2013Doc(int type)
	: CGatewayControlDoc(type)
{
}
#endif

// JP 983 (009202e0): só cria o manipulador (o Initialize dele não é chamado aqui). 009200f0 é a cópia fora de linha do
// NewCreator<CWorkShopEvent2013Handler>::CreateObject.
bool CWorkShop2013Doc::Initialize()
{
	m_pHandler = (CGatewayHandler*)(CGatewayHandlerJp*)_util::NewCreator<CWorkShopEvent2013Handler>::CreateObject();
	return true;
}

// JP 983 (00920030)
bool CWorkShop2013Doc::Release()
{
	if (m_pHandler)
	{
		delete m_pHandler;
		m_pHandler = NULL;
	}

	return true;
}

// JP 983: criadores dos documentos do Gateway desta unidade (CGolfItemDoc 00920640 -> 0xa9, registro 00920680;
// CSpecialBoxDoc 00920260 -> 0xab, registro 009208b0; CWorkShop2013Doc 009202c0 -> 0xae, registro 00920900). O
// CArinBirthDay2014Doc (0xb0, criador 00920be0) também mora nesta faixa do exe, mas está escrito pela B (com o registro) em
// jp/arinbirthday2014doc.cpp: não registrar aqui.
static IContentsDataContainer* CreateGolfItemDoc983()
{
	return new CGolfItemDoc(0xa9);
}

static IContentsDataContainer* CreateSpecialBoxDoc983()
{
	return new CSpecialBoxDoc(0xab);
}

static IContentsDataContainer* CreateWorkShop2013Doc983()
{
	return new CWorkShop2013Doc(0xae);
}

static bool RegisterGolfItemDoc983()
{
	CContentsDoc::CreateCreatorList983();
	CContentsDoc::RegisterCreator983(&CreateGolfItemDoc983);
	return true;
}

static bool RegisterSpecialBoxDoc983()
{
	CContentsDoc::CreateCreatorList983();
	CContentsDoc::RegisterCreator983(&CreateSpecialBoxDoc983);
	return true;
}

static bool RegisterWorkShop2013Doc983()
{
	CContentsDoc::CreateCreatorList983();
	CContentsDoc::RegisterCreator983(&CreateWorkShop2013Doc983);
	return true;
}

static bool s_bGolfItemDoc983 = RegisterGolfItemDoc983();
static bool s_bSpecialBoxDoc983 = RegisterSpecialBoxDoc983();
static bool s_bWorkShop2013Doc983 = RegisterWorkShop2013Doc983();
