#include "Items/ItemDatabase.h"

#include "Items/ItemDef.h"

#include <vector>

namespace ItemDatabase {
	static std::vector<ItemDef> s_ItemDefs;

	bool Load(const char* const filePath) {
		// TODO: Implement item database loading logic.
		return true;
	}

	void Destroy() {
		s_ItemDefs.clear();
	}

	const ItemDef* GetItemDef(ItemHandle id) {
		return s_ItemDefs.data() + id;
	}
}
