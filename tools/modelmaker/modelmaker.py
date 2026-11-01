#!/usr/bin/env python3
"""Cria modelos do PangYa a partir de formatos abertos (só biblioteca padrão).

  modelmaker.py build entrada.gltf -o saida.pet [--texture t.jpg] [--scale 1] [--version 1.1] [--box-margin 0]
  modelmaker.py build peca.gltf -o peca.mpet --skeleton g_def.bpet
  modelmaker.py ase entrada.gltf -o pasta          ASE + PMP + Ani para o crigc (só malha rígida)
  modelmaker.py pak saida.pak caminho/no/pak=arquivo [...]
  modelmaker.py info arquivo.pet

Coordenadas: glTF e OBJ são destros com Y para cima; o jogo é canhoto, então (x, y, z) -> (x, y, -z) e os cantos
de cada triângulo invertem. Matrizes do jogo são de vetor-linha (v' = v * M), gravadas por linha.
"""
import argparse, base64, json, math, os, struct, sys

IDENTITY = [[1.0, 0, 0], [0, 1.0, 0], [0, 0, 1.0], [0, 0, 0]]


def mul(a, b):
    """Produto de matrizes 4x3 de vetor-linha: aplica a, depois b."""
    out = []
    for i in range(4):
        w = 1.0 if i == 3 else 0.0
        out.append([a[i][0] * b[0][k] + a[i][1] * b[1][k] + a[i][2] * b[2][k] + w * b[3][k] for k in range(3)])
    return out


def inverse(m):
    r = m
    a, b, c = r[0], r[1], r[2]
    det = a[0] * (b[1] * c[2] - b[2] * c[1]) - a[1] * (b[0] * c[2] - b[2] * c[0]) + a[2] * (b[0] * c[1] - b[1] * c[0])
    if abs(det) < 1e-12:
        raise ValueError("matriz singular")
    inv = [[(b[1] * c[2] - b[2] * c[1]) / det, (a[2] * c[1] - a[1] * c[2]) / det, (a[1] * b[2] - a[2] * b[1]) / det],
           [(b[2] * c[0] - b[0] * c[2]) / det, (a[0] * c[2] - a[2] * c[0]) / det, (a[2] * b[0] - a[0] * b[2]) / det],
           [(b[0] * c[1] - b[1] * c[0]) / det, (a[1] * c[0] - a[0] * c[1]) / det, (a[0] * b[1] - a[1] * b[0]) / det]]
    t = m[3]
    inv.append([-(t[0] * inv[0][k] + t[1] * inv[1][k] + t[2] * inv[2][k]) for k in range(3)])
    return inv


def point(p, m):
    return [p[0] * m[0][k] + p[1] * m[1][k] + p[2] * m[2][k] + m[3][k] for k in range(3)]


def direction(p, m):
    return [p[0] * m[0][k] + p[1] * m[1][k] + p[2] * m[2][k] for k in range(3)]


def normalize(v):
    n = math.sqrt(sum(x * x for x in v))
    return [x / n for x in v] if n > 1e-9 else [0.0, 1.0, 0.0]


