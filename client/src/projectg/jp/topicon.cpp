// JP 983: unidade 006b12a0–006b1a50 (CTopIcon e CWebIcon do 983, base dos ícones da área de trabalho do lobby; ver
// include/jp/topicon.h). A faixa fica no meio do corte da trajectorypreviewcontroller da F3.7.1 (006b0fec–006b1d30; as
// funções de 006b0ff0–006b1290 são de outro dono, não levantado). Os métodos do CWebIcon que não são construtor nem
// destrutor estão na unidade eventicon (00b16a70–00b16bf0). Sessão B56. Ficam fora os moldes `basic_string::rfind`
// (006b1210, 006b1360).
#include "minatl.h"
#include "projectg.h"
#include "frbutton.h"
#include "tinyxml.h"
#include "extraresource.h"
#include "topicon.h"

// tipos do elemento no XML do tema (ponteiros graváveis em 00e15e00…00e15e08)
const char* g_iconTypeImage983 = "image";
const char* g_iconTypeWeb983 = "weblink";
const char* g_iconTypeEvent983 = "event";

// 006b12a0
void CTopIcon::SetIconImage(FrButton& button)
{
	if (m_normal.size() > 0)
	{
		button.SetStyle(FrButton::BT_NORMAL);
		button.SetButtonImg(m_normal.c_str(), FrButton::NORMAL);
	}
	if (m_over.size() > 0)
	{
		button.SetStyle(FrButton::BT_3TYPES);
		button.SetButtonImg(m_over.c_str(), FrButton::OVER);
	}
	if (m_selected.size() > 0)
	{
		button.SetStyle(FrButton::BT_3TYPES);
		button.SetButtonImg(m_selected.c_str(), FrButton::PRESSED);
	}
	if (m_blink.size() > 0)
		button.SetButtonImg(m_blink.c_str(), FrButton::BLINK);
	else
		button.EnableBlink(false);
	m_pButton = &button;
}

// 006b1390
CTopIcon::CTopIcon()
{
	m_pButton = NULL;
}

// 006b13e0
CTopIcon::~CTopIcon()
{
}

// 006b1490
void CTopIcon::StripExt983(std::string& file)
{
	size_t pos = file.rfind(".");
	if (pos != std::string::npos)
		file.erase(pos);
}

// 006b15c0
CTopIcon* CTopIcon::Create(TiXmlElement* pElem)
{
	std::string type = pElem->Value();
	CTopIcon* pIcon;
	if (type == g_iconTypeEvent983)
		pIcon = new CEventIcon;
	else if (type == g_iconTypeWeb983)
		pIcon = new CWebIcon;
	else if (type == g_iconTypeImage983)
		pIcon = new CDescImageOnlyIcon;
	else
		return NULL;

	if (!pIcon->Load(pElem))
	{
		delete pIcon;
		pIcon = NULL;
	}
	return pIcon;
}

// 006b1830
bool CTopIcon::Load(TiXmlElement* pElem)
{
	m_name = pElem->Attribute("name");
	if (TiXmlElement* pFile = pElem->FirstChildElement("normal"))
		m_normal = CExtraResource::Instance()->GetFile(pFile, true);
	if (TiXmlElement* pFile = pElem->FirstChildElement("over"))
		m_over = CExtraResource::Instance()->GetFile(pFile, true);
	if (TiXmlElement* pFile = pElem->FirstChildElement("selected"))
		m_selected = CExtraResource::Instance()->GetFile(pFile, true);
	if (TiXmlElement* pFile = pElem->FirstChildElement("blink"))
		m_blink = CExtraResource::Instance()->GetFile(pFile, true);

	StripExt983(m_name);
	StripExt983(m_normal);
	StripExt983(m_over);
	StripExt983(m_selected);
	StripExt983(m_blink);
	return m_normal != "";
}
