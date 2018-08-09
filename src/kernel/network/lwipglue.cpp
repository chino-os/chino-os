//
// Chino Network
//
#include "NetworkManager.hpp"
#include "../kdebug.hpp"
#include "../threading/ThreadSynchronizer.hpp"
#include "../threading/Mailslot.hpp"
#include <lwip/sys.h>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Network;
using namespace Chino::Threading;

namespace
{
	struct arch_state
	{
		ObjectPtr<RecursiveMutex> arch_protect;
	};

	static arch_state state_;
}

extern "C"
{
	void sys_init()
	{
		state_.arch_protect = MakeObject<RecursiveMutex>();
	}

	sys_prot_t sys_arch_protect()
	{
		return state_.arch_protect->Take();
	}

	void sys_arch_unprotect(sys_prot_t)
	{
		state_.arch_protect->Give();
	}

	u32_t sys_now()
	{
		return 0;
	}

	err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
	{
		auto sysmbox = reinterpret_cast<Mailslot*>(mbox);
		gsl::span<const uint8_t> message = { reinterpret_cast<const uint8_t*>(&msg), sizeof(msg) };
		sysmbox->Send(message);
		return ERR_OK;
	}
}
