#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>
#include "nodecontainer.h"
#include "scenemanager.h"

class CGolfDoc;
class CFxSpray;
class CFxSequence;
class WBone;
class WPuppet;
class WxStaticPuppetGrp;
class cFile;
struct w_mesh;
struct w_bound_box;
struct sProperty;

struct sSoundBox
{
	char name[32];
	float time;
	float interval;
	float distance;
	Wobb obb;
};

// 983: cores de vertice do modelo base lidas do mapa e guardadas por osso/malha (WPolySoup +0x49b0); aplicadas no
// PostBaseModel983 e no ReloadBasePet. Nomes meus.
struct sVertexColorArray983
{
	std::vector<unsigned long> colors;

	void Resize(unsigned int num);                     // 983: 009372a0
	void Set(unsigned int index, unsigned long color); // 983: 0092caa0
	void ApplyTo(w_mesh* mesh);                        // 983: 0092cac0
};

class cVertexColorTable983
{
public:
	typedef std::map<unsigned int, sVertexColorArray983> MeshColors;

	std::map<std::string, MeshColors> m_bones;

	sVertexColorArray983& Get(const std::string& bone, unsigned int mesh, unsigned int vtxNum); // 983: 00948730
	void Read(int version, cFile* file, WPuppet* pet); // 983: 00948bf0
	void Apply(WPuppet* pet);                          // 983: 009372f0
};

class WPolySoup : public WResource
{
	friend class CSky;
	friend class CQuadTree; // A728: crus do quadtree.cpp viram membros fora do VC10
	friend class CAmbientSound; // A728: idem SOUNDBOX_LIST do ambientsound.cpp
	friend class CGolfRule; // B1456: crus +0x98/+0xc0/+0xe0 do golfrule.cpp viram membros fora do VC10
	friend class CGolfRule30Battle; // B1573: GetShortGameBallPos983 (cru +0x110..0x124 do VC10)

public:
	WPolySoup(CGolfDoc* pDoc);
	virtual ~WPolySoup();

	bool Load(const char* filename, bool bOption);
	bool LoadMapCheckData(unsigned char hole);
	// JP 983: 00941600 / 009418a0 -- leem o mapa do buraco inteiro (sMapData983 dono) e devolvem (x, z) do osso de chao
	// do pin e do tee mais longe ate `range` (o mais perto acima, se nao houver) / de todos os tees. Nomes meus.
	bool GetTeePinByRange983(unsigned char hole, WVector2D* pPin, float range, WVector2D* pTee);
	void GetPinAndTees983(unsigned char hole, WVector2D* pPin, std::vector<WVector2D>* pTees);
	void EndLoad(bool bPlaceHoleCup);
	void Clear();

	void MakeDataStructure();
	void MakeStatistics();
	void UpdateBright();
	void UpdateBgFxWind();
	void EnableShadowmap(bool bEnable);
	void SetRenderMode(int mode);
	void SetCullDistance(float dist) { m_cullDistance = dist; }
	bool IsOutOfBound(const WVector& pos);
	void ChangeFloorTexture(const char* from, const char* to);
	void ChangeObjectTexture(const char* from, const char* to, int index);
	Waabb GetExtraHoleStartArea() { return m_extraHoleStartArea; }
	void ChangePointType(unsigned char from, unsigned char to);
	bool IsResisteredBgSequence(const std::string& name);
	int GetPropertyIndex(const char* name);
	const sProperty& GetProperty(int index);

	// 983: par de inteiros que os registros do mapa versao >= 0x72 trazem no fim (limpeza 00926d40 = -1, -1;
	// leitura 00926d50): id do registro e id do pai na hierarquia montada por sMapData983::BuildNodes983 (0093d870;
	// -1 = sem id / filho da raiz). Nomes meus.
	struct sLink983
	{
		int id;
		int parentId;

		__declspec(noinline) sLink983();     // 983: 0092af70
		__declspec(noinline) void Clear();   // 983: 00926d40
		void Read(int version, cFile* file);
	};

