//
// Kernel Diagnostic
//
#include "KernelLogger.hpp"
#include <libbsp/bsp.hpp>
#include <libarch/arch.h>
#include <stdarg.h>
#include <algorithm>

using namespace Chino::Diagnostic;

KernelLogger::KernelLogger(const BootParameters& bootParams)
{
	BSPInitializeDebug(bootParams);
}

void KernelLogger::PutChar(wchar_t chr)
{
	BSPDebugPutChar(chr);
}

void KernelLogger::PutString(const wchar_t * string)
{
	while (*string)
		PutChar(*string++);
}

void KernelLogger::PutString(const wchar_t * string, size_t count)
{
	for (size_t i = 0; i < count; i++)
		PutChar(*string++);
}

void KernelLogger::PutString(const char * string)
{
	while (*string)
		PutChar(*string++);
}

void KernelLogger::PutString(std::string_view string)
{
	for(auto c : string)
		PutChar(c);
}

wchar_t tbuf[64];
wchar_t bchars[] = { L'0',L'1',L'2',L'3',L'4',L'5',L'6',L'7',L'8',L'9',L'A',L'B',L'C',L'D',L'E',L'F' };
wchar_t str[64] = { 0 };

void _itow(uint64_t i, unsigned base, wchar_t* buf) {
	int pos = 0;
	int opos = 0;
	int top = 0;

	buf[0] = '0';
	buf[1] = '\0';

	if (i == 0 || base > 16) {
		buf[0] = '0';
		buf[1] = '\0';
		return;
	}

	while (i != 0) {
		tbuf[pos] = bchars[i % base];
		pos++;
		i /= base;
	}
	top = pos--;
	for (opos = 0; opos < top; pos--, opos++) {
		buf[opos] = tbuf[pos];
	}
	buf[opos] = 0;
}

void _itow_s(int64_t i, unsigned base, wchar_t* buf) {

	if (base > 16) return;
	if (i < 0) {
		*buf++ = '-';
		i = -i;
	}
	_itow(uint64_t(i), base, buf);
}

void KernelLogger::PutFormat(const wchar_t * format, ...)
{
	if (!format)return;

	va_list		args;
	va_start(args, format);

	for (size_t i = 0; format[i]; i++) {

		switch (format[i]) {

		case L'%':

			switch (format[i + 1]) {

				/*** characters ***/
			case L'c': {
				wchar_t c = va_arg(args, int);
				PutChar(c);
				i++;		// go to next character
				break;
			}

					   /*** address of ***/
			case 's': {
				wchar_t* c = (wchar_t*)va_arg(args, wchar_t*);
				//char str[32]={0};
				//itoa(c, 16, str);
				PutString(c);
				i++;		// go to next character
				break;
			}

					  /*** integers ***/
			case 'd':
			case 'i': {
				int c = va_arg(args, int);
				_itow_s(c, 10, str);
				PutString(str);
				i++;		// go to next character
				break;
			}
			case 'l': {
				switch (format[i + 2]) {
				case 'X':
				case 'x': {
					int64_t c = va_arg(args, int64_t);
					//char str[32]={0};
					_itow((uint64_t)c, 16, str);
					PutString(str);
					i++;		// go to next character
					break;
				}
				default: {
					int64_t c = va_arg(args, int64_t);
					_itow_s(c, 10, str);
					PutString(str);
					break;
				}
				}
				i++;		// go to next character
				break;
			}

			case 'z': {
				switch (format[i + 2]) {
				case 'X':
				case 'x': {
					size_t c = va_arg(args, size_t);
					_itow((uint64_t)c, 16, str);
					PutString(str);
					i++;		// go to next character
					break;
				}
				default: {
					size_t c = va_arg(args, size_t);
					_itow_s(c, 10, str);
					PutString(str);
					break;
				}
				}
				i++;		// go to next character
				break;
			}
					  /*** display in hex ***/
			case 'X':
			case 'x': {
				int c = va_arg(args, int);
				//char str[32]={0};
				_itow((uint32_t)c, 16, str);
				PutString(str);
				i++;		// go to next character
				break;
			}

			default:
				va_end(args);
				return;
			}

			break;

		default:
			PutChar(format[i]);
			break;
		}

	}

	va_end(args);
}

void KernelLogger::PutFormat(const char * format, ...)
{
	if (!format)return;

	va_list		args;
	va_start(args, format);

	for (size_t i = 0; format[i]; i++) {

		switch (format[i]) {

		case L'%':

			switch (format[i + 1]) {

				/*** characters ***/
			case L'c': {
				char c = va_arg(args, int);
				PutChar(c);
				i++;		// go to next character
				break;
			}

					   /*** address of ***/
			case 's': {
				char* c = (char*)va_arg(args, char*);
				//char str[32]={0};
				//itoa(c, 16, str);
				PutString(c);
				i++;		// go to next character
				break;
			}

					  /*** integers ***/
			case 'd':
			case 'i': {
				int c = va_arg(args, int);
				_itow_s(c, 10, str);
				PutString(str);
				i++;		// go to next character
				break;
			}
			case 'l': {
				switch (format[i + 2]) {
				case 'X':
				case 'x': {
					int64_t c = va_arg(args, int64_t);
					//char str[32]={0};
					_itow((uint64_t)c, 16, str);
					PutString(str);
					i++;		// go to next character
					break;
				}
				default: {
					int64_t c = va_arg(args, int64_t);
					_itow_s(c, 10, str);
					PutString(str);
					break;
				}
				}
				i++;		// go to next character
				break;
			}

			case 'z': {
				switch (format[i + 2]) {
				case 'X':
				case 'x': {
					size_t c = va_arg(args, size_t);
					//char str[32]={0};
					_itow((uint64_t)c, 16, str);
					PutString(str);
					i++;		// go to next character
					break;
				}
				default: {
					size_t c = va_arg(args, size_t);
					_itow(c, 10, str);
					PutString(str);
					break;
				}
				}
				i++;		// go to next character
				break;
			}
					  /*** display in hex ***/
			case 'X':
			case 'x': {
				int c = va_arg(args, int);
				//char str[32]={0};
				_itow((uint32_t)c, 16, str);
				PutString(str);
				i++;		// go to next character
				break;
			}
			case 'P':
			case 'p': {
				uintptr_t c = va_arg(args, uintptr_t);
				//char str[32]={0};
				_itow(c, 16, str);
				PutString(str);
				i++;		// go to next character
				break;
			}

			default:
				va_end(args);
				return;
			}

			break;

		default:
			PutChar(format[i]);
			break;
		}

	}

	va_end(args);
}

void KernelLogger::Clear()
{
	BSPDebugClearScreen();
}

void KernelLogger::BlueScreen()
{
	BSPDebugBlueScreen();
}

void KernelLogger::FailFast(const char* message, const char* file, size_t line)
{
	PutFormat("Oops!\n\nAssert Failed: %s\nAt: %s:%z", message, file, line);
	ArchDisableInterrupt();
	while (1)
		ArchHaltProcessor();
}

void KernelLogger::DumpHex(const char* data, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		if (data[i] == 0)
			PutString("00 ");
		else
		{
			if (data[i] < 0x10)
				PutChar('0');
			PutFormat("%x ", data[i]);
		}
	}
}
