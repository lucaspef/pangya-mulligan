// Unidade "wxshadermtl" do 983 (10049310..1005834f; nome de arquivo provisório, F1.3): material de shader
// (WxShaderMtl) — parâmetros por tipo, cada um num mapa por nome e noutro por número de semântica, e as texturas.
// O 645 não tem este código; nomes de classes, métodos e campos são provisórios.
#define WANGREAL_DEVICE
#include <new>
#include <stdlib.h>
#include <string.h>
#include "../wangreal/wd3d8.h"

// 10049310: sem chamador na dll (dono desconhecido; 839 10048e90 também sem nome): só o construtor sobrou —
// grava um bool e dois floats (12 B). Nome descritivo pelo layout (F5, C683).
struct sWxShaderMtlBoolFloat2
{
	bool m_flag;
	float m_value1;
	float m_value2;
	sWxShaderMtlBoolFloat2(bool flag, float value1, float value2);
};

sWxShaderMtlBoolFloat2::sWxShaderMtlBoolFloat2(bool flag, float value1, float value2)
	: m_flag(flag), m_value1(value1), m_value2(value2)
{
}

static const char* s_fxParamTypeNames[] = {
	"TYPE_BOOL", "TYPE_INT", "TYPE_FLOAT", "TYPE_VECTOR2", "TYPE_VECTOR3", "TYPE_VECTOR4", "TYPE_MATRIX",
	"TYPE_MATRIXARRAY", "TYPE_TEXTURE", "TYPE_NUM",
};

const char* WVDFxParamTypeName(int type)
{
	return s_fxParamTypeNames[type];
}

int WVDFxParamTypeFromName(const char* name)
{
	for (unsigned int i = 0; i < 9; i++)
	{
		if (strcmp(name, s_fxParamTypeNames[i]) == 0)
			return i;
	}
	return 9;
}

void WxShaderMtl::RandomizeTime()
{
	m_time = (float)(rand() / 32767.0 * 3600.0);
}

// 1004eea0: bits de variação do shader (WVDSHMACRO_*) a partir do tipo de desenho e do formato do vértice
void WDirect3D8::sShaderFlags983::Set(unsigned long type, unsigned long type2, unsigned long vertexType)
{
	bits.reset();
	switch (vertexType & 0x400e)
	{
	case 6:
		bits.set(1);
		break;
	case 8:
		bits.set(1);
		bits.set(2);
		break;
	case 10:
		bits.set(1);
		bits.set(3);
		break;
	case 12:
		bits.set(1);
		bits.set(4);
		break;
	}
	if (!(type & 0x40000000) && !(type2 & 0x1000))
		bits.set(0);
	if (type2 & 4)
		bits.set(5);
	if (type & 0x10000000)
		bits.set(6);
}

const WxShaderMtlParam<int>* WxShaderMtl::GetBoolParam(int id) const
{
	tIntById::const_iterator it = m_boolById.find(id);
	if (it != m_boolById.end())
		return &it->second;
	return 0;
}

WxShaderMtlParam<int>* WxShaderMtl::GetBoolParam(int id)
{
	tIntById::iterator it = m_boolById.find(id);
	if (it != m_boolById.end())
		return &it->second;
	return 0;
}

const WxShaderMtlParam<int>* WxShaderMtl::GetIntParam(int id) const
{
	tIntById::const_iterator it = m_intById.find(id);
	if (it != m_intById.end())
		return &it->second;
	return 0;
}

WxShaderMtlParam<int>* WxShaderMtl::GetIntParam(int id)
{
	tIntById::iterator it = m_intById.find(id);
	if (it != m_intById.end())
		return &it->second;
	return 0;
}

const WxShaderMtlParam<float>* WxShaderMtl::GetFloatParam(int id) const
{
	tFloatById::const_iterator it = m_floatById.find(id);
	if (it != m_floatById.end())
		return &it->second;
	return 0;
}

WxShaderMtlParam<float>* WxShaderMtl::GetFloatParam(int id)
{
	tFloatById::iterator it = m_floatById.find(id);
	if (it != m_floatById.end())
		return &it->second;
	return 0;
}

const WxShaderMtlParam<D3DXVECTOR2>* WxShaderMtl::GetVector2Param(int id) const
{
	tVector2ById::const_iterator it = m_vector2ById.find(id);
	if (it != m_vector2ById.end())
		return &it->second;
	return 0;
}

WxShaderMtlParam<D3DXVECTOR2>* WxShaderMtl::GetVector2Param(int id)
{
	tVector2ById::iterator it = m_vector2ById.find(id);
	if (it != m_vector2ById.end())
		return &it->second;
	return 0;
}

const WxShaderMtlParam<D3DXVECTOR3>* WxShaderMtl::GetVector3Param(int id) const
{
	tVector3ById::const_iterator it = m_vector3ById.find(id);
	if (it != m_vector3ById.end())
		return &it->second;
	return 0;
}

