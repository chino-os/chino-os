//
// Chino UEFI Gfx
//
#pragma once
#include "BootFont.hpp"
#include <stdint.h>
#include <stddef.h>
#include <array>

namespace Chino
{
	namespace UefiGfx
	{
		class GlyphProvider
		{
		public:
			GlyphProvider(BitmapFont font);

			const unsigned char* GetGlyph(uint16_t chr) const noexcept;
		private:
			void GenerateIndexer();
		private:
			BitmapFont font_;
			std::array<uint16_t, UINT16_MAX> indexes_;
		};
	}
}