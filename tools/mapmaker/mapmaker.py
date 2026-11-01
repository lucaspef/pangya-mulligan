#!/usr/bin/env python3
"""mapmaker: gera um buraco novo do PangYa a partir de uma descrição JSON.

uso:  mapmaker.py <hole.json> -o <saida> [--base-property <curso>_property.xml] [--no-compile]

Saída:
  <saida>/loose/...          árvore de dados: .wep, .pet, .gbin, .sbin, .dds, _property.xml
  <saida>/projectg_zz_mulligan.pak    o mesmo em pak; instalado na pasta do jogo, vence os paks originais

Só usa a biblioteca padrão do Python. O .gbin sai do compilador do FairwayED (tools/bin/fwcompile.exe).
Formato da entrada e instalação: tools/mapmaker/README.md.
"""
import argparse
import json
import math
import os
import random
import re
import struct
import subprocess
import sys
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
FWCOMPILE = os.path.join(HERE, '..', 'bin', 'fwcompile.exe')

# nomes de ponto que o cliente procura (FindTeePinPoints983, PlaceHoleCup), em CP949 no arquivo
TEE, TEE2, PIN = '시작점', '시작점2', '끝점'
PAK_NAME = 'projectg_zz_mulligan.pak'    # o cliente engancha por strcmp e procura do último para o primeiro: precisa ordenar depois de projectg_mland.pak

# superfície: textura, cor base, cor de variação, propriedades do _property.xml (o "pet" decide o tipo de chão no cliente)
SURFACES = {
    'tee':     ('mm_tee.dds',     (118, 176, 84),  (96, 150, 70),   (0.3, 0.3, 100, 100, 'start_point.pet', '공_티샷.wav')),
    'fairway': ('mm_fairway.dds', (104, 164, 72),  (84, 142, 60),   (0.4, 0.2, 95, 100, 'fairway.pet', '공_페어웨이.wav')),
    'rough':   ('mm_rough.dds',   (66, 118, 52),   (48, 92, 42),    (0.2, 0.42, 85, 95, 'rought.pet', '공_러프.wav')),
    'green':   ('mm_green.dds',   (136, 198, 96),  (122, 184, 88),  (0.3, 0.18, 99, 100, 'green.pet', '공_그린.wav')),
    'bunker':  ('mm_bunker.dds',  (226, 208, 158), (204, 184, 134), (0.1, 0.6, 80, 85, 'bunker_thin.pet', '공_벙커.wav')),
    'bed':     ('mm_bed.dds',     (72, 96, 92),    (58, 80, 80),    (0.2, 0.42, 85, 95, 'rought.pet', '공_러프.wav')),
}
ZONE_SURFACE = {'tee': 'tee', 'fairway': 'fairway', 'rough': 'rough', 'green': 'green', 'bunker': 'bunker', 'water': 'bed'}


# -- formas: distância com sinal, negativa dentro ----------------------------------------------------------------------

def sdf_ellipse(s, x, z):
    c, n = math.cos(math.radians(s.get('rot', 0))), math.sin(math.radians(s.get('rot', 0)))
    dx, dz = x - s['center'][0], z - s['center'][1]
    u, v = (dx * c + dz * n) / s['rx'], (-dx * n + dz * c) / s['rz']
    k = math.hypot(u, v)
    return (k - 1.0) * min(s['rx'], s['rz'])


def sdf_path(s, x, z):
    best = 1e18
    pts = s['points']
    for (ax, az), (bx, bz) in zip(pts, pts[1:]):
        px, pz, wx, wz = x - ax, z - az, bx - ax, bz - az
        t = max(0.0, min(1.0, (px * wx + pz * wz) / (wx * wx + wz * wz)))
        best = min(best, math.hypot(px - wx * t, pz - wz * t))
    return best - s['width'] / 2.0


def sdf_rect(s, x, z):
    x0, z0, x1, z1 = s['rect']
    return max(x0 - x, x - x1, z0 - z, z - z1)


def sdf(zone, x, z):
    if 'rect' in zone:
        return sdf_rect(zone, x, z)
    if 'points' in zone:
        return sdf_path(zone, x, z)
    return sdf_ellipse(zone, x, z)


def smooth(t):
    t = max(0.0, min(1.0, t))
    return t * t * (3 - 2 * t)


# -- terreno -----------------------------------------------------------------------------------------------------------

