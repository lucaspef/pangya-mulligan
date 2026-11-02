#include "minatl.h"
#include "utilities.h"
#include "gatewayactor.h"
#include "parttidlist.h"
#include "jp/createcharacteroption.h"
#include "../../shared/localize.h"
#include "engine/wlocalize.h"
#include "vc10_layout.h"
C_ASSERT_VC10(offsetof(CSharedDoc, m_unused47d0) == 0x10704); // A738

namespace S5
{
	namespace UTIL
	{
		static unsigned long s_serverProperty;

		void SendCutInPacket(unsigned long uid, short type,
			unsigned long param1, unsigned long param2, bool bCharCutin)
		{
			WSendPacket packet(0xe5);
			packet.Encode4(uid);
			packet.Encode2(type);
			packet.Encode4(param1);
			packet.Encode4(param2);
			packet.Encode1(bCharCutin == 1);
			packet.Send(TO_GAME);
		}

		// 983 (00b2df40): FrSubEquipBar::Initialize e o desenho das cartas do taco
		const char* GetCardClubCharEmpty983()
		{
			return "card_club_char_empty";
		}

		// 983 (00b2df50)
		const char* GetCardClubCaddieEmpty983()
		{
			return "card_club_caddie_empty";
		}

		// 983 (00b2df70): chamada 3x pelo CTaskMain::ChangeRealMyRoom (00657cd0)
		void SendRealMyRoomEnter983(unsigned long uid, bool bEnter)
		{
			if (IsLocalContent((localContentType_t)0x57)
				&& IsLocalContent((localContentType_t)0x62))
			{
				WSendPacket packet((enumClientPacket)0xb7);
				packet.Encode4(uid);
				packet.Encode1(bEnter);
				packet.Send(TO_GAME);
			}
		}

		// 983 (00b2f070): pacote 0xf (sair da sala) montado num lugar so; o 645 montava em cada chamador (quitwindow,
		// sala de jogo, presenca). Depois do envio o jogador fica sem sala: m_myInfo.roomIndex e m_roomInfo.roomGuid = 0xffff
		void SendExitRoom983(unsigned char reason)
		{
			WSendPacket packet((enumClientPacket)0xf);
			packet.Encode1(reason);
			packet.Encode2(0xffff);
			packet.Encode8(0);
			packet.Encode8(0);
			packet.Send(TO_GAME);
			Doc()->m_myInfo.roomIndex = 0xffff;
			Doc()->m_roomInfo.roomGuid = 0xffff;
		}

		// 983 (00b2e3d0): pacote 0x17a, o mesmo registro do 0xf para as salas do Grand Prix (realGameType 0x14/0x15; sem mexer no doc)
		void SendExitGPRoom983(unsigned char reason)
		{
			WSendPacket packet((enumClientPacket)0x17a);
			packet.Encode1(reason);
			packet.Encode2(0xffff);
			packet.Encode8(0);
			packet.Encode8(0);
			packet.Send(TO_GAME);
		}

		// 983 (00b2ddc0)
		int GetHighPart983(__int64 value)
		{
			return (int)(value >> 32);
		}

		// 983 (00b2ddd0)
		int GetLowPart983(__int64 value)
		{
			return (int)value;
		}

