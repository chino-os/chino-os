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
	context.AccessToken = context.AccessAcquired;
	OnFirstOpen();
}

void ExclusiveObjectAccess::Close(ObjectAccessContext& context)
{
	OnLastClose();
	used_.store(false, std::memory_order_relaxed);
	context.AccessToken = OA_None;
}

void ExclusiveObjectAccess::OnFirstOpen()
{

}

void ExclusiveObjectAccess::OnLastClose()
{

}

void FreeObjectAccess::Open(ObjectAccessContext& context)
{
	context.AccessToken = context.AccessAcquired;
}

void FreeObjectAccess::Close(ObjectAccessContext& context)
{
	context.AccessToken = OA_None;
}

void Chino::ValidateAccess(ObjectAccessContext& context, ObjectAccess accessRequried)
{
	kassert((context.AccessToken & accessRequried) == accessRequried);
}
