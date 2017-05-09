// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Everglade.h

#ifndef __DATA_H__
#define __DATA_H__

#include "Everglade.h"
#include "bss-util/Str.h"
#include "bss-util/DynArray.h"
#include "bss-util/Map.h"

enum CHARACTER_STATS : uint16_t
{
  PLAYER_STRENGTH,
  PLAYER_DEXTERITY,
  PLAYER_WISDOM,
  PLAYER_VITALITY,
  PLAYER_MAXHEALTH,
  PLAYER_MAXSHIELD,
  PLAYER_HEALTHREGEN,
  PLAYER_SHIELDREGEN,
  PLAYER_ARMOR,
  PLAYER_MAGICRESIST,
  PLAYER_DAMAGE,
  PLAYER_MAGICDAMAGE,
  PLAYER_BLEED,
  PLAYER_BLEEDTIME,
  PLAYER_ARMORPIERCE,
  PLAYER_MAGICPEN,
  PLAYER_MOVESPEED,
  PLAYER_STAMINA,
  PLAYER_MAXBLEEDOUT,
  PLAYER_ATTACKSPEED,
  PLAYER_RELOADSPEED,
  PLAYER_DODGECHANCE,
  PLAYER_HEALTHPERKILL,
  PLAYER_HEALTHPERHIT,
  PLAYER_LIFESTEAL,
  PLAYER_ITEMFIND,
  PLAYER_GOLDFIND,
  PLAYER_XPBONUS,
  PLAYER_XPBONUSPERKILL,
  CHAR_STATS_NUM,
};

enum ITEM_TYPE : char {
  ITEM_MISC = 0, // rings and amulets
  ITEM_ARMOR, // All armor, requires DEFENSE stat.
  ITEM_MELEE,
  ITEM_RANGED,
  ITEM_WAND,
  ITEM_SHIELD,
  ITEM_SECONDARY, // Any non functional "shield" items that don't actually do anything but get put in the shield slot anyway.
};

enum ITEM_SUBTYPE : char {
  ITEM_NOSUBTYPE = 0,
  ITEM_ONEHANDED,
  ITEM_TWOHANDED,
  ITEM_SECONDARY_ONLY,
  ITEM_HEAD,
  ITEM_HANDS,
  ITEM_FEET,
  ITEM_LEGS,
  ITEM_CHEST,
  //ITEM_BELT, // These additional armor locations can be enabled if we feel like we need them
  //ITEM_SHOULDER,
  //ITEM_ARMS,
};

enum ITEM_RARITY : char {
  ITEM_COMMON = 0,
  ITEM_UNCOMMON,
  ITEM_RARE,
  ITEM_EXTRAORDINARY,
  ITEM_ANCIENT,
  ITEM_MYTHICAL,
  ITEM_EPIC,
  ITEM_LEGENDARY
};

struct Item {
  ITEM_TYPE Type;
  ITEM_SUBTYPE Subtype;
  ITEM_RARITY Rarity;
  bss::Map<CHARACTER_STATS, double> Stats;
  bss::Map<CHARACTER_STATS, double> Requirements;
};

enum RACES { // TODO: replace with actual races
  RACE_PONY = 0, // Playable races may also be enemies
  RACE_UNICORN,
  RACE_PEGASUS,
  RACE_LEMMING,
  RACE_CHANGELING, // Enemy only races (some smaller bosses may be included here)
};

enum AI_TYPE : char {
  AI_NONE = 0,
  AI_BASIC,
  AI_SWARM,
  AI_TACTICS
};

enum CHAR_STATE : uint64_t
{
  STATE_STAND = 0,
  STATE_WALK = 1,
  STATE_RUN = 2,
  STATE_DASH = 3,
  STATE_BLEEDOUT = 4, // Character is immobile and dying on the ground.
  STATE_FLUNG = 5, // "flung" means being hit by an attack or explosion that pushes the character backwards
  STATE_PRONE = 6, // Prone is used after being flung while a character is on the ground but hasn't gotten up yet.
  STATE_RECOVERY = 7, // Recovery is used when a character is getting up after being flung or recovering from bleedout.
  STATE_MOVEGROUP = 0b111,
};

enum CHAR_STATUS : char
{
  STATUS_STUNNED = 0,
  STATUS_SLOWED,
  STATUS_POISONED,
  STATUS_BURNED,
  STATUS_BLINDED,
  STATUS_SILENCED,
  STATUS_NUM
};

enum SKILLS : uint16_t {
  SKILL_TANK = 0,
  NUM_SKILLS
};

struct Character {
  RACES Race;
  AI_TYPE AI;
  double Stamina;
  uint64_t Exp; // How much experience a player has, or the base experience used to determine how much XP an enemy is worth.
  uint64_t Level;
  uint64_t Health; // Current health
  uint64_t Shield; // Current shield value
  double Stats[CHAR_STATS_NUM];
  double Base[4]; // Base Str, Dex, Wis, Vit stats as determined by the monster or the player's stat distribution
  bss::Map<SKILLS, uint64_t> Skills;
  bss::DynArray<Item> Equipment;
  bss::BitField<uint64_t> state;
  std::pair<double, double> status[STATUS_NUM];
};

struct Build {
  double Base[4];
  bss::Map<SKILLS, uint64_t> Skills;
};

struct Player : Character {
  uint64_t Bleedout; // If player state is bleedout, number of nanoseconds until they die.
  bss::DynArray<Item> Inventory;
  bss::DynArray<Build> Builds;
};

struct Enemy : Character {

};

struct ConnectionInfo {
  uint32_t ipv4;
  uint32_t ipv6[4];
  uint16_t port;
};

struct User {
  enum USER_STATUS : char {
    OFFLINE = 0,
    ONLINE,
    AWAY,
    IN_DUNGEON,
    IN_TOWN,
    DO_NOT_DISTURB,
  };

  uint64_t ID;
  Str Username;
  Str Message; // Status message
  bss::DynArray<uint64_t> Friends;
  USER_STATUS Status;
  bss::DynArray<Player, uint8_t> Characters;
  uint8_t CurrentCharacter; // Index of current character being played
  ConnectionInfo Connection;
};

#endif
