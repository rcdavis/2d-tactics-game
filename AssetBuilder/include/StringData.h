#pragma once

#include <cstdint>
#include <vector>
#include <string>

enum PluralCategory : uint8_t {
	One,
	Other,
	Count,
};

struct ParsedStringForm {
	std::string text;
	PluralCategory category = PluralCategory::Other;
};

struct ParsedStringEntry {
	std::string key;
	std::vector<ParsedStringForm> forms;
};

struct StringBinaryHeader {
	char magic[4] {};
	uint32_t version = 1;
	uint32_t entryCount = 0;
	uint32_t formCount = 0;
	uint32_t stringPoolSize = 0;
};

struct StringEntry {
	uint32_t firstForm = 0;
	uint32_t formCount = 0;
};

struct StringForm {
	uint32_t category = 0;
	uint32_t offset = 0;
};
