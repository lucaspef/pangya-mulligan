#include "minatl.h"
#include "projectg.h"
#include "golfdoc.h"
#include "golfball.h"
#include "fxinterpreter.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "wresrcmng.h"
inline bool WView::InFrustumSafe(const Waabb& box)
{
	const WPlane* p = frustumSafe;
	for (int i = 0; i < 6; ++i, ++p)
	{
		WVector v(p->x > 0.0f ? box.max.x : box.min.x,
			p->y > 0.0f ? box.max.y : box.min.y,
			p->z > 0.0f ? box.max.z : box.min.z);
		if (*p * v >= 0.0f)
			return false;
	}
	return true;
}

#include "mascot.h"
#include "mathconsts.h"
#include "powergauge.h"
#include "chatmsg.h"

extern Fresh* g_pFresh;
extern WMatrix g_camera;
int float2int(float f);

IMPLEMENT_ACTOR(CMascot, IActor)

void CMascot::OnDestroy()
{
	m_pFxInterpreter983->DetachFxAll();
	m_birdFxId983 = -1;

	for (int i = 0; i < 4; i++)
	{
		if (m_mascot[i].pBird)
		{
			delete m_mascot[i].pBird;
			m_mascot[i].pBird = NULL;
		}
	}
}

void CMascot::OnPreLoadInit()
{
	for (int i = 0; i < 4; i++)
	{
		CSceneManager::Instance()->RegisterElement(OBJ_ACTOR, NULL, this,
			MakeStr("mascot%02d", i), 8, NULL);
	}
}

void CMascot::OnLoad()
{
	OnDestroy();

	for (unsigned char i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		m_mascot[i].pInfo = &Doc()->m_userInfo[i].mascotInfo;

		if (GOLFDOC()->GetPlayer(i)->state == 3)
			continue;

		if (m_mascot[i].pInfo->tid == 0)
			continue;

		IFF_STRUCT::sMascot* pMascot =
			ItemManager()->FindMascot(m_mascot[i].pInfo->tid);

		if (pMascot == NULL)
			continue;

		m_mascot[i].pIff = pMascot;
		m_mascot[i].pBird =
			new CCameraBird(MakeStr("%s.pet", pMascot->Data), i);
		m_mascot[i].pBird->SetMotion("Default", -1.0f);

		int texHandle = ChangeMascotMsgInTex(m_mascot[i].pInfo->szMsg, pMascot,
			m_mascot[i].pBird->GetPet(), MakeStr("_golf%02d", i));
		m_mascot[i].pBird->SetAddTex(texHandle);
	}
}

extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
int CMascot::ChangeMascotMsgInTex(const char* msg,
	const IFF_STRUCT::sMascot* pMascot, WPuppet* pPuppet, const char* suffix)
{
	if (msg == NULL || pMascot == NULL || pPuppet == NULL)
		return 0;

	if (pMascot->ShowMsg)
	{
		if (strrchr(pMascot->MsgTex, '.'))
		{
			char texName[256];
			strcpy(texName, pMascot->MsgTex);
			sprintf(strrchr(texName, '.'), "_%s%s", suffix,
				strrchr(pMascot->MsgTex, '.'));

			int hTex = g_resrcmng->LoadTexture(pMascot->MsgTex, 0, 0, texName);
			w_texlist* pTex = g_resrcmng->FindTexture(pMascot->MsgTex);

			if (hTex && pTex)
			{
				Bitmap* pBmp =
					g_resrcmng->LoadBitmap(pMascot->MsgTex, 0, false);
				RECT rc = { 0, 0, pBmp->Width(), pBmp->Height() };

				FrGraphicInterface* pGDI = NULL;
				jp983_rastro("D325 MsgTex msg=%s tid=%08x fresh=%p mgr=%p", msg, pMascot->c.TypeId, g_pFresh,
					g_pFresh ? g_pFresh->GetManager() : NULL);
				if (g_pFresh && g_pFresh->GetManager() && (pGDI = g_pFresh->GetManager()->GetGDI()) &&
					strlen(msg))
				{
					pGDI->SetTextStyle(1);
					pGDI->SetAlpha(1.0f);
					// 983 00761e70: o mascote 0x40000008 escreve em branco; os demais em preto
					if (pMascot->c.TypeId == 0x40000008)
						pGDI->SetTextColor(0xffffffff, 0xfff0f0f0);
					else
						pGDI->SetTextColor(0xff000000, 0xff808080);
					float y = float2int(pBmp->Height() * 0.5f) - 6.0f;
					float x = float2int(pBmp->Width() * 0.5f);
					pGDI->PrintText(WPoint(x, y), 1, msg, pBmp);
				}

				g_resrcmng->ChangeTexturePart(hTex, pBmp, rc);
				pPuppet->ChangeTextureByHandle(hTex, pTex->texhandle, NULL); // 983 00761e70: (nova, original)
				delete pBmp;
				return hTex;
			}

			if (g_resrcmng && hTex)
				g_resrcmng->Release(hTex);
		}
	}

	return 0;
}

