// JP 983: RFAnimation.cpp da Refresh (B157, F3.7.13 item (6); 0045ba40–0045d464). Cena de animação (não exportada): objetos
// (RFAnimationObject) lidos de <object name ani><asset link/></object>, tempo em segundos e progresso 0..1; laço, pêndulo
// (pingpong) e aviso de fim pelo RFDirector. O LTCG passa o this em registrador; aqui são métodos comuns.
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfdirector.h"
#include "refresh/tinyxml/tinyxml.h"

std::wstring RFUtf8ToWide983(const char* text); // 0042aaa0 (rfmanager.cpp)

// 0045ba40
RFAnimation::RFAnimation(RFDirector* director)
	: _version(0), _director(director), _time(0.0f), _playing983(false), _repeat983(false), _pingpong983(false), _reverse983(0),
	_size(1.0f, 1.0f), _progress983(0.0f)
{
	_endTime983 = 0.0f;
	_startTime983 = 0.0f;
	_version = _director->_version;
}

// 0045bb20
RFAnimation::~RFAnimation()
{
	_playing983 = false;
}

// 0045c250: atributos da <scene>, os <param> e os <object>; depois cada objeto lê a sua trilha
void RFAnimation::Load983(const rf::TiXmlNode* scene)
{
	const rf::TiXmlElement* element = scene->ToElement();
	for (const rf::TiXmlAttribute* attribute = element->FirstAttribute(); attribute; attribute = attribute->Next())
	{
		if (!strcmp(attribute->Name(), "name"))
			_name = RFUtf8ToWide983(attribute->Value());
		else if (!strcmp(attribute->Name(), "loop"))
			_repeat983 = atoi(attribute->Value()) == 0;
		else if (!strcmp(attribute->Name(), "pingpong"))
			_pingpong983 = !strcmp(attribute->Value(), "true");
	}
	LoadParams983(element);

	for (const rf::TiXmlElement* node = element->FirstChildElement("object"); node; node = node->NextSiblingElement("object"))
	{
		std::wstring name = RFUtf8ToWide983(node->Attribute("name"));
		RFAnimationObject object(this);
		object._name = name;
		object._ani983 = RFUtf8ToWide983(node->Attribute("ani"));
		for (const rf::TiXmlElement* asset = node->FirstChildElement("asset"); asset; asset = asset->NextSiblingElement("asset"))
		{
			const char* link = asset->Attribute("link");
			if (link)
				object._asset = RFUtf8ToWide983(link);
		}
		_objects.push_back(object);
	}

	for (int i = 0; i < (int)_objects.size(); ++i)
	{
		_objects[i].Load(scene);
		if (_endTime983 < _objects[i]._endTime983)
			_endTime983 = _objects[i]._endTime983;
	}
}

// 0045bf70
void RFAnimation::LoadParams983(const rf::TiXmlElement* scene)
{
	for (const rf::TiXmlElement* param = scene->FirstChildElement("param"); param; param = param->NextSiblingElement("param"))
	{
		const char* type = param->Attribute("type");
		if (!strcmp(type, "RES"))
		{
			float x = (float)atof(param->Attribute("x"));
			float y = (float)atof(param->Attribute("y"));
			_size.width = x;
			_size.height = y;
		}
		else if (!strcmp(type, "LOOP"))
		{
			_repeat983 = atoi(param->Attribute("cnt")) == -1;
			_pingpong983 = atoi(param->Attribute("pendulum")) != 0;
		}
	}
}

// 0045bbf0: canvas de cada objeto = posição + tamanho da cena
void RFAnimation::SetPosition983(const RFVector2& pos)
{
	for (unsigned int i = 0; i < _objects.size(); ++i)
	{
		RFAnimFrame* frame = _objects[i]._frame;
		frame->_canvas983.left = pos.x;
		frame->_canvas983.right = _size.width + pos.x;
		frame->_canvas983.top = pos.y;
		frame->_canvas983.bottom = pos.y + _size.height;
	}
	for (unsigned int i = 0; i < _objects.size(); ++i)
		_objects[i]._frame->GenAnimMatrix(_objects[i]._rect);
}

// 0045ca80: do início (ou do fim, no pêndulo de volta)
void RFAnimation::Play983()
{
	unsigned int count = _objects.size();
	if (count == 0)
	{
		_playing983 = false;
		_time = 0.0f;
		_finished = 0;
		return;
	}
	for (unsigned int i = 0; i < count; ++i)
	{
		RFAnimFrame* frame = _objects[i]._frame;
		if (_reverse983 == 0)
			frame->Play();
		else
			frame->PlayPendulum();
	}
	if (_reverse983 == 0)
	{
		_playing983 = true;
		_time = 0.0f;
		_finished = 0;
		_progress983 = 0.0f;
		return;
	}
	_playing983 = true;
	_finished = 0;
	_time = (float)(_endTime983 / 1000.0);
	_progress983 = 1.0f;
}

// 0045cb40: para, volta os objetos ao 1º quadro e avisa EventAnimFinished pelo diretor
void RFAnimation::Stop983()
{
	_time = 0.0f;
	_playing983 = false;
	_reverse983 = 0;
	_finished = 0;
	for (unsigned int i = 0; i < _objects.size(); ++i)
		_objects[i]._frame->Stop();

	RFDirector* director = _director;
	AnimationEventArgs e;
	e.name = _name;
	EventMap::iterator it = director->_events.find(RFDirector::EventAnimFinished);
	if (it != director->_events.end())
		it->second(e);
}

// 0045cca0: salta para o último quadro
void RFAnimation::End983()
{
	_time = 0.0f;
	_playing983 = false;
	unsigned int count = _objects.size();
	if (count == 0)
		return;
	for (unsigned int i = 0; i < count; ++i)
		_objects[i]._frame->End(_objects[i]._rect);
	_time = (float)(_endTime983 / 1000.0);
	_progress983 = 1.0f;
}

// 0045cda0
void RFAnimation::FrameMove983(float delta)
{
	if (!_playing983)
		return;

	if (_finished == _objects.size())
	{
		if (!_repeat983)
		{
			_director->SendFinishEvent(this);
			_progress983 = 1.0f;
			_playing983 = false;
			_finished = 0;
			return;
		}
		if (_pingpong983)
			_reverse983 = _reverse983 == 0;
		Play983();
	}

	unsigned int time = (unsigned int)(_time * 1000.0 + 0.5);
	_finished = 0;
	_progress983 = ((float)time - _startTime983) / (_endTime983 - _startTime983);
	for (unsigned int i = 0; i < _objects.size(); ++i)
	{
		RFAnimFrame* frame = _objects[i]._frame;
		if (!frame->_playing)
			++_finished;
		else if (_reverse983 == 0)
			frame->FrameMove(time, _objects[i]._rect);
		else
			frame->FrameMovePendulum(time, _objects[i]._rect);
	}

	_time += (float)((_reverse983 == 0) * 2 - 1) * delta;
	if (_time < 0.0f)
		_time = 0.0f;
}

// 0045cfb0 (RFAnimationObject::Render embutido)
void RFAnimation::Draw983(float alpha)
{
	for (unsigned int i = 0; i < _objects.size(); ++i)
		_objects[i].Render(alpha);
}
