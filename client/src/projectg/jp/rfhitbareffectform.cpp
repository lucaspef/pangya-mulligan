// JP 983: unidade 005d0f00–005d1580 — RFHitBarEffectForm, o efeito "_BestShot_" da barra de impacto ("hitbar_impact_effect.xml").
// Nada no 983 a cria (Create983/Play983 sem chamadores). Sessão B137. Nomes meus (…983), menos a classe e os da RF.
#include "stdafx.h"
#include "refresh/rfhitbareffectform.h"
#include "shareddoc.h"
#include "golfdoc.h"

extern int g_rfCreateMode983; // 00e73e48: 0 = filha do pai, 1 = sem pai, 2 = área de trabalho

RFHitBarEffectForm* RFHitBarEffectForm::ms_pInstance;
RFWnd* RFHitBarEffectForm::s_pParent983;

// 005d0f00
void RFHitBarEffectForm::SetInstance983()
{
	ms_pInstance = this;
}

// 005d0f10
void RFHitBarEffectForm::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005d0f20
RFHitBarEffectForm::RFHitBarEffectForm()
{
	ms_pInstance = this;
}

// 005d0f40
RFHitBarEffectForm::~RFHitBarEffectForm()
{
	ms_pInstance = NULL;
}

// 005d11d0
void RFHitBarEffectForm::OnUpdate983(EventArgs& e)
{
	if (!IsAnimationPlaying())
		SetVisible983(false);
}

// 005d11f0
RFHitBarEffectForm* RFHitBarEffectForm::Create983(RFWnd* parent)
{
	RFHitBarEffectForm* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFHitBarEffectForm>(L"hitbar_impact_effect.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFHitBarEffectForm>(L"hitbar_impact_effect.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFHitBarEffectForm>(L"hitbar_impact_effect.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005d1330
void RFHitBarEffectForm::OnInitialUpdate()
{
	RegisterTo(L"", RFWnd::EventUpdate, Subscriber(&RFHitBarEffectForm::OnUpdate983, this));
	SetVisible983(false);
	SetScreenPos(300.0f, 300.0f);
}

// 005d1460
void RFHitBarEffectForm::Play983()
{
	bool show = GOLFDOC()->m_bar.grade > 1;
	SetVisible983(show ? true : false);
	if (show)
	{
		switch (GOLFDOC()->m_bar.grade)
		{
		case 4:
			PlayAnimation(L"_BestShot_");
			break;
		case 3:
			PlayAnimation(L"_BestShot_");
			break;
		case 2:
			PlayAnimation(L"_BestShot_");
			break;
		}
	}
}
