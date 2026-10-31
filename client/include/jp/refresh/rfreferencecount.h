#pragma once
// JP 983: contagem de referências da Refresh (RTTI .?AVRFReferenceCount@@). Saiu do rfwndstruct.h na B147 (as imagens do
// rfimageset.h também derivam dela). Nomes dos métodos meus.

// contagem de referências da Refresh (vtable 00d641d8: só o destrutor, 0041e170); contador em +4
class RFReferenceCount
{
public:
	RFReferenceCount() : _refCount(0) {}
	virtual ~RFReferenceCount() {}

	void AddRef() { ++_refCount; }
	int Release() { if (_refCount > 0) --_refCount; return _refCount; }
	int GetRefCount() const { return _refCount; }

protected:
	int _refCount; // +0x04
};

// solta uma referência e apaga o objeto na última (embutido nos destrutores das estruturas)
#define RF_SAFE_RELEASE983(p) \
	if (p) \
	{ \
		if ((p)->Release() == 0 && (p)) \
			delete (p); \
		(p) = 0; \
	}
