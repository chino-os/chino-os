//
// Kernel Diagnostic
//
#pragma once
#include "../kernel_iface.h"
#include "../utils.hpp"
#include <string_view>

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
			void PutString(std::string_view string);
			void PutFormat(const char* format, ...);
			void DumpHex(const uint8_t* data, size_t count);

			void Clear();
			void BlueScreen();
			[[noreturn]] void FailFast(const char* message, const char* file, size_t line);
		};
	}
}

extern StaticHolder<Chino::Diagnostic::KernelLogger> g_Logger;
