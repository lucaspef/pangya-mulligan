// JP 983: RFCaret da Refresh (B167, 00455fd0 e 004560b0) — primeira parte do motor RFText/RFParagraph do RFTextArea.
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include <string>
#include <vector>
#include "refresh/lib/rftext.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/lib/rfinterface.h"
#include "refresh/lib/rfrenderer.h"

extern RFInterface* g_pRFInterface983; // 00f1de14 (rfmanager.cpp)
extern RFRenderer* g_pRFRenderer983; // 00f1de20 (rfmanager.cpp)

RFCaret983::~RFCaret983()
{
	delete _painter; // 00455fd0: chama o destrutor deletante do objeto em +0x18 e zera
	_painter = 0;
}

float RFCaret983::BlinkPeriod983()
{
	return (float)((double)(int)GetCaretBlinkTime() / 1000.0); // 00426bf0: / [00d66cd0] = 1000.0 (double)
}

void RFCaret983::Init983(RFCaretPainter983* painter, int w, int h)
{
	_painter = painter;
	_w = w;
	_h = h;
}

// 004560b0: só desenha se visível e na fase "ligada" do piscar
void RFCaret983::Draw983(const float* pos, float alpha, float time)
{
	if (_visible && _blinkOn)
		_painter->DrawCaret983(pos, &_w, alpha, time);
}

// 00456260: ctor do RFText (a lista de parágrafos começa com um nó sentinela de 8 bytes apontando para si mesmo)
RFText983::RFText983(int a2, int a3, const int* a4, const int* a5, int a6, int a7, int a8)
{
	_current = 0;
	_head = new RFTextNode983;
	_head->prev = 0;
	_head->next = _head; // *(*[+8]) = [+8] (sentinela circular)
	_map = 0;
	_mapSize = _off = _size = 0;
	_p4c = false;
	_p50 = 0;
	_p24 = a6;
	_p2c = a2;
	_p30 = a3;
	_p28 = a7;
	_p34 = 0x82;
	_p38 = 200;
	_p3c = 500;
	_p40 = 0;
	_p44 = *a4;
	_p48 = *a5;
	_p20 = a8;
	RFParagraph983* p = new RFParagraph983(_p2c, _p34, 0, _p24, _p28, _p20);
	_current = InsertParagraph983(_size, p); // 00457890
}

// 00457d30: deque::push_back (blocos de 4 ponteiros; o mapa cresce por 004580b0 — aqui, realocação simples)
void RFText983::PushBackPara983(RFParagraph983* p)
{
	if (((_off + _size) & 3) == 0 && _mapSize <= ((_size + 4) >> 2)) {
		unsigned int nmap = _mapSize ? _mapSize * 2 : 8; // mapa circular: reorganiza a partir do bloco do início
		RFParagraph983*** nm = (RFParagraph983***)calloc(nmap, sizeof(*nm));
		unsigned int nblk = _size ? (((_off + _size - 1) >> 2) - (_off >> 2) + 1) : 0;
		for (unsigned int i = 0; i < nblk; ++i) {
			unsigned int b = (_off >> 2) + i;
			if (_mapSize && b >= _mapSize) b -= _mapSize;
			nm[(_off >> 2) + i] = _map[b];
		}
		if (_map) free(_map);
		_map = nm;
		_mapSize = nmap;
	}
	unsigned int pos = _off + _size, b = pos >> 2;
	if (b >= _mapSize) b -= _mapSize;
	if (!_map[b]) _map[b] = (RFParagraph983**)calloc(4, sizeof(RFParagraph983*));
	_map[b][pos & 3] = p;
	++_size;
}

// 00457890: idx < size -> insere no meio (458de0); senão anexa. Devolve o parágrafo.
RFParagraph983* RFText983::InsertParagraph983(unsigned int idx, RFParagraph983* p)
{
	if (idx >= _size) {
		PushBackPara983(p);
		return p;
	}
	PushBackPara983(ParaAt983(_size - 1));
	for (unsigned int i = _size - 2; i > idx; --i)
		ParaAt983(i) = ParaAt983(i - 1);
	ParaAt983(idx) = p;
	return p;
}

// 004564b0: _current = 0; deletante de cada parágrafo; depois 00458250 zera o tamanho e libera blocos e mapa
void RFText983::ClearParagraphs983()
{
	_current = 0;
	for (unsigned int i = 0; i < _size; ++i)
		delete ParaAt983(i);
	_size = 0;
	_off = 0;
	for (unsigned int i = 0; i < _mapSize; ++i) {
		if (_map[i]) free(_map[i]);
	}
	if (_map) free(_map);
	_map = 0;
	_mapSize = 0;
}

RFText983::~RFText983()
{
	ClearParagraphs983(); // 004563c0: 004564b0 + 00458250, depois libera o proxy
	delete _head;
}

// 00465370: apaga cada elemento (deletante), esvazia e libera o vetor
RFParagraph983::~RFParagraph983()
{
	for (RFTextElement983** it = _first; it != _last; ++it)
		delete *it;
	_last = _first;
	if (_first)
		free(_first);
	_first = _last = _cap = 0;
}

// 00468cb0
RFTextRun983::~RFTextRun983()
{
	RFTextLine983* b = (RFTextLine983*)_f04;
	for (RFTextLine983* it = b; it != (RFTextLine983*)_f08; ++it)
		it->~RFTextLine983();
	free(b);
}

// 0046a540 (push_back do vetor de linhas, 0x3c B por elemento; cresce por realloc: as strings sao relocaveis)
static void PushLine983(RFTextRun983* r, const RFTextLine983& l)
{
	RFTextLine983* b = (RFTextLine983*)r->_f04;
	size_t n = (RFTextLine983*)r->_f08 - b, cap = (RFTextLine983*)r->_f0c - b;
	if (n == cap) {
		cap = cap ? cap * 2 : 4;
		b = (RFTextLine983*)realloc(b, cap * sizeof(RFTextLine983));
		r->_f04 = (int)b;
		r->_f0c = (int)(b + cap);
	}
	RFTextLine983* d = new (b + n) RFTextLine983;
	d->line = l.line; d->start = l.start; d->idx = l.idx; d->w = l.w; d->h = l.h; d->f14 = l.f14; d->f18 = l.f18;
	d->f1c = l.f1c; d->pad = 0;
	d->text.Assign983(l.text.c_str(), l.text.size);
	r->_f08 = (int)(b + n + 1);
}

// 00469ea0 (slot 0x58) com 0046a370 (fonte efetiva): mede caractere a caractere por GetTextExtent983 (slot 2 do
// RFInterface) e quebra a linha quando a largura acumulada (truncada, fistp com 0xc00) passa da restante.
int RFTextRun983::Layout983(int w30, int w34, int x, int width, int* lines)
{
	// o vetor anterior de linhas e esvaziado (0046b7b0/0046b840)
	{
		RFTextLine983* b = (RFTextLine983*)_f04;
		for (RFTextLine983* it = b; it != (RFTextLine983*)_f08; ++it)
			it->~RFTextLine983();
		_f08 = _f04;
	}
	// 0046a370: fonte efetiva (base w30; se flag 0x10 e +0x74, a alternativa w34) + modificadores do trecho
	const FontInfo* base = (const FontInfo*)w30;
	FontInfo fi;
	if ((_f30 & 0x10) == 0 || !_f74) {
		fi = *base;
		if (_f30 & 1) fi.style |= 1;
		if (_f30 & 2) fi.style |= 2;
		if (_f30 & 4) fi.color = _color;
		if (_f30 & 0x20) fi.style |= 4;
	} else {
		fi = *(const FontInfo*)w34;
	}
	_s78.Assign983(fi.name.c_str(), (unsigned int)fi.name.size());
	_f94 = fi.size; _f98 = (int)fi.style; _f9c = (int)fi.color; _fa0 = (int)fi.outlineColor; _fa4 = (int)fi.shadowColor;
	if (_text.size == 0)
		return -1;
	unsigned int remain = (unsigned int)x;
	RFTextLine983 cur;
	cur.line = *lines; cur.start = 0; cur.idx = 0; cur.w = 0; cur.h = 0; cur.f14 = 0; cur.f18 = 0; cur.f1c = false; cur.pad = 0;
	float accW = 0.0f, accH = 0.0f;
	for (unsigned int i = 0; i < _text.size; ++i) {
		wchar_t ch[2] = { _text.c_str()[i], 0 };
		RFSize sz = g_pRFInterface983->GetTextExtent983(fi, std::wstring(ch));
		accW += sz.width;
		if (accH <= sz.height)
			accH = sz.height;
		cur.idx = (int)i;
		if (remain < (unsigned int)(int)accW) {
			// estourou: fecha a linha corrente (se tem texto) e abre outra com este caractere
			if (cur.text.size != 0)
				PushLine983(this, cur);
			++*lines;
			cur.line = *lines;
			remain = (unsigned int)width;
			cur.start = (int)i;
			cur.w = (int)sz.width;
			cur.h = (int)sz.height;
			cur.text.Assign983(ch, 1);
			accW = sz.width;
			accH = sz.height;
		} else {
			cur.w = (int)accW;
			cur.h = (int)accH;
			RFWStr983 one;
			one.Assign983(ch, 1);
			cur.text.Insert983(cur.text.size, one);
		}
	}
	cur.idx = (int)_text.size;
	PushLine983(this, cur);
	return (int)(remain - cur.w); // o trecho seguinte continua na mesma linha com o que sobrou
}

