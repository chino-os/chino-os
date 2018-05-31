//
// Kernel Object
//
#include "Object.hpp"
#include "../kdebug.hpp"

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

void ExclusiveObjectAccess::Open(ObjectAccessContext& context)
{
	bool exp = false;
	kassert(!used_.compare_exchange_strong(exp, true, std::memory_order_relaxed));
}

void ExclusiveObjectAccess::Close(ObjectAccessContext& context)
{
	used_.store(false, std::memory_order_relaxed);
}

void FreeObjectAccess::Open(ObjectAccessContext& context)
{
}

void FreeObjectAccess::Close(ObjectAccessContext& context)
{
}
