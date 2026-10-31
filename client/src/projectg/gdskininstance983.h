#pragma once
// 983 00542f00: instancia do CGDSkinItem (include/jp/gdskinitem.h, frente B): na 1a chamada pega "TanLines" do CGameDataDB.
// Mesma declaracao que o jp/dummypet.cpp (B) faz localmente; usada por exhibition.cpp e partsinfo.cpp (A). Corpo ainda nao escrito.
class CGDSkinItem;
class CGDSkinItemPetJp
{
public:
	static CGDSkinItem* Instance983(); // 00542f00
};
