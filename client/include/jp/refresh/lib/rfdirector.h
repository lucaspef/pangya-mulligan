#pragma once
#include <map>
#include <vector>
#include "refresh/lib/rfeventhandler.h"
#include "refresh/lib/rftypes.h"
#include "refresh/lib/rfrenderer.h"

// JP 983: RFDirector.h, RFAnimation.h, RFAnimationObject.h e RFAnimFrame.h da Refresh (B157, F3.7.13 item (6)).
// RFDirector = diretor de animação de uma janela/figura (vtable 00d62fac): cenas (RFAnimation, 0x8c, não exportada) lidas de um
// <rfanimation version="1.2"> com <scene name loop pingpong><param type="RES|LOOP" …/><object name ani><asset link/></object></scene>.
// Cada objeto (RFAnimationObject, 0xa8, vtable 00d630f8) tem uma trilha de quadros (RFAnimFrame) e uma textura.
// (o RFDirector.cpp de verdade fica em 004307f0–00431bd0; o RFAnimation.cpp em 0045ba40–0045d464)
class RFWnd;
class RFWindowModifier;
class RFTexture;
class RFRenderer;
class RFInterface;
class RFAnimation;
class RFAnimationObject;
class RFDirector;
namespace rf { class TiXmlNode; class TiXmlElement; }

// (o <assert.h> só declara o _wassert sem NDEBUG; a Refresh foi compilada sem NDEBUG)
extern "C" __declspec(dllimport) void __cdecl _wassert(const wchar_t* message, const wchar_t* file, unsigned line);

// valor animado de um quadro (8 bytes: valor + interpolação até ele; nomes meus). O ponteiro é dono só nos quadros da trilha
// (RFAnimFrame::_frames, apagados no ~RFAnimFrame); as cópias (de/para/agora) só o emprestam.
struct RFFloatAnimationFrame
{
	RFFloatAnimationFrame() : value(0.0f), interpolation(0) { }

	float value; // +0
	RFInterpolation* interpolation; // +4 (RFInterpolation linear ou RFBezierSpline do easeValue)
};

// quadro da trilha (100 bytes: RFAnimFrame divide o tamanho do vetor por 100). Clear 0042af20 (fora de linha); a cópia 00403db0.
struct RFAnimationFrame
{
	RFAnimationFrame() { Clear(); }
	void Clear(); // 0042af20 (tempo 0, visível, sem som, escala 1, centro 0,5, alfa 1)

	unsigned int time; // +0x00 (ms)
	bool visible; // +0x04
	std::wstring sfx; // +0x08
	RFFloatAnimationFrame x; // +0x24 (posição relativa ao tamanho da cena)
	RFFloatAnimationFrame y; // +0x2c
	RFFloatAnimationFrame scaleX; // +0x34
	RFFloatAnimationFrame scaleY; // +0x3c
	RFFloatAnimationFrame centerX; // +0x44 (0,5)
	RFFloatAnimationFrame centerY; // +0x4c (0,5)
	RFFloatAnimationFrame rotation; // +0x54
	RFFloatAnimationFrame alpha; // +0x5c
};

// curva de Bézier cúbica de (0,0) a (1,1) com dois pontos de controle (easeValue="x1,y1,x2,y2"); GetValue(t) procura o ponto
// pré-calculado (RFBezierSpline.cpp, 0042ee70–0042f540; exportada; vtable 00d630e4; as cópias são embutidas: 004038c0/00403950)
class RF_API RFBezierSpline : public RFInterpolation
{
public:
	RFBezierSpline(int resolution, const RFVector2& control1, const RFVector2& control2); // 0042ee70
	virtual ~RFBezierSpline(); // 0042ef20
	virtual float GetValue(float t) const; // 0042ef80
private:
	void GenerateBezierPoints(const RFVector2& p0, const RFVector2& p1, const RFVector2& p2, const RFVector2& p3); // 0042f050
	RFVector2 GetBezierPoint(float t, const RFVector2& p0, const RFVector2& p1, const RFVector2& p2, const RFVector2& p3) const; // 0042f1e0

	RFVector2 _p0; // +0x04 (0, 0)
	RFVector2 _p1; // +0x0c
	RFVector2 _p2; // +0x14
	RFVector2 _p3; // +0x1c (1, 1)
	int _resolution; // +0x24 (1024)
	std::vector<RFVector2> _points; // +0x28 (x = t, y = valor; crescente em x)
};

// trilha de quadros de um objeto (RFAnimFrame.cpp, 0042d440–0042ee70; exportada; 0x19c). De/para = par de quadros da trilha em
// volta do tempo atual; agora = interpolado; a matriz é a do RFRenderer::Draw.
class RF_API RFAnimFrame
{
public:
	RFAnimFrame(); // 0042d440
	virtual ~RFAnimFrame(); // 0042d570