RFTextRun983::RFTextRun983()
	: _f04(0), _f08(0), _f0c(0), _f30(0), _color(0xff000000), _f70(2), _f74(true),
	  _f94(0), _f98(0), _f9c(0), _fa0(0), _fa4(0), _fa8(false)
{
}

// ---- RFTextParser (vtable 00d88510; parse 0046c820, tag 0046d1d0 + 0046d300 + 0046d380, atributos 0046bd20/0046bf00/0046c2b0) ----
// O 983 troca "&lt;"/"&gt;" por U+3008/U+3009, percorre os caracteres e, a cada "<...>" reconhecido (B I U IM A F, abrindo ou
// fechando), muda o estado (flags +0x20: B=1 I=2 F=4 IM=8 A=0x10 U=0x20; href +0x24; src +0x40; cor +0x5c; modo +0x60; sublinhado +0x64).
// Um trecho novo (RFTextRun983: texto +0x14, flags +0x30, href +0x34, src +0x50, cor +0x6c, modo +0x70, sublinhado +0x74)
// nasce quando o estado muda ou depois de uma tag de fechamento. O parser XML dos atributos (TiXml) virou um leitor simples nome="valor".
struct RFParseState983
{
	unsigned int flags;
	std::wstring href, src;
	unsigned int color;
	int mode;
	bool under;
	RFParseState983() : flags(0), color(0xff000000), mode(2), under(true) { }
	bool Same(const RFParseState983& o) const
	{
		return flags == o.flags && color == o.color && href == o.href && src == o.src && mode == o.mode && under == o.under;
	}
};

// RFTextParser do 983 (objeto local do 00466f20, vtable d66c9c): estado + vetor de trechos produzidos (+0x68).
// Os leitores de atributo (TiXml no 983) viram o leitor simples NextAttr983.
struct RFTextParser983
{
	RFParseState983 st;
	std::vector<RFTextRun983*> runs;
	static __declspec(noinline) unsigned int TagLen983(const std::wstring& s, unsigned int pos);
	__declspec(noinline) void ParseImAttrs983(const std::wstring& tag);
	__declspec(noinline) void ParseAAttrs983(const std::wstring& tag);
	__declspec(noinline) void ParseFAttrs983(const std::wstring& tag);
	__declspec(noinline) bool ApplyTag983(const std::wstring& tag, bool& closing);
	__declspec(noinline) bool TryTag983(const std::wstring& in, unsigned int& pos, bool& closing);
	__declspec(noinline) bool Parse983(const RFWStr983& s);
};

// 0046d300: devolve o tamanho da tag ("<" ... ">" sem outro "<" no meio) ou 0
unsigned int RFTextParser983::TagLen983(const std::wstring& s, unsigned int pos)
{
	for (unsigned int i = 0; pos + i < s.size(); ++i) {
		wchar_t c = s[pos + i];
		if (c == L'<' && i != 0)
			return 0;
		if (c == L'>' && i != 0)
			return i + 1;
	}
	return 0;
}

// atributos nome="valor" (ou 'valor') do elemento raiz da tag; chama f para cada um
static bool NextAttr983(const std::wstring& tag, unsigned int& i, std::wstring& name, std::wstring& val)
{
	unsigned int n = (unsigned int)tag.size();
	while (i < n && tag[i] != L'>' && tag[i] != L'/' && tag[i] != L' ' && tag[i] != L'\t')
		++i; // nome da tag (ou fim do atributo anterior)
	for (;;) {
		while (i < n && (tag[i] == L' ' || tag[i] == L'\t' || tag[i] == L'\r' || tag[i] == L'\n' || tag[i] == L'/'))
			++i;
		if (i >= n || tag[i] == L'>')
			return false;
		unsigned int b = i;
		while (i < n && tag[i] != L'=' && tag[i] != L' ' && tag[i] != L'>' && tag[i] != L'/')
			++i;
		name.assign(tag, b, i - b);
		while (i < n && tag[i] == L' ')
			++i;
		if (i >= n || tag[i] != L'=')
			continue;
		++i;
		while (i < n && tag[i] == L' ')
			++i;
		if (i >= n)
			return false;
		wchar_t q = tag[i];
		if (q == L'"' || q == L'\'') {
			++i;
			b = i;
			while (i < n && tag[i] != q)
				++i;
			val.assign(tag, b, i - b);
			if (i < n)
				++i;
		} else {
			b = i;
			while (i < n && tag[i] != L' ' && tag[i] != L'>')
				++i;
			val.assign(tag, b, i - b);
		}
		return true;
	}
}

static int HexDigit983(wchar_t c)
{
	if (c >= L'0' && c <= L'9') return c - L'0';
	if (c >= L'a' && c <= L'f') return c - L'a' + 10;
	if (c >= L'A' && c <= L'F') return c - L'A' + 10;
	return -1;
}

// 0046bd20: <IM src="...">
void RFTextParser983::ParseImAttrs983(const std::wstring& tag)
{
	unsigned int i = 0;
	std::wstring name, val;
	while (NextAttr983(tag, i, name, val))
		if (name == L"src")
			st.src = val;
}

// 0046bf00: <A href="..." activate="rbuttondown|rbuttonup" style="no">
void RFTextParser983::ParseAAttrs983(const std::wstring& tag)
{
	unsigned int i = 0;
	std::wstring name, val;
	st.under = true;
	st.mode = 2;
	while (NextAttr983(tag, i, name, val)) {
		if (name == L"href")
			st.href = val;
		else if (name == L"activate") {
			if (val == L"rbuttondown") st.mode = 0;
			else if (val == L"rbuttonup") st.mode = 1;
		} else if (name == L"style") {
			if (val == L"no") st.under = false;
		}
	}
}

// 0046c2b0: <F color="#RRGGBB"> ou "#RRGGBBAA" -> 0xAARRGGBB
void RFTextParser983::ParseFAttrs983(const std::wstring& tag)
{
	unsigned int i = 0;
	std::wstring name, val;
	while (NextAttr983(tag, i, name, val)) {
		if (name != L"color" || val.size() < 2 || val[0] != L'#')
			continue;
		unsigned int v[4] = { 0xff, 0xff, 0xff, 0xff };
		unsigned int cnt = ((unsigned int)val.size() - 1) >> 1;
		if (cnt > 4)
			cnt = 4;
		for (unsigned int k = 0; k < cnt; ++k) {
			int h = HexDigit983(val[1 + 2 * k]), l = HexDigit983(val[2 + 2 * k]);
			v[k] = (unsigned int)((h < 0 ? 0 : h) * 16 + (l < 0 ? 0 : l));
		}
		st.color = (((v[0] & 0xff) | (v[3] << 8)) << 8 | (v[1] & 0xff)) << 8 | (v[2] & 0xff);
	}
}

