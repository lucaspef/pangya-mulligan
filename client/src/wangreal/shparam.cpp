// Unidade "shparam" do 983 (10001000..100124ef; nome de arquivo provisório, F1.3): tabela de shaders registrados
// pelo jogo, um efeito compilado por combinação de bits de variação e os parâmetros que cada efeito pede ao
// material (WxShaderMtl). O 645 não tem este código; nomes de classes, métodos e campos são provisórios.
#define WANGREAL_DEVICE
#include <new>
#include "wd3d8.h"

// 10001280..10001310: tipo do parâmetro do efeito -> tipo da tabela TYPE_* (9 = não usado)
static int xScalarType(const D3DXPARAMETER_DESC& desc)
{
	switch (desc.Type)
	{
	case D3DXPT_BOOL:
		return 0;
	case D3DXPT_INT:
		return 1;
	case D3DXPT_FLOAT:
		return 2;
	}
	return 9;
}

static int xVectorType(const D3DXPARAMETER_DESC& desc)
{
	switch (desc.Type)
	{
	case D3DXPT_FLOAT:
		switch (desc.Columns)
		{
		case 4:
			return 5;
		case 3:
			return 4;
		case 2:
			return 3;
		}
		return 9;
	}
	return 9;
}

static int xMatrixType(const D3DXPARAMETER_DESC& desc)
{
	if (desc.Type != D3DXPT_FLOAT)
		return 9;
	return desc.Elements ? 7 : 6;
}

static int xObjectType(const D3DXPARAMETER_DESC& desc)
{
	return desc.Type == D3DXPT_TEXTURE ? 8 : 9;
}

static int xParamType(const D3DXPARAMETER_DESC& desc)
{
	switch (desc.Class)
	{
	case D3DXPC_SCALAR:
		return xScalarType(desc);
	case D3DXPC_VECTOR:
		return xVectorType(desc);
	case D3DXPC_MATRIX_ROWS:
		return xMatrixType(desc);
	case D3DXPC_OBJECT:
		return xObjectType(desc);
	}
	return 9;
}

void WDirect3D8::sShaderInst983::OnLostDevice()
{
	if (this->pEffect)
		this->pEffect->OnLostDevice();
}

void WDirect3D8::sShaderInst983::OnResetDevice()
{
	if (this->pEffect)
		this->pEffect->OnResetDevice();
}

HRESULT WDirect3D8::sShaderInst983::Begin()
{
	if (!this->pEffect)
		return E_FAIL;
	UINT passes;
	HRESULT hr = this->pEffect->Begin(&passes, D3DXFX_DONOTSAVESTATE |
		D3DXFX_DONOTSAVESHADERSTATE | D3DXFX_DONOTSAVESAMPLERSTATE);
	if (SUCCEEDED(hr))
		hr = this->pEffect->BeginPass(0);
	return hr;
}

HRESULT WDirect3D8::sShaderInst983::Commit()
{
	if (!this->pEffect)
		return E_FAIL;
	return this->pEffect->CommitChanges();
}

HRESULT WDirect3D8::sShaderInst983::End()
{
	if (!this->pEffect)
		return E_FAIL;
	HRESULT hr = this->pEffect->EndPass();
	if (FAILED(hr))
		return hr;
	return this->pEffect->End();
}

bool WDirect3D8::sShaderFlags983::operator<(const sShaderFlags983& rhs) const
{
	for (size_t i = 0; i < 10; ++i)
	{
		if (!this->Test(i) && rhs.Test(i))
			return true;
		if (this->Test(i) && !rhs.Test(i))
			return false;
	}
	return false;
}

WDirect3D8::sShaderInst983* WDirect3D8::sShaderTable983::sEntry::Find(
	const sShaderFlags983& flags) const
{
	std::map<sShaderFlags983, sShaderInst983*>::const_iterator it =
		this->insts.find(flags);
	if (it != this->insts.end())
		return it->second;
	return 0;
}

WDirect3D8::sShaderTable983::sEntry* WDirect3D8::sShaderTable983::FindProgram(
	unsigned int shaderId) const
{
	std::map<unsigned int, sEntry*>::const_iterator it = this->byId.find(shaderId);
	if (it != this->byId.end())
		return it->second;
	return 0;
}

void WDirect3D8::sShaderTable983::sEntry::OnLostDevice()
{
	for (std::map<sShaderFlags983, sShaderInst983*>::iterator it =
			this->insts.begin(); it != this->insts.end(); ++it)
		it->second->OnLostDevice();
}

