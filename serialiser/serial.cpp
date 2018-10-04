#include <vector>
#include <cstdint>
#include <cinttypes>
#include <string>
#include <cstdio>

#include "Serialiser.hpp"
#include "Deserialiser.hpp"

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;

	/* Serialise to bytes */
	printf("\nEncoding\n\n");

	Serialiser ser(42, "Hello world", true, false, false, true, 1234ULL, -1234LL, 12.34, 12.34f, 'x', "cheese", "string", '-');

	auto& raw = ser.data();

	printf("Hex: ");
	for (auto c : raw) {
		printf("%02hhx ", c);
	}
	printf("\n\n");

	printf("Raw: ");
	for (auto c : raw) {
		printf("%c", c >= 32 && c < 128 ? c : '.');
	}
	printf("\n\n");

	/* Deserialise */
	printf("\nDecoding\n\n");
	Deserialiser deser(raw);

	auto& data = deser.data();

	printf("Values:\n");
	for (auto v : data) {
		printf(" * type=%c ", char(v.type));
		switch (v.type) {
		case T_BOOL:
			printf("value=%s", std::get<bool>(v.data) ? "true" : "false");
			break;
		case T_INT8:
			printf("value=%" PRId8, std::get<int8_t>(v.data));
			break;
		case T_UINT8:
			printf("value=%" PRIu8, std::get<uint8_t>(v.data));
			break;
		case T_INT16:
			printf("value=%" PRId16, std::get<int16_t>(v.data));
			break;
		case T_UINT16:
			printf("value=%" PRIu16, std::get<uint16_t>(v.data));
			break;
		case T_INT32:
			printf("value=%" PRId32, std::get<int32_t>(v.data));
			break;
		case T_UINT32:
			printf("value=%" PRIu32, std::get<uint32_t>(v.data));
			break;
		case T_INT64:
			printf("value=%" PRId64, std::get<int64_t>(v.data));
			break;
		case T_UINT64:
			printf("value=%" PRIu64, std::get<uint64_t>(v.data));
			break;
		case T_FLOAT:
			printf("value=%f", std::get<float>(v.data));
			break;
		case T_DOUBLE:
			printf("value=%f", std::get<double>(v.data));
			break;
		case T_STRING:
			printf("value=\"%s\"", std::get<std::string>(v.data).c_str());
			break;
		default:
			printf(" ERROR! UNKNOWN TYPE '%c'", v.type);
			break;
		}
		printf("\n");
	}
	printf("\n");

	/* Done */

	return 0;
}