// 0046d380: aplica uma tag ao estado; false = tag desconhecida (o '<' vira texto). closing = tag de fechamento.
bool RFTextParser983::ApplyTag983(const std::wstring& tag, bool& closing)
{
	std::wstring up = tag;
	for (unsigned int k = 0; k < up.size(); ++k)
		up[k] = (wchar_t)towupper(up[k]);
	if (up.size() < 2)
		return false;
	if (up[1] == L'/') {
		if (up.size() < 3)
			return false;
		closing = true;
		wchar_t c = up[2];
		if (c == L'B')
			st.flags &= ~1u;
		else if (c == L'I') {
			if (up.size() > 3 && up[3] == L'M') {
				st.flags &= ~8u;
				st.src.clear();
			} else
				st.flags &= ~2u;
		} else if (c == L'A') {
			st.flags &= ~0x10u;
			st.href.clear();
		} else if (c == L'F') {
			st.flags &= ~4u;
			st.color = 0xff000000;
		} else if (c == L'U')
			st.flags &= ~0x20u;
		else
			return false;
		return true;
	}
	closing = false;
	wchar_t c = up[1];
	if (c == L'B')
		st.flags |= 1;
	else if (c == L'I') {
		if (up.size() > 2 && up[2] == L'M') {
			st.flags |= 8;
			ParseImAttrs983(tag);
		} else
			st.flags |= 2;
	} else if (c == L'A') {
		st.flags |= 0x10;
		ParseAAttrs983(tag);
	} else if (c == L'F') {
		st.flags |= 4;
		ParseFAttrs983(tag);
	} else if (c == L'U')
		st.flags |= 0x20;
	else
		return false;
	return true;
}

// 0046d1d0: tenta ler uma tag em in[pos]; se reconhecida avanca pos pelo comprimento
bool RFTextParser983::TryTag983(const std::wstring& in, unsigned int& pos, bool& closing)
{
	unsigned int len = TagLen983(in, pos);
	if (len == 0 || !ApplyTag983(in.substr(pos, len), closing))
		return false;
	pos += len;
	return true;
}

// 00468d80 (copia o estado do parser 983 para um TextBlock novo; o 983 recebe a memoria pronta, aqui o new fica dentro)
static __declspec(noinline) RFTextRun983* MakeRun983(const std::wstring& text, const RFParseState983& st)
{
	std::wstring t = text; // devolve '<' e '>' (b4: 0046c820 troca U+3008/U+3009 de volta)
	for (unsigned int k = 0; k < t.size(); ++k) {
		if (t[k] == 0x3008) t[k] = L'<';
		else if (t[k] == 0x3009) t[k] = L'>';
	}
	RFTextRun983* r = new RFTextRun983;
	r->_text.Assign983(t.c_str(), (unsigned int)t.size());
	r->_f30 = (int)st.flags;
	r->_s34.Assign983(st.href.c_str(), (unsigned int)st.href.size());
	r->_s50.Assign983(st.src.c_str(), (unsigned int)st.src.size());
	r->_color = st.color;
	r->_f70 = st.mode;
	r->_f74 = st.under;
	return r;
}

// 0046c820: "&lt;"/"&gt;" -> U+3008/U+3009, percorre os caracteres, tags mudam o estado; um trecho novo nasce quando o estado
// muda ou depois de uma tag de fechamento. false se o texto e vazio.
bool RFTextParser983::Parse983(const RFWStr983& s)
{
	if (s.size == 0)
		return false;
	std::wstring in(s.c_str(), s.size);
	for (unsigned int p = 0; (p = (unsigned int)in.find(L"&lt;", p)) != (unsigned int)std::wstring::npos; ++p)
		in.replace(p, 4, 1, (wchar_t)0x3008);
	for (unsigned int p = 0; (p = (unsigned int)in.find(L"&gt;", p)) != (unsigned int)std::wstring::npos; ++p)
		in.replace(p, 4, 1, (wchar_t)0x3009);
	RFParseState983 cur; // st = estado do parser, cur = estado do trecho em montagem
	std::wstring text;
	bool force = false; // depois de uma tag de fechamento o proximo caractere abre trecho novo
	for (unsigned int i = 0; i < in.size();) {
		if (in[i] == L'<') {
			bool closing = false;
			if (TryTag983(in, i, closing)) {
				if (closing)
					force = true;
				continue;
			}
			text += L'<';
			++i;
			continue;
		}
		if (cur.Same(st) && !force) {
			text += in[i];
		} else {
			force = false;
			if (!text.empty())
				runs.push_back(MakeRun983(text, cur));
			cur = st;
			text.assign(1, in[i]);
		}
		++i;
	}
	if (!text.empty())
		runs.push_back(MakeRun983(text, cur));
	return true;
}

// 00466f20: roda o parser sobre s e anexa os trechos ao fim do paragrafo; o cursor (+0x20) avanca pelo comprimento anexado,
// limitado ao comprimento total.
void RFParagraph983::ParseAppend983(const RFWStr983& s)
{
	unsigned int added = 0;
	RFTextParser983 parser;
	if (parser.Parse983(s)) {
		for (unsigned int k = 0; k < parser.runs.size(); ++k) {
			_pushElem(parser.runs[k]);
			added += parser.runs[k]->GetLength983();
		}
	}
	unsigned int total = 0;
	for (RFTextElement983** it = _first; it != _last; ++it)
		total += (*it)->GetLength983();
	unsigned int c = (unsigned int)_zero + added;
	_zero = (int)(c < total ? c : total);
}

// 00466d90: procura o elemento que contém o cursor (percorrendo os comprimentos); insere no meio ou anexa; senão cria
// um elemento novo no fim. Depois avança o cursor pelo tamanho do texto, limitado ao comprimento total do parágrafo.
void RFParagraph983::InsertText983(const RFWStr983& s)
{
	RFTextRun983* run = 0;
	if (_first == _last) {
		run = new RFTextRun983;
		run->_text.Insert983(run->_text.size, s);
		_pushElem(run);
	} else {
		unsigned int rest = (unsigned int)_zero;
		RFTextElement983** it = _first;
		for (; it != _last; ++it) {
			unsigned int len = (*it)->GetLength983();
			if (len >= rest)
				break;
			rest -= len;
		}
		RFTextRun983* cur = (it != _last) ? (RFTextRun983*)*it : 0;
		if (cur) {
			if (rest < cur->_text.size)
				cur->_text.Insert983(rest, s);
			else
				cur->_text.Insert983(cur->_text.size, s);
		} else {
			run = new RFTextRun983;
			run->_text.Insert983(run->_text.size, s);
			_pushElem(run);
		}
	}
	unsigned int add = s.size;
	unsigned int total = 0;
	for (RFTextElement983** it = _first; it != _last; ++it)
		total += (*it)->GetLength983();
	unsigned int pos = (unsigned int)_zero + add;
	_zero = (int)(pos > total ? total : pos);
}

void RFParagraph983::_pushElem(RFTextElement983* e)
{
	size_t n = _last - _first, cap = _cap - _first;
	if (n == cap) {
		size_t nc = cap ? cap + cap / 2 + 1 : 1;
		RFTextElement983** nb = (RFTextElement983**)malloc(nc * sizeof(*nb));
		for (size_t i = 0; i < n; ++i)
			nb[i] = _first[i];
		if (_first)
			free(_first);
		_first = nb;
		_last = nb + n;
		_cap = nb + nc;
	}
	*_last++ = e;
}

// insere s em pos (pos >= size anexa); cresce a capacidade em blocos como o std::wstring (res = 7 -> 8+)
void RFWStr983::Insert983(unsigned int pos, const RFWStr983& s)
{
	if (pos > size)
		pos = size;
	unsigned int n = s.size, nsz = size + n;
	if (nsz > res) {
		unsigned int nres = nsz | 7;
		if (nres < res + res / 2) nres = res + res / 2;
		wchar_t* nb = (wchar_t*)malloc((nres + 1) * sizeof(wchar_t));
		const wchar_t* old = c_str();
		for (unsigned int i = 0; i <= size; ++i)
			nb[i] = old[i];
		if (res >= 8)
			free(u.ptr);
		u.ptr = nb;
		res = nres;
	}
	wchar_t* d = res >= 8 ? u.ptr : u.buf;
	for (unsigned int i = size + 1; i > pos; --i) // abre o espaço (inclui o terminador)
		d[i - 1 + n] = d[i - 1];
	const wchar_t* sp = s.c_str(); // s != *this (o 983 nunca insere o próprio texto)
	for (unsigned int i = 0; i < n; ++i)
		d[pos + i] = sp[i];
	size = nsz;
}

