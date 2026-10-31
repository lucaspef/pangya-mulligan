#define REBANG_FXSEQUENCE_CPP
#define REBANG_FXBOX_CPP
#include "minatl.h"
#include "mathconsts.h"
#include "binstr.h"
#include "fxsequence.h"
#include "fxbox.h"
#include "vc10_layout.h"
#include "clientsetting.h"

CFxBox::CFxBox()
{
	m_pVertex[0] = &m_vertex[0];
	m_pVertex[1] = &m_vertex[1];
	m_pVertex[2] = &m_vertex[2];
	m_pVertex[3] = &m_vertex[3];
	m_pResMng = NULL;
	m_bStop = false;
	m_pad983_114 = 0;
	m_frameTime = 33.333332f;
	m_bLimitElapsed = true;
	m_timeScale = 1.0f;
	m_bPause = false;
	m_displayCount = 0;
	m_displayCount2 = 0;
	m_pad983_124 = false;
	m_pCamera = NULL;
	m_groundHeightMax = 99.0f;
	m_groundHeightMin = -99.0f;
	WTVertex* tmp = m_pVertex[2];
	m_pVertex[2] = m_pVertex[3];
	m_pVertex[3] = tmp;
	m_numVertex = 0;

	for (int i = 0; i < 4; i++)
	{
		m_vertex[i].tu = (i & 2) ? 1.0f : 0.0f;
		m_vertex[i].tv = (i & 1) ? 0.0f : 1.0f;
		m_vertex[i].diffuse = 0xffffffff;
	}
}

CFxBox::~CFxBox()
{
	ClearFxKeeper();
	Clear();
}

void CFxBox::Clear()
{
	ClearAll(true);
	ClearSprite();
	ClearOriginalSpray();
	ClearOriginalSequence();
}

void CFxBox::Init(WResourceManager* pResMng)
{
	m_pResMng = pResMng;
}

void CFxBox::SetCamera(const WMatrix* camera)
{
	m_pCamera = camera;
}

void CFxBox::ClearSprite()
{
	for (std::map<std::string, W3dAniSpr*>::iterator it = m_sprite.begin();
		it != m_sprite.end(); ++it)
	{
		if (m_pResMng && (*it).second)
		{
			m_pResMng->Release((*it).second);
			(*it).second = NULL;
		}
	}

	m_sprite.clear();
}

void CFxBox::ClearOriginalSpray()
{
	std::map<std::string, CFxSpray*>::iterator it = m_sprayCache.begin();
	for (; it != m_sprayCache.end(); ++it)
	{
		CFxSpray* spray = (*it).second;
		if (g_resrcmng != NULL && spray->m_param.m_bandana != 0)
		{
			g_resrcmng->Release(spray->m_param.m_bandana);
			spray->m_param.m_bandana = 0;
		}

		delete spray;
	}

	m_sprayCache.clear();
}

void CFxBox::ClearOriginalSequence()
{
	std::map<std::string, CFxSequence*>::iterator it = m_sequenceCache.begin();
	for (; it != m_sequenceCache.end(); ++it)
	{
		CFxSequence* sequence = (*it).second;
		delete sequence;
	}

	m_sequenceCache.clear();
}

void CFxBox::ClearAll(bool bFree)
{
	ClearSequence(bFree);
	ClearSpray(bFree);
}

void CFxBox::ClearSpray(bool bFree)
{
	for (std::map<CFxSpray*, CFxSpray*>::iterator it = m_spray.begin();
		it != m_spray.end();)
	{
		if ((*it).second)
		{
			if (bFree && !(*it).second->m_bMark)
			{
				++it;
				continue;
			}

			DeleteSpray(it);
		}
	}
}

void CFxBox::DeleteSpray(CFxSpray* spray)
{
	std::map<CFxSpray*, CFxSpray*>::iterator it = m_spray.find(spray);
	if (it != m_spray.end())
		DeleteSpray(it);
}

