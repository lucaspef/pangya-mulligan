#pragma once
#include <map>
#include <string>
#include <unordered_set>
#include <vector>
#include "engine/singleton.h"
#include "engine/wresrcmng.h"
#include "topicon.h"

class TiXmlElement;

// JP 983: sorteio com peso dos papéis de parede (registro de 0x18 bytes dentro do CExtraResource; sem RTTI). A chave do mapa
// é a soma dos pesos até a linha, menos 1; `Pick` procura a 1ª chave >= valor. Nomes meus.
class CExtraWallpaper
{
public:
	CExtraWallpaper(); // 004e0da0 (faz `srand(time(NULL))`)

	bool IsEmpty() const; // 004dd060
	// 004dd7b0: 0 = a linha atual; 1 = a seguinte; 2 = sorteada pelo peso; outro = a 1ª
	const std::string& Pick(int mode);
	void Add(const std::string& file, int prob); // 004e0860

protected:
	unsigned int m_total; // +0x00
	std::map<unsigned int, std::string> m_files; // +0x04
	unsigned int m_cur; // +0x14
};

// JP 983: conteúdo extra baixado do servidor de atualização (classe nova; RTTI CExtraResource, vtable 00d04eb0, base
// WSingleton<CExtraResource> 00d04d94, instância 00e74d84, 0x104 bytes — `push 0x104` do criador 00a694f4). Baixa
// "extracontents.xml" por HTTP, escolhe o tema ("ex_theme.txt" ou o 1º), baixa o XML do tema e, dele, os arquivos de
// troca, os ícones do topo, os avisos e os papéis de parede da carga e do lobby. Nomes de método e de campo meus.
class CExtraResource : public WSingleton<CExtraResource>
{
public:
	CExtraResource(); // 004e0e30
	virtual ~CExtraResource(); // 004e09f0 (slot 0 = 004e0f60)

	void Load(); // 004e1060
	const char* GetLoading(int mode); // 004ddbb0
	const char* GetLobby(int mode); // 004ddbe0
	const std::vector<std::string>& GetNotices() const { return m_notices; } // 004dbb40 (cópia COMDAT; pedido A87)

protected:
	bool Download(const std::string& file, const std::string& url); // 004de830: texto de url + file em m_xml
	std::string GetThemeName(); // 004dfd40: 1ª linha útil de "ex_theme.txt"; sem arquivo, "pangya_default"
	bool LoadTheme(); // 004e0070
	bool LoadXml(); // 004e02b0
	// 004df170: baixa o <file name version url> para a pasta extrares e registra a troca; devolve o nome ("" se não há)
	std::string GetFile(TiXmlElement* pFile, bool bExt);
	friend class CDescImageOnlyIcon; // GetFile (00b13e60)
	friend class CTopIcon; // GetFile (006b1830)
	void LoadReplacement(TiXmlElement* pElem); // 004df570
	void LoadTopIcons(TiXmlElement* pElem); // 004dc530
	void LoadNotice(TiXmlElement* pElem); // 004e0f90
	void LoadLoading(TiXmlElement* pElem); // 004e0ba0
	void LoadLobby(TiXmlElement* pElem); // 004e0ca0

	std::string m_xml; // +0x28
	std::string m_dir; // +0x44
	std::string m_themeSrc; // +0x60
	std::string m_themeUrl; // +0x7c
	std::vector<std::string> m_notices; // +0x98
	CExtraWallpaper m_lobby; // +0xa8
	CExtraWallpaper m_loading; // +0xc0
	std::tr1::unordered_set<std::string> m_setD8; // +0xd8 (só o construtor e o destrutor mexem nesta unidade)
};

// PROVISÓRIO (pedido B → A em out/PEDIDOS.md, sessão B19): funções do 983 em unidades da frente A que o fonte ainda não tem.
// As classes de empréstimo não têm campo: só emprestam o `this`.
// chttp.cpp do 983 (HttpDownloadFile983 00a4c9d0 / HttpDownloadText983 00a4cb40): escrito pela B117 em jp/inetfile.cpp
#include "inetfile.h"

class WResourceManagerJp
{
public:
	void AddExtraFile983(const char* name, const char* path); // 00b59fe0
};