// 0040b4b0: assign(ptr, n)
void RFWStr983::Assign983(const wchar_t* s, unsigned int n)
{
	if (n > res) {
		unsigned int nres = n | 7;
		wchar_t* nb = (wchar_t*)malloc((nres + 1) * sizeof(wchar_t));
		if (res >= 8)
			free(u.ptr);
		u.ptr = nb;
		res = nres;
	}
	wchar_t* d = res >= 8 ? u.ptr : u.buf;
	for (unsigned int i = 0; i < n; ++i)
		d[i] = s[i];
	d[n] = 0;
	size = n;
}

// 0040b5b0: erase(pos, n) com n limitado ao fim
void RFWStr983::Erase983(unsigned int pos, unsigned int n)
{
	if (pos > size)
		return;
	if (n > size - pos)
		n = size - pos;
	wchar_t* d = res >= 8 ? u.ptr : u.buf;
	for (unsigned int i = pos; i + n <= size; ++i)
		d[i] = d[i + n];
	size -= n;
}

// 00469710 (slot 0x24 do TextBlock): apaga [pos, pos+n) dentro do texto; devolve quanto ainda falta apagar
int RFTextRun983::Erase983(unsigned int pos, int n)
{
	unsigned int len = GetLength983();
	if (n != 0 && pos < len) {
		if (len < pos + n) {
			_text.Erase983(pos, len - pos);
			return n - (int)(len - pos);
		}
		_text.Erase983(pos, n);
	}
	return 0;
}

// 004666c0: concatena o texto dos elementos (466620/468760) e procura L"\n" (0040a1c0)
int RFParagraph983::FindNewline983() const
{
	int base = 0;
	for (RFTextElement983** it = _first; it != _last; ++it) {
		const RFWStr983* t = (*it)->TextOf983();
		if (!t)
			continue;
		const wchar_t* c = t->c_str();
		for (unsigned int i = 0; i < t->size; ++i) {
			if (c[i] == L'\n')
				return base + (int)i;
		}
		base += (int)t->size;
	}
	return -1;
}

// 00467230: enquanto houver elemento com slot 0x34 verdadeiro, apaga (deletante) e fecha o buraco
void RFParagraph983::RemoveEmpty983()
{
	for (;;) {
		RFTextElement983** it = _first;
		for (; it != _last; ++it) {
			if ((*it)->IsEmpty983()) {
				delete *it;
				break;
			}
		}
		if (it == _last)
			return;
		for (RFTextElement983** q = it; q + 1 != _last; ++q)
			*q = q[1];
		--_last;
	}
}

// 00468540: percorre os elementos; o que contém pos apaga 1 caractere (slot 0x24)
void RFParagraph983::EraseAt983(unsigned int pos, int n)
{
	for (RFTextElement983** it = _first; it != _last; ++it) {
		unsigned int len = (*it)->GetLength983();
		if (pos < len) {
			n = (*it)->Erase983(pos, n);
			if (n == 0)
				return;
			pos = 0;
		} else {
			pos -= len;
		}
	}
}

// 00468910 (loop de slot 0x58 em cada elemento) e [+0x1c] = linhas + 1
int RFParagraph983::RecountLines983()
{
	int x = _font, lines = 0;
	for (RFTextElement983** it = _first; it != _last; ++it)
		x = (*it)->Layout983(_w30, _w34, x, _font, &lines);
	_one = lines + 1;
	return _one;
}

// 00465660 (this = origem, dst = parágrafo novo): o texto depois do cursor (+0x20) vai para dst.
// 004673e0 acha o elemento e o deslocamento; se o deslocamento = 0 o elemento todo muda; no meio, 468e80 cria um
// 00468e80: TextBlock novo com os atributos (flags +0x30, href +0x34, src +0x50, cor +0x6c, modo +0x70, sublinhado +0x74), sem texto.
static __declspec(noinline) RFTextRun983* NewRunAttrs983(const int* flags, const RFWStr983* href, const RFWStr983* src,
	const unsigned int* color, const int* mode, bool under)
{
	RFTextRun983* r = new RFTextRun983;
	r->_f30 = *flags;
	r->_s50.Assign983(src->c_str(), src->size);
	r->_s34.Assign983(href->c_str(), href->size);
	r->_color = *color;
	r->_f70 = *mode;
	r->_f74 = under;
	return r;
}

// TextBlock novo com os atributos (+0x30,+0x34,+0x50,+0x6c,+0x70,+0x74) e o resto do texto.
void RFParagraph983::SplitTo983(RFParagraph983* dst)
{
	if (!dst)
		return;
	unsigned int total = 0;
	for (RFTextElement983** it = _first; it != _last; ++it)
		total += (*it)->GetLength983();
	if ((unsigned int)_zero != total) {
		unsigned int off = (unsigned int)_zero;
		RFTextElement983** it = _first;
		for (; it != _last; ++it) {
			unsigned int len = (*it)->GetLength983();
			if (len >= off)
				break;
			off -= len;
		}
		if (it != _last) {
			RFTextElement983** keepEnd = it;
			if (off == 0) {
				// o elemento todo e os seguintes vão
			} else {
				RFTextRun983* run = (RFTextRun983*)*it;
				if (off != run->_text.size) {
					RFTextRun983* nr = NewRunAttrs983(&run->_f30, &run->_s34, &run->_s50, &run->_color, &run->_f70, run->_f74);
					nr->_text.Assign983(run->_text.c_str() + off, run->_text.size - off);
					run->_text.Erase983(off, run->_text.size - off);
					dst->_pushElem(nr);
				}
				++keepEnd;
			}
			for (RFTextElement983** q = keepEnd; q != _last; ++q)
				dst->_pushElem(*q);
			_last = keepEnd;
		}
	}
	RecountLines983();
	dst->RecountLines983();
}

// 004663d0 (B265): altura de cada linha = maior LineHeight dos elementos que cobrem a linha; o 983 so aceita o resultado quando o
// laco de 00468700 para antes do fim (linha < inicio de um elemento); chegando ao fim usa a altura do "A" na fonte (w30).
// Depois x _fontStyle/100 arredondado, menos na ultima linha do ultimo paragrafo (last), que entra sem espaco.
int RFParagraph983::Height983(bool last) const
{
	int total = 0;
	for (unsigned int line = 0; line < (unsigned int)_one; ++line)
	{
		unsigned int maxH = 0;
		RFTextElement983** it = _first;
		for (; it != _last; ++it)
		{
			int lo, hi;
			(*it)->LineRange983(&lo, &hi);
			if (line < (unsigned int)lo)
				break;
			if (line <= (unsigned int)hi)
			{
				unsigned int h = (unsigned int)(*it)->LineHeight983((int)line);
				if (h > maxH)
					maxH = h;
			}
		}
		int h;
		if (it == _last)
		{
			RFSize sz = g_pRFInterface983->GetTextExtent983(*(const FontInfo*)_w30, std::wstring(L"A"));
			h = (int)(sz.height + 0.5);
		}
		else
			h = (int)maxH;
		if (last && line == (unsigned int)_one - 1)
			total += h;
		else
			total += (int)((double)(unsigned int)(_fontStyle * h) / 100.0 + 0.5);
	}
	return total;
}

// 00459740 (instancia do for_each do RFTextAreaSC; 458bf0): soma Height983 de todos os paragrafos
int RFText983::TotalHeight983()
{
	int total = 0;
	for (unsigned int i = 0; i < _size; ++i)
		total += ParaAt983(i)->Height983(i == _size - 1);
	return total;
}

// 00456e10 (4589c0 -> 4594b0): largura nova em cada paragrafo (+0x14) e recontagem das linhas
void RFText983::SetWidth983(int w)
{
	_p2c = w;
	for (unsigned int i = 0; i < _size; ++i)
	{
		RFParagraph983* p = ParaAt983(i);
		p->_font = w;
		p->RecountLines983();
	}
}

