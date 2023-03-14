/**
 * Canary - A free and open-source MMORPG server emulator
 * Copyright (©) 2019-2022 OpenTibiaBR <opentibiabr@outlook.com>
 * Repository: https://github.com/opentibiabr/canary
 * License: https://github.com/opentibiabr/canary/blob/main/LICENSE
 * Contributors: https://github.com/opentibiabr/canary/graphs/contributors
 * Website: https://docs.opentibiabr.com/
 */

#ifndef SRC_IO_SAVE_SAVE_HPP_
#define SRC_IO_SAVE_SAVE_HPP_

#include "creatures/players/player.h"

using playerHashSet = phmap::flat_hash_set<Player*>;

class Save {
	public:
		static bool savePlayerAsync(Player* player);
		static bool saveHouseAsync(bool saveHouse);
		static void removePlayerFromSaveQueue(Player* player);

	private:
		static playerHashSet playersToSaveList;
		static std::atomic_bool savePlayerInProgress;
		static std::atomic_bool saveHouseInProgress;
		static std::jthread savePlayerThread;
		static std::jthread saveHouseThread;
		static std::mutex listPlayerMutex;
		static std::mutex savePlayerMutex;
		static std::mutex saveHouseMutex;
		static std::mutex eraseListMutex;
};

#endif // SRC_IO_SAVE_SAVE_HPP_