//
// Kernel Object
//
#pragma once
#include "Directory.hpp"
#include "../utils.hpp"

namespace Chino
{
	enum WellKnownDirectory
	{
		WKD_Root,
		WKD_Device
	};

	class ObjectManager
	{
	public:
		ObjectManager();

		Directory & GetRoot() noexcept;
		Directory & GetDirectory(WellKnownDirectory wellKnown) noexcept;
	private:
		ObjectPtr<Directory> root_;
		ObjectPtr<Directory> device_;
	};
}

extern StaticHolder<Chino::ObjectManager> g_ObjectMgr;

