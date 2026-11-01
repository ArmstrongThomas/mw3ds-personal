# MW3DS Personal

Personal development checkout: `mw3ds-personal`.
Work continues on this repository's `main` branch. It includes the stylus controls and the subsequent loading,
performance, minimap, and stats changes. The published `ArmstrongThomas/mw3ds` stylus implementation is separate.

The `upstream` remote points to `https://github.com/3ds-conspiracy/mw3ds.git`. This checkout retains the original
Git history so upstream changes can be fetched, reviewed, and merged. It currently includes upstream through
`266176f` (0.2.3). The personal publishing remote is `origin`:
`https://github.com/ArmstrongThomas/mw3ds-personal.git`.

## Personal versions

Upstream versions are `major.minor.micro`. This fork adds a lowercase release letter: the first personal release on
top of upstream 0.2.3 is **0.2.3.a**, the next patch before another upstream merge is **0.2.3.b**, and so on. Keep
`VERSION.txt`, the Git tag, the converter zip and the container image tag in step (`0.2.3.a`); the CIA's numeric title
version remains 0.2.3 because the 3DS title field has no letter.

This is a separate GitHub repository, retaining shared Git history and `upstream` for normal merges. It does not
occupy or change the existing stylus fork's GitHub relationship.

## Current changes

- Optional left/right-handed stylus aiming, mirrored live minimap, effect tooltips, and independent local/world
  minimap zoom, pan, and follow controls.
- Buffered compressed reads, exterior streaming priorities, protected arrival loading, and staged character updates.
- Original loading splash art available before world loading; full-screen progress stays on the bottom display.
- Music buffers continue being serviced during loading, decompression, stream waits, and paused rebuilds.
- Tabbed stats: Character, Attributes, Skills, Factions. Skills have progress bars and group/page controls; factions
  retain ranks, reputation, and expelled status. L/R changes tabs; the skill page uses Left/Right for groups and
  Up/Down for pages. Touch controls and B to close are supported.

`spec/performance-new3ds.md` records previous testing and hardware feedback. Automated audio playback verification
is limited by missing DSP firmware in the test emulator; the loading-music fix still needs a listening check on 3DS.

## Build

The local, ignored `.env` selects the devkitPro/MSYS2 installation. With Python available:

```powershell
python tools/build/make.py
```

The result is `mw3ds.3dsx` in this folder. Converted game assets remain outside Git. Local Tribunal data is at
`..\MW3DS-Converter\work\out\data-tribunal`; Bloodmoon + Tribunal data is at
`out\data-tribunal-bloodmoon`. Testing must distinguish them from the older vanilla data.
