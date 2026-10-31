#pragma once

#include <functional>
#include <map>
#include <vector>
#include "../shared/localize.h"

#include "icontentsdatacontainer.h"

class CContentsDoc : public WSingleton<CContentsDoc>
{
public:
	CContentsDoc();
	virtual ~CContentsDoc();

	bool Initialize();
	bool Release();

	int InsertContainer(localContentType_t type,
		IContentsDataContainer* pContainer, bool bForce);
	IContentsDataContainer* GetContainer(localContentType_t type);

	// JP 983 (pedido B13): os documentos do Gateway não são mais criados um a um no Initialize. Cada unidade registra um
	// criador num inicializador estático (CreateCreatorList983 00912b70 + RegisterCreator983 00914760) e o Initialize
	// (00915da0) chama cada criador e insere o documento pelo tipo que ele mesmo guarda (+0x08), por 00913d10.
	typedef std::tr1::function<IContentsDataContainer*()> Creator983;
	static bool CreateCreatorList983(); // 00912b70
	static void __stdcall RegisterCreator983(Creator983 creator); // 00914760
	int InsertContainer(localContentType_t type,
		IContentsDataContainer* pContainer); // 00913d10

	template <class T>
	bool GetContainer(localContentType_t type, T& pContainer)
	{
		IContentsDataContainer* p = GetContainer(type);

		if (p != NULL)
		{
			pContainer = (T)p;
			return true;
		}

		return false;
	}

protected:
	std::map<localContentType_t, IContentsDataContainer*> m_containerMap;
	int m_bInitialized;

	static std::vector<Creator983>* s_pCreators983; // 00e7e1b0
};
