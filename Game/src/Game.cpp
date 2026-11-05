#include "Game.h"

#include "Items/ItemDef.h"
#include "StringHandle.h"
#include "Utils/Log.h"

#include "PlatformEvent.h"
#include "IWindow.h"
#include "Renderer/IRenderDevice.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/TextureSystem.h"
#include "Renderer/ShaderSystem.h"
#include "Renderer/FontSystem.h"
#include "Tiles/TileSystem.h"
#include "Tiles/TileMap.h"
#include "Input/Input.h"

#include "Timer.h"
#include "TextureIds.h"
#include "ShaderIds.h"
#include "TileIds.h"
#include "Components.h"
#include "Rng.h"
#include "Localization.h"
#include "StringIds.h"
#include "ItemIds.h"
#include "Items/ItemDatabase.h"
#include "glm/ext/vector_float2.hpp"
#include "UnitFactory.h"

#include <filesystem>

namespace {
	enum class ActionMenuOption : uint8_t {
		Attack,
		Item,
		Wait,
		Count
	};

	constexpr const char* ItemBinPath = "res/items/itemdb.itmbin";
	constexpr const char* FontJsonPath = "res/fonts/MyFont.json";
	constexpr const char* StringBinPath = "res/strings/en.strbin";
	constexpr const char* PrefPathOrgName = "rcdavis";
	constexpr const char* PrefPathAppName = "2d-tactics-game";
	constexpr const char* InputBindingsFile = "bindings.json";

	constexpr ShaderHandle QuadShaderHandle = static_cast<ShaderHandle>(Res::Shaders::Vertex::Id::TextureQuad);

	constexpr TextureHandle SelectionRingTextureHandle = static_cast<TextureHandle>(Res::Textures::Id::SelectionRing);
	constexpr TextureHandle FontTexture = static_cast<TextureHandle>(Res::Textures::Id::MyFont);

	constexpr TileMapHandle ToenTileMapHandle = static_cast<TileMapHandle>(Res::Tiles::Maps::Id::Toen);
	constexpr TileSetHandle ToenTileSetHandle = static_cast<TileSetHandle>(Res::Tiles::Sets::Id::Toen);

	constexpr ItemHandle IronBowItemHandle = static_cast<ItemHandle>(Res::Items::Id::IRON_BOW);
	constexpr ItemHandle IronLanceItemHandle = static_cast<ItemHandle>(Res::Items::Id::IRON_LANCE);
	constexpr ItemHandle VulneraryItemHandle = static_cast<ItemHandle>(Res::Items::Id::VULNERARY);

	constexpr float KeyDownInterval = 0.1f;
}

Game::~Game() {
	Shutdown();
}

