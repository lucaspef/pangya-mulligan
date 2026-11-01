// SPDX-License-Identifier: MIT
// wep.h - the WangED project file (.wep).
//
// Every field keeps the text it was read with, separators included, and only a value
// that gets edited is formatted again. An unmodified project saves back byte for byte.
// Strings stay in the game's CP949.
#ifndef WEP_H
#define WEP_H

#include <string>
#include <vector>
#include "wmath.h"

struct WepField
{
	std::string key, sep, value;	// value without its quotes when quoted is set
	bool quoted;
};

struct WepFields
{
	std::vector<WepField> fields;

	const WepField* Find(const std::string& key) const;
	bool Has(const std::string& key) const { return Find(key) != 0; }
	std::string Get(const std::string& key) const;
	int GetInt(const std::string& key, int def = 0) const;
	float GetFloat(const std::string& key, float def = 0.0f) const;
	WVector GetVec(const std::string& key) const;
	// New keys go in at their place in WangED's key order for the element's type.
	void Set(const std::string& key, const std::string& value);
	void SetVec(const std::string& key, const WVector& v, int decimals = 2);
	void SetFloat(const std::string& key, float f, int decimals = 2);
	void SetInt(const std::string& key, int i);
	void SetColor(const std::string& key, int r, int g, int b);
	void Remove(const std::string& key);
};

struct WepElement : public WepFields
{
	std::string Type() const { return Get("Type"); }
	std::string Class() const { return Get("Class"); }
	bool IsPuppet() const { return Type() == "PUPPET"; }
	bool IsGround() const { return Class() == "WGround"; }
	bool IsPoint() const { return Type() == "POINT"; }
	bool IsCamera() const { return Type() == "CAMERA"; }
	bool IsArea() const { return Type() == "AREA"; }
	bool IsGame() const { return Type() == "GAME"; }

	// Areas store two corners on one line, separated by a tab.
	void GetBox(WVector* lo, WVector* hi) const;
	void SetBox(const WVector& lo, const WVector& hi);
};

struct WepNodePoint
{
	std::string guid;		// the *GUID line before it, V1.30 only
	std::string text;		// the coordinates as read
	WVector pos;
	void Set(const WVector& v);
};

struct WepNode : public WepFields
{
	std::vector<WepNodePoint> points;
};

class WepFile
{
public:
	WepFile() { Clear(); }
	void Clear();
	void NewEmpty();

	bool Load(const std::string& utf8Path, std::string* error);
	bool Parse(const std::string& text, std::string* error);
	std::string Serialize() const;
	bool Save(const std::string& utf8Path, std::string* error) const;

	WepElement* Game();
	const WepElement* Game() const;
	const WepElement* Ground() const;
	int FindElement(const std::string& cls, const std::string& cp949Name) const;
	// The *NumPuppet list holds what the elements use, in first-use order for new entries.
	void SyncPuppetList();
	int InsertElement(const WepElement& e);					// returns its index
	std::string PuppetFile(const std::string& cp949Name) const;	// the path a new element should use

	std::string header;		// the # lines and blank lines before *RootDir
	std::string rootDir;
	std::vector<std::string> puppets;
	int visGroups;
	std::vector<WepNode> nodes;
	std::vector<WepElement> elements;
	std::string trailer;	// "#End of..." to the end of the file
	bool lfOnly;			// the one project with bare LF line ends and hand-trimmed gaps
	bool utf8;				// converted to UTF-8 at some point after WangED wrote it; held as CP949

	// Blank runs between sections; V1.20 and V1.30 agree, a stray file may not.
	std::string gapAfterRoot, gapAfterPuppets, gapAfterVis, gapAfterNodeCount, gapAfterElementCount;
};

WepElement WepMakeElement(const char* type, const char* cls);
WepNode WepMakeNode(const std::string& cp949Name);
// The matrix rule found by comparing 445 tilted puppets with their shipped .gbin.
WMatrix WepPuppetMatrix(const WVector& pos, const WVector& rot, float scale);
WVector WepRotFromMatrix(const WMatrix& m, float* scale);

#endif // WEP_H
