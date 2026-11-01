// pethost: carrega um boneco com o parser REAL do cliente (WResourceManager::GetPuppet -> WPuppet::LoadPET /
// ReadVersion983) e desenha com o caminho real (WxPuppet::Render -> WxTnLBuffer::xRender), sem servidor e sem login.
// Uso: pethost nome.pet [--pak pasta] [--bmp saida.bmp] [--yaw graus] [--dist n] [--anim t] [--legacy]
// Sem --pak o arquivo vem solto, pelo caminho relativo ao diretorio atual.
#include <windows.h>
#include <io.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <string>
#include <vector>
#include "wresrcmng.h"
#include "wdevmng.h"
#include "wvideo.h"
#include "wview.h"
#include "wpuppet.h"
#include "wxpuppet.h"
#include "wbone.h"
#include "wmesh.h"
#include "wscene.h"
#include "bitmap.h"

void __cdecl HookPAKFile(const char* filename);

static WResourceManager* g_res;

// Igual a ShaderIncludeProc983 do jogo: a dll chama como __fastcall
static bool __fastcall ShaderInclude(const char* name, const void** data, unsigned int* bytes)
{
	cFile* file = g_res->GetCFile(name, -1);
	if (!file)
		return false;
	*bytes = file->Length();
	void* mem = operator new(*bytes);
	file->Read(mem, *bytes);
	*data = mem;
	CloseCFile(file);
	return true;
}

// Igual a CProjectG::UploadShaderSource: sem isto ossos com paleta caem no pipeline fixo e saem errados
static void UploadShader(WVideoDev* video)
{
	cFile* file = g_res->GetCFile("pangya.fx", -1);
	if (!file)
	{
		video->SetShaderSource(NULL);
		puts("aviso: sem pangya.fx");
		return;
	}
	std::vector<char> text(file->Length() + 1);
	file->Read(&text[0], file->Length());
	video->SetShaderSource(&text[0]);
	CloseCFile(file);
}

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) { return DefWindowProcA(h, m, w, l); }

static void HookPaks(const char* dir)
{
	std::vector<std::string> names;
	WIN32_FIND_DATAA fd;
	HANDLE h = FindFirstFileA((std::string(dir) + "\\projectg*.pak").c_str(), &fd);
	if (h == INVALID_HANDLE_VALUE)
		return;
	do
		names.push_back(fd.cFileName);
	while (FindNextFileA(h, &fd));
	FindClose(h);
	std::sort(names.begin(), names.end());
	for (size_t i = 0; i < names.size(); ++i)
		HookPAKFile((std::string(dir) + "\\" + names[i]).c_str());
	printf("paks: %d\n", (int)names.size());
}

static void SaveBmp(const char* path, Bitmap* img)
{
	int w = img->bi->bmiHeader.biWidth, h = abs(img->bi->bmiHeader.biHeight);
	int pitch = (w * 3 + 3) & ~3;
	std::vector<unsigned char> row(pitch);
	FILE* f = fopen(path, "wb");
	if (!f)
		return;
	BITMAPFILEHEADER fh = { 0x4d42, (DWORD)(54 + pitch * h), 0, 0, 54 };
	BITMAPINFOHEADER ih = { 40, w, h, 1, 24, 0, (DWORD)(pitch * h) };
	fwrite(&fh, sizeof(fh), 1, f);
	fwrite(&ih, sizeof(ih), 1, f);
	for (int y = h - 1; y >= 0; --y)
	{
		for (int x = 0; x < w; ++x)
		{
			unsigned char r, g, b;
			img->GetPixel(x, y, r, g, b);
			row[x * 3] = b;
			row[x * 3 + 1] = g;
			row[x * 3 + 2] = r;
		}
		fwrite(&row[0], pitch, 1, f);
	}
	fclose(f);
}

