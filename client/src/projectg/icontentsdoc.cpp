#include "minatl.h"
#include <algorithm>

#define REBANG_ICONTENTSDOC
#include "icontentsdoc.h"

#include "contentsdoc.h"
#include "gatewaydoc.h"

// HACK: template instantiation order
typedef char rb_msgMapSize[sizeof(std::map<int, const char*>)];
typedef char
	rb_memberPageMapSize[sizeof(std::map<int, std::list<GUILD_USER_LIST> >)];
typedef char
	rb_searchPageMapSize[sizeof(std::map<int, std::list<GUILD_LIST> >)];

#include "guildinfo.h"
#include "gimmickcontainer.h"
#include "packets/tutorialcontainer.h"
#include "packets/aiplayerinfocontainer.h"
#include "jp/lobbycontainer983.h" // B192.1

// JP 983 (D74/D210): documentos 0x9f e 0xa0 do CContentsDoc::Initialize (00915da0). Sem fonte no 645; so o layout e o
// ciclo de vida. Nomes do RTTI.
// CRandomCourseSound (0x10 bytes, vtable d3c630): sons aleatorios por mapa (lido pelo CAmbientSound::OnLoad). O Initialize
// (0090f700) preenche tres entradas (mapas 0x12, 0x40, 0x14) por tres funcoes; Release = true (00907920); Reset = o da base.
class CRandomCourseSound : public IContentsDataContainer
{
public:
	CRandomCourseSound() { }
	virtual bool Initialize();
	virtual bool Release() { return true; }

private:
	void AddIceInferno(); // 0090edb0
	void AddHio1(); // 0090ef10
	void AddAbbotMine(); // 0090f030

	std::map<unsigned char, std::vector<const char*> > m_map;
};

void CRandomCourseSound::AddIceInferno()
{
	std::vector<const char*> sounds;
	const char* name;
	name = "oneshot_ice_inferno1"; sounds.push_back(name);
	name = "oneshot_ice_inferno2"; sounds.push_back(name);
	name = "oneshot_ice_inferno3"; sounds.push_back(name);
	name = "oneshot_ice_inferno4"; sounds.push_back(name);
	name = "oneshot_ice_inferno5"; sounds.push_back(name);
	name = "oneshot_ice_inferno6"; sounds.push_back(name);
	name = "oneshot_ice_inferno7"; sounds.push_back(name);
	name = "oneshot_ice_inferno8"; sounds.push_back(name);
	m_map.insert(std::make_pair(0x12, sounds));
}

void CRandomCourseSound::AddHio1()
{
	std::vector<const char*> sounds;
	const char* name;
	name = "oneshot_hio1_1"; sounds.push_back(name);
	name = "oneshot_hio1_2"; sounds.push_back(name);
	name = "oneshot_hio1_4"; sounds.push_back(name);
	name = "oneshot_hio1_5"; sounds.push_back(name);
	name = "oneshot_hio1_6"; sounds.push_back(name);
	m_map.insert(std::make_pair(0x40, sounds));
}

void CRandomCourseSound::AddAbbotMine()
{
	std::vector<const char*> sounds;
	const char* name;
	name = "oneshot_abbot_mine1"; sounds.push_back(name);
	name = "oneshot_abbot_mine2"; sounds.push_back(name);
	name = "oneshot_abbot_mine3"; sounds.push_back(name);
	m_map.insert(std::make_pair(0x14, sounds));
}

bool CRandomCourseSound::Initialize()
{
	AddIceInferno();
	AddHio1();
	AddAbbotMine();
	return true;
}

// LobbyAvatarContainer (0x10 bytes, vtable d3c614): Initialize = true (00907930); Release = Reset(); true (00907940);
// Reset limpa a lista (0090c620).
// Elemento da lista (983: dtor 0058ea00): dois std::string em +0x208/+0x224 e a flag "nao e dono" em +0x244. So o que o
// Reset (0090c620 + 0090bb20) usa; o resto do objeto nao e levantado aqui. E o sLobbyAvatarInfo983 de 0x248 B
// (jp/lobbyavataractor.h); com o nome sLobbyAvatar983 (o de 0x3c do Lounge) o linker ficava com UM ~sLobbyAvatar983 e o
// LoadAvatar983 destruia strings 0x208 B alem do local (D715).
struct sLobbyAvatarItem983
{
	unsigned char m_pad0[0x208];
	std::string m_str0;
	std::string m_str1;
	unsigned char m_pad1[4];
	bool m_bKeep;
};

typedef decltype(std::list<sLobbyAvatarItem983*>().begin()._Ptr) LobbyAvatarNode983;

// 983 (0090bb20): apaga os elementos que o container possui (flag +0x244 == 0); devolve o 3o parametro
static bool __stdcall DestroyLobbyAvatars983(LobbyAvatarNode983 first,
	LobbyAvatarNode983 last, bool bResult)
{
	for (; first != last; first = first->_Next)
	{
		sLobbyAvatarItem983* p = first->_Myval;
		if (!p->m_bKeep)
			delete p;
	}
	return bResult;
}

class LobbyAvatarContainer : public IContentsDataContainer
{
public:
	LobbyAvatarContainer() { }
	virtual bool Initialize() { return true; }
	virtual bool Release() { Reset(); return true; }
	virtual bool Reset()
	{
		DestroyLobbyAvatars983(m_list.begin()._Ptr, m_list.end()._Ptr, false);
		m_list.clear();
		return true;
	}
	std::list<sLobbyAvatarItem983*> m_list;
};

CContentsDoc::CContentsDoc()
	: m_bInitialized(1)
{
	// HACK: emits the unreferenced g_EPSILON (.rdata) like the original
	(void)&g_EPSILON;
}

