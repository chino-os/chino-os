//
// Kernel Device
//
#include "Async.hpp"

using namespace Chino;
using namespace Chino::Threading;

AsyncActionCompletionEvent::AsyncActionCompletionEvent()
	:completionEvent_(MakeObject<Event>(false, false)), isCompleted_(false)
{
}

void AsyncActionCompletionEvent::SetResult()
{
	kassert(!isCompleted_);
	isCompleted_.store(true, std::memory_order_release);
	completionEvent_->Signal();
}

void AsyncActionCompletionEvent::SetException(const std::exception_ptr& except)
{
	kassert(!isCompleted_);
	exception_ = except;
	isCompleted_.store(true, std::memory_order_release);
	completionEvent_->Signal();
}

void AsyncActionCompletionEvent::Wait()
{
	completionEvent_->Wait();
}

void AsyncActionCompletionEvent::GetResult()
{
	Wait();

	if (exception_)
		std::rethrow_exception(exception_);
}

bool AsyncActionCompletionEvent::IsCompleted()
{
	return isCompleted_.load(std::memory_order_acquire);
}
