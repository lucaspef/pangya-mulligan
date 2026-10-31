#include "wresrcmng.h"
#include "wpuppet.h"
#include <typeinfo>
#include "wbone.h"
#include "rectmng.h"
#include "wboneset.h"
#include "wpetfile.h"
#include "wmesh.h"
#include "wview.h"
#include "bitmap.h"
#include "wvideo.h"
#include <d3dx9math.h>
#include <stdio.h>
#include <algorithm>


unsigned char WPuppet::ms_lockedBoneSetPos = 0xff;
int WPuppet::box_v[8][3] = {
	{ 0, 0, 0 },
    { 1, 0, 0 },
    { 0, 1, 0 },
    { 1, 1, 0 },
    { 0, 0, 1 },
	{ 1, 0, 1 },
    { 0, 1, 1 },
    { 1, 1, 1 }
};
int WPuppet::box_p[6][4] = {
	{ 0, 2, 3, 1 },
    { 4, 5, 7, 6 },
    { 0, 1, 5, 4 },
    { 1, 3, 7, 5 },
	{ 3, 2, 6, 7 },
    { 2, 0, 4, 6 }
};
WList<WPuppet::w_tex_piece*>* WPuppet::m_tex_piece;
unsigned char WPuppet::ms_boneSetPos;
WMemFillBlock g_mem(0x4800, 5);


WPuppet::WPuppet()
	: m_addedMotionList(4, 0),
	  m_bbList(8, 8),
	  m_fanimList(4, 4)
{
	memset(m_petName, 0, sizeof(m_petName));
	m_frame = 0;
	m_framenum = 0;
	m_storeTexList = 0;
	m_rendMode = 0;
	m_aniLen = 0.0f;
	m_rootbone = 0;
	m_lightmode = 0;
	m_mdList = 0;
	m_bound = Waabb(WVector::ZERO, WVector::ZERO);
	m_boundSphere = WSphere(WVector::ZERO, 0.0f);
	m_BBox.bone = 0;
	m_BBox.info = 0;
	m_faceNum = 0;
	m_mat.Reset();
	m_petVersion983 = PET_VERSION_DEFAULT983;
	m_pOrigin983 = this; // 983 00b8f7e0
	m_boneList.m_pet = this; // 983 00b8fb78
}

WPuppet::~WPuppet()
{
	for (w_face_animation* anim = m_fanimList.Start(); anim;
		anim = m_fanimList.Next())
	{
		delete[] anim->mesh;
		delete anim;
	}
	for (w_bound_box* box = m_bbList.Start(); box; box = m_bbList.Next())
	{
		if (m_pOrigin983 == this) // 983 00b8fc10: o dono e' o boneco de origem
		{
			if (box->info->option)
				delete[] box->info->option;
			delete[] (char*)box->info;
		}
		delete box;
	}
	if (m_pOrigin983 == this)
	{
		if (m_mdList)
		{
			for (w_motion_data* data = m_mdList->Start(); data;
				data = m_mdList->Next())
				delete[] data;
			if (m_mdList)
			{
				delete m_mdList;
				m_mdList = 0;
			}
		}
		if (m_framenum > 0)
		{
			for (int i = 0; i < m_framenum; ++i)
			{
				if (m_frame[i].data)
				{
					delete[] m_frame[i].data;
					m_frame[i].data = 0;
				}
			}
			if (m_frame)
			{
				delete[] m_frame;
				m_frame = 0;
			}
		}
	}
	if (m_rootbone)
	{
		delete m_rootbone;
		m_rootbone = 0;
	}
	if (m_storeTexList)
	{
		for (int handle = m_storeTexList->Start(); handle;
			handle = m_storeTexList->Next())
			GetResrcManager()->Release(handle);
		delete m_storeTexList;
	}
	// 983 00b8fc10: materiais de shader (o boneco de origem devolve as texturas do arquivo)
	for (unsigned int i = 0; i < m_shaderMtls983.size(); ++i)
	{
		WxShaderMtl* mtl = m_shaderMtls983[i];
		if (mtl)
		{
			if (m_pOrigin983 == this)
			{
				for (WxShaderMtl::tTextureByName::iterator it = mtl->m_textures.begin();
					it != mtl->m_textures.end(); ++it)
					GetResrcManager()->Release(it->second.m_hOrgTex);
			}
			delete mtl;
		}
	}
	m_shaderMtls983.clear();
	ClearTexPiece();
	if (m_tex_piece && !m_tex_piece->Start())
	{
		delete m_tex_piece;
		m_tex_piece = 0;
	}
	for (std::map<std::string, w_modifier*>::iterator it = m_modifiers983.begin(); it != m_modifiers983.end(); ++it)
		delete it->second;
	m_modifiers983.clear();
}

// 983 00b92340: dados do boneco de origem; ossos 0x100000 nao sao clonados (o ResetMeshBone os cria se uma malha usar)
void WPuppet::Share(w_share_pet_data* data)
{
	m_pOrigin983 = data->pet;
	strcpy(m_petName, data->pet->m_petName);
	m_frame = data->frame;
	m_framenum = data->numFramedata;
	m_mdList = data->mdList;
	m_bound = data->bound;
	m_faceNum = data->faceNum;
	m_aniLen = data->length;
	m_rootbone = data->pet->m_rootbone->MakeClone(0, 0);
	CopyShaderMtls983(*data->shaderMtls983);
	xBuildTransfMatPtrList();
	m_rootbone->ResetMeshBone(this, data->pet, true);
	m_rootbone->CombineVtxColor(true);
	ForEachBone983(this, InternPalettes983, 0);
	BindPalettes983(0);

	for (w_bound_box* box = data->bbList->Start(); box != 0;
		box = data->bbList->Next())
	{
		if (!box->bone->IsExtraBone983())
			AddBoundBox983(box);
	}

	w_bound_box* found = data->bbList->Find("bound");
	if (found == 0)
		found = data->bbList->Find("center");
	if (found != 0)
	{
		m_BBox.bone = found->bone;
		m_BBox.info = found->info;
	}
	if (m_BBox.bone != 0)
		m_BBox.bone = m_rootbone->FindBone(m_BBox.bone->GetBoneName(), 0);

	for (w_face_animation* anim = data->fanimList->Start(); anim != 0;
		anim = data->fanimList->Next())
	{
		w_face_animation* copy = new w_face_animation;
		*copy = *anim;
		copy->mesh = new w_mesh*[anim->meshNum + 1];
		copy->mesh[anim->meshNum] = 0;
		if (anim->meshNum > 0)
			m_rootbone->GetMeshForFaceAnimation((unsigned char)copy->group,
				copy->mesh, 0);
		m_fanimList.AddItem(copy, copy->name, false);
	}

	SetRenderMode(m_rendMode);
}

WPuppet* WPuppet::MakeClone(bool clone)
{
	w_share_pet_data data;
	data.pet = this;
	data.length = m_aniLen;
	data.faceNum = m_faceNum;
	data.numFramedata = m_framenum;
	data.frame = m_frame;
	data.bound = m_bound;
	data.bbList = &m_bbList;
	data.mdList = m_mdList;
	data.fanimList = &m_fanimList;
	data.shaderMtls983 = &m_shaderMtls983;

	WPuppet* pet = new WPuppet();
	pet->m_petVersion983 = m_petVersion983; // 983 00b9270c
	pet->m_iPetType = m_iPetType;
	pet->m_rendMode = m_rendMode;
	pet->Share(&data);
	return pet;
}

int WPuppet::GetRenderMode() const
{
	return m_rendMode; // +0xe4
}

void WPuppet::SetRenderMode(int mode)
{
	m_rendMode = mode;
	if (m_rootbone)
		m_rootbone->SetRenderMode(mode);
}

void WPuppet::Transform(WView* view, float value, int flag)
{
	m_rootbone->Transform(m_mat, view, value, flag);
}

void WPuppet::UpdateLightSource(WScene* scene)
{
	m_rootbone->CalcLight(scene, m_lightmode);
}

void WPuppet::UpdateLightSource(LightSet* light, bool enable, WScene* scene)
{
	m_rootbone->SetLight(light);
	int mode = m_lightmode;
	if (enable)
		mode = mode | 0x8000000;
	m_rootbone->CalcLight(light, true, mode, scene);
}

void WPuppet::Render(WView* view, bool transform, float value, bool light,
	bool shadow, bool unused)
{
	if (transform)
	{
		int flag = (light != false ? 1 : 0) | (shadow ? 0 : 2);
		Transform(view, value, flag);
	}
	m_rootbone->Render(view, m_rootbone->m_light);
	// 983 00b6ebc0 ainda chama 00b69ae0 (desenho de depuracao de ossos/normais guiado por
	// 3 flags globais 00f14a14..16 que nada escreve: codigo morto, omitido)
}

// WPuppet::RenderHierarchy/RenderNormals (645): sem par no 983 e sem chamadores; o 983 so tem o
// desenho de depuracao dentro de Render (00b69ae0, flags globais nunca ligadas)

float WPuppet::GetMotionLength(const char* name)
{
	if (name != 0)
	{
		w_motion_data* data = FindMotionData(name);
		if (data == 0)
			return -1.0f;
		return data->end - data->s;
	}
	return m_aniLen;
}

w_motion_data* WPuppet::FindMotionData(const char* name)
{
	// 983 (00b73750): so a lista propria; as figuras adicionadas nao entram
	if (m_mdList == 0)
		return 0;
	return m_mdList->Find(name);
}

int WPuppet::GetMotionCountIncludeName(const char* include, const char* exclude)
{
	if (m_mdList == 0)
		return 0;

	int count = 0;
	w_motion_data* md;
	for (md = m_mdList->Start(); md != 0; md = m_mdList->Next())
	{
		if (strstr(md->name, include) != 0 && strstr(md->name, exclude) == 0)
			++count;
	}
	if (count == 0)
	{
		for (w_motion_addition* add = m_addedMotionList.Start();
			add != 0 && md == 0; add = m_addedMotionList.Next())
		{
			for (md = m_mdList->Start(); md != 0; md = m_mdList->Next())
			{
				if (strstr(md->name, include) != 0 &&
					strstr(md->name, exclude) == 0)
					++count;
			}
		}
	}
	return count;
}

char* WPuppet::GetNextMotionName(const char* name)
{
	w_motion_data* data = FindMotionData(name);
	if (data->s == data->e && strcmp(data->name, data->nextmotion) == 0)
		return 0;
	return data->nextmotion;
}

// 983 00b7b180
void WPuppet::ApplyBones983(WBoneSet* set)
{
	if (m_rootbone)
	{
		m_rootbone->ApplyBoneSetList(set);
		ApplyModifiers983();
	}
}

// 983 00b79730
void WPuppet::ApplyModifiers983()
{
	for (std::map<std::string, w_modifier*>::iterator it = m_modifiers983.begin();
		it != m_modifiers983.end(); ++it)
		it->second->Apply(this);
}

// 983 00b7f380 (o 007410c0 da camera procura o PetModifier::CSuperDeformedPet por aqui)
WPuppet::w_modifier* WPuppet::FindModifier983(const std::string& name)
{
	std::map<std::string, w_modifier*>::iterator it = m_modifiers983.find(name);
	if (it != m_modifiers983.end())
		return it->second;
	return 0;
}

// 983 00b8e4f0: um modificador por tipo
void WPuppet::AddModifier983(w_modifier* modifier)
{
	std::string name = typeid(*modifier).name();
	if (FindModifier983(name) == 0)
		m_modifiers983[name] = modifier;
}

// 983 00b86cf0: pose da animacao `name` no tempo `time` (GetBones do 645 sem as animacoes acrescentadas). Com set != 0
// so preenche o conjunto; com set == 0 os ossos recebem as chaves direto e os modificadores rodam no fim.
void WPuppet::Animate983(WBoneSet* set, const char* name, float time, int mode,
	char* bone, WPuppet* filter)
{
	if (filter != 0 && filter == this)
		filter = 0;
	if (name != 0)
	{
		w_motion_data* data = FindMotionData(name);
		if (bone == 0 && *data->applybone != 0)
			bone = data->applybone;
		time = time + data->s;
		if (time > data->e)
		{
			float ratio = (time - data->e) / (data->end - data->e);
			if ((unsigned int)(data->option1 - 1) <= 1)
			{
				WBoneSet first;
				WBoneSet second;
				WBoneSet* dest = set;
				if (set == 0)
				{
					int count = 0;
					if (filter != 0)
						filter->m_rootbone->CountBones(count);
					else
						m_rootbone->CountBones(count);
					first.Clear();
					first.m_data.resize(count);
					dest = &first;
				}
				int num = dest->m_data.size();
				second.Clear();
				second.m_data.resize(num);
				float next;
				if (data->option1 == 2)
				{
					Animate983(dest, 0, time, mode, bone, filter);
					next = time - data->e;
				}
				else
				{
					Animate983(dest, 0, data->e, mode, bone, filter);
					next = 0.0f;
				}
				Animate983(&second, data->nextmotion, next, mode, bone, filter);
				second.BlendTo(dest, ratio);
				if (set == 0)
					ApplyBones983(&first);
				return;
			}
		}
	}
	if (set != 0)
		set->Clear();
	if (time > m_aniLen)
		time = m_aniLen;
	if (bone != 0)
	{
		WBone* found = FindBone(bone);
		if ((mode & 4) == 0)
		{
			found->InsertBoneSetList(set, time, 1, filter);
		}
		else
		{
			for (WBone* child = found->m_child; child != 0; child = child->m_next)
				child->InsertBoneSetList(set, time, 1, filter);
		}
	}
	else
	{
		for (WBone* item = m_rootbone; item != 0; item = item->m_next)
			item->InsertBoneSetList(set, time, 1, filter);
	}
	if (mode & 1)
	{
		if (set == 0)
		{
			// 983: 0092a460 copia a matriz para o +0x110 (m_localMat) fora de linha
			WMatrix local = m_rootbone->m_localMat;
			local.pivot = m_rootbone->GetPivot();
			m_rootbone->m_localMat = local;
			ApplyModifiers983();
			return;
		}
		WBoneKey* key = set->FindBoneKey(m_rootbone->GetBoneMotionName());
		if (key != 0 && (key->flags & 2) != 0)
			key->pivot = m_rootbone->GetPivot();
	}
	if (set == 0)
		ApplyModifiers983();
}

