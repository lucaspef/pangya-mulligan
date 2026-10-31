#include "minatl.h"
#include <float.h>
#include "vc10_layout.h"
#include "wbone.h"
#include "wxpuppet.h"
#include "wpuppetutil.h"
#include "fxbox.h"
#include "quadtree.h"
#include "fx.h"
#include "fxspray.h"
#include "fxsequence.h"
#include "polysoup.h"
#include "wlocalize.h"
#include "golfdoc.h"
#include "clientsetting.h"

// HACK: name-count padding. WPolySoup::MakeStatistics reacts to the number of names declared before
// it (scheduling ties in c2); our PCH declares fewer than the original at the point where projectg's
// SaveScreenShot matches. 8 or more declarations worked here; the club PCH names
// (WList::End/Prev/GetCount, WVector::Negate, ...) moved the window (period 64): 24..53 worked then.
// golfcadie's names in quadtree.h / fx.h count too, so 14 pads were dropped (26 now).
void RbPolySoupNamePad0();
void RbPolySoupNamePad1();
void RbPolySoupNamePad2();
void RbPolySoupNamePad3();
void RbPolySoupNamePad4();
void RbPolySoupNamePad5();
void RbPolySoupNamePad6();
void RbPolySoupNamePad7();
void RbPolySoupNamePad8();
void RbPolySoupNamePad9();
void RbPolySoupNamePad10();
void RbPolySoupNamePad11();
void RbPolySoupNamePad12();
void RbPolySoupNamePad13();
void RbPolySoupNamePad14();
void RbPolySoupNamePad15();
void RbPolySoupNamePad16();
void RbPolySoupNamePad17();
void RbPolySoupNamePad18();
void RbPolySoupNamePad19();
void RbPolySoupNamePad20();
void RbPolySoupNamePad21();
void RbPolySoupNamePad22();
void RbPolySoupNamePad23();
void RbPolySoupNamePad24();
void RbPolySoupNamePad25();
#include "contentsdoc.h"
#include "wind.h"
#include "mathconsts.h"
#include "jp/shareddocjp.h" // 983: GetHoleValue983 (00633560)
#include "taskmanager.h"
#include "golftask.h"

extern "C" __declspec(dllimport) int __cdecl mkdir(const char*);

// HACK: 96.0f is defined in autotarget.cpp as a function-local static aliased
// to __real@42c00000; using the literal here would emit a competing COMDAT that
// all the other objects would bind to.
extern "C" __declspec(selectany) const float rb_real_42c00000 = 96.0f; // C64.4: o estático de função do rebang não existe mais

static bool IsSeaPet(const char* name)
{
	static const char* seaPet[] = {
		"un_sea.pet",
		"silv_dn.pet",
		"ice_unsea.pet",
		"panda_sea.pet",
	};

	for (unsigned int i = 0; i < sizeof(seaPet) / sizeof(seaPet[0]); i++)
	{
		if (stricmp(name, seaPet[i]) == 0)
			return true;
	}

	return false;
}

// 983: não existem mais (PROVISÓRIO)

WPolySoup::cTeeShotGroundBoneTransformer* WPolySoup::ms_pTeeShotTransformer = NULL; // 983: 00e7ea88

WPolySoup::WPolySoup(CGolfDoc* pDoc)
	: m_texture(NULL),
	  m_oldTextureNum(0),
	  m_oldTexture(NULL),
	  m_camera(NULL),
	  m_area(NULL),
	  m_point(NULL),
	  m_attach983(NULL),
	  m_baseModel(NULL),
	  m_bBuildTnL(false),
	  m_bgFxNum(0),
	  m_sbinVersion983(0xfffe0000),
	  m_bSmapLoadFailed(false),
	  m_holecupTex(0)
{
	// 983 CSky::GenerateLensFlare 007059c0: [soup+0x94] = pointNum, [soup+0xc4] = m_point, passo 0x64, type +0, pos +0x24 (A375)
	C_ASSERT_VC10(offsetof(WPolySoup, m_header.pointNum) == 0x94 && offsetof(WPolySoup, m_point) == 0xc4);
	// A728: crus PolyTextureNum/PolyObjNum/PolyTextures/PolyObjs/PolyTexMap/PolyDoc do quadtree.cpp (VC10)
	C_ASSERT_VC10(offsetof(WPolySoup, m_header.textureNum) == 0x9c && offsetof(WPolySoup, m_modelNum) == 0xa8 && offsetof(WPolySoup, m_texture) == 0xb0 && offsetof(WPolySoup, m_model) == 0xcc && offsetof(WPolySoup, m_texIndex) == 0xf8 && offsetof(WPolySoup, m_pDoc) == 0x128 && offsetof(WPolySoup, m_soundBox) == 0x4970);
	C_ASSERT_VC10(sizeof(WPolySoup::sPoint) == 0x64 && offsetof(WPolySoup::sPoint, pos) == 0x24);
	m_pDoc = pDoc;
	m_mapName[0] = 0;
	m_modelNum = 0; // 983: sem o memset do m_header (o sHeader tem construtor)
	m_aabb_e0_983.Reset();

	m_cullDistance = 0.0f;
	m_fadeNear = 800.0f;
	m_fadeFar = 960.0f;

	m_model.clear();
	m_rmrModelNum = 0;

	m_extraHoleStartArea.Reset();
}

WPolySoup::~WPolySoup()
{
	Clear();
	ClearOldTextures();

	if (g_resrcmng && m_holecupTex)
	{
		g_resrcmng->Release(m_holecupTex);
		m_holecupTex = 0;
	}
}

void WPolySoup::Clear()
{
	memset(m_mapName, 0, sizeof(m_mapName));

	if (m_model.size())
	{
		for (int i = 0; i < m_modelNum; i++)
		{
			sModel* model = m_model[i];

			if (model->pet)
			{
				g_resrcmng->Release(model->pet, false);
				model->pet = NULL;
			}

			if (model->quad)
			{
				delete[] model->quad;
				model->quad = NULL;
			}
			model->tri.clear();

			delete model;
		}
	}

	if (m_baseModel)
	{
		if (m_baseModel->pet)
		{
			g_resrcmng->Release(m_baseModel->pet, true);
			m_baseModel->pet = NULL;
		}

		delete m_baseModel;
		m_baseModel = NULL;
	}

	m_aabb_e0_983.Reset(); // 983: 00947eb0 zera aqui
	BackupTextures();

	m_model.clear();
	m_modelNum = 0; // 983: 00947eb0

	if (m_camera)
	{
		delete[] m_camera;
		m_camera = NULL;
	}
	if (m_point)
	{
		delete[] m_point;
		m_point = NULL;
	}
	if (m_area)
	{
		delete[] m_area;
		m_area = NULL;
	}
	if (m_attach983)
	{
		delete[] m_attach983;
		m_attach983 = NULL;
	}

	m_node.clear();
	m_texIndex.clear();

	m_lightList.clear();
	m_npcList.clear();
	m_soundBox.clear();

	memset(m_bgFx, 0, sizeof(m_bgFx));
	m_bgFxNum = 0;

	for (size_t i = 0; i < m_smapVec4988_983.size(); i++)
		g_resrcmng->Release(m_smapVec4988_983[i]);
	m_smapVec4988_983.clear();
	m_smapVec4998_983.clear();

	m_staticPetGrp.Clear();

	if (CSceneManager::Instance())
		CSceneManager::Instance()->ReleaseElement();

	if (CFx::Instance())
		CFx::Instance()->ClearAll(false);
}

void WPolySoup::BackupTextures()
{
	ClearOldTextures();

	m_oldTextureNum = m_header.textureNum;
	m_oldTexture = m_texture;
	m_texture = NULL;
}

void WPolySoup::ClearOldTextures()
{
	for (int i = 0; i < m_oldTextureNum; i++)
	{
		if (g_resrcmng && m_oldTexture[i].texHandle)
		{
			g_resrcmng->Release(m_oldTexture[i].texHandle);
			m_oldTexture[i].texHandle = 0;
		}
	}

	if (m_oldTexture)
	{
		delete[] m_oldTexture;
		m_oldTexture = NULL;
	}
	m_oldTextureNum = 0;
}

// ---------------------------------------------------------------------------------------------------------------
// 983: leitura do arquivo de mapa por registro (versao 0x71 do 645 e 0x72 do 983). Cada registro tem Clear/Read
// (versao, arquivo); a parte nova so vem no arquivo 0x72, senao copia o dado da base.
// ---------------------------------------------------------------------------------------------------------------

// 983: 0092a640 / 0092a550 (membros do cFile, declarados em include/engine/cfile.h).
static inline int ReadVector983(cFile* file, WVector& v)
{
	int n = file->Read(&v.x, 4);
	n += file->Read(&v.y, 4);
	n += file->Read(&v.z, 4);
	return n;
}

int cFile::ReadAabb983(Waabb& aabb)
{
	return ReadVector983(this, aabb.min) + ReadVector983(this, aabb.max);
}

int cFile::ReadMatrix983(WMatrix& mat)
{
	return ReadVector983(this, mat.xa) + ReadVector983(this, mat.ya) + ReadVector983(this, mat.za) +
		   ReadVector983(this, mat.pivot);
}

WPolySoup::sLink983::sLink983()
{
	Clear();
}

void WPolySoup::sLink983::Clear()
{
	id = -1;
	parentId = -1;
}

void WPolySoup::sLink983::Read(int version, cFile* file)
{
	Clear();
	file->Read(&id, 4);
	file->Read(&parentId, 4);
}

WPolySoup::sHeader::sHeader()
{
	ClearBase();
	Clear();
}

void WPolySoup::sHeader::ClearBase()
{
	id[0] = 0;
	version = 0;
	modelNum = 0;
	modelNum1 = 0;
	modelNum2 = 0;
	cameraNum = 0;
	pointNum = 0;
	areaNum = 0;
	textureNum = 0;
	nodeNum = 0;
}

void WPolySoup::sHeader::Clear()
{
	ClearBase();
	attachNum983 = 0;
}

void WPolySoup::sHeader::ReadBase(cFile* file)
{
	ClearBase();
	file->Read(this, 0x28);
}

void WPolySoup::sHeader::Read(cFile* file)
{
	Clear();
	ReadBase(file);
	if ((int)version >= 0x72)
		file->Read(&attachNum983, 4);
}

// 983: 009271c0 (fastcall: arquivo, cabecalho). Aceita ate 0x72 (o 645 so aceitava 0x71).
bool __fastcall WPolySoup::ReadHeader983(cFile* file, sHeader& header)
{
	header.Read(file);
	if ((int)header.version > 0x72)
	{
		MessageBox(NULL,
			K2L_Compatibility("\xb9\xe8\xb0\xe6 \xc6\xc4\xc0\xcf\xc0\xc7 \xb9\xf6\xc1\xaf\xc0\xcc "
							  "\xb8\xc2\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9."),
			K2L_Compatibility("\xb9\xe8\xb0\xe6\xc6\xc4\xc0\xcf \xb9\xf6\xc1\xaf"), 0);
		return false;
	}
	return true;
}

WPolySoup::sCameraBase983::sCameraBase983()
{
	Clear();
}

void WPolySoup::sCameraBase983::Clear()
{
	name[0] = 0;
	mat.Reset();
	fov = 0.0f;
	reserved = 0.0f;
}

void WPolySoup::sCameraBase983::Read(int version, cFile* file)
{
	WVector eye;
	WVector at;

	Clear();
	file->Read(name, 32);
	file->Read(&eye.x, 4);
	file->Read(&eye.y, 4);
	file->Read(&eye.z, 4);
	file->Read(&at.x, 4);
	file->Read(&at.y, 4);
	file->Read(&at.z, 4);
	mat = MakeCameraMatrix(eye, at);
	file->Read(&fov, 4);
	file->Read(&reserved, 4);
}

WPolySoup::sCamera::sCamera()
	: node983(NULL)
{
	Clear();
}

void WPolySoup::sCamera::Clear()
{
	sCameraBase983::Clear();
	link983.Clear();
	mat2_983.Reset();
	if (node983)
	{
		delete node983;
		node983 = NULL;
	}
}

void WPolySoup::sCamera::Read(int version, cFile* file)
{
	Clear();
	sCameraBase983::Read(version, file);
	if (version >= 0x72)
	{
		WVector eye;
		WVector at;

		file->Read(&eye.x, 4);
		file->Read(&eye.y, 4);
		file->Read(&eye.z, 4);
		file->Read(&at.x, 4);
		file->Read(&at.y, 4);
		file->Read(&at.z, 4);
		mat2_983 = MakeCameraMatrix(eye, at);
		link983.Read(version, file);
	}
	else
	{
		mat2_983 = mat;
	}
}

WPolySoup::sPointBase983::sPointBase983()
{
	Clear();
}

void WPolySoup::sPointBase983::Clear()
{
	type = 0;
	name[0] = 0;
	pos.Reset();
	option = "";
}

void WPolySoup::sPointBase983::Read(int version, cFile* file)
{
	Clear();
	file->Read(&type, 1);
	file->Read(name, 32);
	ReadVector983(file, pos);
	if (type)
	{
		char buf[64];
		file->Read(buf, 64);
		option = buf;
	}
}

WPolySoup::sPoint::sPoint()
	: node983(NULL)
{
	Clear();
}

void WPolySoup::sPoint::Clear()
{
	sPointBase983::Clear();
	link983.Clear();
	pos2_983.Reset();
	if (node983)
	{
		delete node983;
		node983 = NULL;
	}
}

void WPolySoup::sPoint::Read(int version, cFile* file)
{
	Clear();
	sPointBase983::Read(version, file);
	if (version >= 0x72)
	{
		ReadVector983(file, pos2_983);
		link983.Read(version, file);
	}
	else
	{
		pos2_983 = pos;
	}
}

WPolySoup::sAreaBase983::sAreaBase983()
{
	Clear();
}

void WPolySoup::sAreaBase983::Clear()
{
	type = 0;
	name[0] = 0;
	aabb.Reset();
}

void WPolySoup::sAreaBase983::Read(int version, cFile* file)
{
	Clear();
	file->Read(&type, 1);
	file->Read(name, 64);
	file->ReadAabb983(aabb);
}

WPolySoup::sArea::sArea()
	: node983(NULL)
{
	Clear();
}

void WPolySoup::sArea::Clear()
{
	sAreaBase983::Clear();
	link983.Clear();
	aabb2_983.Reset();
	if (node983)
	{
		delete node983;
		node983 = NULL;
	}
}

void WPolySoup::sArea::Read(int version, cFile* file)
{
	Clear();
	sAreaBase983::Read(version, file);
	if (version >= 0x72)
	{
		file->ReadAabb983(aabb2_983);
		link983.Read(version, file);
	}
	else
	{
		aabb2_983 = aabb;
	}
}

WPolySoup::sAttach983::sAttach983()
	: node983(NULL)
{
	Clear();
}

void WPolySoup::sAttach983::Clear()
{
	link983.Clear();
	name[0] = 0;
	mat.Reset();
	mat2.Reset();
	bone = NULL;
	value = 0;
	time = 0.0f;
	if (node983)
	{
		delete node983;
		node983 = NULL;
	}
}

void WPolySoup::sAttach983::Read(int version, cFile* file)
{
	Clear();
	link983.Read(version, file);
	file->Read(name, 64);
	file->ReadMatrix983(mat);
	file->ReadMatrix983(mat2);
	file->Read(&value, 4);
}

// 983: hierarquia dos registros do mapa (sNode e derivados; montada por sMapData983::BuildNodes983).

WPolySoup::sNode::sNode()
	: m_parent(NULL), m_local(WMatrix::IDENTITY), m_world(WMatrix::IDENTITY)
{
}

void WPolySoup::sNode::SetLocal(const WMatrix& mat)
{
	m_local = mat;
}

bool WPolySoup::sNode::IsPin983()
{
	return false;
}

bool WPolySoup::sNode::IsTee983()
{
	return false;
}

void WPolySoup::sNode::UpdateWorld(const WMatrix& parent)
{
	m_world = m_local * parent;
}

void WPolySoup::sNode::SetParent(sNode* parent)
{
	if (m_parent)
		m_parent->RemoveChild(this);
	m_parent = parent;
	if (m_parent)
		m_parent->AddChild(this);
}

void WPolySoup::sNode::RemoveChild(sNode* child)
{
	std::list<sNode*>::iterator it =
		std::find(m_children.begin(), m_children.end(), child);
	if (it != m_children.end())
		m_children.erase(it);
}

void WPolySoup::sNode::AddChild(sNode* child)
{
	m_children.push_back(child);
}

WPolySoup::sNode* WPolySoup::sNode::Find(int id)
{
	if (id == GetId())
		return this;

	for (std::list<sNode*>::iterator it = m_children.begin();
		it != m_children.end(); ++it)
	{
		sNode* node = (*it)->Find(id);
		if (node)
			return node;
	}

	return NULL;
}

void WPolySoup::sNode::Update(const WMatrix& parent)
{
	UpdateWorld(parent);

	for (std::list<sNode*>::iterator it = m_children.begin();
		it != m_children.end(); ++it)
		(*it)->Update(m_world);
}

int WPolySoup::sRootNode::GetType()
{
	return 0;
}

int WPolySoup::sRootNode::GetId()
{
	return 0;
}

int WPolySoup::sRootNode::GetParentId()
{
	return -1;
}

WPolySoup::sCameraNode::sCameraNode(sCamera* rec)
	: m_rec(rec)
{
	m_local = rec->mat2_983;
}

int WPolySoup::sCameraNode::GetType()
{
	return 1;
}

int WPolySoup::sCameraNode::GetId()
{
	return m_rec->link983.id;
}

int WPolySoup::sCameraNode::GetParentId()
{
	return m_rec->link983.parentId;
}

void WPolySoup::sCameraNode::UpdateWorld(const WMatrix& parent)
{
	sNode::UpdateWorld(parent);
	m_rec->mat = m_world;
}

WPolySoup::sPointNode::sPointNode(sPoint* rec)
	: m_rec(rec)
{
	m_local.pivot = rec->pos2_983;
}

int WPolySoup::sPointNode::GetType()
{
	return 2;
}

int WPolySoup::sPointNode::GetId()
{
	return m_rec->link983.id;
}

int WPolySoup::sPointNode::GetParentId()
{
	return m_rec->link983.parentId;
}

void WPolySoup::sPointNode::UpdateWorld(const WMatrix& parent)
{
	// 983 0092b190: pivo [this+0x68] -> [[this+0x74]+0x24] (A375)
	C_ASSERT_VC10(sizeof(sPointNode) == 0x78 && offsetof(sPointNode, m_world.pivot) == 0x68 && offsetof(sPointNode, m_rec) == 0x74 && offsetof(sPointNode, m_local.pivot) == 0x38 && offsetof(sPoint, pos2_983) == 0x54);
	sNode::UpdateWorld(parent);
	m_rec->pos = m_world.pivot;
}

WPolySoup::sAreaNode::sAreaNode(sArea* rec)
	: m_rec(rec)
{
	m_local.pivot = WVector((rec->aabb2_983.max.x + rec->aabb2_983.min.x) * 0.5,
		(rec->aabb2_983.max.y + rec->aabb2_983.min.y) * 0.5,
		(rec->aabb2_983.max.z + rec->aabb2_983.min.z) * 0.5);
}

int WPolySoup::sAreaNode::GetType()
{
	return 3;
}

int WPolySoup::sAreaNode::GetId()
{
	return m_rec->link983.id;
}

int WPolySoup::sAreaNode::GetParentId()
{
	return m_rec->link983.parentId;
}

void WPolySoup::sAreaNode::UpdateWorld(const WMatrix& parent)
{
	sNode::UpdateWorld(parent);

	const Waabb& box = m_rec->aabb2_983;
	WVector center((box.max.x + box.min.x) * 0.5, (box.max.y + box.min.y) * 0.5,
		(box.max.z + box.min.z) * 0.5);
	WVector half = box.max - center;
	m_rec->aabb.min = m_world.pivot - half;
	m_rec->aabb.max = half + m_world.pivot;
}

WPolySoup::sModelNode::sModelNode(sModel* rec, bool flag)
	: m_rec(rec), m_flag983(flag)
{
	m_local = rec->localMat983;
}

int WPolySoup::sModelNode::GetType()
{
	return 4;
}

int WPolySoup::sModelNode::GetId()
{
	return m_rec->link983.id;
}

int WPolySoup::sModelNode::GetParentId()
{
	return m_rec->link983.parentId;
}

void WPolySoup::sModelNode::UpdateWorld(const WMatrix& parent)
{
	sNode::UpdateWorld(parent);
	m_rec->mat = m_world;
	if (m_rec->pet)
	{
		m_rec->pet->SetMatrix(m_rec->mat);
		m_rec->pet->Transform(NULL, 1.0f, 0);
	}
}

WPolySoup::sGroundBoneNode::sGroundBoneNode(sAttach983* rec, bool flag)
	: m_rec(rec), m_flag983(flag)
{
	m_local = rec->mat2;
}

int WPolySoup::sGroundBoneNode::GetType()
{
	return 5;
}

int WPolySoup::sGroundBoneNode::GetId()
{
	return m_rec->link983.id;
}

int WPolySoup::sGroundBoneNode::GetParentId()
{
	return m_rec->link983.parentId;
}