		// 983 (00b2e8c0): chamada quando IsControlServerService983(service) da verdadeiro ("AlchemyDlg"/quebra de MP 0x2_00000000, alquimia de
		// cartas 0x1_00000000 no CTaskMain; outros em 0060d4a5, 00acc239, 00ae1bee); quem trata a msg 0x2ee nao foi levantado
		void NotifyServiceStopped983(__int64 service)
		{
			int high = GetHighPart983(service);
			int low = GetLowPart983(service);
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, 0x2ee /*so do 983*/, high, low, 0, 0, 0));
		}

		void SetServerProperty(unsigned long property)
		{
			s_serverProperty = property;
		}

		unsigned long GetServerProperty()
		{
			return s_serverProperty;
		}

		int EncodingUrl(const char* src, char* dst)
		{
			int len = lstrlenA(src);
			if (len == 0)
				return 0;

			const char* p = src;
			char special[] = "/?:=,<>\"-_.~'(){}\\[]";
			int count = 0;
			const char* end = src + len;
			for (; p < end; p++)
			{
				char c = *p;
				if (c == ' ' || (c >= '0' && c <= '9') ||
					(c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
					continue;
				if (strchr(special, c) == NULL)
					count++;
			}

			len += count * 2 + 1;
			if (dst == NULL)
				return len;

			char* out = dst;
			for (p = src; p < end; p++)
			{
				if (*p == ' ')
					*out = '+';
				else if ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'Z') ||
					(*p >= 'a' && *p <= 'z') || strchr(special, *p) != NULL)
					*out = *p;
				else
				{
					*out = '%';
					unsigned char hex = (unsigned char)*p >> 4;
					out++;
					if (hex <= 9)
						hex += '0';
					else
						hex += 'A' - 10;
					*out = hex;

					hex = *p & 0xf;
					out++;
					if (hex <= 9)
						hex += '0';
					else
						hex += 'A' - 10;
					*out = hex;
				}
				out++;
			}
			*out = '\0';
			return len;
		}

