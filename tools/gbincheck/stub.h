// Só o que os trechos reais de polysoup.cpp usam, com o layout do cliente (wmath.h, cfile.h, wpuppetutil.h).
#pragma once
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <list>
#include <map>
#include <string>
#include <vector>

struct WVector
{
	float x, y, z;
	WVector() : x(0), y(0), z(0) { }
	WVector(const float* v) : x(v[0]), y(v[1]), z(v[2]) { }
	void Reset() { x = y = z = 0; }
	float Magnitude() const { return sqrtf(x * x + y * y + z * z); }
};

struct WPlane { float a, b, c, d; };

struct Waabb
{
	WVector min, max;
	void Reset() { min.Reset(); max.Reset(); }
};

struct WMatrix
{
	WVector xa, ya, za, pivot;
	WMatrix() { Reset(); }
	void Reset()
	{
		xa.Reset(); ya.Reset(); za.Reset(); pivot.Reset();
		xa.x = ya.y = za.z = 1;
	}
};

// Olho e alvo vão para a matriz sem a conta do motor: a comparação quer os valores lidos.
inline WMatrix MakeCameraMatrix(const WVector& eye, const WVector& at)
{
	WMatrix m;
	m.pivot = eye;
	m.za = at;
	return m;
}

class cFile
{
public:
	cFile(const std::vector<unsigned char>& d) : data(d), at(0), overrun(false) { }
	int Read(void* dst, int n)
	{
		if (at + n > (long)data.size())
		{
			overrun = true;
			memset(dst, 0, n);
			return 0;
		}
		memcpy(dst, &data[at], n);
		at += n;
		return n;
	}
	int GetByte() { unsigned char c = 0; Read(&c, 1); return c; }
	int Seek(long off, int mode)
	{
		at = mode == SEEK_SET ? off : mode == SEEK_CUR ? at + off : (long)data.size() + off;
		return 0;
	}
	int ReadAabb983(Waabb& aabb);
	int ReadMatrix983(WMatrix& mat);
	const std::vector<unsigned char>& data;
	long at;
	bool overrun;
};

struct sBgModel
{
	unsigned char flag;
	int faceNum;
	char name[32];
	Waabb localAabb;
	Waabb sceneAabb;
	WMatrix mat;
	int detail;
	char option[32];
	sBgModel() { flag = 0; }
};

#include "../../client/src/projectg/nodecontainer.h"

// Só o caminho 0x71 de cVertexColorTable983::Read usa isto: uma malha cobre todos os triângulos do modelo base.
struct w_mesh
{
	w_mesh* next;
	unsigned int vtxNum;
	int indexNum;
	unsigned short* indexList;
	unsigned long* vtxColorList;
};
class WBone
{
public:
	char m_name[64];
	w_mesh* mesh;
	w_mesh* GetMesh() { return mesh; }
};
class WPuppet
{
public:
	WBone* root;
	WBone* GetRootBone() { return root; }
	WBone* FindBone(const char*) { return root; }
};

namespace IFF_STRUCT { struct sCourse { int DiffFlag; }; }
struct sDocStub { struct { IFF_STRUCT::sCourse* pCourse; } m_golfGame; };
sDocStub* Doc();

inline int MessageBox(void*, const char*, const char*, int) { return 0; }
#define K2L_Compatibility(x) (x)
