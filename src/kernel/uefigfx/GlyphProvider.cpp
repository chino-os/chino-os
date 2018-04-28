#include "GlyphProvider.hpp"
#include <algorithm>

using namespace Chino::UefiGfx;

void GlyphProvider::Initialize(BitmapFont font)
{
	this->font = font;
	GenerateIndexer();
}

const unsigned char * GlyphProvider::GetGlyph(uint16_t chr) const noexcept
{
	return indexes[chr];
}

void GlyphProvider::GenerateIndexer()
{
	for (auto& c : indexes)
		c = font.Bitmap;

	auto index = font.Index;
	auto bitmap = font.Bitmap;
	auto height = font.Height;
	for (size_t i = 0; i < font.Chars; i++)
	{
		auto code = index[i];
		indexes[code] = bitmap + i * height;
	}
}