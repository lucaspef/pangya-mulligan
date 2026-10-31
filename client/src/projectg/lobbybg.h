#pragma once

#include <string>
#include <memory>
#include "actor.h"

extern int LOBBYBG_MAP;
extern char* LOBBYBG_MAPNAME;

class WPuppet;

// JP 983: CameraMovingHelper (nome do RTTI do _Ref_count<CameraMovingHelper::sCameraMoving>; unidade propria no 983,
// 00485320-00486700, sem par no 645). Leva a camera (g_camera) de um ponto a outro com uma curva de Penner por eixo;
// usada pelo ntLobbyBg (msg 0x17f) e pela 0068596b. Aqui junto do ntLobbyBg, o unico usuario no caminho do lobby.
class CameraMovingHelper
{
public:
	struct sCameraMoving // 0x4c (new em 004864b0)
	{
		WVector from;     // +0x00
		WVector to;       // +0x0c
		WVector duration; // +0x18 (segundos)
		WVector cur;      // +0x24
		WVector elapsed;  // +0x30
		int ease[3];      // +0x3c: tipo da curva (0..0x15, 00485360)
		bool active[3];   // +0x48

		sCameraMoving() { Reset(); }                            // 00485a90
		void Reset();                                           // 00485890
		bool IsDone() const;                                    // 00485730
		void SetFromTo(const WVector& f, const WVector& t);     // 00485860
		void SetEase(int x, int y, int z);                      // 00485320
		void SetDuration(float d);                              // 00485340
		void Start();                                           // 00485930
		void Update(const float& dt);                           // 00485ab0
	};

	CameraMovingHelper();                                       // 004864b0
	bool SetFromTo(const WMatrix* from, const WMatrix* to);     // 00485c90
	void SetEase(int x, int y, int z);                          // 00485cf0 (00485d50 repassa)
	void SetDuration(float d);                                  // 00485d60
	void Start();                                               // 00485c80: o 983 so liga o pivo (m_track[3])
	void Update(const float& dt);                               // 00486090: aplica em g_camera
	bool IsDone() const;                                        // 00485e80
	void Stop();                                                // 00485ed0

	std::tr1::shared_ptr<sCameraMoving> m_xa;    // +0x00 (o 983 inicia/usa os 4 por nome, sem laco)
	std::tr1::shared_ptr<sCameraMoving> m_ya;    // +0x08
	std::tr1::shared_ptr<sCameraMoving> m_za;    // +0x10
	std::tr1::shared_ptr<sCameraMoving> m_pivot; // +0x18 (destrutor 004861f0)

	void ApplyXa();    // 00485dc0: g_camera.xa = m_xa->cur
	void ApplyYa();    // 00485df0
	void ApplyZa();    // 00485e20
	void ApplyPivot(); // 00485e50
};

class ntLobbyBg : public IActor
{
public:
	DECLARE_OBJECT(ntLobbyBg)

	virtual void OnPreLoadInit();
	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnProcess(float delta);
	virtual void OnDestroy();
	virtual void HandleMsg(const MsgObject& msg);

protected:
	ntLobbyBg();

	void DoReLoad();                              // 00ab1b70
	void SetSkyColor(unsigned long color);
	void LoadMapData983();                        // 00ab1cb0: onda/rio/ceu do mapa m_mapName983
	void SetPoster983(int slot, unsigned long id); // 00ab1bf0: textura do cartaz (mobilia "%s.jpg")

	// 983 (ctor 00ab21d0, new 0x70 em 00ab2410)
	int m_mapNum983;                              // +0x18: estrelas (msg 0x17a param1)
	std::string m_mapName983;                     // +0x1c: "blue"; msg 0x17a param2 ("west" pelo CLobbyMain)
	std::tr1::shared_ptr<CameraMovingHelper> m_pCamHelper983; // +0x38
	std::string m_curCamera983;                   // +0x40: ultima camera pedida (msg 0x17f)
	bool m_bCamMoving983;                         // +0x5c: avisa 0x180 ao terminar
	bool m_bLoaded983;                            // +0x5d
	unsigned long m_posterId983[2];               // +0x60
	WPuppet* m_pPoster983[2];                     // +0x68: "lobby_post01.pet" / "lobby_post02.pet"
};
