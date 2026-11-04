#include "Components.h"

uint8_t Stats_CalculateDamage(const WeaponData& weapon, const Stats& attacker, const Stats& defender) {
	// TODO: Consider skill and hit percentage in damage calculation
	if (weapon.damageType == WeaponData::DamageType::Physical) {
		const int8_t damage = (attacker.strength + weapon.might) - defender.defense;
		return damage > 0 ? damage : 0;
	} else if (weapon.damageType == WeaponData::DamageType::Magical) {
		const int8_t damage = (attacker.magic + weapon.might) - defender.resistance;
		return damage > 0 ? damage : 0;
	}

	return 0;
}

bool Inventory_AddItem(Inventory& inventory, const ItemInstance& item) {
	if (inventory.itemsCount >= Inventory::MaxItems)
		return false;

	inventory.items[inventory.itemsCount++] = item;
	return true;
}

void Inventory_ConsumeItemUse(Inventory& inventory, uint8_t itemSlot) {
	if (itemSlot >= inventory.itemsCount)
		return;

	inventory.items[itemSlot].usesLeft--;
	if (inventory.items[itemSlot].usesLeft > 0)
		return;

	--inventory.itemsCount;
	for (uint8_t i = itemSlot; i < inventory.itemsCount; ++i) {
		inventory.items[i] = inventory.items[i + 1];
	}
	if (inventory.equippedWeaponSlot == itemSlot)
		inventory.equippedWeaponSlot = Inventory::UnequippedWeaponSlot;
	else if (inventory.equippedWeaponSlot > itemSlot)
		--inventory.equippedWeaponSlot;
}
