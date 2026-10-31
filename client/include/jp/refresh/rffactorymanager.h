#pragma once
// JP 983: fábricas de janela da Refresh (src\RFFactoryManager.cpp; jp/rffactorymanager.cpp). RFWindowFactory (vtable 00d64188,
// 0x20: nome do tipo em +0x04; as derivadas RFButtonFactory, RFLabelFactory… são do item (4)/(5)) e RFWindowFactoryManager
// (singleton do RFManager: apelidos +0x00, fábricas +0x10). Nomes dos métodos do manager pelo 839; os da fábrica meus.
#include <map>
#include <string>
#include "refresh/lib/rfapi.h"

class RFWnd;

// B151: exportada pelo 983 (cópias fora de linha no RFWnd.cpp 00416f80–00417050: destrutor, GetTypeName, construtor protegido,
// construtor de cópia e operator= implícitos; destrutor de apagar 004170c0 = push 0x20)

class RF_API RFWindowFactory
{
protected:
	RFWindowFactory(const std::wstring& type) : _type(type) {} // 00416fd0
public:
	virtual ~RFWindowFactory() {} // 00416f80
	virtual RFWnd* Create() = 0; // RFButtonFactory: 00432810 (new RFButton, 0x1f4)
	virtual void Destroy(RFWnd* wnd) = 0; // RFButtonFactory: 00432880 (delete wnd)

	const std::wstring& GetTypeName() const { return _type; } // 00416fc0

protected:
	std::wstring _type; // +0x04

	friend class RFWindowFactoryManager;
};

class RFWindowFactoryManager
{
public:
	RFWindowFactoryManager() { }
	RFWindowFactoryManager(const RFWindowFactoryManager& other); // 00417320 (B268)
	RFWindowFactoryManager& operator=(const RFWindowFactoryManager& other); // 00417390
	void AddFactory(RFWindowFactory* factory); // 00437650
	void RemoveFactory(RFWindowFactory* factory); // 004377b0
	void RemoveFactory(const std::wstring& type); // 00437820
	RFWindowFactory* GetFactory(const std::wstring& type) const; // 00437890 (resolve o apelido antes)
	void AddTypeAlias(const std::wstring& type, const std::wstring& alias); // 004379d0
	void RemoveTypeAlias(const std::wstring& alias); // 00437ba0

protected:
	typedef std::map<std::wstring, std::wstring> AliasMap;
	typedef std::map<std::wstring, RFWindowFactory*> FactoryMap;
	AliasMap _aliases; // +0x00
	FactoryMap _factories; // +0x10
};
