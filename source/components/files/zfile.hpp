#pragma once

#include <cstddef>
#include <cstdio>

// Data files may be zlib-compressed by the converter: 'MWZ1', u32 original size, zlib stream.
// These helpers read either form transparently.

// Whole file (inflated if needed), NUL-terminated; free() it. nullptr if missing or corrupt.
char* zreadAll(const char* path, size_t* size = nullptr);

// A read-only stream over the file's (inflated) contents; close with zclose.
FILE* zopen(const char* path);
void zclose(FILE* f);
int zOpenCount();                              // debug: zopen streams still open

// Optional: read data files from <dataDir>/data.pak, indexed by <dataDir>/data.idx (tools/build/make_pak.py).
// Paths under dataDir are looked up there; other paths and missing entries use the loose file. Call once at startup.
void zarchiveMount(const char* dataDir);
bool zarchiveMounted();
// True if a loose file exists or the path has an entry in the mounted data pack
bool zexists(const char* path);