bool Game::Init() {
	constexpr WindowCreateInfo windowCreateInfo {
		.title = "2D Tactics Game",
		.width = 800,
		.height = 600,
		.graphicsAPI = GraphicsAPI::OpenGL,
		.isResizable = false,
		.isFullscreen = false,
		.useVSync = true,
	};

	if (!mPlatform.Init(windowCreateInfo)) {
		LOG_ERROR("Failed to initialize platform");
		return false;
	}

	if (!TextureSystem::Init(mPlatform.renderDevice)) {
		LOG_ERROR("Failed to initialize texture system");
		return false;
	}

	if (!ShaderSystem::Init(mPlatform.renderDevice)) {
		LOG_ERROR("Failed to initialize shader system");
		return false;
	}

	for (const char* path : Res::Textures::Paths) {
		if (TextureSystem::Load(path) == InvalidTextureHandle) {
			LOG_ERROR("Failed to load texture: {}", path);
			return false;
		}
	}

	if (!TileSystem::Init()) {
		LOG_ERROR("Failed to initialize tile system");
		return false;
	}

	for (const char* path : Res::Tiles::Sets::Paths) {
		if (TileSystem::LoadTileSet(path) == InvalidTileSetHandle) {
			LOG_ERROR("Failed to load tile set: {}", path);
			return false;
		}
	}

	for (const char* path : Res::Tiles::Maps::Paths) {
		if (TileSystem::LoadTileMap(path) == InvalidTileMapHandle) {
			LOG_ERROR("Failed to load tile map: {}", path);
			return false;
		}
	}

	constexpr uint32_t numShaderPaths = static_cast<uint32_t>(Res::Shaders::Vertex::Paths.size());
	for (uint32_t i = 0; i < numShaderPaths; ++i) {
		const char* vs = Res::Shaders::Vertex::GetPath(static_cast<Res::Shaders::Vertex::Id>(i));
		const char* fs = Res::Shaders::Fragment::GetPath(static_cast<Res::Shaders::Fragment::Id>(i));
		if (ShaderSystem::Load(vs, fs) == InvalidShaderHandle) {
			LOG_ERROR("Failed to load shader: {} {}", vs, fs);
			return false;
		}
	}

	Rng::Init();

	if (!ItemDatabase::Load(ItemBinPath)) {
		LOG_ERROR("Failed to initialize item database");
		return false;
	}

	mCamera.Init(windowCreateInfo.width, windowCreateInfo.height);

	if (!Renderer2D::Init(mPlatform.renderDevice, QuadShaderHandle)) {
		LOG_ERROR("Failed to initialize Renderer2D");
		return false;
	}

	mPlatform.renderDevice->SetClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	mPlatform.renderDevice->EnableDepthTest(true);

	const std::filesystem::path preferencesPath = mPlatform.GetPreferencesPath(PrefPathOrgName, PrefPathAppName);
	mActionMap = ActionMap::LoadBindings(preferencesPath / InputBindingsFile);

	mTimeSinceKeyPressed = 0.0f;

	mTilePathing.Init(ToenTileMapHandle);

	UnitFactory::CreateArcher(
		mRegistry,
		{7, 8},
		UnitFactory::Team::Player,
		std::array<ItemHandle, 2>{IronBowItemHandle, VulneraryItemHandle});
	UnitFactory::CreateSoldier(
		mRegistry,
		{0, 0},
		UnitFactory::Team::Player,
		std::array<ItemHandle, 2>{IronLanceItemHandle, VulneraryItemHandle});
	UnitFactory::CreateKnight(
		mRegistry,
		{12, 12},
		UnitFactory::Team::Player,
		std::array<ItemHandle, 2>{IronLanceItemHandle, VulneraryItemHandle});
	UnitFactory::CreateCavalier(
		mRegistry,
		{5, 5},
		UnitFactory::Team::Player,
		std::array<ItemHandle, 2>{IronLanceItemHandle, VulneraryItemHandle});

	UnitFactory::CreateArcher(
		mRegistry,
		{20, 8},
		UnitFactory::Team::Enemy,
		std::array<ItemHandle, 1>{IronBowItemHandle});
	UnitFactory::CreateSoldier(
		mRegistry,
		{22, 0},
		UnitFactory::Team::Enemy,
		std::array<ItemHandle, 1>{IronLanceItemHandle});
	UnitFactory::CreateKnight(
		mRegistry,
		{22, 22},
		UnitFactory::Team::Enemy,
		std::array<ItemHandle, 1>{IronLanceItemHandle});
	UnitFactory::CreateCavalier(
		mRegistry,
		{25, 5},
		UnitFactory::Team::Enemy,
		std::array<ItemHandle, 1>{IronLanceItemHandle});

	mState = State::SelectCharacter;
	mSelectedCharacter = entt::null;
	mCursorPos = {0, 0};

	if (!Localization::Load(StringBinPath)) {
		LOG_ERROR("Failed to initialize localization");
		return false;
	}

	mFontHandle = FontSystem::Load(FontJsonPath, FontTexture);
	if (mFontHandle == InvalidFontHandle) {
		LOG_ERROR("Failed to load font: {}", FontJsonPath);
		return false;
	}

	mIsRunning = true;

	LOG_INFO("Game initialized successfully");

	return true;
}