def trs_to_game(t, q, s, scale):
    """TRS do glTF (vetor-coluna, destro) para 4x3 do jogo (vetor-linha, canhoto)."""
    x, y, z, w = q
    r = [[1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
         [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
         [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)]]
    sign = [1, 1, -1]
    # espelha Z dos dois lados e transpõe (coluna -> linha); a escala fica em cada linha
    rows = [[r[c][l] * sign[c] * sign[l] * s[l] for c in range(3)] for l in range(3)]
    rows.append([t[0] * scale, t[1] * scale, -t[2] * scale])
    return rows


def matrix_to_quat(m):
    """Inverso de WMatrix::SetRotation (mesma fórmula do compilador do CaddieRig)."""
    xx, yx, zx = m[0]
    xy, yy, zy = m[1]
    xz, yz, zz = m[2]
    tr = xx + yy + zz
    if tr > 0:
        s = math.sqrt(tr + 1) * 2
        q = ((yz - zy) / s, (zx - xz) / s, (xy - yx) / s, 0.25 * s)
    elif xx > yy and xx > zz:
        s = math.sqrt(1 + xx - yy - zz) * 2
        q = (0.25 * s, (yx + xy) / s, (zx + xz) / s, (yz - zy) / s)
    elif yy > zz:
        s = math.sqrt(1 + yy - xx - zz) * 2
        q = ((yx + xy) / s, 0.25 * s, (zy + yz) / s, (zx - xz) / s)
    else:
        s = math.sqrt(1 + zz - xx - yy) * 2
        q = ((zx + xz) / s, (zy + yz) / s, 0.25 * s, (xy - yx) / s)
    return normalize(q)


class Model:
    def __init__(self):
        self.bones = []      # {name, parent, local, world}
        self.verts = []      # {pos (jogo, mundo), blends [(osso, peso 0-255)]}
        self.tris = []       # 3 cantos (índice, normal, u, v)
        self.anims = {}      # osso -> {'pos': [(t, xyz)], 'rot': [(t, xyzw)]}
        self.motions = []    # (nome, quadro inicial, quadro final)
        self.texture = None


# ------------------------------------------------------------------ glTF


def read_gltf(path, scale):
    doc = json.load(open(path, encoding="utf8"))
    base = os.path.dirname(os.path.abspath(path))
    blobs = []
    for b in doc.get("buffers", []):
        uri = b["uri"]
        blobs.append(base64.b64decode(uri.split(",", 1)[1]) if uri.startswith("data:")
                     else open(os.path.join(base, uri), "rb").read())
    formats = {5120: "b", 5121: "B", 5122: "h", 5123: "H", 5125: "I", 5126: "f"}
    counts = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4, "MAT4": 16}

    def accessor(i):
        a = doc["accessors"][i]
        v = doc["bufferViews"][a["bufferView"]]
        fmt, n = formats[a["componentType"]], counts[a["type"]]
        size = struct.calcsize("<" + fmt) * n
        stride, off, blob = v.get("byteStride", size), v.get("byteOffset", 0) + a.get("byteOffset", 0), blobs[v["buffer"]]
        out = [struct.unpack_from("<%d%s" % (n, fmt), blob, off + k * stride) for k in range(a["count"])]
        return [x[0] for x in out] if n == 1 else out

    m = Model()
    nodes = doc["nodes"]
    for i, n in enumerate(nodes):
        for c in n.get("children", []):
            nodes[c]["_parent"] = i
    meshes = [(i, n) for i, n in enumerate(nodes) if "mesh" in n]
    if not meshes:
        raise SystemExit("glTF sem malha")
    skin = doc["skins"][meshes[0][1]["skin"]] if "skin" in meshes[0][1] else None

    def trs(n):
        return (n.get("translation", (0, 0, 0)), n.get("rotation", (0, 0, 0, 1)), n.get("scale", (1, 1, 1)))

    def world(i):
        n = nodes[i]
        local = trs_to_game(*trs(n), scale)
        return mul(local, world(n["_parent"])) if "_parent" in n else local

    index = {}
    joints = list(skin["joints"]) if skin else [meshes[0][0]]
    # pai antes de filho: o cliente resolve o pai pelo que já leu
    joints.sort(key=lambda j: _depth(nodes, j))
    for j in joints:
        n = nodes[j]
        index[j] = len(m.bones)
        parent = index.get(n.get("_parent"), -1)
        w = world(j)
        local = w if parent < 0 else mul(w, inverse(m.bones[parent]["world"]))
        m.bones.append({"name": n.get("name") or "Bone%d" % j, "parent": parent, "local": local, "world": w})

    for node_id, n in meshes:
        node_world = world(node_id) if not skin else IDENTITY
        for prim in doc["meshes"][n["mesh"]]["primitives"]:
            attr = prim["attributes"]
            pos = accessor(attr["POSITION"])
            nor = accessor(attr["NORMAL"]) if "NORMAL" in attr else None
            uv = accessor(attr["TEXCOORD_0"]) if "TEXCOORD_0" in attr else [(0.0, 0.0)] * len(pos)
            idx = accessor(prim["indices"]) if "indices" in prim else list(range(len(pos)))
            jnt = accessor(attr["JOINTS_0"]) if "JOINTS_0" in attr else None
            wgt = accessor(attr["WEIGHTS_0"]) if "WEIGHTS_0" in attr else None
            start = len(m.verts)
            for k, p in enumerate(pos):
                game = point([p[0] * scale, p[1] * scale, -p[2] * scale], node_world)
                if jnt:
                    mapped = [(index[skin["joints"][jnt[k][c]]], wgt[k][c]) for c in range(4) if wgt[k][c] > 0]
                else:
                    mapped = [(index.get(node_id, 0), 1.0)]
                m.verts.append({"pos": game, "blends": quantize(mapped)})
            for t in range(0, len(idx), 3):
                corners = []
                for c in (idx[t], idx[t + 2], idx[t + 1]):      # espelhar Z inverte o sentido
                    nv = nor[c] if nor else None
                    normal = direction([nv[0], nv[1], -nv[2]], node_world) if nv else None
                    corners.append((start + c, normalize(normal) if normal else [0.0, 0.0, 0.0], uv[c][0], uv[c][1]))
                m.tris.append(corners)

    images = doc.get("images", [])
    if images and "uri" in images[0] and not images[0]["uri"].startswith("data:"):
        m.texture = os.path.join(base, images[0]["uri"])
    for anim in doc.get("animations", []):
        _read_anim(doc, anim, accessor, nodes, index, m, scale, trs)
    fill_tracks(m)
    return m


