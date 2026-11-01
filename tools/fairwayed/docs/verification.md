# Verification and open questions

All of these run from `fairwayc.exe` against the Japanese data set and the Paechijanjae archive.

## Byte-identical round trips

`fairwayc roundtrip <dir>` loads and re-saves without any editing.

| Files | Count | Identical |
| --- | --- | --- |
| `.gbin` | 341 | 341 |
| `.sgbin` | 634 | 634 |
| `.aibin` | 361 | 361 |
| `.sbin` | 340 | 340 |
| `.wep` | 79 | 79 |
| `_property.xml`, formatted | 22 | 20 |

The other two property files have stray tabs. The editor saves their original bytes until they're edited.

## A `.gbin` through a project and back

`fairwayc imports <data>` reconstructs a project from each version 0x71 `.gbin`, compiles it, and compares fields with the
original. 292 holes:

| Field | Matching |
| --- | --- |
| Cameras, points, areas, nodes (and their counts) | 100% |
| Model name, matrix, detail, script | 36,122 of 36,122 |
| Model face count | 36,106 of 36,122 |
| Model flag | 36,096 of 36,122 |
| Model `localAabb` | 35,316 of 36,122 (97.8%) |
| Model `sceneAabb` | 20,422 of 36,122 (56.5%) |
| Base model faces, colours, par | 100% |
| Base model `localAabb` | 289 of 292 |
| Hole info | 275 of 292 |
| Texture table, same set | 261 of 292 |
| Texture table, same order | 3 of 292 |

Also from that run: the mesh split gives the `.sbin` header's counts for all 291 holes checked, and all 340 holes with
usable colours have one colour per mesh vertex, which is what shows the colour order is right.

## Leaked projects against what shipped

`fairwayc projects <wep> <data>` compiles the 57 leaked 0x71 projects with a matching `.gbin`. Lower numbers are expected
here because many holes were edited after the project leaked: a `DefaultCamera` moved, objects nudged, a `*extra` area added.
Fields a project alone decides and nobody revised are exact: model name, flag, detail, script and face count match on all
6,303 models where the model count agrees.

## Effects and scripts

`fairwayc effects <data>` runs every shipped effect area's file through `fxsim` headless for 400 frames, and every area script
through the decoder, using the same host as the editor. All 541 areas across 23 distinct files attach and produce particles.

## The editor itself

Driven through real window messages on `sand_06.wep`: drag-move, drag-rotate and drag-scale in the Top pane, duplicate, drop,
placing a camera, point, area and puppet, adding a node list and its points, par, delete, undo, save, export and bake. The
saved project and both exported holes were re-parsed and inspected. Screenshots covered all four tabs, textured and surface
shading, the Korean UI, and a hole opened directly from a `.gbin`.

## Not verified

- **No compiled hole has been loaded by the game client.** Matching the loader's reads and the shipped data is the
  strongest evidence available here.
- Hand use with a real mouse and keyboard. The 3D-pane fly controls and in-place property editing weren't exercised by the
  message driver.
- The puppet browser's turntable preview: the tab and list were seen, a selection wasn't made.

## Open questions

- **`sceneAabb`:** always 10 tall or the whole model, and the rule used is exact on just over half. The client only hands it
  to `CSceneManager::RegisterElement`, so a near miss should affect culling at most.
- **Texture table order:** the base model's textures come first; nothing found explains the rest. The client looks entries
  up by name, so order shouldn't matter. Sets differ where a texture is unused by triangles or a puppet was rebuilt later.
- **Base colours:** Light Base Model is a plausible first-order bake (ambient, sun by angle, shadow colour), not a recovered
  algorithm. Shipped alpha-0 corners are kept; what decides them isn't known.
- **`WSecondaryGlobalLight`** never reaches a `.gbin`, so what WangED used it for is a guess. **`Type`** on nodes is 0 in every
  file, and **`*NumVisGroup`** is always 0.
