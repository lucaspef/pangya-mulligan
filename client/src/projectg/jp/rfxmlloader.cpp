// JP 983: Refresh, src\RFXmlLoader.cpp (F3.7.13 item 2, 3º lote, B147; LTCG — prova pela dll de prova, ver
// out/F3.7/s147/RELATORIO.md): o RFXmlLoader (0043dc30–00440290) — carga do arquivo pelo RFInterface, TinyXML da Refresh,
// propriedades, conjuntos de imagem e o leitor da versão 1 (<base> + <child>). Sem assert nesta faixa. Nomes meus.
#undef NDEBUG	// a Refresh foi compilada sem NDEBUG (_wassert no 983)
#include <assert.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include "refresh/rfxmlloader.h"

std::wstring RFUtf8ToWide983(const char* text); // 0042aaa0 (rfmanager.cpp)

// valor do atributo ou o padrão (elemento nulo ou atributo ausente)
static inline const char* RFGetAttribute983(const rf::TiXmlElement* element, const char* name, const char* defaultValue = "")
{
	const char* value = element ? element->Attribute(name) : 0;
	return value ? value : defaultValue;
}

// leitores de valor do XML (B148). No 983 ficaram cópias fora de linha sem chamador (00447d00–00447e70); os leitores os
// embutem. "Child" = atributo do 1º filho com o nome (<padding val="…"/>).
const char* RFAttributeOr983(const rf::TiXmlElement* element, const char* name, const char* defaultValue); // 00447d00
void RFQueryInt983(const rf::TiXmlElement* element, const char* name, int* value, int defaultValue); // 00447d20
void RFQueryFloat983(const rf::TiXmlElement* element, const char* name, float* value, float defaultValue); // 00447d60
void RFQueryChildInt983(const rf::TiXmlElement* element, const char* child, const char* name, int* value,
	int defaultValue); // 00447dd0
void RFQueryChildFloat983(const rf::TiXmlElement* element, const char* child, const char* name, float* value,
	float defaultValue); // 00447e20
void RFQueryChildBool983(const rf::TiXmlElement* element, const char* child, const char* name, bool* value,
	bool defaultValue); // 00447e70
void RFReadPoint983(const rf::TiXmlElement* element, const std::string& name, float* point); // 00441800
void RFReadRect983(const rf::TiXmlElement* element, const std::string& name, RFRect983* rect); // 004418e0

// "Stretch" 0, "Tile" 1
static __forceinline int RFParseTileMode983(const std::string& mode)
{
	if (mode == "Stretch")
		return 0;
	if (mode == "Tile")
		return 1;
	return 0;
}

// 0043dc30
RFXmlLoader::RFXmlLoader(RFInterface* rfInterface)
	: _version(0), _loaded(false), _interface(rfInterface), _style(0)
{
}

// 0043de70
RFXmlLoader::~RFXmlLoader()
{
	Clear();
}

// 0043df70: as estruturas e as imagens não são apagadas aqui
void RFXmlLoader::Clear()
{
	_loaded = false;
	_structs.clear();
	_children.clear();
	_imageSets.clear();
	_document.Clear();
}

// 0043e020
unsigned int RFXmlLoader::GetVersion() const
{
	return _version;
}

// 0043e030
void RFXmlLoader::LoadDocument(const std::wstring& filename, rf::TiXmlDocument& document)
{
	void* file = _interface->FOpen(filename.c_str());
	if (!file)
		RFThrowFileException983(filename, 0);

	_interface->FSeek(file, 0, SEEK_END);
	long size = _interface->FTell(file);
	_interface->FSeek(file, 0, SEEK_SET);
	if (size <= 0)
	{
		_interface->FClose(file);
		RFThrowFileException983(filename, 1);
	}

	std::vector<char> buffer(size + 1);
	_interface->FRead(file, &buffer[0], size);
	_interface->FClose(file);

	document.Parse(&buffer[0], 0, rf::TIXML_DEFAULT_ENCODING);
	if (document.Error())
		RFThrowException983(L"XML parse failed");
}

// 0043e240: <include> de outro arquivo (caminho relativo à base do RFInterface)
void RFXmlLoader::LoadInclude(const std::wstring& filename, WndBaseStruct* parent)
{
	if (filename.size())
	{
		std::wstring path = _interface->GetBasePath(0) + filename;
		rf::TiXmlDocument document;
		LoadDocument(path, document);

		const rf::TiXmlElement* root = document.FirstChildElement("refresh");
		if (!root)
			RFThrowException983(L"Couldn't find root node");
		ReadElements(parent, root, 0);
	}
}

// 0043e380
int RFXmlLoader::Load(const std::wstring& filename)
{
	if (!filename.size())
		RFThrowInvalidArgException983(L"Parameter 'filename' is empty");

	_loaded = false;
	Clear();
	LoadDocument(filename, _document);

	const rf::TiXmlElement* root = _document.FirstChildElement("refresh");
	if (!root)
		RFThrowException983(L"Couldn't find root node");

	for (const rf::TiXmlAttribute* attribute = root->FirstAttribute(); attribute; attribute = attribute->Next())
	{
		if (!strcmp(attribute->Name(), "version"))
			_version = atoi(attribute->Value());
	}

	ReadProperties(root);
	ReadTextures(root);

	if (_version < 2)
	{
		wchar_t text[256];
		wsprintfW(text, L"%s : warning: XML version %d was declared deprecated.\n", filename.c_str(), _version);
		_interface->DebugPrint(text);
	}

	if (_version >= 2)
		ReadElements(0, root, 0);
	else if (_version == 1)
	{
		if (ReadVersion1(root) != 1)
			RFThrowException983(std::wstring(L"XML Version 1 read failed"));
	}
	else
		RFThrowException983(std::wstring(L"Unknown XML version"));

	_loaded = true;
	return 1;
}

// 0043e5c0
WndBaseStruct* RFXmlLoader::GetRoot() const
{
	return _structs[0];
}

// 0043e5d0
WndBaseStruct* RFXmlLoader::GetChild(WndBaseStruct* parent, int index)
{
	ChildMap::iterator it = _children.find(parent);
	if (it == _children.end())
		return 0;
	return it->second[index];
}

// 0043e610
int RFXmlLoader::GetStructCount() const
{
	return (int)_structs.size();
}

// 0043e620
int RFXmlLoader::GetChildCount(WndBaseStruct* parent)
{
	ChildMap::iterator it = _children.find(parent);
	if (it == _children.end())
		return -1;
	return (int)it->second.size();
}

// 0043e660: <properties><property name="UIAnimation" val="…"/></properties>
int RFXmlLoader::ReadProperties(const rf::TiXmlElement* root)
{
	const rf::TiXmlElement* properties = root->FirstChildElement("properties");
	if (properties)
	{
		for (const rf::TiXmlElement* property = properties->FirstChildElement("property"); property;
			property = property->NextSiblingElement("property"))
		{
			if (RFUtf8ToWide983(RFGetAttribute983(property, "name")) == L"UIAnimation")
				_animationPath = RFUtf8ToWide983(RFGetAttribute983(property, "val"));
		}
	}
	return 1;
}