void CMascot::OnReload()
{
	OnLoad();
}

void CMascot::OnInit()
{
}

void CMascot::OnProcess(float delta)
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		((CCameraBird*)0)->CCameraBird::CheckTarget(0.0f);
	for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		if (m_mascot[i].pBird)
			m_mascot[i].pBird->Process(delta);
	}
}

void CMascot::Display()
{
}

void CMascot::HandleMsg(const MsgObject& msg)
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		((CCameraBird*)0)->CCameraBird::CheckTarget(0.0f);
	switch (msg.message)
	{
	case M983(0x164):
		if (m_mascot[GOLFDOC()->m_currentPlayer].pBird)
			m_mascot[GOLFDOC()->m_currentPlayer].pBird->SetActive(false);
		break;

	case M983(0x163):
		if (m_mascot[GOLFDOC()->m_currentPlayer].pBird)
			m_mascot[GOLFDOC()->m_currentPlayer].pBird->SetActive(true);
		break;

	case M983(0x4e):
	{
		unsigned long guid = msg.param1;
		if (guid == MyGuid(true))
		{
			Doc()->m_userInfo[0].mascotInfo = *(sMascotInfo*)msg.param2;
			Doc()->m_myInfo.userEquip.guidMascot =
				Doc()->m_userInfo[0].mascotInfo.guid;
		}

		m_mascot[GOLFDOC()->m_currentPlayer].pInfo =
			&Doc()->m_userInfo[0].mascotInfo;

		IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(
			m_mascot[GOLFDOC()->m_currentPlayer].pInfo->tid);
		if (pMascot == NULL)
			break;

		CCameraBird* pBird = m_mascot[GOLFDOC()->m_currentPlayer].pBird;
		if (pBird == NULL)
			break;

		pBird->ChangePet(MakeStr("%s.pet", pMascot->Data));

		int texHandle = ChangeMascotMsgInTex(
			m_mascot[GOLFDOC()->m_currentPlayer].pInfo->szMsg, pMascot,
			m_mascot[GOLFDOC()->m_currentPlayer].pBird->GetPet(),
			MakeStr("_golf%02d", GOLFDOC()->m_currentPlayer));
		m_mascot[GOLFDOC()->m_currentPlayer].pBird->SetAddTex(texHandle);
	}
	break;

	case 0xc7: // 983: 00762fc3 (mensagem nova do 983)
		m_pFxInterpreter983->SetMode(msg.param1);
		break;

	case 0x307: // 983: 007631ce - area do mascote (5% pela tabela de rand do CPowerGauge)
	{
		unsigned long tid = Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].mascotInfo.tid;
		switch (tid)
		{
		case 0x40000007:
		case 0x40000014:
		case 0x40000015:
		case 0x40000016:
		case 0x40000017:
		case 0x4000001e:
		case 0x4000001f:
		case 0x40000020:
		case 0x40000021:
		case 0x40000023:
		case 0x4000002f:
		case 0x40000030:
		case 0x40000031:
		case 0x40000032:
		case 0x40000033:
			if (CPowerGauge::Instance()->GetNextRand983() % 100 < 5)
			{
				ShowLuckyMsg983(tid);
				CPowerGauge::Instance()->SetDebugAirNightLuckyPangYaArea(
					CPowerGauge::Instance()->GetMascotArea983(tid));
				return;
			}
		}
		CPowerGauge::Instance()->SetDebugAirNightLuckyPangYaArea(0.0f);
	}
	break;

	case 0x308: // 983: 00763162
		if (m_mascot[GOLFDOC()->m_currentPlayer].pBird)
		{
			m_pFxInterpreter983->DetachFx(m_birdFxId983);
			m_birdFxId983 = m_pFxInterpreter983->ParseBoundingBoxScript_fx(
				m_mascot[GOLFDOC()->m_currentPlayer].pBird->GetPet());
		}
		else
			m_pFxInterpreter983->DetachFxAll(-1);
		break;
	}
}

