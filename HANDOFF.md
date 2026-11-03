# Agent handoff

Updated: October 30, 2026. This file is sufficient context to resume with another agent; read `HANDOFF.local.md` for
this PC's paths and device details. No Codex-specific skill or chat history is required to build the project.

## Repository and scope

Active checkout: `mw3ds-personal`. `main` is the personal line at `09926ee`; the upstream merge is on
`merge-upstream-2026-10-31` (also pushed, tagged `0.2.3.e`), with the pre-merge tip kept on
`personal-before-upstream-2026-10-31` and a clean upstream copy on `upstream-main-266176f`.
The old checkout's branches/remotes were not changed by creating this one. The separate published stylus
implementation, `ArmstrongThomas/mw3ds`, is to be left alone.

Remote `upstream`: `https://github.com/3ds-conspiracy/mw3ds.git`. The merge branch includes upstream main
`266176f` (tag `0.2.3`) and all commits through it, including the upstream stylus layouts, local-map and Bloodmoon
work. The history includes stylus commit `a7fdf681` and merge `dc12aff`. The user chose a separate personal repository,
preserving normal upstream merging. Publishing remote `origin` is
`https://github.com/ArmstrongThomas/mw3ds-personal.git`. Continue on the integration branch until it is tested.

## Latest completed work

- Upstream issue 74 (dark egg-mine local maps) is already fixed by upstream `453c312` in this merge: local-map rendering
  adds the OpenMW-style overhead light for interiors. `issue-74` passes 6/6 native; the existing lit-interior shot is
  21.4 average luminance (limit 15–60), and the dark-interior expectations pass.
- Upstream main through `266176f` (0.2.3) is merged on `merge-upstream-2026-10-31`: upstream stylus layouts and glass
  buttons, local-map panning/fog, Bloodmoon content, memory/card work and new tests. The personal look-pad stylus is
  kept as a separate Touch option with left/right hand, alongside the upstream stylus layout (which now also mirrors its
  glass buttons/map for the left hand and uses the C-Stick to walk). Save previews/overwrite, tabbed scrolled stats,
  minimap labels/panning, combat preloading, TCL and the data pack are retained.
- Optional stylus aiming with sensitivity/inversion and left/right-handed controls. Hardware sensitivity preference
  was approximately 2.10x. Player tests use a New Nintendo 3DS XL.
- Mirrored live local/world minimap, marker destination taps, separate zoom/pan states, Follow/Center, effect icons
  and tooltips, FPS, and actual battery percentage. Camera-origin swipes retain ownership across controls; map-origin
  swipes pan without turning the camera. User confirms map interaction and mirrored layout feel good.
- 8 KiB forward decoded read buffer, closer-neighbor exterior loading priority, New 3DS core-2 loader attempt with
  fallbacks, and logging of cell read/integration costs.
- Race previews assemble separately and swap when complete. Corpse outfit updates wait until menus close, assemble
  one part per frame, retain the old mesh until complete, and pause the world with a presented progress panel.
- Save/exterior arrivals prepare nearby cells before simulation, retaining memory guards; two rendered frames run
  with world time stopped. Original splash metadata is available before loading starts. Full-screen progress/status
  is on the bottom display, leaving splash artwork unobstructed above. Unknown work has no fake completion bar.
- Loading music patch: service the existing playlist during engine frames, loading redraw/log hooks, decoded-buffer
  refills, and waits for streamed cells. Audio runs only on its owner's thread, throttled to 100 ms with recursion
  protection. Loading rendering has a recursion guard. No track restart is introduced.
- Stats is now four tabs: Character, Attributes, Skills, Factions. It displays aligned values, vitals/level/skill
  progress bars, major/minor/misc groups, faction ranks/reputation/expulsion, and scrollable Skills/Factions lists.
  Lists scroll by dragging, with the C-stick, or a row at a time with Up/Down; the Prev/Next pager is gone. L/R switches
  tabs; Skills uses Left/Right for groups. B closes. Existing stats calculations are unchanged.
- A tapped minimap marker's name is drawn on the HUD's top layer, outside the 96 px minimap, so long names such as
  "Balmora, Guild of Fighters" are no longer clipped.
- Console `tcl` now toggles collision: with it off the player moves freely through geometry and gravity, using the fly
  controls (ZR/R up, ZL/L down, jump also rises) to get out of a clipped spot; `tcl` again restores collision. It
  reports "Collision -> Off/On".
