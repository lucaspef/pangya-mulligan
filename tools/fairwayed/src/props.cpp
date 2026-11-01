// SPDX-License-Identifier: MIT
// props.cpp - a small XML reader and writer that matches how the shipped files are laid out.
#include "props.h"
#include <stdio.h>
#include <string.h>
#include "gbin.h"
#include "wcommon.h"

std::string XmlNode::Attr(const std::string& key) const
{
	for (size_t i = 0; i < attrs.size(); ++i)
		if (attrs[i].first == key)
			return attrs[i].second;
	return std::string();
}

void XmlNode::SetAttr(const std::string& key, const std::string& value)
{
	for (size_t i = 0; i < attrs.size(); ++i)
	{
		if (attrs[i].first == key)
		{
			attrs[i].second = value;
			return;
		}
	}
	attrs.push_back(std::make_pair(key, value));
}

XmlNode* XmlNode::Child(const std::string& n)
{
	for (size_t i = 0; i < children.size(); ++i)
		if (children[i].name == n)
			return &children[i];
	return 0;
}

const XmlNode* XmlNode::Child(const std::string& n) const
{
	return const_cast<XmlNode*>(this)->Child(n);
}

std::string XmlNode::ChildText(const std::string& n) const
{
	const XmlNode* c = Child(n);
	return c ? c->text : std::string();
}

void XmlNode::SetChildText(const std::string& n, const std::string& value)
{
	XmlNode* c = Child(n);
	if (!c)
	{
		XmlNode fresh;
		fresh.name = n;
		// <textures> stays last, as in every shipped class.
		size_t at = children.size();
		if (at && children[at - 1].name == "textures")
			--at;
		children.insert(children.begin() + at, fresh);
		c = &children[at];
	}
	c->text = value;
}

void XmlNode::RemoveChild(const std::string& n)
{
	for (size_t i = 0; i < children.size(); ++i)
	{
		if (children[i].name == n)
		{
			children.erase(children.begin() + i);
			return;
		}
	}
}

// -- parser -------------------------------------------------------------------

static void SkipSpace(const std::string& t, size_t* at)
{
	while (*at < t.size() && strchr(" \t\r\n", t[*at]))
		++*at;
}

// CP949 trail bytes can equal '<' only in theory: they start at 0x41, and '<' is 0x3C.
static bool ParseElement(const std::string& t, size_t* at, XmlNode* node, std::string* error)
{
	if (*at >= t.size() || t[*at] != '<')
	{
		*error = "expected an element";
		return false;
	}
	size_t p = *at + 1;
	size_t e = p;
	while (e < t.size() && !strchr(" \t\r\n/>", t[e]))
		++e;
	node->name = t.substr(p, e - p);
	p = e;
	for (;;)
	{
		SkipSpace(t, &p);
		if (p >= t.size())
		{
			*error = "unterminated tag " + node->name;
			return false;
		}
		if (t[p] == '/' && p + 1 < t.size() && t[p + 1] == '>')
		{
			*at = p + 2;
			return true;
		}
		if (t[p] == '>')
		{
			++p;
			break;
		}
		size_t eq = t.find('=', p);
		if (eq == std::string::npos)
		{
			*error = "bad attribute in " + node->name;
			return false;
		}
		std::string key = t.substr(p, eq - p);
		size_t q0 = t.find('"', eq);
		size_t q1 = q0 == std::string::npos ? q0 : t.find('"', q0 + 1);
		if (q1 == std::string::npos)
		{
			*error = "bad attribute in " + node->name;
			return false;
		}
		node->attrs.push_back(std::make_pair(key, t.substr(q0 + 1, q1 - q0 - 1)));
		p = q1 + 1;
	}
	for (;;)
	{
		size_t lt = t.find('<', p);
		if (lt == std::string::npos)
		{
			*error = "unterminated element " + node->name;
			return false;
		}
		if (t.compare(lt, 2, "</") == 0)
		{
			if (node->children.empty())
				node->text = t.substr(p, lt - p);
			size_t gt = t.find('>', lt);
			if (gt == std::string::npos)
			{
				*error = "unterminated end tag";
				return false;
			}
			*at = gt + 1;
			return true;
		}
		XmlNode child;
		if (!ParseElement(t, &lt, &child, error))
			return false;
		node->children.push_back(child);
		p = lt;
	}
}

static void WriteElement(std::string* out, const XmlNode& n, int depth, const std::string& nl)
{
	out->append(depth * 4, ' ');
	*out += "<" + n.name;
	for (size_t i = 0; i < n.attrs.size(); ++i)
		*out += " " + n.attrs[i].first + "=\"" + n.attrs[i].second + "\"";
	if (n.children.empty() && n.text.empty())
	{
		*out += " />" + nl;
		return;
	}
	*out += ">";
	if (n.children.empty())
	{
		*out += n.text + "</" + n.name + ">" + nl;
		return;
	}
	*out += nl;
	for (size_t i = 0; i < n.children.size(); ++i)
		WriteElement(out, n.children[i], depth + 1, nl);
	out->append(depth * 4, ' ');
	*out += "</" + n.name + ">" + nl;
}

