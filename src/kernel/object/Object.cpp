//
// Kernel Object
//
#include "Object.hpp"

using namespace Chino;

Object::Object()
	:refCount_(1)
{
}

Object::~Object()
{
}

void Object::AddRef() noexcept
{
	refCount_.fetch_add(1, std::memory_order_relaxed);
}

bool Object::Release() noexcept
{
	if (refCount_.fetch_sub(1, std::memory_order_relaxed) == 1)
	{
		delete this;
		return true;
	}

	return false;
}
