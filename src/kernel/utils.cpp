//
// Kernel Utils
//
#include "utils.hpp"
#include "kdebug.hpp"

BufferedBinaryReader::BufferedBinaryReader(uint8_t* buffer, std::function<size_t(uint8_t*)> onLoad)
	:buffer_(buffer), bufferSize_(0), bufferRead_(0), onLoad_(std::move(onLoad))
{
}

void BufferedBinaryReader::ReadBytes(uint8_t* buffer, size_t size)
{
	while (size)
	{
		auto toRead = std::min(size, bufferSize_ - bufferRead_);
		if (toRead)
		{
			memcpy(buffer, buffer_ + bufferRead_, toRead);
			buffer += toRead;
			bufferRead_ += toRead;
			size -= toRead;
		}
		else
		{
			Load();
		}
	}
}

void BufferedBinaryReader::Load()
{
	assert(bufferRead_ == bufferSize_);
	bufferSize_ = onLoad_(buffer_);
	bufferRead_ = 0;
	assert(bufferSize_);
}