WVector WPuppet::GetDeltaVec(float time)
{
	// 983 (00b69a40): direto no osso raiz
	return m_rootbone->GetDeltaVec(time);
}

WVector WPuppet::GetDeltaVec(char* name, float time)
{
	w_motion_data* data = FindMotionData(name);
	return GetDeltaVec(time + data->s);
}

WVector WPuppet::GetDeltaVec(char* name, float from, float to)
{
	if (from == to)
		return WVector::ZERO;
	w_motion_data* data;
	if (name != 0)
	{
		data = FindMotionData(name);
		float length = data->e - data->s;
		float span = length > to ? to : length;
		if (to > span)
		{
			if (from < span)
				return GetDeltaVec(name, from, span) +
					GetDeltaVec(name, span, to);
			if (from >= span)
			{
				float ratio = (to - data->e) / (data->end - data->e);
				switch (data->option1)
				{
				case 1:
					return WVector::ZERO;
				case 2:
					return GetDeltaVec(0, from + data->s, to + data->s) *
						(1.0f - ratio) +
						GetDeltaVec(data->nextmotion, from - data->e,
							to - data->e) *
						ratio;
				}
			}
		}
	}
	return name ? GetDeltaVec(to + data->s) - GetDeltaVec(from + data->s)
				: GetDeltaVec(to) - GetDeltaVec(from);
}

void WPuppet::SetDefaultFaceAnimation(void)
{
	w_face_animation* anim = new w_face_animation;
	strcpy(anim->name, "\261\342\272\273");
	anim->texHandle = 0;
	anim->tu = 0.0f;
	anim->tv = 0.0f;
	anim->mesh = 0;
	anim->group = 0;
	anim->meshNum = m_rootbone->GetMeshForFaceAnimation(0, 0, 0);
	anim->mesh = new w_mesh*[anim->meshNum + 1];
	m_rootbone->GetMeshForFaceAnimation(0, anim->mesh, 0);
	anim->mesh[anim->meshNum] = 0;
	m_fanimList.AddItem(anim, anim->name, false);
}

char* WPuppet::FindTexAniList(char* name)
{
	char* buffer = 0;
	if (m_fanimList.Find(name) == 0)
		return 0;

	int group = m_fanimList.Find(name)->group;
	for (int pass = 0, len = 0; pass < 2; ++pass)
	{
		if (pass == 1)
		{
			buffer = new char[len + pass];
			*(int*)buffer = 0;
		}
		for (w_face_animation* anim = m_fanimList.Start(); anim != 0;
			anim = m_fanimList.Next())
		{
			if (anim->group != group)
				continue;
			if (strcmpi(anim->name, name) != 0)
			{
				if (pass == 0)
				{
					len += (int)strlen(anim->name) + 1;
				}
				else
				{
					strcat(buffer, anim->name);
					strcat(buffer, " ");
				}
			}
		}
	}
	return buffer;
}

void WPuppet::AddCollisionObject(const char* name, WBlockModel* model)
{
	WBone* bone = m_rootbone->FindBone(name, 0);
	bone->GetConvexArea(model);
}

void WPuppet::SetSelfIllum(char* name, bool enabled)
{
	if (name != 0 && strcmpi(name, "Recursive") != 0)
	{
		WBone* bone = FindBone(name);
		if (bone)
			bone->SetSelfIllum(enabled, false);
		return;
	}
	m_rootbone->SetSelfIllum(enabled, true);
}

void WPuppet::SetSelfIllumColor(char* name, unsigned long color)
{
	if (name != 0 && strcmpi(name, "Recursive") != 0)
	{
		WBone* bone = FindBone(name);
		if (bone)
			bone->SetSelfIllumColor(color, false);
		return;
	}
	m_rootbone->SetSelfIllumColor(color, true);
}

void WPuppet::ShowBone(char* name)
{
	if (name != 0 && strcmp(name, "Recursive") != 0)
	{
		WBone* bone = FindBone(name);
		if (bone)
			bone->ShowBone(false);
		return;
	}
	m_rootbone->ShowBone(true);
}

void WPuppet::HideBone(char* name)
{
	if (name != 0 && strcmp(name, "Recursive") != 0)
	{
		WBone* bone = FindBone(name);
		if (bone)
			bone->HideBone(false);
		return;
	}
	m_rootbone->HideBone(true);
}

unsigned char WPuppet::GetAlpha(char* name)
{
	if (name != 0)
		return FindBone(name)->m_alpha;
	return m_rootbone->m_alpha;
}

void WPuppet::SetAlpha(unsigned char alpha)
{
	m_rootbone->SetAlpha(alpha, 1);
}

void WPuppet::SetLightLine(float value)
{
	m_rootbone->SetLightLine(value);
}

void WPuppet::SetAlpha(char* name, unsigned char alpha)
{
	if (strcmpi(name, "Recursive") == 0)
	{
		m_rootbone->SetAlpha(alpha, 1);
		return;
	}
	WBone* bone = FindBone(name);
	if (bone)
		bone->SetAlpha(alpha, 0);
}

void WPuppet::ToggleSelfIllum(char* name)
{
	if (name != 0)
	{
		FindBone(name)->ToggleSelfIllum();
	}
}

void WPuppet::SetMirrored(bool mirrored)
{
	if (m_rootbone)
		m_rootbone->SetMirrored(mirrored);
}

void WPuppet::SetBoundBox(char* name, Waabb* aabb, char* bonename)
{
	w_bound_box* box = new w_bound_box;
	box->info = (w_bound_box_info*)new char[strlen(name) + 48];
	box->bone = m_rootbone->FindBone(bonename, 0);
	box->info->aabb = *aabb;
	if (box->info->aabb.max.x - box->info->aabb.min.x < 0.001f)
		box->info->aabb.max.x += 0.001f;
	if (box->info->aabb.max.y - box->info->aabb.min.y < 0.001f)
		box->info->aabb.max.y += 0.001f;
	if (box->info->aabb.max.z - box->info->aabb.min.z < 0.001f)
		box->info->aabb.max.z += 0.001f;
	box->info->option = 0;
	box->info->spherePivot = (aabb->min + aabb->max) * 0.5f;
	box->info->sphereLength = (aabb->max - aabb->min).Magnitude() * 0.5f;
	strcpy(box->info->name, name);
	m_bbList.AddItem(box, box->info->name, false);
}

// 983 00b78680
WBone* WPuppet::FindBone(const char* name)
{
	return m_boneList.Find(name);
}

// 983 00b76b00
WBone* WPuppet::w_bone_cache983::Find(const char* name)
{
	WInstanceLock lock(&m_lock);
	WBone* bone = m_list.Find(name);
	if (bone == 0)
	{
		bone = m_pet->m_rootbone->FindBone(name, 0);
		if (bone)
			m_list.AddItem(bone, bone->m_name, false);
	}
	return bone;
}

// 983 00b796f0
void WPuppet::w_bone_cache983::Remove(WBone* bone)
{
	WInstanceLock lock(&m_lock);
	if (m_list.Find(bone->m_name))
		m_list -= bone;
}

WBone* WPuppet::FindBone(int id)
{
	return m_rootbone->FindBone(id);
}

bool WPuppet::GetBoundBoxPlane(char* name, WPlane* plane)
{
	w_bound_box* box = m_bbList.Find(name);
	if (box)
	{
		MakePlaneEq(plane, box->info->aabb, &box->bone->m_matrix);
		return true;
	}
	return false;
}

bool WPuppet::GetAlignBoundBoxPlane(char* name, WPlane* plane, Waabb* box)
{
	Waabb aabb;
	aabb.min.x = aabb.min.y = aabb.min.z = 0.0f;
	aabb.max.x = aabb.max.y = aabb.max.z = 0.0f;

	w_bound_box* bound = m_bbList.Find(name);
	if (bound != 0)
	{
		WVector corner = WVector(bound->info->aabb.max.x,
							 bound->info->aabb.max.y, bound->info->aabb.max.z) *
			bound->bone->m_matrix;
		aabb.min = aabb.max = corner;

		for (int i = 1; i < 8; ++i)
		{
			float z = (i & 4) != 0 ? bound->info->aabb.min.z
								   : bound->info->aabb.max.z;
			float y = (i & 2) != 0 ? bound->info->aabb.min.y
								   : bound->info->aabb.max.y;
			float x = (i & 1) != 0 ? bound->info->aabb.min.x
								   : bound->info->aabb.max.x;
			WVector vec;
			vec = WVector(x, y, z) * bound->bone->m_matrix;
			aabb += WVector(vec);
		}

		if (plane != 0)
			MakePlaneEq(plane, aabb, 0);
		if (box != 0)
			*box = aabb;
		return true;
	}
	return false;
}

bool WPuppet::DotContact(char* name, const WVector& dot)
{
	w_bound_box* box = m_bbList.Find(name);
	WVector corner[8];
	int i;
	for (i = 0; i < 8; ++i)
	{
		float z = box_v[i][2] ? box->info->aabb.max.z : box->info->aabb.min.z;
		float y = box_v[i][1] ? box->info->aabb.max.y : box->info->aabb.min.y;
		float x = box_v[i][0] ? box->info->aabb.max.x : box->info->aabb.min.x;
		corner[i] = WVector(x, y, z) * box->bone->m_matrix;
	}
	for (int f = 0; f < 6; ++f)
	{
		if (WPlane(WCrossProduct(corner[box_p[f][0]] - corner[box_p[f][1]],
					   corner[box_p[f][1]] - corner[box_p[f][2]]),
				corner[box_p[f][3]]) *
				dot >
			0.05f)
			return false;
	}
	return true;
}

void WPuppet::GetBoneName(char** names, int& count)
{
	m_rootbone->GetName(names, count);
}

WVector WPuppet::GetPivot(const char* name)
{
	if (name != 0)
	{
		WBone* bone = FindBone(name);
		if (bone)
			return bone->GetPivot();
	}
	return m_rootbone->GetPivot();
}

WMatrix WPuppet::GetRootMatrix()
{
	return m_rootbone->m_localMat;
}

WBone* WPuppet::FindBoneByTail(const char* tail)
{
	if (m_rootbone)
		return m_rootbone->FindBoneByTail(tail);
	return 0;
}

bool WPuppet::CheckFrameData(int first, int last, int& from, int& to)
{
	int index;
	for (index = 0; index < m_framenum; ++index)
		if (first <= m_frame[index].num)
			break;
	if (index < m_framenum && m_frame[index].num <= last)
	{
		from = index;
		to = index;
		while (to + 1 < m_framenum && m_frame[to + 1].num <= last)
			++to;
		return true;
	}
	return false;
}

void WPuppet::MakeTemporaryCenterBox(void)
{
	Waabb box;
	box.min.x = box.min.y = box.min.z = 0.0f;
	box.max.x = box.max.y = box.max.z = 0.0f;

	if (m_bbList.Find("center") == 0)
	{
		box = m_bound;
		SetBoundBox("center", &box, m_rootbone->m_name);
		m_bHaveCenter = false;
		if (m_BBox.bone != 0)
		{
			w_bound_box* found = m_bbList.Find("center");
			m_BBox.bone = found->bone;
			m_BBox.info = found->info;
		}
	}
	else
	{
		m_bHaveCenter = true;
	}
}

void WPuppet::OptimizeBone(void)
{
	for (WBone* bone = m_rootbone; bone; bone = bone->m_next)
		ReduceBone(bone);
}

bool WPuppet::ReduceBone(WBone* bone)
{
	int reduced = CheckStaticBone(bone) ? 1 : 0;
	WBone* child = bone->m_child;
	while (child != 0)
	{
		if (ReduceBone(child) == true)
		{
			child = MergeBone(bone, child);
		}
		else
		{
			child = child->m_next;
			reduced = false;
		}
	}
	return (bool)reduced;
}