def _depth(nodes, i):
    d = 0
    while "_parent" in nodes[i]:
        i, d = nodes[i]["_parent"], d + 1
    return d


def _read_anim(doc, anim, accessor, nodes, index, m, scale, trs):
    start = max((f[1] for f in m.motions), default=-1) + 1
    length = 0.0
    tracks = {}
    for ch in anim["channels"]:
        node, path = ch["target"]["node"], ch["target"]["path"]
        if node not in index or path not in ("translation", "rotation"):
            continue
        s = anim["samplers"][ch["sampler"]]
        times, values = accessor(s["input"]), accessor(s["output"])
        tracks.setdefault(node, {})[path] = (times, values)
        length = max(length, times[-1])
    frames = int(round(length * 30))
    for node, tr in tracks.items():
        t0, r0, s0 = trs(nodes[node])
        a = m.anims.setdefault(index[node], {"pos": [], "rot": []})
        for key, path in (("pos", "translation"), ("rot", "rotation")):
            times, values = tr.get(path, ([0.0, length], None))
            for k, t in enumerate(times):
                tv = values[k] if values and path == "translation" else t0
                rv = values[k] if values and path == "rotation" else r0
                g = trs_to_game(tv, rv, s0, scale)
                time = start / 30.0 + t
                a[key].append((time, g[3]) if key == "pos" else (time, matrix_to_quat(g)))
    m.motions.append((anim.get("name") or "motion%d" % len(m.motions), start, start + frames))


def fill_tracks(m):
    """Todo osso precisa de trilha: sem chaves o Animate983 zera a pose e o boneco colapsa (compara com os arquivos do jogo)."""
    if not m.motions:
        return
    start, end = m.motions[0][1] / 30.0, m.motions[-1][2] / 30.0
    for i, b in enumerate(m.bones):
        a = m.anims.setdefault(i, {"pos": [], "rot": []})
        if not a["pos"]:
            a["pos"] = [(start, b["local"][3]), (end, b["local"][3])]
        if not a["rot"]:
            q = matrix_to_quat(b["local"])
            a["rot"] = [(start, q), (end, q)]


def quantize(blends):
    """Pesos para bytes que somam 255 (o cliente para de ler quando a soma chega a 255), maior primeiro."""
    blends = sorted(blends, key=lambda b: -b[1])
    total = sum(w for _, w in blends)
    out, left = [], 255
    for i, (b, w) in enumerate(blends):
        v = left if i == len(blends) - 1 else min(left, int(round(255 * w / total)))
        if v > 0:
            out.append((b, v))
            left -= v
    if left:
        out[0] = (out[0][0], out[0][1] + left)
    return out


# ------------------------------------------------------------------ OBJ


