// gbincheck: carrega um .gbin com o parser real do cliente (trechos de polysoup.cpp) e compara dois arquivos.
//   gbincheck a.gbin            imprime os campos que o cliente lê
//   gbincheck a.gbin b.gbin [tol]  imprime as diferenças (DIFF <categoria> <chave>) e o resumo (SUM); sai com 1 se houver
#include "stub.h"

class WPolySoup
{
public:
#include "polysoup_decl.inl"

	// Só os campos que o ReadModel983 preenche.
	struct sModel
	{
		int type, flag;
		float scale;
		sLink983 link983;
		WMatrix mat, localMat983;
		char name[32], option[32];
		sModel() : type(0), flag(0), scale(1) { name[0] = option[0] = 0; }
	};

	static bool __fastcall ReadHeader983(cFile* file, sHeader& header);
	static bool __fastcall ReadCameras983(cFile* file, sHeader& header, sCamera*& camera);
	static bool __fastcall ReadPoints983(cFile* file, sHeader& header, sPoint*& point);
	static bool __fastcall ReadAreas983(cFile* file, sHeader& header, sArea*& area);
	static bool __fastcall ReadTextures983(cFile* file, sHeader& header, sTexture*& texture);
	static bool __fastcall ReadNodes983(cFile* file, sHeader& header, std::vector<CNodeContainer>& node);
	static bool __fastcall ReadModel983(cFile* file, sHeader& header, unsigned char detail, sModel*& model, bool* skipped);
	static bool __fastcall ReadModels983(cFile* file, sHeader& header, std::vector<sModel*>& model);
	static bool __fastcall ReadAttach983(cFile* file, sHeader& header, sAttach983*& attach);
};

struct sVertexColorArray983
{
	std::vector<unsigned long> colors;
	void Resize(unsigned int num);
	void Set(unsigned int index, unsigned long color);
	void ApplyTo(w_mesh* mesh);
};

class cVertexColorTable983
{
public:
	typedef std::map<unsigned int, sVertexColorArray983> MeshColors;
	std::map<std::string, MeshColors> m_bones;
	sVertexColorArray983& Get(const std::string& bone, unsigned int mesh, unsigned int vtxNum);
	void Read(int version, cFile* file, WPuppet* pet);
	void Apply(WPuppet* pet);
};

static IFF_STRUCT::sCourse g_course = { 3 };	// 3: ReadModels983 lê as três listas de detalhe
static sDocStub g_doc = { { &g_course } };
sDocStub* Doc() { return &g_doc; }

// cVertexColorTable983::Apply chama isto; o harness não aplica cores.
void ApplyBoneColors983(WBone*, cVertexColorTable983::MeshColors&);

#include "polysoup_code.inl"

void ApplyBoneColors983(WBone*, cVertexColorTable983::MeshColors&) { }

static std::string F(const char* fmt, double a, double b = 0, double c = 0)
{
	char buf[128];
	sprintf(buf, fmt, a, b, c);
	return buf;
}
static std::string V(const WVector& v) { return F("%.3f %.3f %.3f", v.x, v.y, v.z); }
static std::string M(const WMatrix& m) { return V(m.xa) + " " + V(m.ya) + " " + V(m.za) + " " + V(m.pivot); }

struct Field { std::string cat, key, value; };
struct Hole
{
	std::vector<Field> f;
	void Add(const std::string& c, const std::string& k, const std::string& v)
	{
		Field x = { c, k, v };
		f.push_back(x);
	}
};

static std::string Key(const char* name, int i)
{
	char buf[64];
	sprintf(buf, "%s[%d]", name, i);
	return buf;
}