	struct sHeader
	{
		char id[4];
		unsigned long version;
		int modelNum;
		int modelNum1;
		int modelNum2;
		int cameraNum;
		int pointNum;
		int areaNum;
		int textureNum;
		int nodeNum;
		int attachNum983; // 983: +0x28, so no arquivo versao >= 0x72 (00926ce0); no de sAttach983; nome meu

		__declspec(noinline) sHeader();                      // 983: 0092af50 (base 0092af40 = ClearBase; depois Clear)
		__declspec(noinline) void ClearBase();               // 983: 00926c90
		__declspec(noinline) void ReadBase(cFile* file);     // 983: 00926cb0
		__declspec(noinline) void Clear();                   // 983: 00926cd0
		__declspec(noinline) void Read(cFile* file);         // 983: 00926ce0
	};
	struct sTriangle
	{
		WVector v[3];
		WPlane plane;
		bool bDoubleSide;
		int texHandle;
		int propIndex;
		bool bEdge;
		WBone* bone;

		sTriangle()
			: texHandle(0), propIndex(0), bone(NULL)
		{
		}
	};

	// 983: hierarquia dos registros do mapa (RTTI .?AVsNode@WPolySoup@@ e .?AU*Node@WPolySoup@@; vtable de 7 entradas,
	// sem destrutor virtual). Cada camera/ponto/area/modelo/osso ganha um no em BuildNodes983 (0093d870); o no guarda a
	// matriz local (lida do registro) e escreve a do mundo de volta no registro em UpdateWorld.
	struct sNode
	{
		sNode* m_parent;               // +0x04
		std::list<sNode*> m_children;  // +0x08 (cabeca +0x08, tamanho +0x0c)
		WMatrix m_local;               // +0x14
		WMatrix m_world;               // +0x44

		sNode();                                         // 983: 00936e30
		virtual int GetType() = 0;                       // +0x00 (raiz 0, camera 1, ponto 2, area 3, modelo 4, osso 5)
		virtual int GetId() = 0;                         // +0x04 (link983.id do registro)
		virtual int GetParentId() = 0;                   // +0x08 (link983.parentId do registro)
		virtual void SetLocal(const WMatrix& mat);       // +0x0c 983: 00936eb0
		virtual bool IsPin983();                       // +0x10 983: 00936f10 (so o osso responde: buraco; nome meu)
		virtual bool IsTee983();                       // +0x14 983: 00936f20 (idem: tee)
		virtual void UpdateWorld(const WMatrix& parent); // +0x18 983: 0092b0c0

		__declspec(noinline) void SetParent(sNode* parent);  // 983: 009381e0
		__declspec(noinline) void RemoveChild(sNode* child); // 983: 009360a0
		__declspec(noinline) void AddChild(sNode* child);    // 983: 00936f30
		__declspec(noinline) sNode* Find(int id);            // 983: 00934a30 (recursivo)
		__declspec(noinline) void Update(const WMatrix& parent); // 983: 00934b10 (recursivo)
	};

	struct sRootNode : public sNode // 983: 0x74, vtable 00d3ce74 (embutido em sMapData983 +0x38)
	{
		virtual int GetType();       // 983: 00939330 (0)
		virtual int GetId();         // 983: 00939340 (0)
		virtual int GetParentId();   // 983: 00939350 (-1)
	};

	// 983: cada registro do mapa virou base (o registro do 645, com Clear/Read(versao, arquivo)) + parte do 983
	// (dados da versao 0x72, sLink983 e o no da hierarquia, apagado no Clear). Na versao < 0x72 a parte nova copia a da base.
	struct sCameraBase983 // 0x58 = o sCamera do 645
	{
		char name[32];
		WMatrix mat;
		float fov;
		float reserved;

		__declspec(noinline) sCameraBase983();        // 983: 0092f6b0
		__declspec(noinline) void Clear();                        // 983: 0092af80
		__declspec(noinline) void Read(int version, cFile* file); // 983: 0092afc0
	};

	struct sCamera : public sCameraBase983 // 983: 0x94 (new[] em 0093dfa0)
	{
		sLink983 link983;           // +0x58
		WMatrix mat2_983;           // +0x60 (eye/at proprios na 0x72; senao = mat)
		sNode* node983;            // +0x90

