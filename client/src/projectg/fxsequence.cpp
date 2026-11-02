#define REBANG_FXSEQUENCE_CPP
#include "minatl.h"
#include "vc10_layout.h"
#include "mathconsts.h"
#include "binstr.h"
#include "fxsequence.h"
#include "fxbox.h"

extern WMatrix g_camera;

CFxSequence::CFxSequence(CFxBox& box, bool bDelete)
	: m_pFxBox(&box)
{
	m_bMark = true;
	m_wind.x = m_wind.y = m_wind.z = 0.0f;
	m_bActive = true;
	m_pos = m_vel = WVector(0.0f, 0.0f, 0.0f);
	m_bDelete = bDelete;
	m_time = 0.0f;
	m_delay = 0.0f;
	m_attachX = g_HUGE;
	m_attachY = g_HUGE;
	m_rotation.Reset();
	memset(m_attachMat, 0, sizeof(m_attachMat));
	memset(m_attachPos, 0, sizeof(m_attachPos));
	Clear();
}

CFxSequence::~CFxSequence()
{
}

void CFxSequence::Clear()
{
	m_pAttacher = NULL;
	m_lineList.clear();
	m_curIt = m_lineList.end();
	m_curLine.text[0] = 0;
	m_pCurSpray = NULL;
	memset(&m_defines, 0, sizeof(m_defines));
}

bool CFxSequence::LoadFile(const char* filename)
{
	Clear();

	std::map<std::string, CFxSequence*>::iterator it =
		m_pFxBox->m_sequenceCache.find(filename);
	if (it == m_pFxBox->m_sequenceCache.end())
	{
		cFile* file = m_pFxBox->m_pResMng->GetCFile(filename, 0x2000);
		if (file == NULL)
		{
			return false;
		}

		char buf[1024];
		int lineNo = 0;
		while (file->Scan("%n", buf))
		{
			lineNo++;
			AddLine(buf, lineNo);
		}
		CloseCFile(file);

		CFxSequence* seq = new CFxSequence(*m_pFxBox, true);
		seq->m_lineList = m_lineList;
		m_pFxBox->m_sequenceCache[filename] = seq;
	}
	else
	{
		m_lineList = (*it).second->m_lineList;
		m_curIt = m_lineList.begin();
	}

	SetLineTime();
	return true;
}

void CFxSequence::AddLine(const char* line, int lineNo)
{
	if (*line == 0 || *line == ';')
	{
		return;
	}

	if (m_pFxBox->PreprocessDefines(m_defines, line))
	{
		return;
	}

	cTokenV tokenV;
	tokenV.Init(line, strlen(line));

	char token0[260];
	char token1[260];
	char token2[260];
	tokenV.GetToken(token0, ":", 1);
	tokenV.GetToken(token2, "\r\n", 2);

	cTokenS tokenS;
	tokenS.Init(token0, strlen(token0));
	tokenS.GetToken(token0, "-", 1);
	tokenS.GetToken(token1, "-", 1);

	TrimA(token0, " \t\n\r\0");
	TrimA(token1, " \t\n\r\0");
	TrimA(token2, " \t\n\r\0");

	sLine newLine;
	newLine.line = lineNo;
	if (isalpha(token0[0]))
	{
		newLine.time = -1;
		strcpy(newLine.text, token0);
	}
	else if (token0[0] == 0)
	{
		newLine.time = -2;
		strcpy(newLine.text, token2);
	}
	else
	{
		newLine.minTime = atoi(token0);
		if (token1[0])
			newLine.maxTime = atoi(token1);
		else
			newLine.maxTime = newLine.minTime;
		newLine.time = newLine.minTime;
		strcpy(newLine.text, token2);
	}

	m_lineList.push_back(newLine);
	if (m_curIt == m_lineList.end())
		m_curIt = m_lineList.begin();
}

const CFxSequence::sLine& CFxSequence::GetLastError()
{
	return m_curLine;
}

