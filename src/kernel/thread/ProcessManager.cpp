//
// Chino Thread
//
#include "ProcessManager.hpp"
#include "../kdebug.hpp"

using namespace Chino::Thread;

template<typename T>
typename Chino::list<T>::iterator HandleToListIt(HANDLE handle)
{
	using iterator = typename Chino::list<T>::iterator;
	return iterator(reinterpret_cast<typename Chino::list<T>::node*>(handle));
}

template<typename T>
HANDLE ToHandle(typename Chino::list<T>::iterator value)
{
	return reinterpret_cast<HANDLE>(value.node_);
}

ProcessManager::ProcessManager()
{

}

HANDLE ProcessManager::CreateProcess(std::string_view name, uintptr_t entryPoint)
{
	return ToHandle<Process>(processes_.emplace_back(name));
}

ProcessManager::Process & ProcessManager::GetProcess(HANDLE handle)
{
	kassert(handle);
	return **HandleToListIt<Process>(handle);
}

ProcessManager::Process::Process(std::string_view name)
	:name_(name)
{
}

HANDLE ProcessManager::Process::AddThread(uintptr_t entryPoint)
{
	return ToHandle<Thread>(threads_.emplace_back(entryPoint));
}

ProcessManager::Thread::Thread(uintptr_t entryPoint)
{
}
