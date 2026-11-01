// fwcompile: o compilador do FairwayED pela linha de comando, sem alterar o código do autor.
//   fwcompile <entrada.wep|.gbin> <saida.gbin> --root <pasta de dados> [--bake] [--check]
// --bake calcula as cores de vértice do modelo base (Build > Light Base Model); --check roda o Map Check.
#include <windows.h>
#include <stdio.h>
#include <string>
#include "course.h"
#include "wcommon.h"

int wmain(int argc, wchar_t** argv)
{
	SetConsoleOutputCP(CP_UTF8);
	if (argc < 3)
	{
		printf("uso: fwcompile <entrada> <saida.gbin> --root <dados> [--bake] [--check]\n");
		return 2;
	}
	std::string root;
	bool bake = false, check = false;
	for (int i = 3; i < argc; ++i)
	{
		if (!wcscmp(argv[i], L"--root") && i + 1 < argc)
			root = Narrow(argv[++i]);
		else if (!wcscmp(argv[i], L"--bake"))
			bake = true;
		else if (!wcscmp(argv[i], L"--check"))
			check = true;
	}
	Course course;
	CourseLog log;
	course.SetDataRoot(root);
	bool ok = course.Open(Narrow(argv[1]), &log);
	if (ok && bake)
		course.BakeBaseColors(&log);
	if (ok && check)
		course.Validate(&log);
	if (ok)
		ok = course.ExportHole(Narrow(argv[2]), &log);
	for (size_t i = 0; i < log.lines.size(); ++i)
		printf("%s\n", log.lines[i].c_str());
	return ok ? 0 : 1;
}
