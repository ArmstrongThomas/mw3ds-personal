# New 3DS loading and frame pacing

The `perf/new3ds-streaming` branch includes the stylus controls and upstream main through `8b79e43`.
The reported hardware is a New Nintendo 3DS XL. Both door transitions and outdoor cell boundaries cause stuttering;
changing races during character creation causes a particularly large pause.

## First pass

### Compressed model and cell reads

The console opens MWZ1-compressed files as forward-only cookie streams. Stdio is deliberately unbuffered because its
read-ahead and backward seeks are incompatible with that interface. Previously each scalar `fread` called inflate
with an output buffer as small as four bytes. An 8 KiB decoded-data buffer now batches that work without changing
the caller-visible position or forward-seek contract. It adds 8 KiB per open compressed stream, rather than keeping
whole cells or archives in memory. Converted data does not need to change.

The standalone buffer test serves 16,384 four-byte reads from eight full fills, preserving every byte. This is a
structural reduction in inflate entry calls, not a claim of a corresponding FPS multiplier. The native game uses
whole-file decompression and cannot benchmark the console's cookie-stream path or SD latency.

### Race preview

Browsing races previously rebuilt first-person arms that are not drawn by the face-preview camera, and read every
new third-person body part synchronously in one frame. The preview now skips the first-person rebuild, retains
parts when the skeleton binding is unchanged, and reads at most one new body-part model per update. Head and hair
are processed first. The replacement is assembled separately, keeping the previous complete character visible until
it is ready. A highlighted banner in the race-description panel reports loading or a failed preview.

This bounds the burst of model loads; it does not bound the duration of one model or texture read. First use of a
different animation skeleton can still block. The complete preview keeps the same body parts and meshes. No
unbounded race/face cache is added, and unused pieces still release their geometry and texture references.
Keeping the previous character until the swap temporarily needs geometry for both characters. Both New and Old
3DS emulator profiles pass the preview checks; hardware memory pressure still needs observation.

### NPC clothing changes

Looting clothing previously rebuilt the entire NPC in one update. Model work is deferred while any menu is open;
closing it builds the final outfit with at most one body-part model read per frame. Reopening a menu cancels pending
work. The old complete actor stays visible until all replacement pieces are ready. Leaving or reloading its cell cancels
it, and shutdown frees pending geometry. A failed piece retains the old actor instead of showing a partial body.
This spreads model reads across frames, but an individual model or texture read can still cause a pause.

### Exterior streaming

New 3DS now tries processor 2 for the existing single cell-loader job. The CIA RSF already has `CanAccessCore2: true`.
If the launcher does not permit that core, the existing permitted-system-core/default-core/synchronous fallbacks
remain. Old 3DS retains its system-core path. Only one job is outstanding; this does not introduce concurrent
mutations of the world reference lists.

Missing neighbours are prioritized by distance from the player's position to their cell bounds, with the current
cell first. A player near the south boundary no longer waits behind a fixed north-first loading order. The 3x3
resident-cell window, ground safety checks, and memory limits are unchanged. Sorting nine candidates uses no
per-frame heap allocation.

### Measurements and HUD

The log now separates geometry/texture reading, actor reading, and main-thread cell integration; streamed-cell lines
include the loader core. Preview completion logs include elapsed time and cumulative piece reads.

