//
// Chino Thread
//
#include "ProcessManager.hpp"

using namespace Chino::Thread;

template<typename T>
typename Chino::list<T>::iterator HandleToListIt(HANDLE handle)
{
	using iterator = typename Chino::list<T>::iterator;
	return iterator(reinterpret_cast<T*>(handle));
}

template<typename T>
HANDLE ToHandle(typename Chino::list<T>::iterator value)
{
	return reinterpret_cast<HANDLE>(value.node_);
}

void ProcessManager::Initialize()
{

}

HANDLE ProcessManager::CreateProcess()
{
	return 0;//return ToHandle<Process>(_processes.emplace_back());
}