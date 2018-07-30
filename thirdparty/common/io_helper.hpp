#pragma once
#include <cmath>

template<class T>
bool GetBit(T* address, size_t offset)
{
	using Tv = std::remove_volatile_t<T>;

	auto quot = offset / sizeof(Tv);
	auto rem = offset % sizeof(Tv);
	return ((address[quot] & (1 << rem)) >> rem) != 0;
}

template<class T>
void SetBit(T* address, size_t offset, bool value)
{
	using Tv = std::remove_volatile_t<T>;

	auto quot = offset / sizeof(Tv);
	auto rem = offset % sizeof(Tv);

	if (value)
		address[quot] |= Tv(1) << rem;
	else
		address[quot] &= ~(Tv(1) << rem);
}
