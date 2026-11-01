// SPDX-License-Identifier: MIT
// pmpfile.cpp - PMPF text reader and writer.
#include "pmpfile.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool LooksUtf8(const std::string& s)
{
	return MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), 0, 0) > 0;
}

static std::string Trim(const std::string& s)
{
	size_t a = s.find_first_not_of(" \t\r");
	if (a == std::string::npos)
		return std::string();
	size_t b = s.find_last_not_of(" \t\r");
	return s.substr(a, b - a + 1);
}

const std::string& PmpBlock::Get(const char* key) const
{
	static const std::string empty;
	for (size_t i = 0; i < fields.size(); ++i)
		if (fields[i].key == key)
			return fields[i].value;
	return empty;
}

float PmpBlock::GetFloat(const char* key) const { return (float)atof(Get(key).c_str()); }
int PmpBlock::GetInt(const char* key) const { return atoi(Get(key).c_str()); }

void PmpBlock::Set(const char* key, const std::string& value)
{
	for (size_t i = 0; i < fields.size(); ++i)
	{
		if (fields[i].key == key)
		{
			fields[i].value = value;
			return;
		}
	}
	PmpField f;
	f.key = key;
	f.keyRaw = key;
	f.value = value;
	fields.push_back(f);
}

PmpFile::PmpFile()
{
	Clear();
}

void PmpFile::Clear()
{
	version = "2.21";
	aseName.clear();
	blocks.clear();
	eol = "\r\n";
	cp949 = true;
	finalEol = true;
}

static PmpBlockKind KindOf(const std::string& name)
{
	if (name == "텍스쳐")
		return PMP_TEXTURE;
	if (name == "충돌박스")
		return PMP_BOX;
	if (name == "동작")
		return PMP_MOTION;
	if (name == "스크립트")
		return PMP_SCRIPT;
	return PMP_UNKNOWN;
}

