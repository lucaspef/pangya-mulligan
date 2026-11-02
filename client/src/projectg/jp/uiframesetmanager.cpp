// JP 983: unidade 006b30e0–006c75c0 (CUiFrameSetManager e as classes aninhadas; ver include/jp/uiframesetmanager.h). O mapa
// da F3.7.1 cortava a faixa em 9 unidades (elem_p2, point, rect, named, rect_p2, namedrect, namedpoint, frameset,
// uiframesetmanager): é uma só — os moldes `std::map`/`std::vector` das seis tabelas ocupam quase tudo (006b30e0–006b6bde e
// espalhados depois). Dentro dela também está o corpo do CCryptBuffer983 (006c6d20–006c6f40, include/jp/cryptbuffer.h).
// Sessão B59 (1º lote): leitura das medidas/pontos/retângulos, buscas, construtor/destrutor/Load/Clear do gerente e a cifra.
// Sessão B60 (2º lote): ImageInfo, Frame (menos os pedaços), FrameSet, LoadImageInfos/LoadFrameSets e o ResolveImageName.
// Sessão B61 (3º lote): Frame::LoadPieces 006c4630 + grade 006c3f60, medidas na tela (Dim/Point/Rect::Calc), desenho
// (Frame/FrameSet::Draw/DrawArc, DrawFrameSet 006b90b0) e recarga das imagens (Frame/FrameSet::Reload, ReloadImages983).
#include "minatl.h"
#include "projectg.h"
#include "tinyxml.h"
#include "engine/wresrcmng.h"
#include "engine/woverlay.h"
#include "engine/wview.h"
#include "engine/wmath.h"
#include "uiframesetmanager.h"
#include "cryptbuffer.h"
#include <wincrypt.h>

// campo do CSharedDoc do 983 sem nome no cabeçalho da A (bit 0 de +0x109bd: vento clássico)
#include "shareddoc.h"
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo) + offsetof(sRoomInfo, roomFlags983) == 0x109bd); // A741: era cru Doc()+0x109bd

extern const WRect g_rcZero983; // 00f14668 (0, 0, 0, 0)
extern const CUiFrameSetManager::Size g_sclOne983; // 00e424f8 (1, 1): <imgrect> em pixels da imagem

// 006b30e0
CUiFrameSetManager::Elem::Elem()
{
	m_bInvert = false;
}

// 006b30f0
bool CUiFrameSetManager::Elem::Init(TiXmlElement* pElem, VarMap* pVars)
{
	Reset();
	return Load(pElem, pVars) ? true : false;
}

// 006b3120
void CUiFrameSetManager::Point::Reset()
{
	m_pRef = NULL;
	m_x.Reset();
	m_y.Reset();
}

// 006b3150
void CUiFrameSetManager::Rect::Reset()
{
	m_pRef = NULL;
	m_pos.Reset();
	m_size.Reset();
}

// 006b4dd0
float CUiFrameSetManager::Dim::Calc(const Size& scl) const
{
	float ref = 0.0f;
	float sum = 0.0f;
	if (m_pRef)
	{
		ref = m_pRef->Calc(scl);
		if (m_bInvert)
			ref = (float)(ref * -1.0);
	}

	float value;
	switch (m_sclType)
	{
	case 1:
		value = m_size * scl.w;
		break;
	case 2:
		value = scl.h * m_size;
		break;
	case 3:
		if (Instance()->m_bA4)
			value = m_size * scl.w;
		else
			value = (float)floor(scl.w) * m_size;
		break;
	case 4:
		if (Instance()->m_bA4)
			value = scl.h * m_size;
		else
			value = (float)floor(scl.h) * m_size;
		break;
	default:
		value = m_size;
		break;
	}

	for (unsigned int i = 0; i < m_dims.size(); ++i)
		sum += m_dims[i].Calc(scl);
	return value + ref + sum;
}

// 006b4f20
void CUiFrameSetManager::Point::Calc(const Size& scl, WPoint& out) const
{
	out.x = 0.0f;
	out.y = 0.0f;
	if (m_pRef)
	{
		WPoint ref;
		m_pRef->Calc(scl, ref);
		if (m_bInvert)
		{
			out.x -= ref.x;
			out.y -= ref.y;
		}
		else
		{
			out.x += ref.x;
			out.y += ref.y;
		}
	}
	out.x += m_x.Calc(scl);
	out.y += m_y.Calc(scl);
}

