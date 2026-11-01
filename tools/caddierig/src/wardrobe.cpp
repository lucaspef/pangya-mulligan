// SPDX-License-Identifier: MIT
// wardrobe.cpp - see the header. The rules below come from comparing names and vertex
// counts across the shipped characters; none of them is stated anywhere by the game.
#include "wardrobe.h"
#include <windows.h>
#include <string.h>
#include <algorithm>
#include "document.h"

static const char* const BASE_SLOTS[] = { "ts", "pv", "am", "lg", "hn", "ft", "ha", "fc" };

static std::string Lower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

static bool IsBaseSlot(const std::string& slot)
{
	for (size_t i = 0; i < sizeof(BASE_SLOTS) / sizeof(BASE_SLOTS[0]); ++i)
		if (slot == BASE_SLOTS[i])
			return true;
	return false;
}

bool ParsePartName(const std::string& file, const std::string& prefix, WardrobeItem* out)
{
	std::string n = Lower(file);
	const std::string ext = ".mpet";
	std::string head = Lower(prefix) + "_";
	if (n.size() <= ext.size() || n.compare(n.size() - ext.size(), ext.size(), ext) || n.compare(0, head.size(), head))
		return false;
	n.erase(n.size() - ext.size());
	n.erase(0, head.size());

	out->file = file;
	out->hides.clear();
	size_t bang = n.find('!');
	if (bang != std::string::npos)
	{
		size_t p = bang;
		while (p != std::string::npos)
		{
			size_t next = n.find('!', p + 1);
			std::string hide = n.substr(p + 1, next == std::string::npos ? std::string::npos : next - p - 1);
			hide.erase(std::remove(hide.begin(), hide.end(), ' '), hide.end());		// "_!am .mpet" exists
			out->hides.push_back(hide);
			p = next;
		}
		n.erase(bang);
		while (!n.empty() && n[n.size() - 1] == '_')
			n.erase(n.size() - 1);
	}
	size_t us = n.find('_');
	out->slot = n.substr(0, us);
	out->id = us == std::string::npos ? std::string() : n.substr(us + 1);

	// Not a piece of the doll at all: wings, tails, props.
	out->extra = !IsBaseSlot(out->slot) || out->id.empty();
	// Alternate partial pieces the game itself picks when parts overlap (CPartList::SetPart).
	if (out->id.find("sub") != std::string::npos)
		out->extra = true;
	// On heads, the a, a_z and z series are accessories. They average far fewer vertices
	// than a full hair or face, and carry !ha or !fc when they should cover the original.
	if ((out->slot == "ha" || out->slot == "fc") && !out->id.empty() && (out->id[0] == 'a' || out->id[0] == 'z'))
		out->extra = true;
	return true;
}

static size_t VariantPos(const std::string& name)
{
	for (size_t p = name.rfind('-'); p != std::string::npos && p > 0; p = name.rfind('-', p - 1))
	{
		if (p + 2 < name.size() && isdigit((unsigned char)name[p + 1]) && isdigit((unsigned char)name[p + 2]) &&
			(p + 3 >= name.size() || !isdigit((unsigned char)name[p + 3])))
			return p + 1;
	}
	return std::string::npos;
}

std::string TextureVariantOf(const std::string& name)
{
	size_t p = VariantPos(name);
	return p == std::string::npos ? std::string() : name.substr(p, 2);
}

std::string TextureVariantName(const std::string& name, const std::string& variant)
{
	size_t p = VariantPos(name);
	if (p == std::string::npos)
		return std::string();
	std::string out = name;
	out.replace(p, 2, variant);
	return out;
}

void Wardrobe::Clear()
{
	prefix.clear();
	items.clear();
	slots.clear();
	extraOn.clear();
}

void Wardrobe::Scan(const std::string& dir, const std::string& charPrefix)
{
	Clear();
	prefix = charPrefix;
	for (size_t i = 0; i < sizeof(BASE_SLOTS) / sizeof(BASE_SLOTS[0]); ++i)
	{
		WardrobeSlot s;
		s.slot = BASE_SLOTS[i];
		s.selected = -1;
		s.forceShow = false;
		slots.push_back(s);
	}
	std::string pattern = PathJoin(dir, prefix + "_*.mpet");
	int n = MultiByteToWideChar(CP_UTF8, 0, pattern.c_str(), -1, 0, 0);
	std::wstring w(n, L' ');
	MultiByteToWideChar(CP_UTF8, 0, pattern.c_str(), -1, &w[0], n);
	WIN32_FIND_DATAW fd;
	HANDLE h = FindFirstFileW(w.c_str(), &fd);
	if (h == INVALID_HANDLE_VALUE)
		return;
	do
	{
		int m = WideCharToMultiByte(CP_UTF8, 0, fd.cFileName, -1, 0, 0, 0, 0);
		std::string name(m > 0 ? m - 1 : 0, ' ');
		if (m > 1)
			WideCharToMultiByte(CP_UTF8, 0, fd.cFileName, -1, &name[0], m, 0, 0);
		WardrobeItem item;
		if (ParsePartName(name, prefix, &item))
			items.push_back(item);
	} while (FindNextFileW(h, &fd));
	FindClose(h);

	extraOn.assign(items.size(), false);
	for (size_t i = 0; i < items.size(); ++i)
	{
		int s = SlotIndex(items[i].slot);
		if (s >= 0 && !items[i].extra)
			slots[s].choices.push_back((int)i);
	}
	ResetToDefaults();
}