void Game::Shutdown() {
	LOG_INFO("Shutting down game");

	ItemDatabase::Destroy();
	Localization::Destroy();

	mTilePathing.Destroy();

	mRegistry.clear();
	mSelectedCharacter = entt::null;

	Renderer2D::Shutdown();
	TileSystem::Shutdown();
	TextureSystem::Shutdown();
	ShaderSystem::Shutdown();

	mPlatform.Destroy();

	mIsRunning = false;
}

void Game::Run() {
	if (!Init()) {
		LOG_ERROR("Failed to initialize game");
		return;
	}

	Timer timer;
	timer.Init();

	PlatformEvent event {};
	while (mIsRunning) {
		Input::BeginFrame();

		while (mPlatform.PollEvent(event)) {
			if (event.type == PlatformEvent::Type::Quit) {
				Close();
				break;
			}

			Input::HandleEvent(event);

			switch (event.type) {
			case PlatformEvent::Type::WindowPixelSizeChanged:
				OnResize(event.windowSize.width, event.windowSize.height);
				break;
			default:
				break;
			}
		}
		mActionMap.Update();

		Update(timer.GetElapsedTime());

		Render();
	}
}

void Game::Update(const float elapsedTime) {
	switch (mState) {
	case State::SelectCharacter:
		UpdateSelectCharacterState(elapsedTime);
		break;

	case State::MovementZone:
		UpdateMovementZoneState(elapsedTime);
		break;

	case State::MovingCharacter:
		UpdateMovingCharacterState(elapsedTime);
		break;

	case State::ActionSelectMenu:
		UpdateActionSelectMenuState(elapsedTime);
		break;

	case State::ActionAttackSelectMenu:
		UpdateActionAttackSelectMenuState(elapsedTime);
		break;

	case State::ActionItemSelectMenu:
		UpdateActionItemSelectMenuState(elapsedTime);
		break;

	case State::StatsMenu:
		UpdateStatsMenuState(elapsedTime);
		break;
	}

	if (mActionMap.IsPressed(Action::Pause)) {
		// TODO: Implement pause menu functionality.
		Close();
	}
}

void Game::UpdateSelectCharacterState(const float elapsedTime) {
	UpdateCursor(elapsedTime);
	FocusCamera(mCursorPos);

	if (mActionMap.IsPressed(Action::OpenStatsMenu)) {
		auto view = mRegistry.view<GridPosition>();
		for (auto entity : view) {
			const auto& gridPosition = view.get<GridPosition>(entity);
			if (gridPosition.cell.x == mCursorPos.x && gridPosition.cell.y == mCursorPos.y) {
				mSelectedCharacter = entity;
				mState = State::StatsMenu;
				break;
			}
		}
	} else if (mActionMap.IsPressed(Action::Select)) {
		auto view = mRegistry.view<GridPosition, PlayerTeam>();
		for (auto entity : view) {
			const auto& gridPosition = view.get<GridPosition>(entity);
			if (gridPosition.cell.x == mCursorPos.x && gridPosition.cell.y == mCursorPos.y) {
				mSelectedCharacter = entity;
				mState = State::MovementZone;
				break;
			}
		}
	}
}

void Game::UpdateMovementZoneState(const float elapsedTime) {
	UpdateCursor(elapsedTime);
	FocusCamera(mCursorPos);

	if (mActionMap.IsPressed(Action::Select)) {
		auto& selectedCharPos = mRegistry.get<GridPosition>(mSelectedCharacter);
		const auto& stats = mRegistry.get<Stats>(mSelectedCharacter);
		const auto tiles = mTilePathing.FindMovementZone(
			selectedCharPos.cell.y,
			selectedCharPos.cell.x,
			stats.movement);
		for (const TilePathing::Cell& cell : tiles) {
			if (cell.col == mCursorPos.x && cell.row == mCursorPos.y) {
				bool isOccupied = false;
				auto positionedEntities = mRegistry.view<GridPosition>();
				for (const auto entity : positionedEntities) {
					if (entity == mSelectedCharacter) {
						continue;
					}

					const auto& gridPosition = positionedEntities.get<GridPosition>(entity);
					if (gridPosition.cell.x == mCursorPos.x && gridPosition.cell.y == mCursorPos.y) {
						isOccupied = true;
						break;
					}
				}

				if (!isOccupied) {
					mPrevSelectedCharacterCell = selectedCharPos.cell;
					selectedCharPos.cell = { mCursorPos.x, mCursorPos.y };
					mCurSelection = 0;
					mState = State::ActionSelectMenu;
					return; // Exit early to stay in ActionSelectMenu state
				}
			}
		}

		mSelectedCharacter = entt::null;
		mState = State::SelectCharacter;
	} else if (mActionMap.IsPressed(Action::Cancel)) {
		mSelectedCharacter = entt::null;
		mState = State::SelectCharacter;
	}
}