void CFxBox::DeleteSpray(std::map<CFxSpray*, CFxSpray*>::iterator& it)
{
	CFxSpray* spray = (*it).second;
	std::map<CFxSpray*, CFxSpray*>::iterator itErase = it;
	++it;
	m_spray.erase(itErase);
	OnSprayDeleted(spray);

	if (spray->m_pSequence)
		spray->m_pSequence->DeleteChildSpray(spray);

	CallFxKeeperCallBack(spray);

	delete spray;
}

void CFxBox::ClearSequence(bool bFree)
{
	for (std::map<CFxSequence*, CFxSequence*>::iterator it = m_sequence.begin();
		it != m_sequence.end();)
	{
		if ((*it).second)
		{
			if (bFree && !(*it).second->m_bMark)
			{
				++it;
				continue;
			}

			DeleteSequence(it, false);
		}
	}
}

void CFxBox::DeleteSequence(CFxSequence* sequence, bool bDeleteChild)
{
	std::map<CFxSequence*, CFxSequence*>::iterator it =
		m_sequence.find(sequence);
	if (it != m_sequence.end())
		DeleteSequence(it, bDeleteChild);
}

void CFxBox::DeleteSequence(std::map<CFxSequence*, CFxSequence*>::iterator& it,
	bool bDeleteChild)
{
	CFxSequence* sequence = (*it).second;
	m_sequence.erase(it++);

	std::map<CFxSpray*, CFxSpray*>::iterator child = sequence->m_spray.begin();
	while (child != sequence->m_spray.end())
	{
		CFxSpray* spray = (*child).second;

		if (spray == NULL)
		{
			sequence->m_spray.erase(child++);
		}
		else if (bDeleteChild)
		{
			child++;
			DeleteSpray(spray);
		}
		else
		{
			spray->m_pos += sequence->m_pos;

			spray->m_pSequence = NULL;
			if (spray->m_param.m_bBackground && sequence->m_pAttacher)
			{
				spray->m_pAttacher = sequence->m_pAttacher;
				AddSpray(spray);
			}

			++child;
		}
	}

	OnSequenceDeleted(sequence);

	if (sequence->m_bDelete)
	{
		CallFxKeeperCallBack(sequence);
		delete sequence;
	}
}

bool CFxBox::Finished()
{
	return m_sequence.size() == 0 && m_spray.size() == 0;
}

const CFxSequence::sLine& CFxBox::GetLastError()
{
	return m_lastError;
}

CFxSequence* CFxBox::LoadSequence(const char* filename, bool bDelete,
	defines_t* defines)
{
	CFxSequence* sequence = new CFxSequence(*this, bDelete);

	ClearDefines(sequence->m_defines, defines);

	if (sequence->LoadFile(filename))
	{
		sequence->m_filename = filename;
		return sequence;
	}

	delete sequence;
	return NULL;
}

void CFxBox::AddSequence(CFxSequence* sequence)
{
	sequence->SetLineTime();
	m_sequence[sequence] = sequence;
	OnSequenceCreated(sequence);
}

CFxSpray* CFxBox::LoadSpray(const char* filename, defines_t* defines)
{
	CFxSpray* spray = new CFxSpray(*this);

	ClearDefines(*(defines_t*)spray->m_defines, defines);

	if (spray->ParseFile(filename, defines == NULL))
		return spray;

	delete spray;
	return NULL;
}

void CFxBox::AddSpray(CFxSpray* spray)
{
	m_spray[spray] = spray;
	OnSprayCreated(spray);
}

float CFxBox::ParseNumber(const char* str)
{
	int len = strlen(str);
	if (len == 0)
		return 0.0f;

	char suffix = str[len - 1];
	if (suffix == 'd')
		return (float)atof(str) * g_DEGTORAD;
	else if (suffix == 'f')
		return (float)atof(str) * m_frameTime;
	return (float)atof(str);
}