	void SetCanvas(const RFRect& canvas); // 0042d810
	void AddFrame(const RFAnimationFrame& frame); // 0042d840
	bool InsertFrame(const RFAnimationFrame& frame); // 0042d860
	void GetNowFrame(RFAnimationFrame* frame) const; // 0042d8e0
	void Stop(); // 0042d900
	void End(const RFRect& canvas); // 0042d930
	void Play(); // 0042d990
	void PlayPendulum(); // 0042da40
	bool IsPlaying() const; // 0042db30
	bool IsIdle() const; // 0042db40
	void FrameMovePendulum(unsigned int time, const RFRect& canvas); // 0042db50
	void FrameMove(unsigned int time, const RFRect& canvas); // 0042dd80
	void GenAnimMatrix(const RFRect& canvas); // 0042dfb0
	void Render(RFTexture* texture, float alpha); // 0042e2e0
	const RFMatrix& GetAnimMatrix() const { return _matrix; } // 00403a70
protected:
	void PlaySfx(const std::wstring& name); // 0042d740
public:
	std::vector<RFAnimationFrame> _frames; // +0x04
	RFAnimationFrame _from983; // +0x14
	RFAnimationFrame _to983; // +0x78
	RFAnimationFrame _now; // +0xdc (quadro interpolado; o RFDirector o copia para a janela)
	int _index; // +0x140
	bool _playing; // +0x144
	bool _hold983; // +0x145 (pêndulo passou do fim com o quadro visível: desenha mesmo com _now invisível)
	bool _idle983; // +0x146 (antes do 1º quadro: não desenha)
	bool _first983; // +0x147 (o 1º FrameMove só assume o quadro "de")
	bool _single983; // +0x148 (trilha de um quadro)
	RFRect _canvas983; // +0x14c (RFAnimationObject::Load: 0..largura × 0..altura da cena)
	RFMatrix _matrix; // +0x15c
};

// evento "Draw" de cada objeto antes de desenhar (não exportada; vtable só com o destrutor; nomes meus)
class AnimDrawEventArgs : public EventArgs
{
public:
	RFAnimationObject* object; // +0x08
	float alpha; // +0x0c
};

// objeto de uma cena (o construtor, name/asset e as cópias são embutidos no .h: 00403e30–004041a0)
class RF_API RFAnimationObject : public RFEventHandler
{
public:
	RFAnimationObject(RFAnimation* animation) // 00403e30
	{
		_type983 = 0;
		_rect.left = 0.0f;
		_rect.right = 0.0f;
		_rect.top = 0.0f;
		_rect.bottom = 0.0f;
		_animation = animation;
		_endTime983 = 0.0f;
		_frame = 0;
		_texture = 0;
	}
	virtual ~RFAnimationObject(); // 0042afc0

	const std::wstring& name() const { return _name; } // 00403ef0
	const std::wstring& asset() const { return _asset; } // 00403f00

	void Load(const rf::TiXmlNode* scene); // 0042b0f0 (corpo do B158)
	void Render(float alpha); // 0042c7d0
private:
	void GetXmlValue(const rf::TiXmlNode* node, const std::string& name, RFFloatAnimationFrame* value); // 0042c890 (B158)
public:
	static const std::wstring EventDraw983; // 00e47118 ("Draw"; nome meu)

	std::wstring _name; // +0x30 (<object name>: janela filha que o RFDirector::FrameMove(RFWnd*) move)
	int _type983; // +0x4c (desenha só com 0 ou 2)
	std::wstring _ani983; // +0x50 (<object ani>)
	std::wstring _asset; // +0x6c (<asset link>)
	RFRect _rect; // +0x88 (retângulo do objeto na cena)
	float _endTime983; // +0x98 (fim da trilha em ms; a cena guarda o maior)
	RFAnimFrame* _frame; // +0x9c
	RFAnimation* _animation; // +0xa0
	RFTexture* _texture; // +0xa4 (liberada pelo RFRenderer do RFManager)
};

// cena (RFAnimation.cpp, não exportada; vtable 00d65f90; os métodos têm o this em registrador pelo LTCG; nomes meus)
class RFAnimation : public RFEventHandler
{
public:
	RFAnimation(RFDirector* director); // 0045ba40
	virtual ~RFAnimation(); // 0045bb20

	void Load983(const rf::TiXmlNode* scene); // 0045c250
	void LoadParams983(const rf::TiXmlElement* scene); // 0045bf70 (<param type="RES" x y> / type="LOOP" cnt pendulum)
	void SetPosition983(const RFVector2& pos); // 0045bbf0 (o RFDirector::SetPosition repete o corpo)
	RFAnimationObject& GetObject983(unsigned int index) // embutida (src\RFAnimation.cpp 90)
	{
		if (index >= _objects.size())
			_wassert(L"!\"Parameter 'index' out of range.\"", L"src\\RFAnimation.cpp", 90);
		return _objects[index];
	}
	void Play983(); // 0045ca80
	void Stop983(); // 0045cb40 (avisa EventAnimFinished pelo diretor)
	void End983(); // 0045cca0 (pula para o último quadro)
	void FrameMove983(float delta); // 0045cda0
	void Draw983(float alpha); // 0045cfb0

