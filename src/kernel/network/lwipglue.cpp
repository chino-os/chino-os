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

	err_t sys_mbox_new(sys_mbox_t *mbox, int size)
	{
		*mbox = reinterpret_cast<sys_mbox_t>(new Mailslot());
		return ERR_OK;
	}

	err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
	{
		//g_Logger->PutFormat("POST: %p\n", msg);
		auto sysmbox = reinterpret_cast<Mailslot*>(*mbox);
		gsl::span<const uint8_t> message = { reinterpret_cast<const uint8_t*>(&msg), sizeof(msg) };
		sysmbox->Send(message);
		return ERR_OK;
	}

	void sys_mbox_post(sys_mbox_t *mbox, void *msg)
	{
		//g_Logger->PutFormat("POST: %p\n", msg);
		auto sysmbox = reinterpret_cast<Mailslot*>(*mbox);
		gsl::span<const uint8_t> message = { reinterpret_cast<const uint8_t*>(&msg), sizeof(msg) };
		sysmbox->Send(message);
	}

	err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
	{
		//g_Logger->PutFormat("POST: %p\n", msg);
		auto sysmbox = reinterpret_cast<Mailslot*>(*mbox);
		gsl::span<const uint8_t> message = { reinterpret_cast<const uint8_t*>(&msg), sizeof(msg) };
		sysmbox->Send(message);
		return ERR_OK;
	}

	u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
	{
		auto sysmbox = reinterpret_cast<Mailslot*>(*mbox);
		size_t size;
		if (!timeout)
		{
			sysmbox->Receive(size, { reinterpret_cast<uint8_t*>(msg), sizeof(*msg) });
			//g_Logger->PutFormat("RECV: %p\n", *msg);
			return ERR_OK;
		}
		else
		{
			if (sysmbox->TryReceive(size, { reinterpret_cast<uint8_t*>(msg), sizeof(*msg) }, std::chrono::milliseconds(timeout)))
			{
				//g_Logger->PutFormat("RECV: %p\n", *msg);
				return ERR_OK;
			}
			else
			{
				//g_Logger->PutString("TIMEOUT\n");
				return SYS_ARCH_TIMEOUT;
			}
		}
	}

	err_t sys_mutex_new(sys_mutex_t *mutex)
	{
		*mutex = reinterpret_cast<sys_mutex_t>(new Mutex());
		return ERR_OK;
	}

	void sys_mutex_lock(sys_mutex_t *mutex)
	{
		auto sysmutex = reinterpret_cast<Mutex*>(*mutex);
		sysmutex->Take();
	}

	void sys_mutex_unlock(sys_mutex_t *mutex)
	{
		auto sysmutex = reinterpret_cast<Mutex*>(*mutex);
		sysmutex->Give();
	}

	sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
	{
		g_Logger->PutFormat("$d\n", stacksize);
		auto systhread = g_ProcessMgr->GetCurrentThread()->GetProcess()->AddThread([=]
		{
			thread(arg);
		}, prio, stacksize);
		return reinterpret_cast<sys_thread_t>(systhread.Get());
	}
}
