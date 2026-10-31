// JP 983: Refresh, src\RFFactoryManager.cpp — RFWindowFactoryManager (00437650–00437c10 + moldes do std::map até 00438d40).
// F3.7.13 item (2), sessão B145. LTCG: prova pela dll de prova (out/F3.7/s145/RELATORIO.md). Assert com o __FILE__/linha do 983.
#undef NDEBUG	// a Refresh foi compilada sem NDEBUG (_wassert no 983)
#include <assert.h>
#include "refresh/rffactorymanager.h"

// 00437650
void RFWindowFactoryManager::AddFactory(RFWindowFactory* factory)
{
	if (factory == NULL)
		return;

	if (_factories.find(factory->_type) == _factories.end())
		_factories.insert(std::make_pair(factory->_type, factory));
}

// 004377b0
void RFWindowFactoryManager::RemoveFactory(RFWindowFactory* factory)
{
	FactoryMap::iterator it = _factories.find(factory->_type);
	if (it != _factories.end())
		_factories.erase(it);
}

// 00437820
void RFWindowFactoryManager::RemoveFactory(const std::wstring& type)
{
	FactoryMap::iterator it = _factories.find(type);
	if (it != _factories.end())
		_factories.erase(it);
}

// 00437890
RFWindowFactory* RFWindowFactoryManager::GetFactory(const std::wstring& type) const
{
	std::wstring name;
	AliasMap::const_iterator alias = _aliases.find(type);
	name = (alias != _aliases.end()) ? alias->second : type;

	FactoryMap::const_iterator it = _factories.find(name);
	if (it == _factories.end())
		return NULL;
	return it->second;
}

#line 54 "src\\RFFactoryManager.cpp"
// 004379d0
void RFWindowFactoryManager::AddTypeAlias(const std::wstring& type, const std::wstring& alias)
{
	assert(_aliases.find(alias) == _aliases.end());
	_aliases.insert(std::make_pair(alias, type));
}

// 00437ba0
void RFWindowFactoryManager::RemoveTypeAlias(const std::wstring& alias)
{
	AliasMap::iterator it = _aliases.find(alias);
	if (it != _aliases.end())
		_aliases.erase(it);
}

// 00417320: copia os dois mapas (+0 apelidos, +0x10 fábricas)
RFWindowFactoryManager::RFWindowFactoryManager(const RFWindowFactoryManager& other)
	: _aliases(other._aliases), _factories(other._factories)
{
}

// 00417390
RFWindowFactoryManager& RFWindowFactoryManager::operator=(const RFWindowFactoryManager& other)
{
	if (this != &other)
	{
		_aliases = other._aliases;
		_factories = other._factories;
	}
	return *this;
}
