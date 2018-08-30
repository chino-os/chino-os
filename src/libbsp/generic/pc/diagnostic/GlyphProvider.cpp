#include "GlyphProvider.hpp"
#include <algorithm>

using namespace Chino::UefiGfx;

GlyphProvider::GlyphProvider(BitmapFont font)
	:font_(font)
{
	GenerateIndexer();
}

const unsigned char * GlyphProvider::GetGlyph(uint16_t chr) const noexcept
{
	auto bitmap = font_.Bitmap;
	auto height = font_.Height;
	return bitmap + indexes_[chr] * height;
}

void GlyphProvider::GenerateIndexer()
{
	std::fill(std::begin(indexes_), std::end(indexes_), 0);

	auto index = font_.Index;
	auto height = font_.Height;
	for (uint16_t i = 0; i < font_.Chars; i++)
	{
		auto code = index[i];
		indexes_[code] = i;
	}
}