def read_obj(path, scale, bone="Object01"):
    pos, uvs, nors = [], [], []
    m = Model()
    m.bones.append({"name": bone, "parent": -1, "local": IDENTITY, "world": IDENTITY})
    seen = {}
    mtl = None
    base = os.path.dirname(os.path.abspath(path))

    def corner(token):
        parts = (token.split("/") + ["", ""])[:3]
        p, t, n = int(parts[0]), int(parts[1] or 0), int(parts[2] or 0)
        p = p - 1 if p > 0 else len(pos) + p
        key = p
        if key not in seen:
            seen[key] = len(m.verts)
            x, y, z = pos[p]
            m.verts.append({"pos": [x * scale, y * scale, -z * scale], "blends": [(0, 255)]})
        uv = uvs[t - 1 if t > 0 else len(uvs) + t] if t else (0.0, 0.0)
        nv = nors[n - 1 if n > 0 else len(nors) + n] if n else None
        return (seen[key], normalize([nv[0], nv[1], -nv[2]]) if nv else [0.0, 0.0, 0.0], uv[0], 1.0 - uv[1])

    for line in open(path, encoding="utf8", errors="replace"):
        w = line.split()
        if not w:
            continue
        if w[0] == "v":
            pos.append(tuple(map(float, w[1:4])))
        elif w[0] == "vt":
            uvs.append(tuple(map(float, w[1:3])))
        elif w[0] == "vn":
            nors.append(tuple(map(float, w[1:4])))
        elif w[0] == "f":
            c = [corner(t) for t in w[1:]]
            for i in range(1, len(c) - 1):
                m.tris.append([c[0], c[i + 1], c[i]])
        elif w[0] == "mtllib":
            mtl = os.path.join(base, " ".join(w[1:]))
    if mtl and os.path.exists(mtl):
        for line in open(mtl, encoding="utf8", errors="replace"):
            w = line.split()
            if w and w[0].lower() == "map_kd":
                m.texture = os.path.join(base, " ".join(w[1:]))
                break
    return m


# ------------------------------------------------------------------ .pet / .mpet


def chunk(tag, body):
    return tag + struct.pack("<I", len(body)) + bytes(body)


def write_id(i):
    if i < 0:
        return b"\xff"
    return bytes([i]) if i < 0xfe else b"\xfe" + struct.pack("<H", i)


def fit_box(m, margin):
    lo = [min(v["pos"][k] for v in m.verts) - margin for k in range(3)]
    hi = [max(v["pos"][k] for v in m.verts) + margin for k in range(3)]
    return lo + hi


def coll_entry(name, bone, box):
    out = struct.pack("<2i", 1, 1)
    for text in (name, bone):
        raw = text.encode("cp949")
        out += struct.pack("<I", len(raw)) + raw
    out += struct.pack("<I", 1) + b"\0" + struct.pack("<I", 0)  # script0 "@" = 1 NUL; script1 "@" = vazio, como o jogo
    return out + struct.pack("<6f", *box)


def read_bones(path):
    """Esqueleto de um .bpet/.pet da SUA cópia do jogo: nomes e matrizes (nada disso vai para o git)."""
    b = open(path, "rb").read()
    ver, o, body = (0, 0), 0, None
    while o < len(b):
        tag, n = b[o:o + 4], struct.unpack_from("<I", b, o + 4)[0]
        if tag == b"VERS":
            ver = (b[o + 9], b[o + 8])
        if tag == b"BONE":
            body = b[o + 8:o + 8 + n]
        o += 8 + n
    if body is None:
        raise SystemExit("sem BONE em " + path)
    q, count = 1, body[0]
    if count == 0:
        count = struct.unpack_from("<H", body, 1)[0]
        q = 3
    bones = []
    for _ in range(count):
        e = body.index(0, q)
        name = body[q:e].decode("cp949")
        q = e + 1
        parent = body[q]
        q += 1
        if parent == 0xff:
            parent = -1
        elif parent == 0xfe:
            parent = struct.unpack_from("<H", body, q)[0]
            q += 2
        vals = struct.unpack_from("<12f", body, q)
        q += 48 + (4 if ver >= (1, 3) else 0)
        local = [list(vals[0:3]), list(vals[3:6]), list(vals[6:9]), list(vals[9:12])]
        world = local if parent < 0 else mul(local, bones[parent]["world"])
        bones.append({"name": name, "parent": parent, "local": local, "world": world})
    return bones