bool WPuppet::CheckStaticBone(WBone* bone)
{
	if (bone->HasAnimation() || bone->m_parent == 0)
		return false;

	for (w_motion_data* motion = m_mdList->Start(); motion != 0;
		motion = m_mdList->Next())
	{
		if (strcmp(motion->applybone, bone->m_name) == 0)
			return false;
	}
	for (w_bound_box* box = m_bbList.Start(); box != 0; box = m_bbList.Next())
	{
		if (box->bone == bone)
			return false;
	}
	if (m_rootbone->CheckRigidVtx(bone))
		return false;
	return true;
}

WBone* WPuppet::MergeBone(WBone* target, WBone* source)
{
	WBone* next = source->m_next;
	WBone* parent = source->m_parent;
	parent->ReleaseChild(source);

	WSphere sphere;
	sphere = target->GetBoundSphere();
	float radius = sphere.radius;
	sphere.pos = parent->Transform(sphere.pos);
	Waabb box;
	box = Waabb(WVector(sphere.pos.x - radius, sphere.pos.y - radius,
					sphere.pos.z - radius),
		WVector(radius + sphere.pos.x, radius + sphere.pos.y,
			radius + sphere.pos.z));

	target->MergeMesh(source, m_rootbone);
	target->SetBoundBox(box, true);
	source->ReleaseLink();
	FreeBone(source);
	delete source;
	return next;
}

void WPuppet::CalcBound(Waabb* box, bool flag)
{
	WVector min;
	WVector max;
	min.x = 99999.0f;
	min.y = 99999.0f;
	min.z = 99999.0f;
	max.x = -99999.0f;
	max.y = -99999.0f;
	max.z = -99999.0f;
	if (box == 0)
	{
		SetMatrix(WMatrix::IDENTITY);
		if (m_mdList != 0)
		{
			for (w_motion_data* data = m_mdList->Start(); data != 0;
				data = m_mdList->Next())
			{
				Animate983(0, 0, data->s, flag != 0, 0, 0);
				Transform(0, 1.0f, 0);
				m_rootbone->GetTotalOverlapBox(&min, &max, 0, false);
				Animate983(0, 0, data->e, flag != 0, 0, 0);
				Transform(0, 1.0f, 0);
				m_rootbone->GetTotalOverlapBox(&min, &max, 0, false);
			}
		}
		Animate983(0, 0, 0.0f, flag != 0, 0, 0);
		Transform(0, 1.0f, 0);
		m_rootbone->GetTotalOverlapBox(&min, &max, 0, false);
		m_bound = Waabb(min, max);
	}
	else
	{
		m_bound = *box;
	}
	m_boundSphere.pos = (m_bound.min + m_bound.max) * 0.5f;
	m_boundSphere.radius =
		(float)sqrt((m_boundSphere.pos - m_bound.min).SquareMagnitude());
}

// 983 00b87290 (trecho repetido 5x, em linha): pose no tempo `t` (da propria figura ou de `puppet` com filtro this),
// transforma e acumula a caixa dos ossos em min/max. Macro para nao sobrar copia fora de linha (SEM_PAR).
#define CALC_BOUND_POSE(t) \
	do \
	{ \
		if (puppet != 0) \
		{ \
			WBoneSet set; \
			int count = 0; \
			m_rootbone->CountBones(count); \
			set.Clear(); \
			set.m_data.resize(count); /* 983 b87387: o conjunto local ja tem uma chave por osso */ \
			puppet->Animate983(&set, 0, t, mode, 0, this); \
			ApplyBones983(&set); \
		} \
		else \
			Animate983(0, 0, t, mode, 0, 0); \
		Transform(0, 1.0f, 0); \
		m_rootbone->GetTotalOverlapBox(&min, &max, 0, false); \
	} while (0)

void WPuppet::CalcBound(bool reset, WPuppet* puppet, bool mode, char* name,
	Waabb* box)
{
	WVector min;
	WVector max;
	min.x = 99999.0f;
	min.y = 99999.0f;
	min.z = 99999.0f;
	max.x = -99999.0f;
	max.y = -99999.0f;
	max.z = -99999.0f;
	if (reset)
	{
		m_bound.min.x = m_bound.min.y = m_bound.min.z = 0.0f;
		m_bound.max.x = m_bound.max.y = m_bound.max.z = 0.0f;
	}
	if (box != 0)
	{
		box->min = box->max = WVector::ZERO;
	}
	WList<w_motion_data*>* list;
	if (puppet != 0)
		list = puppet->m_mdList;
	else
		list = m_mdList;
	if (list != 0)
	{
		for (w_motion_data* data = list->Start(); data != 0;
			data = list->Next())
		{
			if (name != 0 && strcmp(data->name, name) != 0)
				continue;
			CALC_BOUND_POSE(data->s);
			if (data->e != data->s)
			{
				CALC_BOUND_POSE(data->e);
				if (name != 0)
					break;
			}
		}
	}
	if (name == 0)
		CALC_BOUND_POSE(0.0f);
	if (box == 0)
	{
		min.x = Min(m_bound.min.x, min.x);
		max.x = Max(m_bound.max.x, max.x);
		min.y = Min(m_bound.min.y, min.y);
		max.y = Max(m_bound.max.y, max.y);
		min.z = Min(m_bound.min.z, min.z);
		max.z = Max(m_bound.max.z, max.z);
		m_bound.min = min;
		m_bound.max = max;
		m_boundSphere.pos = (max + min) * 0.5f;
		m_boundSphere.radius =
			(float)sqrt((m_boundSphere.pos - min).SquareMagnitude());
	}
	else
	{
		box->min = min;
		box->max = max;
	}
}
#undef CALC_BOUND_POSE

Waabb WPuppet::GetTransformedBoundBox(void) const
{
	Waabb bound;
	WVector vtx[8] = { WVector(m_bound.min.x, m_bound.min.y, m_bound.min.z),
		WVector(m_bound.max.x, m_bound.min.y, m_bound.min.z),
		WVector(m_bound.min.x, m_bound.max.y, m_bound.min.z),
		WVector(m_bound.min.x, m_bound.min.y, m_bound.max.z),
		WVector(m_bound.min.x, m_bound.max.y, m_bound.max.z),
		WVector(m_bound.max.x, m_bound.min.y, m_bound.max.z),
		WVector(m_bound.max.x, m_bound.max.y, m_bound.min.z),
		WVector(m_bound.max.x, m_bound.max.y, m_bound.max.z) };
	for (int i = 0; i < 8; ++i)
		vtx[i] = vtx[i] * m_mat;
	bound.min = WVector(3.402823466e38F, 3.402823466e38F, 3.402823466e38F);
	bound.max = WVector(-3.402823466e38F, -3.402823466e38F, -3.402823466e38F);
	for (int j = 0; j < 8; ++j)
	{
		if (vtx[j].x < bound.min.x)
			bound.min.x = vtx[j].x;
		if (vtx[j].y < bound.min.y)
			bound.min.y = vtx[j].y;
		if (vtx[j].z < bound.min.z)
			bound.min.z = vtx[j].z;
		if (vtx[j].x > bound.max.x)
			bound.max.x = vtx[j].x;
		if (vtx[j].y > bound.max.y)
			bound.max.y = vtx[j].y;
		if (vtx[j].z > bound.max.z)
			bound.max.z = vtx[j].z;
	}
	return bound;
}

void WPuppet::UpdateBound(bool transform, bool flag)
{
	WVector vMin(99999.0f, 99999.0f, 99999.0f);
	WVector vMax(-99999.0f, -99999.0f, -99999.0f);
	if (transform)
		m_rootbone->Transform(m_mat, 0, 1.0f, 0);
	m_rootbone->GetTotalOverlapBox(&vMin, &vMax, 0, flag);
	m_bound = Waabb(vMin, vMax);
	m_boundSphere.pos = (m_bound.max + m_bound.min) * 0.5f;
	m_boundSphere.radius =
		(float)sqrt((m_bound.max - m_bound.min).SquareMagnitude()) * 0.5f;
}

void WPuppet::UpdateBSphere(void)
{
	WBone* bone = m_BBox.bone;
	if (bone)
	{
		WMatrix* matrix = &bone->m_matrix;
		w_bound_box_info* info = m_BBox.info;
		WVector min;
		WVector max;
		min = info->aabb.min * *matrix;
		max = info->aabb.max * *matrix;
		m_boundSphere.pos = (min + max) * 0.5f;
		m_boundSphere.radius = (max - m_boundSphere.pos).Magnitude();
	}
}

void WPuppet::SetVtxColor(unsigned long color)
{
	m_rootbone->SetVtxColor(color);
}

void WPuppet::UpdatePhysics(float delta, const WVector& accel)
{
	for (WBone* bone = m_phybone.Start(); bone; bone = m_phybone.Next())
		bone->UpdatePhysicsModel(delta, accel);
}

void WPuppet::SetPhysicsModel(const char* name, int model)
{
	WBone* bone = FindBone(name);
	bone->SetPhysicsModel(model);
	m_phybone += bone;
}

void WPuppet::RotateBone(char* name, float angle, int axis)
{
	char* key = *name == '@' ? name + 1 : name;
	WBone* bone = FindBone(key);
	if (bone == 0)
		return;
	bone->Rotate(angle, axis, *name != '@');
}

void WPuppet::MoveBone(char* name, WVector& vector)
{
	if (strcmpi(name, "Recursive") == 0)
	{
		m_rootbone->Move(vector, true);
		return;
	}
	WBone* bone = FindBone(name);
	if (bone)
		bone->Move(vector, false);
}

void WPuppet::RenderBoundBox(const char* name, WView* view, unsigned long color,
	float size)
{
	static unsigned long box_color[6] = { 0xffffff00, 0xff00ff00, 0xffffff00,
		0xffff0000, 0xff0000ff, 0xffffff00 };
	static WVector emptyMin(99999.0f, 99999.0f, 99999.0f);
	static WVector emptyMax(-99999.0f, -99999.0f, -99999.0f);
	static WVector under(-0.01f, -0.01f, -0.01f);
	static WVector over(0.01f, 0.01f, 0.01f);

	w_bound_box* box = m_bbList.Find(name);
	if (box == 0)
		return;
	WVector v[8];
	WVector a, b, normal, center;
	int i;
	if (WisEqual(box->info->aabb.min, emptyMin, 0.00001f) != 0 &&
		WisEqual(box->info->aabb.max, emptyMax, 0.00001f) != 0)
	{
		for (i = 0; i < 8; ++i)
			v[i] = box->bone->Transform(WVector(
				box_v[i][0] ? (float)(over.x + size) : (float)(under.x - size),
				box_v[i][1] ? (float)(over.y + size) : (float)(under.y - size),
				box_v[i][2] ? (float)(over.z + size)
							: (float)(under.z - size)));
	}
	else
	{
		for (i = 0; i < 8; ++i)
		{
			WVector p(box_v[i][0] ? (float)(size + box->info->aabb.max.x)
								  : (float)(box->info->aabb.min.x - size),
				box_v[i][1] ? (float)(size + box->info->aabb.max.y)
							: (float)(box->info->aabb.min.y - size),
				box_v[i][2] ? (float)(size + box->info->aabb.max.z)
							: (float)(box->info->aabb.min.z - size));
			v[i] = p * box->bone->m_matrix;
		}
	}
	for (int f = 0; f < 6; ++f)
	{
		center = WVector::ZERO;
		if (WisEqual(box->info->aabb.min, emptyMin, 0.00001f) != 0 &&
			WisEqual(box->info->aabb.max, emptyMax, 0.00001f) != 0)
		{
			for (i = 0; i < 4; ++i)
				center += v[box_p[f][i]];
		}
		else
		{
			for (i = 0; i < 4; ++i)
			{
				view->DrawLine(v[box_p[f][(i + 1) % 4]], color, v[box_p[f][i]],
					color, 0);
				center += v[box_p[f][i]];
			}
		}
		a = v[box_p[f][0]] - v[box_p[f][1]];
		b = v[box_p[f][1]] - v[box_p[f][2]];
		a.Normalize();
		b.Normalize();
		float dot = a * b;
		if (dot > -0.999f && dot < 0.999f)
		{
			normal = WCrossProduct(a, b);
			normal.Normalize();
			center *= 0.25f;
			view->DrawLine(center, box_color[f],
				center + normal * (15.0f * g_CM_TO_WU), box_color[f], 0);
			view->DrawLine(center + normal * (15.0f * g_CM_TO_WU), box_color[f],
				center + normal * (10.0f * g_CM_TO_WU) +
					a * (3.0f * g_CM_TO_WU),
				box_color[f], 0);
			view->DrawLine(center + normal * (15.0f * g_CM_TO_WU), box_color[f],
				center + normal * (10.0f * g_CM_TO_WU) -
					a * (3.0f * g_CM_TO_WU),
				box_color[f], 0);
		}
	}
}

