# Wardrobe

Opening a character's `.bpet` opens the Wardrobe (View menu, F9), which dresses the doll from the `.mpet` files beside it.
The game decides this from item data. The tool only reads the file names, `<char>_<slot>_<id>[_!hide...].mpet`:

- **Slots:** `ts pv am lg hn ft ha fc`. Picking a part replaces the slot, as `ReplaceMeshBones` does. `(none)` empties it.
- **Hiding:** `!pv` hides whatever fills the `pv` slot, as `CPetFrame::ApplyHideMask` does. A part never hides itself. Each
  hidden row names the part responsible and has a **Show** box to overrule it.
- **Extras:** anything else stacks with a tick box: wings, tails, props, `_sub_` pieces, and on heads the `a`, `a_z` and `z`
  series, which are add-ons.
- **Colours:** the slot's second list offers the `-NN` texture numbers on disk, such as `i_ts_01-02b.jpg`. Extras have no colour list.
- **Effect on scripts:** `*ptex` finds the face on whichever part wears it, and `*hidebone` reaches each part's own mesh bone.

Outfits last until you open another file. `/wear part.mpet` wears a part from the command line, and `/nowardrobe` hides the panel.

## Limits

The rules are inferred, so some guesses will be wrong:

- `!la`, `!lb`, `!hb`, `!wr`, `!ha01` and `!hb02` name no slot and do nothing.
- A part the game places in a different slot than its name suggests can double up or leave a gap.
- `z`-series heads without `!ha` or `!fc` might be meant as replacements. They stack here, so untick or empty the slot by hand.
- Part-conditional effects don't follow what's worn; the Script menu toggle still decides.
