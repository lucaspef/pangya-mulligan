# Using FairwayED

## Command line

```bash
bin\FairwayED.exe "D:\ProjectG_JP_Data\data\round08_shining sand\map\sand_06.gbin" /root D:\ProjectG_JP_Data\data
```

`/root` is the game's `data` folder, searched by file name for puppets and textures, and remembered from
File > Game Data Folder. Other switches: `/lang ko|en`, `/exec "cmd;cmd"` runs console commands, and
`/shot out.png` saves the window and exits.

## The editor

- **Opening:** a `.wep`, or a `.gbin`, from which a project is reconstructed. The compiled hole, its `.sbin`, the
  course's `_property.xml` and `_fog.txt` are found by name and loaded with it.
- **Viewports:** Top, 3D, Front and Side. Double-click a title to maximize; keys `1`-`4` set wire, flat, textured or
  surface shading on the active pane. Wheel zooms. In ortho panes the right or middle button pans. In the 3D pane hold
  the right button to look, with `WASD` `QE` to fly (`Shift` is faster); the middle button pans.
- **Tools (left bar):** select and move, rotate (`Ctrl` for X, `Shift` for Z), scale, then Add Puppet, Point, Camera,
  Area and Edit Nodes. Clicks place on the base model. `G` snaps to the grid, `H` keeps things on the ground while moving.
  `Shift` in the 3D pane moves vertically. Drag a camera's far diamond to aim it. `Alt`-click selects the base model.
- **Scene tab:** every element and node list, with a property grid. Double-click a value to edit it; colours open a
  colour picker and flags toggle. An edit goes to everything selected that has that key.
- **Puppets tab:** every `.pet` below the data folder, filtered as you type, on a turntable. Double-click to place.
- **Surfaces tab:** the `_property.xml` classes, editable by double-click, and every texture the hole uses with its
  class. Shade `4` colours the course by class, and selecting a class highlights it.
- **Hole tab:** the tee and pin slots, the per-hole `sunlight` and `desc`, and statistics.
- **Build menu:** Map Check (`F8`) tests what the loader relies on. Light Base Model bakes vertex colours from the GAME
  lights with shadow rays. Export (`F7`) writes the `.gbin` and `.sbin`.
- **Areas:** an area's name is its script. Selecting one shows what the client makes of it under the `Name` row, and
  Script Overlays (View menu) draws it: wind arrows, the water sheet, links to `*cam` cameras, tee boxes, NPC models and
  sound reach. Red diamonds mark scripts with a problem.
- **Effects:** `WFxArea` sequences and sprays play in the 3D pane. Toggle them with View > Area Effects. They follow an
  area as it's moved, and restart when its file changes or from View > Restart Effects.
- **View:** `C` looks through the selected camera, `F5` flies along the `auto` node at ball height.
- **Console:** the line under the log; type `help`. `Ctrl+Z`, `Ctrl+Y` undo and redo up to 64 steps.
