#pragma once

class WTextureView;

class CPool
{
public:
	CPool();
	virtual ~CPool();

	static CPool* CreatePoolByMapType(eMapType type);

	void SetArea(const Waabb& area, const char* option);

	virtual void SetReflection(bool bEnable) = 0;
	virtual void Process(float delta) = 0;
	virtual void Prerender(WView* view) = 0;
	virtual void Render(WView* view) = 0;

private:
	virtual void BuildMesh() = 0;

protected:
	int m_type;
	Waabb m_area;
	WFlags m_flags;
};

class CPoolCaustic : public CPool
{
public:
	CPoolCaustic();
	virtual ~CPoolCaustic();

	virtual void SetReflection(bool bEnable) { }
	virtual void Process(float delta);
	virtual void Prerender(WView* view) { }
	virtual void Render(WView* view);

private:
	virtual void BuildMesh();
	void AddLayer(float speedU, float speedV, float scale, unsigned char alpha);

	struct sLayer
	{
		WVector2D speed;
		WTVertex* ppVertex[5];
		WTVertex vertex[4];
		int texHandle;

		sLayer() { texHandle = 0; }

		~sLayer()
		{
			if (g_resrcmng && texHandle)
			{
				g_resrcmng->Release(texHandle);
				texHandle = 0;
			}
		}
	};

	Waabb m_box;
	WVector m_corner[4];
	sLayer m_layer[3];
	int m_layerNum;
};

class CPoolReflective : public CPool
{
public:
	struct sWaveVertex
	{
		WVector pos;
		WVector normal;
		float force;
		float velocity;
		float u;
		float v;
		bool active;
	};

	CPoolReflective();
	virtual ~CPoolReflective();

	virtual void SetReflection(bool bEnable);
	virtual void Process(float delta);
	virtual void Prerender(WView* view);
	virtual void Render(WView* view);

private:
	virtual void BuildMesh();
	void Droplet();
	void MakeNorm();
	void Wave(float delta);
	void ExcludePoint();

	int m_width;
	int m_height;
	sWaveVertex** m_ppVertex;
	unsigned long m_dropTime;
	WTVertex* m_ppVtx[5];
	WTVertex m_vtx[4];
	WTextureView* m_pTexView;
};
