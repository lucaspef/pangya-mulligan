#!/usr/bin/env python3
"""Gera os exemplos do zero (geometria e textura nossas, nada do jogo): bola_mulligan e boneco_mulligan, em glTF + PNG."""
import base64, json, math, os, struct, sys
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))


def pack(arrays):
    """arrays: (dados planos, formato struct, tipo glTF, componente, alvo). Devolve buffer, views e accessors."""
    blob, views, accessors = bytearray(), [], []
    for data, fmt, kind, comp, target, n in arrays:
        while len(blob) % 4:
            blob.append(0)
        raw = struct.pack("<%d%s" % (len(data), fmt), *data)
        view = {"buffer": 0, "byteOffset": len(blob), "byteLength": len(raw)}
        if target:
            view["target"] = target
        views.append(view)
        acc = {"bufferView": len(views) - 1, "componentType": comp, "count": len(data) // n, "type": kind}
        if kind == "VEC3" and fmt == "f":
            acc["min"] = [min(data[i::3]) for i in range(3)]
            acc["max"] = [max(data[i::3]) for i in range(3)]
        accessors.append(acc)
        blob += raw
    return blob, views, accessors


def sphere(radius, rings, segs, cy=0.0, sy=1.0):
    pos, nor, uv, idx = [], [], [], []
    for r in range(rings + 1):
        v = r / rings
        phi = v * math.pi
        for s in range(segs + 1):
            u = s / segs
            th = u * 2 * math.pi
            n = [math.sin(phi) * math.cos(th), math.cos(phi), math.sin(phi) * math.sin(th)]
            pos += [n[0] * radius, n[1] * radius * sy + cy, n[2] * radius]
            nor += n
            uv += [u, v]
    for r in range(rings):
        for s in range(segs):
            a, b = r * (segs + 1) + s, (r + 1) * (segs + 1) + s
            idx += [a, a + 1, b, b, a + 1, b + 1]
    return pos, nor, uv, idx


def doc_base(blob, views, accessors, nodes, meshes, image, extra=None):
    doc = {"asset": {"version": "2.0", "generator": "pangya-mulligan gerar.py"}, "scene": 0,
           "scenes": [{"nodes": [0]}], "nodes": nodes, "meshes": meshes, "accessors": accessors, "bufferViews": views,
           "buffers": [{"byteLength": len(blob),
                        "uri": "data:application/octet-stream;base64," + base64.b64encode(bytes(blob)).decode()}],
           "materials": [{"name": "m", "pbrMetallicRoughness": {"baseColorTexture": {"index": 0}}}],
           "textures": [{"source": 0}], "images": [{"uri": image}]}
    doc.update(extra or {})
    return doc


def ball():
    img = Image.new("RGB", (256, 128), (250, 245, 225))
    d = ImageDraw.Draw(img)
    for x in range(0, 256, 32):
        d.rectangle([x, 0, x + 15, 127], fill=(230, 40, 60))
    d.rectangle([0, 56, 255, 72], fill=(30, 90, 220))
    img.save(os.path.join(HERE, "bola_mulligan.png"))
    pos, nor, uv, idx = sphere(0.155, 12, 18)
    blob, views, acc = pack([(pos, "f", "VEC3", 5126, 34962, 3), (nor, "f", "VEC3", 5126, 34962, 3),
                             (uv, "f", "VEC2", 5126, 34962, 2), (idx, "H", "SCALAR", 5123, 34963, 1)])
    nodes = [{"name": "Ball_Mulligan", "mesh": 0}]
    meshes = [{"primitives": [{"attributes": {"POSITION": 0, "NORMAL": 1, "TEXCOORD_0": 2}, "indices": 3, "material": 0}]}]
    json.dump(doc_base(blob, views, acc, nodes, meshes, "bola_mulligan.png"),
              open(os.path.join(HERE, "bola_mulligan.gltf"), "w"))


def rigged():
    """Um boneco de 3 juntas com nomes do esqueleto dos personagens: Bip01 > Bip01 Spine > Bip01 Head."""
    img = Image.new("RGB", (128, 128), (60, 170, 110))
    d = ImageDraw.Draw(img)
    for y in range(0, 128, 16):
        d.rectangle([0, y, 127, y + 7], fill=(245, 200, 60))
    img.save(os.path.join(HERE, "boneco_mulligan.png"))
    pos, nor, uv, idx = sphere(0.35, 16, 16, cy=1.0, sy=2.8)
    joints, weights = [], []
    for i in range(1, len(pos), 3):
        y = pos[i]
        spine = max(0.0, min(1.0, (y - 0.3) / 0.5))
        head = max(0.0, min(1.0, (y - 1.1) / 0.5))
        w = [(1 - spine), spine * (1 - head), spine * head, 0.0]
        joints += [0, 1, 2, 0]
        weights += w
    times = [i / 10.0 for i in range(11)]
    rot = []
    for t in times:
        a = 0.5 * math.sin(t * 2 * math.pi) * 0.5
        rot += [0.0, 0.0, math.sin(a), math.cos(a)]
    ibm = []
    for y in (0.0, 0.6, 1.3):
        ibm += [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, -y, 0, 1]
    blob, views, acc = pack([(pos, "f", "VEC3", 5126, 34962, 3), (nor, "f", "VEC3", 5126, 34962, 3),
                             (uv, "f", "VEC2", 5126, 34962, 2), (idx, "H", "SCALAR", 5123, 34963, 1),
                             (joints, "B", "VEC4", 5121, 34962, 4), (weights, "f", "VEC4", 5126, 34962, 4),
                             (ibm, "f", "MAT4", 5126, 0, 16), (times, "f", "SCALAR", 5126, 0, 1),
                             (rot, "f", "VEC4", 5126, 0, 4)])
    nodes = [{"name": "Bip01", "children": [1, 3]}, {"name": "Bip01 Spine", "translation": [0, 0.6, 0], "children": [2]},
             {"name": "Bip01 Head", "translation": [0, 0.7, 0]}, {"name": "Corpo", "mesh": 0, "skin": 0}]
    meshes = [{"primitives": [{"attributes": {"POSITION": 0, "NORMAL": 1, "TEXCOORD_0": 2, "JOINTS_0": 4,
                                              "WEIGHTS_0": 5}, "indices": 3, "material": 0}]}]
    extra = {"skins": [{"joints": [0, 1, 2], "inverseBindMatrices": 6}],
             "animations": [{"name": "stand", "samplers": [{"input": 7, "output": 8, "interpolation": "LINEAR"}],
                             "channels": [{"sampler": 0, "target": {"node": 1, "path": "rotation"}}]}]}
    json.dump(doc_base(blob, views, acc, nodes, meshes, "boneco_mulligan.png", extra),
              open(os.path.join(HERE, "boneco_mulligan.gltf"), "w"))


if __name__ == "__main__":
    ball()
    rigged()
