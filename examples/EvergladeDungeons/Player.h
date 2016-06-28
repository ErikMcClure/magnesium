// Copyright �2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Everglade.h

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Everglade.h"
#include "bss-util/cBitField.h"

enum PLAYER_STATS
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
  PLAYER_STATS_NUM,
};

enum PLAYER_STATE
{
  PLAYERSTATE_STAND = 0,
  PLAYERSTATE_WALK = 1,
  PLAYERSTATE_RUN = 2,
  PLAYERSTATE_DASH = 3,
  PLAYERSTATE_MOVEGROUP = 0b11,
};

enum PLAYER_STATUS
{
  STATUS_STUNNED = 0,
  STATUS_SLOWED,
  STATUS_POISON,
  STATUS_SILENCED,
  STATUS_NUM
};

struct Player
{
  GValue stats[PLAYER_STATS_NUM];
  bss_util::cBitField<uint64_t> state;
  std::pair<double, double> status[STATUS_NUM];
  double health;
  double shield;
  double cooldown;
  double bleedout;
  uint64_t xp;
  uint64_t level;
};

#endif