void Game::UpdateMovingCharacterState(const float elapsedTime) {
	// TODO: Implement moving character state functionality.
}

void Game::UpdateActionSelectMenuState(const float elapsedTime) {
	if (mActionMap.IsPressed(Action::MoveUp)) {
		if (mCurSelection > 0) {
			--mCurSelection;
		}
	} else if (mActionMap.IsPressed(Action::MoveDown)) {
		if (mCurSelection < static_cast<uint8_t>(ActionMenuOption::Count) - 1) {
			++mCurSelection;
		}
	}

	if (mActionMap.IsPressed(Action::Select)) {
		switch (static_cast<ActionMenuOption>(mCurSelection)) {
		case ActionMenuOption::Attack:
			mState = State::ActionAttackSelectMenu;
			mCurSelection = 0;
			break;

		case ActionMenuOption::Item:
			mState = State::ActionItemSelectMenu;
			mCurSelection = 0;
			break;

		case ActionMenuOption::Wait:
		{
			LOG_INFO("Wait action selected");
			mState = State::SelectCharacter;
			mSelectedCharacter = entt::null;
		}
		break;

		default:
			break;
		}
	} else if (mActionMap.IsPressed(Action::Cancel)) {
		auto& selectedCharPos = mRegistry.get<GridPosition>(mSelectedCharacter);
		selectedCharPos.cell = mPrevSelectedCharacterCell;
		mState = State::MovementZone;
	}
}

void Game::UpdateActionAttackSelectMenuState(const float elapsedTime) {
	// TODO: Implement action attack select menu state functionality.
	const auto& attackerPos = mRegistry.get<GridPosition>(mSelectedCharacter);
	const auto& attackerStats = mRegistry.get<Stats>(mSelectedCharacter);
	auto& inventory = mRegistry.get<Inventory>(mSelectedCharacter);
	ItemInstance& equippedWeapon = inventory.items[inventory.equippedWeaponSlot];
	const ItemDef* const equippedWeaponDef = ItemDatabase::GetItemDef(equippedWeapon.id);

	const WeaponData* const equippedWeaponData = std::get_if<WeaponData>(&equippedWeaponDef->data);
	if (!equippedWeaponData) {
		LOG_ERROR("Equipped weapon id ({}) is not a valid weapon", equippedWeapon.id);
		return;
	}

	auto attackZone = mTilePathing.FindAttackZone(
		attackerPos.cell.y,
		attackerPos.cell.x,
		equippedWeaponData->minRange,
		equippedWeaponData->maxRange
	);
}

void Game::UpdateActionItemSelectMenuState(const float elapsedTime) {
	auto& inventory = mRegistry.get<Inventory>(mSelectedCharacter);

	if (mActionMap.IsPressed(Action::MoveUp)) {
		if (mCurSelection > 0) {
			--mCurSelection;
		}
	} else if (mActionMap.IsPressed(Action::MoveDown)) {
		if (mCurSelection < inventory.itemsCount - 1) {
			++mCurSelection;
		}
	}

	if (mActionMap.IsPressed(Action::Select)) {
		ItemInstance& selectedItem = inventory.items[mCurSelection];
		const ItemDef* const itemDef = ItemDatabase::GetItemDef(selectedItem.id);

		if (const ConsumableData* const consumableData = std::get_if<ConsumableData>(&itemDef->data)) {
			if (consumableData->effect == ConsumableData::Effect::Heal) {
				auto& stats = mRegistry.get<Stats>(mSelectedCharacter);
				stats.curHp += static_cast<int8_t>(consumableData->amount);
				if (stats.curHp > stats.maxHp)
					stats.curHp = stats.maxHp;
			}
			Inventory_ConsumeItemUse(inventory, mCurSelection);
		}

		mState = State::SelectCharacter;
		mSelectedCharacter = entt::null;
		mCurSelection = 0;
	} else if (mActionMap.IsPressed(Action::Cancel)) {
		mState = State::ActionSelectMenu;
		mCurSelection = static_cast<uint8_t>(ActionMenuOption::Item);
	}
}