void WPolySoup::sGroundBoneNode::SetLocal(const WMatrix& mat)
{
	m_local = mat;
	m_rec->mat2 = mat;
	// 983: 0092a460 (copia para WBone +0x110 fora de linha; aqui a referencia em linha)
	if (m_rec->bone)
		m_rec->bone->m_localMat = m_local;
}

bool WPolySoup::sGroundBoneNode::IsPin983()
{
	return m_rec->value == 1;
}

bool WPolySoup::sGroundBoneNode::IsTee983()
{
	return m_rec->value == 2;
}

void WPolySoup::sGroundBoneNode::UpdateWorld(const WMatrix& parent)
{
	sNode::UpdateWorld(parent);
	m_rec->mat = m_world;
	if (m_rec->bone)
		m_rec->bone->Transform(parent, NULL, 1.0f, 0);
}

WPolySoup::sMapData983::sMapData983(bool owner, bool flag)
	: bOwner(owner), bFlag(flag), cameraNum(0), camera(NULL), pointNum(0), point(NULL),
	  areaNum(0), area(NULL), baseModel(NULL), attachNum(0), attach(NULL), texture(NULL)
{
}

WPolySoup::sMapData983::~sMapData983()
{
	if (bOwner)
	{
		if (camera)
		{
			delete[] camera;
			camera = NULL;
		}
		if (point)
		{
			delete[] point;
			point = NULL;
		}
		if (area)
		{
			delete[] area;
			area = NULL;
		}
		if (baseModel)
			delete baseModel;
		for (unsigned int i = 0; i < model.size(); i++)
		{
			if (model[i])
				delete model[i];
		}
		if (attach)
		{
			delete[] attach;
			attach = NULL;
		}
		if (texture)
		{
			delete[] texture;
			texture = NULL;
		}
	}
}

// 983: 00926da0 -- mais descendentes primeiro (pai antes do filho); empate pelo id.
static bool CompareNodeDepth983(const std::pair<WPolySoup::sNode*, int>& a,
	const std::pair<WPolySoup::sNode*, int>& b)
{
	if (a.second > b.second)
		return true;
	if (a.second < b.second)
		return false;
	return a.first->GetId() < b.first->GetId();
}

bool WPolySoup::sMapData983::BuildNodes983()
{
	std::map<sNode*, int> depth;
	std::map<int, sNode*> nodeById;
	unsigned int i;

	for (i = 0; i < (unsigned int)cameraNum; i++)
	{
		camera[i].node983 = new sCameraNode(&camera[i]);
		depth[camera[i].node983] = 0;
		if (camera[i].link983.id != -1)
			nodeById[camera[i].link983.id] = camera[i].node983;
	}

	for (i = 0; i < (unsigned int)pointNum; i++)
	{
		point[i].node983 = new sPointNode(&point[i]);
		depth[point[i].node983] = 0;
		if (point[i].link983.id != -1)
			nodeById[point[i].link983.id] = point[i].node983;
	}

	for (i = 0; i < (unsigned int)areaNum; i++)
	{
		area[i].node983 = new sAreaNode(&area[i]);
		depth[area[i].node983] = 0;
		if (area[i].link983.id != -1)
			nodeById[area[i].link983.id] = area[i].node983;
	}

	if (baseModel)
	{
		baseModel->node983 = new sModelNode(baseModel, bFlag);
		depth[baseModel->node983] = 0;
		if (baseModel->link983.id != -1)
			nodeById[baseModel->link983.id] = baseModel->node983;
	}

	for (int k = 0; k < (int)model.size(); k++)
	{
		model[k]->node983 = new sModelNode(model[k], bFlag);
		depth[model[k]->node983] = 0;
		if (model[k]->link983.id != -1)
			nodeById[model[k]->link983.id] = model[k]->node983;
	}

	for (i = 0; i < (unsigned int)attachNum; i++)
	{
		attach[i].node983 = new sGroundBoneNode(&attach[i], bFlag);
		depth[attach[i].node983] = 0;
		if (attach[i].link983.id != -1)
			nodeById[attach[i].link983.id] = attach[i].node983;
	}

	// cada no soma 1 em todos os ancestrais
	for (std::map<sNode*, int>::iterator it = depth.begin(); it != depth.end(); ++it)
	{
		sNode* node = it->first;
		while (node->GetParentId() != -1)
		{
			node = nodeById[node->GetParentId()];
			depth[node]++;
		}
	}

	std::vector<std::pair<sNode*, int> > order;
	order.reserve(depth.size());
	for (std::map<sNode*, int>::iterator it = depth.begin(); it != depth.end(); ++it)
		order.push_back(std::pair<sNode*, int>(it->first, it->second));

	std::sort(order.begin(), order.end(), CompareNodeDepth983);

	for (std::vector<std::pair<sNode*, int> >::iterator it = order.begin();
		it != order.end(); ++it)
	{
		sNode* node = it->first;
		node->SetParent(node->GetParentId() == -1 ? &root : root.Find(node->GetParentId()));
	}

	root.Update(WMatrix::IDENTITY);
	return true;
}

WPolySoup::sModel::sModel()
	: type(0), flag(0), noCollision(0), alpha(0xff), windFactor(1.0f), scale(1.0f), pet(NULL),
	  localMat983(WMatrix::IDENTITY), mat(WMatrix::IDENTITY), center(WVector::ZERO), radius(0.0f), pos(WVector::ZERO),
	  quad(NULL), bSingleQuad(false), hitCount(0), id(0), node983(NULL)
{
	// aabb/bbox: o construtor do Waabb ja zera (o 645 chamava Reset() de novo aqui)
	name[0] = 0;
	option[0] = 0;
}

WPolySoup::sModel::~sModel()
{
	if (node983)
	{
		delete node983;
		node983 = NULL;
	}
}

// 983: registro do modelo no arquivo (sBgModel do 645, 0xac) + parte do 983 (0092f7f0 / 0092f850).
struct sBgModel983 : public sBgModel
{
	WPolySoup::sLink983 link983; // +0xac
	WMatrix mat2_983;            // +0xb4 (propria na 0x72; senao = mat)

	__declspec(noinline) void ClearBase();                        // 983: 0092f720
	__declspec(noinline) void ReadBase(int version, cFile* file); // 983: 0092f7d0
	__declspec(noinline) void Clear();                            // 983: 0092f7f0
	__declspec(noinline) void Read(int version, cFile* file);     // 983: 0092f850
};

void sBgModel983::ClearBase()
{
	flag = 0;
	faceNum = 0;
	memset(name, 0, sizeof(name));
	localAabb.Reset();
	sceneAabb.Reset();
	mat.Reset();
	mat.pivot.Reset();
	detail = 0;
	memset(option, 0, sizeof(option));
}

void sBgModel983::ReadBase(int version, cFile* file)
{
	ClearBase();
	file->Read(this, 0xac);
}

void sBgModel983::Clear()
{
	ClearBase();
	link983.Clear();
	mat2_983.Reset();
}

void sBgModel983::Read(int version, cFile* file)
{
	Clear();
	ReadBase(version, file);
	if (version >= 0x72)
	{
		file->ReadMatrix983(mat2_983);
		link983.Read(version, file);
	}
	else
	{
		mat2_983 = mat;
	}
}

static bool IsSilviaCannonArea(const WPolySoup::sArea* area); // 983: 0092b540

void WPolySoup::sTexture::Clear()
{
	name[0] = 0;
}

void WPolySoup::sTexture::Read(int version, cFile* file)
{
	Clear();
	file->Read(this, 0x20);
}

bool __fastcall WPolySoup::ReadCameras983(cFile* file, sHeader& header, sCamera*& camera)
{
	if (header.cameraNum > 0)
	{
		camera = new sCamera[header.cameraNum];
		for (int i = 0; i < header.cameraNum; i++)
			camera[i].Read(header.version, file); // 983: 0093ba10
	}
	return true;
}

bool __fastcall WPolySoup::ReadPoints983(cFile* file, sHeader& header, sPoint*& point)
{
	if (header.pointNum > 0)
	{
		point = new sPoint[header.pointNum];
		for (int i = 0; i < header.pointNum; i++)
			point[i].Read(header.version, file); // 983: 0093bcd0
	}
	return true;
}

bool __fastcall WPolySoup::ReadAreas983(cFile* file, sHeader& header, sArea*& area)
{
	if (header.areaNum > 0)
	{
		area = new sArea[header.areaNum];
		for (int i = 0; i < header.areaNum; i++)
			area[i].Read(header.version, file); // 983: 0093bbf0
	}
	return true;
}

bool __fastcall WPolySoup::ReadTextures983(cFile* file, sHeader& header, sTexture*& texture)
{
	if (header.textureNum > 0)
	{
		texture = new sTexture[header.textureNum];
		for (int i = 0; i < header.textureNum; i++)
			texture[i].Read(header.version, file);
	}
	return true;
}

bool __fastcall WPolySoup::ReadNodes983(cFile* file, sHeader& header, std::vector<CNodeContainer>& node)
{
	if (header.nodeNum > 0)
	{
		node.reserve(header.nodeNum);
		for (int i = 0; i < header.nodeNum; i++)
		{
			struct
			{
				char name[16];
				int num;
				int reserved;
			} nodeInfo;

			file->Read(&nodeInfo, sizeof(nodeInfo));

			CNodeContainer container(std::string(nodeInfo.name));

			if (strstr(nodeInfo.name, "outline"))
				container.SetType(CNodeContainer::TYPE_OUTLINE);
			else if (strstr(nodeInfo.name, "ib"))
				container.SetType(CNodeContainer::TYPE_IB);
			else if (strstr(nodeInfo.name, "ob"))
				container.SetType(CNodeContainer::TYPE_OB);
			else if (strstr(nodeInfo.name, "auto"))
				container.SetType(CNodeContainer::TYPE_AUTO);
			else if (strstr(nodeInfo.name, "beach"))
				container.SetType(CNodeContainer::TYPE_BEACH);
			else if (strstr(nodeInfo.name, "riverex"))
				container.SetType(CNodeContainer::TYPE_RIVEREX);
			else if (strstr(nodeInfo.name, "waterfall"))
				container.SetType(CNodeContainer::TYPE_WATERFALL);
			else if (strstr(nodeInfo.name, "river"))
				container.SetType(CNodeContainer::TYPE_RIVER);
			else if (strstr(nodeInfo.name, "lava"))
				container.SetType(CNodeContainer::TYPE_LAVA);
			else if (strstr(nodeInfo.name, "sea"))
				container.SetType(CNodeContainer::TYPE_SEA);
			else if (strstr(nodeInfo.name, "wave"))
				container.SetType(CNodeContainer::TYPE_WAVE);
			else if (strstr(nodeInfo.name, "npc"))
				container.SetType(CNodeContainer::TYPE_NPC);
			else if (strstr(nodeInfo.name, "camera"))
				container.SetType(CNodeContainer::TYPE_CAMERA);
			else if (strstr(nodeInfo.name, "fill"))
				container.SetType(CNodeContainer::TYPE_FILL983);

			for (int k = 0; k < nodeInfo.num; k++)
			{
				float buf[3];
				file->Read(buf, 12);
				container.Add(WVector(buf));
			}

			node.push_back(container);
		}
	}
	return true;
}

bool __fastcall WPolySoup::ReadModel983(cFile* file, sHeader& header, unsigned char detail,
	sModel*& model, bool* skipped)
{
	sBgModel983 bgModel;
	bgModel.Read(header.version, file); // 983: 0092f850

	if ((detail == 2 && bgModel.detail == 1) || (detail == 1 && bgModel.detail == 2))
	{
		if (skipped)
			*skipped = true;
		return true;
	}

	// 983: a caixa (aabb/bbox) nao vem mais do arquivo; sai das malhas no PostModels983/PostBaseModel983.
	model = new sModel;
	model->link983 = bgModel.link983;
	model->type = bgModel.flag & 7;
	model->flag = bgModel.flag & 0xf8;
	model->mat = bgModel.mat;
	model->localMat983 = bgModel.mat2_983;
	model->scale = bgModel.mat.xa.Magnitude();
	strncpy(model->name, bgModel.name, 31);
	strncpy(model->option, bgModel.option, 31);
	if (skipped)
		*skipped = false;
	return true;
}

bool __fastcall WPolySoup::ReadModels983(cFile* file, sHeader& header, std::vector<sModel*>& model)
{
	IFF_STRUCT::sCourse* pCourse = Doc()->m_golfGame.pCourse;
	unsigned char detail = !pCourse ? 0 : pCourse->DiffFlag;
	int num;

	switch (detail)
	{
	case 0:
		num = header.modelNum;
		break;
	case 1:
		num = header.modelNum1 + header.modelNum;
		break;
	case 2:
		num = header.modelNum2 + header.modelNum;
		break;
	default:
		num = header.modelNum2 + header.modelNum1 + header.modelNum;
		break;
	}

	if (num < 1)
		return true;

	int total = header.modelNum2 + header.modelNum1 + header.modelNum;
	model.resize(num);

	int n = 0;
	for (int i = 0; i < total; i++)
	{
		bool skipped = false;
		ReadModel983(file, header, detail, model[n], &skipped);
		if (!skipped)
			n++;
		if (n == num)
			return true;
	}
	return true;
}

bool __fastcall WPolySoup::ReadAttach983(cFile* file, sHeader& header, sAttach983*& attach)
{
	if (header.attachNum983 > 0)
	{
		attach = new sAttach983[header.attachNum983];
		for (int i = 0; i < header.attachNum983; i++)
			attach[i].Read(header.version, file); // 983: 0093be30
	}
	return true;
}

bool __fastcall WPolySoup::CompareModel983(sModel* a, sModel* b)
{
	return strcmp(a->name, b->name) < 0;
}

// 983: 00927220 -- pula as cores de vertice do modelo base (o que o ReadVertexColors983 le na 0x72) sem montar o boneco.
bool __fastcall WPolySoup::SkipVertexColors983(cFile* file, sHeader& header)
{
	int groupNum;
	file->Read(&groupNum, 4);
	for (int g = 0; g < groupNum; g++)
	{
		int len;
		file->Read(&len, 4);
		file->Seek(len, SEEK_CUR);
		unsigned int meshNum;
		file->Read(&meshNum, 4);
		for (unsigned int m = 0; m < meshNum; m++)
		{
			int vtxNum;
			file->Read(&vtxNum, 4);
			file->Seek(vtxNum * 4, SEEK_CUR);
		}
	}
	return true;
}

// 983: 00927310 -- primeiro osso de chao do pin.
WPolySoup::sAttach983* __fastcall WPolySoup::FindPinAttach983(sAttach983* attach, unsigned int num)
{
	for (unsigned int i = 0; i < num; i++)
	{
		if (attach[i].node983->IsPin983())
			return &attach[i];
	}
	return NULL;
}

// 983: 00927360 -- tee de distancia (time) mais alta ate `range`; sem nenhum, o de distancia mais baixa acima de `range`.
WPolySoup::sAttach983* __fastcall WPolySoup::FindTeeAttach983(sAttach983* attach, unsigned int num, float range)
{
	sAttach983* below = NULL;
	sAttach983* above = NULL;
	for (unsigned int i = 0; i < num; i++)
	{
		if (attach[i].node983->IsTee983())
		{
			if (attach[i].time <= range)
			{
				if (below == NULL || below->time < attach[i].time)
					below = &attach[i];
			}
			if (attach[i].time > range)
			{
				if (above == NULL || above->time > attach[i].time)
					above = &attach[i];
			}
		}
	}
	return below ? below : above;
}

// 983: 00936f80 -- todos os ossos de chao de tee, na ordem do arquivo.
void __fastcall WPolySoup::CollectTeeAttach983(sAttach983* attach, unsigned int num, std::vector<sAttach983*>& tees)
{
	for (unsigned int i = 0; i < num; i++)
	{
		if (attach[i].node983->IsTee983())
			tees.push_back(&attach[i]);
	}
}

// 983: 009385c0 -- o mapa precisa de um pin e de pelo menos um tee; o transformador atual (modo 0xb) reposiciona os tees.
bool __fastcall WPolySoup::TransformGroundBones983(sAttach983* attach, unsigned int num)
{
	std::vector<sAttach983*> tees;
	sAttach983* pin = FindPinAttach983(attach, num);
	CollectTeeAttach983(attach, num, tees);
	if (pin == NULL || tees.size() == 0)
		return false;
	if (!ms_pTeeShotTransformer->Transform(pin, tees))
		return false;
	return true;
}

bool WPolySoup::PostCameras983()
{
	return true;
}

bool WPolySoup::ApplyPointLights983()
{
	for (int i = 0; i < m_header.pointNum; i++)
	{
		sPoint* point = &m_point[i];
		switch (point->type)
		{
		case 1:
			sscanf(point->option.c_str(), "%x %x %x", &g_lightset.diffuse, &g_lightset.ambient,
				&g_lightset.ambient2);
			g_lightset.diffuse = ModifyColor(g_lightset.diffuse, GOLFDOC()->m_bright);
			g_lightset.ambient = ModifyColor(g_lightset.ambient, GOLFDOC()->m_bright);
			g_lightset.ambient2 = ModifyColor(g_lightset.ambient2, GOLFDOC()->m_bright);
			break;
		case 3:
			GOLFDOC()->SetDay(false);
			break;
		}
	}
	return true;
}

bool WPolySoup::PostAreas983()
{
	return true;
}

bool WPolySoup::LoadTextures983()
{
	for (int i = 0; i < m_header.textureNum; i++)
	{
		sTexture* texture = &m_texture[i];
		texture->texHandle = GetResrcManager()->LoadTexture(texture->name, 0, 0, NULL);
		texture->propIndex = GetPropertyIndex(texture->name);
		m_texIndex[texture->texHandle] = i;
	}
	return true;
}

bool WPolySoup::CheckCameraNodes983()
{
	m_bCameraArea = false;
	for (int i = 0; i < m_header.nodeNum; i++)
	{
		if (m_node[i].GetType() == CNodeContainer::TYPE_CAMERA)
			m_bCameraArea = true;
	}
	return true;
}

bool WPolySoup::LoadBaseModel983(cFile* file)
{
	m_baseModel->pet = g_resrcmng->GetPuppet(m_baseModel->name, false, false, false);
	m_baseModel->pet->m_lightmode = 0x1000000;
	m_vtxColors983.Read(m_header.version, file, m_baseModel->pet);

	sModel* base = m_baseModel;
	w_bound_box* center = m_baseModel->pet->m_bbList.Find("center");
	if (center && center->info->option)
	{
		WSliceStr slice(center->info->option);
		slice.Get("*wind", "%f", &base->windFactor);
	}

	if (m_baseModel->pet->m_bbList.Find("npc"))
		m_npcList.push_back(0xffff);

	CSceneManager::Instance()->RegisterElement(OBJ_BASE, m_baseModel->pet, NULL, m_baseModel->name, 3, NULL);
	return true;
}

bool WPolySoup::LoadModels983()
{
	m_modelNum = (int)m_model.size();
	if (m_modelNum > 0)
	{
		int animatedNum = 0;
		for (int i = 0; i < m_modelNum; i++)
		{
			sModel* model = m_model[i];
			model->pet = g_resrcmng->GetPuppet(model->name, IsSeaPet(model->name), model->type != 0, false);
			if (model->type)
				animatedNum++;

			if (model->pet == NULL && m_pDoc->m_modelNameList.Find(model->name) == NULL)
			{
				char* name = new char[strlen(model->name) + 1];
				strcpy(name, model->name);
				m_pDoc->m_modelNameList.AddItem(name, name, false);
			}
		}

		if (m_modelNum > 1)
			std::sort(m_model.begin(), m_model.end(), CompareModel983);

		GetPVS().Reserve((short)animatedNum);
	}
	return true;
}

bool WPolySoup::FindAttachBones983()
{
	for (int i = 0; i < m_header.attachNum983; i++)
	{
		sAttach983* attach = &m_attach983[i];
		attach->bone = m_baseModel->pet->FindBone(attach->name);
	}
	return true;
}

void WPolySoup::SetLightDir983()
{
	for (int i = 0; i < m_header.pointNum; i++)
	{
		sPoint* point = &m_point[i];
		if (point->type == 1)
		{
			g_lightset.nearOne = point->pos;
			if (g_lightset.type == 2)
			{
				g_lightset.nearOne *= -1.0f;
				g_lightset.nearOne.Normalize();
			}
			return;
		}
	}
}

void WPolySoup::ProcessAreas983(bool bOption)
{
	LoadSoundBoxes983();
	SetExtraHoleArea983();
	LoadBgFx983(bOption);
}

void WPolySoup::LoadSoundBoxes983()
{
	for (int i = 0; i < m_header.areaNum; i++)
	{
		sArea* area = &m_area[i];
		if (area->type == 5)
		{
			sSoundBox sound;
			sound.interval = -1.0f;
			sound.distance = 60.0f;
			sound.time = 0.0f;
			sscanf(area->name, "%s %f %f", sound.name, &sound.distance, &sound.interval);
			sound.obb = area->aabb;
			m_soundBox.push_back(sound);
		}
	}
}

// 983: so com a regra do jogo pedindo (*extra: m_bNoStat +0x194; *extra_shortgame: +0x195).
void WPolySoup::SetExtraHoleArea983()
{
	if (!GOLFDOC()->m_bNoStat && !GOLFDOC()->m_b195_983)
		return;

	for (int i = 0; i < m_header.areaNum; i++)
	{
		sArea* area = &m_area[i];
		if (area && ((GOLFDOC()->m_bNoStat && strstr(area->name, "*extra")) ||
						(GOLFDOC()->m_b195_983 && strstr(area->name, "*extra_shortgame"))))
		{
			m_extraHoleStartArea = area->aabb;
			return;
		}
	}
}

