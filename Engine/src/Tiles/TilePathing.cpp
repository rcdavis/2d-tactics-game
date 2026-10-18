#include "Tiles/TilePathing.h"

#include "Tiles/TileSystem.h"
#include "Tiles/TileMap.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <limits>
#include <queue>
#include <tuple>
#include <utility>

TilePathing::~TilePathing() {
	Destroy();
}

void TilePathing::Init(TileMapHandle tileMapHandle) {
	const TileMap* const tileMap = TileSystem::GetTileMap(tileMapHandle);
	const TileSet* const tileSet = TileSystem::GetTileSet(tileMap->tileSetHandle);

	mRowCount = tileMap->tileRowCount;
	mColumnCount = tileMap->tileColumnCount;

	mCells = new Cell[mRowCount * mColumnCount];
	for (uint16_t row = 0; row < mRowCount; ++row) {
		for (uint16_t col = 0; col < mColumnCount; ++col) {
			for (int layer = tileMap->layerCount - 1; layer >= 0; --layer) {
				const uint16_t tileId = tileMap->layers[layer].tiles[row * mColumnCount + col];
				if (tileId == 0)
					continue;

				mCells[row * mColumnCount + col] = {
					.row = row,
					.col = col,
					.movementCost = tileSet->terrains[tileId - 1].movementCost, // tile IDs in the map are 1-based (0 = empty)
				};
				break;
			}
		}
	}
}

void TilePathing::Destroy() {
	delete[] mCells;
	mCells = nullptr;

	mRowCount = 0;
	mColumnCount = 0;
}

std::vector<TilePathing::Cell> TilePathing::FindPath(uint16_t startRow, uint16_t startCol, uint16_t endRow, uint16_t endCol) {
	if (mCells == nullptr || startRow >= mRowCount || startCol >= mColumnCount ||
		endRow >= mRowCount || endCol >= mColumnCount) {
		return {};
	}

	const uint32_t cellCount = static_cast<uint32_t>(mRowCount) * mColumnCount;
	const uint32_t startIndex = static_cast<uint32_t>(startRow) * mColumnCount + startCol;
	const uint32_t endIndex = static_cast<uint32_t>(endRow) * mColumnCount + endCol;
	if (mCells[startIndex].movementCost == 0 || mCells[endIndex].movementCost == 0)
		return {};

	constexpr uint16_t Unreachable = std::numeric_limits<uint16_t>::max();
	constexpr uint32_t NoParent = std::numeric_limits<uint32_t>::max();
	std::vector<uint16_t> distances(cellCount, Unreachable);
	std::vector<uint32_t> parents(cellCount, NoParent);

	using QueueEntry = std::tuple<uint32_t, uint16_t, uint32_t>;
	std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry>> queue;

	distances[startIndex] = 0;
	queue.emplace(Heuristic(startRow, startCol, endRow, endCol), 0, startIndex);

	const auto relaxNeighbor = [&](uint16_t row, uint16_t col, uint32_t currentIndex, uint16_t currentDistance) {
		const uint32_t neighborIndex = static_cast<uint32_t>(row) * mColumnCount + col;
		const uint8_t movementCost = mCells[neighborIndex].movementCost;
		if (movementCost == 0 || currentDistance > Unreachable - movementCost)
			return;

		const uint16_t newDistance = currentDistance + movementCost;
		if (newDistance >= distances[neighborIndex])
			return;

		distances[neighborIndex] = newDistance;
		parents[neighborIndex] = currentIndex;
		queue.emplace(
			static_cast<uint32_t>(newDistance) + Heuristic(row, col, endRow, endCol),
			newDistance,
			neighborIndex
		);
	};

	while (!queue.empty()) {
		const uint16_t distance = std::get<1>(queue.top());
		const uint32_t currentIndex = std::get<2>(queue.top());
		queue.pop();

		if (distance != distances[currentIndex])
			continue;

		if (currentIndex == endIndex)
			break;

		const uint16_t row = static_cast<uint16_t>(currentIndex / mColumnCount);
		const uint16_t col = static_cast<uint16_t>(currentIndex % mColumnCount);
		if (row > 0)
			relaxNeighbor(row - 1, col, currentIndex, distance);
		if (col > 0)
			relaxNeighbor(row, col - 1, currentIndex, distance);
		if (col + 1 < mColumnCount)
			relaxNeighbor(row, col + 1, currentIndex, distance);
		if (row + 1 < mRowCount)
			relaxNeighbor(row + 1, col, currentIndex, distance);
	}

	if (distances[endIndex] == Unreachable)
		return {};

	std::vector<Cell> path;
	for (size_t index = endIndex; index != NoParent; index = parents[index]) {
		path.push_back(mCells[index]);
		if (index == startIndex)
			break;
	}

	if (path.back().row != startRow || path.back().col != startCol)
		return {};

	std::reverse(path.begin(), path.end());
	return path;
}

std::vector<TilePathing::Cell> TilePathing::FindMovementZone(uint16_t startRow, uint16_t startCol, uint8_t movementRange) {
	if (mCells == nullptr || startRow >= mRowCount || startCol >= mColumnCount)
		return {};

	const uint32_t startIndex = static_cast<uint32_t>(startRow) * mColumnCount + startCol;
	if (mCells[startIndex].movementCost == 0)
		return {};

	constexpr uint32_t Unreachable = std::numeric_limits<uint32_t>::max();
	std::vector<uint32_t> distances(static_cast<size_t>(mRowCount) * mColumnCount, Unreachable);

	using QueueEntry = std::pair<uint32_t, uint32_t>; // (distance, cell index)
	std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry>> queue;

	distances[startIndex] = 0;
	queue.emplace(0, startIndex);

	const auto relaxNeighbor = [&](uint16_t row, uint16_t col, uint32_t currentDistance) {
		const uint32_t neighborIndex = static_cast<uint32_t>(row) * mColumnCount + col;
		const uint8_t movementCost = mCells[neighborIndex].movementCost;
		if (movementCost == 0)
			return;

		const uint32_t newDistance = currentDistance + movementCost;
		if (newDistance > movementRange || newDistance >= distances[neighborIndex])
			return;

		distances[neighborIndex] = newDistance;
		queue.emplace(newDistance, neighborIndex);
	};

	std::vector<Cell> zone;
	while (!queue.empty()) {
		const auto [distance, currentIndex] = queue.top();
		queue.pop();

		if (distance != distances[currentIndex])
			continue;

		zone.push_back(mCells[currentIndex]);

		const uint16_t row = static_cast<uint16_t>(currentIndex / mColumnCount);
		const uint16_t col = static_cast<uint16_t>(currentIndex % mColumnCount);
		if (row > 0)
			relaxNeighbor(row - 1, col, distance);
		if (col > 0)
			relaxNeighbor(row, col - 1, distance);
		if (col + 1 < mColumnCount)
			relaxNeighbor(row, col + 1, distance);
		if (row + 1 < mRowCount)
			relaxNeighbor(row + 1, col, distance);
	}

	return zone;
}

uint32_t TilePathing::Heuristic(uint16_t startRow, uint16_t startCol, uint16_t endRow, uint16_t endCol) const {
	const int32_t dRow = static_cast<int32_t>(endRow) - static_cast<int32_t>(startRow);
	const int32_t dCol = static_cast<int32_t>(endCol) - static_cast<int32_t>(startCol);
	return static_cast<uint32_t>(std::abs(dRow) + std::abs(dCol));
}
