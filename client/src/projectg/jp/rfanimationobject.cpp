// JP 983: RFAnimationObject.cpp da Refresh (B157, F3.7.13 item (6); 0042af20–0042cb70). Objeto de uma cena de animação: trilha
// de quadros (RFAnimFrame) e textura; o construtor, name/asset e as cópias são embutidos no rfdirector.h.
// Load/GetXmlValue: B158.
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfdirector.h"
#include "refresh/tinyxml/tinyxml.h"

std::wstring RFUtf8ToWide983(const char* text); // 0042aaa0 (rfmanager.cpp)

const std::wstring RFAnimationObject::EventDraw983(L"Draw"); // 00e47118

// 0042af20
void RFAnimationFrame::Clear()
{
	time = 0;
	visible = true;
	sfx.clear();
	x.value = 0.0f;
	y.value = 0.0f;
	centerX.value = 0.5f;
	centerY.value = 0.5f;
	scaleX.value = 1.0f;
	scaleY.value = 1.0f;
	rotation.value = 0.0f;
	x.interpolation = 0;
	y.interpolation = 0;
	centerX.interpolation = 0;
	centerY.interpolation = 0;
	scaleX.interpolation = 0;
	scaleY.interpolation = 0;
	rotation.interpolation = 0;
	alpha.value = 1.0f;
	alpha.interpolation = 0;
}

// 0042afc0
RFAnimationObject::~RFAnimationObject()
{
	if (_texture)
		RFManager::GetInstance()._renderer->DeleteTexture(_texture);
	if (_frame)
	{
		delete _frame;
		_frame = 0;
	}
}

// 0042c7d0: avisa "Draw" (quem trata desenha por conta própria) e desenha a trilha
void RFAnimationObject::Render(float alpha)
{
	AnimDrawEventArgs e;
	e.object = this;
	e.alpha = alpha;
	EventMap::iterator it = _events.find(EventDraw983);
	if (it != _events.end())
		it->second(e);
	if (!e.handled && (_type983 == 0 || _type983 == 2))
		_frame->Render(_texture, alpha);
}

// retângulo do <asset><rect l t r b/> (inteiros)
static void LoadRect983(const rf::TiXmlElement* asset, RFRect* rect)
{
	RFRect r(0.0f, 0.0f, 0.0f, 0.0f);
	const rf::TiXmlElement* element = asset->FirstChildElement("rect");
	for (const rf::TiXmlAttribute* attribute = element->FirstAttribute(); attribute; attribute = attribute->Next())
	{
		if (!strcmp(attribute->Name(), "l"))
			r.left = (float)atoi(attribute->Value());
		else if (!strcmp(attribute->Name(), "t"))
			r.top = (float)atoi(attribute->Value());
		else if (!strcmp(attribute->Name(), "r"))
			r.right = (float)atoi(attribute->Value());
		else if (!strcmp(attribute->Name(), "b"))
			r.bottom = (float)atoi(attribute->Value());
	}
	*rect = r;
}

