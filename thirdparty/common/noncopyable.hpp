#pragma once

struct NonCopyable
{
	NonCopyable() = default;
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;

	NonCopyable(NonCopyable&&) = default;
	NonCopyable& operator=(NonCopyable&&) = default;
};

struct NonCopyOrMovable
{
	NonCopyOrMovable() = default;
	NonCopyOrMovable(const NonCopyOrMovable&) = delete;
	NonCopyOrMovable& operator=(const NonCopyOrMovable&) = delete;
};