void WPolySoup::LoadBgFx983(bool bOption)
{
	m_bgFxNum = 0;
	for (int i = 0; i < m_header.areaNum; i++)
	{
		sArea* area = &m_area[i];

		if (IsSilviaCannonArea(area))
			continue;

		if (area->type != 6)
			continue;

		Attacher attacher;
		char fxName[64];
		WVector rotation;

		WSliceStr slice(area->name);
		slice.Get("*fx", "%s %f %f %f", fxName, &rotation.x, &rotation.y,
			&rotation.z);

		if (bOption)
		{
			if (GetCurMap() == 4 || GetCurMap() == 5)
			{
				if (stricmp(fxName, "CL_Default.spr") == 0)
					continue;
			}
		}

		m_bgFx[m_bgFxNum].area = area;
		m_bgFx[m_bgFxNum].mat.Reset();
		m_bgFx[m_bgFxNum].mat.Rotate(rotation);
		m_bgFx[m_bgFxNum].mat.pivot = (area->aabb.min + area->aabb.max) * 0.5f;
		m_bgFx[m_bgFxNum].offset.Reset();
		attacher.mat[0] = &m_bgFx[m_bgFxNum].mat;
		attacher.pos[0] = &m_bgFx[m_bgFxNum].offset;

		void* fx = CFx::Instance()->Attach(attacher, fxName, false, NULL);
		if (fx == NULL)
			continue;

		char* ext = strrchr(fxName, '.');
		if (ext == NULL || strlen(ext) < 4)
			continue;

		if (stricmp(ext + 1, "spr") == 0)
		{
			((CFxSpray*)fx)->SetLight(g_lightset);
			m_bgFx[m_bgFxNum].spray = (CFxSpray*)fx;
		}
		else if (stricmp(ext + 1, "seq") == 0)
		{
			((CFxSequence*)fx)->SetLight(g_lightset);
			m_bgFx[m_bgFxNum].sequence = (CFxSequence*)fx;
		}
		else
		{
			continue;
		}

		m_bgFxNum++;

		if (m_bgFxNum >= 256)
			break;
	}

}

// 983: 0093feb0. As cores de vertice guardadas no +0x49b0 sao aplicadas antes de montar os triangulos.
void WPolySoup::PostBaseModel983()
{
	m_baseModel->pet->UpdateBound(false, false);
	m_vtxColors983.Apply(m_baseModel->pet);
	BuildBaseTriangles983();
	m_aabb_e0_983 = m_baseModel->aabb;
}

// 983: Waabb::AddPoint embutido no 0093a850 (o cl nao embute o do cabecalho aqui).
static __forceinline void AddPoint983(Waabb& box, const WVector& vector)
{
	if (vector.x > box.max.x)
		box.max.x = vector.x;
	if (vector.x < box.min.x)
		box.min.x = vector.x;
	if (vector.y > box.max.y)
		box.max.y = vector.y;
	if (vector.y < box.min.y)
		box.min.y = vector.y;
	if (vector.z > box.max.z)
		box.max.z = vector.z;
	if (vector.z < box.min.z)
		box.min.z = vector.z;
}

// 983: 0093a850 -- para cada osso do modelo base (WPuppet::ForEachBone983): triangulos das malhas no espaco do
// mundo (WBone::GetWorldVertices983), caixa e plano. O 983 nao monta lista de triangulos por textura (m_triList do 645).
void WPolySoup::BaseTriCallback983(WPuppet* pet, WBone* bone, void* param)
{
	sBaseTriContext983* ctx = (sBaseTriContext983*)param;
	sModel* model = ctx->soup->m_baseModel;
	int index = ctx->index;

	for (w_mesh* mesh = bone->GetMesh(); mesh; mesh = mesh->next)
	{
		std::vector<WVector> vtx;
		WBone::GetWorldVertices983(bone, mesh, vtx);

		for (int j = 0; j < mesh->indexNum; j += 3)
		{
			model->tri[index].v[0] = vtx[mesh->indexList[j]];
			AddPoint983(ctx->box, model->tri[index].v[0]);
			model->tri[index].v[1] = vtx[mesh->indexList[j + 1]];
			AddPoint983(ctx->box, model->tri[index].v[1]);
			model->tri[index].v[2] = vtx[mesh->indexList[j + 2]];
			AddPoint983(ctx->box, model->tri[index].v[2]);

			WVector* v = model->tri[index].v;
			model->tri[index].plane.normal = WCrossProduct(v[2] - v[1], v[0] - v[1]);
			model->tri[index].plane.normal.Normalize();
			model->tri[index].plane.d = (model->tri[index].plane.normal * -1.0f) * v[0];
			// 983: bit 9 do +0xb4 da malha (o 983 testa 0x200 ali em todo o wbone; layout do w_mesh do 983 pendente)
			model->tri[index].bDoubleSide = ((unsigned long)mesh->xiDrawFlag2 >> 9) & 1;
			model->tri[index].propIndex =
				ctx->soup->m_texture[ctx->soup->m_texIndex[mesh->texHandle]].propIndex;
			model->tri[index].texHandle =
				ctx->soup->m_texture[ctx->soup->m_texIndex[mesh->texHandle]].texHandle;

			if (ctx->bBaseMeshTest)
			{
				if (model->tri[index].plane.b < 0.0f)
				{
					*(unsigned short*)&mesh->vtxColorList[mesh->indexList[j]] = 0;
					*(unsigned short*)&mesh->vtxColorList[mesh->indexList[j + 1]] = 0;
					*(unsigned short*)&mesh->vtxColorList[mesh->indexList[j + 2]] = 0;
				}
				else if (Wabs(model->tri[index].plane.b) < g_EPSILON)
				{
					mesh->vtxColorList[mesh->indexList[j]] &= 0xff0000ff;
					mesh->vtxColorList[mesh->indexList[j + 1]] &= 0xff0000ff;
					mesh->vtxColorList[mesh->indexList[j + 2]] &= 0xff0000ff;
				}
			}

			index++;
		}
	}

	ctx->index = index;
}

// 983: 0093e5e0
void WPolySoup::BuildBaseTriangles983()
{
	m_baseModel->tri.resize(m_baseModel->pet->GetFaceNum());

	bool bBaseMeshTest = COption::Instance()->dIsBaseMeshTestEnabled();
	sBaseTriContext983 ctx;
	ctx.bBaseMeshTest = bBaseMeshTest;
	ctx.soup = this;
	ctx.index = 0;
	WPuppet::ForEachBone983(m_baseModel->pet, BaseTriCallback983, &ctx);

	m_baseModel->aabb = ctx.box;
	m_baseModel->pos = WVector((m_baseModel->aabb.max.x + m_baseModel->aabb.min.x) * 0.5,
		(m_baseModel->aabb.max.y + m_baseModel->aabb.min.y) * 0.5,
		(m_baseModel->aabb.max.z + m_baseModel->aabb.min.z) * 0.5);
	m_baseModel->center = m_baseModel->pos;
	m_baseModel->radius = (m_baseModel->aabb.max - m_baseModel->pos).Magnitude();
}

// 983: 00938750 -- caixa do modelo = vertices do mundo de todas as malhas.
static void __fastcall ModelBoxCallback983(WPuppet* pet, WBone* bone, void* param)
{
	Waabb* box = (Waabb*)param;
	for (w_mesh* mesh = bone->GetMesh(); mesh; mesh = mesh->next)
	{
		std::vector<WVector> vtx;
		WBone::GetWorldVertices983(bone, mesh, vtx);
		for (unsigned int i = 0; i < vtx.size(); i++)
			box->AddPoint(vtx[i]);
	}
}

struct sModelFootContext983
{
	Waabb limit;
	Waabb box;
};

// 983: 00938800 -- so os vertices dentro de ctx->limit (ate 1.0 acima da base do modelo).
static void __fastcall ModelFootCallback983(WPuppet* pet, WBone* bone, void* param)
{
	sModelFootContext983* ctx = (sModelFootContext983*)param;
	for (w_mesh* mesh = bone->GetMesh(); mesh; mesh = mesh->next)
	{
		std::vector<WVector> vtx;
		WBone::GetWorldVertices983(bone, mesh, vtx);
		for (unsigned int i = 0; i < vtx.size(); i++)
		{
			const WVector& v = vtx[i];
			if (ctx->limit.min.x <= v.x && v.x <= ctx->limit.max.x &&
				ctx->limit.min.y <= v.y && v.y <= ctx->limit.max.y &&
				ctx->limit.min.z <= v.z && v.z <= ctx->limit.max.z)
				ctx->box.AddPoint(v);
		}
	}
}

// 983: 0093e780. bbox = pe do modelo (vertices ate 1.0 acima da base, 10 de altura): e ela que o
// OverlappedWithObject testa; com a aabb inteira, modelo largo vira area nao jogavel (Wiz City, B1591).
// O bit 0x40 (00b6a330) = SetBoneMeshDepth983 (D903).
// 983 00b6a330 (WPuppet) / 00b951e0 (WBone, recursivo como o DisableFog): grava w_mesh+0xd4 em todas as malhas; o
// WxTnLBuffer usa o valor no lugar da profundidade calculada dos translucidos. Fora dos headers do motor (sem rebuild geral).
void SetBoneMeshDepth983(WBone* bone, float depth)
{
	do
	{
		for (w_mesh* mesh = bone->GetMesh(); mesh != 0; mesh = mesh->next)
			mesh->f_d4_983 = depth;
		if (bone->GetChild() != 0)
			SetBoneMeshDepth983(bone->GetChild(), depth);
		bone = bone->GetNext();
	} while (bone != 0);
}

void WPolySoup::PostModels983()
{
	for (int i = 0; i < m_modelNum; i++)
	{
		sModel* model = m_model[i];
		if (model->pet == NULL)
			continue;

		model->pet->UpdateBound(true, false);
		model->pet->UpdateLightSource(&g_lightset, false, 0);
		// 983 parte de +/-FLT_MAX (00d3cba0/00d12ef8): com Waabb() zerado toda caixa inclui a origem do mapa e o modelo
		// entra em todos os quads ate (0,0), mais de 32 objetos no raio estouram m_objList (queda dono23 Praticar, D989).
		Waabb box;
		box.Clear();
		WPuppet::ForEachBone983(model->pet, ModelBoxCallback983, &box);
		model->aabb = box;
		if (!model->type)
			m_aabb_e0_983 += model->aabb;

		if (model->aabb.max.y - model->aabb.min.y >= 10.0)
		{
			sModelFootContext983 foot;
			foot.limit = model->aabb;
			foot.box.Clear();
			foot.limit.max.y = model->aabb.min.y + 1.0;
			WPuppet::ForEachBone983(model->pet, ModelFootCallback983, &foot);
			model->bbox = foot.box;
			model->bbox.max.y = model->bbox.min.y + 10.0;
		}
		else
			model->bbox = model->aabb;

		model->pos = (model->aabb.max + model->aabb.min) * 0.5f;
		model->center = model->pos;
		model->radius = (model->aabb.max - model->pos).Magnitude();

		unsigned long flags = 0;
		if (model->type)
			flags = 4;
		if ((model->flag & 0x10) || model->pet->m_boundSphere.radius > 128.0f)
			flags |= 2;
		if (model->flag & 8)
			flags |= 1;
		if (model->flag & 0x20)
			model->pet->DisableFog();
		if (model->flag & 0x40)
			if (model->pet->GetRootBone() != 0)
				SetBoneMeshDepth983(model->pet->GetRootBone(), FLT_MAX); // 983 0093e780 (00b6a330): ordenado como o mais distante

		CSceneManager::Instance()->RegisterElement(model->type ? OBJ_MODEL : OBJ_STATIC, model->pet, NULL,
			model->name, flags, &model->bbox);
		ProcessPostLoadPet(model);
	}
}

// 983: 00926f00 -- pontos de saida e chegada do buraco pelo nome (a tabela do fim do arquivo do 645 saiu na versao 0x72).
// spot[0] = normal, spot[1] = dificil (GetDiffFlag 2); tee = "sijakjeom" / "sijakjeom2", pin[n] = "kkeutjeom1_n" /
// "kkeutjeom2_n" (n = m_courseMap 0..2); "kkeutjeom" preenche os pins que faltarem. Nome meu.
struct sTeePinPoints983 // 0x10
{
	WPolySoup::sPoint* tee;
	WPolySoup::sPoint* pin[3];

	sTeePinPoints983() : tee(NULL)
	{
		for (int i = 0; i < 3; i++)
			pin[i] = NULL;
	}
};

static bool FindTeePinPoints983(WPolySoup::sPoint* point, unsigned int num, sTeePinPoints983* spot)
{
	WPolySoup::sPoint* anyPin = NULL;
	for (unsigned int i = 0; i < num; i++)
	{
		if (strcmp(point[i].name, "\xbd\xc3\xc0\xdb\xc1\xa1") == 0)
			spot[0].tee = &point[i];
		else if (strcmp(point[i].name, "\xbd\xc3\xc0\xdb\xc1\xa1" "2") == 0)
			spot[1].tee = &point[i];
		else if (strcmp(point[i].name, "\xb3\xa1\xc1\xa1" "1_1") == 0)
			spot[0].pin[0] = &point[i];
		else if (strcmp(point[i].name, "\xb3\xa1\xc1\xa1" "1_2") == 0)
			spot[0].pin[1] = &point[i];
		else if (strcmp(point[i].name, "\xb3\xa1\xc1\xa1" "1_3") == 0)
			spot[0].pin[2] = &point[i];
		else if (strcmp(point[i].name, "\xb3\xa1\xc1\xa1" "2_1") == 0)
			spot[1].pin[0] = &point[i];
		else if (strcmp(point[i].name, "\xb3\xa1\xc1\xa1" "2_2") == 0)
			spot[1].pin[1] = &point[i];
		else if (strcmp(point[i].name, "\xb3\xa1\xc1\xa1" "2_3") == 0)
			spot[1].pin[2] = &point[i];
		else if (strcmp(point[i].name, "\xb3\xa1\xc1\xa1") == 0)
			anyPin = &point[i];
	}

	if (spot[0].tee && !spot[1].tee)
		spot[1].tee = spot[0].tee;

	if (anyPin)
	{
		for (int i = 0; i < 3; i++)
		{
			if (!spot[0].pin[i])
				spot[0].pin[i] = anyPin;
			if (!spot[1].pin[i])
				spot[1].pin[i] = anyPin;
		}
	}

	if (!spot[0].tee)
		return false;
	for (unsigned int i = 0; i < 3; i++)
	{
		if (!spot[0].pin[i])
			return false;
	}
	if (!spot[1].tee)
		return false;
	for (unsigned int i = 0; i < 3; i++)
	{
		if (!spot[1].pin[i])
			return false;
	}
	return true;
}

// 983: 0092f910 -- soma de verificacao do inicio do arquivo (n = 1o byte).
bool WPolySoup::ReadMapCheckSum983(cFile* file, unsigned char hole)
{
	unsigned char c;
	file->Seek(0, SEEK_SET);
	int len = file->GetByte();
	GOLFDOC()->GetHoleData(hole).checkSum = 0;
	for (int i = 0; i < len; i++)
	{
		file->Read(&c, 1);
		GOLFDOC()->GetHoleData(hole).checkSum += c;
	}
	return true;
}

// 983: 0093fc80 -- tee e pin do buraco pelos pontos com nome (fora da sala 0xb, que os tira dos ossos de chao).
// O indice do pin vem do buraco ATUAL (m_currentHole), nao do `hole` (assim no 983).
bool WPolySoup::ReadTeePinPoints983(cFile* file, unsigned char hole)
{
	if (Doc()->m_golfGame.gameType == 0xb)
		return true;

	file->Seek(0, SEEK_SET);
	sHeader header;
	if (!ReadHeader983(file, header))
		return false;

	sMapData983 data(true, false);
	if (!ReadCameras983(file, header, data.camera) || !ReadPoints983(file, header, data.point) ||
		!ReadAreas983(file, header, data.area))
		return false;

	data.cameraNum = header.cameraNum;
	data.pointNum = header.pointNum;
	data.areaNum = header.areaNum;
	if (!data.BuildNodes983())
		return false;

	sTeePinPoints983 spot[2];
	if (!FindTeePinPoints983(data.point, data.pointNum, spot))
		return false;

	sTeePinPoints983* use = GetDiffFlag() == 2 ? &spot[1] : &spot[0];
	unsigned char course = Doc()->m_courseMap[GetHoleIndex(GOLFDOC()->m_currentHole) - 1];
	GOLFDOC()->GetHoleData(hole).tee.x = use->tee->pos.x;
	GOLFDOC()->GetHoleData(hole).tee.z = use->tee->pos.z;
	GOLFDOC()->GetHoleData(hole).pin.x = use->pin[course]->pos.x;
	GOLFDOC()->GetHoleData(hole).pin.z = use->pin[course]->pos.z;
	return true;
}

// 983: 0093f7a0 -- com m_bNoStat (fora da sala 0xb), o tee sai sorteado dentro da area "*extra" (m_holeRandom).
bool WPolySoup::ReadExtraTeeArea983(cFile* file, unsigned char hole)
{
	if (Doc()->m_golfGame.gameType == 0xb || !GOLFDOC()->m_bNoStat)
		return true;

	file->Seek(0, SEEK_SET);
	sHeader header;
	if (!ReadHeader983(file, header))
		return false;

	sMapData983 data(true, false);
	if (!ReadCameras983(file, header, data.camera) || !ReadPoints983(file, header, data.point) ||
		!ReadAreas983(file, header, data.area))
		return false;

	data.cameraNum = header.cameraNum;
	data.pointNum = header.pointNum;
	data.areaNum = header.areaNum;
	if (!data.BuildNodes983())
		return false;

	for (unsigned int i = 0; i < (unsigned int)data.areaNum; i++)
	{
		sArea* area = &data.area[i];
		if (strstr(area->name, "*extra"))
		{
			Waabb* extra = &area->aabb;
			if (extra == NULL)
				break;
			if (GOLFDOC()->m_currentHole == hole)
				m_extraHoleStartArea = *extra;
			GOLFDOC()->GetHoleData(hole).tee.x = (float)((extra->max.x - extra->min.x) *
				(float)(Doc()->m_holeRandom[0] & 0x7f) * (1.0 / 128) + extra->min.x);
			GOLFDOC()->GetHoleData(hole).tee.z = (float)((extra->max.z - extra->min.z) *
				(float)(Doc()->m_holeRandom[1] & 0x7f) * (1.0 / 128) + extra->min.z);
			return true;
		}
	}

	if (!NET()->IsConnected(WNetworkSystem::NET_GAME))
		MessageBox(NULL,
			MakeStr(K2L_Compatibility(
				"%d\xc8\xa6\xbf\xa1 \xbe\xee\xc7\xc1\xb7\xce\xc4\xa1 \xbf\xb5\xbf\xaa\xc0\xcc \xbc\xb3\xc1\xa4\xb5\xc7"
				"\xbe\xee \xc0\xd6\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9"),
				hole),
			"Warning", 0);
	return true;
}

// 983: 0092f9a0 -- par do buraco no ultimo byte (antes da tabela de 0x40 B das versoes < 0x72).
bool WPolySoup::ReadHolePar983(cFile* file, unsigned char hole)
{
	file->Seek(0, SEEK_SET);
	sHeader header;
	if (!ReadHeader983(file, header))
		return false;

	int tail = 0;
	if ((int)header.version < 0x72)
		tail = 0x40;
	file->Seek(-1 - tail, SEEK_END);
	file->Read(&GOLFDOC()->GetHoleData(hole).par, 1);
	Doc()->m_holePar[hole - 1] = GOLFDOC()->GetHoleData(hole).par;
	return true;
}

// 983: 0093fa40 -- *extra_shortgame (+0x195): o tee sai sorteado na 1a area do "<curso>_<buraco>.sgbin" (o buraco
// sem o GetHoleValue983). No 983 o arquivo so e liberado no caminho de sucesso (delete[]); nos outros fica aberto.
bool WPolySoup::LoadShortGameTee983(unsigned char hole)
{
	cFile* file = GetResrcManager()->GetCFile(
		MakeStr("%s_%02d.sgbin", Doc()->m_golfGame.pCourse ? Doc()->m_golfGame.pCourse->Data : NULL, hole), -1);
	if (file == NULL)
		return false;

	file->Seek(0, SEEK_SET);
	sHeader header;
	if (!ReadHeader983(file, header))
		return false;

	sMapData983 data(true, false);
	if (!ReadAreas983(file, header, data.area))
		return false;
	if (header.areaNum < 1)
		return false;

	data.areaNum = header.areaNum;
	if (!data.BuildNodes983())
		return false;

	Waabb* area = &data.area[0].aabb;
	if (area == NULL)
		return false;

	if (GOLFDOC()->m_currentHole == hole)
		m_extraHoleStartArea = *area;
	GOLFDOC()->GetHoleData(hole).tee.x = (float)((area->max.x - area->min.x) *
		(float)(Doc()->m_holeRandom[0] & 0x7f) * (1.0 / 128) + area->min.x);
	GOLFDOC()->GetHoleData(hole).tee.z = (float)((area->max.z - area->min.z) *
		(float)(Doc()->m_holeRandom[1] & 0x7f) * (1.0 / 128) + area->min.z);
	delete[] file;
	return true;
}

