#include "minatl.h"
#include "tweaker.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

namespace
{
	void output_trace(const char* format, ...)
	{
		char buffer[512] = { 0 };

		va_list args;
		va_start(args, format);
		int result = _vsnprintf(buffer, sizeof(buffer) - 1, format, args);
		va_end(args);

		if (result != -1)
		{
			size_t length = strlen(buffer);
			buffer[length] = '\n';
			buffer[length + 1] = '\0';
		}
	}

	bool __fastcall tokenize(const std::string& str, const char* delimiter,
		std::vector<std::string>& tokens)
	{
		std::string token;
		size_t delimiter_count = strlen(delimiter);

		std::string::const_iterator it = str.begin();
		while (it != str.end())
		{
			for (size_t i = 0; i < delimiter_count; ++i)
			{
				if (delimiter[i] == *it && token.length() > 0)
				{
					tokens.push_back(token);
					token.clear();
					++it;
					if (it == str.end())
					{
						return true;
					}
					break;
				}
			}

			bool is_delimiter = false;
			for (size_t i = 0; i < delimiter_count; ++i)
			{
				if (delimiter[i] == *it)
				{
					is_delimiter = true;
					break;
				}
			}

			if (!is_delimiter)
			{
				token += *it;
			}
			++it;
		}

		if (token.length() > 0)
		{
			tokens.push_back(token);
		}

		return tokens.empty();
	}
}

tweaker_cmd_target::~tweaker_cmd_target()
{
	tweaker_database::instance()->_unregist_tweaker(this);
}

tweaker_param::tweaker_param()
{
}

tweaker_param::tweaker_param(std::vector<std::string>& params)
{
	if (!params.empty())
	{
		m_params.assign(params.begin(), params.end());
	}
}

tweaker_token tweaker_param::operator[](unsigned int index) const
{
	if (m_params.size() > index)
	{
		return tweaker_token(m_params[index]);
	}

	output_trace("tweaker_token index overrun!");
	return tweaker_token(std::string(""));
}

tweaker::tweaker(const char* name)
	: m_name(name)
{
}

void tweaker::_regist_command(const char* name, const char* command,
	tweaker_cmd_info& info)
{
	if (m_name != name)
	{
		return;
	}

	if (m_command.find(command) == m_command.end())
	{
		output_trace("succeed regist tweaker command : &%s::%s", name, command);
		m_command.insert(
			std::map<std::string, tweaker_cmd_info>::value_type(command, info));
	}
}

std::string tweaker::run_command(const char* param)
{
	if (param == 0)
	{
		return "warning : parameter null";
	}

	std::vector<std::string> tokens;
	tokenize(std::string(param), " ,", tokens);

	if (tokens.empty())
	{
		return "warning : parameter empty";
	}

	std::string command(tokens[0]);
	tokens.erase(tokens.begin());

	return _run_command(command, tweaker_param(tokens));
}

std::string tweaker::_run_command(std::string& command, tweaker_param& param)
{
	std::map<std::string, tweaker_cmd_info>::iterator it =
		m_command.find(command);
	if (it == m_command.end())
	{
		return "unknown command";
	}

	std::string result;
	tweaker_cmd_func func = it->second.function;
	for (std::set<tweaker_cmd_target*>::iterator target = m_alloc.begin();
		target != m_alloc.end(); ++target)
	{
		result = ((*target)->*func)(param);
	}
	return result;
}

bool tweaker::is_exist_command(const std::string& command)
{
	std::vector<std::string> tokens;
	tokenize(command, " ,", tokens);

	if (tokens.empty())
	{
		return false;
	}

	std::map<std::string, tweaker_cmd_info>::iterator it =
		m_command.find(tokens[0]);
	return it != m_command.end();
}

void tweaker::get_command_list(std::vector<std::string>& list,
	bool bWithName) const
{
	for (std::map<std::string, tweaker_cmd_info>::const_iterator it =
			 m_command.begin();
		it != m_command.end(); ++it)
	{
		std::string line;
		if (bWithName)
		{
			line = m_name;
			line += " ";
		}
		line += it->first;
		const tweaker_cmd_info& info = it->second;
		if (!info.help.empty())
		{
			line += " (";
			line += info.help;
			line += ")";
		}
		list.push_back(line);
	}
}

void tweaker::_insert_alloc(tweaker_cmd_target* target)
{
	if (m_alloc.find(target) == m_alloc.end())
	{
		m_alloc.insert(target);
		output_trace("succeed tweaker insert alloc : 0x%p", target);
	}
}

void tweaker::_erase_alloc(tweaker_cmd_target* target)
{
	std::set<tweaker_cmd_target*>::iterator it = m_alloc.find(target);
	if (it != m_alloc.end())
	{
		m_alloc.erase(it);
	}
}

