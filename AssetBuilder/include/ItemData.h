#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct ParsedWeaponData {
	std::string name;
	uint8_t id = 0;
	uint8_t might = 0;
	uint8_t hit = 0;
	uint8_t crit = 0;
	uint8_t minRange = 0;
	uint8_t maxRange = 0;
	uint8_t maxDurability = 0;
};

struct ParsedConsumableData {
	std::string name;
	uint8_t id = 0;
	uint8_t effect = 0;
	uint8_t amount = 0;
	uint8_t maxUses = 0;
};

struct ParsedItemData {
	std::vector<ParsedWeaponData> weapons;
	std::vector<ParsedConsumableData> consumables;
};

struct WeaponData {
	uint8_t id = 0;
	uint8_t nameId = 0;
	uint8_t might = 0;
	uint8_t hit = 0;
	uint8_t crit = 0;
	uint8_t minRange = 0;
	uint8_t maxRange = 0;
	uint8_t maxDurability = 0;
};

struct ConsumableData {
	uint8_t id = 0;
	uint8_t nameId = 0;
	uint8_t effect = 0;
	uint8_t amount = 0;
	uint8_t maxUses = 0;
};

struct ItemBinHeader {
	char magic[4] {};
	uint16_t weaponCount = 0;
	uint16_t consumableCount = 0;
};

uint8_t StringToEffect(const char* const effectStr);
