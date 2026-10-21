#include "StringData.h"

#include <cstring>

PluralCategory StringToPluralCategory(const char* const str) {
	if (strcmp(str, "one") == 0)
		return PluralCategory::One;

	return PluralCategory::Other;
}