void WDirect3D8::sShaderTable983::sEntry::OnResetDevice()
{
	for (std::map<sShaderFlags983, sShaderInst983*>::iterator it =
			this->insts.begin(); it != this->insts.end(); ++it)
		it->second->OnResetDevice();
}

void WDirect3D8::sShaderTable983::OnLostDevice()
{
	for (std::map<unsigned int, sEntry*>::iterator it = this->byId.begin();
		it != this->byId.end(); ++it)
		it->second->OnLostDevice();
}

void WDirect3D8::sShaderTable983::OnResetDevice()
{
	for (std::map<unsigned int, sEntry*>::iterator it = this->byId.begin();
		it != this->byId.end(); ++it)
		it->second->OnResetDevice();
}

WDirect3D8::sShaderInst983* WDirect3D8::sShaderTable983::Find(
	const sShaderMtlState983& key) const
{
	sEntry* entry = this->FindProgram(key.shaderId);
	if (!entry)
		return 0;
	return entry->Find((const sShaderFlags983&)key.flags);
}

// 100098a0: passa ao efeito os parâmetros do material (valor do material, valor forçado ou o padrão do efeito)
void WDirect3D8::sShaderInst983::ApplyMtlParams(WxShaderMtl* pShaderMtl) const
{
	WDirect3D8* dev = WDirect3D8::GetInstance();
	for (std::map<std::string, sUserParam<int> >::const_iterator it =
			this->boolParams.begin(); it != this->boolParams.end(); ++it)
	{
		const sUserParam<int>& param = it->second;
		if (param.GetSemantic() < 9)
		{
			const WxShaderMtlParam<int>* state =
				pShaderMtl->GetBoolParam(param.GetSemantic());
			if (state)
				dev->GetFxParamPool().SetBool(this->pEffect, param.GetHandle(),
					state->Get());
			else
				dev->GetFxParamPool().SetBool(this->pEffect, param.GetHandle(),
					param.GetValue());
		}
		else
		{
			const WxShaderMtlParam<int>* state =
				pShaderMtl->GetBoolParam(it->first);
			if (state)
				dev->GetFxParamPool().SetBool(this->pEffect, param.GetHandle(),
					state->Get());
			else
				dev->GetFxParamPool().SetBool(this->pEffect, param.GetHandle(),
					param.GetValue());
		}
	}
	for (std::map<std::string, sUserParam<int> >::const_iterator it =
			this->intParams.begin(); it != this->intParams.end(); ++it)
	{
		const sUserParam<int>& param = it->second;
		if (param.GetSemantic() < 9)
		{
			const WxShaderMtlParam<int>* state =
				pShaderMtl->GetIntParam(param.GetSemantic());
			if (state)
				dev->GetFxParamPool().SetInt(this->pEffect, param.GetHandle(),
					state->Get());
			else
				dev->GetFxParamPool().SetInt(this->pEffect, param.GetHandle(),
					param.GetValue());
		}
		else
		{
			const WxShaderMtlParam<int>* state =
				pShaderMtl->GetIntParam(it->first);
			if (state)
				dev->GetFxParamPool().SetInt(this->pEffect, param.GetHandle(),
					state->Get());
			else
				dev->GetFxParamPool().SetInt(this->pEffect, param.GetHandle(),
					param.GetValue());
		}
	}
	for (std::map<std::string, sUserParam<float> >::const_iterator it =
			this->floatParams.begin(); it != this->floatParams.end(); ++it)
	{
		const sUserParam<float>& param = it->second;
		if (param.GetSemantic() < 9)
		{
			const WxShaderMtlParam<float>* state =
				pShaderMtl->GetFloatParam(param.GetSemantic());
			if (state)
				dev->GetFxParamPool().SetFloat(this->pEffect, param.GetHandle(),
					state->Get());
			else
				dev->GetFxParamPool().SetFloat(this->pEffect, param.GetHandle(),
					param.GetValue());
		}
		else
		{
			const WxShaderMtlParam<float>* state =
				pShaderMtl->GetFloatParam(it->first);
			if (state)
				dev->GetFxParamPool().SetFloat(this->pEffect, param.GetHandle(),
					state->Get());
			else
				dev->GetFxParamPool().SetFloat(this->pEffect, param.GetHandle(),
					param.GetValue());
		}
	}
	for (std::map<std::string, sUserParam<D3DXVECTOR2> >::const_iterator it =
			this->vec2Params.begin(); it != this->vec2Params.end(); ++it)
	{
		const sUserParam<D3DXVECTOR2>& param = it->second;
		if (param.GetSemantic() < 9)
		{
			const WxShaderMtlParam<D3DXVECTOR2>* state =
				pShaderMtl->GetVector2Param(param.GetSemantic());
			if (state)
				dev->GetFxParamPool().SetVector2(this->pEffect, param.GetHandle(),
					state->Get());
			else
				dev->GetFxParamPool().SetVector2(this->pEffect, param.GetHandle(),
					param.GetValue());
		}
		else
		{
			const WxShaderMtlParam<D3DXVECTOR2>* state =
				pShaderMtl->GetVector2Param(it->first);
			if (state)
				dev->GetFxParamPool().SetVector2(this->pEffect, param.GetHandle(),
					state->Get());
			else
				dev->GetFxParamPool().SetVector2(this->pEffect, param.GetHandle(),
					param.GetValue());
		}
	}
	for (std::map<std::string, sUserParam<D3DXVECTOR3> >::const_iterator it =
			this->vec3Params.begin(); it != this->vec3Params.end(); ++it)
	{
		const sUserParam<D3DXVECTOR3>& param = it->second;
		if (param.GetSemantic() < 9)
		{
			const WxShaderMtlParam<D3DXVECTOR3>* state =
				pShaderMtl->GetVector3Param(param.GetSemantic());
			if (state)
				dev->GetFxParamPool().SetVector3(this->pEffect, param.GetHandle(),
					state->Get());
			else
				dev->GetFxParamPool().SetVector3(this->pEffect, param.GetHandle(),
					param.GetValue());
		}
		else
		{
			const WxShaderMtlParam<D3DXVECTOR3>* state =
				pShaderMtl->GetVector3Param(it->first);
			if (state)
				dev->GetFxParamPool().SetVector3(this->pEffect, param.GetHandle(),
					state->Get());
			else
				dev->GetFxParamPool().SetVector3(this->pEffect, param.GetHandle(),
					param.GetValue());
		}
	}
	for (std::map<std::string, sUserParam<D3DXVECTOR4> >::const_iterator it =
			this->vec4Params.begin(); it != this->vec4Params.end(); ++it)
	{
		const sUserParam<D3DXVECTOR4>& param = it->second;
		if (param.GetSemantic() < 9)
		{
			const WxShaderMtlParam<D3DXVECTOR4>* state =
				pShaderMtl->GetVector4Param(param.GetSemantic());
			if (state)
				dev->GetFxParamPool().SetVector4(this->pEffect, param.GetHandle(),
					state->Get());
			else
				dev->GetFxParamPool().SetVector4(this->pEffect, param.GetHandle(),
					param.GetValue());
		}
		else
		{
			const WxShaderMtlParam<D3DXVECTOR4>* state =
				pShaderMtl->GetVector4Param(it->first);
			if (state)
				dev->GetFxParamPool().SetVector4(this->pEffect, param.GetHandle(),
					state->Get());
			else
				dev->GetFxParamPool().SetVector4(this->pEffect, param.GetHandle(),
					param.GetValue());
		}
	}
	for (std::map<std::string, sUserParam<D3DXMATRIX> >::const_iterator it =
			this->matrixParams.begin(); it != this->matrixParams.end(); ++it)
	{
		const sUserParam<D3DXMATRIX>& param = it->second;
		if (param.GetSemantic() < 9)
		{
			const WxShaderMtlParam<D3DXMATRIX>* state =
				pShaderMtl->GetMatrixParam(param.GetSemantic());
			if (state)
				dev->GetFxParamPool().SetMatrix(this->pEffect, param.GetHandle(),
					state->Get());
			else
				dev->GetFxParamPool().SetMatrix(this->pEffect, param.GetHandle(),
					param.GetValue());
		}
		else
		{
			const WxShaderMtlParam<D3DXMATRIX>* state =
				pShaderMtl->GetMatrixParam(it->first);
			if (state)
				dev->GetFxParamPool().SetMatrix(this->pEffect, param.GetHandle(),
					state->Get());
			else
				dev->GetFxParamPool().SetMatrix(this->pEffect, param.GetHandle(),
					param.GetValue());
		}
	}
	for (std::map<std::string, WxShaderMtl::sTexture>::const_iterator it =
			pShaderMtl->m_textures.begin();
		it != pShaderMtl->m_textures.end(); ++it)
	{
		const WxShaderMtl::sTexture& texture = it->second;
		std::map<std::string, sUserParam<IDirect3DBaseTexture9*> >::const_iterator
			param = this->textureParams.find(texture.name);
		if (param != this->textureParams.end())
		{
			dev->xInstantiateAndFillTexture(texture.m_value);
			dev->GetFxParamPool().SetTexture(this->pEffect, param->second.GetHandle(),
				dev->m_texList[texture.m_value].pTex);
		}
	}
}