#define IS_HEX(c) \
	(((c) >= '0' && (c) <= '9') || ((c) >= 'A' && (c) <= 'F') || \
		((c) >= 'a' && (c) <= 'f'))

		int DecodingUrl(const char* src, char* dst)
		{
			int len = lstrlenA(src);
			if (len == 0)
				return 0;

			const char* p = src;
			const char* end = src + len;
			int count = 0;
			while (p < end - 2)
			{
				if (*p == '%' && IS_HEX(p[1]) && IS_HEX(p[2]))
				{
					count++;
					p += 3;
				}
				else
					p++;
			}

			int result = len - count * 2 + 1;
			if (dst == NULL)
				return result;

			for (; src < end;)
			{
				if (*src == '%' && IS_HEX(src[1]) && IS_HEX(src[2]))
				{
					char hex = src[1];
					if (hex >= '0' && hex <= '9')
						*dst = (hex - '0') * 16;
					else if (hex >= 'A' && hex <= 'F')
						*dst = (hex - 'A' + 10) * 16;
					else
						*dst = (hex - 'a' + 10) * 16;

					hex = src[2];
					if (hex >= '0' && hex <= '9')
						*dst += hex - '0';
					else if (hex >= 'A' && hex <= 'F')
						*dst += hex - 'A' + 10;
					else
						*dst += hex - 'a' + 10;
					dst++;
					src += 3;
				}
				else
				{
					if (*src == '+')
						*dst = ' ';
					else
						*dst = *src;
					dst++;
					src++;
				}
			}
			*dst = '\0';
			return result;
		}

		int GetEquipSkinIndex(eEquipSkin& skin, unsigned long typeId)
		{
			if ((typeId & 0xfc000000) != 0x38000000)
				return FALSE;

			switch ((typeId >> 22) & 0xf)
			{
			case 0:
				skin = EQUIP_SKIN_0;
				break;
			case 1:
				skin = EQUIP_SKIN_1;
				break;
			case 2:
				skin = EQUIP_SKIN_2;
				break;
			case 3:
				skin = EQUIP_SKIN_3;
				break;
			case 4:
				skin = EQUIP_SKIN_4;
				break;
			case 6:
				skin = EQUIP_SKIN_5;
				break;
			default:
				return FALSE;
			}
			return TRUE;
		}

		// 983 00b2fc60: tira da peca as cartas que acabaram de ser removidas
		void __fastcall ClearAttachedCards983(sItemInfo* item,
			std::list<unsigned long>& cardList)
		{
			for (int i = 0; i < 12; i++)
			{
				unsigned long card = item->attachCard[i];
				if (card == 0)
					continue;
				for (std::list<unsigned long>::iterator it = cardList.begin();
					it != cardList.end(); ++it)
				{
					if (card == *it)
						item->attachCard[i] = 0;
				}
			}
		}

		// 983 00b300b0: o 983 acha a peca pelo tipo da peca da carta (>>26):
		// 2 = m_partsList, 4 = m_clubSetMap (o 645 so olhava m_partsList)
		int ErasePartsCardAll(unsigned long uid,
			std::list<unsigned long>& cardList)
		{
			unsigned long partsTid = 0;
			std::list<sSCardAvilityPeriodInfo>& cards =
				Doc()->m_cardAbilityList[0];
			std::list<sSCardAvilityPeriodInfo>::iterator it = cards.begin();
			while (it != cards.end())
			{
				if ((*it).partsUid == uid)
				{
					partsTid = (*it).partsTid;
					cardList.push_back((*it).tid);
					it = Doc()->m_cardAbilityList[0].erase(it);
				}
				else
					++it;
			}

			if (partsTid != 0)
			{
				switch (partsTid >> 26)
				{
				case 2:
					{
						std::list<sItemInfo>::iterator item =
							std::find_if(Doc()->m_partsList.begin(),
								Doc()->m_partsList.end(), _FindByGuid(uid));
						if (item != Doc()->m_partsList.end())
							ClearAttachedCards983(&*item, cardList);
					}
					break;
				case 4:
					{
						std::map<unsigned int, sItemInfo>::iterator item =
							Doc()->m_clubSetMap.find(uid);
						if (item != Doc()->m_clubSetMap.end())
							ClearAttachedCards983(&item->second, cardList);
					}
					break;
				}
			}

			return cardList.size() != 0;
		}

		int Base64Encode(char* pSrc, int nLength, char* pDst)
		{
			unsigned char in[3] = { 0, 0, 0 };

			char* last = pSrc + nLength - 1;
			int j = 0;
			int result = (nLength / 3) * 4 + (nLength % 3 ? 4 : 0) + 1;
			int i = 0;
			for (char* p = pSrc; p <= last; i++, p++)
			{
				int k = i % 3;
				in[k] = *p;
				if (k == 2 || p == last)
				{
					unsigned char out[4];
					out[0] = in[0] >> 2;
					out[1] = ((in[0] & 3) << 4) | (in[1] >> 4);
					out[2] = ((in[1] & 0xf) << 2) | (in[2] >> 6);
					out[3] = in[2] & 0x3f;
					pDst[j++] = s_base64Table[out[0]];
					pDst[j++] = s_base64Table[out[1]];
					pDst[j++] = k == 0 ? '=' : s_base64Table[out[2]];
					pDst[j++] = k < 2 ? '=' : s_base64Table[out[3]];
					in[0] = in[1] = in[2] = 0;
				}
			}
			pDst[j] = '\0';
			return result;
		}

		int Base64Decode(char* pSrc, unsigned char* pDst, int nLength)
		{
			int prev = 0;
			int j = 0;
			int state = 0;
			for (char* p = pSrc; *p != '\0'; p++)
			{
				int value = s_base64Index[*p];
				if (value == -1)
					continue;

				switch (state)
				{
				case 0:
					state = 1;
					break;
				case 1:
					if (j < nLength)
						pDst[j++] = (prev << 2) | ((value >> 4) & 3);
					state = 2;
					break;
				case 2:
					if (j < nLength)
						pDst[j++] = (prev << 4) | ((value >> 2) & 0xf);
					state = 3;
					break;
				case 3:
					if (j < nLength)
						pDst[j++] = (prev << 6) | value;
					state = 0;
					break;
				}
				prev = value;
			}
			if (j > 0)
				pDst[j] = '\0';
			return j;
		}

		int IsLunarYear(unsigned short year)
		{
			if (year % 4 == 0)
			{
				if (year % 100 == 0)
				{
					if (year % 400 == 0)
						return TRUE;
					return FALSE;
				}
				return TRUE;
			}
			return FALSE;
		}

		unsigned short GetMonthlyDayCount(unsigned short year,
			unsigned short month)
		{
			if (month == 2)
			{
				if (IsLunarYear(year))
					return 28;
				return 29;
			}
			if (month == 4 || month == 6 || month == 9 || month == 11)
				return 30;
			return 31;
		}

		unsigned long GetNickColor(unsigned long identity, int paran,
			unsigned short flag)
		{
			if (identity & 0x14)
			{
				if (Doc()->m_myInfo.info.dwIdentity & 0x14)
					return 0x50ffffff;
				if (!(flag & 1))
					return 0;
			}
			else if (IsLocalContent(S3_PARAN_CHANNELING))
			{
				if (paran)
					return 0xff33dbff;
			}
			return 0xffffffff;
		}

		int GetHoleinCoinCount(int coin, int multiple)
		{
			if (coin == 0)
				return 0;
			if (multiple < 2)
				return coin;
			return coin * multiple;
		}

		bool IsEventServerControl(eGameServerProperty property)
		{
			return (Doc()->m_serverProperty & property) ? true : false;
		}

		bool FindVisibleWindowFromExStyle(unsigned long exStyle,
			std::vector<HWND>& windows)
		{
			HWND hWnd = FindWindowA(NULL, NULL);
			while (hWnd)
			{
				if (IsWindowVisible(hWnd))
				{
					LONG style = GetWindowLongA(hWnd, GWL_EXSTYLE);
					if ((style & exStyle) == exStyle)
						windows.push_back(hWnd);
				}
				hWnd = GetWindow(hWnd, GW_HWNDNEXT);
			}
			return !windows.empty();
		}

		int GetWindowBorderSize(HWND hWnd)
		{
			RECT rcWindow, rcClient;
			GetWindowRect(hWnd, &rcWindow);
			GetClientRect(hWnd, &rcClient);
			return ((rcWindow.right - rcWindow.left) -
					   (rcClient.right - rcClient.left)) /
				2;
		}

		int GetWindowTitleBarSize(HWND hWnd)
		{
			RECT rcWindow, rcClient;
			GetWindowRect(hWnd, &rcWindow);
			GetClientRect(hWnd, &rcClient);
			int border = GetWindowBorderSize(hWnd);
			return (rcWindow.bottom - rcWindow.top) -
				(rcClient.bottom - rcClient.top) - border;
		}

		bool IsInRect(const RECT& rect, int x, int y)
		{
			return x >= rect.left && x <= rect.right && y >= rect.top &&
				y <= rect.bottom;
		}

		bool IsOverlappedRect(const RECT& rect1, const RECT& rect2)
		{
			if (IsInRect(rect2, rect1.left, rect1.top) ||
				IsInRect(rect2, rect1.left, rect1.bottom) ||
				IsInRect(rect2, rect1.right, rect1.top) ||
				IsInRect(rect2, rect1.right, rect1.bottom))
				return true;

			if (IsInRect(rect1, rect2.left, rect2.top) ||
				IsInRect(rect1, rect2.left, rect2.bottom) ||
				IsInRect(rect1, rect2.right, rect2.top) ||
				IsInRect(rect1, rect2.right, rect2.bottom))
				return true;

			return false;
		}

		bool CalculateUVPos(WRect& uv, int width, int height, float cellWidth,
			float cellHeight, int index)
		{
			if ((float)width < cellWidth || (float)height < cellHeight)
				return false;

			int rows = (int)((float)height / cellHeight);
			int cols = (int)((float)width / cellWidth);
			if (cols * rows < index)
				return false;

			float du = 1.0f / (float)width;
			uv.x = du * ((index / rows) * cellWidth);
			float dv = 1.0f / (float)height;
			uv.y = dv * ((index % rows) * cellHeight);
			uv.w = du * cellWidth;
			uv.h = dv * cellHeight;
			return true;
		}
	}

	bool UnderCategory(unsigned long charTypeId, unsigned long typeId,
		unsigned long mask)
	{
		if ((typeId & 0x600) != 0 ||
			((typeId >> 18) & 0xff) != (charTypeId & 0x3ffffff))
			return false;

		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(typeId);
		if (pPart != NULL)
		{
			if (mask & (1 << pPart->Category))
				return true;
		}
		return false;
	}

	bool UnderCatogory(unsigned long charTypeId, unsigned long mask,
		IFF_STRUCT::sPart* pPart)
	{
		unsigned long typeId = pPart->c.TypeId;
		if ((typeId & 0x600) != 0 ||
			((typeId >> 18) & 0xff) != (charTypeId & 0x3ffffff))
			return false;

		return (mask & (1 << pPart->Category)) ? true : false;
	}

	void GetLine(char* src, char* dst)
	{
		while (*src)
		{
			if (*src == '\\' && *(src + 1) == 'n')
			{
				*dst = '\n';
				dst++;
				src += 2;
			}
			else
				*dst++ = *src++;
		}
	}

	bool IsUsableCalipers()
	{
		// 983 00b2e8b0: flag do documento (+0x10704, campo ainda sem nome);
		// o 645 perguntava ao gateway actor (M983(0x26f))
		return *(const bool*)&Doc()->m_unused47d0;
	}
}

