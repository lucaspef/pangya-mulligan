#pragma once

#include "guilddefine.h"

struct FrListItem;

namespace _guild
{
	class CGuildMainForm;
	class CGuildInfo;

	class IGuildAuthController
	{
	protected:
		IGuildAuthController();

	public:
		virtual ~IGuildAuthController();

		static void SetLinkDepends(CGuildMainForm* pForm, CGuildInfo* pInfo);

		virtual void DrawGuildTalk(FrListItem* pItem) { }

		virtual void PopupCommandMenu(float x, float y);

		virtual void ProcManagerCommand(const char* command) { }

		virtual void OnGuildIntroConfirm_LButtonUp() { }

		virtual void OnGuildNoticeConfirm_LButtonUp() { }

		virtual void OnGuildJoin_LButtonUp();

		virtual void OnGuildJoinAbort_LButtonUp() { }

		virtual void OnGuildShutDown_LButtonUp() { }

		virtual void OnGuildOut_LButtonUp() { }

		static CGuildMainForm* m_linkForm;
		static CGuildInfo* m_linkInfo;
	};

	class CGuildManager : public IGuildAuthController
	{
	public:
		CGuildManager() { }

		virtual void DrawGuildTalk(FrListItem* pItem);
		virtual void ProcManagerCommand(const char* command);
		virtual void OnGuildIntroConfirm_LButtonUp();
		virtual void OnGuildNoticeConfirm_LButtonUp();
	};

	class CGuildMaster : public CGuildManager
	{
	public:
		CGuildMaster() { }

		virtual void PopupCommandMenu(float x, float y);
		virtual void OnGuildShutDown_LButtonUp();
		virtual void OnGuildOut_LButtonUp();
	};

	class CGuildSubMaster : public CGuildManager
	{
	public:
		CGuildSubMaster() { }

		virtual void PopupCommandMenu(float x, float y);
		virtual void OnGuildOut_LButtonUp();
	};

	class CGuildMember : public IGuildAuthController
	{
	public:
		CGuildMember() { }

		virtual void DrawGuildTalk(FrListItem* pItem);
		virtual void PopupCommandMenu(float x, float y);
		virtual void ProcManagerCommand(const char* command);
		virtual void OnGuildOut_LButtonUp();
	};

	class CGuildWaitMember : public IGuildAuthController
	{
	public:
		CGuildWaitMember() { }

		virtual void OnGuildJoinAbort_LButtonUp();
		virtual void OnGuildOut_LButtonUp();
	};

	class CGuildNull : public IGuildAuthController
	{
	public:
		CGuildNull() { }

		virtual void OnGuildJoin_LButtonUp();
	};
}
