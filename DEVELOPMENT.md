# Development

Read `HANDOFF.md` and the ignored `HANDOFF.local.md` first. This checkout is the personal fork; the older stylus
repository is separate. Commands below are PowerShell and run from this repository root.

## 3DS build

Install devkitPro with the 3DS toolchain. `.env` configures `MW3DS_MSYS2_DIR` on this PC.

```powershell
python tools/build/make.py
```

The executable is `mw3ds.3dsx`. Build logs and preserved binaries belong in ignored `build/`.

## Native tests

The native build uses the same game sources with platform stubs. Optional project-local tools can be installed with:

```powershell
python -m pip install --target .tools ziglang==0.16.0 psutil==7.2.2
$env:PYTHONPATH = (Resolve-Path .tools).Path
New-Item -ItemType Directory -Force .tools/tmp | Out-Null
$env:TEMP = (Resolve-Path .tools/tmp).Path
$env:TMP = $env:TEMP
$env:ZIG_GLOBAL_CACHE_DIR = Join-Path $env:TEMP 'zig-cache'
python tools/test/native/build.py
python tools/test/run_test.py personal-stats-ui --native --data ../MW3DS-Converter/work/out/data-tribunal --sd build/native/test-sd --shots build/shots/stats
```

Use this PC's already installed Python/compiler paths from `HANDOFF.local.md` if needed. A separate `--sd` folder
prevents test saves/settings leaking between runs. Native shutdown is not the console Save-and-quit path.

## Emulator tests

Use an isolated portable Azahar copy. Its config's NAND/SD paths must also point inside that copy, rather than to
the user's regular Azahar data. `.env` names this checkout's automated-test copy.

```powershell
python tools/test/run_test.py personal-stats-ui --fast --wait 180 --data ../MW3DS-Converter/work/out/data-tribunal --shots build/shots/emu-stats
```

The runner stops only its selected emulator copy. Do not target an instance the user is currently using. Tests make
an SD-data junction and do not copy the converted assets. Native/emulator FPS is not a real-console benchmark.
Audio requires DSP firmware; the current isolated copy has none. Do not redistribute firmware or game data.

## Upstream and checkpoints

```powershell
git fetch upstream
git log --oneline HEAD..upstream/main
git diff HEAD...upstream/main
```

Review changes before merging and run appropriate regressions afterward. Preserve history so later merges remain
possible. `origin` is `ArmstrongThomas/mw3ds-personal`; `git push origin main` publishes reviewed personal changes.
Never push personal changes to the old stylus repository.

Before a handoff, update the handoff/specs, record test outcomes and limitations, inspect the diff, and commit the
source/docs checkpoint locally. Keep local device/tool notes in `HANDOFF.local.md`, not tracked documentation.
