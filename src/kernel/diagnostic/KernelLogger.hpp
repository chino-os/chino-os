//
// Kernel Diagnostic
//
#pragma once
#include "../utils.hpp"
#include "../kernel_iface.h"

namespace Chino
{
	namespace Diagnostic
	{
		class KernelLogger
		{
		public:
			KernelLogger(const BootParameters& bootParams);

			void PutChar(wchar_t chr);
			void PutString(const wchar_t* string);
			void PutString(const wchar_t* string, size_t count);
			void PutFormat(const wchar_t* format, ...);
			void PutString(const char* string);
			void PutString(const char* string, size_t count);
			void PutFormat(const char* format, ...);
			void DumpHex(const char* data, size_t count);

			void Clear();
			void BlueScreen();
			[[noreturn]] void FailFast(const char* message, const char* file, size_t line);
		};
	}
}

extern StaticHolder<Chino::Diagnostic::KernelLogger> g_Logger;