	std::wstring _name; // +0x30
	unsigned int _version; // +0x4c (do diretor)
	RFDirector* _director; // +0x50
	float _time; // +0x54 (segundos; diminui no pêndulo de volta)
	unsigned int _finished; // +0x58 (objetos que terminaram neste quadro)
	bool _playing983; // +0x5c
	bool _repeat983; // +0x5d (loop="0" ou LOOP cnt=-1)
	bool _pingpong983; // +0x5e
	int _reverse983; // +0x60 (pêndulo voltando)
	int _64;
	RFSize _size; // +0x68 (<param type="RES">; padrão 1 × 1)
	float _startTime983; // +0x70 (ms)
	float _endTime983; // +0x74 (ms; maior _endTime983 dos objetos)
	float _progress983; // +0x78 (0..1)
	std::vector<RFAnimationObject> _objects; // +0x7c
};

class RFDirector : public RFEventHandler
{
public:
	RF_API RFDirector(); // 004307f0
	RF_API virtual ~RFDirector(); // 00430910
	RF_API RFDirector(const RFDirector& other); // 00404230 (cópia fora de linha; B268)
	RF_API RFDirector& operator=(const RFDirector& other); // 00404330

	RF_API unsigned int GetVersion() const { return _version; } // 00404200
	RF_API RFRenderer* GetRenderer() { return _renderer; } // 00404210
	RF_API RFInterface* GetInterface() { return _interface; } // 00404220

	RF_API void SendFinishEvent(RFAnimation* animation); // 00430a10
	RF_API void UnloadAllAnimation(); // 00430ad0
	RF_API R_ERRORTYPE LoadAnimation(const std::wstring& name); // 00430b50
	RF_API R_ERRORTYPE LoadAnimationInDirect(const std::wstring& path); // 00430c20
	RF_API void SetPosition(const RFVector2& pos); // 00430fa0
	RF_API int GetAnimationCount() const; // 00431140
	RF_API RFAnimation* GetAnimation(int index) const; // 00431150
	RF_API RFAnimation* GetAnimation(const std::wstring& name) const; // 00431190
	RF_API bool IsSceneExist(const std::wstring& name) const; // 00431200
	RF_API RFAnimation* FindScene(const std::wstring& name); // 00431220
	RF_API const RFAnimation* FindScene(const std::wstring& name) const; // 00431230
	RF_API RFAnimation* GetPlayingScene(); // 00431240
	RF_API R_ERRORTYPE Play(const std::wstring& name); // 00431250
	RF_API R_ERRORTYPE Play(RFWnd* wnd, const std::wstring& name); // 00431290
	RF_API R_ERRORTYPE Play(int index); // 004312c0
	RF_API void Stop(); // 00431310
	RF_API bool IsPlaying(const std::wstring& name) const; // 00431320
	RF_API bool IsPlaying() const; // 00431340
	RF_API void End(const std::wstring& name); // 00431350
	RF_API void End(); // 00431370
	RF_API void Loop(const std::wstring& name, bool loop); // 00431380
	RF_API void Loop(int index, bool loop); // 004313a0
	RF_API bool IsLoop(const std::wstring& name) const; // 004313d0
	RF_API bool IsLoop() const; // 00431410
	RF_API void FrameMove(float delta); // 00431420
	RF_API void FrameMove(RFWnd* wnd, float delta); // 00431440
	RF_API void Render(float alpha); // 00431a50

	// cena pelo índice (src\RFDirector.cpp 236; o LTCG a embute nos chamadores: RFPicture::LoadResource/Play/SetAnimation; nome meu)
	void PlayIndex983(int index)
	{
		if (0 <= index && index < (int)_scenes983.size())
		{
			_playing = _scenes983[index];
			_playing->Play983();
		}
		else
			_wassert(L"!\"Parameter 'index' out of range.\"", L"src\\RFDirector.cpp", 236);
	}

	RF_API static const std::wstring EventAnimFinished; // 00e46f20
private:
	RF_API R_ERRORTYPE LoadAnimation_(const std::wstring& path); // 00430c30
public:
	std::vector<RFAnimation*> _scenes983; // +0x30 (cenas na ordem do arquivo)
	unsigned int _version; // +0x40 (version="maior.menor" = maior << 16 | menor; exige > 1.1)
	RFRenderer* _renderer; // +0x44
	RFInterface* _interface; // +0x48
	RFAnimation* _playing; // +0x4c (Play/Stop/IsPlaying)
	std::map<RFWnd*, RFWindowModifier*> _modifiers983; // +0x50 ("AnimationWindowModifier" de cada janela animada)
};
