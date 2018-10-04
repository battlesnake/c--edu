#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <type_traits>

#include "Type.hpp"

class Serialiser
{
	std::vector<uint8_t> buf;

	/*
	 * Since all int types can convert to other int types automatically, and
	 * also to float/bool types, this creates an annoying problem regarding
	 * overload resolution.  We solve this by creating a useless dummy class
	 * (which the compiler will optimise away) which cannot cast
	 * automatically, in order to make overload resolution work as desired.
	 *
	 * TieBreaker<long> will not automatically cast to TieBreaker<int>, so
	 * this extra dummy parameter can be used to match types precisely even
	 * when automatic conversions exist.
	 *
	 */
	template <typename T>
	struct TieBreaker { };

	/* Append type-specifier to output stream */
	void append_type(Type c)
	{
		buf.push_back(c);
	}

	/* Append data byte to output stream */
	void append_byte(uint8_t byte)
	{
		buf.push_back(byte);
	}

	/*
	 * Helper method for appending an int, in a machine-independent
	 * byte-order (big-endian in this case), to the buffer
	 */
	template <typename T>
	void append_int(const T& arg, TieBreaker<T>)
	{
		/*
		 * This could be optimised further to reduce the number of calls
		 * to push_back.
		 */
		static constexpr auto size = sizeof(T);
		static_assert(size == 1 || size == 2 || size == 4 || size == 8, "Unsupported integer type");
		/*
		 * if-constexpr: if-statement evaluated at compile-time
		 *
		 * For more info, see:
		 *
		 * https://hackernoon.com/a-tour-of-c-17-if-constexpr-3ea62f62ff65
		 */
		/* Start writing from MSB to LSB (big-endian format) */
		if constexpr (size >= 8) {
			append_byte(arg >> 56);
			append_byte(arg >> 48);
			append_byte(arg >> 40);
			append_byte(arg >> 32);
		}
		if constexpr (size >= 4) {
			append_byte(arg >> 24);
			append_byte(arg >> 16);
		}
		if constexpr (size >= 2) {
			append_byte(arg >> 8);
		}
		append_byte(arg);
	}

	void append_one(const bool& arg, TieBreaker<bool>)
	{
		append_type(T_BOOL);
		append_byte(arg ? 1 : 0);
	}

	/*
	 * What the france is happening with this function's return type?
	 *
	 * We're using SFINAE to ensure that this overload catches all integers
	 * but only integers.
	 *
	 * For more info see:
	 *
	 * https://cpppatterns.com/patterns/function-template-sfinae.html
	 *
	 * The return type is simply "void" when the enable_if condition is true
	 * (see std::enable_if for more information).  When the condition is
	 * false (T is not an integer), then enable_if has no type member, so
	 * enable_if_t fails to specialise.
	 *
	 * Specialisation failure is not an error (SFINAE), so the compiler
	 * simply ignores this attempt to produce an overload and continues
	 * checking the other overloads until it (hopefully) matches one.
	 *
	 * If we simply had the return type as "void", then the compiler would
	 * match our signature (const T&, TieBreaker<T>) for *every* call to
	 * append_one, which would result in ambiguous overloads for e.g.
	 * std::string causing compiler errors.  When no other overloads
	 * match, the compiler would attempt to instantiate this one and would
	 * fail to.  Unlike specialisation failur, instantiation failure *is* an
	 * error.
	 */
	template <typename T>
	std::enable_if_t<std::numeric_limits<T>::is_integer>
			append_one(const T& arg, TieBreaker<T>)
	{
		static constexpr auto size = sizeof(T);
		static constexpr auto sign = std::numeric_limits<T>::is_signed;
		static_assert(size > 0 && size <= 8, "Unsupported integer type");
		/*
		 * Write out integer type specifier:
		 *  * uppercase for signed, lowercase for unsigned
		 *  * L/D/H/B for 64/32/16/8 bits
		 */
		if constexpr (size == 8) {
			append_type(sign ? T_INT64 : T_UINT64);
		}
		if constexpr (size == 4) {
			append_type(sign ? T_INT32 : T_UINT32);
		}
		if constexpr (size == 2) {
			append_type(sign ? T_INT16 : T_UINT16);
		}
		if constexpr (size == 1) {
			append_type(sign ? T_INT8 : T_UINT8);
		}
		/* Write out int */
		append_int(arg, TieBreaker<T>{});
	}

	void append_one(const double& arg, TieBreaker<double>)
	{
		using IntCast = uint64_t;
		static_assert(sizeof(IntCast) == sizeof(double), "Double is not of expected size");
		/* Cast to int so we serialise the raw IEE754 representation */
		append_type(T_DOUBLE);
		append_int(reinterpret_cast<const IntCast&>(arg), TieBreaker<IntCast>{});
	}

	void append_one(const float& arg, TieBreaker<float>)
	{
		using IntCast = uint32_t;
		static_assert(sizeof(IntCast) == sizeof(float), "Float is not of expected size");
		/* Cast to int so we serialise the raw IEE754 representation */
		append_type(T_FLOAT);
		append_int(reinterpret_cast<const IntCast&>(arg), TieBreaker<IntCast>{});
	}

	/*
	 * TieBreaker is char *, use std::decay for passing TieBreaker argument.
	 *
	 * I've done things this way so that this overload can handle both
	 * `const char *` and `char *`.
	 */
	void append_one(const char *arg, TieBreaker<char *>)
	{
		append_type(T_STRING);
		/* Null-terminated string */
		for (const char *p = arg; *p; p++) {
			append_byte(*p);
		}
		append_byte(0);
	}

	void append_one(const std::string& arg, TieBreaker<std::string>)
	{
		/*
		 * If string contains NULL, the string will be trimmed at that
		 * point.
		 */
		append_one(arg.c_str(), TieBreaker<char *>{});
	}

	void append_many()
	{
		/* Terminal, called once we've reached end of argument list */
	}

	/*
	 * Walks over argument list from left-to-right, appending each argument
	 * into the buffer.
	 *
	 * Since template programming is functional programming, we have no
	 * "for-loops" for iterating over a list.  Instead, we do it using
	 * recursion (as is the case in any purely functional language).
	 */
	template <typename Arg, typename... Args>
	inline void append_many(const Arg& arg, const Args&... args)
	{
		/* Consume leftmost argument */
		append_one(arg, TieBreaker<std::decay_t<Arg>>{});
		/* Recurse to process remaining arguments */
		append_many(args...);
	}

public:

	/*
	 * Variadic constructor
	 *
	 * Takes in variable arguments of different types
	 */
	template <typename... Args>
	Serialiser(const Args&... args)
	{
		/* Append start byte */
		append_type(T_START);
		/* Append all args */
		append_many(args...);
		/* Append end byte */
		append_type(T_END);
	}

	std::vector<uint8_t>& data()
	{
		return buf;
	}

	const std::vector<uint8_t>& data() const
	{
		return buf;
	}
};
