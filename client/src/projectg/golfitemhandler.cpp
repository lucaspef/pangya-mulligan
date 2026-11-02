#include "minatl.h"
#include "vc10_layout.h"
#include "golfitemhandler.h"
#include "golftask.h"
#include "noticedlg.h"
#include "messagemanager.h"
#include "chatmsg.h"
#include "fresh.h"
#include "gatewayactor.h"
#include "contentsdoc.h"
#include "gimmickcontainer.h"
#include "powergauge.h"
#include "quadtree.h"
#include "golfball.h"
#include "golfdoc.h"
#include "parttidlist.h"
static const float WARNING_RATE = 80.0f;

extern Fresh* g_pFresh;
unsigned char GetCurMap();

using namespace _gateway;

CGolfItemHandler::CGolfItemHandler()
	: m_value24_983(0)
{
}

CGolfItemHandler::~CGolfItemHandler()
{
}

bool CGolfItemHandler::Initialize()
{
	// JP 983 (0091a8c0): 26 sub-codigos, na ordem dos registros do exe (tools/f35/fmapreg.py ->
	// out/F3.5/s71/fmap_golfitem_983.tsv). 0-6 iguais ao 645; os 7 e 8 do 645 (lista de itens usaveis, time booster)
	// sairam; caixa especial 9-12 -> 7-0xa; o 13 (aviso do WizCity) saiu. Novos: 0xc-0xe (contador +0x2c do
	// documento), 0x10-0x12 (peca de conjunto e medidor) e 0x17-0x1f (s72).
	AddFunctionType(0, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CGolfItemHandler::OnRequestResetCalipers);
	AddFunctionType(1, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnGetFlagCalipers);
	AddFunctionType(2, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CGolfItemHandler::OnSetFlagCalipersEnable);
	AddFunctionType(3, FUNCTIONTYPE_VOID,
		(void (
			FunctionMapper::*)())&CGolfItemHandler::OnSetFlagCalipersDisable);
	AddFunctionType(5, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnSetCountCalipers);
	AddFunctionType(4, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnGetCountCalipers);
	AddFunctionType(6, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CGolfItemHandler::OnDecreaseCalipers);

	AddFunctionType(7, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CGolfItemHandler::OnIncSpecialBox);
	AddFunctionType(8, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CGolfItemHandler::OnResetSpecialBox);
	AddFunctionType(9, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnGetSpecialBox);
	AddFunctionType(0xa, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CGolfItemHandler::
			OnCheckSomeMoreGetSpecialBox);

	AddFunctionType(0xc, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnGetFlagItem2c983);
	AddFunctionType(0xd, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnSetCountItem2c983);
	AddFunctionType(0xe, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CGolfItemHandler::OnDecreaseItem2c983);
	AddFunctionType(0x10, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnSelectSetPart983);
	AddFunctionType(0x11, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnSetValue24_983);
	AddFunctionType(0x12, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnCheckGaugeFull983);
	AddFunctionType(0x17, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnRollPartTable17_983);
	AddFunctionType(0x18, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnNoticeUser18_983);
	AddFunctionType(0x1a, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnRollPartTable1a_983);
	AddFunctionType(0x1b, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnNoticeUser1b_983);
	AddFunctionType(0x1c, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnFindMyChar1c_983);
	AddFunctionType(0x1d, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnNoticeUser1d_983);
	AddFunctionType(0x1e, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnRollPartTable1e_983);
	AddFunctionType(0x1f, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnNoticeUser1f_983);
	AddFunctionType(0x19, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CGolfItemHandler::*)())&CGolfItemHandler::OnFireController19_983);

	return true;
}

bool CGolfItemHandler::Execute(const MsgObject& msg)
{
	SetFunctionType(msg.param1);
	return Exec(this, (int)&msg);
}

void CGolfItemHandler::OnRequestResetCalipers()
{
	CGolfItemDoc* pDoc = GetDocument<CGolfItemDoc*>();
	if (pDoc == NULL)
		return;

	pDoc->ResetCalipersVariable();
}

