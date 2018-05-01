//
// Kernel Utils
//
#pragma once
#include <memory>

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