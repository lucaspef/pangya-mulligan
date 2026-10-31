#pragma once

#include <list>
#include "taskmanager.h"
#include "avatar_avatar.h"

class ntAvatarChatTask : public CTask
{
public:
	DECLARE_OBJECT(ntAvatarChatTask)

protected:
	ntAvatarChatTask();
	virtual ~ntAvatarChatTask();

public:
	void ShowQuitDialog();
	void AddAvatarBurden(const ntAvatarBurden& burden);
	void RemoveAvatarBurden(const ntAvatarBurden& burden);

protected:
	virtual void Init(const char* wallPaper);
	virtual void Register();
	virtual void Load();
	virtual void PreLoadInit();
	virtual void PreserveBack(const char* name);
	virtual void RestorePreserved();
	virtual void Destroy();
	virtual void Process(float delta);
	virtual void Display();
	virtual int OnPacket(WReceivedPacket& packet);
	virtual void OnAnotherHand(int key, void* pParam);

private:
	void DeleteBurdenPet(std::list<ntAvatarBurden>& dst,
		std::list<ntAvatarBurden>& src);
	void KeepUniqueCompleteList(const ntAvatarBurden& burden);

	std::list<ntAvatarBurden> m_completeList;
	std::list<ntAvatarBurden> m_addList;
	std::list<ntAvatarBurden> m_removeList;
	CRITICAL_SECTION m_cs;
	bool m_bQuit;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbPreloadinitFunclets(); // HACK: EH funclets of the naked PreLoadInit (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnpacketFunclets(); // HACK: EH funclets of the naked OnPacket (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbProcessFunclets(); // HACK: EH funclets of the naked Process (nakedify)
#endif
};