// 983: 00762c40 - fala do mascote quando a area aumenta (tid 0x40000007/0x40000023, pela sorte do buraco) + aviso no chat
void CMascot::ShowLuckyMsg983(unsigned long tid)
{
	if (tid == 0x40000007 || tid == 0x40000023)
	{
		std::string text;
		int hole = GOLFDOC()->m_currentHole - 1;
		if (hole < 0)
			hole = 0;
		else if (hole > 17)
			hole = 17;
		unsigned long rnd = Doc()->m_holeRandom[hole];

		if (tid == 0x40000007)
		{
			switch (rnd % 3)
			{
			case 1:
				text += "\xb9\xe6\xbf\xef\xb0\xed\xbe\xe7\xc0\xcc : \xb9\xe6\xbf\xef\xb0\xed\xbe\xe7\xc0\xcc \xc6\xce\xbe\xdf \xc0\xd3\xc6\xd1\xc6\xae\xc1\xb8\xc0\xcc \xc4\xbf\xc1\xb3\xb4\xd9\xbe\xdf\xbf\xcb!";
				break;
			case 2:
				text += "\xb9\xe6\xbf\xef\xb0\xed\xbe\xe7\xc0\xcc : \xb9\xe6\xbf\xef\xb0\xed\xbe\xe7\xc0\xcc \xc6\xce\xbe\xdf \xc0\xd3\xc6\xd1\xc6\xae\xc1\xb8\xc0\xcc \xb3\xd0\xbe\xee\xc1\xb3\xb4\xd9\xbe\xdf\xbf\xcb!";
				break;
			default:
				text += "\xb9\xe6\xbf\xef\xb0\xed\xbe\xe7\xc0\xcc : \xb9\xe6\xbf\xef\xb0\xed\xbe\xe7\xc0\xcc\xb0\xa1 \xc1\xd6\xb4\xc2 \xbc\xb1\xb9\xb0\xbe\xdf\xbf\xcb!";
				break;
			}
		}
		else if (tid == 0x40000023)
		{
			if ((rnd & 1) == 1)
				text += "\xc5\xe4\xb6\xf3\xc5\xb8\xb8\xb6 : \xc6\xce\xbe\xdf \xc0\xd3\xc6\xd1\xc6\xae\xc1\xb8\xc0\xcc \xc4\xbf\xc1\xb3\xb4\xd9\xbe\xdf\xbf\xcb!";
			else
				text += "\xc5\xe4\xb6\xf3\xc5\xb8\xb8\xb6 : \xb3\xbb\xb0\xa1 \xc1\xd6\xb4\xc2 \xbc\xb1\xb9\xb0\xbe\xdf\xbf\xcb!";
		}

		CChatMsg::Instance()->AddChatMsg(std::string(K2L_Compatibility(text.c_str())), 0xffffff00, false, false);
	}

	CChatMsg::Instance()->AddChatMsg(std::string(MakeStr(
		K2L_Compatibility("%s\xc0\xc7 \xb4\xc9\xb7\xc2\xc0\xb8\xb7\xce \xc6\xce\xbe\xdf \xc0\xd3\xc6\xd1\xc6\xae\xc1\xb8\xc0\xcc \xc4\xbf\xc1\xfd\xb4\xcf\xb4\xd9."),
		ItemManager()->FindMascot(tid)->c.Name)), 0xffff0000, false, false);
}

