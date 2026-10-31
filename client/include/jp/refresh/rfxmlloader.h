#pragma once
// JP 983: leitor de layout da Refresh (RTTI .?AVRFXmlLoader@@, vtable 00d659fc; src\RFXmlLoader.cpp pelo assert de 00444800).
// O RFManager::CreateWnd (00417a90) cria um na pilha com o RFInterface do gerenciador, chama Load(arquivo) e pega a raiz
// (GetRoot) e as filhas (GetChild/GetChildCount); o caminho da animação vem da propriedade "UIAnimation". O XML tem
// <refresh version="N">: <properties>, <Textures> (conjuntos de imagem, rfimageset.h) e as janelas (versão 1: <base> + <child>;
// versão 2: ReadElements). Corpos em jp/rfxmlloader.cpp (B147: 0043dc30–00440290; B148: 00440480–00447e70, os leitores
// por tipo e os estilos). Nomes dos métodos e campos meus (a RTTI só dá a classe).
#include <map>
#include <string>
#include <vector>
#include "refresh/rfinterface.h"
#include "refresh/rfwndstruct.h"
#include "refresh/tinyxml/tinyxml.h"

class RFXmlLoader
{
public:
	typedef std::vector<WndBaseStruct*> StructList;
	typedef std::map<WndBaseStruct*, StructList> ChildMap;
	typedef std::map<std::string, ImageSet*> ImageSetMap;

	explicit RFXmlLoader(RFInterface* rfInterface); // 0043dc30
	virtual ~RFXmlLoader(); // 0043de70 (??_G 0043dd80)

	int Load(const std::wstring& filename); // 0043e380
	void Clear(); // 0043df70

	unsigned int GetVersion() const; // 0043e020
	WndBaseStruct* GetRoot() const; // 0043e5c0
	int GetStructCount() const; // 0043e610
	WndBaseStruct* GetChild(WndBaseStruct* parent, int index); // 0043e5d0
	int GetChildCount(WndBaseStruct* parent); // 0043e620
	std::wstring GetAnimationPath() const { return _animationPath; } // 00416ee0 (cópia fora de linha na unidade do RFManager)
	ImageSet* GetImageSet(const std::string& name); // 0043f7a0 (com AddRef)

	void LoadDocument(const std::wstring& filename, rf::TiXmlDocument& document); // 0043e030
	void LoadInclude(const std::wstring& filename, WndBaseStruct* parent); // 0043e240
	int ReadProperties(const rf::TiXmlElement* root); // 0043e660
	int ReadTextures(const rf::TiXmlElement* root); // 0043e820
	void ReadFrameImageSet(const std::string& name, const rf::TiXmlElement* texture); // 0043eb30
	void ReadSingleImageSet(const std::string& name, const rf::TiXmlElement* texture); // 0043f010
	void ReadNinePatchImageSet(const std::string& name, const rf::TiXmlElement* texture); // 0043f290
	void AddImageSet(const std::string& name, ImageSet* imageSet); // 0043f670
	int ReadVersion1(const rf::TiXmlElement* root); // 0043f7e0
	int ReadChildrenV1(WndBaseStruct* parent, const rf::TiXmlElement* element); // 0043f9c0

	// versão 2 (B148: 00440480–00447c9f): elementos <child>/<base type="…">, estilos e os leitores por tipo
	int ReadElements(WndBaseStruct* parent, const rf::TiXmlElement* element, bool style); // 00440cb0
	bool ReadStyle(WndBaseStruct* parent, const rf::TiXmlElement* element); // 00440480 (arquivo do atributo "style")
	void ApplyStyle(WndBaseStruct* base, const rf::TiXmlElement* element); // 004407f0
	void ApplyAnchors(const RFRect983& oldRect, WndBaseStruct* base); // 00440640
	void ApplyStyleText(const std::wstring& target, const std::wstring& value, WndBaseStruct* base); // 00440b50
	int ReadWnd(const rf::TiXmlElement* element, WndBaseStruct* base); // 00441a50
	int ReadWndEx(const rf::TiXmlElement* element, WndExBase* base); // 00442070
	int ReadLabel(const rf::TiXmlElement* element, LabelBase* base); // 00442a50
	void ReadLabelStyle(const rf::TiXmlElement* element, LabelBase* base); // 00442b70
	int ReadButton(const rf::TiXmlElement* element, ButtonBase* base); // 00442c90
	void ReadButtonSounds(const rf::TiXmlElement* element, ButtonBase* base); // 00443710
	int ReadLabelButton(const rf::TiXmlElement* element, LabelButtonBase* base); // 004439e0
	int ReadAnimButton(const rf::TiXmlElement* element, AnimButtonBase* base); // 00443c70
	int ReadCheck(const rf::TiXmlElement* element, CheckBase* base); // 004442f0
	int ReadListBase(const rf::TiXmlElement* element, ListBase* base); // 00444800
	void ReadListStyle(const rf::TiXmlElement* element, ListBase* base); // 00444ad0
	int ReadList(const rf::TiXmlElement* element, ListSCBase* base); // 00444d40
	int ReadTextAreaBase(const rf::TiXmlElement* element, TextAreaBase* base); // 00445050
	void ReadTextAreaStyle(const rf::TiXmlElement* element, TextAreaBase* base); // 004455d0
	int ReadTextArea(const rf::TiXmlElement* element, TextAreaSCBase* base); // 00445730
	void ReadTextAreaSCStyle(const rf::TiXmlElement* element, TextAreaSCBase* base); // 00445bf0
	int ReadEdit(const rf::TiXmlElement* element, EditBase* base); // 00445e20
	void ReadEditStyle(const rf::TiXmlElement* element, EditBase* base); // 00446470
	int ReadCombo(const rf::TiXmlElement* element, ComboBase* base); // 00446690
	int ReadScrollbar(const rf::TiXmlElement* element, ScrollbarBase* base); // 004469a0
	int ReadSlider(const rf::TiXmlElement* element, SliderBase* base); // 00446ce0
	int ReadStackPanel(const rf::TiXmlElement* element, StackPanelBase* base); // 00447020 (sem chamador: embutido)
	int ReadPicture(const rf::TiXmlElement* element, PictureBase* base); // 00447040
	int ReadProgress(const rf::TiXmlElement* element, ProgressBase* base); // 00447620
	int ReadCustom(const rf::TiXmlElement* element, CustomBase* base); // 00447be0

	unsigned int _version; // +0x04 (atributo "version" da raiz)
	bool _loaded; // +0x08
	rf::TiXmlDocument _document; // +0x0c
	RFInterface* _interface; // +0x54
	StructList _structs; // +0x58 (janelas de topo; a 1ª é a raiz)
	ChildMap _children; // +0x68 (filhas de cada estrutura; mapa do VC10 = 0x10)
	std::wstring _animationPath; // +0x78 (propriedade "UIAnimation")
	WndBaseStruct* _style; // +0x94 (última estrutura lida de um arquivo de estilo; ReadElements com style)
	ImageSetMap _imageSets; // +0x98 (sizeof 0xa8)
};
