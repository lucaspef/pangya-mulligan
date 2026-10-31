#pragma once

class CIniFile
{
public:
	CIniFile();
	CIniFile(const char* path);
	~CIniFile();

	enum IO_MODE
	{
		M_READ,
		M_WRITE
	};

	bool Create(const char* path, IO_MODE mode);
	const char* ReadString(const char* section_name, const char* key_name);
	int ReadInteger(const char* section_name, const char* key_name);
	int WriteString(const char* section_name, const char* key_name,
		const char* key_value);
	int WriteInteger(const char* section_name, const char* key_name,
		int key_value);
};
