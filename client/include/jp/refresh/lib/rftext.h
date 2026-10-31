#pragma once
#include <windows.h>

// JP 983: motor de texto rico da Refresh (B167, F3.7.13 passo 1 de 3 do RFTextArea; plano em out/F3.7/s166/RELATORIO.md).
// Nomes dos métodos meus (…983); o 983 só tem o RTTI. Layouts pelos ctors (out/F3.7/s166/eng.c, ta.c).

// desenhista do cursor (vtable do objeto em RFCaret+0x18; slot 0 = destrutor deletante, slot 1 = desenha)
class RFCaretPainter983
{
public:
	virtual ~RFCaretPainter983() { }
	virtual void DrawCaret983(const float* pos, const int* size, float alpha, float time) = 0; // slot 1
};

// RFCaret (0x1c; ctor inline no RFTextArea::LoadResource 00426bf0; dtor 00455fd0; Draw 004560b0)
class RFCaret983
{
public:
	virtual ~RFCaret983(); // 00455fd0 (deleta o _painter)
	void Init983(RFCaretPainter983* painter, int w, int h); // o que o LoadResource faz depois do new
	void Draw983(const float* pos, float alpha, float time); // 004560b0
	void ResetBlink983() { _blinkOn = true; _timer = 0.0f; } // RFTextArea::ResetCaretBlink 004283a0
	static float BlinkPeriod983(); // GetCaretBlinkTime()/1000
	bool _visible; // +4
	bool _blinkOn; // +5
	int _w; // +8
	int _h; // +0xc
	float _timer; // +0x10
	float _period; // +0x14
	RFCaretPainter983* _painter; // +0x18
	RFCaret983() : _visible(true), _blinkOn(true), _w(0), _h(0), _timer(0.0f), _period(BlinkPeriod983()), _painter(0) { }
};

C_ASSERT(sizeof(RFCaret983) == 0x1c);

// RFParagraph (0x3c; ctor inline em RFText::RFText 00456260 e RFText::Reset 00456570; dtor deletante 00465340 -> 00465370)
// Campos conforme os ctors (out/F3.7/s166/eng.c); os que não têm nome ainda ficam como dwords.
// std::wstring do VS2010 sem o proxy de iteradores (0x18 B, como no 983): buf/ptr, size, reserve (res >= 8 = heap)
class RFTextArea;
struct RFWStr983
{
	union { wchar_t buf[8]; wchar_t* ptr; } u;
	unsigned int size, res;
	RFWStr983() : size(0), res(7) { u.buf[0] = 0; }
	~RFWStr983() { if (res >= 8) free(u.ptr); }
	const wchar_t* c_str() const { return res >= 8 ? u.ptr : u.buf; }
	void Insert983(unsigned int pos, const RFWStr983& s); // pos >= size anexa
	void Assign983(const wchar_t* s, unsigned int n); // 0040b4b0 (assign(ptr, n))
	void Erase983(unsigned int pos, unsigned int n); // 0040b5b0 (erase; n limitado ao fim)
private:
	RFWStr983(const RFWStr983&);
	RFWStr983& operator=(const RFWStr983&);
};
C_ASSERT(sizeof(RFWStr983) == 0x18);

