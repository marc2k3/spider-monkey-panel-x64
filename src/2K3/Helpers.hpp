#pragma once

static int to_int(size_t val)
{
	if (val > INT_MAX)
		return -1;
	else
		return static_cast<int>(val);
}

static uint32_t to_uint(size_t val)
{
	return static_cast<uint32_t>(val);
}

static uint32_t lengthu(auto blah)
{
	return static_cast<uint32_t>(blah.length());
}

static uint32_t sizeu(auto blah)
{
	return static_cast<uint32_t>(blah.size());
}