def rebind(m, skeleton, part):
    """Reaponta a malha para os ossos do esqueleto do personagem, pelo nome (é assim que a peça encaixa no cliente)."""
    names = {b["name"].lower(): i for i, b in enumerate(skeleton)}
    table = {}
    for i, b in enumerate(m.bones):
        if b["name"].lower() not in names:
            raise SystemExit("osso %r não existe no esqueleto" % b["name"])
        table[i] = names[b["name"].lower()]
    for v in m.verts:
        v["blends"] = [(table[b], w) for b, w in v["blends"]]
    # só os ossos usados e os pais deles: o índice do osso-malha é um byte, e os esqueletos passam de 255
    keep = set()
    for v in m.verts:
        for b, _ in v["blends"]:
            while b >= 0:
                keep.add(b)
                b = skeleton[b]["parent"]
    order = sorted(keep)
    remap = {old: new for new, old in enumerate(order)}
    remap[-1] = -1
    skeleton = [dict(skeleton[i], parent=remap[skeleton[i]["parent"]]) for i in order]
    for v in m.verts:
        v["blends"] = [(remap[b], w) for b, w in v["blends"]]
    # osso-malha: raiz própria no fim da lista, é por ele que o cliente anexa a peça (WPuppet::FindMeshBone)
    n = len(m.verts)
    pivot = [sum(v["pos"][k] for v in m.verts) / n for k in range(3)]
    local = [[1.0, 0, 0], [0, 1.0, 0], [0, 0, 1.0], pivot]
    m.bones = skeleton + [{"name": part.upper(), "parent": -1, "local": local, "world": local}]
    m.anims, m.motions = {}, []


def write_pet(m, path, kind, version, texture, margin):
    major, minor = version
    tex = texture.encode("cp949")
    if len(tex) > 31:
        raise SystemExit("nome de textura com mais de 31 bytes: " + texture)
    out = b""
    if version != (1, 0):
        out += chunk(b"VERS", bytes([minor, major, 0xfe, 0xff]))
    # flag 1 = sem atlas; grupo e handle seguem o compilador original (128 + índice)
    out += chunk(b"TEXT", struct.pack("<I", 1) + tex.ljust(32, b"\0") + bytes([1, 2, 0, 0]) +
                 struct.pack("<Ii", 0xffffffff, 128))
    if version >= (1, 2):
        out += chunk(b"SMTL", struct.pack("<I", 0))

    n = len(m.bones)
    body = bytearray(bytes([n]) if n < 256 else b"\0" + struct.pack("<H", n))
    for b in m.bones:
        body += b["name"].encode("cp949") + b"\0" + write_id(b["parent"])
        for row in b["local"]:
            body += struct.pack("<3f", *row)
        if version >= (1, 3):
            body += struct.pack("<f", -1.0)
    out += chunk(b"BONE", body)

    if kind == "pet":
        body = bytearray()
        end = max((k[0] for a in m.anims.values() for k in a["pos"] + a["rot"]), default=0.0)
        for bone in sorted(m.anims):
            a = m.anims[bone]
            body += write_id(bone) + struct.pack("<I", len(a["pos"]))
            for t, p in a["pos"]:
                body += struct.pack("<4f", t, *p)
            body += struct.pack("<I", len(a["rot"]))
            for t, q in a["rot"]:
                body += struct.pack("<5f", t, *q)
            if version >= (1, 1):
                # duas chaves, como no jogo: com uma só o cliente divide por zero depois de t=0 e o osso some
                body += struct.pack("<I", 2) + struct.pack("<8f", 0, 1, 1, 1, end, 1, 1, 1)
            else:
                body += struct.pack("<I", 0)
            if version >= (1, 3):
                body += struct.pack("<I", 0)
        out += chunk(b"ANIM", body + b"\xff")

    body = bytearray()
    if kind == "mpet":
        body += bytes([1]) + struct.pack("<B3x4i", len(m.bones) - 1, 0, len(m.verts), 0, len(m.tris))
    body += struct.pack("<I", len(m.verts))
    worlds = [inverse(b["world"]) for b in m.bones]
    for v in m.verts:
        local = point(v["pos"], worlds[v["blends"][0][0]])
        body += struct.pack("<3f", *local)
        if kind == "mpet":
            body += struct.pack("<f", 1.0)
        for b, w in v["blends"]:
            body += bytes([w]) + write_id(b)
        if len(v["blends"]) == 1:
            body += b"\0\0"
    body += struct.pack("<I", len(m.tris))
    for tri in m.tris:
        for idx, normal, u, v in tri:
            body += struct.pack("<I3f", idx, *normal)
            if version >= (1, 2):
                body += b"\1"
            body += struct.pack("<2f", u, v)
    body += bytes(len(m.tris))
    if version >= (1, 2):
        body += b"\xff" * len(m.tris)      # camada de material: 0xff = nenhuma; 0 derruba o cliente com SMTL vazio
    out += chunk(b"MESH", body)

    name = os.path.splitext(texture)[0].encode("cp949")[:31]
    out += chunk(b"FANM", struct.pack("<I", 1) + b"\2" + name.ljust(32, b"\0") + tex.ljust(32, b"\0"))
    if kind == "pet":
        out += chunk(b"FRAM", struct.pack("<I", 0))
        body = bytearray(struct.pack("<I", len(m.motions)))
        for name_, a, b in m.motions:
            raw = name_.encode("cp949")
            body += struct.pack("<I", len(raw)) + raw + struct.pack("<2I", a, b)
            body += struct.pack("<I", 0)                 # próximo movimento
            body += struct.pack("<I", 10) + b"Transition" + struct.pack("<f", 0.0)
            body += struct.pack("<I", 0)                 # osso de aplicação
        out += chunk(b"MOTI", body)
        root = m.bones[0]["name"]
        box = fit_box(m, margin)
        out += chunk(b"COLL", struct.pack("<I", 2) + coll_entry(root, root, box) + coll_entry("center", root, box))
    with open(path, "wb") as f:
        f.write(out)
    return len(out)


