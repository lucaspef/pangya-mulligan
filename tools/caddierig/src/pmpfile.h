// SPDX-License-Identifier: MIT
// pmpfile.h - PMPF project files ("PMPF VER 2.21").
//
// A project is a flat list of blocks. Blocks keep their fields as text, so
// everything CaddieRig itself never reads (원본파일, 원본프레임, ...) survives.
// All text in memory is UTF-8; the on-disk encoding is remembered per file:
// the tool wrote CP949 at the time, and archived copies have been re-encoded.
#ifndef PMPFILE_H
#define PMPFILE_H

#include <stdio.h>
#include <string>
#include <vector>
#include "wcommon.h"

struct PmpField
{
	std::string keyRaw;		// key text with its alignment spaces
	std::string key;		// trimmed
	std::string value;
};

enum PmpBlockKind
{
	PMP_TEXTURE,	// 텍스쳐
	PMP_BOX,		// 충돌박스
	PMP_MOTION,		// 동작
	PMP_SCRIPT,		// 스크립트
	PMP_UNKNOWN
};

struct PmpBlock
{
	PmpBlockKind kind;
	std::string name;	// text before '('
	std::string arg;	// "void" or a frame such as "0012"
	std::vector<PmpField> fields;

	const std::string& Get(const char* key) const;
	float GetFloat(const char* key) const;
	int GetInt(const char* key) const;
	void Set(const char* key, const std::string& value);
};

class PmpFile
{
public:
	PmpFile();
	void Clear();
	bool Load(const char* path, std::string* error);
	bool Parse(const std::string& utf8, std::string* error);
	bool Save(const char* path, std::string* error) const;
	std::string Serialize() const;

	static PmpBlock NewTexture(const std::string& name, const std::string& file, int link);
	static PmpBlock NewBox(const std::string& name, const std::string& bone, const float box[6]);
	static PmpBlock NewMotion(const std::string& name, int start, int end, const std::string& next,
		const std::string& applyBone, const std::string& source, int srcStart, int srcEnd);
	static PmpBlock NewScript(int frame, const std::string& message, const std::string& source, int srcFrame);

	std::string version;	// "2.21"
	std::string aseName;
	std::vector<PmpBlock> blocks;
	std::string eol;		// "\r\n" or "\n"
	bool cp949;
	bool finalEol;
};

#endif // PMPFILE_H
