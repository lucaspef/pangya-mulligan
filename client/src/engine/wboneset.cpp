#include "wboneset.h"

// HACK: Workaround to preserve original COMDAT order

template <>
WBoneKey* WList<WBoneKey*>::Next()
{
	if (m_surf)
	{
		WBoneKey* item = m_surf->item;
		m_surf = m_surf->next == m_list ? 0 : m_surf->next;
		return item;
	}
	return 0;
}

template <>
void WList<WBoneKey*>::AddHash(int hashCode, listinfo* item)
{
	item->hash = m_hash_list[hashCode];
	m_hash_list[hashCode] = item;
}

template <>
WList<WBoneKey*>::listinfo* WList<WBoneKey*>::Alloc()
{
	if (!m_idle)
	{
		listinfo* block = (listinfo*)WLIST_ALLOC(m_blk_len * sizeof(listinfo));
		m_pre_alloc = Link(m_pre_alloc, block);
		for (int i = 1; i < m_blk_len; ++i)
			m_idle = Link(m_idle, block + i);
	}
	listinfo* item = m_idle;
	m_idle = Unlink(m_idle, item);
	return item;
}

template <>
WBoneKey* WList<WBoneKey*>::Find(const char* keycode) const
{
	if (!keycode || !m_hash_list)
		return 0;
	listinfo* item = m_hash_list[HASHCODE(keycode)];
	while (item)
	{
		if (strcmp(item->keycode, keycode) == 0)
			return item->item;
		item = item->hash;
	}
	return 0;
}

void WBoneSet::AddBoneKey(char* name, WQuat* quat, WVector* pivot,
	WVector* scale, int flags)
{
	WBoneKey* key = m_keyList.Find(name);
	if (!key)
	{
		// 983 00b4f0e0 grava em begin()+n*0x30 sem limite; o D528 cortava em size() e o conjunto dimensionado pela contagem
		// de ossos perdia as chaves seguintes (esqueleto vazio na partida). Corta so alem da memoria alocada (heap alheio, C425).
		if ((size_t)m_keyList.GetCount() >= m_data.capacity())
			return;
		key = &m_data[m_keyList.GetCount()];
		key->name = name;
		m_keyList.AddItem(key, name, false);
	}
	if (flags & 1)
		key->quat = *quat;
	if (flags & 2)
		key->pivot = *pivot;
	if (flags & 4)
		key->scale = *scale;
	key->flags = (unsigned char)flags;
}

WBoneKey* WBoneSet::FindBoneKey(char* name)
{
	return m_keyList.Find(name);
}

void WBoneSet::AddBoneKey(WBoneKey* key)
{
	AddBoneKey(key->name, &key->quat, &key->pivot, &key->scale, key->flags);
}

void WBoneSet::CopyTo(WBoneSet* dest)
{
	for (WBoneKey* key = m_keyList.Start(); key; key = m_keyList.Next())
		dest->AddBoneKey(key->name, &key->quat, &key->pivot, &key->scale,
			key->flags);
}

// 983 (00b4f250): mistura as chaves deste conjunto nas de dest; ratio 1 = copia.
void WBoneSet::BlendTo(WBoneSet* dest, float ratio)
{
	if (Wabs(ratio - 1.0) < g_EPSILON)
	{
		CopyTo(dest);
		return;
	}
	for (WBoneKey* key = m_keyList.Start(); key; key = m_keyList.Next())
	{
		WBoneKey* out = dest->m_keyList.Find(key->name);
		if (!out)
		{
			dest->AddBoneKey(key->name, &key->quat, &key->pivot, &key->scale,
				key->flags);
			continue;
		}
		int flags = out->flags | key->flags;
		if (flags & 1)
		{
			if (out->flags & key->flags & 1)
				out->quat =
					WQuaternionSlerp(key->quat, out->quat, 1.0f - ratio);
			else
				out->quat = (out->flags & 1) ? out->quat : key->quat;
		}
		if (flags & 2)
		{
			if (key->flags & out->flags & 2)
				out->pivot = key->pivot * ratio + out->pivot * (1.0f - ratio);
			else
				out->pivot = (out->flags & 2) ? out->pivot : key->pivot;
		}
		if (flags & 4)
		{
			if (key->flags & out->flags & 4)
				out->scale = key->scale * ratio + out->scale * (1.0f - ratio);
			else
				// 983 testa o bit 2 (pivot) aqui, nao o 4 (00b4f48e): mantido como no binario
				out->scale = (out->flags & 2) ? out->scale : key->scale;
		}
		out->flags |= flags;
	}
}

void __fastcall WBoneSet::Blend(WBoneSet* dest, WBoneSet* first,
	WBoneSet* second, float ratio)
{
	if (dest != first)
	{
		dest->Clear();
		first->BlendTo(dest, 1.0f);
	}
	second->BlendTo(dest, ratio);
}

WBoneSet::WBoneSet()
	: m_keyList(32, 256)
{
	// B1133: o limite do D528 em AddBoneKey descartava todas as chaves dos WBoneSet locais (wpuppet.cpp) com m_data vazio -> personagem/taco/caddie sem esqueleto na partida
	m_data.resize(512);
}

WBoneSet::WBoneSet(int count)
	: m_keyList(32, 256)
{
	Clear();
	m_data.resize(count);
}
