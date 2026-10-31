// JP 983: unidade 004b0df0–004b4330 (CControlledObject). Fonte reconstruída do 983 (classe nova, sem par no 645). O resto da
// faixa são instâncias da STL (vector, map, function, lambdas). O gravador de estado fica em controlledrecord.cpp.
#include "controlledobject.h"
#include "controller.h"
#include "controllerdoc.h"
#include <algorithm>

using _gateway::_controller::CControllerDoc;

static CControllerDoc* s_pDoc; // 00e74c98
static bool s_bActive; // 00e74c9c

CControllerDoc* CControlledObject::GetDoc()
{
	return s_pDoc;
}

void CControlledObject::SetDoc(CControllerDoc* pDoc)
{
	s_pDoc = pDoc;
}

void CControlledObject::SetActive983(bool bActive)
{
	s_bActive = bActive;
}

bool CControlledObject::IsActive983()
{
	return s_bActive;
}

void CControlledObject::Snapshot()
{
	if (s_bActive == true)
		return;
	m_record.Snapshot();
}

void CControlledObject::Save(FILE* fp)
{
	fwrite(&m_ability, sizeof(m_ability), 1, fp);
	m_record.Write(fp);
}

void CControlledObject::Load(FILE* fp)
{
	m_record.Read(fp);
}

void CControlledObject::Fire04()
{
	std::for_each(m_controllers.begin(), m_controllers.end(), [this](IController* pController)
	{
		pController->SetControlled(this);
		pController->On04();
	});
}

void CControlledObject::Fire05()
{
	m_record.Clear();
	std::for_each(m_controllers.begin(), m_controllers.end(), [this](IController* pController)
	{
		pController->SetControlled(this);
		pController->On05();
	});
}

void CControlledObject::Fire06(const float& value)
{
	std::for_each(m_controllers.begin(), m_controllers.end(), [value, this](IController* pController)
	{
		pController->SetControlled(this);
		pController->On06(value);
	});
}

void CControlledObject::Fire12()
{
	std::for_each(m_controllers.begin(), m_controllers.end(), [this](IController* pController)
	{
		pController->SetControlled(this);
		pController->On12();
	});
}

void CControlledObject::Fire07(int arg)
{
	std::for_each(m_controllers.begin(), m_controllers.end(), [arg, this](IController* pController)
	{
		pController->SetControlled(this);
		pController->On07(arg);
	});
}

void CControlledObject::Fire08()
{
	std::for_each(m_controllers.begin(), m_controllers.end(), [this](IController* pController)
	{
		pController->SetControlled(this);
		pController->On08();
	});
}

void CControlledObject::Fire10(int arg)
{
	std::for_each(m_controllers.begin(), m_controllers.end(), [arg, this](IController* pController)
	{
		pController->SetControlled(this);
		pController->On10(arg);
	});
}

void CControlledObject::Fire13()
{
	std::for_each(m_controllers.begin(), m_controllers.end(), [this](IController* pController)
	{
		pController->SetControlled(this);
		pController->On13();
	});
}

void CControlledObject::Fire09()
{
	std::for_each(m_controllers.begin(), m_controllers.end(), [this](IController* pController)
	{
		pController->SetControlled(this);
		pController->On09();
	});
}

void CControlledObject::Fire11()
{
	std::for_each(m_controllers.begin(), m_controllers.end(), [this](IController* pController)
	{
		pController->SetControlled(this);
		pController->On11();
	});
}

void CControlledObject::Attach(IController* pController, int arg)
{
	if (std::find(m_controllers.begin(), m_controllers.end(), pController) == m_controllers.end())
	{
		pController->Set0c(arg);
		m_controllers.push_back(pController);
	}
}

void CControlledObject::On04(int arg)
{
	CControllerDoc* pDoc = GetDoc();
	std::vector<std::string>::iterator it, end;
	for (it = m_controllerNames.begin(), end = m_controllerNames.end(); it != end; ++it)
	{
		IController* pController = pDoc->Find(*it);
		if (pController != NULL)
			Attach(pController, arg);
	}
}

void CControlledObject::RemoveController(const char* name)
{
	std::vector<std::string>::iterator it = std::find(m_controllerNames.begin(), m_controllerNames.end(), std::string(name));
	if (it != m_controllerNames.end())
		m_controllerNames.erase(it);
}

CControlledObject::CControlledObject(eSpecialAbility ability)
	: m_28(false)
	, m_ability(ability)
{
}

CControlledObject::~CControlledObject()
{
}

std::map<eSpecialAbility, CControlledObject::Creator>& CControlledObject::GetCreators()
{
	static std::map<eSpecialAbility, Creator> s_creators;
	return s_creators;
}

void CControlledObject::AddController(const char* name)
{
	m_controllerNames.push_back(std::string(name));
}

CControlledObject* CControlledObject::Create(eSpecialAbility ability)
{
	std::map<eSpecialAbility, Creator>& creators = GetCreators();
	std::map<eSpecialAbility, Creator>::iterator it = creators.find(ability);
	if (it == creators.end())
		return NULL;
	Creator creator = it->second;
	return creator(ability);
}

bool CControlledObject::Register(eSpecialAbility ability, Creator creator)
{
	std::map<eSpecialAbility, Creator>& creators = GetCreators();
	std::map<eSpecialAbility, Creator>::iterator it = creators.find(ability);
	if (it != creators.end())
		return false;
	creators.insert(std::make_pair(ability, creator));
	return true;
}
