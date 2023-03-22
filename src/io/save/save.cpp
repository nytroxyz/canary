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
std::shared_mutex Save::listPlayerMutex;
std::shared_mutex Save::eraseListMutex;
std::shared_mutex Save::savePlayerMutex;
std::mutex Save::saveHouseMutex;

bool Save::savePlayerAsync(Player* player) {
	if (player != nullptr) {
		std::unique_lock<std::shared_mutex> addListLock(listPlayerMutex);
		playersToSaveList.emplace(player);
		addListLock.unlock();
	}

	if (!savePlayerThread.joinable() && !savePlayerInProgress) {
		savePlayerInProgress = true;
		savePlayerThread = std::jthread([player]() {
			while (!playersToSaveList.empty() && player != nullptr) {
				Player* currentPlayer = *playersToSaveList.begin();
				if (currentPlayer != nullptr) {
					bool saved = false;
					for (uint32_t tries = 0; tries < 3; ++tries) {
						if (IOLoginData::savePlayer(currentPlayer)) {
							saved = true;
							break;
						}
					}

					if (!saved) {
						SPDLOG_WARN("Error while saving player: {}", currentPlayer->getName());
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(10000));
					std::unique_lock<std::shared_mutex> eraseList(eraseListMutex);
					phmap::erase_if(playersToSaveList, [currentPlayer](const Player* p) { return p == currentPlayer; });
					eraseList.unlock();
				}
			}

			for (const auto &it : Game().guilds) {
				IOGuild::saveGuild(it.second);
			}

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
	std::unique_lock<std::shared_mutex> eraseLock(eraseListMutex);
	phmap::erase_if(playersToSaveList, [player](const Player* p) { return p == player; });
	eraseLock.unlock();
}

bool Save::checkIfPlayerInList(Player* player) {
	bool playerInListSave = false;
	if (!Save::playersToSaveList.empty()) {
		std::shared_lock<std::shared_mutex> lock(Save::listPlayerMutex);
		for (auto it = Save::playersToSaveList.begin(); it != Save::playersToSaveList.end(); ++it) {
			if (*it == player) {
				playerInListSave = true;
				break;
			}
		}
		lock.unlock();
	}
	return playerInListSave;
}