void WPuppet::ChangeTexture(char* name, int handle)
{
	w_face_animation* anim = m_fanimList.Find(name);
	if (anim == 0)
		return;

	w_mesh** list = anim->mesh;
	for (int i = 0; list[i] != 0; ++i)
	{
		float su = anim->scalex / list[i]->scaleu;
		float sv = anim->scaley / list[i]->scalev;
		float du = anim->tu - su * list[i]->tu;
		float dv = anim->tv - sv * list[i]->tv;
		list[i]->texHandle = handle;
		int tex = list[i]->texHandle;
		list[i]->drawFlag ^= (tex ^ list[i]->drawFlag) & 0x7ff;
		list[i]->tu = anim->tu;
		list[i]->tv = anim->tv;
		list[i]->scaleu = anim->scalex;
		list[i]->scalev = anim->scaley;
		if (list[i]->uv == 0) // 983 00b75c3d
			continue;
		if (Wabs(du - 0.0f) < 0.00001f && Wabs(dv - 0.0f) < 0.00001f &&
			Wabs(su - 1.0f) < 0.00001f && Wabs(sv - 1.0f) < 0.00001f)
			continue;
		for (int v = 0; v < list[i]->vtxNum; ++v)
		{
			w_uv983& uv = (*list[i]->uv)[0][v]; // 983: camada 0
			uv.u = su * uv.u + du;
			uv.v = sv * uv.v + dv;
		}
	}
}

bool WPuppet::ChangeTexturePart(char* name, char* part, Bitmap** bitmap,
	tagRECT* rect)
{
	w_face_animation* anim = m_fanimList.Find(name);
	if (anim == 0)
		return false;
	WResourceManager* mng = GetResrcManager();
	return mng->ChangeTexturePart(anim->filename, part, bitmap, rect);
}

void WPuppet::ChangeTexturePart(char* name, Bitmap* bitmap, const tagRECT& rect)
{
	w_face_animation* anim = m_fanimList.Find(name);
	if (anim != 0)
		GetResrcManager()->ChangeTexturePart(anim->filename, bitmap, rect);
}

bool WPuppet::ChangeTexturePart(int handle, char* name, Bitmap** bitmap,
	tagRECT* rect)
{
	return GetResrcManager()->ChangeTexturePart(handle, name, bitmap, rect);
}

void WPuppet::ChangeTexturePart(int handle, Bitmap* bitmap, const tagRECT& rect)
{
	GetResrcManager()->ChangeTexturePart(handle, bitmap, rect);
}

void WPuppet::ApplyFaceAnimation(char* name, float time)
{
	float su, sv, du, dv;
	if (name != 0)
	{
		w_face_animation* anim = m_fanimList.Find(name);
		if (anim == 0)
			return;
		w_mesh** list = anim->mesh;
		for (int i = 0; list[i] != 0; ++i)
		{
			su = anim->scalex / list[i]->scaleu;
			sv = anim->scaley / list[i]->scalev;
			du = anim->tu - su * list[i]->tu;
			dv = anim->tv - sv * list[i]->tv;
			list[i]->texHandle = anim->texHandle;
			int tex = list[i]->texHandle;
			list[i]->drawFlag ^= (tex ^ list[i]->drawFlag) & 0x7ff;
			list[i]->tu = anim->tu;
			list[i]->tv = anim->tv;
			list[i]->scaleu = anim->scalex;
			list[i]->scalev = anim->scaley;
			if (list[i]->uv == 0) // 983: sem UVs (como em ChangeTexture 00b75c3d)
				continue;
			if (WisEqual(du, 0.0f, 0.00001f) != 0 &&
				WisEqual(dv, 0.0f, 0.00001f) != 0 &&
				WisEqual(su, 1.0f, 0.00001f) != 0 &&
				WisEqual(sv, 1.0f, 0.00001f) != 0)
				continue;
			for (int v = 0; v < list[i]->vtxNum; ++v)
			{
				w_uv983& uv = (*list[i]->uv)[0][v]; // 983: camada 0
				uv.u = su * uv.u + du;
				uv.v = sv * uv.v + dv;
			}
		}
	}
	else
	{
		w_face_animation* anim = m_fanimList.Find("\261\342\272\273");
		if (anim == 0)
			return;
		w_mesh** list = anim->mesh;
		for (int i = 0; list[i] != 0; ++i)
		{
			su = list[i]->originalTu / list[i]->scaleu;
			sv = list[i]->originalTv / list[i]->scalev;
			du = list[i]->originalTu - su * list[i]->tu;
			dv = list[i]->originalTv - sv * list[i]->tv;
			list[i]->texHandle = list[i]->originalTexHandle;
			int tex = list[i]->texHandle;
			list[i]->drawFlag ^= (tex ^ list[i]->drawFlag) & 0x7ff;
			list[i]->tu = list[i]->originalTu;
			list[i]->tv = list[i]->originalTv;
			list[i]->scaleu = list[i]->originalScaleu;
			list[i]->scalev = list[i]->originalScalev;
			if (list[i]->uv == 0) // 983: sem UVs (como em ChangeTexture 00b75c3d)
				continue;
			if (WisEqual(du, 0.0f, 0.00001f) != 0 &&
				WisEqual(dv, 0.0f, 0.00001f) != 0 &&
				WisEqual(su, 1.0f, 0.00001f) != 0 &&
				WisEqual(sv, 1.0f, 0.00001f) != 0)
				continue;
			for (int v = 0; v < list[i]->vtxNum; ++v)
			{
				w_uv983& uv = (*list[i]->uv)[0][v]; // 983: camada 0
				uv.u = su * uv.u + du;
				uv.v = sv * uv.v + dv;
			}
		}
	}
}

int WPuppet::SeekChunkFromPET(cFile* file, char* tag)
{
	int pos = 0;
	while (pos < file->m_nLen)
	{
		file->Seek(pos, 0);
		char head[8];
		file->Read(head, 8);
		if (memcmp(tag, head, 4) == 0)
		{
			file->Seek(pos + 8, 0);
			return 1;
		}
		pos += *(int*)(head + 4) + 8;
	}
	return 0;
}

inline int ReadID(cFile* file);

WBone* WPuppet::LoadPET_Bone(cFile* file, int version)
{
	char name[256];
	WBone* root;
	WBone** bones;
	int count;
	WMatrix mat;
	WMatrix* mats;

	root = 0;
	if (SeekChunkFromPET(file, "BONE") == 0)
		return 0;
	mat.Reset();
	count = file->GetByte();
	if (count == 0)
		file->Read(&count, 2);
	bones = new WBone*[count];
	mats = new WMatrix[count];
	for (int i = 0; i < count; ++i)
	{
		name[0] = (char)file->GetByte();
		if (name[0])
		{
			for (char* p = name; *p; *++p = (char)file->GetByte())
			{
			}
		}
		bones[i] = new WBone;
		bones[i]->SetName(name);
		bones[i]->SetIDNumber(i);
		int parent = ReadID(file);
		if (root != 0)
		{
			if (parent != -1)
				bones[i]->SetParent(root->FindBone(parent));
			else
				root->SetNext(bones[i], true);
		}
		else
		{
			root = bones[i];
		}
		// 983 (00b6efdf): valores de tipo do 983 (o 645 testava 0xff/0x82/0x11b/6, com a escala 0.32 do tipo 6; sem eles a
		// matriz nao era lida e o resto do chunk BONE saia desalinhado -> FindBone nulo no ANIM, F4 c88-3); depois da matriz,
		// a partir do .pet 0xfffe0103, um float do osso (+0xa8)
		if (version == 0x1ff || version == 0x20104 || version == 0x10037)
		{
			file->Read(&mat.xx, 4);
			file->Read(&mat.yx, 4);
			file->Read(&mat.zx, 4);
			file->Read(&mat.xy, 4);
			file->Read(&mat.yy, 4);
			file->Read(&mat.zy, 4);
			file->Read(&mat.xz, 4);
			file->Read(&mat.yz, 4);
			file->Read(&mat.zz, 4);
			file->Read(&mat.xm, 4);
			file->Read(&mat.ym, 4);
			file->Read(&mat.zm, 4);
			if (ComparePetVersion983(m_petVersion983, PET_VERSION_FOVKEYS983) >= 0)
			{
				float value;
				file->Read(&value, 4);
				bones[i]->m_petBoneFloat983 = value;
			}
		}
		bones[i]->SetBasicMatrix(mat);
	}
	delete[] mats;
	delete[] bones;
	for (WBone* previous = root; previous->m_next != 0;
		previous = previous->m_next)
	{
		if (strstr(previous->m_next->GetBoneName(), "Bip") != 0 ||
			strstr(previous->m_next->GetBoneName(), "Bone") != 0 ||
			strstr(previous->m_next->GetBoneName(), "Dummy") != 0)
		{
			WBone* next = previous->m_next;
			previous->SetNext(0, true);
			next->SetNext(root, true);
			root = next;
			break;
		}
	}
	return root;
}

inline int ReadID(cFile* file)
{
	int id = file->GetByte();
	if (id == 0xff)
		id = -1;
	else if (id == 0xfe)
		file->Read(&id, 2);
	return id;
}

bool WPuppet::ChangeMesh(char* name)
{
	char directory[256];

	cFile* file = GetResrcManager()->GetCFile(name, 0xffff);
	if (file != 0)
	{
		GetDirectory(name, directory);
		w_pet_texture_info* texture =
			LoadPET_Texture(file, false, directory[0] != 0 ? directory : 0, 0);
		WBone* bone = LoadPET_Bone(file, 0x1ff); // 983 00b7f1c2 (o 645 passava 0xff)
		bone->Transform(m_mat, 0, 1.0f, 0);
		LoadPET_Mesh(file, bone, texture, false);
		LoadPET_FaceAnim(file, texture, bone, directory);
		if (texture != 0)
			delete[] texture;
		m_rootbone->CombineVtxColor(true);
		CloseCFile(file);
		delete bone;
		SetRenderMode(m_rendMode);
		SetDefaultFaceAnimation();
		return true;
	}
	return false;
}

void WPuppet::SetTexPiece(char* name, int handle, float pu, float pv,
	float scalex, float scaley)
{
	char* part = name;
	if (strchr(name, '/') != 0)
		part = strchr(name, '/') + 1;

	if (m_tex_piece == 0)
	{
		WList<w_tex_piece*>* list = new WList<w_tex_piece*>(0x20, 8);
		m_tex_piece = list;
	}

	w_tex_piece* piece =
		(w_tex_piece*)new char[strlen(part) + sizeof(w_tex_piece)];
	piece->pu = pu;
	piece->pv = pv;
	piece->count = 1;
	piece->scalex = scalex;
	piece->scaley = scaley;
	piece->texHandle = handle;
	strcpy(piece->name, part);
	m_tex_piece->AddItem(piece, piece->name, false);
	m_use_tex_piece += piece;
}

WPuppet::w_tex_piece* WPuppet::GetTexPiece(char* name)
{
	char* part = strchr(name, '/') != 0 ? strchr(name, '/') + 1 : name;
	if (m_tex_piece != 0)
	{
		w_tex_piece* piece = m_tex_piece->Find(part);
		if (piece != 0)
		{
			piece->count++;
			m_use_tex_piece.AddItem(piece, 0, false);
			return piece;
		}
	}
	return 0;
}

void WPuppet::ClearTexPiece(void)
{
	w_tex_piece* piece = m_use_tex_piece.Start();
	w_tex_piece* found;
	for (; piece != 0; piece = m_use_tex_piece.Next())
	{
		if (--piece->count != 0)
			continue;
		if (m_tex_piece != 0)
		{
			*m_tex_piece -= piece;
			for (found = m_tex_piece->Start(); found != 0;
				found = m_tex_piece->Next())
				if (found->texHandle == piece->texHandle)
					break;
		}
		if (found == 0)
			GetResrcManager()->Release(piece->texHandle);
		delete[] (char*)piece;
	}
	m_use_tex_piece.Reset();
}

void WPuppet::LoadPET(cFile* file, bool flag, char* directory, int type)
{
	w_pet_texture_info* texture = 0;
	int count = 0;

	m_iPetType = type;
	ReadVersion983(file);
	if ((type & 1) != 0)
		texture = LoadPET_Texture(file, flag, directory, &count);
	if ((type & 2) != 0)
		LoadPET_ShaderMtl983(file);

	WMatrix mat;
	mat.Reset();
	m_rootbone = LoadPET_Bone(file, type);
	m_rootbone->Transform(mat, 0, 1.0f, 0);
	// 983: bits novos (2 = materiais; o resto subiu um bit) e as matrizes originais sao guardadas aqui
	if (type == 0x1ff || type == 0x20104)
		xBuildTransfMatPtrList();

	if ((type & 8) != 0)
		m_aniLen = LoadPET_Animation(file, m_rootbone);
	if ((type & 0x10) != 0)
		LoadPET_Mesh(file, m_rootbone, texture, type == 0x10037);
	if ((type & 0x20) != 0)
		LoadPET_FaceAnim(file, texture, m_rootbone, directory);
	if ((type & 0x40) != 0)
		LoadPET_Frame(file);
	if ((type & 0x80) != 0)
		m_mdList = LoadPET_Motion(file, 0.0f);
	if ((type & 0x100) != 0)
		LoadPET_Collision(file);
	if ((type & 0x20000) != 0)
		LoadPET_Extra983(file);

	for (int i = 0; i < count; ++i)
	{
		if (texture[i].mapType == 3)
		{
			m_rendMode = 5;
			m_rootbone->ApplySpecularMap(texture[i].texHandle);
			break;
		}
	}
	if (texture != 0)
		delete[] texture;
}

