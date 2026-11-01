# File formats and compiler rules

## The pipeline

```
Max scene ─ ASCII export ─> X.ASE (rig, meshes, materials)
          └ per motion   ─> Y.Ani (same format, TM_ANIMATION tracks only)
X.pmp (textures, boxes, motions, scripts) ── compile ──> X.pet
```

## Corrections for the wiki and ImHex pattern

All found by making the parser account for every byte of the shipped data, rather than resyncing at
each chunk end as the pattern does.

| Item | Wiki / pattern today | What the data shows |
| --- | --- | --- |
| `SkinInfo` | ints first, `ucBoneID` last | `ucBoneID`, 3 padding bytes, then the 4 ints (matches `w_skin_info`) |
| `Animation` after rotations | 4 padding bytes in v1.0, scale list from v1.2 | A scale count and list in **every** version; v1.1 files really use it |
| `TriPoint` from v1.2 | no extra field | A `u8` before the UVs, nearly always 1. It repeats the UV pair: 42 files have a value other than 1 |
| `MESH` from v1.2 | ends after `textureIndex` | A second `u8` per triangle follows, nonzero in 3,146 files. Meaning unknown |
| `VERS` | 2 bytes | 4 bytes; the last two are `FE FF` |
| `SMTL` | not documented | Sits after `TEXT` from v1.2. Mostly a zero count. 51 files have entries, kept as raw bytes: the ones sampled name a shader such as `tex1_light_mtlclrani.fx` with colour parameters. The rebang client never reads this chunk |
| Bone padding in v1.3 | 4 padding bytes | Reads as the float `-1.0` in the bones checked; meaning unknown |
| `BONE` count | `0` then `u16` | Some empty puppets stop after the single `0` byte |
| Empty COLL scripts | not stated | `@` in the PMP compiles to one NUL byte in `script0` and to an empty string in `script1` |

## Quirks in the shipped data

- Four `.mpet` files have a wrong `MESH` size, so the chunk walk hits garbage: `e_ha_24`, `g_ha_a_z27_!ha`,
  `g_ha_a_z29_!ha` and `h_ts_46_!pv`.
- PMPF 2.2 files use `CR CR LF` line endings, a text-mode double write.

## Compiler rules recovered from the data

- **Space:** Max `(x, y, z)` becomes game `(x, z, y) × 0.32`. Rotation rows and columns swap the same way.
- **Triangles:** corners go out as A, C, B. `v` becomes `1 - v`. Normals are rotated into bone space.
- **Rotation keys:** ANI samples are axis-angle deltas. The first is absolute, and each later one is
  `q = q * delta`, with the axis written as `(-x, -z, -y)`.
- **Key reduction:** position keys a linear interpolation would reproduce are dropped.
- **Motions:** clips are laid end to end on one timeline at 30 fps, usually one frame apart.
- **Materials:** `*SUBMATERIAL` numbers are sparse, so faces index by number modulo `NUMSUBMTLS`. Diffuse is
  truncated to bytes, not rounded, so 0.5882 becomes `0x95`.
- **Texture masks:** `[` and `+` textures take alpha from a grey `<name>_mask<ext>` beside them (`CheckMaskTexture`).
- **Texture link:** `텍스쳐연결 : 0` compiles to `flag = 1`, meaning no atlas packing.

## Compiled defaults

`SMTL` is always empty, the second triangle byte is 0, the UV count is 1, scale keys are constant, and `EXTR` is never
written because its table isn't understood.