# ------------------------------------------------------------------ ASE + PMP + Ani (rota do crigc)


def game_to_max(p):
    k = 1.0 / 0.32
    return [p[0] * k, p[2] * k, p[1] * k]


def write_ase(m, directory, name, texture):
    """Uma cena no formato que o CaddieRig compila. Pesos não passam por ASE: cada vértice fica no osso principal."""
    os.makedirs(directory, exist_ok=True)
    L = ["*3DSMAX_ASCIIEXPORT\t200", "*SCENE {", "\t*SCENE_FIRSTFRAME 0", "\t*SCENE_LASTFRAME 100",
         "\t*SCENE_FRAMESPEED 30", "\t*SCENE_TICKSPERFRAME 160", "}", "*MATERIAL_LIST {", "\t*MATERIAL_COUNT 1",
         "\t*MATERIAL 0 {", '\t\t*MATERIAL_NAME "m"', "\t\t*MATERIAL_DIFFUSE 1.0\t1.0\t1.0", "\t\t*MAP_DIFFUSE {",
         '\t\t\t*BITMAP "%s"' % texture, "\t\t}", "\t}", "}"]
    for i, b in enumerate(m.bones):
        w = b["world"]
        vs = [k for k, v in enumerate(m.verts) if v["blends"][0][0] == i]
        tag = "*GEOMOBJECT" if vs else "*HELPEROBJECT"
        L += [tag + " {", '\t*NODE_NAME "%s"' % b["name"]]
        if b["parent"] >= 0:
            L.append('\t*NODE_PARENT "%s"' % m.bones[b["parent"]]["name"])
        rows = [game_to_max(w[0]), game_to_max(w[2]), game_to_max(w[1]), game_to_max(w[3])]
        L.append("\t*NODE_TM {")
        for r, row in enumerate(rows[:3]):
            row = [x * 0.32 for x in row]
            L.append("\t\t*TM_ROW%d %f\t%f\t%f" % (r, *row))
        L += ["\t\t*TM_ROW3 %f\t%f\t%f" % tuple(rows[3]), "\t}"]
        if vs:
            remap = {v: k for k, v in enumerate(vs)}
            tris = [t for t in m.tris if t[0][0] in remap and t[1][0] in remap and t[2][0] in remap]
            L += ["\t*MESH {", "\t\t*MESH_NUMVERTEX %d" % len(vs), "\t\t*MESH_NUMFACES %d" % len(tris),
                  "\t\t*MESH_VERTEX_LIST {"]
            for k, v in enumerate(vs):
                L.append("\t\t\t*MESH_VERTEX %d\t%f\t%f\t%f" % (k, *game_to_max(m.verts[v]["pos"])))
            L += ["\t\t}", "\t\t*MESH_FACE_LIST {"]
            for k, t in enumerate(tris):
                # o compilador grava A C B; devolve a ordem de origem
                a, c, b_ = t[0], t[1], t[2]
                L.append("\t\t\t*MESH_FACE %d: A: %d B: %d C: %d *MESH_SMOOTHING 1 *MESH_MTLID 0" %
                         (k, remap[a[0]], remap[b_[0]], remap[c[0]]))
            L += ["\t\t}", "\t\t*MESH_NUMTVERTEX %d" % (len(tris) * 3), "\t\t*MESH_TVERTLIST {"]
            for k, t in enumerate(tris):
                for c, corner in enumerate((t[0], t[2], t[1])):
                    L.append("\t\t\t*MESH_TVERT %d\t%f\t%f\t0.0" % (k * 3 + c, corner[2], 1.0 - corner[3]))
            L += ["\t\t}", "\t\t*MESH_NUMTVFACES %d" % len(tris), "\t\t*MESH_TFACELIST {"]
            for k in range(len(tris)):
                L.append("\t\t\t*MESH_TFACE %d\t%d\t%d\t%d" % (k, k * 3, k * 3 + 1, k * 3 + 2))
            L += ["\t\t}", "\t\t*MESH_NORMALS {"]
            for k, t in enumerate(tris):
                L.append("\t\t\t*MESH_FACENORMAL %d\t0\t0\t1" % k)
                for corner in (t[0], t[2], t[1]):
                    n = corner[1]
                    L.append("\t\t\t*MESH_VERTEXNORMAL %d\t%f\t%f\t%f" % (corner[0], n[0], n[2], n[1]))
            L += ["\t\t}", "\t}", "\t*MATERIAL_REF 0"]
        L.append("}")
    open(os.path.join(directory, name + ".ASE"), "w", encoding="ascii", newline="\r\n").write("\n".join(L) + "\n")

    box = fit_box(m, 0.0)
    root = m.bones[0]["name"]
    motion = ""
    pmp = ["PMPF VER 2.21", "", "ASE 이름 : %s.ASE" % name, "", "텍스쳐( void ) ", "{", "\t부모텍스쳐 : @",
           "\t텍스쳐이름 : %s" % os.path.splitext(texture)[0], "\t텍스쳐파일 : %s" % texture, "\t텍스쳐연결 : 0", "}", ""]
    for box_name in (root, "center"):
        pmp += ["충돌박스( void ) ", "{", "\t모양(o,ㅁ) : 1", "\t보이기     : 1", "\t박스이름   : %s" % box_name,
                "\t부모본이름 : %s" % root, "\t스크립트0  : @", "\t스크립트1  : @"]
        for label, v in zip(("최소값x", "최소값y", "최소값z", "최대값x", "최대값y", "최대값z"), box):
            pmp.append("\t%s    : %f" % (label, v))
        pmp += ["}", ""]
    if m.motions:
        motion_name, a, b_ = m.motions[0]
        pmp += ["동작( void ) ", "{", "\t동작이름 : %s" % motion_name, "\t다음동작 : ", "\t연결방법 : Transition",
                "\t연결시간 : 0.0000", "\t최상위본 : ", "\t원본시작프레임 : %d" % a, "\t원본종료프레임 : %d" % b_,
                "\t원본파일 : %s.Ani" % name, "\t시작프레임 : %d" % a, "\t종료프레임 : %d" % b_, "}", ""]
        write_ani(m, os.path.join(directory, name + ".Ani"))
    open(os.path.join(directory, name + ".pmp"), "w", encoding="utf8", newline="\r\n").write("\n".join(pmp))


