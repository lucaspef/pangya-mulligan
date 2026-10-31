#pragma once

#include <list>
#include <vector>

class WRect;

namespace IFF_STRUCT
{
	struct sPart;
}

// Value names unknown; only the type names are public in the PDB.
enum eEquipSkin
{
	EQUIP_SKIN_0,
	EQUIP_SKIN_1,
	EQUIP_SKIN_2,
	EQUIP_SKIN_3,
	EQUIP_SKIN_4,
	EQUIP_SKIN_5
};

enum eGameServerProperty
{
};

namespace S5
{
	namespace UTIL
	{
		static char s_base64Table[64] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G',
			'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
			'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
			'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
			'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6',
			'7', '8', '9', '+', '/' };

		static int s_base64Index[256] = { -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1,
			-1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
			14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
			-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
			42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1 };

		// 983 (00b2cfa0): 5o argumento novo, vai como byte (1 = corte da carta do personagem)
		void SendCutInPacket(unsigned long uid, short type,
			unsigned long param1, unsigned long param2, bool bCharCutin = false);
		// 983 (00b2df40 / 00b2df50): nome da imagem do slot de carta vazio do taco (personagem / caddie); nomes meus
		const char* GetCardClubCharEmpty983();
		const char* GetCardClubCaddieEmpty983();
		// 983 (00b2df70): pacote 0xb7 (entrar no my room de uid) com as duas travas de conteudo; o 645 montava no chamador
		void SendRealMyRoomEnter983(unsigned long uid, bool bEnter);
		// 983 (00b2f070): pacote 0xf (sair da sala; reason 1 = sair do jogo, 0 = voltar ao lobby) e sala do jogador = 0xffff
		void SendExitRoom983(unsigned char reason);
		// 983 (00b2e3d0): pacote 0x17a (sair da sala do Grand Prix, realGameType 0x14/0x15), mesmo registro do 0xf
		void SendExitGPRoom983(unsigned char reason);
		// 983 (00b2ddc0 / 00b2ddd0): metade alta / baixa de um __int64 (fora de linha no 983); nomes meus
		int GetHighPart983(__int64 value);
		int GetLowPart983(__int64 value);
		// 983 (00b2e8c0): servico suspenso pelo controle do servidor (mascara de 64 bits do IsControlServerService983) ->
		// msg 0x2ee ao ator principal (param1 = metade alta, param2 = baixa); nome meu
		void NotifyServiceStopped983(__int64 service);
		void SetServerProperty(unsigned long property);
		int EncodingUrl(const char* src, char* dst);
		int GetEquipSkinIndex(eEquipSkin& skin, unsigned long typeId);
		int ErasePartsCardAll(unsigned long uid,
			std::list<unsigned long>& cardList);
		int Base64Encode(char* pSrc, int nLength, char* pDst);
		int Base64Decode(char* pSrc, unsigned char* pDst, int nLength);
		unsigned long GetServerProperty();
		int DecodingUrl(const char* src, char* dst);
		int IsLunarYear(unsigned short year);
		unsigned short GetMonthlyDayCount(unsigned short year,
			unsigned short month);
		unsigned long GetNickColor(unsigned long identity, int paran,
			unsigned short flag);
		int GetHoleinCoinCount(int coin, int multiple);
		bool IsEventServerControl(eGameServerProperty property);
		bool FindVisibleWindowFromExStyle(unsigned long exStyle,
			std::vector<HWND>& windows);
		int GetWindowBorderSize(HWND hWnd);
		int GetWindowTitleBarSize(HWND hWnd);
		bool IsInRect(const RECT& rect, int x, int y);
		bool IsOverlappedRect(const RECT& rect1, const RECT& rect2);
		bool CalculateUVPos(WRect& uv, int width, int height, float cellWidth,
			float cellHeight, int index);
	}

	bool UnderCategory(unsigned long charTypeId, unsigned long typeId,
		unsigned long mask);
	bool UnderCatogory(unsigned long charTypeId, unsigned long mask,
		IFF_STRUCT::sPart* pPart);
	void GetLine(char* src, char* dst);
	bool IsUsableCalipers();
}

namespace _util
{
	const char* GetAvatarServerServiceMessage(int property);
	const char* GetNormalServerServiceMessage(int property);
	bool IsAvatarChatServer(int property);
	void GenerateRandomTidParts(unsigned long charTypeId,
		unsigned long* const tids, unsigned long* const auxTids);
}