class Terrain:
    def __init__(self, spec):
        self.spec = spec
        t = spec['terrain']
        self.w, self.d = t['size']
        self.cell = t.get('cell', 10)
        self.base = t.get('base_height', 0.0)
        self.hills = t.get('hills', [])
        self.zones = spec['zones']
        self.nx = int(self.w / self.cell)
        self.nz = int(self.d / self.cell)

    def height(self, x, z):
        h = self.base
        for hill in self.hills:
            r2 = ((x - hill['x']) ** 2 + (z - hill['z']) ** 2) / (hill['radius'] ** 2)
            h += hill['height'] * math.exp(-r2 * 2.0)
        for zone in self.zones:
            drop = zone.get('height')
            if drop is None:
                drop = {'bunker': -3.0, 'water': -16.0}.get(zone['type'])
            if not drop:
                continue
            fall = zone.get('falloff', 12.0)
            k = smooth(-sdf(zone, x, z) / fall + 0.15)
            if zone['type'] == 'green' and 'tilt' in zone:
                drop += zone['tilt'][0] * (x - zone['center'][0]) + zone['tilt'][1] * (z - zone['center'][1])
            h += drop * k
        return h

    def snap(self, x, z):
        """Puxa o vértice para a borda da zona mais próxima, para as bordas saírem curvas e não em degraus."""
        best, bx, bz = self.cell * 0.55, x, z
        for zone in self.zones:
            d = sdf(zone, x, z)
            if abs(d) >= best:
                continue
            e = 0.5
            gx = sdf(zone, x + e, z) - sdf(zone, x - e, z)
            gz = sdf(zone, x, z + e) - sdf(zone, x, z - e)
            g = math.hypot(gx, gz)
            if g < 1e-6:
                continue
            best, bx, bz = abs(d), x - d * gx / g, z - d * gz / g
        return bx, bz

    def surface(self, x, z):
        name = 'rough'
        for zone in self.zones:
            if sdf(zone, x, z) < 0:
                name = ZONE_SURFACE[zone['type']]
        return name

    def build(self):
        """Grade de vértices e triângulos; o winding é o do cliente: Cross(v2-v1, v0-v1) aponta para cima."""
        nx, nz = self.nx, self.nz
        x0, z0 = -self.w / 2.0, -self.d / 2.0
        verts = []
        for j in range(nz + 1):
            for i in range(nx + 1):
                x, z = x0 + i * self.cell, z0 + j * self.cell
                if 0 < i < nx and 0 < j < nz:
                    x, z = self.snap(x, z)
                verts.append((x, self.height(x, z), z))
        tris = []
        names = list(SURFACES)
        for j in range(nz):
            for i in range(nx):
                a = j * (nx + 1) + i
                b, c, d = a + 1, a + nx + 2, a + nx + 1
                # a diagonal segue a borda quando há duas superfícies no quadrado
                for t in ((a, c, b), (a, d, c)):
                    cx = sum(verts[v][0] for v in t) / 3.0
                    cz = sum(verts[v][2] for v in t) / 3.0
                    tris.append((t, names.index(self.surface(cx, cz))))
        return verts, tris


def vertex_normals(verts, tris):
    acc = [[0.0, 0.0, 0.0] for _ in verts]
    for (v0, v1, v2), _ in tris:
        a, b, c = verts[v0], verts[v1], verts[v2]
        ux, uy, uz = c[0] - b[0], c[1] - b[1], c[2] - b[2]
        wx, wy, wz = a[0] - b[0], a[1] - b[1], a[2] - b[2]
        n = (uy * wz - uz * wy, uz * wx - ux * wz, ux * wy - uy * wx)
        for v in (v0, v1, v2):
            for k in range(3):
                acc[v][k] += n[k]
    out = []
    for n in acc:
        m = math.sqrt(sum(c * c for c in n)) or 1.0
        out.append((n[0] / m, n[1] / m, n[2] / m))
    return out


# -- .pet --------------------------------------------------------------------------------------------------------------

def chunk(tag, body):
    return tag + struct.pack('<I', len(body)) + body


def coll_entry(name, box):
    def text(s):
        return struct.pack('<I', len(s)) + s
    return (struct.pack('<ii', 1, 1) + text(name) + text(b'base') + text(b'\0') + text(b'') + struct.pack('<6f', *box))


