#pragma once

#define MAKE_ENUM_CLASS_BITMASK_TYPE(enumName) static_assert(std::is_enum<enumName>::value, "enumName is not a enum.");\
	constexpr enumName operator|(enumName a, enumName b) noexcept\
	{\
		typedef std::underlying_type_t<enumName> underlying_type;\
		return static_cast<enumName>(static_cast<underlying_type>(a) | static_cast<underlying_type>(b));\
	}\
	constexpr enumName operator&(enumName a, enumName b) noexcept\
	{\
		typedef std::underlying_type_t<enumName> underlying_type;\
		return static_cast<enumName>(static_cast<underlying_type>(a) & static_cast<underlying_type>(b));\
	}\
	constexpr enumName operator^(enumName a, enumName b) noexcept\
	{\
		typedef std::underlying_type_t<enumName> underlying_type;\
		return static_cast<enumName>(static_cast<underlying_type>(a) ^ static_cast<underlying_type>(b));\
	}\
	constexpr enumName operator~(enumName a) noexcept\
	{\
		typedef std::underlying_type_t<enumName> underlying_type;\
		return static_cast<enumName>(~static_cast<underlying_type>(a));\
	}\
	constexpr enumName& operator|=(enumName& a, enumName b) noexcept\
	{\
		return a = (a | b);\
	}\
	constexpr enumName& operator&=(enumName& a, enumName b) noexcept\
	{\
		return a = (a & b);\
	}\
	constexpr enumName& operator^=(enumName& a, enumName b) noexcept\
	{\
		return a = (a ^ b);\
	}