int __cdecl main(int argc, char** argv)
{
	if (argc < 2)
	{
		puts("uso: pethost nome.pet [--pak pasta] [--bmp saida.bmp] [--yaw graus] [--dist n] [--anim t] [--legacy]");
		return 2;
	}
	const char* name = argv[1];
	const char* pak = 0;
	const char* bmp = 0;
	float yaw = 30.0f, dist = 0.0f, animTime = -1.0f;
	bool legacy = false;
	std::vector<const char*> wear;	// pecas .mpet vestidas por nome de osso, como CPartsInfo
	for (int i = 2; i < argc; ++i)
	{
		if (!strcmp(argv[i], "--pak") && i + 1 < argc)
			pak = argv[++i];
		else if (!strcmp(argv[i], "--bmp") && i + 1 < argc)
			bmp = argv[++i];
		else if (!strcmp(argv[i], "--yaw") && i + 1 < argc)
			yaw = (float)atof(argv[++i]);
		else if (!strcmp(argv[i], "--dist") && i + 1 < argc)
			dist = (float)atof(argv[++i]);
		else if (!strcmp(argv[i], "--anim") && i + 1 < argc)
			animTime = (float)atof(argv[++i]);
		else if (!strcmp(argv[i], "--wear") && i + 1 < argc)
			wear.push_back(argv[++i]);
		else if (!strcmp(argv[i], "--legacy"))
			legacy = true;
	}
	setvbuf(stdout, 0, _IONBF, 0);
	WNDCLASSA wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandleA(0);
	wc.lpszClassName = "pethost";
	RegisterClassA(&wc);
	HWND hwnd = CreateWindowA("pethost", "pethost", WS_OVERLAPPEDWINDOW, 40, 40, 816, 639, 0, 0, wc.hInstance, 0);
	ShowWindow(hwnd, SW_SHOWNOACTIVATE);

	WDeviceManager* devices = new WDeviceManager(NULL, 1);
	devices->SetHWND(hwnd);
	WResourceManager* res = new WResourceManager;
	WVideoDev* video = devices->CreateVideoDevice(NULL, (char*)"Window", 1);
	if (!video)
	{
		puts("FALHA: sem dispositivo de video (wangreal.dll na pasta do exe?)");
		return 3;
	}
	video->SetMainThreadId(GetCurrentThreadId());
	res->SetVideoReference(video);
	g_res = res;
	video->Command(WX_VDEV_SET_SHADER_INCLUDE_PROC, (int)ShaderInclude, 0);
	if (pak)
	{
		HookPaks(pak);
		res->SetAutoMatchDirectory("data");	// como InitPakedFile: nome curto acha o caminho no pak
	}
	WView* view = res->GetView(0);
	UploadShader(video);

	WPuppet* pet = res->GetPuppet(name, false, true, legacy);
	if (!pet)
	{
		printf("FALHA: GetPuppet(%s) devolveu nulo\n", name);
		return 4;
	}
	for (size_t i = 0; i < wear.size(); ++i)
	{
		WPuppet* part = res->GetPuppet(wear[i], false, false, false);
		printf("peca %s: %s\n", wear[i], part ? "ok" : "FALHOU");
		if (part)
			pet->ReplaceMeshBones(0, part, 0, 0);
	}
	if (!wear.empty())
	{
		pet->EndMeshBoneWork();
		pet->xUpdateTnLBuffers(true);
	}
	unsigned long caps = 0;
	video->Command(WX_VDEV_GET_CAPS, 1, (int)&caps);
	printf("OK %s versao=%08lx classe=%s caps=%lx faces=%d aniLen=%.3f\n", name, (unsigned long)pet->m_petVersion983,
		dynamic_cast<WxPuppet*>(pet) ? "WxPuppet" : "WPuppet", caps, pet->m_faceNum, pet->GetMotionLength(0));
	if (!bmp)
		ExitProcess(0);

	WMatrix identity;
	identity.Reset();
	pet->SetMatrix(identity);
	pet->UpdateBound(true, false);
	const WSphere& sphere = pet->GetBoundSphere();
	float radius = sphere.radius > 0.01f ? sphere.radius : 3.0f;
	if (dist <= 0.0f)
		dist = radius * 2.6f;
	WVector center = sphere.pos;
	printf("esfera centro=(%.2f %.2f %.2f) raio=%.2f\n", center.x, center.y, center.z, radius);
	float a = yaw * 3.14159265f / 180.0f;
	WVector eye(center.x + sinf(a) * dist, center.y + dist * 0.35f, center.z - cosf(a) * dist);
	WVector dir = center - eye;
	dir.Normalize();
	WVector right = WCrossProduct(WVector(0.0f, 1.0f, 0.0f), dir);
	right.Normalize();
	WVector up = WCrossProduct(dir, right);
	WMatrix cam;
	cam.Reset();
	cam.xa = right;
	cam.ya = up;
	cam.za = dir;
	cam.pivot = eye;

	LightSet light;
	light.type = 1;
	light.nearOne = WVector(-0.4f, -0.8f, 0.45f);
	light.nearOne.Normalize();
	light.diffuse = 0xffc8c8c8;
	light.ambient = 0xff707070;
	light.ambient2 = 0xff707070;
	pet->UpdateLightSource(&light, false, 0);
	view->SetClip(0.1f, 4000.0f, false);
	for (int frame = 0; frame < 3; ++frame)
	{
		if (animTime >= 0.0f)
			pet->Animate983(NULL, NULL, animTime, 0, NULL, NULL);	// mesmo chamado da vitrine (CExhibition)
		view->SetCamera(cam);
		view->BeginScene();
		view->Clear(0xff304050, 6);
		pet->Render(view, true, 1.0f, false, true, false);
		view->Flush(0, 0);
		view->EndScene();
		video->EndScene();
		if (frame == 2)
			break;
		view->Render();
		Sleep(30);
	}
	Bitmap* img = new Bitmap(video->GetWidth(), video->GetHeight(), 24);
	int cap = video->Command(W_VDEV_CAPTURE_SCREEN, 0, (int)img);
	printf("captura=%d %dx%d\n", cap, video->GetWidth(), video->GetHeight());
	if (cap == 1)
		SaveBmp(bmp, img);
	ExitProcess(0);
}