W3dAniSpr* CFxBox::LoadSprite(const char* filename, int width, int height)
{
	char key[272];
	sprintf(key, "%s_%dx%d", filename, width, height);

	std::map<std::string, W3dAniSpr*>::iterator it = m_sprite.find(key);
	if (it != m_sprite.end())
		return (*it).second;

	if (!m_pResMng)
		return NULL;

	W3dAniSpr* sprite = new W3dAniSpr;
	sprite->SetResourceManager(m_pResMng);
	sprite->LoadSpritesInOneTexture(filename, 0, (float)width, (float)height);

	m_sprite[key] = sprite;
	return sprite;
}

bool CFxBox::Process(float elapsed)
{
	if (m_bStop)
		return true;

	if (m_bLimitElapsed)
	{
		if (elapsed > m_frameTime * 1.5f)
			elapsed = m_frameTime * 1.5f;
	}

	std::map<CFxSpray*, CFxSpray*>::iterator it = m_spray.begin();
	while (it != m_spray.end())
	{
		CFxSpray* spray = (*it).second;

		if (!spray)
		{
			std::map<CFxSpray*, CFxSpray*>::iterator itErase = it++;
			m_spray.erase(itErase);
			continue;
		}

		if (!spray->Process(elapsed * m_timeScale))
			DeleteSpray(it);
		else
			++it;
	}

	std::map<CFxSequence*, CFxSequence*>::iterator itSeq = m_sequence.begin();
	while (itSeq != m_sequence.end())
	{
		CFxSequence* sequence = (*itSeq).second;

		if (!sequence)
		{
			std::map<CFxSequence*, CFxSequence*>::iterator itErase = itSeq++;
			m_sequence.erase(itErase);
			continue;
		}

		bool bError;
		if (!sequence->Process(elapsed * m_timeScale, bError))
			DeleteSequence(itSeq, false);
		else
			++itSeq;

		if (bError)
		{
			m_lastError = sequence->GetLastError();
			return false;
		}
	}

	return true;
}

void CFxBox::Display(WView* view)
{
	m_displayCount = 0;
	m_displayCount2 = 0;

	for (std::map<CFxSpray*, CFxSpray*>::iterator it = m_spray.begin();
		it != m_spray.end(); ++it)
	{
		CFxSpray* spray = (*it).second;

		spray->Display(view);

		m_displayCount += spray->m_nDrawn;
		m_displayCount2 += spray->m_nTailDrawn;
	}
}

void CFxBox::ClearDefines(defines_t& defines, defines_t* parent)
{
	for (int i = 0; i < 16; i++)
		defines.name[i][0] = '\0';

	strcpy(defines.name[0], "particle_level");
	if (WSingleton<COption>::IsInstantiated())
	{
		int level = 2 - WSingleton<COption>::Instance()->vGetParticleLevel();
		defines.value[0] = (float)(level < 1 ? 1 : level);
	}
	else
	{
		defines.value[0] = 0;
	}

	if (parent)
	{
		for (int i = 0; i < parent->num; i++)
		{
			strcpy(defines.name[i + 1], parent->name[i]);
			defines.value[i + 1] = parent->value[i];
		}
		defines.num = parent->num + 1;
	}
	else
	{
		defines.num = 1;
	}
}