// 0043e820: <Textures><Texture Name="…" Type="Frame|Single|NinePatch">…</Texture></Textures>
int RFXmlLoader::ReadTextures(const rf::TiXmlElement* root)
{
	const rf::TiXmlElement* textures = root->FirstChildElement("Textures");
	if (textures)
	{
		for (const rf::TiXmlElement* texture = textures->FirstChildElement("Texture"); texture;
			texture = texture->NextSiblingElement("Texture"))
		{
			std::string name = texture->Attribute("Name");
			std::string type = texture->Attribute("Type");
			if (type == "Frame")
				ReadFrameImageSet(name, texture);
			else if (type == "Single")
				ReadSingleImageSet(name, texture);
			else if (type == "NinePatch")
				ReadNinePatchImageSet(name, texture);
		}
	}
	return 1;
}

// 0043eb30: 9 <Image Path="…" TileMode="…"/> na ordem do switch
void RFXmlLoader::ReadFrameImageSet(const std::string& name, const rf::TiXmlElement* texture)
{
	std::wstring path;
	int index = 0;
	int tileMode = 0;

	const rf::TiXmlElement* image = texture->FirstChildElement("Image");
	if (image)
	{
		FrameImageSet* imageSet = new FrameImageSet;
		do
		{
			for (const rf::TiXmlAttribute* attribute = image->FirstAttribute(); attribute; attribute = attribute->Next())
			{
				if (!strcmp(attribute->Name(), "Path"))
					path = RFUtf8ToWide983(attribute->Value());
				else if (!strcmp(attribute->Name(), "TileMode"))
					tileMode = RFParseTileMode983(attribute->Value());
			}

			switch (index)
			{
			case 0: imageSet->_background._file = path; imageSet->_background._tileMode = tileMode; break;
			case 1: imageSet->_edgeLT._file = path; imageSet->_edgeLT._tileMode = tileMode; break;
			case 2: imageSet->_edgeLB._file = path; imageSet->_edgeLB._tileMode = tileMode; break;
			case 3: imageSet->_edgeRT._file = path; imageSet->_edgeRT._tileMode = tileMode; break;
			case 4: imageSet->_edgeRB._file = path; imageSet->_edgeRB._tileMode = tileMode; break;
			case 5: imageSet->_frameT._file = path; imageSet->_frameT._tileMode = tileMode; break;
			case 6: imageSet->_frameB._file = path; imageSet->_frameB._tileMode = tileMode; break;
			case 7: imageSet->_frameL._file = path; imageSet->_frameL._tileMode = tileMode; break;
			case 8: imageSet->_frameR._file = path; imageSet->_frameR._tileMode = tileMode; break;
			}
			++index;
			image = image->NextSiblingElement("Image");
		}
		while (image);

		if (imageSet)
			AddImageSet(name, imageSet);
	}
}

// 0043f010: <Image Path="…" TileMode="…"/>
void RFXmlLoader::ReadSingleImageSet(const std::string& name, const rf::TiXmlElement* texture)
{
	const rf::TiXmlElement* image = texture->FirstChildElement("Image");
	if (image)
	{
		SingleImageSet* imageSet = new SingleImageSet;
		for (const rf::TiXmlAttribute* attribute = image->FirstAttribute(); attribute; attribute = attribute->Next())
		{
			if (!strcmp(attribute->Name(), "Path"))
				imageSet->_path = RFUtf8ToWide983(attribute->Value());
			else if (!strcmp(attribute->Name(), "TileMode"))
				imageSet->_tileMode = RFParseTileMode983(attribute->Value());
		}
		AddImageSet(name, imageSet);
	}
}

// 0043f290: <Image Path="…" ControlPoint="esquerda,cima,direita,baixo"/>
void RFXmlLoader::ReadNinePatchImageSet(const std::string& name, const rf::TiXmlElement* texture)
{
	const rf::TiXmlElement* image = texture->FirstChildElement("Image");
	if (image)
	{
		NinePatchImageSet* imageSet = new NinePatchImageSet;
		for (const rf::TiXmlAttribute* attribute = image->FirstAttribute(); attribute; attribute = attribute->Next())
		{
			if (!strcmp(attribute->Name(), "Path"))
				imageSet->_path = RFUtf8ToWide983(attribute->Value());
			else if (!strcmp(attribute->Name(), "ControlPoint"))
			{
				std::string controlPoint = attribute->Value();
				if (controlPoint.size())
				{
					std::string delimiter = ",";
					std::vector<std::string> tokens;
					std::string::size_type start = 0;
					std::string::size_type end;
					while ((end = controlPoint.find(delimiter, start)) != std::string::npos)
					{
						tokens.push_back(controlPoint.substr(start, end - start));
						start = end + delimiter.size();
					}
					tokens.push_back(controlPoint.substr(start, end - start));

					imageSet->_left = (float)atof(tokens[0].c_str());
					imageSet->_top = (float)atof(tokens[1].c_str());
					imageSet->_right = (float)atof(tokens[2].c_str());
					imageSet->_bottom = (float)atof(tokens[3].c_str());
				}
			}
		}
		AddImageSet(name, imageSet);
	}
}

// 0043f670: o primeiro com o nome fica; o repetido é apagado
void RFXmlLoader::AddImageSet(const std::string& name, ImageSet* imageSet)
{
	if (_imageSets.find(name) == _imageSets.end())
		_imageSets.insert(ImageSetMap::value_type(name, imageSet));
	else if (imageSet)
		delete imageSet;
}

// 0043f7a0
ImageSet* RFXmlLoader::GetImageSet(const std::string& name)
{
	ImageSetMap::iterator it = _imageSets.find(name);
	if (it != _imageSets.end())
	{
		it->second->AddRef();
		return it->second;
	}
	return 0;
}

// 0043f7e0: versão 1 — <base type="WND|WNDEX"> + <child> da raiz
int RFXmlLoader::ReadVersion1(const rf::TiXmlElement* root)
{
	const rf::TiXmlElement* base = root->FirstChildElement("base");
	if (!base)
		return 4;

	std::string type = base->Attribute("type");
	if (type == "WND")
	{
		WndBaseStruct* wnd = new WndExBase;
		ReadWnd(base, wnd);
		_structs.push_back(wnd);
		ReadChildrenV1(wnd, root);
	}
	else if (type == "WNDEX")
	{
		WndExBase* wnd = new WndExBase;
		ReadWndEx(base, wnd);
		WndBaseStruct* wndBase = wnd;
		_structs.push_back(wndBase);
		ReadChildrenV1(wnd, root);
	}
	return 1;
}

