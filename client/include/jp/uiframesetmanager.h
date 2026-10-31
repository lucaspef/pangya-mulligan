#pragma once
#include "vc10_layout.h"
// JP 983: CUiFrameSetManager — conjuntos de quadros da UI lidos do "uiframeset.xml" (elemento <layout>): medidas com nome
// (<nameddim>), pontos (<namedpoint>), retângulos (<namedrect>), imagens (<imageinfo>) e conjuntos de quadros (<frameset>),
// todos guardados em `std::map` pelo nome. Um valor pode citar outro com nome ("nameddim"/"namedpoint"/"namedrect") e o nome
// pode ter "-" na frente (inverte) e pedaços "[var]" trocados pelo número de uma tabela de variáveis. Classes da RTTI do 983
// (todas aninhadas): Elem 00cfff64, Dim 00d00180, Point 00d1a1f8, Rect 00d1a240, Named 00d1a148, NamedDim 00d1a218/00d1a208,
// NamedPoint 00d1a234/00d1a224, NamedRect 00d1a260/00d1a250, FrameSet 00d1a2c8/00d1a2b8, CUiFrameSetManager 00d1a320
// (instância 00e7423c). A classe existe no KR 839 (o Ghidra dá "par 839" em todas). Unidade 006b30e0–006c75c0 (os moldes
// vêm primeiro, 006b30e0–006b6bde). Sessão B59; nomes de método e de campo meus, menos os das classes.

#include <map>
#include <string>
#include <vector>
#include "engine/singleton.h"

#include "engine/wtypes.h"

class TiXmlElement;
class WOverlay;
class WView;

class CUiFrameSetManager : public WSingleton<CUiFrameSetManager>
{
public:
	typedef std::map<std::string, unsigned int> VarMap; // variáveis dos "[nome]" (nó de 0x30; `erase` 006bcb00 com `unsigned int`)

	// largura × altura (0x08; o WSize do motor tem cópia sem const e não serve em std::vector)
	struct Size
	{
		Size() {}
		Size(float w_, float h_) : w(w_), h(h_) {}
		float w, h;
	};

	// base de todo valor lido do XML (0x08)
	class Elem
	{
	public:
		Elem(); // 006b30e0
		virtual ~Elem() {} // 00491910 (com `delete`)
		virtual void Reset() = 0;
		virtual bool Load(TiXmlElement* pElem, VarMap* pVars) = 0;

		bool Init(TiXmlElement* pElem, VarMap* pVars); // 006b30f0: Reset + Load

	protected:
		bool m_bInvert; // +0x04 nome citado com "-" na frente
	};

	// medida (0x24): valor, tipo de escala e filhos <dim>
	class Dim : public Elem
	{
	public:
		Dim(); // 006c2310
		virtual void Reset(); // 006c1ae0
		virtual bool Load(TiXmlElement* pElem, VarMap* pVars); // 006c2370

		// valor na tela para a escala `scl` (tela ÷ 640×480): referência (±) + size (× escala pelo scltype) + soma dos filhos
		float Calc(const Size& scl) const; // 006b4dd0

		// valor fixo, sem nome nem filhos (embutida no 983: pedaço sem <imgrect>, escala do Frame::Reload)
		void Set(float size, int sclType = 0)
		{
			m_pRef = NULL;
			m_size = size;
			m_sclType = sclType;
			m_dims.clear();
		}

	protected:
		Dim* m_pRef;              // +0x08 "nameddim"
		float m_size;             // +0x0c "size"
		int m_sclType;            // +0x10 "scltype" (0–4)
		std::vector<Dim> m_dims;  // +0x14 <dim>
	};

	// ponto (0x54): <x> e <y>
	class Point : public Elem
	{
	public:
		Point(); // 006c2710
		virtual void Reset(); // 006b3120
		virtual bool Load(TiXmlElement* pElem, VarMap* pVars); // 006b9a70