// 006b4fa0
void CUiFrameSetManager::Rect::Calc(const Size& scl, WRect& out) const
{
	out = g_rcZero983;
	if (m_pRef)
	{
		WRect ref;
		m_pRef->Calc(scl, ref);
		if (m_bInvert)
		{
			out.x -= ref.x;
			out.y -= ref.y;
			out.w -= ref.w;
			out.h -= ref.h;
		}
		else
		{
			out.x += ref.x;
			out.y += ref.y;
			out.w += ref.w;
			out.h += ref.h;
		}
	}
	WPoint pos;
	WPoint size;
	m_pos.Calc(scl, pos);
	m_size.Calc(scl, size);
	out.x += pos.x;
	out.y += pos.y;
	out.w += size.x;
	out.h += size.y;
}

// 006b5090
bool CUiFrameSetManager::Rect::HitTest(const Size& scl, const WPoint& pt) const
{
	WRect rc;
	Calc(scl, rc);
	if (rc.x <= pt.x && pt.x < rc.x + rc.w && rc.y <= pt.y && pt.y < rc.y + rc.h)
		return true;
	return false;
}

// 006b5110
void CUiFrameSetManager::Frame::Draw(WView* pView, const Size& scl, int flags, unsigned long color)
{
	if (!m_pOverlay)
		return;
	if (!m_bFilter)
		flags |= 0x80000;
	flags |= (m_bWrapU ? 0 : 0x4000000) | (m_bWrapV ? 0 : 0x2000000);
	for (unsigned int i = 0; i < m_pieces.size(); ++i)
	{
		const Piece& piece = m_pieces[i];
		WRect src;
		piece.m_img.Calc(g_sclOne983, src);
		src.x /= m_size.w;
		src.y /= m_size.h;
		src.w /= m_size.w;
		src.h /= m_size.h;
		WRect dst;
		piece.m_scr.Calc(scl, dst);
		m_pOverlay->Render(pView, src, dst, flags, color, 0.0f, 0);
		if (m_num > 1)
		{
			WPoint step;
			m_spacing.Calc(scl, step);
			for (int n = 1; n < m_num; ++n)
			{
				dst.x += step.x;
				dst.y += step.y;
				m_pOverlay->Render(pView, src, dst, flags, color, 0.0f, 0);
			}
		}
	}
}

// 006b52b0
void CUiFrameSetManager::Frame::DrawArc(WView* pView, const Size& scl, float start, float end, int flags, unsigned long color)
{
	if (!m_pOverlay)
		return;
	if (!m_bFilter)
		flags |= 0x80000;
	flags |= (m_bWrapU ? 0 : 0x4000000) | (m_bWrapV ? 0 : 0x2000000);
	for (unsigned int i = 0; i < m_pieces.size(); ++i)
	{
		const Piece& piece = m_pieces[i];
		WRect src;
		piece.m_img.Calc(g_sclOne983, src);
		src.x /= m_size.w;
		src.y /= m_size.h;
		src.w /= m_size.w;
		src.h /= m_size.h;
		WRect dst;
		piece.m_scr.Calc(scl, dst);
		m_pOverlay->ArcClipRender(pView, src, dst, start, end, flags, color);
		if (m_num > 1)
		{
			WPoint step;
			m_spacing.Calc(scl, step);
			for (int n = 1; n < m_num; ++n)
			{
				dst.x += step.x;
				dst.y += step.y;
				m_pOverlay->ArcClipRender(pView, src, dst, start, end, flags, color);
			}
		}
	}
}

// 006b5460
void CUiFrameSetManager::FrameSet::Draw(WView* pView, const Size& scl, int flags, unsigned long color)
{
	flags &= 0xf9f7ffff;
	for (unsigned int i = 0; i < m_frames.size(); ++i)
		m_frames[i].Draw(pView, scl, flags, color);
}

// 006b54e0
void CUiFrameSetManager::FrameSet::DrawArc(WView* pView, const Size& scl, float start, float end, int flags, unsigned long color)
{
	flags &= 0xf9f7ffff;
	for (unsigned int i = 0; i < m_frames.size(); ++i)
		m_frames[i].DrawArc(pView, scl, start, end, flags, color);
}