		__declspec(noinline) sCamera();                   // 983: 0093d530
		__declspec(noinline) ~sCamera() { Clear(); }     // 983: 0093d5b0 (thunk do Clear)
		__declspec(noinline) void Clear();                        // 983: 0093b960
		__declspec(noinline) void Read(int version, cFile* file); // 983: 0093ba10
	};

	struct sTexture
	{
		char name[32];
		int texHandle;
		int propIndex;

		// 983: 0092af60 -- o construtor so chama Clear (texHandle/propIndex ficam como vierem).
		sTexture() { Clear(); }

		__declspec(noinline) void Clear();                        // 983: 00926d10
		__declspec(noinline) void Read(int version, cFile* file); // 983: 00926d20
	};

	struct sPointBase983 // 0x4c = o sPoint do 645
	{
		unsigned char type;
		char name[32];
		WVector pos;
		std::string option;

		__declspec(noinline) sPointBase983();         // 983: 00936dd0
		__declspec(noinline) void Clear();                        // 983: 00935fd0
		__declspec(noinline) void Read(int version, cFile* file); // 983: 00935ff0
	};

	struct sPoint : public sPointBase983 // 983: 0x64 (new[] em 0093e080)
	{
		sLink983 link983;           // +0x4c
		WVector pos2_983;           // +0x54 (proprio na 0x72; senao = pos)
		sNode* node983;            // +0x60

		__declspec(noinline) sPoint();                    // 983: 0093d5c0
		__declspec(noinline) ~sPoint() { Clear(); }      // 983: 0093d620
		__declspec(noinline) void Clear();                        // 983: 0093bc60
		__declspec(noinline) void Read(int version, cFile* file); // 983: 0093bcd0
	};

	struct sAreaBase983 // 0x5c = o sArea do 645
	{
		unsigned char type;
		char name[64];
		Waabb aabb;

		__declspec(noinline) sAreaBase983();          // 983: 009328d0
		__declspec(noinline) void Clear();                        // 983: 0092f6c0
		__declspec(noinline) void Read(int version, cFile* file); // 983: 0092f6e0
	};

	struct sArea : public sAreaBase983 // 983: 0x80 (passo de m_area em FindArea 00927590 e no ResetHoleData 00960310)
	{
		sLink983 link983;           // +0x5c
		Waabb aabb2_983;            // +0x64 (propria na 0x72; senao = aabb)
		sNode* node983;            // +0x7c

		__declspec(noinline) sArea();                     // 983: 0093d570
		__declspec(noinline) ~sArea() { Clear(); }       // 983: 0093d5b0
		__declspec(noinline) void Clear();                        // 983: 0093bb70
		__declspec(noinline) void Read(int version, cFile* file); // 983: 0093bbf0
	};

	// 983: registro novo da versao 0x72 (0xb8; header.attachNum983 deles, em m_attach983 +0xc8; leitura 0093be30,
	// osso achado no boneco do cenario em 009272b0). Nomes meus.
	struct sAttach983
	{
		sLink983 link983;           // +0x00
		char name[64];              // +0x08 (nome do osso)
		WMatrix mat;                // +0x48
		WMatrix mat2;               // +0x78
		WBone* bone;                // +0xa8
		int value;                  // +0xac
		float time;                 // +0xb0
		sNode* node983;            // +0xb4

		__declspec(noinline) sAttach983();                // 983: 0093d690
		~sAttach983() { Clear(); }
		__declspec(noinline) void Clear();                        // 983: 0093bd50
		__declspec(noinline) void Read(int version, cFile* file); // 983: 0093be30
	};

	struct sBgFx
	{
		sArea* area;
		CFxSpray* spray;
		CFxSequence* sequence;
		WMatrix mat;
		WVector offset;

		sBgFx() : area(NULL), spray(NULL), sequence(NULL), mat(WMatrix::IDENTITY), offset(WVector::ZERO) { } // 983: inline no 00948fb0
	};

	struct sQuad
	{
		short meshNum;
		int* meshIndex;
		unsigned char objNum;
		short* objIndex;

		sQuad()
		{
			meshNum = 0;
			meshIndex = NULL;

			objNum = 0;
			objIndex = NULL;
		}

