//
// Kernel Device
//
#include "Async.hpp"

using namespace Chino;
using namespace Chino::Threading;

AsyncAction::AsyncAction()
	:completionEvent_(MakeObject<Event>(false, false)), isCompleted_(false)
{

}

void AsyncAction::SetResult()
{
	kassert(!isCompleted_);
	isCompleted_.store(true, std::memory_order_release);
	completionEvent_->Signal();
}

void AsyncAction::SetException(const std::exception_ptr& except)
{
	kassert(!isCompleted_);
	exception_ = except;
	isCompleted_.store(true, std::memory_order_release);
	completionEvent_->Signal();
}

void AsyncAction::Wait()
{
	completionEvent_->Wait();
}

void AsyncAction::GetResult()
{
	Wait();

	if (exception_)
		std::rethrow_exception(exception_);
}

bool AsyncAction::IsCompleted()
{
	return isCompleted_.load(std::memory_order_acquire);
}
