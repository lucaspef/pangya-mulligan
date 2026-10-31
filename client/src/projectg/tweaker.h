#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdlib.h>

class tweaker;
class tweaker_param;
class tweaker_database;

class tweaker_cmd_target
{
protected:
	virtual ~tweaker_cmd_target();
};

typedef std::string (tweaker_cmd_target::*tweaker_cmd_func)(
	const tweaker_param&);

class tweaker_token
{
public:
	tweaker_token(const std::string& token)
		: m_token(token)
	{
	}

	const char* to_cstr() const { return m_token.c_str(); }

	int to_int() const { return atoi(m_token.c_str()); }
	float to_float() const { return (float)atof(m_token.c_str()); }

private:
	std::string m_token;
};

class tweaker_param
{
public:
	tweaker_param();
	tweaker_param(std::vector<std::string>& params);

	tweaker_token operator[](unsigned int index) const;

private:
	std::vector<std::string> m_params;
};

class tweaker
{
	friend class tweaker_database;

public:
	struct tweaker_cmd_info
	{
		tweaker_cmd_func function;
		std::string help;
	};

	~tweaker();

	std::string run_command(const char* param);
	bool is_exist_command(const std::string& command);
	void get_command_list(std::vector<std::string>& list, bool bWithName) const;

private:
	tweaker(const char* name);

	void _regist_command(const char* name, const char* command,
		tweaker_cmd_info& info);
	std::string _run_command(std::string& command, tweaker_param& param);

	void _insert_alloc(tweaker_cmd_target* target);
	void _erase_alloc(tweaker_cmd_target* target);
	bool _is_exist_alloc(tweaker_cmd_target* target);

	unsigned int _size_command() const
	{
		return (unsigned int)m_command.size();
	}
	unsigned int _size_alloc() const { return (unsigned int)m_alloc.size(); }

	std::set<tweaker_cmd_target*> m_alloc;
	std::map<std::string, tweaker_cmd_info> m_command;
	std::string m_name;
};

class tweaker_database
{
	friend class tweaker_cmd_target;

public:
	std::string send_command(const char* command);
	bool is_exist_command(const char* command);
	bool is_exist_tweaker(const tweaker* t);
	void get_tweaker_command_all(std::vector<std::string>& list);
	void get_tweaker_names(std::vector<std::string>& list);
	tweaker* get_tweaker(std::string& name);

	static tweaker_database* instance()
	{
		static tweaker_database _inst;
		return &_inst;
	}

private:
	~tweaker_database();

	tweaker* _regist_tweaker(const char* name, tweaker_cmd_target* target);
	void _unregist_tweaker(tweaker_cmd_target* target);

	std::map<std::string, tweaker*> m_tweaker;
};
