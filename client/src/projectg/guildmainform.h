#pragma once
#include "vc10_layout.h"

#include <list>
#include <map>
#include <string>
#include "frform.h"

class FrEdit;
class FrContextMenuCtrl;
class Bitmap;
struct FrListItem;
struct GUILD_USER_LIST;

namespace _guild
{
	class CGuildInfo;

	class CGuildMainForm : public FrForm
	{
	public:
		static const WRTTI m_RTTI;

		struct sMemberItem
		{
			const GUILD_USER_LIST* pInfo;
			bool bEmpty;
		};

		CGuildMainForm();

		void SetLinkGuildInfo(CGuildInfo* pInfo) { m_pLinkGuildInfo = pInfo; }

		void DisableJoinButton();
		void InitSearchMode();
		void RequestMember_Page(int page);
		void UnLockScrollMemberList();
		void RefreshMyGuildHistory();
		void RequestChangeClass(GUILD_CLASS_IDX classIdx);
		void ClearForm();
		void RefreshGuildMember();
		void RefreshGuildSearch();
		void Refresh();

		// Resto da classe do 983 sem acesso no nosso codigo: so reserva os bytes.
		unsigned char m_unused110[0x58];
		FrEdit* m_pIntroduceEdit;
		FrEdit* m_pNoticeEdit;
		unsigned char m_unused170[0x1c];
		FrContextMenuCtrl* m_pCommandMenu;
		FrListItem* m_pSelectedMember;
		unsigned char m_unused194[0x3c];
		const Bitmap* m_pTalkIcon;
		unsigned char m_unused1d4[0x14];
		CGuildInfo* m_pLinkGuildInfo;
		int m_guildActorParam;
		// B1637: map e list com o tamanho do compilador (classe real em ingameguildlobbydlg.h); bytes fixos do VC10
		// punham m_pCommandUserInfo +0xc adiante da real no cl 14.44. 983 SetCommandUserInfo 0066dfe0 grava +0x2d8
		unsigned char m_unused1f0[0xc0];
		unsigned char m_authControllers[sizeof(std::map<int, void*>)];
		float m_talkTextOffset; // +0x2c0
		float m_talkIconOffset; // +0x2c4
		unsigned char m_memberItems[sizeof(std::list<void*>)];
		void* m_pSearchArrow;
		const GUILD_USER_LIST* m_pCommandUserInfo; // +0x2d8
		unsigned char m_searchItems[sizeof(std::list<void*>)];
		int m_memberPage;
		int m_searchPage;
		int m_searchMode;
		unsigned char m_searchText[sizeof(std::string)];
	};
	C_ASSERT_VC10(sizeof(CGuildMainForm) == 0x310 && offsetof(CGuildMainForm, m_pCommandUserInfo) == 0x2d8);
}
