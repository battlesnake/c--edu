#pragma once
#include <variant>
#include <string>

struct Value
{
	using Union = std::variant<bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, float, double, std::string>;

	Type type;

	Union data;

	Value() = default;

	Value(Type type, Union data) :
		type(type),
		data(std::move(data))
	{
	}
};