void Game::UpdateStatsMenuState(const float elapsedTime) {
	if (mActionMap.IsPressed(Action::Cancel) || mActionMap.IsPressed(Action::OpenStatsMenu)) {
		mState = State::SelectCharacter;
		mSelectedCharacter = entt::null;
	}
}

void Game::Render() {
	Renderer2D::Clear(ClearFlags::Color | ClearFlags::Depth);

	Renderer2D::BeginScene(mCamera);

	const TileMap* const tileMap = TileSystem::GetTileMap(ToenTileMapHandle);

	Renderer2D::DrawTexturedQuad(
		{ mCursorPos.x * tileMap->tileWidth, mCursorPos.y * tileMap->tileHeight, 0.6f },
		{ tileMap->tileWidth, tileMap->tileHeight },
		{ 1.0f, 0.0f, 1.0f, 1.0f },
		SelectionRingTextureHandle);
	Renderer2D::DrawTileMap(mCamera, ToenTileMapHandle);

	mRegistry.view<GridPosition, Sprite>().each([tileMap](const auto& gridPosition, const auto& sprite) {
		Renderer2D::DrawTile(
			{ gridPosition.cell.x * tileMap->tileWidth, gridPosition.cell.y * tileMap->tileHeight, 0.5f },
			{ tileMap->tileWidth, tileMap->tileHeight },
			sprite.frameIndex,
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			sprite.tileSetHandle);
	});

	if (mSelectedCharacter != entt::null) {
		if (mState == State::MovementZone) {
			Renderer2D::Flush();
			mPlatform.renderDevice->EnableBlending(true);

			const auto& gridPosition = mRegistry.get<GridPosition>(mSelectedCharacter);
			const auto& stats = mRegistry.get<Stats>(mSelectedCharacter);

			const auto movementZone = mTilePathing.FindMovementZone(
				gridPosition.cell.y,
				gridPosition.cell.x,
				stats.movement);
			for (const TilePathing::Cell& cell : movementZone) {
				Renderer2D::DrawQuad(
					{ cell.col * tileMap->tileWidth, cell.row * tileMap->tileHeight, 0.3f },
					{ tileMap->tileWidth, tileMap->tileHeight },
					{ 0.0f, 0.0f, 0.75f, 0.5f });
			}

			const auto tilePath = mTilePathing.FindPath(
				gridPosition.cell.y,
				gridPosition.cell.x,
				mCursorPos.y,
				mCursorPos.x,
				stats.movement);
			for (const TilePathing::Cell& cell : tilePath) {
				Renderer2D::DrawQuad(
					{ cell.col * tileMap->tileWidth, cell.row * tileMap->tileHeight, 0.4f },
					{ tileMap->tileWidth, tileMap->tileHeight },
					{ 0.75f, 0.0f, 0.0f, 0.5f });
			}

			Renderer2D::Flush();
			mPlatform.renderDevice->EnableBlending(false);
		} else if (mState == State::StatsMenu) {
			RenderStatsMenu();
		} else if (mState == State::ActionSelectMenu) {
			RenderActionSelectMenu();
		} else if (mState == State::ActionAttackSelectMenu) {
			RenderActionAttackSelectMenu();
		} else if (mState == State::ActionItemSelectMenu) {
			RenderActionItemSelectMenu();
		}
	}

	Renderer2D::EndScene();

	Renderer2D::Present();
}

