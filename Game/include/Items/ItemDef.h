#pragma once

#include "Items/ItemHandle.h"
#include "StringHandle.h"
#include <variant>

enum class ItemType : uint8_t {
	Weapon,
	Consumable,
	Count,
};

struct WeaponData {
	uint8_t might = 0;
	uint8_t hit = 0;
	uint8_t crit = 0;
	uint8_t minRange = 0;
	uint8_t maxRange = 0;
	uint8_t maxDurability = 0;
};

struct ConsumableData {
	enum class Effect : uint8_t {
		Heal,
	};

	Effect effect = Effect::Heal;
	uint8_t amount = 0;
	uint8_t maxUses = 0;
};

struct ItemDef {
	ItemHandle id = InvalidItemHandle;
	StringHandle nameHandle = InvalidStringHandle;
	ItemType type = ItemType::Weapon;
	std::variant<WeaponData, ConsumableData> data;
};

struct ItemInstance {
	ItemHandle id = InvalidItemHandle;
	uint8_t usesLeft = 0;
};