		__forceinline void Set(float x, float y) { m_x.Set(x); m_y.Set(y); } // embutida no 983
		__forceinline void Set(const Size& size) { m_x.Set(size.w); m_y.Set(size.h); } // embutida no 983

		void Calc(const Size& scl, WPoint& out) const; // 006b4f20

	protected:
		Point* m_pRef; // +0x08 "namedpoint"
		Dim m_x;       // +0x0c
		Dim m_y;       // +0x30
	};

	// retângulo (0xb4): <pos> e <size>
	class Rect : public Elem
	{
	public:
		Rect(); // 006c2780
		virtual ~Rect(); // 006b8d40 (fora de linha no 983: o pedaço temporário da grade chama)
		virtual void Reset(); // 006b3150
		virtual bool Load(TiXmlElement* pElem, VarMap* pVars); // 006b9bf0

		// (0, 0) e o tamanho da imagem: pedaço sem <imgrect> (embutida no 983)
		__forceinline void Set(const Size& size) { m_pos.Set(0.0f, 0.0f); m_size.Set(size); }

		void Calc(const Size& scl, WRect& out) const; // 006b4fa0
		bool HitTest(const Size& scl, const WPoint& pt) const; // 006b5090

	protected:
		Rect* m_pRef; // +0x08 "namedrect"
		Point m_pos;  // +0x0c
		Point m_size; // +0x60
	};

	// nome (0x20)
	class Named
	{
	public:
		virtual ~Named() {} // 006b77c0 (com `delete`: 006b7770)
		virtual void Reset(); // 006b77f0

		bool LoadName(TiXmlElement* pElem); // 006b99d0: "name", não vazio e sem "-" na frente
		const std::string& GetName() const { return m_name; }

	protected:
		std::string m_name; // +0x04
	};

	class NamedDim : public Named, public Dim // 0x44
	{
	public:
		virtual void Reset(); // 006c1b10
		virtual bool Load(TiXmlElement* pElem, VarMap* pVars); // 006c26d0
	};

	class NamedPoint : public Named, public Point // 0x74
	{
	public:
		virtual void Reset(); // 006b7810
		virtual bool Load(TiXmlElement* pElem, VarMap* pVars); // 006b9bb0
	};

	class NamedRect : public Named, public Rect // 0xd4
	{
	public:
		virtual void Reset(); // 006b7820
		virtual bool Load(TiXmlElement* pElem, VarMap* pVars); // 006b9d30
	};

	// pedaço de um quadro (0x168, sem vtable): retângulo na imagem (<imgrect>) e na tela (<scrrect>)
	struct Piece
	{
		Rect m_img; // +0x00
		Rect m_scr; // +0xb4
	};

	// quadro de um conjunto (0xa0, sem vtable): <frame image filter wrap_u wrap_v scltypeforhiresimage> com os pedaços
	// (<imgrect>/<scrrect> ou a grade <seq name num>) e a repetição (<spacing num> = ponto)
	class Frame
	{
	public:
		// <seq name num>: variável da grade (0x20)
		struct Seq
		{
			Seq() : m_num(0) {}
			std::string m_name;  // +0x00
			unsigned int m_num;  // +0x1c (≥ 2)
		};

		// contexto da grade (local do LoadPieces): o <frame>, os <seq> e o valor corrente de cada variável
		struct Grid
		{
			Grid(TiXmlElement* pElem) : m_pElem(pElem) {}
			TiXmlElement* m_pElem;   // +0x00
			std::vector<Seq> m_seqs; // +0x04
			VarMap m_vars;           // +0x14 (passado aos Rect::Init dos pedaços)
		};

		Frame(); // 006c3000

		void Reset(); // 006c27f0
		bool Load(TiXmlElement* pElem); // 006c4ef0
		bool LoadPieces(TiXmlElement* pElem); // 006c4630: com <seq> → grade (um pedaço por combinação), senão um pedaço
		bool LoadGrid(Grid& grid, unsigned int level); // 006c3f60 (recursiva: um nível por <seq>)
		bool Reload(WView* pView); // 006c2870: escala da vista → imagem (alta resolução pela escala) → sobreposição

