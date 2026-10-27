#include "ItemData.h"

#include <cstring>

uint8_t StringToEffect(const char* const effectStr) {
	if (strcmp(effectStr, "heal") == 0) return 0;
	return -1; // Unknown effect
}
