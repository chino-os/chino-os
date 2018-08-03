//
// Kernel Device
//
#pragma once
#include "../object/Object.hpp"
#include "../threading/ThreadSynchronizer.hpp"
#include <atomic>

namespace Chino
{
	struct IAsyncAction : public virtual IObject
	{
	public:
		virtual void Wait() = 0;
		virtual void GetResult() = 0;

		virtual bool IsCompleted() = 0;
	};

	class AsyncActionCompletionEvent : public Object, public IAsyncAction
	{
	public:
		AsyncActionCompletionEvent();

		void SetResult();
		void SetException(const std::exception_ptr& except);

		virtual void Wait() override;
		virtual void GetResult() override;

		virtual bool IsCompleted() override;
	private:
		ObjectPtr<Threading::Event> completionEvent_;
		std::atomic<bool> isCompleted_;
		std::exception_ptr exception_;
	};
}
