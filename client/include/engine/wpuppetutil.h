#pragma once

#include "wmath.h"

// Definicao parcial (F5, decisao): so os membros que as nossas unidades usam; o resto do 983 fica de fora.
struct sBgModel
{
	unsigned char flag;
	int faceNum;
	char name[32];
	Waabb localAabb;
	Waabb sceneAabb;
	WMatrix mat;
	int detail;
	char option[32];

	sBgModel() { flag = 0; }
};
