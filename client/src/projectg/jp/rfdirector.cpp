// JP 983: RFDirector.cpp da Refresh (B157, F3.7.13 item (6); 004307f0–00431bd0). Diretor de animação: lê o <rfanimation> pelo
// RFInterface (FOpen/FRead), guarda as cenas (RFAnimation) e, no FrameMove(RFWnd*), aplica o quadro de cada objeto à janela
// filha de mesmo nome por um RFWindowModifier "AnimationWindowModifier" (alfa, visível, tamanho e posição).
// Asserts com o arquivo/linha do 983 (src\RFDirector.cpp) chamando o _wassert direto, como o rfdirector.h.
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfinterface.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfdirector.h"
#include "refresh/tinyxml/tinyxml.h"

// 00404230: cópia fora de linha (mapa de eventos +4, cenas +0x30, versão/desenhista/interface/cena tocando, moduladores)
RFDirector::RFDirector(const RFDirector& o)
	: RFEventHandler(o), _scenes983(o._scenes983), _version(o._version), _renderer(o._renderer), _interface(o._interface),
	  _playing(o._playing), _modifiers983(o._modifiers983)
{
}

// 00404330
RFDirector& RFDirector::operator=(const RFDirector& o)
{
	RFEventHandler::operator=(o);
	_scenes983 = o._scenes983;
	_version = o._version;
	_renderer = o._renderer;
	_interface = o._interface;
	_playing = o._playing;
	if (this != &o)
		_modifiers983 = o._modifiers983;
	return *this;
}

// 004307f0
RFDirector::RFDirector()
	: _version(0), _renderer(RFManager::GetInstance()._renderer), _interface(RFManager::GetInstance()._interface), _playing(0)
{
}

// 00430910: as cenas e os modificadores criados pelo FrameMove(RFWnd*)
RFDirector::~RFDirector()
{
	UnloadAllAnimation();
	for (std::map<RFWnd*, RFWindowModifier*>::iterator it = _modifiers983.begin(); it != _modifiers983.end(); ++it)
		delete it->second;
}

// 00430a10
void RFDirector::SendFinishEvent(RFAnimation* animation)
{
	AnimationEventArgs e;
	e.name = animation->_name;
	EventMap::iterator it = _events.find(EventAnimFinished);
	if (it != _events.end())
		it->second(e);
}

// 00430ad0
void RFDirector::UnloadAllAnimation()
{
	for (unsigned int i = 0; i < _scenes983.size(); ++i)
	{
		if (_scenes983[i])
		{
			delete _scenes983[i];
			_scenes983[i] = 0;
		}
	}
	_scenes983.clear();
}

// 00430b50: nome relativo à base do RFInterface
R_ERRORTYPE RFDirector::LoadAnimation(const std::wstring& name)
{
	return LoadAnimation_(_interface->GetBasePath((RFBASEPATHTYPE)0) + name);
}

// 00430c20
R_ERRORTYPE RFDirector::LoadAnimationInDirect(const std::wstring& path)
{
	return LoadAnimation_(path);
}

// 00430c30: <rfanimation version="maior.menor"> (exige > 1.1) com uma <scene> por cena; 1 = lido, 2 = falha
R_ERRORTYPE RFDirector::LoadAnimation_(const std::wstring& path)
{
	void* file = _interface->FOpen(path);
	if (!file)
		return (R_ERRORTYPE)2;

	_interface->FSeek(file, 0, SEEK_END);
	long size = _interface->FTell(file);
	_interface->FSeek(file, 0, SEEK_SET);
	if (size <= 0)
	{
		_interface->FClose(file);
		return (R_ERRORTYPE)2;
	}

	std::vector<char> buffer(size + 1);
	_interface->FRead(file, &buffer[0], size);
	_interface->FClose(file);

	rf::TiXmlDocument document;
	document.Parse(&buffer[0], 0, rf::TIXML_DEFAULT_ENCODING);
	if (document.Error())
		return (R_ERRORTYPE)2;

	rf::TiXmlElement* root = document.FirstChildElement("rfanimation");
	if (root)
	{
		const char* version = root->Attribute("version");
		if (version)
		{
			int major, minor;
			sscanf_s(version, "%d.%d", &major, &minor);
			_version = (unsigned short)major << 16 | (unsigned short)minor;
		}
		if (_version > 0x10001)
		{
			for (rf::TiXmlElement* scene = root->FirstChildElement("scene"); scene; scene = scene->NextSiblingElement("scene"))
			{
				RFAnimation* animation = new RFAnimation(this);
				animation->Load983(scene);
				_scenes983.push_back(animation);
			}
			return R_OK983;
		}
	}
	return (R_ERRORTYPE)2;
}

// 00430fa0 (o corpo do RFAnimation::SetPosition983 embutido)
void RFDirector::SetPosition(const RFVector2& pos)
{
	for (unsigned int i = 0; i < _scenes983.size(); ++i)
		_scenes983[i]->SetPosition983(pos);
}

// 00431140
int RFDirector::GetAnimationCount() const
{
	return _scenes983.size();
}

// 00431150
RFAnimation* RFDirector::GetAnimation(int index) const
{
	if ((unsigned int)index >= _scenes983.size())
	{
		_wassert(L"!\"Parameter 'index' out of range.\"", L"src\\RFDirector.cpp", 163);
		return 0;
	}
	return _scenes983[index];
}

// 00431190
RFAnimation* RFDirector::GetAnimation(const std::wstring& name) const
{
	for (unsigned int i = 0; i < _scenes983.size(); ++i)
	{
		if (_scenes983[i]->_name == name)
			return _scenes983[i];
	}
	return 0;
}

// 00431200
bool RFDirector::IsSceneExist(const std::wstring& name) const
{
	return GetAnimation(name) != 0;
}