def write_ani(m, path):
    L = ["*3DSMAX_ASCIIEXPORT\t200", "*SCENE {", "\t*SCENE_TICKSPERFRAME 160", "}"]
    for i, b in enumerate(m.bones):
        w = b["world"]
        L += ["*HELPEROBJECT {", '\t*NODE_NAME "%s"' % b["name"]]
        if b["parent"] >= 0:
            L.append('\t*NODE_PARENT "%s"' % m.bones[b["parent"]]["name"])
        rows = [game_to_max(w[0]), game_to_max(w[2]), game_to_max(w[1])]
        L.append("\t*NODE_TM {")
        for r, row in enumerate(rows):
            L.append("\t\t*TM_ROW%d %f\t%f\t%f" % (r, *[x * 0.32 for x in row]))
        L += ["\t\t*TM_ROW3 %f\t%f\t%f" % tuple(game_to_max(w[3])), "\t}"]
        a = m.anims.get(i)
        if a:
            L.append("\t*TM_ANIMATION {")
            prev = None
            L.append("\t\t*CONTROL_ROT_TRACK {")
            for t, q in a["rot"]:
                # primeira amostra absoluta, as outras delta (q = anterior * delta), eixo (-x, -z, -y) no jogo
                d = q if prev is None else quat_mul(quat_conj(prev), q)
                prev = q
                s = math.sqrt(max(0.0, 1.0 - min(1.0, d[3] * d[3])))
                angle = 2 * math.atan2(s, d[3])
                axis = (-d[0] / s, -d[2] / s, -d[1] / s) if s > 1e-6 else (1.0, 0.0, 0.0)
                L.append("\t\t\t*CONTROL_ROT_SAMPLE %d\t%f\t%f\t%f\t%f" % (int(round(t * 30)) * 160, *axis, angle))
            L.append("\t\t}")
            L.append("\t}")
        L.append("}")
    open(path, "w", encoding="ascii", newline="\r\n").write("\n".join(L) + "\n")


