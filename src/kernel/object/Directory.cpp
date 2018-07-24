//
// Kernel Object
//
#include "Directory.hpp"
#include "../kdebug.hpp"

using namespace Chino;

Directory::Directory()
{
}

void Directory::AddItem(std::string_view name, IObjectAccess & obj)
{
	items_.emplace(name, &obj);
}

ObjectAccessor<IObjectAccess> Directory::Open(std::string_view name, ObjectAccess access)
{
	ObjectAccessContext context{ access };
	std::string sname(name);
	auto it = items_.find(sname);
	kassert(it != items_.end());
	auto obj = it->second;
	obj->Open(context);
	return { std::move(context), std::move(obj) };
}