// 1000b970: shader registrado com esse nome (0 se não existe)
const WDirect3D8::sShaderTable983::sEntry* WDirect3D8::sShaderTable983::FindByName(
	const char* name) const
{
	std::map<const std::string, sEntry*>::const_iterator it =
		this->byName.find(name);
	if (it != this->byName.end())
		return it->second;
	return 0;
}

WDirect3D8::sShaderTable983::sEntry::sEntry(unsigned int id_, unsigned long input_,
	const std::string& source_)
	: id(id_), input(input_), source(source_)
{
}

WDirect3D8::sShaderInst983::~sShaderInst983()
{
	if (this->pEffect)
		this->pEffect->Release();
}

namespace
{
// 10001670/10001680/100016a0: dono do efeito de teste de Register (solta ao sair)
struct sEffectHolder
{
	ID3DXEffect* p;
	sEffectHolder(ID3DXEffect* p_) : p(p_) { }
	~sEffectHolder()
	{
		if (p)
		{
			p->Release();
			p = 0;
		}
	}
	ID3DXEffect* Get() const { return p; }
};
}

// 10011400: compila o efeito uma vez para ler o que o vertex shader recebe e guarda o fonte; devolve o id (0 = erro)
int WDirect3D8::sShaderTable983::Register(const char* name,
	const std::string& source)
{
	D3DXMACRO macros[2] =
	{
		{ "NUM_MAX_BONES_PER_BATCH", "(50)" },
		{ 0, 0 },
	};
	WDirect3D8* dev = WDirect3D8::GetInstance();
	sEffectHolder effect(dev->xCompileEffect(source, macros));
	if (!effect.Get())
		return 0;
	D3DXHANDLE technique = effect.Get()->GetTechnique(0);
	if (!technique)
		return 0;
	D3DXHANDLE pass = effect.Get()->GetPass(technique, 0);
	if (!pass)
		return 0;
	D3DXPASS_DESC passDesc;
	if (FAILED(effect.Get()->GetPassDesc(pass, &passDesc)))
		return 0;
	D3DXSEMANTIC semantics[MAXD3DDECLLENGTH];
	UINT count;
	unsigned long input = 0;
	int texCoords = 0;
	D3DXGetShaderInputSemantics(passDesc.pVertexShaderFunction, semantics,
		&count);
	for (UINT i = 0; i < count; ++i)
	{
		switch (semantics[i].Usage)
		{
		case D3DDECLUSAGE_POSITION:
			input |= D3DFVF_XYZ;
			break;
		case D3DDECLUSAGE_NORMAL:
			input |= D3DFVF_NORMAL;
			break;
		case D3DDECLUSAGE_TEXCOORD:
			++texCoords;
			break;
		case D3DDECLUSAGE_COLOR:
			input |= D3DFVF_DIFFUSE;
			break;
		}
	}
	input |= texCoords << D3DFVF_TEXCOUNT_SHIFT;
	unsigned int id = this->byName.size() + 1;
	sEntry* entry = new sEntry(id, input, source);
	this->byName[name] = entry;
	this->byId[id] = entry;
	return id;
}