// elemento do parágrafo (trecho/imagem...): polimórfico, deletado pelo parágrafo; slot 0x2c = largura em pixels (00456770)
class RFTextElement983
{
public:
	virtual ~RFTextElement983() { }
	virtual void Slot1_983() { }
	virtual void Slot2_983() { }
	// slot 3 (0xc; TextBlock 00469150, B266): a linha cujo retangulo (largura `width`) contem o ponto; *beyondEnd = o ponto passa do fim do texto
	virtual bool HitLine983(int x, int y, int width, bool* beyondEnd, int* line) { (void)x; (void)y; (void)width; (void)beyondEnd; (void)line; return false; }
	// slot 4 (0x10; 004692a0): link (estilo 0x10) sob o ponto com modo `*mode` (3 = qualquer): copia o alvo em *out; marca o hover (slot 8)
	virtual bool HitTestLink983(int x, int y, const int* mode, std::wstring* out) { (void)x; (void)y; (void)mode; (void)out; return false; }
	virtual void ClearLineFlags983() { } // slot 5 (0x14; 004693f0): +0x1c das linhas = 0 (RFText::Draw as marca de novo)
	virtual void ShowAllLines983() { } // slot 6 (0x18; 00469420): +0x1c das linhas = 1
	virtual void ResetHover983() { } // slot 7 (0x1c; 004696d0): +0xa8 = 0
	virtual void SetHover983(bool hover) { (void)hover; } // slot 8 (0x20; 004696e0): +0xa8 = hover
	virtual int Erase983(unsigned int pos, int n) { (void)pos; return n; } // slot 9 (0x24)
	virtual void Slot10_983() { }
	virtual unsigned int GetLength983() { return 0; } // slot 11 (0x2c)
	virtual unsigned int WidthSum983() { return 0; } // slot 12 (0x30; 00469780): soma das larguras (+0xc) das linhas
	virtual bool IsEmpty983() { return true; } // slot 13 (0x34; TextBlock: [+0x24] == 0)
	virtual bool IsSameStyle983(RFTextElement983* o) { (void)o; return false; } // slot 14 (0x38; TextBlock 004697d0: mesmo tipo e atributos)
	// slots 15..21 (B178; s� o TextBlock � conhecido: vetor de linhas +4/+8 de 0x3c B; os outros elementos devolvem -1)
	virtual int LineStart983(int line) { (void)line; return -1; } // 0x3c (469870): [start] da linha de numero `line`
	virtual int LineEnd983(int line) { (void)line; return -1; } // 0x40 (4698c0): [idx]
	virtual int LineWidth983(int line) { (void)line; return -1; } // 0x44 (469910): [w]
	virtual int LineHeight983(int line) { (void)line; return -1; } // 0x48 (469960): [h]
	// 0x4c (4699e0): *x = pixel dentro da linha; devolve o numero de caracteres antes de x (ou todos da linha e *x -= largura)
	virtual int ColumnToOffset983(int line, unsigned int* x) { (void)line; (void)x; return -1; }
	virtual void LineRange983(int* first, int* last) { *first = -1; *last = -1; } // 0x50 (4699b0)
	// 0x54 (469b70 -> 469bb0 [sticky] / 469d20): linha/coluna/pixel de uma posicao no elemento
	virtual void Locate983(unsigned int pos, int* line, int* col, int* x, bool sticky) { (void)pos; (void)line; (void)col; (void)x; (void)sticky; }
	// slot 22 (0x58; 00469ea0 no TextBlock): layout/quebra de linha; (w30, w34, x corrente, fonte, &linhas) -> novo x.
	// N�O portado no TextBlock (mede por GetTextExtent983): devolve x e n�o conta linhas.
	virtual int Layout983(int w30, int w34, int x, int font, int* lines) { (void)w30; (void)w34; (void)font; (void)lines; return x; }
	virtual void SetLinePos983(int line, int x, int y) { (void)line; (void)x; (void)y; } // slot 23 (0x5c; 0046a2d0): +0x14/+0x18 da linha
	virtual const RFWStr983* TextOf983() const { return 0; } // extra (nome meu): texto do trecho, 0 se n�o for texto
	virtual void DrawLines983(RFTextArea* owner, float alpha) { (void)owner; (void)alpha; } // extra (nome meu): 0046b320 + 004694c0 por linha
};

// TextBlock (0xac; vtable 00d66b24, ctor 00468cb0, dtor 00468d40 -> 00468f70): o trecho de texto do parágrafo.
// Só os campos que o ctor zera; as 4 wstrings (+0x14, +0x34, +0x50, +0x78) são std::wstring do VS2010 (0x18 B).
// uma linha do trecho (0x3c B; copiada por 0046b7b0, zerada por 0046b840, push_back 0046a540)
struct RFTextLine983
{
	int line, start, idx, w, h, f14, f18; // +0..+0x18: numero da linha, inicio, fim (indices), largura, altura, 0, 0
	bool f1c; // +0x1c
	RFWStr983 text; // +0x20
	int pad; // +0x38
};
C_ASSERT(sizeof(RFTextLine983) == 0x3c);

