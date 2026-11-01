// SPDX-License-Identifier: MIT
// wardrobe.h - dressing a character from its part files, using only their names.
//
// The game decides this from item data (PosMask, HideMask). The names follow the same
// structure closely enough to guess from: <char>_<slot>_<id>[_!hide...].mpet.
#ifndef WARDROBE_H
#define WARDROBE_H

#include <string>
#include <vector>

class Document;
struct MeshPart;

struct WardrobeItem
{
	std::string file;				// UTF-8, "i_ts_04_!pv!lg.mpet"
	std::string slot;				// "ts"
	std::string id;					// "04"
	std::vector<std::string> hides;	// "pv", "lg"
	bool extra;						// stacks instead of filling a slot
};

struct WardrobeSlot
{
	std::string slot;
	std::vector<int> choices;		// item indices
	int selected;					// item index, -1 for empty
	bool forceShow;					// wearing it anyway, whatever hides it
	std::string variant;			// colour "02", empty for as authored
};

class Wardrobe
{
public:
	Wardrobe() { }
	void Clear();
	// Lists <prefix>_*.mpet in dir, and wears the _def set that OpenPuppet loaded.
	void Scan(const std::string& dir, const std::string& prefix);
	bool Active() const { return !items.empty(); }

	int SlotIndex(const std::string& slot) const;
	int ItemIndex(const std::string& file) const;
	// Fills the slot or ticks the extra the file belongs to. False if not a known part.
	bool Wear(const std::string& file);
	void ResetToDefaults();
	bool IsWorn(int item) const;
	// The item hiding a slot's selection, or -1.
	int HiddenBy(int slot) const;
	int ExtraHiddenBy(int item) const;

	// Makes the document's parts match. onRemove runs before a part is destroyed.
	void Sync(Document* doc, void (*onRemove)(MeshPart*));

	std::string prefix;
	std::vector<WardrobeItem> items;
	std::vector<WardrobeSlot> slots;
	std::vector<bool> extraOn;		// per item
};

// Public so the rules can be checked on their own.
bool ParsePartName(const std::string& file, const std::string& prefix, WardrobeItem* out);
// "[2#i_ts_01-01b.jpg" with "02" gives "[2#i_ts_01-02b.jpg"; empty if it has no colour number.
std::string TextureVariantName(const std::string& name, const std::string& variant);
// The number in that position, or empty.
std::string TextureVariantOf(const std::string& name);

#endif // WARDROBE_H