// 006b5570
float CUiFrameSetManager::ScaleForView983(WView* pView, float value)
{
	float f;
	if (m_bA4)
	{
		f = 1.0f;
	}
	else
	{
		float h = (float)(pView->GetHeight() / 480.0);
		float whole = (float)floor(h);
		f = (float)((h - whole) / whole + 1.0);
	}
	return f * value;
}

// 006b77f0
void CUiFrameSetManager::Named::Reset()
{
	m_name.clear();
}

// 006b7810
void CUiFrameSetManager::NamedPoint::Reset()
{
	Named::Reset();
	Point::Reset();
}

// 006b7820
void CUiFrameSetManager::NamedRect::Reset()
{
	Named::Reset();
	Rect::Reset();
}

// 006b81c0
const std::string& CUiFrameSetManager::ImageInfo::GetImage(float scl) const
{
	if (!(Wabs((float)(scl - 1.0)) < g_EPSILON) && m_hires.size() != 0)
	{
		std::map<float, std::string>::const_iterator it = m_hires.begin();
		for (; it != m_hires.end(); ++it)
		{
			if (WisEqual(it->first, scl, g_EPSILON) || scl < it->first)
				return it->second;
		}
		std::map<float, std::string>::const_iterator last = m_hires.end();
		--last;
		return last->second;
	}
	return m_image;
}

// 006b8590
std::string CUiFrameSetManager::ResolveImageName(const std::string& name)
{
	if (name.size() == 0)
		return name;
	if (name[0] != '?')
		return name;
	if (strcmp(name.c_str() + 1, "wind_direction_bg_image") == 0)
	{
		if (Doc()->m_roomInfo.roomFlags983 & 1)
			return "[classic_right_wind_b.jpg";
		return "[right_wind_b.jpg";
	}
	if (strcmp(name.c_str() + 1, "wind_speed_bg_image") == 0)
	{
		if (Doc()->m_roomInfo.roomFlags983 & 1)
			return "[classic_right_wind_s.jpg";
		return "[right_wind_s.jpg";
	}
	return "";
}

// 006b8e70
CUiFrameSetManager::NamedDim* CUiFrameSetManager::FindNamedDimAny(const std::string& name)
{
	std::map<std::string, NamedDim>::iterator it = m_namedDims.find(name);
	if (it != m_namedDims.end())
		return &it->second;
	return NULL;
}

// 006b8ee0
CUiFrameSetManager::NamedDim* CUiFrameSetManager::FindNamedDim(const std::string& name)
{
	std::map<std::string, NamedDim>::iterator it = m_namedDims.find(name);
	if (it != m_namedDims.end())
		return &it->second;
	return NULL;
}

// 006b8f50
CUiFrameSetManager::NamedPoint* CUiFrameSetManager::FindNamedPoint(const std::string& name)
{
	std::map<std::string, NamedPoint>::iterator it = m_namedPoints.find(name);
	if (it != m_namedPoints.end())
		return &it->second;
	return NULL;
}

// 006b8fc0
CUiFrameSetManager::NamedRect* CUiFrameSetManager::FindNamedRect(const std::string& name)
{
	std::map<std::string, NamedRect>::iterator it = m_namedRects.find(name);
	if (it != m_namedRects.end())
		return &it->second;
	return NULL;
}

// 006b9030
CUiFrameSetManager::FrameSet* CUiFrameSetManager::FindFrameSet(const std::string& name)
{
	std::map<std::string, FrameSet>::iterator it = m_frameSets.find(name);
	if (it != m_frameSets.end())
		return &it->second;
	return NULL;
}

// 006b8d40
CUiFrameSetManager::Rect::~Rect()
{
}

// 006b90b0
void CUiFrameSetManager::DrawFrameSet(const std::string& name, WView* pView, int flags, unsigned long color)
{
	FrameSet* pSet = FindFrameSet(name);
	if (pSet)
	{
		Size scl((float)(pView->GetWidth() / 640.0), (float)(pView->GetHeight() / 480.0));
		pSet->Draw(pView, scl, flags, color);
	}
}

// 006b94f0
const CUiFrameSetManager::ImageInfo* CUiFrameSetManager::FindImageInfo(const std::string& name)
{
	std::map<std::string, ImageInfo>::iterator it = m_imageInfos.find(name);
	if (it != m_imageInfos.end())
		return &it->second;
	return NULL;
}