bool CFxSequence::ParseLine(const char* line, bool& bStop)
{
	bStop = false;
	if (*line == 0 || *line == ';')
	{
		return true;
	}

	cTokenV tokenV;
	tokenV.Init(line, strlen(line));

	char cmd[260];
	char tok[16][260];
	tokenV.GetToken(cmd, "=", 1);
	TrimA(cmd, " \t\n\r\0");
	for (int i = 0; i < 16; i++)
	{
		tokenV.GetToken(tok[i], ",", 1);
		TrimA(tok[i], " \t()%");
		m_pFxBox->ProcessDefines(m_defines, tok[i]);
	}

	if (cmd[0] == 0)
	{
		return true;
	}

	if (strcmpi(cmd, "spray") == 0)
	{
		if (!m_bActive)
		{
			return true;
		}

		CFxSpray* spray = m_pFxBox->LoadSpray(tok[0], NULL);
		if (spray == NULL)
		{
			bStop = true;
			return false;
		}

		spray->m_param.m_bBackground = strcmpi(tok[7], "strongbond") == 0;
		spray->m_pSequence = this;
		spray->m_attachX = m_attachX;
		spray->m_attachY = m_attachY;
		spray->m_bMark = m_bMark;
		spray->SetWind(m_wind);
		spray->m_pos = WVector((float)atof(tok[1]), (float)atof(tok[2]),
			(float)atof(tok[3]));
		spray->m_offset = RotVec(WVector((float)atof(tok[4]),
									 (float)atof(tok[5]), (float)atof(tok[6])),
			m_rotation);
		bool bActive = m_bActive;
		spray->m_bActive = bActive;
		m_spray[spray] = spray;
		m_pFxBox->AddSpray(spray);
		m_pCurSpray = spray;
	}
	else if (strcmpi(cmd, "add_spawn") == 0)
	{
		if (m_pCurSpray != NULL)
		{
			CFxSpray::sSpawn spawn;
			strcpy(spawn.m_name, tok[0]);
			spawn.m_bAttach = strcmpi(tok[1], "attached") == 0;
			if (strcmpi(tok[2], "interval") == 0)
			{
				spawn.m_type = 0;
				spawn.m_interval = m_pFxBox->ParseNumber(tok[3]);
				spawn.m_max = (int)m_pFxBox->ParseNumber(tok[4]);
			}
			else if (strcmpi(tok[2], "age") == 0)
			{
				spawn.m_type = 1;
				spawn.m_interval = m_pFxBox->ParseNumber(tok[3]);
			}
			else if (strcmpi(tok[2], "ondeath") == 0)
			{
				spawn.m_type = 2;
			}
			else if (strcmpi(tok[2], "onground") == 0)
			{
				spawn.m_type = 3;
			}

			m_pCurSpray->m_param.m_spawn.push_back(spawn);
		}
	}
	else if (strcmpi(cmd, "add_revolution") == 0)
	{
		if (m_pCurSpray != NULL)
		{
			CFxSpray::sRevol revol;
			sscanf(tok[0], "%f", &revol.m_center.x);
			sscanf(tok[1], "%f", &revol.m_center.y);
			sscanf(tok[2], "%f", &revol.m_center.z);
			sscanf(tok[3], "%f", &revol.m_axis.x);
			sscanf(tok[4], "%f", &revol.m_axis.y);
			sscanf(tok[5], "%f", &revol.m_axis.z);
			revol.m_start = (int)m_pFxBox->ParseNumber(tok[6]);
			revol.m_startVal = m_pFxBox->ParseNumber(tok[7]);
			revol.m_end = (int)m_pFxBox->ParseNumber(tok[8]);
			revol.m_endVal = m_pFxBox->ParseNumber(tok[9]);
			m_pCurSpray->m_param.m_revol.push_back(revol);
		}
	}
	else if (strcmpi(cmd, "add_rotation") == 0)
	{
		if (m_pCurSpray != NULL)
		{
			CFxSpray::sNozzleRot rot;
			sscanf(tok[0], "%f", &rot.m_axis.x);
			sscanf(tok[1], "%f", &rot.m_axis.y);
			sscanf(tok[2], "%f", &rot.m_axis.z);
			rot.m_start = (int)m_pFxBox->ParseNumber(tok[3]);
			rot.m_startVal = m_pFxBox->ParseNumber(tok[4]);
			rot.m_end = (int)m_pFxBox->ParseNumber(tok[5]);
			rot.m_endVal = m_pFxBox->ParseNumber(tok[6]);
			m_pCurSpray->m_param.m_rotate.push_back(rot);
		}
	}
	else if (strcmpi(cmd, "add_nrotation") == 0)
	{
		if (m_pCurSpray != NULL)
		{
			CFxSpray::sNozzleRot rot;
			sscanf(tok[0], "%f", &rot.m_axis.x);
			sscanf(tok[1], "%f", &rot.m_axis.y);
			sscanf(tok[2], "%f", &rot.m_axis.z);
			rot.m_start = (int)m_pFxBox->ParseNumber(tok[3]);
			rot.m_startVal = m_pFxBox->ParseNumber(tok[4]);
			rot.m_end = (int)m_pFxBox->ParseNumber(tok[5]);
			rot.m_endVal = m_pFxBox->ParseNumber(tok[6]);
			m_pCurSpray->m_param.m_nozzleRot.push_back(rot);
		}
	}
	else if (strcmpi(cmd, "add_prevolution") == 0)
	{
		if (m_pCurSpray != NULL)
		{
			CFxSpray::sRevol revol;
			sscanf(tok[0], "%f", &revol.m_center.x);
			sscanf(tok[1], "%f", &revol.m_center.y);
			sscanf(tok[2], "%f", &revol.m_center.z);
			sscanf(tok[3], "%f", &revol.m_axis.x);
			sscanf(tok[4], "%f", &revol.m_axis.y);
			sscanf(tok[5], "%f", &revol.m_axis.z);
			revol.m_start = (int)m_pFxBox->ParseNumber(tok[6]);
			revol.m_startVal = m_pFxBox->ParseNumber(tok[7]);
			revol.m_end = (int)m_pFxBox->ParseNumber(tok[8]);
			revol.m_endVal = m_pFxBox->ParseNumber(tok[9]);
			m_pCurSpray->m_param.m_revolGrand.push_back(revol);
		}
	}
	else if (stricmp(cmd, "AttachedToParent") == 0)
	{
		if (m_pCurSpray != NULL)
		{
			m_pCurSpray->m_param.m_bAttach = false;
			if (stricmp(tok[0], "on") == 0)
			{
				m_pCurSpray->m_param.m_bAttach = true;
			}

			int flagX = 0;
			int flagY = 0;
			int flagZ = 0;
			sscanf(tok[1], "%d", &flagX);
			sscanf(tok[2], "%d", &flagY);
			sscanf(tok[3], "%d", &flagZ);
			m_pCurSpray->m_param.m_attachAxis = 0;
			if (flagX)
			{
				m_pCurSpray->m_param.m_attachAxis |= 1;
			}
			if (flagY)
			{
				m_pCurSpray->m_param.m_attachAxis |= 2;
			}
			if (flagZ)
			{
				m_pCurSpray->m_param.m_attachAxis |= 4;
			}
		}
	}
	else if (strcmpi(cmd, "sound") == 0)
	{
		float volume = -1.0f;
		char name[260];
		strcpy(name, tok[0]);
		char* ext = strstr(name, ".wav");
		if (ext)
		{
			*ext = 0;
		}
		sscanf(tok[1], "%f", &volume);
		if (volume < 0.0f)
		{
			g_audio->PlaySfx(name);
		}
		else
		{
			WVector pos = m_pos - g_camera.pivot;
			g_audio->PlaySfx(name, &pos, 5, NULL, NULL, 0.0f, volume);
		}
	}
	else if (strcmpi(cmd, "goto") == 0)
	{
		for (std::list<sLine>::iterator it = m_lineList.begin();
			it != m_lineList.end(); ++it)
		{
			if (strcmpi((*it).text, tok[0]) == 0)
			{
				SetLineTime();
				++it;
				m_curIt = it;
				float lineTime = (float)(*it).minTime;
				float diff = m_time - lineTime;
				if (diff < m_pFxBox->m_frameTime * 2.0f &&
					diff > m_pFxBox->m_frameTime * -2.0f)
				{
					return false;
				}

				m_time = lineTime;
				break;
			}
		}
	}
	else if (strcmpi(cmd, "stop") == 0)
	{
		m_curIt = m_lineList.end();
	}
	// 983 (CFxSequence::ParseLine 009758b0): "clear" e "spray_kill" são
	// novos; sem eles a sequência parava no comando desconhecido (bStop).
	else if (strcmpi(cmd, "clear") == 0)
	{
		m_pFxBox->ClearSpray(true);
	}
	else if (strcmpi(cmd, "spray_kill") == 0)
	{
		if (m_pCurSpray != NULL)
		{
			int killTime;
			sscanf(tok[0], "%d", &killTime);
			m_pCurSpray->m_killTime = killTime;
		}
	}
	else
	{
		bStop = true;
		return false;
	}

	return true;
}

