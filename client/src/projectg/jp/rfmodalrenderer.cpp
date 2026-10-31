// JP 983: unidade 005e51f0–005e5c20 — RFModalRenderer, a pilha das janelas modais da RF e o escurecimento da tela atrás da modal
// de cima. O resto da faixa são os moldes da std::list<RFModalRenderer*> (alocação, _Buynode, clear, destrutor da lista).
#include "refresh/rfmodalrenderer.h"
#include "refresh/rfscenemanager.h"
#include "refreshrenderer.h"
#include <algorithm>

extern const float g_fRFScreenW983; // 00d05628
extern const float g_fRFScreenH983; // 00d0562c

std::list<RFModalRenderer*> RFModalRenderer::s_modals983;

// 005e51f0
RFModalRenderer::RFModalRenderer()
{
}

// 005e5200
RFModalRenderer::~RFModalRenderer()
{
}

// 005e58f0
void RFModalRenderer::Draw983(float alpha)
{
	if (s_modals983.size() != 0 && RFSceneManager::Instance() && this == s_modals983.back())
	{
		RFSize size;
		RFManager* pManager = RFSceneManager::Instance()->m_pManager;
		if (pManager)
			size = pManager->GetRoot983()->GetSize();
		else
			size = RFSize(g_fRFScreenH983, g_fRFScreenW983);
		float count = s_modals983.size() < 7 ? (float)s_modals983.size() : 6.0f;
		if (s_modals983.size() > 1)
			alpha = 1.0f;
		DrawBoxAt983(WPoint(0.0f, 0.0f), WSize(size.width, size.height), (int)((count * 10.0 + 100.0) * alpha) << 24);
	}
}

// 005e5ab0
void RFModalRenderer::Remove983()
{
	std::list<RFModalRenderer*>::iterator it = std::find(s_modals983.begin(), s_modals983.end(), this);
	if (it != s_modals983.end())
		s_modals983.erase(it);
}

// 005e5b70
void RFModalRenderer::Push983()
{
	s_modals983.push_back(this);
}
