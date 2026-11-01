# Rendering

## Blending and lighting

The client's D3D device code isn't in the source, so blending is an equivalent, not a copy: solid texels write depth and
soft texels blend over them, where the client depth-sorts. `2#` two-sided lighting isn't applied.

The viewport has a single fixed light, not the game's. It uses the direction from `CPetBody::SetLight`'s fallback, shining
along `(0, -1, 1)`, and the default course brightness from `polysoup.cpp`: ambient `0x808080`, diffuse white. Characters look
along -Z, so the light falls on their fronts. A course's own sun is in its map, which the tool doesn't read.

## Specular maps

The first texture with `!` in its first three characters is the puppet's specular map, as in `WPuppet::LoadPET`, and every
mesh gets it (`WBone::ApplySpecularMap`). The shipped maps are `!specular_ellipse_sm/md/lg`, `!gold`, `!silver` and `!copper`.
They are brightest at UV (0, 0) and black at 0.75.

Texture coordinates come from the half angle between light and camera, per corner and per frame (`WBone::CalcHilightCoord`).
The result is added to the lit texture in a second texture stage, like draw flag `0x40000`, so highlights follow the camera.

- `SMTL` plays no part. In the data, 482 of the 629 files with a map have no `SMTL` chunk, the other 147 have an empty one, and
  no file has both a map and `SMTL` entries.
- Highlights need Lighting on and Textured shading, and `ARB_multitexture`. They won't match a course's sun.
- The `.fx` shaders named in `SMTL` entries aren't implemented.
