// JP 983: RFAnimFrame.cpp da Refresh (B158, F3.7.13 item (6); 0042d090–0042ee70). Trilha de quadros de um objeto da cena: o
// quadro "agora" é interpolado entre "de" e "para" pela RFInterpolation do quadro "para"; a matriz vai para o RFRenderer::Draw.
// As funções 0042e3a0–0042ee10 são do std::vector<RFAnimationFrame> (push_back/insert/reserve).
#include <math.h>
#include <string>
#include <vector>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfinterface.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfdirector.h"

// 0042d100 (ESI = quadro): apaga as interpolações de um quadro da trilha
static void DeleteInterpolations983(RFAnimationFrame& frame)
{
	if (frame.x.interpolation)
	{
		delete frame.x.interpolation;
		frame.x.interpolation = 0;
	}
	if (frame.y.interpolation)
	{
		delete frame.y.interpolation;
		frame.y.interpolation = 0;
	}
	if (frame.centerX.interpolation)
	{
		delete frame.centerX.interpolation;
		frame.centerX.interpolation = 0;
	}
	if (frame.centerY.interpolation)
	{
		delete frame.centerY.interpolation;
		frame.centerY.interpolation = 0;
	}
	if (frame.scaleX.interpolation)
	{
		delete frame.scaleX.interpolation;
		frame.scaleX.interpolation = 0;
	}
	if (frame.scaleY.interpolation)
	{
		delete frame.scaleY.interpolation;
		frame.scaleY.interpolation = 0;
	}
	if (frame.rotation.interpolation)
	{
		delete frame.rotation.interpolation;
		frame.rotation.interpolation = 0;
	}
	if (frame.alpha.interpolation)
	{
		delete frame.alpha.interpolation;
		frame.alpha.interpolation = 0;
	}
}

// 0042d1a0 (ECX = quadro "agora"; de/para por valor): cada valor = de + (para − de) × curva do "para"(t)
static void Interpolate983(RFAnimationFrame& now, RFAnimationFrame from, RFAnimationFrame to, float t)
{
	float k;
	k = to.x.interpolation->GetValue(t);
	now.x.interpolation = 0;
	now.x.value = (to.x.value - from.x.value) * k + from.x.value;
	k = to.y.interpolation->GetValue(t);
	now.y.interpolation = 0;
	now.y.value = (to.y.value - from.y.value) * k + from.y.value;
	k = to.centerX.interpolation->GetValue(t);
	now.centerX.interpolation = 0;
	now.centerX.value = (to.centerX.value - from.centerX.value) * k + from.centerX.value;
	k = to.centerY.interpolation->GetValue(t);
	now.centerY.interpolation = 0;
	now.centerY.value = (to.centerY.value - from.centerY.value) * k + from.centerY.value;
	k = to.scaleX.interpolation->GetValue(t);
	now.scaleX.interpolation = 0;
	now.scaleX.value = (to.scaleX.value - from.scaleX.value) * k + from.scaleX.value;
	k = to.scaleY.interpolation->GetValue(t);
	now.scaleY.interpolation = 0;
	now.scaleY.value = (to.scaleY.value - from.scaleY.value) * k + from.scaleY.value;
	k = to.rotation.interpolation->GetValue(t);
	now.rotation.interpolation = 0;
	now.rotation.value = (to.rotation.value - from.rotation.value) * k + from.rotation.value;
	k = to.alpha.interpolation->GetValue(t);
	now.alpha.interpolation = 0;
	now.alpha.value = (to.alpha.value - from.alpha.value) * k + from.alpha.value;
}

// 0042d440
RFAnimFrame::RFAnimFrame()
{
	_index = 0;
	_playing = false;
	_hold983 = false;
	_idle983 = false;
	_first983 = true;
	_single983 = false;
}

// 0042d570
RFAnimFrame::~RFAnimFrame()
{
	for (unsigned int i = 0; i < _frames.size(); ++i)
		DeleteInterpolations983(_frames[i]);
	_frames.clear();
}

// 0042d740
void RFAnimFrame::PlaySfx(const std::wstring& name)
{
	RFManager::GetInstance()._interface->PlaySound983(std::wstring(name.c_str()));
}

// 0042d810
void RFAnimFrame::SetCanvas(const RFRect& canvas)
{
	_canvas983 = canvas;
}

// 0042d840
void RFAnimFrame::AddFrame(const RFAnimationFrame& frame)
{
	_frames.push_back(frame);
}