		~sQuad()
		{
			meshNum = 0;
			if (meshIndex)
			{
				delete[] meshIndex;
				meshIndex = NULL;
			}

			objNum = 0;
			if (objIndex)
			{
				delete[] objIndex;
				objIndex = NULL;
			}
		}
	};

	// 983: 0x154 (new em 0093e220; construtor 0093d1a0, destrutor 0093d2b0). Em relacao ao 645: sLink983 no inicio,
	// matriz local antes de mat, tri virou vector (+0x114; aqui ponteiro + reserva), dword sem uso medido em +0x120 e o no em +0x150.
	struct sModel
	{
		sLink983 link983;   // +0x00
		unsigned char type; // +0x08
		unsigned char flag;
		unsigned char noCollision;
		unsigned char alpha;
		float windFactor;
		float scale;
		WPuppet* pet;       // +0x14
		WMatrix localMat983; // +0x18 (matriz do registro na hierarquia; mat2_983 do arquivo)
		WMatrix mat;        // +0x48 (= mundo, escrita por sModelNode::UpdateWorld)
		char name[32];      // +0x78
		Waabb aabb;         // +0x98
		Waabb bbox;         // +0xb0

		__declspec(noinline) sModel();  // 983: 0093d1a0
		__declspec(noinline) ~sModel(); // 983: 0093d2b0

		Wobb obb;
		WVector center;
		float radius;
		WVector pos;                  // +0x108
		std::vector<sTriangle> tri;   // +0x114 (0x10; 983: clear no rebuild e resize(GetFaceNum()) no 00962ad0; o 645 tinha
		                              //         sTriangle* com new[]/delete[])
		sQuad* quad;                  // +0x124
		bool bSingleQuad;             // +0x128
		unsigned char hitCount;       // +0x129
		char option[32];              // +0x12a
		int id;                       // +0x14c
		sNode* node983;               // +0x150
	};

	// 983: nos de cada tipo de registro (construtores 00938210/009382c0/00938320/009383b0/00938450; tamanhos 0x78/0x7c).
	struct sCameraNode : public sNode // 0x78, vtable 00d3cdd4
	{
		sCamera* m_rec; // +0x74
		explicit sCameraNode(sCamera* rec);       // 983: 00938210
		virtual int GetType();                    // 983: 00938290 (1)
		virtual int GetId();                      // 983: 009382a0
		virtual int GetParentId();                // 983: 009382b0
		virtual void UpdateWorld(const WMatrix& parent); // 983: 0092b130 (mundo -> sCamera::mat)
	};

	struct sPointNode : public sNode // 0x78, vtable 00d3cdf4
	{
		sPoint* m_rec; // +0x74
		explicit sPointNode(sPoint* rec);         // 983: 009382c0
		virtual int GetType();                    // 983: 009382f0 (2)
		virtual int GetId();                      // 983: 00938300
		virtual int GetParentId();                // 983: 00938310
		virtual void UpdateWorld(const WMatrix& parent); // 983: 0092b190 (pivo do mundo -> sPoint::pos)
	};

	struct sAreaNode : public sNode // 0x78, vtable 00d3ce14
	{
		sArea* m_rec; // +0x74
		explicit sAreaNode(sArea* rec);           // 983: 00938320
		virtual int GetType();                    // 983: 00938380 (3)
		virtual int GetId();                      // 983: 00938390
		virtual int GetParentId();                // 983: 009383a0
		virtual void UpdateWorld(const WMatrix& parent); // 983: 0092b1c0 (caixa aabb2_983 movida -> sArea::aabb)
	};

	struct sModelNode : public sNode // 0x7c, vtable 00d3ce34
	{
		sModel* m_rec;    // +0x74
		bool m_flag983;   // +0x78 (sMapData983 +1; uso nao medido)
		sModelNode(sModel* rec, bool flag);       // 983: 009383b0
		virtual int GetType();                    // 983: 00938420 (4)
		virtual int GetId();                      // 983: 00938430
		virtual int GetParentId();                // 983: 00938440
		virtual void UpdateWorld(const WMatrix& parent); // 983: 0092b290 (mundo -> sModel::mat + boneco)
	};

