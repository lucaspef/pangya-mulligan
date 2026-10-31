#pragma once

static const char* s_guildActor[2] = { "CGuildActor", "GuildActor" };
static const char* s_guildLineFeed = "\n";
#include <utility>
#include "actor.h"
#include "frcmdtarget.h"
#include "guilddefine.h"

struct GUILD_INFO;
struct GUILD_USER_INFO;
class FrForm;

namespace _guild
{
	class CGuildInfo;
	class CGuildMainForm;
	class FrCreateGuildKit;
	class FrRegisterGuildMark;
	class FrChangeGuildName;
	class FrGuildJoin;

	class CGuildActor : public IActor, public FrCmdTarget
	{
	protected:
		CGuildActor();

	public:
		virtual ~CGuildActor();

		DECLARE_OBJECT(CGuildActor)

		virtual void OnInit();
		virtual void HandleMsg(const MsgObject& msg);

		void RequestGuildUserList(int page);

		bool OnCreateGuildFormResult(int result, FrForm* pForm);
		bool OnRegisterGuildMarkResult(int result, FrForm* pForm);
		bool OnChangeGuildNameResult(int result, FrForm* pForm);
		bool OnGuildJoinFormResult(int result, FrForm* pForm);
		bool OnMainFormCloseResult(int result, FrForm* pForm);
		bool OnGuildShutDownResult(int result, FrForm* pForm);
		bool OnGuildMasterDelegate(int result, FrForm* pForm);
		bool OnGuildOutResult(int result, FrForm* pForm);

	private:
		void InitControl();
		void ExecGuildMessage(const MsgObject& msg);
		void OpenGuileMainForm();
		void OpenGuildItemForm(unsigned long typeId);
		void OpenGuildJoinForm(int guildNumber);
		void OpenGuildQuestionForm(const char* message, int type);
		void ReInitMyGuildInfo_SharedDoc(GUILD_USER_INFO* pInfo);
		std::pair<bool, bool> CheckMyGuildState(GUILD_USER_INFO* pInfo);
		void ChangeGuildEmblem(GUILD_USER_INFO* pInfo);
		void ChangeGuildName(GUILD_INFO* pInfo);
		const char* SystemMsg(int id);

		FrCreateGuildKit* m_pCreateGuildForm;
		FrRegisterGuildMark* m_pRegisterGuildMarkForm;
		FrChangeGuildName* m_pChangeGuildNameForm;
		FrGuildJoin* m_pGuildJoinForm;
		CGuildMainForm* m_pMainForm;
		CGuildInfo* m_pGuildInfo;
	};
}
