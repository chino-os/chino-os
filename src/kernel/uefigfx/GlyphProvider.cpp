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
	return indexes_[chr];
}

void GlyphProvider::GenerateIndexer()
{
	std::fill(std::begin(indexes_), std::end(indexes_), font_.Bitmap);

	auto index = font_.Index;
	auto bitmap = font_.Bitmap;
	auto height = font_.Height;
	for (size_t i = 0; i < font_.Chars; i++)
	{
		auto code = index[i];
		indexes_[code] = bitmap + i * height;
	}
}