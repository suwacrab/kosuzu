#include <stdint.h>
#include "kosuzu.h"

uint32_t kosuzu_hashString(const char *str) {
	uint64_t hash = 0x811C9DC4;
	for(int i=0; str[i] != 0; i++) {
		hash = ((hash ^ str[i]) * 0x1000193) & 0xFFFFFFFF;
	}
	return (uint32_t)hash;
}