// 983: 009403b0 -- abre o mapa do buraco (numero pelo GetHoleValue983) e roda os passos acima no arquivo aberto.
bool WPolySoup::LoadMapCheckData(unsigned char hole)
{
	unsigned char mapHole = DOCJP()->GetHoleValue983(GetHoleIndex(hole));
	cFile* file = GetResrcManager()->GetCFile(
		MakeStr("%s_%02d.gbin", Doc()->m_golfGame.pCourse ? Doc()->m_golfGame.pCourse->Data : NULL, mapHole),
		-1);
	if (file == NULL)
		return false;

	if (!ReadMapCheckSum983(file, hole) || !ReadTeePinPoints983(file, hole) || !ReadExtraTeeArea983(file, hole) ||
		!ReadHolePar983(file, hole))
	{
		CloseCFile(file);
		return false;
	}
	CloseCFile(file);

	if (GOLFDOC()->m_b195_983 == true && GOLFDOC()->GetHoleData(hole).par != 3)
		LoadShortGameTee983(hole);
	return true;
}

// 983: 00941600 -- le o mapa do buraco inteiro (registros num sMapData983 dono, so para achar os ossos de chao) e devolve
// (x, z) do pin e do tee de distancia mais alta ate `range` (o mais perto acima, se nao houver). Chamado pela regra do
// Hole-in-One (00661eb0), que grava os dois no sHoleData do buraco. Nome meu.
bool WPolySoup::GetTeePinByRange983(unsigned char hole, WVector2D* pPin, float range, WVector2D* pTee)
{
	unsigned char mapHole = DOCJP()->GetHoleValue983(GetHoleIndex(hole));
	cFile* file = GetResrcManager()->GetCFile(
		MakeStr("%s_%02d.gbin", Doc()->m_golfGame.pCourse ? Doc()->m_golfGame.pCourse->Data : NULL, mapHole),
		-1);
	if (file == NULL)
		return false;

	sHeader header;
	if (!ReadHeader983(file, header))
	{
		CloseCFile(file);
		return false;
	}

	sMapData983 data(true, false);
	if (!ReadCameras983(file, header, data.camera) || !ReadPoints983(file, header, data.point) ||
		!ReadAreas983(file, header, data.area) || !ReadTextures983(file, header, data.texture) ||
		!ReadNodes983(file, header, data.node) ||
		!ReadModel983(file, header, GetDiffFlag(), data.baseModel, NULL) || !SkipVertexColors983(file, header) ||
		!ReadModels983(file, header, data.model) || !ReadAttach983(file, header, data.attach))
	{
		CloseCFile(file);
		return false;
	}
	CloseCFile(file);

	data.cameraNum = header.cameraNum;
	data.pointNum = header.pointNum;
	data.areaNum = header.areaNum;
	data.attachNum = header.attachNum983;
	if (!data.BuildNodes983() || !TransformGroundBones983(data.attach, data.attachNum))
		return false;

	sAttach983* pin = FindPinAttach983(data.attach, data.attachNum);
	pPin->x = pin->mat.pivot.x;
	pPin->y = pin->mat.pivot.z;
	sAttach983* tee = FindTeeAttach983(data.attach, data.attachNum, range);
	pTee->x = tee->mat.pivot.x;
	pTee->y = tee->mat.pivot.z;
	return true;
}

// 983: 009418a0 -- como o GetTeePinByRange983, mas devolve as posicoes (x, z) de todos os tees, na ordem do arquivo.
// Chamado pela regra do Hole-in-One (00661f30), que grava o primeiro como tee do buraco. Nome meu.
void WPolySoup::GetPinAndTees983(unsigned char hole, WVector2D* pPin, std::vector<WVector2D>* pTees)
{
	unsigned char mapHole = DOCJP()->GetHoleValue983(GetHoleIndex(hole));
	cFile* file = GetResrcManager()->GetCFile(
		MakeStr("%s_%02d.gbin", Doc()->m_golfGame.pCourse ? Doc()->m_golfGame.pCourse->Data : NULL, mapHole),
		-1);
	if (file == NULL)
		return;

	sHeader header;
	if (!ReadHeader983(file, header))
	{
		CloseCFile(file);
		return;
	}

	sMapData983 data(true, false);
	if (!ReadCameras983(file, header, data.camera) || !ReadPoints983(file, header, data.point) ||
		!ReadAreas983(file, header, data.area) || !ReadTextures983(file, header, data.texture) ||
		!ReadNodes983(file, header, data.node) ||
		!ReadModel983(file, header, GetDiffFlag(), data.baseModel, NULL) || !SkipVertexColors983(file, header) ||
		!ReadModels983(file, header, data.model) || !ReadAttach983(file, header, data.attach))
	{
		CloseCFile(file);
		return;
	}
	CloseCFile(file);

	data.cameraNum = header.cameraNum;
	data.pointNum = header.pointNum;
	data.areaNum = header.areaNum;
	data.attachNum = header.attachNum983;
	if (!data.BuildNodes983() || !TransformGroundBones983(data.attach, data.attachNum))
		return;

	sAttach983* pin = FindPinAttach983(data.attach, data.attachNum);
	pPin->x = pin->mat.pivot.x;
	pPin->y = pin->mat.pivot.z;
	for (unsigned int i = 0; i < (unsigned int)data.attachNum; i++)
	{
		if (data.attach[i].node983->IsTee983())
		{
			float x = data.attach[i].mat.pivot.x;
			float z = data.attach[i].mat.pivot.z;
			pTees->push_back(WVector2D(x, z));
		}
	}
}

int WPolySoup::CompareModel(const void* a, const void* b)
{
	sModel* m1 = *(sModel**)a;
	sModel* m2 = *(sModel**)b;

	int ret = strcmpi(m1->name, m2->name);

	if (ret)
		return ret;

	return m1->flag < m2->flag ? -1 : m1->type > m2->type;
}

static bool IsSilviaCannonArea(const WPolySoup::sArea* area)
{
	if (area->type != 6)
		return false;

	WSliceStr slice(area->name);
	char name[64];
	float value[3];

	slice.Get("*fx", "%s %f %f %f", name, &value[0], &value[1], &value[2]);

	if (strcmpi(name, "silviacannon.seq") == 0)
		return true;

	return false;
}

// 983: 009372a0
void sVertexColorArray983::Resize(unsigned int num)
{
	colors.resize(num, 0xffffffff);
}

// 983: 0092caa0
void sVertexColorArray983::Set(unsigned int index, unsigned long color)
{
	colors[index] = color;
}

// 983: 0092cac0 -- so se o numero de cores for o de vertices da malha.
void sVertexColorArray983::ApplyTo(w_mesh* mesh)
{
	if (colors.size() != mesh->vtxNum)
		return;
	for (unsigned int i = 0; i < colors.size(); i++)
		mesh->vtxColorList[i] = colors[i];
}

// 983: 00948730
sVertexColorArray983& cVertexColorTable983::Get(const std::string& bone, unsigned int mesh, unsigned int vtxNum)
{
	MeshColors& meshes = m_bones[bone];
	sVertexColorArray983& colors = meshes[mesh];
	colors.Resize(vtxNum);
	return colors;
}

// 983: 00948bf0. Ate a 0x71: uma cor por indice das malhas do osso raiz, na ordem dos triangulos; na 0x72:
// [n ossos] { [tam][nome] [n malhas] { [n vertices][cor]... } }. Sem ModifyColor (o 983 aplica as cores cruas).
void cVertexColorTable983::Read(int version, cFile* file, WPuppet* pet)
{
	if (version < 0x72)
	{
		WBone* root = pet->GetRootBone();
		std::string name(root->m_name);
		unsigned int m = 0;
		for (w_mesh* mesh = root->GetMesh(); mesh; mesh = mesh->next)
		{
			sVertexColorArray983& colors = Get(name, m, mesh->vtxNum);
			for (int j = 0; j < mesh->indexNum; j += 3)
			{
				for (int k = 0; k < 3; k++)
				{
					unsigned short index = mesh->indexList[j + k];
					unsigned long color;
					file->Read(&color, 4);
					if (color & 0xff000000)
						color |= 0xff000000;
					colors.Set(index, color);
				}
			}
			m++;
		}
	}
	else
	{
		int boneNum;
		file->Read(&boneNum, 4);
		for (int b = 0; b < boneNum; b++)
		{
			int len;
			char name[260];
			file->Read(&len, 4);
			file->Read(name, len);
			name[len] = 0;

			unsigned int meshNum;
			file->Read(&meshNum, 4);
			for (unsigned int m = 0; m < meshNum; m++)
			{
				unsigned int vtxNum;
				file->Read(&vtxNum, 4);
				sVertexColorArray983& colors = Get(std::string(name), m, vtxNum);
				for (unsigned int v = 0; v < vtxNum; v++)
				{
					unsigned long color;
					file->Read(&color, 4);
					colors.Set(v, color);
				}
			}
		}
	}
}

// 983: 009361b0 (ecx = osso, edx = cores por malha). So se o osso tiver o mesmo numero de malhas. Nome meu.
void ApplyBoneColors983(WBone* bone, cVertexColorTable983::MeshColors& meshes)
{
	if (bone->GetMeshNum983() != meshes.size())
		return;

	unsigned int i = 0;
	for (w_mesh* mesh = bone->GetMesh(); mesh; mesh = mesh->next)
	{
		cVertexColorTable983::MeshColors::iterator it = meshes.find(i);
		if (it != meshes.end())
			it->second.ApplyTo(mesh);
		i++;
	}
}

// 983: 009372f0
void cVertexColorTable983::Apply(WPuppet* pet)
{
	for (std::map<std::string, MeshColors>::iterator it = m_bones.begin(); it != m_bones.end(); ++it)
	{
		WBone* bone = pet->FindBone(it->first.c_str());
		if (bone)
			ApplyBoneColors983(bone, it->second);
	}
}

