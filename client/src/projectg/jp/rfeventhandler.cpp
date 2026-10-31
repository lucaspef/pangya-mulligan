// JP 983: RFEventHandler.cpp da Refresh (B155, F3.7.13): os eventos por nome de todo objeto da RF (0042f5a0–0042fb3d). Base do
// RFWnd e do RFManager: sem o construtor o hash_map dos eventos ficava sem iniciar (stub) e o 1º SubscribeEvent de uma janela RF caía.
// RFEvent, Subscriber e os functores estão em linha no rfeventhandler.h.
#include <string>
#include "refresh/lib/rfeventhandler.h"

// 0042f5a0
RFEventHandler::RFEventHandler()
{
}

// 0042f600: os assinantes de cada evento saem antes do mapa
RFEventHandler::~RFEventHandler()
{
	for (EventMap::iterator itor = _events.begin(); itor != _events.end(); ++itor)
		itor->second.Cleanup();
}

// 0042f760: evento novo sem assinantes (nome repetido não faz nada)
void RFEventHandler::AddEvent(const std::wstring& name)
{
	if (_events.find(name) == _events.end())
		_events.insert(EventMap::value_type(name, RFEvent(name)));
}

// 0042f9b0
void RFEventHandler::RemoveEvent(const std::wstring& name)
{
	EventMap::iterator itor = _events.find(name);
	if (itor != _events.end())
	{
		itor->second.ClearSubscribers();
		_events.erase(itor);
	}
}

// 0042fa20: o 983 devolve "find == end" (verdadeiro quando o evento NÃO existe; sete al depois do cmp). Mantido como no 983.
bool RFEventHandler::IsEventPresent(const std::wstring& name) const
{
	return _events.find(name) == _events.end();
}

// 0042fa50: assinar um evento que não existe cria o evento
void RFEventHandler::SubscribeEvent(const std::wstring& name, const Subscriber& subscriber)
{
	EventMap::iterator itor = _events.find(name);
	if (itor == _events.end())
	{
		AddEvent(name);
		itor = _events.find(name);
	}
	itor->second.Subscribe(subscriber);
}

// 0042faa0
void RFEventHandler::ClearEventSubscribers(const std::wstring& name)
{
	EventMap::iterator itor = _events.find(name);
	if (itor != _events.end())
		itor->second.ClearSubscribers();
}

// 0042fb00
R_ERRORTYPE RFEventHandler::SendEvent(const std::wstring& name, EventArgs& e)
{
	EventMap::iterator itor = _events.find(name);
	if (itor == _events.end())
		return R_NOTFOUND983;
	itor->second(e);
	return R_OK983;
}
