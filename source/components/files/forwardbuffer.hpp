#pragma once
#include <cstddef>
#include <cstring>

struct ForwardReadBuffer
{
	char bytes[8192];
	size_t offset, available;

	void reset() { offset = available = 0; }

	template<class Fill>
	size_t read(char* out, size_t wanted, Fill fill)
	{
		size_t done = 0;
		while (done < wanted)
		{
			if (offset == available)
			{
				available = fill(bytes, sizeof(bytes));
				offset = 0;
				if (!available)
					break;
			}
			size_t count = wanted - done < available - offset ? wanted - done : available - offset;
			memcpy(out + done, bytes + offset, count);
			offset += count;
			done += count;
		}
		return done;
	}
};