	struct sGroundBoneNode : public sNode // 0x7c, vtable 00d3ce54 (registro sAttach983)
	{
		sAttach983* m_rec; // +0x74
		bool m_flag983;    // +0x78
		sGroundBoneNode(sAttach983* rec, bool flag); // 983: 00938450
		virtual int GetType();                    // 983: 009384e0 (5)
		virtual int GetId();                      // 983: 009384f0
		virtual int GetParentId();                // 983: 00938500
		virtual void SetLocal(const WMatrix& mat); // 983: 0092b320
		virtual bool IsPin983();                // 983: 00926d80 (value == 1: buraco/pin)
		virtual bool IsTee983();                // 983: 00926d90 (value == 2: tee; time = distancia)
		virtual void UpdateWorld(const WMatrix& parent); // 983: 0092b400 (mundo -> mat + osso)
	};

	// 983: conjunto dos registros de um arquivo de mapa (objeto local de 0xc0 no Load 00949c90 e nos leitores de
	// verificacao 0093f7a0/0093fa40/0093fc80/00941600/009418a0). No Load aponta para os arrays do WPolySoup
	// (bOwner = false); nos leitores de verificacao e dono deles. Nome meu.
	struct sMapData983
	{
		bool bOwner;                    // +0x00
		bool bFlag;                     // +0x01 (vai para sModelNode/sGroundBoneNode)
		int cameraNum;                  // +0x04
		sCamera* camera;                // +0x08
		int pointNum;                   // +0x0c
		sPoint* point;                  // +0x10
		int areaNum;                    // +0x14
		sArea* area;                    // +0x18
		sModel* baseModel;              // +0x1c
		std::vector<sModel*> model;     // +0x20
		int attachNum;                  // +0x30
		sAttach983* attach;             // +0x34
		sRootNode root;                 // +0x38
		sTexture* texture;              // +0xac (delete[] no destrutor; tipo provavel)
		std::vector<CNodeContainer> node; // +0xb0

		sMapData983(bool owner, bool flag); // 983: 009398b0
		~sMapData983();                     // 983: 0093f570
		bool BuildNodes983();               // 983: 0093d870
	};

	// 983: RTTI .?AVcTeeShotGroundBoneTransformer@WPolySoup@@ (vtable 00d0abfc); derivadas Normal/Expert na unidade do
	// CHoleInOneGameHandler (frente B). Reposiciona os ossos de chao dos tees em volta do pin (modo 0xb).
	class cTeeShotGroundBoneTransformer
	{
	public:
		cTeeShotGroundBoneTransformer(unsigned long seed); // 983: 0057a120
		virtual ~cTeeShotGroundBoneTransformer() { }     // 983: 0057a160
		virtual bool Transform(sAttach983* pin, const std::vector<sAttach983*>& tees) = 0; // [1]
		void SetCurrent();                                // 983: 0057a150 (ms_pTeeShotTransformer = this)

		unsigned long m_seed; // +0x04
	};
	static cTeeShotGroundBoneTransformer* ms_pTeeShotTransformer; // 983: 00e7ea88

	class CStaticPetGrp : public CRenderFuncPtr
	{
	public:
		CStaticPetGrp();
		~CStaticPetGrp();

		void Build(const std::vector<sModel*>& models);
		void Clear();
		virtual void Display();

	private:
		int m_num;
		WxStaticPuppetGrp** m_grp;
	};

	sCamera* FindCamera(const char* name);
	sCamera* GetCameraByName(const char* name);
	sPoint* FindPoint(const char* name);
	sArea* FindArea(const char* name);
	sArea* GetAreaByPos(const WVector& pos);
	sModel* FindModel(const char* name);
	CNodeContainer* FindNode(const char* name);
	w_bound_box* GetObjBoundBox(int index, const char* name);
	int GetObjAlpha(int index);
	float GetObjWindFactor(int index);
	WVector GetCameraPos(const WVector& from, const WVector& to);

