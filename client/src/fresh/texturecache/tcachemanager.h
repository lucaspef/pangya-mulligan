#pragma once
#include "rectcache.h"
#include <map>

class Bitmap;

struct sTexCacheInfo : public RectCache::RectInfo
{
	int idx;
	const Bitmap* pBitmap;
	int texHandle;

	void Set(int cacheIdx, const Bitmap& srcBitmap, int handle)
	{
		idx = cacheIdx;
		pBitmap = &srcBitmap;
		texHandle = handle;
	}
};

class TexCacheManager
{
	enum
	{
		MAX_INFOLIST = 256
	};

public:
	TexCacheManager();
	virtual ~TexCacheManager();
	bool Init(int nMaxCache, int w, int h, int bw, int bh);
	void ClearAll();
	const sTexCacheInfo* Draw(const Bitmap& bitmap);
	void UpdateTextureCacheInfo(const sTexCacheInfo& info);
	void RefreshTexCache(const Bitmap& bitmap);
	void InvalidateCache(const Bitmap& bitmap);
	int GetCacheTexture(int index);

protected:
	void ReleaseAllTextures();
	bool CreateTexture(unsigned long texIdx);
	void FillTexture(const sTexCacheInfo& info);
	const sTexCacheInfo* Get(const Bitmap& bitmap) const;
	const sTexCacheInfo* Add(const Bitmap& bitmap);
	void Clear(int idx);

	int* m_aTexture;
	RectCache m_Cache;
	int m_nMaxCache;
	int m_CurCacheIdx;
	unsigned long m_texWidth;
	unsigned long m_texHeight;

	typedef std::map<const Bitmap*, sTexCacheInfo> CACHE_INFO_MAP;
	CACHE_INFO_MAP m_infoMap;
};

inline const sTexCacheInfo* TexCacheManager::Draw(const Bitmap& bitmap)
{
	const sTexCacheInfo* info = Get(bitmap);
	if (info)
		return info;

	return Add(bitmap);
}