void CGolfItemHandler::OnGetFlagCalipers(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;

	CGolfItemDoc* pDoc = GetDocument<CGolfItemDoc*>();
	if (pDoc == NULL)
		return;

	*(bool*)pMsg->param2 = pDoc->GetEnableCalipers();
}

void CGolfItemHandler::OnSetFlagCalipersEnable()
{
	CGolfItemDoc* pDoc = GetDocument<CGolfItemDoc*>();
	if (pDoc == NULL)
		return;

	pDoc->EnableCalipers(true);
}

void CGolfItemHandler::OnSetFlagCalipersDisable()
{
	CGolfItemDoc* pDoc = GetDocument<CGolfItemDoc*>();
	if (pDoc == NULL)
		return;

	pDoc->EnableCalipers(false);
}

void CGolfItemHandler::OnGetCountCalipers(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;

	CGolfItemDoc* pDoc = GetDocument<CGolfItemDoc*>();
	if (pDoc == NULL)
		return;

	*(int*)pMsg->param2 = pDoc->GetCalipersCount();
}

void CGolfItemHandler::OnSetCountCalipers(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;

	CGolfItemDoc* pDoc = GetDocument<CGolfItemDoc*>();
	if (pDoc == NULL)
		return;

	pDoc->SetCalipersCount(pMsg->param2);
}

void CGolfItemHandler::OnDecreaseCalipers()
{
	CGolfItemDoc* pDoc = GetDocument<CGolfItemDoc*>();
	if (pDoc == NULL)
		return;

	pDoc->DeceaseCalipersCount();
}

void CGolfItemHandler::OnIncSpecialBox()
{
	CGolfItemDoc* pDoc = GetDocument<CGolfItemDoc*>();

	pDoc->IncreaseSpecialBoxCount();
}

void CGolfItemHandler::OnResetSpecialBox()
{
	CGolfItemDoc* pDoc = GetDocument<CGolfItemDoc*>();

	pDoc->ResetSpecialBoxCount();

	const std::list<sItemInfo>& itemList = Doc()->GetConstMyItemList();
	std::list<sItemInfo>::const_iterator it;
	for (it = itemList.begin(); it != itemList.end(); ++it)
	{
		if ((*it).tid == 0x1a00015b)
			break;
	}
	if (it == Doc()->GetConstMyItemList().end())

		pDoc->SetOwnSpecialBoxCount(0);
	else

		pDoc->SetOwnSpecialBoxCount((*it).Common[0]);
}

void CGolfItemHandler::OnGetSpecialBox(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;

	CGolfItemDoc* pDoc = GetDocument<CGolfItemDoc*>();

	*(int*)pMsg->param2 = pDoc->GetSpecialBoxCount();
}

bool _gateway::IsSpecialBoxArea()
{
	bool b = GetCurMap() == 19;
	return b;
}

void CGolfItemHandler::OnCheckSomeMoreGetSpecialBox()
{
	CGolfTask* pTask =
		(CGolfTask*)AfxGetTask()->DynamicCast(&CGolfTask::m_RTTI);
	if (pTask == NULL)
		return;

	if (!pTask->DetermineWhetherToAddActor_GroundItemMan())
		return;

	// 983 (00919f36): o byte +0x9c do CGimmickContainer libera a caixa em qualquer mapa
	CGimmickContainer* pGimmick = NULL;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0x9b, pGimmick);
	if (!pGimmick->m_bSpecialBoxAnyMap983 && GetCurMap() != 19)
		return;

	CGolfItemDoc* pDoc = GetDocument<CGolfItemDoc*>();

	int count = pDoc->GetOwnSpecialBoxCount() + pDoc->GetSpecialBoxCount();

	int remain = CalcRemainMoreGetSpecialBoxCount(count);
	IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(0x1a00015b);

	const char* str = MakeStr(_systemmsg::CMessageManager::Instance()->GetMsg(
								  (_systemmsg::eMsgIdentifier)300),
		pItem->Name, 100);
	CChatMsg::Instance()->AddChatMsg(str, 0xffff3030, true, false);

	if (remain < 1)
	{
		str = MakeStr(_systemmsg::CMessageManager::Instance()->GetMsg(
						  (_systemmsg::eMsgIdentifier)301),
			pItem->Name);
		CChatMsg::Instance()->AddChatMsg(str, 0xffff3030, true, false);
	}
	else
	{
		float rate = (float)(remain / 100.0 * 100.0); // 983 (0091a06f): limite 100
		if (WARNING_RATE <= rate)
		{
			str = MakeStr(_systemmsg::CMessageManager::Instance()->GetMsg(
							  (_systemmsg::eMsgIdentifier)302),
				pDoc->GetOwnSpecialBoxCount(), remain);
			CChatMsg::Instance()->AddChatMsg(str, 0xffff3030, true, false);
		}
	}
}

