#pragma once

#include <stdio.h>

struct sResName
{
	char name[64];
};

bool CompareByName(const sResName& a, const sResName& b);

class CIntegrityCheck
{
public:
	CIntegrityCheck();
	virtual ~CIntegrityCheck();

	bool Report(const char* filename);

protected:
	void Log(const char* format, ...);
	void PrintOkMissing(bool ok);
	void PrintOkNg(bool ok);
	bool ReportCharParts();

	FILE* m_fp;
};
