#pragma once
#include <windows.h>
#include <math.h>
#include <bitset>
#include <map>
#include <string>
struct D3DXVECTOR2;
struct D3DXVECTOR3;
struct D3DXVECTOR4;
struct D3DXMATRIX;
#include "wmath.h"
#include "wscene.h"

enum WVDTRANSFORMSTATETYPE
{
	WVDTS_VIEW = 0x2,
	WVDTS_PROJECTION = 0x3,
	WVDTS_TEXTURE0 = 0x10,
	WVDTS_TEXTURE1 = 0x11,
	WVDTS_TEXTURE2 = 0x12,
	WVDTS_TEXTURE3 = 0x13,
	WVDTS_TEXTURE4 = 0x14,
	WVDTS_TEXTURE5 = 0x15,
	WVDTS_TEXTURE6 = 0x16,
	WVDTS_TEXTURE7 = 0x17,
	WVDTS_FORCE_DWORD = 0x7FFFFFFF,
};

enum wVDevMessage
{
	W_VDEV_SETCLIPPER,
	W_VDEV_ACTIVATEAPP,
	W_VDEV_MOVE,
	W_VDEV_SIZE,
	W_VDEV_SETSIZE,
	W_VDEV_CAPTURE_SCREEN,
	W_VDEV_GET_FRONTSURFACE,
	W_VDEV_GET_BACKSURFACE,
	W_VDEV_DRAWBOX,
	W_VDEV_SETHWND,
	W_VDEV_SETRECT,
	W_VDEV_FLUSH,
	W_VDEV_FLIPFRONTSURFACE,
	W_VDEV_GET_CAPTURE_MODE,
	W_VDEV_SET_CAPTURE_MODE,
	W_VDEV_SET_CAPTURE_RECT, // 983: novo (0xf); nome provisório
	W_VDEV_RELEASE_CAPTURERESOURCE,
	W_VDEV_CAPTURED_BG,
	W_VDEV_OVERDRAW_ANALYZE,
	W_VDEV_GET_ZBUFF_HISTOGRAM,
	W_VDEV_SCREEN_SHOT,
	W_VDEV_GET_SPLASH,
	W_VDEV_GET_COPIEDSCREENSPLASH,
	W_VDEV_UPDATE_LOD_TEXTURE,
	W_VDEV_SET_DDS_RES,
	W_VDEV_USE_MIPMAP,
	W_VDEV_MAX_MIPLVL,
	W_VDEV_MIP_CREATE_FILTER,
	W_VDEV_MIP_TEXSTAGE_FILTER,
	W_VDEV_GETAVAIL_VRAM,
	W_VDEV_NUM_TEXTURES,
	W_VDEV_NUM_TNL_BUFFERS,
	WX_VDEV_GET_STATISTICS,
	WX_VDEV_GET_CAPS,
	WX_VDEV_FILLMODE,
	WX_VDEV_DRIVER_INFO,
	WX_VDEV_GET_SORTBUFFER_SIZE,
	WX_VDEV_GET_MERGEBUFFER_SIZE,
	WX_VDEV_GET_MAX_BONES_PER_BATCH, // 983: novo (0x26, devolve 50); nome provisório
	WX_VDEV_SET_SHADER_INCLUDE_PROC, // 983: novo (0x27); nome provisório
};

enum wWxVertexElem
{
	WX_VELEM_POSITION,
	WX_VELEM_NORMAL,
	WX_VELEM_DIFFUSE,
	WX_VELEM_TEX1,
	WX_VELEM_TEX2,
	WX_VELEM_TEX3,
	WX_VELEM_TEX4,
	WX_VELEM_TEX5,
	WX_VELEM_TEX6,
	WX_VELEM_TEX7,
	WX_VELEM_TEX8,
};

enum WVDTEXTURESTAGESTATETYPE
{
	WVDTSS_COLOROP = 0x1,
	WVDTSS_COLORARG1 = 0x2,
	WVDTSS_COLORARG2 = 0x3,
	WVDTSS_ALPHAOP = 0x4,
	WVDTSS_ALPHAARG1 = 0x5,
	WVDTSS_ALPHAARG2 = 0x6,
	WVDTSS_BUMPENVMAT00 = 0x7,
	WVDTSS_BUMPENVMAT01 = 0x8,
	WVDTSS_BUMPENVMAT10 = 0x9,
	WVDTSS_BUMPENVMAT11 = 0xA,
	WVDTSS_TEXCOORDINDEX = 0xB,
	WVDTSS_BUMPENVLSCALE = 0x16,
	WVDTSS_BUMPENVLOFFSET = 0x17,
	WVDTSS_TEXTURETRANSFORMFLAGS = 0x18,
	WVDTSS_COLORARG0 = 0x1A,
	WVDTSS_ALPHAARG0 = 0x1B,
	WVDTSS_RESULTARG = 0x1C,
	WVDTSS_CONSTANT = 0x20,
	WVDTSS_FORCE_DWORD = 0x7FFFFFFF,
};

