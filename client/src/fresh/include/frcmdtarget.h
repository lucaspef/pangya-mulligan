#pragma once

enum eFrCmd
{
	FRCMD_NONE,
	FRCMD_INIT,
	FRCMD_MOUSEMOVE,
	FRCMD_LBUTTONDOWN,
	FRCMD_LBUTTONUP,
	FRCMD_RBUTTONDOWN,
	FRCMD_RBUTTONUP,
	FRCMD_DBLCLICK,
	FRCMD_OWNERDRAW,
	FRCMD_ENTERKEY,
	FRCMD_LOSTKEYFOCUS,
	FRCMD_HOVERON,
	FRCMD_HOVEROFF,
	FRCMD_FINISH,
	FRCMD_DESTROY
};

enum eFrFuncType
{
	FrFuncNone,
	FrFuncVV,
	FrFuncBV,
	FrFuncBI,
	FrFuncVI,
	FrFuncBI2,
	FrFuncBI3
};

class FrCmdTarget;
typedef void (FrCmdTarget::*FRESH_PFN)();
typedef bool (FrCmdTarget::*FRESH_PFN_BV)();
typedef void (FrCmdTarget::*FRESH_PFN_VI)(int);
typedef bool (FrCmdTarget::*FRESH_PFN_BI)(int);

class FrForm;
typedef bool (FrCmdTarget::*FRESH_PFN_RESULT)(int, FrForm*);

union uFreshFunctions
{
	FRESH_PFN pFn;
	FRESH_PFN pFn_FRESH;
	FRESH_PFN pFn_FRESH_VV;
	FRESH_PFN_BV pFn_FRESH_BV;
	FRESH_PFN_VI pFn_FRESH_VI;
	FRESH_PFN_BI pFn_FRESH_BI;
};

struct sFRESH_ENTRY
{
	FRESH_PFN pFn;
	char* pName;
	eFrCmd cmd;
	eFrFuncType eFt;
};

struct sFRESH_HANDLER
{
	FrCmdTarget* pFresh;
	eFrFuncType eFt;
	FRESH_PFN pFn;
};

struct sFRESH_MSGMAP
{
	sFRESH_MSGMAP* pBaseMsgMap;
	sFRESH_ENTRY* pEntry;
};

class FrCmdTarget
{
public:
	FrCmdTarget();
	virtual ~FrCmdTarget();
	bool OnFreshMsg(const char* pName, eFrCmd cmd, int var1,
		sFRESH_HANDLER* pHandler);

protected:
	virtual const sFRESH_MSGMAP* GetMessageMap() const;
	sFRESH_ENTRY* FindFreshEntry(sFRESH_ENTRY* pBegin, const char* pName,
		eFrCmd cmd);

	static sFRESH_MSGMAP _MsgMap;
	static sFRESH_ENTRY _MsgEntries[];
};

#define DECLARE_FRESH_MSGMAP() \
private: \
	static sFRESH_ENTRY _MsgEntries[]; \
\
protected: \
	static sFRESH_MSGMAP _MsgMap; \
	virtual const sFRESH_MSGMAP* GetMessageMap() const;

#define BEGIN_FRESH_MSGMAP(thisClass, baseClass) \
	const sFRESH_MSGMAP* thisClass::GetMessageMap() const \
	{ \
		return &thisClass::_MsgMap; \
	} \
	sFRESH_MSGMAP thisClass::_MsgMap = { &baseClass::_MsgMap, \
		&thisClass::_MsgEntries[0] }; \
	sFRESH_ENTRY thisClass::_MsgEntries[] = {
#define END_FRESH_MSGMAP() \
	{ 0, 0, FRCMD_NONE, FrFuncNone } \
	} \
	;

#define ON_FRESH_VV(name, cmd, memberFxn) \
	{ (FRESH_PFN) & memberFxn, name, cmd, FrFuncVV },

#define ON_FRESH_VI(name, cmd, memberFxn) \
	{ (FRESH_PFN)(FRESH_PFN_VI) & memberFxn, name, cmd, FrFuncVI },

#define ON_FRESH_BV(name, cmd, memberFxn) \
	{ (FRESH_PFN)(FRESH_PFN_BV) & memberFxn, name, cmd, FrFuncBV },

#define ON_FRESH_BI(name, cmd, memberFxn) \
	{ (FRESH_PFN)(FRESH_PFN_BI) & memberFxn, name, cmd, FrFuncBI },