- Save / Load screen rework: two-line saved-game cards (character and place, level and date/time), a selected-row
  highlight, save count, scrolling, and a footer of New save / Save / Load / Delete / Back. Save overwrites the selected
  player save after a confirmation, so the list no longer has to grow for every save. Dragging scrolls freely past the
  selected row; a save scrolled fully out of view is no longer selected, so Save cannot overwrite a hidden one; a few
  pixels of C-stick drift do not clear it. Each save also carries a 400 x 240 RGB565 `.shot` from two HUD-free top
  frames (double buffering needs both); while the Saves screen is open, the selected save's picture fills the top screen
  (or says "No screenshot for this save"). The title screen's Load game list uses the same cards and picture.
- Combat equipment is read ahead: equipped gear's first-person pieces (weapon, shield, tool, ammo in the bow and in
  flight), the selected spell's cast/bolt/hit/area visuals, and every blood splat a hit can throw are cached when the
  equipment or spell changes or at startup. Ready, draw, shot, first cast and the first successful hit no longer stop
  under "Updating equipment..." in normal play; the blood pieces are pinned from the VFX trimmer.
- Optional SD data pack: `tools/build/make_pak.py` packs the converted cell/actor/first-person/texture/art/UI files into
  `data.pak` with a sorted `data.idx` (FNV-1a path hash -> offset/size). The engine mounts it at startup and reads those
  paths with one file open each, instead of making the 3DS FAT driver scan thousands of loose files. The ~0.5 MB index
  is loaded into RAM once. Sound, music, movies, and the small root metadata (game*.json, maps, distant.bin) stay loose;
  no 1 GB CIA is needed for data updates.

## Personal version and converter outputs

- Personal version **0.2.3.e** is in `VERSION.txt`; the CIA's numeric title stays 0.2.3 and the HOME menu text carries
  the `.d`. Bump to `.e`, `.f`, ... for later personal patches before the next upstream merge, then reset to
  `<new upstream>.a`. Tags/Release work for the bare and lettered forms.
- `0.2.3.b` fixed the 0.2.3.a converter package: `tools/build/make_pak.py` is now included in `APP_FILES` (the data-pack
  output had failed with "can't open file .../tools/build/make_pak.py"). The release workflow also checks the file is in
  both the Windows package and the container image.
- `0.2.3.c` merges the Touch tab into **Controls** (four tabs now: Prefs, Audio, Video, Controls). Touch off shows the
  button/stick controls; touch on shows the stylus pad settings; the upstream stylus layout shows its slide settings.
  The hidden `SCR_TOUCH` screen remains only for the existing test hooks.
- `0.2.3.c` also moves the quick-key equipment preload ahead of `vm.rebuild` and stops `equipItem` from warming a
  weapon/shield on a HUD quick-key use, which was the first-load "Updating equipment..." full-screen flash.
