#pragma once

#include "Items/ItemHandle.h"

struct ItemDef;

namespace ItemDatabase {
	bool Load(const char* const filePath);

	void Destroy();

	const ItemDef* GetItemDef(ItemHandle id);
}