namespace _util
{
	const char* GetAvatarServerServiceMessage(int property)
	{
		// HACK: dead dependency (removed by DCE) so this function is emitted
		// after SendCutInPacket, as in the original
		if (0)
			S5::UTIL::SendCutInPacket(0, 0, 0, 0);
		const char* msg = NULL;
		switch (property)
		{
		case 0x2:
		case 0x40:
		case 0x80:
		case 0x100:
		case 0x200:
		case 0x400:
		case 0x800:
		case 0x1000:
			msg = K2L_Compatibility("\xb4\xeb\xc8\xad\xb9\xe6 \xc0\xfc\xbf\xeb "
				  "\xbc\xad\xb9\xf6\xbf\xa1\xbc\xad\xb4\xc2 "
				  "\xb0\xd4\xc0\xd3\xb9\xe6\xc0\xbb \xb8\xb8\xb5\xe9 \xbc\xf6 "
				  "\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		}
		return msg;
	}

	const char* GetNormalServerServiceMessage(int property)
	{
		// HACK: dead dependency (removed by DCE) so this function is emitted
		// after SendCutInPacket, as in the original
		if (0)
			S5::UTIL::SendCutInPacket(0, 0, 0, 0);
		const char* msg = NULL;
		if (property == 0x2000)
			msg = K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbc\xad\xb9\xf6\xbf\xa1\xbc\xad\xb4\xc2 "
				  "\xb4\xeb\xc8\xad\xb9\xe6\xc0\xbb \xb8\xb8\xb5\xe9 \xbc\xf6 "
				  "\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
		return msg;
	}

	bool IsAvatarChatServer(int property)
	{
		return (Doc()->m_curGameServer.eventFlags & property) ? true : false;
	}

	void GenerateRandomTidParts(unsigned long charTypeId,
		unsigned long* const tids, unsigned long* const auxTids)
	{
		CPartTidList tidList;
		*auxTids = 0;
		*tids = 0;
		Doc()->m_itemManager.GetDefCombo(charTypeId, tids);

		// 983 00b2e950: o sorteio por personagem saiu daqui para o
		// CreateCharacterOption (004cf170)
		do
		{
			CreateCharacterOption::Instance()->RandomParts983(tids,
				charTypeId & 0x3ffffff);

			sCharacterInfo info;
			info.tid = charTypeId;
			memcpy(info.tidParts, tids, sizeof(info.tidParts));
			tidList.SetTids(&info, 0xff);
		} while (!tidList.IsComboValid());
	}
}