static bool Load(const char* path, Hole* h)
{
	FILE* fp = fopen(path, "rb");
	if (!fp)
	{
		printf("FAIL %s: nao abre\n", path);
		return false;
	}
	std::vector<unsigned char> data;
	fseek(fp, 0, SEEK_END);
	data.resize(ftell(fp));
	fseek(fp, 0, SEEK_SET);
	fread(&data[0], 1, data.size(), fp);
	fclose(fp);

	cFile file(data);
	WPolySoup::sHeader header;
	if (!WPolySoup::ReadHeader983(&file, header) || memcmp(header.id, "WEPX", 4))
	{
		printf("FAIL %s: cabecalho\n", path);
		return false;
	}
	WPolySoup::sCamera* camera = 0;
	WPolySoup::sPoint* point = 0;
	WPolySoup::sArea* area = 0;
	WPolySoup::sTexture* texture = 0;
	WPolySoup::sAttach983* attach = 0;
	std::vector<CNodeContainer> node;
	WPolySoup::sModel* base = 0;
	std::vector<WPolySoup::sModel*> model;
	WPolySoup::ReadCameras983(&file, header, camera);
	WPolySoup::ReadPoints983(&file, header, point);
	WPolySoup::ReadAreas983(&file, header, area);
	WPolySoup::ReadTextures983(&file, header, texture);
	WPolySoup::ReadNodes983(&file, header, node);
	WPolySoup::ReadModel983(&file, header, 0, base, NULL);

	// O modelo base real vem do .pet; aqui uma malha única com o número de faces gravado no registro.
	int faces = 0;
	{
		long back = file.at;
		file.Seek(back - ((int)header.version >= 0x72 ? 0xac + 56 : 0xac) + 4, SEEK_SET);
		file.Read(&faces, 4);
		file.Seek(back, SEEK_SET);
	}
	std::vector<unsigned short> idx(faces * 3);
	for (size_t i = 0; i < idx.size(); ++i)
		idx[i] = (unsigned short)(i % 65536);
	w_mesh mesh = { 0, 65536, faces * 3, idx.empty() ? 0 : &idx[0], 0 };
	WBone bone;
	strcpy(bone.m_name, "root");
	bone.mesh = &mesh;
	WPuppet pet;
	pet.root = &bone;
	cVertexColorTable983 colors;
	colors.Read(header.version, &file, &pet);
	WPolySoup::ReadModels983(&file, header, model);
	WPolySoup::ReadAttach983(&file, header, attach);

	h->Add("versao", "version", F("%.0f", (double)header.version));
	h->Add("contagem", "models", F("%.0f %.0f %.0f", header.modelNum, header.modelNum1, header.modelNum2));
	h->Add("contagem", "cameras", F("%.0f", header.cameraNum));
	h->Add("contagem", "points", F("%.0f", header.pointNum));
	h->Add("contagem", "areas", F("%.0f", header.areaNum));
	h->Add("contagem", "textures", F("%.0f", header.textureNum));
	h->Add("contagem", "nodes", F("%.0f", header.nodeNum));
	for (int i = 0; i < header.cameraNum; ++i)
		h->Add("camera", Key("camera", i), std::string(camera[i].name) + " " + M(camera[i].mat) + F(" %.3f", camera[i].fov));
	for (int i = 0; i < header.pointNum; ++i)
		h->Add("ponto", Key("point", i), F("%.0f ", point[i].type) + point[i].name + " " + V(point[i].pos) + " " + point[i].option);
	for (int i = 0; i < header.areaNum; ++i)
		h->Add("area", Key("area", i), F("%.0f ", area[i].type) + area[i].name + " " + V(area[i].aabb.min) + " " + V(area[i].aabb.max));
	std::vector<std::string> tex;
	for (int i = 0; i < header.textureNum; ++i)
	{
		char n[33];
		memcpy(n, texture[i].name, 32);
		n[32] = 0;
		tex.push_back(n);
		h->Add("textura_ordem", Key("texture", i), n);
	}
	std::sort(tex.begin(), tex.end());
	std::string all;
	for (size_t i = 0; i < tex.size(); ++i)
		all += tex[i] + " ";
	h->Add("textura_conjunto", "textures", all);
	for (size_t i = 0; i < node.size(); ++i)
	{
		std::string v = node[i].GetName() + F(" tipo %.0f :", node[i].GetType());
		for (size_t k = 0; k < node[i].GetList().size(); ++k)
			v += " " + V(node[i].GetList()[k]);
		h->Add("no", Key("node", (int)i), v);
	}
	h->Add("modelo_base", "name", base->name);
	h->Add("modelo_base", "mat", M(base->mat));
	h->Add("modelo_base", "type_flag", F("%.0f %.0f", base->type, base->flag));
	h->Add("modelo_base", "faces", F("%.0f", faces));
	unsigned long sum = 0, cnt = 0;
	for (std::map<std::string, cVertexColorTable983::MeshColors>::iterator b = colors.m_bones.begin(); b != colors.m_bones.end(); ++b)
		for (cVertexColorTable983::MeshColors::iterator m = b->second.begin(); m != b->second.end(); ++m)
			for (size_t i = 0; i < m->second.colors.size(); ++i)
			{
				sum = sum * 31 + m->second.colors[i];
				++cnt;
			}
	h->Add("cores_base", "hash", F("%.0f %.0f", (double)cnt, (double)sum));
	std::multimap<std::string, std::string> ordered;	// o LoadModels983 ordena por nome antes de usar
	for (size_t i = 0; i < model.size(); ++i)
		ordered.insert(std::make_pair(std::string(model[i]->name),
			F("%.0f %.0f ", model[i]->type, model[i]->flag) + M(model[i]->mat) + " " + model[i]->option));
	int n = 0;
	for (std::multimap<std::string, std::string>::iterator it = ordered.begin(); it != ordered.end(); ++it, ++n)
		h->Add("modelo", Key("model", n), it->first + " " + it->second);
	if (header.attachNum983 > 0)
		h->Add("osso_72", "attach", F("%.0f", header.attachNum983));

	// Os passos do LoadMapCheckData: pontos com nome, depois o par no fim do arquivo.
	sTeePinPoints983 spot[2];
	bool ok = FindTeePinPoints983(point, header.pointNum, spot);
	h->Add("tee_pin", "encontrados", ok ? "sim" : "nao");
	if (ok)
		for (int s = 0; s < 2; ++s)
		{
			std::string v = V(spot[s].tee->pos);
			for (int k = 0; k < 3; ++k)
				v += " | " + V(spot[s].pin[k]->pos);
			h->Add("tee_pin", Key("spot", s), v);
		}
	int tail = (int)header.version < 0x72 ? 0x40 : 0;
	h->Add("fim_arquivo", "sobra", F("%.0f", (double)((long)data.size() - file.at - tail)));
	h->Add("par", "par", F("%.0f", data[data.size() - 1 - tail]));
	h->Add("leitura", "estourou", file.overrun ? "sim" : "nao");
	unsigned long check = 0;
	for (int i = 0, len = data[0]; i < len; ++i)
		check += data[1 + i];
	h->Add("checksum", "soma", F("%.0f", (double)check));
	return true;
}

