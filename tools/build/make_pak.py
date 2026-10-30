"""Packs a converted data folder into one indexed file for the SD card.

    python tools/build/make_pak.py --data out/data-tribunal

writes <data>/data.pak (the files back to back, 512-byte aligned) and <data>/data.idx (a sorted
hash -> offset/size table). The game looks every data path up in data.idx and reads it from data.pak,
instead of opening thousands of loose files: the 3DS's FAT driver scans a whole directory for each
open, which is what makes the small-CIA + SD-folder setup slow.

Audio and movies stay loose by default (few large files; they stream, and the sound code opens them
itself). Files at the data root (game*.json, maps, distant.bin) stay loose too: they are small and
opened before the first cell, and reading them from the pack made startup slower on the emulator.
--pack-root packs them as well; --all also packs sound/music/movies.
"""
import argparse
import struct
import sys
import time
from pathlib import Path

MAGIC = b"MWPAK1\0\0"
HEADER = struct.Struct("<8sII")          # magic, version, file count
RECORD = struct.Struct("<QQQ")           # path hash, offset in .pak, size
VERSION = 1
ALIGN = 512
DEFAULT_LEAVE = ("music", "movies", "sound", "snd")


def fnv1a(path: str) -> int:
    h = 0xCBF29CE484222325
    for b in path.encode("utf-8"):
        h ^= b
        h = (h * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return h


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--data", default="out/data", help="converted data folder (default out/data)")
    ap.add_argument("--out", default=None, help="the .pak to write (default <data>/data.pak)")
    ap.add_argument("--all", action="store_true", help="also pack music, movies and sound")
    ap.add_argument("--pack-root", action="store_true",
                    help="also pack files at the data root (game*.json, maps, distant.bin); by default they stay "
                         "loose, which keeps startup fast")
    ap.add_argument("--leave", default=",".join(DEFAULT_LEAVE),
                    help="comma-separated top-level folders to leave loose (default music,movies,sound,snd)")
    a = ap.parse_args()

    data = Path(a.data)
    if not data.is_dir():
        print(f"no data folder: {data}", file=sys.stderr)
        return 1
    pak = Path(a.out) if a.out else data / "data.pak"
    idx = pak.with_suffix(".idx")
    leave = set() if a.all else {x.strip() for x in a.leave.split(",") if x.strip()}

    files = []
    for p in data.rglob("*"):
        if not p.is_file():
            continue
        rel = p.relative_to(data).as_posix()
        if rel in ("data.pak", "data.idx") or p == pak or p == idx:
            continue
        if not a.pack_root and "/" not in rel:
            continue
        if leave and rel.split("/", 1)[0] in leave:
            continue
        files.append((rel, p))
    files.sort(key=lambda t: t[0])
    if not files:
        print("nothing to pack", file=sys.stderr)
        return 1

    hashes = {}
    for rel, _ in files:
        h = fnv1a(rel)
        if h in hashes:
            print(f"index hash collision: {hashes[h]} and {rel}", file=sys.stderr)
            return 1
        hashes[h] = rel

    total = 0
    t0 = time.time()
    records = []
    pak.parent.mkdir(parents=True, exist_ok=True)
    with open(pak, "wb") as out:
        for n, (rel, p) in enumerate(files, 1):
            size = p.stat().st_size
            off = out.tell()
            with open(p, "rb") as f:
                while True:
                    chunk = f.read(1 << 20)
                    if not chunk:
                        break
                    out.write(chunk)
            pad = (-size) % ALIGN
            if pad:
                out.write(b"\0" * pad)
            records.append((fnv1a(rel), off, size))
            total += size
            if n % 500 == 0 or n == len(files):
                print(f"  {n}/{len(files)} files, {total / (1 << 20):.0f} MB", flush=True)
        end = out.tell()
        if end % ALIGN:
            out.write(b"\0" * (ALIGN - end % ALIGN))

    records.sort()
    with open(idx, "wb") as f:
        f.write(HEADER.pack(MAGIC, VERSION, len(records)))
        for h, off, size in records:
            f.write(RECORD.pack(h, off, size))

    print(f"packed {len(files)} files: {pak} ({total / (1 << 20):.1f} MB), index {idx} "
          f"({idx.stat().st_size / 1024:.0f} KB) in {time.time() - t0:.0f}s")
    if leave:
        print("left loose: " + ", ".join(sorted(leave)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
