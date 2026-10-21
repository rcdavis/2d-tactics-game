#pragma once

#include <string>

#include "StringHandle.h"

namespace Localization {
	bool Load(const char* const filePath);

	void Destroy();

	const char* GetString(StringHandle handle);

	std::string GetPlural(StringHandle handle, uint32_t count);
}