enum WVDRENDERSTATETYPE
{
	WVDRS_ZENABLE = 0x7,
	WVDRS_FILLMODE = 0x8,
	WVDRS_SHADEMODE = 0x9,
	WVDRS_ZWRITEENABLE = 0xE,
	WVDRS_ALPHATESTENABLE = 0xF,
	WVDRS_LASTPIXEL = 0x10,
	WVDRS_SRCBLEND = 0x13,
	WVDRS_DESTBLEND = 0x14,
	WVDRS_CULLMODE = 0x16,
	WVDRS_ZFUNC = 0x17,
	WVDRS_ALPHAREF = 0x18,
	WVDRS_ALPHAFUNC = 0x19,
	WVDRS_DITHERENABLE = 0x1A,
	WVDRS_ALPHABLENDENABLE = 0x1B,
	WVDRS_FOGENABLE = 0x1C,
	WVDRS_SPECULARENABLE = 0x1D,
	WVDRS_FOGCOLOR = 0x22,
	WVDRS_FOGTABLEMODE = 0x23,
	WVDRS_FOGSTART = 0x24,
	WVDRS_FOGEND = 0x25,
	WVDRS_FOGDENSITY = 0x26,
	WVDRS_RANGEFOGENABLE = 0x30,
	WVDRS_STENCILENABLE = 0x34,
	WVDRS_STENCILFAIL = 0x35,
	WVDRS_STENCILZFAIL = 0x36,
	WVDRS_STENCILPASS = 0x37,
	WVDRS_STENCILFUNC = 0x38,
	WVDRS_STENCILREF = 0x39,
	WVDRS_STENCILMASK = 0x3A,
	WVDRS_STENCILWRITEMASK = 0x3B,
	WVDRS_TEXTUREFACTOR = 0x3C,
	WVDRS_WRAP0 = 0x80,
	WVDRS_WRAP1 = 0x81,
	WVDRS_WRAP2 = 0x82,
	WVDRS_WRAP3 = 0x83,
	WVDRS_WRAP4 = 0x84,
	WVDRS_WRAP5 = 0x85,
	WVDRS_WRAP6 = 0x86,
	WVDRS_WRAP7 = 0x87,
	WVDRS_CLIPPING = 0x88,
	WVDRS_LIGHTING = 0x89,
	WVDRS_AMBIENT = 0x8B,
	WVDRS_FOGVERTEXMODE = 0x8C,
	WVDRS_COLORVERTEX = 0x8D,
	WVDRS_LOCALVIEWER = 0x8E,
	WVDRS_NORMALIZENORMALS = 0x8F,
	WVDRS_DIFFUSEMATERIALSOURCE = 0x91,
	WVDRS_SPECULARMATERIALSOURCE = 0x92,
	WVDRS_AMBIENTMATERIALSOURCE = 0x93,
	WVDRS_EMISSIVEMATERIALSOURCE = 0x94,
	WVDRS_VERTEXBLEND = 0x97,
	WVDRS_CLIPPLANEENABLE = 0x98,
	WVDRS_POINTSIZE = 0x9A,
	WVDRS_POINTSIZE_MIN = 0x9B,
	WVDRS_POINTSPRITEENABLE = 0x9C,
	WVDRS_POINTSCALEENABLE = 0x9D,
	WVDRS_POINTSCALE_A = 0x9E,
	WVDRS_POINTSCALE_B = 0x9F,
	WVDRS_POINTSCALE_C = 0xA0,
	WVDRS_MULTISAMPLEANTIALIAS = 0xA1,
	WVDRS_MULTISAMPLEMASK = 0xA2,
	WVDRS_PATCHEDGESTYLE = 0xA3,
	WVDRS_DEBUGMONITORTOKEN = 0xA5,
	WVDRS_POINTSIZE_MAX = 0xA6,
	WVDRS_INDEXEDVERTEXBLENDENABLE = 0xA7,
	WVDRS_COLORWRITEENABLE = 0xA8,
	WVDRS_TWEENFACTOR = 0xAA,
	WVDRS_BLENDOP = 0xAB,
	WVDRS_POSITIONDEGREE = 0xAC,
	WVDRS_NORMALDEGREE = 0xAD,
	WVDRS_SCISSORTESTENABLE = 0xAE,
	WVDRS_SLOPESCALEDEPTHBIAS = 0xAF,
	WVDRS_ANTIALIASEDLINEENABLE = 0xB0,
	WVDRS_MINTESSELLATIONLEVEL = 0xB2,
	WVDRS_MAXTESSELLATIONLEVEL = 0xB3,
	WVDRS_ADAPTIVETESS_X = 0xB4,
	WVDRS_ADAPTIVETESS_Y = 0xB5,
	WVDRS_ADAPTIVETESS_Z = 0xB6,
	WVDRS_ADAPTIVETESS_W = 0xB7,
	WVDRS_ENABLEADAPTIVETESSELLATION = 0xB8,
	WVDRS_TWOSIDEDSTENCILMODE = 0xB9,
	WVDRS_CCW_STENCILFAIL = 0xBA,
	WVDRS_CCW_STENCILZFAIL = 0xBB,
	WVDRS_CCW_STENCILPASS = 0xBC,
	WVDRS_CCW_STENCILFUNC = 0xBD,
	WVDRS_COLORWRITEENABLE1 = 0xBE,
	WVDRS_COLORWRITEENABLE2 = 0xBF,
	WVDRS_COLORWRITEENABLE3 = 0xC0,
	WVDRS_BLENDFACTOR = 0xC1,
	WVDRS_SRGBWRITEENABLE = 0xC2,
	WVDRS_DEPTHBIAS = 0xC3,
	WVDRS_WRAP8 = 0xC6,
	WVDRS_WRAP9 = 0xC7,
	WVDRS_WRAP10 = 0xC8,
	WVDRS_WRAP11 = 0xC9,
	WVDRS_WRAP12 = 0xCA,
	WVDRS_WRAP13 = 0xCB,
	WVDRS_WRAP14 = 0xCC,
	WVDRS_WRAP15 = 0xCD,
	WVDRS_SEPARATEALPHABLENDENABLE = 0xCE,
	WVDRS_SRCBLENDALPHA = 0xCF,
	WVDRS_DESTBLENDALPHA = 0xD0,
	WVDRS_BLENDOPALPHA = 0xD1,
	WVDRS_FORCE_DWORD = 0x7FFFFFFF,
};

enum WVDSAMPLERSTATETYPE
{
	WVDSAMP_ADDRESSU = 0x1,
	WVDSAMP_ADDRESSV = 0x2,
	WVDSAMP_ADDRESSW = 0x3,
	WVDSAMP_BORDERCOLOR = 0x4,
	WVDSAMP_MAGFILTER = 0x5,
	WVDSAMP_MINFILTER = 0x6,
	WVDSAMP_MIPFILTER = 0x7,
	WVDSAMP_MIPMAPLODBIAS = 0x8,
	WVDSAMP_MAXMIPLEVEL = 0x9,
	WVDSAMP_MAXANISOTROPY = 0xA,
	WVDSAMP_SRGBTEXTURE = 0xB,
	WVDSAMP_ELEMENTINDEX = 0xC,
	WVDSAMP_DMAPOFFSET = 0xD,
	WVDSAMP_FORCE_DWORD = 0x7FFFFFFF,
};

enum wVDevState
{
	W_VDEVSTATE_NORMAL,
	W_VDEVSTATE_LOST,
	NUM_W_VDEVSTATES
};

