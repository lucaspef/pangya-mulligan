#include "tcachemanager.h"

TexCacheManager::TexCacheManager()
	: m_aTexture(NULL),
	  m_nMaxCache(0),
	  m_CurCacheIdx(0),
	  m_texWidth(0),
	  m_texHeight(0)
{
}

TexCacheManager::~TexCacheManager()
{
	ClearAll();

	ReleaseAllTextures();
}

bool TexCacheManager::Init(int nMaxCache, int w, int h, int bw, int bh)
{
	m_nMaxCache = nMaxCache;
	m_CurCacheIdx = 0;

	m_texWidth = w;
	m_texHeight = h;

	m_Cache.Init(w, h, bw, bh);

	ReleaseAllTextures();

	m_aTexture = new int[nMaxCache];
	memset(m_aTexture, 0, sizeof(int) * nMaxCache);

	return true;
}

void TexCacheManager::ClearAll()
{
	m_Cache.Clear();
	m_CurCacheIdx = 0;
	m_infoMap.clear();
}

void TexCacheManager::Clear(int idx)
{
	m_Cache.Clear();

	for (CACHE_INFO_MAP::iterator it = m_infoMap.begin();
		it != m_infoMap.end();)
	{
		if ((*it).second.idx == idx)
		{
			CACHE_INFO_MAP::iterator itDel = it++;
			m_infoMap.erase(itDel);
		}
		else
			++it;
	}
}

const sTexCacheInfo* TexCacheManager::Get(const Bitmap& bitmap) const
{
	CACHE_INFO_MAP::const_iterator it = m_infoMap.find(&bitmap);
	if (it != m_infoMap.end())
	{
		return &(*it).second;
	}

	return NULL;
}

void TexCacheManager::UpdateTextureCacheInfo(const sTexCacheInfo& info)
{
	FillTexture(info);
}

int TexCacheManager::GetCacheTexture(int index)
{
	return m_aTexture[index];
}

void TexCacheManager::ReleaseAllTextures()
{
	if (m_aTexture)
	{
		for (int i = 0; i < m_nMaxCache; ++i)
		{
			if (g_resrcmng && m_aTexture[i])
			{
				g_resrcmng->Release(m_aTexture[i]);
				m_aTexture[i] = 0;
			}
		}

		if (m_aTexture)
		{
			delete[] m_aTexture;
			m_aTexture = NULL;
		}
	}
}

bool TexCacheManager::CreateTexture(unsigned long texIdx)
{
	Bitmap temp(m_texWidth, m_texHeight, 32);
	memset(temp.vram, 0, temp.Size());

	m_aTexture[texIdx] =
		g_resrcmng->UploadTexture(NULL, &temp, 0x80000000, NULL);

	return m_aTexture[texIdx] > 0;
}

void TexCacheManager::FillTexture(const sTexCacheInfo& info)
{
	tagRECT r;
	r.left = info.rcPixel.left;
	r.top = info.rcPixel.top;
	r.right = info.rcPixel.right;
	r.bottom = info.rcPixel.bottom;

	if (info.pBitmap->BitsPerPixel() == 32)
	{
		g_resrcmng->FixTexture(info.texHandle,
			const_cast<Bitmap*>(info.pBitmap), 0, &r);
	}
}

const sTexCacheInfo* TexCacheManager::Add(const Bitmap& bitmap)
{
	sTexCacheInfo& info = m_infoMap[&bitmap];

	if (m_Cache.Add(info, bitmap.Width(), bitmap.Height()) == true)
	{
		if (!m_aTexture[m_CurCacheIdx])
			CreateTexture(m_CurCacheIdx);

		info.Set(m_CurCacheIdx, bitmap, m_aTexture[m_CurCacheIdx]);
		FillTexture(info);
		return &info;
	}

	m_CurCacheIdx = (m_CurCacheIdx + 1) % m_nMaxCache;
	Clear(m_CurCacheIdx);

	// Clear() apaga do mapa toda entrada com idx == m_CurCacheIdx, inclusive a recem-criada (idx 0):
	// 'info' ficaria pendurada (D474: AV de escrita em no liberado em RectCache::Add)
	sTexCacheInfo& info2 = m_infoMap[&bitmap];
	if (m_Cache.Add(info2, bitmap.Width(), bitmap.Height()) == true)
	{
		if (!m_aTexture[m_CurCacheIdx])
			CreateTexture(m_CurCacheIdx);

		info2.Set(m_CurCacheIdx, bitmap, m_aTexture[m_CurCacheIdx]);
		FillTexture(info2);
		return &info2;
	}

	CACHE_INFO_MAP::iterator it = m_infoMap.find(&bitmap);
	if (it != m_infoMap.end())
		m_infoMap.erase(it);

	return NULL;
}

void TexCacheManager::RefreshTexCache(const Bitmap& bitmap)
{
	const sTexCacheInfo* info = Get(bitmap);
	if (info)
	{
		UpdateTextureCacheInfo(*info);
	}
}

void TexCacheManager::InvalidateCache(const Bitmap& bitmap)
{
	CACHE_INFO_MAP::iterator it = m_infoMap.find(&bitmap);
	if (it != m_infoMap.end())
	{
		if ((*it).second.idx == m_CurCacheIdx)
		{
			Clear((*it).second.idx);
		}
		else
		{
			m_infoMap.erase(it);
		}
	}
}
