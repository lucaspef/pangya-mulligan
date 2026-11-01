# FairwayED

A course hole editor for Pangya, inspired by WangED, the map editor Ntreev used on the WangReal Engine. It opens `.wep`
projects, shows a hole the way UnrealEd shows a level, and compiles it to the `.gbin` the client loads. Compiled holes have
never been loaded by the game client.

This is not Ntreev's code and has no connection to Ntreev. The original tool hasn't surfaced; only its project files
and outputs have. Everything here is worked out from those and from the client's loader, and the layout and workflow
are invented.

It's written the way it could have been around 2003: C++ close to C++98 with the STL, plain Win32 with classic common
controls, OpenGL 1.1 with display lists, and GDI+ for images. The UI is Korean or English, switchable at run time.
Puppets, textures and effects come from [wshared](wshared), shared with CaddieRig.

## Build

Needs a Visual C++ install with the x86 tools, and the `wshared` submodule. Clone with `--recurse-submodules`, or run
`git submodule update --init` in an existing clone.

```bash
build.bat
```

This produces `bin\FairwayED.exe` (the editor) and `bin\fairwayc.exe` (the batch verifier).

## Documentation

- [Usage](docs/usage.md)
- [Status](docs/status.md)
- [Roadmap](docs/roadmap.md)
- [File formats and compile rules](docs/file-formats.md)
- [Verification and open questions](docs/verification.md)
- [Source map](docs/source-map.md)

## License

MIT, see [LICENSE](LICENSE). That covers the work in this repository only. Parts of it follow a decompilation of the game
client, and no rights in Ntreev's code or assets are granted. No game data is included: bring your own `data` folder.
