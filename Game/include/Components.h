#pragma once

#include <cstdint>
#include <variant>
#include <array>

#include "StringHandle.h"
#include "Tiles/TileHandle.h"
#include "glm/ext/vector_int2.hpp"

using ItemId = uint16_t;

constexpr ItemId InvalidItemId = static_cast<ItemId>(-1);

enum class ItemType : uint8_t {
	Weapon,
	Consumable,
	Count,
};

struct GridPosition {
	glm::ivec2 cell {0, 0};
};

struct Sprite {
	TileSetHandle tileSetHandle = InvalidTileSetHandle;
	uint16_t frameIndex = 0;
};

struct UnitExperience {
	uint8_t level = 1;
	uint8_t experience = 0;
};

struct Stats {
	int8_t curHp = 0;
	int8_t maxHp = 0;

	uint8_t strength = 0;
	uint8_t magic = 0;
	uint8_t skill = 0;
	uint8_t speed = 0;
	uint8_t luck = 0;
	uint8_t defense = 0;
	uint8_t resistance = 0;

	uint8_t movement = 0;
};

struct GrowthRates {
	uint8_t hp = 0;
	uint8_t strength = 0;
	uint8_t magic = 0;
	uint8_t skill = 0;
	uint8_t speed = 0;
	uint8_t luck = 0;
	uint8_t defense = 0;
	uint8_t resistance = 0;
};

struct PlayerTeam {};
struct EnemyTeam {};

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
		Buff,
		Debuff
	};

	Effect effect = Effect::Heal;
	uint8_t amount = 0;
	uint8_t maxUses = 0;
};

struct ItemDef {
	ItemId id = InvalidItemId;
	StringHandle nameHandle = InvalidStringHandle;
	ItemType type = ItemType::Weapon;
	std::variant<WeaponData, ConsumableData> data;
};

struct ItemInstance {
	ItemId id = InvalidItemId;
	uint8_t usesLeft = 0;
};

struct Inventory {
	static constexpr uint8_t MaxItems = 5;
	std::array<ItemInstance, MaxItems> items;
	ItemId equippedWeapon = InvalidItemId;
};