// 006b9560
CUiFrameSetManager::Named* CUiFrameSetManager::FindNamed(const std::string& name)
{
	Named* p = FindNamedDimAny(name);
	if (p)
		return p;
	p = FindNamedPoint(name);
	if (p)
		return p;
	p = FindNamedRect(name);
	if (p)
		return p;
	return FindFrameSet(name);
}

// 006b97b0
bool CUiFrameSetManager::ParseRef(std::string& name, bool& bInvert, VarMap* pVars)
{
	if (name.size() == 0)
		return true;

	if (name[0] == '-')
	{
		bInvert = true;
		name = name.substr(1, name.size() - 1);
	}
	else
	{
		bInvert = false;
	}

	if (pVars)
	{
		for (unsigned int open = name.find('['); open != std::string::npos; open = name.find('['))
		{
			unsigned int close = name.find(']', open + 1);
			if (close == std::string::npos)
				return false;
			unsigned int len = close - open;
			std::string var = name.substr(open + 1, len - 1);
			if (var.size() == 0)
				return false;
			VarMap::iterator it = pVars->find(var);
			if (it == pVars->end())
				return false;
			char buf[32];
			_itoa(it->second, buf, 10);
			name.replace(open, len + 1, buf);
		}
	}
	return true;
}

// 006b99d0
bool CUiFrameSetManager::Named::LoadName(TiXmlElement* pElem)
{
	m_name = pElem->Attribute("name");
	if (m_name.size() == 0 || m_name[0] == '-')
		return false;
	return true;
}

// 006b9a70
bool CUiFrameSetManager::Point::Load(TiXmlElement* pElem, VarMap* pVars)
{
	std::string ref = pElem->Attribute("namedpoint");
	if (ref.size() != 0)
	{
		if (!ParseRef(ref, m_bInvert, pVars))
			return false;
		m_pRef = Instance()->FindNamedPoint(ref);
		if (!m_pRef)
			return false;
	}
	TiXmlElement* pX = pElem->FirstChildElement("x");
	if (pX && !m_x.Init(pX, pVars))
		return false;
	TiXmlElement* pY = pElem->FirstChildElement("y");
	if (pY && !m_y.Init(pY, pVars))
		return false;
	return true;
}

// 006b9bb0
bool CUiFrameSetManager::NamedPoint::Load(TiXmlElement* pElem, VarMap* pVars)
{
	if (!LoadName(pElem))
		return false;
	return Point::Load(pElem, NULL) ? true : false;
}

// 006b9bf0
bool CUiFrameSetManager::Rect::Load(TiXmlElement* pElem, VarMap* pVars)
{
	std::string ref = pElem->Attribute("namedrect");
	if (ref.size() != 0)
	{
		if (!ParseRef(ref, m_bInvert, pVars))
			return false;
		m_pRef = Instance()->FindNamedRect(ref);
		if (!m_pRef)
			return false;
	}
	TiXmlElement* pPos = pElem->FirstChildElement("pos");
	if (pPos && !m_pos.Init(pPos, pVars))
		return false;
	TiXmlElement* pSize = pElem->FirstChildElement("size");
	if (pSize && !m_size.Init(pSize, pVars))
		return false;
	return true;
}

// 006b9d30
bool CUiFrameSetManager::NamedRect::Load(TiXmlElement* pElem, VarMap* pVars)
{
	if (!LoadName(pElem))
		return false;
	return Rect::Load(pElem, NULL) ? true : false;
}

// 006beeb0
WOverlay* CUiFrameSetManager::GetOverlay(const std::string& name)
{
	std::map<std::string, WOverlay*>::iterator it = m_overlays.find(name);
	if (it != m_overlays.end())
		return it->second;
	WOverlay* pOverlay = g_resrcmng->GetOverlay(name.c_str(), 0x80000000);
	if (!pOverlay)
		return NULL;
	m_overlays[name] = pOverlay;
	return pOverlay;
}

