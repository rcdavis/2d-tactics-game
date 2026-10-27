#include "Items/ItemDatabase.h"

#include "Items/ItemDef.h"
#include "Utils/Log.h"

#include <fstream>
#include <vector>

namespace {
	struct ItemBinHeader {
		char magic[4] {};
		uint16_t weaponCount = 0;
		uint16_t consumableCount = 0;
	};

	struct ParsedWeaponData {
		uint8_t id = 0;
		uint8_t nameId = 0;
		uint8_t might = 0;
		uint8_t hit = 0;
		uint8_t crit = 0;
		uint8_t minRange = 0;
		uint8_t maxRange = 0;
		uint8_t maxDurability = 0;
	};

	struct ParsedConsumableData {
		uint8_t id = 0;
		uint8_t nameId = 0;
		uint8_t effect = 0;
		uint8_t amount = 0;
		uint8_t maxUses = 0;
	};
}

namespace ItemDatabase {
	static std::vector<ItemDef> s_ItemDefs;

	bool Load(const char* const filePath) {
		std::ifstream file(filePath, std::ios::binary);
		if (!file) {
			LOG_ERROR("Failed to open item database file: {}", filePath);
			return false;
		}

		ItemBinHeader header;
		file.read(reinterpret_cast<char*>(&header), sizeof(header));

		if (strncmp(header.magic, "ITMB", 4) != 0) {
			LOG_ERROR("Invalid item database file: {}", filePath);
			return false;
		}

		std::vector<ParsedWeaponData> parsedWeapons(header.weaponCount);
		file.read(reinterpret_cast<char*>(parsedWeapons.data()), sizeof(ParsedWeaponData) * header.weaponCount);

		std::vector<ParsedConsumableData> parsedConsumables(header.consumableCount);
		file.read(reinterpret_cast<char*>(parsedConsumables.data()), sizeof(ParsedConsumableData) * header.consumableCount);

		s_ItemDefs.reserve(header.weaponCount + header.consumableCount);
		for (const ParsedWeaponData& weapon : parsedWeapons) {
			s_ItemDefs.push_back(ItemDef {
				.id = weapon.id,
				.nameHandle = static_cast<StringHandle>(weapon.nameId),
				.type = ItemType::Weapon,
				.data = WeaponData {
					.might = weapon.might,
					.hit = weapon.hit,
					.crit = weapon.crit,
					.minRange = weapon.minRange,
					.maxRange = weapon.maxRange,
					.maxDurability = weapon.maxDurability,
				}
			});
		}

		for (const ParsedConsumableData& consumable : parsedConsumables) {
			s_ItemDefs.push_back(ItemDef {
				.id = consumable.id,
				.nameHandle = static_cast<StringHandle>(consumable.nameId),
				.type = ItemType::Consumable,
				.data = ConsumableData {
					.effect = static_cast<ConsumableData::Effect>(consumable.effect),
					.amount = consumable.amount,
					.maxUses = consumable.maxUses,
				}
			});
		}

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
