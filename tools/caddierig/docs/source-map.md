# Source map

| File | Purpose |
| --- | --- |
| `src/pmpfile.*` | PMPF text format; keeps unread fields, encoding and line endings |
| `src/asefile.*` | ASE and ANI parser |
| `src/compile.*` | The compiler, PET to PMP recovery, validation, comparison |
| `src/document.*` | Open project, mesh parts, posing and skinning |
| `src/puppetscript.*` | PuppetScript tokenizer, `*fx` and box script parsing |
| `src/audio.*` | Multi-voice `.wav` playback through waveOut |
| `src/wardrobe.*` | Part name rules and outfit state |
| `src/view.*` | OpenGL viewport |
| `src/main.cpp` | Application window, tree, properties, timeline |
| `src/crigc.cpp` | Batch compiler and regression commands |

From [wshared](../wshared), shared with FairwayED: `petfile.*` (Puppet chunk reader and writer; keeps every
byte, including leaked padding), `texture.*` (JPG, PNG, BMP, TGA and DXT DDS loading), `fxsim.*` (sequence and spray
simulation, with no renderer), `fxdraw.*` (draws its sprites), `wcommon.*` (encodings, paths, the data folder index) and
`wmath.h`.