float WPuppet::LoadPET_Animation(cFile* file, WBone* root)
{
	float length = 0.0f;
	if (!SeekChunkFromPET(file, "ANIM"))
		return 0.0f;
	while (true)
	{
		int id = ReadID(file);
		if (id < 0)
			break;
		length = root->FindBone(id)->LoadAnimationKey(file, m_petVersion983); // 983 00b6a050
	}
	return length;
}

WList<w_motion_data*>* WPuppet::LoadPET_Motion(cFile* file, float scale)
{
	char text[4][512];
	int length;
	float option2;
	WList<w_motion_data*>* list;
	int i;
	int count;
	int startFrame;
	float e;
	float s;
	int endFrame;

	if (SeekChunkFromPET(file, "MOTI") == 0)
		return 0;
	list = new WList<w_motion_data*>(8, 8);
	file->Read(&count, 4);
	for (i = 0; i < count; ++i)
	{
		file->Read(&length, 4);
		file->Read(text[0], length);
		text[0][length] = 0;
		file->Read(&startFrame, 4);
		file->Read(&endFrame, 4);
		s = (float)startFrame / 30.0f;
		e = (float)endFrame / 30.0f;
		file->Read(&length, 4);
		file->Read(text[1], length);
		text[1][length] = 0;
		file->Read(&length, 4);
		file->Read(text[2], length);
		text[2][length] = 0;
		int option1 = strcmpi(text[2], "blending") != 0;
		file->Read(&option2, 4);
		file->Read(&length, 4);
		file->Read(text[3], length);
		text[3][length] = 0;
		w_motion_data* data = (w_motion_data*)new char[(int)strlen(text[0]) +
			(int)strlen(text[1]) + (int)strlen(text[3]) + 0x27];
		data->s = s;
		data->end = e;
		data->e = e;
		if (option1 == 2)
			data->e = data->e - option2;
		if (option2 == 1.0f)
			data->end = option2 + data->end;
		data->option1 = option1;
		data->option2 = option2;
		data->name = data->ptr;
		data->nextmotion = data->name + (int)strlen(text[0]) + 1;
		data->applybone = data->nextmotion + (int)strlen(text[1]) + 1;
		strcpy(data->name, text[0]);
		strcpy(data->nextmotion, text[1]);
		strcpy(data->applybone, text[3]);
		list->AddItem(data, data->name, false);
	}
	return list;
}

// 983 00b81020: troca cada *include("arquivo") do texto pelo conteúdo do arquivo (pelo WResourceManager, como o resto do pet).
// Os g_def.bpet do JP usam isso nas caixas dos dedos/orelhas: *include("ring_effect.txt") traz os *fx("Ring_Carry.seq" "&<tid>")
// de cada anel/brinco. Qualquer falha (sem ')', sem aspas, arquivo ausente) devolve o texto ORIGINAL inteiro (00b8135e: assign(src)).
static void ExpandInclude983(std::string& text, WResourceManager* resMng)
{
	static const char tag[] = "*include(";
	std::string::size_type at = text.find(tag);
	if (at == std::string::npos)
		return;
	std::string src(text.c_str());
	text.clear();
	std::string::size_type done = 0;
	while (at != std::string::npos)
	{
		text.append(src, done, at - done);
		std::string::size_type end = src.find(')', at + 9);
		std::string::size_type q1 = src.find('"', at + 9);
		std::string::size_type q2 = q1 == std::string::npos ? q1 : src.find('"', q1 + 1);
		if (end == std::string::npos || q1 == std::string::npos || q1 >= end || q2 == std::string::npos || q2 >= end)
		{
			text.assign(src.c_str());
			return;
		}
		cFile* file = resMng ? resMng->GetCFile(src.substr(q1 + 1, q2 - q1 - 1).c_str(), -1) : 0;
		if (!file)
		{
			text.assign(src.c_str());
			return;
		}
		std::string body;
		body.resize(file->Length() + 1);
		file->Read(&body[0], file->Length());
		body[file->Length()] = 0;
		CloseCFile(file);
		text.append(body.c_str());
		done = end + 1;
		at = src.find(tag, done);
	}
	if (done < src.size())
		text.append(src.c_str() + done);
}

// 983 00b81390: nomes em std::string (256 de partida, crescem com o comprimento lido; o 645 lia em char[512] sem limite)
void WPuppet::LoadPET_Collision(cFile* file)
{
	if (SeekChunkFromPET(file, "COLL") == 0)
		return;
	Waabb aabb;
	aabb.min.z = 0.0f;
	aabb.min.y = 0.0f;
	aabb.min.x = 0.0f;
	aabb.max.z = 0.0f;
	aabb.max.y = 0.0f;
	aabb.max.x = 0.0f;
	std::string name[4];
	for (int n = 0; n < 4; ++n)
		name[n].resize(256);
	int count;
	file->Read(&count, 4);
	for (int i = 0; i < count; ++i)
	{
		unsigned int length;
		float value[6];
		int head[2];
		file->Read(&head[0], 4);
		file->Read(&head[1], 4);
		for (int n = 0; n <= 3; ++n)
		{
			file->Read(&length, 4);
			if (name[n].size() < length)
				name[n].resize(length);
			file->Read(&name[n][0], length);
			if (length < name[n].size())
				name[n][length] = 0;
		}
		for (int k = 0; k < 6; ++k)
			file->Read(&value[k], 4);
		aabb.min.x = value[0];
		aabb.min.y = value[1];
		aabb.min.z = value[2];
		aabb.max.x = value[3];
		aabb.max.y = value[4];
		aabb.max.z = value[5];
		SetBoundBox(const_cast<char*>(name[0].c_str()), &aabb, const_cast<char*>(name[1].c_str()));
		if (strlen(name[2].c_str()) > 1 || strlen(name[3].c_str()) > 1)
		{
			w_bound_box* box = m_bbList.Find(name[0].c_str());
			ExpandInclude983(name[2], GetResrcManager());
			ExpandInclude983(name[3], GetResrcManager());
			box->info->option = new char[strlen(name[2].c_str()) + strlen(name[3].c_str()) + 2];
			sprintf(box->info->option, "%s %s", name[2].c_str(), name[3].c_str());
		}
		if (m_rootbone->FindBone(name[0].c_str(), 0))
			m_rootbone->FindBone(name[1].c_str(), 0)->SetBoundBox(aabb, false);
	}
	w_bound_box* box = m_bbList.Find("bound");
	if (box)
	{
		m_BBox.bone = box->bone;
		m_BBox.info = box->info;
		return;
	}
	box = m_bbList.Find("center");
	if (box)
	{
		m_BBox.bone = box->bone;
		m_BBox.info = box->info;
	}
}

// 983 00b81820: textos em std::string (256 de partida, crescem com o comprimento lido); ate 20000 quadros (645: 10000)
void WPuppet::LoadPET_Frame(cFile* file)
{
	if (SeekChunkFromPET(file, "FRAM") == 0)
		return;
	std::string text[3];
	for (int n = 0; n < 3; ++n)
		text[n].resize(256);
	unsigned int count;
	file->Read(&count, 4);
	if (count - 1 >= 20000)
		return;
	m_framenum = count;
	m_frame = new w_share_pet_data::w_frame[m_framenum];
	for (int i = 0; i < (int)count; ++i)
	{
		unsigned int length;
		int check;
		int head[2];
		file->Read(&head[0], 4);
		int num = head[0];
		for (int k = 0; k < 3; ++k)
		{
			file->Read(&length, 4);
			if (text[k].size() < length)
				text[k].resize(length);
			file->Read(&text[k][0], length);
			if (length < text[k].size())
				text[k][length] = 0;
		}
		if ((int)count < 2 && strcmp(text[0].c_str(), "") == 0)
			break;
		file->Read(&check, 4);
		if (check == 0)
			check = -1;
		m_frame[i].num = num;
		m_frame[i].check = check;
		ExpandInclude983(text[0], GetResrcManager());
		m_frame[i].data = new char[strlen(text[0].c_str()) + 1];
		strcpy(m_frame[i].data, text[0].c_str());
		for (char* found = strstr(m_frame[i].data, "msg"); found != 0;
			found = strstr(found + 3, "msg"))
		{
			for (int c = 0; found[c] != 0; ++c)
			{
				if (found[c] == ')')
				{
					found[c] = ' ';
					break;
				}
			}
		}
	}
}

w_pet_texture_info* WPuppet::LoadPET_Texture(cFile* file, bool alphaTest,
	char* directory, int* textureCount)
{
	int i, j;
	int count = 0;
	if (!SeekChunkFromPET(file, "TEXT"))
		return 0;
	int texNum;
	file->Read(&texNum, sizeof(texNum));
	if (textureCount)
		*textureCount = texNum;
	w_pet_texture* texture = new w_pet_texture[texNum];
	file->Read(texture, texNum * sizeof(w_pet_texture));
	w_pet_texture_info* info = new w_pet_texture_info[texNum];
	Bitmap** bitmap = new Bitmap*[texNum];
	if (!m_storeTexList)
		m_storeTexList = new WList<int>;
	char filename[260];
	for (i = 0; i < texNum; ++i)
	{
		strcpy(info[i].filename, texture[i].filename);
		info[i].group = texture[i].group;
		info[i].bBind = false;
		info[i].mapType = 0;
		info[i].alpha = (unsigned char)(texture[i].diffuse >> 24);
		info[i].diffuse = texture[i].diffuse;
		info[i].xulDrawFlag2 = 0;
		info[i].drawFlags = 0;
		info[i].texHandle = 0;
		info[i].pu = info[i].pv = 0.0f;
		info[i].scalex = info[i].scaley = 1.0f;
		bitmap[i] = 0;
		if (!strcmp(texture[i].filename, "null"))
			continue;
		if (strstr(texture[i].filename, "2#"))
			info[i].xulDrawFlag2 = 0x200;
		for (j = 0; j < 3; ++j)
		{
			switch (texture[i].filename[j])
			{
			case '+':
				if (alphaTest)
					info[i].drawFlags |= 0x80000000;
				else
					info[i].drawFlags |= 0x400000;
				info[i].xulDrawFlag2 |= 0x10;
				break;
			case '[':
				if (alphaTest)
					info[i].drawFlags |= 0x80000000;
				else
					info[i].drawFlags |= 0x400000;
				break;
			case ']':
				info[i].drawFlags |= 0x80000000;
				info[i].xulDrawFlag2 |= 4;
				break;
			case '^':
			case '{':
				info[i].drawFlags |= 0x80000000;
				break;
			case '~':
				info[i].drawFlags |= 0x20800000;
				if (alphaTest)
					info[i].drawFlags |= 0x80000000;
				else
					info[i].drawFlags |= 0x400000;
				if (!texture[i].flag)
					info[i].bBind = true;
				break;
			case '}':
				info[i].drawFlags |= 0x21800000;
				if (alphaTest)
					info[i].drawFlags |= 0x80000000;
				else
					info[i].drawFlags |= 0x400000;
				break;
			case '@':
				info[i].mapType = 1;
				break;
			case '!':
				info[i].mapType = 3;
				break;
			default:
				if (j == 0 && !texture[i].flag)
					info[i].bBind = true;
				break;
			}
		}
		strcpy(filename, texture[i].filename);
		if (strstr(filename, ".dds") || info[i].mapType == 3)
			info[i].bBind = false;
		if (texNum > 1 && info[i].bBind == true &&
			!GetResrcManager()->CheckTexture(filename))
		{
			w_tex_piece* piece = GetTexPiece(texture[i].filename);
			if (piece)
			{
				info[i].pu = piece->pu;
				info[i].pv = piece->pv;
				info[i].scalex = piece->scalex;
				info[i].scaley = piece->scaley;
				info[i].texHandle = piece->texHandle;
				info[i].bBind = false;
				bitmap[i] = 0;
			}
			else
			{
				bitmap[i] = GetResrcManager()->LoadBitmap(filename, 1, false);
				if (!bitmap[i])
					bitmap[i] = new Bitmap(32, 32, 8);
				if (bitmap[i]->bi->bmiHeader.biWidth > 256 ||
					bitmap[i]->bi->bmiHeader.biHeight > 256 ||
					(bitmap[i]->bi->bmiHeader.biWidth == 256 &&
						bitmap[i]->bi->bmiHeader.biHeight == 256))
				{
					if (strcmp(texture[i].filename, "null"))
					{
						bitmap[i]->Update();
						info[i].texHandle = GetResrcManager()->UploadTexture(
							filename, bitmap[i], 0, 0);
						delete bitmap[i];
						*m_storeTexList += info[i].texHandle;
						bitmap[i] = 0;
					}
				}
				else
					++count;
			}
		}
		else
		{
			bitmap[i] = 0;
			info[i].texHandle =
				GetResrcManager()->LoadTexture(filename, 0, 0, 0);
			*m_storeTexList += info[i].texHandle;
		}
	}
	WRectManager page[8];
	Bitmap surface;
	int* bindIndex = new int[texNum];
	static int fixed_wh[8][2] = {
		{ 256, 256 },
        { 256, 128 },
        { 128, 128 },
        { 128, 64  },
        { 64,  128 },
		{ 64,  64  },
        { 32,  64  },
        { 32,  32  }
	};
	while (count > 0)
	{
		for (i = 0; i < 8; ++i)
			page[i].Clear(fixed_wh[i][0], fixed_wh[i][1]);
		i = 0;
		int max = 8;
		int level = 0;
		int x, y;
		for (; i < texNum; ++i)
		{
			if (bitmap[i] &&
				!page[0].Alloc(bitmap[i]->bi->bmiHeader.biWidth,
					bitmap[i]->bi->bmiHeader.biHeight, x, y))
			{
				bindIndex[i] = 1;
				level = 0;
				for (j = 1; j < max; ++j)
				{
					if (page[j].Alloc(bitmap[i]->bi->bmiHeader.biWidth,
							bitmap[i]->bi->bmiHeader.biHeight, x, y))
					{
						max = level + 1;
						break;
					}
					++level;
				}
			}
			else
				bindIndex[i] = 0;
		}
		page[level].Clear(fixed_wh[level][0], fixed_wh[level][1]);
		surface.Create(fixed_wh[level][0], fixed_wh[level][1], 24);
		int handle =
			GetResrcManager()->UploadTexture(0, &surface, 0x10000000, 0);
		for (i = 0; i < texNum; ++i)
		{
			if (bindIndex[i])
			{
				bitmap[i]->Update();
				page[level].Alloc(bitmap[i]->bi->bmiHeader.biWidth,
					bitmap[i]->bi->bmiHeader.biHeight, x, y);
				info[i].pu = (float)x / (float)surface.bi->bmiHeader.biWidth;
				info[i].pv = (float)y / (float)surface.bi->bmiHeader.biHeight;
				info[i].scalex = (float)bitmap[i]->bi->bmiHeader.biWidth /
					(float)surface.bi->bmiHeader.biWidth;
				info[i].scaley = (float)bitmap[i]->bi->bmiHeader.biHeight /
					(float)surface.bi->bmiHeader.biHeight;
				info[i].texHandle = handle;
				bitmap[i]->Update();
				for (j = 0; j < (int)(bitmap[i]->bi->bmiHeader.biWidth *
									bitmap[i]->bi->bmiHeader.biHeight);
					++j)
				{
					unsigned char r, g, b;
					int px = j % bitmap[i]->bi->bmiHeader.biWidth;
					int py = j / bitmap[i]->bi->bmiHeader.biWidth;
					bitmap[i]->GetPixel(px, py, r, g, b);
					surface.vram[(y + py) * surface.pitch + (x + px) * 3] = b;
					surface.vram[(y + py) * surface.pitch + (x + px) * 3 + 1] =
						g;
					surface.vram[(y + py) * surface.pitch + (x + px) * 3 + 2] =
						r;
				}
				--count;
				delete bitmap[i];
				bitmap[i] = 0;
				SetTexPiece(texture[i].filename, info[i].texHandle, info[i].pu,
					info[i].pv, info[i].scalex, info[i].scaley);
			}
		}
		GetResrcManager()->FixTexture(handle, &surface, 0, 0);
	}
	delete[] bindIndex;
	delete[] bitmap;
	delete[] texture;
	return info;
}