// 10011990: lê do efeito os parâmetros usados pela técnica 0 e separa os de sistema dos do material
WDirect3D8::sShaderInst983::sShaderInst983(ID3DXEffect* pEffect_)
	: pEffect(pEffect_)
{
	D3DXHANDLE technique = this->pEffect->GetTechnique(0);
	D3DXEFFECT_DESC effectDesc;
	this->pEffect->GetDesc(&effectDesc);
	for (UINT i = 0; i < effectDesc.Parameters; ++i)
	{
		D3DXHANDLE handle = this->pEffect->GetParameter(NULL, i);
		if (!this->pEffect->IsParameterUsed(handle, technique))
			continue;
		D3DXPARAMETER_DESC paramDesc;
		this->pEffect->GetParameterDesc(handle, &paramDesc);
		if (paramDesc.Type == D3DXPT_SAMPLER ||
			paramDesc.Type == D3DXPT_SAMPLER1D ||
			paramDesc.Type == D3DXPT_SAMPLER2D ||
			paramDesc.Type == D3DXPT_SAMPLER3D)
			continue;
		int type = xParamType(paramDesc);
		if (type == 9)
			continue;
		sParamDesc desc(handle, type);
		int semantic;
		if (paramDesc.Semantic)
		{
			semantic = WVDShSysParamSemanticFromName(paramDesc.Semantic);
			if (semantic < 13)
			{
				sSysParam sysParam(desc, semantic);
				this->sysParams.push_back(sysParam);
				continue;
			}
		}
		semantic = paramDesc.Semantic ?
			WVDShUserParamSemanticFromName(paramDesc.Semantic) : 9;
		switch (type)
		{
		case 0:
		{
			BOOL value;
			this->pEffect->GetBool(handle, &value);
			this->boolParams[paramDesc.Name] =
				sUserParam<int>(desc, semantic, value);
			break;
		}
		case 1:
		{
			int value;
			this->pEffect->GetInt(handle, &value);
			this->intParams[paramDesc.Name] =
				sUserParam<int>(desc, semantic, value);
			break;
		}
		case 2:
		{
			float value;
			this->pEffect->GetFloat(handle, &value);
			this->floatParams[paramDesc.Name] =
				sUserParam<float>(desc, semantic, value);
			break;
		}
		case 3:
		{
			D3DXVECTOR2 value;
			this->pEffect->GetFloatArray(handle, (float*)&value, 2);
			this->vec2Params[paramDesc.Name] =
				sUserParam<D3DXVECTOR2>(desc, semantic, value);
			break;
		}
		case 4:
		{
			D3DXVECTOR3 value;
			this->pEffect->GetFloatArray(handle, (float*)&value, 3);
			this->vec3Params[paramDesc.Name] =
				sUserParam<D3DXVECTOR3>(desc, semantic, value);
			break;
		}
		case 5:
		{
			D3DXVECTOR4 value;
			this->pEffect->GetVector(handle, &value);
			this->vec4Params[paramDesc.Name] =
				sUserParam<D3DXVECTOR4>(desc, semantic, value);
			break;
		}
		case 6:
		{
			D3DXMATRIX value;
			this->pEffect->GetMatrix(handle, &value);
			this->matrixParams[paramDesc.Name] =
				sUserParam<D3DXMATRIX>(desc, semantic, value);
			break;
		}
		case 8:
			this->textureParams[paramDesc.Name] =
				sUserParam<IDirect3DBaseTexture9*>(desc, semantic, NULL);
			break;
		}
	}
}

