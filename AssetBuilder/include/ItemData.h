#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct WeaponData {
	std::string name;
	uint8_t id = 0;
	uint8_t might = 0;
	uint8_t hit = 0;
	uint8_t crit = 0;
	uint8_t minRange = 0;
	uint8_t maxRange = 0;
	uint8_t maxDurability = 0;
};

struct ConsumableData {
	std::string name;
	uint8_t id = 0;
	uint8_t effect = 0;
	uint8_t amount = 0;
	uint8_t maxUses = 0;
};

struct ItemData {
	std::vector<WeaponData> weapons;
	std::vector<ConsumableData> consumables;
};

uint8_t StringToEffect(const char* const effectStr);