def write_pet(path, verts, tris, tile):
    """Modelo base de um osso, no formato sem chunk VERS dos cenários do jogo (wind_01.pet e afins)."""
    names = [SURFACES[k][0] for k in SURFACES]
    text = struct.pack('<I', len(names))
    for i, name in enumerate(names):
        raw = name.encode('ascii').ljust(32, b'\0')
        text += raw + struct.pack('<BBHIi', 1, i + 1, 0, 0xffffffff, 0x80 + i)
    identity = (1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0)
    bone = b'\x01base\0\xff' + struct.pack('<12f', *identity)
    normals = vertex_normals(verts, tris)
    mesh = bytearray(struct.pack('<I', len(verts)))
    for v in verts:
        mesh += struct.pack('<3fBBH', v[0], v[1], v[2], 255, 0, 0)
    mesh += struct.pack('<I', len(tris))
    for t, _ in tris:
        for v in t:
            mesh += struct.pack('<I3f', v, *normals[v]) + struct.pack('<2f', verts[v][0] / tile, verts[v][2] / tile)
    mesh += bytes(s for _, s in tris)
    xs, ys, zs = zip(*verts)
    box = (min(xs), min(ys) - 1, min(zs), max(xs), max(ys) + 40, max(zs))
    coll = struct.pack('<I', 2) + coll_entry(b'base', box) + coll_entry(b'center', box)
    data = (chunk(b'TEXT', text) + chunk(b'BONE', bone) + chunk(b'ANIM', b'\xff') + chunk(b'MESH', bytes(mesh)) +
            chunk(b'FANM', struct.pack('<I', 0)) + chunk(b'FRAM', struct.pack('<I', 0)) +
            chunk(b'MOTI', struct.pack('<I', 0)) + chunk(b'COLL', coll))
    with open(path, 'wb') as f:
        f.write(data)


# -- texturas: DDS DXT1 geradas aqui, sem nenhum arquivo do jogo ---------------------------------------------------------

def rgb565(c):
    return ((c[0] >> 3) << 11) | ((c[1] >> 2) << 5) | (c[2] >> 3)


