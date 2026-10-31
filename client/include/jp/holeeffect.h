#pragma once
#include <list>
#include "holeinonescoreinfo.h"

class WOverlay;
namespace lua_system
{
	class lua_script;
	class lua_table_helper;
}

// JP 983: animação do hole-in-one na tela de jogo ("holeineffect.lua": raio, sol, anel, ponto, branco e mini, com o "+N" da
// fonte "font_holein_font.png"). vtable 00d0ab14, sizeof 0x1d4 (new em 00752061 no CScreen::OnLoad, guardada em CScreen+0x25c;
// CScreen::HandleMsg chama Start983 e AddCount983). Classe nova do 983 (existe no 839, não no 645). Unidade 0056efc0–00570660.
// Nomes dos campos e dos métodos sem símbolo são meus.
class CHoleEffect : public CFontInfo
{
public:
	// uma figura (m_effects) ou um evento da cena (m_events, "sceneevent"): o evento sobrescreve a figura `index` na hora `start`
	struct sItem983
	{
		int index; // +0x00 (só no evento)
		float fromX; // +0x04 (-1 no evento = manter)
		float fromY; // +0x08
		float toX; // +0x0c
		float toY; // +0x10
		float x; // +0x14
		float y; // +0x18
		float scale; // +0x1c
		float rot; // +0x20
		float maxRot; // +0x24
		float start; // +0x28
		float duration; // +0x2c
		float t; // +0x30 0…1 do deslocamento from → to
		float alpha; // +0x34 0…255
		unsigned long flags; // +0x38 1 aparece, 2 some, 4 desloca, 8 gira, 0x20 encolhe
	};

	CHoleEffect(); // 00570520
	virtual ~CHoleEffect(); // 0056ffe0

	virtual void Process(float dt); // 0056fcd0
	virtual void Display(); // 0056f400
	virtual void Reset(); // 00570330

	void Start983(); // 0056f540 (CScreen::HandleMsg)
	void AddCount983(int count); // 0056f080 (CScreen::HandleMsg)

private:
	void ApplyEvent983(const sItem983& ev); // 0056efc0
	void DrawItem983(WOverlay* overlay, const sItem983& item); // 0056f260
	void LoadOverlays983(); // 0056fa80
	void ProcessEvents983(float dt); // 0056fc60 (dt sem uso)

	WOverlay* m_overlays[6]; // +0x2c ray, sun, ring, point (não carregada), white, mini
	sItem983 m_effects[6]; // +0x44
	float m_duration; // +0x1ac "sceneduration"
	float m_time; // +0x1b0
	int m_fadeVel; // +0x1b4 "fadevel"
	float m_rotVel; // +0x1b8 "rotvel"
	int m_count; // +0x1bc
	int m_bPlaying; // +0x1c0
	std::list<sItem983> m_events; // +0x1c4
	lua_system::lua_script* m_pScript; // +0x1d0
};