- `0.2.3.e` makes the right-handed stylus attack on **L** and cast on **R** (the personal look pad's hand mapping);
  left-handed stylus mirrors it. Both the glass shoulders and the physical shoulders follow the setting; the button
  list and HUD comments follow too. The pack mount checks `data.pak`'s size against the index's aligned furthest offset
  and refuses a short or mismatched copy with a clear log line instead of loading another entry's bytes as a cell.
- Timing builds use separate title IDs: full `0xF8287`, data-pack stub `0xF8288`, loose-data stub `0xF8289`.
- The converter's **Output** choice has: Full CIA, Small CIA + indexed data pack (recommended), and Small CIA + loose
  data folder. The SD choices write `3ds\mw3ds\data` in the output folder; the pack choice runs `make_pak.py` and copies
  the loose root/audio files too.
- Command line equivalent: `python tools/build/build_cia.py --no-data --pack --tribunal --bloodmoon`; the Windows
  converter zip and `ghcr.io/armstrongthomas/mw3ds-converter:<version>` image are built by the tag-triggered
  `.github/workflows/release.yml` (the `image-latest.yml` workflow moves `latest` after publishing).

## Latest artifact and verification

Built directly from this checkout: `mw3ds.3dsx`; preserved copy: `build/mw3ds-personal.3dsx`; root copy:
`mw3ds-personal.3dsx`. Personal version **0.2.3.e**. SHA-256:
`a16dd398b35be3abc0b3b5a51594b8d17ba58967cbaaa0a6c554462bd2b87156` (2,559,128 bytes).
The timing CIAs are all in `build/`:

- full data `mw3ds-full-tribunal-bloodmoon.cia`: 1,440,064,448 bytes, SHA-256
  `1283d8778eb4d2a75bdd7317d576540835d590f4edd104b5296070f5d747b23a`, title ID `0xF8287`.
- data pack `mw3ds-pack-tribunal-bloodmoon.cia`: 2,408,896 bytes, SHA-256
  `b23e34acdd6c16c908cf8c94b2d79284d7d2e52c6f9ecddb109692e86baad807`, title ID `0xF8288`.
- loose data `mw3ds-loose-tribunal-bloodmoon.cia`: 2,408,896 bytes, SHA-256
  `49b36d028e191bd6d61d70753264ee9f711fcdb4eecc96fcefef35303fb05d91`, title ID `0xF8289`.

The pack CIA reads `E:\Games\Morrowind\3ds\MW3DS-Converter\output\3ds\mw3ds\data` (copy its `3ds` folder to the SD
root); the full CIA carries data in RomFS. The loose CIA needs the same folder with `data.pak`/`data.idx` renamed away so
it falls back to loose files. The converter zip is
`build/release-personal-e/MW3DS-Converter-0.2.3.e.zip`, 124,819,345 bytes, SHA-256
`787ee2907251adcef03454ea678b67d65851e605469e34b84424b37a1f6f389c`; `check_version.py 0.2.3.e` and the packaged
`make_pak.py` smoke test pass. The merged build has not been sent to the console yet. New 3DS XL
feedback on the earlier build: equipment preload makes combat smooth; a quick switch to a weapon not yet cached still
shows one load, then that weapon stays cached. Minimap labels persist across taps and between the minimap and full map;
very long names span much of the screen but were accepted. The scrolled Stats lists were confirmed better. The
blood-splat preload, TCL behavior and the save/load preview are pending hardware confirmation. Audible loading music
still needs a listening check.

Native **and** isolated Azahar tests pass with the locally available **Tribunal** data:

| Test | Expectations | Evidence |
| --- | ---: | --- |
| `personal-stats-ui` | 31 | All tabs, scrolled Misc/Factions lists, empty/populated factions, L/R/B |
| `console-commands` | 25 | TGM, TAI, COC/COE, ShowVars, Help, FillMap/Journal, TCL / ToggleCollision |
| `perf-stylus-map-markers` | 12 | Tap Balmora and its long Guild name; label drawn outside the minimap |
| `feat-preload-equipment` | 4 | Bow and arrows cached at equip; ready adds no piece reads |
| `feat-preload-blood` | 3 | All blood splats cached before the first hit; the hit adds no piece reads |
| `feat-save-overwrite` | 6 | New save, reopen, overwrite selected save, list stays at one |
| `feat-save-scroll` | 5 | Swipe past the selected save; it clears when scrolled out of view |
| `feat-save-preview` | 3 | A new save writes its 400 x 240 shot, shown on the top screen |
| `feat-stylus-look` | 38 | Upstream stylus layout: slide, tap slop, glass buttons, speed/buttons settings |
| `perf-save-arrival` | 6 | Nearby cells ready, initial render pause, frozen world time |
| `perf-npc-redress` | 14 | Deferred edits, retained mesh, presented feedback, frozen time, HUD hidden |
| `perf-stylus-minimap-navigation` | 30 | Pan/zoom/follow, independent maps, both hands, camera isolation |
| `feat-controls-tab` | 4 | Controls/Touch one tab; toggle swaps the stylus settings and back |
| `issue-74` | 6 | Dark egg-mine local map lit; lit interior stays in range |
| `bm-cells` (Bloodmoon pack) | 48 | Solstheim/Raven Rock/Frostmoth cells load; 16 content placement monitor warnings |

Logs: `build/*-tribunal-native.log`, `build/*-tribunal-emu.log`; captures: `build/shots/tribunal/`.
Character, Attributes, the scrolled Misc skills list, empty Factions, and populated/scrolled Factions renders were
inspected. Earlier navigation, marker, shrine tooltip, worn-effect, save, shutdown, and Old 3DS compatibility results
are in the performance spec. The stats, marker-name, equipment-preload, blood-preload, console-command and save rows were
rerun on both the native build and the isolated Azahar copy for this revision; the emulator still has no DSP firmware.

## SD data pack

The program-only CIA can read one indexed pack from the SD card:

```
python tools/build/make_pak.py --data out/data-tribunal --out sd/3ds/mw3ds/data/data.pak
```

That writes `data.pak` and `data.idx` beside each other. For the Tribunal data it is 969 MB of 20,487 files plus a
480 KB index, and takes about 90 seconds. Copy those two files to `sdmc:/3ds/mw3ds/data/`, and keep `snd/`, `music/`,
`movies/` and the small root files (`game*.json`, `map*.t3x`, `distant.bin`) there as loose files. The packer leaves
those loose on purpose: the root files are small and opened before the first cell, and reading them from the pack made
startup slower on the emulator. The loose `cells/`, `fp/`, `tex/`, `art/` and `ui/` folders are redundant on the card;
the pack is read in preference to them. Rebuilding after a reconvert takes about 90 seconds and does not rebuild or
reinstall the CIA.

`--pack-root` also packs the root files; `--all` also packs sound/music/movies, but the audio reader still uses direct
`fopen`, so do not remove audio folders until that path is archive-aware. The default leaves `music`, `movies`, `sound`,
`snd` and root files loose.

The merged Bloodmoon + Tribunal pack made by the current converter is staged at
`E:\Games\Morrowind\3ds\MW3DS-Converter\output\3ds` (1.3 GB total):

- `data.pak`: 1,129,084,416 bytes, 23,675 files, SHA-256 `55b6602a27e6f43a35a3bd1757663c932e626cde685893fa27bbd2600fdcff15`.
- `data.idx`: 568,216 bytes, SHA-256 `66b2be73d7b49a50b0fb7077ff8dbc1cf0b4d9e2bf2f4719642d8c27315fa147`.
- The source conversion is `MW3DS-Converter/work/out/data-tribunal-bloodmoon` (4,078 cells, 23,694 packed files); it was
  converted from the Steam install's `Bloodmoon.esm`/`.bsa` and `Tribunal.esm`/`.bsa`.
- A pack crash on 2026-11-03 (missing `cells/38/ext_m3_m3.json`, bad `cells/2e/ext_m3_m2.cel`) came from a bad pair on
  the card: both entries are valid in the source pack above. The full CIA loaded from its RomFS, confirming the engine.
  Delete the SD `3ds/mw3ds/data` folder and copy both files fresh from the converter output, not an older
  `build/bloodmoon-pack` copy.
- `bm-cells` checked 48 Bloodmoon/expanded-world expectations with 0 failed (16 content placement monitor warnings);
  `issue-74` 6/6 and `feat-stylus-look` 38/38 also pass reading the staged pack.

### Azahar baseline (3 runs each, same final build)

| Measured from log | Loose SD layout | Data pack | Difference |
| --- | ---: | ---: | ---: |
| `game data` startup | 23,228 ms | 23,225 ms | ~0 |
| Nine cell loads, summed | 13,303 ms | 12,217 ms | **-1,086 ms (-8.2%)** |

Each run repeated within 1–3 ms. The pack helps the cell/actor/texture opens (the first big cell dropped from 1,801 to
1,604 ms, the 6.4 s cell to 5.8 s) and leaves startup unchanged once root metadata is loose. Azahar's host-backed SD is
much faster at directory scans than the console's FAT driver, so this is a lower bound; real-hardware timing is still
unmeasured.

## Outstanding checks and limits

1. Audible loading music still needs a listening check on hardware. The emulator lacks DSP firmware; its audio-init
   failure means current automated tests do not verify audible music continuity.
2. Equipment preload moves reads to a menu or loading screen; it does not make a read faster. A weapon first equipped
   or quick-switched during play still pays one synchronous read, then stays cached. Very long minimap names cover much
   of the screen; accepted for now.
3. Initial exterior arrivals and individual model/texture reads remain expensive. Loading protection moves work
   behind a pause; it does not establish a faster load or an FPS-based stabilization algorithm.
4. Memory-limited arrival neighbors can be deferred; this is logged rather than forced past heap/linear limits.
5. Original hardware feedback: exterior boundaries improved, race previews feel smoother, initial exterior load
   still took about ten seconds of low FPS, and corpse equipment changes cost roughly twelve seconds or more.
   Per-piece timings exist to investigate the remaining cost. Player outfit changes were much cheaper.
6. The old `bug-outdoor-stream-textures` falling warning was reproduced on the unchanged baseline at
   (-19174, 49552, 1330), vertical speed -1756. Do not report that scenario as a clean pass or a new regression.
7. The personal repository is `https://github.com/ArmstrongThomas/mw3ds-personal`. Shared history and `upstream` allow
   normal Git merges without GitHub's formal fork badge/Sync fork button. The original stylus issue response is
   already published; do not alter it for these personal changes unless requested.

## Supporting docs

- `DEVELOPMENT.md`: build/test/update workflow.
- `FORK.md`: identity and feature overview.
- `spec/personal-stats.md`: stats layout and loading-music implementation.
- `spec/performance-new3ds.md`: earlier changes, hardware feedback, data distinctions, and caveats.
- `HANDOFF.local.md`: exact local tools, data, emulator paths, and 3dslink instructions; not tracked.

When resuming, check `git status` and the newest commits before changing anything. Keep these notes current with
what was actually completed and verified, rather than repeating plans as completed work.