int CGolfItemHandler::CalcRemainMoreGetSpecialBoxCount(int count)
{
	return 100 - count; // 983 (00919740): 0x64
}

// JP 983: contador novo do documento (+0x2c) com o mesmo desenho do calibrador. O bit 14 de CSharedDoc+0x812
// (m_myInfo.info.dwIdentity) dispensa o contador. Qual item e: nao levantado (nomes provisorios).
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.info.dwIdentity) == 0x812);

void CGolfItemHandler::OnGetFlagItem2c983(int param) // 00919910
{
	bool* pResult = (bool*)((const MsgObject*)param)->param2;

	CGolfItemDoc* pDoc = GetDocument<CGolfItemDoc*>();
	if (pDoc == NULL)
		return;

	if ((Doc()->m_myInfo.info.dwIdentity >> 14) & 1)
		*pResult = true;
	else
		*pResult = pDoc->GetItemCount2c983() > 0;
}

void CGolfItemHandler::OnSetCountItem2c983(int param) // 009198b0
{
	const MsgObject* pMsg = (const MsgObject*)param;

	CGolfItemDoc* pDoc = GetDocument<CGolfItemDoc*>();
	if (pDoc == NULL)
		return;

	pDoc->SetItemCount2c983(pMsg->param2);
}

void CGolfItemHandler::OnDecreaseItem2c983() // 00919950
{
	CGolfItemDoc* pDoc = GetDocument<CGolfItemDoc*>();
	if (pDoc == NULL)
		return;

	if (!((Doc()->m_myInfo.info.dwIdentity >> 14) & 1))
		pDoc->DecreaseItemCount2c983();
}

void CGolfItemHandler::OnSetValue24_983(int param) // 00919750
{
	m_value24_983 = ((const MsgObject*)param)->param2;
}

// JP 983 (s72): sub-codigos 0x10, 0x12 e 0x17-0x1f. Nomes provisorios; nada rodou no jogo.
const char* __fastcall GetGameString(int key); // 00540a10 (unidade da frente B: include/jp/gamestringmanager.h)

// objeto global apontado por 00e73e84: por personagem, 10 registros de 0x10 bytes a partir de +0x200 (peca, tipo);
// 004d0bd0 devolve a 1a peca registrada do tipo pedido que o personagem veste (0 se nenhuma). Dono e nome da classe
// nao levantados (o trecho 004ce0d4-004d0e00 e "sem classe" no mapa da frente B).
#include "jp/createcharacteroption.h" // B124.1: o global de 00e73e84 e o CreateCharacterOption
extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // B1399 temporario