// 004574a0: soma das linhas (+0x1c) de todos os parágrafos
unsigned int RFText983::TotalLines983()
{
	unsigned int n = 0;
	for (unsigned int i = 0; i < _size; ++i)
		n += ParaAt983(i)->_one;
	return n;
}

// 00456600: insere o texto no parágrafo corrente (parse = RFTextParser, 00466f20: ParseAppend983; senão InsertText983 no cursor);
// cada '\n' (escolhido por 004666c0) é apagado e divide o parágrafo no cursor (00465660); o novo vira o corrente.
// Depois, se o total de linhas passa de +0x38, o parágrafo mais antigo sai (004577b0 com índice 0).
void RFText983::AddText983(const wchar_t* text, bool parse)
{
	unsigned int n = 0;
	while (text[n])
		++n;
	RFWStr983 tmp;
	tmp.Assign983(text, n);
	RFParagraph983* cur = _current;
	if (tmp.size != 0) {
		cur->_flag = false;
		if (parse)
			cur->ParseAppend983(tmp);
		else
			cur->InsertText983(tmp);
		cur->RecountLines983();
	}
	int idx;
	while ((idx = cur->FindNewline983()) != -1) {
		cur->_zero = idx;
		unsigned int total = 0;
		for (RFTextElement983** it = cur->_first; it != cur->_last; ++it)
			total += (*it)->GetLength983();
		if ((unsigned int)idx < total)
			cur->EraseAt983((unsigned int)idx);
		cur->RemoveEmpty983();
		cur->RecountLines983();
		RFParagraph983* np = new RFParagraph983(_p2c, _p34, 0, _p24, _p28, _p20);
		cur->SplitTo983(np);
		// posição do corrente na deque (+1) e inserção logo depois
		unsigned int pos = _size;
		for (unsigned int i = 0; i < _size; ++i) {
			if (ParaAt983(i) == cur) { pos = i; break; }
		}
		InsertParagraph983(pos + 1, np);
		_current = np;
		cur = np;
	}
	if (TotalLines983() > (unsigned int)_p38 && _size > 1) {
		delete ParaAt983(0);
		for (unsigned int i = 0; i + 1 < _size; ++i)
			ParaAt983(i) = ParaAt983(i + 1);
		--_size;
	}
}

// 004697d0: mesmo tipo (aqui: os dois sao texto), +0x30, cor +0x6c, +0x50, +0x34, +0x70, +0x74
static bool SameWStr983(const RFWStr983& a, const RFWStr983& b)
{
	return a.size == b.size && (a.size == 0 || memcmp(a.c_str(), b.c_str(), a.size * sizeof(wchar_t)) == 0);
}

bool RFTextRun983::IsSameStyle983(RFTextElement983* o)
{
	const RFWStr983* ot = o ? o->TextOf983() : 0;
	if (!ot)
		return false;
	RFTextRun983* r = (RFTextRun983*)o;
	return _f30 == r->_f30 && _color == r->_color && SameWStr983(_s50, r->_s50) && SameWStr983(_s34, r->_s34) &&
		_f70 == r->_f70 && _f74 == r->_f74;
}

// 00466ce0: acha a linha do cursor e vai para o pixel x dela
void RFParagraph983::MoveCursorToX983(unsigned int x)
{
	int line = 0, col = 0, px = 0;
	if (Locate983((unsigned int)_zero, &line, &col, &px))
		MoveToLineX983(line, x);
}

// 00465430 (ecx = src, edx = this no 983): cada trecho de src vai para o fim deste; se o ultimo daqui tem o mesmo estilo
// (slot 0x38) o texto e anexado a ele. No 983 os fundidos ficam com texto vazio e saem pelo RemoveEmpty (00467230) de src
// antes de o vetor ser esvaziado; aqui sao apagados direto (sem tocar nos que ja foram para este paragrafo).
void RFParagraph983::AppendFrom983(RFParagraph983* src)
{
	if (!src)
		return;
	unsigned int n = (unsigned int)(src->_last - src->_first);
	for (unsigned int i = 0; i < n; ++i) {
		RFTextElement983* e = src->_first[i];
		if (_first != _last && _last[-1]->IsSameStyle983(e)) {
			RFTextRun983* dst = (RFTextRun983*)_last[-1];
			const RFWStr983* t = e->TextOf983();
			dst->_text.Insert983(dst->_text.size, *t);
			delete e;
		} else {
			_pushElem(e);
		}
	}
	src->_last = src->_first;
	RecountLines983();
}

// 004577b0: apaga o paragrafo idx (a deque do 983 desloca os de tras; o corrente nao e tocado aqui)
void RFText983::ErasePara983(unsigned int idx)
{
	if (idx >= _size)
		return;
	delete ParaAt983(idx);
	for (unsigned int i = idx; i + 1 < _size; ++i)
		ParaAt983(i) = ParaAt983(i + 1);
	--_size;
}

// 00457a90 (+ std::find 00458e30)
int RFText983::IndexOfPara983(RFParagraph983* p)
{
	if (!p)
		return -1;
	for (unsigned int i = 0; i < _size; ++i)
		if (ParaAt983(i) == p)
			return (int)i;
	return -1;
}

// 00457910: o anterior de p (0 se p e o primeiro ou nao esta)
RFParagraph983* RFText983::PrevPara983(RFParagraph983* p)
{
	int i = IndexOfPara983(p);
	return i > 0 ? ParaAt983((unsigned int)(i - 1)) : 0;
}

// 004579d0: o seguinte de p (0 se p e o ultimo ou nao esta)
RFParagraph983* RFText983::NextPara983(RFParagraph983* p)
{
	int i = IndexOfPara983(p);
	return (i >= 0 && (unsigned int)(i + 1) < _size) ? ParaAt983((unsigned int)(i + 1)) : 0;
}

// 00457620: o paragrafo corrente move o cursor (466830); se bateu na borda (kind/arg) passa para o vizinho:
// esq./cima -> anterior (cursor no fim; cima: na linha ultima, pixel arg), dir./baixo -> seguinte (cursor 0; baixo: pixel arg),
// backspace no inicio -> anexa o corrente ao anterior e apaga o corrente, delete no fim -> anexa o seguinte ao corrente.
void RFText983::HandleCursor983(int cmd)
{
	RFCursorMove983 r;
	if (!_current)
		return;
	_current->MoveCursor983(cmd, &r);
	if (!r.handled)
		return;
	RFParagraph983* cur = _current;
	RFParagraph983* o;
	switch (r.kind) {
	case RFCursorMove983::LeftEdge:
		if ((o = PrevPara983(cur)) != 0) {
			_current = o;
			o->MoveCursorToEnd983();
		}
		break;
	case RFCursorMove983::RightEdge:
		if ((o = NextPara983(cur)) != 0) {
			_current = o;
			o->_zero = 0;
		}
		break;
	case RFCursorMove983::UpEdge:
		if ((o = PrevPara983(cur)) != 0) {
			_current = o;
			o->MoveCursorToEnd983();
			o->MoveCursorToX983((unsigned int)r.arg);
		}
		break;
	case RFCursorMove983::DownEdge:
		if ((o = NextPara983(cur)) != 0) {
			_current = o;
			o->_zero = 0;
			o->MoveCursorToX983((unsigned int)r.arg);
		}
		break;
	case RFCursorMove983::BackspaceAtStart:
		if ((o = PrevPara983(cur)) != 0) {
			o->MoveCursorToEnd983();
			o->AppendFrom983(cur);
			_current = o;
			ErasePara983((unsigned int)IndexOfPara983(cur));
		}
		break;
	case RFCursorMove983::DeleteAtEnd:
		if ((o = NextPara983(cur)) != 0) {
			cur->AppendFrom983(o);
			ErasePara983((unsigned int)IndexOfPara983(o));
		}
		break;
	}
}

// 00456570: apaga os par�grafos (004564b0) e p�e 1 vazio com os par�metros do ctor; o corrente s� muda se a inser��o devolve algo
void RFText983::Reset983()
{
	ClearParagraphs983();
	RFParagraph983* p = new RFParagraph983(_p2c, _p34, 0, _p24, _p28, _p20);
	RFParagraph983* r = InsertParagraph983(_size, p);
	if (r)
		_current = r;
}

