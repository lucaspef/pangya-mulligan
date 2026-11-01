# PuppetScript, sounds and effects

Ported from `petbody.cpp`, `golfplayer.cpp`, `fxinterpreter.cpp`, `fxsequence.cpp`, `fxspray.cpp` and `fxbox.cpp`.

## How scripts run

- **Motion scripts:** playback fires `FRAM` messages using `CPetBody::ScriptProcess`'s frame range and tokenizer.
  `*snd("name")` plays `name.wav`. `*fx("@motion" "file" "&part" "bone"...)` attaches a `.spr` or `.seq` to up to
  four bones or collision boxes, with their positions averaged.
- **Collision box scripts:** `*fx("a.seq")` and `*fx a.seq` in a box's script slots start when the model opens and
  run continuously at the box centre. `*fx("*playbbfx")` restarts them.
- **Effects:** sequences and sprays run the client's particle model: generation, fades, sizes, sprite animation,
  gravity points, bubbles, sprite and ribbon tails, spawns, revolutions, quake and flash, and `*pet` model
  particles. Blend modes come from `wd3d8.cpp`. Effects keep running while the timeline is paused.
- **Controls (Script menu):** toggle sounds, effects and box effects. F5 fires the selected script or box, and F6
  clears motion effects. `&id` effects only run for a worn part, and the tool doesn't track that for effects, so they
  stay off until you enable the part-conditional option (`/partfx`).
- **Finding files:** assets are looked up by name across the search folder, as the client's match directory does.
  With no folder set, the tool uses the `data` folder above the open file. Korean names work even when they're
  on disk as misdecoded CP949 (`pipin-±âº»¹ß.wav`).

## Sound volume

A `sound` line with a range fades with distance from the camera (`CSoundManager::LoadSfx`). The tool scales a copy of each
voice's samples, because `waveOutSetVolume` sets the level of the whole program and Windows saves it per executable. The first
sound of each run sets the program's level to full, which also overrides any level set for it in the Windows volume mixer.
Two ADPCM `.wav` files can't be scaled this way, and always play at full volume.

## Commands that change how the model looks

Run from `golfplayer.cpp`, `petbody.cpp` and `npc.cpp`:

- **`*ptex`:** face animation. The image is full-size, and pure green means "unchanged". The rest is pasted into the
  live texture, with alpha from a `_mask` for `[` textures. Caddies find the target through `FANM`. Avatars always use
  their face texture: here the texture with "face" in its name on any worn part, or else the first texture of an `_fc_`
  part. Later patches accumulate, as in the client.
- **`*hidebone`, `*showbone`, `*hidevis`, `*showvis`:** hide meshes by owning bone, with groups from `avatar/visgroup.def`.
- **`*hide`, `*show`, `*opa n` (percent):** caddies and props only. **`*opa("from to frames")`:** fades on avatars.
- **`*playif("part;part" "motion")`:** switches motion if a named part is attached.
- **`*fxstop`:** stops the named effect. No handler exists in the source, so this is inferred from its arguments.

Seeking replays these from the motion's start, so the model looks right at any frame. Visibility and alpha reset at each
motion start, as `CGolfPlayer::SetMotion` does.

## Commands that are only logged

These need a running game or don't exist in the source:

- `*fly`, `*shot`, `*swing`, `*tail`, `*lockclub`, `*give`, `*ring`, and the `*stop_dist`, `*start_delay`, `*pet_speed`, `*ball_dist`
  and `*grip_pos` parameters drive shots and caddie movement.
- `*hideclub`, `*showclub` act on a club model the tool doesn't load.
- `*lookup`, `*dontlookup` have a keyword entry in `petbody.cpp` but no handler.
- `*petbillboard`, `*no_caddie_view`, `*stepsnd`, `*sfx` have no handler in the source.

## Differences from the client

- **Newer data:** this data is newer than the client source. `#tag` arguments such as `"#대천사"` are skipped by inference.
  `*fxstop` and `*sfx` have no handler in the source, so `*sfx` does nothing here and `*fxstop` follows the inference above.
- **Box effects:** `CGolfPlayer` ignores a box's `*fx("a.seq")` with no second argument, so characters do too. Caddies play
  it, because files like `cien_armor.pet` rely on that form, but I didn't trace their path.
- **Early stops:** five shipped sequences stop early, as they would in the client: one at an unknown `clear` line
  (`tractorbeam_s.seq`), four at a spray file missing from the dump.
- **`*opa n` on caddies:** `golfcadie.cpp` isn't in the source, so the percentage reading comes from `CNpc`.
- **Faces with non-default parts:** the client's choice of face (`FaceTex`) comes from item data the tool doesn't read,
  so the name-based search above can pick a different texture.
- **Effects vs the game:** the port was never compared side by side with the running game, and I haven't heard
  the audio, only confirmed the voices run. The ground is the y = 0 plane, not a course. `*pet` particles hold their
  rest pose and ignore fade alpha. The flash is a plain white overlay; only its value comes from the client.