		// cada pedaço: <imgrect> (÷ tamanho da imagem = UV) → <scrrect>, repetido m_num vezes com o passo <spacing>
		void Draw(WView* pView, const Size& scl, int flags, unsigned long color); // 006b5110 (WOverlay::Render)
		void DrawArc(WView* pView, const Size& scl, float start, float end, int flags, unsigned long color); // 006b52b0

		std::string m_image;         // +0x00 "image" ("?nome" = imagem trocada pelo ResolveImageName)
		Size m_size;                 // +0x1c da ImageInfo (256 × 256)
		bool m_bFilter;              // +0x24 "filter"
		int m_sclType;               // +0x28 "scltypeforhiresimage"
		bool m_bWrapU;               // +0x2c "wrap_u"
		bool m_bWrapV;               // +0x2d "wrap_v"
		std::vector<Piece> m_pieces; // +0x30
		int m_num;                   // +0x40 <spacing num> (1; lido ≥ 2)
		Point m_spacing;             // +0x44 <spacing>
		float m_scale;               // +0x98 escala da última recarga (1.0)
		WOverlay* m_pOverlay;        // +0x9c imagem carregada (Reload; dono: m_overlays do gerente)
	};

	// conjunto de quadros (0x38)
	class FrameSet : public Named, public Elem
	{
	public:
		FrameSet(); // 006c57b0
		virtual void Reset(); // 006c5670
		virtual bool Load(TiXmlElement* pElem, VarMap* pVars); // 006c52a0

		bool Reload(WView* pView); // 006c2ae0
		void Draw(WView* pView, const Size& scl, int flags, unsigned long color); // 006b5460
		void DrawArc(WView* pView, const Size& scl, float start, float end, int flags, unsigned long color); // 006b54e0

	protected:
		std::vector<Frame> m_frames; // +0x28
	};

	// <imageinfo> (0x34, sem vtable): imagem, tamanho e as versões em alta resolução (<hiresimage scl image>) por escala
	class ImageInfo
	{
	public:
		ImageInfo(); // 006c0360

		bool Load(TiXmlElement* pElem); // 006bf890: "image", "w", "h" (> 0) e os <hiresimage> (scl ≥ 1, sem repetir)
		// escala 1 ou sem <hiresimage>: m_image; senão a 1ª de escala ≥ scl (ou ≈), ou a maior
		const std::string& GetImage(float scl) const; // 006b81c0

		std::string m_image;                  // +0x00
		float m_w;                            // +0x1c 256
		float m_h;                            // +0x20 256
		std::map<float, std::string> m_hires; // +0x24 escala → imagem (nó de 0x30)
	};

	// 006b8590: "?wind_direction_bg_image"/"?wind_speed_bg_image" → imagem do vento (clássica pelo bit 0 de CSharedDoc+0x109bd);
	// outro "?…" → ""; sem "?" → o próprio nome. Método (o 983 põe a instância em ecx; `ret 8` = retorno + nome)
	std::string ResolveImageName(const std::string& name);

	CUiFrameSetManager(); // 006c6a10
	virtual ~CUiFrameSetManager(); // 006c6820 (com `delete`: 006c6cf0)

	void Clear(); // 006c6140
	bool Load(const char* filename); // 006c6570
	bool Reload(WView* pView); // 006c66b0: lê de novo o mesmo arquivo e, se deu certo, ReloadImages983

	const ImageInfo* FindImageInfo(const std::string& name); // 006b94f0
	NamedDim* FindNamedDimAny(const std::string& name); // 006b8e70 (só o FindNamed usa)
	NamedDim* FindNamedDim(const std::string& name); // 006b8ee0
	NamedPoint* FindNamedPoint(const std::string& name); // 006b8f50
	NamedRect* FindNamedRect(const std::string& name); // 006b8fc0
	FrameSet* FindFrameSet(const std::string& name); // 006b9030
	Named* FindNamed(const std::string& name); // 006b9560: medida, ponto, retângulo ou conjunto com o nome

