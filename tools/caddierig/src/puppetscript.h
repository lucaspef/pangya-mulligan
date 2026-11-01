// SPDX-License-Identifier: MIT
// puppetscript.h - the commands in FRAM messages and collision box scripts.
//
// Tokenizing follows CPetBody::ParsePuppetScript: a token runs from '*' up to a
// space or through a '('. That means "*hideclub*ptex(" is one unknown token, and
// the client ignores it, so this does too.
#ifndef PUPPETSCRIPT_H
#define PUPPETSCRIPT_H

#include <string>
#include <vector>

struct ScriptCommand
{
	std::string token;				// "*snd(", "*fly", ...
	std::vector<std::string> args;	// the quoted strings that follow
	std::string rest;				// unquoted text up to the next '*'
};

// A parsed *fx( call.
struct ScriptFx
{
	std::string motion;				// from "@name"; only fires during that motion
	std::string file;				// .spr, .seq, or "*playbbfx"
	std::vector<std::string> parts;	// "&id" conditions: fires if any part is worn
	std::vector<std::string> bones;	// up to 4 bones or boxes, positions averaged
};

// An effect named by a collision box.
struct BoxFx
{
	std::string file;
	std::vector<std::string> parts;
};

void ParsePuppetScript(const std::string& script, std::vector<ScriptCommand>* out);
bool ParseFxCommand(const ScriptCommand& cmd, ScriptFx* out);
bool IsSoundCommand(const ScriptCommand& cmd);
bool IsKnownToken(const std::string& token);

// Both box forms: *fx("a.seq" "&id") for characters, *fx a.seq for clubs and props.
void ParseBoxScript(const std::string& option, std::vector<BoxFx>* out);

#endif // PUPPETSCRIPT_H
