#pragma once

// Game summary the server sends for each ghost-mode map (list node value).
struct GHOST_GAME_BRIEF
{
	int ownerUid; // dono do registro; 1o campo do nome do .GST
	int id;
	int ball;
	int pang;
	int score;
	int recordId; // ultimo campo do .GST; vai de volta no 0xc2 sub 9
	int active; // 0 = mapa com registro mas bloqueado

	bool operator==(int _id) const { return id == _id; }
};
