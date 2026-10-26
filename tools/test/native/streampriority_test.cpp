#include <cassert>
#include <cstdio>
#include "mwworld/streampriority.hpp"

int main()
{
	assert(streamCellDistance(0, 0, 4000, 4000) == 0);
	assert(streamCellDistance(0, -1, 4000, 100) < streamCellDistance(0, 1, 4000, 100));
	assert(streamCellDistance(-1, 0, 100, 4000) < streamCellDistance(1, 0, 100, 4000));
	assert(streamCellDistance(1, 0, 8100, 4000) < streamCellDistance(-1, 0, 8100, 4000));
	assert(streamCellDistance(0, 1, 4000, 8100) < streamCellDistance(0, -1, 4000, 8100));
	assert(streamCellDistance(-1, -1, 100, 100) < streamCellDistance(1, 1, 100, 100));
	assert(streamCellDistance(-2, -2, -8192, -8192) == 0);
	puts("streampriority: nearest boundaries, corners and negative grid coordinates passed");
}
