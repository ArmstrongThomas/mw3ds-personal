#ifndef _GNU_SOURCE
#define _GNU_SOURCE        // newlib: fopencookie
#endif
#include "components/files/zfile.hpp"
#include "components/files/forwardbuffer.hpp"

#include <3ds.h>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <zlib.h>

#include "components/debug/debuglog.hpp"
#include "mwsound/soundmanagerimp.hpp"

#ifdef __LARGE64_FILES
typedef _off64_t ZOff;
#else
typedef off_t ZOff;
#endif

// The optional data pack: data.pak holds the files back to back, data.idx a sorted table of
// (FNV-1a hash of the relative path, offset, size). Opening one entry in the pack avoids the FAT
// directory scan that makes the loose SD copy slow (tools/build/make_pak.py writes both files).
// The index (about 0.5 MB for the whole island) is kept in RAM: reading it from the card for every
// lookup was slower than the loose files.
struct ArchiveEntry { u64 offset = 0, size = 0; };
struct ArchiveRecord { u64 hash, offset, size; };
static_assert(sizeof(ArchiveRecord) == 24, "data.idx records are 24 bytes");
static std::string s_archiveRoot, s_archivePak;
static std::vector<ArchiveRecord> s_archive;
static FILE* s_archiveData = nullptr;
static LightLock s_archiveReadLock = 1;

static u64 archiveHash(const std::string& path)
{
	u64 h = 0xCBF29CE484222325ULL;
	for (unsigned char c : path)
	{
		h ^= c;
		h *= 0x100000001B3ULL;
	}
	return h;
}

static void archiveClose()
{
	if (s_archiveData)
		fclose(s_archiveData);
	s_archiveData = nullptr;
	s_archive.clear();
	s_archiveRoot.clear();
	s_archivePak.clear();
}

void zarchiveMount(const char* dataDir)
{
	archiveClose();
	if (!dataDir || !*dataDir)
		return;
	std::string root = dataDir;
	while (!root.empty() && (root.back() == '/' || root.back() == '\\'))
		root.pop_back();
	struct stat st;
	if (stat((root + "/data.pak").c_str(), &st) != 0)
		return;
	std::string idx = root + "/data.idx";
	FILE* f = fopen(idx.c_str(), "rb");
	if (!f)
		return;
	char head[16] = {};
	u32 version = 0, count = 0;
	bool ok = fread(head, 1, sizeof(head), f) == sizeof(head) && memcmp(head, "MWPAK1\0\0", 8) == 0;
	if (ok)
	{
		memcpy(&version, head + 8, 4);
		memcpy(&count, head + 12, 4);
		ok = version == 1 && count > 0 && count < (1u << 20);
	}
	std::vector<ArchiveRecord> records;
	if (ok)
	{
		records.resize(count);
		ok = fread(records.data(), sizeof(ArchiveRecord), records.size(), f) == records.size();
	}
	fclose(f);
	if (!ok)
		return;
	s_archive = std::move(records);
	s_archiveRoot = root;
	s_archivePak = root + "/data.pak";
	s_archiveData = fopen(s_archivePak.c_str(), "rb");
	if (!s_archiveData)
	{
		archiveClose();
		return;
	}
	logf("data pack: %s (%u entries)", s_archivePak.c_str(), (unsigned)s_archive.size());
}

bool zarchiveMounted()
{
	return !s_archive.empty();
}

static bool archiveRelative(const char* path, std::string& rel)
{
	if (s_archive.empty() || path == nullptr)
		return false;
	std::string p = path;
	if (p.size() <= s_archiveRoot.size() || p.compare(0, s_archiveRoot.size(), s_archiveRoot) != 0)
		return false;
	size_t at = s_archiveRoot.size();
	if (p[at] != '/' && p[at] != '\\')
		return false;
	rel.assign(p, at + 1, std::string::npos);
	for (char& c : rel)
		if (c == '\\')
			c = '/';
	return !rel.empty();
}

static bool archiveLookup(const char* path, ArchiveEntry& e)
{
	std::string rel;
	if (!archiveRelative(path, rel))
		return false;
	u64 want = archiveHash(rel);
	size_t lo = 0, hi = s_archive.size();
	while (lo < hi)
	{
		size_t mid = lo + (hi - lo) / 2;
		const ArchiveRecord& r = s_archive[mid];
		if (want < r.hash)
			hi = mid;
		else if (want > r.hash)
			lo = mid + 1;
		else
		{
			e.offset = r.offset;
			e.size = r.size;
			return true;
		}
	}
	return false;
}

bool zexists(const char* path)
{
	ArchiveEntry e;
	if (archiveLookup(path, e))
		return true;
	struct stat st;
	return path && stat(path, &st) == 0;
}

