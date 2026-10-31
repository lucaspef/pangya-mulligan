#pragma once

#include <vector>

extern WResourceManager* g_resrcmng;

class CSea
{
public:
	CSea();
	~CSea();

	// 983: vetores com +4 bytes (std::vector de 16), camada de 0x30
	struct sLayer
	{
		WVector2D speed;
		unsigned long color;
		std::vector<WVector2D> uv;
		std::vector<WTVertex> vertex;
		int texHandle;
		sLayer() : texHandle(0) {}
		~sLayer()
		{
			if (g_resrcmng && texHandle)
				g_resrcmng->Release(texHandle);
		}
	};

	void SetArea(const std::vector<WVector>& area);
	void LoadTexture();
	void Process(float delta);
	void Render();

protected:
	void Init();
	void AddLayer(float speedU, float speedV, float scale, unsigned char alpha);
	const char* TexName(int layer);

	Waabb m_area;
	std::vector<WVector> m_point;
	std::vector<unsigned short> m_index; // triangulos do poligono
	sLayer m_layer[3];
	int m_layerNum;
	unsigned char m_reserved[0x34];
	float m_scale;
};