def quat_mul(a, b):
    ax, ay, az, aw = a
    bx, by, bz, bw = b
    return (aw * bx + ax * bw + ay * bz - az * by, aw * by - ax * bz + ay * bw + az * bx,
            aw * bz + ax * by - ay * bx + az * bw, aw * bw - ax * bx - ay * by - az * bz)


def quat_conj(q):
    return (-q[0], -q[1], -q[2], q[3])


# ------------------------------------------------------------------ pak e info


def write_pak(path, entries):
    """Pak do JP: dados crus + índice com nomes em XOR 0x71 (tipo 0x10, sem compressão), como OpenPAKFILE2 lê."""
    blob, index = bytearray(), bytearray()
    for name, source in entries:
        data = open(source, "rb").read()
        raw = name.encode("cp949")
        index += bytes([len(raw), 0x10]) + struct.pack("<III", len(blob), len(data), len(data) ^ 0x71)
        index += bytes(c ^ 0x71 for c in raw) + b"\0"
        blob += data
    with open(path, "wb") as f:
        f.write(blob + index + struct.pack("<II", len(blob), len(entries)) + b"\x12")


def info(path):
    b = open(path, "rb").read()
    o = 0
    while o < len(b):
        tag, n = b[o:o + 4], struct.unpack_from("<I", b, o + 4)[0]
        print(tag.decode("latin1"), n)
        o += 8 + n


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    for name in ("build", "ase"):
        p = sub.add_parser(name)
        p.add_argument("input")
        p.add_argument("-o", "--output", required=True)
        p.add_argument("--texture")
        p.add_argument("--scale", type=float, default=1.0)
        p.add_argument("--version", default="1.1")
        p.add_argument("--box-margin", type=float, default=0.0)
        p.add_argument("--skeleton")
        p.add_argument("--bone", default="Object01")
    p = sub.add_parser("pak")
    p.add_argument("output")
    p.add_argument("files", nargs="+")
    p = sub.add_parser("info")
    p.add_argument("file")
    a = ap.parse_args()
    if a.cmd == "pak":
        write_pak(a.output, [tuple(f.split("=", 1)) for f in a.files])
        return
    if a.cmd == "info":
        info(a.file)
        return
    m = read_gltf(a.input, a.scale) if a.input.lower().endswith(".gltf") else read_obj(a.input, a.scale, a.bone)
    texture = (a.texture or (os.path.basename(m.texture) if m.texture else "null")).lower()
    if a.cmd == "ase":
        write_ase(m, a.output, os.path.splitext(os.path.basename(a.input))[0], texture)
        return
    kind = os.path.splitext(a.output)[1].lstrip(".").lower()
    if kind not in ("pet", "mpet"):
        raise SystemExit("saída deve ser .pet ou .mpet")
    if a.skeleton:
        rebind(m, read_bones(a.skeleton), os.path.splitext(os.path.basename(a.output))[0])
    elif kind == "mpet":
        raise SystemExit(".mpet precisa de --skeleton: a peça usa o esqueleto do personagem")
    major, minor = (int(x) for x in a.version.split("."))
    size = write_pet(m, a.output, kind, (major, minor), texture, a.box_margin)
    print("%s: %d bytes, %d ossos, %d vértices, %d triângulos, %d movimentos, textura %s" %
          (a.output, size, len(m.bones), len(m.verts), len(m.tris), len(m.motions), texture))


if __name__ == "__main__":
    main()