char* zreadAll(const char* path, size_t* sizeOut)
{
	char* raw = nullptr;
	size_t got = 0;
	ArchiveEntry entry;
	if (archiveLookup(path, entry))
	{
		// One open handle for whole-file reads (game JSON, cells): reopening the 1 GB pack for each one was
		// several hundred milliseconds at startup on the emulator.
		LightLock_Lock(&s_archiveReadLock);
		if (s_archiveData && fseek(s_archiveData, (long)entry.offset, SEEK_SET) == 0)
		{
			raw = (char*)malloc((size_t)entry.size + 1);
			if (raw)
				got = fread(raw, 1, (size_t)entry.size, s_archiveData);
		}
		LightLock_Unlock(&s_archiveReadLock);
	}
	if (!raw)
	{
		FILE* f = fopen(path, "rb");
		if (!f)
			return nullptr;
		fseek(f, 0, SEEK_END);
		long fileSize = ftell(f);
		fseek(f, 0, SEEK_SET);
		raw = (char*)malloc(fileSize + 1);
		if (!raw)
		{
			fclose(f);
			return nullptr;
		}
		got = fread(raw, 1, fileSize, f);
		fclose(f);
	}
	raw[got] = 0;
	if (got < 8 || memcmp(raw, "MWZ1", 4) != 0)
	{
		if (sizeOut)
			*sizeOut = got;
		return raw;
	}
	u32 size;
	memcpy(&size, raw + 4, 4);
	char* out = (char*)malloc(size + 1);
	uLongf outLen = size;
	int rc = out ? uncompress((Bytef*)out, &outLen, (const Bytef*)raw + 8, got - 8) : Z_MEM_ERROR;
	free(raw);
	if (rc != Z_OK || outLen != size)
	{
		logf("zfile: %s: inflate failed (%d)", path, rc);
		free(out);
		return nullptr;
	}
	out[size] = 0;
	if (sizeOut)
		*sizeOut = size;
	return out;
}

// fmemopen doesn't own its buffer: remember which buffer belongs to which stream
static std::map<FILE*, char*> s_buffers;
static LightLock s_lock = 1;         // unlocked (LightLock_Init sets 1)

#ifdef __3DS__
// On the 3DS a compressed file is inflated as it is read: a stream holds zlib's state and a small input buffer
// (~60 KB) instead of the whole inflated file. A 3 MB cell file inflated whole needed 3 MB in one block next to the
// compressed copy at the busiest moment (a cell arriving), which a fragmented Old 3DS heap couldn't give
// (zlib's -4 and the cell missing). Readers only read forward (fseek ahead is a skip)
namespace
{
struct ZStream
{
	FILE* raw;
	z_stream z;
	u32 size, pos;            // inflated size, bytes handed out
	bool end;
	unsigned char in[16 * 1024];
	ForwardReadBuffer output;
};

volatile int s_streams = 0;

size_t inflateBlock(ZStream* s, char* buf, size_t n)
{
	audioLoadingUpdate();
	if (s->end || n == 0)
		return 0;
	s->z.next_out = (Bytef*)buf;
	s->z.avail_out = n;
	while (s->z.avail_out > 0)
	{
		if (s->z.avail_in == 0)
		{
			size_t got = fread(s->in, 1, sizeof(s->in), s->raw);
			if (got == 0)
				break;
			s->z.next_in = s->in;
			s->z.avail_in = got;
		}
		int rc = inflate(&s->z, Z_NO_FLUSH);
		if (rc == Z_STREAM_END)
		{
			s->end = true;
			break;
		}
		if (rc != Z_OK)
		{
			logf("zfile: inflate failed (%d) at %lu", rc, (unsigned long)s->pos);
			s->end = true;
			break;
		}
	}
	size_t done = n - s->z.avail_out;
	return done;
}

ssize_t zstreamRead(void* cookie, char* buf, size_t n)
{
	ZStream* s = (ZStream*)cookie;
	// Keep stdio unbuffered for forward-only seeks, but batch zlib work across small field reads.
	size_t done = s->output.read(buf, n, [&](char* block, size_t size) { return inflateBlock(s, block, size); });
	s->pos += done;
	return done;
}

int zstreamSeek(void* cookie, ZOff* off, int whence)
{
	ZStream* s = (ZStream*)cookie;
	ZOff target = whence == SEEK_SET ? *off : whence == SEEK_CUR ? (ZOff)s->pos + *off : (ZOff)s->size + *off;
	if (target < (ZOff)s->pos)
		return -1;                // (never needed: the readers go forward only)
	char skip[256];
	while ((ZOff)s->pos < target)
	{
		size_t want = target - s->pos < (ZOff)sizeof(skip) ? (size_t)(target - s->pos) : sizeof(skip);
		if (zstreamRead(s, skip, want) <= 0)
			return -1;
	}
	*off = s->pos;
	return 0;
}

int zstreamClose(void* cookie)
{
	ZStream* s = (ZStream*)cookie;
	inflateEnd(&s->z);
	fclose(s->raw);
	free(s);
	LightLock_Lock(&s_lock);
	s_streams--;
	LightLock_Unlock(&s_lock);
	return 0;
}
}
#endif