#ifdef WANGREAL_DEVICE
enum WVDFXPARAMETERTYPE
#else
enum WFxParameterType
#endif
{
	WFxParamWorld = 0x0,
	WFxParamSkinMatrix = 0x1,
	WFxParamPrevView = 0x2,
	WFxParamView = 0x3,
	WFxParamProjection = 0x4,
	WFxParamConstColor = 0x5,
	WFxParamLightDiffuse = 0x6,
	WFxParamLightAmbient = 0x7,
	WFxParamLightDirection = 0x8,
	WFxParamFogRange = 0x9,
	WFxParamFogColor = 0xA,
	WFxParamCameraPosition = 0xB,
	WFxParamExtraTexture = 0xC,
	WFxParamRefTexture = 0xD,
	WFxParamRefTexture2 = 0xE,
	WFxParamRefTexture3 = 0xF,
	WFxParamRefTexture4 = 0x10,
	WFxParamMaterialColor = 0x11,
	WFxParamTextureTransform = 0x12,
	WFxParamBlurUvOffset = 0x13,
	WFxParamShadowColor = 0x14,
	WFxParamCrossFilterOffset = 0x15,
	WFxParamElapsedTime = 0x16,
	WFxParamDownScaleRate = 0x17,
	WFxParamOffsetWeightSampNum = 0x18,
	WFxParamDofNotFocusRangeMin = 0x19,
	WFxParamDofNotFocusRangeMax = 0x1A,
	WFxParamHdrMiddleGray = 0x1B,
	WFxParamHdrOriginalWeight = 0x1C,
	WFxParamHdrCrossFilterWeight = 0x1D,
	WFxParamHdrBrightFilterWeight = 0x1E,
	WFxParamRimLightColor = 0x1F,
	WFxParamRimLightWeight = 0x20,
	WFxParamAmbientBright = 0x21,
	WFxParamAmbientTexWeight = 0x22,
	WFxParamHdrLumiRange = 0x23,
	WFxParamHdrStarPower = 0x24,
	WFxParamNum = 0x25,
};

#ifdef WANGREAL_DEVICE
#include "wsplash.h"
#endif

struct WRenderToTextureSizeInfo
{
	bool m_isAbsolute;
	float m_width;
	float m_height;

	static const WRenderToTextureSizeInfo SIZE_FULL;
	static const WRenderToTextureSizeInfo SIZE_HALF;
	static const WRenderToTextureSizeInfo SIZE_ABS;

#ifdef WANGREAL_DEVICE
	WRenderToTextureSizeInfo()
		: m_isAbsolute(true), m_width(0.0f), m_height(0.0f)
	{
	}
#else
#ifdef REBANG_PCH_POOL // HACK: only in the pool PCH, other units keep their IL (emission order of the ctor in pool)
	WRenderToTextureSizeInfo()
		: m_isAbsolute(true), m_width(0.0f), m_height(0.0f)
	{
	}
#else
	WRenderToTextureSizeInfo() { Reset(); }
#endif
#endif
	WRenderToTextureSizeInfo(bool isAbsolute, float width, float height)
		: m_isAbsolute(isAbsolute), m_width(width), m_height(height)
	{
	}
	void Reset()
	{
		m_isAbsolute = true;
		m_width = 0.0f;
		m_height = 0.0f;
	}
};

struct WRenderToTextureParam
{
	enum
	{
		NUM_SIMULTANEOUSRTS = 2
	};

	struct RtTexInfo
	{
		int m_hTex;
		bool m_needToClear;
		ulong m_clearClr;

		RtTexInfo()
			: m_hTex(0), m_needToClear(false), m_clearClr(0)
		{
		}
	};

	struct DepthSurfInfo
	{
		enum SurfaceUsage
		{
			USE_NOT_NEEDED,
			USE_MAIN_SURFACE,
			USE_SHARED_SURFACE,
			USE_EXCLUSIVE_SURFACE
		};

		SurfaceUsage m_surfUsage;
		int m_hTex;
		bool m_needToClear;
		float m_clearZ;

		DepthSurfInfo()
			: m_surfUsage(USE_NOT_NEEDED),
			  m_hTex(0),
			  m_needToClear(false),
			  m_clearZ(1.0f)
		{
		}
	};

#ifdef WANGREAL_DEVICE
	RtTexInfo m_rtTexInfo[2];
#else
	RtTexInfo m_rtTexInfo0;
	RtTexInfo m_rtTexInfo1;
#endif
	DepthSurfInfo m_depthSurfInfo;

	int GetTexture(unsigned index) const
	{
		if (index < NUM_SIMULTANEOUSRTS)
#ifdef WANGREAL_DEVICE
			return m_rtTexInfo[index].m_hTex;
#else
			return (&m_rtTexInfo0)[index].m_hTex;
#endif
		return 0;
	}
#ifdef WANGREAL_DEVICE
	bool CanClearAtOnce() const
	{
		for (int i = 1; i < NUM_SIMULTANEOUSRTS; ++i)
		{
			if (m_rtTexInfo[i].m_hTex &&
				(m_rtTexInfo[i].m_needToClear != m_rtTexInfo[0].m_needToClear ||
					m_rtTexInfo[i].m_clearClr != m_rtTexInfo[0].m_clearClr))
				return false;
		}
		return true;
	}
#else
	bool CanClearAtOnce() const;
#endif
};

#ifdef WANGREAL_DEVICE
#include "wdevice.h"
#else
class WProc;

class WDevice
{
public:
	virtual ~WDevice();
	virtual char* GetDeviceName();
	virtual char* EnumModeName();
	virtual WProc* ExternProc();
};
#endif

struct WTVertex
{
	union
	{
		struct
		{
			float sx, sy, sz;
		};
		struct
		{
			float x, y, z;
		};
	};
	void SetPosition(const WVector& p)
	{
		x = p.x;
		y = p.y;
		z = p.z;
	}
	float rhw;
	unsigned long diffuse;
	float tu;
	float tv;
	// 983: vz subiu para +0x1c e há 7 pares de coordenadas extras (lu/lv é o primeiro); sizeof 0x28 -> 0x58.
	// Vale também para o exe (F3.4: W3dAniSpr::vtx[4] tem passo 0x58 em 00bafbc0).
	float vz;
	float lu;
	float lv;
	float uvEx[6][2];
};

