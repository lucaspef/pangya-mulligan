#pragma once
#include "wmemblock.h"
#include <string.h>
#include <stdlib.h>

// 983 (s42): no exe a WList também usa malloc/free (como na dll); o g_mem saiu daqui
// (medido em WList::Alloc 00b57ac0, DelItem 00b5a740, AddItem 00b584d0: call [malloc]/[free] do MSVCR100)
#ifndef WLIST_ALLOC
#define WLIST_ALLOC(size) malloc(size)
#define WLIST_FREE(ptr) free(ptr)
#endif

template <class T>
class WList
{
public:
	struct listinfo
	{
		T item;
		char* keycode;
		bool alloc;
		listinfo* prev;
		listinfo* next;
		listinfo* hash;
	};

	__declspec(nothrow) WList(int len = 8, int hashNum = 0)
	{
		m_hash_list = 0;
		m_list = 0;
		m_idle = 0;
		m_pre_alloc = 0;
		m_blk_len = len;
		m_surf = 0;
		m_hashNum = hashNum;
		if (hashNum > 0)
		{
			int size = m_hashNum * sizeof(listinfo*);
#ifdef WANGREAL_DEVICE
			m_hash_list = (listinfo**)malloc(size);
#else
			m_hash_list = (listinfo**)WLIST_ALLOC(size);
#endif
			m_hash_mask = m_hashNum - 1;
			for (int i = 0; i < m_hashNum; ++i)
				m_hash_list[i] = 0;
		}
		else
		{
			m_hash_list = 0;
		}
	}
	__declspec(nothrow) ~WList()
	{
		while (m_pre_alloc)
		{
			listinfo* allocation = m_pre_alloc;
			m_pre_alloc = Unlink(m_pre_alloc, allocation);
#ifdef WANGREAL_DEVICE
			free(allocation);
#else
			WLIST_FREE(allocation);
#endif
		}
		if (m_hash_list)
		{
#ifdef WANGREAL_DEVICE
			free(m_hash_list);
#else
			WLIST_FREE(m_hash_list);
#endif
			m_hash_list = 0;
		}
	}
	T Start()
	{
		m_surf = m_list;
		return Next();
	}
	T Next()
	{
		if (m_surf)
		{
			T item = m_surf->item;
			m_surf = m_surf->next == m_list ? 0 : m_surf->next;
			return item;
		}
		return 0;
	}
	T End()
	{
		m_surf = m_list ? m_list->prev : 0;
		return Prev();
	}
	// HACK: never defined; declaration-count canary (wshadowview WRecvViewHw::Render)
	void RbPad0();
	void RbPad1();
	void RbPad2();
	T Start(void*& pos) const
	{
		pos = m_list;
		if (m_list)
			return m_list->item;
		return 0;
	}
	T Next(void*& pos) const
	{
		if (pos)
		{
			if (((listinfo*)pos)->next == m_list)
			{
				pos = 0;
			}
			else
			{
				pos = ((listinfo*)pos)->next;
				return ((listinfo*)pos)->item;
			}
		}
		return 0;
	}
	T End(void*& pos) const
	{
		pos = m_list ? m_list->prev : 0;
		if (m_list)
			return ((listinfo*)pos)->item;
		return 0;
	}
	T Prev(void*& pos) const
	{
		if (pos)
		{
			if (pos == m_list)
			{
				pos = 0;
			}
			else
			{
				pos = ((listinfo*)pos)->prev;
				return ((listinfo*)pos)->item;
			}
		}
		return 0;
	}
	T Find(const char* keycode) const
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
	void Reset()
	{
		if (m_list)
		{
			if (m_idle)
			{
				m_list->prev->next = m_idle->next;
				m_idle->next->prev = m_list->prev;
				m_idle->next = m_list;
				m_list->prev = m_idle;
				m_list = 0;
			}
			else
			{
				do
				{
					listinfo* list = Unlink(m_list, m_list);
					m_idle = Link(m_idle, m_list);
					m_list = list;
				} while (m_list);
			}
		}
		for (int i = 0; i < m_hashNum; ++i)
			m_hash_list[i] = 0;
	}
	void AddItem(const T& item, const char* keycode, bool alloc)
	{
		listinfo* info = Alloc();
		info->item = item;
		info->alloc = alloc;
		if (alloc)
		{
#ifdef WANGREAL_DEVICE
			info->keycode = (char*)malloc((int)strlen(keycode) + 1);
#else
			info->keycode = (char*)WLIST_ALLOC((int)strlen(keycode) + 1);
#endif
			strcpy(info->keycode, keycode);
		}
		else
		{
			info->keycode = (char*)keycode;
		}
		if (keycode)
			AddHash(HASHCODE(keycode), info);
		m_list = Link(m_list, info);
	}
	void operator+=(const T& item) { AddItem(item, 0, false); }
	void operator-=(const T& item) { DelItem(item); }

protected:
	void DelItem(const T& item)
	{
		listinfo* found = m_list;
		if (found)
		{
			do
			{
				if (found->item == item)
				{
					if (m_surf == found)
						m_surf = m_surf->next == m_list ? 0 : m_surf->next;
					if (found->keycode)
					{
						DelHash(found);
						if (found->alloc)
#ifdef WANGREAL_DEVICE
							free(found->keycode);
#else
							WLIST_FREE(found->keycode);
#endif
					}
					m_list = Unlink(m_list, found);
					m_idle = Link(m_idle, found);
					if (!m_list)
						m_surf = 0;
					break;
				}
				found = found->next;
			} while (found != m_list);
		}
	}
	listinfo* Link(listinfo* head, listinfo* item)
	{
		if (!head)
		{
			item->prev = item;
			item->next = item;
			return item;
		}
		item->next = head;
		item->prev = head->prev;
#ifdef WANGREAL_DEVICE
		item->next->prev = item;
#else
		head->prev = item;
#endif
		item->prev->next = item;
		return head;
	}
	listinfo* Unlink(listinfo* head, listinfo* item)
	{
		item->next->prev = item->prev;
		item->prev->next = item->next;
		if (item != head)
			return head;
		if (item->next == head)
			return 0;
		return item->next;
	}

private:
	listinfo* m_list;
	listinfo* m_surf;
	listinfo* m_pre_alloc;
	listinfo* m_idle;
	int m_blk_len;
	int m_hash_mask;
	int m_hashNum;
	listinfo* Alloc()
	{
		if (!m_idle)
		{
			listinfo* block =
#ifdef WANGREAL_DEVICE
				(listinfo*)malloc(m_blk_len * sizeof(listinfo));
#else
				(listinfo*)WLIST_ALLOC(m_blk_len * sizeof(listinfo));
#endif
			m_pre_alloc = Link(m_pre_alloc, block);
			for (int i = 1; i < m_blk_len; ++i)
				m_idle = Link(m_idle, block + i);
		}
		listinfo* item = m_idle;
		m_idle = Unlink(m_idle, item);
		return item;
	}
	listinfo** m_hash_list;
	void AddHash(int hashCode, listinfo* item)
	{
		item->hash = m_hash_list[hashCode];
		m_hash_list[hashCode] = item;
	}
	void DelHash(listinfo* item)
	{
		int hashCode = HASHCODE(item->keycode);
		listinfo* current = m_hash_list[hashCode];
		if (current == item)
		{
			m_hash_list[hashCode] = item->hash;
			return;
		}
		if (current)
		{
			while (current)
			{
				if (current->hash == item)
				{
					current->hash = item->hash;
					break;
				}
				current = current->hash;
			}
		}
	}
	int HASHCODE(const void* keycode) const
	{
		const unsigned char* string = (const unsigned char*)keycode;
		int length = (int)strlen((const char*)string);
		int last;
		if (length > 0)
			last = length - 1;
		else
			last = 0;
		return (string[last] & 0x15 | string[0] & 0x2a | length * 0x40) &
			m_hash_mask;
	}

public:
	T Prev()
	{
		if (m_surf)
		{
			T item = m_surf->item;
			m_surf = m_surf == m_list ? 0 : m_surf->prev;
			return item;
		}
		return 0;
	}
	int GetCount() const
	{
		int count = 0;
		listinfo* item = m_list;
		while (item)
		{
			item = m_list == item->next ? 0 : item->next;
			++count;
		}
		return count;
	}
};