void PropFile::Clear()
{
	prolog = "<?xml version=\"1.0\" encoding=\"euc-kr\" standalone=\"yes\" ?>\n";
	lineEnd = "\n";
	root = XmlNode();
	path.clear();
	dirty = false;
	original.clear();
}

bool PropFile::Parse(const std::string& text, std::string* error)
{
	Clear();
	size_t at = text.find("?>");
	if (at == std::string::npos)
		at = 0;
	else
		at += 2;
	size_t lt = text.find('<', at);
	if (lt == std::string::npos)
	{
		*error = "no root element";
		return false;
	}
	prolog = text.substr(0, lt);
	lineEnd = prolog.find("\r\n") != std::string::npos ? "\r\n" : "\n";
	size_t p = lt;
	if (!ParseElement(text, &p, &root, error))
		return false;
	return true;
}

std::string PropFile::Serialize() const
{
	std::string out = prolog;
	WriteElement(&out, root, 0, lineEnd);
	return out;
}

bool PropFile::Load(const std::string& p, std::string* error)
{
	std::vector<u8> data;
	if (!ReadWholeFile(p, &data))
	{
		*error = "cannot open " + p;
		return false;
	}
	std::string text(data.begin(), data.end());
	if (!Parse(text, error))
		return false;
	path = p;
	original = text;
	return true;
}

bool PropFile::Save(const std::string& p, std::string* error) const
{
	std::string text = dirty || original.empty() ? Serialize() : original;
	std::vector<u8> data(text.begin(), text.end());
	return WriteWholeFile(p, data, error);
}

int PropFile::DefaultClass() const
{
	const XmlNode* d = Declare();
	if (!d)
		return -1;
	for (size_t i = 0; i < d->children.size(); ++i)
		if (d->children[i].Attr("name") == "default")
			return (int)i;
	return -1;
}

const XmlNode* PropFile::ClassAt(int index) const
{
	const XmlNode* d = Declare();
	if (!d || index < 0 || index >= (int)d->children.size())
		return 0;
	return &d->children[index];
}

int PropFile::ClassOfTexture(const std::string& tex) const
{
	const XmlNode* d = Declare();
	if (!d)
		return -1;
	for (size_t i = 0; i < d->children.size(); ++i)
	{
		const XmlNode* list = d->children[i].Child("textures");
		if (!list)
			continue;
		for (size_t k = 0; k < list->children.size(); ++k)
			if (!_stricmp(list->children[k].text.c_str(), tex.c_str()))
				return (int)i;
	}
	return DefaultClass();
}

std::string PropFile::Field(int classIndex, const std::string& field) const
{
	const XmlNode* c = ClassAt(classIndex);
	if (c && c->Child(field))
		return c->ChildText(field);
	c = ClassAt(DefaultClass());
	return c ? c->ChildText(field) : std::string();
}

XmlNode* PropFile::WepEntry(const std::string& wepName)
{
	XmlNode* p = Project();
	if (!p)
		return 0;
	for (size_t i = 0; i < p->children.size(); ++i)
		if (!_stricmp(p->children[i].Attr("name").c_str(), wepName.c_str()))
			return &p->children[i];
	return 0;
}

void PropFile::SyncCounts()
{
	char buf[32];
	if (XmlNode* d = Declare())
	{
		sprintf_s(buf, "%d", (int)d->children.size());
		d->SetAttr("count", buf);
	}
	if (XmlNode* p = Project())
	{
		sprintf_s(buf, "%d", (int)p->children.size());
		p->SetAttr("count", buf);
	}
}

int PropGroundType(const std::string& pet)
{
	static const struct { const char* key; int type; } order[] = {
		{ "green", 2 }, { "fairway", 1 }, { "bunker", 3 }, { "rough", 4 }, { "snow", 5 }, { "road", 6 },
		{ "ice", 7 }, { "water", 9 }, { "sand", 11 }, { "booster", 13 }, { "special", 12 }, { "wb", 14 }, { "vector", 8 }
	};
	for (size_t i = 0; i < sizeof(order) / sizeof(order[0]); ++i)
		if (pet.find(order[i].key) != std::string::npos)
			return order[i].type;
	return 0;
}

const char* PropGroundName(int type)
{
	switch (type)
	{
	case 1: return "fairway";
	case 2: return "green";
	case 3: return "bunker";
	case 4: return "rough";
	case 5: return "snow";
	case 6: return "road";
	case 7: return "ice";
	case 8: return "vector";
	case 9: return "water";
	case 11: return "sand";
	case 12: return "special";
	case 13: return "booster";
	case 14: return "wb";
	default: return "none";
	}
}