	void CreateDataModel(sModel* model);
	void ChangeDataStructure(sModel* model);
	bool DeleteDataStructure(sModel* model);
	void SetAdditionModel(const char* name, const WVector& pos);

protected:
	void ProcessPostLoadPet(sModel* model);
	void ApplyFx(sModel* model, WBone* bone, const char* fx);
	float GetCameraRadius(WVector eye, WVector target, WVector up);
	bool IsPointInPoly(const std::vector<WVector>& poly, const WVector& pos);
	bool IsClockWise(const std::vector<WVector>& poly, const WVector& pos);
	WVector PosInWorld(const std::vector<WVector>& poly, const WVector& a,
		const WVector& b);

private:
	bool HaveCameraArea() { return m_bCameraArea; }

public:
	int GetModelNum() { return m_modelNum; }
	int GetAreaNum() { return m_header.areaNum; }
	int GetRmrModelNum() { return m_rmrModelNum; }

	sModel* GetBaseModel() { return m_baseModel; }

	sArea* GetArea(int index) { return &m_area[index]; }

private:
	typedef std::list<sSoundBox>::iterator SoundBoxIter;

public:
	std::vector<CNodeContainer> m_node;

private:
	char m_mapName[64];
	sHeader m_header;
	int m_modelNum;
	int m_rmrModelNum;
	sTexture* m_texture;
	int m_oldTextureNum;
	sTexture* m_oldTexture;
	sCamera* m_camera;
	sArea* m_area;
	sPoint* m_point;
	sAttach983* m_attach983; // 983: 0xc8 (NULL na lista de inicialização; new[] em 0093e4d0); nome meu
	std::vector<sModel*> m_model;
	sModel* m_baseModel;
	Waabb m_aabb_e0_983; // 983: 0xe0 (zerado no construtor); nome meu
	std::map<int, int> m_texIndex;
	float m_fadeNear;
	float m_fadeFar;
	Waabb m_extraHoleStartArea;
	CGolfDoc* m_pDoc;
	bool m_bBuildTnL; // 983: 0x12c (m_bUnused não existe mais)
	CStaticPetGrp m_staticPetGrp;
	sBgFx m_bgFx[256];
	int m_bgFxNum;
	bool m_bCameraArea;
	std::list<LightSet> m_lightList;
	std::vector<unsigned long> m_npcList;
	std::vector<int> m_unusedList;
	std::list<sSoundBox> m_soundBox;
	unsigned long m_reserved;
	float m_cullDistance;
	unsigned long m_sbinVersion983; // 983: 0x4984 = 0xfffe0000 no construtor; versao do .sbin (>= 0xfffe0100 = por osso)
	std::vector<int> m_smapVec4988_983; // 983: 0x4988; texturas do shadowmap
	std::vector<w_mesh*> m_smapVec4998_983; // 983: 0x4998; malhas do shadowmap (penduradas em cada osso)
	bool m_bSmapLoadFailed;
	int m_holecupTex;
	struct sCupVtx
	{
		WVector pos;
		w_mesh* mesh;
		int index;
	};

	struct sHoleVtx
	{
		WVector pos;
		WVector normal;
		unsigned long color;
		std::vector<w_uv983> uv;	// 983 (+0x1c, sizeof 0x2c): uma UV por camada da malha (645: uv/buv)
	};

	struct sHoleTri
	{
		w_mesh* mesh;
		int offset;
		int index[3];
		WVector* pos[3];
		WVector* normal[3];
		std::vector<w_uv983> uv[3];	// 983 (sizeof 0x8c igual): UVs de cada ponto por camada (645: uv/buv[3][2])
		unsigned long color[3];
		WVector edge[3];
	};

	struct sHoleMesh
	{
		w_mesh* mesh;
		std::vector<sHoleVtx> vtx;
		std::vector<int> index;
	};

	struct SmInFragment
	{
		unsigned char mesh;
		unsigned short tri;
		unsigned short vtx[3];
		unsigned short uv[3];
	};

	struct SmInFragList
	{
		int num;
		SmInFragment* frag;

		SmInFragList()
		{
			num = 0;
			frag = NULL;
		}
		~SmInFragList()
		{
			if (frag)
			{
				delete[] frag;
				frag = NULL;
			}
		}
	};

	struct SmCase
	{
		int num;
		SmInFragList* list;

