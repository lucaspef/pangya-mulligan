# Roadmap

What FairwayED doesn't do yet, in no particular order.

- **Shadow map baking:** it draws and carries shadow maps but cannot bake them. An exported `.sbin` is the original, or
  empty after Build > Clear Shadow Maps, so moved objects keep their old shadows.
- **`.gbin` version 0x72:** it writes version 0x71 only. Opening a 0x72 hole loses its object hierarchy on export.
- **`.sgbin` and `.aibin` editing:** these can be parsed but not edited in the UI.
- **Puppet animation:** Puppets show their rest pose.
- **More effects:** effects only play from `WFxArea`, not from a Puppet's own `*fx` boxes. They are silent and ignore wind.
- **NPCs:** they're shown as a still model rather than simulated.
- **The rest of the sky:** it has the client's planes and cylinders, but no clouds, stars or moon.
