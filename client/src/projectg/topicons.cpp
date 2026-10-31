#include "minatl.h"

// 983: esta unidade do 645 NAO existe no exe 983 (sem RTTI de CIconInterface, CWizcityOpenEvent nem CWebIcons_WebEvent1/2).
// O sistema de icones do topo foi refeito: include/jp/topicon.h, src/projectg/jp/topicon.cpp, descimageonlyicon.cpp e
// eventicon.cpp (frente B, pedido B56). A unidade fica vazia para a lista de build do rebang seguir valendo; o corpo do 645
// esta no rebang (f0a85b5) e no historico do git.
