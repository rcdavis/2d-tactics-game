#include "Localization.h"

#include <fstream>

#include "Utils/Log.h"
#include "fmt/base.h"
#include "fmt/format.h"

namespace {
	enum class PluralCategory : uint32_t {
		One,
		Other,
		Count
	};

	struct StringEntry {
		uint32_t firstForm = 0;
		uint32_t formCount = 0;
	};

	struct StringForm {
		uint32_t category = 0;
		uint32_t offset = 0;
	};

	struct StringBinHeader {
		char magic[4] = {};
		uint32_t version = 1;
		uint32_t entryCount = 0;
		uint32_t formCount = 0;
		uint32_t stringPoolSize = 0;
	};

	PluralCategory GetPluralCategory(uint32_t count) {
		return count == 1 ? PluralCategory::One : PluralCategory::Other;
	}
}

namespace Localization {
	static StringEntry* s_Entries = nullptr;
	static StringForm* s_Forms = nullptr;
	static char* s_StringPool = nullptr;

	static uint32_t s_EntryCount = 0;
	static uint32_t s_FormCount = 0;
	static uint32_t s_StringPoolSize = 0;

	bool Load(const char* const filePath) {
		std::ifstream file(filePath, std::ios::binary);
		if (!file) {
			LOG_ERROR("Failed to open localization file: {}", filePath);
			return false;
		}

		StringBinHeader header;
		file.read(reinterpret_cast<char*>(&header), sizeof(header));
		if (strncmp(header.magic, "STRB", 4) != 0) {
			LOG_ERROR("Localization file doesn't have valid magic: {}", filePath);
			return false;
		}

		Destroy();

		s_EntryCount = header.entryCount;
		s_FormCount = header.formCount;
		s_StringPoolSize = header.stringPoolSize;

		s_Entries = new StringEntry[s_EntryCount];
		s_Forms = new StringForm[s_FormCount];
		s_StringPool = new char[s_StringPoolSize];

		file.read(reinterpret_cast<char*>(s_Entries), sizeof(StringEntry) * s_EntryCount);
		file.read(reinterpret_cast<char*>(s_Forms), sizeof(StringForm) * s_FormCount);
		file.read(s_StringPool, s_StringPoolSize);

		return true;
	}

	void Destroy() {
		delete[] s_Entries;
		s_Entries = nullptr;

		delete[] s_Forms;
		s_Forms = nullptr;

		delete[] s_StringPool;
		s_StringPool = nullptr;

		s_EntryCount = 0;
		s_FormCount = 0;
		s_StringPoolSize = 0;
	}

	const char* GetString(StringHandle handle) {
		return s_StringPool + s_Forms[s_Entries[handle].firstForm].offset;
	}

	std::string GetPlural(StringHandle handle, uint32_t count) {
		const uint32_t category = static_cast<uint32_t>(GetPluralCategory(count));
		const StringEntry& entry = s_Entries[handle];

		for (uint32_t i = 0; i < entry.formCount; ++i) {
			const StringForm& form = s_Forms[entry.firstForm + i];

			if (form.category == category) {
				const char* const str = s_StringPool + form.offset;
				return fmt::format(fmt::runtime(str), fmt::arg("count", count));
			}
		}

		return "";
	}
}