// 006bf890
bool CUiFrameSetManager::ImageInfo::Load(TiXmlElement* pElem)
{
	m_image = pElem->Attribute("image");
	m_w = (float)atof(pElem->Attribute("w").c_str());
	m_h = (float)atof(pElem->Attribute("h").c_str());
	if (m_image.size() == 0 || m_w <= 0.0f || m_h <= 0.0f)
		return false;

	TiXmlElement* p = pElem->FirstChildElement("hiresimage");
	while (p)
	{
		std::string scl;
		std::string image;
		scl = p->Attribute("scl");
		image = p->Attribute("image");
		float s = (float)atof(scl.c_str());
		if (s < 1.0f || image.size() == 0 || m_hires.find(s) != m_hires.end())
			return false;
		m_hires.insert(std::pair<float, std::string>(s, image));
		p = p->NextSiblingElement("hiresimage");
	}
	return true;
}

// 006c0360
CUiFrameSetManager::ImageInfo::ImageInfo()
{
	m_w = 256.0f;
	m_h = 256.0f;
}

// 006c1ae0
void CUiFrameSetManager::Dim::Reset()
{
	m_pRef = NULL;
	m_size = 0.0f;
	m_sclType = 0;
	m_dims.clear();
}

// 006c1b10
void CUiFrameSetManager::NamedDim::Reset()
{
	Named::Reset();
	Dim::Reset();
}

// 006c2310
CUiFrameSetManager::Dim::Dim()
{
	Reset();
}

// 006c2370
bool CUiFrameSetManager::Dim::Load(TiXmlElement* pElem, VarMap* pVars)
{
	std::string ref = pElem->Attribute("nameddim");
	if (ref.size() != 0)
	{
		if (!ParseRef(ref, m_bInvert, pVars))
			return false;
		m_pRef = Instance()->FindNamedDim(ref);
		if (!m_pRef)
			return false;
	}

	std::string size = pElem->Attribute("size");
	std::string sclType = pElem->Attribute("scltype");
	m_size = size.size() != 0 ? (float)atof(size.c_str()) : 0.0f;
	m_sclType = sclType.size() != 0 ? atoi(sclType.c_str()) : 0;
	if (m_sclType < 0 || m_sclType >= 5)
		return false;

	int count = 0;
	for (TiXmlElement* p = pElem->FirstChildElement("dim"); p; p = p->NextSiblingElement("dim"))
		++count;
	m_dims.reserve(count);
	for (TiXmlElement* p = pElem->FirstChildElement("dim"); p; p = p->NextSiblingElement("dim"))
	{
		m_dims.push_back(Dim());
		if (!m_dims.back().Init(p, pVars))
		{
			m_dims.pop_back();
			return false;
		}
	}
	return true;
}

// 006c26d0
bool CUiFrameSetManager::NamedDim::Load(TiXmlElement* pElem, VarMap* pVars)
{
	if (!LoadName(pElem))
		return false;
	return Dim::Load(pElem, NULL) ? true : false;
}

// 006c2710
CUiFrameSetManager::Point::Point()
{
	Reset();
}

// 006c2780
CUiFrameSetManager::Rect::Rect()
{
	Reset();
}

// 006c27f0
void CUiFrameSetManager::Frame::Reset()
{
	m_image.clear();
	m_size = Size(256.0f, 256.0f);
	m_bFilter = false;
	m_sclType = 0;
	m_bWrapU = false;
	m_bWrapV = false;
	m_pieces.clear();
	m_num = 1;
	m_spacing.Reset();
	m_scale = 1.0f;
	m_pOverlay = NULL;
}

// 006c2870
bool CUiFrameSetManager::Frame::Reload(WView* pView)
{
	Dim dim;
	dim.Set(1.0f, m_sclType);
	float scale = dim.Calc(Size((float)(pView->GetWidth() / 640.0), (float)(pView->GetHeight() / 480.0)));
	if (m_pOverlay && m_scale == scale)
	{
		if (m_image[0] == '?')
		{
			std::string name = Instance()->ResolveImageName(m_image);
			if (m_image.compare(name) == 0)
				return m_pOverlay != NULL;
			const ImageInfo* pInfo = Instance()->FindImageInfo(name);
			if (!pInfo)
				return false;
			m_pOverlay = Instance()->GetOverlay(pInfo->GetImage(m_scale));
		}
		return m_pOverlay != NULL;
	}

	m_pOverlay = NULL;
	m_scale = scale;
	const ImageInfo* pInfo;
	if (m_image[0] == '?')
		pInfo = Instance()->FindImageInfo(Instance()->ResolveImageName(m_image));
	else
		pInfo = Instance()->FindImageInfo(m_image);
	if (!pInfo)
		return false;
	m_pOverlay = Instance()->GetOverlay(pInfo->GetImage(m_scale));
	return m_pOverlay != NULL;
}

