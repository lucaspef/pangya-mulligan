#pragma once

class CLensFlare;
class CPvsFxBox;

struct sCloud
{
	float speed;
	float scale;
	WVector pos;
	float alpha;
	bool bActive;
	int type;
	bool bFlip;

	sCloud() { }
};

class CSky
{
public:
	CSky();
	~CSky();

	void Clear();
	void LoadSky(const char* name, float turn);
	void ChangeWeatherTex(unsigned char weather);
	void ClearClouds();
	void RegisterCloud(char* name);
	void ShowClouds(bool bShow);
	void GenerateClouds(int num, float rate);
	void GenerateStars(int map);
	void GenerateMoons();
	void GenerateLensFlare();
	void ClearSky();
	void Process(float dt);
	void Render(WView* view);
	void RenderLensFlareOverlay983(WView* view);
	void RenderClouds983(WView* view);
	void UpdateBright();
	void SetColor(unsigned long color) { m_color = color; }

protected:
	void RenderUpAndBottom(WView* view);
	void RenderFarCylinder(WView* view);
	void RenderBillboardCloud(WView* view);
	void RenderFlatCloud(WView* view);
	void RenderNearCylinder(WView* view);

private:
	void LoadTexByWeather(const char* name, const char* type);
	int IsThereTexHandle(const char* type, int index);
	void SetSkyTex(int texHandle, unsigned char weather, unsigned char index);
	float CalcScaleByView(WView* view) const;
	float CalcClipFarByView(WView* view) const;

public:
	int m_texFar[5];
	int m_texNear[5];
	int m_texUp[2];
	int m_texDn[2];
	int m_bLoaded;
	WxBatchState m_batch[4];
	float m_cloudRadius;
	float m_cloudFar;
	float m_cloudHeight;
	int m_cloudTex;
	int m_cloudNum;
	sCloud m_clouds[50];
	int m_visibleCloudNum;
	float m_cloudInterval;
	float m_cloudTimer;
	unsigned long m_color; // 983: 0x8e4
	CLensFlare* m_pLensFlare;
	CPvsFxBox* m_pStars;
	CPvsFxBox* m_pMoons;
	float m_upRot;
	float m_nearRot;
	// 983: a classe acaba aqui (0x8fc = operator new do CGolfBg::OnLoad 0072eca0)
};
