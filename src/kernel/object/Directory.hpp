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
		Directory();

		void AddItem(std::string_view name, IObjectAccess& obj);
		ObjectAccessor<IObjectAccess> Open(std::string_view name, ObjectAccess access);
	private:
		std::unordered_map<std::string, ObjectPtr<IObjectAccess>> items_;
	};
}