CCameraBird::CCameraBird(const char* name, unsigned char index)
{
	Init(name, 8);

	m_index = index;
	m_speed = 0.5f;
}

void CCameraBird::Process(float delta)
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		((CCameraBird*)0)->CCameraBird::CheckTarget(0.0f);
	unsigned char index = m_index;
	if (GOLFDOC()->GetPlayer(index)->state != 3 &&
		GOLFDOC()->m_currentPlayer == m_index &&
		!(GOLFDOC()->m_gameMode & 0x23001))
	{
		m_bActive = true;	// 983 00762930: flag em +0xa9 ligada so enquanto o passaro processa
		CCreature::Process(delta);
	}
	else
		m_bActive = false;
}

void CCameraBird::Display()
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		((CCameraBird*)0)->CCameraBird::CheckTarget(0.0f);
	if (!m_pPet || !m_bActive || m_motionTime < 0.0f)
		return;

	unsigned char index = m_index;
	if (GOLFDOC()->GetPlayer(index)->state == 3 ||
		GOLFDOC()->m_currentPlayer != m_index ||
		GOLFDOC()->m_gameMode & 0x22001)
		return;

	m_pPet->SetMatrix(Abs(m_angle) > 0.0f ? RotMat(m_angle, 2) * m_mat : m_mat);
	m_pPet->Animate983(NULL, NULL, m_motionTime, 0, NULL, NULL);
	m_pPet->Transform(g_view, 1.0f, 0);
	m_pPet->UpdateBSphere();

	if (!g_view->InFrustum(m_pPet->GetBoundSphere()))
		return;

	m_pPet->UpdateLightSource(&g_lightset, false, NULL);
	m_pPet->Render(g_view, true, 1.0f, g_view->InFrustumSafe(m_pPet->m_bound),
		false, false);
}

void CCameraBird::CheckTarget(float delta)
{
	if (GolfBall().m_playFrame >= 0 && GOLFDOC()->m_gameMode != 0x1000)
		return;

	AfxGetTask()->GetActor("Player")
		<< MsgObject(NULL, M983(0x124), m_index, (int)m_playerInfo, 0, 0, 0);

	WVector v = m_playerDir;
	v.y = 0.0f;
	v.Normalize();

	m_target = m_playerPos - v * 3.8400002f;
	m_target.y = GolfBall().m_pos.y + 3.8400002f;

	v = m_target - m_mat.pivot;
	float dist = v.Magnitude();
	float t = dist * 0.15625f > 1.0f ? 1.0f : dist * 0.15625f;

	WVector step = v * t;
	m_mat.za = step + (GolfBall().m_pos - m_mat.pivot) * (1.0f - t);
	m_mat.za.y = 0.0f;
	m_mat.za.Normalize();
	m_mat.za *= -1.0f;
	m_mat.ya = WVector::UNIT_POS_Y;
	const WVector& up = m_mat.ya;
	const WVector& at = m_mat.za;
	m_mat.xa = WCrossProduct(up, at);

	WVector pos = m_mat.pivot;
	m_mat.pivot = WVector::ZERO;
	m_mat = RotMat(-55.0f * g_DEGTORAD, 1) * m_mat;
	m_mat.pivot = pos;

	if (dist < 3.2f * 10.0)
	{
		if (dist > 0.5)
			m_mat.pivot += v * (dist * m_speed) * delta;
	}
	else
	{
		m_mat.za = g_camera.za * -1.0f;
		m_mat.xa = WCrossProduct(m_mat.ya, m_mat.za);
		m_mat.pivot = m_target;
	}
}

void CCameraBird::ChangePet(const char* name)
{
	if (g_resrcmng && m_pPet)
	{
		g_resrcmng->Release(m_pPet, false);
		m_pPet = NULL;
	}

	m_pPet = g_resrcmng->GetPuppet(name, false, true, false);
	CSceneManager::Instance()->SetPuppet(name, m_pPet);
}