		SmCase()
		{
			num = 0;
			list = NULL;
		}
		~SmCase()
		{
			if (list)
			{
				delete[] list;
				list = NULL;
			}
		}
	};

	struct SmOutFrag
	{
		unsigned short tri;
		unsigned short vtx[3];
	};

	typedef std::vector<sCupVtx>::iterator CupVtxIter;
	typedef std::vector<sHoleTri>::iterator HoleTriIter;
	typedef std::vector<sHoleMesh>::iterator HoleMeshIter;
	cVertexColorTable983 m_vtxColors983; // 983: +0x49b0 (no 645 era o m_triList; o 983 nao monta lista de triangulos)

private:
	static int __cdecl CompareModel(const void* a, const void* b);
	static int __cdecl smCompareOutFrag(const void* a, const void* b);

	void ReloadBasePet();
	void xBuildTnLBuffers();
	void BackupTextures();
	void ClearOldTextures();
	unsigned char OverlapCamera(int index);

	static bool __fastcall ReadHeader983(cFile* file, sHeader& header); // 983: 009271c0
	// 983: o Load (00949c90) virou uma cadeia de leitores estaticos (devolvem por ponteiro; usados tambem por
	// 009418a0/00941600 num sMapData983) e de passos membros; nomes meus.
	static bool __fastcall ReadCameras983(cFile* file, sHeader& header, sCamera*& camera);      // 983: 0093dfa0
	static bool __fastcall ReadPoints983(cFile* file, sHeader& header, sPoint*& point);         // 983: 0093e080
	static bool __fastcall ReadAreas983(cFile* file, sHeader& header, sArea*& area);            // 983: 0093e150
	static bool __fastcall ReadTextures983(cFile* file, sHeader& header, sTexture*& texture);   // 983: 0092b5c0
	static bool __fastcall ReadNodes983(cFile* file, sHeader& header,
		std::vector<CNodeContainer>& node);                                                      // 983: 00940c20
	static bool __fastcall ReadModel983(cFile* file, sHeader& header, unsigned char detail,
		sModel*& model, bool* skipped);                                                          // 983: 0093e220
	static bool __fastcall ReadModels983(cFile* file, sHeader& header, std::vector<sModel*>& model); // 983: 0093e400
	static bool __fastcall ReadAttach983(cFile* file, sHeader& header, sAttach983*& attach);    // 983: 0093e4d0
	static bool __fastcall CompareModel983(sModel* a, sModel* b);                               // 983: 00926e70
	static bool __fastcall SkipVertexColors983(cFile* file, sHeader& header);                  // 983: 00927220
	static sAttach983* __fastcall FindPinAttach983(sAttach983* attach, unsigned int num);       // 983: 00927310
	static sAttach983* __fastcall FindTeeAttach983(sAttach983* attach, unsigned int num, float range); // 983: 00927360
	static void __fastcall CollectTeeAttach983(sAttach983* attach, unsigned int num,
		std::vector<sAttach983*>& tees);                                                         // 983: 00936f80
	static bool __fastcall TransformGroundBones983(sAttach983* attach, unsigned int num);       // 983: 009385c0
	// 983: o LoadMapCheckData do 645 (seeks no fim do arquivo) virou o 009403b0 + estes passos sobre o arquivo aberto;
	// cada um rele o cabecalho do inicio. Nomes meus.
	bool ReadMapCheckSum983(cFile* file, unsigned char hole);   // 983: 0092f910
	bool ReadTeePinPoints983(cFile* file, unsigned char hole);  // 983: 0093fc80
	bool ReadExtraTeeArea983(cFile* file, unsigned char hole);  // 983: 0093f7a0
	bool ReadHolePar983(cFile* file, unsigned char hole);       // 983: 0092f9a0
	bool LoadShortGameTee983(unsigned char hole);               // 983: 0093fa40 (arquivo .sgbin)
	bool PostCameras983();              // 983: 00927200 (vazio)
	bool ApplyPointLights983();         // 983: 0092fb30
	bool PostAreas983();                // 983: 00927210 (vazio)
	bool LoadTextures983();             // 983: 00939a10
	bool CheckCameraNodes983();         // 983: 0092b660
	bool LoadBaseModel983(cFile* file); // 983: 009493b0
	bool LoadModels983();               // 983: 00939af0
	bool FindAttachBones983();          // 983: 009272b0
	void SetLightDir983();              // 983: 0092b6a0 (via 0092fc40)
	void ProcessAreas983(bool bOption); // 983: 0093e5b0
	void LoadSoundBoxes983();           // 983: 0093be90
	void SetExtraHoleArea983();         // 983: 0092fc50
	void LoadBgFx983(bool bOption);     // 983: 0092b730
	void PostBaseModel983();            // 983: 0093feb0
	void PostModels983();               // 983: 0093e780
	// 983: contexto do percurso dos ossos do modelo base (00b6a410) no BuildBaseTriangles983. Nome meu.
	struct sBaseTriContext983
	{
		WPolySoup* soup;
		int index;
		Waabb box;
		bool bBaseMeshTest;
	};
	void BuildBaseTriangles983();       // 983: 0093e5e0
	static void BaseTriCallback983(WPuppet* pet, WBone* bone, void* param); // 983: 0093a850
	bool LoadProperty983();                                             // 983: 0092fa70
	bool LoadShadowmapInfo();                                           // 983: 009473d0
	bool LoadShadowmapBone983(cFile* file, WBone* bone);               // 983: 00946cd0
	void smLoadSmapMeshes(SmCase* smCase, cFile* file, WBone* bone, w_mesh** meshList,
		WVector* vtxList, float (*uvList)[2]);
	w_mesh* smAddSmapPack(SmInFragList* fragList, cFile* file, int caseIndex,
		int listIndex, WBone* bone, w_mesh** meshList, WVector* vtxList, float (*uvList)[2]);
	void smLoadOutFrags(cFile* file, int num, w_mesh** meshList,
		WVector* vtxList, SmCase* smCase);
	void smBuildOutFragMeshes(w_mesh* mesh, SmOutFrag* frag, int num,
		WVector* vtxList);

