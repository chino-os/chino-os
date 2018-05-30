//
// Kernel Object
//
#include "Directory.hpp"
#include "../kdebug.hpp"

using namespace Chino;

void Directory::AddItem(std::string_view name, Object & obj)
{
	items_.emplace(name, &obj);
}

ObjectPtr<Object> Directory::GetItem(std::string_view name)
{
	return items_.at(std::string(name));
}
