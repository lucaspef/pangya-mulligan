# Source map

| File | Purpose |
| --- | --- |
| `src/wep.*` | `.wep` reader and writer; keeps each field's text until it's edited |
| `src/gbin.*` | `.gbin`, `.sgbin`, `.aibin` and `.sbin`; keeps every byte, including leaked padding |
| `src/props.*` | `_property.xml` reader and writer |
| `src/course.*` | The open hole: compile rules, import, map check, lighting bake, ray casts, undo |
| `src/areascript.*` | What area and node name scripts mean to the client |
| `src/areafx.cpp` | Plays `WFxArea` effects through `fxsim` and `fxdraw` |
| `src/scene.*` | Fixed-function drawing, shadow map overlay, sky, script overlays, markers |
| `src/viewport.cpp` | The four panes, picking, dragging, tools, preview |
| `src/panels.cpp` | Scene tree, property grid, puppet browser, surface table, hole sheet |
| `src/main.cpp` | Application window, menus, console, tool buttons |
| `src/fairwayc.cpp` | Batch verifier |

From [wshared](../wshared), shared with CaddieRig: `petfile.*` (Puppets), `texture.*` (images), `fxsim.*`
and `fxdraw.*` (effects), `wcommon.*` (encodings, paths, the data folder index) and `wmath.h`.
