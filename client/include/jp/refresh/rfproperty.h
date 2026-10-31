#pragma once
// JP 983: propriedades do leitor de layout da Refresh (RTTI .?AVRFProperty@@ e derivadas; vtables 00d658e0…00d659d0). O nome
// (+0x04) é a chave no mapa de propriedades da estrutura do XML (WndBaseStruct +0x16c); o valor tipado fica em +0x20 e é lido
// do texto pelo SetValue (slot 3; corpos no RFManager.cpp: jp/rfmanager.cpp). Sessão B145 (F3.7.13 item 2). Nomes dos métodos
// e dos campos meus (a RTTI só dá as classes).
#include <string>

class RFProperty
{
public:
	RFProperty() {}
	virtual ~RFProperty() {} // 00438dd0 (??_G)
	virtual std::string GetName() const { return _name; } // 00438d70
	virtual void SetName(const std::string& name) { _name = name; } // 00438da0
	virtual void SetValue(const std::string& value) {} // 00438dc0 (o RFRectProperty não troca)

protected:
	std::string _name; // +0x04
};

class RFBooleanProperty : public RFProperty // 0x24
{
public:
	RFBooleanProperty() : _value() {}
	bool Get() const { return _value; }
	void Set(bool value) { _value = value; }
	virtual void SetValue(const std::string& value); // 00464ff0
protected:
	bool _value; // +0x20
};

class RFIntegerProperty : public RFProperty
{
public:
	RFIntegerProperty() : _value() {}
	int Get() const { return _value; }
	void Set(int value) { _value = value; }
	virtual void SetValue(const std::string& value); // 00465070
protected:
	int _value; // +0x20
};

class RFStringProperty : public RFProperty // 0x3c
{
public:
	RFStringProperty() : _value() {}
	std::wstring Get() const { return _value; }
	void Set(const std::wstring& value) { _value = value; }
	const std::wstring& GetRef983() const { return _value; } // (sem cópia: RFLabel::Create atribui direto do campo)
	virtual void SetValue(const std::string& value); // 00464f70: texto UTF-8 → wstring
protected:
	std::wstring _value; // +0x20
};

class RFStartPositionProperty : public RFProperty // 0 manual, 1 "CenterParent", 2 "CenterScreen"
{
public:
	RFStartPositionProperty() : _value() {}
	int Get() const { return _value; }
	void Set(int value) { _value = value; }
	virtual void SetValue(const std::string& value); // 004650a0
protected:
	int _value; // +0x20
};

class RFEscKeyActionProperty : public RFProperty // 1 "Close"
{
public:
	RFEscKeyActionProperty() : _value() {}
	int Get() const { return _value; }
	void Set(int value) { _value = value; }
	virtual void SetValue(const std::string& value); // 00465110
protected:
	int _value; // +0x20
};

class RFAnchorProperty : public RFProperty // bits Left 1, Top 2, Right 4, Bottom 8
{
public:
	RFAnchorProperty() : _value() {}
	int Get() const { return _value; }
	void Set(int value) { _value = value; }
	virtual void SetValue(const std::string& value); // 00465140
protected:
	int _value; // +0x20
};

class RFColorProperty : public RFProperty // ARGB
{
public:
	RFColorProperty() : _value() {}
	unsigned long Get() const { return _value; }
	void Set(unsigned long value) { _value = value; }
	virtual void SetValue(const std::string& value); // 00465160
protected:
	unsigned long _value; // +0x20
};

class RFHAlignProperty : public RFProperty // Left 0, Center 1, Right 2
{
public:
	RFHAlignProperty() : _value() {}
	int Get() const { return _value; }
	void Set(int value) { _value = value; }
	virtual void SetValue(const std::string& value); // 00465180
protected:
	int _value; // +0x20
};

class RFVAlignProperty : public RFProperty // Top 0, Center 1, Bottom 2
{
public:
	RFVAlignProperty() : _value() {}
	int Get() const { return _value; }
	void Set(int value) { _value = value; }
	virtual void SetValue(const std::string& value); // 004651a0
protected:
	int _value; // +0x20
};

class RFTextTrimmingProperty : public RFProperty // None 0, CharacterEllipsis 1
{
public:
	RFTextTrimmingProperty() : _value() {}
	int Get() const { return _value; }
	void Set(int value) { _value = value; }
	virtual void SetValue(const std::string& value); // 004651c0
protected:
	int _value; // +0x20
};

class RFOrientationProperty : public RFProperty // Vertical 0, Horizontal 1
{
public:
	RFOrientationProperty() : _value() {}
	int Get() const { return _value; }
	void Set(int value) { _value = value; }
	virtual void SetValue(const std::string& value); // 004651e0
protected:
	int _value; // +0x20
};

class RFRectProperty : public RFProperty // 0x30; ??_G 0043bbe0; valor lido à parte pelo leitor (SetValue não trocado)
{
public:
	RFRectProperty() : _left(), _top(), _right(), _bottom() {}
	template<class T> const T& As983() const { return *reinterpret_cast<const T*>(&_left); } // os 4 valores como RFRect (RFLabel::Create)
protected:
	int _left; // +0x20
	int _top; // +0x24
	int _right; // +0x28
	int _bottom; // +0x2c
};
