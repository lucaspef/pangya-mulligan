# Using CaddieRig

## Command line

```bash
bin\CaddieRig.exe D:\ProjectG_JP_Data\data\caddie\pipin\pipin_ex.pet /root D:\ProjectG_JP_Data\data
```

`/root` is the folder searched for textures, also under Options. Other switches: `/lang ko|en`,
`/motion name`, `/frame n`, `/attach part.mpet`, `/wear part.mpet`, `/nowardrobe`, `/skeleton`, `/boxes`, `/wire`, `/flat`,
`/play`, `/mute`, `/partfx`.

## The tool

- **Starting a project:** open an `.ase`. You get one texture block per bitmap, a box per mesh node, and a
  `center` box, which is the shape the archived projects have.
- **Editing:** the tree shows what is in the puppet. Double-click a property to edit the PMP block behind it.
  The change recompiles that chunk straight away and the viewport updates.
- **Texture flags:** the `[ + ] { ~ } @ ! 2#` filename prefixes appear as toggles on each texture.
- **Motions and scripts:** Edit menu imports an `.Ani` and places it on the timeline, or adds a script at the
  current frame. During playback, scripts print to the log and their sounds and effects run.
- **Characters:** opening a `.bpet` also loads its `.apet` and the `<x>_*_def.mpet` parts. Parts bind to bones by name.
  The Wardrobe (F9) dresses the character in other parts.
- **Output:** File > Export picks the kind from the extension; Options picks the version. Validate (F8)
  checks the limits the client's loader relies on.