void Wardrobe::ResetToDefaults()
{
	extraOn.assign(items.size(), false);
	for (size_t s = 0; s < slots.size(); ++s)
	{
		slots[s].selected = -1;
		slots[s].forceShow = false;
		slots[s].variant.clear();
	}
	// Exactly what Document::OpenPuppet attaches: <prefix>_*_def.mpet
	for (size_t i = 0; i < items.size(); ++i)
	{
		if (items[i].id != "def")
			continue;
		int s = SlotIndex(items[i].slot);
		if (s >= 0 && !items[i].extra)
			slots[s].selected = (int)i;
		else
			extraOn[i] = true;
	}
}

int Wardrobe::SlotIndex(const std::string& slot) const
{
	for (size_t i = 0; i < slots.size(); ++i)
		if (slots[i].slot == slot)
			return (int)i;
	return -1;
}

int Wardrobe::ItemIndex(const std::string& file) const
{
	for (size_t i = 0; i < items.size(); ++i)
		if (!_stricmp(items[i].file.c_str(), file.c_str()))
			return (int)i;
	return -1;
}

bool Wardrobe::Wear(const std::string& file)
{
	int i = ItemIndex(file);
	if (i < 0)
		return false;
	int s = SlotIndex(items[i].slot);
	if (s >= 0 && !items[i].extra)
	{
		slots[s].selected = i;
		slots[s].variant.clear();
	}
	else
		extraOn[i] = true;
	return true;
}

bool Wardrobe::IsWorn(int item) const
{
	if (extraOn[item])
		return true;
	for (size_t s = 0; s < slots.size(); ++s)
		if (slots[s].selected == item)
			return true;
	return false;
}

// CPetFrame::ApplyHideMask also never lets a part hide itself.
int Wardrobe::HiddenBy(int slot) const
{
	const WardrobeSlot& target = slots[slot];
	if (target.selected < 0)
		return -1;
	for (size_t i = 0; i < items.size(); ++i)
	{
		if ((int)i == target.selected || !IsWorn((int)i))
			continue;
		for (size_t h = 0; h < items[i].hides.size(); ++h)
			if (items[i].hides[h] == target.slot)
				return (int)i;
	}
	return -1;
}

// Hides name a slot; an add-on in a doll slot isn't what they mean, so only extras
// outside those slots ("!tl", "!hb") can be hidden.
int Wardrobe::ExtraHiddenBy(int item) const
{
	if (SlotIndex(items[item].slot) >= 0)
		return -1;
	for (size_t i = 0; i < items.size(); ++i)
	{
		if ((int)i == item || !IsWorn((int)i))
			continue;
		for (size_t h = 0; h < items[i].hides.size(); ++h)
			if (items[i].hides[h] == items[item].slot)
				return (int)i;
	}
	return -1;
}

void Wardrobe::Sync(Document* doc, void (*onRemove)(MeshPart*))
{
	BuildLog log;
	for (size_t p = doc->parts.size(); p-- > 0;)
	{
		MeshPart* part = doc->parts[p];
		int i = ItemIndex(part->name);
		if (i >= 0 && !IsWorn(i))
		{
			if (onRemove)
				onRemove(part);
			doc->RemovePart(p);
		}
	}
	for (size_t i = 0; i < items.size(); ++i)
	{
		if (!IsWorn((int)i))
			continue;
		MeshPart* part = 0;
		for (size_t p = 0; p < doc->parts.size(); ++p)
			if (!_stricmp(doc->parts[p]->name.c_str(), items[i].file.c_str()))
				part = doc->parts[p];
		if (!part)
			doc->AttachPart(PathJoin(doc->baseDir, items[i].file), &log);
	}
	for (size_t p = 0; p < doc->parts.size(); ++p)
	{
		MeshPart* part = doc->parts[p];
		int i = ItemIndex(part->name);
		if (i < 0)
			continue;
		int s = SlotIndex(items[i].slot);
		if (s >= 0 && slots[s].selected == i)
			part->visible = slots[s].forceShow || HiddenBy(s) < 0;
		else
			part->visible = ExtraHiddenBy(i) < 0;
	}
}