void Game::RenderStatsMenu() {
	Renderer2D::Flush();
	mPlatform.renderDevice->EnableBlending(true);

	Renderer2D::DrawQuad(
		{ 100.0f, 50.0f, 0.8f },
		{ 600.0f, 500.0f },
		{ 0.0f, 0.0f, 0.0f, 0.75f });

	char buffer[32] { 0 };
	constexpr std::array<StringHandle, 11> statNames = {
		static_cast<StringHandle>(Res::Strings::Id::MENU_LEVEL),
		static_cast<StringHandle>(Res::Strings::Id::MENU_EXP),
		static_cast<StringHandle>(Res::Strings::Id::MENU_HP),
		static_cast<StringHandle>(Res::Strings::Id::MENU_STRENGTH),
		static_cast<StringHandle>(Res::Strings::Id::MENU_MAGIC),
		static_cast<StringHandle>(Res::Strings::Id::MENU_SKILL),
		static_cast<StringHandle>(Res::Strings::Id::MENU_SPEED),
		static_cast<StringHandle>(Res::Strings::Id::MENU_LUCK),
		static_cast<StringHandle>(Res::Strings::Id::MENU_DEFENSE),
		static_cast<StringHandle>(Res::Strings::Id::MENU_RESISTANCE),
		static_cast<StringHandle>(Res::Strings::Id::MENU_MOVEMENT)
	};

	const auto& stats = mRegistry.get<Stats>(mSelectedCharacter);
	const auto& unitExp = mRegistry.get<UnitExperience>(mSelectedCharacter);
	const auto& inventory = mRegistry.get<Inventory>(mSelectedCharacter);
	const std::array<int, 11> statValues = {
		unitExp.level,
		unitExp.experience,
		stats.curHp,
		stats.strength,
		stats.magic,
		stats.skill,
		stats.speed,
		stats.luck,
		stats.defense,
		stats.resistance,
		stats.movement
	};

	for (size_t i = 0; i < statNames.size(); ++i) {
		const float y = 100.0f + i * 25.0f;
		const char* const strName = Localization::GetString(statNames[i]);
		snprintf(buffer, sizeof(buffer), "%s: %d", strName, statValues[i]);
		Renderer2D::DrawText(
			mFontHandle,
			buffer,
			{ 120.0f, y, 0.9f },
			16.0f,
			{1.0f, 1.0f, 1.0f, 1.0f});
	}

	for (size_t i = 0; i < inventory.items.size(); ++i) {
		const float y = 100.0f + i * 25.0f;
		const ItemInstance& item = inventory.items[i];
		if (item.id == InvalidItemHandle)
			continue;

		const ItemDef* const itemDef = ItemDatabase::GetItemDef(item.id);
		const char* const itemName = Localization::GetString(itemDef->nameHandle);
		uint8_t maxUses = 0;
		if (const WeaponData* const weaponData = std::get_if<WeaponData>(&itemDef->data))
			maxUses = weaponData->maxDurability;
		else if (const ConsumableData* const consumableData = std::get_if<ConsumableData>(&itemDef->data))
			maxUses = consumableData->maxUses;

		snprintf(buffer, sizeof(buffer), "%s: %d/%d", itemName, item.usesLeft, maxUses);
		Renderer2D::DrawText(
			mFontHandle,
			buffer,
			{ 320.0f, y, 0.9f },
			16.0f,
			{1.0f, 1.0f, 1.0f, 1.0f});
	}

	Renderer2D::Flush();
	mPlatform.renderDevice->EnableBlending(false);
}