// 00456fa0 (+ 00457090 por par�grafo): texto de cada par�grafo (466620 = elementos em sequ�ncia), L"\n" entre eles
void RFText983::GetText983(std::wstring& out)
{
	out.clear();
	for (unsigned int i = 0; i < _size; ++i) {
		RFParagraph983* p = ParaAt983(i);
		for (RFTextElement983** it = p->_first; it != p->_last; ++it) {
			const RFWStr983* t = (*it)->TextOf983();
			if (t)
				out.append(t->c_str(), t->size);
		}
		if (i + 1 != _size)
			out.append(1, L'\n');
	}
}

// ---- consulta de linhas do TextBlock (slots 0x3c..0x54, B178) ----
static const RFTextLine983* FindLine983(const RFTextRun983* r, int line)
{
	for (const RFTextLine983* it = (const RFTextLine983*)r->_f04; it != (const RFTextLine983*)r->_f08; ++it) {
		if (it->line == line)
			return it;
	}
	return 0;
}

int RFTextRun983::LineStart983(int line) { const RFTextLine983* l = FindLine983(this, line); return l ? l->start : -1; } // 00469870
int RFTextRun983::LineEnd983(int line) { const RFTextLine983* l = FindLine983(this, line); return l ? l->idx : -1; } // 004698c0
int RFTextRun983::LineWidth983(int line) { const RFTextLine983* l = FindLine983(this, line); return l ? l->w : -1; } // 00469910
int RFTextRun983::LineHeight983(int line) { const RFTextLine983* l = FindLine983(this, line); return l ? l->h : -1; } // 00469960

// 004699b0: primeira e ultima linha do trecho (-1 se nao ha)
void RFTextRun983::LineRange983(int* first, int* last)
{
	*first = -1;
	*last = -1;
	const RFTextLine983* b = (const RFTextLine983*)_f04;
	const RFTextLine983* e = (const RFTextLine983*)_f08;
	if (b != e) {
		*first = b->line;
		*last = e[-1].line;
	}
}

// fonte guardada pelo Layout983 em +0x78/+0x94 (o 983 passa this+0x78 direto a GetTextExtent)
static float MeasureRun983(const RFTextRun983* r, const wchar_t* s, unsigned int n)
{
	FontInfo fi;
	fi.name.assign(r->_s78.c_str(), r->_s78.size);
	fi.size = r->_f94;
	fi.style = (unsigned int)r->_f98;
	fi.color = (unsigned int)r->_f9c;
	fi.outlineColor = (unsigned int)r->_fa0;
	fi.shadowColor = (unsigned int)r->_fa4;
	return g_pRFInterface983->GetTextExtent983(fi, std::wstring(s, n)).width;
}

// 004699e0: x (pixel na linha) -> numero de caracteres antes de x; se x passa da largura da linha, x -= largura
int RFTextRun983::ColumnToOffset983(int line, unsigned int* x)
{
	const RFTextLine983* l = FindLine983(this, line);
	if (!l)
		return -1;
	if (*x > (unsigned int)l->w) {
		*x -= (unsigned int)l->w;
		return l->idx - l->start;
	}
	float acc = 0.0f;
	unsigned int i = 0;
	while (i < l->text.size) {
		acc += MeasureRun983(this, l->text.c_str() + i, 1);
		if ((unsigned int)(int)acc > *x)
			break;
		++i;
	}
	*x = 0;
	return (int)i;
}

// 00469b70: sticky -> 469bb0 (primeira linha que contem pos); senao 469d20 (a ultima). col = pos - start; x = largura da linha
void RFTextRun983::Locate983(unsigned int pos, int* line, int* col, int* x, bool sticky)
{
	const RFTextLine983* b = (const RFTextLine983*)_f04;
	const RFTextLine983* e = (const RFTextLine983*)_f08;
	const RFTextLine983* f = 0;
	if (sticky) {
		for (const RFTextLine983* it = b; it != e; ++it) {
			if ((unsigned int)it->start <= pos && pos <= (unsigned int)it->idx) { f = it; break; }
		}
	} else {
		for (const RFTextLine983* it = e; it != b; --it) {
			if ((unsigned int)it[-1].start <= pos && pos <= (unsigned int)it[-1].idx) { f = it - 1; break; }
		}
	}
	if (!f)
		return;
	*line = f->line;
	*col = (int)(pos - (unsigned int)f->start);
	*x = (int)MeasureRun983(this, f->text.c_str(), f->text.size);
}

// ---- movimento do cursor do paragrafo (00466830 e auxiliares, B178) ----
// 00466200 / 00466a70: soma dos comprimentos (slot 0x2c)
unsigned int RFParagraph983::TotalLength983() const
{
	unsigned int n = 0;
	for (RFTextElement983** it = _first; it != _last; ++it)
		n += (*it)->GetLength983();
	return n;
}

// 004673e0: sticky (fim de linha): primeiro elemento com restante <= comprimento; senao (467500): o ultimo elemento cujo
// inicio <= pos (468a50 varre do fim). *off = deslocamento dentro do elemento.
RFTextElement983* RFParagraph983::FindElement983(unsigned int pos, unsigned int* off, bool sticky) const
{
	if (sticky) {
		*off = 0;
		RFTextElement983** it = _first;
		for (; it != _last; ++it) {
			unsigned int len = (*it)->GetLength983();
			if (len >= pos)
				break;
			pos -= len;
		}
		if (it == _last)
			return 0;
		*off = pos;
		return *it;
	}
	*off = 0;
	unsigned int acc = TotalLength983();
	RFTextElement983** it = _last;
	for (;;) {
		if (it == _first)
			return 0;
		acc -= it[-1]->GetLength983();
		if (acc <= pos) {
			*off = pos - acc;
			return it[-1];
		}
		--it;
	}
}

// 004672f0: linha/coluna/pixel x da posicao; o pixel soma a largura da mesma linha nos elementos anteriores (004689b0)
RFTextElement983* RFParagraph983::Locate983(unsigned int pos, int* line, int* col, int* x)
{
	unsigned int off = pos;
	RFTextElement983* e = FindElement983(pos, &off, _flag);
	if (!e)
		return 0;
	e->Locate983(off, line, col, x, _flag);
	for (RFTextElement983** it = _first; it != _last; ++it) {
		int lo, hi;
		(*it)->LineRange983(&lo, &hi);
		if (*line < lo)
			break;
		if (*line <= hi) {
			if (*it == e)
				break;
			*x += (*it)->LineWidth983(*line);
		}
	}
	return e;
}

// 00468800: posicao do inicio da linha (soma dos comprimentos dos elementos sem a linha)
unsigned int RFParagraph983::LineStartPos983(int line) const
{
	unsigned int pos = 0;
	for (RFTextElement983** it = _first; it != _last; ++it) {
		int s = (*it)->LineStart983(line);
		if (s != -1) {
			pos += (unsigned int)s;
			break;
		}
		pos += (*it)->GetLength983();
	}
	return pos;
}

// 00466c20 + 004688b0: cursor = inicio da linha + caracteres ate o pixel x
void RFParagraph983::MoveToLineX983(int line, unsigned int x)
{
	_zero = (int)LineStartPos983(line);
	for (RFTextElement983** it = _first; it != _last; ++it) {
		int lo, hi;
		(*it)->LineRange983(&lo, &hi);
		if (line < lo)
			break;
		if (line <= hi) {
			_zero += (*it)->ColumnToOffset983(line, &x);
			if (x == 0)
				return;
		}
	}
}

void RFParagraph983::MoveToLineHome983(int line) { _zero = (int)LineStartPos983(line); } // 00466ad0

// 00466b80 + 00468850: fim da linha = soma dos [idx] da linha; elementos anteriores entram inteiros
void RFParagraph983::MoveToLineEnd983(int line)
{
	unsigned int pos = 0;
	bool found = false;
	for (RFTextElement983** it = _first; it != _last; ++it) {
		int n = (*it)->LineEnd983(line);
		if (n == -1) {
			if (found)
				break;
			n = (int)(*it)->GetLength983();
		} else {
			found = true;
		}
		pos += (unsigned int)n;
	}
	_zero = (int)pos;
}

