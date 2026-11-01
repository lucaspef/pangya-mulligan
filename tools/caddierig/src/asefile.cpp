// SPDX-License-Identifier: MIT
// asefile.cpp - line based ASE parser. Tag names are compared in CP949-neutral
// ASCII; quoted node names stay in the file's own encoding (CP949).
#include "asefile.h"
#include "pmpfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace
{
	struct Scan
	{
		const char* p;
		explicit Scan(const char* s) : p(s) { }

		void Skip()
		{
			while (*p == ' ' || *p == '\t' || *p == '\r')
				++p;
		}
		std::string Tag()
		{
			Skip();
			const char* s = p;
			while (*p && *p != ' ' && *p != '\t' && *p != '\r')
				++p;
			return std::string(s, p - s);
		}
		int Int()
		{
			Skip();
			char* e;
			int v = strtol(p, &e, 10);
			p = e;
			return v;
		}
		float Float()
		{
			Skip();
			char* e;
			float v = (float)strtod(p, &e);
			p = e;
			return v;
		}
		WVector Vec()
		{
			WVector v;
			v.x = Float();
			v.y = Float();
			v.z = Float();
			return v;
		}
		std::string Quoted()
		{
			const char* s = strchr(p, '"');
			if (!s)
				return std::string();
			const char* e = strrchr(s + 1, '"');
			if (!e)
				return std::string(s + 1);
			return std::string(s + 1, e - s - 1);
		}
		// Advances past the next "name:" label and reads the integer after it.
		int Labeled(const char* label)
		{
			const char* s = strstr(p, label);
			if (!s)
				return 0;
			p = s + strlen(label);
			return Int();
		}
	};

	// Everything nested inside a block is matched by tag, so depth only has to
	// be tracked for the material list where sub materials recurse.
	struct Parser
	{
		AseFile& ase;
		std::vector<AseMaterial*> mtlStack;
		AseNode* node;
		int mapDepth;		// *MAP_xxx blocks open inside the current material
		int skipDepth;		// depth of the first non-diffuse map, 0 when none

		explicit Parser(AseFile& a) : ase(a), node(0), mapDepth(0), skipDepth(0) { }

		void Line(const char* line);
		void MaterialLine(const std::string& tag, Scan& s);
	};
}

void Parser::MaterialLine(const std::string& tag, Scan& s)
{
	AseMaterial* m = mtlStack.back();
	if (tag == "*MATERIAL_NAME")
		m->name = s.Quoted();
	else if (tag == "*NUMSUBMTLS")
		m->numSubs = s.Int();
	else if (tag == "*MATERIAL_DIFFUSE")
	{
		m->diffuse[0] = s.Float();
		m->diffuse[1] = s.Float();
		m->diffuse[2] = s.Float();
	}
	else if (tag == "*MATERIAL_TRANSPARENCY")
		m->transparency = s.Float();
	else if (tag == "*BITMAP" && m->bitmap.empty())
		m->bitmap = s.Quoted();
}