void Game::RenderActionSelectMenu() {
	const TileMap* const tileMap = TileSystem::GetTileMap(ToenTileMapHandle);

	Renderer2D::Flush();
	mPlatform.renderDevice->EnableBlending(true);

	const auto& selectedCharPos = mRegistry.get<GridPosition>(mSelectedCharacter);

	const glm::vec2 actionMenuPos = {
		selectedCharPos.cell.x * tileMap->tileWidth + 16.0f,
		selectedCharPos.cell.y * tileMap->tileHeight
	};

	Renderer2D::DrawQuad(
		{ actionMenuPos.x, actionMenuPos.y, 0.7f },
		{ 64.0f, 80.0f },
		{ 0.0f, 0.0f, 0.0f, 0.75f });

	char buffer[32] {};
	constexpr std::array<StringHandle, static_cast<size_t>(ActionMenuOption::Count)> actionMenuOptions = {
		static_cast<StringHandle>(Res::Strings::Id::ACTION_MENU_ATTACK),
		static_cast<StringHandle>(Res::Strings::Id::ACTION_MENU_ITEM),
		static_cast<StringHandle>(Res::Strings::Id::ACTION_MENU_WAIT),
	};
	for (size_t i = 0; i < actionMenuOptions.size(); ++i) {
		const float y = actionMenuPos.y + i * 25.0f;
		const char* const optionText = Localization::GetString(actionMenuOptions[i]);
		snprintf(buffer, sizeof(buffer), "%s", optionText);
		Renderer2D::DrawText(
			mFontHandle,
			buffer,
			{ actionMenuPos.x + 8.0f, y, 0.8f },
			16.0f,
			{1.0f, 1.0f, 1.0f, 1.0f});
	}

	Renderer2D::DrawTexturedQuad(
		{ actionMenuPos.x, actionMenuPos.y + mCurSelection * 25.0f, 0.9f },
		{ 64.0f, 25.0f },
		{ 1.0f, 0.0f, 1.0f, 1.0f },
		SelectionRingTextureHandle);

	Renderer2D::Flush();
	mPlatform.renderDevice->EnableBlending(false);
}

void Game::RenderActionAttackSelectMenu() {
	const TileMap* const tileMap = TileSystem::GetTileMap(ToenTileMapHandle);

	const auto& attackerPos = mRegistry.get<GridPosition>(mSelectedCharacter);
	const auto& inventory = mRegistry.get<Inventory>(mSelectedCharacter);
	const ItemInstance equippedWeapon = inventory.items[inventory.equippedWeaponSlot];
	const ItemDef* const equippedWeaponDef = ItemDatabase::GetItemDef(equippedWeapon.id);

	const WeaponData* const equippedWeaponData = std::get_if<WeaponData>(&equippedWeaponDef->data);
	if (!equippedWeaponData) {
		LOG_ERROR("Equipped weapon id ({}) is not a valid weapon", equippedWeapon.id);
		return;
	}

	const auto attackZone = mTilePathing.FindAttackZone(
		attackerPos.cell.y,
		attackerPos.cell.x,
		equippedWeaponData->minRange,
		equippedWeaponData->maxRange
	);

	Renderer2D::Flush();
	mPlatform.renderDevice->EnableBlending(true);

	for (const TilePathing::Cell& cell : attackZone) {
		Renderer2D::DrawQuad(
			{ cell.col * tileMap->tileWidth, cell.row * tileMap->tileHeight, 0.3f },
			{ tileMap->tileWidth, tileMap->tileHeight },
			{ 1.0f, 0.0f, 0.0f, 0.5f });
	}

	Renderer2D::Flush();
	mPlatform.renderDevice->EnableBlending(false);
}

