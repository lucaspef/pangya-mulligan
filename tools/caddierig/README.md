# CaddieRig

A tool for authoring Pangya's Puppet models, inspired by PuppetMasterG, the tool Ntreev used for them. It takes 3ds Max
exports and a `.pmp` project and compiles them into `.pet`, `.mpet`, `.bpet` and `.apet` files. It also opens
finished puppets for inspection and editing, with animation, scripts, effects, sound and outfits. Compiled files have never
been loaded by the game client.

This is not Ntreev's code and has no connection to Ntreev. The original tool hasn't surfaced; only its
inputs and outputs have. Everything here is worked out from those files and from the client's loader.

It's written the way it could have been around 2003: C++ close to C++98 with the STL, plain Win32 with classic
common controls, OpenGL 1.1 immediate mode, and GDI+ for images. The UI is Korean or English, switchable at run time, and
new projects save as CP949. Puppets, textures and effects come from [wshared](wshared), shared with FairwayED.

## Build

Needs a Visual C++ install with the x86 tools, and the `wshared` submodule. Clone with `--recurse-submodules`, or run
`git submodule update --init` in an existing clone. `build.bat` prefers the 2022 build tools; set `VCVARS` to override.

```bash
build.bat
```

This produces `bin\CaddieRig.exe` (the application) and `bin\crigc.exe` (the batch compiler and test tool).

## Documentation

- [Usage](docs/usage.md)
- [Status](docs/status.md)
- [Roadmap](docs/roadmap.md)
- [File formats and compiler rules](docs/file-formats.md), including corrections to the wiki and ImHex pattern
- [PuppetScript, sounds and effects](docs/puppetscript.md)
- [Wardrobe](docs/wardrobe.md)
- [Rendering](docs/rendering.md): blending, lighting and specular maps
- [Verification and open questions](docs/verification.md)
- [Source map](docs/source-map.md)

## License

MIT, see [LICENSE](LICENSE). That covers the work in this repository only. Parts of it follow a decompilation of the game
client, and no rights in Ntreev's code or assets are granted. No game data is included: bring your own `data` folder.
