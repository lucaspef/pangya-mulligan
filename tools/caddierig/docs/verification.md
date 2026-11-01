# Verification and open questions

## What was verified

Run against `D:\ProjectG_JP_Data\data` and `Paechijanjae`.

| Check | Result |
| --- | --- |
| Load and re-save every puppet (800.5 MB) | 9,049 of 9,049 byte-identical |
| Strict parse of the same files | Only the 4 damaged `.mpet` tails left over; 3 bone-less `.apet` files have tracks with no bones to index |
| Open and pose every puppet at 4 times | 9,049 opened, no crashes, no NaN vertices |
| Load and re-save every PMP | 7 of 7 byte-identical; all survive a CP949 round trip |
| Compile `ball_hw` and `west_rank` from ASE + PMP against the shipped `.pet` | Bones, vertices, indices, UVs, normals, triangle textures, boxes, names, flags, diffuse all match. Only `group` differs |
| Compile `ball_karen` as v1.3 (hand-written PMP with no boxes) | Same match for what was compiled; the shipped boxes aren't in that PMP |
| Compile `1등모션.ani` against shipped `g_def.apet` | Spine matches at 0.00° on the frames sampled. Some leg frames differ, up to 13° across the file (see below) |
| Run every `.spr` and `.seq` for 3 seconds, headless (`fxcheck`) | 2,001 of 2,002 start; the other has a mojibake file name a script would not use. 0 NaN particles |
| Parse every shipped script | 13,341 messages, 18,576 commands; 17 have unknown tokens, typos such as `*hideclub*hideclub` and `*pte(`, plus `*sfx` |
| Resolve script references | `*snd` 1,999 found, 35 missing; `*fx` 4,786 found, 15 missing; box `*fx` 839 found, 2 missing. Sampled misses aren't in the dump |
| Decode every `.wav` (`audiocheck`) | 6,861 of 6,862; `lucia_radio_event.wav` has no readable format chunk. One test voice of each format ran to completion at inaudible volume |
| Resolve every `*ptex` (`ptexcheck`) | 8,453 uses, all with a target; all 799 distinct texture and image pairs load with a valid rectangle |
| Script visuals in the app | Checked by screenshot: Pipin's blink, her fade, Cien's `*hide`, the `e_bomb` prop hidden |
| Wear every part of every character (`wardrobecheck`) | 14 characters, 3,422 slot parts and 2,955 extras: all worn and posed, 0 NaN vertices. Checked for loading and posing, not for looking right |
| Normals against triangle winding, `avatar` folder (`normalcheck`) | 99.84% of 4 million triangles agree, at rest and mid motion. The 29 default faces look along -Z |
| Effects in the app | Checked by screenshot: dust on Pipin, aura on Cien, snow in `ball_h`, halo on Arin. 25 seconds of looping playback stayed at 80 MB |
| GUI, driven by window messages | Tree select, in-place edit, flag toggle, save, validate, add box, shading, playback, Korean and English, new project from ASE |

Reproduce with:

```bash
bin\crigc.exe verify D:\ProjectG_JP_Data\data
```

```bash
bin\crigc.exe posetest D:\ProjectG_JP_Data\data
```

```bash
bin\crigc.exe verifypmp D:\Projects\retreev\Paechijanjae\data
```

`build`, `compare`, `anicheck` and `metacheck` cover the rest; run `crigc` with no arguments for the usage.

## What is imagined or unverified

- **The tool itself:** the layout and workflow are invented. No screenshot of PuppetMasterG was available. Its name and
  `PMPF VER 2.21` come from your documentation and the PMP files.
- **`group` numbering:** I assign 2, 4, 6... Shipped values follow the same even pattern from a different start
  (6, 22-30, 78), which looks like a counter kept across a session. That's a guess, and nothing here reproduces it.
- **Skin weights from ASE:** no surviving ASE contains any, so meshes compile rigidly, one bone per node.
  Skinned puppets open, pose and re-save fine, but you can't author weights.
- **Character compiles:** `i_def.ASE` and its siblings didn't survive, so a full character compile has never been
  compared. Opening those PMPs gives metadata only. The PMPs also predate the shipped files: most motions sit at
  different frames in the shipped `.apet`, and only 2-6 per character still match, so they can't be compared frame for frame.
- **Leg differences:** the leg mismatches return to exact at later frames, so they aren't accumulated error. A
  later revision of those motions would explain them, but I can't confirm it.
- **Not exercised:** compiled files have never been loaded by the game client. The GUI's Import Motion and
  Export commands weren't driven through their file dialogs. The code behind Export's compile path was tested
  through `crigc`; the merge behind Import Motion was tested through `anicheck`.

Limits specific to one area are in [PuppetScript](puppetscript.md), [Wardrobe](wardrobe.md), [Rendering](rendering.md)
and [File formats](file-formats.md).