void CFxSequence::SetLineTime()
{
	for (std::list<sLine>::iterator it = m_lineList.begin();
		it != m_lineList.end(); ++it)
	{
		sLine& cur = *it;
		if ((*it).time == -1 || (*it).time == -2)
		{
			continue;
		}

		if ((*it).maxTime != (*it).minTime)
		{
			(*it).time =
				(*it).minTime + rand() % ((*it).maxTime - (*it).minTime);
		}

		std::list<sLine>::iterator first = it;
		std::list<sLine>::iterator prev = it;
		while (first != m_lineList.begin())
		{
			--prev;
			if ((*prev).time == -1 || (*prev).time == -2)
			{
				break;
			}
			--first;
		}
		const unsigned long& t = (*it).time;
		for (; first != it; ++first)
		{
			if ((*first).time > t)
			{
				m_lineList.insert(first, cur);
				it = m_lineList.erase(it);
				--it;
				break;
			}
		}
	}
}

bool CFxSequence::Process(float elapsed, bool& bStop)
{
	bStop = false;
	if (!m_bActive)
	{
		if (m_delay > 0.0f)
		{
			m_delay -= elapsed;
			if (m_delay < 0.0f)
			{
				m_bActive = true;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return true;
		}
	}

	bool bResult;
	for (;;)
	{
		if (m_curIt == m_lineList.end())
		{
			bResult = false;
			break;
		}

		sLine& line = *m_curIt;
		if (line.time == -1)
		{
			++m_curIt;
			continue;
		}

		if (line.time != -2 && line.time > (unsigned long)(m_time + 0.5f))
		{
			bResult = true;
			break;
		}

		++m_curIt;
		if (line.text[0] != 0)
		{
			if (!ParseLine(line.text, bStop))
			{
				--m_curIt;
				bResult = true;
				break;
			}

			if (bStop)
			{
				m_curLine = line;
				bResult = true;
				break;
			}
		}
	}

	m_time += elapsed;
	return bResult;
}

void CFxSequence::DeleteChildSpray(CFxSpray* spray)
{
	std::map<CFxSpray*, CFxSpray*>::iterator it = m_spray.find(spray);
	if (it != m_spray.end())
	{
		if ((*it).second == m_pCurSpray)
		{
			m_pCurSpray = NULL;
		}

		m_spray.erase(it);
	}
}

void CFxSequence::SetLight(const LightSet& light)
{
	for (std::map<CFxSpray*, CFxSpray*>::iterator it = m_spray.begin();
		it != m_spray.end(); ++it)
	{
		(*it).second->SetLight(light);
	}
}

void CFxSequence::SetWind(const WVector& wind)
{
	for (std::map<CFxSpray*, CFxSpray*>::iterator it = m_spray.begin();
		it != m_spray.end(); ++it)
	{
		(*it).second->SetWind(wind);
	}
}

void CFxSequence::SetDistLimit(float minDist, float maxDist)
{
	for (std::map<CFxSpray*, CFxSpray*>::iterator it = m_spray.begin();
		it != m_spray.end(); ++it)
	{
		(*it).second->SetDistLimit(minDist, maxDist);
	}
}

// A472: posicao do efeito = 983 (+0x110c em safetee_ball/Talk_Room/grandprix_logoeff; +0x1108 = m_bActive)
C_ASSERT_VC10(offsetof(CFxSequence, m_bActive) == 0x1108);
C_ASSERT_VC10(offsetof(CFxSequence, m_pos) == 0x110c);
// D872: os mesmos pelos dois compiladores (cl 14.44 medido em out/D872/probe.py)
C_ASSERT(offsetof(CFxSequence, m_bActive) == VC10_FXSEQ_ACTIVE && offsetof(CFxSequence, m_pos) == VC10_FXSEQ_POS);
C_ASSERT(offsetof(CFxSpray, m_bActive) == VC10_FXSPRAY_ACTIVE && offsetof(CFxSpray, m_pos) == VC10_FXSPRAY_POS);
C_ASSERT(offsetof(CFxSpray, m_bgRot) == VC10_FXSPRAY_BGROT);
// B1467: golfrule.cpp (bullet.seq/bullet.spr) escreve +0x10d0/+0x1150 do VC10 relativos a estas ancoras (sem STL no trecho;
// 0x10d0 = 4 antes do nosso m_rotation; medido cl 14.44 m_rotation 0x10cc, m_offset 0x114c, out/B1467/probe.py)
C_ASSERT(offsetof(CFxSequence, m_rotation) == VC10_FXSEQ_ACTIVE - 0x34 && offsetof(CFxSpray, m_offset) == VC10_FXSPRAY_POS + 0x18);