// 00431220
RFAnimation* RFDirector::FindScene(const std::wstring& name)
{
	return GetAnimation(name);
}

// 00431230
const RFAnimation* RFDirector::FindScene(const std::wstring& name) const
{
	return GetAnimation(name);
}

// 00431240
RFAnimation* RFDirector::GetPlayingScene()
{
	return _playing;
}

// 00431250: recomeça do início
R_ERRORTYPE RFDirector::Play(const std::wstring& name)
{
	RFAnimation* animation = GetAnimation(name);
	if (!animation)
		return R_FAIL983;
	_playing = animation;
	animation->Stop983();
	animation->Play983();
	return R_OK983;
}

// 00431290 (devolve 0 sempre, como o 983)
R_ERRORTYPE RFDirector::Play(RFWnd* wnd, const std::wstring& name)
{
	if (wnd)
	{
		RFAnimation* animation = GetAnimation(name);
		if (animation)
		{
			_playing = animation;
			animation->Play983();
		}
	}
	return R_FAIL983;
}

// 004312c0
R_ERRORTYPE RFDirector::Play(int index)
{
	if (0 <= index && index < (int)_scenes983.size())
	{
		_playing = _scenes983[index];
		_playing->Play983();
		return R_OK983;
	}
	_wassert(L"!\"Parameter 'index' out of range.\"", L"src\\RFDirector.cpp", 236);
	return R_FAIL983;
}

// 00431310
void RFDirector::Stop()
{
	if (_playing)
		_playing->Stop983();
}

// 00431320
bool RFDirector::IsPlaying(const std::wstring& name) const
{
	RFAnimation* animation = GetAnimation(name);
	if (!animation)
		return false;
	return animation->_playing983;
}

// 00431340
bool RFDirector::IsPlaying() const
{
	if (_playing)
		return _playing->_playing983;
	return false;
}

// 00431350
void RFDirector::End(const std::wstring& name)
{
	RFAnimation* animation = GetAnimation(name);
	if (animation)
		animation->End983();
}

// 00431370
void RFDirector::End()
{
	if (_playing)
		_playing->End983();
}

// 00431380
void RFDirector::Loop(const std::wstring& name, bool loop)
{
	RFAnimation* animation = GetAnimation(name);
	if (animation)
		animation->_repeat983 = loop;
}

// 004313a0
void RFDirector::Loop(int index, bool loop)
{
	if ((unsigned int)index < _scenes983.size())
		_scenes983[index]->_repeat983 = loop;
}

// 004313d0
bool RFDirector::IsLoop(const std::wstring& name) const
{
	RFAnimation* animation = GetAnimation(name);
	if (!animation)
		_wassert(L"!\"Animation not found.\"", L"src\\RFDirector.cpp", 301);
	return animation->_repeat983;
}

// 00431410
bool RFDirector::IsLoop() const
{
	if (_playing)
		return _playing->_repeat983;
	return false;
}

// 00431420
void RFDirector::FrameMove(float delta)
{
	if (_playing)
		_playing->FrameMove983(delta);
}

// 00431440: anda a cena e copia o quadro de cada objeto para a janela de mesmo nome (a própria ou uma filha)
void RFDirector::FrameMove(RFWnd* wnd, float delta)
{
	if (!_playing || !_playing->_playing983 || !wnd)
		return;

	_playing->FrameMove983(delta);
	RFAnimation* animation = _playing;
	std::wstring name;
	RFSize size = animation->_size;
	for (unsigned int i = 0; i < animation->_objects.size(); ++i)
	{
		RFAnimationObject& object = animation->GetObject983(i);
		name = object._name;
		RFWnd* target = name == wnd->_name ? wnd : wnd->GetChild(name);
		if (!target)
			continue;

		RFAnimationFrame frame;
		frame = object._frame->_now;

		RFWindowModifier* modifier;
		std::map<RFWnd*, RFWindowModifier*>::iterator it = _modifiers983.find(target);
		if (it == _modifiers983.end())
		{
			modifier = new RFWindowModifier();
			modifier->SetName(L"AnimationWindowModifier");
			target->AddModifier(modifier);
			_modifiers983.insert(std::make_pair(target, modifier));
		}
		else
			modifier = it->second;

		modifier->SetAlpha(frame.alpha.value);
		if (target->_animationMask & 1)
		{
			bool visible = frame.visible;
			if (target->_visible != visible)
			{
				target->_visible = visible;
				target->_visualState = (VisualStateTypes::Type)(visible ? 2 : 0);
				target->OnShow(visible);
			}
		}

		if (target->_type == 0 || target->_type == 1 || target->_type == 7)
		{
			RFRect rect = object._rect;
			rect.right = frame.scaleX.value * rect.right;
			rect.bottom = frame.scaleY.value * rect.bottom;
			RFSize extent;
			extent.width = (float)(int)(rect.right - rect.left) - (int)target->_initialSize.width;
			extent.height = (float)(int)(rect.bottom - rect.top) - (int)target->_initialSize.height;
			modifier->SetSize(extent);
			target->OnLayoutChanged(2);
		}

		float dx = target->_localPos.x - target->_initialPos.x;
		float dy = target->_localPos.y - target->_initialPos.y;
		RFVector2 pos;
		pos.x = (float)(int)(((float)(frame.x.value * size.width + 0.5) - target->_localPos.x) + dx);
		pos.y = (float)(int)(((float)(frame.y.value * size.height + 0.5) - target->_localPos.y) + dy);
		modifier->SetPos(pos);
		target->OnLayoutChanged(1);
	}
}

// 00431a50
void RFDirector::Render(float alpha)
{
	if (_playing)
		_playing->Draw983(alpha);
}
