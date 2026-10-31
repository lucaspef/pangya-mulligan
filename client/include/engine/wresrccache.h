#pragma once
#include "wlist.h"
#include "wresource.h"
#include <string.h>

template <class T>
class WResrcCache
{
public:
	struct w_origin
	{
		T resource;
		int count;
		int checkflag;
		char name[1];
	};

	struct w_cache
	{
		w_origin* origin;
		T resource;
	};

	__declspec(nothrow) WResrcCache()
		: resrcList(32, 32), originList(16, 32)
	{
	}

	__declspec(nothrow) ~WResrcCache()
	{
		w_origin* origin = originList.Start();
		while (origin)
		{
			delete[] (char*)origin; // 983: sem o g_mem
			origin = originList.Next();
		}
		w_cache* cache = resrcList.Start();
		while (cache)
		{
			delete[] (char*)cache; // 983: sem o g_mem
			cache = resrcList.Next();
		}
	}

	T FirstOrigin()
	{
		w_origin* origin = originList.Start();
		if (origin)
			return origin->resource;
		return 0;
	}

	T NextOrigin()
	{
		w_origin* origin = originList.Next();
		if (origin)
			return origin->resource;
		return 0;
	}

	T FirstResource()
	{
		w_cache* cache = resrcList.Start();
		if (cache)
			return cache->resource;
		return 0;
	}

	T NextResource()
	{
		w_cache* cache = resrcList.Next();
		if (cache)
			return cache->resource;
		return 0;
	}

	T FindOrigin(const char* name)
	{
		w_origin* origin = originList.Find(name);
		if (origin)
			return origin->resource;
		return 0;
	}

	char* FindIdleOrigin(char* first, int count)
	{
		for (w_origin* origin = originList.Start(); origin;
			origin = originList.Next())
		{
			if (origin->count == 0)
			{
				if (first)
				{
					if (strcmp(origin->name, first) == 0)
						first = 0;
				}
				else
				{
					if (origin->checkflag >= count)
						return origin->name;
					origin->checkflag++;
				}
			}
		}
		return 0;
	}

	void ClearOrigin(const char* name)
	{
		w_origin* origin = originList.Find(name);
		originList -= origin;
		delete[] (char*)origin; // 983: sem o g_mem
	}

	bool AddResource(const char* name, T resrc)
	{
		w_origin* origin = originList.Find(name);
		if (!origin)
		{
			AddOrigin(name, resrc);
			origin = originList.Find(name);
		}
		w_cache* cache = (w_cache*)new char[sizeof(w_cache)]; // 983: sem o g_mem
		cache->origin = origin;
		cache->resource = resrc;
		origin->count++;
		origin->checkflag = 0;
		resrcList.AddItem(cache, origin->name, false);
		if (origin->count > 1)
			return false;
		return true;
	}

	bool DeleteResource(T resrc)
	{
		for (w_cache* cache = resrcList.Start(); cache;
			cache = resrcList.Next())
		{
			if (cache->resource == resrc)
			{
				w_origin* origin = cache->origin;
				resrcList -= cache;
				delete[] (char*)cache; // 983: sem o g_mem
				origin->count--;
				if (origin->resource != resrc)
					return true;
				return false;
			}
		}
		return false;
	}

private:
	void AddOrigin(const char* name, T resrc)
	{
		w_origin* origin =
			(w_origin*)new char[strlen(name) + sizeof(w_origin)]; // 983: sem o g_mem
		strcpy(origin->name, name);
		origin->resource = resrc;
		origin->count = 0;
		origin->checkflag = 0;
		originList.AddItem(origin, origin->name, false);
	}

public:
	WList<w_cache*> resrcList;
	WList<w_origin*> originList;
};