// 0042d860: mantém a trilha em ordem de tempo; tempo repetido não entra
bool RFAnimFrame::InsertFrame(const RFAnimationFrame& frame)
{
	if (!_frames.empty())
	{
		for (unsigned int i = 0; i < _frames.size(); ++i)
		{
			if (frame.time < _frames[i].time)
			{
				_frames.insert(_frames.begin() + i, frame);
				return true;
			}
			if (frame.time == _frames[i].time)
				return false;
		}
	}
	_frames.push_back(frame);
	return true;
}

// 0042d8e0
void RFAnimFrame::GetNowFrame(RFAnimationFrame* frame) const
{
	*frame = _now;
}

// 0042d900
void RFAnimFrame::Stop()
{
	_index = 0;
	_now.Clear();
	_playing = false;
}

// 0042d930
void RFAnimFrame::End(const RFRect& canvas)
{
	_index = (int)_frames.size() - 1;
	_now.Clear();
	_now = _frames[_index];
	GenAnimMatrix(canvas);
}

// 0042d990
void RFAnimFrame::Play()
{
	_index = 0;
	_now.Clear();
	if (_frames.size() == 1)
	{
		_from983 = _to983 = _now = _frames[_index];
		_first983 = true;
		_single983 = true;
		_playing = true;
		return;
	}
	_from983 = _now = _frames[_index];
	_to983 = _frames[_index + 1];
	_first983 = true;
	_single983 = false;
	_playing = true;
}

// 0042da40: do último quadro para o primeiro; "de" leva a visibilidade do quadro anterior a ele
void RFAnimFrame::PlayPendulum()
{
	_index = (int)_frames.size() - 1;
	_now.Clear();
	if (_index < 1)
	{
		_from983 = _to983 = _now = _frames[_index];
		_first983 = true;
		_single983 = true;
		_playing = true;
		return;
	}
	_from983 = _frames[_index - 1];
	_to983 = _frames[_index];
	_to983.visible = _from983.visible;
	if (_index - 2 >= 0)
		_from983.visible = _frames[_index - 2].visible;
	else if (_from983.time > 0)
		_from983.visible = false;
	_now = _from983;
	_first983 = true;
	_single983 = false;
	_playing = true;
}

// 0042db30
bool RFAnimFrame::IsPlaying() const
{
	return _playing;
}

// 0042db40
bool RFAnimFrame::IsIdle() const
{
	return _idle983;
}

// 0042db50: tempo correndo para trás (do fim da trilha ao início)
void RFAnimFrame::FrameMovePendulum(unsigned int time, const RFRect& canvas)
{
	_hold983 = false;
	_idle983 = false;
	if (_single983 == true)
	{
		if (time > _now.time)
			return;
		PlaySfx(_now.sfx);
		_playing = false;
		if (time != 0)
			_idle983 = true;
	}
	else
	{
		float t = ((float)time - (float)_from983.time) / (float)(_to983.time - _from983.time);
		if (t > 1.0f)
		{
			if (_frames[_index].visible == true)
				_hold983 = true;
			else
				_idle983 = true;
		}
		else if (t > 0.0f)
			Interpolate983(_now, _from983, _to983, t);
		else
		{
			--_index;
			if (_index < 1)
			{
				_now = _from983;
				PlaySfx(_now.sfx);
				_playing = false;
			}
			else
			{
				_from983 = _frames[_index - 1];
				_to983 = _frames[_index];
				_to983.visible = _from983.visible;
				if (_index - 2 >= 0)
					_from983.visible = _frames[_index - 2].visible;
				else if (_from983.time > 0)
					_from983.visible = false;
				_now = _to983;
				PlaySfx(_now.sfx);
			}
		}
	}
	GenAnimMatrix(canvas);
}

// 0042dd80
void RFAnimFrame::FrameMove(unsigned int time, const RFRect& canvas)
{
	_hold983 = false;
	_idle983 = false;
	if (_single983 == true)
	{
		if (time < _now.time)
		{
			_idle983 = true;
			return;
		}
		PlaySfx(_now.sfx);
		_playing = false;
	}
	else
	{
		float t = ((float)time - (float)_from983.time) / (float)(_to983.time - _from983.time);
		if (t < 0.0f)
		{
			_idle983 = true;
			return;
		}
		if (_first983 == true)
		{
			_now = _from983;
			PlaySfx(_now.sfx);
			_first983 = false;
		}
		else if (t < 1.0f)
			Interpolate983(_now, _from983, _to983, t);
		else
		{
			++_index;
			if (_index + 1 >= (int)_frames.size())
			{
				_now = _to983;
				PlaySfx(_now.sfx);
				_playing = false;
			}
			else
			{
				_from983 = _frames[_index];
				_to983 = _frames[_index + 1];
				_now = _from983;
				PlaySfx(_now.sfx);
			}
		}
	}
	GenAnimMatrix(canvas);
}