WxShaderMtlParam<D3DXVECTOR3>* WxShaderMtl::GetVector3Param(int id)
{
	tVector3ById::iterator it = m_vector3ById.find(id);
	if (it != m_vector3ById.end())
		return &it->second;
	return 0;
}

const WxShaderMtlParam<D3DXVECTOR4>* WxShaderMtl::GetVector4Param(int id) const
{
	tVector4ById::const_iterator it = m_vector4ById.find(id);
	if (it != m_vector4ById.end())
		return &it->second;
	return 0;
}

WxShaderMtlParam<D3DXVECTOR4>* WxShaderMtl::GetVector4Param(int id)
{
	tVector4ById::iterator it = m_vector4ById.find(id);
	if (it != m_vector4ById.end())
		return &it->second;
	return 0;
}

const WxShaderMtlParam<D3DXMATRIX>* WxShaderMtl::GetMatrixParam(int id) const
{
	tMatrixById::const_iterator it = m_matrixById.find(id);
	if (it != m_matrixById.end())
		return &it->second;
	return 0;
}

WxShaderMtlParam<D3DXMATRIX>* WxShaderMtl::GetMatrixParam(int id)
{
	tMatrixById::iterator it = m_matrixById.find(id);
	if (it != m_matrixById.end())
		return &it->second;
	return 0;
}

const WxShaderMtl::sTexture* WxShaderMtl::GetTextureParam(int id) const
{
	tTextureById::const_iterator it = m_textureById.find(id);
	if (it != m_textureById.end())
		return &it->second;
	return 0;
}

WxShaderMtl::sTexture* WxShaderMtl::GetTextureParam(int id)
{
	tTextureById::iterator it = m_textureById.find(id);
	if (it != m_textureById.end())
		return &it->second;
	return 0;
}

// 1004fed0: estados básicos (semânticas 0..8) a partir do tipo de desenho do 645
void WxShaderMtl::SetRenderFlags(unsigned long type, unsigned long type2)
{
	WxShaderMtlParam<int>* cull = GetIntParam(0);
	switch (type2 & 0x600)
	{
	case 0:
		cull->SetValue(3);
		break;
	case 0x200:
		cull->SetValue(1);
		break;
	case 0x400:
		cull->SetValue(2);
		break;
	}
	WxShaderMtlParam<int>* blend = GetBoolParam(1);
	WxShaderMtlParam<int>* src = GetIntParam(2);
	WxShaderMtlParam<int>* dest = GetIntParam(3);
	switch (type & 0x41800000)
	{
	case 0:
		blend->SetValue(1);
		src->SetValue(5);
		dest->SetValue(6);
		break;
	case 0x800000:
		blend->SetValue(1);
		src->SetValue(5);
		dest->SetValue(2);
		break;
	case 0x1000000:
	case 0x41000000:
		blend->SetValue(1);
		src->SetValue(1);
		dest->SetValue(3);
		break;
	case 0x1800000:
	case 0x41800000:
		blend->SetValue(1);
		src->SetValue(1);
		dest->SetValue(4);
		break;
	case 0x40000000:
		blend->SetValue(0);
		src->SetValue(2);
		dest->SetValue(1);
		break;
	case 0x40800000:
		blend->SetValue(1);
		src->SetValue(2);
		dest->SetValue(2);
		break;
	}
	GetIntParam(6)->SetValue((type >> 24) & 0x80);
	GetBoolParam(7)->SetValue(!(type & 0x20000000));
}

// 10050040: todas as texturas voltam a ser as do arquivo
void WxShaderMtl::RestoreTextures()
{
	m_flags.reset();
	for (tTextureByName::iterator it = m_textures.begin(); it != m_textures.end(); ++it)
		it->second.SetValue(it->second.m_hOrgTex);
}

// 100500b0: troca a textura em uso de quem tem hOrgTex como textura do arquivo
void WxShaderMtl::ReplaceTexture(int hTex, int hOrgTex)
{
	for (tTextureByName::iterator it = m_textures.begin(); it != m_textures.end(); ++it)
	{
		if (hOrgTex == it->second.m_hOrgTex)
			it->second.m_value = hTex;
	}
}

const WxShaderMtlParam<int>* WxShaderMtl::GetBoolParam(const std::string& name) const
{
	tIntByName::const_iterator it = m_boolByName.find(name);
	if (it != m_boolByName.end())
		return &it->second;
	return 0;
}

WxShaderMtlParam<int>* WxShaderMtl::GetBoolParam(const std::string& name)
{
	tIntByName::iterator it = m_boolByName.find(name);
	if (it != m_boolByName.end())
		return &it->second;
	return 0;
}

const WxShaderMtlParam<int>* WxShaderMtl::GetIntParam(const std::string& name) const
{
	tIntByName::const_iterator it = m_intByName.find(name);
	if (it != m_intByName.end())
		return &it->second;
	return 0;
}

WxShaderMtlParam<int>* WxShaderMtl::GetIntParam(const std::string& name)
{
	tIntByName::iterator it = m_intByName.find(name);
	if (it != m_intByName.end())
		return &it->second;
	return 0;
}

