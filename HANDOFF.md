# Agent handoff

Updated: October 26, 2026. This file is sufficient context to resume with another agent; read `HANDOFF.local.md` for
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
- Combat equipment is read ahead: equipped gear's first-person pieces (weapon, shield, tool, ammo in the bow and in
  flight) and the selected spell's cast/bolt/hit/area visuals are cached when the equipment or spell changes. Ready,
  draw, shot and first cast no longer stop under "Updating equipment..." in normal play.

## Latest artifact and verification

Built directly from this checkout: `mw3ds.3dsx`; preserved copy: `build/mw3ds-personal.3dsx`.
SHA-256: `ce199386e2453b93bd67f71785160dda946faf58f4be620b9c1efadf2dbc74a7` (2,385,372 bytes).
Both native and devkitPro builds pass. This revision has not been sent to hardware yet; the previous build was sent
successfully over 3dslink on October 26. Hardware feedback on the scrolled stats lists, the long minimap names, the
equipment preload and audible loading music is pending.

Native **and** isolated Azahar tests pass with the locally available **Tribunal** data:

| Test | Expectations | Evidence |
| --- | ---: | --- |
| `personal-stats-ui` | 31 | All tabs, scrolled Misc/Factions lists, empty/populated factions, L/R/B |
| `perf-stylus-map-markers` | 12 | Tap Balmora and its long Guild name; label drawn outside the minimap |
| `feat-preload-equipment` | 4 | Bow and arrows cached at equip; ready adds no piece reads |
| `perf-save-arrival` | 6 | Nearby cells ready, initial render pause, frozen world time |
| `perf-npc-redress` | 14 | Deferred edits, retained mesh, presented feedback, frozen time, HUD hidden |
| `perf-stylus-minimap-navigation` | 30 | Pan/zoom/follow, independent maps, both hands, camera isolation |

Logs: `build/*-tribunal-native.log`, `build/*-tribunal-emu.log`; captures: `build/shots/tribunal/`.
Character, Attributes, the scrolled Misc skills list, empty Factions, and populated/scrolled Factions renders were
inspected. Earlier navigation, marker, shrine tooltip, worn-effect, save, shutdown, and Old 3DS compatibility results
are in the performance spec. The stats, marker-name and equipment-preload rows were rerun on both the native build and
the isolated Azahar copy for this revision; the emulator still has no DSP firmware.

## Outstanding checks and limits

1. Ask the user to listen during loading on hardware and review the new Stats screen, the long minimap marker names,
   and combat's first ready/draw/shot/cast after the preload change. The emulator lacks DSP firmware; its audio-init
   failure means current automated tests do not verify audible music continuity. Preload moves equipment reads to a
   menu or loading screen; it does not make a read faster, and hardware timing of the move is still unmeasured.
2. Initial exterior arrivals and individual model/texture reads remain expensive. Loading protection moves work
   behind a pause; it does not establish a faster load or an FPS-based stabilization algorithm.
3. Memory-limited arrival neighbors can be deferred; this is logged rather than forced past heap/linear limits.
4. Original hardware feedback: exterior boundaries improved, race previews feel smoother, initial exterior load
   still took about ten seconds of low FPS, and corpse equipment changes cost roughly twelve seconds or more.
   Per-piece timings exist to investigate the remaining cost. Player outfit changes were much cheaper.
5. The old `bug-outdoor-stream-textures` falling warning was reproduced on the unchanged baseline at
   (-19174, 49552, 1330), vertical speed -1756. Do not report that scenario as a clean pass or a new regression.
6. The personal repository is `https://github.com/ArmstrongThomas/mw3ds-personal`. Shared history and `upstream` allow
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