// 983: 00e2d464, 00e2d490 e 00e2d4bc, uma peca por personagem. Os tres testes do exe deixam passar indices alem do
// fim da tabela (ate 0xe) e leem a tabela seguinte do .rdata (personagem 0xb/0xc no 0x1a = 70010089/7021000a, que
// qualquer um veste). B1332: um vetor so, com os dwords do 983 de 00e2d464 a 00e2d4f4, para ler o mesmo dado.
static const unsigned long PART_TABLES_983[37] = {
	0x08008822, 0x08046835, 0x08088824, 0x080c8039, 0x08106026, 0x08148039, 0x08186035, 0x081c802a, // 00e2d464 (0x17)
	0x0820803f, 0x0824803c, 0x08288012,
	0x08008822, 0x08046835, 0x08088824, 0x080c8039, 0x08106026, 0x08148039, 0x08186035, 0x081c802a, // 00e2d490 (0x1a)
	0x0820803f, 0x0824803c, 0x08288012,
	0x70010089, 0x7021000a,                                                                         // 00e2d4bc (0x1e)
	0x00d61c1c, 0x00000000, 0x56413f2e, 0x6c6f4743, 0x65744966, 0x6e61486d, 0x72656c64, 0x61675f40, // 00e2d4c4 (o que segue)
	0x61776574, 0x00404079, 0x00cfe61c, 0x00cfe614, 0x00cfe5fc };
#define PART_TABLE_17 (PART_TABLES_983 + 0)
#define PART_TABLE_1A (PART_TABLES_983 + 11)
#define PART_TABLE_1E (PART_TABLES_983 + 22)

C_ASSERT_VC10(offsetof(CSharedDoc, m_pGolfDoc) == 0x11024);
C_ASSERT_VC10(offsetof(CGolfDoc, m_currentPlayer) == 0x189);
C_ASSERT_VC10(offsetof(CGolfDoc, m_currentHole) == 0x17c);
C_ASSERT_VC10(offsetof(CGolfDoc, m_gameMode) == 0x158);
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, charInfo) == 0x6802);
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, userEquip.guidChar) == 0x3b3a);
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, info.sNick) == 0x38c8);

void CGolfItemHandler::SetGaugeFull983() // 009198d0
{
	CPowerGauge::Instance()->SetValue40_983(1.0f);
}

bool CGolfItemHandler::IsSetPartGround983(int kind) // 00919970
{
	unsigned char ground = GetPVS().GetGroundType(GolfBall().m_pos, true, NULL);

	switch (kind)
	{
	case 0xa:
		switch (ground)
		{
		case 2:
			return true;
		}
		break;
	case 0x11:
		if ((Doc()->m_userInfo[Doc()->m_pGolfDoc->m_currentPlayer].charInfo.tid & 0x3ffffff) != 5)
			break;
	case 0xb:
	case 0xe:
		switch (ground)
		{
		case 0:
		case 2:
			return true;
		}
		break;
	}
	return false;
}

void CGolfItemHandler::OnSelectSetPart983(int param) // 0x10: 00919ab0
{
	int param2 = ((const MsgObject*)param)->param2;

	CPowerGauge::Instance()->SetValue40_983(0.0f);

	sCharacterInfo& charInfo = Doc()->m_userInfo[Doc()->m_pGolfDoc->m_currentPlayer].charInfo;
	m_value24_983 = CreateCharacterOption::Instance()->FindEquipped983(charInfo.tid, charInfo.tidParts, 0);
	if (m_value24_983 == 0)
		return;

	int kind = m_value24_983 & 0x1ff;
	if (!IsSetPartGround983(kind) && param2 == 0 && (Doc()->m_pGolfDoc->m_gameMode & 8) && kind == 0xa)
		SetGaugeFull983();
}

void CGolfItemHandler::OnCheckGaugeFull983(int param) // 0x12: 00919b50
{
	int value = ((const MsgObject*)param)->param2;
	if (value != 0 && (value & 0x1ff) == 0xa)
		SetGaugeFull983();
}

// 0x17, 0x1a e 0x1e: na vez do jogador local, com a peca da tabela vestida, liga *param2 uma vez em duas
#define ROLL_PART_TABLE_983(table, first) \
	if (Doc()->m_myInfo.info.dwUID != Doc()->m_userInfo[Doc()->m_pGolfDoc->m_currentPlayer].info.dwUID) \
		return; \
	sCharacterInfo* pChar = \
		Doc()->FindChar983(Doc()->m_userInfo[Doc()->m_pGolfDoc->m_currentPlayer].userEquip.guidChar); \
	if (pChar == NULL) \
		return; \
	int type = pChar->tid & 0x3ffffff; \
	if (type >= first && type >= 0xf) \
		return; \
	if (!Doc()->GetMyPartTidList().IsEquipped(table[type], 0)) \
		return; \
	if (rand() % 2 == 1) \
		*((const MsgObject*)param)->Param2<bool*>() = true;

