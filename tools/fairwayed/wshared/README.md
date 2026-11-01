# wshared

The C++ code FairwayED and CaddieRig share: a small static library in the same style as they are, close to C++98 with the
STL, plain Win32, OpenGL 1.1 and GDI+. It exists for these two projects and isn't meant as a general library.

```bash
build.bat
```

This produces `lib\wshared.lib`. Both projects include this repository as a submodule, and their `build.bat` run it for you.
Add `src` to the include path. Users of the library also link `user32`, `gdi32`, `gdiplus`,
`ole32`, `opengl32` and `glu32`.

| File | Purpose |
| --- | --- |
| `src/wcommon.*` | CP949, UTF-8 and UTF-16 conversion, UTF-8 file access, paths, data folder index |
| `src/petfile.*` | Puppet (`.pet`, `.mpet`, `.bpet`, `.apet`) reader and writer; keeps every byte |
| `src/texture.*` | JPG, PNG, BMP, TGA and DXT DDS loading into OpenGL |
| `src/fxsim.*` | Sequence and spray simulation, with no renderer |
| `src/fxdraw.*` | Draws `fxsim`'s sprite particles in OpenGL |
| `src/wmath.h` | Vectors, matrices and quaternions in the engine's conventions |

`fxsim` knows a `*pet` particle's model only as an opaque `FxPetModel*`. A host's `LoadPetModel` returns whatever it draws
with, and casts it back when drawing those sprays.

## License

MIT, see [LICENSE](LICENSE). That covers the work in this repository only. `fxsim` and parts of the other files were worked
out from a decompilation of the game client, and no rights in Ntreev's code or assets are granted. This project has no
connection to Ntreev.