// 0042dfb0: centro (arredondado, em pixels do objeto) → escala → rotação (graus) → posição na cena
void RFAnimFrame::GenAnimMatrix(const RFRect& canvas)
{
	_matrix.m[0][0] = 1.0f; _matrix.m[0][1] = 0.0f; _matrix.m[0][2] = 0.0f; _matrix.m[0][3] = 0.0f;
	_matrix.m[1][0] = 0.0f; _matrix.m[1][1] = 1.0f; _matrix.m[1][2] = 0.0f; _matrix.m[1][3] = 0.0f;
	_matrix.m[2][0] = 0.0f; _matrix.m[2][1] = 0.0f; _matrix.m[2][2] = 1.0f; _matrix.m[2][3] = 0.0f;
	_matrix.m[3][0] = 0.0f; _matrix.m[3][1] = 0.0f; _matrix.m[3][2] = 0.0f; _matrix.m[3][3] = 1.0f;

	float x = (_canvas983.right - _canvas983.left) * _now.x.value + _canvas983.left;
	float y = (_canvas983.bottom - _canvas983.top) * _now.y.value + _canvas983.top;

	RFMatrix center;
	center.m[0][0] = 1.0f; center.m[0][1] = 0.0f; center.m[0][2] = 0.0f; center.m[0][3] = 0.0f;
	center.m[1][0] = 0.0f; center.m[1][1] = 1.0f; center.m[1][2] = 0.0f; center.m[1][3] = 0.0f;
	center.m[2][0] = 0.0f; center.m[2][1] = 0.0f; center.m[2][2] = 1.0f; center.m[2][3] = 0.0f;
	center.m[3][0] = -(float)floor((float)(_now.centerX.value * canvas.right + 0.5));
	center.m[3][1] = -(float)floor((float)(_now.centerY.value * canvas.bottom + 0.5));
	center.m[3][2] = 0.0f;
	center.m[3][3] = 1.0f;

	float angle = (float)(_now.rotation.value * 0.017453292384743690);
	RFMatrix rotation;
	float s = sinf(angle);
	float c = cosf(angle);
	rotation.m[0][0] = c; rotation.m[0][1] = s; rotation.m[0][2] = 0.0f; rotation.m[0][3] = 0.0f;
	rotation.m[1][0] = -s; rotation.m[1][1] = c; rotation.m[1][2] = 0.0f; rotation.m[1][3] = 0.0f;
	rotation.m[2][0] = 0.0f; rotation.m[2][1] = 0.0f; rotation.m[2][2] = 1.0f; rotation.m[2][3] = 0.0f;
	rotation.m[3][0] = 0.0f; rotation.m[3][1] = 0.0f; rotation.m[3][2] = 0.0f; rotation.m[3][3] = 1.0f;

	RFMatrix scale;
	scale.m[0][0] = _now.scaleX.value; scale.m[0][1] = 0.0f; scale.m[0][2] = 0.0f; scale.m[0][3] = 0.0f;
	scale.m[1][0] = 0.0f; scale.m[1][1] = _now.scaleY.value; scale.m[1][2] = 0.0f; scale.m[1][3] = 0.0f;
	scale.m[2][0] = 0.0f; scale.m[2][1] = 0.0f; scale.m[2][2] = 0.0f; scale.m[2][3] = 0.0f; // m[2][2] = 0 no 983
	scale.m[3][0] = 0.0f; scale.m[3][1] = 0.0f; scale.m[3][2] = 0.0f; scale.m[3][3] = 1.0f;

	rotation = scale * rotation;
	_matrix = center * rotation;
	_matrix.m[3][0] += x;
	_matrix.m[3][1] += y;
}

// 0042e2e0
void RFAnimFrame::Render(RFTexture* texture, float alpha)
{
	if (_idle983)
		return;
	if (_hold983 != true && !_now.visible)
		return;
	alpha *= _now.alpha.value;
	if (0.003921569f > alpha) // < 1/255
		return;
	RFManager::GetInstance()._renderer->Draw(texture, alpha, _matrix);
}