	void PlaceHoleCup();
	// 983: contexto do percurso dos ossos do PlaceHoleCup (0094ab60). Nome meu.
	struct sHoleCupContext983
	{
		WPolySoup* soup;
		bool done;
	};
	static void HoleCupCallback983(WPuppet* pet, WBone* bone, void* param); // 983: 0094a680
	void AddHolecup(WBone* bone, std::vector<sHoleMesh>& meshes,
		std::vector<sCupVtx>& cup, std::vector<sHoleTri>& tris);
	void GatherHolecupTris(WBone* bone, std::vector<sHoleTri>& tris,
		std::vector<sCupVtx>& cup);
	void GenHolecupTri(sHoleTri* tri, w_mesh* mesh, int offset);
	void FindNewHolecupVtxs(std::vector<sCupVtx>& cup,
		std::vector<sHoleTri>& tris);
	void GenNewHolecupVtx(sHoleVtx* vtx, sHoleTri* tri);
	void DigHolecup(WBone* bone, std::vector<sCupVtx>& cup,
		std::vector<sHoleTri>& tris);
	void CreateNewHolecupMeshes(std::vector<sHoleMesh>& meshes,
		std::vector<sCupVtx>& cup, std::vector<sHoleTri>& tris);
	int ClipHolecup(std::vector<WVector>& poly, sHoleTri& tri,
		std::vector<sCupVtx>& cup);
	void BuildNewHolecupMesh(std::vector<sHoleMesh>& meshes, sHoleTri& tri,
		std::vector<WVector>& poly);
	void AddNewHolecupTri(std::vector<sHoleMesh>& meshes, sHoleTri& tri,
		WVector* a, WVector* b, WVector* c, int ia, int ib, int ic);
	void ApplyHoleMesh(sHoleMesh& mesh, std::vector<sHoleTri>& tris);
	bool IsInPoly(WVector& pos, std::vector<sCupVtx>& cup);
	bool IsInTri(WVector& pos, WVector** const tri);
	bool AreCrossed(WVector* a1, WVector* a2, WVector* b1, WVector* b2);
	bool GetIntersection(WVector* a1, WVector* a2, WVector* b1, WVector* b2,
		WVector* out, WVector* dummy);
};
