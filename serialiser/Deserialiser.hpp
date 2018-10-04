#pragma once
#include <vector>
#include <string>
#include <stdexcept>

#include "Type.hpp"
#include "Value.hpp"

class Deserialiser
{
	std::vector<Value> out;

	const uint8_t *begin;
	const uint8_t *end;

	const uint8_t *it;

	/* Read a byte from the input, or fail if we're at end of input */
	uint8_t get()
	{
		if (it == end) {
			throw std::runtime_error("Unexpected end of input");
		}
		return *it++;
	}

	/* Read a specific data type from the input */

	uint8_t read_uint8()
	{
		return get();
	}

	uint16_t read_uint16()
	{
		return uint16_t(get()) << 8 | uint16_t(get());
	}

	uint32_t read_uint32()
	{
		return uint32_t(get()) << 24 | uint32_t(get()) << 16 | uint32_t(get()) << 8 | uint32_t(get());
	}

	uint64_t read_uint64()
	{
		return uint64_t(get()) << 56 | uint64_t(get()) << 48 | uint64_t(get()) << 40 | uint64_t(get()) << 32 | uint64_t(get()) << 24 | uint64_t(get()) << 16 | uint64_t(get()) << 8 | uint64_t(get());
	}

	bool read_bool()
	{
		return read_int8() != 0;
	}

	int8_t read_int8()
	{
		return read_uint8();
	}

	int16_t read_int16()
	{
		return read_uint16();
	}

	int32_t read_int32()
	{
		return read_uint32();
	}

	int64_t read_int64()
	{
		return read_uint64();
	}

	float read_float()
	{
		uint32_t tmp = read_uint32();
		return reinterpret_cast<float&>(tmp);
	}

	double read_double()
	{
		uint64_t tmp = read_uint64();
		return reinterpret_cast<double&>(tmp);
	}

	std::string read_string()
	{
		std::string str;
		char c;
		while ((c = get()) != 0) {
			str += c;
		}
		return str;
	}

	/* Deserialise the input stream to a list of values */

	void deserialise()
	{
		/* Assert start byte*/
		if (get() != T_START) {
			throw std::runtime_error("Missing start byte");
		}

		/* Keep reading until we find the end-byte as type */
		Type type;
		do {
			/* Read next byte as type */
			type = Type(*it++);
			switch (type) {
			case T_END:
				/* End byte */
				break;
			case T_BOOL:
				out.emplace_back(type, read_bool());
				break;
			case T_INT8:
				out.emplace_back(type, read_int8());
				break;
			case T_UINT8:
				out.emplace_back(type, read_uint8());
				break;
			case T_INT16:
				out.emplace_back(type, read_int16());
				break;
			case T_UINT16:
				out.emplace_back(type, read_uint16());
				break;
			case T_INT32:
				out.emplace_back(type, read_int32());
				break;
			case T_UINT32:
				out.emplace_back(type, read_uint32());
				break;
			case T_INT64:
				out.emplace_back(type, read_int64());
				break;
			case T_UINT64:
				out.emplace_back(type, read_uint64());
				break;
			case T_FLOAT:
				out.emplace_back(type, read_float());
				break;
			case T_DOUBLE:
				out.emplace_back(type, read_double());
				break;
			case T_STRING:
				out.emplace_back(type, read_string());
				break;
			default:
				throw std::runtime_error("Unknown type: '" + std::string(1, type) + "' at position " + std::to_string(it - begin));
			}
		} while (type != T_END);
	}

public:

	Deserialiser(const std::vector<uint8_t>& in) :
		begin(&*in.cbegin()),
		end(&*in.cend()),
		it(begin)

	{
		deserialise();
	}

	std::vector<Value>& data()
	{
		return out;
	}

	const std::vector<Value>& data() const
	{
		return out;
	}
};
