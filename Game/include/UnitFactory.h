#pragma once

#include "entt/entity/fwd.hpp"
#include "glm/ext/vector_int2.hpp"
#include "Items/ItemHandle.h"

#include <span>

namespace UnitFactory {
	enum class Team : uint8_t {
		Player,
		Enemy,
	};

	entt::entity CreateArcher(
		entt::registry& registry,
		glm::ivec2 cell,
		Team team,
		std::span<const ItemHandle> inventoryItems);

	entt::entity CreateSoldier(
		entt::registry& registry,
		glm::ivec2 cell,
		Team team,
		std::span<const ItemHandle> inventoryItems);

	entt::entity CreateKnight(
		entt::registry& registry,
		glm::ivec2 cell,
		Team team,
		std::span<const ItemHandle> inventoryItems);

	entt::entity CreateCavalier(
		entt::registry& registry,
		glm::ivec2 cell,
		Team team,
		std::span<const ItemHandle> inventoryItems);
} // namespace UnitFactory
