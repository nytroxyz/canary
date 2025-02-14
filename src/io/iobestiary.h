/**
 * Canary - A free and open-source MMORPG server emulator
 * Copyright (©) 2019-2022 OpenTibiaBR <opentibiabr@outlook.com>
 * Repository: https://github.com/opentibiabr/canary
 * License: https://github.com/opentibiabr/canary/blob/main/LICENSE
 * Contributors: https://github.com/opentibiabr/canary/graphs/contributors
 * Website: https://docs.opentibiabr.com/
 */

#ifndef SRC_IO_IOBESTIARY_H_
#define SRC_IO_IOBESTIARY_H_

#include "declarations.hpp"
#include "lua/scripts/luascript.h"
#include "creatures/players/player.h"

class Game;

class Charm {
	public:
		Charm() = default;
		Charm(std::string initname, charmRune_t initcharmRune_t, std::string initdescription, charm_t inittype, uint16_t initpoints, int32_t initbinary) :
			name(initname), id(initcharmRune_t), description(initdescription), type(inittype), points(initpoints), binary(initbinary) { }
		virtual ~Charm() = default;

		std::string name;
		std::string cancelMsg;
		std::string logMsg;
		std::string description;

		charm_t type;
		charmRune_t id = CHARM_NONE;
		CombatType_t dmgtype = COMBAT_NONE;
		uint8_t effect = CONST_ME_NONE;

		SoundEffect_t soundImpactEffect = SoundEffect_t::SILENCE;
		SoundEffect_t soundCastEffect = SoundEffect_t::SILENCE;

		int8_t percent = 0;
		int8_t chance = 0;
		uint16_t points = 0;
		int32_t binary = 0;
};

class IOBestiary {
	public:
		IOBestiary() = default;

		// non-copyable
		IOBestiary(const IOBestiary &) = delete;
		void operator=(const IOBestiary &) = delete;

		static IOBestiary &getInstance() {
			// Guaranteed to be destroyed
			static IOBestiary instance;
			// Instantiated on first use
			return instance;
		}

		Charm* getBestiaryCharm(charmRune_t activeCharm, bool force = false);
		void addBestiaryKill(Player* player, MonsterType* mtype, uint32_t amount = 1);
		bool parseCharmCombat(Charm* charm, Player* player, Creature* target, int64_t realDamage);
		void addCharmPoints(Player* player, uint16_t amount, bool negative = false);
		void sendBuyCharmRune(Player* player, charmRune_t runeID, uint8_t action, uint16_t raceid);
		void setCharmRuneCreature(Player* player, Charm* charm, uint16_t raceid);
		void resetCharmRuneCreature(Player* player, Charm* charm);

		int8_t calculateDifficult(uint32_t chance) const;
		uint8_t getKillStatus(MonsterType* mtype, uint32_t killAmount) const;

		uint16_t getBestiaryRaceUnlocked(Player* player, BestiaryType_t race) const;

		int32_t bitToggle(int32_t input, Charm* charm, bool on) const;

		bool hasCharmUnlockedRuneBit(Charm* charm, int32_t input) const;

		std::list<charmRune_t> getCharmUsedRuneBitAll(Player* player);
		std::list<uint16_t> getBestiaryFinished(Player* player) const;

		charmRune_t getCharmFromTarget(Player* player, MonsterType* mtype);

		std::map<uint16_t, uint32_t> getBestiaryKillCountByMonsterIDs(Player* player, std::map<uint16_t, std::string> mtype_list) const;
		std::map<uint8_t, int16_t> getMonsterElements(MonsterType* mtype) const;
		std::map<uint16_t, std::string> findRaceByName(const std::string &race, bool Onlystring = true, BestiaryType_t raceNumber = BESTY_RACE_NONE) const;
};

constexpr auto g_iobestiary = &IOBestiary::getInstance;

#endif // SRC_IO_IOBESTIARY_H_
