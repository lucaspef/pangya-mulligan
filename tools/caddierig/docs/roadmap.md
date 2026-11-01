# Roadmap

What CaddieRig doesn't do yet, in no particular order. The other documents have the details.

- **Skin weights:** no surviving ASE contains any, so meshes compile rigidly, one bone per node. Skinned puppets open, pose
  and re-save fine, but you can't author weights.
- **`EXTR` chunks:** never written when compiling, because the table isn't understood.
- **Rendering:** `2#` two-sided lighting and the `.fx` shaders named in `SMTL` entries aren't applied, and a course's own sun
  isn't read. The viewport has one fixed light.
- **Script commands that need the game:** shots, swings and caddie movement are only logged, and `*hideclub` and `*showclub` act
  on a club model the tool doesn't load.
- **Effects:**
  - The ground is the y = 0 plane, not a course.
  - `*pet` particles hold their rest pose and ignore fade alpha.
  - The flash is a plain white overlay.
- **Wardrobe:**
  - Part-conditional effects don't follow what's worn.
  - The rules are inferred from file names, not item data, so some parts double up or leave a gap.
