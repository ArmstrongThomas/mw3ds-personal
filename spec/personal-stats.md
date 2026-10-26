# Personal stats screen and loading music

The personal checkout lives at `E:\Games\Morrowind\3ds\mw3ds-personal`, on `main`, retaining the upstream history.
The separate `ArmstrongThomas/mw3ds` stylus repository is not modified or used as a publishing remote here.

## Stats

The scrolling prose stats screen is replaced with four touch/controller tabs:

- Character: race, class, birthsign, level progress, and current/maximum vitals with their existing colors.
- Attributes: all eight on one page, with aligned current/base values and coloring for modifiers.
- Skills: Major, Minor, and Misc groups, five rows per page, values and progress toward the next point.
- Factions: membership, rank, faction reputation, expelled status, player reputation, and bounty; four per page.

L/R changes tabs. On Skills, Left/Right changes groups; Up/Down and Prev/Next change pages. B closes the screen.
Names that exceed their available width are shortened without splitting UTF-8 characters. Empty groups and
membership lists have explicit empty states. Existing stats/leveling calculations are reused; this changes display.

## Music

Loading screens and paused character work bypassed the normal session update that replenishes music buffers.
`audioLoadingUpdate` now services the current playlist from the audio owner's thread, throttled to 100 ms and
protected against recursive pumping. It runs from engine frames, loading redraws/log events, streamed-cell joins,
and decoded-stream buffer refills. Worker threads do not access music state. Loading redraws have a reentrancy guard
because music/file logging can invoke the loading log hook. Tracks are neither restarted nor replaced by the fix.

An individual blocking SD read can still delay servicing buffers. Emulator audio verification is limited by missing
DSP firmware; a hardware listening check is required.

## Verification

Native tests with the locally available Tribunal conversion pass stats UI (28 expectations), minimap navigation
(30), corpse pause/rebuild (14), and protected save arrivals (6). Native Character, Attributes, Skills, empty Factions,
and populated/paginated Factions renders were inspected at 320x240.

UI delivery gates: Hard Gate PASS (working touch/controller tabs and pagination, values remain in bounds, no fake
stats); Purpose Gate PASS (organized character information using existing game typography, palette, and framing);
Liveliness PASS (Morrowind direction, ENERGY 1 / RHYTHM 1 / MOTION 1); Craftsmanship PASS for native compilation,
functional checks, and inspected renders. Emulator results are recorded separately in `build/*-tribunal-emu.log`.

The same Tribunal cases also pass on the actual 3DS executable in the personal checkout's isolated Azahar copy:
stats (28), minimap navigation (30), corpse rebuilding (14), and save arrivals (6). These are not audible-music tests.