int RFParagraph983::AdvanceCursor983(int delta)
{
	unsigned int total = TotalLength983();
	unsigned int np = (unsigned int)_zero + (unsigned int)delta;
	if (np <= total) {
		_zero = (int)np;
		return 0;
	}
	_zero = (int)total;
	return delta;
}

// 004660e0: so apaga se pos < total (e n != 0)
void RFParagraph983::DeleteN983(unsigned int pos, int n)
{
	if (n != 0 && pos < TotalLength983())
		EraseAt983(pos, n);
}

// 00456ab0 (chamada pelo RFTextArea::OnChar): apaga o trecho em composicao anterior (+0x50 caracteres antes do cursor), guarda
// o tamanho do novo (so se active) e insere text, desde que o total nao passe do limite +0x3c.
void RFText983::SetComposition983(const wchar_t* text, bool active)
{
	if (_p4c) {
		RFParagraph983* cur = _current;
		unsigned int len = (unsigned int)_p50;
		cur->_zero = (unsigned int)cur->_zero < len ? 0 : (int)((unsigned int)cur->_zero - len);
		cur->DeleteN983((unsigned int)cur->_zero, (int)len);
		cur->RemoveEmpty983();
		cur->RecountLines983();
	}
	unsigned int n = 0;
	while (text[n])
		++n;
	if (!active) {
		_p4c = false;
		_p50 = 0;
	} else {
		_p50 = (int)n;
		_p4c = n != 0;
	}
	unsigned int total = 0;
	for (unsigned int i = 0; i < _size; ++i)
		total += ParaAt983(i)->TotalLength983();
	if (n + total <= (unsigned int)_p3c)
		AddText983(text, false);
}

// 00466080 (+ 004660e0 com contagem 1): apaga 1 caractere no cursor, tira os trechos vazios e recontas as linhas
void RFParagraph983::DeleteAtCursor983()
{
	if ((unsigned int)_zero < TotalLength983())
		EraseAt983((unsigned int)_zero);
	RemoveEmpty983();
	RecountLines983();
}

// 00466830: cmd 0 cima, 1 baixo, 2 esquerda, 3 direita, 4 home, 5 end, 8 delete, 9 backspace (6 e 7 nao fazem nada).
// r->handled = o cursor bateu na borda do paragrafo; r->kind/arg dizem qual (o RFText passa para o paragrafo vizinho).
void RFParagraph983::MoveCursor983(int cmd, RFCursorMove983* r)
{
	int line = 0, col = 0, x = 0;
	switch (cmd) {
	case CURSOR_UP983:
		if (Locate983((unsigned int)_zero, &line, &col, &x)) {
			if (line > 0) {
				MoveToLineX983(line - 1, (unsigned int)x);
				return;
			}
			r->kind = RFCursorMove983::UpEdge;
			r->arg = x;
		} else {
			r->kind = RFCursorMove983::UpEdge;
			r->arg = 0;
		}
		r->handled = true;
		_flag = false;
		return;
	case CURSOR_DOWN983:
		if (Locate983((unsigned int)_zero, &line, &col, &x)) {
			if ((unsigned int)line < (unsigned int)(_one - 1)) {
				MoveToLineX983(line + 1, (unsigned int)x);
				return;
			}
			r->handled = true;
			r->kind = RFCursorMove983::DownEdge;
			r->arg = x;
		} else {
			r->kind = RFCursorMove983::DownEdge;
			r->arg = 0;
			r->handled = true;
		}
		_flag = false;
		return;
	case CURSOR_LEFT983:
		if ((unsigned int)_zero >= 1) {
			--_zero;
		} else {
			_zero = 0;
			r->kind = RFCursorMove983::LeftEdge;
			r->handled = true;
		}
		_flag = false;
		return;
	case CURSOR_RIGHT983:
		if (AdvanceCursor983(1) != 0) {
			r->handled = true;
			r->kind = RFCursorMove983::RightEdge;
		}
		_flag = false;
		return;
	case CURSOR_HOME983:
		if (Locate983((unsigned int)_zero, &line, &col, &x))
			MoveToLineHome983(line);
		_flag = false;
		return;
	case CURSOR_END983:
		if (Locate983((unsigned int)_zero, &line, &col, &x)) {
			MoveToLineEnd983(line);
			_flag = true;
		}
		return;
	case CURSOR_DELETE983:
		if ((unsigned int)_zero < TotalLength983()) {
			DeleteAtCursor983();
			return;
		}
		r->handled = true;
		r->kind = RFCursorMove983::DeleteAtEnd;
		return;
	case CURSOR_BACKSPACE983:
		if (_zero != 0) {
			--_zero;
			DeleteAtCursor983();
			return;
		}
		_zero = 0;
		r->handled = true;
		r->kind = RFCursorMove983::BackspaceAtStart;
		return;
	}
}

// ---- B266: hit-test de links, hover, posicao das linhas, cursor pelo ponto e desenho (RFTextArea 00426e00-00428040) ----

// 00469150 + 004691d0 (find_if 0046b0f0): a primeira linha marcada visivel (+0x1c) cujo retangulo [f14, f14+width] x [f18, f18+h]
// contem o ponto (bordas incluidas); *beyondEnd = o ponto nao esta antes do fim do texto da linha (f14 + w <= x)
bool RFTextRun983::HitLine983(int x, int y, int width, bool* beyondEnd, int* line)
{
	for (RFTextLine983* it = (RFTextLine983*)_f04; it != (RFTextLine983*)_f08; ++it) {
		if (!it->f1c)
			continue;
		float left = (float)it->f14;
		float right = left + (float)(unsigned int)width;
		float top = (float)it->f18;
		float bottom = top + (float)(unsigned int)it->h;
		float fx = (float)x, fy = (float)y;
		if (left <= fx && fx <= right && top <= fy && fy <= bottom) {
			*beyondEnd = !(it->w + it->f14 > x);
			*line = it->line;
			return true;
		}
	}
	return false;
}

// 004692a0 + 0046b1d0: so trechos de link (estilo 0x10). Cada linha testada marca o hover do trecho (slot 8): 1 na que contem o
// ponto (e para), 0 nas anteriores. Achada a linha, o modo tem de ser o do trecho (+0x70) ou 3; copia o alvo (+0x50)
bool RFTextRun983::HitTestLink983(int x, int y, const int* mode, std::wstring* out)
{
	if (!(_f30 & 0x10))
		return false;
	RFTextLine983* it = (RFTextLine983*)_f04;
	for (; it != (RFTextLine983*)_f08; ++it) {
		bool hit = false;
		if (it->f1c) {
			float left = (float)it->f14;
			float right = left + (float)(unsigned int)it->w;
			float top = (float)it->f18;
			float bottom = top + (float)(unsigned int)it->h;
			float fx = (float)x, fy = (float)y;
			hit = left <= fx && fx <= right && top <= fy && fy <= bottom;
		}
		SetHover983(hit);
		if (hit)
			break;
	}
	if (it == (RFTextLine983*)_f08)
		return false;
	if (*mode != _f70 && *mode != 3)
		return false;
	out->assign(_s50.c_str(), _s50.size);
	return true;
}

// 004693f0 / 00469420
void RFTextRun983::ClearLineFlags983()
{
	for (RFTextLine983* it = (RFTextLine983*)_f04; it != (RFTextLine983*)_f08; ++it)
		it->f1c = false;
}

void RFTextRun983::ShowAllLines983()
{
	for (RFTextLine983* it = (RFTextLine983*)_f04; it != (RFTextLine983*)_f08; ++it)
		it->f1c = true;
}

// 00469780
unsigned int RFTextRun983::WidthSum983()
{
	unsigned int sum = 0;
	for (RFTextLine983* it = (RFTextLine983*)_f04; it != (RFTextLine983*)_f08; ++it)
		sum += (unsigned int)it->w;
	return sum;
}

// 0046a2d0: posicao (+0x14/+0x18) da linha de numero `line`
void RFTextRun983::SetLinePos983(int line, int x, int y)
{
	for (RFTextLine983* it = (RFTextLine983*)_f04; it != (RFTextLine983*)_f08; ++it) {
		if (it->line == line) {
			it->f14 = x;
			it->f18 = y;
			return;
		}
	}
}

