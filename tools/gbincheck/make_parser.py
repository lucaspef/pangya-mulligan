#!/usr/bin/env python3
"""Recorta de client/src/projectg/polysoup.{cpp,h} o parser real do .gbin para o gbincheck.

Nada é reescrito: os trechos são copiados como estão, e o stub.h só fornece o que eles usam (cFile, WVector...).
Assim o harness acompanha o cliente: se o parser mudar, o próximo build usa o código novo.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, '..', '..', 'client', 'src', 'projectg')


def cut(text, start, end):
    a = text.index(start)
    return text[a:text.index(end, a)]


def main(out):
    # latin-1 preserva os bytes CP949 dos comentários
    cpp = open(os.path.join(SRC, 'polysoup.cpp'), encoding='latin-1', newline='').read()
    hdr = open(os.path.join(SRC, 'polysoup.h'), encoding='latin-1', newline='').read()
    decl = cut(hdr, 'struct sLink983', 'struct sBgFx')
    code = [
        cut(cpp, 'static inline int ReadVector983', '// 983: hierarquia dos registros do mapa (sNode'),
        cut(cpp, '// 983: registro do modelo no arquivo', 'static bool IsSilviaCannonArea(const WPolySoup::sArea* area); //'),
        cut(cpp, 'void WPolySoup::sTexture::Clear()', 'bool __fastcall WPolySoup::CompareModel983'),
        cut(cpp, 'struct sTeePinPoints983 // 0x10', '// 983: 0092f910'),
        cut(cpp, 'void sVertexColorArray983::Resize', '// 983: 009361b0'),
    ]
    with open(os.path.join(out, 'polysoup_decl.inl'), 'w', encoding='latin-1', newline='') as f:
        f.write(decl)
    with open(os.path.join(out, 'polysoup_code.inl'), 'w', encoding='latin-1', newline='') as f:
        f.write('\n'.join(code))


if __name__ == '__main__':
    main(sys.argv[1])
