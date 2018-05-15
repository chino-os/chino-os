#include "BootVideo.hpp"

using namespace Chino::UefiGfx;

// these vectors together act as a corner of a bounding rect
// This allows GotoXY() to reposition all the text that follows it
static unsigned int _xPos = 0, _yPos = 0;
static unsigned _startX = 0, _startY = 0;

// current color
static unsigned _color = 0;

BootVideo::BootVideo(uint32_t * frameBuffer, size_t bufferSize, size_t frameWidth, size_t frameHeight, uint32_t foreground)
	:glyphProvider_(GetBootFont()), frameBuffer_(frameBuffer), bufferSize_(bufferSize), frameWidth_(frameWidth), frameHeight_(frameHeight), foreground_(foreground), background_(0xFF000000)
{
}

void BootVideo::PutChar(wchar_t chr)
{
	auto& font = GetBootFont();
	if (chr == L'\r')
		currentX_ = margin_;
	else if (chr == L'\n')
	{
		currentX_ = margin_;
		currentY_ += font.Height;
	}
	else
	{
		int cx, cy;
		const unsigned char *glyph = glyphProvider_.GetGlyph(chr);
		auto pixel = currentFramePointer_;

		auto startPixel = pixel;
		for (cy = 0; cy < font.Height; cy++, glyph++)
		{
			unsigned char line = *glyph;
			auto pixel = startPixel;
			for (cx = 0; cx < font.Width; cx++)
			{
				*pixel++ = (line & 0x80u) ? foreground_ : background_;
				line <<= 1;
			}
			startPixel += frameWidth_;
		}
		currentX_ += font.Width;
		if (currentX_ + margin_ >= frameWidth_)
		{
			currentX_ = margin_;
			currentY_ += font.Height;
		}
	}
	if (currentY_ + font.Height >= frameHeight_)
	{
		ClearScreen();
		currentY_ = margin_;
	}
	FixCurrentFramePointer();
}

void BootVideo::MovePositionTo(size_t x, size_t y)
{
	currentX_ = std::min(x + margin_, frameWidth_ - 1 - margin_);
	currentY_ = std::min(y + margin_, frameHeight_ - 1 - margin_);
	FixCurrentFramePointer();
}

void BootVideo::ClearScreen()
{
	auto end = (uint32_t*)((uint8_t*)frameBuffer_ + bufferSize_) + 1;
	for (auto pixel = frameBuffer_; pixel < end; pixel++)
		*pixel = background_;
	MovePositionTo(0, 0);
}

void BootVideo::FixCurrentFramePointer()
{
	currentFramePointer_ = frameBuffer_ + currentY_ * frameWidth_ + currentX_;
}