WDirect3D8::sShaderTable983::sEntry::~sEntry()
{
	for (std::map<sShaderFlags983, sShaderInst983*>::iterator it =
			this->insts.begin(); it != this->insts.end(); ++it)
		delete it->second;
}

// 100122c0: compila o efeito com as macros WVDSHMACRO_* dos bits ligados
WDirect3D8::sShaderInst983* WDirect3D8::sShaderTable983::sEntry::Create(
	const sShaderFlags983& flags)
{
	D3DXMACRO macros[12] =
	{
		{ "NUM_MAX_BONES_PER_BATCH", "(50)" },
	};
	int count = 1;
	for (size_t i = 0; i < 10; ++i)
	{
		if (flags.bits.test(i))
		{
			macros[count].Name = WVDShMacroName(i);
			macros[count].Definition = NULL;
			++count;
		}
	}
	macros[count].Name = NULL;
	macros[count].Definition = NULL;
	sShaderInst983* inst = new sShaderInst983(
		WDirect3D8::GetInstance()->xCompileEffect(this->source, macros));
	this->insts[flags] = inst;
	return inst;
}

WDirect3D8::sShaderInst983* WDirect3D8::sShaderTable983::Create(
	const sShaderMtlState983& key)
{
	sEntry* entry = this->FindProgram(key.shaderId);
	return entry->Create((const sShaderFlags983&)key.flags);
}

void WDirect3D8::sShaderTable983::Release()
{
	for (std::map<unsigned int, sEntry*>::iterator it = this->byId.begin();
		it != this->byId.end(); ++it)
		delete it->second;
	this->byName.clear();
	this->byId.clear();
}
