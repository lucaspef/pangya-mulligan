#pragma once
#include <vector>
#include <map>
#include <string>
#include "wtypes.h"

class WOverlay;
class FrWndManager;

class FrEmoticon
{
public:
	FrEmoticon(FrWndManager* pManager);
	virtual ~FrEmoticon();
	bool Init();
	bool Draw(int icon, const WRect& dst, unsigned long diffuse, bool flip);
	bool Draw(int icon, float x, float y, unsigned long diffuse, bool flip);
	float PrintText11(const WPoint& pos, unsigned long align, const char* text,
		unsigned long emoDiffuse);
	float GetTextWidth11(const char* text);
	int GetIconIndex(const char* key);
	const char* GetIconName(int index);
	int GetIconNum();
	void GetAlias(std::string (*buffer)[2], int index);
	void SetClippingArea(WRect* clip);
	float GetTextWidth(const char* text);
	float PrintText(const WPoint& point, unsigned long style, const char* text,
		unsigned long color);
	int GetWidth() { return m_selWidth; }
	int GetHeight() { return m_selHeight; }
	void SetAnim(bool enable) { m_EnableAnim = enable; }
	void SetAnimTime(unsigned long time) { m_AnimStart = time; }

protected:
	struct sEmoticon
	{
		std::string Name;
		std::string Alias[2];
		WOverlay* Overlay;
		std::vector<int> Frames;
	};
	void ParseAnimFrame(const std::string& Frames, std::vector<int>& Vector);
	bool LoadXml(const char* FileName);
	int m_Fps;
	std::vector<sEmoticon*> m_Emoticons;
	std::map<std::string, WOverlay*> m_ResMap;
	bool m_EnableAnim;
	int m_AnimStart;
	WOverlay* m_overlay;
	int m_selWidth;
	int m_selHeight;
	int m_selXNum;
	int m_selYNum;
	int m_selNum;
	FrWndManager* m_pManager;
};
