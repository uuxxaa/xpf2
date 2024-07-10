#pragma once
#include <string>

/**
	When using an enum class to define a set of bitflags, normal bitflag
	enum operations, such as |, ^, and &, don't work without lots of casts.
	Use this macro to define |, ^, and & for your enum class, where |, ^ and ~
	will return an enum class type, and & will evaluate to true or false.
	The implementation causes error C3281 (global operator cannot have
	managed type in signature) for managed code.
*/
#define ENUM_CLASS_FLAG_OPERATORS(TEnum)                                                                                                                    \
	constexpr TEnum operator~(TEnum a) noexcept                                                                                                        \
	{                                                                                                                                                  \
		return static_cast<TEnum>(~static_cast<std::underlying_type<TEnum>::type>(a));                                                                 \
	}                                                                                                                                                  \
	constexpr TEnum operator|(TEnum a, TEnum b) noexcept                                                                                               \
	{                                                                                                                                                  \
		return static_cast<TEnum>(static_cast<std::underlying_type<TEnum>::type>(a) | static_cast<std::underlying_type<TEnum>::type>(b));              \
	}                                                                                                                                                  \
	constexpr bool operator&(TEnum a, TEnum b) noexcept                                                                                                \
	{                                                                                                                                                  \
		return !!(static_cast<std::underlying_type<TEnum>::type>(a) & static_cast<std::underlying_type<TEnum>::type>(b));                              \
	}                                                                                                                                                  \
	constexpr TEnum operator^(TEnum a, TEnum b) noexcept                                                                                               \
	{                                                                                                                                                  \
		return static_cast<TEnum>(static_cast<std::underlying_type<TEnum>::type>(a) ^ static_cast<std::underlying_type<TEnum>::type>(b));              \
	}                                                                                                                                                  \
	inline TEnum& operator|=(TEnum& a, TEnum b) noexcept                                                                                               \
	{                                                                                                                                                  \
		return reinterpret_cast<TEnum&>(reinterpret_cast<std::underlying_type<TEnum>::type&>(a) |= static_cast<std::underlying_type<TEnum>::type>(b)); \
	}                                                                                                                                                  \
	inline TEnum& operator&=(TEnum& a, TEnum b) noexcept                                                                                               \
	{                                                                                                                                                  \
		return reinterpret_cast<TEnum&>(reinterpret_cast<std::underlying_type<TEnum>::type&>(a) &= static_cast<std::underlying_type<TEnum>::type>(b)); \
	}                                                                                                                                                  \
	inline TEnum& operator^=(TEnum& a, TEnum b) noexcept                                                                                               \
	{                                                                                                                                                  \
		return reinterpret_cast<TEnum&>(reinterpret_cast<std::underlying_type<TEnum>::type&>(a) ^= static_cast<std::underlying_type<TEnum>::type>(b)); \
	}