// 006c2ae0
bool CUiFrameSetManager::FrameSet::Reload(WView* pView)
{
	for (unsigned int i = 0; i < m_frames.size(); ++i)
	{
		if (!m_frames[i].Reload(pView))
			return false;
	}
	return true;
}

// 006c2b50
bool CUiFrameSetManager::ReloadImages983(WView* pView)
{
	for (std::map<std::string, FrameSet>::iterator it = m_frameSets.begin(); it != m_frameSets.end(); ++it)
	{
		if (!it->second.Reload(pView))
			return false;
	}
	return true;
}

// 006c3000
CUiFrameSetManager::Frame::Frame()
{
	Reset();
}

// 006c3070
void CUiFrameSetManager::SetA4983(bool bA4, WView* pView)
{
	if (m_bA4 != bA4)
	{
		m_bA4 = bA4;
		ReloadImages983(pView);
	}
}

// 006c30a0
bool CUiFrameSetManager::LoadImageInfos(TiXmlElement* pLayout)
{
	TiXmlElement* p = pLayout->FirstChildElement("imageinfo");
	while (p)
	{
		ImageInfo value;
		if (!value.Load(p) || FindImageInfo(value.m_image))
			return false;
		m_imageInfos.insert(std::pair<std::string, ImageInfo>(value.m_image, value));
		p = p->NextSiblingElement("imageinfo");
	}
	return true;
}

// 006c3280
bool CUiFrameSetManager::LoadNamedDims(TiXmlElement* pLayout)
{
	TiXmlElement* p = pLayout->FirstChildElement("nameddim");
	while (p)
	{
		NamedDim value;
		if (!value.Init(p, NULL) || FindNamed(value.GetName()))
			return false;
		m_namedDims.insert(std::pair<std::string, NamedDim>(value.GetName(), value));
		p = p->NextSiblingElement("nameddim");
	}
	return true;
}

// 006c3540
bool CUiFrameSetManager::LoadNamedPoints(TiXmlElement* pLayout)
{
	TiXmlElement* p = pLayout->FirstChildElement("namedpoint");
	while (p)
	{
		NamedPoint value;
		if (!value.Init(p, NULL) || FindNamed(value.GetName()))
			return false;
		m_namedPoints.insert(std::pair<std::string, NamedPoint>(value.GetName(), value));
		p = p->NextSiblingElement("namedpoint");
	}
	return true;
}

// 006c3980
bool CUiFrameSetManager::LoadNamedRects(TiXmlElement* pLayout)
{
	TiXmlElement* p = pLayout->FirstChildElement("namedrect");
	while (p)
	{
		NamedRect value;
		if (!value.Init(p, NULL) || FindNamed(value.GetName()))
			return false;
		m_namedRects.insert(std::pair<std::string, NamedRect>(value.GetName(), value));
		p = p->NextSiblingElement("namedrect");
	}
	return true;
}

// 006c3f60
bool CUiFrameSetManager::Frame::LoadGrid(Grid& grid, unsigned int level)
{
	if (level + 1 == grid.m_seqs.size())
	{
		for (unsigned int i = 0; i < grid.m_seqs[level].m_num; ++i)
		{
			grid.m_vars[grid.m_seqs[level].m_name] = i;
			m_pieces.push_back(Piece());
			Piece& piece = m_pieces.back();
			TiXmlElement* pImg = grid.m_pElem->FirstChildElement("imgrect");
			if (pImg)
			{
				if (!piece.m_img.Init(pImg, &grid.m_vars))
				{
					m_pieces.pop_back();
					return false;
				}
			}
			else
			{
				piece.m_img.Set(m_size);
			}
			TiXmlElement* pScr = grid.m_pElem->FirstChildElement("scrrect");
			if (!pScr || !piece.m_scr.Init(pScr, &grid.m_vars))
			{
				m_pieces.pop_back();
				return false;
			}
		}
	}
	else
	{
		for (unsigned int i = 0; i < grid.m_seqs[level].m_num; ++i)
		{
			grid.m_vars[grid.m_seqs[level].m_name] = i;
			if (!LoadGrid(grid, level + 1))
				return false;
		}
	}
	return true;
}