#ifdef __3DS__
// A FILE over one entry of data.pak: reads stop at the entry's end, so a reader cannot run into the next file
struct ArchiveRange
{
	FILE* raw;
	u64 start, size, pos;
};

ssize_t archiveRangeRead(void* cookie, char* buf, size_t n)
{
	ArchiveRange* r = (ArchiveRange*)cookie;
	if (r->pos >= r->size || n == 0)
		return 0;
	if (n > r->size - r->pos)
		n = (size_t)(r->size - r->pos);
	if (fseek(r->raw, (long)(r->start + r->pos), SEEK_SET) != 0)
		return 0;
	size_t got = fread(buf, 1, n, r->raw);
	r->pos += got;
	return (ssize_t)got;
}

int archiveRangeSeek(void* cookie, ZOff* off, int whence)
{
	ArchiveRange* r = (ArchiveRange*)cookie;
	ZOff target = whence == SEEK_SET ? *off : whence == SEEK_CUR ? (ZOff)r->pos + *off : (ZOff)r->size + *off;
	if (target < 0)
		target = 0;
	if (target > (ZOff)r->size)
		target = (ZOff)r->size;
	r->pos = (u64)target;
	*off = target;
	return 0;
}

int archiveRangeClose(void* cookie)
{
	ArchiveRange* r = (ArchiveRange*)cookie;
	fclose(r->raw);
	free(r);
	return 0;
}

static FILE* archiveRangeOpen(const ArchiveEntry& e)
{
	FILE* raw = fopen(s_archivePak.c_str(), "rb");
	if (!raw || fseek(raw, (long)e.offset, SEEK_SET) != 0)
	{
		if (raw)
			fclose(raw);
		return nullptr;
	}
	ArchiveRange* r = (ArchiveRange*)malloc(sizeof(ArchiveRange));
	if (!r)
	{
		fclose(raw);
		return nullptr;
	}
	r->raw = raw;
	r->start = e.offset;
	r->size = e.size;
	r->pos = 0;
	cookie_io_functions_t io = { archiveRangeRead, nullptr, archiveRangeSeek, archiveRangeClose };
	FILE* f = fopencookie(r, "rb", io);
	if (!f)
	{
		archiveRangeClose(r);
		return nullptr;
	}
	setvbuf(f, nullptr, _IONBF, 0);
	return f;
}

// A raw file (loose or an archive range) as a zopen stream: inflate an MWZ1 header, else the file itself
static FILE* rawStream(FILE* raw)
{
	if (!raw)
		return nullptr;
	char head[8];
	if (fread(head, 1, 8, raw) == 8 && memcmp(head, "MWZ1", 4) == 0)
	{
		ZStream* s = (ZStream*)malloc(sizeof(ZStream));
		if (!s)
		{
			fclose(raw);
			return nullptr;
		}
		memset(&s->z, 0, sizeof(s->z));
		s->raw = raw;
		memcpy(&s->size, head + 4, 4);
		s->pos = 0;
		s->end = false;
		s->output.reset();
		if (inflateInit(&s->z) != Z_OK)
		{
			fclose(raw);
			free(s);
			return nullptr;
		}
		cookie_io_functions_t io = { zstreamRead, nullptr, zstreamSeek, zstreamClose };
		FILE* f = fopencookie(s, "rb", io);
		if (!f)
		{
			zstreamClose(s);
			return nullptr;
		}
		setvbuf(f, nullptr, _IONBF, 0);
		LightLock_Lock(&s_lock);
		s_streams++;
		LightLock_Unlock(&s_lock);
		return f;
	}
	fseek(raw, 0, SEEK_SET);
	return raw;
}
#endif

FILE* zopen(const char* path)
{
#if defined(__3DS__)
	ArchiveEntry e;
	if (archiveLookup(path, e))
	{
		FILE* raw = archiveRangeOpen(e);
		if (raw)
			return rawStream(raw);
	}
	FILE* raw = fopen(path, "rb");
	return rawStream(raw);
#else
	size_t size = 0;
	char* data = zreadAll(path, &size);
	if (!data)
		return nullptr;
	FILE* f = fmemopen(data, size ? size : 1, "rb");
	if (!f)
	{
		free(data);
		return nullptr;
	}
	LightLock_Lock(&s_lock);
	s_buffers[f] = data;
	LightLock_Unlock(&s_lock);
	return f;
#endif
}

void zclose(FILE* f)
{
	if (!f)
		return;
	fclose(f);
	LightLock_Lock(&s_lock);
	auto it = s_buffers.find(f);
	if (it != s_buffers.end())
	{
		free(it->second);
		s_buffers.erase(it);
	}
	LightLock_Unlock(&s_lock);
}

// Debug: streams opened with zopen and not yet closed
int zOpenCount()
{
	LightLock_Lock(&s_lock);
	int n = (int)s_buffers.size();
#ifdef __3DS__
	n += s_streams;
#endif
	LightLock_Unlock(&s_lock);
	return n;
}
