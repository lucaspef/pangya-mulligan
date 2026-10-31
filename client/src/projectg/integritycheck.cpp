#include "minatl.h"
#include "integritycheck.h"
#include "wresrcmng.h"
#include "wpuppet.h"
#include "wbone.h"
#include "wmesh.h"

#define RB_STR_MPET "%s.mpet"

// Unused; keeps g_CM_TO_WU in .rdata like the original.
static float CmToWu(float cm)
{
	return cm * g_CM_TO_WU;
}

CIntegrityCheck::CIntegrityCheck()
	: m_fp(NULL)
{
}

CIntegrityCheck::~CIntegrityCheck()
{
}

bool CIntegrityCheck::Report(const char* filename)
{
	m_fp = fopen(filename, "wt");

	ReportCharParts();

	fclose(m_fp);

	return true;
}

void CIntegrityCheck::Log(const char* format, ...)
{
	char buf[2048];
	va_list args;

	va_start(args, format);
	if (_vsnprintf(buf, sizeof(buf) - 1, format, args) == -1)
		buf[sizeof(buf) - 1] = 0;
	va_end(args);

	fprintf(m_fp, buf);
}

bool CompareByName(const sResName& a, const sResName& b)
{
	if (strcmp(a.name, b.name) < 0)
		return true;
	return false;
}

void CIntegrityCheck::PrintOkMissing(bool ok)
{
	if (ok)
		Log("ok!!\n");
	else
		Log("missing!!\n");
}

void CIntegrityCheck::PrintOkNg(bool ok)
{
	if (ok)
		Log("ok!!\n");
	else
		Log("ng!!\n");
}

bool CIntegrityCheck::ReportCharParts()
{
	std::map<unsigned int, IFF_STRUCT::sPart>& partMap =
		ItemManager()->m_PartMap;
	std::map<unsigned int, IFF_STRUCT::sChar>& charMap =
		ItemManager()->m_CharMap;

	std::map<unsigned long, WPuppet*> orgPuppets;
	int maxNumVtx = 0;
	int maxNumIdx = 0;
	std::map<std::string, std::string> missing;
	std::map<std::string, std::string> loadFailed;
	std::map<std::string, std::string> succeeded;
	std::map<std::string, std::string> failed;

	Log("* part check - total %d\n\n", partMap.size());

	for (PartIterator it = partMap.begin(); it != partMap.end(); ++it)
	{
		if (strlen((*it).second.Data) == 0)
			continue;

		char path[MAX_PATH];
		sprintf(path, RB_STR_MPET, (*it).second.Data);
		Log("\t* part : \"%s\"\n", (*it).second.c.Name);
		Log("\t\t* meshpet : %s - ", path);

		cFile* file = g_resrcmng->GetCFile(path, -1);
		PrintOkMissing(file != NULL);
		if (!file)
		{
			missing[path] = path;
			continue;
		}
		CloseCFile(file);

		Log("\t\t* meshpet loading - ");
		WPuppet* puppet = g_resrcmng->GetPuppet(path, false, true, false);
		PrintOkNg(puppet != NULL);
		if (!puppet)
		{
			loadFailed[path] = path;
			continue;
		}

		for (WBone* bone = puppet->FindMeshBone(NULL); bone;
			bone = puppet->FindMeshBone(bone))
		{
			int i = 0;
			for (w_mesh* mesh = bone->m_mesh; mesh; mesh = mesh->next, i++)
			{
				Log("\t\t* mesh%d : numVtx(%d), numIdx(%d), drawFlag(0x%08x), "
					"numMesh, drawFlag2(0x%08x)\n",
					i, mesh->vtxNum, mesh->indexNum,
					mesh->drawFlag & 0xfffc0000, mesh->xiDrawFlag2 & 0x1fff);
				if (mesh->vtxNum > maxNumVtx)
					maxNumVtx = mesh->vtxNum;
				if (mesh->indexNum > maxNumIdx)
					maxNumIdx = mesh->indexNum;
			}
		}

		int charTypeId = (((*it).second.c.TypeId >> 18) & 0xff) | 0x4000000;
		std::map<unsigned long, WPuppet*>::iterator found;
		found = orgPuppets.find(charTypeId);
		WPuppet* orgPuppet = NULL;
		if (found == orgPuppets.end())
		{
			CharIterator cit;
			cit = charMap.find(charTypeId);
			if (cit != charMap.end())
			{
				orgPuppet = g_resrcmng->GetPuppet(
					MakeStr("%s.bpet", (*cit).second.Data), false, true, false);
				if (orgPuppet)
				{
					orgPuppets.insert(
						std::map<unsigned long, WPuppet*>::value_type(
							charTypeId, orgPuppet));
				}
			}
		}
		else
		{
			orgPuppet = (*found).second;
		}

		Log("\t\t* compare original matrix - ");
		bool same = false;
		if (orgPuppet)
			same = puppet->CompareBasicMatrix(orgPuppet);
		if (same)
			succeeded[path] = path;
		else
			failed[path] = path;
		PrintOkNg(same);

		g_resrcmng->Release(puppet, true);
	}

	std::map<std::string, std::string>::iterator i;

	Log("\n");
	Log("\t* missing mpets - total %d\n", missing.size());
	for (i = missing.begin(); i != missing.end(); ++i)
		Log("\t\t%s\n", (*i).second.c_str());

	Log("\n");
	Log("\t* loading failed mpets - total %d\n", loadFailed.size());
	for (i = loadFailed.begin(); i != loadFailed.end(); ++i)
		Log("\t\t%s\n", (*i).second.c_str());

	Log("\n");
	Log("\t* original matrix comparison succeeded mpets - %d\n",
		succeeded.size());
	for (i = succeeded.begin(); i != succeeded.end(); ++i)
		Log("\t\t%s\n", (*i).second.c_str());

	Log("\n");
	Log("\t* original matrix comparison failed mpets - %d\n", failed.size());
	for (i = failed.begin(); i != failed.end(); ++i)
		Log("\t\t%s\n", (*i).second.c_str());

	Log("\n\n");
	Log("\t* maxNumVtx : %d\n", maxNumVtx);
	Log("\t* maxNumIdx : %d\n", maxNumIdx);

	for (std::map<unsigned long, WPuppet*>::iterator p = orgPuppets.begin();
		p != orgPuppets.end(); p++)
	{
		g_resrcmng->Release((*p).second, true);
	}

	return true;
}