bool PmpFile::Parse(const std::string& text, std::string* error)
{
	blocks.clear();
	aseName.clear();
	// PMPF 2.2 wrote its CRLFs through a text-mode stream, so they came out as CR CR LF
	if (text.find("\r\r\n") != std::string::npos)
		eol = "\r\r\n";
	else
		eol = text.find("\r\n") != std::string::npos ? "\r\n" : "\n";
	finalEol = !text.empty() && text[text.size() - 1] == '\n';

	std::vector<std::string> lines;
	size_t pos = 0;
	while (pos < text.size())
	{
		size_t nl = text.find('\n', pos);
		if (nl == std::string::npos)
			nl = text.size();
		std::string line = text.substr(pos, nl - pos);
		while (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		lines.push_back(line);
		pos = nl + 1;
	}

	if (lines.empty() || lines[0].compare(0, 9, "PMPF VER ") != 0)
	{
		if (error)
			*error = "not a PMPF file";
		return false;
	}
	version = lines[0].substr(9);

	PmpBlock* cur = 0;
	for (size_t i = 1; i < lines.size(); ++i)
	{
		const std::string& line = lines[i];
		std::string t = Trim(line);
		if (t.empty() || t == "{")
			continue;
		if (t == "}")
		{
			cur = 0;
			continue;
		}
		if (!cur)
		{
			if (t.compare(0, 3, "ASE") == 0)
			{
				size_t c = t.find(" : ");
				if (c != std::string::npos)
					aseName = t.substr(c + 3);
				continue;
			}
			size_t open = t.find('(');
			size_t close = t.rfind(')');
			if (open == std::string::npos || close == std::string::npos || close < open)
				continue;
			PmpBlock b;
			b.name = Trim(t.substr(0, open));
			b.arg = Trim(t.substr(open + 1, close - open - 1));
			b.kind = KindOf(b.name);
			blocks.push_back(b);
			cur = &blocks.back();
			continue;
		}
		// "모양(o,ㅁ) : 1" - the separator is the first " : "; empty values may end in " :"
		size_t c = line.find(" : ");
		PmpField f;
		if (c != std::string::npos)
		{
			f.keyRaw = line.substr(0, c);
			f.value = line.substr(c + 3);
		}
		else
		{
			f.keyRaw = line;
			size_t k = f.keyRaw.rfind(" :");
			if (k != std::string::npos)
				f.keyRaw.erase(k);
		}
		if (!f.keyRaw.empty() && f.keyRaw[0] == '\t')
			f.keyRaw.erase(0, 1);
		f.key = Trim(f.keyRaw);
		cur->fields.push_back(f);
	}
	return true;
}

bool PmpFile::Load(const char* path, std::string* error)
{
	FILE* f = OpenUtf8(path, L"rb");
	if (!f)
	{
		if (error)
			*error = "cannot open file";
		return false;
	}
	std::string data;
	char buf[65536];
	size_t n;
	while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
		data.append(buf, n);
	fclose(f);
	cp949 = !LooksUtf8(data);
	return Parse(cp949 ? Cp949ToUtf8(data) : data, error);
}

std::string PmpFile::Serialize() const
{
	std::string s = "PMPF VER " + version + eol + eol;
	s += "ASE 이름 : " + aseName + eol + eol;
	for (size_t i = 0; i < blocks.size(); ++i)
	{
		const PmpBlock& b = blocks[i];
		s += b.name + "( " + b.arg + " ) " + eol + "{" + eol;
		for (size_t k = 0; k < b.fields.size(); ++k)
		{
			s += "\t" + b.fields[k].keyRaw + " : " + b.fields[k].value + eol;
		}
		s += "}" + eol + eol;
	}
	// blocks are separated, not followed, by a blank line
	if (!blocks.empty())
		s.erase(s.size() - eol.size());
	if (!finalEol && s.size() >= eol.size())
		s.erase(s.size() - eol.size());
	return s;
}

bool PmpFile::Save(const char* path, std::string* error) const
{
	FILE* f = OpenUtf8(path, L"wb");
	if (!f)
	{
		if (error)
			*error = "cannot create file";
		return false;
	}
	std::string s = Serialize();
	if (cp949)
		s = Utf8ToCp949(s);
	fwrite(s.data(), 1, s.size(), f);
	fclose(f);
	return true;
}

static void AddField(PmpBlock& b, const char* keyRaw, const std::string& value)
{
	PmpField f;
	f.keyRaw = keyRaw;
	f.key = Trim(keyRaw);
	f.value = value;
	b.fields.push_back(f);
}

PmpBlock PmpFile::NewTexture(const std::string& name, const std::string& file, int link)
{
	PmpBlock b;
	b.kind = PMP_TEXTURE;
	b.name = "텍스쳐";
	b.arg = "void";
	AddField(b, "부모텍스쳐", "@");
	AddField(b, "텍스쳐이름", name);
	AddField(b, "텍스쳐파일", file);
	char num[16];
	sprintf_s(num, "%d", link);
	AddField(b, "텍스쳐연결", num);
	return b;
}

PmpBlock PmpFile::NewBox(const std::string& name, const std::string& bone, const float box[6])
{
	PmpBlock b;
	b.kind = PMP_BOX;
	b.name = "충돌박스";
	b.arg = "void";
	AddField(b, "모양(o,ㅁ)", "1");
	AddField(b, "보이기    ", "1");
	AddField(b, "박스이름  ", name);
	AddField(b, "부모본이름", bone);
	AddField(b, "스크립트0 ", "@");
	AddField(b, "스크립트1 ", "@");
	static const char* keys[6] = { "최소값x   ", "최소값y   ", "최소값z   ", "최대값x   ", "최대값y   ", "최대값z   " };
	for (int i = 0; i < 6; ++i)
	{
		char num[32];
		sprintf_s(num, "%f", box[i]);
		AddField(b, keys[i], num);
	}
	return b;
}

PmpBlock PmpFile::NewMotion(const std::string& name, int start, int end, const std::string& next,
	const std::string& applyBone, const std::string& source, int srcStart, int srcEnd)
{
	PmpBlock b;
	b.kind = PMP_MOTION;
	b.name = "동작";
	b.arg = "void";
	char num[32];
	AddField(b, "동작이름", name);
	sprintf_s(num, "%.4f", srcStart / 30.0f);
	AddField(b, "원본시작시간", num);
	sprintf_s(num, "%.4f", srcEnd / 30.0f);
	AddField(b, "원본종료시간", num);
	AddField(b, "다음동작", next);
	AddField(b, "연결방법", "Transition");
	AddField(b, "연결시간", "0.0000");
	AddField(b, "최상위본", applyBone);
	sprintf_s(num, "%d", srcStart);
	AddField(b, "원본시작프레임", num);
	sprintf_s(num, "%d", srcEnd);
	AddField(b, "원본종료프레임", num);
	AddField(b, "원본길이사용", "Yes");
	AddField(b, "원본파일", source);
	AddField(b, "원본유무", source.empty() ? "없음" : "있음");
	sprintf_s(num, "%d", start);
	AddField(b, "시작프레임", num);
	sprintf_s(num, "%d", end);
	AddField(b, "종료프레임", num);
	AddField(b, "그룹이름", "def");
	return b;
}

PmpBlock PmpFile::NewScript(int frame, const std::string& message, const std::string& source, int srcFrame)
{
	PmpBlock b;
	b.kind = PMP_SCRIPT;
	b.name = "스크립트";
	char num[32];
	sprintf_s(num, "%04d", frame);
	b.arg = num;
	AddField(b, "메시지", message);
	AddField(b, "플래그", "0");
	AddField(b, "원본파일", source);
	sprintf_s(num, "%d", srcFrame);
	AddField(b, "원본프레임", num);
	return b;
}
