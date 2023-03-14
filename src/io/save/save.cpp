/**
 * Canary - A free and open-source MMORPG server emulator
 * Copyright (©) 2019-2022 OpenTibiaBR <opentibiabr@outlook.com>
 * Repository: https://github.com/opentibiabr/canary
 * License: https://github.com/opentibiabr/canary/blob/main/LICENSE
 * Contributors: https://github.com/opentibiabr/canary/graphs/contributors
 * Website: https://docs.opentibiabr.com/
 */

#include "pch.hpp"

#include "io/save/save.hpp"
#include "io/iologindata.h"
#include "game/game.h"
#include "creatures/players/player.h"
#include "game/scheduling/scheduler.h"

playerHashSet Save::playersToSaveList;
std::atomic_bool Save::savePlayerInProgress = false;
std::atomic_bool Save::saveHouseInProgress = false;
std::jthread Save::savePlayerThread;
std::jthread Save::saveHouseThread;
std::mutex Save::listPlayerMutex;
std::mutex Save::savePlayerMutex;
std::mutex Save::saveHouseMutex;
std::mutex Save::eraseListMutex;

bool Save::savePlayerAsync(Player* player) {
	if (player != nullptr) {
		playersToSaveList.emplace(player);
	}

	if (!savePlayerThread.joinable() && !savePlayerInProgress) {
		savePlayerInProgress = true;
		savePlayerThread = std::jthread([player]() {
			while (!playersToSaveList.empty() && player != nullptr) {
				std::unique_lock<std::mutex> savePlayerLock(savePlayerMutex);
				Player* currentPlayer = *playersToSaveList.begin();

				if (currentPlayer != nullptr) {
					IOLoginData::savePlayer(currentPlayer);
				}

				playersToSaveList.erase(currentPlayer);
				savePlayerLock.unlock();
			}

			std::unique_lock<std::mutex> savePlayerLock(savePlayerMutex);
			for (const auto &it : Game().guilds) {
				IOGuild::saveGuild(it.second);
			}
			savePlayerLock.unlock();
			savePlayerInProgress = false;
		});
		savePlayerThread.detach();
	}
	return true;
}

bool Save::saveHouseAsync(bool saveHouse) {
	if (!saveHouseThread.joinable() && !saveHouseInProgress) {
		saveHouseInProgress = true;
		saveHouseThread = std::jthread([&saveHouse]() {
			std::unique_lock<std::mutex> saveHouseLock(saveHouseMutex);
			while (saveHouse) {
				if (!Map::save()) {
					SPDLOG_WARN("Error while saving house");
				}
				saveHouse = false;
			}
			saveHouseLock.unlock();
			saveHouseInProgress = false;
		});
		saveHouseThread.detach();
	}
	return true;
}

void Save::removePlayerFromSaveQueue(Player* player) {
	if (!player) {
		return;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::lock_guard<std::mutex> eraseLock(eraseListMutex);
	playersToSaveList.erase(player);
}