void WPuppet::LoadPET_FaceAnim(cFile* file, w_pet_texture_info* texture,
	WBone* bone, char* name)
{
	w_pet_face_animation record;
	int i;
	w_face_animation* anim;
	int count;

	if (SeekChunkFromPET(file, "FANM") == 0)
		return;
	for (anim = m_fanimList.Start(); anim != 0; anim = m_fanimList.Next())
	{
		delete[] anim->mesh;
		delete anim;
	}
	m_fanimList.Reset();
	file->Read(&count, 4);
	for (i = 0; i < count; ++i)
	{
		file->Read(&record, 65);
		int n = 0;
		while (strcmpi(texture[n].filename, record.fName) != 0)
			++n;
		anim = new w_face_animation;
		strcpy(anim->name, record.animName);
		strcpy(anim->filename, record.fName);
		anim->texHandle = texture[n].texHandle;
		anim->tu = texture[n].pu;
		anim->tv = texture[n].pv;
		anim->scalex = texture[n].scalex;
		anim->scaley = texture[n].scaley;
		anim->mesh = 0;
		anim->group = record.group;
		anim->meshNum = m_rootbone->GetMeshForFaceAnimation(
			(unsigned char)anim->group, 0, 0);
		anim->mesh = new w_mesh*[anim->meshNum + 1];
		m_rootbone->GetMeshForFaceAnimation((unsigned char)anim->group,
			anim->mesh, 0);
		anim->mesh[anim->meshNum] = 0;
		m_fanimList.AddItem(anim, anim->name, false);
	}
}

// 983 00b6a3d0: compara versoes do .pet pelos 2 bytes de baixo (-1, 0, 1). Nome meu.
int __fastcall ComparePetVersion983(unsigned long a, unsigned long b)
{
	if (a == b)
		return 0;
	unsigned int ha = (a >> 8) & 0xff;
	unsigned int hb = (b >> 8) & 0xff;
	if (ha != hb)
		return ha < hb ? -1 : 1;
	return (a & 0xff) < (b & 0xff) ? -1 : 1;
}

// 983 00b6a0f0
void WPuppet::ReadVersion983(cFile* file)
{
	m_petVersion983 = PET_VERSION_DEFAULT983;
	if (SeekChunkFromPET(file, "VERS"))
		file->Read(&m_petVersion983, 4);
}

// 983 00b6a090: chunk "EXTR": ossos (por id) marcados com o bit 0x100000
void WPuppet::LoadPET_Extra983(cFile* file)
{
	if (SeekChunkFromPET(file, "EXTR") == 0)
		return;
	unsigned int count = 0;
	file->Read(&count, 4);
	for (unsigned int i = 0; i < count; ++i)
		m_rootbone->FindBone(ReadID(file))->m_flag.Enable(0x100000);
}

// 983 00b69610: matriz de um parametro de material (16 floats)
static inline int ReadFloat4_983(cFile* file, float* v)
{
	int n = file->Read(&v[0], 4);
	n += file->Read(&v[1], 4);
	n += file->Read(&v[2], 4);
	n += file->Read(&v[3], 4);
	return n;
}

int cFile::ReadD3DXMatrix983(D3DXMATRIX& mat)
{
	return ReadFloat4_983(this, mat.m[0]) + ReadFloat4_983(this, mat.m[1]) + ReadFloat4_983(this, mat.m[2]) +
		   ReadFloat4_983(this, mat.m[3]);
}

// 983 00b7ef90: texto do .pet (tamanho de 4 bytes + caracteres); devolve os bytes lidos. Nome meu.
int ReadPetString983(std::string& str, cFile* file)
{
	unsigned int len;
	int n = file->Read(&len, 4);
	str.resize(len);
	if (len)
		return n + file->Read(&str[0], len);
	return n;
}

// 983 00b7f000: nome e tipo de um parametro de material (a semantica fica 9). Nome meu.
void ReadShaderMtlParamDesc983(WxShaderMtlParamDesc& desc, cFile* file)
{
	ReadPetString983(desc.name, file);
	file->Read(&desc.type, 4);
}

// 983 00b7f020: registra o efeito do material no video (pelo nome; se ainda nao existe, le o arquivo .fx). Nome meu.
bool RegisterShaderMtl983(WxShaderMtl* mtl, WResourceManager* resMng)
{
	WVideoDev* video = resMng->VideoReference();
	int id = video->GetShaderId(mtl->m_name.c_str());
	if (id == 0)
	{
		cFile* file = resMng->GetCFile(mtl->m_name.c_str(), -1);
		if (file)
		{
			std::string text;
			text.resize(file->Length() + 1);
			file->Read(&text[0], file->Length());
			text[file->Length()] = 0;
			id = video->RegisterShader(mtl->m_name.c_str(), text);
			CloseCFile(file);
		}
	}
	mtl->m_shaderId = id;
	return id != 0;
}

// 983 00b8ef50: um material do chunk "SMTL": nome do efeito e os parametros por nome (tipo 7, matriz em lista, nao
// tem leitor). Nome meu.
void LoadShaderMtl983(WxShaderMtl* mtl, cFile* file, WResourceManager* resMng)
{
	ReadPetString983(mtl->m_name, file);
	WxShaderMtlParamDesc desc;
	unsigned int count;
	file->Read(&count, 4);
	for (unsigned int i = 0; i < count; ++i)
	{
		desc.Clear();
		ReadShaderMtlParamDesc983(desc, file);
		switch (desc.type)
		{
		case 0:
		{
			int value;
			file->Read(&value, 4);
			mtl->m_boolByName[desc.name] = WxShaderMtlParam<int>(desc, value != 0);
			break;
		}
		case 1:
		{
			int value;
			file->Read(&value, 4);
			mtl->m_intByName[desc.name] = WxShaderMtlParam<int>(desc, value);
			break;
		}
		case 2:
		{
			float value;
			file->Read(&value, 4);
			mtl->m_floatByName[desc.name] = WxShaderMtlParam<float>(desc, value);
			break;
		}
		case 3:
		{
			float x, y;
			file->Read(&x, 4);
			file->Read(&y, 4);
			mtl->m_vector2ByName[desc.name] = WxShaderMtlParam<D3DXVECTOR2>(desc, D3DXVECTOR2(x, y));
			break;
		}
		case 4:
		{
			float x, y, z;
			file->Read(&x, 4);
			file->Read(&y, 4);
			file->Read(&z, 4);
			mtl->m_vector3ByName[desc.name] = WxShaderMtlParam<D3DXVECTOR3>(desc, D3DXVECTOR3(x, y, z));
			break;
		}
		case 5:
		{
			float x, y, z, w;
			file->Read(&x, 4);
			file->Read(&y, 4);
			file->Read(&z, 4);
			file->Read(&w, 4);
			mtl->m_vector4ByName[desc.name] = WxShaderMtlParam<D3DXVECTOR4>(desc, D3DXVECTOR4(x, y, z, w));
			break;
		}
		case 6:
		{
			D3DXMATRIX value;
			file->ReadD3DXMatrix983(value);
			mtl->m_matrixByName[desc.name] = WxShaderMtlParam<D3DXMATRIX>(desc, value);
			break;
		}
		case 8:
		{
			std::string name;
			ReadPetString983(name, file);
			int hTex = resMng->LoadTexture(name.c_str(), 0, 0, 0);
			mtl->m_textures[desc.name] = WxShaderMtl::sTexture(desc, hTex, name);
			break;
		}
		}
	}
}

// 983 00b90200: materiais de shader do .pet; o que nao registra fica 0 (a face com essa camada fica sem material)
void WPuppet::LoadPET_ShaderMtl983(cFile* file)
{
	if (!SeekChunkFromPET(file, "SMTL"))
		return;
	WVideoDev* video = GetResrcManager()->VideoReference();
	if (!video->IsSupportVs() || !video->IsSupportPs())
		return;
	unsigned int count;
	file->Read(&count, 4);
	m_shaderMtls983.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		WxShaderMtl* mtl = new WxShaderMtl;
		LoadShaderMtl983(mtl, file, GetResrcManager());
		bool bReg = RegisterShaderMtl983(mtl, GetResrcManager());
		if (bReg)
		{
			mtl->RandomizeTime();
			mtl->InitBasicParams();
		}
		else
		{
			delete mtl;
			mtl = 0;
		}
		m_shaderMtls983[i] = mtl;
	}
}

// 983 00b91470: cada malha do osso com material ganha uma copia propria, com a textura hOrgTex trocada por hTex
void WPuppet::CloneShaderMtls983(WBone* bone, int hTex, int hOrgTex)
{
	for (w_mesh* mesh = bone->m_mesh; mesh; mesh = mesh->next)
	{
		WxShaderMtl* src = mesh->xpShaderMtl983;
		if (src)
		{
			mesh->xpShaderMtl983 = new WxShaderMtl;
			*mesh->xpShaderMtl983 = *src;
			mesh->xpShaderMtl983->RestoreTextures();
			mesh->xpShaderMtl983->ReplaceTexture(hTex, hOrgTex);
			mesh->xpShaderMtl983->RandomizeTime();
			m_shaderMtls983.push_back(mesh->xpShaderMtl983);
		}
	}
}

// 983 00b91750
void WPuppet::CopyShaderMtls983(const std::vector<WxShaderMtl*>& source)
{
	m_shaderMtls983.resize(source.size());
	for (unsigned int i = 0; i < source.size(); ++i)
	{
		if (source[i] == 0)
			m_shaderMtls983[i] = 0;
		else
		{
			m_shaderMtls983[i] = new WxShaderMtl;
			*m_shaderMtls983[i] = *source[i];
			m_shaderMtls983[i]->RestoreTextures();
			m_shaderMtls983[i]->RandomizeTime();
		}
	}
	std::pair<const std::vector<WxShaderMtl*>*, std::vector<WxShaderMtl*>*> lists(&source, &m_shaderMtls983);
	ForEachBone983(this, RemapShaderMtls983, &lists);
}

