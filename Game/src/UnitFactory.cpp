#include "UnitFactory.h"

#include "entt/entity/registry.hpp"
#include "Components.h"
#include "TileIds.h"
#include "Items/ItemDatabase.h"

namespace UnitFactory {
	entt::entity CreateArcher(
		entt::registry& registry,
		glm::ivec2 cell,
		Team team,
		std::span<const ItemHandle> inventoryItems
	) {
		constexpr uint16_t blueArcherFrameIndex = 113;
		constexpr uint16_t redArcherFrameIndex = 120;
		constexpr TileSetHandle tileSetHandle = static_cast<TileSetHandle>(Res::Tiles::Sets::Id::Toen);

		UnitExperience unitExperience {
			.level = 1,
			.experience = 0,
		};

		Stats stats {
			.curHp = 18,
			.maxHp = 18,
			.strength = 4,
			.magic = 0,
			.skill = 6,
			.speed = 3,
			.luck = 0,
			.defense = 4,
			.resistance = 0,
			.movement = 6,
		};

		GrowthRates growthRates {
			.hp = 60,
			.strength = 40,
			.magic = 10,
			.skill = 60,
			.speed = 55,
			.luck = 30,
			.defense = 40,
			.resistance = 40,
		};

		Inventory inventory;
		for (const ItemHandle itemHandle : inventoryItems) {
			Inventory_AddItem(inventory, ItemDatabase::CreateItemInstance(itemHandle));
		}

		entt::entity unit = registry.create();
		registry.emplace<GridPosition>(unit, cell);
		registry.emplace<UnitExperience>(unit, unitExperience);
		registry.emplace<Stats>(unit, stats);
		registry.emplace<GrowthRates>(unit, growthRates);
		registry.emplace<Inventory>(unit, inventory);

		if (team == Team::Player) {
			registry.emplace<Sprite>(unit, tileSetHandle, blueArcherFrameIndex);
			registry.emplace<PlayerTeam>(unit);
		} else if (team == Team::Enemy) {
			registry.emplace<Sprite>(unit, tileSetHandle, redArcherFrameIndex);
			registry.emplace<EnemyTeam>(unit);
		}

		return unit;
	}

	entt::entity CreateSoldier(
		entt::registry& registry,
		glm::ivec2 cell,
		Team team,
		std::span<const ItemHandle> inventoryItems
	) {
		constexpr uint16_t blueSoldierFrameIndex = 114;
		constexpr uint16_t redSoldierFrameIndex = 121;
		constexpr TileSetHandle tileSetHandle = static_cast<TileSetHandle>(Res::Tiles::Sets::Id::Toen);

		UnitExperience unitExperience {
			.level = 1,
			.experience = 0,
		};

		Stats stats {
			.curHp = 20,
			.maxHp = 20,
			.strength = 4,
			.magic = 0,
			.skill = 4,
			.speed = 4,
			.luck = 0,
			.defense = 4,
			.resistance = 0,
			.movement = 6,
		};

		GrowthRates growthRates {
			.hp = 80,
			.strength = 40,
			.magic = 15,
			.skill = 50,
			.speed = 45,
			.luck = 25,
			.defense = 40,
			.resistance = 35,
		};

		Inventory inventory;
		for (const ItemHandle itemHandle : inventoryItems) {
			Inventory_AddItem(inventory, ItemDatabase::CreateItemInstance(itemHandle));
		}

		entt::entity unit = registry.create();
		registry.emplace<GridPosition>(unit, cell);
		registry.emplace<UnitExperience>(unit, unitExperience);
		registry.emplace<Stats>(unit, stats);
		registry.emplace<GrowthRates>(unit, growthRates);
		registry.emplace<Inventory>(unit, inventory);

		if (team == Team::Player) {
			registry.emplace<Sprite>(unit, tileSetHandle, blueSoldierFrameIndex);
			registry.emplace<PlayerTeam>(unit);
		} else if (team == Team::Enemy) {
			registry.emplace<Sprite>(unit, tileSetHandle, redSoldierFrameIndex);
			registry.emplace<EnemyTeam>(unit);
		}

		return unit;
	}

