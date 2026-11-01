# File formats and compile rules

## `.wep`

CP949 text with CR LF line ends, as WangED wrote it. The archive's copies have since been converted to UTF-8, and
one project has bare LF; both are detected and written back the way they came. Keys are `*Key` lines.

| Section | Holds |
| --- | --- |
| `*RootDir` | The artist's data folder; FairwayED resolves by file name instead |
| `*NumPuppet` | Each `.pet` used, one line per file |
| `*NumVisGroup` | Always 0 in the data. Any other value is refused, because the layout is unknown |
| `*Node n` | `Name`, `Type`, `Num`, `Show`, then `Npos` lines. V1.30 adds `GUID`, also before each `Npos` |
| `*Element n` | `Type` of `GAME`, `CAMERA`, `POINT`, `AREA` or `PUPPET`, with `Class` where it has one |

V1.30 adds `NoFog`, `NoAlphaSort` and `NoShadow` to puppets, and `GUID` to everything.

Classes seen: `WCamera`, `WPoint`, `WSun`, `WMoon`, `WGlobalLight`, `WSecondaryGlobalLight`, `WEventArea`, `WNPC`,
`WLake`, `WObject`, `WRotObj`, `WVtxObj`, `WGround`.

**Invented:** `WSoundArea`, `WFxArea`, `WWindArea` and `WGreenArea` for `.gbin` area types 5, 6, 4 and 7, `WArea<n>` for the
unnamed 3 and 9, and the `DiffLayer` values `diff1` and `diff2`. All shipped projects say `normal`.

## Project to `.gbin`

| `.gbin` field | Comes from |
| --- | --- |
| Point type | `WPoint` 0, `WGlobalLight` 1, `WSun` 2, `WMoon` 3. `WSecondaryGlobalLight` isn't written |
| Light option | GAME `Diffuse Ambient Ambient2` as `rrggbb` hex |
| Sun and moon option | `Data` |
| Area type | Class: event 0, NPC 1, lake 2, wind 4, sound 5, fx 6, green 7 |
| Model flag | `WRotObj` 2, `WVtxObj` 4, `ForceOpaque` 8, `NoLOD` 0x10 |
| Model order | Project order, whatever the detail, with the header counting each detail |
| Model matrix | `scale * Rx(-rx) * Ry(-ry) * Rz(rz)`, degrees, row vectors |
| `localAabb` | The model's vertices through that matrix |
| `sceneAabb` | Vertices in the model's bottom 10 units, boxed to that height (partial, see verification) |
| Base colours | Per corner, in mesh order |
| Hole info | Par and the x, z of the eight named points |

Mesh order comes from `WBone::SetMesh`: triangles split by texture and group in first-use order, and each new mesh goes
on the front of the chain, so the file holds the last mesh first. Vertices merge per `GetIndexedmesh`. Split this way,
vertex and index counts agree with the `.sbin` header for all 291 holes that have one.

Tee and pin points are stored with y = 0; the client reads only x and z. The editor stands them on the ground for display
and leaves the stored value alone.

## `_property.xml`

Read by `CPropertyInfo::Init` in `golfdoc.cpp`. A class has `bound`, `roll`, `min`/`max` (1 to 3), `pass`, `pet`, sounds
and `textures`; anything missing falls back on the class named `default`. `pet` chooses the ground type by substring, in
the order `PropGroundType` lists. `<project>` has per-hole `sunlight` and `desc`.

Saving an unedited file writes the original bytes. After an edit it's formatted again, which matches 20 of the 22 shipped
files; the other two have stray tabs.

## Area and node scripts

An area's name is its script, read with `WSliceStr`: find the key anywhere, then parse what follows. The client ignores
what it can't find, and Map Check reports the cases that matter. `src/areascript.cpp` decodes them, and the property
grid shows the result in `▸` rows under the raw `Name`.

| Type | Class in FairwayED | Script | Read by |
| --- | --- | --- | --- |
| 0 | `WEventArea` | `*extra`, `*extra_shortgame`: random tee box. `*cam name`: switches to an installed camera while the ball is inside | `LoadMapCheckData`, `GetAreaByPos`, `CGolfRule` |
| 1 | `WNPC` | `*type n *pet file *num n`, optional `*vel`, `*s` | `CNPC::SetNPC` |
| 2 | `WLake` | `*type n`, `*noripple` (forces type 1) | `CPool::SetArea` |
| 3, 9 | `WArea3`, `WArea9` | Type 9 has `*pow n`, type 3 nothing | Nothing in the decompiled client |
| 4 | `WWindArea` | `[*+-=]strength heading tilt [normal/hard/easy]` | `CWind::AddWindArea` |
| 5 | `WSoundArea` | `file distance interval` | `WPolySoup::Load` |
| 6 | `WFxArea` | `*fx file.spr rx ry rz`, radians, at the box centre | `WPolySoup::Load` |
| 7 | `WGreenArea` | None. In no shipped hole | `CQuadTree::ResetHoleData` |

- **Wind:** with no prefix or `+` the strength is added to the hole's wind, `*` multiplies it, `-` subtracts it and `=`
  replaces it. The client's "pitch" field is really the heading: the direction is `(RotMat(yaw, x) * RotMat(pitch, y)).za`.
  Omitting the difficulty means `hard`, which is live only on DiffFlag 2.
- **`silviacannon.seq`:** the area doesn't play it. `CWind` does when its special wind fires, using the Y rotation as the
  wind direction, so FairwayED doesn't play it either.
- **Typos that ship:** `*.extra` on `moon_01`-style holes and `* type 0` on lakes both silently do nothing in the client.
- **Node names** are scripts too: `npc_santa *r 5`, `npc_walex03 *vel 3` and so on choose a creature in `CNPC::SetNPC(node)`.

**Invented:** `WWindArea` and `WGreenArea`, like the other class names above that no project shows. Types 4 and 7 never
appear in a leaked `.wep`.

## Companions

`<course>_fog.txt` is three lines: colour, near and far, and two more values not used here. Sky textures are
`<course>_far`, `_near`, `_up` and `_dn`, drawn with `CSky::LoadSky`'s geometry around the camera.

## The fade around a course

The base model's outer corners have alpha 0 in the `.gbin` colours (a few percent of them, RGB near white). The course
blends out through them to whatever is behind, which is the sky's `_dn` plane 196 units below the camera, so a course's
ground colour and `_dn` texture have to agree for the edge to disappear. The editor's Light Base Model keeps these alphas.