const WxShaderMtlParam<float>* WxShaderMtl::GetFloatParam(const std::string& name) const
{
	tFloatByName::const_iterator it = m_floatByName.find(name);
	if (it != m_floatByName.end())
		return &it->second;
	return 0;
}

WxShaderMtlParam<float>* WxShaderMtl::GetFloatParam(const std::string& name)
{
	tFloatByName::iterator it = m_floatByName.find(name);
	if (it != m_floatByName.end())
		return &it->second;
	return 0;
}

const WxShaderMtlParam<D3DXVECTOR2>* WxShaderMtl::GetVector2Param(const std::string& name) const
{
	tVector2ByName::const_iterator it = m_vector2ByName.find(name);
	if (it != m_vector2ByName.end())
		return &it->second;
	return 0;
}

WxShaderMtlParam<D3DXVECTOR2>* WxShaderMtl::GetVector2Param(const std::string& name)
{
	tVector2ByName::iterator it = m_vector2ByName.find(name);
	if (it != m_vector2ByName.end())
		return &it->second;
	return 0;
}

const WxShaderMtlParam<D3DXVECTOR3>* WxShaderMtl::GetVector3Param(const std::string& name) const
{
	tVector3ByName::const_iterator it = m_vector3ByName.find(name);
	if (it != m_vector3ByName.end())
		return &it->second;
	return 0;
}

WxShaderMtlParam<D3DXVECTOR3>* WxShaderMtl::GetVector3Param(const std::string& name)
{
	tVector3ByName::iterator it = m_vector3ByName.find(name);
	if (it != m_vector3ByName.end())
		return &it->second;
	return 0;
}

const WxShaderMtlParam<D3DXVECTOR4>* WxShaderMtl::GetVector4Param(const std::string& name) const
{
	tVector4ByName::const_iterator it = m_vector4ByName.find(name);
	if (it != m_vector4ByName.end())
		return &it->second;
	return 0;
}

WxShaderMtlParam<D3DXVECTOR4>* WxShaderMtl::GetVector4Param(const std::string& name)
{
	tVector4ByName::iterator it = m_vector4ByName.find(name);
	if (it != m_vector4ByName.end())
		return &it->second;
	return 0;
}

const WxShaderMtlParam<D3DXMATRIX>* WxShaderMtl::GetMatrixParam(const std::string& name) const
{
	tMatrixByName::const_iterator it = m_matrixByName.find(name);
	if (it != m_matrixByName.end())
		return &it->second;
	return 0;
}

WxShaderMtlParam<D3DXMATRIX>* WxShaderMtl::GetMatrixParam(const std::string& name)
{
	tMatrixByName::iterator it = m_matrixByName.find(name);
	if (it != m_matrixByName.end())
		return &it->second;
	return 0;
}

const WxShaderMtl::sTexture* WxShaderMtl::GetTextureParam(const std::string& name) const
{
	tTextureByName::const_iterator it = m_textures.find(name);
	if (it != m_textures.end())
		return &it->second;
	return 0;
}

WxShaderMtl::sTexture* WxShaderMtl::GetTextureParam(const std::string& name)
{
	tTextureByName::iterator it = m_textures.find(name);
	if (it != m_textures.end())
		return &it->second;
	return 0;
}

// 10056370: os 9 estados básicos (WVDSHBASICUSERPARAMSEMANTIC_*) com o valor padrão do Direct3D
void WxShaderMtl::InitBasicParams()
{
	WxShaderMtlParamDesc desc;
	desc.semantic = 0;
	desc.type = 1;
	m_intById[0] = WxShaderMtlParam<int>(desc, 3);
	desc.Clear();
	desc.semantic = 1;
	desc.type = 0;
	m_boolById[1] = WxShaderMtlParam<int>(desc, 0);
	desc.Clear();
	desc.semantic = 2;
	desc.type = 1;
	m_intById[2] = WxShaderMtlParam<int>(desc, 2);
	desc.Clear();
	desc.semantic = 3;
	desc.type = 1;
	m_intById[3] = WxShaderMtlParam<int>(desc, 1);
	desc.Clear();
	desc.semantic = 4;
	desc.type = 0;
	m_boolById[4] = WxShaderMtlParam<int>(desc, 1);
	desc.Clear();
	desc.semantic = 5;
	desc.type = 1;
	m_intById[5] = WxShaderMtlParam<int>(desc, 5);
	desc.Clear();
	desc.semantic = 6;
	desc.type = 1;
	m_intById[6] = WxShaderMtlParam<int>(desc, 0);
	desc.Clear();
	desc.semantic = 7;
	desc.type = 0;
	m_boolById[7] = WxShaderMtlParam<int>(desc, 1);
	desc.Clear();
	desc.semantic = 8;
	desc.type = 1;
	m_intById[8] = WxShaderMtlParam<int>(desc, 4);
}

WxShaderMtl::WxShaderMtl() : m_shaderId(0), m_time(0)
{
}

WxShaderMtl::~WxShaderMtl()
{
}
