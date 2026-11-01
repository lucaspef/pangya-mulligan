// SPDX-License-Identifier: MIT
// puppetscript.cpp - see the header.
#include "puppetscript.h"
#include <string.h>

// commonutil.cpp: GetStrConst
static const char* GetStrConst(const char* source, std::string* dest)
{
	dest->clear();
	while (*source == ' ' || *source == '\t')
		++source;
	if (*source && *source++ == '"')
	{
		while (*source && *source != '"')
			dest->push_back(*source++);
		if (*source)
			++source;
	}
	return source;
}

// commonutil.cpp: GetCharsTillSpace
static const char* GetCharsTillSpace(const char* source, std::string* dest)
{
	dest->clear();
	while (*source == ' ' || *source == '\t')
		++source;
	while (*source && *source != ' ')
		dest->push_back(*source++);
	return source;
}

void ParsePuppetScript(const std::string& script, std::vector<ScriptCommand>* out)
{
	out->clear();
	const char* p = script.c_str();
	while ((p = strchr(p, '*')) != 0)
	{
		int len = 0;
		while (p[len])
		{
			if (p[++len] == ' ')
				break;
			if (p[len] == '(')
			{
				++len;
				break;
			}
		}
		if (len <= 1)
			break;

		ScriptCommand cmd;
		cmd.token.assign(p, len);
		p += len;
		if (cmd.token[cmd.token.size() - 1] == '(')
		{
			for (;;)
			{
				const char* q = p;
				while (*q == ' ' || *q == '\t')
					++q;
				if (*q != '"')
					break;
				std::string arg;
				p = GetStrConst(p, &arg);
				cmd.args.push_back(arg);
			}
		}
		else
		{
			const char* next = strchr(p, '*');
			cmd.rest = next ? std::string(p, next - p) : std::string(p);
		}
		out->push_back(cmd);
	}
}

bool IsSoundCommand(const ScriptCommand& cmd)
{
	return !_stricmp(cmd.token.c_str(), "*snd(") && !cmd.args.empty() && !cmd.args[0].empty();
}

bool ParseFxCommand(const ScriptCommand& cmd, ScriptFx* out)
{
	if (_stricmp(cmd.token.c_str(), "*fx(") || cmd.args.empty() || cmd.args[0].empty())
		return false;
	out->motion.clear();
	out->file.clear();
	out->parts.clear();
	out->bones.clear();
	size_t i = 0;
	// Newer data leads with a "#tag" this client does not know, as in "#대천사" "@motion".
	while (i < cmd.args.size() && !cmd.args[i].empty() && cmd.args[i][0] == '#')
		++i;
	if (i >= cmd.args.size() || cmd.args[i].empty())
		return false;
	if (cmd.args[i][0] == '@')
	{
		out->motion = cmd.args[i].substr(1);
		++i;
	}
	if (i >= cmd.args.size() || cmd.args[i].empty())
		return false;
	out->file = cmd.args[i++];
	for (; i < cmd.args.size() && !cmd.args[i].empty() && cmd.args[i][0] == '&'; ++i)
		out->parts.push_back(cmd.args[i].substr(1));
	for (; i < cmd.args.size() && out->bones.size() < 4 && !cmd.args[i].empty(); ++i)
		out->bones.push_back(cmd.args[i]);
	return true;
}

bool IsKnownToken(const std::string& token)
{
	// Everything CPetBody and CGolfPlayer dispatch on, plus the parameter words the
	// caddie and shot code look up by name with strstr.
	static const char* known[] = {
		"*ptex(", "*snd(", "*fly", "*hide", "*show", "*fx(", "*lookup", "*dontlookup",
		"*shot", "*tail", "*showclub", "*hideclub", "*swing", "*lockclub", "*showbone(",
		"*hidebone(", "*playif(", "*opa(", "*showvis(", "*hidevis(", "*stop_dist",
		"*start_delay", "*pet_speed", "*ball_dist", "*grip_pos", "*opa", "*give", "*ring",
		"*no_caddie_view", "*petbillboard", "*stepsnd(", "*fxstop(",
	};
	for (size_t i = 0; i < sizeof(known) / sizeof(known[0]); ++i)
		if (!_stricmp(token.c_str(), known[i]))
			return true;
	return false;
}

void ParseBoxScript(const std::string& option, std::vector<BoxFx>* out)
{
	out->clear();
	const char* p = option.c_str();
	while ((p = strstr(p, "*fx")) != 0)
	{
		p += 3;
		BoxFx fx;
		if (*p == '(')
		{
			p = GetStrConst(p + 1, &fx.file);
			std::string cond;
			for (;;)
			{
				const char* next = GetStrConst(p, &cond);
				if (cond.empty() || cond[0] != '&')
					break;
				fx.parts.push_back(cond.substr(1));
				p = next;
			}
		}
		else
		{
			p = GetCharsTillSpace(p, &fx.file);
		}
		const char* dot = strrchr(fx.file.c_str(), '.');
		if (dot && (!_stricmp(dot, ".spr") || !_stricmp(dot, ".seq")))
			out->push_back(fx);
		if (!*p)
			break;
	}
}
