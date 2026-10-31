#pragma once
// JP 983: cabeçalhos da biblioteca Refresh (RF) como a biblioteca os vê (F3.7.13 item 3, sessão B149). O 983 EXPORTA as
// classes da RF (1119 nomes na tabela de exportação do exe, out/F3.7/s149/exports983.tsv): as classes foram declaradas com
// __declspec(dllexport), por isso o exe tem as cópias fora de linha de todos os métodos embutidos e os construtores de cópia /
// operator= implícitos de cada classe (as unidades da biblioteca emitem tudo). Assinaturas pelos nomes decorados exportados.
// Só as unidades da biblioteca (src/projectg/jp/rf<arquivo da RF>.cpp) incluem estes cabeçalhos; as janelas do jogo ainda usam
// as visões de refresh/rfscenemanager.h e refresh/rfbase.h (unificar depois: ver out/F3.7/s149/RELATORIO.md).
#define RF_API __declspec(dllexport)

// caminho dos cabeçalhos da RF no __FILE__ dos asserts do 983 (o RFWnd.h com "E:" e "Refresh" maiúsculos; os outros minúsculos)
#define RF_INCLUDE_RFWND_H983 "E:\\program_code\\server_client_launcher\\trunk\\code\\client\\Refresh\\include\\RFWnd.h"
#define RF_INCLUDE_RFEVENTHANDLER_H983 "e:\\program_code\\server_client_launcher\\trunk\\code\\client\\refresh\\include\\RFEventHandler.h"
