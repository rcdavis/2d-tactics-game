#include "Items/ItemDatabase.h"

#include "Items/ItemDef.h"

#include <vector>

// HACK: Loading dummy item definitions for now.
#include "StringIds.h"

namespace ItemDatabase {
	static std::vector<ItemDef> s_ItemDefs;

	bool Load(const char* const filePath) {
		// TODO: Implement item database loading logic.
		// HACK: Loading dummy item definitions for now.
		s_ItemDefs.push_back(ItemDef {
			.id = 0,
			.nameHandle = static_cast<StringHandle>(Res::Strings::Id::IRON_LANCE),
			.type = ItemType::Weapon,
			.data = WeaponData {
				.might = 7,
				.hit = 80,
				.crit = 0,
				.minRange = 1,
				.maxRange = 1,
				.maxDurability = 45,
			}
		});

		s_ItemDefs.push_back(ItemDef {
			.id = 1,
			.nameHandle = static_cast<StringHandle>(Res::Strings::Id::IRON_BOW),
			.type = ItemType::Weapon,
			.data = WeaponData {
				.might = 6,
				.hit = 85,
				.crit = 0,
				.minRange = 2,
				.maxRange = 2,
				.maxDurability = 45,
			}
		});

		return true;
	}

	void Destroy() {
		s_ItemDefs.clear();
	}

	const ItemDef* GetItemDef(ItemHandle id) {
		return s_ItemDefs.data() + id;
	}

	ItemInstance CreateItemInstance(ItemHandle id) {
		const ItemDef* const itemDef = GetItemDef(id);

		if (const WeaponData* const weaponData = std::get_if<WeaponData>(&itemDef->data)) {
			return ItemInstance {
				.id = id,
				.usesLeft = weaponData->maxDurability
			};
		} else if (const ConsumableData* const consumableData = std::get_if<ConsumableData>(&itemDef->data)) {
			return ItemInstance {
				.id = id,
				.usesLeft = consumableData->maxUses
			};
		}

		return ItemInstance {
			.id = id,
			.usesLeft = 0
		};
	}
}