#ifdef WANGREAL_DEVICE
// 983: novo. Lista de vértices dada como vetor (WTVertex*) ou como vetor de ponteiros (WTVertex**); é o que
// permitiu um ModifyVertices só. Nome provisório (10012b10, 10012b30, 10012b50).
class WTVertexList
{
public:
	WTVertexList(WTVertex* v)
	{
		m_v = v;
		m_pv = 0;
	}
	WTVertexList(WTVertex** pv)
	{
		m_v = 0;
		m_pv = pv;
	}
	const WTVertex& operator[](int i) const
	{
		if (m_v)
			return m_v[i];
		return *m_pv[i];
	}

private:
	WTVertex* m_v;
	WTVertex** m_pv;
};
#endif

// 983: material de shader (unidade wxshadermtl, F2.8; o exe 983 tem cópia própria em 00bb1be0–00bb83d0,
// s150, comum aos dois ramos; o .pet guarda os materiais no WPuppet +0x1fc). Nomes provisórios (não há PDB do 839/983).
// Descritor de um parâmetro do material: nome no efeito, semântica (WVDSHBASICUSERPARAMSEMANTIC_*, 9 = nenhuma)
// e tipo (TYPE_BOOL..TYPE_TEXTURE, 9 = nenhum).
struct WxShaderMtlParamDesc
{
	std::string name;
	int semantic; // +0x1c
	int type; // +0x20
	WxShaderMtlParamDesc() { Clear(); } // 1004ee40
	void Clear() // 1004db20
	{
		name.clear();
		semantic = 9;
		type = 9;
	}
};

// valor de um parâmetro (+0x24) e o valor imposto por cima dele (10013300, 10013320)
template <class T>
struct WxShaderMtlParam : WxShaderMtlParamDesc
{
	T m_value;
	bool m_bForced;
	T m_forced;
	WxShaderMtlParam() {}
	WxShaderMtlParam(const WxShaderMtlParamDesc& desc, const T& value)
		: WxShaderMtlParamDesc(desc), m_value(value), m_bForced(false)
	{
	}
	void SetValue(const T& value) { m_value = value; } // 100493c0
	void Force(const T& value)
	{
		m_bForced = true;
		m_forced = value;
	}
	void Unforce() { m_bForced = false; }
	bool IsForced() const { return m_bForced; } // 10001700, 100019a0...
	const T& Get() const
	{
		if (IsForced())
			return m_forced;
		return m_value;
	}
};
typedef WxShaderMtlParam<int> WxShaderMtlState;

// `struct` no 983: o nome RTTI é .?AUWxShaderMtl@@ (F2.10)
struct WxShaderMtl
{
public:
	// textura do material: m_value = textura do dispositivo em uso, m_hOrgTex = a do arquivo
	struct sTexture : WxShaderMtlParam<int>
	{
		int m_hOrgTex; // +0x30
		std::string m_file; // +0x34
		sTexture() : m_hOrgTex(0) {} // como o valor-inicializado do 645/dll: so m_hOrgTex zera (exe 983 00b8e770)
		// exe 983 00b80a10 (WPuppet: textura do material lida do .pet)
		sTexture(const WxShaderMtlParamDesc& desc, int hTex, const std::string& file)
			: WxShaderMtlParam<int>(desc, hTex), m_hOrgTex(hTex), m_file(file)
		{
		}
	};
	typedef std::map<std::string, WxShaderMtlParam<int> > tIntByName;
	typedef std::map<int, WxShaderMtlParam<int> > tIntById;
	typedef std::map<std::string, WxShaderMtlParam<float> > tFloatByName;
	typedef std::map<int, WxShaderMtlParam<float> > tFloatById;
	typedef std::map<std::string, WxShaderMtlParam<D3DXVECTOR2> > tVector2ByName;
	typedef std::map<int, WxShaderMtlParam<D3DXVECTOR2> > tVector2ById;
	typedef std::map<std::string, WxShaderMtlParam<D3DXVECTOR3> > tVector3ByName;
	typedef std::map<int, WxShaderMtlParam<D3DXVECTOR3> > tVector3ById;
	typedef std::map<std::string, WxShaderMtlParam<D3DXVECTOR4> > tVector4ByName;
	typedef std::map<int, WxShaderMtlParam<D3DXVECTOR4> > tVector4ById;
	typedef std::map<std::string, WxShaderMtlParam<D3DXMATRIX> > tMatrixByName;
	typedef std::map<int, WxShaderMtlParam<D3DXMATRIX> > tMatrixById;
	typedef std::map<std::string, sTexture> tTextureByName;
	typedef std::map<int, sTexture> tTextureById;

	WxShaderMtl(); // 10057a90
	virtual ~WxShaderMtl(); // 10058010

	std::string m_name; // +4
	int m_shaderId; // +0x20: id devolvido por RegisterShader
	float m_time; // +0x24: parâmetro de sistema 11
	std::bitset<10> m_flags; // +0x28
	tIntByName m_boolByName; // +0x2c
	tIntById m_boolById; // +0x3c
	tIntByName m_intByName; // +0x4c
	tIntById m_intById; // +0x5c
	tFloatByName m_floatByName; // +0x6c
	tFloatById m_floatById; // +0x7c
	tVector2ByName m_vector2ByName; // +0x8c
	tVector2ById m_vector2ById; // +0x9c
	tVector3ByName m_vector3ByName; // +0xac
	tVector3ById m_vector3ById; // +0xbc
	tVector4ByName m_vector4ByName; // +0xcc
	tVector4ById m_vector4ById; // +0xdc
	tMatrixByName m_matrixByName; // +0xec
	tMatrixById m_matrixById; // +0xfc
	tTextureByName m_textures; // +0x10c
	tTextureById m_textureById; // +0x11c