void CGolfItemHandler::OnRollPartTable17_983(int param) // 00919b70
{
	ROLL_PART_TABLE_983(PART_TABLE_17, 0xb)
}

void CGolfItemHandler::OnRollPartTable1a_983(int param) // 00919c10
{
	ROLL_PART_TABLE_983(PART_TABLE_1A, 0xb)
}

void CGolfItemHandler::OnRollPartTable1e_983(int param) // 00919cb0
{
	ROLL_PART_TABLE_983(PART_TABLE_1E, 2)
}

void CGolfItemHandler::OnFindMyChar1c_983(int param) // 0x1c: 00919d50 (o 983 acha o personagem e nao usa)
{
	if (Doc()->m_myInfo.info.dwUID != Doc()->m_userInfo[Doc()->m_pGolfDoc->m_currentPlayer].info.dwUID)
		return;
	Doc()->FindChar983(Doc()->m_userInfo[Doc()->m_pGolfDoc->m_currentPlayer].userEquip.guidChar);
}

void CGolfItemHandler::OnFireController19_983(int param) // 0x19: 00919d90
{
	const MsgObject* pMsg = (const MsgObject*)param;
	unsigned long uid = pMsg->Param2<unsigned long>();
	// param3 = 2o Decode4 do pacote (CGolfTask::OnPacket, 006a450c); o tipo exato do leitor 009198f0 nao e conhecido
	unsigned int value = pMsg->Param3<unsigned int>();
	if (value == 0 || uid == (unsigned long)-1)
		return;

	unsigned char player = Doc()->FindPlayerByUid983(uid);
	jp983_rastro("B1399 0x19 uid=%lu player=%d my=%lu", uid, player, Doc()->m_myInfo.info.dwUID); // temporario
	if (player == 0xff)
		return;

	// 0x753b = mensagem do controlador de itens (frente B); 0x12 para o jogador local, 0x11 para os outros
	if (Doc()->m_myInfo.info.dwUID != uid)
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753b, 0x11, player, 0xe, (int)value, 0);
	else
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, 0x753b, 0x12, player, 0xe, (int)value, 0);
}

// 0x18, 0x1b, 0x1d e 0x1f: manda a mensagem a tela e escreve no chat o texto do id com o apelido do jogador
#define NOTICE_USER_983(screenMsg, stringId, color) \
	AfxGetTask()->GetActor("Screen") << MsgObject(NULL, screenMsg, 0, 0, 0, 0, 0); \
	unsigned long uid = ((const MsgObject*)param)->Param2<unsigned long>(); \
	int player = 0; \
	for (int i = 0; i < 4; i++) \
	{ \
		if (Doc()->m_userInfo[i].info.dwUID == uid) \
			player = i; \
	} \
	std::string nick = Doc()->m_userInfo[player].info.sNick; \
	if (!nick.empty()) \
	{ \
		std::string text = MakeStr(GetGameString(stringId), nick.c_str()); \
		CChatMsg::Instance()->AddChatMsg(text, color, false, false); \
	}

void CGolfItemHandler::OnNoticeUser18_983(int param) // 0091a100
{
	NOTICE_USER_983(0x334, 0x9c44, 0xffff7878)
}

void CGolfItemHandler::OnNoticeUser1b_983(int param) // 0091a2a0
{
	NOTICE_USER_983(0x35c, 0x9c45, 0xffff7878)
}

void CGolfItemHandler::OnNoticeUser1d_983(int param) // 0091a5e0
{
	NOTICE_USER_983(0x35d, 0x9c46, 0xffff4646)
}

void CGolfItemHandler::OnNoticeUser1f_983(int param) // 0091a440
{
	NOTICE_USER_983(0x35f, 0x9c49, 0xffff7878)
}