class RFTextRun983 : public RFTextElement983
{
public:
	RFTextRun983();
	virtual ~RFTextRun983(); // libera o vetor de linhas (+4/+8/+0xc; 00468d40 -> 00468f70)
	virtual unsigned int GetLength983() { return _text.size; } // 00469770: [+0x24]
	virtual int Erase983(unsigned int pos, int n); // 00469710
	virtual bool IsEmpty983() { return _text.size == 0; } // 004697c0
	virtual bool IsSameStyle983(RFTextElement983* o); // 004697d0
	// slot 0x58, 00469ea0 (+ 0046a370: fonte efetiva): w30/w34 = FontInfo* base/alternativa do paragrafo, x = largura restante da
	// linha, width = largura das linhas seguintes. Preenche o vetor de linhas (+4/+8: RFTextLine983, 0x3c B) e o FontInfo em
	// +0x78/+0x94; devolve a largura restante (ou -1 se o trecho esta vazio).
	virtual int Layout983(int w30, int w34, int x, int width, int* lines);
	virtual const RFWStr983* TextOf983() const { return &_text; }
	virtual int LineStart983(int line);
	virtual int LineEnd983(int line);
	virtual int LineWidth983(int line);
	virtual int LineHeight983(int line);
	virtual int ColumnToOffset983(int line, unsigned int* x);
	virtual void LineRange983(int* first, int* last);
	virtual void Locate983(unsigned int pos, int* line, int* col, int* x, bool sticky);
	// B266 (hit-test, hover, posicao das linhas e desenho; asm 00469150-004696e0, 0046a2d0, 004694c0)
	virtual bool HitLine983(int x, int y, int width, bool* beyondEnd, int* line); // 00469150 + 004691d0
	virtual bool HitTestLink983(int x, int y, const int* mode, std::wstring* out); // 004692a0 + 0046b1d0
	virtual void ClearLineFlags983(); // 004693f0
	virtual void ShowAllLines983(); // 00469420
	virtual void ResetHover983() { _fa8 = false; } // 004696d0
	virtual void SetHover983(bool hover) { _fa8 = hover; } // 004696e0
	virtual unsigned int WidthSum983(); // 00469780
	virtual void SetLinePos983(int line, int x, int y); // 0046a2d0
	virtual void DrawLines983(RFTextArea* owner, float alpha); // 0046b320 + 004694c0
	int _f04, _f08, _f0c; // +4,+8,+0xc
	int _f10; // +0x10 (não escrito)
	RFWStr983 _text; // +0x14
	int _f2c; // +0x2c (não escrito)
	int _f30; // +0x30 = 0
	RFWStr983 _s34; // +0x34
	int _f4c; // +0x4c (não escrito)
	RFWStr983 _s50; // +0x50
	int _f68; // +0x68 (não escrito)
	unsigned int _color; // +0x6c = 0xff000000
	int _f70; // +0x70 = 2
	bool _f74; // +0x74 = 1
	RFWStr983 _s78; // +0x78
	int _f90; // +0x90 (não escrito)
	int _f94; // +0x94 = 0
	int _f98; // +0x98 = 0
	int _f9c; // +0x9c = 0
	int _fa0; // +0xa0 = 0
	int _fa4; // +0xa4 = 0
	bool _fa8; // +0xa8 = 0
};

