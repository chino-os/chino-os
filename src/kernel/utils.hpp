//
// Kernel Utils
//
#pragma once
#include <memory>
#include <functional>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <gsl/gsl>

template<class T>
class StaticHolder
{
public:
	~StaticHolder()
	{
		get()->~T();
	}

	template<class ...Args>
	void construct(Args&& ...args)
	{
		new (get()) T(std::forward<Args>(args)...);
	}

	T* get() noexcept
	{
		return reinterpret_cast<T*>(std::addressof(storage_));
	}

	T* operator->() noexcept
	{
		return get();
	}
private:
	std::aligned_storage_t<sizeof(T), alignof(T)> storage_;
};

class BufferedBinaryReader
{
public:
	BufferedBinaryReader(uint8_t* buffer, std::function<size_t(uint8_t*)> onLoad, size_t bufferSize = 0, size_t bufferRead = 0);

	void ReadBytes(uint8_t* buffer, size_t size);
	size_t AbandonBuffer() noexcept;
private:
	void Load();
private:
	std::function<size_t(uint8_t*)>  onLoad_;
	uint8_t * buffer_;
	size_t bufferSize_;
	size_t bufferRead_;
};