	void RandomizeTime(); // 10049390
	// valor de um parâmetro do material, pelo número (semântica < 9) ou pelo nome; 0 = não tem
	const WxShaderMtlParam<int>* GetBoolParam(int id) const; // 1004fad0
	WxShaderMtlParam<int>* GetBoolParam(int id); // 1004fb10
	const WxShaderMtlParam<int>* GetIntParam(int id) const; // 1004fb50
	WxShaderMtlParam<int>* GetIntParam(int id); // 1004fb90
	const WxShaderMtlParam<float>* GetFloatParam(int id) const; // 1004fbd0
	WxShaderMtlParam<float>* GetFloatParam(int id); // 1004fc10
	const WxShaderMtlParam<D3DXVECTOR2>* GetVector2Param(int id) const; // 1004fc50
	WxShaderMtlParam<D3DXVECTOR2>* GetVector2Param(int id); // 1004fc90
	const WxShaderMtlParam<D3DXVECTOR3>* GetVector3Param(int id) const; // 1004fcd0
	WxShaderMtlParam<D3DXVECTOR3>* GetVector3Param(int id); // 1004fd10
	const WxShaderMtlParam<D3DXVECTOR4>* GetVector4Param(int id) const; // 1004fd50
	WxShaderMtlParam<D3DXVECTOR4>* GetVector4Param(int id); // 1004fd90
	const WxShaderMtlParam<D3DXMATRIX>* GetMatrixParam(int id) const; // 1004fdd0
	WxShaderMtlParam<D3DXMATRIX>* GetMatrixParam(int id); // 1004fe10
	const sTexture* GetTextureParam(int id) const; // 1004fe50
	sTexture* GetTextureParam(int id); // 1004fe90
	void SetRenderFlags(unsigned long type, unsigned long type2); // 1004fed0
	void RestoreTextures(); // 10050040
	void ReplaceTexture(int hTex, int hOrgTex); // 100500b0
	const WxShaderMtlParam<int>* GetBoolParam(const std::string& name) const; // 10050c40
	WxShaderMtlParam<int>* GetBoolParam(const std::string& name); // 10050cb0
	const WxShaderMtlParam<int>* GetIntParam(const std::string& name) const; // 10050d20
	WxShaderMtlParam<int>* GetIntParam(const std::string& name); // 10050d90
	const WxShaderMtlParam<float>* GetFloatParam(const std::string& name) const; // 10050e00
	WxShaderMtlParam<float>* GetFloatParam(const std::string& name); // 10050e70
	const WxShaderMtlParam<D3DXVECTOR2>* GetVector2Param(const std::string& name) const; // 10050ee0
	WxShaderMtlParam<D3DXVECTOR2>* GetVector2Param(const std::string& name); // 10050f60
	const WxShaderMtlParam<D3DXVECTOR3>* GetVector3Param(const std::string& name) const; // 10050fe0
	WxShaderMtlParam<D3DXVECTOR3>* GetVector3Param(const std::string& name); // 10051060
	const WxShaderMtlParam<D3DXVECTOR4>* GetVector4Param(const std::string& name) const; // 100510e0
	WxShaderMtlParam<D3DXVECTOR4>* GetVector4Param(const std::string& name); // 10051160
	const WxShaderMtlParam<D3DXMATRIX>* GetMatrixParam(const std::string& name) const; // 100511e0
	WxShaderMtlParam<D3DXMATRIX>* GetMatrixParam(const std::string& name); // 10051260
	const sTexture* GetTextureParam(const std::string& name) const; // 100512e0
	sTexture* GetTextureParam(const std::string& name); // 10051360
	void InitBasicParams(); // 10056370
};
// 983: nomes dos tipos de parâmetro (unidade wxshadermtl): "TYPE_BOOL"…; 9 = não achou
const char* WVDFxParamTypeName(int type); // 10049330
int WVDFxParamTypeFromName(const char* name); // 10049340
// 983: tabelas de nomes da unidade wvdshnames (F2.8; nomes provisórios)
const char* WVDShMacroName(int macro); // 10058350: "WVDSHMACRO_*"
int WVDShSysParamSemanticFromName(const char* name); // 100583c0: "WORLD"…; 13 = não achou
int WVDShMacroFromName(const char* name); // 10058360; 10 = não achou
const char* WVDShSysParamSemanticName(int semantic); // 100583b0
const char* WVDShUserParamSemanticName(int semantic); // 10058410
int WVDShUserParamSemanticFromName(const char* name); // 10058420: "WVDSHBASICUSERPARAMSEMANTIC_*"; 9 = não achou

#ifdef WANGREAL_DEVICE
// 983: tipos novos da interface (nomes provisórios). Offsets = 16 pares (x, y); Weights = 16 vetores de 4.
enum WVDSHSYSPARAMSEMANTICTYPE
{
	WVDSHSYSPARAM_FORCE_DWORD = 0x7FFFFFFF
};
struct WFxSampleOffsets
{
	float x, y;
};
struct WFxSampleWeights
{
	float x, y, z, w;
};