struct RFCursorMove983;
class RFParagraph983
{
public:
	virtual ~RFParagraph983(); // 00465340 deletante -> 00465370 (rftext.cpp)
	RFParagraph983(int font, int fontStyle, int color, int a, int b, int owner)
		: _first(0), _last(0), _cap(0), _font(font), _fontStyle(0x82), _one(1), _zero(0), _flag(false), _f28(0), _f2c(0),
		  _w30(a), _w34(b), _w38(owner) { (void)fontStyle; (void)color; }
	RFTextElement983** _first; // +4  vector<RFTextElement983*>: begin,end,cap (00465370)
	RFTextElement983** _last;  // +8
	RFTextElement983** _cap;   // +0xc
	int _f10; // +0x10 (não escrito pelo ctor)
	int _font; // +0x14 (RFText+0x2c)
	int _fontStyle; // +0x18 (0x82 no ctor; RFText+0x34 no Reset)
	int _one; // +0x1c = 1
	int _zero; // +0x20
	bool _flag; // +0x24
	int _f28; // +0x28
	int _f2c; // +0x2c
	int _w30; // +0x30 (RFText+0x24)
	int _w34; // +0x34 (RFText+0x28)
	int _w38; // +0x38 (RFText+0x20)
	void InsertText983(const RFWStr983& s); // 00466d90: insere no cursor (_zero, +0x20)
	void _pushElem(RFTextElement983* e); // 004676e0/00467880 (push_back inline)
	int FindNewline983() const; // 004666c0 (466620 + find('\n')): posi��o no texto do par�grafo ou -1
	void RemoveEmpty983(); // 00467230: apaga os elementos vazios (slot 0x34)
	void EraseAt983(unsigned int pos, int n = 1); // 00468540: apaga n caracteres (slot 0x24)
	void DeleteN983(unsigned int pos, int n); // 004660e0: se pos < total, apaga n caracteres a partir de pos (n != 0)
	int RecountLines983(); // 00468910 + [+0x1c] = linhas + 1
	int Height983(bool last) const; // 004663d0 (B265): soma das alturas das linhas (x _fontStyle/100; a ultima sem espaco se last)
	void ParseAppend983(const RFWStr983& s); // 00466f20 (+ parser 0046c820, tags 0046d1d0/0046d380): <B> <I> <U> <IM src> <A href> <F color>; anexa os trechos
	void SplitTo983(RFParagraph983* dst); // 00465660: a parte depois do cursor vai para dst
	// movimento do cursor (B178): 00466830 e auxiliares (cursor = _zero +0x20, _flag +0x24 = "fim de linha" pegajoso)
	void MoveCursor983(int cmd, RFCursorMove983* r); // 00466830 (cmd por ponteiro em EAX, r em EBX)
	unsigned int TotalLength983() const; // 00466200
	RFTextElement983* FindElement983(unsigned int pos, unsigned int* off, bool sticky) const; // 004673e0 (+ 467500, 468a50)
	RFTextElement983* Locate983(unsigned int pos, int* line, int* col, int* x); // 004672f0 (+ 004689b0)
	unsigned int LineStartPos983(int line) const; // 00468800
	void MoveToLineX983(int line, unsigned int x); // 00466c20 (+ 004688b0)
	void MoveToLineHome983(int line); // 00466ad0
	void MoveToLineEnd983(int line); // 00466b80 (+ 00468850)
	int AdvanceCursor983(int delta); // 00466a70: desloca; devolve delta se passou do fim (cursor no fim), senao 0
	void DeleteAtCursor983(); // 00466080 (+ 004660e0)
	void MoveCursorToEnd983() { _zero = (int)TotalLength983(); } // 00466b40
	void MoveCursorToX983(unsigned int x); // 00466ce0: na linha do cursor, para o pixel x
	void AppendFrom983(RFParagraph983* src); // 00465430: anexa os trechos de src (funde o que tem o mesmo estilo)
	int Position983(int x, int y, const int* hAlign); // 00465cd0 (B266): posiciona as linhas (x pelo alinhamento) e o cursor +0x28/+0x2c; devolve a altura
};

C_ASSERT(sizeof(RFParagraph983) == 0x3c);

// resultado do movimento do cursor (pilha do chamador de 00466830 em EBX): handled +0, kind +4, arg +8
struct RFCursorMove983
{
	enum Kind { LeftEdge = 1, RightEdge = 2, UpEdge = 3, DownEdge = 4, BackspaceAtStart = 5, DeleteAtEnd = 6 };
	bool handled; int kind; int arg;
	RFCursorMove983() : handled(false), kind(0), arg(0) { }
};
enum RFCursorCmd983 { CURSOR_UP983 = 0, CURSOR_DOWN983 = 1, CURSOR_LEFT983 = 2, CURSOR_RIGHT983 = 3, CURSOR_HOME983 = 4,
	CURSOR_END983 = 5, CURSOR_DELETE983 = 8, CURSOR_BACKSPACE983 = 9 };
C_ASSERT(sizeof(RFTextRun983) == 0xac);

