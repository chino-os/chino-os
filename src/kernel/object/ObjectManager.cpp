//
// Kernel Object
//
#include "ObjectManager.hpp"
#include "../kdebug.hpp"

using namespace Chino;

ObjectManager::ObjectManager()
	:root_(MakeObject<Directory>()), device_(MakeObject<Directory>())
{
	root_->AddItem("Device", *device_);
}

Directory & ObjectManager::GetRoot() noexcept
{
	return *root_;
}

Directory & ObjectManager::GetDirectory(WellKnownDirectory wellKnown)
{
	switch (wellKnown)
	{
	case WKD_Root:
		return *root_;
	case WKD_Device:
		return *device_;
	default:
		kassert(!"Invalid directory.");
	}

	throw std::invalid_argument("Invalid directory.");
}
