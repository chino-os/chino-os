//
// Kernel Object
//
#pragma once
#include "../kernel_iface.h"
#include <atomic>
#include <type_traits>
#include <utility>
#include <enum_flags.hpp>
#include "../kdebug.hpp"
#include "noncopyable.hpp"

namespace Chino
{
	enum ObjectAccess : size_t
	{
		OA_None = 0,
		OA_Read = 0x01,
		OA_Write = 0x02,
		OA_Execute = 0x04
	};

	MAKE_ENUM_CLASS_BITMASK_TYPE(ObjectAccess);

	struct ObjectAccessContext
	{
		union
		{
			ObjectAccess AccessToken;
			ObjectAccess AccessAcquired;
		};
	};

	struct IObjectAccess
	{
		virtual void Open(ObjectAccessContext& context) = 0;
		virtual void Close(ObjectAccessContext& context) = 0;
	};

	class ExclusiveObjectAccess : public virtual IObjectAccess
	{
	public:
		ExclusiveObjectAccess();

		virtual void Open(ObjectAccessContext& context) override;
		virtual void Close(ObjectAccessContext& context) override;
	protected:
		virtual void OnFirstOpen();
		virtual void OnLastClose();
	private:
		std::atomic<bool> used_;
	};

	class FreeObjectAccess: public virtual IObjectAccess
	{
	public:
		FreeObjectAccess();

		virtual void Open(ObjectAccessContext& context) override;
		virtual void Close(ObjectAccessContext& context) override;
	protected:
		virtual void OnFirstOpen();
		virtual void OnLastClose();
	private:
		std::atomic<long> useCount_;
	};

	class Object : public virtual IObjectAccess
	{
	public:
		Object();
		virtual ~Object();

		void AddRef() noexcept;
		bool Release() noexcept;
	private:
		std::atomic<long> refCount_;
	};

	template<class T>
	class ObjectPtr
	{
	public:
		constexpr ObjectPtr(nullptr_t = nullptr) noexcept
			:obj_(nullptr)
		{
		}

		ObjectPtr(T* obj) noexcept
			:obj_(obj)
		{
			AddRef();
		}

		ObjectPtr(const ObjectPtr& other) noexcept
			: obj_(other.obj_)
		{
			AddRef();
		}

		ObjectPtr(const ObjectPtr&& other) noexcept
			: obj_(other.obj_)
		{
			other.obj_ = nullptr;
		}

		template<class ...Args>
		ObjectPtr(std::in_place_t, Args&& ...args) noexcept
			: obj_(new T(std::forward<Args>(args)...))
		{
		}

		template<class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
		ObjectPtr(const ObjectPtr<U>& other) noexcept
			: obj_(other.obj_)
		{
			AddRef();
		}

		template<class U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
		ObjectPtr(ObjectPtr<U>&& other) noexcept
			: obj_(other.obj_)
		{
			other.obj_ = nullptr;
		}

		~ObjectPtr()
		{

		}

		void Reset(T* obj = nullptr) noexcept
		{
			if (obj != obj_)
			{
				Release();
				obj_ = obj;
				AddRef();
			}
		}

		template<class U>
		ObjectPtr<U> As() const
		{
			auto ptr = static_cast<U*>(obj_);
			kassert(!obj_ || (ptr && obj_));
			return ObjectPtr<U>(ptr);
		}

		T* Get() const noexcept { return obj_; }

		T* operator->() const noexcept { return obj_; }
		T& operator*() const noexcept { return *obj_; }

		template<class U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
		ObjectPtr& operator=(const ObjectPtr<U>& other) noexcept
		{
			Reset(other.obj_);
			return *this;
		}

		template<class U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
		ObjectPtr& operator=(ObjectPtr<U>&& other) noexcept
		{
			Reset(other.obj_);
			other.obj_ = nullptr;
			return *this;
		}

		ObjectPtr& operator=(ObjectPtr&& other) noexcept
		{
			Reset(other.obj_);
			other.obj_ = nullptr;
			return *this;
		}

		operator bool() const noexcept
		{
			return obj_;
		}

	private:
		void AddRef() noexcept
		{
			if (obj_)
				obj_->AddRef();
		}

		void Release() noexcept
		{
			if (obj_)
				obj_->Release();
			obj_ = nullptr;
		}
	private:
		template<class U>
		friend class ObjectPtr;

		T * obj_;
	};

	template<typename T, typename ...Args>
	ObjectPtr<T> MakeObject(Args &&... args)
	{
		return ObjectPtr<T>(std::in_place, std::forward<Args>(args)...);
	}

	void ValidateAccess(ObjectAccessContext& context, ObjectAccess accessRequried);

	template<class T>
	class ObjectAccessor : NonCopyable
	{
	public:
		ObjectAccessor()
		{

		}

		template<class U>
		ObjectAccessor(ObjectAccessor<U>&& other)
			:context_(std::move(other.context_)), obj_(std::move(other.obj_))
		{

		}

		ObjectAccessor(ObjectAccessContext&& context, ObjectPtr<T>&& obj) noexcept
			:context_(std::move(context)), obj_(std::move(obj))
		{
		}

		~ObjectAccessor()
		{
			if (obj_.Get())
				std::move(obj_)->Close(context_);
			context_ = {};
		}

		ObjectAccessor& operator=(ObjectAccessor&& other) noexcept
		{
			if (obj_.Get())
				obj_->Close(context_);
			obj_ = std::move(other.obj_);
			context_ = std::move(other.context_);
			return *this;
		}

		T* operator->() const noexcept
		{
			return obj_.Get();
		}

		template<class U>
		ObjectAccessor<U> MoveAs()
		{
			auto obj = obj_.template As<U>();
			obj_.Reset();
			return { std::move(context_), std::move(obj) };
		}

		void Reset() noexcept
		{
			if (obj_.Get())
				std::move(obj_)->Close(context_);
			context_ = {};
		}
	private:
		template<class U>
		friend class ObjectAccessor;

		ObjectAccessContext context_;
		ObjectPtr<T> obj_;
	};

	template<typename T>
	ObjectAccessor<T> MakeAccessor(ObjectPtr<T> obj, ObjectAccess access)
	{
		ObjectAccessContext context;
		context.AccessAcquired = OA_Read | OA_Write;
		obj->Open(context);
		return { std::move(context), std::move(obj) };
	}
}
