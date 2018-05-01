//
// Chino UEFI Gfx
//
#pragma once
#include <stdint.h>
#include "../utils.hpp"
#include "GlyphProvider.hpp"

namespace Chino
{
	namespace UefiGfx
	{
		class BootVideo
		{
		public:
			BootVideo(uint32_t* frameBuffer, size_t bufferSize, size_t frameWidth, size_t frameHeight);

			void PutChar(wchar_t chr);
			void PutString(const wchar_t* string);
			void PutString(const wchar_t* string, size_t count);
			void PutFormat(const wchar_t* format, ...);
			void MovePositionTo(size_t x, size_t y);
			void ClearScreen();
			void SetBackground(uint32_t color);
		private:
			void FixCurrentFramePointer();
		private:
			GlyphProvider glyphProvider;
			uint32_t* frameBuffer, *currentFramePointer;
			size_t bufferSize;
			size_t frameWidth, frameHeight;
			size_t currentX, currentY;
			uint32_t foreground, background;
		};
	}
}

extern StaticHolder<Chino::UefiGfx::BootVideo> g_BootVideo;