void Parser::Line(const char* line)
{
	Scan s(line);
	std::string tag = s.Tag();
	if (tag.empty())
		return;

	if (tag == "}")
	{
		// Only material scopes need popping; map blocks inside them just nest.
		if (mapDepth > 0)
		{
			if (mapDepth == skipDepth)
				skipDepth = 0;
			--mapDepth;
		}
		else if (!mtlStack.empty())
			mtlStack.pop_back();
		return;
	}

	if (!mtlStack.empty())
	{
		if (tag == "*SUBMATERIAL")
		{
			mtlStack.back()->subs.push_back(AseMaterial());
			AseMaterial* sub = &mtlStack.back()->subs.back();
			sub->diffuse[0] = sub->diffuse[1] = sub->diffuse[2] = 1.0f;
			sub->transparency = 0.0f;
			sub->index = s.Int();
			sub->numSubs = 0;
			mtlStack.push_back(sub);
		}
		else if (tag.compare(0, 4, "*MAP") == 0 && strchr(line, '{'))
		{
			// Only MAP_DIFFUSE names the texture; specular/opacity maps must not win.
			++mapDepth;
			if (tag != "*MAP_DIFFUSE" && !skipDepth)
				skipDepth = mapDepth;
		}
		else if (tag != "*BITMAP" || !skipDepth)
		{
			MaterialLine(tag, s);
		}
		return;
	}

	if (tag == "*SCENE_FIRSTFRAME")
		ase.firstFrame = s.Int();
	else if (tag == "*SCENE_LASTFRAME")
		ase.lastFrame = s.Int();
	else if (tag == "*SCENE_FRAMESPEED")
		ase.frameSpeed = s.Int();
	else if (tag == "*SCENE_TICKSPERFRAME")
		ase.ticksPerFrame = s.Int();
	else if (tag == "*MATERIAL")
	{
		ase.materials.push_back(AseMaterial());
		AseMaterial* m = &ase.materials.back();
		m->diffuse[0] = m->diffuse[1] = m->diffuse[2] = 1.0f;
		m->transparency = 0.0f;
		m->index = s.Int();
		m->numSubs = 0;
		mtlStack.push_back(m);
	}
	else if (tag == "*GEOMOBJECT" || tag == "*HELPEROBJECT")
	{
		ase.nodes.push_back(AseNode());
		node = &ase.nodes.back();
		node->helper = tag == "*HELPEROBJECT";
		node->materialRef = -1;
		node->tm[0] = WVector(1, 0, 0);
		node->tm[1] = WVector(0, 1, 0);
		node->tm[2] = WVector(0, 0, 1);
	}
	else if (node)
	{
		if (tag == "*NODE_NAME")
		{
			if (node->name.empty())
				node->name = s.Quoted();
		}
		else if (tag == "*NODE_PARENT")
			node->parent = s.Quoted();
		else if (tag == "*TM_ROW0")
			node->tm[0] = s.Vec();
		else if (tag == "*TM_ROW1")
			node->tm[1] = s.Vec();
		else if (tag == "*TM_ROW2")
			node->tm[2] = s.Vec();
		else if (tag == "*TM_ROW3")
			node->tm[3] = s.Vec();
		else if (tag == "*MATERIAL_REF")
			node->materialRef = s.Int();
		else if (tag == "*MESH_NUMVERTEX")
			node->verts.resize(s.Int());
		else if (tag == "*MESH_NUMFACES")
			node->faces.resize(s.Int());
		else if (tag == "*MESH_VERTEX")
		{
			int i = s.Int();
			if (i >= 0 && i < (int)node->verts.size())
				node->verts[i] = s.Vec();
		}
		else if (tag == "*MESH_FACE")
		{
			int i = s.Int();
			if (i >= 0 && i < (int)node->faces.size())
			{
				AseFace& f = node->faces[i];
				f.v[0] = s.Labeled("A:");
				f.v[1] = s.Labeled("B:");
				f.v[2] = s.Labeled("C:");
				f.smooth = s.Labeled("*MESH_SMOOTHING");
				f.mtl = s.Labeled("*MESH_MTLID");
			}
		}
		else if (tag == "*MESH_NUMTVERTEX")
			node->tverts.resize(s.Int());
		else if (tag == "*MESH_NUMTVFACES")
			node->tfaces.assign(s.Int() * 3, 0);
		else if (tag == "*MESH_TVERT")
		{
			int i = s.Int();
			if (i >= 0 && i < (int)node->tverts.size())
				node->tverts[i] = s.Vec();
		}
		else if (tag == "*MESH_TFACE")
		{
			int i = s.Int();
			if (i >= 0 && i * 3 + 2 < (int)node->tfaces.size())
			{
				node->tfaces[i * 3] = s.Int();
				node->tfaces[i * 3 + 1] = s.Int();
				node->tfaces[i * 3 + 2] = s.Int();
			}
		}
		else if (tag == "*MESH_FACENORMAL")
		{
			s.Int();
			node->faceNormals.push_back(s.Vec());
		}
		else if (tag == "*MESH_VERTEXNORMAL")
		{
			s.Int();
			node->vertNormals.push_back(s.Vec());
		}
		else if (tag == "*CONTROL_POS_SAMPLE")
		{
			AseKeyPos k;
			k.tick = s.Int();
			k.v = s.Vec();
			node->posKeys.push_back(k);
		}
		else if (tag == "*CONTROL_ROT_SAMPLE")
		{
			AseKeyRot k;
			k.tick = s.Int();
			k.axis = s.Vec();
			k.angle = s.Float();
			node->rotKeys.push_back(k);
		}
	}
}

AseFile::AseFile()
{
	firstFrame = 0;
	lastFrame = 0;
	frameSpeed = 30;
	ticksPerFrame = 160;
}

int AseFile::FindNode(const std::string& name) const
{
	for (size_t i = 0; i < nodes.size(); ++i)
		if (!_stricmp(nodes[i].name.c_str(), name.c_str()))
			return (int)i;
	return -1;
}

bool AseFile::Load(const char* path, std::string* error)
{
	FILE* f = OpenUtf8(path, L"rb");
	if (!f)
	{
		if (error)
			*error = "cannot open file";
		return false;
	}
	static char line[4096];
	if (!fgets(line, sizeof(line), f) || strncmp(line, "*3DSMAX_ASCIIEXPORT", 19))
	{
		fclose(f);
		if (error)
			*error = "not an ASCII scene export";
		return false;
	}
	nodes.reserve(512);
	materials.reserve(256);
	Parser parser(*this);
	while (fgets(line, sizeof(line), f))
		parser.Line(line);
	fclose(f);
	return true;
}
