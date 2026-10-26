#include <cassert>
#include <cstdio>
#include <vector>
#include "components/files/forwardbuffer.hpp"

int main()
{
	std::vector<char> input(65539);
	for (size_t i = 0; i < input.size(); i++) input[i] = (char)(i * 37);
	size_t cursor = 0, calls = 0;
	auto fill = [&](char* out, size_t size) {
		calls++;
		size_t n = input.size() - cursor < size ? input.size() - cursor : size;
		memcpy(out, input.data() + cursor, n);
		cursor += n;
		return n;
	};
	ForwardReadBuffer buffer;
	buffer.reset();
	char field[4];
	assert(buffer.read(field, 0, fill) == 0 && calls == 0);
	for (size_t i = 0; i < 65536; i += sizeof(field))
	{
		assert(buffer.read(field, sizeof(field), fill) == sizeof(field));
		assert(memcmp(field, input.data() + i, sizeof(field)) == 0);
	}
	assert(calls == 8); // 16,384 scalar reads need eight decode blocks.
	assert(buffer.read(field, sizeof(field), fill) == 3);
	assert(memcmp(field, input.data() + 65536, 3) == 0);
	assert(buffer.read(field, 1, fill) == 0);
	cursor = calls = 0;
	buffer.reset();
	std::vector<char> large(input.size() + 100);
	assert(buffer.read(large.data(), large.size(), fill) == input.size());
	assert(memcmp(large.data(), input.data(), input.size()) == 0);
	puts("forwardbuffer: scalar reads, block boundaries, EOF, reset, and large reads passed (16384 reads / 8 fills)");
}