CContentsDoc::~CContentsDoc()
{
	// 983 (00914340): libera a lista estatica de criadores antes do Release
	delete s_pCreators983;
	s_pCreators983 = NULL;
	Release();
}

std::vector<CContentsDoc::Creator983>* CContentsDoc::s_pCreators983;

bool CContentsDoc::CreateCreatorList983()
{
	if (s_pCreators983 == NULL)
		s_pCreators983 = new std::vector<Creator983>;
	return true;
}

void __stdcall CContentsDoc::RegisterCreator983(Creator983 creator)
{
	s_pCreators983->push_back(creator);
}

// JP 983 (00913d10): tipo já presente -> 0; senão Initialize do documento e insere
int CContentsDoc::InsertContainer(localContentType_t type,
	IContentsDataContainer* pContainer)
{
	localContentType_t key = type;
	if (m_containerMap.find(key) != m_containerMap.end())
		return 0;

	int result = 1;
	if (pContainer != NULL && pContainer->Initialize())
		m_containerMap.insert(std::make_pair(type, pContainer));
	else
		result = 0;

	m_bInitialized &= result;
	return result;
}

// 983 (00914030): for_each sobre os criadores registrados, com o corpo do funtor embutido; o funtor guarda so o this
struct sInsertCreated983
{
	CContentsDoc* m_pDoc;
	explicit sInsertCreated983(CContentsDoc* pDoc) : m_pDoc(pDoc) { }
	sInsertCreated983(const sInsertCreated983& other) : m_pDoc(other.m_pDoc) { }
};

static sInsertCreated983 ForEachCreators983(
	std::tr1::function<IContentsDataContainer*()>* pFirst,
	std::tr1::function<IContentsDataContainer*()>* pLast, sInsertCreated983 fn)
{
	for (; pFirst != pLast; ++pFirst)
	{
		std::tr1::function<IContentsDataContainer*()> creator = *pFirst;
		_gateway::CGatewayControlDoc* pDoc =
			(_gateway::CGatewayControlDoc*)creator();
		fn.m_pDoc->InsertContainer(pDoc->GetType983(), pDoc);
	}
	return fn;
}

// JP 983 (00915da0). O 983 insere oito documentos aqui e depois os dos criadores registrados (os do Gateway, tipos
// 0xa9-0xb0). Os documentos de evento do 645 (CWorldTourEvent, CHalloweenEvent, CChristmasEvent, CTicketExchange,
// CMissionEvent, CPeriodContents, CChristmasSockEvent, _gateway::CSecondPwdDoc, _gateway::CTradeDoc) não existem mais no
// exe 983 (sem RTTI) e não são criados: GetContainer devolve NULL para eles, como no 983.
bool CContentsDoc::Initialize()
{
	// 983: o resultado de cada InsertContainer n�o � usado aqui (ele mesmo faz m_bInitialized &= resultado).
	InsertContainer((localContentType_t)0x1e, new CGiftBoxEvent(), true);
	InsertContainer((localContentType_t)0x9a, new CVectorSlideTestContainer(), false);
	InsertContainer((localContentType_t)0x9b, new CGimmickContainer(), false);
	InsertContainer((localContentType_t)0x9f, new CRandomCourseSound(), false);
	InsertContainer((localContentType_t)0xa0, new LobbyAvatarContainer(), false);
	// 0xa3 CAIPlayerInfoContainer (0x34, construtor 00915b90; pedido C75.2): linhas .aibin e resultados das IAs (pacote 0x256).
	InsertContainer((localContentType_t)0xa3, new CAIPlayerInfoContainer983, false);
	// 0xa4 CTutorialContainer (0x18; 00915f9f, pedido C58.1): estado do tutorial (CTutorial +0xdac, CTutorialMain +0x458).
	InsertContainer((localContentType_t)0xa4, new CTutorialContainer983, false);
	// 0xa5 CLobbyContainer (0x1c, vtable 00d3c600; 00915fc8; B192.1/C102.1): doc da lista de salas/sala (dono em +0x14)
	InsertContainer((localContentType_t)0xa5, new CLobbyContainer983, false);
	if (s_pCreators983 != NULL)
	{
		ForEachCreators983(s_pCreators983->begin()._Ptr,
			s_pCreators983->end()._Ptr, sInsertCreated983(this));
	}

	if (m_bInitialized == 1)
	{
		return true;
	}

	return false;
}

bool CContentsDoc::Release()
{
	bool bResult = true;

	std::map<localContentType_t, IContentsDataContainer*>::iterator it;
	for (it = m_containerMap.begin(); it != m_containerMap.end(); ++it)
	{
		if ((*it).second != NULL)
		{
			if (!(*it).second->Release())
			{
				bResult = false;
			}

			delete (*it).second;
			(*it).second = NULL;
		}
	}

	m_containerMap.clear();

	return bResult;
}

int CContentsDoc::InsertContainer(localContentType_t type,
	IContentsDataContainer* pContainer, bool bForce)
{
	int result = 1;

	if ((IsLocalContent(type) && pContainer != NULL) || bForce == true)
	{
		if (pContainer->Initialize())
		{
			m_containerMap.insert(std::make_pair(type, pContainer));
		}
		else
		{
			result = 0;
		}
	}
	else if (pContainer != NULL)
	{
		delete pContainer;
	}
	else
	{
		result = 0;
	}

	m_bInitialized &= result;
	return result;
}

IContentsDataContainer* CContentsDoc::GetContainer(localContentType_t type)
{
	std::map<localContentType_t, IContentsDataContainer*>::iterator it;
	it = m_containerMap.find(type);

	if (it != m_containerMap.end())
	{
		return (*it).second;
	}

	return NULL;
}
