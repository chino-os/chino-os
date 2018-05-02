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
			BootVideo(uint32_t* frameBuffer, size_t bufferSize, size_t frameWidth, size_t frameHeight, uint32_t foreground = 0x00ff99ff);

			void PutChar(wchar_t chr);
			void PutString(const wchar_t* string);
			void PutString(const wchar_t* string, size_t count);
			void PutFormat(const wchar_t* format, ...);
			void MovePositionTo(size_t x, size_t y);
			void ClearScreen();
			void SetForeground(uint32_t color) { foreground_ = color; }
			void SetBackground(uint32_t color) { background_ = color; }
			void SetMargin(size_t margin) { margin_ = margin; }
		private:
			void FixCurrentFramePointer();
		private:
			GlyphProvider glyphProvider_;
			uint32_t* frameBuffer_, *currentFramePointer_;
			size_t bufferSize_;
			size_t frameWidth_, frameHeight_;
			size_t currentX_, currentY_;
			size_t margin_;
			uint32_t foreground_, background_;
		};
	}
}

extern StaticHolder<Chino::UefiGfx::BootVideo> g_BootVideo;