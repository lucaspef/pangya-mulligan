#pragma once

class cFile
{
public:
	cFile()
		: m_nLen(0)
	{
	}
	virtual ~cFile() { }
	virtual bool Open(const char* pFilename) { return 0; }
	virtual int Read(void* data, int len) = 0;
	virtual int GetByte() = 0;
	virtual int Tell() = 0;
	virtual void Seek(int pos, int pivot) = 0;
	bool Scan(const char* format, ...);
	// 983: leitores do arquivo de mapa (0092a640 / 0092a550, thiscall no cFile; corpos em src/projectg/polysoup.cpp).
	// Devolvem o total de bytes lidos. Nomes meus.
	int ReadAabb983(class Waabb& aabb);
	int ReadMatrix983(class WMatrix& mat);
	// 983 00b69610: matriz de material de shader (16 floats; corpo em src/engine/wpuppet.cpp). Nome meu.
	int ReadD3DXMatrix983(struct D3DXMATRIX& mat);
	int Length() { return m_nLen; }

	int m_nLen;
};

cFile* __cdecl GetCFile(const char* filename, int mode, int reserved);
void __cdecl CloseCFile(cFile* file);