// "#define <name> <a> [op <b>]": adds name = a op b (a and b numbers or earlier names) to the
// defines; false when the line is not a #define.
bool CFxBox::PreprocessDefines(defines_t& defines, const char* line)
{
	char word[260];
	char name[260];
	int op;
	float values[2];
	float* pValue;

	cTokenV token;
	token.Init(line, strlen(line));

	token.GetToken(word, " ", 1);

	if (stricmp(word, "#define") == 0)
	{
		token.GetTokenWithoutFail(name, " \r\n\t", 4);

		int i;
		for (i = 0; i < defines.num; i++)
		{
			if (stricmp(name, defines.name[i]) == 0)
				break;
		}

		if (i < defines.num)
			return true;

		op = 0;
		pValue = values;

		for (int n = 0; n < 16; n++)
		{
			token.GetTokenWithoutFail(word, " \r\n\t", 4);
			if (word[0] == '\0')
				continue;

			if (stricmp(word, "*") == 0)
			{
				op = 3;
				continue;
			}
			if (stricmp(word, "/") == 0)
			{
				op = 4;
				continue;
			}
			if (stricmp(word, "+") == 0)
			{
				op = 1;
				continue;
			}
			if (stricmp(word, "-") == 0)
			{
				op = 2;
				continue;
			}

			for (i = 0; i < defines.num; i++)
			{
				if (stricmp(word, defines.name[i]) == 0)
				{
					*pValue++ = defines.value[i];
					break;
				}
			}

			if (i == defines.num)
				*pValue++ = (float)atof(word);
		}

		float value;
		switch (op)
		{
		case 1:
			value = values[0] + values[1];
			break;
		case 2:
			value = values[0] - values[1];
			break;
		case 3:
			value = values[0] * values[1];
			break;
		case 4:
			value = values[0] / values[1];
			break;
		default:
			value = values[0];
			break;
		}

		strcpy(defines.name[defines.num], name);
		defines.value[defines.num] = value;
		defines.num++;

		return true;
	}

	return false;
}

void CFxBox::ProcessDefines(defines_t& defines, char* str)
{
	for (int i = 0; i < defines.num; i++)
	{
		if (stricmp(str, defines.name[i]) == 0)
			gcvt(defines.value[i], 10, str);
	}
}

void CFxBox::AddFxKeeper(sFxKeeper& keeper)
{
	if (keeper.keeper)
		keeper.keeper->SetFxManager(this);
	m_fxKeeper.push_back(keeper);
}

void CFxBox::CallFxKeeperCallBack(void* param)
{
	std::list<sFxKeeper>::iterator it;
	for (it = m_fxKeeper.begin(); it != m_fxKeeper.end(); ++it)
	{
		((*it).keeper->*(*it).func)(param);
	}
}

void CFxBox::EraseFxKeeper(CFxKeeper* keeper)
{
	if (keeper == NULL)
		return;

	for (std::list<sFxKeeper>::iterator it = m_fxKeeper.begin();
		it != m_fxKeeper.end(); ++it)
	{
		if ((*it).keeper == keeper)
		{
			m_fxKeeper.erase(it);
			return;
		}
	}
}

void CFxBox::ClearFxKeeper()
{
	m_fxKeeper.clear();
}

void CFxBox::SetScreenCenter(float x, float y)
{
	for (std::map<CFxSpray*, CFxSpray*>::iterator it = m_spray.begin();
		it != m_spray.end(); ++it)
	{
		CFxSpray* spray = (*it).second;

		if (spray)
		{
			spray->m_attachX = x;
			spray->m_attachY = y;
		}
	}

	for (std::map<CFxSequence*, CFxSequence*>::iterator it2 =
			 m_sequence.begin();
		it2 != m_sequence.end(); ++it2)
	{
		CFxSequence* sequence = (*it2).second;

		if (sequence)
		{
			sequence->m_attachX = x;
			sequence->m_attachY = y;
		}
	}
}

// D863: fxspray.cpp (visao parcial no VC10) le estes 3 pelo offset do 983
C_ASSERT_VC10(offsetof(CFxBox, m_pCamera) == 0x17c && offsetof(CFxBox, m_pVertex) == 0x180 &&
	offsetof(CFxBox, m_vertex) == 0x194);
// D873: bongdarishopdlg le os _Mysize dos mapas (CFxBox::Finished inline do 983)
C_ASSERT(offsetof(CFxBox, m_spray) + VC10_MAP_MYSIZE == VC10_FXBOX_SPRAYN);
C_ASSERT(offsetof(CFxBox, m_sequence) + VC10_MAP_MYSIZE == VC10_FXBOX_SEQN);