class WVideoDev : public WDevice
{
	friend class WView;

public:
	virtual ~WVideoDev() { }
	virtual wVDevState GetDeviceState() const = 0;
	virtual void SetMainThreadId(unsigned long id) { m_mainThreadId = id; }
	// 983: + nº de coordenadas de textura e material de shader (nomes provisórios); DrawIndexedTriangles é pura
	virtual void DrawPolygonFan(WTVertex**, int, int, int, unsigned long, int = 0,
		WxShaderMtl* = 0) = 0;
	virtual void DrawIndexedTriangles(WTVertex*, int, unsigned short*, int, int,
		int, int = 0, WxShaderMtl* = 0) = 0;
	virtual void DrawLine(WTVertex**, int) = 0;
	virtual int Command(wVDevMessage, int, int) = 0;
	virtual int CreateTexture(tagBITMAPINFO*, int, int = 0) = 0; // 983: + 1 argumento (repassado ao CreateTextureSurface)
	virtual void UpdateTexture(int, tagBITMAPINFO*, void*, unsigned long) = 0;
	virtual void DestroyTexture(int) = 0;
	virtual int UploadCompressedTexture(void*, unsigned int, int) = 0;
	virtual void UpdateCompressedTexture(int, void*, unsigned int, int) = 0;
	virtual void FixTexturePart(int, const tagRECT&, tagBITMAPINFO*, void*,
		int) = 0;
	virtual bool IsTextureFilled(int) const = 0;
	virtual int GetTextureWidth(int) const = 0;
	virtual int GetTextureHeight(int) const = 0;
	virtual void SetRenderTargetSizeInfo(int,
		const WRenderToTextureSizeInfo&) = 0;
	// 983: novo (nome provisório; ver out/F1.2/vtable_983.tsv)
	virtual void AddElapsedTime(float) = 0;
	virtual bool BeginScene() = 0;
	virtual void EndScene() = 0;
	virtual void Paint() = 0;
	virtual void SetGlobalRenderState(int, int, int) = 0;
	// 983: novo (nome provisório; ver out/F1.2/vtable_983.tsv)
	virtual void SetFxSampleOffsets(const WFxSampleOffsets*) = 0;
	virtual void SetFxSampleWeights(const WFxSampleWeights*) = 0;
	virtual void ClearFxSampleOffsets() = 0;
	virtual void ClearFxSampleWeights() = 0;
	virtual bool IsSupportVS() const = 0;
	virtual bool IsSupportPS() const = 0;
	virtual bool IsSupportMRT() const = 0;
	virtual bool IsSupportClipPlane() const = 0;
	virtual void Clear(unsigned long, int, float) = 0;
	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;
	// 983: novo (nome provisório; ver out/F1.2/vtable_983.tsv)
	virtual int GetBaseWidth() const = 0;
	virtual int GetBaseHeight() const = 0;
	virtual int GetBackBufferBpp() const = 0;
	virtual bool IsWindowed() const = 0;
	virtual bool IsFillScreenMode() const = 0;
	virtual float GetMonitorSupportFPS() const = 0;
	virtual bool SetFogEnable(bool) = 0;
	virtual void SetFogState(float, float, unsigned long) = 0;
	virtual WVideoDev* MakeClone(char*, HWND__*, int) = 0;
	virtual bool Reset(bool, int, int, int, int, int, long, int) = 0;
	// 983: novo (nome provisório; ver out/F1.2/vtable_983.tsv)
	virtual int GetShaderId(const char*) = 0;
	virtual int RegisterShader(const char*, const std::string&) = 0;
	virtual unsigned long VertexSize(unsigned long) = 0;
	virtual unsigned char xGetStride(int) = 0;
	virtual bool xHasVertexElem(unsigned long, wWxVertexElem) const = 0;
	virtual int xGetVertexElemOffset(unsigned long, wWxVertexElem) const = 0;
	virtual int xGetBlendWeightSize(unsigned long) const = 0;
	virtual unsigned long xDetermineFVF(int, int, int) = 0;
	// 983: sobrecarga nova (com o handle do shader); o MSVC a põe no slot antes da de 3 argumentos
	virtual unsigned long xDetermineFVF(int, int, int, int) = 0;
	virtual unsigned long xDetermineBufferUsage(unsigned long) = 0;
	virtual int xCreateVertexBuffer(int, unsigned long, unsigned long) = 0;
	virtual int xCreateIndexBuffer(int, unsigned long) = 0;
	virtual void xReleaseVertexBuffer(int) = 0;
	virtual void xReleaseIndexBuffer(int) = 0;
	virtual unsigned char* xLockVertexBuffer(int, unsigned int,
		unsigned int) = 0;
	virtual unsigned char* xLockIndexBuffer(int, unsigned int,
		unsigned int) = 0;
	virtual void xUnlockVertexBuffer(int) = 0;
	virtual void xUnlockIndexBuffer(int) = 0;
	virtual void xDrawIndexedTriangles(const WxViewState&,
		const WxBatchState&) = 0;
	virtual void xSetTransform(WVDTRANSFORMSTATETYPE, const WMatrix4&) = 0;
	virtual void xSetPrevViewTransform(const WMatrix4&) = 0;
	virtual void SetShaderSource(const char*) = 0;
	virtual void BeginUsingCustomRenderState() = 0;
	virtual void SetCustomRenderState(WVDRENDERSTATETYPE, unsigned long) = 0;
	virtual void SetCustomTextureStageState(unsigned long,
		WVDTEXTURESTAGESTATETYPE, unsigned long) = 0;
	virtual void SetCustomTransform(WVDTRANSFORMSTATETYPE, const WMatrix4&) = 0;
	virtual void SetCustomTexture(unsigned long, int) = 0;
	virtual void SetCustomClipPlane(unsigned long, const WPlane&) = 0;
	virtual void SetCustomSamplerState(unsigned long, WVDSAMPLERSTATETYPE,
		unsigned long) = 0;
	virtual void SetCustomFxMacro(unsigned long) = 0;
	virtual void SetCustomFxParamInt(WVDFXPARAMETERTYPE, int) = 0;
	virtual void SetCustomFxParamVector2(WVDFXPARAMETERTYPE,
		const WVector2D&) = 0;
	virtual void SetCustomFxParamVector3(WVDFXPARAMETERTYPE,
		const WVector&) = 0;
	virtual void SetCustomFxParamVector4(WVDFXPARAMETERTYPE,
		const WVector4&) = 0;
	virtual void SetCustomFxParamMatrix(WVDFXPARAMETERTYPE,
		const WMatrix4&) = 0;
	virtual void SetCustomFxParamTexture(WVDFXPARAMETERTYPE, int) = 0;
	// 983: novo (nome provisório; ver out/F1.2/vtable_983.tsv)
	virtual void SetCustomFxFlag(unsigned int) = 0;
	virtual void SetCustomShSysParamVector4(WVDSHSYSPARAMSEMANTICTYPE,
		const WVector4&) = 0;
	virtual void SetCustomShSysParamMatrix(WVDSHSYSPARAMSEMANTICTYPE,
		const WMatrix4&) = 0;
	virtual void EndUsingCustomRenderState() = 0;
	virtual bool BeginRenderToTexture(const WRenderToTextureParam&) = 0;
	virtual void EndRenderToTexture(const WRenderToTextureParam&) = 0;
	virtual bool SupportRenderTargetFormat() const = 0;
	virtual bool IsSupportedDisplayMode(bool, int, int, int) = 0;
	virtual bool GetWindowDisplayMode(int&, int&, int&) = 0;
	virtual int GetBufferingMeshNum() const = 0;
	virtual void SetViewPort(unsigned long, unsigned long, unsigned long,
		unsigned long) = 0;
	// 983: novo (nome provisório; ver out/F1.2/vtable_983.tsv)
	virtual void SetRimLight(bool) = 0;
	virtual void SetRimLightParam0(float) = 0;
	virtual void SetRimLightParam1(float) = 0;
	virtual void SetRimLightParam2(float) = 0;
	virtual void SetRimLightWidth(int) = 0;
	virtual bool IsRimLight() const = 0;

protected:
	unsigned long m_mainThreadId;
	unsigned long m_renderCount;

protected:
	float m_clip_scale_z;
	float m_clip_near_scale;
};