bool WPolySoup::Load(const char* filename, bool bOption)
{
	int caps;

	g_resrcmng->VideoReference()->Command(WX_VDEV_GET_CAPS, 1, (int)&caps);

	if (caps)
		m_bBuildTnL = true;

	Clear();

	g_lightset.type = 2;
	g_lightset.diffuse = 0xffffff;
	g_lightset.ambient = 0x808080;
	g_lightset.ambient2 = 0xffffff;

	if (!LoadProperty983())
		return false;

	strncpy(m_mapName, filename, 63);
	m_mapName[63] = 0;

	cFile* file = GetResrcManager()->GetCFile(filename, -1);
	if (file == NULL)
	{
		MessageBox(NULL,
			MakeStr(K2L_Compatibility("\xb9\xe8\xb0\xe6 \xc6\xc4\xc0\xcf(%s)\xc0\xbb "
									  "\xc0\xd0\xbe\xee\xbf\xc0\xc1\xf6 "
									  "\xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
				filename),
			K2L_Compatibility("\xc6\xc4\xc0\xcf \xc0\xd0\xb1\xe2 \xbd\xc7\xc6\xd0"), 0);
		return false;
	}

	// 983: 00949c90 -- cadeia de leitores (o arquivo inteiro primeiro), depois a hierarquia e os pos-processamentos.
	if (ReadHeader983(file, m_header) && ReadCameras983(file, m_header, m_camera) && PostCameras983() &&
		ReadPoints983(file, m_header, m_point) && ApplyPointLights983() && ReadAreas983(file, m_header, m_area) &&
		PostAreas983() && ReadTextures983(file, m_header, m_texture) && LoadTextures983() &&
		ReadNodes983(file, m_header, m_node) && CheckCameraNodes983())
	{
		if (g_lightset.type == 2)
		{
			g_lightset.nearOne *= -1.0f;
			g_lightset.nearOne.Normalize();
		}

		if (ReadModel983(file, m_header, 0, m_baseModel, NULL) && LoadBaseModel983(file) &&
			ReadModels983(file, m_header, m_model) && LoadModels983() && ReadAttach983(file, m_header, m_attach983) &&
			FindAttachBones983())
		{
			CloseCFile(file);

			// 983: 00949f0e..00949fbe -- hierarquia dos registros (sLink983): cada um vira no, o pai vem do
			// parentId e a matriz do mundo volta para o registro (camera/ponto/area/modelo/osso). Na 0x71 todos
			// os ids sao -1 (filhos da raiz, mundo = local).
			sMapData983 data(false, true);
			data.cameraNum = m_header.cameraNum;
			data.camera = m_camera;
			data.pointNum = m_header.pointNum;
			data.point = m_point;
			data.areaNum = m_header.areaNum;
			data.area = m_area;
			data.baseModel = m_baseModel;
			data.model = m_model;
			data.attachNum = m_header.attachNum983;
			data.attach = m_attach983;
			if (!data.BuildNodes983())
				return false;

			// 983: 00949fc8..0094a027 -- no modo 0xb (Hole-in-One) dentro do CGolfTask o mapa precisa de pin e tees.
			if (IS_KINDOF(CGolfTask, AfxGetTask()) && Doc()->m_golfGame.gameType == 0xb &&
				!TransformGroundBones983(data.attach, data.attachNum))
				return false;

			SetLightDir983();
			ProcessAreas983(bOption);
			PostBaseModel983();
			PostModels983();

			m_bSmapLoadFailed = false;
			if (COption::Instance()->vIsShadowmapEnabled())
				m_bSmapLoadFailed = !LoadShadowmapInfo();

			CFx::Instance()->SetPvsFxBoxGroundHeight(m_baseModel->aabb.max.y, m_baseModel->aabb.min.y);
			return true;
		}
	}

	CloseCFile(file);
	return false;
}

// 983: 0092fa70 -- propriedades das texturas do curso (antes era no meio do Load).
bool WPolySoup::LoadProperty983()
{
	IFF_STRUCT::sCourse* course = ItemManager()->FindCourse(
		((GetCurMap() > 127 && GetCurMap() != 253) ? GetCurMap() - 128 : GetCurMap()) | 0x28000000);

	unsigned char hole = GOLFDOC()->m_currentHole;
	if (hole != 19)
	{
		if (course && course->TexProp[0])
		{
			m_pDoc->LoadXml(course->TexProp, hole - 1, false);
			return true;
		}
		m_pDoc->LoadXml(MakeStr("%s_property.xml", Doc()->m_golfGame.pCourse ? Doc()->m_golfGame.pCourse->Data : NULL),
			GOLFDOC()->m_currentHole - 1, false);
	}
	return true;
}

void WPolySoup::ProcessPostLoadPet(sModel* model)
{
	for (w_bound_box* box = model->pet->m_bbList.Start(); box;
		box = model->pet->m_bbList.Next())
	{
		const char* option = box->info->option;

		if (stricmp(box->info->name, "night_only") == 0)
		{
			if (GOLFDOC()->m_bDay)
			{
				if (box->bone && box->bone->GetChild())
					box->bone->GetChild()->SetInvisible(true, true);
			}
		}
		else if (stricmp(box->info->name, "day_only") == 0)
		{
			if (!GOLFDOC()->m_bDay)
			{
				if (box->bone && box->bone->GetChild())
					box->bone->GetChild()->SetInvisible(true, true);
			}
		}
		else if (stricmp(box->info->name, "center") == 0)
		{
			WSliceStr slice(option);
			slice.Get("*wind", "%f", &model->windFactor);
			slice.Get("*nocol", "%d", &model->noCollision);

			if (option)
			{
				if (strstr(option, "*nofog"))
					model->pet->DisableFog();
				if (strstr(option, "*blend"))
					model->pet->EnableAlphaBlend();
			}
		}
		else if (stricmp(box->info->name, "npc") == 0)
		{
		}
		else if (strstr(box->info->name, "sound"))
		{
			sSoundBox sound;
			sound.interval = -1.0f;
			sound.distance = 60.0f;
			sound.time = 0.0f;
			sscanf(option, "%s %f %f", sound.name, &sound.distance,
				&sound.interval);
			sound.obb = box->bone->GetMatrix();
			sound.obb *= (box->info->aabb.max - box->info->aabb.min) * 0.5f;
			sound.obb.center = box->info->spherePivot * box->bone->GetMatrix();

			m_soundBox.push_back(sound);
		}
		else if (strstr(_strlwr(box->info->name), "abv"))
		{
			box->bone->HideBone(false);
		}

		if (option == NULL)
			continue;

		const char* fx = strstr(option, "*fx");
		if (fx == NULL)
			continue;

		Attacher attacher;
		char cmd[8], file[32] = "", type[32] = "";
		sscanf(fx, "%s %s %s", cmd, file, type);

		if (box->bone == NULL || box->bone->IsInvisible())
			continue;

		const char* ext = strrchr(file, '.');
		if (ext == NULL || strlen(ext) < 4)
			continue;

		const char* extName = ext + 1;
		if (stricmp(extName, "spr") != 0 && stricmp(extName, "seq") != 0)
			continue;

		if (strcmpi(type, "tree") == 0)
		{
			ApplyFx(model, box->bone->GetChild(), file);
		}
		else
		{
			Attacher attacher;
			attacher.mat[0] = &box->bone->GetMatrix();
			attacher.pos[0] = &box->info->spherePivot;

			defines_t defines;
			strcpy(defines.name[0], "pet_scale");
			defines.value[0] = model->scale;
			defines.num = 1;

			if (stricmp(extName, "spr") == 0)
				CFx::Instance()->Attach(attacher, file, false, &defines);
			else
				CFx::Instance()->Attach(attacher, file, false, &defines);
		}
	}
}

bool WPolySoup::IsResisteredBgSequence(const std::string& name)
{
	for (int i = 0; i < m_bgFxNum; i++)
	{
		if (m_bgFx[i].sequence && m_bgFx[i].sequence->m_filename == name)
			return true;
	}

	return false;
}

void WPolySoup::ApplyFx(sModel* model, WBone* bone, const char* fx)
{
	w_bound_box* box = model->pet->FindBoundBox(bone->GetBoneName());

	if (box)
	{
		Attacher attacher;
		attacher.mat[0] = &box->bone->GetMatrix();
		attacher.pos[0] = &box->info->spherePivot;

		defines_t defines;
		strcpy(defines.name[0], "pet_scale");
		defines.value[0] = model->scale;
		defines.num = 1;

		if (stricmp(fx, ".spr") == 0)
			CFx::Instance()->Attach(attacher, fx, false, &defines);
		else
			CFx::Instance()->Attach(attacher, fx, false, &defines);
	}

	if (bone->GetNext())
		ApplyFx(model, bone->GetNext(), fx);

	if (bone->GetChild())
		ApplyFx(model, bone->GetChild(), fx);
}

void WPolySoup::MakeDataStructure()
{
	GetPVS().SetPolySoup(this);

	GetPVS().MakeBaseStructure(m_baseModel, false);

	for (short i = 0; i < m_modelNum; i++)
		GetPVS().MakeObjStructure(i, -1, false);
	m_rmrModelNum = m_modelNum;
}

void WPolySoup::CreateDataModel(sModel* model)
{
	if (!IsLocalContent(S4_REAL_MYROOM))
		return;

	model->id = m_rmrModelNum;

	m_model.push_back(model);

	GetPVS().MakeObjStructure(model->id, m_modelNum, false);

	m_rmrModelNum++;

	m_modelNum++;
}

void WPolySoup::ChangeDataStructure(sModel* model)
{
	int index = 0;

	if (!IsLocalContent(S4_REAL_MYROOM))
		return;

	for (std::vector<sModel*>::iterator it = m_model.begin();
		it != m_model.end(); ++it, ++index)
	{
		if (stricmp((*it)->name, model->name) == 0)
		{
			*it = model;

			GetPVS().MakeObjStructure(model->id, index, true);

			for (int i = 0; i < m_modelNum; i++)
				GetPVS().MakeObjStructure(m_model[i]->id, i, true);

			return;
		}
	}

	CreateDataModel(model);
}

bool WPolySoup::DeleteDataStructure(sModel* model)
{
	bool bResult = false;
	int index = 0;
	int num = m_modelNum;

	if (IsLocalContent(S4_REAL_MYROOM))
	{
		std::vector<sModel*>::iterator it;
		for (it = m_model.begin(); it != m_model.end(); ++it, ++index)
		{
			if (stricmp((*it)->name, model->name) == 0)
			{
				if (!(it != m_model.end()))
					break;

				bResult = true;
				(*it)->pet = NULL;

				GetPVS().MakeBaseStructure(m_baseModel, true);

				GetPVS().MakeObjStructure((*it)->id, index, true);

				for (int i = 0; i < num; i++)
					GetPVS().MakeObjStructure(m_model[i]->id, i, true);

				for (it = m_model.begin(); it != m_model.end(); ++it)
				{
					if (stricmp((*it)->name, model->name) == 0)
					{
						if (!(it != m_model.end()))
							break;

						(*it)->tri.clear();
						if ((*it)->quad)
						{
							delete[] (*it)->quad;
							(*it)->quad = NULL;
						}

						m_model.erase(it);

						m_modelNum--;
						break;
					}
				}

				break;
			}
		}
	}

	return bResult;
}

const sProperty& WPolySoup::GetProperty(int index)
{
	return m_pDoc->m_propertyInfo.GetProperty(index);
}

int WPolySoup::GetPropertyIndex(const char* name)
{
	return m_pDoc->m_propertyInfo.GetPropertyIndex(name);
}

void WPolySoup::EndLoad(bool bPlaceHoleCup)
{
	ClearOldTextures();

	if (bPlaceHoleCup)
		PlaceHoleCup();

	xBuildTnLBuffers();
}

void WPolySoup::ChangeFloorTexture(const char* from, const char* to)
{
	if (!IsLocalContent(S4_REAL_MYROOM))
		return;

	if (from == NULL || to == NULL)
		return;

	int fromHandle = g_resrcmng->FindTexture(from)->texhandle;

	int toHandle = g_resrcmng->LoadTexture(to, 0, 0, 0);

	if (m_baseModel->pet->GetRootBone()->GetMesh())
		m_baseModel->pet->ChangeTextureByHandle(toHandle, fromHandle,
			m_baseModel->pet);
}

void WPolySoup::ChangeObjectTexture(const char* from, const char* to, int index)
{
	if (!IsLocalContent(S4_REAL_MYROOM))
		return;

	if (from == NULL || to == NULL)
		return;

	if (index == -1)
	{
		int fromHandle = g_resrcmng->FindTexture(from)->texhandle;

		int toHandle = g_resrcmng->LoadTexture(to, 0, 0, 0);

		w_mesh* mesh = m_model[0]->pet->GetRootBone()->GetMesh();
		if (mesh)
		{
			mesh = mesh->next;
			while (mesh)
			{
				if (mesh)
					break;
				mesh = mesh->next;
			}

			if (mesh)
				m_model[0]->pet->ChangeTextureByHandle(toHandle, fromHandle,
					m_model[0]->pet);
		}
	}
	else
	{
		for (int i = 0; i < m_modelNum; i++)
		{
			if (m_model[i]->id == index)
			{
				index = i;
				break;
			}
		}

		int fromHandle = g_resrcmng->FindTexture(from)->texhandle;

		int toHandle = g_resrcmng->LoadTexture(to, 0, 0, 0);

		w_mesh* mesh = m_model[index]->pet->GetRootBone()->GetMesh();
		if (mesh)
		{
			mesh = mesh->next;
			while (mesh)
			{
				if (mesh)
				{
					m_model[index]->pet->ChangeTextureByHandle(toHandle,
						fromHandle, m_model[index]->pet);
					return;
				}
				mesh = mesh->next;
			}
			m_model[index]->pet->ChangeTextureByHandle(toHandle, fromHandle,
				m_model[index]->pet);
			mesh = NULL;
		}
	}
}

void WPolySoup::xBuildTnLBuffers()
{
	if (m_bBuildTnL)
	{
		m_baseModel->pet->xUpdateTnLBuffers(true);
		m_staticPetGrp.Build(m_model);
	}
}
unsigned char WPolySoup::OverlapCamera(int index)
{
	if (m_model[index]->flag & 8)
		return 0xff;
	Waabb aabb = m_model[index]->aabb;

	// HACK: 300.0f * g_CM_TO_WU (96.0f); see rb_real_42c00000 above. The
	// original still emitted g_CM_TO_WU into .rdata.
	(void)g_CM_TO_WU;
	float radius =
		Min(WVectorLen(aabb.max - aabb.min) * 0.5f, rb_real_42c00000);
	float dist = 0.0f;

	for (int i = 0; i < 3; i++)
	{
		float d;
		if (g_camera.pivot.p[i] < aabb.min.p[i])
			d = g_camera.pivot.p[i] - aabb.min.p[i];
		else if (g_camera.pivot.p[i] > aabb.max.p[i])
			d = g_camera.pivot.p[i] - aabb.max.p[i];
		else
			continue;
		dist += d * d;
	}

	return (unsigned char)(Between(0.25f, sqrtf(dist) / radius, 1.0f) * 255.0f);
}

bool WPolySoup::IsPointInPoly(const std::vector<WVector>& poly,
	const WVector& pos)
{
	int num = poly.size();
	if (num < 3)
		return false;

	int sum = 0;
	for (int i = 0; i < num; i++)
	{
		WVector a = poly[i];
		WVector b = poly[(i + 1) % num];
		int qa = a.x > pos.x ? (a.z > pos.z ? 1 : 4) : (a.z > pos.z ? 2 : 3);
		int qb = b.x > pos.x ? (b.z > pos.z ? 1 : 4) : (b.z > pos.z ? 2 : 3);
		int d = qb - qa;

		switch (d)
		{
		case 2:
		case -2:
			if (b.x - (a.x - b.x) * (b.z - pos.z) / (a.z - b.z) > pos.x)
				d = -d;
			break;
		case 3:
			d = -1;
			break;
		case -3:
			d = 1;
			break;
		}

		sum += d;
	}

	if (Abs(sum) == 4)
		return true;
	return false;
}

bool WPolySoup::IsOutOfBound(const WVector& pos)
{
	std::vector<CNodeContainer>::const_iterator it;
	for (it = m_node.begin(); it != m_node.end(); it++)
	{
		CNodeContainer::eType type = (*it).GetType();

		if (type == CNodeContainer::TYPE_OB ||
			type == CNodeContainer::TYPE_OUTLINE)
		{
			if (!IsPointInPoly(it->GetList(), pos))
				return true;
		}
		else if (type == CNodeContainer::TYPE_IB)
		{
			if (IsPointInPoly(it->GetList(), pos))
				return true;
		}
	}

	if (GetPVS().IsOBArea(pos))
		return true;

	return false;
}

WVector WPolySoup::GetCameraPos(const WVector& from, const WVector& to)
{
	WVector pos = from;
	std::vector<CNodeContainer>::iterator it;

	for (it = m_node.begin(); it != m_node.end(); it++)
	{
		CNodeContainer::eType type = it->GetType();
		if ((HaveCameraArea() && type == CNodeContainer::TYPE_CAMERA) ||
			(!HaveCameraArea() && type == CNodeContainer::TYPE_OUTLINE))
		{
			if (!IsPointInPoly(it->GetList(), from))
				pos = PosInWorld(it->GetList(), from, to);
			break;
		}
	}

	return pos;
}

WVector WPolySoup::PosInWorld(const std::vector<WVector>& poly,
	const WVector& a, const WVector& b)
{
	WVector dir, e, f, c, g;
	int num = poly.size();
	dir = a - b;
	dir.y = 0.0f;
	float length = dir.Magnitude();

	if (IsClockWise(poly, b))
	{
		for (int i = 0; i < num; i++)
		{
			e = poly[i] - b;
			e.y = 0.0f;
			if (e * dir < 0.0f)
				continue;
			c = WCrossProduct(dir, e);

			f = poly[(i + 1) % num] - b;
			f.y = 0.0f;
			if (f * dir < 0.0f)
				continue;
			g = WCrossProduct(dir, f);

			if (c.y < 0.0f && g.y > 0.0f)
			{
				float r = GetCameraRadius(e, f, dir);
				if (length > r)
					length = r;
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < num; i++)
		{
			e = poly[i] - b;
			if (e * dir < 0.0f)
				continue;
			c = WCrossProduct(dir, e);

			f = poly[(i + 1) % num] - b;
			f.y = 0.0f;
			if (f * dir < 0.0f)
				continue;
			g = WCrossProduct(dir, f);

			if (c.y > 0.0f && g.y < 0.0f)
			{
				float r = GetCameraRadius(e, f, dir);
				if (length > r)
					length = r;
				break;
			}
		}
	}

	dir.Normalize();
	WVector pos = b + dir * length;
	return pos;
}

float WPolySoup::GetCameraRadius(WVector eye, WVector target, WVector up)
{
	eye.y = 0.0f;
	target.y = 0.0f;
	up.y = 0.0f;

	float lenA = eye.Magnitude();
	float lenB = target.Magnitude();

	eye.Normalize();
	target.Normalize();
	up.Normalize();
	LogOut(0, "GetCameraRadius : %f\n", up.x);

	float angle[2];
	angle[0] = (float)acos(Between(-1.0f, eye * up, 1.0f));
	angle[1] = (float)acos(Between(-1.0f, target * up, 1.0f));

	return (float)(sin(angle[0] + angle[1]) * lenB * lenA /
		(sin(angle[0]) * lenA + sin(angle[1]) * lenB));
}

bool WPolySoup::IsClockWise(const std::vector<WVector>& poly,
	const WVector& pos)
{
	WVector a = poly[1] - pos;
	WVector b = poly[0] - pos;
	a.x = a.x * b.z - b.x * a.z;
	if (a.x > 0.0f)
		return true;
	return false;
}

WPolySoup::sCamera* WPolySoup::FindCamera(const char* name)
{
	if (!name || !*name)
		return NULL;

	for (int i = 0; i < m_header.cameraNum; i++)
	{
		if (strcmpi(m_camera[i].name, name) == 0)
			return &m_camera[i];
	}

	return NULL;
}

WPolySoup::sPoint* WPolySoup::FindPoint(const char* name)
{
	if (!name || !*name)
		return NULL;

	for (int i = 0; i < m_header.pointNum; i++)
	{
		if (strcmpi(m_point[i].name, name) == 0)
			return &m_point[i];
	}

	return NULL;
}

WPolySoup::sArea* WPolySoup::FindArea(const char* name)
{
	if (!name || !*name)
		return NULL;

	for (int i = 0; i < m_header.areaNum; i++)
	{
		if (strstr(m_area[i].name, name))
			return &m_area[i];
	}

	return NULL;
}

WPolySoup::sModel* WPolySoup::FindModel(const char* name)
{
	if (!name || !*name)
		return m_baseModel;

	for (int i = 0; i < m_modelNum; i++)
	{
		if (strcmpi(m_model[i]->name, name) == 0)
			return m_model[i];
	}

	return NULL;
}

CNodeContainer* WPolySoup::FindNode(const char* name)
{
	if (!name || !*name)
		return NULL;

	for (std::vector<CNodeContainer>::iterator it = m_node.begin();
		it != m_node.end(); it++)
	{
		if (strcmpi(it->GetName().c_str(), name) == 0)
			return &(*it);
	}

	return NULL;
}

WPolySoup::sArea* WPolySoup::GetAreaByPos(const WVector& pos)
{
	for (int i = 0; i < m_header.areaNum; i++)
	{
		if (strstr(m_area[i].name, "*cam"))
		{
			if (m_area[i].aabb.IsInclude(pos))
				return &m_area[i];
		}
	}

	return NULL;
}

WPolySoup::sCamera* WPolySoup::GetCameraByName(const char* name)
{
	for (int i = 0; i < m_header.cameraNum; i++)
	{
		if (stricmp(m_camera[i].name, name) == 0)
			return &m_camera[i];
	}

	return NULL;
}

w_bound_box* WPolySoup::GetObjBoundBox(int index, const char* name)
{
	if (index == 0xffff)
		return m_baseModel->pet->m_bbList.Find(name);

	return m_model[index]->pet->m_bbList.Find(name);
}

int WPolySoup::GetObjAlpha(int index)
{
	if (index == 0xffff)
		return 0xff;

	return m_model[index]->alpha;
}

float WPolySoup::GetObjWindFactor(int index)
{
	if (index == 0xffff)
		return m_baseModel->windFactor;

	return m_model[index]->windFactor;
}

void WPolySoup::UpdateBgFxWind()
{
	CFx::Instance()->UpdateExtWind(Wind().GetGlobalWind());

	for (int i = 0; i < m_bgFxNum; i++)
	{
		if (m_bgFx[i].spray)
			m_bgFx[i].spray->SetWind(Wind().GetWind(m_bgFx[i].spray->Pos()));

		if (m_bgFx[i].sequence &&
			CFx::Instance()->FindSequence(m_bgFx[i].sequence))
			m_bgFx[i].sequence->SetWind(
				Wind().GetWind(m_bgFx[i].sequence->Pos()));
	}
}

void WPolySoup::SetAdditionModel(const char* name, const WVector& pos)
{
	if (!IsLocalContent(S4_REAL_MYROOM))
		return;

	if (!name)
		return;

	sModel* model = m_model[m_modelNum++];

	model->type = 0;
	model->flag = 0;

	model->mat = m_model[4]->mat;

	model->mat.xm = pos.x;
	model->mat.ym = pos.y;
	model->mat.zm = pos.z;

	Waabb& aabb = model->aabb;
	model->pos = (aabb.min + aabb.max) * 0.5f;
	model->center = pos * 0.5f;
	WVector size = aabb.max - aabb.min;
	float zz = size.z * size.z;
	model->radius = sqrtf(size.x * size.x + size.y * size.y + zz) * 0.5f;
	model->scale = m_model[4]->scale;
	strncpy(model->name, name, 31);
	strncpy(model->option, m_model[4]->option, 31);

	model->pet = g_resrcmng->GetPuppet(model->name, IsSeaPet(model->name),
		model->type != 0, false);

	w_bound_box* bb = model->pet->m_bbList.Find("char0");
	if (bb)
	{
		model->obb = bb->bone->m_matrix;
		model->obb *= (bb->info->aabb.max - bb->info->aabb.min) * 0.5f +
			WVector::ONE * 1.6f;
		model->obb.center = bb->info->spherePivot * bb->bone->m_matrix;
		g_view->DrawOBB(model->obb, 0xff00ff00);
	}

	if (!m_pDoc->m_modelNameList.Find(model->name))
	{
		char* copy = new char[strlen(model->name) + 1];
		strcpy(copy, model->name);
		m_pDoc->m_modelNameList.AddItem(copy, copy, false);
	}

	model->pet->SetMatrix(model->mat);
	model->pet->UpdateBound(true, false);
	model->pet->UpdateLightSource(&g_lightset, false, NULL);
	ProcessPostLoadPet(model);

	unsigned long flag = 0;
	if (model->type)
		flag |= 4;
	if ((model->flag & 0x10) || model->pet->m_boundSphere.radius > 128.0f)
		flag |= 2;
	if (model->flag & 8)
		flag |= 1;

	CSceneManager::Instance()->RegisterElement(model->type ? OBJ_MODEL
														   : OBJ_STATIC,
		model->pet, NULL, model->name, flag, &model->bbox);
}

void WPolySoup::SetRenderMode(int mode)
{
	for (int i = 0; i < m_modelNum; i++)
	{
		sModel* model = m_model[i];

		if (model->pet)
			model->pet->SetRenderMode(mode);
	}

	m_baseModel->pet->SetRenderMode(mode);
}

void WPolySoup::MakeStatistics()
{
	mkdir("statistics");
	FILE* fp = fopen(MakeStr("statistics/%s_%02d_stat.txt", GetCurMapName(),
						 GOLFDOC()->m_currentHole),
		"wt");
	if (!fp)
		return;

	int total = 0;
	int maxFace = 0;
	int minFace = 1000;
	int count[11];
	memset(count, 0, sizeof(count));

	for (int i = 0; i < m_modelNum; i++)
	{
		sModel* model = m_model[i];
		int faceNum = model->pet->m_faceNum;
		int index = faceNum / 100;
		if (index > 10)
			index = 10;
		count[index]++;

		total += model->pet->m_faceNum;
		maxFace = Max(model->pet->m_faceNum, maxFace);
		minFace = Min(faceNum, minFace);
	}

	fprintf(fp,
		"*[%s_%02d]\xc0\xc7 \xbf\xc0\xba\xea\xc1\xa7\xc6\xae \xb8\xde\xbd\xac \xba\xd0\xc6\xf7\xb5\xb5\n\n",
		GetCurMapName(), GOLFDOC()->m_currentHole);

	for (int i = 0; i < 11; i++)
	{
		if (i == 10)
			fprintf(fp, "%3d~    : ", 1000);
		else
			fprintf(fp, "%3d~%3d : ", i * 100, i * 100 + 100);

		for (int j = count[i]; j > 0; j--)
			fprintf(fp, "*");

		fprintf(fp, "(%d)\n", count[i]);
	}

	int average = total / GetModelNum();
	int sum = 0;
	for (int i = 0; i < m_modelNum; i++)
	{
		int faceNum = m_model[i]->pet->m_faceNum;
		sum += faceNum * faceNum;
	}
	int variance = (sum - m_modelNum * average * average) / (m_modelNum - 1);

	fprintf(fp, "\n\xc7\xd5\xb0\xe8   : %d(%d)\n",
		m_baseModel->pet->m_faceNum + total, m_modelNum);
	fprintf(fp, "\xc6\xf2\xb1\xd5\xb0\xaa : %d\n", average);
	fprintf(fp, "\xc3\xd6\xbc\xd2\xb0\xaa : %d\n", minFace);
	fprintf(fp, "\xc3\xd6\xb4\xeb\xb0\xaa : %d\n", maxFace);
	fprintf(fp, "\xc7\xa5\xc1\xd8\xc6\xed\xc2\xf7 : %.0f\n",
		sqrt((double)variance));
	fprintf(fp, "\xc5\xd8\xbd\xba\xc3\xc4\xb0\xb9\xbc\xf6 : %d\n",
		m_header.textureNum);

	fclose(fp);
}

static float Interpolate(float a, float b, float c, float u, float v)
{
	return (b - a) * u + (c - a) * v + a;
}

static void BarycentricXZ(WVector* a, WVector* p, WVector* b, WVector* c,
	float* u, float* v)
{
	WVector2D pp(p->x, p->z);
	WVector2D d[3];

	d[0].x = pp.x - a->x;
	d[0].y = pp.y - a->z;
	d[1].x = b->x - a->x;
	d[1].y = b->z - a->z;
	d[2].x = c->x - a->x;
	d[2].y = c->z - a->z;

	*u = (d[2].y * d[0].x - d[2].x * d[0].y) /
		(d[2].y * d[1].x - d[2].x * d[1].y);
	*v = (d[1].x * d[0].y - d[1].y * d[0].x) /
		(d[2].y * d[1].x - d[2].x * d[1].y);
}

static void Barycentric(float* u, float* v, const WVector* p, const WVector* a,
	const WVector* b, const WVector* c)
{
	WVector n;
	n = WCrossProduct(*b - *a, *c - *a);

	float ax = Abs(n.x);
	float ay = Abs(n.y);
	float az = Abs(n.z);
	WVector2D pp;
	WVector2D d[3];

	if (ax >= ay && ax >= az)
	{
		pp.x = p->z;
		pp.y = p->y;
		d[0].x = pp.x - a->z;
		d[0].y = pp.y - a->y;
		d[1].x = b->z - a->z;
		d[1].y = b->y - a->y;
		d[2].x = c->z - a->z;
		d[2].y = c->y - a->y;
	}
	else if (ay >= ax && ay >= az)
	{
		pp.x = p->x;
		pp.y = p->z;
		d[0].x = pp.x - a->x;
		d[0].y = pp.y - a->z;
		d[1].x = b->x - a->x;
		d[1].y = b->z - a->z;
		d[2].x = c->x - a->x;
		d[2].y = c->z - a->z;
	}
	else
	{
		pp.x = p->x;
		pp.y = p->y;
		d[0].x = pp.x - a->x;
		d[0].y = pp.y - a->y;
		d[1].x = b->x - a->x;
		d[1].y = b->y - a->y;
		d[2].x = c->x - a->x;
		d[2].y = c->y - a->y;
	}

	*u = (d[2].y * d[0].x - d[2].x * d[0].y) /
		(d[2].y * d[1].x - d[2].x * d[1].y);
	*v = (d[1].x * d[0].y - d[1].y * d[0].x) /
		(d[2].y * d[1].x - d[2].x * d[1].y);
}

// 983 009473d0: .sbin novo (versao >= 0xfffe0100) traz a lista de ossos por nome; o antigo e so o osso raiz.
// Osso que nao existe encerra a lista sem erro.
bool WPolySoup::LoadShadowmapInfo()
{
	char name[64] = "";
	const char* dot = strrchr(m_mapName, '.');
	if (dot == NULL || dot - m_mapName >= 58)
		return false;

	strncpy(name, m_mapName, dot - m_mapName);
	strcat(name, ".sbin");

	// ReloadBasePet recria o boneco: as malhas antigas ja foram liberadas com ele
	m_smapVec4988_983.clear();
	m_smapVec4998_983.clear();
	cFile* file = GetResrcManager()->GetCFile(name, -1);
	if (file == NULL)
		return false;

	unsigned long version;
	file->Read(&version, 4);
	if ((version & 0xffff0000) == 0xfffe0000)
		m_sbinVersion983 = version;

	bool ok = true;
	if ((m_sbinVersion983 & 0xffff) < 0x0100)
	{
		file->Seek(-4, 1);
		ok = LoadShadowmapBone983(file, m_baseModel->pet->GetRootBone());
	}
	else
	{
		unsigned int boneNum;
		file->Read(&boneNum, 4);
		for (unsigned int i = 0; ok && i < boneNum; i++)
		{
			char boneName[260] = "";
			int len;
			file->Read(&len, 4);
			file->Read(boneName, len);
			boneName[len] = 0;
			WBone* bone = m_baseModel->pet->FindBone(boneName);
			if (bone == NULL)
				break;
			ok = LoadShadowmapBone983(file, bone);
		}
	}
	CloseCFile(file);
	return ok;
}

// 983 00946cd0: corpo do LoadShadowmapInfo do 645, por osso; sem UV = sucesso
bool WPolySoup::LoadShadowmapBone983(cFile* file, WBone* bone)
{
	std::vector<w_mesh*> meshList;
	for (w_mesh* mesh = bone->GetMesh(); mesh; mesh = mesh->next)
		meshList.push_back(mesh);

	int meshNum;
	file->Read(&meshNum, 4);
	if (meshNum != (int)meshList.size())
		return false;
	if (meshNum == 0)
		return true;

	for (int i = 0; i < meshNum; i++)
	{
		int vtxNum, indexNum;
		file->Read(&vtxNum, 4);
		file->Read(&indexNum, 4);
		if (vtxNum != meshList[i]->vtxNum || indexNum != meshList[i]->indexNum)
			return false;
	}

	int numVtx = 0, numUV = 0;
	file->Read(&numVtx, 4);
	file->Read(&numUV, 4);
	if (numUV == 0)
		return true;

	WVector* vtxList = new WVector[numVtx];
	file->Read(vtxList, numVtx * sizeof(WVector));
	float (*uvList)[2] = new float[numUV][2];
	file->Read(uvList, numUV * sizeof(float[2]));

	SmCase smCase[2];
	smLoadSmapMeshes(smCase, file, bone, &meshList[0], vtxList, uvList);
	smLoadOutFrags(file, meshNum, &meshList[0], vtxList, smCase);
	delete[] vtxList;
	delete[] uvList;

	int maxVtx = 0;
	for (w_mesh* mesh = bone->GetMesh(); mesh; mesh = mesh->next)
	{
		if (maxVtx < mesh->vtxNum)
			maxVtx = mesh->vtxNum;
	}
	bone->AllocVTX(maxVtx);
	return true;
}

// 983 00945db0: malhas novas no fim da lista do osso; texturas/malhas vao para os vetores +0x4988/+0x4998
void WPolySoup::smLoadSmapMeshes(SmCase* smCase, cFile* file, WBone* bone, w_mesh** meshList,
	WVector* vtxList, float (*uvList)[2])
{
	w_mesh* last = bone->GetMesh();

	while (last && last->next)
		last = last->next;

	w_mesh* first;
	w_mesh* mesh;
	int size;

	file->Read(&size, 4);

	first = NULL;

	if (size > 0)
	{
		file->Read(&smCase[0].num, 4);

		if (smCase[0].num > 0)
			smCase[0].list = new SmInFragList[smCase[0].num];

		for (int i = 0; i < smCase[0].num; i++)
		{
			mesh = smAddSmapPack(&smCase[0].list[i], file, 0, i, bone, meshList,
				vtxList, uvList);

			last->next = mesh;
			last = mesh;

			if (i == 0)
				first = mesh;
		}
	}

	switch (GetDiffFlag())
	{
	case 0:
		file->Read(&size, 4);
		file->Seek(size, 1);
		file->Read(&size, 4);
		file->Seek(size, 1);
		break;

	case 2:
		file->Read(&size, 4);
		file->Seek(size, 1);
	case 1:
		file->Read(&size, 4);

		if (size > 0)
			file->Read(&smCase[1].num, 4);
		break;
	}

	if (smCase[1].num > 0)
		smCase[1].list = new SmInFragList[smCase[0].num];

	for (int i = 0; i < smCase[1].num; i++)
	{
		mesh = smAddSmapPack(&smCase[1].list[i], file, GetDiffFlag(), i,
			bone, meshList, vtxList, uvList);

		last->next = mesh;
		last = mesh;
	}

	if (GetDiffFlag() == 1)
	{
		file->Read(&size, 4);
		file->Seek(size, 1);
	}

	for (mesh = first; mesh; mesh = mesh->next)
	{
		m_smapVec4988_983.push_back(mesh->drawFlag & 0x7ff);
		m_smapVec4998_983.push_back(mesh);
	}
}

struct sDDSHeader
{
	unsigned long dwSize;
	unsigned long dwFlags;
	unsigned long dwHeight;
	unsigned long dwWidth;
	unsigned long dwLinearSize;
	unsigned long dwDepth;
	unsigned long dwMipMapCount;
	unsigned long dwReserved1[11];
	unsigned long dwPfSize;
	unsigned long dwPfFlags;
	unsigned long dwFourCC;
	unsigned long dwRGBBitCount;
	unsigned long dwRBitMask;
	unsigned long dwGBitMask;
	unsigned long dwBBitMask;
	unsigned long dwABitMask;
	unsigned long dwCaps;
	unsigned long dwCaps2;
	unsigned long dwCaps3;
	unsigned long dwCaps4;
	unsigned long dwReserved2;
};

// 983 (00943c97..00943cf1 no smAddSmapPack, 00945a0b..00945a69 no DigHolecup): a malha nova recebe uma paleta com o osso
// dela (tri.bones[0][0]) no boneco do cenario; sem isso o WxTnLBuffer le paleta nula (WBonePalette983::GetMatrix, F4 c89-2)
static void SetMeshPalette983(WPuppet* pet, WBone* bone, w_mesh* mesh)
{
	WBone::CalcMeshAABB(*bone, *mesh);
	WBonePalette983 palette;
	WTriBones983 tri;
	tri.bones[0][0] = bone;
	palette.AddTriangle(tri.bones);
	mesh->palette983 = pet->GetPalette983(palette);
	pet->BindPalettes983(0);
}

w_mesh* WPolySoup::smAddSmapPack(SmInFragList* fragList, cFile* file,
	int caseIndex, int listIndex, WBone* bone, w_mesh** meshList, WVector* vtxList,
	float (*uvList)[2])
{
	int width, height;
	file->Read(&width, 4);
	file->Read(&height, 4);

	sDDSHeader ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = 0x80000;
	ddsd.dwMipMapCount = 1;
	ddsd.dwPfSize = 0x20;
	ddsd.dwPfFlags = 4;
	ddsd.dwFourCC = '1TXD';
	ddsd.dwCaps = 0x1000;
	ddsd.dwWidth = width;
	ddsd.dwHeight = height;
	ddsd.dwLinearSize = width * height / 2;

	int size = ddsd.dwLinearSize + 0x80;
	unsigned char* buf = new unsigned char[size];
	memcpy(buf + 4, &ddsd, sizeof(ddsd));
	buf[0] = 'D';
	buf[1] = 'D';
	buf[2] = 'S';
	buf[3] = ' ';
	file->Read(buf + 0x80, ddsd.dwLinearSize);

	char name[32];
	sprintf(name, "%s-%d-%d.dds", m_baseModel->name, caseIndex, listIndex);

	int texHandle = g_resrcmng->LoadTexture_DDS(buf, size, name, 0, 0);
	delete[] buf;

	int fragNum;
	file->Read(&fragNum, 4);

	SmInFragment* frag = new SmInFragment[fragNum];
	fragList->num = fragNum;
	fragList->frag = frag;
	file->Read(frag, fragNum * sizeof(SmInFragment));

	w_mesh* mesh = new w_mesh; // 983: construtor zera

	mesh->originalTexHandle = mesh->texHandle = texHandle; // 983 0094368c: +0x84 e +0x38
	mesh->drawFlag = (texHandle & 0x7ff) | 0x21100000;
	mesh->diffuse = 0xffffff;
	mesh->alpha = 1.0f;

	mesh->indexNum = fragNum * 3;
	mesh->indexList = new unsigned short[mesh->indexNum];
	WVector* vtx = new WVector[mesh->indexNum];
	float (*uv)[2] = new float[mesh->indexNum][2];
	unsigned long* color = new unsigned long[mesh->indexNum];
	int vtxNum = 0;

	for (int i = 0; i < fragNum; i++)
	{
		for (int k = 0; k < 3; k++)
		{
			int code = frag[i].vtx[k];
			int uvIndex = frag[i].uv[k];
			int j = 0;
			w_mesh* src;

			if (code < 0x8000)
			{
				src = meshList[frag[i].mesh];
				unsigned short i0 = src->indexList[frag[i].tri * 3];
				unsigned short i1 = src->indexList[frag[i].tri * 3 + 1];
				int i2 = src->indexList[frag[i].tri * 3 + 2];
				float u, v;
				WVector* vec = src->vecList;
				Barycentric(&u, &v, &vtxList[code], &vec[i0], &vec[i1],
					&vec[i2]);

				unsigned char* c = (unsigned char*)src->vtxColorList;
				unsigned long c0 = (unsigned char)Interpolate(c[i0 * 4 + 3],
					c[i1 * 4 + 3], c[i2 * 4 + 3], u, v);
				unsigned long col = (c0 << 24) | 0xffffff;

				for (j = 0; j < vtxNum; j++)
				{
					if (WisEqual(vtx[j].x, vtxList[code].x, g_EPSILON) &&
						WisEqual(vtx[j].y, vtxList[code].y, g_EPSILON) &&
						WisEqual(vtx[j].z, vtxList[code].z, g_EPSILON) &&
						color[j] == col &&
						WisEqual(uv[j][0], uvList[uvIndex][0], g_EPSILON) &&
						WisEqual(uv[j][1], uvList[uvIndex][1], g_EPSILON))
						break;
				}

				if (j == vtxNum)
				{
					vtx[vtxNum] = vtxList[code];
					color[vtxNum] = col;
					memcpy(uv[vtxNum], uvList[uvIndex], sizeof(float) * 2);
					mesh->indexList[i * 3 + k] = vtxNum;
					vtxNum++;
				}
				else
				{
					mesh->indexList[i * 3 + k] = j;
				}
			}
			else
			{
				src = meshList[frag[i].mesh];
				code -= 0x8000;

				for (j = 0; j < vtxNum; j++)
				{
					if (WisEqual(vtx[j].x, src->vecList[code].x, g_EPSILON) &&
						WisEqual(vtx[j].y, src->vecList[code].y, g_EPSILON) &&
						WisEqual(vtx[j].z, src->vecList[code].z, g_EPSILON) &&
						color[j] == (src->vtxColorList[code] | 0xffffff) &&
						WisEqual(uv[j][0], uvList[uvIndex][0], g_EPSILON) &&
						WisEqual(uv[j][1], uvList[uvIndex][1], g_EPSILON))
						break;
				}

				if (j == vtxNum)
				{
					vtx[vtxNum] = src->vecList[code];
					memcpy(uv[vtxNum], uvList[uvIndex], sizeof(float) * 2);
					color[vtxNum] = src->vtxColorList[code] | 0xffffff;
					mesh->indexList[i * 3 + k] = vtxNum;
					vtxNum++;
				}
				else
				{
					mesh->indexList[i * 3 + k] = j;
				}
			}
		}
	}

	mesh->vtxNum = vtxNum;
	mesh->vecList = new WVector[vtxNum];
	memcpy(mesh->vecList, vtx, vtxNum * sizeof(WVector));
	mesh->vtxColorList = new unsigned long[vtxNum];
	memcpy(mesh->vtxColorList, color, vtxNum * sizeof(unsigned long));
	mesh->originalVtxColorList = new unsigned long[vtxNum];
	memcpy(mesh->originalVtxColorList, color, vtxNum * sizeof(unsigned long));
	mesh->NewUV983(1, vtxNum); // 983: UVs em camadas (w_mesh +0x50)
	if (vtxNum > 0)
		memcpy(mesh->UVData983(), uv, vtxNum * sizeof(float[2]));

	SetMeshPalette983(m_baseModel->pet, bone, mesh);

	delete[] vtx;
	delete[] uv;
	delete[] color;

	return mesh;
}

void WPolySoup::smLoadOutFrags(cFile* file, int num, w_mesh** meshList,
	WVector* vtxList, SmCase* smCase)
{
	int c, i, j, k, r;
	SmInFragList* list;

	int extra = 0;
	for (c = 0; c < 2; c++)
	{
		if (smCase[c].list)
		{
			for (i = 0; i < smCase[c].num; i++)
			{
				list = smCase[c].list;
				if (list)
				{
					for (j = 0; j < list[i].num; j++)
					{
						if (list[i].frag[j].tri != 0xffff)
							extra++;
					}
				}
			}
		}
	}

	int outNum;
	file->Read(&outNum, 4);

	int detailNum = 0;
	int skipAfter = 0;
	int skipBefore = 0;

	switch (GetDiffFlag())
	{
	case 0:
		file->Read(&skipBefore, 4);
		file->Read(&skipAfter, 4);
		break;

	case 1:
		file->Read(&detailNum, 4);
		file->Read(&skipAfter, 4);
		break;

	case 2:
		file->Read(&skipBefore, 4);
		file->Read(&detailNum, 4);
		break;
	}

	int* count = new int[num * 4];
	memset(count, 0, num * 4 * sizeof(int));

	SmOutFrag* frag = new SmOutFrag[detailNum + outNum + extra];

	if (outNum > 0)
		file->Read(&count[num], num * sizeof(int));
	if (skipBefore > 0)
		file->Seek(num * sizeof(int), 1);
	if (detailNum > 0)
		file->Read(&count[num * 2], num * sizeof(int));
	if (skipAfter > 0)
		file->Seek(num * sizeof(int), 1);

	for (c = 0; c < 2; c++)
	{
		if (smCase[c].list)
		{
			for (j = 0; j < smCase[c].num; j++)
			{
				list = smCase[c].list;
				if (list)
				{
					for (k = 0; k < list[j].num; k++)
					{
						SmInFragment* in = &list[j].frag[k];
						if (in->tri != 0xffff)
							count[num * 3 + in->mesh]++;
					}
				}
			}
		}
	}

	count[0] = 0;
	for (i = 1; i < num; i++)
		count[i] = count[i - 1] + count[num * 3 + i - 1] + count[num + i - 1] +
			count[num * 2 + i - 1];

	memset(&count[num * 3], 0, num * sizeof(int));

	for (i = 0; i < num; i++)
	{
		if (count[num + i] > 0)
			file->Read(&frag[count[i]], count[num + i] * sizeof(SmOutFrag));
	}

	if (skipBefore)
		file->Seek(skipBefore * sizeof(SmOutFrag), 1);

	if (detailNum > 0)
	{
		for (i = 0; i < num; i++)
		{
			if (count[num * 2 + i] > 0)
				file->Read(&frag[count[i] + count[num + i]],
					count[num * 2 + i] * sizeof(SmOutFrag));
		}
	}

	for (c = 0; c < 2; c++)
	{
		if (smCase[c].list)
		{
			for (j = 0; j < smCase[c].num; j++)
			{
				list = smCase[c].list;
				if (list)
				{
					for (k = 0; k < list[j].num; k++)
					{
						SmInFragment* in = &list[j].frag[k];
						if (in->tri != 0xffff)
						{
							int mesh = in->mesh;
							int* slot = &count[num * 3 + mesh];
							SmOutFrag* out = &frag[count[num * 2 + mesh] +
								count[num + mesh] + count[mesh] + *slot];
							out->tri = in->tri;
							memcpy(out->vtx, in->vtx, sizeof(out->vtx));
							(*slot)++;
						}
					}
				}
			}
		}
	}

	if (skipAfter)
		file->Seek(skipAfter * sizeof(SmOutFrag), 1);

	for (i = 0; i < num; i++)
	{
		if (count[num * 2 + i] > 0)
			file->Read(&frag[count[i] + count[num + i]],
				count[num * 2 + i] * sizeof(SmOutFrag));

		count[num + i] += count[num * 3 + i] + count[num * 2 + i];
	}

	for (i = 0; i < num; i++)
	{
		if (count[num + i] > 0)
		{
			int unique = 0;

			for (j = 0; j < count[num + i] - 1; j++)
			{
				for (k = j + 1; k < count[num + i]; k++)
				{
					for (r = 0; r < 3; r++)
					{
						if (frag[count[i] + j].vtx[0] ==
								frag[count[i] + k].vtx[r] &&
							frag[count[i] + j].vtx[1] ==
								frag[count[i] + k].vtx[(r + 1) % 3] &&
							frag[count[i] + j].vtx[2] ==
								frag[count[i] + k].vtx[(r + 2) % 3])
							break;
					}

					if (r < 3)
						break;
				}

				if (k == count[num + i])
					unique++;
			}

			if (unique + 1 != count[num + i])
			{
				SmOutFrag* packed = new SmOutFrag[unique + 1];

				for (unique = 0, j = 0; j < count[num + i] - 1; j++)
				{
					for (k = j + 1; k < count[num + i]; k++)
					{
						for (r = 0; r < 3; r++)
						{
							if (frag[count[i] + j].vtx[0] ==
									frag[count[i] + k].vtx[r] &&
								frag[count[i] + j].vtx[1] ==
									frag[count[i] + k].vtx[(r + 1) % 3] &&
								frag[count[i] + j].vtx[2] ==
									frag[count[i] + k].vtx[(r + 2) % 3])
								break;
						}

						if (r < 3)
							break;
					}

					if (k == count[num + i])
						memcpy(&packed[unique++], &frag[count[i] + j],
							sizeof(SmOutFrag));
				}

				memcpy(&packed[unique++], &frag[count[i] + j],
					sizeof(SmOutFrag));

				qsort(packed, unique, sizeof(SmOutFrag), smCompareOutFrag);
				smBuildOutFragMeshes(meshList[i], packed, unique, vtxList);

				delete[] packed;
			}
			else
			{
				qsort(&frag[count[i]], count[num + i], sizeof(SmOutFrag),
					smCompareOutFrag);
				smBuildOutFragMeshes(meshList[i], &frag[count[i]],
					count[num + i], vtxList);
			}
		}
	}

	delete[] count;
	delete[] frag;
}

int WPolySoup::smCompareOutFrag(const void* a, const void* b)
{
	const SmOutFrag* fa = (const SmOutFrag*)a;
	const SmOutFrag* fb = (const SmOutFrag*)b;

	if (fa->tri < fb->tri)
		return -1;
	return fa->tri > fb->tri;
}

void WPolySoup::smBuildOutFragMeshes(w_mesh* mesh, SmOutFrag* frag, int num,
	WVector* vtxList)
{
	if (num == 0)
		return;

	unsigned short* indexList = mesh->indexList;
	WVector* vecList = mesh->vecList;
	WVector* normList = mesh->normList;
	unsigned long* colorList = mesh->vtxColorList;

	int i, j, k;
	int triNum = 1;
	for (i = 1; i < num; i++)
	{
		if (frag[i].tri != frag[i - 1].tri)
			triNum++;
	}

	int keep = mesh->indexNum - triNum * 3;
	int indexNum = keep + num * 3;
	unsigned short* newIndex = new unsigned short[indexNum];
	WVector* vtx = new WVector[num * 3];
	WVector* normal = new WVector[num * 3];
	// 983 (00943d70): um buffer de UV por camada da malha (645: so a camada 0)
	w_uvset983 uv(mesh->uv ? mesh->uv->size() : 0, w_uvlayer983(num * 3));
	w_uvlayer983 vtxUv(uv.size()); // uma entrada por camada, do vertice atual
	unsigned long* color = new unsigned long[num * 3];

	int vtxNum = 0;
	for (i = 0; i < num; i++)
	{
		int tri = frag[i].tri * 3;
		int i0 = indexList[tri];
		int i1 = indexList[tri + 1];
		int i2 = indexList[tri + 2];

		for (k = 0; k < 3; k++)
		{
			int code = frag[i].vtx[k];

			if (code < 0x8000)
			{
				float u, v;
				Barycentric(&u, &v, &vtxList[code], &vecList[i0], &vecList[i1],
					&vecList[i2]);

				WVector n;
				n.x = Interpolate(normList[i0].x, normList[i1].x,
					normList[i2].x, u, v);
				n.y = Interpolate(normList[i0].y, normList[i1].y,
					normList[i2].y, u, v);
				n.z = Interpolate(normList[i0].z, normList[i1].z,
					normList[i2].z, u, v);

				unsigned char* c0 = (unsigned char*)&colorList[i0];
				unsigned char* c1 = (unsigned char*)&colorList[i1];
				unsigned char* c2 = (unsigned char*)&colorList[i2];
				float a0 = c0[3];
				float r0 = c0[2];
				float g0 = c0[1];
				float b0 = (float)(colorList[i0] & 0xff);
				float a2 = c2[3];
				float g2 = c2[1];
				unsigned char a =
					(unsigned char)Interpolate(a0, c1[3], a2, u, v);
				unsigned char r = (unsigned char)Interpolate(r0,
					(float)((colorList[i1] >> 16) & 0xff),
					(float)((colorList[i2] >> 16) & 0xff), u, v);
				unsigned char g =
					(unsigned char)Interpolate(g0, c1[1], g2, u, v);
				unsigned char b = (unsigned char)Interpolate(b0,
					(float)(colorList[i1] & 0xff),
					(float)(colorList[i2] & 0xff), u, v);
				unsigned long col = (a << 24) | (r << 16) | (g << 8) | b;

				size_t l;
				for (l = 0; l < uv.size(); ++l)
				{
					const w_uvlayer983& src = (*mesh->uv)[l];
					vtxUv[l].u = Interpolate(src[i0].u, src[i1].u, src[i2].u, u, v);
					vtxUv[l].v = Interpolate(src[i0].v, src[i1].v, src[i2].v, u, v);
				}

				// 983 0094448c: posicao e normal com epsilon; UV de TODAS as camadas exato (645: so a camada 0, com epsilon)
				for (j = 0; j < vtxNum; j++)
				{
					if (!WisEqual(vtx[j].x, vtxList[code].x, g_EPSILON) ||
						!WisEqual(vtx[j].y, vtxList[code].y, g_EPSILON) ||
						!WisEqual(vtx[j].z, vtxList[code].z, g_EPSILON) ||
						!WisEqual(normal[j].x, n.x, g_EPSILON) ||
						!WisEqual(normal[j].y, n.y, g_EPSILON) ||
						!WisEqual(normal[j].z, n.z, g_EPSILON))
						continue;
					for (l = 0; l < uv.size(); ++l)
						if (uv[l][j].u != vtxUv[l].u || uv[l][j].v != vtxUv[l].v)
							break;
					if (l == uv.size())
						break;
				}

				if (j == vtxNum)
				{
					vtx[vtxNum] = vtxList[code];
					normal[vtxNum] = n;
					for (l = 0; l < uv.size(); ++l)
						uv[l][vtxNum] = vtxUv[l];
					color[vtxNum] = col;
					newIndex[keep + i * 3 + k] = mesh->vtxNum + vtxNum;
					vtxNum++;
				}
				else
				{
					newIndex[keep + i * 3 + k] = mesh->vtxNum + j;
				}
			}
			else
			{
				newIndex[keep + i * 3 + k] = code + 0x8000;
			}
		}
	}

	keep = 0;
	if (frag[0].tri > 0)
	{
		memcpy(newIndex, indexList, frag[0].tri * 3 * sizeof(unsigned short));
		keep = frag[0].tri * 3;
	}

	for (i = 1; i < num; i++)
	{
		int prev = frag[i - 1].tri * 3;
		int cur = frag[i].tri * 3;

		if (cur - prev > 3)
		{
			memcpy(&newIndex[keep], &indexList[prev + 3],
				(cur - prev - 3) * sizeof(unsigned short));
			keep += cur - prev - 3;
		}
	}

	int prev = frag[i - 1].tri * 3;
	int rest = mesh->indexNum - prev;

	if (rest > 3)
		memcpy(&newIndex[keep], &indexList[prev + 3],
			(rest - 3) * sizeof(unsigned short));

	WVector* newVec = new WVector[mesh->vtxNum + vtxNum];
	memcpy(newVec, mesh->vecList, mesh->vtxNum * sizeof(WVector));
	memcpy(&newVec[mesh->vtxNum], vtx, vtxNum * sizeof(WVector));

	WVector* newNorm = new WVector[mesh->vtxNum + vtxNum];
	memcpy(newNorm, mesh->normList, mesh->vtxNum * sizeof(WVector));
	memcpy(&newNorm[mesh->vtxNum], normal, vtxNum * sizeof(WVector));

	unsigned long* newColor = new unsigned long[mesh->vtxNum + vtxNum];
	memcpy(newColor, mesh->vtxColorList, mesh->vtxNum * sizeof(unsigned long));
	memcpy(&newColor[mesh->vtxNum], color, vtxNum * sizeof(unsigned long));

	unsigned long* newOrigColor = new unsigned long[mesh->vtxNum + vtxNum];
	memcpy(newOrigColor, newColor,
		(mesh->vtxNum + vtxNum) * sizeof(unsigned long));

	// 983: UVs em camadas (w_mesh +0x50): cresce todas
	for (size_t l = 0; l < uv.size(); ++l)
	{
		w_uvlayer983& layer = (*mesh->uv)[l];
		layer.resize(mesh->vtxNum + vtxNum);
		std::copy(uv[l].begin(), uv[l].begin() + vtxNum, layer.begin() + mesh->vtxNum);
	}

	delete[] vtx;
	delete[] normal;
	delete[] color;

	delete[] mesh->vecList;
	delete[] mesh->normList;
	delete[] mesh->vtxColorList;
	delete[] mesh->originalVtxColorList;
	delete[] mesh->indexList;

	mesh->vtxNum += vtxNum;
	mesh->indexNum = indexNum;
	mesh->vecList = newVec;
	mesh->normList = newNorm;
	mesh->vtxColorList = newColor;
	mesh->originalVtxColorList = newOrigColor;
	mesh->indexList = newIndex;
}

void WPolySoup::EnableShadowmap(bool bEnable)
{
	if (bEnable)
	{
		if (!m_smapVec4998_983.empty())
		{
			for (size_t i = 0; i < m_smapVec4998_983.size(); i++)
			{
				if ((m_smapVec4998_983[i]->drawFlag & 0x21100000) == 0x21100000)
					m_smapVec4998_983[i]->alpha = 1.0f;
			}
		}
		else if (!m_bSmapLoadFailed)
			ReloadBasePet();
	}
	else
	{
		for (size_t i = 0; i < m_smapVec4998_983.size(); i++)
		{
			if ((m_smapVec4998_983[i]->drawFlag & 0x21100000) == 0x21100000)
				m_smapVec4998_983[i]->alpha = 0.0f;
		}
	}
}

// 983: 0094ac10 -- sem reler o arquivo (o 645 relia as cores da 0x71 com Seek): matriz dos ossos de ligacao pelos
// sAttach983 e cores pela tabela +0x49b0.
void WPolySoup::ReloadBasePet()
{
	if (g_resrcmng == NULL)
		return;

	if (m_baseModel->pet)
	{
		g_resrcmng->Release(m_baseModel->pet, true);
		m_baseModel->pet = NULL;
	}

	m_baseModel->pet =
		g_resrcmng->GetPuppet(m_baseModel->name, false, false, false);

	m_baseModel->pet->m_lightmode = 0x1000000;
	m_baseModel->pet->SetMatrix(m_baseModel->mat);
	m_baseModel->pet->Transform(NULL, 1.0f, 0);

	for (int i = 0; i < m_header.attachNum983; i++)
	{
		sAttach983* attach = &m_attach983[i];
		m_baseModel->pet->FindBone(attach->name)->SetMatrix(attach->mat, NULL);
	}

	m_baseModel->pet->UpdateBound(false, false);
	m_vtxColors983.Apply(m_baseModel->pet);

	m_bSmapLoadFailed = !LoadShadowmapInfo();

	if (strstr(m_mapName, "rank") == NULL)
		PlaceHoleCup();

	m_baseModel->pet->xUpdateTnLBuffers(true);

	WSingleton<CSceneManager>::Instance()->SetPuppet(m_baseModel->name,
		m_baseModel->pet);
}
// 983: o raio do copo = Scale983 (0046f540, jp/aiplayer.cpp): mapa = CSharedDoc gameType, canal = +0x10a99
float __fastcall Scale983(int map, unsigned long channelType, float value);
#define HOLECUP_RADIUS983() \
	Scale983(Doc()->m_golfGame.gameType, \
		Doc()->m_curChannel.Type, GolfBall().m_radius)

void WPolySoup::PlaceHoleCup()
{
	int i;
	for (i = 0; i < m_header.pointNum; i++)
	{
		if (strstr(m_point[i].name, "\xb3\xa1\xc1\xa1"))
			break;
	}

	if (i == m_header.pointNum)
		return;

	sHoleCupContext983 ctx;
	ctx.soup = this;
	ctx.done = false;
	WPuppet::ForEachBone983(m_baseModel->pet, HoleCupCallback983, &ctx);
}

// 983: 0094a680 -- corta o copo no primeiro osso com triangulos sob o pino; o percurso 00b98290 para quando
// devolve false, aqui ctx.done (BoneCallback983 nosso e void). O 645 so olhava o osso raiz.
void WPolySoup::HoleCupCallback983(WPuppet* pet, WBone* bone, void* param)
{
	sHoleCupContext983* ctx = (sHoleCupContext983*)param;
	if (ctx->done)
		return;

	std::vector<sCupVtx> cup;
	std::vector<sHoleTri> tris;
	std::vector<sHoleMesh> meshes;

	WMatrix inv = ~bone->GetMatrix();
	WVector pin = GOLFDOC()->GetHoleData().pin;
	WVector center = pin * inv;

	// 983 0094a7c8: raio no espaco do osso (divide pela escala do eixo x)
	const WVector& xa = bone->GetMatrix().xa;
	float radius = HOLECUP_RADIUS983() / sqrtf(xa * xa);

	sCupVtx vtx;
	int i;
	for (i = 0; i < 33; i++)
	{
		float angle = i * (g_PI / 16.0f);
		vtx.pos.x = (float)cos(angle) * radius + center.x;
		vtx.pos.z = center.z - (float)sin(angle) * radius;
		cup.push_back(vtx);
	}

	ctx->soup->GatherHolecupTris(bone, tris, cup);

	if (tris.size() == 0)
		return;

	ctx->soup->CreateNewHolecupMeshes(meshes, cup, tris);
	ctx->soup->AddHolecup(bone, meshes, cup, tris);

	w_mesh* mesh = bone->GetMesh();
	int maxVtx = 0;
	for (; mesh; mesh = mesh->next)
	{
		if (maxVtx < mesh->vtxNum)
			maxVtx = mesh->vtxNum;
	}

	bone->AllocVTX(maxVtx);
	ctx->done = true;
}

void WPolySoup::GatherHolecupTris(WBone* bone, std::vector<sHoleTri>& tris,
	std::vector<sCupVtx>& cup)
{
	std::vector<int> holeTex;
	int i, j, k;
	sHoleTri hole;

	for (i = 0; i < m_header.textureNum; i++)
	{
		const sProperty& prop = GetProperty(m_texture[i].propIndex);

		if (prop.ground == 2)
			holeTex.push_back(m_texture[i].texHandle);
	}

	for (int pass = 0; pass < 2; pass++)
	{
		for (w_mesh* mesh = bone->GetMesh(); mesh; mesh = mesh->next)
		{
			if (pass == 0)
			{
				if ((mesh->drawFlag & 0x300000) != 0x100000)
				{
					if (std::find(holeTex.begin(), holeTex.end(),
							mesh->drawFlag & 0x7ff) == holeTex.end())
						continue;
				}
			}
			else
			{
				if ((mesh->drawFlag & 0x300000) == 0x100000)
					continue;

				if (std::find(holeTex.begin(), holeTex.end(),
						mesh->drawFlag & 0x7ff) != holeTex.end())
					continue;
			}

			for (i = 0; i < mesh->indexNum; i += 3)
			{
				hole.pos[0] = &mesh->vecList[mesh->indexList[i]];
				hole.pos[1] = &mesh->vecList[mesh->indexList[i + 1]];
				hole.pos[2] = &mesh->vecList[mesh->indexList[i + 2]];

				WVector normal = WCrossProduct(*hole.pos[1] - *hole.pos[0],
					*hole.pos[2] - *hole.pos[0])
									 .Normalize();

				if (normal.y < 0.001f)
					continue;

				for (j = 0; j < (int)cup.size() - 1; j++)
				{
					if (IsInTri(cup[j].pos, hole.pos))
					{
						GenHolecupTri(&hole, mesh, i);
						tris.push_back(hole);
						break;
					}
				}

				if (j < (int)cup.size() - 1)
					continue;

				for (k = 0; k < 3; k++)
				{
					if (IsInPoly(*hole.pos[k], cup))
					{
						GenHolecupTri(&hole, mesh, i);
						tris.push_back(hole);
						break;
					}
				}

				if (k < 3)
					continue;

				for (j = 0; j < (int)cup.size() - 1; j++)
				{
					sCupVtx* v = &cup[j];

					for (k = 0; k < 3; k++)
					{
						if (AreCrossed(&v[0].pos, &v[1].pos, hole.pos[k],
								hole.pos[(k + 1) % 3]))
						{
							GenHolecupTri(&hole, mesh, i);
							tris.push_back(hole);
							break;
						}
					}

					if (k < 3)
						break;
				}
			}
		}

		for (j = 0; j < (int)tris.size(); j++)
		{
			if ((tris[j].mesh->drawFlag & 0x300000) != 0x100000)
				goto found;
		}
	}

found:
	for (i = 0; i < (int)cup.size(); i++)
	{
		sCupVtx& cv = cup[i];

		for (j = 0; j < (int)tris.size(); j++)
		{
			sHoleTri& tv = tris[j];

			if (IsInTri(cv.pos, tv.pos))
			{
				cv.mesh = tv.mesh;
				cup[i].mesh = tris[j].mesh;
				cup[i].index = tris[j].offset;
				break;
			}
		}
	}
}

bool WPolySoup::IsInTri(WVector& pos, WVector** const tri)
{
	WVector2D v[2], p;

	v[0].x = tri[1]->x - tri[0]->x;
	v[0].y = tri[1]->z - tri[0]->z;
	v[0] /= v[0].Magnitude();
	v[1].x = tri[2]->x - tri[0]->x;
	v[1].y = tri[2]->z - tri[0]->z;
	v[1] /= v[1].Magnitude();
	p.x = pos.x;
	p.y = pos.z;

	float s = v[0].x * v[1].y - v[0].y * v[1].x;

	if ((s > 0.0f ? s : -s) < 0.001f)
		return false;

	for (int i = 0; i < 3; i++)
	{
		v[0].x = tri[(i + 1) % 3]->x - tri[i]->x;
		v[0].y = tri[(i + 1) % 3]->z - tri[i]->z;
		v[0] /= v[0].Magnitude();
		v[1].x = p.x - tri[i]->x;
		v[1].y = p.y - tri[i]->z;
		v[1] /= v[1].Magnitude();

		float c = v[0].x * v[1].y - v[0].y * v[1].x;

		if (c * s < 0.0f)
			return false;
	}

	return true;
}

bool WPolySoup::IsInPoly(WVector& pos, std::vector<sCupVtx>& cup)
{
	if (cup.size() < 4)
		return false;

	int n = cup.size();
	WVector2D v[3];
	for (int k = 0; k < 3; k++)
	{
		v[k].x = cup[k].pos.x;
		v[k].y = cup[k].pos.z;
	}
	WVector2D p(pos.x, pos.z);
	WVector2D e[2];
	e[0] = v[1] - v[0];
	float len = e[0].Magnitude();
	e[0].x /= len;
	e[0].y /= len;
	e[1] = v[2] - v[0];
	e[1] /= e[1].Magnitude();
	float s = e[0].x * e[1].y - e[0].y * e[1].x;

	for (int i = 0; i < (int)cup.size() - 1; i++)
	{
		WVector2D c0(cup[i].pos.x, cup[i].pos.z);
		WVector2D c1(cup[i + 1].pos.x, cup[i + 1].pos.z);
		e[0] = c1 - c0;
		len = e[0].Magnitude();
		e[0].x /= len;
		e[0].y /= len;
		e[1] = p - c0;
		e[1] /= e[1].Magnitude();

		if ((e[0].x * e[1].y - e[0].y * e[1].x) * s <= 0.0f)
			return false;
	}

	return true;
}

bool WPolySoup::AreCrossed(WVector* a1, WVector* a2, WVector* b1, WVector* b2)
{
	WVector2D n, m;

	n.x = a2->x - a1->x;
	n.y = a2->z - a1->z;
	n /= n.Magnitude();
	m.x = b1->x - a1->x;
	m.y = b1->z - a1->z;
	m /= m.Magnitude();

	float s[2];
	s[0] = n.x * m.y - n.y * m.x;

	m.x = b2->x - a1->x;
	m.y = b2->z - a1->z;
	m /= m.Magnitude();

	s[1] = n.x * m.y - n.y * m.x;

	if (s[0] * s[1] > 0.0f)
		return false;

	n.x = b2->x - b1->x;
	n.y = b2->z - b1->z;
	n /= n.Magnitude();
	m.x = a1->x - b1->x;
	m.y = a1->z - b1->z;
	m /= m.Magnitude();

	s[0] = n.x * m.y - n.y * m.x;

	m.x = a2->x - b1->x;
	m.y = a2->z - b1->z;
	m /= m.Magnitude();

	s[1] = n.x * m.y - n.y * m.x;

	if (s[0] * s[1] > 0.0f)
		return false;

	return true;
}

void WPolySoup::GenHolecupTri(sHoleTri* tri, w_mesh* mesh, int offset)
{
	tri->mesh = mesh;
	tri->offset = offset;

	for (int i = 0; i < 3; i++)
	{
		tri->index[i] = mesh->indexList[offset + i];
		tri->normal[i] = &mesh->normList[tri->index[i]];
		tri->color[i] = mesh->vtxColorList[tri->index[i]];
		tri->uv[i].resize(mesh->uv->size());
		for (size_t k = 0; k < tri->uv[i].size(); ++k)
			tri->uv[i][k] = (*mesh->uv)[k][tri->index[i]];

		WVector* p = tri->pos[i];
		tri->edge[i].z = 0.0f;
		tri->edge[i].x = p->z - tri->pos[(i + 1) % 3]->z;
		tri->edge[i].y = tri->pos[(i + 1) % 3]->x - p->x;
		tri->edge[i].Normalize();
		tri->edge[i].z = -1.0f *
			(tri->pos[i]->x * tri->edge[i].x + tri->pos[i]->z * tri->edge[i].y);
	}
}

bool WPolySoup::GetIntersection(WVector* out, WVector* p0, WVector* p1,
	WVector* e0, WVector* e1, WVector* line)
{
	float d[2];

	d[0] = p0->x * line->x + p0->z * line->y + line->z;

	if ((int)(Wabs(d[0]) < 0.001f))
		return false;

	d[1] = p1->x * line->x + p1->z * line->y + line->z;

	if (Wabs(d[1]) < 0.001f)
		return false;

	if (d[1] * d[0] > 0.0f)
		return false;

	out->x = (p1->x - p0->x) * d[0] / (d[0] - d[1]) + p0->x;
	out->z = (p1->z - p0->z) * d[0] / (d[0] - d[1]) + p0->z;

	float ax = Wabs(e1->x - e0->x);
	float az = Wabs(e1->z - e0->z);

	bool in;
	if (ax > az)
		in = (out->x - e0->x) * (e1->x - out->x) > 0.0f;
	else
		in = (out->z - e0->z) * (e1->z - out->z) > 0.0f;

	return in;
}

void WPolySoup::CreateNewHolecupMeshes(std::vector<sHoleMesh>& meshes,
	std::vector<sCupVtx>& cup, std::vector<sHoleTri>& tris)
{
	std::vector<WVector> poly;
	std::vector<sHoleTri>::iterator it = tris.begin();

	while (it != tris.end())
	{
		if (ClipHolecup(poly, *it, cup) >= 3)
		{
			BuildNewHolecupMesh(meshes, *it, poly);
			it++;
		}
		else
		{
			it = tris.erase(it);
		}
	}
}

int WPolySoup::ClipHolecup(std::vector<WVector>& poly, sHoleTri& tri,
	std::vector<sCupVtx>& cup)
{
	std::vector<WVector> work;
	std::vector<WVector>* in;
	std::vector<WVector>* out;
	int i, k;
	int side[2];
	float d;

	poly.clear();
	int n = cup.size();
	WVector* v = NULL;

	for (k = 0; k < n; k++)
		work.push_back(cup[k].pos);

	in = &work;
	out = &poly;

	for (k = 0; k < 3; k++)
	{
		v = &(*in)[0];
		d = v->x * tri.edge[k].x + v->z * tri.edge[k].y + tri.edge[k].z;
		side[0] = d > 0.001f ? 1 : (d < -0.001f ? -1 : 0);

		if (side[0] <= 0)
			out->push_back(*v);

		for (i = 1; i < n; i++)
		{
			d = (*in)[i].x * tri.edge[k].x + (*in)[i].z * tri.edge[k].y +
				tri.edge[k].z;
			side[1] = d > 0.001f ? 1 : (d < -0.001f ? -1 : 0);

			if (side[0] == 0)
			{
				if (side[1] < 0)
					out->push_back((*in)[i]);
			}
			else if (side[1] == 0)
			{
				out->push_back((*in)[i]);
			}
			else
			{
				if (side[1] * side[0] < 0)
				{
					WVector cross;
					GetIntersection(&cross, &(*in)[i - 1], &(*in)[i],
						tri.pos[k], tri.pos[(k + 1) % 3], &tri.edge[k]);
					out->push_back(cross);
				}

				if (side[1] < 0)
					out->push_back((*in)[i]);
			}

			side[0] = side[1];
		}

		if (out->size() < 3)
			return out->size();

		if (!WisEqual(*out->begin(), *out->rbegin(), g_EPSILON))
			out->push_back(*out->begin());

		std::vector<WVector>* temp = in;
		in = out;
		out = temp;

		if (k < 2)
			out->clear();

		n = in->size();
	}

	return poly.size();
}

void WPolySoup::BuildNewHolecupMesh(std::vector<sHoleMesh>& meshes,
	sHoleTri& tri, std::vector<WVector>& poly)
{
	std::vector<WVector> lines;
	int n = poly.size();
	lines.reserve(n);

	for (int i = 0; i < n; i++)
	{
		int j = (i + 1 < n ? i + 1 : i + 2) % n;

		WVector& a = poly[i];
		WVector line(a.z - poly[j].z, poly[j].x - a.x, 0.0f);
		line.Normalize();
		float dx = line.x * a.x;
		float dz = line.y * a.z;
		line.z = (dx + dz) * -1.0f;
		lines.push_back(line);
	}

	bool done[3];
	int first = -1;
	bool open = false;
	int prevCount = 0;
	memset(done, 0, sizeof(done));
	int prev[3];

	for (int i = 0; i < n; i++)
	{
		int cur[3];
		int count = 0;

		for (int k = 0; k < 3; k++)
		{
			float d = tri.pos[k]->x * lines[i].x + tri.pos[k]->z * lines[i].y +
				lines[i].z;
			if (d > 0.001f)
				cur[count++] = k;
		}

		if (count == 2)
		{
			if (cur[0] == 0 && cur[1] == 2)
			{
				cur[1] = 0;
				cur[0] = 2;
			}

			if (i == 0)
				first = cur[0];
		}

		switch (count)
		{
		case 0:
			open = true;
			break;
		case 1:
			if (prevCount == 1)
			{
				if (cur[0] != prev[0] && !done[prev[0]])
				{
					done[prev[0]] = true;
					AddNewHolecupTri(meshes, tri, tri.pos[prev[0]],
						tri.pos[cur[0]], &poly[i], tri.index[prev[0]],
						tri.index[cur[0]], -1);
				}
			}
			else if (prevCount == 2)
			{
				if (cur[0] != prev[1] && !done[prev[1]])
				{
					done[prev[1]] = true;
					AddNewHolecupTri(meshes, tri, tri.pos[prev[1]],
						tri.pos[cur[0]], &poly[i], tri.index[prev[1]],
						tri.index[cur[0]], -1);
				}
			}

			if (i < n - 1)
				AddNewHolecupTri(meshes, tri, tri.pos[cur[0]], &poly[i + 1],
					&poly[i], tri.index[cur[0]], -1, -1);

			open = true;
			break;
		case 2:
			if (prevCount == 1)
			{
				int p = prev[0];

				if (cur[0] != p && !done[p])
				{
					done[p] = true;
					open = true;
					AddNewHolecupTri(meshes, tri, tri.pos[p], tri.pos[cur[0]],
						&poly[i], tri.index[p], tri.index[cur[0]], -1);
				}
			}

			if (i < n - 1)
			{
				if (!done[cur[0]])
				{
					done[cur[0]] = true;
					AddNewHolecupTri(meshes, tri, tri.pos[cur[0]],
						tri.pos[cur[1]], &poly[i], tri.index[cur[0]],
						tri.index[cur[1]], -1);
					AddNewHolecupTri(meshes, tri, tri.pos[cur[1]], &poly[i + 1],
						&poly[i], tri.index[cur[1]], -1, -1);
				}
				else if (cur[0] == first && open)
				{
					AddNewHolecupTri(meshes, tri, tri.pos[cur[0]], &poly[i + 1],
						&poly[i], tri.index[cur[0]], -1, -1);
				}
				else
				{
					AddNewHolecupTri(meshes, tri, tri.pos[cur[1]], &poly[i + 1],
						&poly[i], tri.index[cur[1]], -1, -1);
				}
			}
			break;
		}

		memcpy(prev, cur, sizeof(int) * count);
		prevCount = count;
	}
}

void WPolySoup::AddNewHolecupTri(std::vector<sHoleMesh>& meshes, sHoleTri& tri,
	WVector* a, WVector* b, WVector* c, int ia, int ib, int ic)
{
	WVector* pos[3] = { a, b, c };
	int index[3] = { ia, ib, ic };
	sHoleMesh* target = NULL;
	std::vector<sHoleMesh>::iterator it;
	for (it = meshes.begin(); it != meshes.end(); it++)
	{
		if ((*it).mesh == tri.mesh)
		{
			target = &*it;
			break;
		}
	}
	if (it == meshes.end())
	{
		sHoleMesh mesh;
		mesh.mesh = tri.mesh;
		meshes.push_back(mesh);
		it = meshes.end();
		it--;
		target = &*it;
	}
	std::vector<sHoleVtx>& vtxs = target->vtx;
	std::vector<int>& indices = target->index;
	for (int i = 0; i < 3; i++)
	{
		if (index[i] == -1)
		{
			sHoleVtx vtx;
			vtx.pos = *pos[i];
			GenNewHolecupVtx(&vtx, &tri);
			int n = (int)vtxs.size();
			int j;
			for (j = 0; j < n; j++)
			{
				sHoleVtx& v = vtxs[j];
				if (WisEqual(v.pos.x, vtx.pos.x, g_EPSILON) &&
					WisEqual(v.pos.y, vtx.pos.y, g_EPSILON) &&
					WisEqual(v.pos.z, vtx.pos.z, g_EPSILON) &&
					WisEqual(v.normal.x, vtx.normal.x, g_EPSILON) &&
					WisEqual(v.normal.y, vtx.normal.y, g_EPSILON) &&
					WisEqual(v.normal.z, vtx.normal.z, g_EPSILON))
				{
					size_t k = 0;
					while (k < v.uv.size() && v.uv[k].u == vtx.uv[k].u && v.uv[k].v == vtx.uv[k].v) // 983 0092e9c0: exato
						++k;
					if (k == v.uv.size())
						break;
				}
			}
			if (j == n)
			{
				indices.push_back(tri.mesh->vtxNum + n);
				vtxs.push_back(vtx);
			}
			else
			{
				indices.push_back(tri.mesh->vtxNum + j);
			}
		}
		else
		{
			indices.push_back(index[i]);
		}
	}
}

void WPolySoup::GenNewHolecupVtx(sHoleVtx* vtx, sHoleTri* tri)
{
	float u, v;
	BarycentricXZ(tri->pos[0], &vtx->pos, tri->pos[1], tri->pos[2], &u, &v);
	vtx->pos.y =
		Interpolate(tri->pos[0]->y, tri->pos[1]->y, tri->pos[2]->y, u, v);
	if (tri->mesh->normList)
	{
		vtx->normal.x = Interpolate(tri->normal[0]->x, tri->normal[1]->x,
			tri->normal[2]->x, u, v);
		vtx->normal.y = Interpolate(tri->normal[0]->y, tri->normal[1]->y,
			tri->normal[2]->y, u, v);
		vtx->normal.z = Interpolate(tri->normal[0]->z, tri->normal[1]->z,
			tri->normal[2]->z, u, v);
	}
	const unsigned char* c0 = (const unsigned char*)&tri->color[0];
	const unsigned char* c1 = (const unsigned char*)&tri->color[1];
	const unsigned char* c2 = (const unsigned char*)&tri->color[2];
	float a0 = c0[3];
	float r0 = c0[2];
	float g0 = c0[1];
	float b0 = *(const ulong*)c0 & 0xff;
	unsigned char a = (unsigned char)Interpolate(a0, c1[3], c2[3], u, v);
	unsigned char r = (unsigned char)Interpolate(r0, c1[2], c2[2], u, v);
	unsigned char g = (unsigned char)Interpolate(g0, c1[1], c2[1], u, v);
	unsigned char b = (unsigned char)Interpolate(b0, *(const ulong*)c1 & 0xff,
		*(const ulong*)c2 & 0xff, u, v);
	vtx->color = (a << 24) | (r << 16) | (g << 8) | b;
	vtx->uv.resize(tri->uv[0].size());
	for (size_t k = 0; k < vtx->uv.size(); ++k)
	{
		vtx->uv[k].u = Interpolate(tri->uv[0][k].u, tri->uv[1][k].u, tri->uv[2][k].u, u, v);
		vtx->uv[k].v = Interpolate(tri->uv[0][k].v, tri->uv[1][k].v, tri->uv[2][k].v, u, v);
	}
}

void WPolySoup::AddHolecup(WBone* bone, std::vector<sHoleMesh>& meshes,
	std::vector<sCupVtx>& cup, std::vector<sHoleTri>& tris)
{
	DigHolecup(bone, cup, tris);

	std::vector<sHoleMesh>::iterator it;

	for (it = meshes.begin(); it != meshes.end(); it++)
	{
		ApplyHoleMesh(*it, tris);
	}
}

void WPolySoup::ApplyHoleMesh(sHoleMesh& hole, std::vector<sHoleTri>& tris)
{
	w_mesh* mesh;
	int vtxNum;
	int indexNum;
	int removed;
	WVector* vecList;
	WVector* normList;
	ulong* colorList;
	ulong* originalColorList;
	ushort* indexList;
	std::vector<sHoleTri>::iterator tri;

	mesh = hole.mesh;

	vtxNum = mesh->vtxNum + (int)hole.vtx.size();
	vecList = new WVector[vtxNum];

	if (mesh->normList)
		normList = new WVector[vtxNum];
	else
		normList = NULL;

	colorList = new ulong[vtxNum];
	originalColorList = new ulong[vtxNum];
	// 983 (00944bb0): UVs em camadas (w_mesh +0x50), todas as camadas da malha (645: uvData/uvBackup)
	w_uvset983* newUV = new w_uvset983(*mesh->uv);
	for (w_uvset983::iterator it = newUV->begin(); it != newUV->end(); ++it)
		it->resize(vtxNum);

	removed = 0;
	for (tri = tris.begin(); tri != tris.end(); tri++)
	{
		if ((*tri).mesh == mesh)
			removed++;
	}

	indexNum = (int)hole.index.size() - removed * 3 + mesh->indexNum;
	indexList = new ushort[indexNum];

	memcpy(vecList, mesh->vecList, sizeof(WVector) * mesh->vtxNum);

	if (normList)
		memcpy(normList, mesh->normList, sizeof(WVector) * mesh->vtxNum);

	memcpy(colorList, mesh->vtxColorList, sizeof(ulong) * mesh->vtxNum);
	memcpy(originalColorList, mesh->originalVtxColorList,
		sizeof(ulong) * mesh->vtxNum);

	int v = mesh->vtxNum;
	if (v < vtxNum)
		do
		{
			sHoleVtx& vtx = hole.vtx[v - mesh->vtxNum];

			vecList[v] = vtx.pos;

			if (normList)
				normList[v] = vtx.normal;

			colorList[v] = vtx.color;
			originalColorList[v] = vtx.color;
			for (size_t k = 0; k < vtx.uv.size(); ++k)
				(*newUV)[k][v] = vtx.uv[k];

			v++;
		} while (v < vtxNum);

	for (tri = tris.begin(); tri != tris.end(); tri++)
	{
		if ((*tri).mesh == mesh)
		{
			int prev;
			int written;

			if ((*tri).offset > 0)
				memcpy(indexList, mesh->indexList,
					sizeof(ushort) * (*tri).offset);

			written = prev = (*tri).offset;

			for (tri++; tri != tris.end(); tri++)
			{
				if ((*tri).mesh != mesh)
					break;

				int gap = (*tri).offset - prev;
				if (gap > 3)
				{
					memcpy(&indexList[written], &mesh->indexList[prev + 3],
						sizeof(ushort) * (gap - 3));
					written += (*tri).offset - prev - 3;
				}

				prev = (*tri).offset;
			}

			if (mesh->indexNum - prev > 3)
			{
				memcpy(&indexList[written], &mesh->indexList[prev + 3],
					sizeof(ushort) * (mesh->indexNum - prev - 3));
				written += mesh->indexNum - prev - 3;
			}

			for (int j = written, k = 0; j < indexNum; j++, k++)
				indexList[j] = (ushort)hole.index[k];

			break;
		}
	}

	delete[] mesh->vecList;
	delete[] mesh->normList;
	delete[] mesh->vtxColorList;
	delete[] mesh->originalVtxColorList;
	delete mesh->uv;
	delete[] mesh->indexList;

	mesh->vtxNum = vtxNum;

	mesh->vecList = vecList;
	mesh->normList = normList;
	mesh->vtxColorList = colorList;
	mesh->originalVtxColorList = originalColorList;
	mesh->uv = newUV;
	mesh->indexNum = indexNum;
	mesh->indexList = indexList;
}

void WPolySoup::DigHolecup(WBone* bone, std::vector<sCupVtx>& cup,
	std::vector<sHoleTri>& tris)
{
	FindNewHolecupVtxs(cup, tris);

	WVector up(0.0f, 1.0f, 0.0f);
	WMatrix inv;
	WVector center;
	WVector light;
	WVector dir;
	float radius;
	float bottom;
	w_mesh* mesh = bone->GetMesh();

	if (mesh == NULL)
		return;

	for (; mesh->next; mesh = mesh->next)
		;

	w_mesh* hole = new w_mesh; // 983: construtor zera
	mesh->next = hole;

	if (m_holecupTex == 0)
		m_holecupTex = g_resrcmng->LoadTexture("holecup.dds", 0, 0, 0);

	hole->originalTexHandle = hole->texHandle = m_holecupTex; // 983 00945184: +0x38 e +0x84
	hole->drawFlag = (m_holecupTex & 0x7ff) | 0x10000000; // 983 0094519c (0x8000000 era o bit do 645)
	hole->diffuse = 0xffffff;
	hole->alpha = 1.0f;

	int n = (int)cup.size();

	hole->vtxNum = n * 2 + 4;
	hole->indexNum = n * 6;

	hole->vecList = new WVector[hole->vtxNum];
	hole->normList = new WVector[hole->vtxNum];
	hole->vtxColorList = new ulong[hole->vtxNum];
	hole->originalVtxColorList = new ulong[hole->vtxNum];
	hole->NewUV983(1, hole->vtxNum); // 983: UVs em camadas (w_mesh +0x50)
	hole->indexList = new ushort[hole->indexNum];

	radius = HOLECUP_RADIUS983();

	inv = ~bone->GetMatrix();
	center =
		WVector(
			WSingleton<CSharedDoc>::Instance()->m_pGolfDoc->GetHoleData().pin) *
		inv;
	bottom = cup[0].pos.y - GolfBall().m_radius * 6.0f; // 983 00945342: o 645 usava 1.0
	light = RotVec(g_lightset.nearOne, inv) * -1.0f;
	light.Normalize();

	int i;
	for (i = 0; i < n; i++)
	{
		sCupVtx& vtx = cup[i];

		dir = WVector(center.x - vtx.pos.x, 0.0f, center.z - vtx.pos.z);
		dir.Normalize();

		float angle;

		hole->vecList[i * 2] = hole->vecList[i * 2 + 1] = vtx.pos;
		hole->vecList[i * 2 + 1].y = bottom;

		hole->normList[i * 2] = hole->normList[i * 2 + 1] = dir;

		if (i == n - 1)
			angle = g_PI * 2.0f;
		else
		{
			float c = -dir.x < -1.0f ? -1.0f : (-dir.x > 1.0f ? 1.0f : -dir.x);
			angle = (float)acos(c);

			if (dir.z < 0.0f)
				angle = g_PI * 2.0f - angle;
		}

		float u = angle * 0.47746482f;

		hole->UVData983()[i * 2][0] = u;
		hole->UVData983()[i * 2][1] = 0.015625f;
		hole->UVData983()[i * 2 + 1][0] = u;
		hole->UVData983()[i * 2 + 1][1] = 0.90625f;

		float lit = light * dir * 255.0f;
		ulong color;
		if (lit <= 0.0f)
			color = g_lightset.ambient;
		else
			color = AddDiffuse(g_lightset.ambient, g_lightset.diffuse,
				(unsigned char)(int)lit);

		color |= 0xff000000;

		hole->originalVtxColorList[i * 2 + 1] = color;
		hole->originalVtxColorList[i * 2] = color;
		hole->vtxColorList[i * 2 + 1] = color;
		hole->vtxColorList[i * 2] = color;
	}

	int base = n * 2;
	ulong ambient = g_lightset.ambient | 0xff000000;
	WVector offset[4] = {
		WVector(-1.0f, 0.0f, 1.0f),
		WVector(1.0f, 0.0f, 1.0f),
		WVector(-1.0f, 0.0f, -1.0f),
		WVector(1.0f, 0.0f, -1.0f),
	};

	for (int j = 0; j < 4; j++)
	{
		hole->vecList[base + j] = center + offset[j] * radius;
		hole->vecList[base + j].y = bottom;

		hole->normList[base + j] = up;

		hole->UVData983()[base + j][0] = 0.0f;
		hole->UVData983()[base + j][1] = 0.96875f;

		hole->originalVtxColorList[base + j] = ambient;
		hole->vtxColorList[base + j] = ambient;
	}

	for (i = 0; i < n - 1; i++)
	{
		hole->indexList[i * 6] = (ushort)(i * 2);
		hole->indexList[i * 6 + 1] = (ushort)(i * 2 + 2);
		hole->indexList[i * 6 + 2] = (ushort)(i * 2 + 1);
		hole->indexList[i * 6 + 3] = (ushort)(i * 2 + 1);
		hole->indexList[i * 6 + 4] = (ushort)(i * 2 + 2);
		hole->indexList[i * 6 + 5] = (ushort)(i * 2 + 3);
	}

	hole->indexList[i * 6] = (ushort)n * 2;
	hole->indexList[i * 6 + 1] = (ushort)(n * 2 + 1);
	hole->indexList[i * 6 + 2] = (ushort)(n * 2 + 2);
	hole->indexList[i * 6 + 3] = (ushort)(n * 2 + 2);
	hole->indexList[i * 6 + 4] = (ushort)(n * 2 + 1);
	hole->indexList[i * 6 + 5] = (ushort)(n * 2 + 3);

	SetMeshPalette983(m_baseModel->pet, bone, hole);
}

void WPolySoup::FindNewHolecupVtxs(std::vector<sCupVtx>& cup,
	std::vector<sHoleTri>& tris)
{
	std::vector<sHoleTri>::iterator tri;
	std::vector<sCupVtx>::iterator it;
	std::vector<sCupVtx>::iterator prev;
	bool inside[2];
	sCupVtx newVtx;
	sHoleVtx vtx;

	for (tri = tris.begin(); tri != tris.end(); tri++)
	{
		prev = cup.begin();
		inside[0] = IsInTri((*prev).pos, (*tri).pos);
		it = prev;
		for (it++; it != cup.end(); it++)
		{
			inside[1] = IsInTri((*it).pos, (*tri).pos);
			if (inside[0] ^ inside[1])
			{
				for (int k = 0; k < 3; k++)
				{
					if (GetIntersection(&newVtx.pos, &(*prev).pos, &(*it).pos,
							(*tri).pos[k], (*tri).pos[(k + 1) % 3],
							(WVector*)&(*tri).edge[k]))
					{
						newVtx.mesh = (*tri).mesh;
						newVtx.index = (*tri).offset;
						it = cup.insert(it, newVtx);
						it++;
						break;
					}
				}
			}
			inside[0] = inside[1];
			prev = it;
		}
	}
	for (it = cup.begin(); it != cup.end(); it++)
	{
		vtx.pos = (*it).pos;
		for (tri = tris.begin(); tri != tris.end(); tri++)
		{
			if ((*tri).mesh == (*it).mesh && (*tri).offset == (*it).index)
			{
				GenNewHolecupVtx(&vtx, &*tri);
				(*it).pos.y = vtx.pos.y;
				break;
			}
		}
	}
}

WPolySoup::CStaticPetGrp::CStaticPetGrp()
	: m_num(0), m_grp(NULL)
{
}

WPolySoup::CStaticPetGrp::~CStaticPetGrp()
{
	Clear();
}

// 983 0092c810: vector por referencia, conta = size() (SetAdditionModel sobe m_modelNum sem crescer o vector)
void WPolySoup::CStaticPetGrp::Build(const std::vector<sModel*>& models)
{
	Clear();

	const int num = (int)models.size();

	int groups = 0;
	int i = 0;
	int j;
	int k;
	int index;
	int run;
	const char* last = "\0";

	m_num = 0;

	while (i < num)
	{
		if (models[i]->pet && models[i]->type == 0 &&
			strcmpi(models[i]->name, last))
		{
			last = models[i]->name;
			groups++;
		}

		i++;
	}

	if (groups == 0)
		return;

	m_grp = new WxStaticPuppetGrp*[groups];
	memset(m_grp, 0, sizeof(WxStaticPuppetGrp*) * groups);

	last = "\0";
	run = 1;
	index = 0;
	i = 0;

	while (index < groups)
	{
		sModel* const* cur = &models[i];

		if ((*cur)->pet && (*cur)->type == 0 && strcmpi((*cur)->name, last))
		{
			last = (*cur)->name;
			j = i + 1;
			sModel* const* next = cur;

			while (j < num && (*cur)->type == 0 &&
				strcmpi(next[1]->name, last) == 0)
			{
				j++;
				run++;
				next++;
			}

			WPuppet** pets = new WPuppet*[run];

			for (k = 0; k < run; k++)
				pets[k] = models[i + k]->pet;

			m_grp[index] =
				g_resrcmng->xGetStaticPuppetGrp(run, (*cur)->name, pets);

			i += run;
			index++;
			delete[] pets;
		}
		else
			i++;

		run = 1;
	}

	WSingleton<CSceneManager>::Instance()->RegisterElement(OBJ_STATIC, NULL,
		this, "StaticPetGrp", 2, NULL);

	m_num = groups;
}

void WPolySoup::CStaticPetGrp::Clear()
{
	for (int i = 0; i < m_num; i++)
	{
		g_resrcmng->Release(m_grp[i]);
	}

	if (m_grp)
	{
		delete[] m_grp;
		m_grp = NULL;
	}
	m_num = 0;
}

void WPolySoup::CStaticPetGrp::Display()
{
	g_view->xSetLight(0, g_lightset);

	for (int i = 0; i < m_num; i++)
	{
		if (m_grp[i])
		{
			m_grp[i]->xUpdateCullFlag(g_view);
			m_grp[i]->xRender(g_view);
		}
	}
}

void WPolySoup::UpdateBright()
{
	if (m_baseModel->pet)
		m_baseModel->pet->UpdateLightSource(&g_lightset, false, NULL);

	for (int i = 0; i < m_modelNum; i++)
	{
		if (m_model[i]->pet)
			m_model[i]->pet->UpdateLightSource(&g_lightset, false, NULL);
	}
}

void WPolySoup::ChangePointType(unsigned char from, unsigned char to)
{
	sPoint* point = m_point;
	unsigned int num = m_header.pointNum;
	for (unsigned int i = 0; i < num; i++, point++)
	{
		if (point == NULL)
			continue;
		if (point->type == from)
		{
			point->type = to;
			return;
		}
	}
}
C_ASSERT_VC10(offsetof(CSharedDoc, m_curChannel.Type) == 0x10a99); // A748: ex-cru Doc()+0x10a99