	// 006b97b0: tira o "-" da frente (→ bInvert) e troca cada "[var]" pelo número da variável; falha se faltar "]" ou a variável
	static bool ParseRef(std::string& name, bool& bInvert, VarMap* pVars);

	// 006b90b0: desenha o conjunto `name` na escala da vista (largura ÷ 640, altura ÷ 480)
	void DrawFrameSet(const std::string& name, WView* pView, int flags, unsigned long color);
	float ScaleForView983(WView* pView, float value); // 006b5570: × (1 + parte fracionária ÷ inteira de altura ÷ 480), sem m_bA4
	WOverlay* GetOverlay(const std::string& name); // 006beeb0: cache m_overlays, senão g_resrcmng->GetOverlay(nome, 0x80000000)
	bool ReloadImages983(WView* pView); // 006c2b50: Frame::Reload de todos os conjuntos (chamado também pela A: clientsetting)
	bool GetA4983() const { return m_bA4; } // leitura do +0xa4 (006c83b0: CWind::Display). Nome meu
	void SetA4983(bool bA4, WView* pView); // 006c3070: se m_bA4 mudou, grava e ReloadImages983 (COption::UpdatePowerGaugeLayout983)

protected:
	bool LoadImageInfos(TiXmlElement* pLayout); // 006c30a0
	bool LoadNamedDims(TiXmlElement* pLayout); // 006c3280
	bool LoadNamedPoints(TiXmlElement* pLayout); // 006c3540
	bool LoadNamedRects(TiXmlElement* pLayout); // 006c3980
	bool LoadFrameSets(TiXmlElement* pLayout); // 006c6240

	std::string m_filename;                          // +0x28
	std::map<std::string, ImageInfo> m_imageInfos;   // +0x44 (nó de 0x60)
	std::map<std::string, NamedDim> m_namedDims;     // +0x54 (nó de 0x70)
	std::map<std::string, NamedPoint> m_namedPoints; // +0x64 (nó de 0xa0)
	std::map<std::string, NamedRect> m_namedRects;   // +0x74 (nó de 0x100)
	std::map<std::string, FrameSet> m_frameSets;     // +0x84 (nó de 0x64)
	std::map<std::string, WOverlay*> m_overlays;     // +0x94 (nó de 0x30; soltos no Clear)
	bool m_bA4;                                      // +0xa4 escala sem arredondar (scltype 3/4 e ScaleForView983)
};

C_ASSERT_VC10(sizeof(CUiFrameSetManager::Dim) == 0x24);
C_ASSERT_VC10(sizeof(CUiFrameSetManager::Point) == 0x54);
C_ASSERT_VC10(sizeof(CUiFrameSetManager::Rect) == 0xb4);
C_ASSERT_VC10(sizeof(CUiFrameSetManager::Named) == 0x20);
C_ASSERT_VC10(sizeof(CUiFrameSetManager::NamedDim) == 0x44);
C_ASSERT_VC10(sizeof(CUiFrameSetManager::NamedPoint) == 0x74);
C_ASSERT_VC10(sizeof(CUiFrameSetManager::NamedRect) == 0xd4);
C_ASSERT_VC10(sizeof(CUiFrameSetManager::Piece) == 0x168);
C_ASSERT_VC10(sizeof(CUiFrameSetManager::Frame::Seq) == 0x20);
C_ASSERT_VC10(sizeof(CUiFrameSetManager::Frame) == 0xa0);
C_ASSERT_VC10(sizeof(CUiFrameSetManager::FrameSet) == 0x38);
C_ASSERT_VC10(sizeof(CUiFrameSetManager::ImageInfo) == 0x34);
C_ASSERT_VC10(sizeof(CUiFrameSetManager) == 0xa8);