def dxt1_block(px):
    lo = min(px, key=sum)
    hi = max(px, key=sum)
    c0, c1 = rgb565(hi), rgb565(lo)
    if c0 == c1:
        return struct.pack('<HHI', c0, c1, 0)
    if c0 < c1:
        c0, c1, hi, lo = c1, c0, lo, hi
    pal = [hi, lo, tuple((2 * a + b) // 3 for a, b in zip(hi, lo)), tuple((a + 2 * b) // 3 for a, b in zip(hi, lo))]
    bits = 0
    for i, p in enumerate(px):
        best = min(range(4), key=lambda k: sum((a - b) ** 2 for a, b in zip(p, pal[k])))
        bits |= best << (2 * i)
    return struct.pack('<HHI', c0, c1, bits)


def texture_pixels(name, base, alt, size):
    """Ruído de valor em duas escalas, repetível nas bordas; a grama leva faixas leves de corte."""
    rng = random.Random(name)
    grids = []
    for cells in (8, 32):
        grids.append((cells, [[rng.random() for _ in range(cells)] for _ in range(cells)]))
    mow = name in ('mm_fairway.dds', 'mm_green.dds')
    px = []
    for y in range(size):
        row = []
        for x in range(size):
            n = 0.0
            for cells, g in grids:
                u, v = x * cells / size, y * cells / size
                i, j = int(u), int(v)
                fu, fv = smooth(u - i), smooth(v - j)
                a = g[j % cells][i % cells] * (1 - fu) + g[j % cells][(i + 1) % cells] * fu
                b = g[(j + 1) % cells][i % cells] * (1 - fu) + g[(j + 1) % cells][(i + 1) % cells] * fu
                n += (a * (1 - fv) + b * fv) * (0.6 if cells == 8 else 0.4)
            n += rng.random() * 0.12 - 0.06
            if mow and (x * 4 // size) % 2:
                n += 0.1
            n = max(0.0, min(1.0, n))
            row.append(tuple(int(a * n + b * (1 - n)) for a, b in zip(base, alt)))
        px.append(row)
    return px


def write_dds(path, name, base, alt, size=128):
    px = texture_pixels(name, base, alt, size)
    mips = []
    while True:
        out = bytearray()
        s = len(px)
        for y in range(0, s, 4):
            for x in range(0, s, 4):
                out += dxt1_block([px[min(y + j, s - 1)][min(x + i, s - 1)] for j in range(4) for i in range(4)])
        mips.append(bytes(out))
        if s <= 4:
            break
        px = [[tuple(sum(px[y + b][x + a][c] for a in (0, 1) for b in (0, 1)) // 4 for c in range(3))
               for x in range(0, s, 2)] for y in range(0, s, 2)]
    head = struct.pack('<4s7I', b'DDS ', 124, 0x000A1007, size, size, len(mips[0]), 0, len(mips))
    head += b'\0' * 44
    head += struct.pack('<2I4s5I', 32, 4, b'DXT1', 0, 0, 0, 0, 0)
    head += struct.pack('<5I', 0x401008, 0, 0, 0, 0)
    with open(path, 'wb') as f:
        f.write(head)
        for m in mips:
            f.write(m)


# -- .wep --------------------------------------------------------------------------------------------------------------

def fmt(*v):
    return ' '.join('%.2f' % c for c in v)


def write_wep(path, spec, pet_name, terrain):
    tee = spec['tee']
    pins = spec['pins']
    sun = spec.get('sun', [420, 760, -380])
    light = spec.get('light', {})
    elements = []

    def element(body):
        elements.append(body)

    element('\t*Type  "GAME"\n\t*WadFile ""\n\t*BspFile ""\n\t*Ambient %s\n\t*Diffuse %s\n\t*Ambient2 255 255 255\n'
            '\t*Shadow %s\n\t*Par %d\n' % (light.get('ambient', '150 150 150'), light.get('diffuse', '225 225 215'),
                                           light.get('shadow', '120 120 135'), spec['par']))
    cam = spec.get('camera', {})
    eye = cam.get('eye', [tee[0], 45, tee[1] - 70])
    at = cam.get('at', [pins[0][0], 10, pins[0][1]])
    element('\t*Type  "CAMERA"\n\t*Class ""\n\t*Name  "DefaultCamera"\n\t*Pos %s\n\t*Dest %s\n\t*Fov 50.00\n\t*Bank 0.00\n'
            % (fmt(*eye), fmt(*at)))

    def point(name, x, z, y=0.0, cls='WPoint'):
        data = '\t*Data ""\n' if cls == 'WSun' else ''
        element('\t*Type  "POINT"\n\t*Class "%s"\n\t*Name  "%s"\n\t*Pos %s\n%s' % (cls, name, fmt(x, y, z), data))

    point(TEE, *tee)
    point(TEE2, *spec.get('tee2', tee))
    for group in (1, 2):
        for i in range(3):
            p = pins[min(i, len(pins) - 1)]
            point('%s%d_%d' % (PIN, group, i + 1), *p)
    point('그리드', pins[0][0] + 40, pins[0][1] + 8)
    point('태양', sun[0], sun[2], sun[1], 'WSun')
    point('광원1', sun[0], sun[2], sun[1], 'WGlobalLight')
    element('\t*Type  "PUPPET"\n\t*Class "WGround"\n\t*Filename "%s"\n\t*Pos 0.00 0.00 0.00\n\t*Rot 0.0 0.0 0.0\n'
            '\t*Scale 1.00\n\t*FlagShow 1\n\t*NoLOD 0\n\t*ForceOpaque 0\n\t*DiffLayer "normal"\n' % pet_name)

    def area(cls, name, box):
        element('\t*Type  "AREA"\n\t*Class "%s"\n\t*Name  "%s"\n\t*MinMax %s\t%s\n'
                % (cls, name, fmt(*box[:3]), fmt(*box[3:])))

    for zone in spec['zones']:
        if zone['type'] == 'water':
            level = zone.get('level', -2.0)
            c = zone['center']
            area('WLake', zone.get('script', '*type 0'),
                 [c[0] - zone['rx'] - 8, level - 16, c[1] - zone['rz'] - 8, c[0] + zone['rx'] + 8, level, c[1] + zone['rz'] + 8])
    for wind in spec.get('wind', []):
        area('WWindArea', wind['script'], wind['box'])
    for extra in spec.get('areas', []):
        area(extra.get('class', 'WEventArea'), extra['script'], extra['box'])

    margin = spec['terrain'].get('outline_margin', 40)
    hw, hd = terrain.w / 2.0 - margin, terrain.d / 2.0 - margin
    nodes = [('outline', [(-hw, -hd), (-hw, hd), (hw, hd), (hw, -hd)])]
    for i, poly in enumerate(spec.get('out_of_bounds', [])):
        nodes.append(('ib%d' % i, poly))
    nodes.append(('auto', spec.get('flyover') or [tee, pins[0]]))

    out = ['#WEP V1.20 for Pangya', '#Generated by tools/mapmaker.', '', '', '*RootDir ""', '',
           '*NumPuppet 1', '   0 : "%s"' % pet_name, '', '*NumVisGroup 0', '', '*NumNodeList %d' % len(nodes), '']
    for i, (name, pts) in enumerate(nodes):
        out += ['*Node %d' % i, '{', '\t*Name\t"%s"' % name, '\t*Type\t0', '\t*Num\t\t%d' % len(pts), '\t*Show\t1']
        out += ['\t*Npos\t%.3f 0.000 %.3f' % (p[0], p[1]) for p in pts]
        out += ['}', '']
    out += ['*NumElement %d' % len(elements), '', '']
    for i, body in enumerate(elements):
        out += ['*Element %d' % i, '{', body.rstrip('\n').replace('\r', ''), '}', '', '']
    with open(path, 'wb') as f:
        f.write('\n'.join(out).replace('\n', '\r\n').encode('cp949'))


# -- _property.xml, .sbin, pak -------------------------------------------------------------------------------------------

def class_xml(name, surface):
    tex, _, _, (bound, roll, lo, hi, pet, sound) = SURFACES[surface]
    return ('        <class name="%s" sort="mm">\n            <bound>%s</bound>\n            <roll>%s</roll>\n'
            '            <min>%s</min>\n            <max>%s</max>\n            <pet>%s</pet>\n'
            '            <bound_sound>%s</bound_sound>\n            <textures>\n                <item>%s</item>\n'
            '            </textures>\n        </class>\n' % (name, bound, roll, lo, hi, pet, sound, tex))


def write_property(path, course, base=None):
    classes = ''.join(class_xml('mm_' + s, s) for s in SURFACES)
    if base:
        # mantém as superfícies do curso original, assim os outros buracos continuam certos
        with open(base, 'rb') as f:
            xml = f.read().decode('cp949')
        count = int(re.search(r'<declare count="(\d+)"', xml).group(1))
        xml = xml.replace('<declare count="%d"' % count, '<declare count="%d"' % (count + len(SURFACES)), 1)
        xml = xml.replace('</declare>', classes + '    </declare>', 1)
    else:
        head = ('<?xml version="1.0" encoding="euc-kr" standalone="yes" ?>\n<round name="%s">\n    <declare count="%d">\n'
                '        <class name="default" sort="mm">\n            <bound>0.5</bound>\n            <roll>0.5</roll>\n'
                '            <min>90.0</min>\n            <max>100.0</max>\n            <min2>90</min2>\n'
                '            <max2>100</max2>\n            <min3>90</min3>\n            <max3>100</max3>\n'
                '            <pass>0</pass>\n            <bound_sound>충돌_rough2.wav</bound_sound>\n'
                '            <roll_sound>구름_rough.wav</roll_sound>\n            <pet>rought.pet</pet>\n'
                '        </class>\n' % (course, len(SURFACES) + 1))
        wep = ''.join('        <wep name="%s_%02d.wep">\n            <sunlight>1.0</sunlight>\n        </wep>\n' % (course, i)
                      for i in range(1, 19))
        xml = head + classes + '    </declare>\n    <project count="18">\n' + wep + '    </project>\n</round>\n'
    with open(path, 'wb') as f:
        f.write(xml.encode('cp949'))


def write_sbin(path):
    """.sbin sem ossos: substitui a sombra do buraco original, que não serve para esta malha."""
    with open(path, 'wb') as f:
        f.write(struct.pack('<II', 0xfffe0100, 0))


def write_pak(path, root, names):
    """Tudo guardado sem compressão: tipo 0x10 (nome e tamanho com XOR 0x71), método 0 (cFilePacked)."""
    with open(path, 'wb') as f:
        table = bytearray()
        for name in names:
            with open(os.path.join(root, name.replace('/', os.sep)), 'rb') as src:
                data = src.read()
            raw = name.encode('cp949')
            table += struct.pack('<BBIII', len(raw), 0x10, f.tell(), len(data), len(data) ^ 0x71)
            table += bytes(c ^ 0x71 for c in raw) + b'\0'
            f.write(data)
        where = f.tell()
        f.write(table)
        f.write(struct.pack('<IIB', where, len(names), 0x12))


def write_preview(path, terrain, spec, scale=1.0):
    """Vista de cima em PNG: superfície, relevo sombreado, tee (branco) e bandeiras (vermelho)."""
    w, h = int(terrain.w * scale), int(terrain.d * scale)
    rows = []
    for py in range(h):
        z = terrain.d / 2.0 - (py + 0.5) / scale
        row = bytearray(b'\0')
        for px in range(w):
            x = (px + 0.5) / scale - terrain.w / 2.0
            s = terrain.surface(x, z)
            c = SURFACES[s][1]
            slope = (terrain.height(x - 2, z) - terrain.height(x + 2, z) + terrain.height(x, z + 2) - terrain.height(x, z - 2)) * 0.1
            k = max(0.55, min(1.3, 1.0 + slope))
            if s == 'bed':
                c, k = (64, 128, 196), 1.0
            row += bytes(min(255, int(v * k)) for v in c)
            for mark, color, r in [(spec['tee'], (255, 255, 255), 7)] + [(p, (230, 40, 40), 4) for p in spec['pins']]:
                if math.hypot(x - mark[0], z - mark[1]) < r:
                    row[-3:] = bytes(color)
        rows.append(bytes(row))

    def png_chunk(tag, data):
        return struct.pack('>I', len(data)) + tag + data + struct.pack('>I', zlib.crc32(tag + data))
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n' + png_chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0)) +
                png_chunk(b'IDAT', zlib.compress(b''.join(rows), 6)) + png_chunk(b'IEND', b''))


# -- principal ---------------------------------------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('spec')
    ap.add_argument('-o', '--out', required=True)
    ap.add_argument('--base-property', help='_property.xml do curso, extraído da sua cópia: preserva os outros buracos')
    ap.add_argument('--no-compile', action='store_true', help='não chama o fwcompile (sem .gbin)')
    args = ap.parse_args()
    sys.stdout.reconfigure(encoding='utf-8')
    with open(args.spec, encoding='utf-8') as f:
        spec = json.load(f)

    course, hole, folder = spec['course'], spec['hole'], spec['course_dir']
    stem = '%s_%02d' % (course, hole)
    loose = os.path.join(args.out, 'loose')
    mapdir = '/'.join((folder, 'map'))
    textdir = '/'.join((folder, 'text'))
    for d in (mapdir, textdir):
        os.makedirs(os.path.join(loose, d.replace('/', os.sep)), exist_ok=True)

    terrain = Terrain(spec)
    verts, tris = terrain.build()
    pet = stem + '.pet'
    write_pet(os.path.join(loose, mapdir, pet), verts, tris, spec['terrain'].get('tile', 32.0))
    write_wep(os.path.join(loose, mapdir, stem + '.wep'), spec, pet, terrain)
    for key, (tex, base, alt, _) in SURFACES.items():
        write_dds(os.path.join(loose, mapdir, tex), tex, base, alt)
    write_property(os.path.join(loose, textdir, course + '_property.xml'), course, args.base_property)
    write_sbin(os.path.join(loose, mapdir, stem + '.sbin'))
    write_preview(os.path.join(args.out, 'preview.png'), terrain, spec)
    print('terreno: %d vertices, %d triangulos, %d x %d celulas' % (len(verts), len(tris), terrain.nx, terrain.nz))
    used = {}
    for _, s in tris:
        used[s] = used.get(s, 0) + 1
    print('superficies:', ' '.join('%s=%d' % (list(SURFACES)[k], v) for k, v in sorted(used.items())))

    names = ['%s/%s%s' % (mapdir, stem, e) for e in ('.pet', '.sbin')]
    names += ['%s/%s' % (mapdir, SURFACES[k][0]) for k in SURFACES]
    names.append('%s/%s_property.xml' % (textdir, course))
    if not args.no_compile:
        sbin = os.path.join(loose, mapdir, stem + '.sbin')
        with open(sbin, 'rb') as f:
            keep = f.read()
        r = subprocess.run([FWCOMPILE, os.path.join(loose, mapdir, stem + '.wep'), os.path.join(loose, mapdir, stem + '.gbin'),
                            '--root', loose, '--bake', '--check'], capture_output=True, text=True, encoding='utf-8')
        print(r.stdout)
        if r.returncode:
            sys.exit('fwcompile falhou')
        with open(sbin, 'wb') as f:
            f.write(keep)
        names.append('%s/%s.gbin' % (mapdir, stem))
        write_pak(os.path.join(args.out, PAK_NAME), loose, names)
        print('pak:', os.path.join(args.out, PAK_NAME), len(names), 'arquivos')


if __name__ == '__main__':
    main()
