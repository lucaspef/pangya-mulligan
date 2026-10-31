#pragma once

class CPangFBI : public WSingleton<CPangFBI>
{
public:
	CPangFBI();
	virtual ~CPangFBI();

	void ProcessReport(const char* type, const char* data);
};
