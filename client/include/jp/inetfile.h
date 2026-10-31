#pragma once
#include <string>

// JP 983: o chttp.cpp do 983 (unidade 00a4c160–00a4cc70, entre a objectfactory e a clientsocket). O `cHttp` do 645 não
// existe mais: o download é feito por três classes num namespace anônimo (CINet / CINetFile / CINetBuffer, WinINet) e por
// estas funções soltas. Corpo em src/projectg/jp/inetfile.cpp. Nomes meus (sessão B117).

// 00a4c9d0: baixa `url` para o arquivo `path` (If-Modified-Since com a data do arquivo; 304 = já está em dia)
bool __fastcall HttpDownloadFile983(const std::string& url, const std::string& path);
// 00a4ca80: HttpDownloadFile983(baseUrl + urlName, dir + name)
bool __fastcall HttpDownloadFileTo983(const std::string& name, const std::string& urlName, const std::string& dir,
	const std::string& baseUrl);
// 00a4cb40: devolve o texto de `url` + `file` ("" se falhar)
std::string __fastcall HttpDownloadText983(const std::string& file, const std::string& url);
// 00a4c1a0: último erro guardado (variável por thread)
unsigned long HttpGetLastError983();
