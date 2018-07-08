//
// Kernel Object
//
#pragma once
#include "Object.hpp"
#include <unordered_map>
#include <string_view>

namespace Chino
{
	class Directory : public Object, public FreeObjectAccess
	{
	public:
		void AddItem(std::string_view name, Object& obj);
		ObjectAccessor<Object> Open(std::string_view name, ObjectAccess access);
	private:
		std::unordered_map<std::string, ObjectPtr<Object>> items_;
	};
}
