#pragma once
#include "vc10_layout.h"

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
		// D717: std::map m_authControllers (+0x2b0) tem 0x10 B (ingameguildlobbydlg.h, dono do ctor); 983 SetCommandUserInfo 0066dfe0 grava +0x2d8
		unsigned char m_unused1f0[0xd0];
		float m_talkTextOffset; // +0x2c0
		float m_talkIconOffset; // +0x2c4
		unsigned char m_unused2c8[0x10];
		const GUILD_USER_LIST* m_pCommandUserInfo; // +0x2d8
		unsigned char m_unused2dc[0x34];
	};
	C_ASSERT_VC10(sizeof(CGuildMainForm) == 0x310 && offsetof(CGuildMainForm, m_pCommandUserInfo) == 0x2d8);
}
