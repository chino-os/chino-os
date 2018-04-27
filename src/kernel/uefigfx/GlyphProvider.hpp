//
// Chino UEFI Gfx
//
#pragma once
#include "BootFont.hpp"
#include <stdint.h>
#include <stddef.h>

namespace Chino
{
	namespace UefiGfx
	{
		class GlyphProvider
		{
		public:
			GlyphProvider() = default;

			void Initialize(BitmapFont font);
			const unsigned char* GetGlyph(uint16_t chr) const noexcept;
		private:
			void GenerateIndexer();
		private:
			BitmapFont font;
			const unsigned char* indexes[UINT16_MAX];
		};
	}
}