// Compara texto com números: cada token numérico pode variar até tol.
static bool Same(const std::string& a, const std::string& b, double tol)
{
	if (a == b)
		return true;
	if (tol <= 0)
		return false;
	size_t i = 0, j = 0;
	while (i < a.size() && j < b.size())
	{
		char* ea;
		char* eb;
		double x = strtod(a.c_str() + i, &ea), y = strtod(b.c_str() + j, &eb);
		if (ea != a.c_str() + i && eb != b.c_str() + j)
		{
			if (fabs(x - y) > tol)
				return false;
			i = ea - a.c_str();
			j = eb - b.c_str();
		}
		else if (a[i] != b[j])
			return false;
		else
		{
			++i;
			++j;
		}
	}
	return i == a.size() && j == b.size();
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("uso: gbincheck a.gbin [b.gbin]\n");
		return 2;
	}
	Hole a, b;
	if (!Load(argv[1], &a))
		return 2;
	if (argc == 2)
	{
		for (size_t i = 0; i < a.f.size(); ++i)
			printf("%s: %s\n", a.f[i].key.c_str(), a.f[i].value.c_str());
		return 0;
	}
	if (!Load(argv[2], &b))
		return 2;
	double tol = argc > 3 ? atof(argv[3]) : 0;
	std::map<std::string, std::pair<std::string, std::string> > bm;
	std::map<std::string, std::pair<int, int> > tally;
	for (size_t i = 0; i < a.f.size(); ++i)
	{
		bm[a.f[i].key].first = a.f[i].value;
		tally[a.f[i].cat].first++;
	}
	for (size_t i = 0; i < b.f.size(); ++i)
		bm[b.f[i].key].second = b.f[i].value;
	int diffs = 0;
	for (size_t i = 0; i < a.f.size(); ++i)
	{
		std::pair<std::string, std::string>& p = bm[a.f[i].key];
		if (Same(p.first, p.second, tol))
			tally[a.f[i].cat].second++;
		else
		{
			++diffs;
			printf("DIFF %s %s\n  a: %.200s\n  b: %.200s\n", a.f[i].cat.c_str(), a.f[i].key.c_str(), p.first.c_str(), p.second.c_str());
		}
	}
	for (std::map<std::string, std::pair<int, int> >::iterator it = tally.begin(); it != tally.end(); ++it)
		printf("SUM %s %d %d\n", it->first.c_str(), it->second.second, it->second.first);
	return diffs ? 1 : 0;
}