	entt::entity CreateKnight(
		entt::registry& registry,
		glm::ivec2 cell,
		Team team,
		std::span<const ItemHandle> inventoryItems
	) {
		constexpr uint16_t blueKnightFrameIndex = 115;
		constexpr uint16_t redKnightFrameIndex = 122;
		constexpr TileSetHandle tileSetHandle = static_cast<TileSetHandle>(Res::Tiles::Sets::Id::Toen);

		UnitExperience unitExperience {
			.level = 1,
			.experience = 0,
		};

		Stats stats {
			.curHp = 20,
			.maxHp = 20,
			.strength = 5,
			.magic = 0,
			.skill = 2,
			.speed = 0,
			.luck = 0,
			.defense = 9,
			.resistance = 0,
			.movement = 5,
		};

		GrowthRates growthRates {
			.hp = 75,
			.strength = 50,
			.magic = 20,
			.skill = 45,
			.speed = 30,
			.luck = 25,
			.defense = 60,
			.resistance = 45,
		};

		Inventory inventory;
		for (const ItemHandle itemHandle : inventoryItems) {
			Inventory_AddItem(inventory, ItemDatabase::CreateItemInstance(itemHandle));
		}

		entt::entity unit = registry.create();
		registry.emplace<GridPosition>(unit, cell);
		registry.emplace<UnitExperience>(unit, unitExperience);
		registry.emplace<Stats>(unit, stats);
		registry.emplace<GrowthRates>(unit, growthRates);
		registry.emplace<Inventory>(unit, inventory);

		if (team == Team::Player) {
			registry.emplace<Sprite>(unit, tileSetHandle, blueKnightFrameIndex);
			registry.emplace<PlayerTeam>(unit);
		} else if (team == Team::Enemy) {
			registry.emplace<Sprite>(unit, tileSetHandle, redKnightFrameIndex);
			registry.emplace<EnemyTeam>(unit);
		}

		return unit;
	}

	entt::entity CreateCavalier(
		entt::registry& registry,
		glm::ivec2 cell,
		Team team,
		std::span<const ItemHandle> inventoryItems
	) {
		constexpr uint16_t blueCavalierFrameIndex = 116;
		constexpr uint16_t redCavalierFrameIndex = 123;
		constexpr TileSetHandle tileSetHandle = static_cast<TileSetHandle>(Res::Tiles::Sets::Id::Toen);

		UnitExperience unitExperience {
			.level = 1,
			.experience = 0,
		};

		Stats stats {
			.curHp = 20,
			.maxHp = 20,
			.strength = 5,
			.magic = 0,
			.skill = 2,
			.speed = 5,
			.luck = 0,
			.defense = 6,
			.resistance = 0,
			.movement = 8,
		};

		GrowthRates growthRates {
			.hp = 70,
			.strength = 45,
			.magic = 15,
			.skill = 40,
			.speed = 50,
			.luck = 30,
			.defense = 45,
			.resistance = 40,
		};

		Inventory inventory;
		for (const ItemHandle itemHandle : inventoryItems) {
			Inventory_AddItem(inventory, ItemDatabase::CreateItemInstance(itemHandle));
		}

		entt::entity unit = registry.create();
		registry.emplace<GridPosition>(unit, cell);
		registry.emplace<UnitExperience>(unit, unitExperience);
		registry.emplace<Stats>(unit, stats);
		registry.emplace<GrowthRates>(unit, growthRates);
		registry.emplace<Inventory>(unit, inventory);

		if (team == Team::Player) {
			registry.emplace<Sprite>(unit, tileSetHandle, blueCavalierFrameIndex);
			registry.emplace<PlayerTeam>(unit);
		} else if (team == Team::Enemy) {
			registry.emplace<Sprite>(unit, tileSetHandle, redCavalierFrameIndex);
			registry.emplace<EnemyTeam>(unit);
		}

		return unit;
	}
} // namespace UnitFactory
