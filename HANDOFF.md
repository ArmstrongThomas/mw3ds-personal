# Agent handoff

Updated: October 30, 2026. This file is sufficient context to resume with another agent; read `HANDOFF.local.md` for
this PC's paths and device details. No Codex-specific skill or chat history is required to build the project.

## Repository and scope

Active checkout: `mw3ds-personal`, branch `main`. It was copied with full history from the prior performance checkout,
including its uncommitted work. The old checkout's branches/remotes were not changed by creating this one. The
separate published stylus implementation, `ArmstrongThomas/mw3ds`, is to be left alone.

Remote `upstream`: `https://github.com/3ds-conspiracy/mw3ds.git`. Included upstream main: `8b79e43`; latest fetch in
the earlier checkout was October 26. The history includes stylus commit `a7fdf681` and merge `dc12aff`. The user chose
a separate personal repository, preserving normal upstream merging. Publishing remote `origin` is
`https://github.com/ArmstrongThomas/mw3ds-personal.git`. Continue here and review/merge future upstream updates.

## Latest completed work

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
- Optional SD data pack: `tools/build/make_pak.py` packs the converted cell/actor/first-person/texture/art/UI files and
  game JSON into `data.pak` with a sorted `data.idx` (FNV-1a path hash -> offset/size). The engine mounts it at startup
  and reads those paths with one file open each, instead of making the 3DS FAT driver scan thousands of loose files.
  The ~0.5 MB index is loaded into RAM once. Sound, music and movies stay loose; no 1 GB CIA is needed for data updates.

## Latest artifact and verification

Built directly from this checkout: `mw3ds.3dsx`; preserved copy: `build/mw3ds-personal.3dsx`; root copy:
`mw3ds-personal.3dsx`. SHA-256: `e2b6e9a22cf2a1eb8b16114736978a4facc43f2b483ce608c918eeab77480493`
(2,399,184 bytes).
Both native and devkitPro builds pass. The blood-preload, TCL, save/load rework and full save-preview builds were sent
over 3dslink on October 26; this data-pack build is built and tested but not sent. New 3DS XL feedback on the earlier
build: equipment preload makes combat smooth; a quick switch to a weapon not yet cached still shows one load, then that
weapon stays cached. Minimap labels persist across taps and between the minimap and full map; very long names span much of
the screen but were accepted. The scrolled Stats lists were confirmed better. The blood-splat preload, TCL behavior and
the save/load preview are pending hardware confirmation. Audible loading music still needs a listening check.

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
| `perf-save-arrival` | 6 | Nearby cells ready, initial render pause, frozen world time |
| `perf-npc-redress` | 14 | Deferred edits, retained mesh, presented feedback, frozen time, HUD hidden |
| `perf-stylus-minimap-navigation` | 30 | Pan/zoom/follow, independent maps, both hands, camera isolation |

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

That writes `data.pak` and `data.idx` beside each other. Copy those two files to `sdmc:/3ds/mw3ds/data/`, and keep
`snd/`, `music/` and `movies/` there as loose folders (the packer leaves them loose on purpose: they stream, and the
sound code opens them directly). The loose `cells/`, `fp/`, `tex/`, `art/`, `ui/` folders and `game*.json` are then
redundant on the card; the pack is read in preference to them. Rebuilding after a reconvert takes about 90 seconds and
does not rebuild or reinstall the CIA.

`--all` also packs sound/music/movies, but the audio reader still uses direct `fopen`, so do not remove those loose
folders until the audio path is archive-aware. The packer's default leaves `music`, `movies`, `sound` and `snd` loose.

The native and isolated Azahar builds loaded the whole Tribunal island through the pack (`feat-save-preview` with
`--data build/native/pack`); real-hardware load timing and the speed difference from loose files are still unmeasured.

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
