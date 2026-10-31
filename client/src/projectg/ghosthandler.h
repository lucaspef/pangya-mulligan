#pragma once

// HACK: everything but CGhostHandler is only seen by ghosthandler.cpp. projectg.cpp
// (SaveScreenShot) reacts to the number of names declared before it, and
// ghostdocument.h (included by ghostmain and ghostmanager together with this header)
// still has another struct under the name GHOST_GAME_BRIEF.
#ifdef REBANG_GHOSTHANDLER_CPP
#include <stdio.h>
#include <list>
#include <string>
#include "actor.h"
#include "singleton.h"
#include "frcmdtarget.h"

class CGhostDocument;
class FrForm;
class WReceivedPacket;

#include "ghosttypes.h"
#include "commonutil.h"

inline const char* MakeGhostGameFileName(const GHOST_GAME_BRIEF* brief)
{
	return MakeStr("%d_%d_%d_%d_%d_%d.GST", brief->ownerUid, brief->id, brief->ball,
		brief->pang, brief->score, brief->recordId);
}

inline unsigned long GetGhostGameBriefFromFileName(GHOST_GAME_BRIEF* brief,
	const char* fileName)
{
	sscanf(fileName, "%d_%d_%d_%d_%d_%d.GST", &brief->ownerUid, &brief->id,
		&brief->ball, &brief->pang, &brief->score, &brief->recordId);
	return brief->ownerUid;
}

class CGhostRecorder : public IActor
{
public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; }

	CGhostRecorder();
	virtual ~CGhostRecorder() { }

	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnReload();
	virtual void OnProcess(float delta) { }
	virtual void OnDisplay() { }
	virtual void OnDestroy();

protected:
	virtual void HandleMsg(const MsgObject& msg);

private:
	bool IsSaveAvailable();

	CGhostDocument* m_pDocument;
};
#else
struct GHOST_GAME_BRIEF;
#endif

class CGhostHandler : public FrCmdTarget, public WSingleton<CGhostHandler>
{
public:
	CGhostHandler();
	virtual ~CGhostHandler();

	bool IsAvailableGhostSystem();
	void RequestGhostGameList(unsigned long uid);
	bool OpenEventGhostGameDlg();
	bool IsExistEventGhost();
	void OpenNotifyDlg(int type);
	void OnGhostPacket(WReceivedPacket& packet);
	void LoadingComplete(bool bSuccess);
	void UploadComplete(bool bSuccess, const std::string& fileName);
	void RecordComplete();
	void ExceptionFinishGame();
	void SendErrorReport(const std::string& error, const std::string& fileName);
	void ClearLocalGhostFiles();

private:
	void OpenGhostGameDlg(unsigned long uid, const std::string& nick,
		const std::list<GHOST_GAME_BRIEF>& gameList,
		bool (FrCmdTarget::*callback)(int, FrForm*));
	bool OnResultGhostGameDlg(int result, FrForm* form);
	bool OnResultEventGhostGameDlg(int result, FrForm* form);
#if !defined(REBANG_SEMANTIC_ONLY) && defined(REBANG_GHOSTHANDLER_CPP)
public:
	static void __cdecl
	RbOnghostpacketFunclets(); // HACK: EH funclets of the naked OnGhostPacket (nakedify)
#endif
};