// 006c4630
bool CUiFrameSetManager::Frame::LoadPieces(TiXmlElement* pElem)
{
	unsigned int count = 0;
	for (TiXmlElement* p = pElem->FirstChildElement("seq"); p; p = p->NextSiblingElement("seq"))
		++count;
	if (count != 0)
	{
		Grid grid(pElem);
		unsigned int total = 1;
		grid.m_seqs.reserve(count);
		TiXmlElement* p = pElem->FirstChildElement("seq");
		while (p)
		{
			Seq seq;
			seq.m_name = p->Attribute("name");
			seq.m_num = atoi(p->Attribute("num").c_str());
			if (seq.m_name.size() == 0 || seq.m_num <= 1)
				return false;
			total *= seq.m_num;
			grid.m_seqs.push_back(seq);
			p = p->NextSiblingElement("seq");
		}
		m_pieces.reserve(total);
		if (!LoadGrid(grid, 0))
			return false;
		return true;
	}

	Piece piece;
	TiXmlElement* pImg = pElem->FirstChildElement("imgrect");
	if (pImg)
	{
		if (!piece.m_img.Init(pImg, NULL))
			return false;
	}
	else
	{
		piece.m_img.Set(m_size);
	}
	TiXmlElement* pScr = pElem->FirstChildElement("scrrect");
	if (!pScr || !piece.m_scr.Init(pScr, NULL))
		return false;
	m_pieces.push_back(piece);
	return true;
}

// 006c4ef0
bool CUiFrameSetManager::Frame::Load(TiXmlElement* pElem)
{
	Reset();
	m_image = pElem->Attribute("image");
	if (m_image.size() == 0)
		return false;

	const ImageInfo* pInfo;
	if (m_image[0] == '?')
		pInfo = Instance()->FindImageInfo(Instance()->ResolveImageName(m_image));
	else
		pInfo = Instance()->FindImageInfo(m_image);
	if (!pInfo)
		return false;
	m_size.w = pInfo->m_w;
	m_size.h = pInfo->m_h;

	std::string filter = pElem->Attribute("filter");
	std::string wrapU = pElem->Attribute("wrap_u");
	std::string wrapV = pElem->Attribute("wrap_v");
	std::string sclType = pElem->Attribute("scltypeforhiresimage");
	if (filter.size() != 0)
		m_bFilter = atoi(filter.c_str()) != 0;
	if (wrapU.size() != 0)
		m_bWrapU = atoi(wrapU.c_str()) != 0;
	if (wrapV.size() != 0)
		m_bWrapV = atoi(wrapV.c_str()) != 0;
	if (sclType.size() != 0)
		m_sclType = atoi(sclType.c_str());
	if (!LoadPieces(pElem))
		return false;

	TiXmlElement* pSpacing = pElem->FirstChildElement("spacing");
	if (pSpacing)
	{
		std::string num = pSpacing->Attribute("num");
		m_num = atoi(num.c_str());
		if (m_num <= 1 || !m_spacing.Init(pSpacing, NULL))
			return false;
	}
	return true;
}

// 006c52a0
bool CUiFrameSetManager::FrameSet::Load(TiXmlElement* pElem, VarMap* pVars)
{
	if (!LoadName(pElem))
		return false;

	int count = 0;
	for (TiXmlElement* p = pElem->FirstChildElement("frame"); p; p = p->NextSiblingElement("frame"))
		++count;
	m_frames.reserve(count);
	for (TiXmlElement* p = pElem->FirstChildElement("frame"); p; p = p->NextSiblingElement("frame"))
	{
		m_frames.push_back(Frame());
		if (!m_frames.back().Load(p))
		{
			m_frames.pop_back();
			return false;
		}
	}
	return true;
}

// 006c5670
void CUiFrameSetManager::FrameSet::Reset()
{
	Named::Reset();
	m_frames.clear();
}

// 006c57b0
CUiFrameSetManager::FrameSet::FrameSet()
{
	Reset();
}