// 983 00b6ed80 (BoneCallback983): o material da malha passa a ser a copia do mesmo indice
void __fastcall WPuppet::RemapShaderMtls983(WPuppet* pet, WBone* bone, void* param)
{
	std::pair<const std::vector<WxShaderMtl*>*, std::vector<WxShaderMtl*>*>* lists =
		(std::pair<const std::vector<WxShaderMtl*>*, std::vector<WxShaderMtl*>*>*)param;
	for (w_mesh* mesh = bone->m_mesh; mesh; mesh = mesh->next)
	{
		if (mesh->xpShaderMtl983 != 0)
		{
			for (unsigned int i = 0; i < lists->first->size(); ++i)
			{
				if (mesh->xpShaderMtl983 == (*lists->first)[i])
				{
					mesh->xpShaderMtl983 = (*lists->second)[i];
					break;
				}
			}
		}
	}
}

// 983 00b81ca0
WBone* WPuppet::CreateMissingBone983(WBone* bone, WPuppet* owner)
{
	std::list<WBone*> created;
	WBone* first = 0;
	WBone* prev = 0;
	WBone* source = m_pOrigin983->FindBone(bone->m_name);
	w_common_bone_data data;
	for (;;)
	{
		source->GetCommonData983(&data);
		WBone* copy = new WBone;
		copy->CopyBone(&data);
		copy->m_flag.Turn(0x100000, source->IsExtraBone983());
		created.push_back(copy);
		if (first == 0)
			first = copy;
		if (prev != 0)
			prev->SetParent(copy);
		source = source->m_parent;
		WBone* parent = FindBone(source->m_name);
		if (parent != 0)
		{
			copy->SetParent(parent);
			break;
		}
		prev = copy;
	}
	RegisterCreatedBones983(created);
	return first;
}

// 983 00b81ba0
void WPuppet::RegisterCreatedBones983(std::list<WBone*>& bones)
{
	for (std::list<WBone*>::iterator it = bones.begin(); it != bones.end(); ++it)
	{
		WBone* bone = *it;
		m_boneMatrices983[bone->m_id] = &bone->m_matrix;
		for (w_bound_box* box = m_pOrigin983->m_bbList.Start(); box != 0; box = m_pOrigin983->m_bbList.Next())
		{
			if (strcmp(box->bone->m_name, bone->m_name) == 0)
				AddBoundBox983(box);
		}
	}
}

// 983 00b74010
void WPuppet::AddBoundBox983(w_bound_box* box)
{
	w_bound_box* copy = new w_bound_box;
	copy->info = box->info;
	copy->bone = m_rootbone->FindBone(box->bone->m_name, 0);
	m_bbList.AddItem(copy, copy->info->name, false);
}

// 983 00bcfe70: um ponto do triangulo; .pet antigo = 1 camada de UV
void w_pet_tri_point::Read983(cFile* file, unsigned long version)
{
	file->Read(&pos, 4);
	file->Read(&nx, 4);
	file->Read(&ny, 4);
	file->Read(&nz, 4);
	if (ComparePetVersion983(version, PET_VERSION_UVLAYERS983) < 0)
		uvNum983 = 1;
	else
		file->Read(&uvNum983, 1);
	for (unsigned int i = 0; i < uvNum983; ++i)
		file->Read(uv983[i], 8);
}

void WPuppet::LoadPET_Mesh(cFile* file, WBone* root,
	w_pet_texture_info* texture, bool meshPet)
{
	WVector position, normal, va, vb, vc;
	int i;
	if (!SeekChunkFromPET(file, "MESH"))
		return;
	w_skin_info* skins = 0;
	int skinCount;
	if (meshPet)
	{
		skinCount = file->GetByte();
		skins = new w_skin_info[skinCount];
		file->Read(skins, skinCount * sizeof(w_skin_info));
	}
	int vertexCount;
	file->Read(&vertexCount, sizeof(vertexCount));
	w_pet_vertex* vertices = new w_pet_vertex[vertexCount];
	for (i = 0; i < vertexCount; ++i)
	{
		file->Read(&position, sizeof(position));
		if (meshPet)
			file->Read(&vertices[i].weight, sizeof(float));
		else
			vertices[i].weight = 1.0f;
		short blend[16][2];
		int total = 0;
		int count = 0;
		do
		{
			blend[count][0] = file->GetByte();
			int id = ReadID(file);
			blend[count][1] = id;
			total += blend[count][0];
			++count;
		} while (total < 255);
		normal = root->FindBone(blend[0][1])->Transform(position);
		vertices[i].x = normal.x;
		vertices[i].y = normal.y;
		vertices[i].z = normal.z;
		vertices[i].blendWeight = (unsigned char)blend[0][0];
		vertices[i].bone983 = root->FindBone(blend[0][1]);
		vertices[i].list = 0;
		if (count > 1)
		{
			vertices[i].list = new w_pet_vertex::w_blended[count];
			vertices[i].listNum = count;
			for (int j = 0; j < count; ++j)
			{
				WBone* blended = root->FindBone(blend[j][1]);
				vertices[i].list[j].blendWeight = (unsigned char)blend[j][0];
				vertices[i].list[j].bone =
					m_rootbone->FindBone(blended->GetBoneName(), 0);
			}
		}
		else
		{
			file->GetByte();
			file->GetByte();
		}
	}
	int triangleCount;
	file->Read(&triangleCount, sizeof(triangleCount));
	w_pet_tri_point* triangles = new w_pet_tri_point[triangleCount * 3];
	w_pet_face983* textures = new w_pet_face983[triangleCount];
	for (i = 0; i < triangleCount; ++i)
	{
		textures[i].tex = 0;
		textures[i].layer = -1;
		textures[i].layerMtl = 0;
	}
	for (i = 0; i < triangleCount * 3; ++i)
		triangles[i].Read983(file, m_petVersion983);
	for (i = 0; i < triangleCount; ++i)
		textures[i].tex = &texture[file->GetByte()];
	if (ComparePetVersion983(m_petVersion983, PET_VERSION_FACELAYER983) >= 0)
	{
		// 983: camada de material por face (0xff = nenhuma)
		WVideoDev* video = GetResrcManager()->VideoReference();
		for (i = 0; i < triangleCount; ++i)
		{
			unsigned char layer = (unsigned char)file->GetByte();
			textures[i].layer = layer == 0xff ? -1 : layer;
		}
		if (video->IsSupportVs() && video->IsSupportPs())
		{
			// 983: material de shader da camada (sem conferir o tamanho, como no 983)
			for (i = 0; i < triangleCount; ++i)
			{
				if (textures[i].layer >= 0)
					textures[i].layerMtl = m_shaderMtls983[textures[i].layer];
			}
		}
	}
	m_faceNum = triangleCount;
	WBone** bones;
	if (!meshPet)
	{
		// 983: .pet 0xfffe0101+ ja traz as normais no espaco do modelo (sem o osso falso nem a rotacao)
		bool newPet = ComparePetVersion983(m_petVersion983, PET_VERSION_MODELNORMALS983) >= 0;
		WBone* fake = 0;
		if (!newPet)
			fake = root->FindFakeBodyBone(vertices, vertexCount);
		for (i = 0; i < triangleCount * 3; ++i)
		{
			if (Wabs(triangles[i].nx) < g_EPSILON &&
				Wabs(triangles[i].ny) < g_EPSILON &&
				Wabs(triangles[i].nz) < g_EPSILON)
			{
				int j = i / 3 * 3;
				WBone* bone = vertices[triangles[j].pos].bone983;
				va = bone->Transform(WVector(vertices[triangles[j].pos].x,
					vertices[triangles[j].pos].y,
					vertices[triangles[j].pos].z));
				vb = bone->Transform(WVector(vertices[triangles[j + 1].pos].x,
					vertices[triangles[j + 1].pos].y,
					vertices[triangles[j + 1].pos].z));
				vc = bone->Transform(WVector(vertices[triangles[j + 2].pos].x,
					vertices[triangles[j + 2].pos].y,
					vertices[triangles[j + 2].pos].z));
				normal = WCrossProduct(va - vb, vc - vb);
			}
			else if (newPet)
			{
				normal = WVector(triangles[i].nx, triangles[i].ny, triangles[i].nz);
			}
			else
			{
				WBone* bone = vertices[triangles[i].pos].bone983;
				if (fake &&
					(strstr(bone->m_name, "Bip") ||
						strstr(bone->m_name, "Bone") ||
						strstr(bone->m_name, "Jaw")))
					normal = RotVec(WVector(triangles[i].nx, triangles[i].ny,
										triangles[i].nz),
						fake->GetMatrix());
				else
					normal = RotVec(WVector(triangles[i].nx, triangles[i].ny,
										triangles[i].nz),
						bone->GetMatrix());
			}
			normal.Normalize();
			triangles[i].nx = normal.x;
			triangles[i].ny = normal.y;
			triangles[i].nz = normal.z;
		}
		bones = new WBone*[triangleCount * 3];
		for (i = 0; i < triangleCount * 3; ++i)
			bones[i] = vertices[triangles[i].pos].bone983; // 983: o osso lido, sem procurar pelo nome
		m_rootbone->SetMesh(vertices, vertexCount, triangles, textures, bones,
			triangleCount, this);
	}
	else
	{
		int end;
		for (i = 0; i < skinCount; ++i)
		{
			end = (skins[i].iSTriIdx + skins[i].nTris) * 3;
			for (int j = skins[i].iSTriIdx * 3; j < end; ++j)
			{
				if (Wabs(triangles[j].nx) < g_EPSILON &&
					Wabs(triangles[j].ny) < g_EPSILON &&
					Wabs(triangles[j].nz) < g_EPSILON)
				{
					int k = j / 3 * 3;
					w_pet_vertex* a =
						&vertices[triangles[k].pos + skins[i].iSVtxIdx];
					w_pet_vertex* b =
						&vertices[triangles[k + 1].pos + skins[i].iSVtxIdx];
					w_pet_vertex* c =
						&vertices[triangles[k + 2].pos + skins[i].iSVtxIdx];
					WBone* bone = a->bone983;
					va = bone->Transform(WVector(a->x, a->y, a->z));
					vb = bone->Transform(WVector(b->x, b->y, b->z));
					vc = bone->Transform(WVector(c->x, c->y, c->z));
					normal = WCrossProduct(va - vb, vc - vb);
					normal.Normalize();
				}
				else
					normal = WVector(triangles[j].nx, triangles[j].ny,
						triangles[j].nz);
				triangles[j].nx = normal.x;
				triangles[j].ny = normal.y;
				triangles[j].nz = normal.z;
			}
		}
		bones = new WBone*[triangleCount * 3];
		for (i = 0; i < skinCount; ++i)
		{
			end = (skins[i].iSTriIdx + skins[i].nTris) * 3;
			int j = skins[i].iSTriIdx * 3;
			if (j < end)
			{
				do
				{
					bones[j] = vertices[triangles[j].pos + skins[i].iSVtxIdx].bone983;
					++j;
				} while (j < end);
			}
		}
		for (i = 0; i < skinCount; ++i)
		{
			WBone* bone = m_rootbone->FindBone(skins[i].ucBoneID);
			bone->SetBoneMesh(vertices + skins[i].iSVtxIdx, skins[i].nVtxs,
				triangles + skins[i].iSTriIdx * 3, textures + skins[i].iSTriIdx,
				bones + skins[i].iSTriIdx * 3, skins[i].nTris, this);
		}
	}
	for (i = 0; i < vertexCount; ++i)
		if (vertices[i].list)
			delete[] vertices[i].list;
	delete[] bones;
	delete[] textures;
	delete[] triangles;
	delete[] vertices;
	if (meshPet)
		delete[] skins;
	// 983 (00b7cf52): as paletas das malhas recem-lidas recebem as matrizes do boneco (sem isso orig/transf ficam vazias e o
	// WxTnLBuffer le ponteiro nulo em WBonePalette983::GetMatrix: F4 c88-6)
	if (!meshPet)
		BindPalettes983(0);
}

void WPuppet::GetDirectory(char* path, char* directory)
{
	char* src = path;
	char* dst = directory;
	char value;
	do
	{
		value = *src;
		*dst = value;
		++src;
		++dst;
	} while (value != 0);
	char* slash = strrchr(directory, '/');
	if (slash)
		*slash = 0;
	else
		*directory = 0;
}

void WPuppet::GetName(char* path, char* name)
{
	char* slash = strrchr(path, '/');
	strcpy(name, slash != 0 ? slash + 1 : path); // 983 00b6a180
}

void WPuppet::MergeRootMotion(WList<w_motion_data*>* list, WBone* bone)
{
	for (w_motion_data* data = list->Start(); data != 0; data = list->Next())
	{
		for (WBone* b = bone; b != 0; b = b->m_next)
		{
			if (data->applybone[0] == 0)
				break;
			if (strcmp(b->m_name, data->applybone) == 0)
				data->applybone[0] = 0;
		}
	}
}