enum WVDFXMACROFLAG
{
	WVDFXMACRO_SKINNING = 0x1,
	WVDFXMACRO_SKINNING_WEIGHT1 = 0x2,
	WVDFXMACRO_SKINNING_WEIGHT2 = 0x4,
	WVDFXMACRO_SKINNING_WEIGHT3 = 0x8,
	WVDFXMACRO_TEXTURE = 0x10,
	WVDFXMACRO_NORMAL = 0x20,
	WVDFXMACRO_VERTEX_COLOR = 0x40,
	WVDFXMACRO_MATERIAL_COLOR = 0x80,
	WVDFXMACRO_TEXTURE_STAGE2 = 0x100,
	WVDFXMACRO_TEXTURE_STAGE2_ADD = 0x200,
	WVDFXMACRO_USE_CONSTCOLOR = 0x400,
	WVDFXMACRO_FOG = 0x800,
	WVDFXMACRO_NO_LIGHTING = 0x1000,
	WVDFXMACRO_POSTPROCESSING_DOF = 0x2000,
	WVDFXMACRO_SCENE_FXAA = 0x4000,
	WVDFXMACRO_POSTPROCESSING_MOTIONBLUR = 0x8000,
	WVDFXMACRO_OVERLAY = 0x10000,
	WVDFXMACRO_MRT_VELOCITY = 0x20000,
	WVDFXMACRO_SCENE_OFFSET_WEIGHT_SAMP = 0x40000,
	WVDFXMACRO_PROJ_TEXCOORD = 0x80000,
	WVDFXMACRO_SHADOW_CASTER = 0x100000,
	WVDFXMACRO_SHADOW_RECVER = 0x200000,
	WVDFXMACRO_SCENE_SCALE = 0x400000,
	WVDFXMACRO_SCENE_LUMI_AVG = 0x800000,
	WVDFXMACRO_SCENE_LUMI_AVG_SCALE = 0x1000000,
	WVDFXMACRO_POSTPROCESSING_CROSS_FILTER_TONE_MAP = 0x2000000,
	WVDFXMACRO_LUMI = 0x4000000,
	WVDFXMACRO_BLURRING = 0x8000000,
	WVDFXMACRO_RIMLIGHTING = 0x10000000,
	WVDFXMACRO_MRT_DOF_FACTOR = 0x20000000,
	WVDFXMACRO_SCENE_CALC_ADAPTATION = 0x40000000,
	WVDFXMACRO_STAR_CROSS = 0x80000000,
};

#else
class WVideoDev : public WDevice
{
	friend class WView;

public:
	virtual ~WVideoDev();
	virtual wVDevState GetDeviceState() = 0;
	virtual void SetMainThreadId(unsigned int threadId);
	// 983 (s44): + n� de coordenadas de textura e material de shader, como na dll (nomes provis�rios)
	virtual void DrawPolygonFan(WTVertex** p, int iType, int iNum, int iType2,
		unsigned int dwVertexTypeDesc, int nTexCoord983 = 0,
		WxShaderMtl* pShaderMtl983 = 0) = 0;
	virtual void DrawIndexedTriangles(WTVertex* p, int pNum,
		unsigned short* fList, int fNum, unsigned int iType,
		unsigned int iType2, int nTexCoord983 = 0,
		WxShaderMtl* pShaderMtl983 = 0);
	virtual void DrawLine(WTVertex** p, int type) = 0;
	virtual int Command(wVDevMessage message, int param1, int param2) = 0;
	virtual int CreateTexture(LPBITMAPINFO src, int type, int createParam983 = 0) = 0; // 983: + 1 argumento (como na dll)
	virtual void UpdateTexture(int texHandle, LPBITMAPINFO src, LPVOID data,
		unsigned int type) = 0;
	virtual void DestroyTexture(int texHandle) = 0;
	virtual int UploadCompressedTexture(void* pSrc, size_t srcSize,
		int type) = 0;
	virtual void UpdateCompressedTexture(int texHandle, void* pSrcData,
		size_t srcDataSize, int type) = 0;
	virtual void FixTexturePart(int texHandle, const RECT& rc, BITMAPINFO* src,
		void* data, int type) = 0;
	virtual bool IsTextureFilled(int handle) = 0;
	virtual int GetTextureWidth(int hTex) = 0;
	virtual int GetTextureHeight(int hTex) = 0;
	virtual void SetRenderTargetSizeInfo(int handle,
		const WRenderToTextureSizeInfo& sizeInfo) = 0;
	// 983 (s42): slot novo da vtable da dll, na ordem medida na F1.2 (out/F1.2/vtable_983.tsv); assinatura provis�ria no exe
	virtual void AddElapsedTime(float) = 0;
	virtual bool BeginScene() = 0;
	virtual void EndScene() = 0;
	virtual void Paint() = 0;
	virtual void SetGlobalRenderState(int rs, int rsEx, int flags983) = 0; // 983: 3 argumentos (005b130d; igual ao lado da dll)
	// 983 (s42): slot novo da vtable da dll, na ordem medida na F1.2 (out/F1.2/vtable_983.tsv); assinatura provis�ria no exe
	virtual void SetFxSampleOffsets(const void*) = 0;
	virtual void SetFxSampleWeights(const void*) = 0;
	virtual void ClearFxSampleOffsets() = 0;
	virtual void ClearFxSampleWeights() = 0;
	virtual bool IsSupportVs() = 0;
	virtual bool IsSupportPs() = 0;
	virtual bool IsSupportMrt() = 0;
	virtual bool IsSupportClipPlane() = 0;
	virtual void Clear(unsigned int color, int flags, float z) = 0;
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	// 983 (s42): slot novo da vtable da dll, na ordem medida na F1.2 (out/F1.2/vtable_983.tsv); assinatura provis�ria no exe
	virtual int GetBaseWidth() const = 0;
	virtual int GetBaseHeight() const = 0;
	virtual int GetBackBufferBpp() const = 0;
	virtual bool IsWindowed() = 0;
	virtual bool IsFillScreenMode() = 0;
	virtual float GetMonitorSupportFps() const = 0;
	virtual bool SetFogEnable(bool enable) = 0;
	virtual void SetFogState(float fogStart, float fogEnd,
		unsigned int fogColor) = 0;
	virtual WVideoDev* MakeClone(char* modeName, HWND hWnd, int iTnL) = 0;
	// 983: 8 argumentos como na dll (WDirect3D8::Reset; WDeviceManager::ResetVideoDevice 00b689a0 faz ret 0x20)
	virtual bool Reset(bool bWindowed, int bbWidth, int bbHeight, int iWidth,
		int iHeight, int iColor, long lWndStyle, int fillMode) = 0;
	// 983 (s42): slot novo da vtable da dll, na ordem medida na F1.2 (out/F1.2/vtable_983.tsv); assinatura provis�ria no exe
	virtual int GetShaderId(const char*) = 0;
	virtual int RegisterShader(const char*, const std::string&) = 0;