bool tweaker::_is_exist_alloc(tweaker_cmd_target* target)
{
	std::set<tweaker_cmd_target*>::iterator it = m_alloc.find(target);
	return it != m_alloc.end();
}

tweaker* tweaker_database::_regist_tweaker(const char* name,
	tweaker_cmd_target* target)
{
	std::map<std::string, tweaker*>::iterator it = m_tweaker.find(name);
	if (it != m_tweaker.end())
	{
		it->second->_insert_alloc(target);
		return it->second;
	}

	tweaker* t = new tweaker(name);
	// HACK: dead delete; places ??_Gtweaker right before this function
	if (0)
		delete t;
	t->_insert_alloc(target);
	m_tweaker.insert(std::map<std::string, tweaker*>::value_type(name, t));
	output_trace("succeed tweaker regist : 0x%p", target);
	return t;
}

void tweaker_database::_unregist_tweaker(tweaker_cmd_target* target)
{
	if (0)
		_regist_tweaker(0, 0);
	for (std::map<std::string, tweaker*>::iterator it = m_tweaker.begin();
		it != m_tweaker.end(); ++it)
	{
		tweaker* t = it->second;
		if (t->_is_exist_alloc(target))
		{
			t->_erase_alloc(target);
			if (t->_size_alloc() == 0)
			{
				delete t;
				m_tweaker.erase(it);
				output_trace("succeed tweaker unregist : 0x%p", target);
				return;
			}
		}
	}
}

tweaker_database::~tweaker_database()
{
	if (0)
		_unregist_tweaker(0);
	std::map<std::string, tweaker*>::const_iterator it;
	for (it = m_tweaker.begin(); it != m_tweaker.end(); ++it)
	{
		delete it->second;
	}
	m_tweaker.clear();
}

std::string tweaker_database::send_command(const char* command)
{
	std::string line(command);
	std::string name;
	if (line.empty())
	{
		return "> command string too short.";
	}

	std::vector<std::string> tokens;
	tokenize(line, " ,", tokens);
	name = tokens[0];
	tokens.erase(tokens.begin());
	if (tokens.empty())
	{
		return "> command string too short.";
	}

	if (name == "class" || name == "struct")
	{
		name += " ";
		name += tokens[0];
		tokens.erase(tokens.begin());
	}

	tweaker* t = get_tweaker(name);
	if (t == 0 || tokens.empty())
	{
		return "> tweaker not found.";
	}

	line = tokens[0];
	tokens.erase(tokens.begin());

	std::string result("> ");
	result += t->_run_command(line, tweaker_param(tokens));
	return result;
}

bool tweaker_database::is_exist_command(const char* command)
{
	std::string line(command);
	std::string name;
	if (line.empty())
	{
		return false;
	}

	std::vector<std::string> tokens;
	tokenize(line, " ,", tokens);
	name = tokens[0];
	tokens.erase(tokens.begin());
	if (!tokens.empty())
	{
		if (name == "class" || name == "struct")
		{
			name += " ";
			name += tokens[0];
			tokens.erase(tokens.begin());
		}

		tweaker* t = get_tweaker(name);
		if (t != 0 && !tokens.empty())
		{
			line = tokens[0];
			return t->is_exist_command(line);
		}
	}

	return false;
}

bool tweaker_database::is_exist_tweaker(const tweaker* t)
{
	if (0)
		((tweaker*)0)->_is_exist_alloc(0);
	std::map<std::string, tweaker*>::const_iterator it;
	for (it = m_tweaker.begin(); it != m_tweaker.end(); ++it)
	{
		if (it->second == t)
		{
			return true;
		}
	}
	return false;
}

void tweaker_database::get_tweaker_command_all(std::vector<std::string>& list)
{
	list.std::vector<std::string>::~vector();

	unsigned int count = 0;
	std::map<std::string, tweaker*>::const_iterator it;
	for (it = m_tweaker.begin(); it != m_tweaker.end(); ++it)
	{
		count += it->second->_size_command();
	}
	list.reserve(count);

	for (it = m_tweaker.begin(); it != m_tweaker.end(); ++it)
	{
		it->second->get_command_list(list, true);
	}
}

void tweaker_database::get_tweaker_names(std::vector<std::string>& list)
{
	list.std::vector<std::string>::~vector();
	list.reserve((unsigned int)m_tweaker.size());

	std::map<std::string, tweaker*>::const_iterator it;
	for (it = m_tweaker.begin(); it != m_tweaker.end(); ++it)
	{
		list.push_back(it->first);
	}
}

tweaker* tweaker_database::get_tweaker(std::string& name)
{
	std::map<std::string, tweaker*>::iterator it = m_tweaker.find(name);
	if (it != m_tweaker.end())
	{
		return it->second;
	}
	return 0;
}

// HACK: defined after the first delete so ??_Gtweaker is emitted after the
// constructor (the original dtor is implicit, no line info)
tweaker::~tweaker()
{
}
