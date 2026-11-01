// SPDX-License-Identifier: MIT
// props.h - the course's surface table, <course>_property.xml.
//
// Read by CPropertyInfo::Init in golfdoc.cpp: a class per surface, mapped from the
// textures of the base model, and a <wep> entry per hole. Fields a class leaves out come
// from the class named "default".
#ifndef PROPS_H
#define PROPS_H

#include <string>
#include <vector>

struct XmlNode
{
	std::string name;
	std::vector<std::pair<std::string, std::string> > attrs;
	std::string text;
	std::vector<XmlNode> children;

	std::string Attr(const std::string& key) const;
	void SetAttr(const std::string& key, const std::string& value);
	XmlNode* Child(const std::string& childName);
	const XmlNode* Child(const std::string& childName) const;
	std::string ChildText(const std::string& childName) const;
	void SetChildText(const std::string& childName, const std::string& value);
	void RemoveChild(const std::string& childName);
};

class PropFile
{
public:
	PropFile() { Clear(); }
	void Clear();
	bool Load(const std::string& utf8Path, std::string* error);
	bool Parse(const std::string& text, std::string* error);
	std::string Serialize() const;	// always formats; 20 of the 22 shipped files come out identical
	// Until something is edited the bytes it was read with go back out, including the stray
	// tabs the other two have.
	bool Save(const std::string& utf8Path, std::string* error) const;
	bool Loaded() const { return !root.name.empty(); }

	XmlNode* Declare() { return root.Child("declare"); }
	const XmlNode* Declare() const { return root.Child("declare"); }
	XmlNode* Project() { return root.Child("project"); }
	// Index of the class a texture belongs to, or the default class. -1 without one.
	int ClassOfTexture(const std::string& cp949Texture) const;
	int DefaultClass() const;
	const XmlNode* ClassAt(int index) const;
	std::string Field(int classIndex, const std::string& field) const;	// falls back on default
	XmlNode* WepEntry(const std::string& wepName);
	void SyncCounts();

	std::string prolog;		// the <?xml ...?> line and line ending
	std::string lineEnd;
	XmlNode root;
	std::string path;
	bool dirty;
	std::string original;
};

// WPropertyInfo: the sort of ground a class's <pet> selects, by substring and in this order.
int PropGroundType(const std::string& pet);
const char* PropGroundName(int type);

#endif // PROPS_H