	virtual unsigned long VertexSize(unsigned long dwVertexTypeDesc) = 0;

	virtual BYTE xGetStride(int hVb) = 0;
	virtual bool xHasVertexElem(unsigned long fvf,
		wWxVertexElem elem) const = 0;
	virtual int xGetVertexElemOffset(unsigned long fvf,
		wWxVertexElem elem) const = 0;
	virtual int xGetBlendWeightSize(unsigned long fvf) const = 0;
	virtual unsigned long xDetermineFVF(int iDrawFlag, int iDrawFlag2,
		int iMaxBoneNum) = 0;
	// 983 (s42): sobrecarga nova; o MSVC a p�e no slot antes da de 3 argumentos
	virtual unsigned long xDetermineFVF(int, int, int, int) = 0;
	virtual unsigned long xDetermineBufferUsage(unsigned long fvf) = 0;
	virtual int xCreateVertexBuffer(int numVertices, unsigned long fvf,
		unsigned long dwUsage) = 0;
	virtual int xCreateIndexBuffer(int numIndices, unsigned long dwUsage) = 0;
	virtual void xReleaseVertexBuffer(int hVb) = 0;
	virtual void xReleaseIndexBuffer(int hIb) = 0;
	virtual unsigned char* xLockVertexBuffer(int hVb, unsigned int uiOffset,
		unsigned int uiSize) = 0;
	virtual unsigned char* xLockIndexBuffer(int hIb, unsigned int uiOffset,
		unsigned int uiSize) = 0;
	virtual void xUnlockVertexBuffer(int hVb) = 0;
	virtual void xUnlockIndexBuffer(int hIb) = 0;
	virtual void xDrawIndexedTriangles(const WxViewState& viewState,
		const WxBatchState& batchState) = 0;
	virtual void xSetTransform(WVDTRANSFORMSTATETYPE state,
		const WMatrix4& matrix) = 0;
	virtual void xSetPrevViewTransform(const WMatrix4& matrix) = 0;
	virtual void SetShaderSource(const char* shaderSrc) = 0;
	virtual void BeginUsingCustomRenderState() = 0;
	virtual void SetCustomRenderState(WVDRENDERSTATETYPE state,
		unsigned int value) = 0;
	virtual void SetCustomTextureStageState(unsigned int stage,
		WVDTEXTURESTAGESTATETYPE type, unsigned int value) = 0;
	virtual void SetCustomTransform(WVDTRANSFORMSTATETYPE state,
		const WMatrix4& matrix) = 0;
	virtual void SetCustomTexture(unsigned int stage, int hTex) = 0;
	virtual void SetCustomClipPlane(unsigned int index,
		const WPlane& value) = 0;
	virtual void SetCustomSamplerState(unsigned int sampler,
		WVDSAMPLERSTATETYPE type, unsigned int value) = 0;
	virtual void SetCustomFxMacro(unsigned int fxMacro) = 0;
	virtual void SetCustomFxParamInt(WFxParameterType paramType, int value) = 0;
	virtual void SetCustomFxParamVector2(WFxParameterType paramType,
		const WVector2D& value) = 0;
	virtual void SetCustomFxParamVector3(WFxParameterType paramType,
		const WVector& value) = 0;
	virtual void SetCustomFxParamVector4(WFxParameterType paramType,
		const WVector4& value) = 0;
	virtual void SetCustomFxParamMatrix(WFxParameterType paramType,
		const WMatrix4& value) = 0;
	virtual void SetCustomFxParamTexture(WFxParameterType paramType,
		int hTex) = 0;
	// 983 (s42): slot novo da vtable da dll, na ordem medida na F1.2 (out/F1.2/vtable_983.tsv); assinatura provis�ria no exe
	virtual void SetCustomFxFlag(unsigned int) = 0;
	virtual void SetCustomShSysParamVector4(int, const WVector4&) = 0;
	virtual void SetCustomShSysParamMatrix(int, const WMatrix4&) = 0;
	virtual void EndUsingCustomRenderState() = 0;
	virtual bool BeginRenderToTexture(WRenderToTextureParam* param) = 0;
	virtual void EndRenderToTexture(WRenderToTextureParam* param) = 0;
	virtual bool SupportRenderTargetFormat() = 0;
	virtual bool IsSupportedDisplayMode(bool bWindowed, int iWidth, int iHeight,
		int iColor) = 0;
	virtual bool GetWindowDisplayMode(int& iWidth, int& iHeight,
		int& iColor) = 0;
	virtual int GetBufferingMeshNum() const = 0;
	ulong GetRenderCount() { return m_renderCount; }
	void SetRenderCount(ulong count) { m_renderCount = count; }
	virtual void SetViewPort(unsigned int x, unsigned int y, unsigned int w,
		unsigned int h) = 0;
	// 983 (s42): slot novo da vtable da dll, na ordem medida na F1.2 (out/F1.2/vtable_983.tsv); assinatura provis�ria no exe
	virtual void SetRimLight(bool) = 0;
	virtual void SetRimLightParam0(float) = 0;
	virtual void SetRimLightParam1(float) = 0;
	virtual void SetRimLightParam2(float) = 0;
	virtual void SetRimLightWidth(int) = 0;
	virtual bool IsRimLight() const = 0;

protected:
	ulong m_mainThreadId;
	ulong m_renderCount;
	float m_clip_scale_z;
	float m_clip_near_scale;
};

class WInputDev : public WDevice
{
public:
	virtual ~WInputDev();
	virtual bool InitDevice(HWND window, bool active);
	virtual void SetActive(bool active);
	virtual void Reset();
	virtual void Update(unsigned long time);
	virtual int GetState(int type, int index);
	virtual unsigned long GetEventTime(int index);
	virtual bool IsAlphaNumericMode();
	virtual void SetAlphaNumericMode(bool enabled);
	virtual void SetAlphaNumericMode();
	virtual bool IsUpdated();
	virtual unsigned long GetLastInputTime();
	virtual void ResetInputTime();
	virtual void SetState(int type, int value);
	virtual void SetOpenStatus(bool open);
	virtual WInputDev* MakeClone(char* mode, HWND window);

protected:
	bool active;
	bool bUpdated;
	unsigned long dwLastInputTime;
};
#endif