// 0043f9c0: <child type="…"> da versão 1 (TEXTAREA/TEXTAREASC e LIST/LISTSC criam a mesma estrutura)
int RFXmlLoader::ReadChildrenV1(WndBaseStruct* parent, const rf::TiXmlElement* element)
{
	const rf::TiXmlElement* child = element->FirstChildElement("child");
	if (!child)
		return 5;

	do
	{
		std::string type = child->Attribute("type");
		WndBaseStruct* base;
		if (type == "LABEL")
		{
			LabelBase* label = new LabelBase;
			ReadLabel(child, label);
			base = label;
			_children[parent].push_back(base);
		}
		else if (type == "BUTTON")
		{
			ButtonBase* button = new ButtonBase;
			ReadButton(child, button);
			base = button;
			_children[parent].push_back(base);
		}
		else if (type == "LABELBUTTON")
		{
			LabelButtonBase* button = new LabelButtonBase;
			ReadLabelButton(child, button);
			base = button;
			_children[parent].push_back(base);
		}
		else if (type == "ANIMBUTTON")
		{
			AnimButtonBase* button = new AnimButtonBase;
			ReadAnimButton(child, button);
			base = button;
			_children[parent].push_back(base);
		}
		else if (type == "CHECK")
		{
			CheckBase* check = new CheckBase;
			ReadCheck(child, check);
			base = check;
			_children[parent].push_back(base);
		}
		else if (type == "TEXTAREA")
		{
			TextAreaSCBase* textArea = new TextAreaSCBase;
			ReadTextArea(child, textArea);
			base = textArea;
			_children[parent].push_back(base);
		}
		else if (type == "TEXTAREASC")
		{
			TextAreaSCBase* textArea = new TextAreaSCBase;
			ReadTextArea(child, textArea);
			base = textArea;
			_children[parent].push_back(base);
		}
		else if (type == "EDIT")
		{
			EditBase* edit = new EditBase;
			ReadEdit(child, edit);
			base = edit;
			_children[parent].push_back(base);
		}
		else if (type == "LIST")
		{
			ListSCBase* list = new ListSCBase;
			ReadList(child, list);
			base = list;
			_children[parent].push_back(base);
		}
		else if (type == "LISTSC")
		{
			ListSCBase* list = new ListSCBase;
			ReadList(child, list);
			base = list;
			_children[parent].push_back(base);
		}
		else if (type == "COMBO")
		{
			ComboBase* combo = new ComboBase;
			ReadCombo(child, combo);
			base = combo;
			_children[parent].push_back(base);
		}
		else if (type == "SCROLLBAR")
		{
			ScrollbarBase* scrollbar = new ScrollbarBase;
			ReadScrollbar(child, scrollbar);
			base = scrollbar;
			_children[parent].push_back(base);
		}
		else if (type == "SLIDER")
		{
			SliderBase* slider = new SliderBase;
			ReadSlider(child, slider);
			base = slider;
			_children[parent].push_back(base);
		}
		else if (type == "PICTURE")
		{
			PictureBase* picture = new PictureBase;
			ReadPicture(child, picture);
			base = picture;
			_children[parent].push_back(base);
		}
		else if (type == "PROGRESS")
		{
			ProgressBase* progress = new ProgressBase;
			ReadProgress(child, progress);
			base = progress;
			_children[parent].push_back(base);
		}
		else if (type == "CUSTOM")
		{
			CustomBase* custom = new CustomBase;
			ReadCustom(child, custom);
			base = custom;
			_children[parent].push_back(base);
		}
		child = child->NextSiblingElement("child");
	}
	while (child);
	return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
// B148: versão 2 do XML (00440480–00447e70). <child>/<base type="…"> com os leitores por tipo; um atributo style="arquivo"
// lê o arquivo de estilo (a estrutura criada fica em _style) e aplica por cima o que o elemento diz (ApplyStyle). Os XML
// antigos (<asset type="NORMAL">, <snd>) e os novos (<Assets><Asset Name="Normal" TextureName="…">) são aceitos.

// 00447d00
const char* RFAttributeOr983(const rf::TiXmlElement* element, const char* name, const char* defaultValue)
{
	const char* value = element->Attribute(name);
	return value ? value : defaultValue;
}

// 00447d20
void RFQueryInt983(const rf::TiXmlElement* element, const char* name, int* value, int defaultValue)
{
	const char* text = element ? element->Attribute(name) : 0;
	if (text)
		*value = atoi(text);
	else if (value)
		*value = defaultValue;
}

// 00447d60
void RFQueryFloat983(const rf::TiXmlElement* element, const char* name, float* value, float defaultValue)
{
	const char* text = element ? element->Attribute(name) : 0;
	if (text)
		*value = (float)atof(text);
	else if (value)
		*value = defaultValue;
}

// 00447dd0
void RFQueryChildInt983(const rf::TiXmlElement* element, const char* child, const char* name, int* value, int defaultValue)
{
	const rf::TiXmlElement* node = element ? element->FirstChildElement(child) : 0;
	const char* text = node ? node->Attribute(name) : 0;
	if (text)
		*value = atoi(text);
	else if (value)
		*value = defaultValue;
}

// 00447e20
void RFQueryChildFloat983(const rf::TiXmlElement* element, const char* child, const char* name, float* value,
	float defaultValue)
{
	const rf::TiXmlElement* node = element ? element->FirstChildElement(child) : 0;
	const char* text = node ? node->Attribute(name) : 0;
	if (text)
		*value = (float)atof(text);
	else if (value)
		*value = defaultValue;
}

// 00447e70
void RFQueryChildBool983(const rf::TiXmlElement* element, const char* child, const char* name, bool* value,
	bool defaultValue)
{
	const rf::TiXmlElement* node = element ? element->FirstChildElement(child) : 0;
	const char* text = node ? node->Attribute(name) : 0;
	if (text)
		*value = atoi(text) != 0;
	else if (value)
		*value = defaultValue;
}

// 00441800: <pos x="…" y="…"/>
void RFReadPoint983(const rf::TiXmlElement* element, const std::string& name, float* point)
{
	const rf::TiXmlElement* node = element->FirstChildElement(name.c_str());
	if (node)
	{
		for (const rf::TiXmlAttribute* attribute = node->FirstAttribute(); attribute; attribute = attribute->Next())
		{
			if (!strcmp(attribute->Name(), "x"))
				point[0] = (float)atof(attribute->Value());
			else if (!strcmp(attribute->Name(), "y"))
				point[1] = (float)atof(attribute->Value());
		}
	}
}

// 004418e0: <rect l="…" t="…" r="…" b="…"/> (só os atributos presentes mudam)
void RFReadRect983(const rf::TiXmlElement* element, const std::string& name, RFRect983* rect)
{
	const rf::TiXmlElement* node = element->FirstChildElement(name.c_str());
	if (node)
	{
		for (const rf::TiXmlAttribute* attribute = node->FirstAttribute(); attribute; attribute = attribute->Next())
		{
			if (!strcmp(attribute->Name(), "l"))
				rect->left = (float)atof(attribute->Value());
			else if (!strcmp(attribute->Name(), "t"))
				rect->top = (float)atof(attribute->Value());
			else if (!strcmp(attribute->Name(), "r"))
				rect->right = (float)atof(attribute->Value());
			else if (!strcmp(attribute->Name(), "b"))
				rect->bottom = (float)atof(attribute->Value());
		}
	}
}

// 00440730: a filha ancorada à direita/embaixo acompanha a borda da mãe (oldRect = retângulo da mãe antes do estilo)
static void RFAdjustAnchor983(const RFRect983& oldRect, const WndBaseStruct* parent, WndBaseStruct* child)
{
	const int anchor = child->_anchor.Get();
	if (anchor & 4)
	{
		if (anchor & 1)
		{
			float width = child->_rect.right - child->_rect.left;
			float margin = oldRect.right - (width + child->_x);
			child->_rect.right = (parent->_rect.right - child->_x) - margin;
		}
		else
		{
			float width = child->_rect.right - child->_rect.left;
			float margin = oldRect.right - (width + child->_x);
			child->_x = parent->_rect.right - (width + margin);
		}
	}
	if (anchor & 8)
	{
		if (anchor & 2)
		{
			float height = child->_rect.bottom - child->_rect.top;
			float margin = oldRect.bottom - (height + child->_y);
			child->_rect.bottom = (parent->_rect.bottom - child->_y) - margin;
		}
		else
		{
			float height = child->_rect.bottom - child->_rect.top;
			float margin = oldRect.bottom - (height + child->_y);
			child->_y = parent->_rect.bottom - (height + margin);
		}
	}
}

// 00440480: style="arquivo" — lê o arquivo (texturas e elementos; a estrutura criada vai para _style)
bool RFXmlLoader::ReadStyle(WndBaseStruct* parent, const rf::TiXmlElement* element)
{
	std::wstring style = RFUtf8ToWide983(RFGetAttribute983(element, "style"));
	if (!style.size())
		return false;

	std::wstring path = _interface->GetBasePath(0) + style;
	rf::TiXmlDocument document;
	LoadDocument(path, document);

	const rf::TiXmlElement* root = document.FirstChildElement("refresh");
	if (!root)
		RFThrowException983(L"Couldn't find root node");
	ReadTextures(root);
	ReadElements(parent, root, true);
	return true;
}

// 00440640
void RFXmlLoader::ApplyAnchors(const RFRect983& oldRect, WndBaseStruct* base)
{
	const int count = GetChildCount(base);
	if (count == -1)
	{
		if (base->GetType() == 4)
			RFAdjustAnchor983(oldRect, base, static_cast<LabelButtonBase*>(base)->_label);
		if (base->GetType() == 0x11)
		{
			// corpo vazio no 983 (a 2ª chamada de GetType sobra)
		}
		return;
	}

	for (int i = 0; i < count; ++i)
	{
		WndBaseStruct* child = GetChild(base, i);
		RFAdjustAnchor983(oldRect, base, child);
		ApplyAnchors(child->_rect, child);
	}
}

// 004407f0: o elemento por cima da estrutura do estilo (posição, propriedades, campos do tipo, âncoras, <Style> com textos)
void RFXmlLoader::ApplyStyle(WndBaseStruct* base, const rf::TiXmlElement* element)
{
	RFRect983 oldRect = base->_rect;
	ReadWnd(element, base);
	switch (base->GetType())
	{
	case 2:
		ReadLabelStyle(element, static_cast<LabelBase*>(base));
		break;
	case 3:
		ReadButtonSounds(element, static_cast<ButtonBase*>(base));
		break;
	case 0xb:
		ReadEditStyle(element, static_cast<EditBase*>(base));
		break;
	case 0xc:
		ReadListStyle(element, static_cast<ListBase*>(base));
		break;
	case 0xf:
		ReadTextAreaStyle(element, static_cast<TextAreaBase*>(base));
		break;
	case 0x10:
		ReadTextAreaSCStyle(element, static_cast<TextAreaSCBase*>(base));
		break;
	}
	ApplyAnchors(oldRect, base);

	const rf::TiXmlElement* style = element->FirstChildElement("Style");
	if (style)
	{
		for (const rf::TiXmlElement* setter = style->FirstChildElement(); setter; setter = setter->NextSiblingElement())
		{
			std::wstring target = RFUtf8ToWide983(setter->Attribute("Target"));
			std::wstring property = RFUtf8ToWide983(setter->Attribute("Property")); // lido e não usado no 983
			std::wstring value = RFUtf8ToWide983(setter->Attribute("Value"));
			ApplyStyleText(target, value, base);
		}
	}
	ReadElements(base, element, false);
}

// 00440b50: <Style Target="nome" Value="texto"> troca o texto do Label filho com esse nome (ou o do LabelButton)
void RFXmlLoader::ApplyStyleText(const std::wstring& target, const std::wstring& value, WndBaseStruct* base)
{
	const int count = GetChildCount(base);
	if (count == -1)
	{
		if (base->_name == target && base->GetType() == 4)
			static_cast<LabelButtonBase*>(base)->_label->_text = value;
		return;
	}

	for (int i = 0; i < count; ++i)
	{
		WndBaseStruct* child = GetChild(base, i);
		if (child->GetType() == 2 && child->_name == target)
			static_cast<LabelBase*>(child)->_text = value;
	}
}

// 00440cb0: <child>/<base type="…"> da versão 2 (TEXTAREA/TEXTAREASC e LIST/LISTSC criam a mesma estrutura, como na v1)
int RFXmlLoader::ReadElements(WndBaseStruct* parent, const rf::TiXmlElement* element, bool style)
{
	const rf::TiXmlElement* child = element->FirstChildElement();
	if (!child)
		RFThrowException983(L"Counld't found child element");

	do
	{
		std::string value = child->Value();
		if (value == "child" || value == "base")
		{
			std::string type = child->Attribute("type");
			if (!ReadStyle(parent, child))
			{
				WndBaseStruct* base = 0;
				if (type == "WND" || type == "PANEL")
				{
					WndExBase* wnd = new WndExBase;
					ReadWnd(child, wnd);
					base = wnd;
				}
				else if (type == "WNDEX")
				{
					WndExBase* wnd = new WndExBase;
					ReadWndEx(child, wnd);
					base = wnd;
				}
				else if (type == "LABEL")
				{
					LabelBase* label = new LabelBase;
					ReadLabel(child, label);
					base = label;
				}
				else if (type == "BUTTON")
				{
					ButtonBase* button = new ButtonBase;
					ReadButton(child, button);
					base = button;
				}
				else if (type == "LABELBUTTON")
				{
					LabelButtonBase* button = new LabelButtonBase;
					ReadLabelButton(child, button);
					base = button;
				}
				else if (type == "ANIMBUTTON")
				{
					AnimButtonBase* button = new AnimButtonBase;
					ReadAnimButton(child, button);
					base = button;
				}
				else if (type == "CHECK")
				{
					CheckBase* check = new CheckBase;
					ReadCheck(child, check);
					base = check;
				}
				else if (type == "TEXTAREA")
				{
					TextAreaSCBase* textArea = new TextAreaSCBase;
					ReadTextArea(child, textArea);
					base = textArea;
				}
				else if (type == "TEXTAREASC")
				{
					TextAreaSCBase* textArea = new TextAreaSCBase;
					ReadTextArea(child, textArea);
					base = textArea;
				}
				else if (type == "EDIT")
				{
					EditBase* edit = new EditBase;
					ReadEdit(child, edit);
					base = edit;
				}
				else if (type == "LIST")
				{
					ListSCBase* list = new ListSCBase;
					ReadList(child, list);
					base = list;
				}
				else if (type == "LISTSC")
				{
					ListSCBase* list = new ListSCBase;
					ReadList(child, list);
					base = list;
				}
				else if (type == "COMBO")
				{
					ComboBase* combo = new ComboBase;
					ReadCombo(child, combo);
					base = combo;
				}
				else if (type == "SCROLLBAR")
				{
					ScrollbarBase* scrollbar = new ScrollbarBase;
					ReadScrollbar(child, scrollbar);
					base = scrollbar;
				}
				else if (type == "SLIDER")
				{
					SliderBase* slider = new SliderBase;
					ReadSlider(child, slider);
					base = slider;
				}
				else if (type == "STACKPANEL")
				{
					StackPanelBase* panel = new StackPanelBase;
					ReadStackPanel(child, panel);
					base = panel;
				}
				else if (type == "PICTURE")
				{
					PictureBase* picture = new PictureBase;
					ReadPicture(child, picture);
					base = picture;
				}
				else if (type == "PROGRESS")
				{
					ProgressBase* progress = new ProgressBase;
					ReadProgress(child, progress);
					base = progress;
				}
				else if (type == "CUSTOM")
				{
					CustomBase* custom = new CustomBase;
					ReadCustom(child, custom);
					base = custom;
				}

				if (base)
				{
					ReadElements(base, child, false);
					if (!parent)
						_structs.push_back(base);
					else
						_children[parent].push_back(base);
				}
				if (style)
					_style = base;
			}
			else if (_style)
				ApplyStyle(_style, child);
		}
		child = child->NextSiblingElement();
	}
	while (child);
	return 1;
}

// 00441a50: nome, <pos>, <rect>, <snd type="SHOW|HIDE">texto</snd>, <embed src>, e os elementos com o nome de uma
// propriedade registrada (<Dragable val="…"/>)
int RFXmlLoader::ReadWnd(const rf::TiXmlElement* element, WndBaseStruct* base)
{
	base->_name = RFUtf8ToWide983(element->Attribute("name"));
	RFReadPoint983(element, "pos", &base->_x);
	RFReadRect983(element, "rect", &base->_rect);

	for (const rf::TiXmlElement* sound = element->FirstChildElement("snd"); sound; sound = sound->NextSiblingElement("snd"))
	{
		const rf::TiXmlNode* text = sound->FirstChild();
		if (text)
		{
			std::string type = sound->Attribute("type");
			if (type == "SHOW")
				base->_showSound = RFUtf8ToWide983(text->Value());
			else if (type == "HIDE")
				base->_hideSound = RFUtf8ToWide983(text->Value());
		}
	}

	LoadInclude(RFUtf8ToWide983(RFGetAttribute983(element ? element->FirstChildElement("embed") : 0, "src")), base);

	for (const rf::TiXmlElement* child = element->FirstChildElement(); child; child = child->NextSiblingElement())
	{
		RFProperty* property = base->GetProperty(child->Value());
		if (property)
			property->SetValue(child->Attribute("val"));
	}
	return 1;
}

// 00442070: fundo — <Assets><Asset Name="Default" TextureName="…"> ou, no XML antigo, <asset type="BG|EDGE_*|FRAME_*"
// path="…" tileMode="…"> numa FrameImageSet nova
int RFXmlLoader::ReadWndEx(const rf::TiXmlElement* element, WndExBase* base)
{
	ReadWnd(element, base);

	const rf::TiXmlElement* assets = element->FirstChildElement("Assets");
	if (assets)
	{
		for (const rf::TiXmlElement* asset = assets->FirstChildElement("Asset"); asset;
			asset = asset->NextSiblingElement("Asset"))
		{
			std::string name = asset->Attribute("Name");
			if (name == "Default")
			{
				std::string textureName = asset->Attribute("TextureName");
				base->_background = GetImageSet(textureName);
			}
		}
		return 1;
	}

	const rf::TiXmlElement* asset = element->FirstChildElement("asset");
	FrameImageSet* imageSet = 0;
	if (asset)
	{
		imageSet = new FrameImageSet;
		do
		{
			std::string type = asset->Attribute("type");
			std::string tileModeName = RFGetAttribute983(asset, "tileMode", "Stretch");
			const int tileMode = RFParseTileMode983(tileModeName);
			if (type == "BG")
			{
				imageSet->_background._file = RFUtf8ToWide983(asset->Attribute("path"));
				imageSet->_background._tileMode = tileMode;
			}
			else if (type == "EDGE_LT")
			{
				imageSet->_edgeLT._file = RFUtf8ToWide983(asset->Attribute("path"));
				imageSet->_edgeLT._tileMode = tileMode;
			}
			else if (type == "EDGE_RT")
			{
				imageSet->_edgeRT._file = RFUtf8ToWide983(asset->Attribute("path"));
				imageSet->_edgeRT._tileMode = tileMode;
			}
			else if (type == "EDGE_LB")
			{
				imageSet->_edgeLB._file = RFUtf8ToWide983(asset->Attribute("path"));
				imageSet->_edgeLB._tileMode = tileMode;
			}
			else if (type == "EDGE_RB")
			{
				imageSet->_edgeRB._file = RFUtf8ToWide983(asset->Attribute("path"));
				imageSet->_edgeRB._tileMode = tileMode;
			}
			else if (type == "FRAME_T")
			{
				imageSet->_frameT._file = RFUtf8ToWide983(asset->Attribute("path"));
				imageSet->_frameT._tileMode = tileMode;
			}
			else if (type == "FRAME_B")
			{
				imageSet->_frameB._file = RFUtf8ToWide983(asset->Attribute("path"));
				imageSet->_frameB._tileMode = tileMode;
			}
			else if (type == "FRAME_L")
			{
				imageSet->_frameL._file = RFUtf8ToWide983(asset->Attribute("path"));
				imageSet->_frameL._tileMode = tileMode;
			}
			else if (type == "FRAME_R")
			{
				imageSet->_frameR._file = RFUtf8ToWide983(asset->Attribute("path"));
				imageSet->_frameR._tileMode = tileMode;
			}
			asset = asset->NextSiblingElement("asset");
		}
		while (asset);
	}
	base->_background = imageSet;
	return 1;
}

// 00442a50: <label val="…"/> (obrigatório) e <style val="…"/>
int RFXmlLoader::ReadLabel(const rf::TiXmlElement* element, LabelBase* base)
{
	ReadWnd(element, base);
	base->_text = RFUtf8ToWide983(element->FirstChildElement("label")->Attribute("val"));
	RFQueryChildInt983(element, "style", "val", &base->_style, 0);
	return 1;
}

// 00442b70: o mesmo por cima do estilo (o que falta fica)
void RFXmlLoader::ReadLabelStyle(const rf::TiXmlElement* element, LabelBase* base)
{
	const rf::TiXmlElement* label = element->FirstChildElement("label");
	if (label)
		base->_text = RFUtf8ToWide983(label->Attribute("val"));
	RFQueryChildInt983(element, "style", "val", &base->_style, base->_style);
}

// 00442c90: imagens Normal/Push/Hover/Disable (ou <asset type="NORMAL|DOWN|OVER|DISABLE">texto</asset> em 4 SingleImageSet)
// e os sons
int RFXmlLoader::ReadButton(const rf::TiXmlElement* element, ButtonBase* base)
{
	ReadWnd(element, base);

	const rf::TiXmlElement* assets = element->FirstChildElement("Assets");
	if (assets)
	{
		for (const rf::TiXmlElement* asset = assets->FirstChildElement("Asset"); asset;
			asset = asset->NextSiblingElement("Asset"))
		{
			std::string name = asset->Attribute("Name");
			std::string textureName = asset->Attribute("TextureName");
			if (name == "Normal")
				base->_normalImage = GetImageSet(textureName);
			else if (name == "Push")
				base->_pushImage = GetImageSet(textureName);
			else if (name == "Hover")
				base->_hoverImage = GetImageSet(textureName);
			else if (name == "Disable")
				base->_disableImage = GetImageSet(textureName);
		}
	}
	else
	{
		const rf::TiXmlElement* asset = element->FirstChildElement("asset");
		SingleImageSet* normal = 0;
		SingleImageSet* push = 0;
		SingleImageSet* hover = 0;
		SingleImageSet* disable = 0;
		if (asset)
		{
			normal = new SingleImageSet;
			push = new SingleImageSet;
			hover = new SingleImageSet;
			disable = new SingleImageSet;
			do
			{
				const rf::TiXmlNode* text = asset->FirstChild();
				if (text)
				{
					std::string type = asset->Attribute("type");
					if (type == "NORMAL")
						normal->_path = RFUtf8ToWide983(text->Value());
					else if (type == "DOWN")
						push->_path = RFUtf8ToWide983(text->Value());
					else if (type == "OVER")
						hover->_path = RFUtf8ToWide983(text->Value());
					else if (type == "DISABLE")
						disable->_path = RFUtf8ToWide983(text->Value());
				}
				asset = asset->NextSiblingElement("asset");
			}
			while (asset);
		}
		base->_normalImage = normal;
		base->_pushImage = push;
		base->_hoverImage = hover;
		base->_disableImage = disable;
	}

	ReadButtonSounds(element, base);
	return 1;
}

// 00443710: <snd type="OVER|DOWN|RELEASE">texto</snd>
void RFXmlLoader::ReadButtonSounds(const rf::TiXmlElement* element, ButtonBase* base)
{
	for (const rf::TiXmlElement* sound = element->FirstChildElement("snd"); sound; sound = sound->NextSiblingElement("snd"))
	{
		const rf::TiXmlNode* text = sound->FirstChild();
		if (text)
		{
			std::string type = sound->Attribute("type");
			if (type == "OVER")
				base->_hoverSound = RFUtf8ToWide983(text->Value());
			else if (type == "DOWN")
				base->_pushSound = RFUtf8ToWide983(text->Value());
			else if (type == "RELEASE")
				base->_clickSound = RFUtf8ToWide983(text->Value());
		}
	}
}

// 004439e0: botão + <control type="LABEL"> (o Label do botão); sem <NormalColor val> as cores de estado ficam desligadas
int RFXmlLoader::ReadLabelButton(const rf::TiXmlElement* element, LabelButtonBase* base)
{
	ReadButton(element, base);
	if (!strcmp(RFGetAttribute983(element ? element->FirstChildElement("NormalColor") : 0, "val"), ""))
		base->_hasNormalColor = false;

	for (const rf::TiXmlElement* control = element->FirstChildElement("control"); control;
		control = control->NextSiblingElement("control"))
	{
		std::string type = control->Attribute("type");
		if (type == "LABEL")
		{
			LabelBase* label = new LabelBase;
			ReadLabel(control, label);
			base->_label = label;
		}
	}
	return 1;
}

// 00443c70: botão + imagens ENTER/LEAVE; "Group"/"GROUP" põe a imagem em _normalImage
int RFXmlLoader::ReadAnimButton(const rf::TiXmlElement* element, AnimButtonBase* base)
{
	if (!ReadButton(element, base))
		return 0;
	base->_group = false;

	const rf::TiXmlElement* assets = element->FirstChildElement("Assets");
	if (assets)
	{
		for (const rf::TiXmlElement* asset = assets->FirstChildElement("Asset"); asset;
			asset = asset->NextSiblingElement("Asset"))
		{
			std::string name = asset->Attribute("Name");
			std::string textureName = asset->Attribute("TextureName");
			if (name == "Group")
			{
				base->_normalImage = GetImageSet(textureName);
				base->_group = true;
			}
		}
		return 1;
	}

	const rf::TiXmlElement* asset = element->FirstChildElement("asset");
	SingleImageSet* enter = 0;
	SingleImageSet* leave = 0;
	if (asset)
	{
		enter = new SingleImageSet;
		leave = new SingleImageSet;
		do
		{
			std::string type = asset->Attribute("type");
			if (type == "GROUP")
			{
				if (enter)
					delete enter;
				if (leave)
					delete leave;
				const rf::TiXmlNode* text = asset->FirstChild();
				if (!text)
					return 0;
				static_cast<SingleImageSet*>(base->_normalImage)->_path = RFUtf8ToWide983(text->Value());
				base->_group = true;
				return 1;
			}
			else if (type == "ENTER")
			{
				const rf::TiXmlElement* image = asset->FirstChildElement();
				if (image)
					enter->_path = RFUtf8ToWide983(image->Value());
			}
			else if (type == "LEAVE")
			{
				const rf::TiXmlElement* image = asset->FirstChildElement();
				if (image)
					leave->_path = RFUtf8ToWide983(image->Value());
			}
			asset = asset->NextSiblingElement("asset");
		}
		while (asset);
	}
	base->_enterImage = enter;
	base->_leaveImage = leave;
	return 1;
}

// 004442f0: imagens Normal/Check (ou <asset type="NORMAL|CHECK">texto</asset>)
int RFXmlLoader::ReadCheck(const rf::TiXmlElement* element, CheckBase* base)
{
	ReadWnd(element, base);

	const rf::TiXmlElement* assets = element->FirstChildElement("Assets");
	if (assets)
	{
		for (const rf::TiXmlElement* asset = assets->FirstChildElement("Asset"); asset;
			asset = asset->NextSiblingElement("Asset"))
		{
			std::string name = asset->Attribute("Name");
			std::string textureName = asset->Attribute("TextureName");
			if (name == "Normal")
				base->_uncheckedImage = GetImageSet(textureName);
			else if (name == "Check")
				base->_checkedImage = GetImageSet(textureName);
		}
		return 1;
	}

	const rf::TiXmlElement* asset = element->FirstChildElement("asset");
	SingleImageSet* unchecked = 0;
	SingleImageSet* checked = 0;
	if (asset)
	{
		unchecked = new SingleImageSet;
		checked = new SingleImageSet;
		do
		{
			const char* type = asset->Attribute("type");
			if (!strcmp(type, "NORMAL"))
				unchecked->_path = RFUtf8ToWide983(asset->FirstChild()->Value());
			else if (!strcmp(type, "CHECK"))
				checked->_path = RFUtf8ToWide983(asset->FirstChild()->Value());
			asset = asset->NextSiblingElement("asset");
		}
		while (asset);
	}
	base->_uncheckedImage = unchecked;
	base->_checkedImage = checked;
	return 1;
}

// 00444800: janela com fundo + <padding>, <column>, <itemrect> (largura e altura > 0), <selection>, <deselectable>,
// <rclickaction>
int RFXmlLoader::ReadListBase(const rf::TiXmlElement* element, ListBase* base)
{
	ReadWndEx(element, base);
	RFReadRect983(element, "padding", &base->_padding);
	RFQueryChildInt983(element, "column", "val", &base->_column, 0);
	RFReadRect983(element, "itemrect", &base->_itemrect);
#line 1572 "src\\RFXmlLoader.cpp"
	assert(base->_itemrect.Width() > 0);
	assert(base->_itemrect.Height() > 0);
	RFQueryChildInt983(element, "selection", "val", &base->_selection, 0);
	RFQueryChildBool983(element, "deselectable", "val", &base->_deselectable, false);
	RFQueryChildInt983(element, "rclickaction", "val", &base->_rclickaction, 0);
	return 1;
}

// 00444ad0
void RFXmlLoader::ReadListStyle(const rf::TiXmlElement* element, ListBase* base)
{
	RFReadRect983(element, "padding", &base->_padding);
	RFReadRect983(element, "itemrect", &base->_itemrect);
	RFQueryChildInt983(element, "column", "val", &base->_column, base->_column);
	RFQueryChildInt983(element, "selection", "val", &base->_selection, base->_selection);
	RFQueryChildBool983(element, "deselectable", "val", &base->_deselectable, base->_deselectable);
	RFQueryChildInt983(element, "rclickaction", "val", &base->_rclickaction, base->_rclickaction);
}

// 00444d40: lista + barras <control type="HSCROLLBAR|VSCROLLBAR"> (a que não lê é apagada)
int RFXmlLoader::ReadList(const rf::TiXmlElement* element, ListSCBase* base)
{
	ReadListBase(element, base);
	for (const rf::TiXmlElement* control = element->FirstChildElement("control"); control;
		control = control->NextSiblingElement("control"))
	{
		std::string type = control->Attribute("type");
		if (type == "HSCROLLBAR")
		{
			ScrollbarBase* scrollbar = new ScrollbarBase;
			if (ReadScrollbar(control, scrollbar))
				base->_hScrollbar = scrollbar;
			else if (scrollbar)
				delete scrollbar;
		}
		else if (type == "VSCROLLBAR")
		{
			ScrollbarBase* scrollbar = new ScrollbarBase;
			if (ReadScrollbar(control, scrollbar))
				base->_vScrollbar = scrollbar;
			else if (scrollbar)
				delete scrollbar;
		}
	}
	return 1;
}

// 00445050: janela com fundo + cursor (Caret/CARET) + <padding>, <style>, <halign>
int RFXmlLoader::ReadTextAreaBase(const rf::TiXmlElement* element, TextAreaBase* base)
{
	ReadWndEx(element, base);
	base->_style = 0;

	const rf::TiXmlElement* assets = element->FirstChildElement("Assets");
	if (assets)
	{
		for (const rf::TiXmlElement* asset = assets->FirstChildElement("Asset"); asset;
			asset = asset->NextSiblingElement("Asset"))
		{
			std::string name = asset->Attribute("Name");
			std::string textureName = asset->Attribute("TextureName");
			if (name == "Caret")
				base->_caret = GetImageSet(textureName);
		}
	}
	else
	{
		const rf::TiXmlElement* asset = element->FirstChildElement("asset");
		SingleImageSet* caret = 0;
		if (asset)
		{
			caret = new SingleImageSet;
			do
			{
				std::string type = asset->Attribute("type");
				if (type == "CARET")
					caret->_path = RFUtf8ToWide983(asset->Attribute("path"));
				asset = asset->NextSiblingElement("asset");
			}
			while (asset);
		}
		base->_caret = caret;
	}

	RFReadRect983(element, "padding", &base->_padding);
	RFQueryChildInt983(element, "style", "val", &base->_style, 0);
	RFQueryChildInt983(element, "halign", "val", &base->_halign, 0);
	return 1;
}

// 004455d0
void RFXmlLoader::ReadTextAreaStyle(const rf::TiXmlElement* element, TextAreaBase* base)
{
	RFReadRect983(element, "padding", &base->_padding);
	RFQueryChildInt983(element, "style", "val", &base->_style, base->_style);
	RFQueryChildInt983(element, "halign", "val", &base->_halign, base->_halign);
}

// 00445730: área de texto + barras + <showalways hscroll vscroll>, <autovisible hscroll vscroll>, <autovisiblerect>
int RFXmlLoader::ReadTextArea(const rf::TiXmlElement* element, TextAreaSCBase* base)
{
	ReadTextAreaBase(element, base);
	for (const rf::TiXmlElement* control = element->FirstChildElement("control"); control;
		control = control->NextSiblingElement("control"))
	{
		std::string type = control->Attribute("type");
		if (type == "HSCROLLBAR")
		{
			ScrollbarBase* scrollbar = new ScrollbarBase;
			if (ReadScrollbar(control, scrollbar))
				base->_hScrollbar = scrollbar;
			else if (scrollbar)
				delete scrollbar;
		}
		else if (type == "VSCROLLBAR")
		{
			ScrollbarBase* scrollbar = new ScrollbarBase;
			if (ReadScrollbar(control, scrollbar))
				base->_vScrollbar = scrollbar;
			else if (scrollbar)
				delete scrollbar;
		}
	}
	RFQueryChildBool983(element, "showalways", "hscroll", &base->_showAlwaysH, false);
	RFQueryChildBool983(element, "showalways", "vscroll", &base->_showAlwaysV, false);
	RFQueryChildBool983(element, "autovisible", "hscroll", &base->_autoVisibleH, false);
	RFQueryChildBool983(element, "autovisible", "vscroll", &base->_autoVisibleV, false);
	RFReadRect983(element, "autovisiblerect", &base->_autoVisibleRect);
	return 1;
}

// 00445bf0
void RFXmlLoader::ReadTextAreaSCStyle(const rf::TiXmlElement* element, TextAreaSCBase* base)
{
	RFQueryChildBool983(element, "showalways", "hscroll", &base->_showAlwaysH, base->_showAlwaysH);
	RFQueryChildBool983(element, "showalways", "vscroll", &base->_showAlwaysV, base->_showAlwaysV);
	RFQueryChildBool983(element, "autovisible", "hscroll", &base->_autoVisibleH, base->_autoVisibleH);
	RFQueryChildBool983(element, "autovisible", "vscroll", &base->_autoVisibleV, base->_autoVisibleV);
	RFReadRect983(element, "autovisiblerect", &base->_autoVisibleRect);
}

// <passwordchar val="…"/>: o 1º caractere
static __forceinline void RFReadPasswordChar983(const rf::TiXmlElement* element, EditBase* base)
{
	const rf::TiXmlElement* passwordChar = element->FirstChildElement("passwordchar");
	if (passwordChar)
	{
		std::wstring text = RFUtf8ToWide983(passwordChar->Attribute("val"));
		if (text.size())
			base->_passwordChar = text[0];
	}
}

// 00445e20: janela com fundo + cursor + <padding>, <style>, <halign>, <valign>, <passwordchar>
int RFXmlLoader::ReadEdit(const rf::TiXmlElement* element, EditBase* base)
{
	ReadWndEx(element, base);
	base->_style = 0;
	base->_passwordChar = 0;

	const rf::TiXmlElement* assets = element->FirstChildElement("Assets");
	if (assets)
	{
		for (const rf::TiXmlElement* asset = assets->FirstChildElement("Asset"); asset;
			asset = asset->NextSiblingElement("Asset"))
		{
			std::string name = asset->Attribute("Name");
			std::string textureName = asset->Attribute("TextureName");
			if (name == "Caret")
				base->_caret = GetImageSet(textureName);
		}
	}
	else
	{
		const rf::TiXmlElement* asset = element->FirstChildElement("asset");
		SingleImageSet* caret = 0;
		if (asset)
		{
			caret = new SingleImageSet;
			do
			{
				std::string type = asset->Attribute("type");
				if (type == "CARET")
					caret->_path = RFUtf8ToWide983(asset->Attribute("path"));
				asset = asset->NextSiblingElement("asset");
			}
			while (asset);
		}
		base->_caret = caret;
	}

	RFReadRect983(element, "padding", &base->_padding);
	RFQueryChildInt983(element, "style", "val", &base->_style, base->_style);
	RFQueryChildInt983(element, "halign", "val", &base->_halign, 0);
	RFQueryChildInt983(element, "valign", "val", &base->_valign, 0);
	RFReadPasswordChar983(element, base);
	return 1;
}

// 00446470
void RFXmlLoader::ReadEditStyle(const rf::TiXmlElement* element, EditBase* base)
{
	RFReadRect983(element, "padding", &base->_padding);
	RFQueryChildInt983(element, "style", "val", &base->_style, base->_style);
	RFQueryChildInt983(element, "halign", "val", &base->_halign, base->_halign);
	RFQueryChildInt983(element, "valign", "val", &base->_valign, base->_valign);
	RFReadPasswordChar983(element, base);
}

// 00446690: <control type="BUTTON|TEXTAREA|LISTAREA">
int RFXmlLoader::ReadCombo(const rf::TiXmlElement* element, ComboBase* base)
{
	ReadWnd(element, base);
	for (const rf::TiXmlElement* control = element->FirstChildElement("control"); control;
		control = control->NextSiblingElement("control"))
	{
		std::string type = control->Attribute("type");
		if (type == "BUTTON")
		{
			ButtonBase* button = new ButtonBase;
			ReadButton(control, button);
			base->_button = button;
		}
		else if (type == "TEXTAREA")
		{
			TextAreaSCBase* textArea = new TextAreaSCBase;
			ReadTextArea(control, textArea);
			base->_textArea = textArea;
		}
		else if (type == "LISTAREA")
		{
			ListSCBase* list = new ListSCBase;
			ReadList(control, list);
			base->_listArea = list;
		}
	}
	return 1;
}

// 004469a0: sem <control> não lê (0); botões BUTTON_LT/BUTTON_RB/THUMB e <scrollrect>
int RFXmlLoader::ReadScrollbar(const rf::TiXmlElement* element, ScrollbarBase* base)
{
	ReadWnd(element, base);
	const rf::TiXmlElement* control = element->FirstChildElement("control");
	if (!control)
		return 0;

	base->_horizontal = atoi(element->Attribute("horizontal")) != 0;
	do
	{
		std::string type = control->Attribute("type");
		if (type == "BUTTON_LT")
		{
			ButtonBase* button = new ButtonBase;
			ReadButton(control, button);
			base->_buttonLT = button;
		}
		else if (type == "BUTTON_RB")
		{
			ButtonBase* button = new ButtonBase;
			ReadButton(control, button);
			base->_buttonRB = button;
		}
		else if (type == "THUMB")
		{
			ButtonBase* button = new ButtonBase;
			ReadButton(control, button);
			base->_thumb = button;
		}
		control = control->NextSiblingElement("control");
	}
	while (control);
	RFReadRect983(element, "scrollrect", &base->_scrollRect);
	return 1;
}

// 00446ce0: como a barra, com <sliderect>
int RFXmlLoader::ReadSlider(const rf::TiXmlElement* element, SliderBase* base)
{
	ReadWnd(element, base);
	const rf::TiXmlElement* control = element->FirstChildElement("control");
	if (!control)
		return 0;

	base->_horizontal = atoi(element->Attribute("horizontal")) != 0;
	do
	{
		std::string type = control->Attribute("type");
		if (type == "BUTTON_LT")
		{
			ButtonBase* button = new ButtonBase;
			ReadButton(control, button);
			base->_buttonLT = button;
		}
		else if (type == "BUTTON_RB")
		{
			ButtonBase* button = new ButtonBase;
			ReadButton(control, button);
			base->_buttonRB = button;
		}
		else if (type == "THUMB")
		{
			ButtonBase* button = new ButtonBase;
			ReadButton(control, button);
			base->_thumb = button;
		}
		control = control->NextSiblingElement("control");
	}
	while (control);
	RFReadRect983(element, "sliderect", &base->_slideRect);
	return 1;
}

// 00447020
int RFXmlLoader::ReadStackPanel(const rf::TiXmlElement* element, StackPanelBase* base)
{
	ReadWnd(element, base);
	return 1;
}

// 00447040: imagem Static/Animation (ou <asset type="STATIC|ANIMATION">texto</asset>)
int RFXmlLoader::ReadPicture(const rf::TiXmlElement* element, PictureBase* base)
{
	ReadWnd(element, base);
	base->_static = true;

	const rf::TiXmlElement* assets = element->FirstChildElement("Assets");
	if (assets)
	{
		for (const rf::TiXmlElement* asset = assets->FirstChildElement("Asset"); asset;
			asset = asset->NextSiblingElement("Asset"))
		{
			std::string name = asset->Attribute("Name");
			std::string textureName = asset->Attribute("TextureName");
			base->_image = GetImageSet(textureName);
			if (name == "Static")
				base->_static = true;
			else if (name == "Animation")
				base->_static = false;
		}
		return 1;
	}

	const rf::TiXmlElement* asset = element->FirstChildElement("asset");
	SingleImageSet* image = 0;
	if (asset)
	{
		image = new SingleImageSet;
		do
		{
			std::string type = asset->Attribute("type");
			if (type == "STATIC")
			{
				const rf::TiXmlNode* text = asset->FirstChild();
				if (text)
				{
					image->_path = RFUtf8ToWide983(text->Value());
					base->_static = true;
				}
			}
			else if (type == "ANIMATION")
			{
				image->_path = RFUtf8ToWide983(asset->FirstChild()->Value());
				base->_static = false;
			}
			asset = asset->NextSiblingElement("asset");
		}
		while (asset);
	}
	base->_image = image;
	return 1;
}

// 00447620: imagem Bar (ou <asset type="BAR" tileMode="…">texto</asset>)
int RFXmlLoader::ReadProgress(const rf::TiXmlElement* element, ProgressBase* base)
{
	ReadWnd(element, base);

	const rf::TiXmlElement* assets = element->FirstChildElement("Assets");
	if (assets)
	{
		for (const rf::TiXmlElement* asset = assets->FirstChildElement("Asset"); asset;
			asset = asset->NextSiblingElement("Asset"))
		{
			std::string name = asset->Attribute("Name");
			std::string textureName = asset->Attribute("TextureName");
			if (name == "Bar")
				base->_image = GetImageSet(textureName);
		}
		return 1;
	}

	const rf::TiXmlElement* asset = element->FirstChildElement("asset");
	SingleImageSet* image = 0;
	if (asset)
	{
		image = new SingleImageSet;
		do
		{
			std::string type = asset->Attribute("type");
			std::string tileModeName = RFGetAttribute983(asset, "tileMode", "Stretch");
			const int tileMode = RFParseTileMode983(tileModeName);
			if (type == "BAR")
			{
				const rf::TiXmlNode* text = asset->FirstChild();
				if (text)
				{
					image->_path = RFUtf8ToWide983(text->Value());
					image->_tileMode = tileMode;
				}
			}
			asset = asset->NextSiblingElement("asset");
		}
		while (asset);
	}
	base->_image = image;
	return 1;
}

// 00447be0: <custom type="classe"/>
int RFXmlLoader::ReadCustom(const rf::TiXmlElement* element, CustomBase* base)
{
	ReadWnd(element, base);
	base->_className = RFUtf8ToWide983(element->FirstChildElement("custom")->Attribute("type"));
	return 1;
}