// 004694c0 (ebx = linha, ecx = {trecho, area, alfa}): uma linha do trecho; ver DrawLines983
static __forceinline void DrawLineBody983(const RFTextRun983* run, RFTextArea* owner, float alpha, RFTextLine983* line)
{
	line->f1c = true;
	if (run->_f30 & 8) {
		RFVector2 pos((float)line->f14, (float)line->f18);
		owner->SendDrawImageEvent(pos, std::wstring(run->_s34.c_str(), run->_s34.size));
		return;
	}
	FontInfo font;
	font.name.assign(run->_s78.c_str(), run->_s78.size);
	font.size = run->_f94;
	font.style = (unsigned int)run->_f98;
	unsigned int color = (unsigned int)run->_f9c;
	font.outlineColor = (unsigned int)run->_fa0;
	font.shadowColor = (unsigned int)run->_fa4;
	unsigned int a = (unsigned int)((double)(color >> 24) / 255.0 * alpha * 255.0);
	font.color = (((a << 8 | ((color >> 16) & 0xff)) << 8 | ((color >> 8) & 0xff)) << 8) | (color & 0xff);
	float depth = owner->GetRC()->_depth;
	if (run->_fa8)
		g_pRFRenderer983->FillRectangle(0xfaadd6ff, (float)line->f14, (float)line->f18, (float)(unsigned int)line->w,
			(float)(unsigned int)line->h, depth);
	g_pRFInterface983->PrintText983(font, line->f14, line->f18, std::wstring(line->text.c_str()), depth);
}

// copia isolada que o 983 mantem alem da inline em 0046b320 (LTCG)
__declspec(noinline) void DrawLine983(const RFTextRun983* run, RFTextArea* owner, float alpha, RFTextLine983* line)
{
	DrawLineBody983(run, owner, alpha, line);
}

// 0046b320 -> 004694c0 por linha: marca a linha visivel; trecho de imagem (estilo 8) = evento RichTextDrawImage na posicao da
// linha; texto = fundo do hover (0xfaadd6ff) e PrintText com a fonte do trecho (alfa da cor x alfa recebido)
void RFTextRun983::DrawLines983(RFTextArea* owner, float alpha)
{
	for (RFTextLine983* line = (RFTextLine983*)_f04; line != (RFTextLine983*)_f08; ++line)
		DrawLineBody983(this, owner, alpha, line);
}

// 00465cd0 (esi = paragrafo; x, y, &alinhamento): cada linha do paragrafo recebe a posicao (x pelo alinhamento da largura +0x14:
// 1 = centro, 2 = direita; y = soma das alturas x _fontStyle/100); o cursor (+0x20, +0x24) vira _f28/_f2c (pixel da linha do
// cursor + x). Sem trechos: altura de um "A" da fonte (+0x30). Devolve a altura total.
int RFParagraph983::Position983(int x, int y, const int* hAlign)
{
	_f28 = x;
	_f2c = y;
	int total = 0;
	if (_first == _last) {
		RFSize sz = g_pRFInterface983->GetTextExtent983(*(const FontInfo*)_w30, std::wstring(L"A"));
		int h0 = (int)(sz.height + 0.5);
		int h = (int)((double)(unsigned int)(h0 * _fontStyle) / 100.0 + 0.5);
		if (*hAlign == 1)
			x += (int)((unsigned int)_font >> 1);
		else if (*hAlign == 2)
			x += _font;
		_f28 = x;
		_f2c = y;
		return h;
	}
	int xs = x;
	for (unsigned int line = 0; line < (unsigned int)_one; ++line) {
		int cl = -1, cc = 0, cx = 0;
		Locate983((unsigned int)_zero, &cl, &cc, &cx);
		int xline = xs;
		unsigned int lineW = 0;
		for (RFTextElement983** it = _first; it != _last; ++it) {
			int lo, hi;
			(*it)->LineRange983(&lo, &hi);
			if ((int)line < lo)
				break;
			if ((int)line <= hi)
				lineW += (unsigned int)(*it)->LineWidth983((int)line);
		}
		if (lineW < (unsigned int)_font) {
			if (*hAlign == 1)
				xline = (int)(((unsigned int)_font - lineW) >> 1) + xs;
			else if (*hAlign == 2)
				xline = (int)((unsigned int)_font - lineW) + xs;
		}
		if ((int)line == cl) {
			_f28 = cx + xline;
			_f2c = total + y;
		}
		unsigned int curX = 0, maxH = 0;
		for (RFTextElement983** it = _first; it != _last; ++it) {
			int lo, hi;
			(*it)->LineRange983(&lo, &hi);
			if ((int)line < lo)
				break;
			if ((int)line <= hi) {
				(*it)->SetLinePos983((int)line, (int)curX + xline, total + y);
				curX += (unsigned int)(*it)->LineWidth983((int)line);
				unsigned int h = (unsigned int)(*it)->LineHeight983((int)line);
				if (h > maxH)
					maxH = h;
			}
		}
		total += (int)((double)(unsigned int)(_fontStyle * (int)maxH) / 100.0 + 0.5);
	}
	return total;
}

// 00456cd0
void RFText983::ResetHover983()
{
	for (unsigned int i = 0; i < _size; ++i) {
		RFParagraph983* p = ParaAt983(i);
		for (RFTextElement983** it = p->_first; it != p->_last; ++it)
			(*it)->ResetHover983();
	}
}

// 00459550 (find_if sobre os paragrafos e seus trechos, chamada em 00427170 e 00426f60)
bool RFText983::HitTestLink983(int x, int y, int mode, std::wstring* out)
{
	for (unsigned int i = 0; i < _size; ++i) {
		RFParagraph983* p = ParaAt983(i);
		for (RFTextElement983** it = p->_first; it != p->_last; ++it) {
			if ((*it)->HitTestLink983(x, y, &mode, out))
				return true;
		}
	}
	return false;
}

// 00457550 -> 004598f0: o primeiro trecho com uma linha sob (x, y) leva o cursor do seu paragrafo ao inicio da linha + o pixel
// relX (coluna), liga o "fim de linha" se o ponto passa do texto, e o paragrafo vira o corrente (+4)
bool RFText983::SetCursorAtPoint983(int x, int y, int relX)
{
	for (unsigned int i = 0; i < _size; ++i) {
		RFParagraph983* p = ParaAt983(i);
		for (RFTextElement983** it = p->_first; it != p->_last; ++it) {
			int line = 0;
			if ((*it)->HitLine983(x, y, p->_font, &p->_flag, &line)) {
				p->_zero = (int)p->LineStartPos983(line);
				p->MoveToLineX983(line, (unsigned int)relX);
				_current = p;
				return true;
			}
		}
	}
	return false;
}

// 00459800 (via 00458c60)
unsigned int RFText983::MaxWidth983()
{
	unsigned int maxW = 0;
	for (unsigned int i = 0; i < _size; ++i) {
		RFParagraph983* p = ParaAt983(i);
		unsigned int sum = 0;
		for (RFTextElement983** it = p->_first; it != p->_last; ++it)
			sum += (*it)->WidthSum983();
		if (sum > maxW)
			maxW = sum;
	}
	return maxW;
}

// 00458950 -> 00459440 (chamada pela Invaildate 00427fb0): y avanca pela altura de cada paragrafo
void RFText983::Position983(int x, int y)
{
	for (unsigned int i = 0; i < _size; ++i)
		y += ParaAt983(i)->Position983(x, y, &_p44);
}

// 00456bc0 (alfa = GetAlpha do controle): 4592c0 (slot 5 em todos) e 459320 (0046b320 por linha com {trecho, +0x38 do paragrafo, alfa})
void RFText983::Draw983(float alpha)
{
	for (unsigned int i = 0; i < _size; ++i) {
		RFParagraph983* p = ParaAt983(i);
		for (RFTextElement983** it = p->_first; it != p->_last; ++it)
			(*it)->ClearLineFlags983();
	}
	for (unsigned int i = 0; i < _size; ++i) {
		RFParagraph983* p = ParaAt983(i);
		for (RFTextElement983** it = p->_first; it != p->_last; ++it)
			(*it)->DrawLines983((RFTextArea*)p->_w38, alpha);
	}
}