int WPuppet::LoadPET(char* name, bool flag, int type)
{
	char hint[260];
	char directory[256];
	char petname[256];

	cFile* file = GetResrcManager()->GetCFile(name, -1);
	if (file == 0)
		return 1;

	GetDirectory(name, directory);
	GetName(name, petname);
	strcpy(m_petName, petname);

	LoadPET(file, flag, directory[0] != 0 ? directory : 0, type);
	CloseCFile(file);

	if (m_mdList != 0)
		MergeRootMotion(m_mdList, m_rootbone);
	m_rootbone->CombineVtxColor(true);
	SetRenderMode(m_rendMode);
	SetDefaultFaceAnimation();
	MakeTemporaryCenterBox();

	// 983: as matrizes originais ja foram guardadas no LoadPET(cFile*); sem a conferencia de ossos rigidos do 645
	SetMatrix(WMatrix::IDENTITY);
	Transform(0, 1.0f, 0);

	sprintf(hint, "WPuppet:%s", name);
	SetLeakHint(hint);
	return 0;
}

void WPuppet::FixNormal(void)
{
	WMatrix matrix;
	matrix.Reset();
	m_rootbone->FixNormal(matrix);
}

// 983 00b81b30
void WPuppet::CollectBoneMatrices983(WBone* bone, std::map<int, const WMatrix*>& list)
{
	do
	{
		if ((((ulong)bone->m_flag) & 0x10000) == 0)
		{
			if (list.find(bone->m_id) == list.end())
				list[bone->m_id] = &bone->m_matrix;
		}
		if (bone->m_child != 0)
			CollectBoneMatrices983(bone->m_child, list);
		bone = bone->m_next;
	} while (bone != 0);
}

// 983 00b91d10
void WPuppet::xBuildTransfMatPtrList(void)
{
	m_boneMatrices983.clear();
	CollectBoneMatrices983(m_rootbone, m_boneMatrices983);
	if (!ms_xOrginalMatrixList.Has(m_petName))
		ms_xOrginalMatrixList.Add(m_petName, m_boneMatrices983);
}

// 983 00b91e30: as matrizes originais do outro boneco ficam pelos ids dos ossos deste (ou do de origem); cada osso de
// malha anexado ganha materiais proprios e paletas deste boneco
void WPuppet::AttachMeshBone(WPuppet* other, int handle, int oldHandle)
{
	if (!ms_xOrginalMatrixList.Has(other->m_petName))
	{
		std::map<int, const WMatrix*> list;
		std::map<int, const WMatrix*> mine;
		other->CollectBoneMatrices983(other->m_rootbone, list);
		for (std::map<int, const WMatrix*>::iterator it = list.begin(); it != list.end(); ++it)
		{
			WBone* bone = other->FindBone(it->first);
			if (bone != 0)
			{
				WBone* found = FindBone(bone->m_name);
				if (found == 0)
					found = m_pOrigin983->FindBone(bone->m_name);
				if (found != 0)
					mine[found->m_id] = it->second;
			}
		}
		ms_xOrginalMatrixList.Add(other->m_petName, mine);
	}

	WBone* tail;
	for (tail = m_rootbone; tail->m_next != 0; tail = tail->m_next)
	{
	}

	WBone* bone = 0;
	while ((bone = other->FindMeshBone(bone)) != 0)
	{
		tail = tail->AttachMeshBone(bone, handle, oldHandle);
		CloneShaderMtls983(tail, handle, oldHandle);
		tail->ResetMeshBone(this, other, false);
		tail->CombineVtxColor(false);
		InternPalettes983(this, tail, 0);
		for (w_mesh* mesh = tail->m_mesh; mesh != 0; mesh = mesh->next)
		{
			for (std::list<WBonePalette983>::iterator it = m_palettes983.begin(); it != m_palettes983.end(); ++it)
			{
				if (mesh->palette983 == &*it)
				{
					it->Bind(this, other);
					break;
				}
			}
		}
	}
}

// 983 00b8cb30: o osso anexado leva os materiais das malhas (+0x1fc) e sai das listas do boneco (FreeBone); depois os
// ossos 0x100000 deixam de ter o other como dono
void WPuppet::DetachMeshBone(WPuppet* other)
{
	WBone* bone = 0;
	while ((bone = other->FindMeshBone(bone)) != 0)
	{
		WBone* found;
		WBone* previous;
		FindMeshBone(&found, &previous, bone->m_name);
		if (found != 0)
		{
			if (previous != 0)
				previous->SetNext(found->m_next, false);
			else
				m_rootbone = found->m_next;
			found->SetNext(0, true);
			for (w_mesh* mesh = found->m_mesh; mesh != 0; mesh = mesh->next)
			{
				if (mesh->xpShaderMtl983 != 0)
				{
					std::vector<WxShaderMtl*>::iterator it = std::find(m_shaderMtls983.begin(),
						m_shaderMtls983.end(), mesh->xpShaderMtl983);
					if (it != m_shaderMtls983.end())
						m_shaderMtls983.erase(it);
					delete mesh->xpShaderMtl983;
				}
			}
			FreeBone(found);
			delete found;
		}
	}
	ForEachBone983(this, RemoveOwner983, &other);
}

// 983 00b6a200
void WPuppet::RemoveOwner983(WPuppet* pet, WBone* bone, void* param)
{
	if (bone->IsExtraBone983())
		bone->RemoveOwner983(*(WPuppet**)param);
}

// 983 00b8c970: tira o osso (que vai ser apagado) das paletas, do mapa de matrizes, das caixas e do cache de nomes
void WPuppet::FreeBone(WBone* bone)
{
	if ((((ulong)bone->m_flag) & 0x10000) != 0)
	{
		for (w_mesh* mesh = bone->m_mesh; mesh != 0; mesh = mesh->next)
		{
			for (std::list<WBonePalette983>::iterator it = m_palettes983.begin(); it != m_palettes983.end(); ++it)
			{
				if (mesh->palette983 == &*it)
				{
					m_palettes983.erase(it);
					break;
				}
			}
		}
	}
	if (bone->IsExtraBone983())
		ForgetExtraBone983(bone);
	m_boneList.Remove(bone);
}

// 983 00b7d5b0
void WPuppet::ForgetExtraBone983(WBone* bone)
{
	m_boneMatrices983.erase(bone->m_id);
	for (std::list<WBonePalette983>::iterator it = m_palettes983.begin(); it != m_palettes983.end(); ++it)
		it->Remove(bone);
	for (w_bound_box* box = m_bbList.Start(); box != 0; box = m_bbList.Next())
	{
		if (box->bone == bone)
		{
			delete box;
			m_bbList -= box;
		}
	}
}

// 983 00b8cc60: filhos 0x100000 que nenhum boneco usa mais saem da arvore
void WPuppet::ReleaseUnusedExtraBones983(WPuppet* pet, WBone* bone, void* param)
{
	WBone* child = bone->m_child;
	while (child != 0)
	{
		WBone* next = child->m_next;
		if (child->IsExtraBone983() && child->m_list983_1d0.size() == 0)
		{
			bone->ReleaseChild(child);
			child->SetNext(0, true);
			pet->FreeBone(child);
			delete child;
		}
		child = next;
	}
}

// 983 00b6a430
void WPuppet::ForEachBonePost983(WPuppet* pet, BoneCallback983 func, void* param)
{
	if (pet->m_rootbone)
		WBone::ForEachBonePost983(pet, pet->m_rootbone, func, param);
}

// 983 00b921e0
void WPuppet::ReplaceMeshBones(WPuppet* oldPuppet, WPuppet* newPuppet,
	int handle, int oldHandle)
{
	if (oldPuppet)
		DetachMeshBone(oldPuppet);
	if (newPuppet)
		AttachMeshBone(newPuppet, handle, oldHandle);
	ForEachBonePost983(this, ReleaseUnusedExtraBones983, 0);
}

void WPuppet::ChangeTextureByHandle(int from, int to, WPuppet* other)
{
	if (other == 0)
	{
		m_rootbone->ChangeTextureByHandle(from, to, true, true);
		return;
	}
	WBone* bone = 0;
	while ((bone = other->FindMeshBone(bone)) != 0)
	{
		WBone* found = 0;
		FindMeshBone(&found, 0, bone->m_name);
		if (found != 0)
			found->ChangeTextureByHandle(from, to, false, false);
	}
}

void WPuppet::EndMeshBoneWork(void)
{
	MergeNormal();
}

void WPuppet::MergeNormal(void)
{
	WBone::w_normalmerge merge(*this);
}

void WPuppet::FindMeshBone(WBone** found, WBone** previous, const char* name)
{
	if (found)
		*found = 0;
	if (previous)
		*previous = 0;
	WBone* bone = m_rootbone;
	if (stricmp(bone->m_name, name) == 0)
	{
		*found = bone;
		return;
	}
	WBone* next = bone->m_next;
	while (next != 0)
	{
		if (stricmp(next->m_name, name) == 0)
		{
			*found = bone->m_next;
			if (previous)
				*previous = bone;
			return;
		}
		bone = bone->m_next;
		next = bone->m_next;
	}
}

WBone* WPuppet::FindMeshBone(WBone* previous)
{
	if (previous == 0)
	{
		WBone* first = m_rootbone;
		while (first != 0 && first->m_mesh == 0)
			first = first->m_next;
		return first;
	}
	WBone* bone = m_rootbone;
	while (bone != 0 && bone != previous)
		bone = bone->m_next;
	if (bone != 0)
	{
		bone = bone->m_next;
		while (bone != 0 && bone->m_mesh == 0)
			bone = bone->m_next;
	}
	return bone;
}

w_face_animation* WPuppet::FindFaceAnim(char* name)
{
	return m_fanimList.Find(name);
}

void WPuppet::DisableFog(void)
{
	if (m_rootbone != 0)
		m_rootbone->DisableFog();	// 983 00b6a320: WBone::DisableFog ja desce pela arvore
}

void WPuppet::EnableAlphaBlend(void)
{
}

void WPuppet::ScaleBone(char* name, float scale)
{
	WBone* bone = FindBone(name);
	if (bone != 0)
		bone->SetScale(scale);
}

bool WPuppet::CompareBasicMatrix(WPuppet* other) const
{
	if (m_rootbone && other->m_rootbone)
		return m_rootbone->CompareBasicMatrix(other->m_rootbone);
	return false;
}

WPuppet::w_original_matrix_list983 WPuppet::ms_xOrginalMatrixList; // 983 00f14a18

bool WPuppet::w_original_matrix_list983::xHas(const std::string& name)
{
	return m_list.find(name) != m_list.end();
}

bool WPuppet::w_original_matrix_list983::Has(const std::string& name)
{
	m_lock.Lock();
	bool has = xHas(name);
	m_lock.Unlock();
	return has;
}

void WPuppet::w_original_matrix_list983::Add(const std::string& name, const std::map<int, const WMatrix*>& list)
{
	m_lock.Lock();
	if (!xHas(name))
	{
		tMatrixById& mats = m_list[name];
		for (std::map<int, const WMatrix*>::const_iterator it = list.begin(); it != list.end(); ++it)
			mats.insert(tMatrixById::value_type(it->first, *it->second));
	}
	m_lock.Unlock();
}

// o 983 nao confere se achou (o nome sempre foi guardado antes pelo LoadPET ou pelo AttachMeshBone)
const WPuppet::w_original_matrix_list983::tMatrixById& WPuppet::w_original_matrix_list983::Get(const std::string& name)
{
	m_lock.Lock();
	std::map<std::string, tMatrixById>::iterator it = m_list.find(name);
	m_lock.Unlock();
	return it->second;
}

// 983: 00b6a410
void WPuppet::ForEachBone983(WPuppet* pet, BoneCallback983 func, void* param)
{
	if (pet->m_rootbone)
		WBone::ForEachBone983(pet, pet->m_rootbone, func, param);
}

// ---- 983: paletas de ossos das malhas (w_mesh::palette983) ----

// junta a paleta numa das do boneco que ainda caiba (menos no tipo 0x20104); senao guarda uma copia nova
WBonePalette983* WPuppet::GetPalette983(const WBonePalette983& palette)
{
	if (m_iPetType != 0x20104)
	{
		for (std::list<WBonePalette983>::iterator it = m_palettes983.begin(); it != m_palettes983.end(); ++it)
		{
			if (it->Merge(palette))
				return &*it;
		}
	}
	m_palettes983.push_back(palette);
	return &m_palettes983.back();
}

void WPuppet::BindPalettes983(WPuppet* source)
{
	for (std::list<WBonePalette983>::iterator it = m_palettes983.begin(); it != m_palettes983.end(); ++it)
		it->Bind(this, source);
}

// por osso (ForEachBone983): as malhas passam a usar paletas deste boneco, com os ossos dele
void __fastcall WPuppet::InternPalettes983(WPuppet* pet, WBone* bone, void* param)
{
	for (w_mesh* mesh = bone->m_mesh; mesh; mesh = mesh->next)
	{
		WBonePalette983 palette;
		palette.CopyFrom(*mesh->palette983, pet);
		mesh->palette983 = pet->GetPalette983(palette);
	}
}

// HACK(?): Necessary to preserve emission order
#include "wmemblock.inl"
