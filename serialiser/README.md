# Serialiser

This example demonstrates:

 * Serialisation / deserialisation, with considerations for endianness

 * Variadic templates / parameter packs

 * Parameter pack expansion

 * Consuming a variadic argument list C++-style with strong static type-checking (unlike `<stdarg.h>`).

 * Basic usage of forward-iterators

 * `std::vector`

 * `std::variant` / `std::get`

 * for-each (range-based loop)

 * How to prevent implicit conversions from causing ambiguities regarding overload resolution

 * SFINAE for having a method match by complex conditions using type-traits rather than basic pattern-matching.

 * `static_assert`

 * `if-constexpr`

 * `reinterpret_cast`

## To run

This requires your C++ compiler to support C++17.

`make run`