void Game::RenderActionItemSelectMenu() {
	constexpr float BoxWidth = 150.0f;
	const TileMap* const tileMap = TileSystem::GetTileMap(ToenTileMapHandle);

	Renderer2D::Flush();
	mPlatform.renderDevice->EnableBlending(true);

	const auto& selectedCharPos = mRegistry.get<GridPosition>(mSelectedCharacter);
	const auto& inventory = mRegistry.get<Inventory>(mSelectedCharacter);

	const glm::vec2 actionMenuPos = {
		selectedCharPos.cell.x * tileMap->tileWidth + 16.0f,
		selectedCharPos.cell.y * tileMap->tileHeight
	};

	Renderer2D::DrawQuad(
		{ actionMenuPos.x, actionMenuPos.y, 0.7f },
		{ BoxWidth, 80.0f },
		{ 0.0f, 0.0f, 0.0f, 0.75f }
	);

	char buffer[32] {};
	for (uint8_t i = 0; i < inventory.itemsCount; ++i) {
		const auto& item = inventory.items[i];
		const ItemDef* const itemDef = ItemDatabase::GetItemDef(item.id);
		const char* const itemName = Localization::GetString(itemDef->nameHandle);
		const float y = actionMenuPos.y + i * 25.0f;

		uint8_t maxUses = 0;
		if (const WeaponData* const weaponData = std::get_if<WeaponData>(&itemDef->data))
			maxUses = weaponData->maxDurability;
		else if (const ConsumableData* const consumableData = std::get_if<ConsumableData>(&itemDef->data))
			maxUses = consumableData->maxUses;

		snprintf(buffer, sizeof(buffer), "%s %d/%d", itemName, item.usesLeft, maxUses);

		Renderer2D::DrawText(
			mFontHandle,
			buffer,
			{ actionMenuPos.x + 8.0f, y, 0.8f },
			16.0f,
			{1.0f, 1.0f, 1.0f, 1.0f});
	}

	Renderer2D::DrawTexturedQuad(
		{ actionMenuPos.x, actionMenuPos.y + mCurSelection * 25.0f, 0.9f },
		{ BoxWidth, 25.0f },
		{ 1.0f, 0.0f, 1.0f, 1.0f },
		SelectionRingTextureHandle);

	Renderer2D::Flush();
	mPlatform.renderDevice->EnableBlending(false);
}

void Game::OnResize(uint16_t width, uint16_t height) {
	mPlatform.renderDevice->SetViewport(0, 0, width, height);
}

void Game::Close() {
	const std::filesystem::path prefPath = mPlatform.GetPreferencesPath(PrefPathOrgName, PrefPathAppName);
	mActionMap.SaveBindings(prefPath / InputBindingsFile);
	mIsRunning = false;
}

void Game::UpdateCursor(const float elapsedTime) {
	const TileMap* const tileMap = TileSystem::GetTileMap(ToenTileMapHandle);

	mTimeSinceKeyPressed += elapsedTime;

	if (mTimeSinceKeyPressed > KeyDownInterval) {
		if (mActionMap.IsDown(Action::MoveUp)) {
			if (mCursorPos.y > 0)
				mCursorPos.y--;
			mTimeSinceKeyPressed = 0.0f;
		} else if (mActionMap.IsDown(Action::MoveDown)) {
			if (mCursorPos.y < tileMap->tileRowCount - 1)
				mCursorPos.y++;
			mTimeSinceKeyPressed = 0.0f;
		} else if (mActionMap.IsDown(Action::MoveLeft)) {
			if (mCursorPos.x > 0)
				mCursorPos.x--;
			mTimeSinceKeyPressed = 0.0f;
		} else if (mActionMap.IsDown(Action::MoveRight)) {
			if (mCursorPos.x < tileMap->tileColumnCount - 1)
				mCursorPos.x++;
			mTimeSinceKeyPressed = 0.0f;
		}
	}
}

void Game::FocusCamera(const glm::ivec2& cell) {
	const TileMap* const tileMap = TileSystem::GetTileMap(ToenTileMapHandle);

	const glm::vec2 worldEndPos = {
		tileMap->tileColumnCount * tileMap->tileWidth,
		tileMap->tileRowCount * tileMap->tileHeight
	};

	mCamera.position = {
		cell.x * tileMap->tileWidth - mCamera.width / 2.0f,
		cell.y * tileMap->tileHeight - mCamera.height / 2.0f
	};

	if (mCamera.position.x < 0.0f)
		mCamera.position.x = 0.0f;
	else if (mCamera.position.x + mCamera.width > worldEndPos.x)
		mCamera.position.x = worldEndPos.x - mCamera.width;

	if (mCamera.position.y < 0.0f)
		mCamera.position.y = 0.0f;
	else if (mCamera.position.y + mCamera.height > worldEndPos.y)
		mCamera.position.y = worldEndPos.y - mCamera.height;
}