// RFText (0x54; ctor 00456260, usado em RFTextArea+0x180). A lista de parágrafos (std::list inline: nó sentinela em +8) e
// o parser (00456600 AddText, 00457890 ...) ainda não estão portados.
struct RFTextNode983 { RFTextNode983* next; RFTextNode983* prev; };
class RFText983
{
public:
	virtual ~RFText983(); // 004563c0 (limpa a lista e o nó sentinela)
	RFText983(int a2, int a3, const int* a4, const int* a5, int a6, int a7, int a8);
	RFParagraph983* _current; // +4 (parágrafo corrente = retorno de 00457890)
	RFTextNode983* _head; // +8 (sentinela da lista de parágrafos; next/prev apontam para si)
	// std::deque<RFParagraph983*> inline (+8 = proxy; blocos de 4 ponteiros = 16 B, como o do 983)
	RFParagraph983*** _map; // +0xc
	unsigned int _mapSize; // +0x10
	unsigned int _off; // +0x14
	unsigned int _size; // +0x18
	void ClearParagraphs983(); // 004564b0 + 00458250: apaga os parágrafos e libera os blocos
	RFParagraph983* InsertParagraph983(unsigned int idx, RFParagraph983* p); // 00457890 (idx < size: no meio; senão anexa)
	RFParagraph983*& ParaAt983(unsigned int i) { unsigned int b = (_off + i) >> 2; if (b >= _mapSize) b -= _mapSize; return _map[b][(_off + i) & 3]; }
	void PushBackPara983(RFParagraph983* p); // 00457d30 (deque::push_back)
	int IndexOfPara983(RFParagraph983* p); // 00457a90: indice do paragrafo (-1 se nao esta)
	RFParagraph983* PrevPara983(RFParagraph983* p); // 00457910
	RFParagraph983* NextPara983(RFParagraph983* p); // 004579d0
	void ErasePara983(unsigned int idx); // 004577b0: apaga o paragrafo idx (delete) e fecha o buraco
	void HandleCursor983(int cmd); // 00457620: move o cursor; nas bordas passa/une paragrafos
	void AddText983(const wchar_t* text, bool parse); // 00456600 (parse: RFTextParser 00466f20 N�O portado)
	unsigned int TotalLines983(); // 004574a0
	void SetWidth983(int w); // 00456e10 (embutida no RFTextArea::OnLayoutChanged): +0x2c e nova quebra de linha de todos os paragrafos
	int TotalHeight983(); // 00459740 (for_each de Height983 com last = ultimo paragrafo; usado pelo RFTextAreaSC)
	void SetComposition983(const wchar_t* text, bool active); // 00456ab0 (OnChar/IME): troca o trecho em composicao (+0x4c/+0x50) e insere text
	// B266 (motor de hit-test/desenho do RFTextArea; asm 00456bc0, 00456cd0, 00457550, 00458950, 00459440-00459800)
	void ResetHover983(); // 00456cd0: +0xa8 = 0 em todos os trechos (slot 7)
	bool HitTestLink983(int x, int y, int mode, std::wstring* out); // 00459550: primeiro trecho com link sob o ponto (slot 4)
	bool SetCursorAtPoint983(int x, int y, int relX); // 00457550 + 004598f0: cursor na linha sob o ponto (slot 3) e relX; paragrafo corrente
	unsigned int MaxWidth983(); // 00459800: maior soma de larguras entre os paragrafos (slot 12)
	void Position983(int x, int y); // 00458950 + 00459440: Position983 de cada paragrafo, y acumulado (alinhamento = _p44)
	void Draw983(float alpha); // 00456bc0: apaga as marcas das linhas e desenha cada trecho (slot 5 / 0046b320)
	void Reset983(); // 00456570: limpa e cria 1 parágrafo vazio (como o ctor)
	void GetText983(std::wstring& out); // 00456fa0: parágrafos juntos por L"\n"
	int _f1c; // +0x1c (não escrito)
	int _p20; // +0x20 = a8
	int _p24; // +0x24 = a6
	int _p28; // +0x28 = a7
	int _p2c; // +0x2c = a2
	int _p30; // +0x30 = a3
	int _p34; // +0x34 = 0x82
	int _p38; // +0x38 = 200
	int _p3c; // +0x3c = 500
	int _p40; // +0x40 = 0
	int _p44; // +0x44 = *a4
	int _p48; // +0x48 = *a5
	bool _p4c; // +0x4c = 0
	int _p50; // +0x50 = 0
};

C_ASSERT(sizeof(RFText983) == 0x54);