// 0042b0f0: <asset name=_asset type="IMAGE|UI|TEXT"> dá o retângulo/textura; <anim name=_ani983><frame cnt=…> dá a trilha
void RFAnimationObject::Load(const rf::TiXmlNode* scene)
{
	for (const rf::TiXmlElement* asset = scene->FirstChildElement("asset"); asset; asset = asset->NextSiblingElement("asset"))
	{
		if (RFUtf8ToWide983(asset->Attribute("name")) != _asset)
			continue;
		const char* type = asset->Attribute("type");
		if (!strcmp(type, "IMAGE"))
		{
			LoadRect983(asset, &_rect);
			_type983 = 0;
			RFRenderer* renderer = RFManager::GetInstance()._renderer;
			_texture = renderer->CreateTexture(RFUtf8ToWide983(asset->Attribute("path")));
		}
		else if (!strcmp(type, "UI"))
		{
			LoadRect983(asset, &_rect);
			_type983 = 1;
		}
		else if (!strcmp(type, "TEXT"))
		{
			// o 983 lê o texto e a fonte e não guarda nada (só o tipo)
			std::wstring text = RFUtf8ToWide983(asset->Attribute("text"));
			std::wstring face;
			face = RFUtf8ToWide983(asset->FirstChildElement("facename")->Attribute("val"));
			atoi(asset->FirstChildElement("size")->Attribute("val"));
			atoi(asset->FirstChildElement("style")->Attribute("val"));
			atoi(asset->FirstChildElement("color")->Attribute("val"));
			_type983 = 2;
		}
	}

	for (const rf::TiXmlElement* anim = scene->FirstChildElement("anim"); anim; anim = anim->NextSiblingElement("anim"))
	{
		if (RFUtf8ToWide983(anim->Attribute("name")) != _ani983)
			continue;
		const rf::TiXmlElement* key = anim->FirstChildElement("frame");
		if (!key)
			continue;
		RFAnimFrame* track = new RFAnimFrame;
		track->SetCanvas(RFRect(0.0f, 0.0f, _animation->_size.width, _animation->_size.height));
		do
		{
			RFAnimationFrame frame;
			for (const rf::TiXmlAttribute* attribute = key->FirstAttribute(); attribute; attribute = attribute->Next())
			{
				if (!strcmp(attribute->Name(), "cnt"))
					frame.time = atoi(attribute->Value());
			}
			if (_endTime983 < (float)frame.time)
				_endTime983 = (float)frame.time;
			if (key->FirstChild("posx"))
				GetXmlValue(key, "posx", &frame.x);
			if (key->FirstChild("posy"))
				GetXmlValue(key, "posy", &frame.y);
			if (key->FirstChild("centerx"))
				GetXmlValue(key, "centerx", &frame.centerX);
			if (key->FirstChild("centery"))
				GetXmlValue(key, "centery", &frame.centerY);
			if (key->FirstChild("scalex"))
				GetXmlValue(key, "scalex", &frame.scaleX);
			if (key->FirstChild("scaley"))
				GetXmlValue(key, "scaley", &frame.scaleY);
			if (key->FirstChild("rotate"))
				GetXmlValue(key, "rotate", &frame.rotation);
			if (key->FirstChild("alpha"))
				GetXmlValue(key, "alpha", &frame.alpha);
			const rf::TiXmlElement* sound = key->FirstChildElement("sound");
			if (sound)
				frame.sfx = RFUtf8ToWide983(sound->Attribute("val"));
			const rf::TiXmlElement* visible = key->FirstChildElement("visible");
			if (visible)
				frame.visible = atoi(visible->Attribute("val")) != 0;
			track->_frames.push_back(frame);
			key = key->NextSiblingElement("frame");
		} while (key);
		_frame = track;
	}
}

// 0042c890: <name val="…" easeValue="x1,y1,x2,y2"/>; sem easeValue = linear
void RFAnimationObject::GetXmlValue(const rf::TiXmlNode* node, const std::string& name, RFFloatAnimationFrame* value)
{
	value->value = (float)atof(node->FirstChildElement(name.c_str())->Attribute("val"));
	std::string ease;
	if (!node->FirstChildElement(name.c_str())->Attribute("easeValue"))
		value->interpolation = new RFInterpolation;
	else
	{
		ease = node->FirstChildElement(name.c_str())->Attribute("easeValue");
		if (ease.size() != 0)
		{
			std::vector<char> buffer(ease.size() + 1);
			memcpy(&buffer[0], ease.c_str(), ease.size());
			char* context;
			const char separator[] = ",";
			float x1 = (float)atof(strtok_s(&buffer[0], separator, &context));
			float y1 = (float)atof(strtok_s(0, separator, &context));
			float x2 = (float)atof(strtok_s(0, separator, &context));
			float y2 = (float)atof(strtok_s(0, separator, &context));
			value->interpolation = new RFBezierSpline(1024, RFVector2(x1, y1), RFVector2(x2, y2));
		}
	}
}
