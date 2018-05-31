//
// Kernel Object
//
#pragma once
#include "Object.hpp"
#include <unordered_map>
#include <string_view>
#include "noncopyable.hpp"

namespace Chino
{
	class Directory;

	template<class T>
	class ObjectAccessor : NonCopyable
	{
	public:
		template<class U>
		ObjectAccessor(ObjectAccessor<U>&& other)
			:context_(std::move(other.context_)), obj_(std::move(other.obj_))
		{

		}

		~ObjectAccessor()
		{
			if (obj_.Get())
				obj_->Close(context_);
		}

		T* operator->() const noexcept
		{
			return obj_.Get();
		}

		template<class U>
		ObjectAccessor<U> MoveAs() noexcept
		{
			return { std::move(context_), std::move(obj_).template As<U>() };
		}
	private:
		friend class Directory;
		template<class U>
		friend class ObjectAccessor;

		ObjectAccessor(ObjectAccessContext&& context, ObjectPtr<T>&& obj) noexcept
			:context_(std::move(context)), obj_(std::move(obj))
		{
		}
	private:
		ObjectAccessContext context_;
		ObjectPtr<T> obj_;
	};

	class Directory : public Object, public FreeObjectAccess
	{
	public:
		void AddItem(std::string_view name, Object& obj);
		ObjectAccessor<Object> Open(std::string_view name, ObjectAccess access);
	private:
		std::unordered_map<std::string, ObjectPtr<Object>> items_;
	};
}