// 006c6140
void CUiFrameSetManager::Clear()
{
	m_filename.clear();
	m_imageInfos.clear();
	m_namedDims.clear();
	m_namedPoints.clear();
	m_namedRects.clear();
	m_frameSets.clear();
	for (std::map<std::string, WOverlay*>::iterator it = m_overlays.begin(); it != m_overlays.end(); ++it)
		g_resrcmng->Release(it->second);
	m_overlays.clear();
}

// 006c6240
bool CUiFrameSetManager::LoadFrameSets(TiXmlElement* pLayout)
{
	TiXmlElement* p = pLayout->FirstChildElement("frameset");
	while (p)
	{
		FrameSet value;
		if (!value.Init(p, NULL) || FindNamed(value.GetName()))
			return false;
		m_frameSets.insert(std::pair<std::string, FrameSet>(value.GetName(), value));
		p = p->NextSiblingElement("frameset");
	}
	return true;
}

// 006c6570
bool CUiFrameSetManager::Load(const char* filename)
{
	Clear();
	m_filename = filename;

	TiXmlDocument doc;
	doc.LoadFileEx(m_filename.c_str());
	if (doc.Error())
		return false;
	TiXmlElement* pLayout = doc.FirstChildElement("layout");
	if (!pLayout)
		return false;
	if (!LoadImageInfos(pLayout))
		return false;
	if (!LoadNamedDims(pLayout))
		return false;
	if (!LoadNamedPoints(pLayout))
		return false;
	if (!LoadNamedRects(pLayout))
		return false;
	if (!LoadFrameSets(pLayout))
		return false;
	return true;
}

// 006c66b0
bool CUiFrameSetManager::Reload(WView* pView)
{
	std::string filename = m_filename;
	return Load(filename.c_str()) && ReloadImages983(pView);
}

// 006c6820
CUiFrameSetManager::~CUiFrameSetManager()
{
	Clear();
}

// 006c6a10
CUiFrameSetManager::CUiFrameSetManager()
{
	m_bA4 = false;
	Load("uiframeset.xml");
}

// 006c6d20
CCryptBuffer983::CCryptBuffer983(const sCryptParam983* param, const char* password)
{
	m_hProv = 0;
	m_hKey = 0;
	m_pData = NULL;
	m_size = 0;
	m_bOk = false;
	if (CryptAcquireContextA(&m_hProv, NULL, param->provider, param->provType, CRYPT_VERIFYCONTEXT) != TRUE)
		return;
	HCRYPTHASH hHash = 0;
	if (CryptCreateHash(m_hProv, param->hashAlg, 0, 0, &hHash) == TRUE
		&& CryptHashData(hHash, (const BYTE*)password, strlen(password), 0) == TRUE
		&& CryptDeriveKey(m_hProv, param->cipherAlg, hHash, param->keyFlags, &m_hKey) == TRUE)
		m_bOk = true;
	if (hHash && !CryptDestroyHash(hHash))
		GetLastError();
}

// 006c6de0
CCryptBuffer983::~CCryptBuffer983()
{
	if (m_pData)
	{
		delete[] m_pData;
		m_pData = NULL;
	}
	if (m_hKey && !CryptDestroyKey(m_hKey))
		GetLastError();
	if (m_hProv && !CryptReleaseContext(m_hProv, 0))
		GetLastError();
}

// 006c6e30
void CCryptBuffer983::Encrypt(const void* data, unsigned int size)
{
	m_bOk = false;
	m_size = size;
	if (!CryptEncrypt(m_hKey, 0, TRUE, 0, NULL, (DWORD*)&m_size, 0))
		return;
	m_pData = new unsigned char[m_size];
	memcpy(m_pData, data, size);
	if (!CryptEncrypt(m_hKey, 0, TRUE, 0, m_pData, (DWORD*)&size, m_size))
	{
		if (m_pData)
		{
			delete[] m_pData;
			m_pData = NULL;
		}
		m_size = 0;
		return;
	}
	m_bOk = true;
}

// 006c6ed0
void CCryptBuffer983::Decrypt(const void* data, unsigned int size)
{
	m_bOk = false;
	m_size = size;
	m_pData = new unsigned char[size];
	memcpy(m_pData, data, m_size);
	if (!CryptDecrypt(m_hKey, 0, TRUE, 0, m_pData, (DWORD*)&m_size))
	{
		if (m_pData)
		{
			delete[] m_pData;
			m_pData = NULL;
		}
		m_size = 0;
		return;
	}
	m_bOk = true;
}