The gameplay HUD shows FPS and battery status in both regular and stylus layouts. Battery readings use
`MCUHWC_GetBatteryLevel`, which reports a percentage, and are cached for ten seconds. Failed or unavailable service
access displays `Bat --`; it never turns the coarser PTMU levels into invented percentages.
Service reference: [MCUHWC:GetBatteryLevel](https://www.3dbrew.org/wiki/MCUHWC:GetBatteryLevel).

Stylus mode also shows a 96x96 minimap inside the look pad, mirrored to the right for left-handed use.
The Local/World button above the stylus minimap changes its view without opening a menu, pausing, or changing weapon
readiness. Tapping a marker shows its place or destination. The map and visible effect icons own gestures that start
on them; a camera swipe beginning in the empty pad keeps ownership across those controls. The Map footer opens the full
interactive map. Outdoor local maps reuse the existing cached overhead render and refresh after moving 2048 units;
creating that picture can itself cost a frame.

## Further candidates requiring hardware profiles

- Crossing into a cell not yet ready still waits for an outstanding loader job and may load synchronously. Nearest
  priority and a separate CPU core reduce contention but do not eliminate SD reads or that safety wait.
- Cell integration appends references, syncs actors, applies hidden geometry, and rebuilds resident lists on the main
  thread. The new integration timing is intended to determine whether this needs incremental scheduling.
- Zero-reference textures are immediately retired. A bounded texture cache might improve repeatedly entering and
  leaving a building, but must account for the GPU-visible linear heap, deferred frees, and Old 3DS memory limits.
  The extra C++ heap on New 3DS does not automatically enlarge its 32 MiB GPU-visible linear allocation.
- The renderer builds/sorts the visible opaque draw list every first-eye pass; PlayerBody computes an equipment key
  every third-person update. These are candidates for revision-based caching if the CPU profiles justify it.

## Combat equipment preload

The first ready, draw or shot of a weapon used to stop the game under "Updating equipment..." while the view model read
its pieces. The equipped gear's first-person pieces are now read ahead and cached: every equipped item's arm pieces,
plus the weapon, shield, tool, ammo-in-bow piece and the ammo's in-flight model, are read when the equipment changes
(a menu or the arrival's loading screen is normally up). Ready/draw then find them cached. The selected spell's cast,
bolt, hit and area pieces (and the hand glow and OpenMW's fallbacks) are read when it is selected, before a cast can
start; quick keys load them on the following frame. This moves reads out of combat; it does not make one faster, and a
piece can still be released when linear memory runs short. (`viewmodel.cpp preloadEquipped`,
`effectmanager.cpp preloadCastVfx`, `character.cpp viewModelUpdate`, test `feat-preload-equipment`.)

## Hardware comparison

Use the same save, data, view-distance setting, 3D slider position, and route for both builds. Compare the race menu
while changing race, sex, face and hair; walk repeatedly across an outdoor boundary; enter and leave a building.
Check the FPS readout, actual pause duration, and `sdmc:/3ds/mw3ds/log.txt`. Confirm the battery percentage is shown.
Inspect the loader core and phase timings before attributing a remaining pause to rendering or SD access.

Native correctness tests and the standalone buffer/priority tests cannot establish real-console frame-time gains.

## First-pass verification

- Native and devkitPro 3DS builds succeed.
- Standalone forward-buffer and neighbour-priority tests pass.
- `perf-race-preview`: 12 expectations pass on each of the New and Old 3DS profiles.
- `feat-stylus-ui`: 41 expectations pass; `chargen-race-turn`: 4; `hw-chargenfaces`: 1.
- `bug-walk-unloaded-cell`: 3 expectations pass. `bug-cell-border-reload` completes with no monitor warnings.
- `bug-outdoor-stream-textures`: all 10 expectations pass, but the runner reports a falling monitor at
  (-19174, 49552, 1330), vz -1756. An isolated, unchanged branch-base build reproduces the same monitor at the same
  position and velocity. This scenario is not counted as a clean test pass.
- Both HUD layouts, the loading-preview label, and completed race previews were rendered and inspected.
- Battery-service access and processor-2 availability still require confirmation on the physical console.

## Follow-up verification

First-pass New 3DS XL feedback: FPS and battery display correctly, outdoor transitions feel substantially better,
and door transitions may feel better. Race changes still take about as long overall and the partial character
assembly looked distracting. Looting NPC armor exposed another synchronous model rebuild. Those observations led
to the complete-preview swaps, restored minimap, and staged NPC clothing changes above.

The revised build passes these Azahar checks using the actual 3DS executable and converted game data:

- `perf-race-preview`: 14 expectations on each New and Old 3DS profile, including unchanged visible geometry while
  loading and at most one model read per update.
- `feat-stylus-ui`: 44 expectations, including camera drags over the minimap and opening the full map.
- `perf-npc-redress`: 5 expectations, covering paused inventory, complete visible geometry, changed inventory during
  preparation, completion, and at most one model read per update.
- `perf-dress-quit`: exits through the real shutdown path with a clothing job pending.
- `bug-npc-strip`: 2 expectations using the normal corpse-looting driver.
- `bug-walk-unloaded-cell`: 3 expectations; `bug-cell-border-reload`: no driver or monitor warnings.

The native versions of the preview, stylus, and NPC rebuild checks also pass. Native shutdown is not a valid check:
that harness deliberately exits before the console's Save and quit path. Azahar lacks DSP firmware in this test
setup and does not implement the battery-percentage service; neither audio nor real battery readings are validated
there. Emulator execution speed is not a New 3DS frame-time measurement.

Tomorrow's hardware checks: change races/sex/face/hair rapidly, loot several armor pieces while leaving the loot menu
open, then cross exterior boundaries and enter/leave a building. Confirm complete character swaps, map visibility,
camera drags across the map, full-map interaction, and FPS/battery readability. Watch for any remaining individual
model-read spikes or memory failures. Use the same save and settings when comparing with the first build.

## Second hardware feedback and follow-up

New 3DS XL feedback on the second build: complete race-preview swaps feel less janky. Initial exterior entry still
causes roughly ten seconds of reduced frame rate. Corpse equipment edits cause roughly twelve seconds or more of
disruption, while player equipment changes are much cheaper. Moving individual reads across frames did not resolve
that cost. Deferring all appearance work until menu close avoids repeatedly rebuilding intermediate outfits during
looting. Per-piece logs now separate model/texture loading from attachment, plus total rebuild elapsed time; these
are needed to locate the remaining hardware cost. This is not a claim that the eventual rebuild is now inexpensive.

Upstream was fetched again on October 26: `origin/main` remains `8b79e43`, already an ancestor of this branch,
including `72756a6` (keep whole-file inflate off the console heap). The `0.1.0` tag was fetched too.

The follow-up `build/mw3ds-perf-v3.3dsx` passes native stylus (48), deferred NPC changes (11), normal looting (2),
race preview (14), and gesture-ownership checks. Azahar passes stylus (48), deferred NPC changes (11), restoring the
original outfit before closing the menu with zero model reads (4), pending-job shutdown, and rendered Local/World
toggles (4). Both minimap modes were visually inspected. These checks establish behavior, not hardware timing.

## Loading protection, feedback, and live HUD

The fourth build prepares the missing cells in the arrival's existing 3x3 exterior grid behind the loading screen,
before starting simulation. This runs for initial loads, save reloads, and cell travel. It joins any outstanding
stream job first and retains heap/linear-memory guards: cells that cannot fit are logged as deferred rather than
forced into memory. Character resources are prepared there too, followed by two rendered frames without simulation.
This moves the initial streaming work into loading; it is not an FPS-based stabilization heuristic or a speedup.

Corpse rebuilds now pause simulation and controls, draw a centered status panel before the first model read, and show
completed parts / total parts. The panel is drawn even with the gameplay HUD hidden, avoiding an indefinite wait for
presentation. Menu edits still defer all model work until closing the menu. Model failures retain the old actor.

Full loading screens retain the original splash art on top; their status and progress are on the bottom display.
The smaller in-game loading panel remains centered on the top display. Full-screen bars track cells within the arrival
phase, character preparation, or three save phases (serialization, file writing, and finalization including metadata).
They count work units, not elapsed time or estimated overall loading percentage. Unknown-length stages show a named
status without a fabricated bar. Synchronous terrain and uncached player-equipment reads show status before the read.
Manual New save writes are deferred out of UI rendering into the engine's loading scope; failure still reports a
notice and preserves the existing save-write behavior. Save and quit also shows its centered saving status.

The live minimap, its toggle, FPS/battery layout, aiming hints, and active-effect row mirror with stylus handedness.
Map marker taps and effect tooltips do not open a menu or pause the world. The active-effect row uses the standard
HUD's effect list, including abilities and worn constant-effect enchantments from upstream `cb7e59a`. Shrine casting
visuals and sounds remain on the existing upstream path (`d998504`); the blessing's icon and tooltip work in both
handedness layouts. A selected local marker is retained while the stylus HUD is active.

The stylus activation notice lasts five seconds of simulation time. Menus and loading pauses suspend its countdown;
rapid UI test captures can therefore all include it. It is not a permanent overlay.

Targeted native and Azahar checks cover protected save arrivals (6), corpse pause/progress (14), stylus controls and
mirrored toggles (52), shrine visuals/effects/tooltips in both layouts (10 each), worn constant-effect stats and
rendered icons (5), and manual New save (1). Live marker interaction is also checked in both layouts (8), including
that it leaves the screen in gameplay with the world unpaused. Physical-console timing and memory pressure still
need testing. Existing baseline outdoor-falling warnings remain separately documented above.

## Minimap navigation

The fifth build adds separate session-local zoom/pan state for the local and world minimaps. Their mode button is
between minus and plus controls; zoom ranges from 1x to 8x locally and 0.5x to 8x on the world map. Dragging from the
map pans its cached picture, within its bounds, without rotating the camera or pausing gameplay. Follow returns to
the player and resumes tracking; the indoor world map instead offers Center because it cannot locate the player
on the world picture. Changing cells resumes tracking while retaining zoom. Switching map modes retains each
view's zoom and last pan state.

Gesture ownership distinguishes camera, map, and button contacts. Camera-origin swipes remain camera input across
the map and icons; map-origin swipes remain map input across the surrounding panel. Dragging the map cannot trigger
the held jump/levitation action when crossing its button. Zoom and Follow/Center touches do not become map drags.
The controls mirror with handedness. Marker taps, active effects, the full map, and the original loading splash
behavior remain supported.

The original splash list is now read alongside the startup UI metadata, before loading the world. Loading scopes
draw their first frame immediately, so the splash is available during loading, rather than appearing only after
the world and progress work have completed. A native run confirmed 11 splash references were available before
the first cell read.

Native navigation checks cover world/local zoom, pan, recentering, camera isolation, independent view state, and
handedness (30 expectations). The existing stylus suite still passes (52); standalone gesture checks also pass.
Native zoomed/panned local-map renders in both handedness layouts were inspected. Additional emulator testing uses
the separate `azahar-zoom` copy, leaving the user's running `azahar-perf` instance alone.

The separate emulator passes navigation (30), marker interaction (8), existing stylus controls (52), and
left-handed shrine-effect tooltips (10), without driver or monitor warnings. Build: `build/mw3ds-perf-v5.3dsx`.

Hardware baseline update: the user switched the console SD card to the release's Tribunal data before testing the
fifth build. The fifth build was sent successfully over 3dslink. Automated results above used the prior local data;
they are not Tribunal-data validation results.
